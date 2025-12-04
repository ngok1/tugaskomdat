const socket = io();

// DOM refs
const devicesBody = document.getElementById('devices-body');
const selectedDeviceLabel = document.getElementById('selected-device');
const logsDiv = document.getElementById('logs');

let devices = [];
let selectedDeviceId = null;
let chart = null;

// Chart setup
function createChart(labels = [], data = []) {
  const ctx = document.getElementById('bandwidthChart').getContext('2d');
  if (chart) chart.destroy();
  chart = new Chart(ctx, {
    type: 'line',
    data: {
      labels,
      datasets: [{
        label: 'Bandwidth (kbps)',
        data,
        fill: false,
        borderColor: 'rgb(59 130 246)',
        tension: 0.2
      }]
    },
    options: {
      responsive: true,
      scales: {
        x: { display: true },
        y: { beginAtZero: true }
      }
    }
  });
}

// render devices table
function renderDevices() {
  devicesBody.innerHTML = '';
  devices.forEach(d => {
    const tr = document.createElement('tr');
    tr.className = 'cursor-pointer hover:bg-gray-50';
    tr.onclick = () => selectDevice(d.id);
    tr.innerHTML = `
      <td class="py-2">${d.name}</td>
      <td class="py-2 text-gray-600">${d.ip}</td>
      <td class="py-2"><span class="px-2 py-1 rounded text-xs ${d.status==='online' ? 'bg-green-100 text-green-800' : 'bg-red-100 text-red-800'}">${d.status}</span></td>
    `;
    devicesBody.appendChild(tr);
  });
}

// select device and load history
async function selectDevice(id) {
  selectedDeviceId = id;
  const d = devices.find(x => x.id === id);
  selectedDeviceLabel.textContent = d ? `${d.name} (${d.ip})` : id;
  // fetch history
  const res = await fetch(`/api/history?deviceId=${encodeURIComponent(id)}&limit=100`);
  const hist = await res.json();
  const labels = hist.map(h => new Date(h.ts).toLocaleTimeString());
  const data = hist.map(h => h.bandwidth);
  createChart(labels, data);
}

// add log entry to UI (newest on top)
function addLog(entry) {
  const el = document.createElement('div');
  const time = new Date(entry.ts).toLocaleTimeString();
  el.innerHTML = `<div class="border-b py-1"><span class="text-xs text-gray-500 mr-2">${time}</span><span class="text-sm">${entry.message}</span></div>`;
  logsDiv.prepend(el);
}

// update devices on realtime message
socket.on('device:update', (payload) => {
  const updates = Array.isArray(payload) ? payload : [payload];
  let chartNeedsUpdate = false;
  updates.forEach(u => {
    const idx = devices.findIndex(d => d.id === u.id);
    if (idx >= 0) {
      devices[idx].status = u.status;
      devices[idx].bandwidth = u.bandwidth;
      devices[idx].lastSeen = u.lastSeen;
    } else {
      devices.push(u);
    }

    // if update for selected device, append to chart
    if (selectedDeviceId && u.id === selectedDeviceId && chart) {
      chart.data.labels.push(new Date(u.lastSeen).toLocaleTimeString());
      chart.data.datasets[0].data.push(u.bandwidth);
      // keep last 50 points
      if (chart.data.labels.length > 50) {
        chart.data.labels.shift();
        chart.data.datasets[0].data.shift();
      }
      chartNeedsUpdate = true;
    }
  });
  renderDevices();
  if (chartNeedsUpdate) chart.update();
});

// logs
socket.on('device:log', (entry) => {
  addLog(entry);
});

// init with server state
socket.on('devices:init', (list) => {
  devices = list;
  renderDevices();
});

socket.on('logs:init', (logs) => {
  logs.reverse().forEach(addLog);
});

// initial fetch (in case)
(async function init() {
  try {
    const res = await fetch('/api/devices');
    devices = await res.json();
    renderDevices();
    if (devices.length > 0) selectDevice(devices[0].id);

    const r2 = await fetch('/api/logs?limit=50');
    const logs = await r2.json();
    logs.reverse().forEach(addLog);
  } catch (err) {
    console.error('Init error', err);
  }
})();