/*
  ESP32: Remocon device discovery, direct CID commands and state publication.
  Wiring: GPIO 4 -> 330 ohm resistor -> LED anode; LED cathode -> GND.
  Uses an external active-high LED; no assumption about onboard LEDs.
  Wi-Fi: leave WIFI_SSID as it is and set it from https://test.iosignal.net/wifi/ over USB,
  or write your SSID and password below (then the sketch values are used at every boot).
  The same page can set another IOSignal server (host, port) and a device key (id.key);
  those stored values win over IO_HOST, IO_PORT below. Without a key the board is anonymous.
*/
#include <IOSignal.h>
#include <IOSignalConfig.h>
#include <string.h>

const char* WIFI_SSID = "WIFI_SSID";
const char* WIFI_PASS = "WIFI_PASS";
const char* IO_HOST = "io.remocon.kr";
const uint16_t IO_PORT = 55488;
const uint8_t LED_PIN = 4;

WiFiClient client;
IOSignal io;
IOSignalConfig config;
// Group the web app shows (g= in the info): board settings 'group'. A change is sent at once.
IOSignalSetting group(config, "group", "Group (g=)", "HOME");
void sendInfo() {
  char g[33], info[96];
  snprintf(g, sizeof(g), "%s", group.value());
  for (char* c = g; *c; c++) if (*c == ';') *c = ' ';  // ';' separates info fields
  snprintf(info, sizeof(info), "n=ESP32-LED;g=%s;ui=on,off,toggle", g);
  io.signal("@$info", info);
}
bool ledOn = false;

void publishState() { io.signal("@$state", ledOn ? "on=1" : "on=0"); }

void onReady() {
  Serial.print("READY CID: "); Serial.println(io.cid);
  io.subscribe("#search");
  sendInfo();
  publishState();
  io.signal("#notify", io.cid);
}

void onMessage(char* tag, uint8_t type, uint8_t* payload, size_t size) {
  if (strcmp(tag, "#search") == 0) {
    io.signal("#notify", io.cid);
    return;
  }
  if (strcmp(tag, "@") != 0) return;
  // Only NUL-terminated TEXT is read as a command; anything else counts as unknown.
  const bool text = type == IOSignal::TEXT && payload && size > 0 && payload[size - 1] == 0;
  const char* command = text ? reinterpret_cast<const char*>(payload) : "";
  if (strcmp(command, "on") == 0) ledOn = true;
  else if (strcmp(command, "off") == 0) ledOn = false;
  else if (strcmp(command, "toggle") == 0) ledOn = !ledOn;
  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
  publishState(); // Sent for every command, unknown ones too: the app waits for it.
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  config.begin(Serial, "ESP32-LED", WIFI_SSID, WIFI_PASS);
  group.maxLength(32);
  group.onChange([](const char*) { if (io.state == IO_READY) sendInfo(); });
  config.setServer(IO_HOST, IO_PORT);
  config.printServer();
  io.setRxBuffer(256);
  io.setLog([](const char* line) { Serial.println(line); }); // e.g. no reply from a wrong port
  io.onReady(onReady);
  io.onMessage(onMessage);
  if (config.key()) io.auth(config.key()); // device key set over USB, or anonymous
  io.begin(&client, config.host(), config.port());
}

void loop() {
  config.loop(); // browser Wi-Fi setup over USB, reconnect while offline
  if (config.connected()) io.loop();
}
