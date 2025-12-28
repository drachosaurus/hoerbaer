import { createApi, fetchBaseQuery } from "@reduxjs/toolkit/query/react";
import { WifiStatus } from "./WifiStatus";
import { AlarmClockSettings } from "./AlarmClockSettings";

const baseUrl = process.env.NODE_ENV === "development" ? "http://192.168.15.131/api" : "/api";

// Define a service using a base URL and expected endpoints
export const deviceApi = createApi({
  reducerPath: "deviceApi",
  baseQuery: fetchBaseQuery({ baseUrl }),
  endpoints: (builder) => ({
    getTime: builder.query<{ epochtime: number }, void>({
      query: () => `/time`
    }),
    getAlarmClockSettings: builder.query<AlarmClockSettings, void>({
      query: () => `/alarmclock`
    }),
    setAlarmClockSettings: builder.mutation<{ success: boolean }, AlarmClockSettings>({
      query: ({ ...patch }) => ({
        url: `/alarmclock`,
        method: "POST",
        body: patch
      })
    }),
    getAlarmTones: builder.query<string[], void>({
      query: () => `/alarmtones`
    }),
    getWifiStatus: builder.query<WifiStatus, void>({
      query: () => `/wifi`
    }),
    setWifiCredentials: builder.mutation<{ success: boolean }, { ssid: string; key: string }>({
      query: ({ ...patch }) => ({
        url: `/wifi/credentials`,
        method: "POST",
        body: patch
      })
    }),
    setWifiOffWhenEyesClosed: builder.mutation<{ success: boolean }, { offWhenEyesClosed: boolean }>({
      query: ({ ...patch }) => ({
        url: `/wifi/off-when-eyes-closed`,
        method: "POST",
        body: patch
      })
    })
  })
});

// Export hooks for usage in functional components, which are
// auto-generated based on the defined endpoints
export const {
  useGetAlarmClockSettingsQuery,
  useGetAlarmTonesQuery,
  useSetAlarmClockSettingsMutation,
  useGetTimeQuery,
  useGetWifiStatusQuery,
  useSetWifiCredentialsMutation,
  useSetWifiOffWhenEyesClosedMutation
} = deviceApi;
