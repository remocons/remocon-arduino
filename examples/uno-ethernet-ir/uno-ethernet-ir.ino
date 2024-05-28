/*
 *  IOSignal Remocon Example. 
 *  Arduino Uno + Ethernet shield W5100 + IR Receiver
 *  https://github.com/remocons/remocon-arduino
 *
 
  An example of an Arduino detecting a remote control signal and forwarding it to another device, or an Arduino receiving a signal from a web app.
    - Connect the IR Receiver module to IR_RECEIVE_PIN.
    - iosignal server information is used without modification. If you use another server, change the server address and port number.
    - compile and upload to Arduino.
    - Access https://remocon.kr and https://bayo.tv with a modern web browser.
    - control youtube player on bayo.tv
    - It works with NEC compatible remotes, for other remotes you can modify the library and signal code.

  [kr] 리모컨 신호를 아두이노에서 감지하고 다른 장치에 신호를 전달하거나 웹앱의 신호를 아두이노가 수신하는 예제입니다. 
    - IR수신모듈을 IR_RECEIVE_PIN에 연결하세요.
    - iosignal 서버 정보는 수정 없이 그대로 사용합니다. 다른 서버를 사용할 경우 서버 주소와 포트번호를 변경해줍니다.
    - 아두이노에 컴파일하고 업로딩합니다.
    - 모던 웹브라우저로  https://remocon.kr 와 https://bayo.tv 에 접속하세요.
    - bayo.tv 의 동영상을 제어하세요!
    - NEC 호환 리모컨으로 작동되며, 다른 리모컨의 경우 라이브러리와 신호 코드를 수정하시면됩니다.

 *
 *  I used a library for a low-capacity NEC-compatible remote.
 *  If you are using a different remote, change the library
 *
 */



#include <Arduino.h>
#define NO_LED_FEEDBACK_CODE
#define IR_RECEIVE_PIN 2
#include "TinyIRReceiver.hpp" 
#include <SPI.h>
#include <Ethernet.h>
#include <string.h>
#include <IOSignal.h>

// If you have multiple devices, you'll need to change the MAC address.
byte mac[]{0xDE, 0xAD, 0x1E, 0xEF, 0xFE, 0x50};
EthernetClient client;
IOSignal io;
const char *name = "UnoR3-Eth-IR:HOME";
const char *ui = "on,off,toggle";

void setup()
{
  pinMode(5, OUTPUT); // LED
  pinMode(6, OUTPUT); // Relay
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);

  Serial.begin(115200);
  initPCIInterruptForTinyReceiver();

  Ethernet.init(10);
  Ethernet.begin(mac); // DHCP
  Serial.println(F("Init.."));
  Serial.print(F("IP:"));
  Serial.println(Ethernet.localIP());

  io.setRxBuffer(80);
  io.begin(&client, "io.remocon.kr", 55488 );
  io.onReady(&onReady);
  io.onMessage(&onMessage);
  // io.auth( "id_key" );  // Only if you have an authentication key.


}

void deviceOn()
{
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  io.signal("@$state", "on");
}

void deviceOff()
{
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  io.signal("@$state", "off");
}

void deviceToggle()
{
  int nextState = !digitalRead(5);
  digitalWrite(5, nextState);
  digitalWrite(6, nextState);
  if (nextState)
  {
    io.signal("@$state", "on");
  }
  else
  {
    io.signal("@$state", "off");
  }
}

void loop()
{
    io.loop();

    if (TinyIRReceiverData.justWritten)
    {
      TinyIRReceiverData.justWritten = false;

        // Serial.println( TinyIRReceiverData.Command );
      if (TinyIRReceiverData.Flags == IRDATA_FLAGS_IS_REPEAT)
      {
        // Serial.println(F("Repeat"));
      }
      else
      {
        // Serial.print(F("A="));
        // Serial.print(TinyIRReceiverData.Address, HEX);
        Serial.print(F(" C="));
        Serial.println(TinyIRReceiverData.Command, HEX);

        switch (TinyIRReceiverData.Command)
        {
        // Change to the code value of your NEC compatible remote
        case 0x01 :
          io.signal("#screen", "next");
          break;
        case 0x08 :
          io.signal("#screen", "prev");
          break;
        case 0x05 :
          io.signal("#screen", "up");
          break;
        case 0 :
          io.signal("#screen", "down");
          break;
        case 0x1F :
          deviceToggle(); 
          break;
        case 0x1E :
          io.signal("cid1@", "toggle");
          break;
        case 0x1A :
          io.signal("cid2@", "toggle");
          break;
        default:
          io.signal("#screen", "playToggle");
        }
      }
    }

}

void onReady()
{
  Serial.print(F("onReady cid: "));
  Serial.println(io.cid);
  io.signal("@$state", "off");
  io.signal("@$name", name );
  io.signal("@$ui", ui);
  io.signal("#notify", io.cid);
  io.subscribe("#search");
}

void onMessage(char *tag, uint8_t payloadType, uint8_t *payload, size_t payloadSize)
{
  // signal message info
  Serial.print(F(">> signal tag: "));
  Serial.print(tag);
  Serial.print(F(" type: "));
  Serial.print(payloadType);
  Serial.print(F(" size: "));
  Serial.println(payloadSize);

  if (payloadType == IOSignal::PAYLOAD_TYPE::TEXT)
  {
    Serial.print(F("string payload: "));
    Serial.println((char *)payload);
  }

  if (strcmp(tag, "#search") == 0)
  {
    io.signal("#notify", io.cid);
  }

  if (strcmp(tag, "@ui") == 0)
  {
    io.signal2((char *)payload, "@ui", io.cid, ui);
  }

  if (strcmp(tag, "@") == 0)
  {
    if (strcmp((char *)payload, "on") == 0)
    {
      deviceOn();
    }
    else if (strcmp((char *)payload, "off") == 0)
    {
      deviceOff();
    }
    else if (strcmp((char *)payload, "toggle") == 0)
    {
      deviceToggle();
    }
  }
}

