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
const char* ackTopic     = "smarthome/plc1/ack";

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

uint16_t mode_flag = 1;
uint16_t fire_flag = 0;

uint8_t pir = 0, ldr = 0, smoke = 0, door = 0;

bool light_state = 0, fan_state = 0, buzzer_state = 0;
bool prev_light = 0, prev_fan = 0, prev_buzzer = 0;

bool plc_online = false;
uint8_t hb_fail_count = 0;

unsigned long lastPublish = 0;

// ================= RS485 =================
void preTransmission() { digitalWrite(MAX485_DE, HIGH); }
void postTransmission(){ digitalWrite(MAX485_DE, LOW); }

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

  // 🔥 BLOCK IF FIRE
  if (fire_flag == 1) return;

  // ===== MODE =====
  if (doc.containsKey("mode")) {
    mode_flag = doc["mode"];
    node.writeSingleRegister(9, mode_flag);   // ✅ FIXED (D10 -> index 9)
  }

  // ===== MANUAL CONTROL =====
  if (mode_flag == 1) {

    if (doc.containsKey("light")) {
      light_state = doc["light"];
    }

    if (doc.containsKey("fan")) {
      fan_state = doc["fan"];
    }

    if (doc.containsKey("buzzer")) {
      buzzer_state = doc["buzzer"];
    }
  }

  // ===== ACK =====
  StaticJsonDocument<128> ack;
  ack["mode"] = mode_flag;
  ack["light"] = light_state;
  ack["fan"] = fan_state;
  ack["buzzer"] = buzzer_state;

  char buffer[128];
  serializeJson(ack, buffer);
  client.publish(ackTopic, buffer);
}

// ================= MQTT =================
void maintainMQTT() {
  while (!client.connected()) {
    if (client.connect("ESP32_Gateway")) {
      client.subscribe(controlTopic);
    } else {
      delay(2000);
    }
  }
  client.loop();
}

// ================= PLC READ =================
void readPLC() {

  uint8_t result;

  // ===== TEMP + HEARTBEAT =====
  result = node.readHoldingRegisters(0, 2);
  if (result == node.ku8MBSuccess) {

    temperature_raw = node.getResponseBuffer(0);
    heartbeat       = node.getResponseBuffer(1);

    // ===== HEARTBEAT CHECK =====
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 1000) {
      lastCheck = millis();

      if (heartbeat == last_heartbeat) {
        hb_fail_count++;
      } else {
        hb_fail_count = 0;
      }

      last_heartbeat = heartbeat;
      plc_online = (hb_fail_count < 5);
    }

  } else {
    plc_online = false;
    return;
  }

  // ===== MODE + FIRE =====
  result = node.readHoldingRegisters(9, 2);   // ✅ FIXED OFFSET
  if (result == node.ku8MBSuccess) {
    mode_flag = node.getResponseBuffer(0);
    fire_flag = node.getResponseBuffer(1);
  }

  // ===== INPUTS (FIXED) =====
  result = node.readCoils(0, 4);   // ✅ CHANGED (X inputs mapped here)
  if (result == node.ku8MBSuccess) {
    pir   = node.getResponseBuffer(0);
    ldr   = node.getResponseBuffer(1);
    smoke = node.getResponseBuffer(2);
    door  = node.getResponseBuffer(3);
  }

  // ===== OUTPUTS =====
  result = node.readCoils(0, 3);
  if (result == node.ku8MBSuccess) {
    light_state  = node.getResponseBuffer(0);
    fan_state    = node.getResponseBuffer(1);
    buzzer_state = node.getResponseBuffer(2);
  }
}

// ================= SAFETY =================
void safetyCheck() {

  if (fire_flag == 1 && plc_online) {
    node.writeSingleCoil(0, 0);
    node.writeSingleCoil(1, 0);
    node.writeSingleCoil(2, 1);
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
  doc["smoke"] = smoke;
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

  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }

  maintainMQTT();
  readPLC();
  safetyCheck();

  // ✅ SINGLE CONTROL POINT
  if (plc_online && mode_flag == 1 && fire_flag == 0) {

    if (light_state != prev_light) {
      node.writeSingleCoil(0, light_state);   // ✅ FIXED ADDRESS
      prev_light = light_state;
    }

    if (fan_state != prev_fan) {
      node.writeSingleCoil(1, fan_state);     // ✅ FIXED
      prev_fan = fan_state;
    }

    if (buzzer_state != prev_buzzer) {
      node.writeSingleCoil(2, buzzer_state);  // ✅ FIXED
      prev_buzzer = buzzer_state;
    }
  }

  if (millis() - lastPublish > 3000) {
    lastPublish = millis();
    publishStatus();
  }
}