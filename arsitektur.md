### Arsitektur (diagram)

[Browser Client]
  - UI: HTML + Tailwind + Chart.js
  - HTTP -> /api/devices, /api/history, /api/logs
  - Socket.IO <--> Server (realtime updates)

            ┌──────────────────────────────┐
            │     Express + Socket.IO      │
            │  - Serves static frontend    │
            │  - REST endpoints (history)  │
            │  - Broadcasts via Socket.IO  │
            └────────────┬─────────┬───────┘
                         │         │
          GET /api/*    │         │ emits
                         │         ▼
                 ┌───────┴────────────┐
                 │  DeviceSimulator    │
                 │  - tick every N s   │
                 │  - generates status │
                 │  - stores history   │
                 └───────┬────────────┘
                         │
          in-memory      │
         history/logs    ▼
                ┌────────────────┐
                │  In-memory DB  │
                │  (history,logs)│
                └────────────────┘