#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Relay.h"

void Relay_Init(void)
{
	RCC_APB2PeriphClockCmd(RELAY_RCC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructrue;
	GPIO_InitStructrue.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructrue.GPIO_Pin = RELAY_PIN;
	GPIO_InitStructrue.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RELAY_PORT, &GPIO_InitStructrue);
}
