#ifndef __MQTT_H
#define __MQTT_H

#include "stm32f10x.h"

extern uint8_t soil_threshold;

void MQTT_Init(void);
void MQTT_Upload(int soil, int temp, int humi, int light, int alarm_flag);

#endif
