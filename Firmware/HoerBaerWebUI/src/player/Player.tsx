import { useDispatch, useSelector } from "react-redux";
import { useNavigate } from "react-router";
import { RootState } from "../store";
import {
  togglePlay,
  nextSong,
  previousSong,
  setVolume,
} from "../store/playerSlice";
import { sendCommand } from "../api/websocket";
import AlbumArt from "./AlbumArt";
import PlayerControls from "./PlayerControls";
import VolumeControl from "./VolumeControl";

const Player = () => {
  const dispatch = useDispatch();
  const navigate = useNavigate();
  const { currentSong, isPlaying, currentTime, volume } = useSelector(
    (state: RootState) => state.player
  );

  const formatTime = (seconds: number) => {
    const mins = Math.floor(seconds / 60);
    const secs = Math.floor(seconds % 60);
    return `${mins}:${secs.toString().padStart(2, "0")}`;
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
    <div className="min-h-screen bg-background-light dark:bg-background-dark text-wood-dark dark:text-wood-light font-body flex flex-col items-center justify-center p-6 selection:bg-accent selection:text-white transition-colors duration-300">
      <main className="w-full max-w-md mx-auto relative z-10">
        {/* Header */}
        <header className="flex justify-between items-center mb-8 px-2">
          <div className="flex items-center gap-2">
            <span className="w-2 h-2 bg-green-500 rounded-full animate-pulse"></span>
            <span className="text-sm font-bold tracking-wide text-wood-DEFAULT dark:text-wood-light uppercase">
              Connected
            </span>
          </div>
          <button 
            onClick={() => navigate("/songs")}
            className="p-2 rounded-full hover:bg-wood-light/20 dark:hover:bg-wood-dark/40 transition-colors"
          >
            <span className="material-icons-round text-3xl text-wood-DEFAULT dark:text-wood-light">
              queue_music
            </span>
          </button>
        </header>

        <AlbumArt currentSong={currentSong} currentTime={currentTime} formatTime={formatTime} />

        <PlayerControls
          isPlaying={isPlaying}
          onTogglePlay={handleTogglePlay}
          onNext={handleNext}
          onPrevious={handlePrevious}
        />

        <VolumeControl volume={volume} onVolumeChange={(v) => dispatch(setVolume(v))} />
      </main>

      {/* Background Pattern */}
      <div className="fixed inset-0 pointer-events-none opacity-[0.03] dark:opacity-[0.05] z-0 bg-[radial-gradient(#8D6E63_1px,transparent_1px)] bg-[length:20px_20px]"></div>
    </div>
  );
};

export default Player;
