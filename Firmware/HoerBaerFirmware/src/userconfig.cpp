#include "log.h"
#include "config.h"
#include "userconfig.h"
#include "userconfig_default.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <utility>

#define DEFAULT_WIFI_SSID "myssid"
#define DEFAULT_WIFI_PASSWORD "mypassword"

DynamicJsonDocument jsonBuffer(4096); // size calculated with https://arduinojson.org/v6/assistant

namespace {
template <typename T, typename... Args>
std::shared_ptr<T> makeSharedPsram(Args&&... args) {
    void* raw = heap_caps_malloc(sizeof(T), MALLOC_CAP_SPIRAM);
    if (raw == nullptr) {
        throw std::bad_alloc();
    }

    T* instance = new (raw) T(std::forward<Args>(args)...);
    auto deleter = [](T* ptr) {
        if (ptr != nullptr) {
            ptr->~T();
            heap_caps_free(ptr);
        }
    };

    return std::shared_ptr<T>(instance, deleter);
}

bool parseUidString(const char* uidString, std::array<uint8_t, 10>& uidBuffer, uint8_t& uidSize) {
    if (uidString == nullptr) {
        return false;
    }

    uidBuffer.fill(0);
    uidSize = 0;

    size_t index = 0;
    const size_t length = std::strlen(uidString);

    while (index < length) {
        while (index < length && (uidString[index] == ':' || uidString[index] == '-' || uidString[index] == ' ')) {
            ++index;
        }

        if (index >= length) {
            break;
        }

        if (uidSize >= uidBuffer.size()) {
            return false;
        }

        if (index + 1 >= length) {
            return false;
        }

        char high = uidString[index];
        char low = uidString[index + 1];

        if (!std::isxdigit(static_cast<unsigned char>(high)) ||
            !std::isxdigit(static_cast<unsigned char>(low))) {
            return false;
        }

        char hexByte[3] = {high, low, '\0'};
        char* endPtr = nullptr;
        auto value = std::strtoul(hexByte, &endPtr, 16);
        if (endPtr == nullptr || *endPtr != '\0' || value > 0xFF) {
            return false;
        }

        uidBuffer[uidSize++] = static_cast<uint8_t>(value & 0xFF);
        index += 2;
    }

    return uidSize > 0;
}
} // namespace

UserConfig::UserConfig(std::shared_ptr<SDCard> sdCard)
    : sdCard(std::move(sdCard)),
      wifiConfig(makeSharedPsram<WifiConfig>()),
      hbiConfig(makeSharedPsram<HBIConfig>()),
      audioConfig(makeSharedPsram<AudioConfig>()),
      slotDirectories(makeSharedPsram<SlotDirectoryList>()),
      rfidMappings(makeSharedPsram<RfidMappingList>()),
      name(PsramString("Baer")),
      timezone(PsramString("CET-1CEST,M3.5.0,M10.5.0/3")),
      batteryPresent(true) {
    wifiConfig->enabled = false;
    wifiConfig->ssid = PsramString(DEFAULT_WIFI_SSID);
    wifiConfig->password = PsramString(DEFAULT_WIFI_PASSWORD);

    auto& hbi = *hbiConfig;
    hbi.reverseNose = false;
    hbi.releaseInsteadOfPress = false;
    hbi.ledBrightness = 255;
    std::fill(std::begin(hbi.ioMapping), std::end(hbi.ioMapping), IO_MAPPING_TYPE_NONE);
    hbi.ioMapping[0] = IO_MAPPING_TYPE_PLAY_SLOT;
    hbi.ioMapping[1] = IO_MAPPING_TYPE_PLAY_SLOT;
    hbi.ioMapping[2] = IO_MAPPING_TYPE_PLAY_SLOT;
    hbi.ioMapping[3] = IO_MAPPING_TYPE_PLAY_SLOT;
    hbi.ioMapping[19] = IO_MAPPING_TYPE_CONTROL_STOP;
    hbi.ioMapping[20] = IO_MAPPING_TYPE_CONTROL_PLAY;
    hbi.ioMapping[21] = IO_MAPPING_TYPE_CONTROL_PAUSE;
    hbi.ioMapping[22] = IO_MAPPING_TYPE_CONTROL_NEXT;
    hbi.ioMapping[23] = IO_MAPPING_TYPE_CONTROL_PREV;

    auto& audio = *audioConfig;
    audio.initalVolume = 130;
    audio.minVolume = 0;
    audio.maxVolume = 255;
    audio.volumeEncoderStep = 5;
    audio.mono = false;

    slotDirectories->emplace_back("/PAW01");
    slotDirectories->emplace_back("/PAW02");
    slotDirectories->emplace_back("/PAW03");
    slotDirectories->emplace_back("/PAW04");
}

