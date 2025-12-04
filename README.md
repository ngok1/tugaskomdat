# Simple Web Monitoring

Deskripsi
--------
Proyek sederhana untuk monitoring 3 perangkat (simulasi) dengan update realtime (WebSocket) dan endpoint REST untuk data historis.  

Requirement
-----------
- Node.js v14+ (direkomendasikan v16/v18)
- npm

Instal dan jalankan
-------------------
1. Simpan semua file sesuai struktur:
   - package.json
   - server.js
   - devices.js
   - README.md
   - public/
     - index.html
     - main.js

2. Install dependensi:
   npm install

3. Jalankan server:
   npm start

4. Buka browser:
   http://localhost:3000

-----------
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