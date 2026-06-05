#include "stm32f10x.h"
#include "Delay.h"

#define SOIL_MOISTURE_PORT GPIOB
#define SOIL_MOISTURE_PIN  GPIO_Pin_1
#define SOIL_MOISTURE_RCC  RCC_APB2Periph_GPIOB

#define DELAY_TIME 50 //数据显示时间间隔
#define ADC_DRY 3400 //干点
#define ADC_WET 1200 //湿点

void Soil_Moisture_Init(void)
{
    /* 1. 开时钟：B引脚和ADC模块	*/
    RCC_APB2PeriphClockCmd(SOIL_MOISTURE_RCC | RCC_APB2Periph_ADC1, ENABLE);

    /* 2. GPIO：PB1 模拟输入 */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Pin = SOIL_MOISTURE_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SOIL_MOISTURE_PORT, &GPIO_InitStructure);

    /* 3. ADC 基本配置 */
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    /* 4. 配置 ADC 通道：PB1 = ADC1_CH9 */
    ADC_RegularChannelConfig(
        ADC1,
        ADC_Channel_9,
        1,
        ADC_SampleTime_239Cycles5   // 采样时间拉长，稳定
    );

    /* 5. 使能 ADC */
    ADC_Cmd(ADC1, ENABLE);

    /* 6. ADC 校准 */
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}


/**
  * @brief  模拟输入，ADC
  * @retval 经ADC后的值，范围：0~4095
  */
uint16_t Soil_Moisture_Value(void)
{
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    return ADC_GetConversionValue(ADC1);
}

/**
  * @brief  指数滤波，再将0~4096的值映射为湿度百分比
  * @retval 湿度百分比
  */
uint8_t Soil_Moisture_Percent(void)
{
    static uint16_t avg = 0;
    uint16_t new = Soil_Moisture_Value();

    if(avg == 0)
        avg = new;
    else
        avg = (avg * 9 + new) / 10;

    uint16_t adc = avg;

    if (adc > ADC_DRY) adc = ADC_DRY;
    if (adc < ADC_WET) adc = ADC_WET;

    return (ADC_DRY - adc) * 100 / (ADC_DRY - ADC_WET);
}
