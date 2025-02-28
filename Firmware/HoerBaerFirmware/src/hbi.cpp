#include <Arduino.h>
#include "config.h"
#include "log.h"
#include "hbi.h"

#define QUEUE_CMD_INPUT_INTERRUPT 0xA0
#define QUEUE_CMD_ENCODER_L 0xB0
#define QUEUE_CMD_ENCODER_R 0xB1
#define QUEUE_CMD_SHUTDOWN 0xFF

#define ENCODER_DEBOUNCE_MS 10
#define ENCODER_BUTTON_LONG_MS 2000
#define ENCODER_BUTTON_DOWN_TICKS_NOT_STARTED 0 // magic number for tick counter
#define ENCODER_BUTTON_DOWN_TICKS_LONG_DONE UINT32_MAX // magic number for tick counter

static QueueHandle_t hbiWorkerInputQueue; // has to be static because of ISR usage
TickType_t encDebounceLastTicks = 0;
TickType_t encButtonDownTicks = ENCODER_BUTTON_DOWN_TICKS_NOT_STARTED;

HBI::HBI(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema, shared_ptr<HBIConfig> hbiConfig, shared_ptr<AudioPlayer> audioPlayer, void (*shutdownCallback)(void))
{
    this->i2c = i2c;
    this->i2cSema = i2cSema;
    this->hbiConfig = hbiConfig;
    this->audioPlayer = audioPlayer;
    this->shutdownCallback = shutdownCallback;

    this->ledDriver1 = make_unique<TLC59108>(i2c, I2C_ADDR_LED_DRIVER1);
    this->ledDriver2 = make_unique<TLC59108>(i2c, I2C_ADDR_LED_DRIVER2);
    this->ledDriver3 = make_unique<TLC59108>(i2c, I2C_ADDR_LED_DRIVER3);

    this->ioExpander1 = make_unique<PCF8574>(i2c, I2C_ADDR_IO_EXPANDER1);
    this->ioExpander2 = make_unique<PCF8574>(i2c, I2C_ADDR_IO_EXPANDER2);
    this->ioExpander3 = make_unique<PCF8574>(i2c, I2C_ADDR_IO_EXPANDER3);

    this->lastKnownButtonMask = 0x00FFFFFF;

    pinMode(GPIO_HBI_ENCODER_BTN, INPUT);
    pinMode(GPIO_HBI_ENCODER_A, INPUT);
    pinMode(GPIO_HBI_ENCODER_B, INPUT);
}

HBI::~HBI()
{
    uint8_t command = QUEUE_CMD_SHUTDOWN;
    xQueueSend(hbiWorkerInputQueue, &command, portMAX_DELAY);
}

void HBIWorkerTask(void * param) 
{
    HBI* hbi = (static_cast<HBI*>(param));
    hbi->runWorkerTask();
}

