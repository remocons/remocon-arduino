/*
 *  IOSignal Remocon example: ESP32-C3 shared switch
 *
 *  Joins the shared switch group #switch (remocon/switch3/v1). remocon.kr shows it
 *  as one "공유 스위치" card for every board in the group.
 *    s1 drives the LED on GPIO8 (active low). Signals are printed to serial.
 *
 *  Board: ESP32C3 Dev Module, based on a Super Mini; pins can differ by product.
 *    GPIO2: external pull-up, GPIO8: external pull-up and LED (active low),
 *    GPIO9: external pull-up and BOOT button.
 *
 *  Setup: set WIFI_ID and WIFI_KEY below, upload, then open https://remocon.kr
 *  in a browser on the same router.
 *
 *  [kr] 공유 스위치 그룹(#switch)에 참여해 s1 값으로 GPIO8 LED를 켭니다.
 *
 *  Taeo Lee <sixgen@gmail.com>
 *  https://github.com/remocons/remocon-arduino
 *  MIT License
 */

#include <WiFi.h>
#include <IOSignal.h>
#include <string.h>

#define LED_PIN 8
#define WIFI_ID  "WIFI_SSID"
#define WIFI_KEY "WIFI_PASS"
#define SERVER_URL "io.remocon.kr"
#define SERVER_PORT 55488
#define SWITCH_CH "#switch"
#define SWITCH_STATE SWITCH_CH "$state"
#define SWITCH_INFO "name=공유 스위치;type=remocon/switch3/v1"

WiFiClient client;
IOSignal io;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  
  WiFi.mode( WIFI_STA );
  WiFi.begin( WIFI_ID, WIFI_KEY );
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  io.setRxBuffer( 200 );
  io.begin( &client, SERVER_URL, SERVER_PORT );
  io.onReady( &onReady );
  io.onMessage( &onMessage );
}

void loop() {
  io.loop();
}

void onReady()
{
  Serial.print("onReady cid: ");
  Serial.println( io.cid );
  io.subscribe( "#search" );
  io.subscribe( SWITCH_STATE );
  io.signal( SWITCH_CH "$info", SWITCH_INFO );
  io.signal( "#notify", SWITCH_CH );
}

void onMessage( char *tag, uint8_t payloadType, uint8_t* payload, size_t payloadSize)
{
  // signal message info
  Serial.print(">> signal tag: " ); Serial.print( tag );
  Serial.print(" type: " ); Serial.print( payloadType );
  Serial.print(" size: " ); Serial.println( payloadSize );
  if( strcmp(tag, "#search") == 0 ) io.signal( "#notify", SWITCH_CH );

  if( payloadType == IOSignal::PAYLOAD_TYPE::TEXT && payload && payloadSize && memchr(payload, 0, payloadSize) ){
    Serial.print("string payload: " ); Serial.println( (char *)payload  );
    if( strcmp( tag, SWITCH_STATE ) == 0 ){
      // Read only this device's key, and keep the LED if the key is missing.
      const char* value = strstr( (const char*)payload, "s1=" );
      if( value ) digitalWrite( LED_PIN, value[3] == '1' ? LOW : HIGH );
    }
  }

}
