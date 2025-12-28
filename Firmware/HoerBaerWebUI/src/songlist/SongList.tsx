import { useState } from "react";
import { useDispatch, useSelector } from "react-redux";
import { useNavigate } from "react-router";
import { RootState } from "../store";
import { playSong, togglePlay, nextSong, previousSong, Song } from "../store/playerSlice";
import { sendPlaySlot, sendCommand } from "../api/websocket";
import SearchBar from "../components/SearchBar";
import PawSection from "../player/PawSection";
import MiniPlayer from "./MiniPlayer";

const SongList = () => {
  const dispatch = useDispatch();
  const navigate = useNavigate();
  const { paws, currentSong, currentTime, isPlaying } = useSelector((state: RootState) => state.player);
  const [searchQuery, setSearchQuery] = useState("");

  const handlePlaySong = (song: Song) => {
    // Find the slot (paw index) and song index
    const slotIndex = paws.findIndex((p) => p.id === song.paw);
    const songIndex = paws[slotIndex]?.songs.findIndex((s) => s.id === song.id);
    
    if (slotIndex !== -1 && songIndex !== -1) {
      sendPlaySlot(slotIndex, songIndex);
    }
    
    dispatch(playSong(song));
  };

  const handleTogglePlay = () => {
    sendCommand(isPlaying ? "pause" : "play");
    dispatch(togglePlay());
  };

  const handleNext = () => {
    sendCommand("next");
    dispatch(nextSong());
  };

  const handlePrevious = () => {
    sendCommand("previous");
    dispatch(previousSong());
  };

  return (
    <div className="min-h-screen bg-background-light dark:bg-background-dark text-text-light dark:text-text-dark flex flex-col transition-colors duration-300">
      {/* Header */}
      <header className="fixed top-0 w-full z-50 bg-white/90 dark:bg-wood-dark/90 backdrop-blur-md border-b border-primary/10 dark:border-white/10">
        <div className="px-6 py-4 flex justify-between items-center max-w-md mx-auto">
          <button 
            onClick={() => navigate("/")}
            className="p-2 -ml-2 rounded-full hover:bg-gray-100 dark:hover:bg-white/10 transition-colors"
          >
            <span className="material-icons-round text-3xl text-primary dark:text-accent">
              arrow_back_ios_new
            </span>
          </button>
          <h1 className="text-xl font-bold text-primary dark:text-accent tracking-wide">
            Bear Tunes
          </h1>
          <div className="w-10 h-10 rounded-full bg-secondary/20 flex items-center justify-center overflow-hidden border-2 border-primary dark:border-accent">
            <span className="material-icons-round text-primary dark:text-accent">pets</span>
          </div>
        </div>
      </header>

      {/* Main Content */}
      <main className="flex-1 pt-24 pb-28 px-4 max-w-md mx-auto w-full space-y-8">
        <SearchBar value={searchQuery} onChange={setSearchQuery} />

        {paws.map((paw) => {
          const filteredSongs = paw.songs.filter((song) =>
            searchQuery
              ? song.title.toLowerCase().includes(searchQuery.toLowerCase()) ||
                song.artist.toLowerCase().includes(searchQuery.toLowerCase())
              : true
          );

          if (filteredSongs.length === 0) return null;

          return (
            <PawSection
              key={paw.id}
              pawId={paw.id}
              pawName={paw.name}
              pawIcon={paw.icon}
              pawColor={paw.color}
              songs={filteredSongs}
              currentSong={currentSong}
              isPlaying={isPlaying}
              onSongClick={handlePlaySong}
            />
          );
        })}
      </main>

      {currentSong && (
        <MiniPlayer
          currentSong={currentSong}
          currentTime={currentTime}
          isPlaying={isPlaying}
          pawName={paws.find((p) => p.id === currentSong.paw)?.name}
          onTogglePlay={handleTogglePlay}
          onNext={handleNext}
          onPrevious={handlePrevious}
        />
      )}
    </div>
  );
};

export default SongList;
