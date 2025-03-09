/*
 *  IOSignal Remocon Example. 
 *  esp01-relay-button
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

#include <ESP8266WiFiMulti.h>
#include <IOSignal.h>
#include <Bounce2.h>

#define RELAY_PIN    0
#define LED_PIN      1
#define BUTTON_PIN   2

ESP8266WiFiMulti wifiMulti;
WiFiClient client;
IOSignal io;

Bounce2::Button aBtn = Bounce2::Button();

const char *name = "ESP01-Relay-Btn:HOME";
const char *ui = "Relay,LED";
char states[] = "11";
char last_states[] = "11";
uint8_t pinMap[] = { 0,1 }; 

void stateChange(int i){
  //local
  char state = states[i];
  if( state != last_states[i] ){
    digitalWrite( pinMap[i] , states[i] - '0' );
    last_states[i] = state;
  }

  //publish device states
  io.signal("@$state", states );

} 


void toggle(int i){
  char state = states[i];
   if( state == '1'){
      states[i] = '0';
   }else{
      states[i] = '1';
   }
   stateChange(i);
}


void setup() {  
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);

  pinMode( LED_PIN , OUTPUT);
  digitalWrite( LED_PIN, HIGH);  // active low

  // NOTICE. ESP01. 
  // You can't use Serial-tx and built-in LED together.
  // Serial.begin(115200);
  
  aBtn.attach( BUTTON_PIN, INPUT_PULLUP ); 
  aBtn.interval(5); // debounce interval in milliseconds
  aBtn.setPressedState(LOW); 
  
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP( "WIFI_SSID", "WIFI_PASS");
  // wifiMulti.addAP( "twesomego", "qwer1234");  
  // You can add multiple APs.  

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
  }
  delay(500);

  io.setRxBuffer( 200 );
  io.begin( &client , "io.remocon.kr", 55488);
  io.onReady( &onReady );
  io.onMessage( &onMessage );
  // io.auth( "ID_KEY" );   // Only if you have an authentication key.

}


void loop() {
    io.loop();   
    aBtn.update();
    if ( aBtn.pressed() ) {
      toggle( RELAY_PIN );
      delay(100);
    }  

}




void onReady()
{
  // NOTICE. 
  // The ESP01's serial communication pins are used as IO pins, so serial communication is not available.
  // Serial.print("onReady cid: ");
  // Serial.println( io.cid );

  io.signal("@$state", states );
  io.signal("@$ui", ui );
  io.signal("@$name", name );
  io.signal("#notify", io.cid );
  io.subscribe("#search");
  
}

void onMessage( char *tag, uint8_t payloadType, uint8_t* payload, size_t payloadSize)
{

  if( strcmp(tag, "#search") == 0){
    io.signal( "#notify", io.cid );
  }

  if( strcmp(tag, "@ui") == 0){
    io.signal2( (char *)payload, "@ui", io.cid , ui );
  }
      
  if( strcmp(tag, "@") == 0){
    if( strcmp((char *)payload, "Relay") == 0){
      toggle( RELAY_PIN);
    }
    else if( strcmp((char *)payload, "LED") == 0){
      toggle( LED_PIN );
    }
  }
      
}
