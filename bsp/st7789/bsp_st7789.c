#include "bsp_st7789.h"

static uint8_t BUFA[20*240*2] = {0};
static uint8_t BUFB[20*240*2] = {0};
volatile static uint8_t dma_isbusy = 0;
volatile static uint8_t spi_usedma = 0;



static SPI_InitTypeDef sconf;

static void ST7789_GPIO_Init(){
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

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
    GPIO_InitTypeDef res_dc;
    res_dc.GPIO_Mode = GPIO_Mode_OUT;
    res_dc.GPIO_OType = GPIO_OType_PP;
    res_dc.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_10;
    res_dc.GPIO_PuPd = GPIO_PuPd_NOPULL;
    res_dc.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOE,&res_dc);

    GPIO_InitTypeDef cs;
    cs.GPIO_Mode = GPIO_Mode_OUT;
    cs.GPIO_OType = GPIO_OType_PP;
    cs.GPIO_Pin = GPIO_Pin_4;
    cs.GPIO_PuPd = GPIO_PuPd_NOPULL;
    cs.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_SetBits(GPIOA,GPIO_Pin_4);
    GPIO_Init(GPIOA,&cs);
}

static void ST7789_SPI_Init(){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
    sconf.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
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

static void ST7789_DMA_Init(){
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);
    DMA_InitTypeDef conf;
    conf.DMA_BufferSize = 0;
    conf.DMA_Channel = DMA_Channel_3;
    conf.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    conf.DMA_FIFOMode = DMA_FIFOMode_Disable;
    conf.DMA_Memory0BaseAddr =  (uint32_t)BUFA;//后续使用时再设置具体地址
    conf.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    conf.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    conf.DMA_MemoryInc = DMA_MemoryInc_Enable;
    conf.DMA_Mode = DMA_Mode_Normal;
    conf.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;
    conf.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    conf.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
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
    ST7789_GPIO_Init();
    ST7789_SPI_Init();
    ST7789_DMA_Init();
}

//等待SPI1空闲
void WaitIdle(){
    while(SPI_GetFlagStatus(SPI1,SPI_FLAG_TXE) == RESET);
    while(SPI_GetFlagStatus(SPI1,SPI_FLAG_BSY) == SET);
}

void ST_HardRET(){
    //硬件复位
    GPIO_ResetBits(GPIOE,GPIO_Pin_8);
    delay_ms(20);
    GPIO_SetBits(GPIOE,GPIO_Pin_8);
    delay_ms(150); 
}


//SPI基本函数
void ST_SPI_TRAMode(){
    if(spi_usedma == 0) return;
    spi_usedma = 0;
    //关闭DMA
    while(dma_isbusy);
    dma_isbusy = 1;
    WaitIdle();

    //SPI1切换为字节模式
    SPI_Cmd(SPI1,DISABLE);
    sconf.SPI_DataSize = SPI_DataSize_8b;
    sconf.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    SPI_Init(SPI1,&sconf);
    SPI_Cmd(SPI1,ENABLE);
}

void ST_SPI_DMAMode(){
    if(spi_usedma == 1) return;
    spi_usedma = 1;
    //SPI1切换为半字模式
    WaitIdle();

    SPI_Cmd(SPI1,DISABLE);
    sconf.SPI_DataSize = SPI_DataSize_8b;
    sconf.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    SPI_Init(SPI1,&sconf);
    SPI_Cmd(SPI1,ENABLE);

    //使能DMA
    dma_isbusy = 0;
}

void ST_SendByte(uint8_t data){
    while(SPI_GetFlagStatus(SPI1,SPI_FLAG_TXE)==RESET);
    SPI_SendData(SPI1,data);
    while(SPI_GetFlagStatus(SPI1,SPI_FLAG_TXE)==RESET);
    while(SPI_GetFlagStatus(SPI1,SPI_FLAG_BSY)==SET);
    
}

void ST_SendCmd(uint8_t cmd,uint8_t argc,...){
    ST_SPI_TRAMode();
    ST7789_CSE();
    //开启命令模式
    ST7789_CMode();
    ST_SendByte(cmd);
    if(argc < 1) {
        ST7789_CSD();
        delay_ms(1);
        return;
    }
    va_list args;
    va_start(args,argc);
    //开启数据模式
    ST7789_DMode();
    for(int i=0;i<argc;i++){
        uint8_t data = va_arg(args,int);
        ST_SendByte(data);
    }
    va_end(args);
    ST7789_CSD();
    delay_ms(1);
}

void ST_SendData(uint8_t *data,uint32_t len){
    ST_SPI_TRAMode();
    ST7789_CSE();
    ST7789_DMode();     //数据模式
    for(int i=0;i<len;i++){
        uint8_t b = data[i];
        ST_SendByte(b);
    }
    ST7789_CSD();
}

void Send_Block_Blocking(uint8_t *data,uint16_t len){
    ST_SPI_DMAMode();
    while(dma_isbusy);
    dma_isbusy = 1;
    while(SPI_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY));//DMA完成，不意味着SPI传输结束。

    DMA_Cmd(DMA2_Stream3,DISABLE);
    DMA_ClearFlag(DMA2_Stream3,DMA_FLAG_TCIF3);
    ST7789_CSD();

    DMA2_Stream3->NDTR = len;
    DMA2_Stream3->M0AR = (uint32_t)data;

    DMA_Cmd(DMA2_Stream3,ENABLE);
    ST7789_CSE();
    ST7789_DMode();
    SPI_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE);
}

