#include <Arduino.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <esp_heap_caps.h>
#include <esp_task_wdt.h>
#include "log.h"
#include "config.h"
#include "webserver.h"

static StaticJsonDocument<256> statusJsonBuffer;
static StaticJsonDocument<256> infoJsonBuffer;

void WSUpdateWorkerTask(void* param) {
    WebServer* webServer = static_cast<WebServer*>(param);
    webServer->runUpdateWorkerTask();
}

void WebServer::updateWsCurrentStateBuffer() {
    auto playingInfo = this->audioPlayer->getPlayingInfo();
    auto volume = this->audioPlayer->getCurrentVolume();
    auto maxVolume = this->audioPlayer->getMaxVolume();
    auto powerState = this->power->getState();

    statusJsonBuffer.clear();
    statusJsonBuffer["t"] = "state";

    if(playingInfo != nullptr) {
        statusJsonBuffer["state"] = playingInfo->pausedAtPosition > 0 ? "paused" : "playing";
        statusJsonBuffer["slot"] = playingInfo->slot;
        statusJsonBuffer["index"] = playingInfo->index;
        statusJsonBuffer["total"] = playingInfo->total;
        statusJsonBuffer["duration"] = playingInfo->duration;
        statusJsonBuffer["currentTime"] = playingInfo->currentTime;
    }
    else {
        statusJsonBuffer["state"] = "idle";
        statusJsonBuffer["slot"] = nullptr;
        statusJsonBuffer["index"] = nullptr;
        statusJsonBuffer["total"] = nullptr;
        statusJsonBuffer["duration"] = nullptr;
        statusJsonBuffer["currentTime"] = nullptr;
    }

    statusJsonBuffer["volume"] = volume;
    statusJsonBuffer["maxVolume"] = maxVolume;

    JsonObject bat = statusJsonBuffer.createNestedObject("bat");
    bat["v"] = powerState.voltage;
    bat["pct"] = powerState.percentage;
    bat["chg"] = powerState.charging;
}

WebServer::WebServer(std::shared_ptr<AudioPlayer> audioPlayer, std::shared_ptr<SDCard> sdCard, std::shared_ptr<Power> power, std::shared_ptr<UserConfig> userConfig) 
{    
    this->audioPlayer = audioPlayer;
    this->sdCard = sdCard;
    this->power = power;
    this->userConfig = userConfig;

    // Allocate AsyncWebServer in PSRAM to reduce internal heap pressure
    // auto spiffs = fsAccess->getFs();
    this->server = std::make_unique<AsyncWebServer>(80);
    this->ws = std::make_unique<AsyncWebSocket>("/ws");

    // CORS headers
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type, Access-Control-Allow-Headers, X-Requested-With");

    // serve /api/slots from the cache file on the SD card
    this->server->serveStatic("/api/slots", this->sdCard->getFs(), SDCARD_FILE_META_CACHE);
    
    this->server->on("/api/info", HTTP_GET, [this](AsyncWebServerRequest *request){

        Log::println("WEBSERVER", "HTTP GET /api/info FROM %s", 
            request->client()->remoteIP().toString().c_str());

        infoJsonBuffer.clear();
        infoJsonBuffer["name"] = this->userConfig->getName();
        infoJsonBuffer["timezone"] = this->userConfig->getTimezone();

        auto wifiConfig = this->userConfig->getWifiConfig();
        JsonObject wifi = infoJsonBuffer.createNestedObject("wifi");
        wifi["enabled"] = wifiConfig->enabled;
        wifi["ssid"] = wifiConfig->ssid;

        String jsonResponse;
        serializeJson(infoJsonBuffer, jsonResponse);

        request->send(200, "application/json", jsonResponse);
    });
    
    // serve app files from SPIFFS
    this->server->serveStatic("/", SPIFFS, "/webui").setDefaultFile("index.html");
    this->server->serveStatic("/static", SPIFFS, "/webui");
    this->server->serveStatic("/songs", SPIFFS, "/webui/index.html");

    this->server->onNotFound([](AsyncWebServerRequest *request){
        if (request->method() == HTTP_OPTIONS) {
            request->send(200); // CORS preflight requests
            return;
        }
        
        Log::println("WEBSERVER", "HTTP %s Unhandled Request %s FROM %s - return 404", 
            request->methodToString(), request->url().c_str(), request->client()->remoteIP().toString().c_str());

        request->send(404);
    });

    // Capture shared pointers by value to ensure they remain valid in async callbacks
    auto audioPlayerPtr = this->audioPlayer;
    auto wsPtr = this->ws.get();
    
    this->ws->onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            Log::println("WEBSERVER", "ws client connected: %u from %s", 
                client->id(), client->remoteIP().toString().c_str());

            this->updateWsCurrentStateBuffer();
            String jsonResponse;
            serializeJson(statusJsonBuffer, jsonResponse);
            client->text(jsonResponse);

            client->setCloseClientOnQueueFull(false);
        } 
        else if (type == WS_EVT_DISCONNECT) {
            Log::println("WEBSERVER", "ws client %u disconnected", client->id());
        } 
        else if (type == WS_EVT_ERROR) {
            Log::println("WEBSERVER", "ws error from client %u", client->id());
        }
    });

    // REST API endpoint for commands
    this->server->on("/api/cmd", HTTP_POST, [this](AsyncWebServerRequest *request){}, NULL,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
            
            Log::println("WEBSERVER", "HTTP POST /api/cmd FROM %s, len=%zu", 
                request->client()->remoteIP().toString().c_str(), len);

            StaticJsonDocument<128> commandDoc;
            auto error = deserializeJson(commandDoc, data, len);
            
            if (!error) {
                const char* action = commandDoc["cmd"];
                
                if (action) {
                    Log::println("WEBSERVER", "Command received: %s", action);
                    this->processCommand(action, commandDoc);
                    request->send(200, "application/json", "{\"status\":\"ok\"}");
                } else {
                    Log::println("WEBSERVER", "Command missing 'cmd' field");
                    request->send(400, "application/json", "{\"error\":\"missing cmd field\"}");
                }
            } else {
                Log::println("WEBSERVER", "Failed to parse command JSON: %s", error.c_str());
                request->send(400, "application/json", "{\"error\":\"invalid json\"}");
            }
        });

    this->server->addHandler(ws.get());
}

