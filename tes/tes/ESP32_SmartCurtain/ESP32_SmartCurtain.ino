/*
==================================================
 ESP32 SMART CURTAIN MQTT
 Multi Client Local IoT System

 Broker:
 Mosquitto MQTT

 Web:
 ESP32 LittleFS

 Feature:
 - Multi device control
 - MQTT
 - Broker IP setting
 - Saved configuration
 - Dashboard
 - Control page
 - Status animation

==================================================
*/


#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <ArduinoJson.h>

#include "config.h"



// ================================
// WIFI
// ================================


const char* ssid =
WIFI_SSID;


const char* password =
WIFI_PASSWORD;



// ================================
// OBJECT
// ================================


WiFiClient wifiClient;


PubSubClient mqtt(
wifiClient
);


WebServer server(80);


Preferences preferences;





// ================================
// MQTT CONFIG
// ================================


String mqttIP;


#define MQTT_PORT 1883



String mqttStatus =
"Disconnected";





// ================================
// DEVICE DATABASE
// ================================


struct Curtain
{

String id;

String name;

int position;

String mode;

String state;

};





Curtain curtain[3]=
{


{
"curtain01",
"Living Room",
0,
"manual",
"CLOSED"
},



{
"curtain02",
"Bedroom",
50,
"auto",
"HALF OPEN"
},



{
"curtain03",
"Office",
100,
"schedule",
"FULL OPEN"
}


};






unsigned long lastPublish=0;

unsigned long bootTime;

// ==================================================
// LOAD MQTT BROKER IP
// ==================================================


void loadMQTTConfig()
{

preferences.begin(
"mqtt",
false
);


// ambil IP tersimpan
mqttIP =
preferences.getString(
"broker",
MQTT_DEFAULT_IP
);


preferences.end();



Serial.print(
"MQTT Broker : "
);


Serial.println(
mqttIP
);



}




// ==================================================
// SAVE MQTT BROKER IP
// ==================================================


void saveMQTTConfig(
String ip
)
{


preferences.begin(
"mqtt",
false
);



preferences.putString(
"broker",
ip
);



preferences.end();



mqttIP = ip;



Serial.println(
"New MQTT Broker:"
);


Serial.println(
mqttIP
);



}






// ==================================================
// UPDATE CURTAIN STATE
// ==================================================


void updateState(
int index
)
{


int pos =
curtain[index].position;



if(pos<=0)
{

curtain[index].state =
"CLOSED";

}

else if(pos<50)
{

curtain[index].state =
"SLIGHT OPEN";

}

else if(pos<100)
{

curtain[index].state =
"HALF OPEN";

}

else
{

curtain[index].state =
"FULL OPEN";

}


}









// ==================================================
// MQTT CALLBACK
// Format:
// curtain01,75,manual
//
// ==================================================


void mqttCallback(
char* topic,
byte* payload,
unsigned int length
)
{


String message;



for(
int i=0;
i<length;
i++
)
{

message +=
(char)payload[i];

}



Serial.println(
"MQTT RX:"
);


Serial.println(
message
);




int p1 =
message.indexOf(",");



int p2 =
message.indexOf(
",",
p1+1
);



if(
p1<0 ||
p2<0
)
return;




String id =
message.substring(
0,
p1
);



int position =
message.substring(
p1+1,
p2
).toInt();



String mode =
message.substring(
p2+1
);





for(
int i=0;
i<3;
i++
)
{


if(
curtain[i].id ==
id
)
{


curtain[i].position =
constrain(
position,
0,
100
);



curtain[i].mode =
mode;



updateState(i);



}

}


}









// ==================================================
// MQTT CONNECT
// ==================================================


void connectMQTT()
{


mqtt.setServer(
mqttIP.c_str(),
MQTT_PORT
);



while(
!mqtt.connected()
)
{


Serial.println(
"Connecting MQTT..."
);



if(
mqtt.connect(
"ESP32_SMART_CURTAIN"
)
)
{


Serial.println(
"MQTT Connected"
);



mqttStatus =
"Connected";



// menerima semua device


mqtt.subscribe(
"smartcurtain/+/control"
);



}

else
{


mqttStatus =
"Disconnected";


Serial.println(
"Retry MQTT..."
);



delay(
3000
);



}



}


}









// ==================================================
// PUBLISH STATUS
// ==================================================


void publishStatus()
{


StaticJsonDocument<1500> doc;



doc["mqtt"] =
mqttStatus;



doc["wifi"] =
WiFi.status()==WL_CONNECTED ?
"Connected":
"Disconnected";



doc["ip"] =
WiFi.localIP().toString();



doc["broker"] =
mqttIP;




JsonObject weather =
doc.createNestedObject(
"weather"
);



weather["temperature"]=30;

weather["humidity"]=65;

weather["condition"]="Sunny";





JsonArray devices =
doc.createNestedArray(
"curtain"
);



for(
int i=0;
i<3;
i++
)
{


JsonObject item =
devices.createNestedObject();



item["id"] =
curtain[i].id;


item["name"] =
curtain[i].name;


item["position"] =
curtain[i].position;


item["mode"] =
curtain[i].mode;


item["state"] =
curtain[i].state;



}



String output;



serializeJson(
doc,
output
);



mqtt.publish(
"smartcurtain/status",
output.c_str()
);



}

