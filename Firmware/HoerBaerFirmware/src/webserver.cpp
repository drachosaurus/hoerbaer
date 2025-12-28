#include <Arduino.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <esp_heap_caps.h>
#include <esp_task_wdt.h>
#include "log.h"
#include "config.h"
#include "webserver.h"

struct SpiRamAllocator {
  void* allocate(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
  }

  void deallocate(void* pointer) {
    heap_caps_free(pointer);
  }

  void* reallocate(void* ptr, size_t new_size) {
    return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
  }
};

WebServer::WebServer(std::shared_ptr<AudioPlayer> audioPlayer, std::shared_ptr<SDCard> sdCard) 
{    
    this->audioPlayer = audioPlayer;
    this->sdCard = sdCard;
    this->actionQueue = xQueueCreate(10, sizeof (uint8_t));

    // Allocate AsyncWebServer in PSRAM to reduce internal heap pressure
    // auto spiffs = fsAccess->getFs();
    this->server = std::make_unique<AsyncWebServer>(80);

    // CORS headers
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type, Access-Control-Allow-Headers, X-Requested-With");
    
    this->server->serveStatic("/api/slots", sdCard->getFs(), SDCARD_FILE_META_CACHE);

    // this->server->serveStatic("/alarmclock", spiffs, "/webinterface/index.html");
    // this->server->serveStatic("/wifi", spiffs, "/webinterface/index.html");
    // this->server->serveStatic("/", spiffs, "/webinterface/")
    //     .setDefaultFile("index.html");

    this->server->onNotFound([&](AsyncWebServerRequest *request){
        if (request->method() == HTTP_OPTIONS) {
            request->send(200); // CORS preflight requests
            return;
        }
        
        Serial.println(
            "HTTP " + String(request->methodToString()) + 
            " Unhandled Request " + request->url() + 
            " FROM " + request->client()->remoteIP().toString() + 
            " - return 404");

        request->send(404);
    });
}

void WebServer::start() {
    this->server->begin();
}

QueueHandle_t WebServer::getActionQueueHandle() {
    return this->actionQueue;
}