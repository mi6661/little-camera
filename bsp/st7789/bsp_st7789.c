#include "bsp_st7789.h"

static SPI_InitTypeDef spi2;

static uint16_t ST7789_GRAM[ST_W*10] = {0};

void ST_DMA_Init(){
    DMA_InitTypeDef dma_conf;
    dma_conf.DMA_BufferSize = (ST_W*10);
    dma_conf.DMA_DIR = DMA_DIR_PeripheralDST;//外设作为数据目的地
    dma_conf.DMA_PeripheralBaseAddr = SPI2->DR;
    dma_conf.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dma_conf.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设寄存器关闭自增，因为SPI2->DR寄存器地址是固定的
    dma_conf.DMA_MemoryBaseAddr = (uint32_t)ST7789_GRAM;
    dma_conf.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dma_conf.DMA_MemoryInc = DMA_MemoryInc_Enable;          //内存地址自增
    dma_conf.DMA_Mode = DMA_Mode_Normal;
    dma_conf.DMA_Priority = DMA_Priority_High;
    dma_conf.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5,&dma_conf);
    
    //配置中断处理通道
    NVIC_InitTypeDef nvic_conf;
    nvic_conf.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    nvic_conf.NVIC_IRQChannelCmd = ENABLE;
    nvic_conf.NVIC_IRQChannelPreemptionPriority = 0;
    nvic_conf.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&nvic_conf);

    //开启DMA中断
    DMA_ITConfig(DMA1_Channel5,DMA_IT_TC | DMA_IT_HT | DMA_IT_TE,ENABLE);
    DMA_Cmd(DMA1_Channel5,ENABLE);
}

void ST_SPI2_Init(){
    //初始化SPI2
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
    spi2.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi2.SPI_Mode = SPI_Mode_Master;
    spi2.SPI_DataSize = SPI_DataSize_8b;
    spi2.SPI_CPOL = SPI_CPOL_Low;
    spi2.SPI_CPHA = SPI_CPHA_1Edge;
    spi2.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    spi2.SPI_FirstBit = SPI_FirstBit_MSB;
    spi2.SPI_NSS = SPI_NSS_Soft;
    spi2.SPI_CRCPolynomial = 7;
    SPI_Init(SPI2,&spi2);
    SPI_Cmd(SPI2,ENABLE);
}

void ST_GPIO_Init(){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    GPIO_InitTypeDef mosi_clk;
    mosi_clk.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
    mosi_clk.GPIO_Mode = GPIO_Mode_AF_PP;
    mosi_clk.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB,&mosi_clk);

    GPIO_InitTypeDef nss;
    nss.GPIO_Pin = GPIO_Pin_12;
    nss.GPIO_Mode = GPIO_Mode_Out_PP;
    nss.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB,&nss);
    GPIO_SetBits(GPIOB,GPIO_Pin_12);//拉高片选线

    GPIO_InitTypeDef miso;
    miso.GPIO_Pin = GPIO_Pin_14;
    miso.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    miso.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB,&miso);

    GPIO_InitTypeDef res;//st7789复位引脚
    res.GPIO_Pin = GPIO_Pin_8;
    res.GPIO_Mode = GPIO_Mode_Out_PP;
    res.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&res);
    //硬件复位操作
    GPIO_ResetBits(GPIOA,GPIO_Pin_8);
    delay_ms(20);
    GPIO_SetBits(GPIOA,GPIO_Pin_8);
    delay_ms(200);

    GPIO_InitTypeDef dc;//st7789功能选择线
    dc.GPIO_Pin = GPIO_Pin_9;
    dc.GPIO_Mode = GPIO_Mode_Out_PP;
    dc.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&dc);
    

    GPIO_SetBits(GPIOB, GPIO_Pin_12); // CS 拉高
    GPIO_SetBits(GPIOA, GPIO_Pin_9);  // DC 拉高
    GPIO_SetBits(GPIOA, GPIO_Pin_8);  // RST 拉高
}


void DMA1_Channel5_IRQHandler(){
    if(DMA_GetITStatus(DMA1_IT_TC5)){
        //TODO 完成一次DMA传输
        DMA_ClearITPendingBit(DMA1_IT_TC5);
    }
    if(DMA_GetITStatus(DMA1_IT_HT5)){
        //TODO DMA传输完成总量一半
        DMA_ClearITPendingBit(DMA1_IT_HT5);
    }
    if(DMA_GetITStatus(DMA1_IT_TE5)){
        //TODO DMA传输异常
        DMA_ClearITPendingBit(DMA1_IT_TE5);
    }
    
}

void ST7789_Init(){
    delay_ms(200);
    ST_SPI2_Init();
    ST_GPIO_Init();
    ST_DMA_Init();
}



/**
 * 功能性函数
 */


void ST7789_SendByte(uint8_t data){
    while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)==RESET);
    SPI_I2S_SendData(SPI2,data);
    while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_BSY)==SET);
}

