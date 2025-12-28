#include "log.h"
#include "config.h"
#include "userconfig.h"
#include "audioplayer.h"
#include "id3parser.h"

#include <Audio.h>
Audio audio;

// this is used to passback Audio-Lib callback functions.
unique_ptr<AudioPlayer> currentInstance = nullptr;

AudioPlayer::AudioPlayer(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema, shared_ptr<UserConfig> userConfig, shared_ptr<SDCard> sdCard)
{
    this->i2c = i2c;
    this->i2cSema = i2cSema;
    this->audioConfig = userConfig->getAudioConfig();
    this->slotDirectories = userConfig->getSlotDirectories();
    this->sdCard = sdCard;

    this->codec = make_unique<TAS5806>(i2c, I2C_ADDR_AUDIO_CODEC);
    pinMode(GPIO_AUDIO_CODEC_NPDN, OUTPUT);
    digitalWrite(GPIO_AUDIO_CODEC_NPDN, LOW);

    this->playingInfo = nullptr;
    this->currentVolume = this->audioConfig->initalVolume;
    currentInstance = unique_ptr<AudioPlayer>(this);

    // Define a map to store file names per slot directory with artist and title in PSRAM
    this->slotFiles = std::unique_ptr<std::vector<std::vector<std::tuple<std::string, std::string, std::string>>>>(
        new (heap_caps_malloc(sizeof(std::vector<std::vector<std::tuple<std::string, std::string, std::string>>>), MALLOC_CAP_SPIRAM)) 
        std::vector<std::vector<std::tuple<std::string, std::string, std::string>>>());

    // TODO: 
    // - slotFiles anstatt slotDirectories verwenden?
    // - slotFiles auch für Next brauchen?
}

AudioPlayer::~AudioPlayer()
{
    digitalWrite(GPIO_AUDIO_CODEC_NPDN, LOW);
    currentInstance.reset();
    currentInstance = nullptr;
}

void AudioPlayer::initialize()
{
    // DS p.45, 7.5.3.1 Startup Procedures
    digitalWrite(GPIO_AUDIO_CODEC_NPDN, HIGH);
    Log::println("AUDIO", "Codec on");
    usleep(40 * 1000);

    this->codec->resetChip();
    Log::println("AUDIO", "Codec reset");
    usleep(40 * 1000);

    // Initializing audio pinout enables I2C
    audio.setPinout(GPIO_AUDIO_BCLK, GPIO_AUDIO_LRCLK, GPIO_AUDIO_DOUT);
    Log::println("AUDIO", "I2S clocks enabled");

    xSemaphoreTake(this->i2cSema, portMAX_DELAY);

    this->codec->setParamsAndHighZ(this->audioConfig->mono);
    Log::println("AUDIO", "Codec params and highZ mode set");
    usleep(10 * 1000);

    this->codec->setModePlay();
    Log::println("AUDIO", "Codec play mode set");

    this->codec->setVolume(this->currentVolume);
    this->codec->printMonRegisters();

    audio.setVolume(21); // 0 .. 21 - audio lib volume is not used. codec hw volume is used

    xSemaphoreGive(this->i2cSema);
}

void AudioPlayer::populateAudioMetadata() 
{
    if(this->sdCard->fileExists(SDCARD_FILE_META_CACHE))
    {
        Log::println("AUDIO", "Loading audio file metadata cache from file.");
        auto size = this->sdCard->getFileSize(SDCARD_FILE_META_CACHE);
        DynamicJsonDocument jsonBuffer(size);
        this->sdCard->readParseJsonFile(SDCARD_FILE_META_CACHE, jsonBuffer);
        this->deserializeLoadedSlotsAndMetadata(jsonBuffer);
    }
    else {
        Log::println("AUDIO", "Populating audio file metadata cache...");
        TickType_t start = xTaskGetTickCount();
        int nFound = 0;
        int nNoMeta = 0;
        for(size_t iDir = 0; iDir < this->slotDirectories->size(); iDir++)
        {
            Log::println("AUDIO", "Scanning slot directory: %s", this->slotDirectories->at(iDir).c_str());

            std::vector<std::tuple<std::string, std::string, std::string>> files;
            std::string slotPath(this->slotDirectories->at(iDir).c_str());

            this->sdCard->listFiles(slotPath, [&](const std::string& filePath) {
                auto [title, artist] = ID3Parser::readId3Tags(sdCard->getFs(), filePath);
                if (title.empty() && artist.empty())
                    nNoMeta++;
                else
                    nFound++;
                files.emplace_back(filePath, title, artist);
            });
    
            // Store the vector in the map
            slotFiles->push_back(std::move(files));
        }
        TickType_t duration = xTaskGetTickCount() - start;
        Log::println("AUDIO", "Found %d files with metadata, %d without metadata (used %d ms)", nFound, nNoMeta, pdTICKS_TO_MS(duration));

        Log::println("AUDIO", "Saving metadata cache to file...");
        DynamicJsonDocument jsonBuffer(JSON_BUFFER_SIZE_TRACK_METADATA);
        this->serializeLoadedSlotsAndMetadata(jsonBuffer);
        this->sdCard->writeJsonFile(SDCARD_FILE_META_CACHE, jsonBuffer);
        Log::println("AUDIO", "Metadata cache saved to file.");
    }
}

