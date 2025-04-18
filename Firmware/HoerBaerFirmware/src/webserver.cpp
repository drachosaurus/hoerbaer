#include <Arduino.h>
#include <SPIFFS.h>
#include "ArduinoJson.h"
#include "webserver.h"
#include <AsyncTCP.h>

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
        Serial.println(
            "HTTP GET /api/slots FROM " + request->client()->remoteIP().toString() + 
            " - get alarm clock info");
            
        AsyncResponseStream *response = request->beginResponseStream("application/json");

        JsonVariant doc;
        JsonObject root = doc.to<JsonObject>();

        root["test"] = "test";
        // auto info = this->alarmClock->getCurrentSettings();
        // AlarmClock::serializeSettings(info, doc);
        
        serializeJson(root, *response);
        request->send(response);
    });

    // this->server->addHandler(new AsyncCallbackJsonWebHandler("/api/alarmclock", [&](AsyncWebServerRequest *request, const JsonVariant &json) {
    //     JsonObject jsonObj = json.as<JsonObject>();
        
    //     Serial.println(
    //         "HTTP " + String(request->methodToString()) + 
    //         " /api/alarmclock FROM " + request->client()->remoteIP().toString() + 
    //         " - set alarm clock settings");

    //     AlarmClockSettings settings;
    //     AlarmClock::deserializeSettings(settings, jsonObj);

    //     this->alarmClock->updateSettingsAndSaveToFs(settings);
        
    //     request->send(200, "text/json", "{ \"success\": true }");
    // }));

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