// ==================================================
// SEND FILE FROM LITTLEFS
// ==================================================


void sendFile(
String path
)
{


if(
!LittleFS.exists(path)
)
{

server.send(
404,
"text/plain",
"File Not Found"
);


return;

}



File file =
LittleFS.open(
path,
"r"
);



server.streamFile(
file,
"text/html"
);



file.close();


}









// ==================================================
// API STATUS
// Dipakai semua halaman UI
// ==================================================


void apiStatus()
{


StaticJsonDocument<2000> doc;



doc["wifi"] =
WiFi.status()==WL_CONNECTED ?
"Connected":
"Disconnected";



doc["mqtt"] =
mqttStatus;



doc["broker"] =
mqttIP;



doc["ip"] =
WiFi.localIP().toString();



doc["uptime"] =
String(
(millis()-bootTime)/1000
)
+
" sec";





JsonObject weather =
doc.createNestedObject(
"weather"
);



weather["temperature"]=30;

weather["humidity"]=65;

weather["condition"]="Sunny";





JsonArray list =
doc.createNestedArray(
"curtain"
);




for(
int i=0;
i<3;
i++
)
{


JsonObject item =
list.createNestedObject();



item["id"] =
curtain[i].id;


item["name"] =
curtain[i].name;


item["position"] =
curtain[i].position;


item["mode"] =
curtain[i].mode;


item["state"] =
curtain[i].state;



}





String response;



serializeJson(
doc,
response
);



server.send(
200,
"application/json",
response
);



}









// ==================================================
// API CONTROL
//
// Contoh:
// /api/control?
// device=curtain01
// position=75
// mode=auto
//
// ==================================================


void apiControl()
{


if(
!server.hasArg("device") ||
!server.hasArg("position") ||
!server.hasArg("mode")
)
{


server.send(
400,
"text/plain",
"Parameter Missing"
);



return;

}




String device =
server.arg(
"device"
);



String position =
server.arg(
"position"
);



String mode =
server.arg(
"mode"
);




String payload =
device
+
","
+
position
+
","
+
mode;





String topic =
"smartcurtain/"
+
device
+
"/control";





mqtt.publish(
topic.c_str(),
payload.c_str()
);





server.send(
200,
"text/plain",
"Command Sent"
);



}









// ==================================================
// API CONFIG MQTT BROKER
//
// Contoh:
// /api/config?
// broker=192.168.1.50
//
// ==================================================


void apiConfig()
{


if(
!server.hasArg("broker")
)
{


server.send(
400,
"text/plain",
"Broker IP Missing"
);


return;

}



String newIP =
server.arg(
"broker"
);



saveMQTTConfig(
newIP
);




// putus koneksi lama

mqtt.disconnect();



// reconnect dengan IP baru

connectMQTT();



server.send(
200,
"text/plain",
"Broker Updated"
);



}









// ==================================================
// SETUP WEB ROUTE
// ==================================================


void setupWebServer()
{



// Dashboard

server.on(
"/",
HTTP_GET,
[]()
{

sendFile(
"/index.html"
);

}

);





// Control


server.on(
"/control.html",
HTTP_GET,
[]()
{

sendFile(
"/control.html"
);

}

);






// Status


server.on(
"/status.html",
HTTP_GET,
[]()
{

sendFile(
"/status.html"
);

}

);





// API


server.on(
"/api/status",
HTTP_GET,
apiStatus
);



server.on(
"/api/control",
HTTP_GET,
apiControl
);



server.on(
"/api/config",
HTTP_GET,
apiConfig
);





server.begin();



Serial.println(
"Web Server Ready"
);



}

// ==================================================
// SETUP
// ==================================================


void setup()
{


Serial.begin(
115200
);



bootTime =
millis();




// --------------------------
// LittleFS
// --------------------------


if(
!LittleFS.begin(true)
)
{

Serial.println(
"LittleFS Failed"
);

}

else
{

Serial.println(
"LittleFS Ready"
);

}







// --------------------------
// MQTT CONFIG
// --------------------------


loadMQTTConfig();






// --------------------------
// WIFI
// --------------------------


WiFi.begin(
ssid,
password
);



Serial.print(
"Connecting WiFi"
);



while(
WiFi.status()!=WL_CONNECTED
)
{

delay(500);

Serial.print(
"."
);

}



Serial.println();

Serial.println(
"WiFi Connected"
);



Serial.print(
"ESP32 IP : "
);



Serial.println(
WiFi.localIP()
);








// --------------------------
// MQTT
// --------------------------


mqtt.setCallback(
mqttCallback
);



connectMQTT();






// --------------------------
// WEB SERVER
// --------------------------


setupWebServer();



}









// ==================================================
// LOOP
// ==================================================


void loop()
{


// Web request

server.handleClient();





// MQTT

if(
!mqtt.connected()
)
{


connectMQTT();


}



mqtt.loop();






// Publish status setiap 3 detik


if(
millis()-lastPublish > 3000
)
{


publishStatus();


lastPublish =
millis();


}



}