#pragma once

#include <WiFi.h>

#include "userconfig.h"

class WLAN {
    private:
        std::shared_ptr<UserConfig> userConfig;
        bool connected;
        std::string ssid;
        std::string password;
        int32_t ipV4;
    public:
        WLAN(std::shared_ptr<UserConfig> userConfig);
        void connectIfConfigured();
        void disconnect();
        bool getEnabled();
        bool getConnected();
        std::string getSSID();
        std::string getHostname();
        int32_t getIPV4();
        uint8_t getRSSI();
};
