/*
 *  IOSignal Remocon example: Remocon8266 controller for a 4-channel relay board
 *
 *  Controls a board running esp8266_relay4ch by its CID (DEFAULT_TARGET, a fixed
 *  CID from that board's device key). The target can change without new firmware:
 *    - this board is a card in the web app, and its card sends "target=<cid>";
 *    - or from a CLI: remocon pub "<controller cid>@" "target=2001";
 *    - or over USB on the board settings page (https://test.iosignal.net/wifi/).
 *    It is the "target" setting (IOSignalSetting): kept in flash and used after a reboot.
 *    Clearing it on the settings page brings back DEFAULT_TARGET.
 *  Buttons SW1-SW4 toggle relays 1-4, and a button on D6 (GPIO12, to GND) turns
 *  all relays off, or all on when every relay is off. The OLED shows the target
 *  CID, the four relay states and this board's CID. Needs an IOSignal release with IOSignalConfig.h (after 9.12.0).
 *
 *  Wi-Fi: leave WIFI_SSID as it is and set it from https://test.iosignal.net/wifi/ over USB
 *  (the board keeps it), or put it in _secrets.h / edit WIFI_SSID and WIFI_PASS below (then the
 *  sketch values are used at every boot). The same page can set another IOSignal server and a
 *  device key (id.key); those stored values win over the sketch ones. Use a device key different
 *  from the relay board's (set it over USB, or SECRET_ID_KEY in this folder's _secrets.h).
 *
 *  [kr] CID로 4채널 릴레이 보드를 제어합니다. 버튼 4개, D6 전체 버튼, OLED 상태 표시. 대상 CID는 앱이나 CLI로 바꿉니다.
 *  Wi-Fi는 USB로 연결해 https://test.iosignal.net/wifi/ 에서 넣을 수 있습니다.
 *
 *  Taeo Lee <sixgen@gmail.com>
 *  https://github.com/remocons/remocon-arduino
 *  MIT License
 */

#include <EEPROM.h>   // only to move a target saved by an older version of this sketch
#include <IOSignal.h>
#include <IOSignalConfig.h>
#include <Bounce2.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <string.h>

// Wi-Fi values (and an optional device key) can go in _secrets.h next to this sketch,
// which keeps them out of version control. Leave the placeholders to use the Wi-Fi set over USB.
#if __has_include("_secrets.h")
#include "_secrets.h"
#endif
#ifndef SECRET_WIFI_SSID
#define SECRET_WIFI_SSID "WIFI_SSID"
#define SECRET_WIFI_PASS "WIFI_PASS"
#endif
const char* WIFI_SSID = SECRET_WIFI_SSID;
const char* WIFI_PASS = SECRET_WIFI_PASS;

// Buttons, in relay order: SW1 -> relay 1 ... SW4 -> relay 4.
#define BTN_UP     16   // D0  SW1 *EXTERNAL HARDWARE PULLUP
#define BTN_LEFT    0   // D3  SW2
#define BTN_DOWN   14   // D5  SW3
#define BTN_RIGHT  13   // D7  SW4
const uint8_t BUTTON_PINS[4] = { BTN_UP, BTN_LEFT, BTN_DOWN, BTN_RIGHT };
// External button between D6 and GND: all relays on, or all off when any is on.
#define BTN_ALL    12   // D6  (OUT2 on the board), internal pull-up

// I2C OLED: SDA GPIO4 (D2), SCL GPIO5 (D1).

const char* DEFAULT_TARGET = "the-cid"; // CID of the relay board until the card sets another one.

// The card: a CID input and the relay states, kept in <cid>@$panel ("p=1" in the info).
// The "text" widget needs a web app that knows it. An app that does not falls back
// to the ui buttons, which toggle the relays through this board.
const char* devicePanel =
  "{\"schema\":1,\"title\":\"Relay Control\",\"controls\":["
  "{\"widget\":\"text\",\"label\":\"Target CID\",\"key\":\"target\",\"cmd\":\"target={v}\",\"maxLength\":12},"
  "{\"widget\":\"value\",\"label\":\"Relays 1-4\",\"key\":\"relays\"},"
  "{\"widget\":\"value\",\"label\":\"Connected\",\"key\":\"link\"},"
  "{\"widget\":\"buttons\",\"label\":\"Toggle\",\"options\":[\"r1\",\"r2\",\"r3\",\"r4\",\"all\"],\"cmd\":\"{v}\"}"
  "]}";

