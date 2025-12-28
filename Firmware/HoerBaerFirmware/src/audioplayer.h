#pragma once

#include <memory>
#include <string_view>
#include <Wire.h>
#include "userconfig.h"
#include "devices/TAS5806.h"

using namespace std;

#define JSON_BUFFER_SIZE_TRACK_METADATA (40*1024)

typedef struct {
    std::string path;
    int slot;
    int index;
    int total;
    uint32_t pausedAtPosition;
    uint32_t duration;
    uint32_t currentTime;
    int serial;
} PlayingInfo;

class AudioPlayer {
    private:
        shared_ptr<TwoWire> i2c;
        SemaphoreHandle_t i2cSema;
        shared_ptr<AudioConfig> audioConfig;
        shared_ptr<SlotDirectoryList> slotDirectories;
        unique_ptr<TAS5806> codec;
        shared_ptr<PlayingInfo> playingInfo;
        shared_ptr<SDCard> sdCard;
        std::unique_ptr<std::vector<std::vector<std::tuple<std::string, std::string, std::string>>>> slotFiles;
        TickType_t lastPlayingInfoUpdate;
        int currentVolume;
        void serializeLoadedSlotsAndMetadata(JsonDocument& doc);
        void deserializeLoadedSlotsAndMetadata(JsonDocument& doc);
        void playSong(std::string path, uint32_t position);
        void playFromSlot(int iSlot, int increment);
    public:
        AudioPlayer(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema, shared_ptr<UserConfig> userConfig, shared_ptr<SDCard> sdCard);
        ~AudioPlayer();
        void initialize();
        void populateAudioMetadata();
        void loop();
        shared_ptr<PlayingInfo> getPlayingInfo();
        void volumeUp();
        void volumeDown();
        void playSlotIndex(int iSlot, int iTrack);
        bool playFileByPath(std::string_view path);
        void playNextFromSlot(int iSlot);
        void play();
        void stop();
        void pause();
        void next();
        void prev();
        int getCurrentVolume();
        int getMaxVolume();
};
