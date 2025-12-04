flowchart LR
  Browser[Browser Client<br/>(HTML + Tailwind + Chart.js)]
  Browser -->|HTTP GET /api/*| Express[Express Server<br/>(static + REST)]
  Browser <-->|Socket.IO| SocketServer[Socket.IO Server]
  Express --> SocketServer
  SocketServer -->|emit events| Browser

  subgraph Backend
    DeviceSim[DeviceSimulator<br/>(simulator tick)]
    History[(In-memory History per device)]
    Logs[(In-memory Logs)]
    DeviceSim --> History
    DeviceSim --> Logs
    DeviceSim -->|onUpdate/onLog| SocketServer
    Express -->|GET /api/history, /api/devices, /api/logs| History
    Express -->|GET /api/logs| Logs
  end