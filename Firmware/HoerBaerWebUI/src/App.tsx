import { Route, Routes } from "react-router";
import { useEffect, useRef, useState } from "react";
import { useDispatch } from "react-redux";
import { useGetSlotsQuery, useGetInfoQuery } from "./api/deviceApi";
import { setPaws, setDeviceName, updateFromWebSocket } from "./store/playerSlice";
import { createWebSocketService, setWebSocketInstance, WebSocketMessage, ConnectionStatus } from "./api/websocket";
import Player from "./player/Player";
import SongList from "./songlist/SongList";

function App() {
  const dispatch = useDispatch();
  const { data: slots, isSuccess } = useGetSlotsQuery();
  const { data: deviceInfo, isSuccess: isInfoSuccess } = useGetInfoQuery();
  const wsRef = useRef<ReturnType<typeof createWebSocketService> | null>(null);
  const [wsStatus, setWsStatus] = useState<ConnectionStatus>("disconnected");

  useEffect(() => {
    if (isSuccess && slots) {
      dispatch(setPaws(slots));
    }
  }, [isSuccess, slots, dispatch]);

  useEffect(() => {
    if (isInfoSuccess && deviceInfo) {
      dispatch(setDeviceName(deviceInfo.name));
    }
  }, [isInfoSuccess, deviceInfo, dispatch]);

  useEffect(() => {
    // Create and connect WebSocket
    const ws = createWebSocketService();
    wsRef.current = ws;
    setWebSocketInstance(ws);

    ws.connect(
      (message: WebSocketMessage) => {
        if (message.t === "state") {
          console.log("WebSocket state update:", message);
          dispatch(updateFromWebSocket(message));
        }
      },
      (status: ConnectionStatus) => {
        setWsStatus(status);
      }
    );

    // Cleanup on unmount
    return () => {
      ws.disconnect();
      setWebSocketInstance(null as any);
    };
  }, [dispatch]);

  return (
    <Routes>
      <Route path="/" element={<Player connectionStatus={wsStatus} />} />
      <Route path="/songs" element={<SongList />} />
    </Routes>
  );
}

export default App;
