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
}

export type WebSocketMessage = StateMessage;

export class WebSocketService {
  private ws: WebSocket | null = null;
  private reconnectTimeout: NodeJS.Timeout | null = null;
  private reconnectDelay = 1000;
  private maxReconnectDelay = 30000;
  private url: string;
  private onMessageCallback: ((message: WebSocketMessage) => void) | null = null;

  constructor(url: string) {
    this.url = url;
  }

  connect(onMessage: (message: WebSocketMessage) => void) {
    this.onMessageCallback = onMessage;
    this.createConnection();
  }

  private createConnection() {
    try {
      this.ws = new WebSocket(this.url);

      this.ws.onopen = () => {
        console.log("WebSocket connected");
        this.reconnectDelay = 1000; // Reset reconnect delay on successful connection
      };

      this.ws.onmessage = (event) => {
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
        this.scheduleReconnect();
      };
    } catch (error) {
      console.error("Failed to create WebSocket connection:", error);
      this.scheduleReconnect();
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