void Send_Block_NonBlocking(uint8_t *data,uint16_t len){
    ST_SPI_DMAMode();
    if(dma_isbusy)return;
    dma_isbusy = 1;
    while(SPI_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY));//DMA完成，不意味着SPI传输结束。
    DMA_Cmd(DMA2_Stream3,DISABLE);
    DMA_ClearFlag(DMA2_Stream3,DMA_FLAG_TCIF3);
    ST7789_CSD();

    DMA2_Stream3->NDTR = len;
    DMA2_Stream3->M0AR = (uint32_t)data;

    DMA_Cmd(DMA2_Stream3,ENABLE);
    ST7789_CSE();
    SPI_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE);
}

void ST_DisplayArea(uint16_t x,uint16_t y,
                        uint16_t w,uint16_t h){
    //设置列范围
    if(w <= 0 || h <= 0 || w > 240 || h > 320) return;
    ST_SendCmd(ST7789_CMD_CASET,4,
        ((x)>>8)&0xFF,
        (x)&0xFF,
        ((x+w-1)>>8)&0xFF,
        (x+w-1)&0xFF);
    //设置行范围
    ST_SendCmd(ST7789_CMD_RASET,4,
        ((y)>>8)&0xFF,
        (y)&0xFF,
        ((y+h-1)>>8)&0xFF,
        (y+h-1)&0xFF);
}




/*********************ST7789功能性函数***********************/



/**
 * @brief 屏幕硬件信息初始化
 * @return void
 */
void Screen_Init(){
    ST_HardRET();
    ST_SendCmd(ST7789_CMD_SWRESET,0);       //软件复位 
    delay_ms(200);  
    ST_SendCmd(ST7789_CMD_SLPOUT,0);        //退出睡眠模式
    delay_ms(150);
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
    delay_ms(10);
}

/**
 * @brief 设置ST7789的数据填充窗口
 * @param x     # 屏幕横坐标
 * @param y     # 屏幕纵坐标
 * @param w     # 屏幕宽度
 * @param h     # 屏幕高度
 * @return void
 */
void ST7789_SetWindow(uint16_t x,uint16_t y,
                        uint16_t w,uint16_t h){
    ST_DisplayArea(x,y,w,h);
    ST_SendCmd(ST7789_CMD_RAMWR,0);                       
}


/**
 * @brief 通过DMA向ST7789发送数据块
 * @param pix_map   #指向数据块的指针
 * @param datasize  #指向的数据块的大小
 * @return void
 */
void ST7789_WritePixels_DMA(uint8_t *px_map,uint32_t datasize){
    ST_SendCmd(ST7789_CMD_RAMWR,0);
    Send_Block_Blocking(px_map,datasize);
}

/**
 * @brief 绘制单色矩形
 * @param x     #矩形横坐标
 * @param y     #矩形纵坐标
 * @param w     #矩形宽度
 * @param h     #矩形高度
 * @param color #矩形颜色
 * @return void
 */
void ST7789_Rectangle(uint16_t x,uint16_t y,
                        uint16_t w,uint16_t h,uint16_t color)
{
    char cap = 'a';
    for(int i=y;i<y+h;i+=20){
        ST7789_SetWindow(x,i,w,20);
        switch (cap)
        {
        case 'a':
            cap = 'b';
            for(int a=0;a<w*20*2;a+=2){
                BUFA[a]     = color >> 8;
                BUFA[a+1]   = (uint8_t)(color & 0xFF00);
            }
            Send_Block_Blocking(BUFA,w*20*2);
            break;
        case 'b':
            cap = 'a';
            for(int b=0;b<w*20*2;b+=2){
                BUFB[b]     = color >> 8;
                BUFB[b+1]   = (uint8_t)(color & 0xFF);
            }
            Send_Block_Blocking(BUFB,w*20*2);
            break;
        default:
            break;
        }
    }
}

/**
 * @brief   绘制图片
 * @param   x   #绘制的横坐标
 * @param   y   #绘制的纵坐标
 * @param   w   #图片宽度
 * @param   h   #图片高度
 * @param   data    #指向图片数据块的指针
 */
void ST7789_Image(uint16_t x,uint16_t y,
                        uint16_t w,uint16_t h,
                        const uint16_t* data)
{
    char cap = 'a';
    uint16_t index = 0;
    int row = 20;
    for(int i=y;i<y+h;i+=20){
        if(i == y+h-1) row = h - i;
        ST7789_SetWindow(x,i,w,row);
        switch (cap)
        {
        case 'a':
            cap = 'b';
            for(int a=0;a<w*row;a++)
                BUFA[a] = data[index++];
            Send_Block_Blocking(BUFA,w*row);
            break;
        case 'b':
            cap = 'a';
            for(int b=0;b<w*row;b++)
                BUFB[b] = data[index++];
            Send_Block_Blocking(BUFB,w*row);
            break;
        default:
            break;
        }
    }
}


/**
 * @brief   ST7789开发测试函数
 * @return void
 */
void ST7789_Debug(){
    for(int i=0;i<9500;i=i+2){
        BUFB[i]     = 0b11111000;
        BUFB[i+1]   = 0b00000000;

        BUFA[i]     = 0b00000000;
        BUFA[i+1]   = 0b00011111;
    }
    Screen_Init();

    ST7789_SetWindow(0,0,50,50);
    //ST_SendData(BUFB,50*50*2);

    Send_Block_Blocking(BUFA,50*50*2);
    
    while (1)
    {

    }   
}