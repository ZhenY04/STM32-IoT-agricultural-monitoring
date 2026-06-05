#ifndef __BH1750_H
#define __BH1750_H

#include "stm32f10x.h"                  // Device header

/* BH1750 I2C 地址*/
#define BH1750_ADDR  0x23

/* BH1750 指令 */
#define BH1750_POWER_ON   0x01
#define BH1750_RESET      0x07
#define BH1750_CONT_H_RES 0x10    // 连续高分辨率模式（1 lx）

void BH1750_Init(void);
uint16_t BH1750_ReadRaw(void);
uint16_t BH1750_ReadLux(void);

#endif
