interface SearchBarProps {
  value: string;
  onChange: (value: string) => void;
  placeholder?: string;
}

const SearchBar = ({ value, onChange, placeholder = "Find a song..." }: SearchBarProps) => {
  return (
    <div className="relative">
      <span className="absolute left-4 top-1/2 transform -translate-y-1/2 material-icons-round text-gray-400 dark:text-gray-500">
        search
      </span>
      <input
        className="w-full pl-12 pr-4 py-3 rounded-2xl bg-white dark:bg-wood-dark border-2 border-transparent focus:border-primary focus:ring-0 dark:text-white shadow-wood transition-all placeholder-gray-400 dark:placeholder-gray-500 text-lg"
        placeholder={placeholder}
        type="text"
        value={value}
        onChange={(e) => onChange(e.target.value)}
      />
    </div>
  );
};

export default SearchBar;
