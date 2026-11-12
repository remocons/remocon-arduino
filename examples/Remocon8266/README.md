# Remocon8266 전용 보드

OLED와 네 방향 버튼이 있는 Remocon8266 보드 예제입니다. 출력 3개를 3구 스위치로 제어하고 버튼으로 bayo.tv를 제어합니다.

## 준비물

- 보드: Remocon8266 보드 (D1 mini 기반) (폴더의 회로도 PDF 참고)
- 네트워크: Wi-Fi 2.4GHz 공유기
- 앱: https://remocon.kr 또는 IOSignal Desktop 장치 제어 화면
- 연동: bayo.tv: 버튼이 #screen 신호를 보냅니다

## 배선

| 보드 핀 | 연결 | 비고 |
| --- | --- | --- |
| GPIO16 (D0) | 위 버튼 SW1 | 외부 풀업 |
| GPIO0 (D3) | 왼쪽 버튼 SW2 | 내부 풀업 |
| GPIO14 (D5) | 아래 버튼 SW3 | 내부 풀업 |
| GPIO13 (D7) | 오른쪽 버튼 SW4 | 내부 풀업 |
| GPIO4, GPIO5 | SSD1306 OLED (SDA, SCL) | I2C |
| GPIO15 (D8) | OUT1: LED, 네오픽셀, 서보 등 |  |
| GPIO12 (D6) | OUT2 |  |

## 사용 순서

1. Wi-Fi는 업로드한 뒤 USB로 연결해 https://test.iosignal.net/wifi/ (보드 설정)에서 넣습니다(데스크톱 Chrome·Edge, 보드가 저장). 같은 페이지에서 다른 IOSignal 서버(호스트:포트)와 장치 키도 넣을 수 있습니다. 스케치의 Wi-Fi 값에 실제 값을 쓰면 그 값이 부팅할 때마다 우선합니다.
2. 스케치를 업로드합니다. OLED에 로고와 Ready CID가 나옵니다.
3. 같은 공유기에 연결한 브라우저로 https://remocon.kr 을 엽니다.
4. Remocon8266 카드의 1번(LED), 2번(OUT1), 3번(OUT2) 스위치를 누릅니다.
5. 네 방향 버튼으로 bayo.tv 신호를 보내 봅니다.

## 시그널 절차

| 보내는 쪽 | 받는 쪽 | 태그 | 값 | 설명 |
| --- | --- | --- | --- | --- |
| 장치 | 서버 | `#search` | `구독` | 앱의 검색 요청을 받도록 구독합니다. |
| 장치 | 서버 | `@$info` | `n=Remocon8266;g=HOME;t=switch3.1` | 이름, 분류, 화면 타입입니다. 서버에 보관됩니다. g는 앱이 장치를 묶어 보여 주는 그룹이며 보드 설정 group으로 바꿉니다(바꾸면 바로 다시 보냄). |
| 장치 | 서버 | `@$state` | `s1=0;s2=0;s3=0` | 현재 상태입니다. 서버에 보관됩니다. |
| 장치 | 앱 | `#notify` | `<cid>` | 접속했음을 알립니다. |
| 앱 | 장치 | `#search` |  | 앱이 시작할 때, 1초 뒤, 이후 15초마다 보냅니다. |
| 장치 | 앱 | `#notify` | `<cid>` | 검색에 자기 CID로 답합니다. |
| 앱 | 서버 | `<cid>@$info, $state` | `구독` | 보관된 정보와 상태를 바로 받아 카드를 만듭니다. |
| 앱 | 장치 | `<cid>@` | `s2=1` | 앱의 스위치가 값을 지정하는 명령을 보냅니다. 장치가 받는 태그는 @입니다. |
| 장치 | 앱 | `@$state` | `s1=0;s2=1;s3=0` | 명령을 처리한 뒤 상태를 보냅니다. 앱의 대기 표시가 풀립니다. |
| 서버 | 앱 | `<cid>@$state` | `close` | 장치가 끊기면 서버가 보내고 카드가 사라집니다. |
| 앱 | 장치 | `<cid>@msg` | `문자열` | OLED에 표시합니다. 규칙 문서에 없는 예제 전용 명령입니다. |
| 장치 | bayo.tv | `#screen` | `playToggle, left, down, right` | 네 방향 버튼을 누르면 보냅니다. |

## 참고

- s1은 내장 LED, s2는 OUT1, s3은 OUT2입니다. 내장 LED는 active low라 값을 반전합니다.

## 컴파일

```sh
arduino-cli compile --fqbn esp8266:esp8266:d1_mini examples/Remocon8266
```