void AudioPlayer::serializeLoadedSlotsAndMetadata(JsonDocument& doc) 
{
    for(size_t iDir = 0; iDir < this->slotDirectories->size(); iDir++) 
    {
        JsonObject slot = doc.createNestedObject();
        slot["path"] = this->slotDirectories->at(iDir).c_str();
        auto files = slot.createNestedArray("files");

        auto& slotFiles = this->slotFiles->at(iDir);
        for (const auto& [filePath, title, artist] : slotFiles) 
        {
            JsonObject file = files.createNestedObject();
            file["path"] = filePath.c_str();
            file["title"] = title.c_str();
            file["artist"] = artist.c_str();
        }
    }
}

void AudioPlayer::deserializeLoadedSlotsAndMetadata(JsonDocument& doc) 
{
    for(size_t iDir = 0; iDir < this->slotDirectories->size(); iDir++) 
    {
        std::vector<std::tuple<std::string, std::string, std::string>> files;

        auto jsonFiles = doc[iDir]["files"].as<JsonArray>();
        for (const auto& file : jsonFiles) 
        {
            std::string path = file["path"].as<std::string>();
            std::string title = file["title"].as<std::string>();
            std::string artist = file["artist"].as<std::string>();
            files.emplace_back(path, title, artist);
        }

        slotFiles->push_back(std::move(files));
    }
}

// declared in Audio.h
void audio_info(const char *info) 
{
    // Log::println("AUDIO", "Lib info: %s", info);
}

void audio_eof_mp3(const char *path)
{
    Log::println("AUDIO", "End of MP3 file");
    if(currentInstance != nullptr)
        currentInstance->next();
}

void AudioPlayer::loop()
{
    audio.loop();
    vTaskDelay(1); // https://github.com/schreibfaul1/ESP32-audioI2S/issues/887

    auto tickCount = xTaskGetTickCount();
    if(tickCount - lastPlayingInfoUpdate > pdMS_TO_TICKS(AUDIO_PLAYING_INGO_UPDATE_INTERVAL_MILLIS))
    {
        lastPlayingInfoUpdate = tickCount;
        if(this->playingInfo != nullptr)
        {
            this->playingInfo->serial++;
            this->playingInfo->currentTime = audio.getAudioCurrentTime();
            this->playingInfo->duration = audio.getAudioFileDuration();
        }
    }
}

shared_ptr<PlayingInfo> AudioPlayer::getPlayingInfo()
{
    return this->playingInfo;
}

void AudioPlayer::volumeUp()
{
    this->currentVolume += this->audioConfig->volumeEncoderStep;
    if(this->currentVolume > this->audioConfig->maxVolume)
        this->currentVolume = this->audioConfig->maxVolume;

    xSemaphoreTake(this->i2cSema, portMAX_DELAY);
    this->codec->setVolume(this->currentVolume);
    xSemaphoreGive(this->i2cSema);

    Log::println("AUDIO", "Increase volume to: %d", this->currentVolume);
}

void AudioPlayer::volumeDown()
{
    this->currentVolume -= this->audioConfig->volumeEncoderStep;
    if(this->currentVolume < this->audioConfig->minVolume)
        this->currentVolume = this->audioConfig->minVolume;

    xSemaphoreTake(this->i2cSema, portMAX_DELAY);
    this->codec->setVolume(this->currentVolume);
    xSemaphoreGive(this->i2cSema);

    Log::println("AUDIO", "Decrease volume to: %d", this->currentVolume);
}

void AudioPlayer::setVolume(int volume)
{
    if(volume < this->audioConfig->minVolume)
        volume = this->audioConfig->minVolume;
    if(volume > this->audioConfig->maxVolume)
        volume = this->audioConfig->maxVolume;

    this->currentVolume = volume;

    xSemaphoreTake(this->i2cSema, portMAX_DELAY);
    this->codec->setVolume(this->currentVolume);
    xSemaphoreGive(this->i2cSema);

    Log::println("AUDIO", "Set volume to: %d", this->currentVolume);
}   

void AudioPlayer::playSong(std::string path, uint32_t position)
{
    this->codec->setMute(true);
    audio.connecttoFS(this->sdCard->getFs(), path.c_str());
    audio.setFilePos(position);
    this->codec->setMute(false);
}

void AudioPlayer::playFromSlot(int iSlot, int increment)
{
    if (iSlot < 0 || static_cast<size_t>(iSlot) >= this->slotDirectories->size())
    {
        Log::println("AUDIO", "Invalid slot: %d", iSlot);
        return;
    }

    Log::println("AUDIO", "Play next from slot: %d", iSlot);
    
    auto index = 0;
    auto total = 0;

    // then same slot is triggered, reuse total and increase index
    if(this->playingInfo != nullptr && this->playingInfo->slot == iSlot)
    {
        total = this->playingInfo->total;
        index = this->playingInfo->index + increment;
        this->playingInfo->serial++;

        if(index >= total)
        {
            Log::println("AUDIO", "Slot end reached, start at zero");
            index = 0;
        }
    }
    else 
    {
        total = this->slotFiles->at(iSlot).size();
        if(increment == -1) // start from behind, when we are skipping back
            index = total - 1;
    }

    playSlotIndex(iSlot, index);
}

