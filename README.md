# Simple Web Monitoring
Nama : Wimo Alifansha Wibowo 

NIM : 241091900441

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

Arsitektur (Diagram)
--------------------
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
          GET /api/*     │         │ emits
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