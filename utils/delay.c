#include "utils.h"

volatile uint32_t g_ms_tick = 0;


void TIM3_IRQHandler(){
    //if(TIM_GetFlagStatus(TIM3,TIM_FLAG_Update) == SET){
        g_ms_tick++;
        TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
    //}
}

uint32_t millis(){
    return g_ms_tick;
}

void delay_enable(){
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

    TIM_TimeBaseInitTypeDef conf;
    conf.TIM_ClockDivision = TIM_CKD_DIV1;
    conf.TIM_CounterMode = TIM_CounterMode_Up;
    conf.TIM_Period = 1000 - 1;
    conf.TIM_Prescaler = 72 - 1;
    TIM_TimeBaseInit(TIM3,&conf);
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);

    NVIC_InitTypeDef nvic;
    nvic.NVIC_IRQChannel = TIM3_IRQn;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&nvic);
    TIM_Cmd(TIM3,ENABLE);
}

void delay_ms(int ms){
    uint32_t start = millis();
    while(millis()-start < ms);
}