#include "stm32f10x.h"                  // Device header
#include "MQTT.h"
#include "Serial.h"
#include "Delay.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "WIFI_Config.h"
#include "MQTT_Config.h"

extern char Serial_RxBuffer[];
extern volatile uint16_t Serial_RxIndex;
extern uint8_t soil_threshold;   // ����main��ı���

static int msg_id = 0;

/**
 * @brief  MQTT��ʼ����ĿǰΪ����ʽ���������Ż�Ϊ�����жϡ�״̬���ķ�ʽ�������١��ȶ�
 */
void MQTT_Init(void)
{
	Serial_SendString("AT+RST\r\n");
	Delay_ms(3000);
	
	Serial_SendString("AT+CWMODE=1\r\n");
	Delay_ms(1000);

    Serial_SendString("AT+CWJAP=\"" WIFI_SSID "\",\"" WIFI_PASSWORD "\"\r\n");
    Delay_ms(6000);

    /* ===== ����MQTT���� ===== */
    Serial_SendString("AT+MQTTUSERCFG=0,1,\"" MQTT_CLIENT_ID "\",\"" MQTT_USERNAME "\"," "\"" MQTT_PASSWORD "\"" ",0,0,\"\"\r\n");
    Delay_ms(2000);

    /* ===== ����MQTT������ ===== */
    Serial_SendString("AT+MQTTCONN=0,\"" MQTT_BROKER "\",\" MQTT_PORT \",1\r\n");
    Delay_ms(3000);
}

/**
 * @brief  MQTT�ϴ�
 */
void MQTT_Upload(int soil, int temp, int humi, int light, int alarm_flag)
{
    char json[200];
    char cmd[256];

    msg_id++;    //��ϢID�����������

    /* ===== Build JSON ===== */
    sprintf(json,
        "{\"id\":\"%d\",\"version\":\"1.0\",\"params\":{"
        "\"Soil\":{\"value\":%d},"
        "\"Temp\":{\"value\":%d},"
        "\"Humi\":{\"value\":%d},"
        "\"Light\":{\"value\":%d},"
				"\"SoilAlarm\":{\"value\":%d}"
        "}}",
        msg_id, soil, temp, humi, light, alarm_flag);

    int len = strlen(json);

    /* ===== AT cmd ===== */
    sprintf(cmd,
        "AT+MQTTPUBRAW=0,\"%s\",%d,0,0\r\n",
        MQTT_TOPIC,
        len);

    /* ===== Send AT cmd ===== */
    Serial_SendString(cmd);

    /* wait for module reply (simplified version) */
    Delay_ms(200);

    /* ===== send JSON ===== */
    Serial_SendString(json);
}
