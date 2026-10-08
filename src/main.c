#include "main.h"

void delay(int n){
    for(int i=0;i<n;i++)
        for(int j=0;j<n;j++);
}

void main(void){
    //初始化usart1
    BTIO_Init();
    //屏幕初始化
    OLED_Init();
    OLED_Test();
    //mmc初始化
    MMC_Init();
    MMC_WriteBytes("hello",sizeof("hello"));
    printf("Welcome!");
    //开启呼吸灯
    Blight_Init();
    
    while(1){

    }
}








