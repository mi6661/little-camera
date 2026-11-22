#ifndef __BSP_ST7789_H
#define __BSP_ST7789_H
#include "st7789.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_dma.h"
#include "delay/bsp_delay.h"
#include "misc.h"
#include <stdarg.h>



#define STW 240
#define STH 320
#define LINE_COUNT 10



//命令模式
#define ST7789_CMode()  GPIO_ResetBits(GPIOE,GPIO_Pin_10);
//数据模式
#define ST7789_DMode()  GPIO_SetBits(GPIOE,GPIO_Pin_10);
//使能ST7789
#define ST7789_CSE()    GPIO_ResetBits(GPIOA,GPIO_Pin_4);
//失能ST7789
#define ST7789_CSD()    GPIO_SetBits(GPIOA,GPIO_Pin_4);


void ST7789_Init();
void ST7789_Rectangle(uint16_t x,uint16_t y,
                        uint16_t w,uint16_t h,uint16_t color);
void ST7789_Image(uint16_t x,uint16_t y,
                        uint16_t w,uint16_t h,
                        const uint16_t* data);
void ST7789_Debug();

#endif // !__BSP_ST7789_H