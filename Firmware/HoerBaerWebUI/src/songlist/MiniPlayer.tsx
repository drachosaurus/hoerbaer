import { Song } from "../store/playerSlice";

interface MiniPlayerProps {
  currentSong: Song;
  currentTime: number;
  isPlaying: boolean;
  pawName?: string;
  onTogglePlay: () => void;
  onNext: () => void;
  onPrevious: () => void;
}

const MiniPlayer = ({ 
  currentSong, 
  currentTime,
  isPlaying, 
  pawName,
  onTogglePlay,
  onNext,
  onPrevious
}: MiniPlayerProps) => {
  const progress = currentSong.duration > 0 
    ? (currentTime / currentSong.duration) * 100 
    : 0;

  return (
    <div className="fixed bottom-0 left-0 right-0 bg-white/95 dark:bg-[#3D3028]/95 backdrop-blur-lg border-t border-gray-100 dark:border-white/5">
      <div className="w-full bg-gray-200 dark:bg-white/10 h-1">
        <div 
          className="bg-primary h-1 rounded-r-full transition-all duration-300"
          style={{ width: `${progress}%` }}
        ></div>
      </div>
      <div className="max-w-md mx-auto px-4 py-3 flex items-center justify-between">
        <div className="flex items-center space-x-3 flex-1 min-w-0">
          <div className="w-12 h-12 rounded-xl bg-primary shadow-lg overflow-hidden shrink-0 relative">
            <div className="absolute inset-0 bg-gradient-to-tr from-primary to-secondary opacity-80"></div>
            <span className="absolute inset-0 flex items-center justify-center material-icons-round text-white/50 text-2xl">
              music_note
            </span>
          </div>
          <div className="min-w-0">
            <h4 className="font-bold text-gray-900 dark:text-white text-sm truncate">
              {currentSong.title}
            </h4>
            <p className="text-xs text-gray-500 dark:text-gray-400 truncate">
              {currentSong.artist}{pawName && ` • ${pawName}`}
            </p>
          </div>
        </div>
        <div className="flex items-center space-x-4 pl-4">
          <button 
            onClick={onPrevious}
            className="text-gray-400 hover:text-primary dark:text-gray-400 dark:hover:text-accent transition-colors"
          >
            <span className="material-icons-round text-3xl">skip_previous</span>
          </button>
          <button 
            onClick={onTogglePlay}
            className="w-12 h-12 rounded-full bg-primary text-white flex items-center justify-center shadow-lg hover:bg-primary/90 transition-transform active:scale-95 border-2 border-[#6d4a30]"
          >
            <span className="material-icons-round text-3xl">
              {isPlaying ? "pause" : "play_arrow"}
            </span>
          </button>
          <button 
            onClick={onNext}
            className="text-gray-400 hover:text-primary dark:text-gray-400 dark:hover:text-accent transition-colors"
          >
            <span className="material-icons-round text-3xl">skip_next</span>
          </button>
        </div>
      </div>
    </div>
  );
};

export default MiniPlayer;
