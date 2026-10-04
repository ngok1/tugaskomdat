# Smart Curtain MQTT ESP32

Catatan Konfigurasi Singkat

## Deskripsi

Simulasi Smart Curtain berbasis ESP32 dengan MQTT Mosquitto lokal.
ESP32 berfungsi sebagai controller virtual curtain dan menyediakan Web UI melalui LittleFS.

Fitur:

* MQTT local network
* Multi device client (HP/Laptop/Tablet)
* Multi curtain virtual
* Dashboard, Control, Status
* Kontrol posisi 0-100%
* Mode Manual, Auto, Schedule, Sleep
* Konfigurasi broker MQTT melalui web

---

# Hardware

* ESP32 Dev Module
* PC/Laptop/Raspberry Pi sebagai MQTT Broker
* Router WiFi lokal

---

# Software & Library

## Arduino IDE

Board:

* ESP32 by Espressif Systems

## Library tambahan:

* PubSubClient
* ArduinoJson

## Library bawaan ESP32:

* WiFi
* WebServer
* LittleFS
* Preferences

---

# Struktur Folder

SmartCurtain_MQTT

```
├── ESP32_SmartCurtain
│   ├── ESP32_SmartCurtain.ino
│   └── config.h
│
└── data
    ├── index.html
    ├── control.html
    └── status.html
```

---

# Konfigurasi Awal

Edit file:

`config.h`

Isi:

```
WIFI_SSID
WIFI_PASSWORD
MQTT_DEFAULT_IP
```

Contoh:

```
WiFi:
Rumah

Password:
12345678

MQTT Broker:
192.168.1.10
```

---

# Pengaturan Jaringan

Semua perangkat harus berada dalam jaringan lokal yang sama:

```
ESP32
 |
Router WiFi
 |
----------------
|              |
HP/Laptop    MQTT Broker
```

Contoh IP:

```
Router:
192.168.1.1

ESP32:
192.168.1.20

Mosquitto:
192.168.1.10
```

---

# MQTT Broker

Menggunakan:
Mosquitto MQTT

Port default:

```
1883
```

Pastikan Mosquitto menerima koneksi LAN.

Topic utama:

Status:

```
smartcurtain/status
```

Control:

```
smartcurtain/curtain01/control
smartcurtain/curtain02/control
smartcurtain/curtain03/control
```

Format command:

```
device,position,mode
```

Contoh:

```
curtain01,75,auto
```

---

# Upload ESP32

Urutan:

1. Upload sketch:

```
ESP32_SmartCurtain.ino
```

2. Upload filesystem:

```
Tools
→ ESP32 Sketch Data Upload
```

Folder `data` harus sejajar dengan folder sketch.

---

# Akses Web UI

Setelah ESP32 terhubung:

Buka:

```
http://IP_ESP32
```

Contoh:

```
http://192.168.1.20
```

---

# Troubleshooting Singkat

MQTT tidak connect:

* cek IP broker
* cek Mosquitto aktif
* cek port 1883
* cek firewall komputer

Web tidak tampil:

* cek upload LittleFS
* cek folder `data`

ESP32 tidak terhubung:

* cek SSID/password
* cek jarak WiFi
* cek jaringan lokal

---

# Catatan

IP MQTT dapat diganti melalui Web UI tanpa upload ulang firmware.
ESP32 menyimpan konfigurasi broker menggunakan Preferences.
