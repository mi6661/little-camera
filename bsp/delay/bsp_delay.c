#include "bsp_delay.h"

static uint32_t timestamp = 0;//时间戳

//标志是否初始化
static uint8_t is_init = 0;



void Delay_Init(){
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

    TIM_TimeBaseInitTypeDef tconf;
    tconf.TIM_ClockDivision = TIM_CKD_DIV1; //一分频时，APB1 时钟频率x2
    tconf.TIM_CounterMode = TIM_CounterMode_Up;
    tconf.TIM_Period = 10 - 1;
    tconf.TIM_Prescaler = 8400 - 1;
    TIM_TimeBaseInit(TIM3,&tconf);

    NVIC_InitTypeDef nconf;
    nconf.NVIC_IRQChannel = TIM3_IRQn;
    nconf.NVIC_IRQChannelCmd = ENABLE;
    nconf.NVIC_IRQChannelPreemptionPriority = 0;
    nconf.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&nconf);

    TIM_Cmd(TIM3,ENABLE);
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
}

/**
 * @brief TIM3定时中断函数
 */
void TIM3_IRQHandler(){
    timestamp++;
    TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
}

uint32_t get_timestamp(){
    return timestamp;
}

void delay_ms(int n){
    if(!is_init){
        Delay_Init();
        is_init = 1;
    } 
    uint32_t start = get_timestamp();
    while(get_timestamp() - start < n);
}