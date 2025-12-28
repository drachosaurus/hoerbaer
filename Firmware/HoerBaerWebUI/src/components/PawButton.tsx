interface PawButtonProps {
  icon: string;
  size?: "small" | "medium" | "large";
  variant?: "primary" | "secondary";
  onClick?: () => void;
  children?: React.ReactNode;
}

const PawButton = ({ 
  icon, 
  size = "medium", 
  variant = "secondary", 
  onClick,
  children 
}: PawButtonProps) => {
  const sizeClasses = {
    small: "w-16 h-16",
    medium: "w-20 h-20",
    large: "w-28 h-28",
  };

  const variantClasses = {
    primary: {
      outer: "bg-primary shadow-floating",
      inner: "w-16 h-16 bg-[#5D4037] dark:bg-[#271c19] border-4 border-[#8D6E63] dark:border-[#3E2723]",
      ears: "bg-[#D7CCC8] dark:bg-[#4E342E] opacity-90 border-2 border-primary/20",
    },
    secondary: {
      outer: "bg-[#D4A373] dark:bg-[#5D4037] shadow-wood",
      inner: "w-12 h-12 bg-[#8D6E63] dark:bg-[#3E2723]",
      ears: "bg-[#FAEDCD] dark:bg-[#8D6E63] opacity-80",
    },
  };

  const isPrimary = variant === "primary";
  const earPositions = isPrimary
    ? [
        { className: "absolute -top-3 left-2 w-7 h-7" },
        { className: "absolute -top-6 left-1/2 -translate-x-1/2 w-8 h-8" },
        { className: "absolute -top-3 right-2 w-7 h-7" },
      ]
    : [
        { className: "absolute -top-2 left-1 w-5 h-5" },
        { className: "absolute -top-4 left-1/2 -translate-x-1/2 w-6 h-6" },
        { className: "absolute -top-2 right-1 w-5 h-5" },
      ];

  return (
    <button
      onClick={onClick}
      className={`relative group ${sizeClasses[size]} flex items-center justify-center transform transition-transform active:scale-95`}
    >
      <div className={`absolute inset-0 ${variantClasses[variant].outer} rounded-full group-active:shadow-wood-pressed`}></div>
      {earPositions.map((ear, index) => (
        <div
          key={index}
          className={`${ear.className} ${variantClasses[variant].ears} rounded-full`}
        ></div>
      ))}
      <div className={`relative z-10 ${variantClasses[variant].inner} rounded-full flex items-center justify-center shadow-inner text-[#FAEDCD] dark:text-[#D7CCC8] ${isPrimary ? 'shadow-[inset_0_2px_4px_rgba(0,0,0,0.3)] text-white' : ''}`}>
        {children || <span className={`material-icons-round ${isPrimary ? 'text-5xl ml-1' : 'text-3xl'}`}>{icon}</span>}
      </div>
    </button>
  );
};

export default PawButton;
