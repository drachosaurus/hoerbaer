#pragma once

#include "SD.h"

class SDCard {
    private:
        bool cardMounted = false;
        void mountOrThrow();
    public:
        SDCard();
        bool cardPresent();
        void checkCreateFile(const std::string filename, const std::string_view& content);
        void listFiles();
};
