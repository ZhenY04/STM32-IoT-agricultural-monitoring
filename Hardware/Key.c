#include "stm32f10x.h"                  // Device header
#include "Key.h"
#include "Delay.h"

void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(KEY_RCC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructrue;
	GPIO_InitStructrue.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructrue.GPIO_Pin = KEY_MODE_PIN | KEY_PLUS_PIN | KEY_MINUS_PIN;
	GPIO_InitStructrue.GPIO_Speed = GPIO_Speed_50MHz;    //实际上并没有“输入速度”这一说，在此没有意义，但还得写
	GPIO_Init(KEY_PORT, &GPIO_InitStructrue);
}

uint8_t Key_Scan(uint16_t GPIO_Pin)
{
    if(GPIO_ReadInputDataBit(KEY_PORT, GPIO_Pin) == 0)
    {
        Delay_ms(20);
        if(GPIO_ReadInputDataBit(KEY_PORT, GPIO_Pin) == 0)    //二次确认
        {
            while(GPIO_ReadInputDataBit(KEY_PORT, GPIO_Pin) == 0);
            return 1;   // 有一次“按下事件”
        }
    }
    return 0;
}
