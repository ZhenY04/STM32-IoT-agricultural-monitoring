#include "BH1750.h"
#include "MyI2C.h"
#include "Delay.h"
#include "stm32f10x.h"                  // Device header

/**
  * @brief  向 BH1750 发送一个命令
  */
static void BH1750_SendCmd(uint8_t cmd)
{
	MyI2C_Start();
	MyI2C_SendByte((BH1750_ADDR << 1) | 0);   // 写
	MyI2C_ReceiveAck();
	MyI2C_SendByte(cmd);
	MyI2C_ReceiveAck();
	MyI2C_Stop();
}

/**
  * @brief  BH1750 初始化
  */
void BH1750_Init(void)
{
	MyI2C_Init();
	
	BH1750_SendCmd(BH1750_POWER_ON);   // 上电
	Delay_ms(5);
	BH1750_SendCmd(BH1750_RESET);      // 复位
	Delay_ms(5);
	BH1750_SendCmd(BH1750_CONT_H_RES); // 连续高分辨率模式
}

/**
  * @brief  读取 BH1750 原始数据（16bit）
  * @retval 原始光照数据
  */
uint16_t BH1750_ReadRaw(void)
{
	uint8_t high, low;
	uint16_t value;
	
	/* 等待测量完成（H-Resolution 最大 120ms） */
	Delay_ms(180);

	MyI2C_Start();
	MyI2C_SendByte((BH1750_ADDR << 1) | 1);   // 读
	MyI2C_ReceiveAck();
	
	high = MyI2C_ReceiveByte();
	MyI2C_SendAck(0);     // ACK
	
	low = MyI2C_ReceiveByte();
	MyI2C_SendAck(1);     // NACK
	
	MyI2C_Stop();

	value = (high << 8) | low;
	return value;
}

/**
  * @brief  读取光照值（Lux）
  * @retval 光照强度（lx）
  */
uint16_t BH1750_ReadLux(void)
{
	uint16_t raw;
	raw = BH1750_ReadRaw();
	
	/* 数据手册：lux = raw / 1.2 */
	return raw * 10 / 12;
}
