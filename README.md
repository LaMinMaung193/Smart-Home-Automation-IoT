# Smart Home Industrial IoT System

## Overview

This project implements a real-time Smart Home Industrial IoT system that integrates embedded systems, MQTT communication, and a web-based SCADA-style dashboard.

The system simulates an industrial automation environment where sensors and actuators are monitored and controlled through a centralized interface. It includes safety mechanisms, real-time communication, and a scalable architecture suitable for both smart home and small industrial applications.

---

## Key Features

### Embedded Gateway (ESP32)

* Interfaces with sensors (PIR, LDR, smoke, door, temperature)
* Communicates with PLC via RS485 (Modbus RTU)
* Publishes system state via MQTT
* Executes control commands from backend
* Implements safety logic (fire override, offline protection)

### Real-Time Communication (MQTT)

* Lightweight publish/subscribe architecture
* Decouples hardware, backend, and frontend
* Supports real-time monitoring and control

### Web Dashboard (Frontend)

* Live system monitoring (PLC status, sensors, devices)
* Manual and automatic mode switching
* Device control (light, fan, buzzer)
* Fire and smoke alert visualization
* Real-time charts (temperature and LDR trends)

### Backend Server (Node.js)

* Secure API with JWT authentication
* Sends control commands via MQTT
* Acts as a bridge between user and ESP32
* Handles access control and authorization

### Safety and Reliability

* Fire condition overrides all manual control
* Smoke detection provides early warning
* PLC offline detection disables control execution
* Heartbeat-based communication monitoring

---

## System Architecture

```
                ┌────────────────────┐
                │   Web Dashboard    │
                │ (HTML/CSS/JS + MQTT)
                └─────────┬──────────┘
                          │
                          │ MQTT (WebSocket)
                          ▼
                ┌────────────────────┐
                │   MQTT Broker      │
                │ (HiveMQ Public)    │
                └─────────┬──────────┘
                          │
          ┌───────────────┴───────────────┐
          │                               │
          ▼                               ▼
┌────────────────────┐         ┌────────────────────┐
│   Backend Server   │         │     ESP32 Gateway  │
│   (Node.js API)    │         │  (Modbus + MQTT)   │
└────────────────────┘         └─────────┬──────────┘
                                         │
                                         ▼
                              ┌────────────────────┐
                              │   PLC / Sensors    │
                              │  Actuators (I/O)   │
                              └────────────────────┘
```

---

## MQTT Topics

### 1. Status Topic

**Topic:** `smarthome/plc1/status`
**Publisher:** ESP32
**Subscriber:** Dashboard

**Description:**
Publishes real-time system state.

**Payload Example:**

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

---

### 2. Control Topic

**Topic:** `smarthome/plc1/control`
**Publisher:** Backend
**Subscriber:** ESP32

**Description:**
Sends control commands to devices.

**Payload Example:**

```json
{
  "light": true
}
```

**Supported Commands:**

* `mode` → 0 (AUTO), 1 (MANUAL)
* `light` → true / false
* `fan` → true / false
* `buzzer` → true / false

---

### 3. Acknowledgment Topic

**Topic:** `smarthome/plc1/ack`
**Publisher:** ESP32

**Description:**
Confirms command execution.

---

## Dashboard Features

* PLC online/offline monitoring
* Sensor visualization:

  * PIR (motion)
  * LDR (light level)
  * Smoke detection
  * Door status
* Device status monitoring:

  * Light
  * Fan
  * Buzzer
* Fire alarm with sound alert
* Smoke warning indicator
* Real-time charts:

  * Temperature trend
  * LDR trend

---

## Backend API

### Base URL

```
https://smart-home-automation-iot.onrender.com
```

### Endpoints

#### POST /control

Sends control command (requires authentication)

**Headers:**

```
Authorization: Bearer <token>
Content-Type: application/json
```

**Body Example:**

```json
{
  "device": "light",
  "state": true
}
```

---

## Technologies Used

* Embedded Systems: ESP32 (Arduino framework)
* Communication: MQTT (HiveMQ public broker)
* Industrial Protocol: Modbus RTU (RS485)
* Frontend: HTML, CSS, JavaScript, Chart.js
* Backend: Node.js, Express.js
* Authentication: JSON Web Token (JWT)

---

## How to Run

### 1. Frontend Dashboard

```bash
python3 -m http.server 5500
```

Open:

```
http://localhost:5500
```

---

### 2. Backend Server

```bash
npm install
node server.js
```

---

### 3. ESP32

* Configure WiFi credentials
* Upload firmware via Arduino IDE
* Ensure MQTT broker and RS485 connections are correct

---

### 4. MQTT Broker (Optional Local)

```bash
mosquitto
```

---

## System Logic

* AUTO mode: PLC controls devices automatically
* MANUAL mode: User controls devices via dashboard
* Fire condition:

  * Overrides all control
  * Activates buzzer
  * Turns off other devices
* Smoke condition:

  * Early warning indicator
  * Can be extended to trigger fire logic
* PLC offline:

  * Control commands are ignored
  * Dashboard shows warning

---

## Security Notes

* Public MQTT broker is used (not secure for production)
* Backend uses JWT authentication for protected routes
* Recommended improvements:

  * Use private MQTT broker
  * Enable TLS encryption
  * Add role-based access control

---

## Future Improvements

* Persistent data logging (database integration)
* Mobile application integration
* Push notifications (fire/smoke alerts)
* AI-based anomaly detection
* Multi-device and multi-room scalability
* Cloud IoT integration (AWS IoT, Azure IoT)

---

## Author & Contributor

La Min Maung
Electrical Engineering (Mechatronics) Student
KMITL, Thailand

---

## Project Goal

To design and implement a scalable, real-time industrial IoT system that combines embedded hardware, communication protocols, and modern web technologies for monitoring and control applications.
