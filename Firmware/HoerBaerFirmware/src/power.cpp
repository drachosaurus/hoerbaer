#include <Arduino.h>
#include "config.h"
#include "power.h"

Power::Power() 
{
  pinMode(GPIO_POWER_3V3_NPSAVE, OUTPUT);
  pinMode(GPIO_POWER_12V_ENABLE, OUTPUT);
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
}

void Power::DisableAudioVoltage() 
{
  digitalWrite(GPIO_POWER_12V_ENABLE, LOW);
}