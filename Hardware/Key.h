#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

#define KEY_PORT      GPIOA
#define KEY_MODE_PIN  GPIO_Pin_4
#define KEY_PLUS_PIN  GPIO_Pin_2
#define KEY_MINUS_PIN GPIO_Pin_0
#define KEY_RCC       RCC_APB2Periph_GPIOA
     
#define KEY_MODE      0
#define KEY_PLUS      1
#define KEY_MINUS     2
#define KEY_NUM       3

void Key_Init(void);
void Key_Tick(void);
uint8_t Key_GetEvent(uint8_t key);

#endif
