#pragma once

class Power {
    public:
        Power();
        void DisableVCCPowerSave();
        void EnableVCCPowerSave();
        void EnableAudioVoltage();
        void DisableAudioVoltage();
};
