# Remocon8266 4채널 릴레이 제어기

Remocon8266 보드가 4채널 릴레이 보드(esp8266_relay4ch)를 CID로 제어합니다. 기본 대상은 DEFAULT_TARGET(the-cid)이고, 웹앱 카드나 CLI에서 target=<cid>로 펌웨어 변경 없이 바꿀 수 있으며 플래시(EEPROM)에 저장됩니다. 버튼 4개로 릴레이 1~4를, D6 버튼으로 전체를 토글하고 OLED에 상태를 보여 줍니다.

## 준비물

- 보드: Remocon8266 보드 (D1 mini 기반)
- 네트워크: Wi-Fi 2.4GHz 공유기
- 앱: https://remocon.kr 장치 제어 화면에 Relay Control 카드가 나옵니다. 대상 CID 입력칸은 웹앱의 text 위젯이 필요하고, 없으면 r1~r4 버튼만 나옵니다
- 연동: esp8266_relay4ch 보드: 제어 대상. 기본 CID는 스케치의 DEFAULT_TARGET(the-cid)이며, 장치 키로 고정된 릴레이 보드의 CID로 바꿉니다

## 배선

| 보드 핀 | 연결 | 비고 |
| --- | --- | --- |
| GPIO16 (D0) | SW1 버튼 → 릴레이 1 | 외부 풀업 |
| GPIO0 (D3) | SW2 버튼 → 릴레이 2 | 내부 풀업 |
| GPIO14 (D5) | SW3 버튼 → 릴레이 3 | 내부 풀업 |
| GPIO13 (D7) | SW4 버튼 → 릴레이 4 | 내부 풀업 |
| GPIO12 (D6) | 외부 버튼, 다른 쪽은 GND → 릴레이 전체 | 내부 풀업. 하나라도 켜져 있으면 전체 끄기, 모두 꺼져 있으면 전체 켜기 |
| GPIO4 (D2), GPIO5 (D1) | SSD1306 128x64 OLED | I2C SDA, SCL. 맨 위는 대상 CID, 가운데는 릴레이 1~4, 맨 아래는 이 제어기의 CID입니다. |

## 사용 순서

1. 스케치 폴더에 _secrets.h를 만들어 SECRET_WIFI_SSID, SECRET_WIFI_PASS를 #define합니다(제어기에 키를 쓰려면 릴레이와 다른 키로 SECRET_ID_KEY도). 파일이 없으면 스케치의 WIFI_SSID, WIFI_PASS 값을 직접 바꿉니다.
2. 릴레이 보드에 esp8266_relay4ch를 올리고 장치 키로 CID를 고정합니다.
3. arduino-cli compile --upload -p <포트> --fqbn esp8266:esp8266:d1_mini 로 업로드합니다(Arduino IDE는 LOLIN(WEMOS) D1 R2 & mini).
4. OLED에 Relay <대상 CID>와 상자 4개가 나오면 버튼을 눌러 봅니다. 맨 아래 줄은 이 제어기의 CID입니다.
5. 대상을 바꾸려면 카드의 Target CID에 입력하거나 remocon pub "<제어기 cid>@" "target=<cid>" 를 보냅니다.

## 시그널 절차

| 보내는 쪽 | 받는 쪽 | 태그 | 값 | 설명 |
| --- | --- | --- | --- | --- |
| 제어기 | 서버 | `#search` | `구독` | 앱의 검색에 답합니다. |
| 제어기 | 서버 | `@$panel` | `화면 JSON (text, value, buttons)` | Target CID 입력칸, 릴레이 상태, r1~r4, all 버튼 화면입니다. $info보다 먼저 보내 보관시킵니다. |
| 제어기 | 서버 | `@$info` | `n=Relay Control;ui=r1,r2,r3,r4,all;p=1` | p=1은 화면을 @$panel로 보낸다는 뜻입니다. ui는 화면을 모르는 앱이 보여 줄 버튼입니다. |
| 제어기 | 서버 | `@$state` | `target=the-cid;relays=0001;link=1` | 대상 CID, 릴레이 상태, 대상 연결 여부입니다. |
| 제어기 | 서버 | `<target>@$state` | `구독` | 보관된 릴레이 상태를 바로 받고, 이후 변경을 받습니다. |
| 앱 | 제어기 | `<cid>@` | `target=2001` | 대상을 바꾸고 플래시에 저장합니다. 1~12자, 공백과 @ # $ ; = 은 안 됩니다. |
| 앱 | 제어기 | `<cid>@` | `r1 ~ r4` | 해당 릴레이를 토글합니다. 버튼 SW1~SW4와 같습니다. |
| 앱 | 제어기 | `<cid>@` | `all` | D6 버튼과 같습니다. 모두 꺼져 있으면 all=1, 아니면 all=0을 릴레이에 보냅니다. |
| 제어기 | 릴레이 | `<target>@` | `r1=1` | 현재 상태의 반대 값을 보냅니다. |
| 릴레이 | 제어기 | `<target>@$state` | `r1=1;r2=0;r3=0;r4=1` | OLED와 카드 상태를 갱신합니다. |
| 서버 | 제어기 | `<target>@$state` | `close` | 대상이 끊기면 link=0이 되고 버튼이 멈춥니다. 다시 접속하면 이어집니다. |

## 참고

- text 위젯을 모르는 앱은 @$panel 화면 전체를 쓰지 않고 ui의 r1~r4, all 버튼만 보여 줍니다.
- 릴레이 상태를 모르면(link=0) 토글 명령을 보내지 않습니다.
- 릴레이와 같은 키로 인증하면 한쪽이 로그아웃되므로 별도 키를 씁니다.
- IOSignal 9.6.0 이상이 필요합니다(unsubscribe()를 씁니다).
- 플래시에 저장된 대상 CID가 있으면 DEFAULT_TARGET보다 먼저 씁니다. DEFAULT_TARGET을 바꿔 다시 올려도, 이전에 원격으로 지정한 CID가 남아 있으면 그 값을 씁니다.

## 컴파일

```sh
arduino-cli compile --fqbn esp8266:esp8266:d1_mini examples/Remocon8266-4ch-control
```
