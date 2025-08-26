# GestroDrive 🚗✋

Gesture-Controlled Robotic Car System built with **ESP32**, **ESP32-CAM**, and **MPU6050**.  
Developed as a capstone project by **Arth Raval** and **Vansh Patel**.  

---

## 📌 Project Overview
GestroDrive is a robotic car that can be controlled by natural **hand gestures** using an ESP32 hand unit.  
The system integrates:
- **ESP-NOW** peer-to-peer communication for low-latency control  
- **ESP32-CAM** for live MJPEG video streaming  
- **PC-hosted web dashboard** for monitoring and manual control  
- **Ultrasonic crash detection** with buzzer alerts  
- **Automatic headlights** with LDR sensors  

---

## 🗂️ Repository Structure
GestroDrive/
│
├── HandUnit/ # PlatformIO project for the hand controller
│ ├── platformio.ini
│ └── src/
│ └── main.cpp
│
├── CarUnit/ # PlatformIO project for the car (ESP32-CAM)
│ ├── platformio.ini
│ └── src/
│ └── main.cpp
│
├── Dashboard/ # Web dashboard for monitoring and control
│ ├── index.html
│ └── README.md
│
├── Docs/ # Documentation
│ ├── Interim Report.pdf
│ ├── What-I-Learned-Summary.md
│ └── Setup-Notes.md
│
└── README.md # This file

---

## ⚙️ Hardware Setup
**Hand Unit**
- ESP32-DevKitC  
- MPU6050 (I²C: SDA=21, SCL=22)  
- SSD1306 OLED display  
- 3.7V LiPo battery + TP4056 charger + MT3608 boost  

**Car Unit**
- ESP32-CAM (OV2640 camera, WROVER module with PSRAM)  
- L298N motor driver + PCF8574 I/O expander  
- HC-SR04 ultrasonic sensor  
- LDRs + LEDs for headlights/taillights  
- Buzzer for alerts  
- 7.4V 2S LiPo battery + LM2596 buck converter  

**Networking**
- ESP-NOW: Hand → Car (gesture commands)  
- Wi-Fi: Car joins local Wi-Fi (SSID = `ESPTEST`)  
- mDNS: Camera accessible via `http://cam.local`  

---

## 💻 Software Setup
- **HandUnit**: PlatformIO, Arduino framework, COM4 @ 115200 baud  
- **CarUnit**: PlatformIO, Arduino framework, COM7 @ 115200 baud  
- **Dashboard**: `index.html` served locally or embedded into ESP32 FS  

**Key Libraries**
- Adafruit MPU6050, SSD1306, GFX  
- ESPAsyncWebServer, AsyncTCP  
- esp_camera, PCF8574  

---

## 🚀 Features
- **Gesture Control**: Forward, Backward, Left, Right, Stop  
- **Web Dashboard**: Toggle between Gesture & Web mode, live telemetry, manual control buttons  
- **Live Video**: MJPEG streaming via `/stream` endpoint  
- **Crash Detection**: Stop if obstacle ≤30 cm for ≥150 ms, buzzer alert  
- **Lighting**: Auto headlights from LDR readings  

---

## 📖 Documentation
- **[Interim Report (PDF)](Docs/Interim%20Report.pdf)** – complete technical report  
- **[What I Learned Summary](Docs/What-I-Learned-Summary.md)** – technical learning outcomes  
- **[Setup Notes](Docs/Setup-Notes.md)** – hardware/software setup guide  

---

## 👥 Team
- [Arth Raval](https://github.com/arth-7)  
- Vansh Patel  

---

## 📜 License
This project is released under the **MIT License** – free to use and modify.  

---
