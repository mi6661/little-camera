#include "bsp_btio.h"


/**
 * uart1初始化
 * @brief 使用uart1作为蓝牙终端IO接口
 */
void BTIO_Init(){
    //开启GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1,ENABLE);
    //配置GPIOA9(Tx) GPIOA10(Rx)
    GPIO_InitTypeDef AP9;
    AP9.GPIO_Mode = GPIO_Mode_AF_PP;
    AP9.GPIO_Pin = GPIO_Pin_9;
    AP9.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&AP9);

    GPIO_InitTypeDef AP10;
    AP10.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    AP10.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOA,&AP10);

    //5.配置串口参数
    USART_InitTypeDef usart1_init;
    USART_StructInit(&usart1_init);
    USART_Init(USART1,&usart1_init);
    USART_Cmd(USART1,ENABLE);
}


int _write(int file, char *ptr, int len) {

    for (int i = 0; i < len; i++) {
        // 把你的 UART 发送函数放这里
        //uart_send_byte(ptr[i]);
        USART_SendData(USART1,ptr[i]);
        while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
        //while(!(USART1->SR & USART_SR_TXE));
        //USART1->DR = ptr[i];
    }
    return len;
}

int *__errno(void)
{
    static int errno_val = 0;
    return &errno_val;
}