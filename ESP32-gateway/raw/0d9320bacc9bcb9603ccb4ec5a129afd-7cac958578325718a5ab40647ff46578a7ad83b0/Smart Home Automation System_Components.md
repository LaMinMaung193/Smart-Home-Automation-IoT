# Smart Home Automation System

---

## Communication Hardware (PLC ↔ ESP)

| Item                 | Specification (What to search)       |   Qty | Est. Price (THB) | Notes                    |
| -------------------- | ------------------------------------ | ----: | ---------------: | ------------------------ |
| MAX485 Module        | MAX485 TTL to RS485 Converter Module |     1 |            40–60 | Essential for Modbus RTU |
| ESP32 Dev Board      | ESP32 DevKit V1 / NodeMCU-32S        |     1 |          200–280 | Recommended over ESP8266 |
| USB Cable            | Micro-USB / Type-C                   |     1 |            30–50 | For programming ESP      |
| DB9 to RS485 Adapter | DB9 Male RS485 Adapter               |     1 |            50–80 | PLC serial connection    |
| Jumper Wires         | Dupont Male–Female                   | 1 set |            30–50 | Wiring convenience       |

MAX485 
- Supports half-duplex RS-485
- 3.3 V compatible (ESP32 safe)

---

## Sensors (Inputs to PLC)
| Sensor             | Model / Keyword                 | Type             | Qty | Price (THB) | PLC Input |
| ------------------ | ------------------------------- | ---------------- | --: | ----------: | --------- |
| PIR Motion Sensor  | HC-SR501                        | Digital          |   1 |       40–60 | X         |
| LDR Module         | LDR Light Sensor Module (LM393) | Digital          |   1 |       30–50 | X         |
| Temperature Sensor | DS18B20 Waterproof              | Digital (1-Wire) |   1 |       60–90 | X         |
| Reed Switch        | Magnetic Door Sensor            | Digital          |   1 |       20–40 | X         |
| Smoke Sensor       | MQ-2 Gas Sensor Module          | Digital          |   1 |      70–100 | X         |

`note`
- Avoid raw analog sensors unless PLC has ADC
- Use modules with digital output (DO pin)

---

## Actuators (Outputs from PLC)
| Actuator     | Specification                         | Qty | Price (THB) | PLC Output |
| ------------ | ------------------------------------- | --: | ----------: | ---------- |
| Relay Module | 5V / 24V Relay Module (Opto-isolated) | 2–4 |  50–80 each | Y          |
| AC Lamp      | 220 V Bulb + Holder                   |   1 |      50–100 | Load       |
| DC Fan       | 12 V DC Fan                           |   1 |      80–150 | Load       |
| Buzzer       | 24 V Active Buzzer                    |   1 |       30–50 | Y          |
| Warning LED  | 24 V Indicator Lamp                   |   1 |       30–50 | Y          |

- PLC relay output → relay module → AC load
- Good for demo & safety

---

## Power Supply & Protection
| Item             | Specification  | Qty | Price (THB) | Purpose       |
| ---------------- | -------------- | --: | ----------: | ------------- |
| 24V Power Supply | 24V 2A SMPS    |   1 |     200–300 | PLC + sensors |
| Buck Converter   | LM2596 DC-DC   |   1 |       40–60 | 24V → 5V      |
| 5V Regulator     | AMS1117 / Buck |   1 |       20–40 | ESP power     |
| Terminal Block   | Screw Terminal |   1 |       30–50 | Wiring        |
| Fuse Holder      | 5×20mm Fuse    |   1 |       20–40 | Safety        |

---

## Optional
| Item             | Why it helps              | Price (THB) |
| ---------------- | ------------------------- | ----------: |
| DIN Rail         | Professional PLC mounting |       50–80 |
| PLC Enclosure    | Clean demo & safety       |     150–300 |
| RS485 Terminator | Stable communication      |          20 |
| Label Stickers   | Clear I/O labeling        |          20 |

---

## Estimated Total Budget (Core System)
| Category       |             Approx. Cost |
| -------------- | -----------------------: |
| PLC            |                     ~700 |
| ESP32 + MAX485 |                     ~300 |
| Sensors        |                     ~250 |
| Actuators      |                     ~300 |
| Power & wiring |                     ~300 |
| **Total**      | **~1,800 – 2,000 THB**   |

---

**`Trust me! but mhrr yin pyan wal ya ml`**

---
## Modbus Address Table
### 🔹 PLC → ESP32 (Read Coils / Inputs)

| PLC Device | Description | Modbus Type      | Address | Data   |
| ---------- | ----------- | ---------------- | ------- | ------ |
| X0         | PIR Motion  | Discrete Input   | 10001   | 0/1    |
| X1         | LDR Dark    | Discrete Input   | 10002   | 0/1    |
| X2         | Smoke Alarm | Discrete Input   | 10003   | 0/1    |
| X3         | Door Reed   | Discrete Input   | 10004   | 0/1    |
| D0         | Temperature | Holding Register | 40001   | °C ×10 |



---

### 🔹 ESP32 → PLC (Write Coils)

| PLC Device | Description | Modbus Type | Address |
| ---------- | ----------- | ----------- | ------- |
| Y0         | Light       | Coil        | 00001   |
| Y1         | Fan         | Coil        | 00002   |
| Y2         | Buzzer      | Coil        | 00003   |


- Discrete Inputs = read-only
- Coils = control outputs
- Holding Registers = numeric values

---

## Hardware Wiring (ESP32 ↔ MAX485)
### MAX485 Pins

| MAX485           | ESP32 |
| ---------------- | ----- |
| RO               | RX2   |
| DI               | TX2   |
| RE + DE (joined) | GPIO4 |
| VCC              | 5V    |
| GND              | GND   |


### RS485 Side

| MAX485 | PLC |
| ------ | --- |
| A      | A   |
| B      | B   |
| GND    | GND |


- Always connect GND between PLC & ESP.

---

## ESP32 Software Structure
ESP32 will act as:

- Modbus RTU Master
- WiFi Client
- IoT Publisher
- Remote Control Receiver

---

## Overall Software Architecture
```text
SETUP()
 ├── Serial Debug
 ├── RS485 Init
 ├── Modbus Init
 ├── WiFi Connect
 └── Cloud Init (MQTT/Blynk)

LOOP()
 ├── Read PLC Inputs
 ├── Read Temperature Register
 ├── Publish to Cloud
 ├── Check Remote Commands
 ├── Write Coils to PLC
 └── Repeat
```
