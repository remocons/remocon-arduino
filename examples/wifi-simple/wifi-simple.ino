/*
 *  IOSignal Remocon example: the simplest Wi-Fi client (ESP8266, ESP32)
 *
 *  Joins the shared switch group #switch (switch3.1) and prints the
 *  shared state to serial (115200). No other hardware is needed.
 *  remocon.kr shows the group as one "공유 스위치" card.
 *
 *  Optional LED: uncomment LED_PIN below and s1 drives an active-low LED.
 *    ESP32-C3 Super Mini: GPIO8 is the on-board LED (pins can differ by product).
 *
 *  Setup: upload, then set Wi-Fi over USB on https://test.iosignal.net/wifi/ (the board keeps it),
 *  or write WIFI_ID and WIFI_KEY below. Then open https://remocon.kr on the same router.
 *  The same page can set another IOSignal server and a device key.
 *
 *  [kr] 공유 스위치 그룹(#switch)에 참여해 상태를 시리얼로 출력합니다. 하드웨어 없이 통신만 확인합니다.
 *       LED_PIN 주석을 풀면 s1 값으로 LED(active low)를 켭니다. ESP32-C3 Super Mini는 GPIO8입니다.
 *
 *  Taeo Lee <sixgen@gmail.com>
 *  https://github.com/remocons/remocon-arduino
 *  MIT License
 */

#if !defined(ESP8266) && !defined(ESP32)
 #error "Select an ESP8266 or ESP32 board for this example."
#endif
#include <IOSignal.h>
#include <IOSignalConfig.h>
#include <string.h>
#define WIFI_ID  "WIFI_SSID"
#define WIFI_KEY "WIFI_PASS"
#define SERVER_URL "io.remocon.kr"
#define SERVER_PORT 55488
#define SWITCH_CH "#switch"
#define SWITCH_STATE SWITCH_CH "$state"
// #define LED_PIN 8  // Optional: active-low LED driven by s1 (ESP32-C3 Super Mini on-board LED)

WiFiClient client;
IOSignal io;
IOSignalConfig config;
// Group the web app shows (g= in the info): board settings 'group'. A change is sent at once.
IOSignalSetting group(config, "group", "Group (g=)", "HOME");
void sendInfo() {
  char g[33], info[96];
  snprintf(g, sizeof(g), "%s", group.value());
  for (char* c = g; *c; c++) if (*c == ';') *c = ' ';  // ';' separates info fields
  snprintf(info, sizeof(info), "n=공유 스위치;g=%s;t=switch3.1", g);
  io.signal(SWITCH_CH "$info", info);
}

void setup() {
  Serial.begin(115200);
#ifdef LED_PIN
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
#endif
  // Wi-Fi, server and device key: set over USB, or the sketch values.
  config.begin( Serial, "Wi-Fi Simple", WIFI_ID, WIFI_KEY );
  group.maxLength(32);
  group.onChange([](const char*) { if (io.state == IO_READY) sendInfo(); });
  config.setServer( SERVER_URL, SERVER_PORT );
  config.printServer();
  io.setRxBuffer( 200 );
  io.setLog( [](const char* line) { Serial.println(line); } );
  io.onReady( &onReady );
  io.onMessage( &onMessage );
  if ( config.key() ) io.auth( config.key() ); // a device key set over USB, else anonymous
  io.begin( &client, config.host(), config.port() );
}

void loop() {
  config.loop(); // USB setup requests, Wi-Fi reconnect
  if ( config.connected() ) io.loop();
}

void onReady()
{
  Serial.print("onReady cid: "); Serial.println( io.cid );
  io.subscribe( "#search" );
  io.subscribe( SWITCH_STATE );
  sendInfo();
  io.signal( "#notify", SWITCH_CH );
}

void onMessage( char *tag, uint8_t payloadType, uint8_t* payload, size_t payloadSize)
{
  Serial.print(">> signal tag: " ); Serial.print( tag );
  Serial.print(" type: " ); Serial.print( payloadType );
  Serial.print(" size: " ); Serial.println( payloadSize );
  if( strcmp(tag, "#search") == 0 ) io.signal( "#notify", SWITCH_CH );
  if( payloadType == IOSignal::PAYLOAD_TYPE::TEXT && payload && payloadSize && memchr(payload, 0, payloadSize) ){
    Serial.print("string payload: " ); Serial.println( (char *)payload  );
#ifdef LED_PIN
    if( strcmp( tag, SWITCH_STATE ) == 0 ){
      // Read only this device's key, and keep the LED if the key is missing.
      const char* value = strstr( (const char*)payload, "s1=" );
      if( value ) digitalWrite( LED_PIN, value[3] == '1' ? LOW : HIGH );
    }
#endif
  }
}
