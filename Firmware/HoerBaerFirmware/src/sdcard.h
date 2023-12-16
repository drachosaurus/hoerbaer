#pragma once

#include "SD.h"

class SDCard {
    public:
        SDCard();
        void listFiles();
        fs::SDFS getFs();
};