void HBI::runWorkerTask() 
{
    uint8_t command;
    while(1) 
    {
        if(xQueueReceive(hbiWorkerInputQueue, &command, pdMS_TO_TICKS(200)) == pdTRUE) 
        {
            switch(command) 
            {
                case QUEUE_CMD_INPUT_INTERRUPT: 
                {
                    xSemaphoreTake(this->i2cSema, portMAX_DELAY);
                    uint8_t io1 = this->ioExpander1->read8();
                    uint8_t io2 = this->ioExpander2->read8();
                    uint8_t io3 = this->ioExpander3->read8();
                    xSemaphoreGive(this->i2cSema);
                    this->dispatchButtonInput(io1 | (io2 << 8) | (io3 << 16));
                    break;
                }
                case QUEUE_CMD_ENCODER_L:
                {
                    Log::println("HBI", "Encoder left");
                    this->audioPlayer->volumeDown();
                    break;
                }
                case QUEUE_CMD_ENCODER_R:
                {
                    Log::println("HBI", "Encoder right");
                    this->audioPlayer->volumeUp();
                    break;
                }
                case QUEUE_CMD_SHUTDOWN:
                {
                    Log::println("HBI", "Shutdown");
                    vTaskDelete(NULL);
                    break;
                }
                default:
                {
                    Log::println("HBI", "Unknown command: %d", command);
                    break;
                }
            }
        }

        this->checkLongPressState();
        this->setLedState();
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

    attachInterrupt(GPIO_HBI_ENCODER_A, []() {
        auto ticks = xTaskGetTickCount();
        auto diff = ticks - encDebounceLastTicks;
        if(encDebounceLastTicks != 0) 
        {
            encDebounceLastTicks = ticks;
            if(diff < pdMS_TO_TICKS(ENCODER_DEBOUNCE_MS))
                return;
            auto encoderA = digitalRead(GPIO_HBI_ENCODER_A);
            auto encoderB = digitalRead(GPIO_HBI_ENCODER_B);
            uint8_t command = encoderA == encoderB ? QUEUE_CMD_ENCODER_L : QUEUE_CMD_ENCODER_R;
            xQueueSendFromISR(hbiWorkerInputQueue, &command, NULL);
        }
        else
            encDebounceLastTicks = ticks;
    }, CHANGE);

    xTaskCreate(HBIWorkerTask, "hbi_worker", 
        TASK_STACK_SIZE_HBI_WORKER, 
        this, 
        TASK_PRIO_HBI_WORKER,
        NULL);
}

void HBI::dispatchButtonInput(uint32_t buttonMask)
{
    auto diff = buttonMask ^ this->lastKnownButtonMask;
    if(diff == 0)
        return;
    bool release = (diff & buttonMask) != 0;
    this->lastKnownButtonMask = buttonMask;

    // release && "don't use release instead of press" ==> drop
    if(release == !this->hbiConfig->releaseInsteadOfPress)
        return;

    uint8_t mapping = IO_MAPPING_TYPE_NONE;
    int iSlot = 0;

    for(int i=0; i<24; i++) 
    {
        if((diff & (1 << i)) > 0) 
        {
            Log::println("HBI", "Button: %d %s", i, (release ? "release" : "press"));
            mapping = this->hbiConfig->ioMapping[i];
            break;
        }
        
        if(this->hbiConfig->ioMapping[i] == IO_MAPPING_TYPE_PLAY_SLOT)
            iSlot++;
    }

    switch(mapping) 
    {
        case IO_MAPPING_TYPE_PLAY_SLOT:
            Log::println("HBI", "Play slot: %d", iSlot);
            this->audioPlayer->playNextFromSlot(iSlot);
            break;

        case IO_MAPPING_TYPE_CONTROL_PLAY:
            Log::println("HBI", "Control play");
            this->audioPlayer->play();
            break;

        case IO_MAPPING_TYPE_CONTROL_STOP:
            Log::println("HBI", "Control stop");
            this->audioPlayer->stop();
            break;
        
        case IO_MAPPING_TYPE_CONTROL_PAUSE:
            Log::println("HBI", "Control pause");
            this->audioPlayer->pause();
            break;

        case IO_MAPPING_TYPE_CONTROL_NEXT:
            Log::println("HBI", "Control next");
            this->audioPlayer->next();
            break;

        case IO_MAPPING_TYPE_CONTROL_PREV:
            Log::println("HBI", "Control prev");
            this->audioPlayer->prev();
            break;

        case IO_MAPPING_TYPE_NONE:
        default:
            break;
    }
}

void HBI::checkLongPressState()
{
    // check long press timeout
    auto encBtn = digitalRead(GPIO_HBI_ENCODER_BTN);
    auto ticks = xTaskGetTickCount();
    auto diff = ticks - encButtonDownTicks;
    if(encButtonDownTicks != ENCODER_BUTTON_DOWN_TICKS_NOT_STARTED && encButtonDownTicks != ENCODER_BUTTON_DOWN_TICKS_LONG_DONE && diff > pdMS_TO_TICKS(ENCODER_BUTTON_LONG_MS)) 
    {
        encButtonDownTicks = ENCODER_BUTTON_DOWN_TICKS_LONG_DONE;
        dispatchEncoderButton(true);
    } 
    else if(encButtonDownTicks == ENCODER_BUTTON_DOWN_TICKS_NOT_STARTED && encBtn == LOW) 
    {
        // Log::println("HBI", "Encoder button down");
        encButtonDownTicks = xTaskGetTickCount();
    }
    else if(encButtonDownTicks != ENCODER_BUTTON_DOWN_TICKS_NOT_STARTED && encButtonDownTicks != ENCODER_BUTTON_DOWN_TICKS_LONG_DONE && encBtn == HIGH && diff < pdMS_TO_TICKS(ENCODER_BUTTON_LONG_MS) && diff > pdMS_TO_TICKS(ENCODER_DEBOUNCE_MS))
    {
        // Log::println("HBI", "Encoder button short");
        encButtonDownTicks = ENCODER_BUTTON_DOWN_TICKS_NOT_STARTED;
        dispatchEncoderButton(false);
    }
    else if(encButtonDownTicks == ENCODER_BUTTON_DOWN_TICKS_LONG_DONE && encBtn == HIGH) 
    {
        // Log::println("HBI", "Encoder reset to not started");
        encButtonDownTicks = ENCODER_BUTTON_DOWN_TICKS_NOT_STARTED;
    }
}

void HBI::dispatchEncoderButton(bool longPress)
{
    if(longPress) 
    {
        Log::println("HBI", "Encoder button long press. Call shutdown callback.");
        this->shutdownCallback();
    }
    else
        Log::println("HBI", "Encoder button short press. Do nothing.");
}

void HBI::setLedState() 
{
    if(this->audioPlayer->getPlayingInfo() == nullptr)
    {
        xSemaphoreTake(this->i2cSema, portMAX_DELAY);
        this->ledDriver1->setAllBrightness((uint8_t)0x00);
        this->ledDriver2->setAllBrightness((uint8_t)0x00);
        this->ledDriver3->setAllBrightness((uint8_t)0x00);
        xSemaphoreGive(this->i2cSema);
        return;
    }

    auto playingInfo = this->audioPlayer->getPlayingInfo();
    if(playingInfo == nullptr)
        return;

    auto slot = playingInfo->slot;

    // find out which led driver to use
    int iSlot = 0;
    for(int i=0; i<24; i++) 
    {
        if(iSlot == slot)
        {
            // set led
            xSemaphoreTake(this->i2cSema, portMAX_DELAY);
            this->ledDriver1->setAllBrightness((uint8_t)0x00);
            this->ledDriver2->setAllBrightness((uint8_t)0x00);
            this->ledDriver3->setAllBrightness((uint8_t)0x00);

            switch(i / 8) 
            {
                case 0:
                    this->ledDriver1->setBrightness(i % 8, (uint8_t)0xFF);
                    break;
                case 1:
                    this->ledDriver2->setBrightness(i % 8, (uint8_t)0xFF);
                    break;
                case 2:
                    this->ledDriver3->setBrightness(i % 8, (uint8_t)0xFF);
                    break;
            }

            xSemaphoreGive(this->i2cSema);
            break;
        }

        if(this->hbiConfig->ioMapping[i] == IO_MAPPING_TYPE_PLAY_SLOT)
            iSlot++;
    }
}