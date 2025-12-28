import { Route, Routes } from "react-router";
import { useEffect, useRef } from "react";
import { useDispatch } from "react-redux";
import { useGetSlotsQuery } from "./api/deviceApi";
import { setPaws, updateFromWebSocket } from "./store/playerSlice";
import { createWebSocketService, WebSocketMessage } from "./api/websocket";
import Player from "./player/Player";
import SongList from "./songlist/SongList";

function App() {
  const dispatch = useDispatch();
  const { data: slots, isSuccess } = useGetSlotsQuery();
  const wsRef = useRef<ReturnType<typeof createWebSocketService> | null>(null);

  useEffect(() => {
    if (isSuccess && slots) {
      dispatch(setPaws(slots));
    }
  }, [isSuccess, slots, dispatch]);

  useEffect(() => {
    // Create and connect WebSocket
    const ws = createWebSocketService();
    wsRef.current = ws;

    ws.connect((message: WebSocketMessage) => {
      if (message.t === "state") {
        console.log("WebSocket state update:", message);
        dispatch(updateFromWebSocket(message));
      }
    });

    // Cleanup on unmount
    return () => {
      ws.disconnect();
    };
  }, [dispatch]);

  return (
    <Routes>
      <Route path="/" element={<Player />} />
      <Route path="/songs" element={<SongList />} />
    </Routes>
  );
}

export default App;
