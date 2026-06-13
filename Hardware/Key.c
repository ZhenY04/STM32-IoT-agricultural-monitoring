#include "stm32f10x.h"
#include "Key.h"

#define KEY_DEBOUNCE_TICKS 2  // 10ms * 2 = 20ms

static volatile uint8_t Key_Event[KEY_NUM];
static uint8_t Key_State[KEY_NUM];  // 0: released, 1: pressed
static uint8_t Key_Count[KEY_NUM];

static const uint16_t Key_Pin[KEY_NUM] = 
{
    KEY_MODE_PIN,
    KEY_PLUS_PIN,
    KEY_MINUS_PIN
};

void Key_Init(void)
{
    RCC_APB2PeriphClockCmd(KEY_RCC, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructrue;
    GPIO_InitStructrue.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructrue.GPIO_Pin   = KEY_MODE_PIN | KEY_PLUS_PIN | KEY_MINUS_PIN;
    GPIO_InitStructrue.GPIO_Speed = GPIO_Speed_50MHz;                             // 实际上并没有“输入速度”这一说，在此没有意义，但还得写
    GPIO_Init(KEY_PORT, &GPIO_InitStructrue);
}

void Key_Tick(void)
{
    uint8_t i;

    for(i = 0; i < KEY_NUM; i++)
    {
        if(GPIO_ReadInputDataBit(KEY_PORT, Key_Pin[i]) == 0)
        {
            if(Key_Count[i] < KEY_DEBOUNCE_TICKS)
            {
                Key_Count[i]++;
            }
            else if(Key_State[i] == 0)
            {
                Key_State[i] = 1;
                Key_Event[i] = 1;
            }
        }
        else
        {
            Key_Count[i] = 0;
            Key_State[i] = 0;
        }
    }
}

uint8_t Key_GetEvent(uint8_t key)
{
    uint8_t event;

    if(key >= KEY_NUM)
    {
        return 0;
    }

    event          = Key_Event[key];
    Key_Event[key] = 0;

    return event;
}
