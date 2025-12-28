# HörBär Remote Control API Specification

This document outlines the proposed API endpoints for the HörBär music player remote control web interface.

## Data Models

### Song
```typescript
{
  id: string;
  title: string;
  artist: string;
  paw: string;      // ID of the paw this song belongs to
  duration: number; // Duration in seconds
}
```

### Paw
```typescript
{
  id: string;
  name: string;
  icon: string;     // Material icon name
  color: string;    // Color identifier (blue, indigo, green, orange)
  songs: Song[];
}
```

### Player State
```typescript
{
  currentSong: Song | null;
  isPlaying: boolean;
  currentTime: number;  // Current playback position in seconds
  volume: number;       // Volume level (0.0 - 1.0)
  shuffle: boolean;
  repeat: boolean;
}
```

## Proposed API Endpoints

### GET /api/paws
Get all paws and their songs.

**Response:**
```json
{
  "paws": [
    {
      "id": "play-paw",
      "name": "Play Paw",
      "icon": "play_arrow",
      "color": "blue",
      "songs": [...]
    },
    ...
  ]
}
```

### GET /api/player/status
Get current player state.

**Response:**
```json
{
  "currentSong": {
    "id": "2",
    "title": "Baby Shark",
    "artist": "Pinkfong",
    "paw": "play-paw",
    "duration": 210
  },
  "isPlaying": true,
  "currentTime": 84,
  "volume": 0.66,
  "shuffle": false,
  "repeat": true
}
```

### POST /api/player/play
Play a specific song or resume playback.

**Request Body (optional):**
```json
{
  "songId": "2"  // If not provided, resume current song
}
```

**Response:**
```json
{
  "success": true,
  "currentSong": {...}
}
```

### POST /api/player/pause
Pause playback.

**Response:**
```json
{
  "success": true
}
```

### POST /api/player/next
Skip to next song in the current paw's playlist.

**Response:**
```json
{
  "success": true,
  "currentSong": {...}
}
```

### POST /api/player/previous
Go to previous song in the current paw's playlist.

**Response:**
```json
{
  "success": true,
  "currentSong": {...}
}
```

### POST /api/player/volume
Set volume level.

**Request Body:**
```json
{
  "volume": 0.75  // 0.0 - 1.0
}
```

**Response:**
```json
{
  "success": true,
  "volume": 0.75
}
```

### POST /api/player/shuffle
Toggle shuffle mode.

**Response:**
```json
{
  "success": true,
  "shuffle": true
}
```

### POST /api/player/repeat
Toggle repeat mode.

**Response:**
```json
{
  "success": true,
  "repeat": false
}
```

### POST /api/player/seek
Seek to a specific position in the current song.

**Request Body:**
```json
{
  "time": 120  // Position in seconds
}
```

**Response:**
```json
{
  "success": true,
  "currentTime": 120
}
```

## WebSocket Events (Optional Enhancement)

For real-time updates, consider implementing WebSocket events:

### Server -> Client Events

- `player:update` - Player state changed
- `song:changed` - Current song changed
- `playback:progress` - Periodic playback position updates

Example payload:
```json
{
  "event": "player:update",
  "data": {
    "isPlaying": true,
    "currentTime": 85
  }
}
```

## Notes

- All endpoints should return appropriate HTTP status codes (200, 400, 404, 500, etc.)
- Consider implementing CORS headers if the web interface is served from a different origin
- Authentication/authorization might be needed depending on your security requirements
- The web interface currently uses mock data stored in Redux - these will need to be replaced with actual API calls
