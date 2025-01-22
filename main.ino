#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h> // เพิ่มสำหรับ SPIFFS

// กำหนด SSID และ Password ของ WiFi
const char* ssid = "BP9"; // <-- เปลี่ยนเป็น SSID ของคุณ
const char* password = "aaaaaaaa"; // <-- เปลี่ยนเป็น Password ของคุณ

// กำหนดขาที่ต่อกับเซ็นเซอร์วัดความชื้น
const int sensorPin = A0;  
// กำหนดขาที่ต่อกับ Relay Module
const int relayPin = D1;   
// กำหนดค่าความชื้นที่ต้องการ
const int threshold = 400;  

// สร้าง Web Server object ที่ Port 80
AsyncWebServer server(80);

// ตัวแปรสำหรับเก็บสถานะการเชื่อมต่ออินเทอร์เน็ต
bool internetConnected = false;

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);

  // เชื่อมต่อ WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());

  // ตรวจสอบการเชื่อมต่ออินเทอร์เน็ต
  checkInternetConnection();

  // เริ่ม SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Route สำหรับหน้าแรก
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");  // ส่งชื่อไฟล์ index.html
  });

  // Route สำหรับส่งข้อมูลสถานะ
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->method() == HTTP_GET) {
      int sensorValue = analogRead(sensorPin);
      String pumpState = digitalRead(relayPin) == HIGH ? "ON" : "OFF";
      String internetState = internetConnected ? "Connected" : "Disconnected";
      String json = "{\"moisture\": " + String(sensorValue) + ", \"pumpStatus\": \"" + pumpState + "\", \"internetStatus\": \"" + internetState + "\"}";
      request->send(200, "application/json", json);
    } else {
      request->send(404);  // ส่ง 404 Not Found
    }
  });

  // Route สำหรับควบคุมปั๊มน้ำ
  server.on("/toggle", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->method() == HTTP_POST) {
      digitalWrite(relayPin, !digitalRead(relayPin));
      request->send(200, "text/plain", "OK");
    } else {
      request->send(404);  // ส่ง 404 Not Found
    }
  });

  // เริ่ม Web Server
  server.begin();
}

void loop() {
  // ตรวจสอบค่าความชื้นและควบคุมปั๊ม (เหมือนเดิม)
  int sensorValue = analogRead(sensorPin);  
  if (sensorValue < threshold) {
    digitalWrite(relayPin, HIGH);  
  } else {
    digitalWrite(relayPin, LOW); 
  }

  // ตรวจสอบการเชื่อมต่ออินเทอร์เน็ตทุกๆ 10 วินาที
  if (millis() % 10000 == 0) {
    checkInternetConnection();
  }

  delay(1000); 
}

// ฟังก์ชันสำหรับตรวจสอบการเชื่อมต่ออินเทอร์เน็ต
void checkInternetConnection() {
  if (WiFi.status() == WL_CONNECTED) {
    internetConnected = true;
    Serial.println("Internet connected");
  } else {
    internetConnected = false;
    Serial.println("Internet disconnected");
  }
}
