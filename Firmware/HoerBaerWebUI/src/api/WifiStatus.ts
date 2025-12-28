import { WifiConnectionState } from "./WifiConnectionState";

export interface WifiStatus {
  connectionState: WifiConnectionState;
  ssid: string;
  ip: string;
  mac: string;
  configuredSsid: string;
  rssi: number;
  offWhenEyesClosed: boolean;
}
