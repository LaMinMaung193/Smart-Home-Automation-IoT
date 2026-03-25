#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ModbusMaster.h>

// ================= WIFI =================
const char* ssid = "A";
const char* password = "44444444";

// ================= MQTT =================
const char* mqtt_server = "broker.hivemq.com";
const char* statusTopic = "smarthome/plc1/status";
const char* controlTopic = "smarthome/plc1/control";

// ================= RS485 =================
#define RXD2 16
#define TXD2 17
#define MAX485_DE 4

ModbusMaster node;

// ================= Objects =================
WiFiClient espClient;
PubSubClient client(espClient);

// ================= PLC Data Variables =================
uint8_t pir = 0, ldr = 0, smoke = 0, door = 0;
uint16_t temperature_raw = 0;
uint16_t mode_flag = 1;   // Default MANUAL for simulation
uint16_t fire_flag = 0;

bool light_state = 0, fan_state = 0, buzzer_state = 0;
bool plc_online = false;

unsigned long lastPublish = 0;

// ================= RS485 Control =================
void preTransmission() {
  digitalWrite(MAX485_DE, HIGH);
}

void postTransmission() {
  digitalWrite(MAX485_DE, LOW);
}

// ================= WIFI =================
void setup_wifi() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
}

// ================= MQTT Callback =================
void callback(char* topic, byte* payload, unsigned int length) {

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload, length);

  if (error) {
    Serial.println("JSON Parse Failed");
    return;
  }

  // 🔥 Fire always highest priority
  if (fire_flag == 1) {
    Serial.println("Fire active - command ignored");
    return;
  }

  // Manual mode required
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

    Serial.println("Manual command applied");
  }
  else {
    Serial.println("AUTO mode - command ignored");
  }
}

// ================= MQTT Maintain =================
void maintainMQTT() {

  if (!client.connected()) {

    if (client.connect("ESP32_Gateway")) {
      Serial.println("MQTT Connected");
      client.subscribe(controlTopic);
    }
  }

  client.loop();
}

// ================= READ PLC =================
void readPLCStatus() {

  uint8_t result;

  result = node.readHoldingRegisters(0, 1);  // Temperature

  if (result == node.ku8MBSuccess) {

    plc_online = true;
    temperature_raw = node.getResponseBuffer(0);

    node.readDiscreteInputs(0, 4);
    pir = node.getResponseBuffer(0);
    ldr = node.getResponseBuffer(1);
    smoke = node.getResponseBuffer(2);
    door = node.getResponseBuffer(3);

    node.readHoldingRegisters(10, 2);
    mode_flag = node.getResponseBuffer(0);
    fire_flag = node.getResponseBuffer(1);

    node.readCoils(0, 3);
    light_state = node.getResponseBuffer(0);
    fan_state = node.getResponseBuffer(1);
    buzzer_state = node.getResponseBuffer(2);
  }
  else {

    plc_online = false;
    Serial.println("Modbus Read Error - Simulation Mode");

    // ===== SIMULATION VALUES =====
    temperature_raw = 280;
    pir = 1;
    ldr = 0;
    door = 1;
    fire_flag = 0;

    // ⚠️ DO NOT overwrite mode_flag
    // ⚠️ DO NOT overwrite light_state/fan_state/buzzer_state
  }
}

// ================= PUBLISH =================
void publishStatus() {

  StaticJsonDocument<256> doc;

  doc["plc_online"] = plc_online;
  doc["mode"] = (mode_flag == 0) ? "AUTO" : "MANUAL";
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
  readPLCStatus();

  if (millis() - lastPublish > 3000) {
    lastPublish = millis();
    publishStatus();
  }
}
