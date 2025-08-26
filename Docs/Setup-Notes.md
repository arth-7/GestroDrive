# GestroDrive Hardware & Software Setup 🛠

## Hand Unit (Controller)
**Hardware**
- ESP32-DevKitC  
- MPU6050 (I²C: SDA=21, SCL=22)  
- SSD1306 OLED (I²C shared with MPU6050)  
- LiPo 3.7V battery + TP4056 charger + MT3608 boost to 5V  
- Enclosure with wrist strap  

**Software**
- PlatformIO (Arduino framework)  
- Libraries: `Adafruit_MPU6050`, `Adafruit_SSD1306`, `Adafruit_GFX`, `ESPAsyncWebServer`, `AsyncTCP`  
- Upload port: COM4, baud: 115200  
- Sends gestures (F/B/L/R/S) to car unit via ESP-NOW  

---

## Car Unit (Vehicle)
**Hardware**
- ESP32-CAM (WROVER module, OV2640 camera)  
- L298N motor driver + PCF8574 I/O expander  
- HC-SR04 ultrasonic sensor  
- LDRs for auto headlights  
- Buzzer + LEDs for alerts  
- TT motors with wheels, powered by 7.4V 2S LiPo  
- LM2596 buck converter for 5V regulation  

**Software**
- PlatformIO (Arduino framework)  
- Libraries: `ESPAsyncWebServer`, `AsyncTCP`, `esp_camera`, `PCF8574`  
- Upload port: COM7, baud: 115200  
- Provides endpoints:  
  - `/status` – JSON telemetry (distance, LEDs, buzzer, mode)  
  - `/cmd?c=X` – motor control (F/B/L/R/S/T)  
  - `/stream` – MJPEG video feed  
- Obstacle detection logic: stop if distance ≤30 cm for 150 ms  

---

## PC Dashboard
**Hardware**
- Any PC/laptop on same Wi-Fi network  

**Software**
- Local `index.html` dashboard (in `/Dashboard`)  
- Fetches `/status` and `/cmd` from ESP32-CAM  
- Embeds MJPEG stream from `/stream`  

---

## Networking
- Car + PC connect to same Wi-Fi SSID (`ESPTEST`).  
- Hand Unit uses ESP-NOW on locked Wi-Fi channel.  
- mDNS enabled: `http://cam.local` resolves to ESP32-CAM.  

---
