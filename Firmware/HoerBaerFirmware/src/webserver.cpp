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


void WSUpdateWorkerTask(void* param) 
{
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
    
    // Capture sdCard shared_ptr to get non-const fs reference in lambda
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
    
    this->ws->onEvent([this, audioPlayerPtr, wsPtr](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        (void)len;

        if (type == WS_EVT_CONNECT) {
            Log::println("WEBSERVER", "ws client connected: %u from %s", 
                client->id(), client->remoteIP().toString().c_str());

            this->updateWsCurrentStateBuffer();
            String jsonResponse;
            serializeJson(statusJsonBuffer, jsonResponse);
            client->text(jsonResponse);

            client->setCloseClientOnQueueFull(false);
            client->ping();
        } 
        else if (type == WS_EVT_DISCONNECT) {
            wsPtr->textAll("client disconnected");
            Log::println("WEBSERVER", "ws disconnect");

        } 
        else if (type == WS_EVT_ERROR) {
            Log::println("WEBSERVER", "ws error");
        } 
        else if (type == WS_EVT_PONG) {
            Log::println("WEBSERVER", "ws pong");
       } 
        else if (type == WS_EVT_DATA) {
            AwsFrameInfo *info = (AwsFrameInfo *)arg;
            // Serial.printf("index: %" PRIu64 ", len: %" PRIu64 ", final: %" PRIu8 ", opcode: %" PRIu8 "\n", info->index, info->len, info->final, info->opcode);
            String msg = "";
            if (info->final && info->index == 0 && info->len == len) {
                if (info->opcode == WS_TEXT) {
                    data[len] = 0;
                    Log::println("WEBSERVER", "ws : %s", (char *)data);

                    // deserialize incoming message

                    StaticJsonDocument<128> incommingCommand;
                    auto error = deserializeJson(incommingCommand, data);
                    if (!error) {
                        const char* commandType = incommingCommand["t"];
                        if (strcmp(commandType, "cmd") == 0) {
                            const char* action = incommingCommand["cmd"];
                            Log::println("WEBSERVER", "Command received over websocket: %s", action);
                            if (strcmp(action, "play") == 0) {
                                audioPlayerPtr->play();
                            } 
                            else if(strcmp(action, "playSlot") == 0) {
                                int slot = incommingCommand["slot"];
                                int index = incommingCommand["index"];
                                audioPlayerPtr->playSlotIndex(slot, index);
                            }
                            else if (strcmp(action, "pause") == 0) {
                                audioPlayerPtr->pause();
                            } 
                            else if (strcmp(action, "next") == 0) {
                                audioPlayerPtr->next();
                            } 
                            else if (strcmp(action, "previous") == 0) {
                                audioPlayerPtr->prev();
                            }
                            else if (strcmp(action, "setVol") == 0) {
                                int volume = incommingCommand["volume"];
                                audioPlayerPtr->setVolume(volume);
                            } 
                            else {
                                Log::println("WEBSERVER", "ws: Unknown action %s", action);
                            }
                        }
                    } 
                    else {
                        Log::println("WEBSERVER", "ws: Failed to parse JSON");
                        return;
                    }
                }
            }
        }
    });

    this->server->addHandler(ws.get());
}

void WebServer::start() {
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
    while (true) 
    {
        if (ws->count() > 0)
        {
            this->updateWsCurrentStateBuffer();
            String jsonResponse;
            serializeJson(statusJsonBuffer, jsonResponse);
            ws->textAll(jsonResponse);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}