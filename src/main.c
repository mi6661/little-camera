#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "blight/bsp_blight.h"
#include "delay/bsp_delay.h"
#include "st7789/bsp_st7789.h"


#define DUMP 50



int main(){
    Blight_Init();
    ST7789_Init();
    ST7789_Debug();
    while(1){
        
    }
}