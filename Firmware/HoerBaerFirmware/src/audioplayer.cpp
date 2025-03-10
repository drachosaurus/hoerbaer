#include "log.h"
#include "config.h"
#include "userconfig.h"
#include "audioplayer.h"

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

    this->codec->setParamsAndHighZ();
    Log::println("AUDIO", "Codec params and highZ mode set");
    usleep(10 * 1000);

    this->codec->setModePlay();
    Log::println("AUDIO", "Codec play mode set");

    this->codec->setVolume(this->currentVolume);
    this->codec->printMonRegisters();

    audio.setVolume(21); // 0 .. 21 - audio lib volume is not used. codec hw volume is used

    xSemaphoreGive(this->i2cSema);
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

void AudioPlayer::playSong(std::string path, uint32_t position)
{
    this->codec->setMute(true);
    audio.connecttoFS(this->sdCard->getFs(), path.c_str());
    audio.setFilePos(position);
    this->codec->setMute(false);
}

void AudioPlayer::playFromSlot(int iSlot, int increment)
{
    if(iSlot < 0 || iSlot >= this->slotDirectories->size())
    {
        Log::println("AUDIO", "Invalid slot: %d", iSlot);
        return;
    }

    string slotDir = this->slotDirectories->at(iSlot);
    Log::println("AUDIO", "Play next from slot: %d", iSlot);
    
    auto index = 0;
    auto total = 0;

    // then same slot is triggered, reuse total and increase index
    if(this->playingInfo != nullptr && this->playingInfo->slot == iSlot)
    {
        total = this->playingInfo->total;
        index = this->playingInfo->index + increment;

        if(index >= total)
        {
            Log::println("AUDIO", "Slot end reached, start at zero");
            index = 0;
        }
    }
    else 
    {
        total = this->sdCard->countFiles(slotDir);
        if(increment == -1) // start from behind, when we are skipping back
            index = total - 1;
    }

    string nextFile = this->sdCard->nextFile(slotDir, index);
    if(nextFile.empty())
    {
        Log::println("AUDIO", "No files anymore in slot %d after index %d", iSlot, index);
        return;
    }

    Log::println("AUDIO", "Play slot %d, index %d, total %d, path %s", iSlot, index, total, nextFile.c_str());
    this->playSong(nextFile, 0);

    this->playingInfo = make_shared<PlayingInfo>();
    this->playingInfo->path = nextFile;
    this->playingInfo->slot = iSlot;
    this->playingInfo->index = index;
    this->playingInfo->total = total;
    this->playingInfo->pausedAtPosition = 0;
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
