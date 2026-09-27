# Remocon

 This is an example of remote control of an Arduino in conjunction with a web browser app (https://remocon.kr).
 The  [`IOSignal`](https://github.com/remocons/iosignal-arduino) Arduino client library was used.


 웹브라우저 https://remocon.kr 웹앱과 연동하여 아두이노를 원격 제어하는 예제소스입니다.
  [`IOSignal`](https://github.com/remocons/iosignal-arduino) 아두이노 클라이언트 라이브러리가 사용되었습니다.



## 호환 장치

Most Arduino boards that are compatible with the IOSignal library are also compatible. Please refer to the IOSignal library when using a general purpose Arduino compatible board.

IOSignal 라이브러리와 호환되는 대부분의 아두이노 보드도 호환 가능합니다.
범용 아두이노 호환 보드 사용시 `IOSignal` 라이브러리를 참고하시기 바랍니다.

- Arduino UnoR3 + Ethernet shield W5100
- ESP8266
- ESP32
- Arduino Uno R4 WiFi

## 다운로드 설치

1. 아두이노 라이브러리 매니저에서 'remocon' 검색하여 설치합니다.
2. 아두이노 IDE를 재시작합니다.

## 예제 업로딩

1. File > Examples > Examples for custom libraries > Remocon 에서 예제를 선택합니다.
2. WiFi 예제는 공유기 SSID와 PW를 수정합니다.
3. 컴파일 및 업로딩 합니다.
4. 장치가 정상 작동시 리모컨 서버에 자동 접속됩니다.

## IoT 리모컨 앱으로 원격제어

1. 아두이노와 동일한 스마트폰에서 https://remocon.kr 에 접속합니다.
2. 아두이노 장치가 정상 작동된 경우, 리모컨 앱에 장치정보가 인식됩니다.
3. 앱에서 장치 제어 버튼을 눌러서 제어합니다.

- 리모컨 앱은 가입없이 사용가능합니다.
- 아두이노는 인증키 없이 사용 가능합니다.
- 단, 동일 공유기에 접속된 장치만 제어 가능합니다.

## 리모컨앱 가입 및 인증키 발급

- 리모컨앱과 리모컨장치간에 안전한 암호통신을 위해서는 인증키 사용이 필수적입니다.
- 리모컨 앱 가입 및 장치에 인증키 등록시 외부 네트웍에서 원격제어가 가능해집니다.
- 리모컨앱 가입은 간단히 구글ID로 로그인만 하시면 가입됩니다.
- 인증키는 현재 가입(로그인)시 2개가 자동 생성됩니다.(무료)
- 해당 인증키로 2개의 장치의 암호화된 글로벌 원격 제어 가능합니다.

## 아두이노에 인증키 등록하기

- 리모컨앱 로그인시 계정(Account) 메뉴를 선택합니다.
- 장치 목록 정보에서 ID와KEY정보가 합쳐진 ID_KEY 정보를 복사하세요.
- ID_KEY는 `ar89.CCehesdtXvznC-9Z3322` 유형으로 보입니다.
- 아두이노 소스코드에서 io.auth("id_key"); 부분을 수정합니다.
- 기본 상태는 io.auth()부분이 주석처리 된 상태입니다.

```
// io.auth("id_key");
```

위 코드를 아래와 같이 수정 후, 주석을 제거합니다.

```
io.auth("ar89.CCehesdtXvznC-9Z3322");

```



## 관련 사이트

### [remocon.kr](https://remocon.kr)

### [IOSignal.net](https://iosignal.net)