#pragma once

#include <memory>
#include <Wire.h>
#include "userconfig.h"
#include "devices/TAS5806.h"

using namespace std;

typedef struct {
    std::string directory;
    std::string filename;
    int index;
    int total;
} PlayingInfo;

class AudioPlayer {
    private:
        shared_ptr<TwoWire> i2c;
        SemaphoreHandle_t i2cSema;
        shared_ptr<AudioConfig> audioConfig;
        unique_ptr<TAS5806> codec;
        shared_ptr<PlayingInfo> playingInfo;
        int currentVolume;
    public:
        AudioPlayer(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema, shared_ptr<AudioConfig> audioConfig);
        void initialize();
        void loop();
        shared_ptr<PlayingInfo> getPlayingInfo();
        void volumeUp();
        void volumeDown();
};
