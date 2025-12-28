#include <Arduino.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <esp_heap_caps.h>
#include <esp_task_wdt.h>
#include "log.h"
#include "config.h"
#include "webserver.h"

static StaticJsonDocument<192> statusJsonBuffer;

void WSUpdateWorkerTask(void* param) 
{
    WebServer* webServer = static_cast<WebServer*>(param);
    webServer->runUpdateWorkerTask();
}

void WebServer::updateWsCurrentStateBuffer() {
    auto playingInfo = this->audioPlayer->getPlayingInfo();
    auto volume = this->audioPlayer->getCurrentVolume();
    auto maxVolume = this->audioPlayer->getMaxVolume();

    statusJsonBuffer["t"] = "state";

    if(playingInfo != nullptr) {
        statusJsonBuffer["state"] = playingInfo->pausedAtPosition > 0 ? "paused" : "playing";
        statusJsonBuffer["slot"] = playingInfo->slot;
        statusJsonBuffer["index"] = playingInfo->index;
        statusJsonBuffer["total"] = playingInfo->total;
        statusJsonBuffer["duration"] = playingInfo->duration;
        statusJsonBuffer["currentTime"] = playingInfo->currentTime;
        statusJsonBuffer["serial"] = playingInfo->serial;
    }
    else {
        statusJsonBuffer["state"] = "idle";
        statusJsonBuffer["slot"] = nullptr;
        statusJsonBuffer["index"] = nullptr;
        statusJsonBuffer["total"] = nullptr;
        statusJsonBuffer["duration"] = nullptr;
        statusJsonBuffer["currentTime"] = nullptr;
        statusJsonBuffer["serial"] = nullptr;
    }

    statusJsonBuffer["volume"] = volume;
    statusJsonBuffer["maxVolume"] = maxVolume;
}

WebServer::WebServer(std::shared_ptr<AudioPlayer> audioPlayer, std::shared_ptr<SDCard> sdCard) 
{    
    this->audioPlayer = audioPlayer;
    this->sdCard = sdCard;

    // Allocate AsyncWebServer in PSRAM to reduce internal heap pressure
    // auto spiffs = fsAccess->getFs();
    this->server = std::make_unique<AsyncWebServer>(80);
    this->ws = std::make_unique<AsyncWebSocket>("/ws");

    // CORS headers
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type, Access-Control-Allow-Headers, X-Requested-With");
    
    this->server->serveStatic("/api/slots", sdCard->getFs(), SDCARD_FILE_META_CACHE);

    // this->server->serveStatic("/alarmclock", spiffs, "/webinterface/index.html");
    // this->server->serveStatic("/wifi", spiffs, "/webinterface/index.html");
    // this->server->serveStatic("/", spiffs, "/webinterface/")
    //     .setDefaultFile("index.html");

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
                            else if (strcmp(action, "pause") == 0) {
                                audioPlayerPtr->pause();
                            } 
                            else if (strcmp(action, "next") == 0) {
                                audioPlayerPtr->next();
                            } 
                            else if (strcmp(action, "previous") == 0) {
                                audioPlayerPtr->prev();
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