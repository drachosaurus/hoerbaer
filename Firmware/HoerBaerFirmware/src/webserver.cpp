#include <Arduino.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include "log.h"
#include "webserver.h"

WebServer::WebServer(std::shared_ptr<AudioPlayer> audioPlayer) 
{    
    this->audioPlayer = audioPlayer;
    this->actionQueue = xQueueCreate(10, sizeof (uint8_t));

    // auto spiffs = fsAccess->getFs();
    this->server = std::make_unique<AsyncWebServer>(80);

    // CORS headers
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type, Access-Control-Allow-Headers, X-Requested-With");
    
    this->server->on("/api/slots", HTTP_GET, [&](AsyncWebServerRequest *request) {
        Log::println("WEBSRV", "GET /api/slots FROM %s - get slots",
            request->client()->remoteIP().toString().c_str());
            
        AsyncResponseStream *response = request->beginResponseStream("application/json");

        StaticJsonDocument<JSON_BUFFER_SIZE_TRACK_METADATA> doc;
        this->audioPlayer->serializeLoadedSlotsAndMetadata(doc);

        serializeJson(doc, *response);
        request->send(response);
    });

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