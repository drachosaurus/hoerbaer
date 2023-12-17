#include "log.h"
#include "config.h"
#include "userconfig.h"
#include "audioplayer.h"

#include <Audio.h>
Audio audio;

AudioPlayer::AudioPlayer(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema, shared_ptr<AudioConfig> audioConfig)
{
    this->i2c = i2c;
    this->i2cSema = i2cSema;
    this->audioConfig = audioConfig;

    this->codec = make_unique<TAS5806>(i2c, I2C_ADDR_AUDIO_CODEC);
    pinMode(GPIO_AUDIO_CODEC_NPDN, OUTPUT);
    digitalWrite(GPIO_AUDIO_CODEC_NPDN, LOW);

    this->playingInfo = nullptr;
    this->currentVolume = this->audioConfig->initalVolume;
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

    audio.setVolume(21); // 0 .. 21

    xSemaphoreGive(this->i2cSema);
}

// declared in Audio.h
void audio_info(const char *info){
    Log::println("AUDIO", "Lib info: %s", info);
}

// void AudioPlayer::test()
// {
   
//     // audio.connecttohost("http://mp3.ffh.de/radioffh/hqlivestream.mp3"); //  128k mp3
//     // audio.connecttoFS(SD, "3min1khz.wav");
// }

void AudioPlayer::loop()
{
    audio.loop();
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

void AudioPlayer::playSong(std::string directory, std::string filename, uint32_t position)
{
    std::string file = "04 I Like Birds.mp3";
    audio.connecttoFS(SD, file.c_str());
    audio.setFilePos(position);

    this->playingInfo = make_shared<PlayingInfo>();
    this->playingInfo->directory = directory;
    this->playingInfo->filename = file;
    this->playingInfo->index = 0;
    this->playingInfo->total = 1;
    this->playingInfo->pausedAtPosition = position;
}

void AudioPlayer::playNextFromSlot(int iSlot)
{
    Log::println("AUDIO", "Play next from slot: %d", iSlot);
    this->playSong("directory", "04 I Like Birds.mp3", 0);
}

void AudioPlayer::play()
{
    if(this->playingInfo == nullptr)
    {
        Log::println("AUDIO", "Play: Nothing paused, nothing to resume.");
        return;
    }

    Log::println("AUDIO", "Play: resume directory %s, song %s, position %u.", 
        this->playingInfo->directory.c_str(), this->playingInfo->filename.c_str(), this->playingInfo->pausedAtPosition);
    this->playSong(this->playingInfo->directory, "04 I Like Birds.mp3", this->playingInfo->pausedAtPosition);
}

void AudioPlayer::stop()
{
    this->playingInfo = nullptr;
    audio.stopSong();
    Log::println("AUDIO", "Stopped");
}

void AudioPlayer::pause()
{
    this->playingInfo->pausedAtPosition = audio.getFilePos();
    audio.stopSong();
    Log::println("AUDIO", "Pause: directory %s, song %s, position %u.", 
        this->playingInfo->directory.c_str(), this->playingInfo->filename.c_str(), this->playingInfo->pausedAtPosition);
}