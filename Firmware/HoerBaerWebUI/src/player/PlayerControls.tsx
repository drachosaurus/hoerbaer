interface PlayerControlsProps {
  isPlaying: boolean;
  onPrevious: () => void;
  onTogglePlay: () => void;
  onNext: () => void;
}

const PlayerControls = ({ isPlaying, onPrevious, onTogglePlay, onNext }: PlayerControlsProps) => {
  return (
    <div className="grid grid-cols-3 gap-4 items-center justify-items-center mb-8">
      {/* Previous Button */}
      <button
        onClick={onPrevious}
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
        onClick={onTogglePlay}
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
        onClick={onNext}
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
  );
};

export default PlayerControls;
