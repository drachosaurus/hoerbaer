#include <Arduino.h>
#include "log.h"
#include "Adafruit_MAX1704X.h"
#include "config.h"
#include "power.h"

using namespace std;

Adafruit_MAX17048 fuelGauge;

Power::Power(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema) 
{
  pinMode(GPIO_POWER_3V3_NPSAVE, OUTPUT);
  pinMode(GPIO_POWER_12V_ENABLE, OUTPUT);

  this->i2c = i2c;
  this->i2cSema = i2cSema;
}

void Power::DisableVCCPowerSave() 
{
  digitalWrite(GPIO_POWER_3V3_NPSAVE, HIGH);
}

void Power::EnableVCCPowerSave() 
{
  digitalWrite(GPIO_POWER_3V3_NPSAVE, LOW);
}

void Power::EnableAudioVoltage() 
{
  digitalWrite(GPIO_POWER_12V_ENABLE, HIGH);
  Log::println("POWER", "12V enabled");
}

void Power::DisableAudioVoltage() 
{
  digitalWrite(GPIO_POWER_12V_ENABLE, LOW);
  Log::println("POWER", "12V disabled");
}

void Power::InitializeChargerAndGauge()
{
  xSemaphoreTake(this->i2cSema, portMAX_DELAY);
  fuelGauge.begin(this->i2c.get());
  xSemaphoreGive(this->i2cSema);

  Log::println("POWER", "Fuel gauge initialized");
}

void Power::CheckBatteryVoltage()
{
  xSemaphoreTake(this->i2cSema, portMAX_DELAY);
  
  Log::println("POWER", "Battery voltage: %f", fuelGauge.cellVoltage());
  Log::println("POWER", "Battery percentage: %f", fuelGauge.cellPercent());

  xSemaphoreGive(this->i2cSema);
}