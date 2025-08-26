#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_camera.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <PCF8574.h>

// ================== Motor & I/O ==================
#define ENA 12
#define ENB 2
#define I2C_SDA 32
#define I2C_SCL 14
PCF8574 pcf(0x20);

// Motor pins on PCF8574
#define IN1 0
#define IN2 1
#define IN3 2
#define IN4 3

// Ultrasonic + Buzzer + LDR
#define ECHO_PIN 13
#define TRIG_PCF_PIN 6
#define BUZZER_PIN 0      // NOTE: GPIO0 can affect boot; consider moving to a safe pin or PCF8574.
#define LDR_PIN 33
#define LED1_PCF_PIN 4
#define LED2_PCF_PIN 5
#define LED3_PCF_PIN 7

// Camera Pins (ESP32-CAM)
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 21
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 19
#define Y4_GPIO_NUM 18
#define Y3_GPIO_NUM 5
#define Y2_GPIO_NUM 4
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

const char* ssid = "ESPTEST";
const char* password = "vansh2004";

enum MotorMode { STOP = 0, FORWARD, BACKWARD, LEFT, RIGHT };
struct struct_message { MotorMode motorMode; } handCommand;

MotorMode currentMode = STOP;
bool webControlMode = false; // false = Gesture, true = Web

float latestDistance = 0;
bool latestBuzzer = false;
bool latestLED = false;

AsyncWebServer server(80);

// Debounce for ultrasonic
unsigned long obstacleStart = 0;
bool obstacleActive = false;

// --------- Motor Control ---------
void stopMotors() {
  pcf.digitalWrite(IN1, LOW);
  pcf.digitalWrite(IN2, LOW);
  pcf.digitalWrite(IN3, LOW);
  pcf.digitalWrite(IN4, LOW);
  ledcWrite(0, 0);
  ledcWrite(1, 0);
}

void driveMotors(MotorMode mode) {
  int speed = 200;
  stopMotors();
  switch (mode) {
    case FORWARD:
      pcf.digitalWrite(IN1, LOW);  pcf.digitalWrite(IN2, HIGH);
      pcf.digitalWrite(IN3, LOW);  pcf.digitalWrite(IN4, HIGH);
      break;
    case BACKWARD:
      pcf.digitalWrite(IN1, HIGH); pcf.digitalWrite(IN2, LOW);
      pcf.digitalWrite(IN3, HIGH); pcf.digitalWrite(IN4, LOW);
      break;
    case LEFT:
      pcf.digitalWrite(IN1, HIGH); pcf.digitalWrite(IN2, LOW);
      pcf.digitalWrite(IN3, LOW);  pcf.digitalWrite(IN4, HIGH);
      break;
    case RIGHT:
      pcf.digitalWrite(IN1, LOW);  pcf.digitalWrite(IN2, HIGH);
      pcf.digitalWrite(IN3, HIGH); pcf.digitalWrite(IN4, LOW);
      break;
    default: break;
  }
  ledcWrite(0, speed);
  ledcWrite(1, speed);
}

// --------- ESP-NOW Callback ---------
void onReceive(const uint8_t * mac, const uint8_t *incomingData, int len) {
  if (webControlMode) return; // Ignore gestures in Web mode
  memcpy(&handCommand, incomingData, sizeof(handCommand));
  currentMode = handCommand.motorMode;
  Serial.print("[ESP-NOW] Received motorMode: ");
  Serial.println(handCommand.motorMode);
}

// --------- Ultrasonic ---------
float measureDistanceCM() {
  pcf.digitalWrite(TRIG_PCF_PIN, LOW); delayMicroseconds(2);
  pcf.digitalWrite(TRIG_PCF_PIN, HIGH); delayMicroseconds(10);
  pcf.digitalWrite(TRIG_PCF_PIN, LOW); delayMicroseconds(2);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
  if (duration == 0) return -1;
  return duration * 0.0343 / 2.0;
}

// --------- LDR LEDs ---------
void updateLED() {
  int ldr = analogRead(LDR_PIN);
  bool ledState = (ldr < 1000);
  pcf.digitalWrite(LED1_PCF_PIN, ledState);
  pcf.digitalWrite(LED2_PCF_PIN, ledState);
  pcf.digitalWrite(LED3_PCF_PIN, ledState);
  latestLED = ledState;
}

