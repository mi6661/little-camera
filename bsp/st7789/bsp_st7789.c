#include "bsp_st7789.h"

static uint16_t BUFA[20*240] = {0};
static uint16_t BUFB[20*240] = {0};
volatile static uint8_t dma_isbusy = 0;
volatile static uint8_t spi_usedma = 0;


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
    ST7789_CSD();
}

void ST_SPI_Init(){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
    sconf.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    sconf.SPI_CPHA = SPI_CPHA_1Edge;
    sconf.SPI_CPOL = SPI_CPOL_Low;
    sconf.SPI_CRCPolynomial = 7;
    sconf.SPI_DataSize  = SPI_DataSize_8b;
    sconf.SPI_Direction = SPI_Direction_1Line_Tx;
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
    conf.DMA_Memory0BaseAddr =  (uint32_t)BUFA;//后续使用时再设置具体地址
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
    DMA_ClearITPendingBit(DMA2_Stream3,DMA_IT_TCIF3);
    SPI_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,DISABLE);
    dma_isbusy = 0;
}

void ST7789_Init(){
    ST_GPIO_Init();
    ST_SPI_Init();
    ST_DMA_Init();
}

//等待SPI1空闲
void WaitIdle(){
    while(SPI_GetFlagStatus(SPI1,SPI_FLAG_TXE) == RESET);
    while(SPI_GetFlagStatus(SPI1,SPI_FLAG_BSY) == SET);
}

void ST_HardRET(){
    //硬件复位操作
    GPIO_ResetBits(GPIOD,GPIO_Pin_10);
    delay_ms(20);
    GPIO_SetBits(GPIOD,GPIO_Pin_10);
    delay_ms(200);
}


//SPI基本函数
void ST_SPI_TRAMode(){
    if(spi_usedma == 0) return;
    spi_usedma = 0;
    //关闭DMA
    while(dma_isbusy);
    dma_isbusy = 1;
    while(SPI_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY));

    //SPI1切换为字节模式
    SPI_Cmd(SPI1,DISABLE);
    sconf.SPI_DataSize = SPI_DataSize_8b;
    SPI_Init(SPI1,&sconf);
    SPI_Cmd(SPI1,ENABLE);
}

void ST_SPI_DMAMode(){
    if(spi_usedma == 1) return;
    spi_usedma = 1;
    //SPI1切换为半字模式
    WaitIdle();

    SPI_Cmd(SPI1,DISABLE);
    sconf.SPI_DataSize = SPI_DataSize_16b;
    SPI_Init(SPI1,&sconf);
    SPI_Cmd(SPI1,ENABLE);

    //使能DMA
    dma_isbusy = 0;
}

void ST_SendByte(uint8_t data){
    ST7789_CSE();
    while(SPI_GetFlagStatus(SPI1,SPI_FLAG_TXE)==RESET);
    SPI_SendData(SPI1,data);
    while(SPI_GetFlagStatus(SPI1,SPI_FLAG_BSY)==SET);
    delay_ms(1);
    ST7789_CSD();
}

void ST_SendCmd(uint8_t cmd,uint8_t argc,...){
    ST_SPI_TRAMode();
    //开启命令模式
    ST7789_CMode();
    ST_SendByte(cmd);
    if(argc < 1) return;
    //开启数据模式
    ST7789_DMode();
    va_list args;
    va_start(args,argc);
    for(int i=0;i<argc;i++){
        uint8_t data = va_arg(args,int);
        ST_SendByte(data);
    }
    va_end(args);
}

void Send_Block_Blocking(uint16_t *data,uint16_t len){
    while(dma_isbusy);
    dma_isbusy = 1;
    while(SPI_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY));//DMA完成，不意味着SPI传输结束。

    DMA_Cmd(DMA2_Stream3,DISABLE);
    DMA_ClearFlag(DMA2_Stream3,DMA_FLAG_TCIF3);
    ST7789_CSD();

    DMA2_Stream3->NDTR = len;
    DMA2_Stream3->M0AR = (uint32_t)data;

    ST_SPI_DMAMode();
    DMA_Cmd(DMA2_Stream3,ENABLE);
    ST7789_CSE();
    ST7789_DMode();
    SPI_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE);
}

