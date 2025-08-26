#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WebServer.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define BATTERY_PIN 34  // Voltage divider to read battery
#define MAX_BATTERY_V 4.2
#define MIN_BATTERY_V 3.3

Adafruit_MPU6050 mpu;

enum MotorMode { STOP = 0, FORWARD, BACKWARD, LEFT, RIGHT };
struct struct_message { MotorMode motorMode; } outgoingData;

uint8_t carAddress[] = {0xA0, 0xB7, 0x65, 0x0E, 0x50, 0x40};

MotorMode lastGesture = STOP;
MotorMode stableGesture = STOP;
unsigned long lastSendTime = 0;
unsigned long lastStopTime = 0;
unsigned long gestureStableStart = 0;
bool lastSendOK = true;

// ---------- Gesture Detection ----------
MotorMode detectGesture() {
  sensors_event_t a, g, temp;
  if (!mpu.getEvent(&a, &g, &temp)) return lastGesture;

  // Invert axis to fix orientation
  float ax = -a.acceleration.x;
  float ay = -a.acceleration.y;

  if (ay > 4) return FORWARD;
  if (ay < -4) return BACKWARD;
  if (ax > 4) return RIGHT;
  if (ax < -4) return LEFT;
  return STOP;
}

// ---------- Battery Percentage with Averaging ----------
float getBatteryPercent() {
  long sum = 0;
  for (int i = 0; i < 20; i++) {  // Average 20 samples
    sum += analogRead(BATTERY_PIN);
    delayMicroseconds(200);
  }
  float raw = sum / 20.0;
  float voltage = (raw / 4095.0) * 3.3 * 2.0; // Divider ×2
  float percent = (voltage - MIN_BATTERY_V) / (MAX_BATTERY_V - MIN_BATTERY_V) * 100.0;
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  return percent;
}

// ---------- OLED Update ----------
void updateOLED() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // --- Top Line: Battery ---
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Battery:");

  int batPercent = (int)getBatteryPercent();
  display.print(batPercent);
  display.print("%");

  // Battery bar (0-100%)
  int barWidth = map(batPercent, 0, 100, 0, 40);
  display.drawRect(80, 0, 42, 8, SSD1306_WHITE);
  display.fillRect(81, 1, barWidth, 6, SSD1306_WHITE);

  // --- Center: Gesture ---
  display.setTextSize(2);
  String gestureText;
  switch (stableGesture) {
    case FORWARD: gestureText = "Forward"; break;
    case BACKWARD: gestureText = "Back"; break;
    case LEFT: gestureText = "Left"; break;
    case RIGHT: gestureText = "Right"; break;
    default: gestureText = "STOP"; break;
  }

  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(gestureText, 0, 0, &x1, &y1, &w, &h);
  int x = (SCREEN_WIDTH - w) / 2;
  display.setCursor(x, 24);
  display.print(gestureText);

  // --- Bottom: Send status ---
  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print("Send: ");
  display.print(lastSendOK ? "OK" : "Fail");

  display.display();
}

// ---------- ESP-NOW Callback ----------
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  lastSendOK = (status == ESP_NOW_SEND_SUCCESS);
  Serial.print("[ESP-NOW] ");
  Serial.println(lastSendOK ? "OK" : "Fail");
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    while(1);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Booting...");
  display.display();

  // MPU6050
  if (!mpu.begin()) {
    Serial.println("[MPU] Init failed!");
    display.setCursor(0,10);
    display.println("MPU FAIL");
    display.display();
    while (1);
  }
  Serial.println("[MPU] Ready");

  // ESP-NOW
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);  
  esp_wifi_set_ps(WIFI_PS_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ESP-NOW] Init failed!");
    while (1);
  }
  esp_now_register_send_cb(onDataSent);

  esp_now_peer_info_t peerInfo{};
  memcpy(peerInfo.peer_addr, carAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  Serial.println("[ESP-NOW] Ready");
}

void loop() {
  MotorMode gesture = detectGesture();
  unsigned long now = millis();

  // --- Stable gesture detection ---
  if (gesture != stableGesture) {
    stableGesture = gesture;
    gestureStableStart = now;
  }

  if (now - gestureStableStart < 200) return; // wait 200ms for stability

  // --- Debounce STOP ---
  if (stableGesture == STOP && now - lastStopTime < 500) return;

  // Send only if changed or 300ms passed
  if (stableGesture != lastGesture || now - lastSendTime > 300) {
    lastGesture = stableGesture;
    lastSendTime = now;
    if (stableGesture == STOP) lastStopTime = now;

    outgoingData.motorMode = stableGesture;
    esp_err_t result = esp_now_send(carAddress, (uint8_t*)&outgoingData, sizeof(o*_
