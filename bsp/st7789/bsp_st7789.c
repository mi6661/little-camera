#include "bsp_st7789.h"

static uint16_t BUFA[20*240] = {0};
static uint16_t BUFB[20*240] = {0};
volatile uint8_t dma_isbusy = 0;
static SPI_InitTypeDef sconf;

void ST_GPIO_Init(){
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
    GPIO_InitTypeDef sck_mosi;
    sck_mosi.GPIO_Mode = GPIO_Mode_AF;
    sck_mosi.GPIO_OType = GPIO_OType_PP;
    sck_mosi.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    sck_mosi.GPIO_PuPd = GPIO_PuPd_NOPULL;
    sck_mosi.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_SPI1);
    GPIO_Init(GPIOA,&sck_mosi);

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
    GPIO_InitTypeDef res_dc;
    res_dc.GPIO_Mode = GPIO_Mode_OUT;
    res_dc.GPIO_OType = GPIO_OType_PP;
    res_dc.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_12;
    res_dc.GPIO_PuPd = GPIO_PuPd_NOPULL;
    res_dc.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOD,&res_dc);

    GPIO_InitTypeDef cs;
    cs.GPIO_Mode = GPIO_Mode_OUT;
    cs.GPIO_OType = GPIO_OType_PP;
    cs.GPIO_Pin = GPIO_Pin_4;
    cs.GPIO_PuPd = GPIO_PuPd_NOPULL;
    cs.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA,&cs);

    //硬件复位
    GPIO_ResetBits(GPIOD,GPIO_Pin_10);
    delay_ms(20);
    GPIO_SetBits(GPIOD,GPIO_Pin_10);
    delay_ms(200);
}

void ST_SPI_Init(){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
    sconf.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    sconf.SPI_CPHA = SPI_CPHA_1Edge;
    sconf.SPI_CPOL = SPI_CPOL_Low;
    sconf.SPI_CRCPolynomial = 7;
    sconf.SPI_DataSize  = SPI_DataSize_8b;
    sconf.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    sconf.SPI_FirstBit = SPI_FirstBit_MSB;
    sconf.SPI_Mode = SPI_Mode_Master;
    sconf.SPI_NSS = SPI_NSS_Soft;
    SPI_Init(SPI1,&sconf);
    SPI_Cmd(SPI1,ENABLE);
}

void ST_DMA_Init(){
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);
    DMA_InitTypeDef conf;
    conf.DMA_BufferSize = 0;
    conf.DMA_Channel = DMA_Channel_3;
    conf.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    conf.DMA_FIFOMode = DMA_FIFOMode_Disable;
    conf.DMA_Memory0BaseAddr =  BUFA;//后续使用时再设置具体地址
    conf.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    conf.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    conf.DMA_MemoryInc = DMA_MemoryInc_Enable;
    conf.DMA_Mode = DMA_Mode_Normal;
    conf.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;
    conf.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    conf.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    conf.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    conf.DMA_Priority = DMA_Priority_High;
    DMA_Init(DMA2_Stream3,&conf);

    NVIC_InitTypeDef nvic;
    nvic.NVIC_IRQChannel = DMA2_Stream3_IRQn;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 1;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    NVIC_Init(&nvic);
    
    //开启中断
    DMA_ITConfig(DMA2_Stream3,DMA_IT_TC,ENABLE);
    DMA_Cmd(DMA2_Stream3,DISABLE);
}

void DMA2_Stream3_IRQHandler(){
    //传输完成
    SPI_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,DISABLE);
    dma_isbusy = 0;
    DMA_ClearITPendingBit(DMA2_Stream3,DMA_IT_TCIF3);
}


void ST7789_Init(){
    ST_GPIO_Init();
    ST_SPI_Init();
    ST_DMA_Init();
}


void WaitIdle(){
    while(SPI_GetFlagStatus(SPI1,SPI_FLAG_TXE) == RESET);
    while(SPI_GetFlagStatus(SPI1,SPI_FLAG_BSY) == SET);
}

//传统基本函数

void ST_SendByte(uint8_t data){
    ST7789_CSE();
    while(SPI_GetFlagStatus(SPI1,SPI_FLAG_TXE)==RESET);
    SPI_SendData(SPI1,data);
    while(SPI_GetFlagStatus(SPI1,SPI_FLAG_BSY)==SET);
    ST7789_CSD();
}

