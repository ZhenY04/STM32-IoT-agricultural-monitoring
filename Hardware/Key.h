#ifndef __KEY_H
#define __KEY_H

#define KEY_PORT GPIOA
#define KEY_MODE_PIN  GPIO_Pin_4
#define KEY_PLUS_PIN  GPIO_Pin_2
#define KEY_MINUS_PIN  GPIO_Pin_0
#define KEY_RCC  RCC_APB2Periph_GPIOA

void Key_Init(void);
uint8_t Key_Scan(uint16_t GPIO_Pin);

#endif
