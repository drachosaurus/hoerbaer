import { Switch as HeadlessSwitch } from "@headlessui/react";

interface Props {
  checked?: boolean;
  onChange?(checked: boolean): void;
  label: string;
}

export default function Switch({ checked, onChange, label }: Props) {
  return (
    <div>
      <HeadlessSwitch.Group>
        <div className="flex items-center">
          <HeadlessSwitch
            checked={checked}
            onChange={onChange}
            className={`${
              checked ? "bg-blue-600" : "bg-gray-200"
            } relative inline-flex h-6 w-11 items-center rounded-full transition-colors focus:outline-none focus:ring-2 focus:ring-indigo-500 focus:ring-offset-2`}
          >
            <span
              className={`${
                checked ? "translate-x-6" : "translate-x-1"
              } inline-block h-4 w-4 transform rounded-full bg-white transition-transform`}
            />
          </HeadlessSwitch>
          <HeadlessSwitch.Label className="ml-4">{label}</HeadlessSwitch.Label>
        </div>
      </HeadlessSwitch.Group>
    </div>
  );
}
