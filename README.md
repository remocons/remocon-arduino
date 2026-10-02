# Remocon

[remocon.kr](https://remocon.kr/) 및 [IOSignal 통신 실습실(test.iosignal.net)](https://test.iosignal.net/) 웹앱과 연동하여 아두이노 장치를 원격 제어하고 시그널을 송수신하는 예제 모음입니다.
통신에는 [IOSignal Arduino 클라이언트](https://github.com/remocons/iosignal-arduino)를 사용합니다.

Arduino remote-control and messaging examples for [remocon.kr](https://remocon.kr/) and [test.iosignal.net](https://test.iosignal.net/), using the IOSignal client library.
Choose an example for your board, replace the Wi-Fi placeholders, then compile and upload it.

## 설치 및 시작

1. Arduino IDE의 라이브러리 매니저에서 `Remocon`을 검색하여 의존 라이브러리와 함께 설치합니다.
   소스로 설치하려면 [공개 저장소](https://github.com/remocons/remocon-arduino)의 ZIP을 받아 **Sketch > Include Library > Add .ZIP Library**로 추가합니다.
2. 보드 매니저에서 사용할 보드 패키지를 설치하고 **Tools > Board / Port**에서 보드와 포트를 선택합니다.
3. **File > Examples > Remocon**에서 보드에 맞는 예제를 선택합니다. 목록이 갱신되지 않으면 IDE를 재시작합니다.
4. Wi-Fi 예제의 `WIFI_SSID`, `WIFI_PASS`를 공유기 정보로 바꿉니다. 예제에 따라 `WIFI_ID` / `WIFI_KEY` 매크로 또는 `WiFi.begin()` / `wifiMulti.addAP()`의 문자열을 수정합니다.
5. 필요한 LED·버튼·릴레이를 연결하고 핀 번호를 확인한 뒤 컴파일 및 업로드합니다.
6. 시리얼 모니터를 **115200 baud**로 열어 연결 상태를 확인합니다. ESP-01 예제는 TX 핀을 LED로 사용하므로 시리얼 출력을 사용하지 않습니다.

공개 예제의 기본 서버는 `io.remocon.kr`, TCP 포트는 `55488`입니다.
Ethernet 예제는 DHCP를 사용하므로 유선 연결과 DHCP 가능한 공유기가 필요합니다. 여러 장치를 사용할 때는 MAC 주소를 서로 다르게 설정하세요.

## 보드별 튜토리얼

[공개 통신 실습실과 ESP32 실습](https://iosignal.net/docs/examples/workbench/start)에서 기본 통신, CID 개별 제어, 그룹 제어, RGB LED를 순서대로 진행할 수 있습니다. 새 스케치는 공개 저장소 소스에 포함되며 Library Manager 반영 전에는 저장소에서 직접 열어 사용하세요.

## 보드별 예제

| 예제 경로 (`examples/` 아래) | 대상 보드 | 동작 / 준비물 |
| --- | --- | --- |
| `esp32c3-button-led` | ESP32-C3 Super Mini | GPIO 9 버튼으로 `#homeButton` 송신, 수신 시 GPIO 8 LED 토글 |
| `iris-group` | iris (ESP32-C3) | DIP로 그룹·장치 번호를 설정하고 버튼·웹앱으로 WS2812 4개 제어 |
| `esp32-cid-led` | ESP32 + GPIO 4 외부 LED | 장치 검색·CID 명령·실제 상태 발행, Remocon 또는 공개 통신 실습실 |
| `esp32-group-led` | ESP32 1~2대 + GPIO 4 외부 LED | 두 자리 상태 문자열로 그룹 출력 제어, 보드별 OUTPUT_INDEX 선택 |
| `wifi-simple` | ESP8266, ESP32 | `#$switch` 공유스위치 상태를 시리얼로 출력 |
| `esp32c3/esp32c3-simple` | ESP32-C3 Super Mini 기준 | 공유스위치로 GPIO 8 LED 제어, active low |
| `esp8266-d1-mini` | ESP8266 D1 mini | 내장 LED와 GPIO 14(D5) 버튼 제어 |
| `esp01-relay-button` | ESP8266 ESP-01 | 릴레이 GPIO 0, LED GPIO 1, 버튼 GPIO 2 |
| `Remocon8266` | Remocon8266 전용 보드 | OLED·버튼·출력 제어, 폴더 내 회로도 참고 |
| `esp32s3-ws2812` | ESP32-S3 | WS2812 GPIO 48, BOOT 버튼 GPIO 0, `#homeButton` / `#robot` 채널 |
| `uno-r4-wifi-heart` | Arduino UNO R4 WiFi | LED 매트릭스 하트와 내장 LED, D12 버튼 |
| `uno-ethernet-button` | UNO R3 + W5100 Ethernet 실드 | D2 버튼, D5 LED, D6 릴레이 출력 |
| `uno-ethernet-ir` | UNO R3 + W5100 Ethernet 실드 | D2 NEC 적외선 수신기, D5·D6 출력 |

모든 예제가 모든 보드에서 컴파일되는 것은 아닙니다. ESP32-C3/S3 제품별 LED 종류와 핀 번호도 다를 수 있으므로 실제 보드에 맞게 수정하세요.
ESP-01은 부팅 시 GPIO 0·2 상태의 영향을 받으므로 버튼·릴레이 배선도 확인해야 합니다.

공통 통신 의존성은 `IOSignal`, `Boho`, `Crypto`이며 예제에 따라 `Bounce2`, `U8g2`, `IRremote`, `FastLED`, `Ethernet`을 사용합니다.
Wi-Fi 및 UNO R4 LED 매트릭스 헤더는 해당 보드 패키지에서 제공합니다.
의존성 선언은 [library.properties](library.properties)에 있습니다. `architectures=*`는 패키지 전체의 메타데이터이며, 각 예제의 보드 호환성을 보장하지 않습니다.

## 웹앱에서 제어하기

- [remocon.kr](https://remocon.kr/): 장치 검색·제어 UI와 계정 기반 관리 서비스를 사용합니다.
- [IOSignal 통신 실습실 — test.iosignal.net](https://test.iosignal.net/): 보드별 프리셋 또는 직접 입력으로 송수신·구독·로그를 확인합니다. [웹앱 소스 저장소](https://github.com/remocons/test_iosignal_net)에서 내려받아 로컬 실행할 수도 있습니다.

통신 실습실에서는 보드를 선택하고 예제와 같은 서버에 연결하세요. 기본 Remocon 예제는 웹앱의 서버 URL을 `wss://io.remocon.kr/ws`로 설정합니다. 보드별 연결·제어 순서는 [튜토리얼](https://iosignal.net/docs/examples/workbench/start)을 참고하세요.

`esp32s3-ws2812`는 [IOSignal 통신 실습실(test.iosignal.net)](https://test.iosignal.net/)의 RGB 프리셋과 연동하는 채널 통신 예제입니다. `#homeButton` 신호로 색상을 바꾸며, `#robot`에는 4바이트 페이로드를 보내면 앞의 3바이트를 RGB로 사용합니다.

통신 실습실은 [HTTPS](https://test.iosignal.net/)와 [HTTP](http://test.iosignal.net/) 접속을 모두 제공합니다. 공개 서버에는 HTTPS + WSS를 권장합니다. HTTPS 페이지의 WS 연결은 브라우저 및 로컬 네트워크 접근 권한에 따라 제한될 수 있으며, HTTP로 전환해도 모든 제한이 해결되지는 않습니다. 상단 전환 버튼으로 이동한 후 다시 연결하세요. [연결 조건과 로컬 실습 안내](https://iosignal.net/docs/examples/workbench/start)를 참고하세요.

다음은 remocon.kr 사용 순서입니다.

1. 아두이노와 **동일한 공유기에 연결된 스마트폰 또는 PC**에서 [remocon.kr](https://remocon.kr)을 엽니다.
2. 장치 목록을 제공하는 예제에서는 장치가 접속하면 앱에서 해당 장치를 선택해 제어합니다.
3. `wifi-simple` 및 `esp32c3-simple`은 장치 버튼 UI를 등록하지 않고 **공유스위치** 채널을 구독합니다. 앱의 공유스위치를 조작하여 확인하세요.

인증키를 설정하지 않는 사용 흐름은 동일 공유기 환경을 기준으로 합니다. 외부 네트워크에서 내 장치를 제어하려면 앱 계정의 장치 인증키를 설정하세요.
계정 메뉴·인증키 발급 수량·서비스 이용 범위는 현재 웹앱 안내를 확인하세요.

## 장치 인증키 설정

앱에 로그인한 뒤 Account의 장치 정보에서 `ID_KEY`를 복사합니다.
스케치의 `setup()`에서 다음 줄의 주석을 해제하고 본인의 값으로 교체합니다. 해당 줄이 없으면 `io.begin()` 및 콜백 등록 다음에 추가합니다.

```cpp
io.auth("ID_KEY"); // ID_KEY를 앱에서 발급받은 실제 값으로 교체
```

Wi-Fi 비밀번호와 장치 인증키를 공개 저장소에 올리지 마세요. 배포용 예제에는 `WIFI_SSID`, `WIFI_PASS`, `ID_KEY` 자리표시자를 유지합니다.
이미 공개한 실제 인증키는 앱에서 폐기·재발급하세요. 파일에서 지워도 이전 Git 기록에서는 사라지지 않습니다.

## 연결 문제 확인

- **Wi-Fi 연결 대기만 반복됨:** SSID·비밀번호, 보드가 지원하는 Wi-Fi 대역, 공유기 연결 상태를 확인합니다.
- **헤더 파일을 찾을 수 없음:** 보드 선택과 위 의존 라이브러리 설치를 확인합니다.
- **서버 접속 실패:** 인터넷 연결, DNS, TCP 55488 포트의 통신 허용 여부를 확인합니다.
- **장치가 앱에 나타나지 않음:** 예제가 장치 UI를 제공하는지, 동일 공유기인지, 게스트 네트워크·VPN을 사용하는지 확인합니다. 인증 사용 시 장치 키와 앱 계정도 확인합니다.
- **LED가 반대로 동작함:** active low / active high 방식과 핀 배치를 확인합니다.

## 라이선스 및 참고

[MIT License](LICENSE)

- [Remocon 웹앱](https://remocon.kr)
- [IOSignal 통신 실습실](https://test.iosignal.net/) · [소스 저장소](https://github.com/remocons/test_iosignal_net)
- [IOSignal Arduino 라이브러리](https://github.com/remocons/iosignal-arduino)
- [Arduino 라이브러리 규격](https://docs.arduino.cc/arduino-cli/library-specification/)

### 보드별 공개 실습

- [ESP32-C3 Super Mini 버튼/LED](examples/esp32c3-button-led): LED GPIO8, BOOT GPIO9, `#homeButton` 송수신.
- [iris 그룹 제어](examples/iris-group): WS2812 4개, GROUP/SELF 버튼, DIP 그룹·장치 번호.
- [보드별 튜토리얼](https://iosignal.net/docs/examples/workbench/start): D1 mini, ESP-01, ESP32, ESP32-S3도 보드별로 안내합니다.

기존 보드 예제는 실제 작동 확인된 보드를 기준으로 합니다. 새로 구성한 C3 버튼/LED 및 iris 공개 스케치는 별도 하드웨어 재검증 대상입니다.
