#include <Arduino.h>
#include "config.h"
#include "hbi.h"

HBI::HBI(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema)
{
    this->i2c = i2c;
    this->i2cSema = i2cSema;
}

// void InputListenerTask( void * parameter ) 
// {
//     while(1) {
//         vTaskDelay(100);
//     }
// }

void HBI::startInputListener()
{
    attachInterrupt(GPIO_HBI_INT, []() {
        Serial.println("HBI interrupt");
    }, CHANGE);

    // xTaskCreate(InputListenerTask, "hbi_input", 100, NULL, TASK_PRIO_HBI_INPUT_LISTENER, this->listenerTaskHandle);
}

