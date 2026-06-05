#ifndef __RELAY_H
#define __RELAY_H

#define RELAY_PORT GPIOB
#define RELAY_PIN  GPIO_Pin_12
#define RELAY_RCC  RCC_APB2Periph_GPIOB

void Relay_Init(void);

#endif
