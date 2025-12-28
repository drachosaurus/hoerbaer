interface Props {
  title: string;
  children: React.ReactNode;
}

export default function ContentGroup({ title, children }: Props) {
  return (
    <>
      <h2 className="text-base font-semibold leading-7 text-gray-900">{title}</h2>
      <div className="mt-1 mb-4 text-sm leading-6 text-gray-600">{children}</div>
    </>
  );
}
