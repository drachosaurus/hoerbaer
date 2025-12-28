import { ChangeEventHandler } from "react";

interface Props {
  id: string;
  type?: "text" | "password" | undefined;
  label: string;
  value?: string;
  onChange?: ChangeEventHandler<HTMLInputElement> | undefined;
}

export default function TextBox({ id, type, label, value, onChange }: Props) {
  return (
    <div className="sm:col-span-3">
      <label htmlFor={id} className="block text-sm font-medium leading-6 text-gray-900">
        {label}
      </label>
      <div className="mt-2">
        <input
          type={type || "text"}
          name={id}
          id={id}
          value={value}
          className="block w-full rounded-md border-0 py-1.5 text-gray-900 shadow-sm ring-1 ring-inset ring-gray-300 placeholder:text-gray-400 focus:ring-2 focus:ring-inset focus:ring-indigo-600 sm:text-sm sm:leading-6"
          onChange={onChange}
        />
      </div>
    </div>
  );
}
