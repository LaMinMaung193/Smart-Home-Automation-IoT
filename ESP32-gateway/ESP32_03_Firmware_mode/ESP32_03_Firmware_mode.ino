#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ModbusMaster.h>

// ================= WIFI =================
const char* ssid = "A";
const char* password = "44444444";

// ================= MQTT =================
const char* mqtt_server = "broker.hivemq.com";
const char* statusTopic  = "smarthome/plc1/status";
const char* controlTopic = "smarthome/plc1/control";

// ================= RS485 =================
#define RXD2 16
#define TXD2 17
#define MAX485_DE 4

ModbusMaster node;
WiFiClient espClient;
PubSubClient client(espClient);

// ================= PLC DATA =================
uint16_t temperature_raw = 0;
uint16_t heartbeat = 0;
uint16_t last_heartbeat = 0;
uint16_t mode_flag = 1;     // 0=AUTO, 1=MANUAL
uint16_t fire_flag = 0;

uint8_t pir = 0, ldr = 0, smoke = 0, door = 0;
bool light_state = 0, fan_state = 0, buzzer_state = 0;

bool plc_online = false;
uint8_t success_count = 0;
uint8_t fail_count = 0;

unsigned long lastPublish = 0;

// ================= RS485 Direction =================
void preTransmission() { digitalWrite(MAX485_DE, HIGH); }
void postTransmission(){ digitalWrite(MAX485_DE, LOW);  }

// ================= WIFI =================
void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

// ================= MQTT CALLBACK =================
void callback(char* topic, byte* payload, unsigned int length) {

  StaticJsonDocument<200> doc;
  if (deserializeJson(doc, payload, length)) return;

  // 🔥 FIRE PRIORITY
  if (fire_flag == 1) {
    Serial.println("Fire active - commands blocked");
    return;
  }

  // MODE CHANGE FROM DASHBOARD
  if (doc.containsKey("mode")) {
    int newMode = doc["mode"];
    mode_flag = newMode;
    if (plc_online)
      node.writeSingleRegister(10, newMode);   // D10
  }

  // MANUAL CONTROL ONLY
  if (mode_flag == 1) {

    if (doc.containsKey("light")) {
      light_state = doc["light"];
      if (plc_online)
        node.writeSingleCoil(0, light_state);
    }

    if (doc.containsKey("fan")) {
      fan_state = doc["fan"];
      if (plc_online)
        node.writeSingleCoil(1, fan_state);
    }

    if (doc.containsKey("buzzer")) {
      buzzer_state = doc["buzzer"];
      if (plc_online)
        node.writeSingleCoil(2, buzzer_state);
    }
  }
}

// ================= MQTT =================
void maintainMQTT() {
  if (!client.connected()) {
    if (client.connect("ESP32_Gateway")) {
      client.subscribe(controlTopic);
    }
  }
  client.loop();
}

// ================= PLC READ =================
void readPLC() {

  uint8_t result;

  // ---- Read Temperature + Heartbeat ----
  result = node.readHoldingRegisters(0, 2);  // D0 & D1

  if (result == node.ku8MBSuccess) {

    temperature_raw = node.getResponseBuffer(0);
    heartbeat       = node.getResponseBuffer(1);

    success_count++;
    fail_count = 0;

    if (success_count >= 3) plc_online = true;

    // Heartbeat validation
    if (heartbeat == last_heartbeat) {
      plc_online = false;
    }

    last_heartbeat = heartbeat;

    // ---- Read Mode & Fire ----
    node.readHoldingRegisters(10, 2);
    mode_flag = node.getResponseBuffer(0);
    fire_flag = node.getResponseBuffer(1);

    // ---- Read Inputs ----
    node.readDiscreteInputs(0, 4);
    pir   = node.getResponseBuffer(0);
    ldr   = node.getResponseBuffer(1);
    smoke = node.getResponseBuffer(2);
    door  = node.getResponseBuffer(3);

    // ---- Read Outputs ----
    node.readCoils(0, 3);
    light_state  = node.getResponseBuffer(0);
    fan_state    = node.getResponseBuffer(1);
    buzzer_state = node.getResponseBuffer(2);
  }
  else {

    fail_count++;
    success_count = 0;

    if (fail_count >= 3) plc_online = false;

    // ===== Simulation Mode =====
    if (!plc_online) {
      temperature_raw = 280;
      pir = 1;
      ldr = 0;
      door = 1;
      fire_flag = 0;
      // mode_flag NOT overwritten
    }
  }
}

// ================= PUBLISH =================
void publishStatus() {

  StaticJsonDocument<256> doc;

  doc["plc_online"] = plc_online;
  doc["mode"] = mode_flag;
  doc["fire"] = fire_flag;
  doc["temperature"] = temperature_raw / 10.0;
  doc["pir"] = pir;
  doc["ldr"] = ldr;
  doc["door"] = door;
  doc["light"] = light_state;
  doc["fan"] = fan_state;
  doc["buzzer"] = buzzer_state;

  char buffer[256];
  serializeJson(doc, buffer);
  client.publish(statusTopic, buffer);
}

// ================= SETUP =================
void setup() {

  Serial.begin(115200);

  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_DE, LOW);

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  node.begin(1, Serial2);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// ================= LOOP =================
void loop() {

  maintainMQTT();
  readPLC();

  if (millis() - lastPublish > 3000) {
    lastPublish = millis();
    publishStatus();
  }
}
