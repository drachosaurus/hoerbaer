export enum AlarmClockLightsModes {
  Off = 0,
  Sunshine = 80,
  Rainbow = 81,
  Vegas = 99
}

export const HumanizeAlarmClockLightsModes = (mode: AlarmClockLightsModes): string => {
  switch (mode) {
    case AlarmClockLightsModes.Off:
      return "Aus";
    case AlarmClockLightsModes.Sunshine:
      return "Sunnenaufgang";
    case AlarmClockLightsModes.Rainbow:
      return "Regenbogen";
    case AlarmClockLightsModes.Vegas:
      return "Vegas";
  }
};

export interface AlarmClockDaySettings {
  eyesOpenHour: number;
  eyesOpenMinute: number;
  eyesCloseHour: number;
  eyesCloseMinute: number;

  lightsEnabled: boolean;
  lightsMode: AlarmClockLightsModes;
  lightsHour: number;
  lightsMinute: number;

  toneEnabled: boolean;
  toneFile?: string;
  toneText: string;
  toneHour: number;
  toneMinute: number;
}

export interface AlarmClockSettings {
  weekend: AlarmClockDaySettings;
  weekday: AlarmClockDaySettings;
}
