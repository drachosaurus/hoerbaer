import { useDispatch, useSelector } from "react-redux";
import { useNavigate } from "react-router";
import { RootState } from "../store";
import {
  togglePlay,
  nextSong,
  previousSong,
  setVolume,
} from "../store/playerSlice";
import { sendCommand, sendSetVolume, ConnectionStatus } from "../api/websocket";
import AlbumArt from "./AlbumArt";
import PlayerControls from "./PlayerControls";
import VolumeControl from "./VolumeControl";

interface PlayerProps {
  connectionStatus: ConnectionStatus;
}

const Player = ({ connectionStatus }: PlayerProps) => {
  const dispatch = useDispatch();
  const navigate = useNavigate();
  const { currentSong, isPlaying, currentTime, volume, battery, deviceName, maxVolume } = useSelector(
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

  const handleVolumeChange = (v: number) => {
    // Convert 0-1 range to device volume range (0-maxVolume)
    const deviceVolume = Math.round(v * maxVolume);
    sendSetVolume(deviceVolume);
    dispatch(setVolume(v));
  };

  return (
    <div className="min-h-screen bg-background-light dark:bg-background-dark text-wood-dark dark:text-wood-light font-body flex flex-col items-center justify-center p-6 selection:bg-accent selection:text-white transition-colors duration-300">
      <main className="w-full max-w-md mx-auto relative z-10">
        {/* Header */}
        <header className="flex justify-between items-center mb-8 px-2">
          <div className="flex items-center gap-2">
            <span className={`w-2 h-2 rounded-full ${
              connectionStatus === "connected" 
                ? "bg-green-500 animate-pulse" 
                : connectionStatus === "reconnecting" 
                ? "bg-yellow-500 animate-pulse" 
                : "bg-red-500"
            }`}></span>
            <span className="text-sm font-bold tracking-wide text-wood-DEFAULT dark:text-wood-light uppercase">
              {connectionStatus === "connected" 
                ? (deviceName || "Connected")
                : connectionStatus === "reconnecting" 
                ? "Reconnecting..." 
                : "Disconnected"}
            </span>
          </div>
          <div className="flex items-center gap-2">
            {battery && (
              <div className="flex items-center gap-1 text-wood-DEFAULT dark:text-wood-light">
                <span className="text-sm font-bold">{battery.percentage}%</span>
                <span className="material-icons-round text-2xl">
                  {battery.charging 
                    ? "battery_charging_full" 
                    : battery.percentage > 80 
                    ? "battery_full" 
                    : battery.percentage > 50 
                    ? "battery_5_bar" 
                    : battery.percentage > 20 
                    ? "battery_3_bar" 
                    : "battery_1_bar"}
                </span>
              </div>
            )}
          </div>
        </header>

        <AlbumArt currentSong={currentSong} currentTime={currentTime} formatTime={formatTime} />

        <PlayerControls
          isPlaying={isPlaying}
          onTogglePlay={handleTogglePlay}
          onNext={handleNext}
          onPrevious={handlePrevious}
        />

        <VolumeControl volume={volume} onVolumeChange={handleVolumeChange} />

        {/* Song List Button */}
        <button
          onClick={() => navigate("/songs")}
          className="w-full mt-4 bg-white dark:bg-wood-dark/40 p-4 rounded-2xl shadow-sm border border-wood-light/50 dark:border-wood-dark/30 flex items-center justify-center gap-3 hover:bg-wood-light/20 dark:hover:bg-wood-dark/60 transition-colors"
        >
          <span className="material-icons-round text-2xl text-wood-DEFAULT dark:text-wood-light">
            queue_music
          </span>
          <span className="text-lg font-bold text-wood-DEFAULT dark:text-wood-light">
            Song Library
          </span>
        </button>
      </main>

      {/* Background Pattern */}
      <div className="fixed inset-0 pointer-events-none opacity-[0.03] dark:opacity-[0.05] z-0 bg-[radial-gradient(#8D6E63_1px,transparent_1px)] bg-[length:20px_20px]"></div>
    </div>
  );
};

export default Player;
