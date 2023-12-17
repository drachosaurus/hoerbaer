#pragma once

#include <SD.h>
#include <ArduinoJson.h>

class SDCard {
    private:
        bool cardMounted = false;
        void mountOrThrow();
    public:
        SDCard();
        bool cardPresent();
        bool fileExists(const std::string filename);
        void writeJsonFile(const std::string filename, JsonDocument& jsonDocument);
        void listFiles();
        void readParseJsonFile(const std::string filename, JsonDocument& targetJsonDocument);
};
