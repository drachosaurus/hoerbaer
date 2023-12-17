#include <Arduino.h>
#include "config.h"
#include "log.h"
#include "hbi.h"

#define QUEUE_CMD_INPUT_INTERRUPT 0xA0
#define QUEUE_CMD_CLEAR 0xC1
#define QUEUE_CMD_SHUTDOWN 0xFF

static QueueHandle_t hbiWorkerInputQueue; // has to be static because of ISR usage
bool vegasMode = false;

HBI::HBI(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema)
{
    this->i2c = i2c;
    this->i2cSema = i2cSema;

    this->ledDriver1 = make_unique<TLC59108>(i2c, I2C_ADDR_LED_DRIVER1);
    this->ledDriver2 = make_unique<TLC59108>(i2c, I2C_ADDR_LED_DRIVER2);
    this->ledDriver3 = make_unique<TLC59108>(i2c, I2C_ADDR_LED_DRIVER3);

    this->ioExpander1 = make_unique<PCF8574>(i2c, I2C_ADDR_IO_EXPANDER1);
    this->ioExpander2 = make_unique<PCF8574>(i2c, I2C_ADDR_IO_EXPANDER2);
    this->ioExpander3 = make_unique<PCF8574>(i2c, I2C_ADDR_IO_EXPANDER3);

    this->lastKnownButtonMask = 0x00FFFFFF;
}

void VegasStep(HBI* hbi) 
{
    uint8_t a = (rand() % static_cast<int>(8));
    uint8_t b = (rand() % static_cast<int>(8));
    uint8_t c = (rand() % static_cast<int>(8));
    uint8_t d = (rand() % static_cast<int>(8));
    uint8_t e = (rand() % static_cast<int>(8));
    uint8_t f = (rand() % static_cast<int>(8));
    xSemaphoreTake(hbi->i2cSema, portMAX_DELAY);
    hbi->ledDriver1->setAllBrightness((uint8_t)0x00);
    hbi->ledDriver1->setBrightness(a, (uint8_t)0xFF);
    hbi->ledDriver1->setBrightness(b, (uint8_t)0x00);
    hbi->ledDriver2->setAllBrightness((uint8_t)0xFF);
    hbi->ledDriver2->setBrightness(c, (uint8_t)0x00);
    hbi->ledDriver2->setBrightness(d, (uint8_t)0xFF);
    hbi->ledDriver3->setAllBrightness((uint8_t)0x00);
    hbi->ledDriver3->setBrightness(e, (uint8_t)0xFF);
    hbi->ledDriver3->setBrightness(f, (uint8_t)0x00);
    rand();
    xSemaphoreGive(hbi->i2cSema);
}

void HBIWorkerTask(void * param) 
{
    HBI* hbi = (static_cast<HBI*>(param));
    uint8_t command;
    while(1) 
    {
        if(xQueueReceive(hbiWorkerInputQueue, &command, pdMS_TO_TICKS(200)) != pdTRUE) 
        {
            if(vegasMode) 
                VegasStep(hbi);
            continue;
        }

        switch(command) 
        {
            case QUEUE_CMD_INPUT_INTERRUPT: 
            {
                xSemaphoreTake(hbi->i2cSema, portMAX_DELAY);
                uint8_t io1 = hbi->ioExpander1->read8();
                uint8_t io2 = hbi->ioExpander2->read8();
                uint8_t io3 = hbi->ioExpander3->read8();
                xSemaphoreGive(hbi->i2cSema);
                hbi->dispatchButtonInput(io1 | (io2 << 8) | (io3 << 16));
                break;
            }
            case QUEUE_CMD_CLEAR: 
                xSemaphoreTake(hbi->i2cSema, portMAX_DELAY);
                hbi->ledDriver1->setAllBrightness((uint8_t)0x00);
                hbi->ledDriver2->setAllBrightness((uint8_t)0x00);
                hbi->ledDriver3->setAllBrightness((uint8_t)0x00);
                xSemaphoreGive(hbi->i2cSema);
                break;
            case QUEUE_CMD_SHUTDOWN:
                Log::println("HBI", "Shutdown");
                vTaskDelete(NULL);
                break;
            default:
                Log::println("HBI", "Unknown command: %d", command);
                break;
        }
    }
}

void HBI::start()
{
    // Initialize devices
    xSemaphoreTake(this->i2cSema, portMAX_DELAY);

    this->ledDriver1->init(GPIO_HBI_LEDDRIVER_RST);
    this->ledDriver2->init();
    this->ledDriver3->init();
    Log::println("HBI", "LED drivers initialized.");

    this->ledDriver1->setLedOutputMode(TLC59108::LED_MODE::PWM_IND);
    this->ledDriver2->setLedOutputMode(TLC59108::LED_MODE::PWM_IND);
    this->ledDriver3->setLedOutputMode(TLC59108::LED_MODE::PWM_IND);
    Log::println("HBI", "LED drivers output mode set");

    xSemaphoreGive(this->i2cSema);

    hbiWorkerInputQueue = xQueueCreate(10, sizeof(uint8_t));

    attachInterrupt(GPIO_HBI_INPUT_INT, []() {
        uint8_t command = QUEUE_CMD_INPUT_INTERRUPT;
        xQueueSendFromISR(hbiWorkerInputQueue, &command, NULL);
    }, FALLING);

    xTaskCreate(HBIWorkerTask, "hbi_worker", 
        TASK_STACK_SIZE_HBI_WORKER, 
        this, 
        TASK_PRIO_HBI_WORKER,
        NULL);
}

HBI::~HBI()
{
    uint8_t command = QUEUE_CMD_SHUTDOWN;
    xQueueSend(hbiWorkerInputQueue, &command, portMAX_DELAY);
}

void HBI::enableVegas()
{
    vegasMode = true;
    uint8_t command = QUEUE_CMD_CLEAR;
    xQueueSend(hbiWorkerInputQueue, &command, portMAX_DELAY);
    Log::println("HBI", "Vegas enabled");
}

void HBI::disableVegas()
{
    vegasMode = false;
    uint8_t command = QUEUE_CMD_CLEAR;
    xQueueSend(hbiWorkerInputQueue, &command, portMAX_DELAY);
    Log::println("HBI", "Vegas disabled");
}

void HBI::dispatchButtonInput(uint32_t buttonMask)
{
    auto diff = buttonMask ^ this->lastKnownButtonMask;
    if(diff == 0)
        return;
    bool up = (diff & buttonMask) != 0;

    Log::println("HBI", "Button: %d %s", diff, (up ? "up" : "down"));

    this->lastKnownButtonMask = buttonMask;
}
