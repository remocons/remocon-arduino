/*
 *  IOSignal Remocon Example. 
 *  SparkFun ESP8266 Thing - Dev Board + Button
 *  You can use general ESP8266 board too.
 *
 *  Example of the Remote Control WebApp integration.
 *  Open and control https://remocon.kr with a web browser connected to the same router
 *
 *  IoT리모컨 WebApp 연동 예제입니다.
 *  동일한 공유기에 연결된 웹브라우저로 https://remocon.kr 을 열고 제어하세요  
 *
 *  Lee Dongeun <sixgen@gmail.com>
 *  https://github.com/remocons/remocon-arduino
 *
 *  MIT License
 */


#include <Arduino.h>
#include <ESP8266WiFiMulti.h>
#include <IOSignal.h>
 
ESP8266WiFiMulti wifiMulti;
WiFiClient client;
IOSignal io;
const char *name = "SFE-thing-dev:home";
const char *ui = "on,off,toggle";

const int buttonPin = 2;
int lastButtonState = HIGH; 

void deviceOn(){
  digitalWrite(LED_BUILTIN, LOW); // Low means ON
  io.signal("@$state", "on" );
}

void deviceOff(){
  digitalWrite(LED_BUILTIN, HIGH);
  io.signal("@$state", "off" );
}

void deviceToggle(){
  int nextState = !digitalRead(LED_BUILTIN);
  digitalWrite(LED_BUILTIN, nextState);
   if( nextState){
    io.signal("@$state", "off" );
   }else{
    io.signal("@$state", "on" );
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

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP( "WIFI_SSID", "WIFI_PASS");
  wifiMulti.addAP( "twesomego", "qwer1234");  
  // You can add multiple APs.  
  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(500);

  io.setRxBuffer( 200 );
  io.begin( &client , "io.remocon.kr", 55488);
  io.onReady( &onReady );
  io.onMessage( &onMessage );
  // io.auth( "ID_KEY" ); 

}


void loop() {
  
      io.loop();
      if(isPressed()){
        Serial.println(F("pressed"));
        io.signal("#screen", "next" );
      }

}




void onReady()
{
  Serial.print("onReady cid: ");
  Serial.println( io.cid );
  io.signal("@$state", "off" );
  io.signal("@$ui", ui );
  io.signal("@$name", name );
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