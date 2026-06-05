#include "DHT11.h"
#include "Delay.h"

#define DHT11_PORT GPIOB
#define DHT11_PIN  GPIO_Pin_0
#define DHT11_RCC  RCC_APB2Periph_GPIOB

/* ===== DATA 方向切换 ===== */
static void DHT11_IO_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
}

static void DHT11_IO_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
}

/* ===== 初始化 ===== */
void DHT11_Init(void)
{
    RCC_APB2PeriphClockCmd(DHT11_RCC, ENABLE);
    DHT11_IO_OUT();
    GPIO_SetBits(DHT11_PORT, DHT11_PIN);
}

/* ===== 起始信号 ===== */
static uint8_t DHT11_Start(void)
{
    DHT11_IO_OUT();
    GPIO_ResetBits(DHT11_PORT, DHT11_PIN);
    Delay_ms(20);               // ≥18ms
    GPIO_SetBits(DHT11_PORT, DHT11_PIN);
    Delay_us(30);
    DHT11_IO_IN();

    // 等待 DHT11 拉低
    if (GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN))
        return 1;

    // 等待 DHT11 拉高
    while (!GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN));
    while (GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN));

    return 0;
}

/* ===== 读 1 位 ===== */
static uint8_t DHT11_Read_Bit(void)
{
    while (!GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN));
    Delay_us(40);

    if (GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN))
    {
        while (GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN));
        return 1;
    }
    else
    {
        return 0;
    }
}

/* ===== 读 1 字节 ===== */
static uint8_t DHT11_Read_Byte(void)
{
    uint8_t i, data = 0;

    for (i = 0; i < 8; i++)
    {
        data <<= 1;
        data |= DHT11_Read_Bit();
    }

    return data;
}

/* ===== 读数据 ===== */
uint8_t DHT11_Read_Data(DHT11_Data_TypeDef *data)
{
    uint8_t buf[5];
    uint8_t i;

    if (DHT11_Start())
        return 1;

    for (i = 0; i < 5; i++)
        buf[i] = DHT11_Read_Byte();

    if ((buf[0] + buf[1] + buf[2] + buf[3]) != buf[4])
        return 2;

    data->humi_int = buf[0];
    data->humi_dec = buf[1];
    data->temp_int = buf[2];
    data->temp_dec = buf[3];

    return 0;
}