void WebServer::processCommand(const char* action, JsonDocument& commandDoc) {
    if (strcmp(action, "play") == 0) {
        Log::println("WEBSERVER", "Executing PLAY command");
        this->audioPlayer->play();
        broadcastCurrentState();
    } 
    else if(strcmp(action, "playSlot") == 0) {
        int slot = commandDoc["slot"];
        int index = commandDoc["index"];
        Log::println("WEBSERVER", "Executing PLAYSLOT command: slot=%d, index=%d", slot, index);
        this->audioPlayer->playSlotIndex(slot, index);
        broadcastCurrentState();
    }
    else if (strcmp(action, "pause") == 0) {
        Log::println("WEBSERVER", "Executing PAUSE command");
        this->audioPlayer->pause();
        broadcastCurrentState();
    } 
    else if (strcmp(action, "next") == 0) {
        Log::println("WEBSERVER", "Executing NEXT command");
        this->audioPlayer->next();
        broadcastCurrentState();
    } 
    else if (strcmp(action, "previous") == 0) {
        Log::println("WEBSERVER", "Executing PREVIOUS command");
        this->audioPlayer->prev();
        broadcastCurrentState();
    }
    else if (strcmp(action, "setVol") == 0) {
        int volume = commandDoc["volume"];
        Log::println("WEBSERVER", "Executing SETVOL command: volume=%d", volume);
        this->audioPlayer->setVolume(volume);
        broadcastCurrentState();
    } 
    else {
        Log::println("WEBSERVER", "Unknown action: %s", action);
    }
}

void WebServer::start() {

    // we use SPIFFS to store the web UI files
    if(!SPIFFS.begin(true)) {
        Log::println("WEBSERVER", "An error has occurred while mounting SPIFFS");
        return;
    }

    this->server->begin();

    xTaskCreate(
        WSUpdateWorkerTask, "ws_update_worker",
        TASK_STACK_SIZE_WS_UPDATE_WORKER_WORDS,
        this,
        TASK_PRIO_WS_UPDATE_WORKER,
        NULL
    );
}

void WebServer::runUpdateWorkerTask() {
    while (true) {
        broadcastCurrentState();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void WebServer::broadcastCurrentState() {
    if (ws->count() > 0) {
        this->updateWsCurrentStateBuffer();
        String jsonResponse;
        serializeJson(statusJsonBuffer, jsonResponse);
        ws->textAll(jsonResponse);
    }
}