void UserConfig::initializeFromSdCard() {
    if (!sdCard->cardPresent()) {
        Log::println("USRCFG", "SD card not present. Keeping defaults.");
        return;
    }

    try {
        if (!sdCard->fileExists(SDCARD_FILE_CONFIG)) {
            Log::println("USRCFG", "Config file not present. Write defaults json.");
            sdCard->writeTextFile(SDCARD_FILE_CONFIG, defaultUserConfig);
        }

        jsonBuffer.clear();
        sdCard->readParseJsonFile(SDCARD_FILE_CONFIG, jsonBuffer);

        initializeGlobals();
        initializeWifi();
        initializeHBI();
        initializeAudio();
        initializeSlots();
        initializeRfid();

    } catch (const std::exception& e) {
        Log::println("USRCFG", "Unable to initialize user config - %s", e.what());
        return;
    } catch (...) {
        Log::println("USRCFG", "Unable to initialize user config - Unknown error");
        return;
    }
}

void UserConfig::initializeGlobals() {
    try {
        name = PsramString(jsonBuffer["name"].as<const char*>());
        timezone = PsramString(jsonBuffer["timezone"].as<const char*>());
        batteryPresent = jsonBuffer["batteryPresent"];
        Log::println("USRCFG", "Loaded: name: %s, timezone: %s, battery: %s", name.c_str(), timezone.c_str(), batteryPresent ? "present" : "not present");
    } catch (const std::exception& e) {
        Log::println("USRCFG", "Unable to initialize globals - %s", e.what());
        return;
    } catch (...) {
        Log::println("USRCFG", "Unable to initialize globals - Unknown error");
        return;
    }
}

void UserConfig::initializeWifi() {
    try {
        JsonObject wifi = jsonBuffer["wifi"];
        wifiConfig->enabled = wifi["enabled"];
        wifiConfig->ssid = PsramString(wifi["ssid"].as<const char*>());
        wifiConfig->password = PsramString(wifi["password"].as<const char*>());
        Log::println("USRCFG", "Loaded WIFI config: enabled: %d, ssid: %s", wifiConfig->enabled, wifiConfig->ssid.c_str());
    } catch (const std::exception& e) {
        Log::println("USRCFG", "Unable to initialize WIFI config - %s", e.what());
        return;
    } catch (...) {
        Log::println("USRCFG", "Unable to initialize WIFI config - Unknown error");
        return;
    }
}

void UserConfig::initializeHBI() {
    try {
        JsonObject hbi = jsonBuffer["hbi"];
        hbiConfig->reverseNose = hbi["reverseNose"];
        hbiConfig->releaseInsteadOfPress = hbi["releaseInsteadOfPress"] | false;
        hbiConfig->ledBrightness = hbi["ledBrightness"];
        Log::println("USRCFG", "Loaded HBI config:");
        Log::println("USRCFG", "- reverseNose: %s", hbiConfig->reverseNose ? "true" : "false");
        Log::println("USRCFG", "- releaseInsteadOfPress: %s", hbiConfig->releaseInsteadOfPress ? "true" : "false");
        Log::println("USRCFG", "- ledBrightness: %d", hbiConfig->ledBrightness);
        for (int i = 0; i < 24; i++) {
            hbiConfig->ioMapping[i] = hbi["ioMapping"][i];
            Log::println("USRCFG", "- IO%d: 0x%02X", i, hbiConfig->ioMapping[i]);
        }
    } catch (const std::exception& e) {
        Log::println("USRCFG", "Unable to initialize HBI config - %s", e.what());
        return;
    } catch (...) {
        Log::println("USRCFG", "Unable to initialize HBI config - Unknown error");
        return;
    }
}

