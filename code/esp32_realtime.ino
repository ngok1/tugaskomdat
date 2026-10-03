#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ===== WiFi =====
const char* SSID = "NAMA_WIFI_ANDA";
const char* PASSWORD = "PASSWORD_WIFI";

// ===== MQTT Broker =====
const char* MQTT_SERVER = "192.168.1.10"; // GANTI IP BROKER MQTT DISINI
const int MQTT_PORT = 1883;
const char* MQTT_USER = "";
const char* MQTT_PASS = "";

// ===== MQTT Topics =====
const char* TOPIC_COMMAND = "home/curtain/command";
const char* TOPIC_STATUS = "home/curtain/status";

WebServer server(80);
WiFiClient espClient;
PubSubClient mqtt(espClient);

struct RoomState {
  String name;
  String mode;
  int position;
  int target;
  bool moving;
  bool sensor_open;
  bool sensor_closed;
};

RoomState rooms[] = {
  {"Living Room", "OPEN", 72, 100, false, false, false},
  {"Bedroom", "MORNING", 50, 50, false, false, false},
  {"Kitchen", "AFTERNOON", 30, 30, false, false, false},
  {"Dining", "OPEN", 100, 100, false, true, false},
  {"Study", "EVENING", 10, 10, false, false, false}
};

const int ROOM_COUNT = 5;

String getRoomModeLabel(String mode) {
  mode.toUpperCase();
  if (mode == "OPEN") return "OPEN";
  if (mode == "CLOSED") return "CLOSED";
  if (mode == "MORNING") return "MORNING";
  if (mode == "AFTERNOON") return "AFTERNOON";
  if (mode == "EVENING") return "EVENING";
  if (mode == "CUSTOM") return "CUSTOM";
  return "UNKNOWN";
}

int getTargetByMode(String mode) {
  if (mode == "OPEN") return 100;
  if (mode == "CLOSED") return 0;
  if (mode == "MORNING") return 50;
  if (mode == "AFTERNOON") return 30;
  if (mode == "EVENING") return 10;
  if (mode == "CUSTOM") return 50;
  return 0;
}

void applyCommandToRoom(String roomName, String command, int customPosition = -1) {
  for (int i = 0; i < ROOM_COUNT; i++) {
    if (rooms[i].name == roomName) {
      if (command == "OPEN") {
        rooms[i].mode = "OPEN";
        rooms[i].target = 100;
        rooms[i].moving = true;
      }
      else if (command == "CLOSED") {
        rooms[i].mode = "CLOSED";
        rooms[i].target = 0;
        rooms[i].moving = true;
      }
      else if (command == "MORNING") {
        rooms[i].mode = "MORNING";
        rooms[i].target = 50;
        rooms[i].moving = true;
      }
      else if (command == "AFTERNOON") {
        rooms[i].mode = "AFTERNOON";
        rooms[i].target = 30;
        rooms[i].moving = true;
      }
      else if (command == "EVENING") {
        rooms[i].mode = "EVENING";
        rooms[i].target = 10;
        rooms[i].moving = true;
      }
      else if (command == "CUSTOM") {
        rooms[i].mode = "CUSTOM";
        rooms[i].target = customPosition;
        rooms[i].moving = true;
      }
      else if (command == "STOP") {
        rooms[i].moving = false;
      }

      break;
    }
  }
}

void updatePositions() {
  for (int i = 0; i < ROOM_COUNT; i++) {
    if (rooms[i].moving) {
      int diff = rooms[i].target - rooms[i].position;
      if (abs(diff) > 0) {
        rooms[i].position += (diff > 0) ? 2 : -2;
        if (abs(rooms[i].target - rooms[i].position) <= 2) {
          rooms[i].position = rooms[i].target;
          rooms[i].moving = false;
        }
      }
    }

    rooms[i].sensor_open = rooms[i].position >= 95;
    rooms[i].sensor_closed = rooms[i].position <= 5;
  }
}

String buildStatusJson() {
  StaticJsonDocument<2048> doc;
  doc["device"] = "ESP32-Smart-Curtain";
  doc["ip"] = WiFi.localIP().toString();
  doc["rssi"] = WiFi.RSSI();
  doc["mqtt_connected"] = mqtt.connected();

  JsonArray arr = doc.createNestedArray("rooms");

  for (int i = 0; i < ROOM_COUNT; i++) {
    JsonObject r = arr.createNestedObject();
    r["name"] = rooms[i].name;
    r["mode"] = rooms[i].mode;
    r["position"] = rooms[i].position;
    r["target"] = rooms[i].target;
    r["moving"] = rooms[i].moving;
    r["sensor_open"] = rooms[i].sensor_open;
    r["sensor_closed"] = rooms[i].sensor_closed;
  }

  String output;
  serializeJson(doc, output);
  return output;
}

void publishStatusToBroker() {
  if (!mqtt.connected()) return;

  String payload = buildStatusJson();
  mqtt.publish(TOPIC_STATUS, payload.c_str());
}

void handleApiStatus() {
  server.send(200, "application/json", buildStatusJson());
}

void handleApiCommand() {
  if (server.hasArg("plain") == false) {
    server.send(400, "application/json", "{\"error\":\"No body\"}");
    return;
  }

  String body = server.arg("plain");
  Serial.println(body);

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, body);

  if (err) {
    server.send(400, "application/json", "{\"error\":\"invalid json\"}");
    return;
  }

  String room = doc["room"] | "Living Room";
  String command = doc["command"] | "STOP";
  int customPosition = doc["position"] | -1;

  if (command == "CUSTOM" && customPosition >= 0) {
    applyCommandToRoom(room, "CUSTOM", customPosition);
  } else {
    applyCommandToRoom(room, command, customPosition);
  }

  publishStatusToBroker();
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.print("MQTT RX: ");
  Serial.print(topic);
  Serial.print(" => ");
  Serial.println(msg);

  if (String(topic) == TOPIC_COMMAND) {
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, msg);

    if (err) {
      Serial.println("Invalid MQTT command");
      return;
    }

    String room = doc["room"] | "Living Room";
    String command = doc["command"] | "STOP";
    int customPosition = doc["position"] | -1;

    if (command == "CUSTOM" && customPosition >= 0) {
      applyCommandToRoom(room, "CUSTOM", customPosition);
    } else {
      applyCommandToRoom(room, command, customPosition);
    }

    publishStatusToBroker();
  }
}

void reconnectMqtt() {
  while (!mqtt.connected()) {
    Serial.print("Connecting MQTT...");
    String clientId = "ESP32-Curtain-";
    clientId += String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println(" OK");
      mqtt.subscribe(TOPIC_COMMAND);
    } else {
      Serial.print(" failed, rc=");
      Serial.println(mqtt.state());
      delay(3000);
    }
  }
}

void setupWebServer() {
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/plain", "ESP32 Smart Curtain API");
  });

  server.on("/api/status", HTTP_GET, handleApiStatus);
  server.on("/api/command", HTTP_POST, handleApiCommand);

  server.begin();
  Serial.println("WebServer started");
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  Serial.println("Connecting WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);

  setupWebServer();
}

void loop() {
  if (!mqtt.connected()) reconnectMqtt();
  mqtt.loop();

  updatePositions();
  server.handleClient();

  static unsigned long lastPublish = 0;
  if (millis() - lastPublish > 5000) {
    publishStatusToBroker();
    lastPublish = millis();
  }
}
