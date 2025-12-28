interface Props {
  title: string;
  children: React.ReactNode;
}

export default function Page({ title, children }: Props) {
  return (
    <main>
      <div className="mx-auto max-w-7xl py-4 px-6">
        <h1 className="text-3xl font-bold tracking-tight text-gray-900 pb-8">{`${title}`}</h1>
        {children}
      </div>
    </main>
  );
}
