import { useState } from "react";
import { useDispatch, useSelector } from "react-redux";
import { useNavigate } from "react-router";
import { RootState } from "../store";
import { playSong, Song } from "../store/playerSlice";

const SongList = () => {
  const dispatch = useDispatch();
  const navigate = useNavigate();
  const { paws, currentSong, isPlaying } = useSelector((state: RootState) => state.player);
  const [searchQuery, setSearchQuery] = useState("");

  const handlePlaySong = (song: Song) => {
    dispatch(playSong(song));
  };

  const getPawColorClass = (color: string) => {
    const colors: { [key: string]: string } = {
      blue: "bg-blue-100 text-blue-500",
      indigo: "bg-indigo-100 text-indigo-500",
      green: "bg-green-100 text-green-600",
      orange: "bg-orange-100 text-orange-500",
    };
    return colors[color] || "bg-gray-100 text-gray-500";
  };

  const getPawBadgeClass = (color: string) => {
    const colors: { [key: string]: string } = {
      blue: "bg-primary/10 dark:bg-accent/10 text-primary dark:text-accent",
      indigo: "bg-indigo-100 text-indigo-600 dark:bg-indigo-900 dark:text-indigo-200",
      green: "bg-green-100 text-green-600 dark:bg-green-900 dark:text-green-200",
      orange: "bg-orange-100 text-orange-600 dark:bg-orange-900 dark:text-orange-200",
    };
    return colors[color] || "bg-gray-100 text-gray-600";
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
        {/* Search */}
        <div className="relative">
          <span className="absolute left-4 top-1/2 transform -translate-y-1/2 material-icons-round text-gray-400 dark:text-gray-500">
            search
          </span>
          <input
            className="w-full pl-12 pr-4 py-3 rounded-2xl bg-white dark:bg-wood-dark border-2 border-transparent focus:border-primary focus:ring-0 dark:text-white shadow-wood transition-all placeholder-gray-400 dark:placeholder-gray-500 text-lg"
            placeholder="Find a song..."
            type="text"
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
          />
        </div>

        {/* Paw Sections */}
        {paws.map((paw) => (
          <section key={paw.id}>
            <div className="flex items-center mb-4 space-x-3">
              <div className="w-12 h-12 bg-secondary rounded-full flex items-center justify-center shadow-lg border-2 border-[#B88A5C]">
                <span className="material-icons-round text-white text-2xl drop-shadow-md">
                  {paw.icon}
                </span>
              </div>
              <h2 className="text-2xl font-bold text-primary dark:text-accent">{paw.name}</h2>
              <span
                className={`${getPawBadgeClass(
                  paw.color
                )} px-3 py-1 rounded-full text-xs font-bold uppercase tracking-wider`}
              >
                {paw.songs.length} songs
              </span>
            </div>

            <div className="bg-white dark:bg-wood-dark rounded-3xl p-2 shadow-wood dark:shadow-none dark:border dark:border-white/5 space-y-1">
              {paw.songs
                .filter((song) =>
                  searchQuery
                    ? song.title.toLowerCase().includes(searchQuery.toLowerCase()) ||
                      song.artist.toLowerCase().includes(searchQuery.toLowerCase())
                    : true
                )
                .map((song, index) => {
                  const isCurrentSong = currentSong?.id === song.id;
                  return (
                    <button
                      key={song.id}
                      onClick={() => handlePlaySong(song)}
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
                            : `${getPawColorClass(paw.color)} group-hover:scale-105 transition-transform`
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
          </section>
        ))}
      </main>

      {/* Mini Player */}
      {currentSong && (
        <div className="fixed bottom-0 left-0 right-0 bg-white/95 dark:bg-[#3D3028]/95 backdrop-blur-lg border-t border-gray-100 dark:border-white/5">
          <div className="w-full bg-gray-200 dark:bg-white/10 h-1">
            <div className="bg-primary h-1 w-1/3 rounded-r-full"></div>
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
                  {currentSong.artist} • {paws.find((p) => p.id === currentSong.paw)?.name}
                </p>
              </div>
            </div>
            <div className="flex items-center space-x-4 pl-4">
              <button className="text-gray-400 hover:text-primary dark:text-gray-400 dark:hover:text-accent transition-colors">
                <span className="material-icons-round text-3xl">skip_previous</span>
              </button>
              <button className="w-12 h-12 rounded-full bg-primary text-white flex items-center justify-center shadow-lg hover:bg-primary/90 transition-transform active:scale-95 border-2 border-[#6d4a30]">
                <span className="material-icons-round text-3xl">
                  {isPlaying ? "pause" : "play_arrow"}
                </span>
              </button>
              <button className="text-gray-400 hover:text-primary dark:text-gray-400 dark:hover:text-accent transition-colors">
                <span className="material-icons-round text-3xl">skip_next</span>
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default SongList;
