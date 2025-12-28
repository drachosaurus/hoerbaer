import { createApi, fetchBaseQuery } from "@reduxjs/toolkit/query/react";
import { WifiStatus } from "./WifiStatus";
import { AlarmClockSettings } from "./AlarmClockSettings";

const baseUrl = import.meta.env.DEV ? "http://192.168.15.164/api" : "/api";

export interface SlotFile {
  path: string;
  title: string;
  artist: string;
}

export interface Slot {
  path: string;
  files: SlotFile[];
}

// Define a service using a base URL and expected endpoints
export const deviceApi = createApi({
  reducerPath: "deviceApi",
  baseQuery: fetchBaseQuery({ baseUrl }),
  endpoints: (builder) => ({
    getSlots: builder.query<Slot[], void>({
      query: () => `/slots`
    })
  })
});

// Export hooks for usage in functional components, which are
// auto-generated based on the defined endpoints
export const {
  useGetSlotsQuery
} = deviceApi;
