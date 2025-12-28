import { useDispatch, useSelector } from "react-redux";
import { useNavigate } from "react-router";
import { RootState } from "../store";
import {
  togglePlay,
  nextSong,
  previousSong,
  setVolume,
  toggleShuffle,
  toggleRepeat,
} from "../store/playerSlice";

const Player = () => {
  const dispatch = useDispatch();
  const navigate = useNavigate();
  const { currentSong, isPlaying, volume, shuffle, repeat } = useSelector(
    (state: RootState) => state.player
  );

  const formatTime = (seconds: number) => {
    const mins = Math.floor(seconds / 60);
    const secs = Math.floor(seconds % 60);
    return `${mins}:${secs.toString().padStart(2, "0")}`;
  };

  return (
    <div className="min-h-screen bg-background-light dark:bg-background-dark text-wood-dark dark:text-wood-light font-body flex flex-col items-center justify-center p-6 selection:bg-accent selection:text-white transition-colors duration-300">
      <main className="w-full max-w-md mx-auto relative z-10">
        {/* Header */}
        <header className="flex justify-between items-center mb-8 px-2">
          <button className="p-2 rounded-full hover:bg-wood-light/20 dark:hover:bg-wood-dark/40 transition-colors">
            <span className="material-icons-round text-3xl text-wood-DEFAULT dark:text-wood-light">
              menu
            </span>
          </button>
          <div className="flex items-center gap-2">
            <span className="w-2 h-2 bg-green-500 rounded-full animate-pulse"></span>
            <span className="text-sm font-bold tracking-wide text-wood-DEFAULT dark:text-wood-light uppercase">
              Connected
            </span>
          </div>
          <button className="p-2 rounded-full hover:bg-wood-light/20 dark:hover:bg-wood-dark/40 transition-colors">
            <span className="material-icons-round text-3xl text-wood-DEFAULT dark:text-wood-light">
              settings
            </span>
          </button>
        </header>

        {/* Album Art Circle */}
        <div className="relative w-full aspect-square mb-10 group">
          <div className="absolute inset-0 bg-primary/10 dark:bg-primary/20 rounded-full scale-110 blur-xl"></div>
          <div className="relative w-full h-full bg-wood-light dark:bg-wood-dark rounded-full shadow-wood border-8 border-white dark:border-wood-dark/50 flex flex-col items-center justify-center overflow-hidden">
            {/* Bear Ears */}
            <div className="absolute top-0 w-full h-full pointer-events-none">
              <div className="absolute -top-4 left-8 w-16 h-16 bg-primary rounded-full -z-10"></div>
              <div className="absolute -top-4 right-8 w-16 h-16 bg-primary rounded-full -z-10"></div>
            </div>

            {/* Album Art */}
            <div className="relative w-48 h-48 rounded-full overflow-hidden border-4 border-secondary/50 shadow-inner mb-4">
              <img
                alt="Album art"
                className="w-full h-full object-cover"
                src="https://lh3.googleusercontent.com/aida-public/AB6AXuBCQ03a6jxfSeT_sKZ54kiqKY8IwKjVOwmtvsjlm0C8pV3Y74jFefHoe6CKQXktTc3GwV1Qc5PpDj_GjyBo6v7ZhGFvqOKP5uIimmcH16aJTWxat1lB22q9yjxcFbc1rZtOVbTS2b8XEI3sd0cmQosPM2Oim3wlKVEoQbD7PyWMxXQnFhMukqZu9jW13izf7uWtz7QRAsRx85XeBNcOa0qnGysghS_F432iaR4_vFGPKqRmosFpN2rArWB-6SFq-bkcO8IR3eB2mww"
              />
              <div className="absolute inset-0 flex items-center justify-center bg-black/10">
                <div className="w-12 h-12 bg-black/80 rounded-full flex items-center justify-center backdrop-blur-sm">
                  <span className="material-icons-round text-white text-2xl animate-pulse">
                    music_note
                  </span>
                </div>
              </div>
            </div>

            {/* Song Info */}
            <div className="text-center px-6 mt-2 relative z-10">
              <h2 className="font-display text-2xl font-bold text-wood-dark dark:text-white mb-1">
                {currentSong?.title || "No song playing"}
              </h2>
              <p className="text-wood-DEFAULT dark:text-wood-light text-sm font-semibold">
                {currentSong?.artist || "Select a song"}
              </p>
            </div>

            {/* Progress Bar */}
            <div className="w-3/4 mt-6">
              <div className="h-2 bg-secondary/50 dark:bg-wood-dark/50 rounded-full overflow-hidden">
                <div className="h-full bg-primary w-2/3 rounded-full relative">
                  <div className="absolute right-0 top-1/2 -translate-y-1/2 w-3 h-3 bg-white rounded-full shadow-md"></div>
                </div>
              </div>
              <div className="flex justify-between text-xs font-bold text-wood-DEFAULT/60 dark:text-wood-light/60 mt-2">
                <span>1:24</span>
                <span>{currentSong ? formatTime(currentSong.duration) : "0:00"}</span>
              </div>
            </div>
          </div>
        </div>

        {/* Playback Controls */}
        <div className="grid grid-cols-3 gap-4 items-center justify-items-center mb-8">
          {/* Previous Button */}
          <button
            onClick={() => dispatch(previousSong())}
            className="relative group w-20 h-20 flex items-center justify-center transform transition-transform active:scale-95"
          >
            <div className="absolute inset-0 bg-[#D4A373] dark:bg-[#5D4037] rounded-full shadow-wood group-active:shadow-wood-pressed"></div>
            <div className="absolute -top-2 left-1 w-5 h-5 bg-[#FAEDCD] dark:bg-[#8D6E63] rounded-full opacity-80"></div>
            <div className="absolute -top-4 left-1/2 -translate-x-1/2 w-6 h-6 bg-[#FAEDCD] dark:bg-[#8D6E63] rounded-full opacity-80"></div>
            <div className="absolute -top-2 right-1 w-5 h-5 bg-[#FAEDCD] dark:bg-[#8D6E63] rounded-full opacity-80"></div>
            <div className="relative z-10 w-12 h-12 bg-[#8D6E63] dark:bg-[#3E2723] rounded-full flex items-center justify-center shadow-inner text-[#FAEDCD] dark:text-[#D7CCC8]">
              <span className="material-icons-round text-3xl">skip_previous</span>
            </div>
          </button>

          {/* Play/Pause Button */}
          <button
            onClick={() => dispatch(togglePlay())}
            className="relative group w-28 h-28 flex items-center justify-center transform transition-transform active:scale-95"
          >
            <div className="absolute inset-0 bg-primary rounded-full shadow-floating group-active:shadow-wood-pressed"></div>
            <div className="absolute -top-3 left-2 w-7 h-7 bg-[#D7CCC8] dark:bg-[#4E342E] rounded-full opacity-90 border-2 border-primary/20"></div>
            <div className="absolute -top-6 left-1/2 -translate-x-1/2 w-8 h-8 bg-[#D7CCC8] dark:bg-[#4E342E] rounded-full opacity-90 border-2 border-primary/20"></div>
            <div className="absolute -top-3 right-2 w-7 h-7 bg-[#D7CCC8] dark:bg-[#4E342E] rounded-full opacity-90 border-2 border-primary/20"></div>
            <div className="relative z-10 w-16 h-16 bg-[#5D4037] dark:bg-[#271c19] rounded-full flex items-center justify-center shadow-[inset_0_2px_4px_rgba(0,0,0,0.3)] text-white border-4 border-[#8D6E63] dark:border-[#3E2723]">
              <span className="material-icons-round text-5xl ml-1">
                {isPlaying ? "pause" : "play_arrow"}
              </span>
            </div>
          </button>

          {/* Next Button */}
          <button
            onClick={() => dispatch(nextSong())}
            className="relative group w-20 h-20 flex items-center justify-center transform transition-transform active:scale-95"
          >
            <div className="absolute inset-0 bg-[#D4A373] dark:bg-[#5D4037] rounded-full shadow-wood group-active:shadow-wood-pressed"></div>
            <div className="absolute -top-2 left-1 w-5 h-5 bg-[#FAEDCD] dark:bg-[#8D6E63] rounded-full opacity-80"></div>
            <div className="absolute -top-4 left-1/2 -translate-x-1/2 w-6 h-6 bg-[#FAEDCD] dark:bg-[#8D6E63] rounded-full opacity-80"></div>
            <div className="absolute -top-2 right-1 w-5 h-5 bg-[#FAEDCD] dark:bg-[#8D6E63] rounded-full opacity-80"></div>
            <div className="relative z-10 w-12 h-12 bg-[#8D6E63] dark:bg-[#3E2723] rounded-full flex items-center justify-center shadow-inner text-[#FAEDCD] dark:text-[#D7CCC8]">
              <span className="material-icons-round text-3xl">skip_next</span>
            </div>
          </button>
        </div>

        {/* Volume Control */}
        <div className="bg-white dark:bg-wood-dark/40 p-4 rounded-2xl shadow-sm border border-wood-light/50 dark:border-wood-dark/30 flex items-center gap-4">
          <button className="text-wood-DEFAULT dark:text-wood-light/70 hover:text-primary transition-colors">
            <span className="material-icons-round">volume_mute</span>
          </button>
          <input
            type="range"
            min="0"
            max="100"
            value={volume * 100}
            onChange={(e) => dispatch(setVolume(parseInt(e.target.value) / 100))}
            className="flex-1 h-3 bg-wood-light dark:bg-black/30 rounded-full cursor-pointer"
            aria-label="Volume control"
          />
          <button className="text-wood-DEFAULT dark:text-wood-light/70 hover:text-primary transition-colors">
            <span className="material-icons-round">volume_up</span>
          </button>
        </div>

        {/* Additional Controls */}
        <div className="flex justify-between mt-6 px-4">
          <button
            onClick={() => dispatch(toggleShuffle())}
            className={`flex flex-col items-center gap-1 transition-colors ${
              shuffle
                ? "text-primary dark:text-primary font-bold"
                : "text-wood-DEFAULT/60 dark:text-wood-light/40 hover:text-primary dark:hover:text-primary"
            }`}
          >
            <span className="material-icons-round text-xl">shuffle</span>
            <span className="text-[10px] font-bold uppercase tracking-wider">Shuffle</span>
          </button>
          <button className="flex flex-col items-center gap-1 text-wood-DEFAULT/60 dark:text-wood-light/40 hover:text-primary dark:hover:text-primary transition-colors">
            <span className="material-icons-round text-xl">favorite_border</span>
            <span className="text-[10px] font-bold uppercase tracking-wider">Like</span>
          </button>
          <button
            onClick={() => dispatch(toggleRepeat())}
            className={`flex flex-col items-center gap-1 transition-colors ${
              repeat
                ? "text-primary dark:text-primary font-bold"
                : "text-wood-DEFAULT/60 dark:text-wood-light/40 hover:text-primary dark:hover:text-primary"
            }`}
          >
            <span className="material-icons-round text-xl">repeat</span>
            <span className="text-[10px] font-bold uppercase tracking-wider">Repeat</span>
          </button>
          <button 
            onClick={() => navigate("/songs")}
            className="flex flex-col items-center gap-1 text-wood-DEFAULT/60 dark:text-wood-light/40 hover:text-primary dark:hover:text-primary transition-colors"
          >
            <span className="material-icons-round text-xl">queue_music</span>
            <span className="text-[10px] font-bold uppercase tracking-wider">List</span>
          </button>
        </div>
      </main>

      {/* Background Pattern */}
      <div className="fixed inset-0 pointer-events-none opacity-[0.03] dark:opacity-[0.05] z-0 bg-[radial-gradient(#8D6E63_1px,transparent_1px)] bg-[length:20px_20px]"></div>
    </div>
  );
};

export default Player;
