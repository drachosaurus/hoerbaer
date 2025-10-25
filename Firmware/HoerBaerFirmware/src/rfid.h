#pragma once

#include <Arduino.h>
#include <MFRC522.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <memory>

#include "config.h"
#include "userconfig.h"
#include "audioplayer.h"

class RFID {
public:
    RFID(std::shared_ptr<UserConfig> userConfig, std::shared_ptr<AudioPlayer> audioPlayer);
    void initialize();

private:
    void beginBus();
    void configureReader();
    void configureInterrupt();
    static void IRAM_ATTR onInterruptStatic(void* arg);
    static void workerTaskEntry(void* arg);
    void handleInterruptFromISR();
    void workerLoop();
    void processTag();

    std::shared_ptr<UserConfig> _userConfig;
    std::shared_ptr<AudioPlayer> _audioPlayer;
    std::unique_ptr<TwoWire> _bus;
    std::unique_ptr<MFRC522_I2C> _driver;
    std::unique_ptr<MFRC522> _reader;
    QueueHandle_t _irqQueue;
    TaskHandle_t _workerTaskHandle;
};
