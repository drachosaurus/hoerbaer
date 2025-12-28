#pragma once

#include <memory>
#include <FreeRTOS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "audioplayer.h"

class WebServer {
    private:
        std::unique_ptr<AsyncWebServer> server;
        std::shared_ptr<AudioPlayer> audioPlayer;
        std::shared_ptr<SDCard> sdCard;
        QueueHandle_t actionQueue;
    public:
        WebServer(std::shared_ptr<AudioPlayer> audioPlayer, std::shared_ptr<SDCard> sdCard);
        void start();
        QueueHandle_t getActionQueueHandle();
};
