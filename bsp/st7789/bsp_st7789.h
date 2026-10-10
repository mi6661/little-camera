#ifndef __BSP_ST7789_H
#define __BSP_ST7789_H

#include "stm32f10x.h"
#include "utils.h"

#define ST7789_CMD_NOP      0x00    //空操作
#define ST7789_CMD_SWRESET  0x01    //软复位

#define ST7789_CMD_SLPIN    0x10    //进入睡眠模式
#define ST7789_CMD_SLPOUT   0x11    //退出睡眠模式
#define ST7789_CMD_PTLON    0x12    //开启局部显示
#define ST7789_CMD_NORON    0x13    //开启正常显示（退出局部显示模式）


#define ST7789_CMD_INVOFF   0x20    //显示反转关闭 (Display Inversion Off)
#define ST7789_CMD_INVON    0x21    //显示反转开启 (Display Inversion On，部分IPS屏发蓝/反色时需开启)
#define ST7789_CMD_GAMSET   0x26    //伽马设置 (Gamma Set，带1个参数选择Gamma曲线)
#define ST7789_CMD_DISPOFF  0x28    //关闭显示 (Display Off)
#define ST7789_CMD_DISPON   0x29    //开启显示 (Display On)


#define	ST7789_CMD_CASET    0x2A    //列地址设置 (Column Address Set)，带 4 个参数：XS_H, XS_L, XE_H, XE_L（起始列高/低字节、结束列高/低字节）
#define	ST7789_CMD_RASET    0x2B    //PASET	行地址设置 (Row Address Set)，带 4 个参数：YS_H, YS_L, YE_H, YE_L（起始行高/低字节、结束行高/低字节）
#define	ST7789_CMD_RAMWR    0x2C    //内存写入 (Memory Write)，发送此命令后，后续连续发送的 DC=1 数据即为 RGB 像素点颜色值
#define	ST7789_CMD_RAMRD    0x2E    //内存读出 (Memory Read)


#define ST7789_CMD_MADCTL	    0x36 //存储器数据访问控制 (Memory Data Access Control)，常用于设置屏幕扫描方向、行列翻转、RGB/BGR 颜色顺序（1个参数）
#define ST7789_CMD_COLMOD	    0x3A //接口像素格式 (Interface Pixel Format)，用于设置色彩深度：常用 0x55 代表 16-bit/pixel (RGB565)，0x66 代表 18-bit (RGB666)
#define ST7789_CMD_PORCTRL	    0xB2 //惯常用于初始化配置（Porch Setting）
#define ST7789_CMD_GCTRL	    0xB7 //栅极控制 (Gate Control)
#define ST7789_CMD_VCOMS	    0xBB //VCOM 电压设置
#define ST7789_CMD_LCMCTRL	    0xC0 //LCM 控制
#define ST7789_CMD_VDVVRHEN	    0xC2 //VDV 和 VRH 命令使能
#define ST7789_CMD_VRHS	        0xC3 //VRH 电压设置
#define ST7789_CMD_VDVS	        0xC4 //VDV 电压设置
#define ST7789_CMD_FRCTRL1	    0xC6 //帧率控制 (Frame Rate Control in Normal Mode)
#define ST7789_CMD_PWCTRL1	    0xD0 //电源控制 1
#define ST7789_CMD_PVGAMCTRL    0xE0 //正向伽马较准 (Positive Voltage Gamma Control)
#define ST7789_CMD_NVGAMCTRL    0xE1 //负向伽马较准 (Negative Voltage Gamma Control)



void ST7789_Init();

void ST7789_Debug();


#endif // !__BSP_ST7789_H 