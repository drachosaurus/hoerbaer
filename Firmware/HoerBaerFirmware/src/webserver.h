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
        std::unique_ptr<AsyncWebSocket> ws;
        std::shared_ptr<AudioPlayer> audioPlayer;
        std::shared_ptr<SDCard> sdCard;
        QueueHandle_t actionQueue;

        void updateWsCurrentStateBuffer();
        
    public:
        WebServer(std::shared_ptr<AudioPlayer> audioPlayer, std::shared_ptr<SDCard> sdCard);
        void start();
        void runUpdateWorkerTask();
        QueueHandle_t getActionQueueHandle();
};
