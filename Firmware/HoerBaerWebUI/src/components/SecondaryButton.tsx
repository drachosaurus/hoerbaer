import { MouseEventHandler } from "react";

interface Props {
  type?: "submit" | "reset" | "button" | undefined;
  onClick?: MouseEventHandler<HTMLButtonElement> | undefined;
  children: React.ReactNode;
}

export default function SecondaryButton({ type, onClick, children }: Props) {
  return (
    <button
      type={type}
      onClick={onClick}
      className="bg-transparent hover:bg-blue-500 text-blue-700 font-semibold hover:text-white py-2 px-4 border border-blue-500 hover:border-transparent rounded"
    >
      {children}
    </button>
  );
}
