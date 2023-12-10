#pragma once

#include <memory>
#include <Wire.h>
// #include <Audio.h>
#include "devices/TAS5806.h"

using namespace std;

class AudioPlayer {
    private:
        shared_ptr<TwoWire> i2c;
        SemaphoreHandle_t i2cSema;
        unique_ptr<TAS5806> codec;
    public:
        AudioPlayer(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema);
        void initialize();
        void loop();
        void test();
};
