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

#define LOOP_PERIOD_MS   50        // 屏幕刷新周期
#define UPLOAD_PERIOD_MS 200       // 四分之一MQTT上传云平台周期
#define SOIL_HYSTERESIS_PERCENT 5  // 土壤湿度阈值滞后量，防止继电器频繁开关

typedef enum
{
    MODE_SOIL = 0,
    MODE_BH1750,
    MODE_DHT11
} SystemMode;

SystemMode CurrentMode = MODE_SOIL;

static uint32_t dht11_timer  = 0;
static uint32_t upload_timer = 0;

static DHT11_Data_TypeDef dht11_data;

uint8_t soil_threshold          = 30;  // 默认阈值
static  uint8_t soil_alarm_flag = 0;   // 土壤湿度告警状态变量

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

            OLED_Clear();
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

        // 读取传感器
        uint8_t  soil  = Soil_Moisture_Percent();
        uint16_t light = BH1750_ReadLux();

        dht11_timer += LOOP_PERIOD_MS;
        if(dht11_timer >= 1000)
        {
            if(DHT11_Read_Data(&dht11_data) == 0)
            {
                dht11_timer = 0;
            }
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

        // 控制MOS或继电器（高电平触发模式下）开断
        if(soil_alarm_flag)
        {
            GPIO_SetBits(RELAY_PORT, RELAY_PIN);  // 导通
        }
        else
        {
            GPIO_ResetBits(RELAY_PORT, RELAY_PIN);  // 断开
        }

        // OLED根据模式显示
        switch(CurrentMode)
        {
            case MODE_SOIL: 
                OLED_ShowString(1,1,"Soil:");
                OLED_ShowNum(2,1,soil,3);
                OLED_ShowChar(2,4,'%');
						
				OLED_ShowString(3,1,"Threshold:");
				OLED_ShowNum(4,1,soil_threshold,3);
				OLED_ShowChar(4,4,'%');
                break;

            case MODE_BH1750: 
                OLED_ShowString(1,1,"Light:");
                OLED_ShowNum(2,1,light,5);
                OLED_ShowString(2,7,"lux");
                break;

            case MODE_DHT11: 
                OLED_ShowString(1,1,"Temp:");
                OLED_ShowNum(1,6,dht11_data.temp_int,2);
                OLED_ShowChar(1,8,'C');

                OLED_ShowString(2,1,"Humi:");
                OLED_ShowNum(2,6,dht11_data.humi_int,2);
                OLED_ShowChar(2,8,'%');
                break;
        }
				
        // MQTT上传
        upload_timer += LOOP_PERIOD_MS;
        
        // 理论上每UPLOAD_PERIOD_MS上传一次，但实际一个循环约为250ms而非50ms
        // 因此，实际上每5*UPLOAD_PERIOD_MS上传一次
        if(upload_timer >= UPLOAD_PERIOD_MS)
        {
            upload_timer = 0;
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
