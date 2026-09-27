/*
  WiFi에 연결하고, 앱과 아두이노의 신호를 중계해주는 서버에 연결후 local #$switch 채널을 구독합니다.
  동일한 공유기에 연결된 브라우저로 https://remocon.kr 웹앱의 공유스위치 상태를 실시간 수신합니다.
*/

#if defined(ESP8266)
 #include <ESP8266WiFi.h>
#elif defined(ESP32)
 #include <WiFi.h>
#endif
#include <IOSignal.h>
#define WIFI_ID  "twesomego" //"WIFI_SSID"
#define WIFI_KEY "qwer1234" //"WIFI_PASS"
#define SERVER_URL "io.remocon.kr"
#define SERVER_PORT 55488
#define SWICH_CH "#$switch"

WiFiClient client;
IOSignal io;

void setup() {
  Serial.begin(115200);
  WiFi.mode( WIFI_STA );
  WiFi.begin( WIFI_ID, WIFI_KEY );
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  io.setRxBuffer( 200 );
  io.begin( &client, SERVER_URL, SERVER_PORT );
  io.onReady( &onReady );
  io.onMessage( &onMessage );
  // io.auth( "three.ZTBmfEkKfhJJK9Oine");
}

void loop() { io.loop(); }

void onReady()
{
  Serial.print("onReady cid: "); Serial.println( io.cid );
  io.subscribe( SWICH_CH );
}

void onMessage( char *tag, uint8_t payloadType, uint8_t* payload, size_t payloadSize)
{
  Serial.print(">> signal tag: " ); Serial.print( tag );
  Serial.print(" type: " ); Serial.print( payloadType );
  Serial.print(" size: " ); Serial.println( payloadSize );
  if( payloadType == IOSignal::PAYLOAD_TYPE::TEXT ){  
    Serial.print("string payload: " ); Serial.println( (char *)payload  );
  }
}
