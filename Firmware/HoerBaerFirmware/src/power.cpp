#include <Arduino.h>
#include "log.h"
#include "Adafruit_MAX1704X.h"
#include "config.h"
#include "power.h"

using namespace std;

Adafruit_MAX17048 fuelGauge;

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
  initialized = false;
  lastBatteryCheck = 0;
}

void Power::disableVCCPowerSave() 
{
  // Disable 3.3v regulator power save mode
  digitalWrite(GPIO_POWER_3V3_NPSAVE, HIGH);

  // Enable peripherial VCC
  #ifdef GPIO_POWER_VCC_P_ENABLE
    digitalWrite(GPIO_POWER_VCC_P_ENABLE, HIGH);
  #endif

  // Wait for 3.3V to stabilize
  delay(POWER_PERIPHERIAL_STARTUP_DELAY);
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
  batteryPresent = fuelGauge.begin(this->i2c.get());
  fuelGauge.sleep(false);

  initialized = true;
  xSemaphoreGive(this->i2cSema);

  if(!batteryPresent) {
    Log::println("POWER", "No battery present. Skip initialization of fuel gauge.");
    return;
  }

  xSemaphoreTake(this->i2cSema, portMAX_DELAY);
  auto chipId = fuelGauge.getChipID();
  fuelGauge.getAlertVoltages(minV, maxV);
  auto hyber = fuelGauge.getHibernationThreshold();
  xSemaphoreGive(this->i2cSema);

  Log::println("POWER", "Fuel gauge initialized, chip ID: 0x%x, minV: %f, maxV %f, hybernation: %f", chipId, minV, maxV, hyber);
}

void Power::setGaugeToSleep() 
{
  if(!initialized) {
    Log::println("POWER", "Fuel gauge not initialized, cannot set to sleep");
    return;
  }
  
  Log::println("POWER", "Set fuel gauge to sleep");
  xSemaphoreTake(this->i2cSema, portMAX_DELAY);
  fuelGauge.sleep(true);
  xSemaphoreGive(this->i2cSema);
}

void Power::updateState() 
{
  if(!initialized) {
    Log::println("POWER", "Fuel gauge not initialized, unable to update state");
    return;
  }
  
  xSemaphoreTake(this->i2cSema, portMAX_DELAY);
  state.voltage = fuelGauge.cellVoltage();
  state.percentage = fuelGauge.cellPercent();
  xSemaphoreGive(this->i2cSema);

  state.charging = isCharging();
}

bool Power::checkBatteryShutdownLoop() 
{
  if(!batteryPresent)
    return false;

  auto tickCount = xTaskGetTickCount();
  if(tickCount - lastBatteryCheck < pdMS_TO_TICKS(POWER_BATTERY_CHECK_INTERVAL_MILLIS))
    return false;

  lastBatteryCheck = tickCount;
  return checkBatteryShutdown();
}

PowerState& Power::getState() {
  return state;
}

bool Power::checkBatteryShutdown()
{
  if(!batteryPresent)
    return false;

  updateState();
  Log::println("POWER", "Battery: %.2fV (%.1f percent), charging: %i", state.voltage, state.percentage, state.charging);

  if(state.charging)
    return false;

  return state.voltage > 0.5 && state.voltage <= POWER_SHUTDOWN_VOLTAGE; // over 0.5 => battery is connected and no transmission errors or something
}
