# Dashboard

Web-based control interface for GestroDrive.

- Shows live ESP32-CAM video stream (`/stream`)
- Displays sensor status (distance, buzzer, LEDs)
- Supports switching between Gesture mode and Web mode
- Allows navigation commands (Forward, Backward, Left, Right, Stop)

## How to use
- Open `index.html` in a browser while connected to the ESP32-CAM WiFi.
- Or serve the HTML from the ESP32’s filesystem (`/data` folder + SPIFFS/LittleFS).
