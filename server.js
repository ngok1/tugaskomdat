const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const DeviceSimulator = require('./devices');

const app = express();
const server = http.createServer(app);
const io = new Server(server);

app.use(express.json());
app.use(express.static('public'));

// initial 3 simulated devices
const initialDevices = [
  { id: 'dev-1', name: 'Wimo', ip: '192.168.1.1' },
  { id: 'dev-2', name: 'Alifansha', ip: '192.168.1.2' },
  { id: 'dev-3', name: 'Wibowo', ip: '192.168.1.3' }
];

const simulator = new DeviceSimulator(initialDevices);

// REST endpoints
app.get('/api/devices', (req, res) => {
  res.json(simulator.getDevices());
});

app.get('/api/history', (req, res) => {
  const deviceId = req.query.deviceId;
  const limit = parseInt(req.query.limit || '200', 10);
  if (!deviceId) return res.status(400).json({ error: 'deviceId required' });
  res.json(simulator.getHistory(deviceId, limit));
});

app.get('/api/logs', (req, res) => {
  const limit = parseInt(req.query.limit || '100', 10);
  res.json(simulator.getLogs(limit));
});

// Socket.IO realtime
io.on('connection', (socket) => {
  console.log('client connected', socket.id);

  // optional: emit current devices on connect
  socket.emit('devices:init', simulator.getDevices());
  socket.emit('logs:init', simulator.getLogs(50));

  socket.on('disconnect', () => {
    console.log('client disconnected', socket.id);
  });
});

// When simulator has updates, broadcast
simulator.onUpdate = (update) => {
  // update can be single device object or array
  io.emit('device:update', update);
};

simulator.onLog = (logEntry) => {
  io.emit('device:log', logEntry);
};

// start polling
simulator.start(5000); // poll every 5s

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
  console.log(`Server listening on http://localhost:${PORT}`);
});