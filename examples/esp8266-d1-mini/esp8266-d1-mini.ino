/*
 *  IOSignal Remocon example: ESP8266 D1 mini
 *
 *  remocon.kr shows it as a light (remocon/light/v1): on, off, toggle.
 *    The built-in LED (GPIO2, active low) is the light.
 *    A button on GPIO14 (D5, to GND) toggles it.
 *
 *  Setup: replace "WIFI_SSID" and "WIFI_PASS" in wifiMulti.addAP(), upload,
 *  then open https://remocon.kr in a browser on the same router.
 *
 *  [kr] D1 mini의 내장 LED를 전등 타입으로 켜고 끕니다. D5 버튼으로도 토글합니다.
 *
 *  Taeo Lee <sixgen@gmail.com>
 *  https://github.com/remocons/remocon-arduino
 *  MIT License
 */

#include <ESP8266WiFiMulti.h>
#include <IOSignal.h>
#include <string.h>
#include <Bounce2.h>
#define BUTTON_PIN    14
#define BUILTIN_LED   2

Bounce2::Button aBtn = Bounce2::Button();
ESP8266WiFiMulti wifiMulti;
WiFiClient client;
IOSignal io;

const char *deviceInfo = "name=D1-mini;group=HOME;type=remocon/light/v1";

void publishState() { io.signal("@$state", digitalRead(BUILTIN_LED) == LOW ? "on=1" : "on=0"); }

void deviceOn(){
  digitalWrite(BUILTIN_LED, LOW);
  io.signal("@$state", "on=1" );
}

void deviceOff(){
  digitalWrite(BUILTIN_LED, HIGH);
  io.signal("@$state", "on=0" );
}

void deviceToggle(){
  int nextState = !digitalRead(BUILTIN_LED);
  Serial.println( nextState);
  digitalWrite(BUILTIN_LED, nextState);
   if( !nextState){  // active low
    deviceOn();
   }else{
    deviceOff();
   }
}


void setup() {  
  pinMode( BUILTIN_LED , OUTPUT);
  digitalWrite( BUILTIN_LED , HIGH);  //
  
  aBtn.attach( BUTTON_PIN, INPUT_PULLUP ); 
  aBtn.interval(5); // debounce interval in milliseconds
  aBtn.setPressedState(LOW); 

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP( "WIFI_SSID", "WIFI_PASS");

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
  // io.auth("ID_KEY"); // Optional: use your own device key.
}


void loop() {
  io.loop();
  aBtn.update();
  if ( aBtn.pressed() ) {
    deviceToggle();
    Serial.println("button");
    delay(100);
  }  
}

void onReady()
{
  Serial.print("onReady cid: ");
  Serial.println( io.cid );
  io.subscribe("#search");
  io.signal("@$info", deviceInfo);
  publishState();
  io.signal("#notify", io.cid);
  
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

  if( payloadType == IOSignal::PAYLOAD_TYPE::TEXT && payload && payloadSize && memchr(payload, 0, payloadSize) ){
    Serial.print("string payload: " );
    Serial.println( (char *)payload  );
  }

  if( strcmp(tag, "#search") == 0){
    io.signal( "#notify", io.cid );
  }

  // Ignore non-text or unterminated payloads before using string APIs.
  if (payloadType != IOSignal::PAYLOAD_TYPE::TEXT || !payload ||
      !payloadSize || !memchr(payload, 0, payloadSize)) return;

  if( strcmp(tag, "@") == 0){
    if( strcmp((char *)payload, "on") == 0){
      deviceOn();
    }else if( strcmp((char *)payload, "off") == 0){
      deviceOff();
    }else if( strcmp((char *)payload, "toggle") == 0){
      deviceToggle();
    } else {
      publishState(); // Unknown commands still get a reply.
    }
  }
      
}
