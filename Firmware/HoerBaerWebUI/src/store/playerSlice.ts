import { createSlice, PayloadAction } from "@reduxjs/toolkit";
import { Slot } from "../api/deviceApi";

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
  shuffle: boolean;
  repeat: boolean;
  paws: Paw[];
}

const initialState: PlayerState = {
  currentSong: null,
  isPlaying: false,
  currentTime: 0,
  volume: 0.66,
  shuffle: false,
  repeat: true,
  paws: [
    {
      id: "play-paw",
      name: "Play Paw",
      icon: "play_arrow",
      color: "blue",
      songs: [
        { id: "1", title: "Teddy Bear Picnic", artist: "Classic Kids", path: "/mock/1.mp3", paw: "play-paw", duration: 225 },
        { id: "2", title: "Baby Shark", artist: "Pinkfong", path: "/mock/2.mp3", paw: "play-paw", duration: 210 },
        { id: "3", title: "Twinkle Twinkle", artist: "Lullabies", path: "/mock/3.mp3", paw: "play-paw", duration: 180 },
      ]
    },
    {
      id: "sleepy-paw",
      name: "Sleepy Paw",
      icon: "bedtime",
      color: "indigo",
      songs: [
        { id: "4", title: "White Noise: Rain", artist: "Nature Sounds", path: "/mock/4.mp3", paw: "sleepy-paw", duration: 600 },
        { id: "5", title: "Brahms' Lullaby", artist: "Piano Classics", path: "/mock/5.mp3", paw: "sleepy-paw", duration: 240 },
      ]
    },
    {
      id: "story-paw",
      name: "Story Paw",
      icon: "auto_stories",
      color: "green",
      songs: [
        { id: "6", title: "Three Little Pigs", artist: "Fairy Tales", path: "/mock/6.mp3", paw: "story-paw", duration: 420 },
        { id: "7", title: "Little Red Riding Hood", artist: "Fairy Tales", path: "/mock/7.mp3", paw: "story-paw", duration: 390 },
      ]
    },
    {
      id: "learn-paw",
      name: "Learn Paw",
      icon: "school",
      color: "orange",
      songs: [
        { id: "8", title: "The ABC Song", artist: "Learning", path: "/mock/8.mp3", paw: "learn-paw", duration: 150 },
        { id: "9", title: "Numbers Song", artist: "Learning", path: "/mock/9.mp3", paw: "learn-paw", duration: 165 },
      ]
    },
  ]
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
  setPaws,
} = playerSlice.actions;

export default playerSlice.reducer;
