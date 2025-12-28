#pragma once

#include <memory>
#include <FreeRTOS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "audioplayer.h"
#include "power.h"

class WebServer {
    private:
        std::unique_ptr<AsyncWebServer> server;
        std::unique_ptr<AsyncWebSocket> ws;
        std::shared_ptr<AudioPlayer> audioPlayer;
        std::shared_ptr<SDCard> sdCard;
        std::shared_ptr<Power> power;
        std::shared_ptr<UserConfig> userConfig;

        void updateWsCurrentStateBuffer();
        
    public:
        WebServer(std::shared_ptr<AudioPlayer> audioPlayer, std::shared_ptr<SDCard> sdCard, std::shared_ptr<Power> power, std::shared_ptr<UserConfig> userConfig);
        void start();
        void runUpdateWorkerTask();
};
