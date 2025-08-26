# What I Learned from the GestroDrive Project 

## Microcontrollers
- **ESP32-WROOM-32U (Hand Unit)**  
  - Read MPU6050 sensor for gesture recognition.  
  - Learned ESP-NOW peer-to-peer communication (low latency, no AP).  
  - Managed I²C devices (MPU6050 + OLED) on shared SDA/SCL.  
  - Understood ESP32 dual-core execution (sensor loop vs Wi-Fi tasks).  

- **ESP32-WROVER-CAM (Car Unit)**  
  - Integrated OV2640 camera with MJPEG streaming.  
  - Used 4MB PSRAM for buffering video frames.  
  - Gained experience with AsyncWebServer for `/status`, `/cmd`, `/stream`.  
  - Configured mDNS (cam.local) for hostname resolution.  

## Sensors & Peripherals
- **MPU6050 (IMU)** – Learned accelerometer/gyroscope fusion, calibration, and tilt thresholds.  
- **OLED (SSD1306)** – Displayed battery %, gesture direction, and transmission status.  
- **HC-SR04 Ultrasonic** – Practiced time-of-flight calculation: Distance (cm) = (Echo µs × 0.0343)/2.  
- **LDRs** – Used ADC readings to auto-switch headlights.  
- **Buzzer + LEDs** – Implemented automatic buzzer alerts & head/tail light control.  

## Motor Driver
- **L298N with PCF8574 I/O Expander**  
  - Controlled IN1–IN4 via I²C expander; ENA/ENB driven by PWM from ESP32.  
  - Learned H-bridge fundamentals and current limits.  

## Power System
- **Hand Unit**: 3.7V LiPo + TP4056 charging + MT3608 boost converter.  
- **Car Unit**: 7.4V 2S LiPo + TP5100 charging + LM2596 buck to 5V/3.3V.  
- Learned safe use of BMS/fuses for protection.  

## Technical Skills Gained
- ESP-NOW wireless communication.  
- PWM motor speed control.  
- Web server endpoints for IoT dashboards.  
- Debounced crash detection logic (30 cm threshold, 150 ms).  
- Embedded power system design.  
- Documentation, PCB design, and enclosure modeling in Fusion 360.  

---
