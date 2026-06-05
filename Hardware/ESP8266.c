#include "string.h"
#include "ESP8266.h"
#include "Serial.h"
#include "Delay.h"

void ESP8266_SendCmd(char *cmd)
{
    Serial_SendString(cmd);
}
