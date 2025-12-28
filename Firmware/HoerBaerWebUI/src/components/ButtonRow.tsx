interface Props {
  children: React.ReactNode;
}

export default function ButtonRow({ children }: Props) {
  return <div className="mt-4 flex items-center gap-x-4">{children}</div>;
}
