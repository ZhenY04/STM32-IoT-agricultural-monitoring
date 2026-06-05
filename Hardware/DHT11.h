#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f10x.h"

typedef struct
{
    uint8_t humi_int;
    uint8_t humi_dec;
    uint8_t temp_int;
    uint8_t temp_dec;
} DHT11_Data_TypeDef;

void DHT11_Init(void);
uint8_t DHT11_Read_Data(DHT11_Data_TypeDef *data);

#endif
