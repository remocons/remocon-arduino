/*
  ESP32: Remocon group channel control, compatible with test_iosignal_net.
  Wiring: GPIO 4 -> 330 ohm resistor -> LED anode; LED cathode -> GND.
  Upload to two boards: OUTPUT_INDEX=0 on the first, 1 on the second.
  Tutorial: https://iosignal.net/docs/examples/workbench/group-led
*/
#include <WiFi.h>
#include <IOSignal.h>
#include <string.h>

const char* WIFI_SSID = "WIFI_SSID";
const char* WIFI_PASS = "WIFI_PASS";
const char* IO_HOST = "io.remocon.kr";
const uint16_t IO_PORT = 55488;
const uint8_t LED_PIN = 4;
const uint8_t OUTPUT_INDEX = 0; // Change to 1 on the second board.
static_assert(OUTPUT_INDEX < 2, "OUTPUT_INDEX must be 0 or 1");

// Keep these tags identical on the two boards and the web control preset.
#define GROUP_CHANNEL "#lab"
const char* GROUP_NAME_TAG = GROUP_CHANNEL "$name";
const char* GROUP_UI_TAG = GROUP_CHANNEL "$ui";
const char* GROUP_STATES_TAG = GROUP_CHANNEL "$states";

WiFiClient client;
IOSignal io;
bool ledOn = false;
unsigned long lastWifiRetry = 0;

void onReady() {
  Serial.print("READY CID: "); Serial.println(io.cid);
  io.subscribe("#scan/ch");
  io.subscribe(GROUP_STATES_TAG);
  io.signal(GROUP_NAME_TAG, "ESP32 LEDs:Tutorial");
  io.signal(GROUP_UI_TAG, "LED 1,LED 2");
  io.signal("@$state", ledOn ? "on" : "off");
  io.signal("#detect/ch", GROUP_CHANNEL);
  // Do not overwrite the shared setting on boot. If retained, the server
  // provides it on subscription according to the service's retention policy.
}

void onMessage(char* tag, uint8_t type, uint8_t* payload, size_t size) {
  if (strcmp(tag, "#scan/ch") == 0) {
    io.signal("#detect/ch", GROUP_CHANNEL);
    return;
  }
  if (strcmp(tag, GROUP_STATES_TAG) != 0 || type != IOSignal::TEXT ||
      !payload || size != 3 || payload[2] != 0) return;
  if ((payload[0] != '0' && payload[0] != '1') ||
      (payload[1] != '0' && payload[1] != '1')) return;
  ledOn = payload[OUTPUT_INDEX] == '1';
  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
  io.signal("@$state", ledOn ? "on" : "off");
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
