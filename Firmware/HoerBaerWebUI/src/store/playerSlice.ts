import { createSlice, PayloadAction } from "@reduxjs/toolkit";
import { Slot } from "../api/deviceApi";
import { StateMessage } from "../api/websocket";

export interface Song {
  id: string;
  title: string;
  artist: string;
  path: string;
  paw: string;
  duration: number;
}

export interface Paw {
  id: string;
  name: string;
  icon: string;
  color: string;
  songs: Song[];
}

export interface PlayerState {
  currentSong: Song | null;
  isPlaying: boolean;
  currentTime: number;
  volume: number;
  maxVolume: number;
  shuffle: boolean;
  repeat: boolean;
  paws: Paw[];
  battery: {
    voltage: number;
    percentage: number;
    charging: boolean;
  } | null;
  deviceName: string | null;
}

const initialState: PlayerState = {
  currentSong: null,
  isPlaying: false,
  currentTime: 0,
  volume: 0.66,
  maxVolume: 255,
  shuffle: false,
  repeat: true,
  battery: null,
  deviceName: null,
  paws: [],
};

export const playerSlice = createSlice({
  name: "player",
  initialState,
  reducers: {
    playSong: (state, action: PayloadAction<Song>) => {
      state.currentSong = action.payload;
      state.isPlaying = true;
      state.currentTime = 0;
    },
    togglePlay: (state) => {
      state.isPlaying = !state.isPlaying;
    },
    pause: (state) => {
      state.isPlaying = false;
    },
    play: (state) => {
      state.isPlaying = true;
    },
    nextSong: (state) => {
      if (!state.currentSong) return;
      
      const currentPaw = state.paws.find(p => p.id === state.currentSong!.paw);
      if (!currentPaw) return;
      
      const currentIndex = currentPaw.songs.findIndex(s => s.id === state.currentSong!.id);
      const nextIndex = (currentIndex + 1) % currentPaw.songs.length;
      
      state.currentSong = currentPaw.songs[nextIndex];
      state.currentTime = 0;
    },
    previousSong: (state) => {
      if (!state.currentSong) return;
      
      const currentPaw = state.paws.find(p => p.id === state.currentSong!.paw);
      if (!currentPaw) return;
      
      const currentIndex = currentPaw.songs.findIndex(s => s.id === state.currentSong!.id);
      const prevIndex = (currentIndex - 1 + currentPaw.songs.length) % currentPaw.songs.length;
      
      state.currentSong = currentPaw.songs[prevIndex];
      state.currentTime = 0;
    },
    setCurrentTime: (state, action: PayloadAction<number>) => {
      state.currentTime = action.payload;
    },
    setVolume: (state, action: PayloadAction<number>) => {
      state.volume = Math.max(0, Math.min(1, action.payload));
    },
    toggleShuffle: (state) => {
      state.shuffle = !state.shuffle;
    },
    toggleRepeat: (state) => {
      state.repeat = !state.repeat;
    },
    setDeviceName: (state, action: PayloadAction<string>) => {
      state.deviceName = action.payload;
    },
    setPaws: (state, action: PayloadAction<Slot[]>) => {
      // Map paw numbers to colors and icons
      const pawConfig: Record<string, { icon: string; color: string }> = {
        PAW01: { icon: "play_arrow", color: "blue" },
        PAW02: { icon: "bedtime", color: "indigo" },
        PAW03: { icon: "auto_stories", color: "green" },
        PAW04: { icon: "school", color: "orange" },
        PAW05: { icon: "music_note", color: "purple" },
        PAW06: { icon: "favorite", color: "pink" },
        PAW07: { icon: "star", color: "yellow" },
        PAW08: { icon: "nightlight", color: "teal" },
        PAW09: { icon: "celebration", color: "red" },
        PAW10: { icon: "sports_soccer", color: "lime" },
        PAW11: { icon: "rocket_launch", color: "cyan" },
        PAW12: { icon: "pets", color: "amber" },
        PAW13: { icon: "landscape", color: "emerald" },
        PAW14: { icon: "cake", color: "rose" },
        PAW15: { icon: "train", color: "violet" },
        PAW16: { icon: "beach_access", color: "sky" },
      };

      state.paws = action.payload.map((slot) => {
        const pawId = slot.path.replace("/", "");
        const config = pawConfig[pawId] || { icon: "music_note", color: "gray" };
        
        return {
          id: pawId,
          name: pawId,
          icon: config.icon,
          color: config.color,
          songs: slot.files.map((file) => ({
            id: file.path,
            title: file.title || file.path.split("/").pop()?.replace(".mp3", "") || "Unknown",
            artist: file.artist || "Unknown Artist",
            path: file.path,
            paw: pawId,
            duration: 0, // Duration not provided by API
          })),
        };
      });
    },
    updateFromWebSocket: (state, action: PayloadAction<StateMessage>) => {
      const wsState = action.payload;
      
      // Update maxVolume
      if (wsState.maxVolume > 0) {
        state.maxVolume = wsState.maxVolume;
      }
      
      // Update volume (convert from device scale to 0-1)
      if (wsState.maxVolume > 0) {
        state.volume = wsState.volume / wsState.maxVolume;
      }
      
      // Update battery state
      if (wsState.bat) {
        state.battery = {
          voltage: Math.round(wsState.bat.v * 10) / 10,
          percentage: Math.round(wsState.bat.pct),
          charging: wsState.bat.chg,
        };
      }
      
      // Handle idle state
      if (wsState.state === "idle" || wsState.slot === null || wsState.index === null) {
        state.currentSong = null;
        state.isPlaying = false;
        state.currentTime = 0;
        return;
      }
      
      // Update playing state
      state.isPlaying = wsState.state === "playing";
      state.currentTime = wsState.currentTime || 0;
      
      // Find the paw and song based on slot and index (both are zero-based array indices)
      const paw = state.paws[wsState.slot];
      
      if (paw && wsState.index < paw.songs.length) {
        const song = paw.songs[wsState.index];
        // Update current song if it's different
        if (state.currentSong?.id !== song.id) {
          state.currentSong = {
            ...song,
            duration: wsState.duration || song.duration,
          };
        } else if (wsState.duration && state.currentSong) {
          // Update duration if provided
          state.currentSong.duration = wsState.duration;
        }
      }
    },
  },
});

export const {
  playSong,
  togglePlay,
  pause,
  play,
  nextSong,
  previousSong,
  setCurrentTime,
  setVolume,
  toggleShuffle,
  toggleRepeat,
  setDeviceName,
  setPaws,
  updateFromWebSocket,
} = playerSlice.actions;

export default playerSlice.reducer;
