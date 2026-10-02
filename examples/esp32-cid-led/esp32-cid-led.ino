/*
  ESP32: Remocon device discovery, direct CID commands and state publication.
  Wiring: GPIO 4 -> 330 ohm resistor -> LED anode; LED cathode -> GND.
  Uses an external active-high LED; no assumption about onboard LEDs.
  Tutorial: https://iosignal.net/docs/examples/workbench/cid-led
*/
#include <WiFi.h>
#include <IOSignal.h>
#include <string.h>

const char* WIFI_SSID = "WIFI_SSID";
const char* WIFI_PASS = "WIFI_PASS";
const char* IO_HOST = "io.remocon.kr";
const uint16_t IO_PORT = 55488;
const uint8_t LED_PIN = 4;
const char* DEVICE_NAME = "ESP32-LED:Tutorial";

WiFiClient client;
IOSignal io;
bool ledOn = false;
unsigned long lastWifiRetry = 0;

void publishState() { io.signal("@$state", ledOn ? "on" : "off"); }

void onReady() {
  Serial.print("READY CID: "); Serial.println(io.cid);
  io.subscribe("#search");
  io.signal("@$name", DEVICE_NAME);
  io.signal("@$ui", "on,off,toggle");
  publishState();
  io.signal("#notify", io.cid);
}

void onMessage(char* tag, uint8_t type, uint8_t* payload, size_t size) {
  if (strcmp(tag, "#search") == 0) {
    io.signal("#notify", io.cid);
    return;
  }
  if (strcmp(tag, "@") != 0 || type != IOSignal::TEXT || !payload ||
      size == 0 || payload[size - 1] != 0) return;
  const char* command = reinterpret_cast<const char*>(payload);
  if (strcmp(command, "on") == 0) ledOn = true;
  else if (strcmp(command, "off") == 0) ledOn = false;
  else if (strcmp(command, "toggle") == 0) ledOn = !ledOn;
  else return;
  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
  publishState();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  io.setRxBuffer(256);
  io.onReady(onReady);
  io.onMessage(onMessage);
  io.begin(&client, IO_HOST, IO_PORT);
  // Optional: your own device key for the selected server.
  // io.auth("ID_KEY");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    io.loop();
  } else if (millis() - lastWifiRetry >= 10000) {
    lastWifiRetry = millis();
    Serial.println("Waiting for WiFi...");
    WiFi.reconnect();
  }
}
