interface VolumeControlProps {
  volume: number;
  onVolumeChange: (volume: number) => void;
}

const VolumeControl = ({ volume, onVolumeChange }: VolumeControlProps) => {
  return (
    <div className="bg-white dark:bg-wood-dark/40 p-4 rounded-2xl shadow-sm border border-wood-light/50 dark:border-wood-dark/30 flex items-center gap-4">
      <button className="text-wood-DEFAULT dark:text-wood-light/70 hover:text-primary transition-colors">
        <span className="material-icons-round">volume_mute</span>
      </button>
      <input
        type="range"
        min="0"
        max="100"
        value={volume * 100}
        onChange={(e) => onVolumeChange(parseInt(e.target.value) / 100)}
        className="flex-1 h-3 bg-wood-light dark:bg-black/30 rounded-full cursor-pointer"
        aria-label="Volume control"
      />
      <button className="text-wood-DEFAULT dark:text-wood-light/70 hover:text-primary transition-colors">
        <span className="material-icons-round">volume_up</span>
      </button>
    </div>
  );
};

export default VolumeControl;
