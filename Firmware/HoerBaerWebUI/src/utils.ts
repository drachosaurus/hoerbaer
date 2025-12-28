import { ListBoxItem } from "./components/ListBox";

export const hours: ListBoxItem[] = Array.from({ length: 24 }, (_, i) => ({
  displayValue: i.toString().padStart(2, "0"),
  value: i
}));

export const minutes: ListBoxItem[] = Array.from({ length: 12 }, (_, i) => ({
  displayValue: (i * 5).toString().padStart(2, "0"),
  value: i * 5
}));
