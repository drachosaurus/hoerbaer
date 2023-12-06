#include <Arduino.h>
#include "config.h"
#include "log.h"
#include "hbi.h"

HBI::HBI(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema)
{
    this->i2c = i2c;
    this->i2cSema = i2cSema;

    this->ledDriver1 = make_unique<TLC59108>(i2c, I2C_ADDR_LED_DRIVER1);
    this->ledDriver2 = make_unique<TLC59108>(i2c, I2C_ADDR_LED_DRIVER2);
    this->ledDriver3 = make_unique<TLC59108>(i2c, I2C_ADDR_LED_DRIVER3);

    this->ioExpander1 = make_unique<PCF8574>(i2c, I2C_ADDR_IO_EXPANDER1);
}

// void InputListenerTask( void * parameter ) 
// {
//     while(1) {
//         vTaskDelay(100);
//     }
// }

void HBI::start()
{
    attachInterrupt(GPIO_HBI_INPUT_INT, []() {
        Log::println("HBI interrupt");
    }, FALLING);

    sleep(1);

    this->ledDriver1->init(GPIO_HBI_LEDDRIVER_RST);
    this->ledDriver2->init();
    this->ledDriver3->init();
    Log::println("HBI: LED drivers initialized.");

    this->ledDriver1->setLedOutputMode(TLC59108::LED_MODE::PWM_IND);
    this->ledDriver2->setLedOutputMode(TLC59108::LED_MODE::PWM_IND);
    this->ledDriver3->setLedOutputMode(TLC59108::LED_MODE::PWM_IND);
    Log::println("HBI: LED drivers output mode set");

    this->ledDriver1->setAllBrightness(0xC0);
    this->ledDriver2->setAllBrightness(0xC0);
    this->ledDriver3->setAllBrightness(0xC0);
    Log::println("HBI: Lights on!");
    
    // xTaskCreate(InputListenerTask, "hbi_input", 100, NULL, TASK_PRIO_HBI_INPUT_LISTENER, this->listenerTaskHandle);
}

void HBI::test() {
    uint8_t io1 = this->ioExpander1->read8();
    Log::println("IO1: %d", io1);
}

