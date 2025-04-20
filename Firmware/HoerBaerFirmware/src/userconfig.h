#pragma once

#include <memory>
#include <vector>
#include "sdcard.h"

using namespace std;

typedef struct {
    bool enabled;
    std::string ssid;
    std::string password;
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
    uint8_t ioMapping[32];
} HBIConfig;

typedef struct {
    int initalVolume;
    int minVolume;
    int maxVolume;
    int volumeEncoderStep;
    bool mono;
} AudioConfig;

class UserConfig {
    private:
        shared_ptr<SDCard> sdCard;
        shared_ptr<WifiConfig> wifiConfig;
        shared_ptr<HBIConfig> hbiConfig;
        shared_ptr<AudioConfig> audioConfig;
        shared_ptr<vector<string>> slotDirectories;
        std::string name;
        std::string timezone;
        bool batteryPresent;
        void initializeGlobals();
        void initializeWifi();
        void initializeHBI();
        void initializeAudio();
        void initializeSlots();
    public:
        UserConfig(std::shared_ptr<SDCard> sdCard);
        void initializeFromSdCard();
        shared_ptr<WifiConfig> getWifiConfig();
        shared_ptr<HBIConfig> getHBIConfig();
        shared_ptr<AudioConfig> getAudioConfig();
        shared_ptr<vector<string>> getSlotDirectories();
        string getName();
        string getTimezone();
        bool getBatteryPresent();
};
