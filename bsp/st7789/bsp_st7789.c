#include "bsp_st7789.h"

//命令模式
void ST7789_CMode(){
    GPIO_ResetBits(GPIOA,GPIO_Pin_9);
}

//数据模式
void ST7789_DMode(){
    GPIO_SetBits(GPIOA,GPIO_Pin_9);
}

//使能ST7789
void ST7789_CSE(){
    GPIO_ResetBits(GPIOB,GPIO_Pin_12);
}

//失能ST7789
void ST7789_CSD(){
    GPIO_SetBits(GPIOB,GPIO_Pin_12);
}

static SPI_InitTypeDef spi2;
void ST7789_Init(){
    delay_ms(200);
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
    //for(int i=0;i<10000;i++);
    delay_ms(200);
    GPIO_SetBits(GPIOA,GPIO_Pin_8);
    //for(int i=0;i<500000;i++);
    delay_ms(200);

    GPIO_InitTypeDef dc;//st7789功能选择线
    dc.GPIO_Pin = GPIO_Pin_9;
    dc.GPIO_Mode = GPIO_Mode_Out_PP;
    dc.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&dc);
    
    // GPIO_InitTypeDef blk;//st7789背光pwm引脚
    // blk.GPIO_Pin = GPIO_Pin_10;
    // blk.GPIO_Mode = GPIO_Mode_Out_PP;
    // blk.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_Init(GPIOA,&blk);
    // GPIO_SetBits(GPIOA,GPIO_Pin_10);//这里先直接拉高电平（最亮）
    // //GPIO_ResetBits(GPIOA,GPIO_Pin_10);

    GPIO_SetBits(GPIOB, GPIO_Pin_12); // CS 拉高
    GPIO_SetBits(GPIOA, GPIO_Pin_9);  // DC 拉高
    GPIO_SetBits(GPIOA, GPIO_Pin_8);  // RST 拉高
    
    SPI_Cmd(SPI2,ENABLE);
}






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

    SPI_Cmd(SPI2,DISABLE);
    spi2.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    SPI_Init(SPI2,&spi2);
    SPI_Cmd(SPI2,ENABLE);

    ST7789_DMode();
    while(1){
        for(int y=0;y<320;y++){
            for(int x=0;x<240;x++){
                uint16_t pixel = 0b1111100000000000;
                //uint16_t pixel = 0b0000000000011111;
                ST7789_SendByte_Plus((uint8_t)(pixel >> 8));
                ST7789_SendByte_Plus((uint8_t)(pixel & 0xFF));
            }
        }
        for(int y=0;y<320;y++){
            for(int x=0;x<240;x++){
                //uint16_t pixel = 0b1111100000000000;
                uint16_t pixel = 0b0000000000011111;
                ST7789_SendByte_Plus((uint8_t)(pixel >> 8));
                ST7789_SendByte_Plus((uint8_t)(pixel & 0xFF));
            }
        }
    }
    

    ST7789_CSD();
}










