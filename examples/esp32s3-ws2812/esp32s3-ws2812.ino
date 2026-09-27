/*
  ESP32s3 module + ws2812 + button( boot button)
 */

#include <WiFi.h>
#include <IOSignal.h>
#include <Bounce2.h>
#include <FastLED.h>

#define LED_PIN     48          // 연결된 핀 번호 (보드에 맞게 변경)
#define NUM_LEDS    1           // neopixel 내장 LED 개수
#define BRIGHTNESS  100
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
uint8_t hue = 0;  // 초기 색상

#define BUTTON_PIN  0 //12 
#define WIFI_ID "Hollys2 2G"
#define WIFI_KEY "a0312917933"

WiFiClient client;
IOSignal io;
Bounce2::Button downBtn = Bounce2::Button();

void setup() {

  downBtn.attach(BUTTON_PIN, INPUT_PULLUP);
  downBtn.interval(5);           
  downBtn.setPressedState(LOW);  

  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting... ");

  WiFi.mode(WIFI_STA);
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
  io.begin( &client , "io.remocon.kr", 55488);  
  // io.begin( &client , "192.168.0.204", 55488);
  io.onReady( &onReady );
  io.onMessage( &onMessage );

    // FastLED 초기화
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

}


void onReady()
{
  Serial.print("onReady cid: ");
  Serial.println( io.cid );
  io.subscribe("#homeButton");
  io.subscribe("#robot");

  leds[0] = CRGB(255, 0, 0);  // R, G, B
  FastLED.show();
}

void onMessage( char *tag, uint8_t payloadType, uint8_t* payload, size_t payloadSize)
{

  Serial.print(">> signal tag: " );
  Serial.print( tag );
  Serial.print(" type: " );
  Serial.print( payloadType );
  Serial.print(" size: " );
  Serial.println( payloadSize );

  if( strcmp(tag, "#homeButton") == 0){
    // HSV 색상 모델로 색상 설정 (Hue 회전)
    hue+=10;             // Hue 값을 점진적으로 증가 (0~255 사이에서 회전)
    leds[0] = CHSV(hue, 255, 255);  // hue: 0~255, full saturation & brightness
    FastLED.show();
  }

  if( (strcmp(tag, "#robot") == 0) && payloadSize == 4 ){
    Serial.print("#robot ");
    Serial.print( payload[0] );
    Serial.print( " ");
    Serial.print( payload[1] );
    Serial.print( " ");
    Serial.print( payload[2] );
    Serial.print( " ");
    Serial.println( payload[3] );
    
    // 하늘색 (예: R,G,B를 수동 지정)
    leds[0].r = payload[0];
    leds[0].g = payload[1];
    leds[0].b = payload[2];
    FastLED.show();

  }
   
}


void loop() {
    io.loop();
    downBtn.update();
    if (downBtn.pressed()) {
        Serial.println("down");
        io.signal("#homeButton","esp32");
    }    
}