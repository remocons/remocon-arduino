
/*
 *  IOSignal Remocon Example. 
 *  Arduino Uno + Ethernet shield W5100 + Button
 *  https://github.com/remocons/remocon-arduino
 *

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

#include <SPI.h>
#include <Ethernet.h>
#include <string.h>
#include <IOSignal.h>
#include <Bounce2.h>

// If you have multiple devices, you'll need to change the MAC address.
byte mac[]{0, 0, 0, 0, 0, 0x07}; 
EthernetClient client;
IOSignal io;
const char *name = "Uno-Btn:HOME";
const char *ui = "on,off,toggle";

const int buttonPin = 2;
int lastButtonState = HIGH; 

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(5, OUTPUT); // LED
  pinMode(6, OUTPUT); // Relay
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);

  Serial.begin(115200);
  Serial.println(F("Init.."));

  Ethernet.init(10);
  Ethernet.begin(mac); // DHCP
  Serial.print(F("IP:"));
  Serial.println(Ethernet.localIP());
  
  io.setRxBuffer( 80 );
  io.begin( &client, "io.remocon.kr", 55488);
  io.onReady( &onReady );
  io.onMessage( &onMessage );
  // io.auth( "id_key" );

}

void deviceOn(){
   digitalWrite( 5, HIGH);
   digitalWrite( 6, HIGH);
  io.signal("@$state", "on" );
}

void deviceOff(){
   digitalWrite(5, LOW);
   digitalWrite(6, LOW);
  io.signal("@$state", "off" );
}

void deviceToggle(){
  int nextState = !digitalRead(5);
  digitalWrite(5, nextState);
  digitalWrite(6, nextState);
   if(nextState){
    io.signal("@$state", "on" );
   }else{
    io.signal("@$state", "off" );
   }
}

int isPressed(){
  int currentState = digitalRead(buttonPin);
  if(lastButtonState == HIGH && currentState == LOW){
    lastButtonState = LOW;
    return 1;
  } 
  else{
    lastButtonState = currentState;
    return 0;
  } 
}

void loop() {

      io.loop();
      if(isPressed()){
        Serial.println(F("pressed"));
        deviceToggle();
        io.signal("#screen", "playToggle" );
      }

}


void onReady()
{
  Serial.print(F("onReady cid: "));
  Serial.println( io.cid );
  io.signal("@$state", "off" );
  io.signal("@$ui", ui );
  io.signal("@$name", name );
  io.signal("#notify", io.cid );
  io.subscribe("#search");
  
}

void onMessage( char *tag, uint8_t payloadType, uint8_t* payload, size_t payloadSize)
{
  Serial.print(F(">> signal tag: ") );
  Serial.print( tag );
  Serial.print(F(" type: ") );
  Serial.print( payloadType );
  Serial.print(F(" size: " ));
  Serial.println( payloadSize );

  if( payloadType == IOSignal::PAYLOAD_TYPE::TEXT ){  
    Serial.print(F("string payload: ") );
    Serial.println( (char *)payload  );
  }

  if( strcmp(tag, "#search") == 0){
      io.signal( "#notify", io.cid );
  }
      
  if( strcmp(tag, "@ui") == 0){
      io.signal2( (const char*)payload, "@ui", io.cid , ui);
  }
      
  if( strcmp(tag, "@") == 0){
    if( strcmp((const char*)payload, "on") == 0){
      deviceOn();
    }else if( strcmp((const char*)payload, "off") == 0){
      deviceOff();
    }else if( strcmp((const char*)payload, "toggle") == 0){
      deviceToggle();
    }
  }
      
}