void AudioPlayer::playSlotIndex(int iSlot, int iTrack)
{
    auto total = this->slotFiles->at(iSlot).size();
    if (iTrack < 0 || iTrack >= total) {
        Log::println("AUDIO", "Invalid track index: %d for slot %d", iTrack, iSlot);
        return;
    }

    string nextFile = get<0>(this->slotFiles->at(iSlot).at(iTrack));
    if(nextFile.empty())
    {
        Log::println("AUDIO", "No files anymore in slot %d after index %d", iSlot, iTrack);
        return;
    }

    Log::println("AUDIO", "Play slot %d, index %d, total %d, path %s", iSlot, iTrack, total, nextFile.c_str());
    this->playSong(nextFile, 0);

    this->playingInfo = make_shared<PlayingInfo>();
    this->playingInfo->path = nextFile;
    this->playingInfo->slot = iSlot;
    this->playingInfo->index = iTrack;
    this->playingInfo->total = total;
    this->playingInfo->pausedAtPosition = 0;
    this->playingInfo->currentTime = 0;
    this->playingInfo->duration = audio.getAudioFileDuration();
    this->playingInfo->serial++;

    Log::println("AUDIO", "Started: duration %u", 
        this->playingInfo->duration);
}

bool AudioPlayer::playFileByPath(std::string_view path)
{
    if (!this->slotFiles) {
        std::string pathStr(path);
        Log::println("AUDIO", "Cannot play %s: slot files not initialized", pathStr.c_str());
        return false;
    }

    for (size_t slot = 0; slot < this->slotFiles->size(); ++slot) {
        auto& files = this->slotFiles->at(slot);
        for (size_t index = 0; index < files.size(); ++index) {
            if (std::get<0>(files.at(index)) == path) {
                this->playSlotIndex(static_cast<int>(slot), static_cast<int>(index));
                return true;
            }
        }
    }

    std::string pathStr(path);
    Log::println("AUDIO", "File %s not found in loaded slot metadata", pathStr.c_str());
    return false;
}

void AudioPlayer::playNextFromSlot(int iSlot)
{
    this->playFromSlot(iSlot, 1);
}

void AudioPlayer::play()
{
    if(this->playingInfo == nullptr || this->playingInfo->pausedAtPosition == 0)
    {
        Log::println("AUDIO", "Play: Nothing paused, nothing to resume.");
        return;
    }

    Log::println("AUDIO", "Play: resume %s, position %u.", 
        this->playingInfo->path.c_str(), this->playingInfo->pausedAtPosition);

    this->playSong(this->playingInfo->path, this->playingInfo->pausedAtPosition);
    this->playingInfo->pausedAtPosition = 0;
    this->playingInfo->serial++;
}

void AudioPlayer::stop()
{
    this->playingInfo = nullptr;
    audio.stopSong();
    Log::println("AUDIO", "Stopped");
}

void AudioPlayer::pause()
{
    if(this->playingInfo == nullptr)
    {
        Log::println("AUDIO", "Pause: Nothing playing, nothing to pause.");
        return;
    }

    if(this->playingInfo->pausedAtPosition > 0)
    {
        Log::println("AUDIO", "Pause: Already paused.");
        return;
    }

    this->playingInfo->pausedAtPosition = audio.getFilePos();
    audio.stopSong();
    this->playingInfo->serial++;

    Log::println("AUDIO", "Pause: %s, position %u.", 
        this->playingInfo->path.c_str(), this->playingInfo->pausedAtPosition);
}

void AudioPlayer::next()
{
    if(this->playingInfo == nullptr)
        return;

    Log::println("AUDIO", "Next track");
    auto slot = this->playingInfo->slot;
    
    if(this->playingInfo->index == this->playingInfo->total - 1) 
    {
        Log::println("AUDIO", "Next: End of slot %d reached, jump to next slot", slot);
        slot++;
    }
    
    if(slot >= this->slotDirectories->size()) 
    {
        Log::println("AUDIO", "Next: End of slots reached, jump next slot 0");
        slot = 0;
    }
    
    this->playFromSlot(slot, 1);
}

void AudioPlayer::prev()
{
    if(this->playingInfo == nullptr)
        return;

    Log::println("AUDIO", "Prev track");
    auto slot = this->playingInfo->slot;

    if(this->playingInfo->index == 0) 
    {
        Log::println("AUDIO", "Prev: Start of slot %d reached, jump to prev slot", slot);
        slot--;
    }

    if(slot < 0)
    {
        Log::println("AUDIO", "Prev: Start of slots reached, jump to last slot");
        slot = this->slotDirectories->size() - 1;
    }

    this->playFromSlot(slot, -1);
}

int AudioPlayer::getCurrentVolume() {
    return this->currentVolume;
}

int AudioPlayer::getMaxVolume() {
    return this->audioConfig->maxVolume;
}
