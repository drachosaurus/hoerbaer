#pragma once

#include <Arduino.h>

#define byte uint8_t
#include <MFRC522DriverPinSimple.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522v2.h>

#include <SPI.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <array>
#include <memory>

#include "audioplayer.h"
#include "config.h"
#include "userconfig.h"

class RFID {
public:
    RFID(std::shared_ptr<UserConfig> userConfig, std::shared_ptr<AudioPlayer> audioPlayer);
    void initialize();

private:
    void beginBus();
    bool configureReader();
    static void workerTaskEntry(void* arg);
    void workerLoop();
    void processTag();
    bool isSameUid(const MFRC522::Uid& uid) const;
    void rememberUid(const MFRC522::Uid& uid);
    void handleMappedTag(const MFRC522::Uid& uid, const char* uidString);

    std::shared_ptr<UserConfig> _userConfig;
    std::shared_ptr<AudioPlayer> _audioPlayer;
    std::unique_ptr<SPIClass> _spiBus;
    std::unique_ptr<MFRC522DriverPinSimple> _chipSelectPin;
    std::unique_ptr<MFRC522DriverSPI> _driver;
    std::unique_ptr<MFRC522> _reader;
    TaskHandle_t _workerTaskHandle;
    std::array<uint8_t, 10> _lastUidBytes;
    uint8_t _lastUidSize;
    bool _hasLastUid;
};
