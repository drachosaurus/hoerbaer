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
  pinMode(GPIO_POWER_HV_ENABLE, OUTPUT);

  #ifdef GPIO_POWER_VCC_P_ENABLE
    pinMode(GPIO_POWER_VCC_P_ENABLE, OUTPUT);
  #endif

  digitalWrite(GPIO_POWER_HV_ENABLE, LOW);

  this->i2c = i2c;
  this->i2cSema = i2cSema;
}

void Power::disableVCCPowerSave() 
{
  // Disable 3.3v regulator power save mode
  digitalWrite(GPIO_POWER_3V3_NPSAVE, HIGH);

  // Enable peripherial VCC
  #ifdef GPIO_POWER_VCC_P_ENABLE
    digitalWrite(GPIO_POWER_VCC_P_ENABLE, HIGH);
  #endif
}

void Power::enableVCCPowerSave()  
{
  // Disable peripherial VCC
  #ifdef GPIO_POWER_VCC_P_ENABLE
    digitalWrite(GPIO_POWER_VCC_P_ENABLE, LOW);
  #endif

  // Set 3.3v regulator to power save mode
  digitalWrite(GPIO_POWER_3V3_NPSAVE, LOW);
}

void Power::enableAudioVoltage() 
{
  digitalWrite(GPIO_POWER_HV_ENABLE, HIGH);
  Log::println("POWER", "12V enabled");
}

void Power::disableAudioVoltage() 
{
  digitalWrite(GPIO_POWER_HV_ENABLE, LOW);
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
  xSemaphoreTake(this->i2cSema, portMAX_DELAY);
  auto voltage = fuelGauge.cellVoltage();
  xSemaphoreGive(this->i2cSema);

  auto charging = isCharging();
  Log::println("POWER", "Battery: %.2fV, charging: %i", voltage, charging);

  if(charging)
    return false;

  return voltage <= POWER_SHUTDOWN_VOLTAGE;
}
