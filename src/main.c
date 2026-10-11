#include "main.h"

void delay(int n){
    for(int i=0;i<n;i++)
        for(int j=0;j<n;j++);
}

void main(void){
    //初始化usart1
    //BTIO_Init();
    //屏幕初始化
    //OLED_Init();
    //OLED_Test();
    //printf("Welcome!");
    //开启呼吸灯
    Blight_Init();
    //初始化ST7789
    delay_enable();
    ST7789_Init();
    ST7789_Debug();
    while(1){

    }
}








