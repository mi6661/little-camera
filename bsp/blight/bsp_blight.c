#include "bsp_blight.h"

void Blight_Init(){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef gpioc_init;
    gpioc_init.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioc_init.GPIO_Pin = GPIO_Pin_13;
    gpioc_init.GPIO_Speed = GPIO_Speed_2MHz;   /* PC13 最高只支持 2MHz */
    GPIO_Init(GPIOC,&gpioc_init);

    //Sysclk = 72MHz
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
    TIM_TimeBaseInitTypeDef tim2_conf;
    tim2_conf.TIM_Period = 5000 - 1;
    tim2_conf.TIM_ClockDivision = TIM_CKD_DIV1;
    tim2_conf.TIM_Prescaler = 7200 - 1;
    tim2_conf.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2,&tim2_conf);
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);

    // 4. NVIC 配置
    NVIC_InitTypeDef nvic_init;
    nvic_init.NVIC_IRQChannel                   = TIM2_IRQn;
    nvic_init.NVIC_IRQChannelPreemptionPriority = 1;
    nvic_init.NVIC_IRQChannelSubPriority        = 0;
    nvic_init.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic_init);
    TIM_Cmd(TIM2,ENABLE);
}


/**
 * 中断服务函数
 * 函数名必须和 startup_stm32f10x_md.s 向量表里的名字完全一致
 */
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);   // 必须清标志位
        GPIO_WriteBit(GPIOC, GPIO_Pin_13,
                (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13)));
    }
}