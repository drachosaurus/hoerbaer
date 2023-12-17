#include "log.h"
#include "config.h"
#include "audioplayer.h"

#include <Audio.h>
Audio audio;

AudioPlayer::AudioPlayer(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema)
{
    this->i2c = i2c;
    this->i2cSema = i2cSema;

    this->codec = make_unique<TAS5806>(i2c, I2C_ADDR_AUDIO_CODEC);
    pinMode(GPIO_AUDIO_CODEC_NPDN, OUTPUT);
}

void AudioPlayer::initialize()
{
    // DS p.45, 7.5.3.1 Startup Procedures
    digitalWrite(GPIO_AUDIO_CODEC_NPDN, HIGH);
    Log::println("AUDIO", "Codec on");
    usleep(10 * 1000); // Wait 10ms to let the codec power up

    this->codec->resetChip();
    Log::println("AUDIO", "Codec reset");
    usleep(10 * 1000);

    // Initializing audio pinout enables I2C
    audio.setPinout(GPIO_AUDIO_BCLK, GPIO_AUDIO_LRCLK, GPIO_AUDIO_DOUT);
    Log::println("AUDIO", "I2S clocks enabled");

    xSemaphoreTake(this->i2cSema, portMAX_DELAY);

    this->codec->setParamsAndHighZ();
    Log::println("AUDIO", "Codec params and highZ mode set");
    usleep(10 * 1000);

    this->codec->setModePlay();
    Log::println("AUDIO", "Codec play mode set");

    xSemaphoreGive(this->i2cSema);
}

// declared in Audio.h
void audio_info(const char *info){
    Log::println("AUDIO", "Lib info: %s", info);
}

void AudioPlayer::test()
{
    xSemaphoreTake(this->i2cSema, portMAX_DELAY);
    this->codec->setVolume(130);
    this->codec->printMonRegisters();
    xSemaphoreGive(this->i2cSema);

    audio.setVolume(21); // 0 .. 21
    // audio.connecttohost("http://mp3.ffh.de/radioffh/hqlivestream.mp3"); //  128k mp3
    // audio.connecttoFS(SD, "3min1khz.wav");
    audio.connecttoFS(SD, "04 I Like Birds.mp3");
}

void AudioPlayer::loop()
{
    audio.loop();
}