void UserConfig::initializeAudio() {
    try {
        JsonObject audio = jsonBuffer["audio"];
        audioConfig->initalVolume = audio["initalVolume"];
        audioConfig->minVolume = audio["minVolume"];
        audioConfig->maxVolume = audio["maxVolume"];
        audioConfig->volumeEncoderStep = audio["volumeEncoderStep"];
        audioConfig->mono = audio["mono"];
        Log::println("USRCFG", "Loaded Audio config: initalVolume: %d, minVolume: %d, maxVolume: %d, volumeEncoderStep: %d, %s",
                     audioConfig->initalVolume, audioConfig->minVolume, audioConfig->maxVolume, audioConfig->volumeEncoderStep,
                     audioConfig->mono ? "mono" : "stereo");
    } catch (const std::exception& e) {
        Log::println("USRCFG", "Unable to initialize AUDIO config - %s", e.what());
        return;
    } catch (...) {
        Log::println("USRCFG", "Unable to initialize AUDIO config - Unknown error");
        return;
    }
}

void UserConfig::initializeSlots() {
    try {
        slotDirectories->clear();
        auto slotsJsonArray = jsonBuffer["slots"].as<JsonArray>();
        Log::println("USRCFG", "Loaded Slots config:");
        for (JsonVariant slot : slotsJsonArray) {
            const char* slotPath = slot.as<const char*>();
            slotDirectories->emplace_back(slotPath);
            Log::println("USRCFG", "- %s", slotPath);
        }
    } catch (const std::exception& e) {
        Log::println("USRCFG", "Unable to initialize SLOTS config - %s", e.what());
        return;
    } catch (...) {
        Log::println("USRCFG", "Unable to initialize SLOTS config - Unknown error");
        return;
    }
}

void UserConfig::initializeRfid() {
    try {
        rfidMappings->clear();
        JsonObject rfid = jsonBuffer["rfid"];
        if (rfid.isNull()) {
            Log::println("USRCFG", "No RFID config found");
            return;
        }

        Log::println("USRCFG", "Loaded RFID config:");
        for (JsonPair kv : rfid) {
            const char* uidString = kv.key().c_str();
            const char* filePath = kv.value().as<const char*>();
            if (filePath == nullptr || std::strlen(filePath) == 0) {
                Log::println("USRCFG", "- Invalid file path for UID %s", uidString ? uidString : "<null>");
                continue;
            }

            RfidTagMapping mapping{};
            mapping.uid.fill(0);
            mapping.uidSize = 0;

            if (!parseUidString(uidString, mapping.uid, mapping.uidSize)) {
                Log::println("USRCFG", "- Unable to parse UID %s", uidString ? uidString : "<null>");
                continue;
            }

            mapping.filePath = PsramString(filePath);
            rfidMappings->emplace_back(std::move(mapping));
            Log::println("USRCFG", "- UID %s -> %s", uidString, filePath);
        }
    } catch (const std::exception& e) {
        Log::println("USRCFG", "Unable to initialize RFID config - %s", e.what());
        return;
    } catch (...) {
        Log::println("USRCFG", "Unable to initialize RFID config - Unknown error");
        return;
    }
}

std::shared_ptr<WifiConfig> UserConfig::getWifiConfig() {
    return wifiConfig;
}

std::shared_ptr<HBIConfig> UserConfig::getHBIConfig() {
    return hbiConfig;
}

std::shared_ptr<AudioConfig> UserConfig::getAudioConfig() {
    return audioConfig;
}

std::shared_ptr<SlotDirectoryList> UserConfig::getSlotDirectories() {
    return slotDirectories;
}

std::shared_ptr<RfidMappingList> UserConfig::getRfidMappings() {
    return rfidMappings;
}

string UserConfig::getName() {
    return std::string(name.c_str());
}

string UserConfig::getTimezone() {
    return std::string(timezone.c_str());
}

bool UserConfig::getBatteryPresent() {
    return batteryPresent;
}