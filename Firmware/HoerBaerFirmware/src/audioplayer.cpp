#include "log.h"
#include "config.h"
#include "audioplayer.h"

#include <I2S.h>
// Audio audio;

AudioPlayer::AudioPlayer(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema)
{
    this->i2c = i2c;
    this->i2cSema = i2cSema;
    // this->audio = make_unique<Audio>();
    // this->audio->setPinout(GPIO_AUDIO_BCLK, GPIO_AUDIO_LRCLK, GPIO_AUDIO_DOUT);
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
    // audio.setPinout(GPIO_AUDIO_BCLK, GPIO_AUDIO_LRCLK, GPIO_AUDIO_DOUT);
    // Log::println("AUDIO", "I2S clocks enabled");
    I2S.setAllPins(GPIO_AUDIO_BCLK, GPIO_AUDIO_LRCLK, GPIO_AUDIO_DOUT, I2S_PIN_NO_CHANGE, I2S_PIN_NO_CHANGE);
    I2S.begin(I2S_LEFT_JUSTIFIED_MODE, 32000, 16);

    xSemaphoreTake(this->i2cSema, portMAX_DELAY);

    this->codec->setParamsAndHighZ();
    Log::println("AUDIO", "Codec params and highZ mode set");
    usleep(10 * 1000);

    this->codec->setModePlay();
    Log::println("AUDIO", "Codec play mode set");

    xSemaphoreGive(this->i2cSema);
}


    uint16_t audioBuf[320];

void AudioPlayer::test()
{
    xSemaphoreTake(this->i2cSema, portMAX_DELAY);
    this->codec->setVolume(10);
    this->codec->printMonRegisters();


    float amplitude = 0.5; // Amplitude of the sine wave (between -1 and 1)
    // L channel
    for(int i=0; i<160; i++)
    {
        float value = amplitude * sin(2 * M_PI * i / 160);
        audioBuf[i] = static_cast<uint16_t>((value + 1) * (0xFFFF / 2)); // Convert the value to a byte between 0 and 255
    }

    // R channel
    for(int i=0; i<160; i++)
        audioBuf[i + 160] = audioBuf[i];

    // for(int i=0; i<320; i++)
    //     Serial.println(audioBuf[i]);


       
    // audio.setVolume(10); // 0 .. 21
    // Log::println("AUDIO", "Begin test");
    // audio.connecttospeech("Hallo ich bin ein Bär.", "de"); // Google TTS
    // Log::println("AUDIO", "End test");

    xSemaphoreGive(this->i2cSema);
}

void AudioPlayer::loop()
{
    I2S.write(audioBuf, 2 * 320);
    // audio.loop();
}