#include "rfid.h"

#include <cstdio>
#include <utility>

#include "log.h"

namespace {
constexpr size_t RFID_IRQ_QUEUE_LENGTH = 4;
constexpr uint32_t RFID_IRQ_EVENT = 1U;
constexpr size_t RFID_UID_BUFFER_LENGTH = 32;
}

RFID::RFID(std::shared_ptr<UserConfig> userConfig, std::shared_ptr<AudioPlayer> audioPlayer)
        : _userConfig(std::move(userConfig)),
            _audioPlayer(std::move(audioPlayer)),
            _bus(std::make_unique<TwoWire>(RFID_I2C_PORT)),
            _driver(nullptr),
            _reader(nullptr),
            _irqQueue(nullptr),
            _workerTaskHandle(nullptr) {
        _driver = std::make_unique<MFRC522_I2C>(GPIO_RFID_RST, RFID_I2C_ADDRESS, *_bus);
        _reader = std::make_unique<MFRC522>(_driver.get());
}

void RFID::initialize() {
    beginBus();
    configureReader();
    configureInterrupt();
}

void RFID::beginBus() {
    pinMode(GPIO_RFID_RST, OUTPUT);
    digitalWrite(GPIO_RFID_RST, HIGH);

    _bus->begin(GPIO_RFID_SDA, GPIO_RFID_SCL, RFID_I2C_FREQUENCY);
    _bus->setClock(RFID_I2C_FREQUENCY);
}

void RFID::configureReader() {
    if (!_reader || !_driver) {
        Log::println("RFID", "Reader driver not available");
        return;
    }

    digitalWrite(GPIO_RFID_RST, LOW);
    delay(5);
    digitalWrite(GPIO_RFID_RST, HIGH);
    delay(5);

    _reader->PCD_Init();
    _reader->PCD_SetAntennaGain(MFRC522::PCD_RxGain::RxGain_max);

    _reader->PCD_WriteRegister(MFRC522::PCD_Register::ComIEnReg, 0xA0 | 0x20 | 0x10);
    _reader->PCD_WriteRegister(MFRC522::PCD_Register::DivIEnReg, 0x80);
    _reader->PCD_WriteRegister(MFRC522::PCD_Register::ComIrqReg, 0x7F);

    Log::println("RFID", "MFRC522 ready on I2C address 0x%02X", RFID_I2C_ADDRESS);
}

void RFID::configureInterrupt() {
    if (_irqQueue == nullptr) {
        _irqQueue = xQueueCreate(RFID_IRQ_QUEUE_LENGTH, sizeof(uint32_t));
    }

    if (_irqQueue == nullptr) {
        Log::println("RFID", "Failed to allocate IRQ queue");
        return;
    }

    pinMode(GPIO_RFID_IRQ, INPUT_PULLUP);

    attachInterruptArg(GPIO_RFID_IRQ, &RFID::onInterruptStatic, this, FALLING);

    if (_workerTaskHandle == nullptr) {
        auto result = xTaskCreate(
            RFID::workerTaskEntry,
            "rfid_irq",
            TASK_STACK_SIZE_RFID_WORKER_WORDS,
            this,
            TASK_PRIO_RFID_WORKER,
            &_workerTaskHandle);

        if (result != pdPASS) {
            Log::println("RFID", "Failed to create worker task");
            _workerTaskHandle = nullptr;
        }
    }
}

void IRAM_ATTR RFID::onInterruptStatic(void* arg) {
    auto* self = static_cast<RFID*>(arg);
    if (self != nullptr) {
        self->handleInterruptFromISR();
    }
}

void RFID::handleInterruptFromISR() {
    if (_irqQueue == nullptr) {
        return;
    }

    BaseType_t higherPriorityTaskWoken = pdFALSE;
    uint32_t event = RFID_IRQ_EVENT;
    xQueueSendFromISR(_irqQueue, &event, &higherPriorityTaskWoken);
    if (higherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

void RFID::workerTaskEntry(void* arg) {
    auto* self = static_cast<RFID*>(arg);
    if (self != nullptr) {
        self->workerLoop();
    }
}

void RFID::workerLoop() {
    uint32_t event = 0;
    while (true) {
        if (xQueueReceive(_irqQueue, &event, portMAX_DELAY) == pdTRUE) {
            processTag();
        }
    }
}

void RFID::processTag() {
    if (!_reader) {
        return;
    }

    _reader->PCD_WriteRegister(MFRC522::PCD_Register::ComIrqReg, 0x7F);

    if (!_reader->PICC_IsNewCardPresent()) {
        return;
    }

    if (!_reader->PICC_ReadCardSerial()) {
        return;
    }

    char uidBuffer[RFID_UID_BUFFER_LENGTH] = {0};
    size_t offset = 0;
    for (uint8_t i = 0; i < _reader->uid.size && offset < sizeof(uidBuffer); ++i) {
        int written = snprintf(uidBuffer + offset, sizeof(uidBuffer) - offset, "%02X", _reader->uid.uidByte[i]);
        if (written < 0) {
            break;
        }
        offset += static_cast<size_t>(written);
        if (i + 1 < _reader->uid.size && offset < sizeof(uidBuffer) - 1) {
            uidBuffer[offset++] = ':';
        }
    }

    Log::println("RFID", "Tag UID %s", uidBuffer);

    _reader->PICC_HaltA();
    _reader->PCD_StopCrypto1();
}
