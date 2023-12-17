#pragma once

#include <memory>
#include <map>
#include "sdcard.h"

typedef struct {
    bool enabled;
    std::string ssid;
    std::string password;
} WifiConfig;

#define IO_MAPPING_TYPE_NONE 0x00
#define IO_MAPPING_TYPE_PLAY_FOLDER 0x10
#define IO_MAPPING_TYPE_CONTROL_PLAY 0x20
#define IO_MAPPING_TYPE_CONTROL_STOP 0x21
#define IO_MAPPING_TYPE_CONTROL_PAUSE 0x22
#define IO_MAPPING_TYPE_CONTROL_NEXT 0x23
#define IO_MAPPING_TYPE_CONTROL_PREV 0x24

typedef struct {
    uint8_t type;
    std::string value;
} IOMapping;

typedef struct {
    bool reverseNose;
    bool releaseInsteadOfPress;
    IOMapping ioMapping[32];
} HBIConfig;

typedef struct {
    int initalVolume;
    int minVolume;
    int maxVolume;
    int volumeEncoderStep;
} AudioConfig;

class UserConfig {
    private:
        std::shared_ptr<SDCard> sdCard;
        std::shared_ptr<WifiConfig> wifiConfig;
        std::shared_ptr<HBIConfig> hbiConfig;
        std::shared_ptr<AudioConfig> audioConfig;
    public:
        UserConfig(std::shared_ptr<SDCard> sdCard);
        void initializeFromSdCard();
        std::shared_ptr<WifiConfig> getWifiConfig();
        std::shared_ptr<HBIConfig> getHBIConfig();
        std::shared_ptr<AudioConfig> getAudioConfig();
};
