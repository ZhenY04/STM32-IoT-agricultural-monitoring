#include "stm32f10x.h"
#include "Delay.h"
#include "Timer.h"

#include "Key.h"
#include "OLED.h"

#include "Soil_Moisture.h"
#include "DHT11.h"
#include "MyI2C.h"
#include "BH1750.h"

#include "Serial.h"
#include "MQTT.h"

#include "Relay.h"

#define MQTT_UPLOAD_PERIOD_TICKS 500  // 10ms * 500 = 5s
#define DHT11_SAMPLE_PERIOD_TICKS 100  // 10ms * 100 = 1s
#define DISPLAY_REFRESH_PERIOD_TICKS 5  // 10ms * 5 = 50ms

#define SOIL_HYSTERESIS_PERCENT 5  // Prevent frequent relay switching

typedef enum
{
    MODE_SOIL = 0,
    MODE_BH1750,
    MODE_DHT11,
    MODE_COUNT
} SystemMode;

SystemMode CurrentMode = MODE_SOIL;

static uint32_t dht11_sample_tick = 0;
static uint32_t mqtt_upload_tick = 0;
static uint32_t display_refresh_tick = 0;

static DHT11_Data_TypeDef dht11_data;

typedef struct
{
    SystemMode mode;
    uint8_t soil;
    uint8_t soil_threshold;
    uint16_t light;
    uint8_t temperature;
    uint8_t humidity;
} DisplayState;

static DisplayState display_state = {MODE_COUNT, 0, 0, 0, 0, 0};

uint8_t soil_threshold          = 30;  // Default threshold
static  uint8_t soil_alarm_flag = 0;   // Soil moisture alarm flag

static void DisplayStaticContent(SystemMode mode)
{
    switch(mode)
    {
        case MODE_SOIL:
            OLED_ShowString(1, 1, "Soil:");
            OLED_ShowChar(2, 4, '%');
            OLED_ShowString(3, 1, "Threshold:");
            OLED_ShowChar(4, 4, '%');
            break;

        case MODE_BH1750:
            OLED_ShowString(1, 1, "Light:");
            OLED_ShowString(2, 7, "lux");
            break;

        case MODE_DHT11:
            OLED_ShowString(1, 1, "Temp:");
            OLED_ShowChar(1, 8, 'C');
            OLED_ShowString(2, 1, "Humi:");
            OLED_ShowChar(2, 8, '%');
            break;

        default:
            break;
    }
}

static uint8_t DisplayDataChanged(SystemMode mode, uint8_t soil, uint16_t light)
{
    switch(mode)
    {
        case MODE_SOIL:
            return (soil != display_state.soil) ||
                   (soil_threshold != display_state.soil_threshold);

        case MODE_BH1750:
            return light != display_state.light;

        case MODE_DHT11:
            return (dht11_data.temp_int != display_state.temperature) ||
                   (dht11_data.humi_int != display_state.humidity);

        default:
            return 0;
    }
}

static void DisplayData(SystemMode mode, uint8_t soil, uint16_t light)
{
    switch(mode)
    {
        case MODE_SOIL:
            OLED_ShowNum(2, 1, soil, 3);
            OLED_ShowNum(4, 1, soil_threshold, 3);
            break;

        case MODE_BH1750:
            OLED_ShowNum(2, 1, light, 5);
            break;

        case MODE_DHT11:
            OLED_ShowNum(1, 6, dht11_data.temp_int, 2);
            OLED_ShowNum(2, 6, dht11_data.humi_int, 2);
            break;

        default:
            break;
    }

    display_state.soil = soil;
    display_state.soil_threshold = soil_threshold;
    display_state.light = light;
    display_state.temperature = dht11_data.temp_int;
    display_state.humidity = dht11_data.humi_int;
}

int main(void)
{
    Key_Init();
    Timer_Init();

    OLED_Init();

    Soil_Moisture_Init();
    MyI2C_Init();
    BH1750_Init();
    DHT11_Init();

    Serial_Init();
	MQTT_Init();
	dht11_sample_tick = Timer_GetTick();
	mqtt_upload_tick = Timer_GetTick();
	display_refresh_tick = Timer_GetTick();
	
	Relay_Init();

    OLED_Clear();

    while(1)
    {
        if(Key_GetEvent(KEY_MODE))
        {
            CurrentMode++;
            if(CurrentMode > MODE_DHT11)
            {
                CurrentMode = MODE_SOIL;
            }
        }

        if(Key_GetEvent(KEY_PLUS))
        {
            if(soil_threshold < 100)
            {
                soil_threshold += 10;
            }
        }

        if(Key_GetEvent(KEY_MINUS))
        {
            if(soil_threshold > 0)
            {
                soil_threshold -= 10;
            }
        }

        uint8_t soil_pump_off_threshold = soil_threshold + SOIL_HYSTERESIS_PERCENT;
        if(soil_pump_off_threshold > 100)
        {
            soil_pump_off_threshold = 100;
        }

        // Sensor reading
        uint8_t  soil  = Soil_Moisture_Percent();
        uint16_t light = BH1750_ReadLux();

        // DHT11 driven by TIM2 to keep at least 1s between samplings
        if((uint32_t)(Timer_GetTick() - dht11_sample_tick) >= DHT11_SAMPLE_PERIOD_TICKS)
        {
            dht11_sample_tick = Timer_GetTick();
            DHT11_Read_Data(&dht11_data);
        }
				
        if(soil_alarm_flag)
        {
            if(soil >= soil_pump_off_threshold)
            {
                soil_alarm_flag = 0;
            }
        }
        else
        {
            if(soil < soil_threshold)
            {
                soil_alarm_flag = 1;
            }
        }

        // Relay control
        if(soil_alarm_flag)
        {
            GPIO_SetBits(RELAY_PORT, RELAY_PIN);
        }
        else
        {
            GPIO_ResetBits(RELAY_PORT, RELAY_PIN);
        }

        // OLED display updates are limited to 50ms intervals.
        if((uint32_t)(Timer_GetTick() - display_refresh_tick) >= DISPLAY_REFRESH_PERIOD_TICKS)
        {
            display_refresh_tick = Timer_GetTick();
            if(display_state.mode != CurrentMode)
            {
                OLED_Clear();
                DisplayStaticContent(CurrentMode);
                display_state.mode = CurrentMode;
                DisplayData(CurrentMode, soil, light);
            }
            else if(DisplayDataChanged(CurrentMode, soil, light))
            {
                DisplayData(CurrentMode, soil, light);
            }
        }
				
        // MQTT upload based on TIM2 interrupt
        // Upload period: TIM2 interrupt period * MQTT_UPLOAD_PERIOD_TICKS = 10ms * 500 = 5s
        if((uint32_t)(Timer_GetTick() - mqtt_upload_tick) >= MQTT_UPLOAD_PERIOD_TICKS)
        {
            mqtt_upload_tick += MQTT_UPLOAD_PERIOD_TICKS;
            MQTT_Upload(
                soil,
                dht11_data.temp_int,
                dht11_data.humi_int,
                light,
                soil_alarm_flag
            );
        }
    }
}
