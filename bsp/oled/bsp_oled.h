#ifndef __BSP_OLED_H
#define __BSP_OLED_H

#include "stm32f10x.h"


#define I2C1_TIMEOUT   10000   /* 简单超时计数，防止卡死 */



/**
 * OLED初始化
 */
void OLED_Init();

uint8_t wait_flag(uint32_t flag, FlagStatus st);
uint8_t wait_event(uint32_t evt);

/* 写一个寄存器字节：devAddr 为 7 位地址(不含读写位) */
uint8_t I2C1_WriteReg(uint8_t devAddr, uint8_t reg, uint8_t data);

/* 读一个寄存器字节 */
uint8_t I2C1_ReadReg(uint8_t devAddr, uint8_t reg, uint8_t *buf);

static uint8_t OLED_Cmd(uint8_t c);
static uint8_t OLED_Data(uint8_t d);//显存数据写入
static void OLED_PanelInit(void);
void OLED_Display(void);
void OLED_Test(void);

//图形基本接口
void draw_pixel(int x,int y,int color);
void fill_rect(int x,int y,int w,int h,int color);


#endif // !__BSP_OLED_H