WiFiClient client;
IOSignal io;
IOSignalConfig config;
// Group the web app shows (g= in the info): board settings 'group'. A change is sent at once.
IOSignalSetting group(config, "group", "Group (g=)", "HOME");
void sendInfo() {
  char g[33], info[96];
  snprintf(g, sizeof(g), "%s", group.value());
  for (char* c = g; *c; c++) if (*c == ';') *c = ' ';  // ';' separates info fields
  snprintf(info, sizeof(info), "n=Relay Control;g=%s;ui=r1,r2,r3,r4,all;p=1", g);
  io.signal("@$info", info);
}
// The relay board's CID: a setting the browser shows and changes, and a remote command may change.
IOSignalSetting targetSetting(config, "target", "Target CID", DEFAULT_TARGET, IOSignalSetting::CID, IOSignalSetting::REMOTE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
Bounce2::Button buttons[4];
Bounce2::Button allButton;

char target[MAX_CID_LEN + 1] = ""; // CID being watched (the target setting once connected).
char states[5] = "0000";           // Last relay states reported by the target.
bool stateKnown = false;           // True after the target sent its state, false after it closed.

// Older versions kept the target in EEPROM: a marker byte 0xA5, then the CID with its 0.
// Moved once into the target setting, then the marker is cleared so a later reset to the
// default is not undone. Remove this when no board runs the older version.
void moveOldTarget() {
  EEPROM.begin(32);
  if (EEPROM.read(0) != 0xA5) { EEPROM.end(); return; }
  char old[MAX_CID_LEN + 1];
  uint8_t i = 0;
  for (; i <= MAX_CID_LEN; i++) {
    old[i] = EEPROM.read(1 + i);
    if (!old[i]) break;
  }
  if (i <= MAX_CID_LEN && !targetSetting.isStored() && targetSetting.set(old)) {
    Serial.print("TARGET moved from EEPROM: ");
    Serial.println(old);
  }
  EEPROM.write(0, 0);
  EEPROM.end();  // commits
}

// OLED: target line, four boxes (a filled box is a relay that is on), then this board's CID.
void showState() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_t0_13_mr);
  u8g2.setFontPosTop();
  u8g2.setCursor(0, 0);
  if (!config.connected()) u8g2.print("Wi-Fi..");
  else if (io.state != IO_READY) u8g2.print("Connecting..");
  else {
    u8g2.print("Relay ");
    u8g2.print(target);
    if (!stateKnown) u8g2.print(" ?");
  }
  for (uint8_t i = 0; i < 4; i++) {
    const uint8_t x = 4 + i * 31, y = 20, w = 26, h = 30;
    char label[2] = { char('1' + i), 0 };
    if (stateKnown && states[i] == '1') {
      u8g2.drawBox(x, y, w, h);
      u8g2.setDrawColor(0);
      u8g2.drawStr(x + 10, y + 9, label);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawFrame(x, y, w, h);
      u8g2.drawStr(x + 10, y + 9, stateKnown ? label : "?");
    }
  }
  if (io.state == IO_READY) {
    u8g2.setCursor(0, 53);
    u8g2.print("CID ");
    u8g2.print(io.cid);
  }
  u8g2.sendBuffer();
}

// The controller's own state for its card: target=the-cid;relays=0100;link=1
void publishState() {
  char text[48];
  snprintf(text, sizeof(text), "target=%s;relays=%s;link=%c", target, stateKnown ? states : "----", stateKnown ? '1' : '0');
  io.signal("@$state", text);
}

void watchTarget() {
  char sub[MAX_CID_LEN + 8];
  snprintf(sub, sizeof(sub), "%s@$state", target);
  io.subscribe(sub); // The server sends the kept state at once, then every change.
}

// The target setting changed (card, CLI or browser): watch the new board.
void setTarget(const char* cid) {
  if (strcmp(cid, target) != 0) {
    char sub[MAX_CID_LEN + 8];
    snprintf(sub, sizeof(sub), "%s@$state", target);
    io.unsubscribe(sub);
    strcpy(target, cid);
    stateKnown = false;
    Serial.print("TARGET ");
    Serial.println(target);
    watchTarget();
  }
  showState();
  publishState();
}

void onReady() {
  Serial.print("onReady cid: ");
  Serial.println(io.cid);
  stateKnown = false;
  io.subscribe("#search");
  io.signal("@$panel", devicePanel); // Before the info, so it is kept when the app sees p=1.
  sendInfo();
  publishState();
  io.signal("#notify", io.cid);
  watchTarget();
  showState();
}

// Sends r1=1 style commands. The relay board answers with its new state.
void toggleRelay(uint8_t i) {
  if (!stateKnown) return; // The opposite of an unknown state is unknown.
  char command[5] = { 'r', char('1' + i), '=', states[i] == '1' ? '0' : '1', 0 };
  io.signal2(target, "@", command);
  Serial.print("COMMAND ");
  Serial.println(command);
}

