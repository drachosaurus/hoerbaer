#pragma once

#include <memory>
#include <Wire.h>
#include "userconfig.h"
#include "devices/TAS5806.h"

using namespace std;

typedef struct {
    std::string path;
    int slot;
    int index;
    int total;
    uint32_t pausedAtPosition;
} PlayingInfo;

class AudioPlayer {
    private:
        shared_ptr<TwoWire> i2c;
        SemaphoreHandle_t i2cSema;
        shared_ptr<AudioConfig> audioConfig;
        shared_ptr<vector<string>> slotDirectories;
        unique_ptr<TAS5806> codec;
        shared_ptr<PlayingInfo> playingInfo;
        shared_ptr<SDCard> sdCard;
        int currentVolume;
        void playSong(std::string path, uint32_t position);
        void playFromSlot(int iSlot, int increment);
    public:
        AudioPlayer(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema, shared_ptr<UserConfig> userConfig, shared_ptr<SDCard> sdCard);
        void initialize();
        void loop();
        shared_ptr<PlayingInfo> getPlayingInfo();
        void volumeUp();
        void volumeDown();
        void playNextFromSlot(int iSlot);
        void play();
        void stop();
        void pause();
        void next();
        void prev();
};
