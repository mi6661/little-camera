#include "bsp_blight.h"

void Blight_TIM2_Init(){
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div4);
    //APB1 -> 42MHz
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
    TIM_TimeBaseInitTypeDef conf;
    conf.TIM_ClockDivision = TIM_CKD_DIV1;
    conf.TIM_CounterMode = TIM_CounterMode_Up;
    conf.TIM_Period = 10000 - 1;
    conf.TIM_Prescaler = 8400 - 1;
    TIM_TimeBaseInit(TIM2,&conf);

    //配置中断向量
    NVIC_InitTypeDef nvic;
    nvic.NVIC_IRQChannel = TIM2_IRQn;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&nvic);

    TIM_Cmd(TIM2,ENABLE);
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
}

void Blight_GPIO_Init(){
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
    GPIO_InitTypeDef conf;
    conf.GPIO_Mode = GPIO_Mode_OUT;
    conf.GPIO_Pin = GPIO_Pin_2;
    conf.GPIO_Speed = GPIO_Speed_100MHz;
    conf.GPIO_OType = GPIO_OType_PP;
    conf.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB,&conf);
}


void Blight_Init(){
    Blight_GPIO_Init();
    Blight_TIM2_Init();
}


volatile static uint8_t BLED = 0;
/**
 * @brief 定时器2的定时中断函数
 */
void TIM2_IRQHandler(){
    if(BLED) GPIO_SetBits(GPIOB,GPIO_Pin_2);
    else GPIO_ResetBits(GPIOB,GPIO_Pin_2);
    BLED = 1 - BLED;
    TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
}