void Send_Block_NonBlocking(uint16_t *data,uint16_t len){
    if(dma_isbusy)return;
    dma_isbusy = 1;
    while(SPI_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY));//DMA完成，不意味着SPI传输结束。
    DMA_Cmd(DMA2_Stream3,DISABLE);
    DMA_ClearFlag(DMA2_Stream3,DMA_FLAG_TCIF3);
    ST7789_CSD();

    DMA2_Stream3->NDTR = len;
    DMA2_Stream3->M0AR = (uint32_t)data;

    ST_SPI_DMAMode();
    DMA_Cmd(DMA2_Stream3,ENABLE);
    ST7789_CSE();
    SPI_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE);
}

void ST_DisplayArea(uint16_t x,uint16_t y,uint16_t w,uint16_t h){
    //设置列范围
    ST_SendCmd(ST7789_CMD_CASET,4,((x)>>8)&0xFF,(x)&0xFF,((x+w)>>8)&0xFF,(x+w)&0xFF);
    //设置行范围
    ST_SendCmd(ST7789_CMD_RASET,4,((y)>>8)&0xFF,(y)&0xFF,((y+h)>>8)&0xFF,(y+h)&0xFF);
}

//屏幕硬件信息初始化
void Screen_Init(){
    ST_SendCmd(ST7789_CMD_SWRESET,0);       //软件复位 
    delay_ms(200);  
    ST_SendCmd(ST7789_CMD_SLPOUT,0);        //退出睡眠模式
    delay_ms(200);
    ST_SendCmd(ST7789_CMD_NORON,0);         //全局显示模式
    ST_SendCmd(ST7789_CMD_MADCTL,1,0x00);   //内存控制方式（默认模式）
    ST_SendCmd(ST7789_CMD_DISPON,0);        //开启显示
    delay_ms(200);
    //设置显示范围
    ST_DisplayArea(0,0,239,319);
    //设置像素格式
    ST_SendCmd(ST7789_CMD_COLMOD,1,0x55);   //RGB(565)模式
    //开启显示
    ST_SendCmd(ST7789_CMD_RAMWR,0);         //开启显示
}




//其他函数
void ClearBuf(uint16_t *buf,uint16_t color){
    for(uint16_t i=0;i<20;i++){
        for(uint16_t j=0;j<240;j++){
            buf[i*240+j] = color;
        }
    }
}


void ST7789_SetWindow(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2){
    ST_DisplayArea(x1,y1,x2-x1,y2-y1);
}


void ST7789_WritePixels_DMA(uint16_t *px_map,uint16_t datasize){
    ST_SendCmd(ST7789_CMD_RAMWR,0);
    Send_Block_Blocking(px_map,datasize);
}


void GreenScreen(){
    ST_DisplayArea(0,0,239,319);
    ST_SendCmd(ST7789_CMD_RAMWR,0);
    uint16_t r = 0b1111100000000000;
    uint16_t g = 0b0000011111100000;
    uint16_t b = 0b0000000000011111;
    ClearBuf(BUFA,r);
    for(int i=0;i<16;i++) Send_Block_Blocking(BUFA,240*20);
    ClearBuf(BUFB,g);
    for(int i=0;i<16;i++) Send_Block_Blocking(BUFB,240*20);
    ClearBuf(BUFA,b);
    for(int i=0;i<16;i++) Send_Block_Blocking(BUFA,240*20);
}



void ST7789_Debug(){
    Screen_Init();
    GreenScreen();

    ClearBuf(BUFA,0x001F);
    ClearBuf(BUFB,0xF100);
    
    
    // ST_DisplayArea(30,30,90,90);
    // ST_SendCmd(ST7789_CMD_RAMWR,0);
    while(1){
        
        // Send_Block_Blocking(BUFB,240*20);
    }

}