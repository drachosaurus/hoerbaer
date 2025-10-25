#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <new>
#include <string>
#include <vector>
#include "sdcard.h"

extern "C" {
#include "esp_heap_caps.h"
}

template <typename T>
struct PsramAllocator {
    using value_type = T;

    PsramAllocator() noexcept = default;

    template <class U>
    PsramAllocator(const PsramAllocator<U>&) noexcept {}

    [[nodiscard]] T* allocate(std::size_t n) {
        auto* ptr = static_cast<T*>(heap_caps_malloc(n * sizeof(T), MALLOC_CAP_SPIRAM));
        if (ptr == nullptr) {
            throw std::bad_alloc();
        }
        return ptr;
    }

    void deallocate(T* p, std::size_t) noexcept {
        heap_caps_free(p);
    }
};

template <class T, class U>
bool operator==(const PsramAllocator<T>&, const PsramAllocator<U>&) {
    return true;
}

template <class T, class U>
bool operator!=(const PsramAllocator<T>&, const PsramAllocator<U>&) {
    return false;
}

using PsramString = std::basic_string<char, std::char_traits<char>, PsramAllocator<char>>;

using namespace std;

typedef struct {
    bool enabled;
    PsramString ssid;
    PsramString password;
} WifiConfig;

#define IO_MAPPING_TYPE_NONE 0x00
#define IO_MAPPING_TYPE_PLAY_SLOT 0x01
#define IO_MAPPING_TYPE_CONTROL_PLAY 0x02
#define IO_MAPPING_TYPE_CONTROL_PAUSE 0x03
#define IO_MAPPING_TYPE_CONTROL_STOP 0x04
#define IO_MAPPING_TYPE_CONTROL_NEXT 0x05
#define IO_MAPPING_TYPE_CONTROL_PREV 0x06

#define LED_DEFAULT 0x00
#define LED_POWER_ON 0x10

typedef struct {
    bool reverseNose;
    bool releaseInsteadOfPress;
    uint8_t ledBrightness;
    uint8_t ioMapping[32];
} HBIConfig;

typedef struct {
    int initalVolume;
    int minVolume;
    int maxVolume;
    int volumeEncoderStep;
    bool mono;
} AudioConfig;

struct RfidTagMapping {
    std::array<uint8_t, 10> uid;
    uint8_t uidSize;
    PsramString filePath;
};

using SlotDirectoryList = std::vector<PsramString, PsramAllocator<PsramString>>;
using RfidMappingList = std::vector<RfidTagMapping, PsramAllocator<RfidTagMapping>>;

class UserConfig {
    private:
        shared_ptr<SDCard> sdCard;
        shared_ptr<WifiConfig> wifiConfig;
        shared_ptr<HBIConfig> hbiConfig;
        shared_ptr<AudioConfig> audioConfig;
        shared_ptr<SlotDirectoryList> slotDirectories;
        shared_ptr<RfidMappingList> rfidMappings;
        PsramString name;
        PsramString timezone;
        bool batteryPresent;
        void initializeGlobals();
        void initializeWifi();
        void initializeHBI();
        void initializeAudio();
        void initializeSlots();
        void initializeRfid();
    public:
        UserConfig(std::shared_ptr<SDCard> sdCard);
        void initializeFromSdCard();
        shared_ptr<WifiConfig> getWifiConfig();
        shared_ptr<HBIConfig> getHBIConfig();
        shared_ptr<AudioConfig> getAudioConfig();
        shared_ptr<SlotDirectoryList> getSlotDirectories();
        shared_ptr<RfidMappingList> getRfidMappings();
        string getName();
        string getTimezone();
        bool getBatteryPresent();
};
