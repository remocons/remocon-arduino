# Remocon

**한국어** | [English](README.en.md)

[remocon.kr](https://remocon.kr/) 및 [IOSignal 통신 실습실(test.iosignal.net)](https://test.iosignal.net/) 웹앱과 연동하여 아두이노 장치를 원격 제어하고 시그널을 송수신하는 예제 모음입니다.
통신에는 [IOSignal Arduino 클라이언트](https://github.com/remocons/iosignal-arduino)를 사용합니다.

<p align="center">
  <img src="docs/assets/iosignal-ecosystem.png" alt="IOSignal로 연결되는 서버, CLI, Node.js, Arduino와 원격 제어 웹앱" width="720" />
</p>

## 버전

**9.0.0**부터 Remocon은 공개 [iosignal](https://github.com/remocons/iosignal)·[iosignal-arduino](https://github.com/remocons/iosignal-arduino)와 같은 버전 번호를 씁니다. 같은 번호의 패키지는 함께 쓰도록 맞춘 조합입니다. 9.6.0은 IOSignal Arduino **9.6.0 이상**이 필요합니다. 버전별 변경은 아래 [버전별 변경 사항](#버전별-변경-사항)을 참고하세요.

## 설치 및 시작

1. Arduino IDE의 라이브러리 매니저에서 `Remocon`을 검색하여 의존 라이브러리와 함께 설치합니다.
   소스로 설치하려면 [공개 저장소](https://github.com/remocons/remocon-arduino)의 ZIP을 받아 **Sketch > Include Library > Add .ZIP Library**로 추가합니다.
2. 보드 매니저에서 사용할 보드 패키지를 설치하고 **Tools > Board / Port**에서 보드와 포트를 선택합니다.
3. **File > Examples > Remocon**에서 보드에 맞는 예제를 선택합니다. 목록이 갱신되지 않으면 IDE를 재시작합니다.
4. Wi-Fi 예제의 `WIFI_SSID`, `WIFI_PASS`를 공유기 정보로 바꿉니다. 예제에 따라 `WIFI_ID` / `WIFI_KEY` 매크로 또는 `WiFi.begin()` / `wifiMulti.addAP()`의 문자열을 수정합니다. iris·릴레이 예제는 스케치 폴더에 `_secrets.h`가 있으면 그 값을 씁니다.
5. 필요한 LED·버튼·릴레이를 연결하고 핀 번호를 확인한 뒤 컴파일 및 업로드합니다.
6. 시리얼 모니터를 **115200 baud**로 열어 연결 상태를 확인합니다. ESP-01 예제는 TX 핀을 LED로 사용하므로 시리얼 출력을 사용하지 않습니다.

예제는 모두 `io.begin()` 전에 `io.setRxBuffer()`로 수신 버퍼 크기를 정합니다. 이 크기는 접속할 때 서버에 전달되므로, 예제를 고칠 때도 `setRxBuffer()`를 `begin()` 앞에 두세요. 서버는 이보다 큰 메시지를 장치에 보내지 않습니다.

공개 예제의 기본 서버는 `io.remocon.kr`, TCP 포트는 `55488`입니다.
Ethernet 예제는 DHCP를 사용하므로 유선 연결과 DHCP 가능한 공유기가 필요합니다. 여러 장치를 사용할 때는 MAC 주소를 서로 다르게 설정하세요.

## Arduino 문서

IOSignal Arduino 라이브러리 설치와 UNO Ethernet 실습은 [iosignal.net Arduino 문서](https://iosignal.net/docs/arduino/)에 있습니다. 예제별 배선과 사용 순서는 각 예제 폴더의 `README.md`를 참고하세요. 새 스케치는 공개 저장소 소스에 포함되며 Library Manager 반영 전에는 저장소에서 직접 열어 사용하세요.

## 보드별 예제

| 예제 경로 (`examples/` 아래) | 대상 보드 | 동작 / 준비물 |
| --- | --- | --- |
| `iris-group` | iris (ESP32-C3) | DIP로 그룹·장치 번호를 설정하고 버튼·웹앱으로 WS2812 4개 제어 |
| `iris-basic` | iris (ESP32-C3) | 내장 RGB LED 4개를 4구 스위치(`switch4.1`)로 켜고 끔, 버튼 2개 |
| `iris-basic-control` | iris (ESP32-C3) | 다른 iris의 `iris-basic`을 찾아 제어하고 상태를 자기 LED에 표시 |
| `iris-rgb` | iris (ESP32-C3) | LED 4개의 색상·밝기 제어(`iris.2`), DIP로 번호·그룹 |
| `esp8266_relay4ch` | ESP-12F 4채널 릴레이 보드 | 릴레이 4개(`relay4.1`), `all=1`/`all=0` |
| `Remocon8266-4ch-control` | Remocon8266 전용 보드 | CID로 4채널 릴레이 보드를 제어, 버튼 4개·D6 전체 버튼·OLED 상태 |
| `esp32-cid-led` | ESP32 + GPIO 4 외부 LED | 장치 검색·CID 명령·실제 상태 발행, Remocon 또는 공개 통신 실습실 |
| `esp32-group-led` | ESP32 1~2대 + GPIO 4 외부 LED | 그룹을 2구 스위치(`switch2.1`)로 공유, 보드별 OUTPUT_INDEX 선택 |
| `wifi-simple` | ESP8266, ESP32, ESP32-C3 | `#switch` 공유 스위치 그룹에 참여해 상태를 시리얼로 출력. `LED_PIN`을 켜면 `s1`로 LED 제어(active low, ESP32-C3 Super Mini는 GPIO 8) |
| `esp8266-d1-mini` | ESP8266 D1 mini | 내장 LED와 GPIO 14(D5) 버튼 제어 |
| `esp01-relay-button` | ESP8266 ESP-01 | 릴레이 GPIO 0, LED GPIO 1, 버튼 GPIO 2 |
| `Remocon8266` | Remocon8266 전용 보드 | OLED·버튼·출력 제어, 폴더 내 회로도 참고 |
| `uno-r4-wifi-heart` | Arduino UNO R4 WiFi | LED 매트릭스 하트와 내장 LED, D12 버튼 |
| `uno-ethernet-button` | UNO R3 + W5100 Ethernet 실드 | D2 버튼, D5 LED, D6 릴레이 출력 |
| `uno-ethernet-ir` | UNO R3 + W5100 Ethernet 실드 | D2 NEC 적외선 수신기, D5·D6 출력 |

모든 예제가 모든 보드에서 컴파일되는 것은 아닙니다. ESP32-C3 제품별 LED 종류와 핀 번호도 다를 수 있으므로 실제 보드에 맞게 수정하세요.
ESP-01은 부팅 시 GPIO 0·2 상태의 영향을 받으므로 버튼·릴레이 배선도 확인해야 합니다.

공통 통신 의존성은 `IOSignal`(9.6.0 이상), `Boho`, `Crypto`이며 예제에 따라 `Bounce2`, `U8g2`, `IRremote`, `FastLED`, `Ethernet`을 사용합니다.
Wi-Fi 및 UNO R4 LED 매트릭스 헤더는 해당 보드 패키지에서 제공합니다.
의존성 선언은 [library.properties](library.properties)에 있습니다. `architectures=*`는 패키지 전체의 메타데이터이며, 각 예제의 보드 호환성을 보장하지 않습니다.

## 웹앱에서 제어하기

- [remocon.kr](https://remocon.kr/): 장치 검색·제어 UI와 계정 기반 관리 서비스를 사용합니다.
- [IOSignal 통신 실습실 — test.iosignal.net](https://test.iosignal.net/): 보드별 프리셋 또는 직접 입력으로 송수신·구독·로그를 확인합니다. [웹앱 소스 저장소](https://github.com/remocons/test_iosignal_net)에서 내려받아 로컬 실행할 수도 있습니다.

통신 실습실에서는 보드를 선택하고 예제와 같은 서버에 연결하세요. 기본 Remocon 예제는 웹앱의 서버 URL을 `wss://io.remocon.kr/ws`로 설정합니다.

통신 실습실은 [HTTPS](https://test.iosignal.net/)와 [HTTP](http://test.iosignal.net/) 접속을 모두 제공합니다. 공개 서버에는 HTTPS + WSS를 권장합니다. HTTPS 페이지의 WS 연결은 브라우저 및 로컬 네트워크 접근 권한에 따라 제한될 수 있으며, HTTP로 전환해도 모든 제한이 해결되지는 않습니다. HTTP로 접속하려면 브라우저 주소창에 `http://test.iosignal.net`을 직접 입력하여 새로 접속하세요.

다음은 remocon.kr 사용 순서입니다.

1. 아두이노와 **동일한 공유기에 연결된 스마트폰 또는 PC**에서 [remocon.kr](https://remocon.kr)을 엽니다.
2. 장치 목록을 제공하는 예제에서는 장치가 접속하면 앱에서 해당 장치를 선택해 제어합니다.
3. `wifi-simple`은 `#switch` 그룹에 참여하며 앱에 **공유 스위치** 카드로 나타납니다. 그 카드의 스위치를 조작하여 확인하세요.

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
- **인증 후 장치가 멈춤(재접속하지 않음):** 같은 `ID_KEY`로 다른 장치나 프로그램이 로그인하면 서버가 이전 연결을 끊고, 장치는 다시 접속하지 않고 멈춥니다(IOSignal Arduino 7.0.0부터). 장치마다 다른 인증키를 사용하세요.
- **큰 메시지가 오지 않음:** 수신 버퍼(`setRxBuffer`)보다 큰 메시지는 서버가 보내지 않거나(9.0 서버), 장치가 건너뜁니다. 필요하면 버퍼 크기를 늘리세요. 보드 메모리를 고려해야 합니다.

## 버전별 변경 사항

### 9.6.0

- **버전 번호를 공개 iosignal 9.6.0과 맞췄습니다.** 의존성을 `IOSignal (>=9.6.0)`으로 올렸습니다. IOSignal Arduino 9.6.0에는 `unsubscribe()`와 전송 실패 시 재접속이 들어 있습니다.
- 새 예제: `iris-basic`, `iris-basic-control`, `iris-rgb`, `esp8266_relay4ch`, `Remocon8266-4ch-control`.
- `iris-group`, `uno-ethernet-button`, `uno-ethernet-ir`이 장치 검색·제어 규칙(`@$info`, `key=value` 상태, `#search`/`#notify`)을 따릅니다. `uno-ethernet-ir`의 리모컨 키 코드가 바뀌었습니다.
- `Remocon8266`, `esp01-relay-button`, `esp8266-d1-mini`, `uno-r4-wifi-heart`, `esp32-cid-led`, `esp32-group-led`, `wifi-simple`, `esp32c3/esp32c3-simple`도 같은 규칙을 따르도록 바꿨습니다. 이전 방식(`$name`/`$ui`, `#scan/ch`, `#$switch`)은 더 쓰지 않습니다. 공유 스위치 채널은 `#switch`입니다. 이 8개는 컴파일과 규칙 검사만 했고 실제 보드에서는 확인하지 않았습니다.
- `esp32c3-button-led`와 `esp32s3-ws2812` 예제를 삭제했습니다.
- 예제마다 `README.md`가 있습니다.

### 9.0.0

- **버전 번호를 공개 iosignal과 맞췄습니다.** 4.0.0 다음 버전입니다.
- 의존성을 `IOSignal (>=9.0.0)`으로 올렸습니다. IOSignal Arduino 9.0.0은 수신 버퍼 크기를 서버에 알리고, 버퍼보다 큰 메시지는 연결을 끊지 않고 건너뜁니다.
- 예제 코드는 바뀌지 않았습니다. 모든 예제가 이미 `begin()` 전에 `setRxBuffer()`를 호출합니다.

### 4.0.0

- 의존성을 `IOSignal (>=7.0.0)`으로 지정했습니다. 같은 ID로 다른 곳에서 로그인해 연결이 교체되면 장치가 익명으로 다시 접속하지 않고 멈춥니다.

### 3.0.0

- 보드별 튜토리얼과 제어 예제(`esp32c3-button-led`, `iris-group`, `esp32-cid-led`, `esp32-group-led` 등)를 추가했습니다.

## 라이선스 및 참고

[MIT License](LICENSE)

- [Remocon 웹앱](https://remocon.kr)
- [IOSignal 통신 실습실](https://test.iosignal.net/) · [소스 저장소](https://github.com/remocons/test_iosignal_net)
- [IOSignal Arduino 라이브러리](https://github.com/remocons/iosignal-arduino)
- [Arduino 라이브러리 규격](https://docs.arduino.cc/arduino-cli/library-specification/)

### 보드별 공개 실습

- [iris 그룹 제어](examples/iris-group): WS2812 4개, GROUP/SELF 버튼, DIP 그룹·장치 번호.

