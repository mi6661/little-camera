#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "blight/bsp_blight.h"







int main(){
    Blight_Init();

    while(1){
        // delay(1024);
        // GPIO_SetBits(GPIOB,GPIO_Pin_2);
        // delay(1024);
        // GPIO_ResetBits(GPIOB,GPIO_Pin_2);
    }
}