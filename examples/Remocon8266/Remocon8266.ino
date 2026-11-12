/*
 *  IOSignal Remocon example: Remocon8266 board
 *
 *  remocon.kr shows it as a 3-way switch (switch3.1):
 *    s1 = built-in LED (active low), s2 = OUT1 (GPIO15, D8), s3 = OUT2 (GPIO12, D6).
 *  The four buttons send #screen commands to a bayo.tv screen:
 *    SW1 (GPIO16) playToggle, SW2 (GPIO0) left, SW3 (GPIO14) down, SW4 (GPIO13) right.
 *  The OLED (I2C, GPIO4/5) shows the CID and states; @msg text is shown on it.
 *  See the schematic PDF in this folder.
 *
 *  Wi-Fi: upload, then set it over USB on https://test.iosignal.net/wifi/ (the board keeps it),
 *  or write it in the sketch (then the sketch values are used at every boot). The same page can
 *  set another IOSignal server and a device key.
 *  Then open https://remocon.kr in a browser on the same router.
 *
 *  [kr] Remocon8266 보드의 출력 3개를 3구 스위치로 제어하고, 버튼 4개로 bayo.tv 화면을 조작합니다.
 *
 *  Taeo Lee <sixgen@gmail.com>
 *  https://github.com/remocons/remocon-arduino
 *  MIT License
 */


#include <IOSignal.h>
#include <IOSignalConfig.h>
#include <string.h>
#include <Bounce2.h>
#include <U8g2lib.h>
#include <Wire.h>

// Buttons
#define BTN_UP     16   // D0  SW1 *EXTERNAL HARDWARE PULLUP
#define BTN_LEFT    0   // D3  SW2
#define BTN_DOWN   14   // D5  SW3
#define BTN_RIGHT  13   // D7  SW4

//I2C  OLED & else
#define PIN_SDA     4   // D2
#define PIN_SDL     5   // D1

//OUTPUT
#define OUT1       15   // D8  LED, NeoPixel, Servo or else.
#define OUT2       12   // D6 

#define WIFI_ID  "WIFI_SSID"
#define WIFI_KEY "WIFI_PASS"

#define SERVER_URL "io.remocon.kr"
#define SERVER_PORT 55488

IOSignalConfig config;
WiFiClient client;
IOSignal io;
// Group the web app shows (g= in the info): board settings 'group'. A change is sent at once.
IOSignalSetting group(config, "group", "Group (g=)", "HOME");
void sendInfo() {
  char g[33], info[96];
  snprintf(g, sizeof(g), "%s", group.value());
  for (char* c = g; *c; c++) if (*c == ';') *c = ' ';  // ';' separates info fields
  snprintf(info, sizeof(info), "n=Remocon8266;g=%s;t=switch3.1", g);
  io.signal("@$info", info);
}


// 1 means on, whatever the pin polarity.
char states[] = "000";
char last_states[] = "000";
const bool activeLow[] = { true, false, false };
uint8_t pinMap[] = { LED_BUILTIN , OUT1, OUT2 };

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

// Define button objects
Bounce2::Button upBtn = Bounce2::Button();
Bounce2::Button leftBtn = Bounce2::Button();
Bounce2::Button downBtn = Bounce2::Button();
Bounce2::Button rightBtn = Bounce2::Button();

void publishState() {
  char text[30];
  snprintf(text, sizeof(text), "s1=%c;s2=%c;s3=%c", states[0], states[1], states[2]);
  io.signal("@$state", text);
}

