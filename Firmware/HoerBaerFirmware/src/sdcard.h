#pragma once

#include <SD.h>
#include <ArduinoJson.h>

class SDCard {
    private:
        bool cardMounted = false;
        void mountOrThrow();
    public:
        SDCard();
        fs::FS& getFs();
        bool cardPresent();
        bool fileExists(const std::string filename);
        void writeJsonFile(const std::string filename, JsonDocument& jsonDocument);
        void listFiles();
        std::string nextFile(std::string dir, int skip);
        int countFiles(std::string dir);
        void readParseJsonFile(const std::string filename, JsonDocument& targetJsonDocument);
};