void ST_SendCmd(uint8_t cmd,uint8_t argc,...){
    //开启命令模式
    ST7789_CMode();
    ST_SendByte(cmd);
    if(argc < 1) return;
    //开启数据模式
    WaitIdle();
    ST7789_DMode();
    va_list args;
    va_start(args,argc);
    for(int i=0;i<argc;i++){
        uint8_t data = va_arg(args,int);
        ST_SendByte(data);
    }
    va_end(args);
}

void ST_HardRET(){
    //硬件复位操作
    GPIO_ResetBits(GPIOD,GPIO_Pin_10);
    delay_ms(20);
    GPIO_SetBits(GPIOD,GPIO_Pin_10);
    delay_ms(200);
}

void ST_DisplayArea(uint16_t x,uint16_t y,uint16_t w,uint16_t h){
    //设置列范围
    ST_SendCmd(ST7789_CMD_CASET,4,((x)>>8)&0xFF,(x)&0xFF,((x+w)>>8)&0xFF,(x+w)&0xFF);
    //设置行范围
    ST_SendCmd(ST7789_CMD_RASET,4,((y)>>8)&0xFF,(y)&0xFF,((y+h)>>8)&0xFF,(y+h)&0xFF);
}

//DMA基本函数
void Send_Block_Blocking(uint16_t *data,uint16_t len){
    while(dma_isbusy);
    dma_isbusy = 1;
    DMA_Cmd(DMA2_Stream3,DISABLE);
    DMA_ClearFlag(DMA2_Stream3,DMA_FLAG_TCIF3);

    DMA2_Stream3->NDTR = len;
    DMA2_Stream3->M0AR = (uint32_t)data;

    DMA_Cmd(DMA2_Stream3,ENABLE);
    SPI_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE);
}

void Send_Block_NonBlocking(uint16_t *data,uint16_t len){
    if(dma_isbusy)return;
    dma_isbusy = 1;
    DMA_Cmd(DMA2_Stream3,DISABLE);
    DMA_ClearFlag(DMA2_Stream3,DMA_FLAG_TCIF3);

    DMA2_Stream3->NDTR = len;
    DMA2_Stream3->M0AR = (uint32_t)data;

    DMA_Cmd(DMA2_Stream3,ENABLE);
    SPI_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE);
}

//其他函数
void ClearBuf(uint16_t *buf,uint16_t color){
    for(uint16_t i=0;i<20;i++){
        for(uint16_t j=0;j<240;j++){
            buf[i*240+j] = color;
        }
    }
}

void ST7789_Debug(){
    ST_SendCmd(ST7789_CMD_SWRESET,0); //软件复位 
    delay_ms(200);  
    ST_SendCmd(ST7789_CMD_SLPOUT,0);   //退出睡眠模式
    delay_ms(200);
    ST_SendCmd(ST7789_CMD_NORON,0);    //全局显示模式
    ST_SendCmd(ST7789_CMD_MADCTL,1,0x00);//内存控制方式（默认模式）

    ST_SendCmd(ST7789_CMD_DISPON,0);//开启显示
    delay_ms(200);

    //设置显示范围
    ST_DisplayArea(0,0,239,319);
    //设置像素格式
    ST_SendCmd(ST7789_CMD_COLMOD,1,0x55);

    //开启显示
    ST_SendCmd(ST7789_CMD_RAMWR,0);
    WaitIdle();
    //开始使用DMA
    ST7789_DMode();
    SPI_Cmd(SPI1,DISABLE);
    sconf.SPI_DataSize = SPI_DataSize_16b;
    SPI_Init(SPI1,&sconf);
    SPI_Cmd(SPI1,ENABLE);
    ClearBuf(BUFA,0b1111100000000000);
    ClearBuf(BUFB,0b0000000000011111);
    ST7789_CSE();
    while(1){
        for(int i=0;i<16;i++)
            Send_Block_Blocking(BUFA,240*20);
        for(int i=0;i<16;i++)
            Send_Block_Blocking(BUFB,240*20);
    }
}