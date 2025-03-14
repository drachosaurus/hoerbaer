#include <Arduino.h>
#include <cstdarg>
#include "log.h"

SemaphoreHandle_t logSema;

void Log::init()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    
#if ARDUINO_USB_CDC_ON_BOOT
    usleep(300 * 1000); // give usb serial some time to connect (switch to CDC)
#endif
    logSema = xSemaphoreCreateBinary();
    xSemaphoreGive(logSema);
}

void Log::println(const char * module, const char * fmt, ...) 
{
    va_list va;
    va_start (va, fmt);
    char buf[255];
    vsprintf(buf, fmt, va);
    va_end (va);
    xSemaphoreTake(logSema, portMAX_DELAY);
    Serial.printf("%s\t%s\n", module, buf);
    xSemaphoreGive(logSema);
}
