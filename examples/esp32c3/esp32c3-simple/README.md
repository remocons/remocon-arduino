# ESP32-C3 공유 스위치 LED

#switch 그룹의 s1 값으로 ESP32-C3의 내장 LED를 켭니다.

## 준비물

- 보드: ESP32-C3 Super Mini
- 네트워크: Wi-Fi 2.4GHz 공유기
- 앱: https://remocon.kr 또는 IOSignal Desktop 장치 제어 화면

## 배선

| 보드 핀 | 연결 | 비고 |
| --- | --- | --- |
| GPIO8 | 내장 LED | active low, 배선 불필요 |

## 사용 순서

1. 스케치의 WiFi 이름과 비밀번호를 실제 값으로 바꿉니다.
2. 스케치를 업로드합니다.
3. 같은 공유기에 연결한 브라우저로 https://remocon.kr 을 엽니다.
4. 공유 스위치 1번을 눌러 LED가 바뀌는지 봅니다.

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

- 보드 제품마다 핀 번호가 다를 수 있습니다.

## 컴파일

```sh
arduino-cli compile --fqbn esp32:esp32:esp32c3 examples/esp32c3/esp32c3-simple
```
