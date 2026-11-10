# WiFi 공유 스위치 수신

#switch 그룹에 참여해 공유 상태를 시리얼로 출력합니다. 하드웨어 없이 통신만 확인하는 가장 간단한 예제입니다.

## 준비물

- 보드: ESP8266 D1 mini
- 보드: ESP32 개발 보드
- 네트워크: Wi-Fi 2.4GHz 공유기
- 앱: https://remocon.kr 또는 IOSignal Desktop 장치 제어 화면

## 사용 순서

1. 스케치의 WiFi 이름과 비밀번호를 실제 값으로 바꿉니다.
2. 스케치를 업로드하고 시리얼 모니터 115200을 엽니다.
3. 같은 공유기에 연결한 브라우저로 https://remocon.kr 을 엽니다.
4. 공유 스위치를 눌러 시리얼에 s1=1 같은 값이 나오는지 봅니다.

## 시그널 절차

| 보내는 쪽 | 받는 쪽 | 태그 | 값 | 설명 |
| --- | --- | --- | --- | --- |
| 장치 | 서버 | `#switch$state` | `구독` | 공유 상태를 구독합니다. 보관된 값이 있으면 바로 받습니다. |
| 장치 | 서버 | `#search` | `구독` | 앱의 검색 요청을 받도록 구독합니다. |
| 장치 | 서버 | `#switch$info` | `name=공유 스위치;type=remocon/switch3/v1` | 그룹 정보입니다. 서버에 보관됩니다. |
| 장치 | 앱 | `#notify` | `#switch` | 그룹은 CID 대신 채널 이름으로 알립니다. |
| 앱 | 장치 | `#search` |  | 앱이 그룹을 찾습니다. |
| 장치 | 앱 | `#notify` | `#switch` | 구성원마다 답하고 앱은 하나로 합칩니다. |
| 앱 | 장치 | `#switch$state` | `s1=1;s2=0;s3=0` | 앱이 전체 문자열을 다시 씁니다. 구성원은 자기 키를 읽습니다. |

## 참고

- 값을 출력만 하고 핀은 제어하지 않습니다.

## 컴파일

```sh
arduino-cli compile --fqbn esp8266:esp8266:d1_mini examples/wifi-simple
arduino-cli compile --fqbn esp32:esp32:esp32 examples/wifi-simple
```
