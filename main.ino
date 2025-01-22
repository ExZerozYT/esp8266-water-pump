#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

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

  // Route สำหรับหน้าแรก
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "<!DOCTYPE html><html><head><title>ESP8266 Water Pump Control</title></head><body><h1>ESP8266 Water Pump Control</h1><p>Internet: <span id='internetStatus'>Loading...</span></p><p>Moisture: <span id='moisture'>Loading...</span></p><p>Pump Status: <span id='pumpStatus'>Loading...</span></p><button onclick='togglePump()'>Toggle Pump</button><script>setInterval(updateStatus, 1000);function updateStatus() {fetch('/status').then(response => response.json()).then(data => {document.getElementById('internetStatus').textContent = data.internetStatus;document.getElementById('moisture').textContent = data.moisture;document.getElementById('pumpStatus').textContent = data.pumpStatus;});}function togglePump() {fetch('/toggle', {method: 'POST'});}</script></body></html>");
  });

  // Route สำหรับส่งข้อมูลสถานะ
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    int sensorValue = analogRead(sensorPin);
    String pumpState = digitalRead(relayPin) == HIGH ? "ON" : "OFF";
    String internetState = internetConnected ? "Connected" : "Disconnected";
    String json = "{\"moisture\": " + String(sensorValue) + ", \"pumpStatus\": \"" + pumpState + "\", \"internetStatus\": \"" + internetState + "\"}";
    request->send(200, "application/json", json);
  });

  // Route สำหรับควบคุมปั๊มน้ำ
  server.on("/toggle", HTTP_POST, [](AsyncWebServerRequest *request){
    digitalWrite(relayPin, !digitalRead(relayPin));
    request->send(200, "text/plain", "OK");
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
  IPAddress remote_ip; // ประกาศตัวแปร IPAddress
  // พยายามเชื่อมต่อกับ Google DNS server
  if (WiFi.hostByName("8.8.8.8", remote_ip) == 1) {  // ใช้ remote_ip ใน hostByName
    internetConnected = true;
    Serial.println("Internet connected");
  } else {
    internetConnected = false;
    Serial.println("Internet disconnected");
  }
}
