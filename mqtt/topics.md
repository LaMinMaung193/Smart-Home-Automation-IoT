# MQTT Topics Documentation

## 📡 Broker

* Public Broker: `broker.hivemq.com`
* Protocol: MQTT over WebSocket (Frontend), TCP (ESP32/Backend)

---

## 🟢 Topic: `smarthome/plc1/status`

### Description

Publishes real-time system status from ESP32 (PLC Gateway) to dashboard.

### Publisher

* ESP32

### Subscribers

* Frontend Dashboard

### Payload Example

```json
{
  "plc_online": true,
  "mode": 1,
  "fire": 0,
  "temperature": 26.5,
  "pir": 1,
  "ldr": 0,
  "smoke": 0,
  "door": 1,
  "light": 1,
  "fan": 0,
  "buzzer": 0
}
```

### Field Description

| Field       | Description                    |
| ----------- | ------------------------------ |
| plc_online  | PLC communication status       |
| mode        | 0 = AUTO, 1 = MANUAL           |
| fire        | Fire alarm flag                |
| temperature | Temperature (°C)               |
| pir         | Motion detected (1/0)          |
| ldr         | Light level (1=Bright, 0=Dark) |
| smoke       | Smoke detected (1/0)           |
| door        | Door status                    |
| light       | Light state                    |
| fan         | Fan state                      |
| buzzer      | Buzzer state                   |

---

## 🔵 Topic: `smarthome/plc1/control`

### Description

Sends control commands from dashboard/backend to ESP32.

### Publisher

* Backend Server

### Subscriber

* ESP32

### Payload Example

```json
{
  "light": true
}
```

### Supported Commands

| Command | Description               |
| ------- | ------------------------- |
| mode    | Set AUTO (0) / MANUAL (1) |
| light   | Turn light ON/OFF         |
| fan     | Turn fan ON/OFF           |
| buzzer  | Turn buzzer ON/OFF        |

---

## 🟡 Topic: `smarthome/plc1/ack`

### Description

Acknowledgment from ESP32 after executing command.

### Publisher

* ESP32

### Subscriber

* (Optional: Dashboard / Debug)

### Payload Example

```json
{
  "mode": 1,
  "light": 1,
  "fan": 0,
  "buzzer": 0
}
```

---

## 🔁 Data Flow Overview

```text
ESP32 → status → Dashboard
Dashboard → Backend → control → ESP32
ESP32 → ack → (optional monitoring)
```

---

## ⚠️ Notes

* Fire condition overrides manual control
* PLC offline status disables control execution
* Smoke acts as early warning before fire trigger
* MQTT topics are public — not secure for production

---

## 🔐 Future Improvements

* Add authentication (username/password)
* Use private broker (e.g., Mosquitto, AWS IoT)
* Enable TLS encryption
