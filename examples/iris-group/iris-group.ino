// iris ESP32-C3: pin map based on the tested iris-v1a board.
// Tutorial: https://iosignal.net/docs/examples/workbench/iris
#include <FastLED.h>
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

const uint8_t DATA_PIN = 4, STATUS_LED = 8;
const uint8_t BUTTON_PINS[] = {1, 2};
const uint8_t DIP_PINS[] = {21, 20, 10, 7, 6, 5};
CRGB leds[4];
Bounce2::Button buttons[2];
uint8_t groupId = 0, deviceId = 0;
char channel[16], stateTag[24], nameTag[24], uiTag[24];
char states[9] = "00000000";
void applyState() {
  const bool on = states[deviceId] == '1';
  fill_solid(leds, 4, on ? CRGB(100,80,50) : CRGB::Black);
  FastLED.show();
  digitalWrite(STATUS_LED, on ? LOW : HIGH);
}
void publishState() { applyState(); io.signal(stateTag, states); }
void onReady() {
  io.subscribe(stateTag); io.subscribe("#scan/ch");
  io.signal(nameTag, "Iris Lights");
  io.signal(uiTag, "L1,L2,L3,L4,L5,L6,L7,L8");
  io.signal("#detect/ch", channel);
  // Do not overwrite the shared state on reconnect.
}
void onMessage(char* tag, uint8_t type, uint8_t* payload, size_t size) {
  if (strcmp(tag, "#scan/ch") == 0) { io.signal("#detect/ch", channel); return; }
  if (strcmp(tag, stateTag) != 0 || type != IOSignal::TEXT || !payload || size != 9 || payload[8] != 0) return;
  for (uint8_t i = 0; i < 8; i++) if (payload[i] != '0' && payload[i] != '1') return;
  memcpy(states, payload, 9); applyState();
}
void setup() {
  pinMode(STATUS_LED, OUTPUT); digitalWrite(STATUS_LED, HIGH);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, 4); FastLED.clear(true);
  for (uint8_t i = 0; i < 6; i++) pinMode(DIP_PINS[i], INPUT_PULLUP);
  for (uint8_t i = 0; i < 3; i++) {
    groupId = (groupId << 1) | (digitalRead(DIP_PINS[i]) == LOW);
    deviceId = (deviceId << 1) | (digitalRead(DIP_PINS[i+3]) == LOW);
  }
  snprintf(channel, sizeof(channel), "#Iris-g%u", groupId);
  snprintf(stateTag, sizeof(stateTag), "%s$states", channel);
  snprintf(nameTag, sizeof(nameTag), "%s$name", channel);
  snprintf(uiTag, sizeof(uiTag), "%s$ui", channel);
  for (uint8_t i = 0; i < 2; i++) {
    buttons[i].attach(BUTTON_PINS[i], INPUT_PULLUP);
    buttons[i].interval(10); buttons[i].setPressedState(LOW);
  }
  connectNetwork();
}
void loop() {
  pollNetwork();
  for (uint8_t i = 0; i < 2; i++) buttons[i].update();
  if (buttons[0].pressed()) {
    const char next = strcmp(states, "00000000") == 0 ? '1' : '0';
    memset(states, next, 8); publishState();
  }
  if (buttons[1].pressed()) {
    states[deviceId] = states[deviceId] == '1' ? '0' : '1'; publishState();
  }
}
