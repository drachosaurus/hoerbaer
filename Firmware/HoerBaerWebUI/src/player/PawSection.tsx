import { Song } from "../store/playerSlice";

interface PawSectionProps {
  pawId: string;
  pawName: string;
  pawIcon: string;
  pawColor: string;
  songs: Song[];
  totalSongs?: number;
  currentSong: Song | null;
  isPlaying: boolean;
  onSongClick: (song: Song) => void;
  isExpanded: boolean;
  onToggle: () => void;
}

const PawSection = ({ 
  pawName, 
  pawIcon, 
  songs,
  totalSongs, 
  currentSong, 
  isPlaying, 
  onSongClick,
  isExpanded,
  onToggle
}: PawSectionProps) => {
  return (
    <section>
      <button
        onClick={onToggle}
        className="w-full flex items-center mb-4 space-x-3 hover:opacity-80 transition-opacity"
      >
        <div className="w-12 h-12 bg-secondary rounded-full flex items-center justify-center shadow-lg border-2 border-[#B88A5C]">
          <span className="material-icons-round text-white text-2xl drop-shadow-md">
            {pawIcon}
          </span>
        </div>
        <h2 className="text-2xl font-bold text-primary dark:text-accent">{pawName}</h2>
        <span className="bg-wood-light/50 dark:bg-wood-dark/50 text-wood-DEFAULT dark:text-wood-light px-3 py-1 rounded-full text-xs font-bold uppercase tracking-wider">
          {totalSongs !== undefined && totalSongs !== songs.length
            ? `${songs.length} of ${totalSongs} songs`
            : `${songs.length} songs`}
        </span>
        <span className="material-icons-round text-primary dark:text-accent ml-auto transition-transform duration-200" style={{ transform: isExpanded ? 'rotate(180deg)' : 'rotate(0deg)' }}>
          expand_more
        </span>
      </button>

      {isExpanded && (
        <div className="bg-white dark:bg-wood-dark rounded-3xl p-2 shadow-wood dark:shadow-none dark:border dark:border-white/5 space-y-1">
        {songs.map((song, index) => {
          const isCurrentSong = currentSong?.id === song.id;
          return (
            <button
              key={song.id}
              onClick={() => onSongClick(song)}
              className={`w-full flex items-center p-3 rounded-2xl transition-colors group text-left ${
                isCurrentSong
                  ? "bg-primary/10 dark:bg-accent/10 border border-primary/20 dark:border-accent/20"
                  : "hover:bg-secondary/10 dark:hover:bg-white/5"
              }`}
            >
              <div
                className={`w-12 h-12 rounded-xl flex items-center justify-center mr-4 shrink-0 font-bold text-lg ${
                  isCurrentSong
                    ? "bg-primary text-white relative overflow-hidden"
                    : "bg-wood-light/30 dark:bg-wood-dark/30 text-wood-DEFAULT dark:text-wood-light group-hover:scale-105 transition-transform"
                }`}
              >
                {isCurrentSong && isPlaying ? (
                  <div className="absolute inset-0 flex items-center justify-center space-x-1">
                    <div className="w-1 h-3 bg-white rounded-full animate-bounce animate-delay-0"></div>
                    <div className="w-1 h-5 bg-white rounded-full animate-bounce animate-delay-100"></div>
                    <div className="w-1 h-2 bg-white rounded-full animate-bounce animate-delay-200"></div>
                  </div>
                ) : (
                  index + 1
                )}
              </div>
              <div className="flex-1 min-w-0">
                <h3
                  className={`font-bold truncate ${
                    isCurrentSong
                      ? "text-primary dark:text-accent"
                      : "text-gray-800 dark:text-gray-100"
                  }`}
                >
                  {song.title}
                </h3>
                <p
                  className={`text-sm truncate ${
                    isCurrentSong
                      ? "text-primary/70 dark:text-accent/70"
                      : "text-gray-500 dark:text-gray-400"
                  }`}
                >
                  {song.artist}
                </p>
              </div>
              <span
                className={`material-icons-round ${
                  isCurrentSong
                    ? "text-primary dark:text-accent"
                    : "text-primary dark:text-accent opacity-0 group-hover:opacity-100"
                } transition-opacity`}
              >
                {isCurrentSong && isPlaying ? "pause_circle" : "play_circle"}
              </span>
            </button>
          );
        })}
      </div>
      )}
    </section>
  );
};

export default PawSection;
