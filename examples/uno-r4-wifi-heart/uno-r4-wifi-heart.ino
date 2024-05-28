/*
 *  IOSignal Remocon Example. 
 *  Arduino Uno R4 WiFi
 *  https://github.com/remocons/remocon-arduino

  An example of communicating between Arduinos and controlling them with a webapp.
    1. change the LED_PIN and BUTTON_PIN numbers according to your board.
    2. set WIFI_SSID and KEY when using wifi.
    3. iosignal server information is used without modification. If you use another server, change the server address and port number.
    4. compile and upload to Arduino.
    5. Access https://remocon.kr with a modern web browser.

  [kr] 아두이노들간의 통신과 웹앱으로 제어하는 예제입니다.
    1. 보드에 따라 LED_PIN과 BUTTON_PIN 번호를 변경하세요.
    2. wifi 사용시 WIFI_SSID와 KEY 를 설정하세요.
    3. iosignal 서버 정보는 수정 없이 그대로 사용합니다. 다른 서버를 사용할 경우 서버 주소와 포트번호를 변경해줍니다.
    4. 아두이노에 컴파일하고 업로딩합니다.
    5. 모던 웹브라우저로  https://remocon.kr 에 접속하세요.
 */


#include "WiFiS3.h"
#include <Arduino.h>
#include <IOSignal.h>
#include <Bounce2.h>
#include "Arduino_LED_Matrix.h"

#define BUTTON_PIN    D12

Bounce2::Button aBtn = Bounce2::Button();
ArduinoLEDMatrix matrix;
WiFiClient client;
IOSignal io;
const char *name = "UnoR4-WiFi:HOME";
const char *ui = "on,off,toggle";

int lastButtonState = HIGH; 

const uint32_t heart[] = { 0x3184a444, 0x44042081, 0x100a0040 };
const uint32_t fullOff[] = { 0, 0, 0 };

void deviceOn(){
  digitalWrite(LED_BUILTIN, HIGH);
  matrix.loadFrame(heart);
  io.signal("@$state", "on" );
}

void deviceOff(){
  digitalWrite(LED_BUILTIN, LOW);
  matrix.loadFrame(fullOff);
  io.signal("@$state", "off" );
}

void deviceToggle(){
  int nextState = !digitalRead(LED_BUILTIN);
   if( nextState){
      deviceOn();
   }else{
      deviceOff();
   }
}


void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  aBtn.attach( BUTTON_PIN, INPUT_PULLUP ); 
  aBtn.interval(5); // debounce interval in milliseconds
  aBtn.setPressedState(LOW); 
  
  WiFi.begin("WIFI_SSID", "WIFI_PASS");
  Serial.begin(115200);
  matrix.begin();
  matrix.loadFrame(heart);
  delay(500);

  io.setRxBuffer( 200 );
  io.begin( &client, "io.remocon.kr", 55488 );
  io.onReady( &onReady );
  io.onMessage( &onMessage );
  // io.auth( "id_key" );

}


void loop() {
  
    io.loop();
    aBtn.update();
    if ( aBtn.pressed() ) {
      Serial.println(F("pressed"));
      deviceToggle();
      io.signal("#screen", "playToggle" );
      // io.signal("#screen", "next" );
    }  


}


void onReady()
{
  Serial.print("onReady cid: ");
  Serial.println( io.cid );
  io.signal("@$state", "off" );
  io.signal("@$name", name );
  io.signal("@$ui", ui );
  io.signal("#notify", io.cid );
  io.subscribe("#search");
  
}

void onMessage( char *tag, uint8_t payloadType, uint8_t* payload, size_t payloadSize)
{

  // signal message info
  Serial.print(">> signal tag: " );
  Serial.print( tag );
  Serial.print(" type: " );
  Serial.print( payloadType );
  Serial.print(" size: " );
  Serial.println( payloadSize );

  if( payloadType == IOSignal::PAYLOAD_TYPE::TEXT ){  
    Serial.print("string payload: " );
    Serial.println( (char *)payload  );
  }

  if( strcmp(tag, "#search") == 0){
    io.signal( "#notify", io.cid );
  }

  if( strcmp(tag, "@ui") == 0){
    io.signal2( (char *)payload, "@ui", io.cid , ui );
  }
      
  if( strcmp(tag, "@") == 0){
    if( strcmp((char *)payload, "on") == 0){
      deviceOn();
    }else if( strcmp((char *)payload, "off") == 0){
      deviceOff();
    }else if( strcmp((char *)payload, "toggle") == 0){
      deviceToggle();
    }
  }
      
}

