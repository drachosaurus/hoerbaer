import { Route, Routes } from "react-router";
import Player from "./player/Player";
import SongList from "./songlist/SongList";

function App() {
  return (
    <Routes>
      <Route path="/" element={<Player />} />
      <Route path="/songs" element={<SongList />} />
    </Routes>
  );
}

export default App;
