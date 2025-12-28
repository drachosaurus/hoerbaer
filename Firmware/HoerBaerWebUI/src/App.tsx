import { Route, Routes } from "react-router";
import { useEffect } from "react";
import { useDispatch } from "react-redux";
import { useGetSlotsQuery } from "./api/deviceApi";
import { setPaws } from "./store/playerSlice";
import Player from "./player/Player";
import SongList from "./songlist/SongList";

function App() {
  const dispatch = useDispatch();
  const { data: slots, isSuccess } = useGetSlotsQuery();

  useEffect(() => {
    if (isSuccess && slots) {
      dispatch(setPaws(slots));
    }
  }, [isSuccess, slots, dispatch]);

  return (
    <Routes>
      <Route path="/" element={<Player />} />
      <Route path="/songs" element={<SongList />} />
    </Routes>
  );
}

export default App;
