#include <Arduino.h>
#include "config.h"
#include "log.h"
#include "hbi.h"

#define QUEUE_CMD_INPUT_INTERRUPT 0xA0
#define QUEUE_CMD_ENCODER_L 0xB0
#define QUEUE_CMD_ENCODER_R 0xB1

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
    this->currentVegasStep = -1;
    this->playButtonsIoMask = 0;
    this->pauseButtonsIoMask = 0;
    this->powerLedsIoMask = 0;

    pinMode(GPIO_HBI_ENCODER_BTN, INPUT);
    pinMode(GPIO_HBI_ENCODER_A, INPUT);
    pinMode(GPIO_HBI_ENCODER_B, INPUT);

    int curSlot = 0;
    for(int io=0; io<24; io++) {
        uint8_t buttonIoMapping = (hbiConfig->ioMapping[io] & 0x0F);
        uint8_t ledIoMapping = (hbiConfig->ioMapping[io] & 0xF0);

        if(buttonIoMapping == IO_MAPPING_TYPE_PLAY_SLOT) {
            this->slotIos[curSlot] = io;
            this->ioSlots[io] = curSlot;
            curSlot++;
        }
        else if(buttonIoMapping == IO_MAPPING_TYPE_CONTROL_PLAY)
            this->playButtonsIoMask |= (1 << io);
        else if(buttonIoMapping == IO_MAPPING_TYPE_CONTROL_PAUSE)
            this->pauseButtonsIoMask |= (1 << io);

        if(ledIoMapping == LED_POWER_ON)
            this->powerLedsIoMask |= (1 << io);
    }
    this->slotCount = curSlot;
}

void HBIWorkerTask(void * param) 
{
    HBI* hbi = (static_cast<HBI*>(param));
    hbi->runWorkerTask();
}

uint32_t HBI::getButtonsState()
{
    xSemaphoreTake(this->i2cSema, portMAX_DELAY);
    uint8_t io1 = this->ioExpander1->read8();
    uint8_t io2 = this->ioExpander2->read8();
    uint8_t io3 = this->ioExpander3->read8();
    xSemaphoreGive(this->i2cSema);
    return io1 | (io2 << 8) | (io3 << 16);
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
                    uint32_t buttonMask = this->getButtonsState();
                    this->dispatchButtonInput(buttonMask);
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

void HBI::initialize()
{
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

    // Initialize devices
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
        TASK_STACK_SIZE_HBI_WORKER_WORDS, 
        this, 
        TASK_PRIO_HBI_WORKER,
        NULL);
}

void HBI::setReadyToPlay(bool ready) {
    this->readyToPlay = ready;
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

    int slotNumber = 0;

    for(int i=0; i<24; i++) 
    {
        if((diff & (1 << i)) > 0) 
        {
            Log::println("HBI", "Button: %d %s", i, (release ? "release" : "press"));
            mapping = this->hbiConfig->ioMapping[i] & 0x0F; // buttons commands are in the lower 4 bits
            slotNumber = ioSlots[i];
            break;
        }
    }

    if(!actionButtonsEnabled) {
        Log::println("HBI", "Action buttons disabled, ignoring button input.");
        return;
    }

    switch(mapping) 
    {
        case IO_MAPPING_TYPE_PLAY_SLOT:
            Log::println("HBI", "Play slot: %d", slotNumber);
            this->audioPlayer->playNextFromSlot(slotNumber);
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
    uint32_t ledState = 0;

    // set leds from current playing slot
    auto playingInfo = this->audioPlayer->getPlayingInfo();
    if(playingInfo != nullptr) {
        ledState |= 1 << slotIos[playingInfo->slot];
        if(playingInfo->pausedAtPosition > 0)
            ledState |= pauseButtonsIoMask;
        else
            ledState |= playButtonsIoMask;
    }

    // set leds from vegas step
    if(this->currentVegasStep > -1)
        ledState |= 1 << slotIos[this->currentVegasStep];
    
    // set leds from power on LED mapping
    if(this->readyToPlay)
        ledState |= this->powerLedsIoMask;

    if(this->currentLedState == ledState)
        return;

    this->currentLedState = ledState;

    xSemaphoreTake(this->i2cSema, portMAX_DELAY);

    this->ledDriver1->setAllBrightness((uint8_t)0x00);
    this->ledDriver2->setAllBrightness((uint8_t)0x00);
    this->ledDriver3->setAllBrightness((uint8_t)0x00);

    for(int i = 0; i<24; i++) 
    {
        if(ledState & (1 << i))
        {
            if(i < 8)
                this->ledDriver1->setBrightness(i, (uint8_t)0xFF);
            else if(i < 16)
                this->ledDriver2->setBrightness(i - 8, (uint8_t)0xFF);
            else
                this->ledDriver3->setBrightness(i - 16, (uint8_t)0xFF);
        }
    }

    xSemaphoreGive(this->i2cSema);
}

void HBI::lightUpAllLeds() {
    xSemaphoreTake(this->i2cSema, portMAX_DELAY);
    this->ledDriver1->setAllBrightness((uint8_t)0xFF);
    this->ledDriver1->setLedOutputMode(TLC59108::LED_MODE::PWM_IND);
    this->ledDriver2->setAllBrightness((uint8_t)0xFF);
    this->ledDriver2->setLedOutputMode(TLC59108::LED_MODE::PWM_IND);
    this->ledDriver3->setAllBrightness((uint8_t)0xFF);
    this->ledDriver3->setLedOutputMode(TLC59108::LED_MODE::PWM_IND);
    xSemaphoreGive(this->i2cSema);
}

void HBI::shutOffAllLeds() {
    xSemaphoreTake(this->i2cSema, portMAX_DELAY);
    this->ledDriver1->setAllBrightness((uint8_t)0x00);
    this->ledDriver1->setLedOutputMode(TLC59108::LED_MODE::OFF);
    this->ledDriver2->setAllBrightness((uint8_t)0x00);
    this->ledDriver2->setLedOutputMode(TLC59108::LED_MODE::OFF);
    this->ledDriver3->setAllBrightness((uint8_t)0x00);
    this->ledDriver3->setLedOutputMode(TLC59108::LED_MODE::OFF);
    xSemaphoreGive(this->i2cSema);
}

void HBI::waitUntilEncoderButtonReleased() {
    while(digitalRead(GPIO_HBI_ENCODER_BTN) == LOW)
        vTaskDelay(100);
}

bool HBI::getAnyButtonPressed() {
    auto buttonState = this->getButtonsState();
    return buttonState != 0xFFFFFF;
}

void HBI::runVegasStep() {
    this->currentVegasStep++;
    if(this->currentVegasStep >= slotCount)
        this->currentVegasStep = 0;
}

void HBI::setActionButtonsEnabled(bool enabled) {
    Log::println("HBI", "Set action buttons enabled: %s", enabled ? "true" : "false");
    this->actionButtonsEnabled = enabled;
}
