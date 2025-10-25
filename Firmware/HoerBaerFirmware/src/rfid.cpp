#include "rfid.h"

#include <algorithm>
#include <cstdio>
#include <string_view>
#include <utility>

#include "log.h"

namespace {
constexpr size_t RFID_UID_BUFFER_LENGTH = 32;
constexpr TickType_t RFID_POLL_DELAY = pdMS_TO_TICKS(100);
}

RFID::RFID(std::shared_ptr<UserConfig> userConfig, std::shared_ptr<AudioPlayer> audioPlayer)
    : _userConfig(std::move(userConfig)),
      _audioPlayer(std::move(audioPlayer)),
      _spiBus(std::make_unique<SPIClass>(HSPI)),
      _chipSelectPin(std::make_unique<MFRC522DriverPinSimple>(GPIO_RFID_SS)),
            _driver(nullptr),
            _reader(nullptr),
            _workerTaskHandle(nullptr),
            _lastUidBytes{},
            _lastUidSize(0),
            _hasLastUid(false) {
        _driver = std::make_unique<MFRC522DriverSPI>(*_chipSelectPin, *_spiBus);
    _reader = std::make_unique<MFRC522>(*_driver);
}

void RFID::initialize() {
    beginBus();
    if (!configureReader()) {
        return;
    }

    if (_workerTaskHandle == nullptr) {
        auto result = xTaskCreate(
            RFID::workerTaskEntry,
            "rfid_worker",
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

void RFID::beginBus() {
    pinMode(GPIO_RFID_RST, OUTPUT);
    digitalWrite(GPIO_RFID_RST, HIGH);

    pinMode(GPIO_RFID_SS, OUTPUT);
    digitalWrite(GPIO_RFID_SS, HIGH);

    if (_spiBus) {
        _spiBus->begin(GPIO_RFID_CLK, GPIO_RFID_MISO, GPIO_RFID_MOSI);
    }
}

bool RFID::configureReader() {
    if (!_reader || !_driver) {
        Log::println("RFID", "Reader driver not available");
        return false;
    }

    digitalWrite(GPIO_RFID_RST, LOW);
    delay(5);
    digitalWrite(GPIO_RFID_RST, HIGH);
    delay(5);

    if (!_reader->PCD_Init()) {
        Log::println("RFID", "Failed to initialize MFRC522 over SPI");
        return false;
    }

    _reader->PCD_SetAntennaGain(MFRC522::PCD_RxGain::RxGain_max);

    auto version = _reader->PCD_GetVersion();
    Log::println("RFID", "MFRC522 ready (version 0x%02X) on SPI", static_cast<unsigned int>(version));
    return true;
}

void RFID::workerTaskEntry(void* arg) {
    auto* self = static_cast<RFID*>(arg);
    if (self != nullptr) {
        self->workerLoop();
    }
}

void RFID::workerLoop() {
    while (true) {
        processTag();
        vTaskDelay(RFID_POLL_DELAY);
    }
}

void RFID::processTag() {
    if (!_reader) {
        return;
    }
    
    if (!_reader->PICC_IsNewCardPresent()) {
        _hasLastUid = false;
        return;
    }

    if (!_reader->PICC_ReadCardSerial()) {
        return;
    }

    if (isSameUid(_reader->uid)) {
        _reader->PICC_HaltA();
        _reader->PCD_StopCrypto1();
        return;
    }

    rememberUid(_reader->uid);

    char uidBuffer[RFID_UID_BUFFER_LENGTH] = {0};
    size_t offset = 0;
    for (uint8_t i = 0; i < _reader->uid.size && offset < sizeof(uidBuffer); ++i) {
        int written = std::snprintf(uidBuffer + offset, sizeof(uidBuffer) - offset, "%02X", _reader->uid.uidByte[i]);
        if (written < 0) {
            break;
        }
        offset += static_cast<size_t>(written);
        if (i + 1 < _reader->uid.size && offset < sizeof(uidBuffer) - 1) {
            uidBuffer[offset++] = ':';
        }
    }

    Log::println("RFID", "Tag UID %s", uidBuffer);

    handleMappedTag(_reader->uid, uidBuffer);

    _reader->PICC_HaltA();
    _reader->PCD_StopCrypto1();
}

bool RFID::isSameUid(const MFRC522::Uid& uid) const {
    if (!_hasLastUid || uid.size != _lastUidSize) {
        return false;
    }
    return std::equal(uid.uidByte, uid.uidByte + uid.size, _lastUidBytes.begin());
}

void RFID::rememberUid(const MFRC522::Uid& uid) {
    std::fill(_lastUidBytes.begin(), _lastUidBytes.end(), 0);
    std::copy(uid.uidByte, uid.uidByte + uid.size, _lastUidBytes.begin());
    _lastUidSize = uid.size;
    _hasLastUid = true;
}

void RFID::handleMappedTag(const MFRC522::Uid& uid, const char* uidString) {
    auto mappings = _userConfig ? _userConfig->getRfidMappings() : nullptr;
    if (!mappings || mappings->empty()) {
        Log::println("RFID", "No RFID mappings configured");
        return;
    }

    auto it = std::find_if(mappings->begin(), mappings->end(), [&](const RfidTagMapping& mapping) {
        if (mapping.uidSize != uid.size) {
            return false;
        }
        return std::equal(uid.uidByte, uid.uidByte + uid.size, mapping.uid.begin());
    });

    if (it == mappings->end()) {
        Log::println("RFID", "No mapping entry for UID %s", uidString ? uidString : "<unknown>");
        return;
    }

    if (!_audioPlayer) {
        Log::println("RFID", "Audio player unavailable for UID %s", uidString ? uidString : "<unknown>");
        return;
    }

    if (it->filePath.empty()) {
        Log::println("RFID", "No file path associated with UID %s", uidString ? uidString : "<unknown>");
        return;
    }

    Log::println("RFID", "Mapped UID %s -> %s", uidString ? uidString : "<unknown>", it->filePath.c_str());
    std::string_view pathView(it->filePath.data(), it->filePath.size());
    if (!_audioPlayer->playFileByPath(pathView)) {
        Log::println("RFID", "Failed to play mapped file %s", it->filePath.c_str());
    }
}
