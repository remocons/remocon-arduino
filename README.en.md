# Remocon

[한국어](README.md) | **English**

Arduino remote-control and messaging examples for the [remocon.kr](https://remocon.kr/) and [IOSignal Communication Workbench (test.iosignal.net)](https://test.iosignal.net/) web apps.
Communication uses the [IOSignal Arduino client](https://github.com/remocons/iosignal-arduino).

<p align="center">
  <img src="docs/assets/iosignal-ecosystem.png" alt="Server, CLI, Node.js, Arduino and remote web apps connected through IOSignal" width="720" />
</p>

## Installation and getting started

1. Search for `Remocon` in the Arduino IDE Library Manager and install it with its dependencies.
   To install from source, download a ZIP from the [public repository](https://github.com/remocons/remocon-arduino) and add it through **Sketch > Include Library > Add .ZIP Library**.
2. Install the appropriate board package in Boards Manager, then select your board and port under **Tools > Board / Port**.
3. Choose an example for your board under **File > Examples > Remocon**. Restart the IDE if the list has not updated.
4. Replace `WIFI_SSID` and `WIFI_PASS` with your Wi-Fi credentials. Depending on the example, edit the `WIFI_ID` / `WIFI_KEY` macros or the strings passed to `WiFi.begin()` / `wifiMulti.addAP()`.
5. Connect any required LEDs, buttons, or relays, check the pin assignments, then compile and upload.
6. Open the Serial Monitor at **115200 baud** to check the connection. The ESP-01 example uses the TX pin for an LED and does not provide serial output.

The default server for the public examples is `io.remocon.kr`, using TCP port `55488`.
Ethernet examples use DHCP and require a wired connection to a router that supports DHCP. Assign a unique MAC address to each device when using multiple devices.

## Board tutorials

The [public workbench and ESP32 tutorials](https://iosignal.net/docs/examples/workbench/start) cover basic communication, individual control by CID, group control, and RGB LEDs in sequence. New sketches are included in the public repository; open them directly from the repository until they become available through Library Manager.

## Examples by board

| Example path (under `examples/`) | Target board | Behavior / hardware |
| --- | --- | --- |
| `esp32c3-button-led` | ESP32-C3 Super Mini | Send `#homeButton` with the GPIO 9 button; toggle the GPIO 8 LED on receipt |
| `iris-group` | iris (ESP32-C3) | Set group and device numbers with DIP switches; control four WS2812 LEDs using buttons or the web app |
| `esp32-cid-led` | ESP32 + external LED on GPIO 4 | Device discovery, CID commands, and actual state reporting with Remocon or the public workbench |
| `esp32-group-led` | One or two ESP32 boards + external LED on GPIO 4 | Control group outputs with a two-character state string; select OUTPUT_INDEX for each board |
| `wifi-simple` | ESP8266, ESP32 | Print the `#$switch` shared-switch state to serial |
| `esp32c3/esp32c3-simple` | Based on ESP32-C3 Super Mini | Control the GPIO 8 LED with the shared switch; active low |
| `esp8266-d1-mini` | ESP8266 D1 mini | Built-in LED and GPIO 14 (D5) button control |
| `esp01-relay-button` | ESP8266 ESP-01 | Relay on GPIO 0, LED on GPIO 1, button on GPIO 2 |
| `Remocon8266` | Dedicated Remocon8266 board | OLED, button, and output control; see the schematic in the folder |
| `esp32s3-ws2812` | ESP32-S3 | WS2812 on GPIO 48, BOOT button on GPIO 0, `#homeButton` / `#robot` channels |
| `uno-r4-wifi-heart` | Arduino UNO R4 WiFi | LED matrix heart, built-in LED, and D12 button |
| `uno-ethernet-button` | UNO R3 + W5100 Ethernet shield | D2 button, D5 LED, D6 relay output |
| `uno-ethernet-ir` | UNO R3 + W5100 Ethernet shield | D2 NEC infrared receiver, D5 and D6 outputs |

Not every example compiles for every board. LED types and pin assignments can also vary between ESP32-C3/S3 products; adjust the sketch for your actual board.
On ESP-01, GPIO 0 and GPIO 2 affect boot behavior, so check the button and relay wiring as well.

The common communication dependencies are `IOSignal`, `Boho`, and `Crypto`. Individual examples also use `Bounce2`, `U8g2`, `IRremote`, `FastLED`, or `Ethernet`.
Wi-Fi and UNO R4 LED matrix headers are provided by the relevant board package.
Dependencies are declared in [library.properties](library.properties). The `architectures=*` declaration is package metadata and does not guarantee that every example supports every board.

## Controlling devices from the web apps

- [remocon.kr](https://remocon.kr/): Device discovery and control UI, with account-based management services.
- [IOSignal Communication Workbench — test.iosignal.net](https://test.iosignal.net/): Send, receive, subscribe, and inspect logs using board presets or manual input. You can also download the [web app source](https://github.com/remocons/test_iosignal_net) and run it locally.

In the workbench, select your board and connect to the same server as the example. For the default Remocon examples, set the web app server URL to `wss://io.remocon.kr/ws`. See the [tutorials](https://iosignal.net/docs/examples/workbench/start) for board-specific connection and control steps.

`esp32s3-ws2812` is a channel messaging example for the RGB preset in the [IOSignal Communication Workbench (test.iosignal.net)](https://test.iosignal.net/). A `#homeButton` signal changes the color. Send a four-byte payload to `#robot` to use its first three bytes as RGB values.

The workbench is available over both [HTTPS](https://test.iosignal.net/) and [HTTP](http://test.iosignal.net/). HTTPS + WSS is recommended for public servers. WS connections from an HTTPS page may be restricted by the browser and local network access permissions; switching to HTTP does not resolve every restriction. To use HTTP, enter `http://test.iosignal.net` directly in the browser address bar and open it anew. See the [connection requirements and local practice guide](https://iosignal.net/docs/examples/workbench/start).

To use remocon.kr:

1. Open [remocon.kr](https://remocon.kr) on a **phone or PC connected to the same router as the Arduino**.
2. For examples that provide a device listing, select the device in the app after it connects, then control it.
3. `wifi-simple` and `esp32c3-simple` subscribe to the **shared-switch** channel without registering a device button UI. Operate the app's shared switch to test them.

Use without an authentication key assumes that the devices are connected to the same router. To control your device from an external network, configure a device authentication key from your app account.
Check the current web app guidance for account menus, authentication key quotas, and service availability.

## Setting a device authentication key

Sign in to the app and copy `ID_KEY` from the device information under Account.
In the sketch's `setup()`, uncomment the following line and replace the placeholder with your own value. If the line is absent, add it after `io.begin()` and callback registration.

```cpp
io.auth("ID_KEY"); // Replace ID_KEY with the actual value issued by the app
```

Do not publish Wi-Fi passwords or device authentication keys in the public repository. Keep the `WIFI_SSID`, `WIFI_PASS`, and `ID_KEY` placeholders in distributed examples.
Revoke and reissue any real authentication key that has already been published. Removing it from a file does not remove it from earlier Git history.

## Troubleshooting

- **Stuck waiting for Wi-Fi:** Check the SSID, password, Wi-Fi bands supported by the board, and router connectivity.
- **Header file not found:** Check your board selection and installation of the dependencies listed above.
- **Server connection fails:** Check internet connectivity, DNS, and whether communication on TCP port 55488 is allowed.
- **Device does not appear in the app:** Check whether the example provides a device UI, whether you are on the same router, and whether a guest network or VPN is in use. When using authentication, also check the device key and app account.
- **LED behavior is inverted:** Check active-low / active-high behavior and pin assignments.

## License and references

[MIT License](LICENSE)

- [Remocon web app](https://remocon.kr)
- [IOSignal Communication Workbench](https://test.iosignal.net/) · [Source repository](https://github.com/remocons/test_iosignal_net)
- [IOSignal Arduino library](https://github.com/remocons/iosignal-arduino)
- [Arduino library specification](https://docs.arduino.cc/arduino-cli/library-specification/)

### Public board exercises

- [ESP32-C3 Super Mini button/LED](examples/esp32c3-button-led): LED on GPIO 8, BOOT on GPIO 9, `#homeButton` sending and receiving.
- [iris group control](examples/iris-group): Four WS2812 LEDs, GROUP/SELF buttons, DIP switches for group and device numbers.
- [Board tutorials](https://iosignal.net/docs/examples/workbench/start): Also covers D1 mini, ESP-01, ESP32, and ESP32-S3.

Existing board examples are based on boards whose operation has been verified. The newly assembled C3 button/LED and iris public sketches require separate hardware revalidation.
