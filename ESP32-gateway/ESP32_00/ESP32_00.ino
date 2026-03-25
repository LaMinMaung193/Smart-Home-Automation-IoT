#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "A";
const char* password = "44444444";

const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;

#define LED_PIN 2   // Change to 4 if LED not working

// ================= WIFI =================
void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// ================= MQTT RECONNECT =================
void reconnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP32_SmartHome_Test")) {
      Serial.println("MQTT Connected!");
      client.subscribe("smarthome/control");
      Serial.println("Subscribed to smarthome/control");
    } else {
      Serial.print("MQTT Failed, rc=");
      Serial.println(client.state());
      Serial.println("Retrying in 3 seconds...");
      delay(3000);
    }
  }
}

// ================= CALLBACK =================
void callback(char* topic, byte* payload, unsigned int length) {

  Serial.println("===== MESSAGE RECEIVED =====");

  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Topic: ");
  Serial.println(topic);

  Serial.print("Message: ");
  Serial.println(message);

  if (message == "ON") {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED TURNED ON");
  }

  else if (message == "OFF") {
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED TURNED OFF");
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// ================= LOOP =================
void loop() {

  if (!client.connected()) {
    reconnect();
  }

  client.loop();   // VERY IMPORTANT — keeps MQTT alive

  // Publish every 5 seconds WITHOUT blocking
  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    client.publish("smarthome/test", "Hello from La Min ESP32");
    Serial.println("Heartbeat Sent");
  }
}
