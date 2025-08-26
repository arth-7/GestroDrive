# Car Unit (ESP32-CAM / WROVER)
- AsyncWebServer endpoints: `/status`, `/cmd?c=F|B|L|R|S`, `/stream` (MJPEG).
- Motor control via PCF8574 + PWM, ultrasonic safety, LDR-based headlights.

## Build
- Open this folder in VS Code → PlatformIO.
- Select environment `freenove_esp32_wrover` and Upload.
