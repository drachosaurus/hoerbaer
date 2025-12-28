import { useEffect, useState } from "react";
import Page from "../components/Page";
import { useGetTimeQuery } from "../api/deviceApi";

export default function Home() {
  const getTime = (offset: number) => {
    const now = new Date();
    now.setTime(now.getTime() - offset);
    return `${now.toLocaleDateString()} ${now.toLocaleTimeString()}`;
  };

  const { data } = useGetTimeQuery();
  const [timeDisplay, setTimeDisplay] = useState(getTime(0));

  useEffect(() => {
    const offset = data != null ? data.epochtime - new Date().getTime() / 1000 : 0;
    console.log("data", data, "offset", offset);
    setTimeDisplay(getTime(offset));
    const interval = setInterval(() => setTimeDisplay(getTime(offset)), 1000);
    return () => clearInterval(interval);
  }, [data]);

  return (
    <Page title="Rrrrriiiingg...">
      <div>
        <div className="px-4 sm:px-0">
          <h3 className="text-base font-semibold leading-7 text-gray-900">Home</h3>
          <p className="mt-1 max-w-2xl text-sm leading-6 text-gray-500">
            Zuhause
          </p>
          <p className="mt-4 text-lg text-gray-700">{timeDisplay}</p>
        </div>
      </div>
    </Page>
  );
}