void ST7789_SendByte_Plus(uint8_t data){
    while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)==RESET);
    SPI_I2S_SendData(SPI2,data);
    //while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_BSY)==SET);
}

void WaitIdle(){
    while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)==RESET);
    while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_BSY)==SET);
}

void Send_Block_DMA(uint16_t *data,uint16_t len){
    DMA_Cmd(DMA1_Channel5,DISABLE);
    DMA_ClearFlag(DMA1_FLAG_TC5);

    //配置本次传输
    DMA1_Channel5->CNDTR = len;
    DMA1_Channel5->CMAR = (uint32_t)data;
}


// void ST7789_SendData(uint8_t *data,uint16_t len){
//     for(int i=0;i<len;i++){
//         ST7789_SendByte(data[i]);
//     }
// }

void ST7789_SendCmd(uint8_t cmd,uint32_t args,uint16_t argc){
    ST7789_CMode();
    ST7789_SendByte(cmd);
    //WaitIdle();
    if(argc <= 0) return;
    ST7789_DMode();
    for(int i=0;i<argc;i++){
        uint8_t t = (uint8_t)(args >> 8*(argc-1-i) & 0xFF);
        ST7789_SendByte(t);
    }
}

void ST7789_DisplayArea(uint16_t xs,uint16_t xe,uint16_t ys,uint16_t ye){
    uint32_t tmp = 0;
    //设置列范围
    tmp = (xs << 16) | xe ;
    ST7789_SendCmd(ST7789_CMD_CASET,tmp,4);
    //设置行范围
    tmp = (ys << 16) | ye ;
    ST7789_SendCmd(ST7789_CMD_RASET,tmp,4);

}


void ST7789_Debug(){
    ST7789_CSE();
    ST7789_SendCmd(ST7789_CMD_SWRESET,0,0); //软件复位 
    //for(int i=0;i<1000000;i++); 
    delay_ms(200);  
    ST7789_SendCmd(ST7789_CMD_SLPOUT,0,0);   //退出睡眠模式
    //for(int i=0;i<1000000;i++); 
    delay_ms(200);
    ST7789_SendCmd(ST7789_CMD_NORON,0,0);    //全局显示模式
    ST7789_SendCmd(ST7789_CMD_MADCTL,0x00,1);//内存控制方式（默认模式）

    //ST7789_SendCmd(ST7789_CMD_INVON,0,0);   //开启反色
    ST7789_SendCmd(ST7789_CMD_DISPON,0,0);//开启显示
    //for(int i=0;i<1000000;i++);
    delay_ms(200);

    //设置显示范围
    ST7789_DisplayArea(0,239,0,319);
    //设置像素格式
    ST7789_SendCmd(ST7789_CMD_COLMOD,0x55,1);

    //开启显示
    ST7789_SendCmd(ST7789_CMD_RAMWR,0,0);
    SPI_Cmd(SPI2,ENABLE);
    /*从这里开始，屏幕已经通过低速SPI配置完成，现在开用DMA加高速SPI刷新屏幕数据*/
    SPI_Cmd(SPI2,DISABLE);
    spi2.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    spi2.SPI_DataSize = SPI_DataSize_16b;//调整位16bit模特
    SPI_Init(SPI2,&spi2);
    

    ST7789_DMode();
    while(1){
        // for(int y=0;y<320;y++){
        //     for(int x=0;x<240;x++){
        //         uint16_t pixel = 0b1111100000000000;
        //         //uint16_t pixel = 0b0000000000011111;
        //         ST7789_SendByte_Plus((uint8_t)(pixel >> 8));
        //         ST7789_SendByte_Plus((uint8_t)(pixel & 0xFF));
        //     }
        // }
        // for(int y=0;y<320;y++){
        //     for(int x=0;x<240;x++){
        //         //uint16_t pixel = 0b1111100000000000;
        //         uint16_t pixel = 0b0000000000011111;
        //         ST7789_SendByte_Plus((uint8_t)(pixel >> 8));
        //         ST7789_SendByte_Plus((uint8_t)(pixel & 0xFF));
        //     }
        // }
        for(int i=0;i<ST_W*10;i++){
            ST7789_GRAM[i] = 0b1111100000000000;
        }
        while(DMA_GetFlagStatus(DMA1_FLAG_TC5)!=SET);
        SPI_I2S_DMACmd(SPI2,SPI_I2S_DMAReq_Tx,ENABLE);

        for(int i=0;i<ST_W*10;i++){
            ST7789_GRAM[i] = 0b1111100000000000;
        }
        while(DMA_GetFlagStatus(DMA1_FLAG_TC5)!=SET);
        SPI_I2S_DMACmd(SPI2,SPI_I2S_DMAReq_Tx,ENABLE);
    }
    

    ST7789_CSD();
}










