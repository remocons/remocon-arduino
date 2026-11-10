/*
  ESP32: Remocon group channel control.
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

// Keep the channel identical on the two boards. The web app finds the group through #search
// and rewrites the shared state; each board reads its own key (s1 on the first, s2 on the second).
#define GROUP_CHANNEL "#lab"
const char* GROUP_INFO_TAG = GROUP_CHANNEL "$info";
const char* GROUP_STATE_TAG = GROUP_CHANNEL "$state";
const char* GROUP_INFO = "name=ESP32 LEDs;group=Tutorial;type=remocon/switch2/v1";
const char STATE_KEY[] = { 's', char('1' + OUTPUT_INDEX), '=', 0 };

WiFiClient client;
IOSignal io;
bool ledOn = false;
unsigned long lastWifiRetry = 0;

void onReady() {
  Serial.print("READY CID: "); Serial.println(io.cid);
  io.subscribe("#search");
  io.subscribe(GROUP_STATE_TAG);
  io.signal(GROUP_INFO_TAG, GROUP_INFO);
  io.signal("#notify", GROUP_CHANNEL);
  // Do not overwrite the shared state on boot: the retained value arrives with the subscription.
}

void onMessage(char* tag, uint8_t type, uint8_t* payload, size_t size) {
  if (strcmp(tag, "#search") == 0) {
    io.signal("#notify", GROUP_CHANNEL);
    return;
  }
  if (strcmp(tag, GROUP_STATE_TAG) != 0 || type != IOSignal::TEXT ||
      !payload || !size || !memchr(payload, 0, size)) return;
  const char* value = strstr((const char*)payload, STATE_KEY);
  if (!value) return;
  ledOn = value[3] == '1';
  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
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
