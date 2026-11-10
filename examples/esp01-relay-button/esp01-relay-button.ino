/*
 *  IOSignal Remocon example: ESP-01 relay and button
 *
 *  remocon.kr shows it as a 2-way switch (remocon/switch2/v1):
 *    s1 = relay on GPIO0, s2 = LED on GPIO1 (active low).
 *    The button on GPIO2 toggles the relay.
 *  GPIO1 is the TX pin, so this sketch does not use serial.
 *  GPIO0 and GPIO2 must be HIGH at boot; check the relay and button wiring.
 *
 *  Setup: replace "WIFI_SSID" and "WIFI_PASS" in wifiMulti.addAP(), upload,
 *  then open https://remocon.kr in a browser on the same router.
 *
 *  [kr] ESP-01의 릴레이(GPIO0)와 LED(GPIO1)를 2구 스위치로 제어하고, GPIO2 버튼으로 릴레이를 토글합니다.
 *
 *  Taeo Lee <sixgen@gmail.com>
 *  https://github.com/remocons/remocon-arduino
 *  MIT License
 */

#include <ESP8266WiFiMulti.h>
#include <IOSignal.h>
#include <string.h>
#include <Bounce2.h>

#define RELAY_PIN    0
#define LED_PIN      1
#define BUTTON_PIN   2

ESP8266WiFiMulti wifiMulti;
WiFiClient client;
IOSignal io;

Bounce2::Button aBtn = Bounce2::Button();

const char *deviceInfo = "name=ESP01-Relay-Btn;group=HOME;type=remocon/switch2/v1";
// 1 means on, whatever the pin polarity.
char states[] = "10";
char last_states[] = "10";
const bool activeLow[] = { false, true };
uint8_t pinMap[] = { RELAY_PIN, LED_PIN };

void publishState() {
  char text[23];
  snprintf(text, sizeof(text), "s1=%c;s2=%c", states[0], states[1]);
  io.signal("@$state", text);
}

void stateChange(int i){
  //local
  char state = states[i];
  if( state != last_states[i] ){
    digitalWrite( pinMap[i] , (states[i] == '1') != activeLow[i] );
    last_states[i] = state;
  }

  //publish device states
  publishState();

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
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

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

  // You can add multiple APs.  

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
  }
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
      toggle(0);
      delay(100);
    }  

}




void onReady()
{
  // NOTICE. 
  // The ESP01's serial communication pins are used as IO pins, so serial communication is not available.
  // Serial.print("onReady cid: ");
  // Serial.println( io.cid );

  io.subscribe("#search");
  io.signal("@$info", deviceInfo);
  publishState();
  io.signal("#notify", io.cid);
  
}

void onMessage( char *tag, uint8_t payloadType, uint8_t* payload, size_t payloadSize)
{

  if( strcmp(tag, "#search") == 0){
    io.signal( "#notify", io.cid );
  }

  // Ignore non-text or unterminated payloads before using string APIs.
  if (payloadType != IOSignal::PAYLOAD_TYPE::TEXT || !payload ||
      !payloadSize || !memchr(payload, 0, payloadSize)) return;

  if( strcmp(tag, "@") == 0){
    // s1=1 sets an output. Unknown commands still get a reply.
    const char *c = (const char *)payload;
    int i = c[0] == 's' ? c[1] - '1' : -1;
    if( i >= 0 && i < 2 && c[2] == '=' && (c[3] == '0' || c[3] == '1') && c[4] == 0 ){
      states[i] = c[3];
      stateChange(i);
    }else{
      publishState();
    }
  }
      
}