void stateChange(int i){
  //local state
  char state = states[i];
  if( state != last_states[i] ){
    digitalWrite( pinMap[i] , (states[i] == '1') != activeLow[i] );
    last_states[i] = state;
  }
  Serial.print("states: ");
  Serial.println( states);

  //publish device states
  publishState();

  u8g2.clearBuffer();
  u8g2.drawStr(0, 10, "Remote Info");
  u8g2.setCursor(0, 25);
  u8g2.print("CID: ");
  u8g2.print( io.cid);
  u8g2.setCursor(0, 40);
  u8g2.print("States: ");
  u8g2.print( states);
  u8g2.sendBuffer();
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


void check_events(void) {
  upBtn.update();
  leftBtn.update();
  downBtn.update();
  rightBtn.update();
}

void handle_events(void) {
  // 0 = not pushed, 1 = pushed
  if (upBtn.pressed()) {
    Serial.println("up");
    io.signal("#screen","playToggle");
  } else if (leftBtn.pressed()) {
    Serial.println("left");
    io.signal("#screen","left");
  } else if (downBtn.pressed()) {
    Serial.println("down");
    io.signal("#screen","down");
  } else if (rightBtn.pressed()) {
    Serial.println("right");
    io.signal("#screen","right");
    // io.signal("cid@","D");
  }
}

void u8g2_prepare(void) {
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

// logo
uint16_t p[][2] = {
  { 42, 32 },
  { 61, 32 },
  { 67, 7 },
  { 76, 17 },
  { 66, 56 },
  { 76, 46 }
};

void draw_logo() {
  // 128x64 frame
  u8g2.drawTriangle(p[0][0], p[0][1], p[1][0], p[1][1], p[2][0], p[2][1]);
  u8g2.drawTriangle(p[1][0], p[1][1], p[2][0], p[2][1], p[3][0], p[3][1]);
  u8g2.drawTriangle(p[0][0], p[0][1], p[1][0], p[1][1], p[4][0], p[4][1]);
  u8g2.drawTriangle(p[1][0], p[1][1], p[4][0], p[4][1], p[5][0], p[5][1]);
  u8g2.drawDisc(84, 30, 5, U8G2_DRAW_ALL);
}

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // active low 
  pinMode(OUT1, OUTPUT);
  digitalWrite(OUT1, LOW); 
  pinMode(OUT2, OUTPUT);
  digitalWrite(OUT2, LOW); 

  upBtn.attach(BTN_UP, INPUT); // external pullup
  upBtn.interval(5);           
  upBtn.setPressedState(LOW);  

  leftBtn.attach(BTN_LEFT, INPUT_PULLUP); 
  leftBtn.interval(5);              
  leftBtn.setPressedState(LOW);     

  downBtn.attach(BTN_DOWN, INPUT_PULLUP);
  downBtn.interval(5);           
  downBtn.setPressedState(LOW);  

  rightBtn.attach(BTN_RIGHT, INPUT_PULLUP);
  rightBtn.interval(5);           
  rightBtn.setPressedState(LOW);  

  Serial.begin(115200);
  Serial.flush();

  u8g2.begin();
  u8g2_prepare();

  u8g2.clearBuffer(); 
    draw_logo();
    u8g2.sendBuffer();
    delay(800);
 
  u8g2.clearBuffer(); 
    u8g2.setFont(u8g2_font_profont22_mf);
    u8g2.drawStr(0, 20, "REMOCON.KR");
    u8g2.sendBuffer();
    delay(2000);

  // u8g2.setFont(u8g2_font_ncenB08_tr);  // choose a suitable font
  u8g2.setFont(u8g2_font_t0_13_mr);  // choose a suitable font
  u8g2.clearBuffer(); 
    u8g2.drawStr(0, 20, "Connecting..");
    u8g2.sendBuffer();
  
  Serial.println();
  // Wi-Fi, server and device key: set over USB, or the sketch values.
  config.begin( Serial, "Remocon8266", WIFI_ID, WIFI_KEY );
  group.maxLength(32);
  group.onChange([](const char*) { if (io.state == IO_READY) sendInfo(); });
  config.setServer( SERVER_URL, SERVER_PORT );
  config.printServer();
  io.setRxBuffer( 200 );
  io.setLog( [](const char* line) { Serial.println(line); } );
  io.onReady( &onReady );
  io.onMessage( &onMessage );
  if ( config.key() ) io.auth( config.key() ); // a device key set over USB, else anonymous
  io.begin( &client, config.host(), config.port() );
}

  
void loop() {
    config.loop(); // USB setup requests, Wi-Fi reconnect
    if ( config.connected() ) io.loop();
    check_events();
    handle_events();
}




void onReady()
{
  Serial.print("onReady cid: ");
  Serial.println( io.cid );
  io.subscribe("#search");
  sendInfo();
  publishState();
  io.signal("#notify", io.cid);
  
  u8g2.clearBuffer();
    u8g2.setCursor(0, 0);
    u8g2.print("Ready CID: ");
    u8g2.print(io.cid);
    u8g2.drawStr(0, 15, "Access the webapp");
    u8g2.drawStr(0, 30, "https://remocon.kr");
    u8g2.drawStr(0, 45, "Control DIY IoT");
    u8g2.sendBuffer();
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

  if( strcmp(tag, "@msg") == 0){

    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "msg");
    u8g2.setCursor(0, 25);
    u8g2.print( (char *)payload );
    u8g2.sendBuffer();

  }
      
  if( strcmp(tag, "@") == 0){
    // s1=1 sets an output. Unknown commands still get a reply.
    const char *c = (const char *)payload;
    int i = c[0] == 's' ? c[1] - '1' : -1;
    if( i >= 0 && i < 3 && c[2] == '=' && (c[3] == '0' || c[3] == '1') && c[4] == 0 ){
      states[i] = c[3];
      stateChange(i);
    }else{
      publishState();
    }
  }
      
}
