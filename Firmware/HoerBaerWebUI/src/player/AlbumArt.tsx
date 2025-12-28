import { Song } from "../store/playerSlice";

interface AlbumArtProps {
  currentSong: Song | null;
  currentTime: number;
  formatTime: (seconds: number) => string;
}

const AlbumArt = ({ currentSong, currentTime, formatTime }: AlbumArtProps) => {
  const progress = currentSong && currentSong.duration > 0 
    ? (currentTime / currentSong.duration) * 100 
    : 0;
  return (
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
            <div 
              className="h-full bg-primary rounded-full relative transition-all duration-300"
              style={{ width: `${progress}%` }}
            >
              <div className="absolute right-0 top-1/2 -translate-y-1/2 w-3 h-3 bg-white rounded-full shadow-md"></div>
            </div>
          </div>
        </div>

        {/* Time Display - Centered below progress bar */}
        <div className="absolute bottom-6 left-1/2 -translate-x-1/2 text-xs font-bold text-wood-DEFAULT/60 dark:text-wood-light/60 whitespace-nowrap">
          {formatTime(currentTime)} / {currentSong ? formatTime(currentSong.duration) : "0:00"}
        </div>
      </div>
    </div>
  );
};

export default AlbumArt;
