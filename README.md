# 🏭 Smart Home Industrial IoT System

## 📌 Overview

This project presents a **real-time Smart Home Industrial IoT System** integrating embedded systems, MQTT communication, and a web-based dashboard.

The system is designed to simulate an **industrial automation environment**, where sensors and actuators are monitored and controlled through a centralized interface.

---

## 🚀 Key Features

* 🔌 **ESP32 Gateway Integration**

  * Interfaces with sensors and actuators
  * Simulates PLC behavior in offline mode

* 📡 **MQTT Communication**

  * Real-time publish/subscribe messaging
  * Lightweight and scalable architecture

* 🖥️ **Web Dashboard**

  * Live monitoring of system status
  * Control of devices (light, fan, buzzer)
  * Mode switching (AUTO / MANUAL)

* ⚠️ **Safety Logic**

  * Control disabled in AUTO mode
  * Emergency lock when fire is detected
  * System protection when PLC is offline

---

## 🏗️ System Architecture

```
ESP32 Gateway  →  MQTT Broker  →  Web Dashboard
        ↑                               ↓
   Sensors & Actuators          User Control Interface
```

---

## 📊 MQTT Topics

| Topic                    | Description                               |
| ------------------------ | ----------------------------------------- |
| `smarthome/status`       | System mode and fire status               |
| `smarthome/sensors`      | Sensor data (temperature, PIR, LDR, door) |
| `smarthome/availability` | PLC online/offline status                 |
| `smarthome/control`      | Control commands from dashboard           |

---

## 🧪 Technologies Used

* Embedded Systems: ESP32 / ESP8266 (Arduino)
* Communication: MQTT (Mosquitto / Public Broker)
* Frontend: HTML, CSS, JavaScript
* Protocols: WebSocket (MQTT over Web)

---

## 🖥️ Dashboard Features

* Real-time sensor monitoring
* Device control interface
* Mode switching (AUTO / MANUAL)
* Fire alert indicator
* PLC availability status

---

## ⚙️ How to Run

### 1. Start Dashboard

```bash
python3 -m http.server 5500
```

Open:

```
http://localhost:5500
```

---

### 2. MQTT Broker (Optional Local)

```bash
mosquitto
```

---

### 3. Test Data (Without ESP32)

```bash
mosquitto_pub -h test.mosquitto.org -t smarthome/sensors -m '{"temperature":28,"pir":1,"ldr":200,"door":0}'
```

---

## 📷 Demo

*(Add your dashboard screenshot here)*

---

## 🎯 Future Improvements

* 📈 Real-time data visualization (charts)
* 🔐 User authentication system
* ☁️ Cloud deployment
* 🤖 AI-based predictive maintenance
* 📊 Data logging & analytics backend

---

## 👨‍💻 Author

**La Min Maung**
Electrical Engineering (Mechatronics) Student
KMITL, Thailand

---

## 💡 Project Goal

To develop a **scalable, real-time industrial IoT system** combining embedded systems, communication protocols, and modern web technologies — suitable for smart home and SME industrial applications.
