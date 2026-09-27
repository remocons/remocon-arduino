/*
  WiFi에 연결하고, 앱과 아두이노의 신호를 중계해주는 서버에 연결합니다.
  local 채널 #$switch 를 구독합니다.
  동일한 공유기에 연결된 브라우저로 https://remocon.kr 웹앱 실행후, 공유스위치의 상태를 실시간 수신합니다.

  보드유형: ESP32C3 DEV Module : super mini 기준. 단, 실제 제품 마다 핀번호가 다를수있음.
  gpio2 external hw pullup. POWER_SPI pin.
  gpio8 external hw pullup. LED active low.
  gpio9 external hw pullup. BOOT Button.
  
*/

#include <WiFi.h>
#include <IOSignal.h>

#define LED_PIN 8
#define WIFI_ID  "WIFI_SSID"
#define WIFI_KEY "WIFI_PASS"
#define SERVER_URL "io.remocon.kr"
#define SERVER_PORT 55488
#define SWICH_CH "#$switch"

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

  io.setRxBuffer( 2000 );
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
  io.subscribe( SWICH_CH );
}

void onMessage( char *tag, uint8_t payloadType, uint8_t* payload, size_t payloadSize)
{
  // signal message info
  Serial.print(">> signal tag: " ); Serial.print( tag );
  Serial.print(" type: " ); Serial.print( payloadType );
  Serial.print(" size: " ); Serial.println( payloadSize );

  if( payloadType == IOSignal::PAYLOAD_TYPE::TEXT ){  
    Serial.print("string payload: " ); Serial.println( (char *)payload  );
    if( strcmp( tag, SWICH_CH ) == 0 ){
      if (payload[0] == '1') {
        digitalWrite( LED_PIN, LOW);
      } else {
        digitalWrite( LED_PIN, HIGH);
      }
    }
  }

}
