// ESP32-C3 Super Mini: onboard LED GPIO8 (active LOW), BOOT button GPIO9.
// Tutorial: https://iosignal.net/docs/examples/workbench/c3
#include <WiFi.h>
#include <IOSignal.h>
#include <Bounce2.h>
#include <string.h>
const char* WIFI_SSID = "WIFI_SSID";
const char* WIFI_PASS = "WIFI_PASS";
WiFiClient client;
IOSignal io;
unsigned long wifiRetry = 0;
void connectNetwork() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  io.setRxBuffer(256);
  io.onReady(onReady);
  io.onMessage(onMessage);
  io.begin(&client, "io.remocon.kr", 55488);
  // io.auth("ID_KEY"); // Optional: your own key for this server.
}
void pollNetwork() {
  if (WiFi.status() == WL_CONNECTED) io.loop();
  else if (millis() - wifiRetry >= 10000) { wifiRetry = millis(); WiFi.reconnect(); }
}

const uint8_t LED_PIN = 8, BUTTON_PIN = 9;
Bounce2::Button button;
bool ledOn = false;
void onReady() { io.subscribe("#homeButton"); }
void onMessage(char* tag, uint8_t type, uint8_t* payload, size_t size) {
  if (strcmp(tag, "#homeButton") != 0) return;
  ledOn = !ledOn;
  digitalWrite(LED_PIN, ledOn ? LOW : HIGH);
}
void setup() {
  pinMode(LED_PIN, OUTPUT); digitalWrite(LED_PIN, HIGH);
  button.attach(BUTTON_PIN, INPUT_PULLUP); button.interval(10); button.setPressedState(LOW);
  connectNetwork();
}
void loop() {
  pollNetwork(); button.update();
  if (button.pressed()) io.signal("#homeButton", "button");
}
