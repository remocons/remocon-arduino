// iris ESP32-C3 group lights: boards in one group share an 8-way switch (remocon/switch8/v1).
// DIP switches (read at boot, LOW = 1): GPIO21, 20, 10 group id 0-7; GPIO7, 6, 5 device id 0-7.
// Each board lights its four WS2812 LEDs (GPIO4) from switch s<device id + 1>.
// Button GPIO1 turns the whole group on or off, GPIO2 toggles this board. Status LED: GPIO8.
// Pins follow iris board v1a.
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
  WiFi.setTxPower(WIFI_POWER_8_5dBm); // Small ESP32-C3 boards drop sent packets at full power (antenna).
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
char channel[16], stateTag[24], infoTag[24], groupInfo[64];
char states[9] = "00000000";
// Buttons write the whole state, so wait for the stored value. A new group has none.
bool stateKnown = false;
unsigned long readyAt = 0;
void applyState() {
  const bool on = states[deviceId] == '1';
  fill_solid(leds, 4, on ? CRGB(100,80,50) : CRGB::Black);
  FastLED.show();
  digitalWrite(STATUS_LED, on ? LOW : HIGH);
}
// The eight lights share one state: s1=0;s2=1;...
void publishState() {
  applyState();
  char text[40] = "";
  for (uint8_t i = 0; i < 8; i++) {
    char item[6];
    snprintf(item, sizeof(item), "s%u=%c", i + 1, states[i]);
    if (i) strcat(text, ";");
    strcat(text, item);
  }
  io.signal(stateTag, text);
}
void onReady() {
  readyAt = millis();
  io.subscribe(stateTag); io.subscribe("#search");
  io.signal(infoTag, groupInfo);
  io.signal("#notify", channel);
  // Do not overwrite the shared state on reconnect.
}
void onMessage(char* tag, uint8_t type, uint8_t* payload, size_t size) {
  if (strcmp(tag, "#search") == 0) { io.signal("#notify", channel); return; }
  if (strcmp(tag, stateTag) != 0 || type != IOSignal::TEXT || !payload || !size || !memchr(payload, 0, size)) return;
  for (uint8_t i = 0; i < 8; i++) {
    char key[4];
    snprintf(key, sizeof(key), "s%u=", i + 1);
    const char* value = strstr((const char*)payload, key);
    if (value) states[i] = value[3] == '1' ? '1' : '0';
  }
  stateKnown = true;
  applyState();
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
  snprintf(stateTag, sizeof(stateTag), "%s$state", channel);
  snprintf(infoTag, sizeof(infoTag), "%s$info", channel);
  snprintf(groupInfo, sizeof(groupInfo), "name=Iris Lights %u;type=remocon/switch8/v1", groupId);
  for (uint8_t i = 0; i < 2; i++) {
    buttons[i].attach(BUTTON_PINS[i], INPUT_PULLUP);
    buttons[i].interval(10); buttons[i].setPressedState(LOW);
  }
  connectNetwork();
}
void loop() {
  pollNetwork();
  for (uint8_t i = 0; i < 2; i++) buttons[i].update();
  if (!stateKnown && (!readyAt || millis() - readyAt < 2000)) return;
  if (buttons[0].pressed()) {
    const char next = strcmp(states, "00000000") == 0 ? '1' : '0';
    memset(states, next, 8); publishState();
  }
  if (buttons[1].pressed()) {
    states[deviceId] = states[deviceId] == '1' ? '0' : '1'; publishState();
  }
}
