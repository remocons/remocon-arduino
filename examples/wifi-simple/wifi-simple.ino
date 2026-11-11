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
 *  Setup: set WIFI_ID and WIFI_KEY below, upload, then open https://remocon.kr
 *  in a browser on the same router.
 *
 *  [kr] 공유 스위치 그룹(#switch)에 참여해 상태를 시리얼로 출력합니다. 하드웨어 없이 통신만 확인합니다.
 *       LED_PIN 주석을 풀면 s1 값으로 LED(active low)를 켭니다. ESP32-C3 Super Mini는 GPIO8입니다.
 *
 *  Taeo Lee <sixgen@gmail.com>
 *  https://github.com/remocons/remocon-arduino
 *  MIT License
 */

#if defined(ESP8266)
 #include <ESP8266WiFi.h>
#elif defined(ESP32)
 #include <WiFi.h>
#else
 #error "Select an ESP8266 or ESP32 board for this example."
#endif
#include <IOSignal.h>
#include <string.h>
#define WIFI_ID  "WIFI_SSID"
#define WIFI_KEY "WIFI_PASS"
#define SERVER_URL "io.remocon.kr"
#define SERVER_PORT 55488
#define SWITCH_CH "#switch"
#define SWITCH_STATE SWITCH_CH "$state"
#define SWITCH_INFO "n=공유 스위치;t=switch3.1"
// #define LED_PIN 8  // Optional: active-low LED driven by s1 (ESP32-C3 Super Mini on-board LED)

WiFiClient client;
IOSignal io;

void setup() {
  Serial.begin(115200);
#ifdef LED_PIN
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
#endif
  WiFi.mode( WIFI_STA );
  WiFi.begin( WIFI_ID, WIFI_KEY );
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  io.setRxBuffer( 200 );
  io.begin( &client, SERVER_URL, SERVER_PORT );
  io.onReady( &onReady );
  io.onMessage( &onMessage );
  // io.auth("ID_KEY"); // Optional: use your own device key.
}

void loop() { io.loop(); }

void onReady()
{
  Serial.print("onReady cid: "); Serial.println( io.cid );
  io.subscribe( "#search" );
  io.subscribe( SWITCH_STATE );
  io.signal( SWITCH_CH "$info", SWITCH_INFO );
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
