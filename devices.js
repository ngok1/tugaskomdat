class DeviceSimulator {
  constructor(devices = []) {
    // device: { id, name, ip, status, bandwidth, lastSeen }
    this.devices = devices.map(d => ({
      ...d,
      status: 'online',
      bandwidth: 0,
      lastSeen: Date.now()
    }));
    this.history = new Map(); // deviceId -> [ { ts, status, bandwidth } ]
    this.logs = []; // { ts, deviceId, message }
    this.onUpdate = null;
    this.onLog = null;
  }

  start(intervalMs = 5000) {
    // initialize history
    this.devices.forEach(d => this._pushHistory(d.id, d.status, d.bandwidth));
    this._timer = setInterval(() => this._tick(), intervalMs);
  }

  stop() {
    clearInterval(this._timer);
  }

  _tick() {
    const updates = [];
    for (const d of this.devices) {
      // status change simulation:
      if (d.status === 'online') {
        if (Math.random() < 0.08) { // 8% chance to go down
          d.status = 'offline';
          d.bandwidth = 0;
          d.lastSeen = Date.now();
          const msg = `${d.name} (${d.ip}) is DOWN`;
          this._pushLog(d.id, msg);
          updates.push({ ...d });
        } else {
          // online: random bandwidth between 50 - 1200 kbps
          d.bandwidth = Math.round(50 + Math.random() * 1150);
          d.lastSeen = Date.now();
          updates.push({ ...d });
        }
      } else { // offline
        if (Math.random() < 0.30) { // 30% chance to come back online
          d.status = 'online';
          d.bandwidth = Math.round(20 + Math.random() * 800);
          d.lastSeen = Date.now();
          const msg = `${d.name} (${d.ip}) is UP`;
          this._pushLog(d.id, msg);
          updates.push({ ...d });
        } else {
          // stay offline
          d.bandwidth = 0;
          d.lastSeen = Date.now();
          updates.push({ ...d });
        }
      }
      this._pushHistory(d.id, d.status, d.bandwidth);
    }

    if (this.onUpdate) {
      // send array of updates
      this.onUpdate(updates);
    }
  }

  _pushHistory(deviceId, status, bandwidth) {
    const ts = Date.now();
    const arr = this.history.get(deviceId) || [];
    arr.push({ ts, status, bandwidth });
    // keep last 1000 entries max
    if (arr.length > 1000) arr.shift();
    this.history.set(deviceId, arr);
  }

  _pushLog(deviceId, message) {
    const entry = { ts: Date.now(), deviceId, message };
    this.logs.unshift(entry); // newest first
    if (this.logs.length > 1000) this.logs.pop();
    if (this.onLog) this.onLog(entry);
  }

  getDevices() {
    return this.devices.map(d => ({ id: d.id, name: d.name, ip: d.ip, status: d.status, bandwidth: d.bandwidth, lastSeen: d.lastSeen }));
  }

  getHistory(deviceId, limit = 200) {
    const arr = this.history.get(deviceId) || [];
    // return last `limit` items (oldest -> newest)
    return arr.slice(Math.max(0, arr.length - limit));
  }

  getLogs(limit = 100) {
    return this.logs.slice(0, limit);
  }
}

module.exports = DeviceSimulator;