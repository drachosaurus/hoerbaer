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
    // If already connected or connecting, just update callbacks
    if (this.ws && (this.ws.readyState === WebSocket.OPEN || this.ws.readyState === WebSocket.CONNECTING)) {
      console.log("WebSocket already connected/connecting, updating callbacks only");
      this.onMessageCallback = onMessage;
      this.onStatusChangeCallback = onStatusChange || null;
      return;
    }
    
    console.log("Starting new WebSocket connection");
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
          console.error("Raw message data:", event.data);
          // Ignore non-JSON messages (e.g., connection acknowledgments)
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
      try {
        const jsonString = JSON.stringify(message);
        console.log("Sending WebSocket message:", jsonString);
        this.ws.send(jsonString);
      } catch (error) {
        console.error("Failed to send WebSocket message:", error, message);
      }
    } else {
      console.warn("WebSocket is not connected, cannot send message. ReadyState:", this.ws?.readyState);
      console.warn("WebSocket states - CONNECTING:", WebSocket.CONNECTING, "OPEN:", WebSocket.OPEN, "CLOSING:", WebSocket.CLOSING, "CLOSED:", WebSocket.CLOSED);
    }
  }
}

// Singleton instance for sending commands and connection management
let wsInstance: WebSocketService | null = null;

export const createWebSocketService = () => {
  // Return existing instance if already created (prevents duplicate connections in React Strict Mode)
  if (wsInstance) {
    console.log("Reusing existing WebSocket instance");
    return wsInstance;
  }
  
  console.log("Creating new WebSocket instance");
  const wsUrl = import.meta.env.DEV ? "ws://192.168.15.164/ws" : `ws://${window.location.host}/ws`;
  const instance = new WebSocketService(wsUrl);
  wsInstance = instance;
  return instance;
};

export const setWebSocketInstance = (instance: WebSocketService | null) => {
  wsInstance = instance;
};

// Commands are now sent via REST API instead of WebSocket
// Import and use the deviceApi mutation in components instead
// These functions are kept for backward compatibility but will use fetch
const sendCommandViaREST = async (command: any) => {
  const baseUrl = import.meta.env.DEV ? "http://192.168.15.164/api" : "/api";
  try {
    const response = await fetch(`${baseUrl}/cmd`, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(command),
    });
    if (!response.ok) {
      console.error("Command failed:", response.status, response.statusText);
    }
  } catch (error) {
    console.error("Failed to send command:", error);
  }
};

export const sendCommand = (cmd: "play" | "pause" | "next" | "previous") => {
  sendCommandViaREST({ cmd });
};

export const sendPlaySlot = (slot: number, index: number) => {
  sendCommandViaREST({ cmd: "playSlot", slot, index });
};

export const sendSetVolume = (volume: number) => {
  sendCommandViaREST({ cmd: "setVol", volume });
};