// --------- Motor + Sensor Task ---------
void motorControlTask(void *pv) {
  while (true) {
    latestDistance = measureDistanceCM();
    unsigned long now = millis();

    // --- Debounced Obstacle Detection (30 cm for 2 sec) ---
    if (latestDistance > 0 && latestDistance <= 30) {
      if (!obstacleActive) {
        if (obstacleStart == 0) obstacleStart = now;
        if (now - obstacleStart >= 150) {  // NOTE: 150ms here; comment says 2s. Adjust if desired.
          obstacleActive = true;
        }
      }
    } else {
      obstacleStart = 0;
      obstacleActive = false;
    }

    if (obstacleActive) {
      stopMotors();
      ledcWrite(2, 128);
      latestBuzzer = true;
      if (currentMode == BACKWARD) driveMotors(BACKWARD);
    } else {
      ledcWrite(2, 0);
      latestBuzzer = false;
      driveMotors(currentMode);
    }

    updateLED();
    Serial.printf("[STATUS] Dist=%.1f cm, Buzzer=%d, LED=%d, Mode=%d, Web=%d\n",
                  latestDistance, latestBuzzer, latestLED, currentMode, webControlMode);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// --------- Camera Setup ---------
bool setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM; config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM; config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM; config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM; config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM; config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM; config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM; config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM; config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QQVGA;  
  config.jpeg_quality = 20;             
  config.fb_count = 1;                  
  return esp_camera_init(&config) == ESP_OK;
}

// --------- Async Routes ---------
void setupServer() {
  // Status JSON
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{";
    json += "\"distance\":" + String(latestDistance, 1) + ",";
    json += "\"buzzer\":" + String(latestBuzzer ? "true" : "false") + ",";
    json += "\"led\":" + String(latestLED ? "true" : "false") + ",";
    json += "\"web\":" + String(webControlMode ? "true" : "false");
    json += "}";
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("Access-Control-Allow-Origin", "*");  // keep CORS if dashboard is hosted elsewhere
    request->send(response);
  });

  // Command handler
  server.on("/cmd", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!request->hasArg("c")) {
      request->send(400, "text/plain", "Missing command");
      return;
    }
    char cmd = request->arg("c")[0];
    switch(cmd) {
      case 'F': currentMode = FORWARD; webControlMode = true; break;
      case 'B': currentMode = BACKWARD; webControlMode = true; break;
      case 'L': currentMode = LEFT; webControlMode = true; break;
      case 'R': currentMode = RIGHT; webControlMode = true; break;
      case 'S': currentMode = STOP; webControlMode = true; break;
      case 'T': webControlMode = !webControlMode; currentMode = STOP; break;
    }
    // Add CORS here too if hosting dashboard off-device
    auto *resp = request->beginResponse(200, "text/plain", "OK");
    resp->addHeader("Access-Control-Allow-Origin", "*");
    request->send(resp);
  });

  // Camera stream (non-blocking)
  server.on("/stream", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginChunkedResponse(
      "multipart/x-mixed-replace; boundary=frame",
      [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) return 0;
        size_t len = 0;
        len += snprintf((char*)buffer, maxLen, "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", fb->len);
        if (len + fb->len + 2 > maxLen) {
          esp_camera_fb_return(fb);
          return 0;
        }
        memcpy(buffer + len, fb->buf, fb->len);
        len += fb->len;
        buffer[len++] = '\r';
        buffer[len++] = '\n';
        esp_camera_fb_return(fb);
        return len;
      }
    );
    request->send(response);
  });
}

// --------- Setup ---------
void setup() {
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);
  pcf.begin();
  for (int i = 0; i < 8; i++) pcf.pinMode(i, OUTPUT);

  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT); digitalWrite(BUZZER_PIN, LOW);
  ledcSetup(2, 4000, 8); ledcAttachPin(BUZZER_PIN, 2); ledcWrite(2, 0);
  ledcSetup(0, 5000, 8); ledcAttachPin(ENA, 0);
  ledcSetup(1, 5000, 8); ledcAttachPin(ENB, 1);
  stopMotors();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(300);
  if (MDNS.begin("cam")) Serial.println("✅ cam.local ready");

  wifi_ap_record_t ap_info;
  if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
    esp_wifi_set_ps(WIFI_PS_NONE);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(ap_info.primary, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);
    Serial.printf("[WiFi] Locked to channel %d\n", ap_info.primary);
  }

  setupCamera();
  setupServer();
  server.begin();

  if (esp_now_init() == ESP_OK) {
    esp_now_register_recv_cb(onReceive);
    Serial.println("[ESP-NOW] Ready");
  }

  xTaskCreatePinnedToCore(motorControlTask, "motorTask", 4096, NULL, 1, NULL, 0);
}

void loop() {
  // Nothing, AsyncWebServer handles requests
}
