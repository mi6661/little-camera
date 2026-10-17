#include "bsp_st7789.h"

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
    SPI_InitTypeDef sconf;
    sconf.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
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

void ST7789_Init(){
    ST_GPIO_Init();
    ST_SPI_Init();
}

void WaitIdle(){
    while(SPI_GetFlagStatus(SPI1,SPI_FLAG_TXE) == RESET);
    while(SPI_GetFlagStatus(SPI1,SPI_FLAG_BSY) == SET);
}

void ST_SendByte(uint8_t data){
    while(SPI_GetFlagStatus(SPI1,SPI_FLAG_TXE)==RESET);
    SPI_SendData(SPI1,data);
    while(SPI_GetFlagStatus(SPI1,SPI_FLAG_BSY)==SET);
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


void ST7789_Debug(){
    ST7789_CSE();
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
    ST7789_DMode();
    uint8_t tag = 0;
    //while(1){
        for(int i=0;i<320;i++){
            for(int j=0;j<240;j++){
                if(tag){
                    ST_SendByte(0x00);
                    ST_SendByte(0x00);
                } 
                else{
                    ST_SendByte(0x00);
                    ST_SendByte(0x00);
                } 
            }
        }
        tag = 1 - tag;
    //}
    ST7789_CSD();
}