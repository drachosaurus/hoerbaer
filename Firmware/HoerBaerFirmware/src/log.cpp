#include <Arduino.h>
#include <cstdarg>
#include "log.h"

void Log::init()
{
    Serial.begin(115200);
#if ARDUINO_USB_CDC_ON_BOOT
    usleep(300 * 1000); // give usb serial some time to connect (switch to CDC)
#endif
}

void Log::println(const char * fmt) 
{
    Serial.println(fmt);
}

void Log::println(const char * fmt, ...) 
{
    va_list va;
    va_start (va, fmt);
    char buf[255];
    vsprintf(buf, fmt, va);
    Serial.println(buf);
    va_end (va);
}
