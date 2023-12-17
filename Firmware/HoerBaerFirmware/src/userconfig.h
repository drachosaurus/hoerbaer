#pragma once

#include <memory>
#include "sdcard.h"

class UserConfig {
    private:
        std::shared_ptr<SDCard> sdCard;
        bool wifiEnabled;
        std::string wifiSSID;
        std::string wifiPassword;
    public:
        UserConfig(std::shared_ptr<SDCard> sdCard);
        void initializeFromSdCard();
        bool getWifiEnabled();
        std::string getWifiSSID();
        std::string getWifiPassword();
};
