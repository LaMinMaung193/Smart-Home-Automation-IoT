#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ModbusMaster.h>

// ================= DEBUG MODE =================
#define DEBUG 1
#if DEBUG
  #define DBG(x) Serial.println(x)
#else
  #define DBG(x)
#endif

// ================= WIFI =================
const char* ssid = "A";
const char* password = "44444444";

// ================= MQTT =================
const char* mqtt_server = "broker.hivemq.com";
const char* statusTopic = "smarthome/plc1/status";
const char* controlTopic = "smarthome/plc1/control";
const char* availabilityTopic = "smarthome/plc1/availability";

// ================= RS485 =================
#define RXD2 16
#define TXD2 17
#define MAX485_DE 4

ModbusMaster node;
WiFiClient espClient;
PubSubClient client(espClient);

// ================= PLC VARIABLES =================
uint8_t pir=0, ldr=0, smoke=0, door=0;
uint16_t temperature_raw=0;
uint16_t mode_flag=1;
uint16_t fire_flag=0;

bool light_state=0, fan_state=0, buzzer_state=0;
bool plc_online=false;

unsigned long lastPublish=0;

// ================= RS485 =================
void preTransmission(){ digitalWrite(MAX485_DE, HIGH); }
void postTransmission(){ digitalWrite(MAX485_DE, LOW); }

// ================= WIFI =================
void setup_wifi(){
  WiFi.begin(ssid,password);
  DBG("Connecting WiFi...");
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
  }
  DBG("WiFi Connected");
}

// ================= MQTT CALLBACK =================
void callback(char* topic, byte* payload, unsigned int length){

  StaticJsonDocument<200> doc;
  if(deserializeJson(doc,payload,length)){
    DBG("JSON parse error");
    return;
  }

  if(fire_flag==1){
    DBG("Fire active - commands blocked");
    return;
  }

  // ---- MODE CHANGE ----
  if(doc.containsKey("mode")){
    mode_flag = doc["mode"];
    if(plc_online){
      node.writeSingleRegister(10, mode_flag);  // D10
      DBG("Mode written to PLC");
    }
  }

  if(mode_flag==1){

    if(doc.containsKey("light")){
      bool cmd = doc["light"];
      if(plc_online){
        node.writeSingleCoil(0, cmd);
        node.readCoils(0,1); // Read back
        light_state = node.getResponseBuffer(0);
      }else{
        light_state = cmd;
      }
      DBG("Light command processed");
    }

    if(doc.containsKey("fan")){
      bool cmd = doc["fan"];
      if(plc_online){
        node.writeSingleCoil(1, cmd);
        node.readCoils(1,1);
        fan_state = node.getResponseBuffer(0);
      }else{
        fan_state = cmd;
      }
      DBG("Fan command processed");
    }

    if(doc.containsKey("buzzer")){
      bool cmd = doc["buzzer"];
      if(plc_online){
        node.writeSingleCoil(2, cmd);
        node.readCoils(2,1);
        buzzer_state = node.getResponseBuffer(0);
      }else{
        buzzer_state = cmd;
      }
      DBG("Buzzer command processed");
    }
  }
  else{
    DBG("AUTO mode - command ignored");
  }
}

// ================= MQTT =================
void maintainMQTT(){

  if(!client.connected()){

    if(client.connect(
        "ESP32_Gateway",
        availabilityTopic,
        1,
        true,
        "offline")){

      DBG("MQTT Connected");
      client.publish(availabilityTopic,"online",true);
      client.subscribe(controlTopic);
    }
  }

  client.loop();
}

// ================= PLC READ =================
void readPLCStatus(){

  uint8_t result = node.readHoldingRegisters(0,1);

  if(result==node.ku8MBSuccess){

    plc_online=true;
    temperature_raw=node.getResponseBuffer(0);

    node.readDiscreteInputs(0,4);
    pir=node.getResponseBuffer(0);
    ldr=node.getResponseBuffer(1);
    smoke=node.getResponseBuffer(2);
    door=node.getResponseBuffer(3);

    node.readHoldingRegisters(10,2);
    mode_flag=node.getResponseBuffer(0);
    fire_flag=node.getResponseBuffer(1);

    node.readCoils(0,3);
    light_state=node.getResponseBuffer(0);
    fan_state=node.getResponseBuffer(1);
    buzzer_state=node.getResponseBuffer(2);
  }
  else{

    plc_online=false;
    DBG("PLC Offline - Simulation Mode");

    temperature_raw=280;
    pir=1;
    ldr=0;
    door=1;
    fire_flag=0;
  }
}

// ================= PUBLISH =================
void publishStatus(){

  StaticJsonDocument<256> doc;

  doc["plc_online"]=plc_online;
  doc["mode"]=mode_flag;
  doc["fire"]=fire_flag;
  doc["temperature"]=temperature_raw/10.0;
  doc["pir"]=pir;
  doc["ldr"]=ldr;
  doc["door"]=door;
  doc["light"]=light_state;
  doc["fan"]=fan_state;
  doc["buzzer"]=buzzer_state;

  char buffer[256];
  serializeJson(doc,buffer);
  client.publish(statusTopic,buffer);
}

// ================= SETUP =================
void setup(){

  Serial.begin(115200);

  pinMode(MAX485_DE,OUTPUT);
  digitalWrite(MAX485_DE,LOW);

  Serial2.begin(9600,SERIAL_8N1,RXD2,TXD2);

  node.begin(1,Serial2);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  setup_wifi();

  client.setServer(mqtt_server,1883);
  client.setCallback(callback);
}

// ================= LOOP =================
void loop(){

  maintainMQTT();
  readPLCStatus();

  if(millis()-lastPublish>3000){
    lastPublish=millis();
    publishStatus();
  }
}
