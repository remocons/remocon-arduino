/*
 *  IOSignal Remocon example: Remocon8266 controller for a 4-channel relay board
 *
 *  Controls a board running esp8266_relay4ch by its CID (default "1414", a fixed
 *  CID from that board's device key). The target can change without new firmware:
 *    - this board is a card in the web app, and its card sends "target=<cid>";
 *    - or from a CLI: remocon pub "<controller cid>@" "target=2001".
 *    The new CID is kept in flash and used after a reboot.
 *  Buttons SW1-SW4 toggle relays 1-4, and a button on D6 (GPIO12, to GND) turns
 *  all relays off, or all on when every relay is off. The OLED shows the target
 *  CID and the four relay states. Needs IOSignal 9.6.0 or later (unsubscribe()).
 *
 *  Setup: put Wi-Fi in _secrets.h, or edit WIFI_SSID and WIFI_PASS below. Use a
 *  device key different from the relay board's (SECRET_CONTROL_ID_KEY).
 *
 *  [kr] CID로 4채널 릴레이 보드를 제어합니다. 버튼 4개, D6 전체 버튼, OLED 상태 표시. 대상 CID는 앱이나 CLI로 바꿉니다.
 *
 *  Taeo Lee <sixgen@gmail.com>
 *  https://github.com/remocons/remocon-arduino
 *  MIT License
 */

#include <ESP8266WiFiMulti.h>
#include <EEPROM.h>
#include <IOSignal.h>
#include <Bounce2.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <string.h>

// Wi-Fi values (and an optional device key) can go in _secrets.h next to this sketch,
// which keeps them out of version control. Without it, edit WIFI_SSID and WIFI_PASS below.
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

const char* DEFAULT_TARGET = "1414"; // CID of the relay board until the card sets another one.

// The card: a CID input and the relay states. The "text" widget needs a web app that knows it.
// An app that does not falls back to the ui buttons, which toggle the relays through this board.
const char* deviceInfo =
  "{\"name\":\"Relay Control\",\"ui\":\"r1,r2,r3,r4,all\",\"schema\":{\"schema\":1,\"title\":\"Relay Control\",\"controls\":["
  "{\"widget\":\"text\",\"label\":\"Target CID\",\"key\":\"target\",\"cmd\":\"target={v}\",\"maxLength\":12},"
  "{\"widget\":\"value\",\"label\":\"Relays 1-4\",\"key\":\"relays\"},"
  "{\"widget\":\"value\",\"label\":\"Connected\",\"key\":\"link\"},"
  "{\"widget\":\"buttons\",\"label\":\"Toggle\",\"options\":[\"r1\",\"r2\",\"r3\",\"r4\",\"all\"],\"cmd\":\"{v}\"}"
  "]}}";

ESP8266WiFiMulti wifiMulti;
WiFiClient client;
IOSignal io;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
Bounce2::Button buttons[4];
Bounce2::Button allButton;

char target[MAX_CID_LEN + 1] = ""; // CID of the relay board.
char states[5] = "0000";           // Last relay states reported by the target.
bool stateKnown = false;           // True after the target sent its state, false after it closed.

// Flash keeps the target CID: a marker byte, then the CID with its 0.
const uint8_t EEPROM_MARK = 0xA5;
void loadTarget() {
  EEPROM.begin(32);
  bool ok = EEPROM.read(0) == EEPROM_MARK;
  for (uint8_t i = 0; ok && i <= MAX_CID_LEN; i++) {
    target[i] = EEPROM.read(1 + i);
    if (!target[i]) break;
    if (i == MAX_CID_LEN) ok = false;
  }
  if (!ok || !target[0]) strcpy(target, DEFAULT_TARGET);
}
void saveTarget() {
  EEPROM.write(0, EEPROM_MARK);
  for (uint8_t i = 0; i <= strlen(target); i++) EEPROM.write(1 + i, target[i]);
  EEPROM.commit();
}

// A CID is 1-12 characters without spaces or the tag characters @ # $ ; =.
bool validCid(const char* cid) {
  const size_t n = strlen(cid);
  if (!n || n > MAX_CID_LEN) return false;
  for (size_t i = 0; i < n; i++)
    if (cid[i] <= ' ' || cid[i] > '~' || strchr("@#$;=", cid[i])) return false;
  return true;
}

// OLED: target line, then four boxes. A filled box is a relay that is on.
void showState() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_t0_13_mr);
  u8g2.setFontPosTop();
  u8g2.setCursor(0, 0);
  if (io.state != IO_READY) u8g2.print("Connecting..");
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
  u8g2.setCursor(0, 53);
  u8g2.print(stateKnown ? states : "----");
  u8g2.sendBuffer();
}

// The controller's own state for its card: target=1414;relays=0100;link=1
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

void setTarget(const char* cid) {
  if (strcmp(cid, target) != 0) {
    char sub[MAX_CID_LEN + 8];
    snprintf(sub, sizeof(sub), "%s@$state", target);
    io.unsubscribe(sub);
    strcpy(target, cid);
    saveTarget();
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
  io.signal("@$info", deviceInfo);
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
    if (strncmp(text, "target=", 7) == 0 && validCid(text + 7)) setTarget(text + 7);
    else if (text[0] == 'r' && text[1] >= '1' && text[1] <= '4' && !text[2]) toggleRelay(text[1] - '1');
    else if (strcmp(text, "all") == 0) toggleAll();
    else publishState(); // Unknown commands still get a reply.
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Remocon8266 relay control start");
  loadTarget();
  Serial.print("TARGET ");
  Serial.println(target);

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

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASS);
  Serial.print("Wait for WiFi... ");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  io.setRxBuffer(200);
  io.onReady(&onReady);
  io.onMessage(&onMessage);
  io.begin(&client, "io.remocon.kr", 55488);
  // Use a key of its own: the same key as the relay board would log one of them out.
#ifdef SECRET_ID_KEY
  io.auth(SECRET_ID_KEY);
#endif
}

void loop() {
  io.loop();
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
