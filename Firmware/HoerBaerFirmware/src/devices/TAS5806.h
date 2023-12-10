#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <memory>

class TAS5806 
{
    private:
        std::shared_ptr<TwoWire> wire;
        uint8_t deviceAddress;
        void readPrintBinaryRegister(uint8_t reg, const char * name, uint8_t expected);
        void readPrintValueRegister(uint8_t reg, const char * name, uint8_t expected);
    public:
        TAS5806(std::shared_ptr<TwoWire> wire, uint8_t deviceAddress);
        void resetChip();
        void setParamsAndHighZ();
        void setModePlay();
        void setVolume(uint8_t volume);
        void printMonRegisters();
};