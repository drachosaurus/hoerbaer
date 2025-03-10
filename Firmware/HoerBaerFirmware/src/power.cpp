#include <Arduino.h>
#include "log.h"
#include "Adafruit_MAX1704X.h"
#include "config.h"
#include "power.h"

using namespace std;

Adafruit_MAX17048 fuelGauge;

TickType_t lastBatteryCheck = 0;

Power::Power(shared_ptr<TwoWire> i2c, SemaphoreHandle_t i2cSema) 
{
  pinMode(GPIO_POWER_CHG_STAT, INPUT_PULLUP);
  pinMode(GPIO_POWER_3V3_NPSAVE, OUTPUT);
  pinMode(GPIO_POWER_12V_ENABLE, OUTPUT);

  digitalWrite(GPIO_POWER_12V_ENABLE, LOW);

  this->i2c = i2c;
  this->i2cSema = i2cSema;
}

void Power::disableVCCPowerSave() 
{
  digitalWrite(GPIO_POWER_3V3_NPSAVE, HIGH);
}

void Power::enableVCCPowerSave()  
{
  digitalWrite(GPIO_POWER_3V3_NPSAVE, LOW);
}

void Power::enableAudioVoltage() 
{
  digitalWrite(GPIO_POWER_12V_ENABLE, HIGH);
  Log::println("POWER", "12V enabled");
}

void Power::disableAudioVoltage() 
{
  digitalWrite(GPIO_POWER_12V_ENABLE, LOW);
  Log::println("POWER", "12V disabled");
}

bool Power::isCharging() 
{
  return digitalRead(GPIO_POWER_CHG_STAT) == LOW;
}

void Power::initializeChargerAndGauge()
{
  float minV = 0.0f;
  float maxV = 0.0f;

  xSemaphoreTake(this->i2cSema, portMAX_DELAY);
  fuelGauge.begin(this->i2c.get());
  fuelGauge.sleep(false);
  auto chipId = fuelGauge.getChipID();
  fuelGauge.getAlertVoltages(minV, maxV);
  auto hyber = fuelGauge.getHibernationThreshold();
  xSemaphoreGive(this->i2cSema);

  Log::println("POWER", "Fuel gauge initialized, chip ID: 0x%x, minV: %f, maxV %f, hybernation: %f", chipId, minV, maxV, hyber);
}

void Power::setGaugeToSleep() 
{
  fuelGauge.sleep(true);
}

bool Power::checkBatteryShutdownLoop() 
{
  auto tickCount = xTaskGetTickCount();
  if(tickCount - lastBatteryCheck < pdMS_TO_TICKS(POWER_BATTERY_CHECK_INTERVAL_MILLIS))
    return false;

    lastBatteryCheck = tickCount;
    return checkBatteryShutdown();
}

bool Power::checkBatteryShutdown()
{
  // TODO: reenable
  // if(isCharging()) {
  //   Log::println("POWER", "Charging, no shutdown.");
  //   return false;
  // }

  // TODO: abschalten crashed
  // TODO: ohne Batterie?

  xSemaphoreTake(this->i2cSema, portMAX_DELAY);
  auto voltage = fuelGauge.cellVoltage();
  auto percent = fuelGauge.cellPercent();
  xSemaphoreGive(this->i2cSema);

  Log::println("POWER", "Battery: %fV (%f percent)", voltage, percent);

  return false;
  return voltage <= POWER_SHUTDOWN_VOLTAGE;
}
