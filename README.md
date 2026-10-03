# Smart Curtain Control

Smart Curtain Control adalah sistem kontrol tirai otomatis berbasis ESP32 yang dapat diakses melalui web dashboard. Sistem ini mendukung:
- Dashboard tampilan posisi tiap ruangan
- Control mode: OPEN, CLOSED, MORNING, AFTERNOON, EVENING, CUSTOM
- Status tiap room/ruangan
- Koneksi ke broker MQTT
- API HTTP untuk frontend agar dapat berkomunikasi dengan ESP32

Struktur project:
- index.html
- dashboard.html
- control.html
- status.html
- esp32_realtime.ino

# Spesifikasi
- ESP32
- WiFi
- MQTT Broker (contoh: Mosquitto)
- Browser modern (Chrome, Edge, Firefox)

# Fitur
- Monitoring posisi tirai tiap ruangan
- Pilih ruangan/area
- Mode preset: OPEN, CLOSED, MORNING, AFTERNOON, EVENING
- Custom position 0-100%
- Status koneksi perangkat/server
- Preview visual tirai pada halaman status
- API HTTP:
  - /api/status
  - /api/command

# Struktur File
project/
├── index.html
├── dashboard.html
├── control.html
├── status.html
├── esp32_realtime.ino
└── README.md

# Persiapan
1. Install Arduino IDE
2. Install library berikut di Arduino IDE:
   - PubSubClient
   - ArduinoJson

3. Pastikan broker MQTT sudah aktif, misalnya Mosquitto.

4. Ubah konfigurasi WiFi dan MQTT pada file:
   esp32_realtime.ino

Ubah bagian ini:

```cpp
const char* SSID = "NAMA_WIFI_ANDA";
const char* PASSWORD = "PASSWORD_WIFI";

const char* MQTT_SERVER = "192.168.1.10";
const int MQTT_PORT = 1883;
const char* MQTT_USER = "";
const char* MQTT_PASS = "";