import { createSlice, PayloadAction } from "@reduxjs/toolkit";

export interface Song {
  id: string;
  title: string;
  artist: string;
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
        { id: "1", title: "Teddy Bear Picnic", artist: "Classic Kids", paw: "play-paw", duration: 225 },
        { id: "2", title: "Baby Shark", artist: "Pinkfong", paw: "play-paw", duration: 210 },
        { id: "3", title: "Twinkle Twinkle", artist: "Lullabies", paw: "play-paw", duration: 180 },
      ]
    },
    {
      id: "sleepy-paw",
      name: "Sleepy Paw",
      icon: "bedtime",
      color: "indigo",
      songs: [
        { id: "4", title: "White Noise: Rain", artist: "Nature Sounds", paw: "sleepy-paw", duration: 600 },
        { id: "5", title: "Brahms' Lullaby", artist: "Piano Classics", paw: "sleepy-paw", duration: 240 },
      ]
    },
    {
      id: "story-paw",
      name: "Story Paw",
      icon: "auto_stories",
      color: "green",
      songs: [
        { id: "6", title: "Three Little Pigs", artist: "Fairy Tales", paw: "story-paw", duration: 420 },
        { id: "7", title: "Little Red Riding Hood", artist: "Fairy Tales", paw: "story-paw", duration: 390 },
      ]
    },
    {
      id: "learn-paw",
      name: "Learn Paw",
      icon: "school",
      color: "orange",
      songs: [
        { id: "8", title: "The ABC Song", artist: "Learning", paw: "learn-paw", duration: 150 },
        { id: "9", title: "Numbers Song", artist: "Learning", paw: "learn-paw", duration: 165 },
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
} = playerSlice.actions;

export default playerSlice.reducer;
