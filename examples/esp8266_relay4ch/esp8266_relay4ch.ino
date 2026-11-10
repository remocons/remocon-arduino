/*
 *  IOSignal Remocon Example. 
 *
 *  Taeo Lee <sixgen@gmail.com>
 *  https://github.com/remocons/remocon-arduino
 *
 *  MIT License
 */


#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <IOSignal.h>

// Wi-Fi values (and an optional device key) can go in _secrets.h next to this sketch,
// which keeps them out of version control. Without it, edit WIFI_SSID and WIFI_PASS below.
#if __has_include("_secrets.h")
#include "_secrets.h"
#endif
#ifndef SECRET_WIFI_SSID
#define SECRET_WIFI_SSID "WIFI_SSID"
#define SECRET_WIFI_PASS "WIFI_PASS"
#endif
const char* WIFI_SSID = SECRET_WIFI_SSID;
const char* WIFI_PASS = SECRET_WIFI_PASS;

WiFiClient client;
IOSignal io;

const char *deviceInfo = "name=Relay4CH;group=1414;type=remocon/relay4/v1";
char states[] = "10000";
char last_states[] = "10000";
uint8_t pinMap[] = { 2, 13, 12, 14, 16};


void publishState() {
  char text[32];
  snprintf(text, sizeof(text), "r1=%c;r2=%c;r3=%c;r4=%c", states[1], states[2], states[3], states[4]);
  io.signal("@$state", text);
}

void stateChange(int i){
  //local
  char state = states[i];
  if( state != last_states[i] ){
    digitalWrite( pinMap[i] , states[i] - '0' );
    last_states[i] = state;
  }
  Serial.print("states: ");
  Serial.println( states);

  // for remote peers.
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
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);
  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);


  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.begin(115200);
  Serial.flush();
  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(500);


  io.setRxBuffer( 200 );
  io.begin( &client, "io.remocon.kr", 55488 );
#ifdef SECRET_ID_KEY
  io.auth(SECRET_ID_KEY); // Optional: your own key (id.key) from _secrets.h.
#endif
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
    const char *c = (const char *)payload;
    if( c[0] == 'r' && c[1] >= '1' && c[1] <= '4' && c[2] == '=' && (c[3] == '0' || c[3] == '1') && c[4] == 0 ){
      states[c[1] - '0'] = c[3];
      stateChange(c[1] - '0');
    }else if( strcmp(c, "all=1") == 0 || strcmp(c, "all=0") == 0 ){
      for(int i = 1; i <= 4; i++){
        states[i] = c[4];
        stateChange(i);
      }
    // The name commands below are for IR routers, which do not know the state.
    }else if( strcmp((char *)payload, "L") == 0){
      toggle(0);
    }else if( strcmp((char *)payload, "A") == 0){
      toggle(1);
    }else if( strcmp((char *)payload, "B") == 0){
      toggle(2);
    }else if( strcmp((char *)payload, "C") == 0){
      toggle(3);
    }else if( strcmp((char *)payload, "D") == 0){
      toggle(4);
    }else{
      publishState(); // Unknown commands still get a reply.
    }
  }
      
}