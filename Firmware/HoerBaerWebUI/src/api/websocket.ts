export interface StateMessage {
  t: "state";
  state: "playing" | "paused" | "idle";
  slot: number | null;
  index: number | null;
  total: number | null;
  duration: number | null;
  currentTime: number | null;
  serial: number | null;
  volume: number;
  maxVolume: number;
  bat?: {
    v: number;
    pct: number;
    chg: boolean;
  };
}

export type WebSocketMessage = StateMessage;

export type ConnectionStatus = "connected" | "disconnected" | "reconnecting";

export class WebSocketService {
  private ws: WebSocket | null = null;
  private reconnectTimeout: NodeJS.Timeout | null = null;
  private heartbeatTimeout: NodeJS.Timeout | null = null;
  private reconnectDelay = 1000;
  private maxReconnectDelay = 30000;
  private heartbeatInterval = 5000; // 5 seconds
  private url: string;
  private onMessageCallback: ((message: WebSocketMessage) => void) | null = null;
  private onStatusChangeCallback: ((status: ConnectionStatus) => void) | null = null;

  constructor(url: string) {
    this.url = url;
  }

  connect(onMessage: (message: WebSocketMessage) => void, onStatusChange?: (status: ConnectionStatus) => void) {
    this.onMessageCallback = onMessage;
    this.onStatusChangeCallback = onStatusChange || null;
    this.createConnection();
  }

  private createConnection() {
    try {
      this.notifyStatusChange("reconnecting");
      this.ws = new WebSocket(this.url);

      this.ws.onopen = () => {
        console.log("WebSocket connected");
        this.reconnectDelay = 1000; // Reset reconnect delay on successful connection
        this.notifyStatusChange("connected");
        this.resetHeartbeat(); // Start heartbeat monitoring
      };

      this.ws.onmessage = (event) => {
        this.resetHeartbeat(); // Reset heartbeat on any message
        try {
          const message = JSON.parse(event.data) as WebSocketMessage;
          if (this.onMessageCallback) {
            this.onMessageCallback(message);
          }
        } catch (error) {
          console.error("Failed to parse WebSocket message:", error);
        }
      };

      this.ws.onerror = (error) => {
        console.error("WebSocket error:", error);
      };

      this.ws.onclose = () => {
        console.log("WebSocket closed, reconnecting...");
        this.clearHeartbeat();
        this.notifyStatusChange("disconnected");
        this.scheduleReconnect();
      };
    } catch (error) {
      console.error("Failed to create WebSocket connection:", error);
      this.notifyStatusChange("disconnected");
      this.scheduleReconnect();
    }
  }

  private resetHeartbeat() {
    this.clearHeartbeat();
    this.heartbeatTimeout = setTimeout(() => {
      console.warn("WebSocket heartbeat timeout - no messages received for 5 seconds");
      this.notifyStatusChange("disconnected");
      // Force close and reconnect
      if (this.ws) {
        this.ws.close();
      }
    }, this.heartbeatInterval);
  }

  private clearHeartbeat() {
    if (this.heartbeatTimeout) {
      clearTimeout(this.heartbeatTimeout);
      this.heartbeatTimeout = null;
    }
  }

  private notifyStatusChange(status: ConnectionStatus) {
    if (this.onStatusChangeCallback) {
      this.onStatusChangeCallback(status);
    }
  }

  private scheduleReconnect() {
    if (this.reconnectTimeout) {
      clearTimeout(this.reconnectTimeout);
    }

    this.reconnectTimeout = setTimeout(() => {
      this.reconnectDelay = Math.min(this.reconnectDelay * 2, this.maxReconnectDelay);
      this.createConnection();
    }, this.reconnectDelay);
  }

  disconnect() {
    if (this.reconnectTimeout) {
      clearTimeout(this.reconnectTimeout);
      this.reconnectTimeout = null;
    }

    this.clearHeartbeat();

    if (this.ws) {
      this.ws.close();
      this.ws = null;
    }
  }

  send(message: any) {
    if (this.ws && this.ws.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(message));
    } else {
      console.warn("WebSocket is not connected, cannot send message");
    }
  }
}

export const createWebSocketService = () => {
  const wsUrl = import.meta.env.DEV ? "ws://192.168.15.164/ws" : `ws://${window.location.host}/ws`;
  return new WebSocketService(wsUrl);
};

// Singleton instance for sending commands
let wsInstance: WebSocketService | null = null;

export const setWebSocketInstance = (instance: WebSocketService) => {
  wsInstance = instance;
};

export const sendCommand = (cmd: "play" | "pause" | "next" | "previous") => {
  if (wsInstance) {
    wsInstance.send({ t: "cmd", cmd });
  } else {
    console.warn("WebSocket instance not initialized");
  }
};

export const sendPlaySlot = (slot: number, index: number) => {
  if (wsInstance) {
    wsInstance.send({ t: "cmd", cmd: "playSlot", slot, index });
  } else {
    console.warn("WebSocket instance not initialized");
  }
};