// all=1 when every relay is off, otherwise all=0. The relay board handles both in one command.
void toggleAll() {
  if (!stateKnown) return;
  const char* command = strcmp(states, "0000") == 0 ? "all=1" : "all=0";
  io.signal2(target, "@", command);
  Serial.print("COMMAND ");
  Serial.println(command);
}

void onMessage(char* tag, uint8_t type, uint8_t* payload, size_t size) {
  if (strcmp(tag, "#search") == 0) { io.signal("#notify", io.cid); return; }
  if (type != IOSignal::TEXT || !payload || !size || !memchr(payload, 0, size)) return;
  const char* text = (const char*)payload;

  // State of the target: <target>@$state r1=0;r2=1;r3=0;r4=0, or close when it left.
  const size_t n = strlen(target);
  if (strncmp(tag, target, n) == 0 && strcmp(tag + n, "@$state") == 0) {
    if (strcmp(text, "close") == 0) {
      Serial.println("TARGET closed");
      stateKnown = false;
    } else {
      for (uint8_t i = 0; i < 4; i++) {
        char key[4];
        snprintf(key, sizeof(key), "r%u=", i + 1);
        const char* value = strstr(text, key);
        if (value) states[i] = value[3] == '1' ? '1' : '0';
      }
      stateKnown = true;
      Serial.print("STATE ");
      Serial.println(text);
    }
    showState();
    publishState();
    return;
  }

  // Commands to this controller: target=<cid>, or r1-r4 to toggle a relay.
  if (strcmp(tag, "@") == 0) {
    Serial.print("CMD ");
    Serial.println(text);
    if (strncmp(text, "target=", 7) == 0) {
      // Checked and saved by the setting; setTarget() runs through onChange.
      // A bad or unchanged CID still gets the state as the reply.
      if (!targetSetting.setFromRemote(text + 7) || strcmp(target, text + 7) == 0) publishState();
    }
    else if (text[0] == 'r' && text[1] >= '1' && text[1] <= '4' && !text[2]) toggleRelay(text[1] - '1');
    else if (strcmp(text, "all") == 0) toggleAll();
    else publishState(); // Unknown commands still get a reply.
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Remocon8266 relay control start");

  for (uint8_t i = 0; i < 4; i++) {
    // GPIO16 has an external pull-up and no internal one.
    buttons[i].attach(BUTTON_PINS[i], BUTTON_PINS[i] == BTN_UP ? INPUT : INPUT_PULLUP);
    buttons[i].interval(5);
    buttons[i].setPressedState(LOW);
  }
  allButton.attach(BTN_ALL, INPUT_PULLUP);
  allButton.interval(10);
  allButton.setPressedState(LOW);

  u8g2.begin();
  showState();

  // Stored Wi-Fi (set over USB), or the sketch values when they are not the placeholders.
  config.begin(Serial, "Relay Control", WIFI_SSID, WIFI_PASS);
  group.maxLength(32);
  group.onChange([](const char*) { if (io.state == IO_READY) sendInfo(); });
  config.setServer("io.remocon.kr", 55488);
#ifdef SECRET_ID_KEY
  config.setKey(SECRET_ID_KEY);
#endif
  config.printServer();
  moveOldTarget();
  strcpy(target, targetSetting.value());
  targetSetting.onChange(setTarget);
  Serial.print("TARGET ");
  Serial.println(target);

  io.setRxBuffer(200);
  io.setLog([](const char* line) { Serial.println(line); }); // e.g. no reply from a wrong port
  io.onReady(&onReady);
  io.onMessage(&onMessage);
  // Use a key of its own: the same key as the relay board would log one of them out.
  // Stored over USB first, then SECRET_ID_KEY, else anonymous.
  if (config.key()) io.auth(config.key());
  io.begin(&client, config.host(), config.port());
}

bool wasConnected = false;

void loop() {
  config.loop(); // Wi-Fi setup over USB, reconnect while offline
  const bool connected = config.connected();
  if (connected != wasConnected) {
    wasConnected = connected;
    if (connected) { Serial.print("IP: "); Serial.println(WiFi.localIP()); }
    showState();
  }
  if (connected) io.loop();
  for (uint8_t i = 0; i < 4; i++) {
    buttons[i].update();
    if (buttons[i].pressed()) {
      Serial.print("BUTTON ");
      Serial.println(i + 1);
      toggleRelay(i);
    }
  }
  allButton.update();
  if (allButton.pressed()) {
    Serial.println("BUTTON all");
    toggleAll();
  }
}
