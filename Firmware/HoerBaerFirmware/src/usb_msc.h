#pragma once

#include <memory>
#include "sdcard.h"

using namespace std;

class USBStorage {
    public:
        USBStorage(shared_ptr<SDCard> sdCard);
        void initialize();
};
