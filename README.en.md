# Remocon

[한국어](README.md) | **English**

Arduino remote-control and messaging examples for the [remocon.kr](https://remocon.kr/) and [IOSignal Communication Workbench (test.iosignal.net)](https://test.iosignal.net/) web apps.
Communication uses the [IOSignal Arduino client](https://github.com/remocons/iosignal-arduino).

<p align="center">
  <img src="docs/assets/iosignal-ecosystem.png" alt="Server, CLI, Node.js, Arduino and remote web apps connected through IOSignal" width="720" />
</p>

## Versions

From **9.0.0**, Remocon uses the same version number as the public [iosignal](https://github.com/remocons/iosignal) and [iosignal-arduino](https://github.com/remocons/iosignal-arduino) packages. Packages with the same number are meant to be used together. 9.14.3 requires IOSignal Arduino **9.14.0 or later** (9.14.1 or later for anonymous login in the board settings). See [Changes by version](#changes-by-version) below.

## Installation and getting started

1. Search for `Remocon` in the Arduino IDE Library Manager and install it with its dependencies.
   To install from source, download a ZIP from the [public repository](https://github.com/remocons/remocon-arduino) and add it through **Sketch > Include Library > Add .ZIP Library**.
2. Install the appropriate board package in Boards Manager, then select your board and port under **Tools > Board / Port**.
3. Choose an example for your board under **File > Examples > Remocon**. Restart the IDE if the list has not updated.
4. Replace `WIFI_SSID` and `WIFI_PASS` with your Wi-Fi credentials. Depending on the example, edit the `WIFI_ID` / `WIFI_KEY` macros or the strings passed to `WiFi.begin()` / `wifiMulti.addAP()`. The iris and relay examples use `_secrets.h` in the sketch folder when it exists.
5. Connect any required LEDs, buttons, or relays, check the pin assignments, then compile and upload.
6. Open the Serial Monitor at **115200 baud** to check the connection. The ESP-01 example uses the TX pin for an LED and does not provide serial output.

Every example sets its receive buffer with `io.setRxBuffer()` before `io.begin()`. The size is sent to the server when connecting, so keep `setRxBuffer()` before `begin()` when you change an example. The server does not send the device messages larger than that.

The default server for the public examples is `io.remocon.kr`, using TCP port `55488`.
Ethernet examples use DHCP and require a wired connection to a router that supports DHCP. Assign a unique MAC address to each device when using multiple devices.

## Arduino documentation

Installing the IOSignal Arduino library and the UNO Ethernet exercise are in the [iosignal.net Arduino docs](https://iosignal.net/docs/arduino/). See the `README.md` in each example folder for wiring and steps. New sketches are included in the public repository; open them directly from the repository until they become available through Library Manager.

## Examples by board

| Example path (under `examples/`) | Target board | Behavior / hardware |
| --- | --- | --- |
| `iris-group` | iris (ESP32-C3) | Set group and device numbers with DIP switches; control four WS2812 LEDs using buttons or the web app |
| `iris-basic` | iris (ESP32-C3) | Turn the four built-in RGB LEDs on and off as a 4-way switch (`switch4.1`), two buttons |
| `iris-basic-controller` | iris (ESP32-C3) | Find another iris running `iris-basic`, control it and mirror its state on its own LEDs |
| `iris-button-controller` | iris (ESP32-C3) | Two buttons control another device (target CID and button actions are board settings); the four LEDs show the target's channels |
| `iris-rgb` | iris (ESP32-C3) | Hue and brightness for four LEDs (`iris.2`), number and group from DIP switches |
| `esp8266-relay4ch` | ESP-12F 4-channel relay board | Four relays (`relay4.1`), `all=1`/`all=0` |
| `Remocon8266-button-controller` | Remocon8266 board | Buttons control another device by CID (the defaults fit a 4-channel relay board): four buttons, a D6 button, OLED state; button actions are board settings |
| `button-controller` | ESP8266 D1 mini, ESP32, ESP32-C3, ESP32-S3 + two buttons | Two buttons control another device (relay board, light, group channel); target CID and button actions are board settings |
| `esp32-cid-led` | ESP32 + external LED on GPIO 4 | Device discovery, CID commands, and actual state reporting with Remocon or the public workbench |
| `esp32-group-led` | One or two ESP32 boards + external LED on GPIO 4 | Share a group as a 2-way switch (`switch2.1`); each board's output is the board setting `index` |
| `wifi-simple` | ESP-01, ESP8266, ESP32, ESP32-C3 | Joins the `#switch` shared switch group and prints its state. Board settings `out`, `out_high` and `button` add an LED or relay following `s1` and a button toggling `s1` |
| `d1-mini-led-button` | ESP8266 D1 mini | Built-in LED and GPIO 14 (D5) button control |
| `esp01-relay-button` | ESP8266 ESP-01 | Relay on GPIO 0, LED on GPIO 1, button on GPIO 2 |
| `Remocon8266` | Dedicated Remocon8266 board | OLED, button, and output control; see the schematic in the folder |
| `uno-r4-wifi-heart` | Arduino UNO R4 WiFi | LED matrix heart, built-in LED, and D12 button |
| `uno-ethernet-button` | UNO R3 + W5100 Ethernet shield | D2 button, D5 LED, D6 relay output |
| `uno-ethernet-ir` | UNO R3 + W5100 Ethernet shield | D2 NEC infrared receiver, D5 and D6 outputs |
| `esp32cam` | ESP32-CAM (AI Thinker) | Sends pictures to the remocon.kr device card (`camera.1`), optionally encrypted with an E2E key |

Not every example compiles for every board. LED types and pin assignments can also vary between ESP32-C3 products; adjust the sketch for your actual board.
On ESP-01, GPIO 0 and GPIO 2 affect boot behavior, so check the button and relay wiring as well.

The common communication dependencies are `IOSignal` (9.13.0 or later), `Boho`, and `Crypto`. Individual examples also use `Bounce2`, `U8g2`, `IRremote`, `FastLED`, or `Ethernet`.
Wi-Fi and UNO R4 LED matrix headers are provided by the relevant board package.
Dependencies are declared in [library.properties](library.properties). The `architectures=*` declaration is package metadata and does not guarantee that every example supports every board.

## Controlling devices from the web apps

- [remocon.kr](https://remocon.kr/): Device discovery and control UI, with account-based management services.
- [IOSignal Communication Workbench — test.iosignal.net](https://test.iosignal.net/): Send, receive, subscribe, and inspect logs using board presets or manual input. You can also download the [web app source](https://github.com/remocons/test_iosignal_net) and run it locally.

In the workbench, select your board and connect to the same server as the example. For the default Remocon examples, set the web app server URL to `wss://io.remocon.kr/ws`.

The workbench is available over both [HTTPS](https://test.iosignal.net/) and [HTTP](http://test.iosignal.net/). HTTPS + WSS is recommended for public servers. WS connections from an HTTPS page may be restricted by the browser and local network access permissions; switching to HTTP does not resolve every restriction. To use HTTP, enter `http://test.iosignal.net` directly in the browser address bar and open it anew.

To use remocon.kr:

1. Open [remocon.kr](https://remocon.kr) on a **phone or PC connected to the same router as the Arduino**.
2. For examples that provide a device listing, select the device in the app after it connects, then control it.
3. `wifi-simple` joins the `#switch` group and appears in the app as a **shared switch** card. Operate the switches on that card to test it.

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
- **Device stops after authentication and does not reconnect:** When another device or program logs in with the same `ID_KEY`, the server closes the earlier connection and that device stops instead of reconnecting (IOSignal Arduino 7.0.0 and later). Use a separate key for each device.
- **Large messages do not arrive:** Messages larger than the receive buffer (`setRxBuffer`) are not sent by a 9.0 server, or are skipped by the device. Increase the buffer if needed, keeping the board's memory in mind.

## Changes by version

### Next release (not released yet)

- **Example folders were renamed.** Behaviour is unchanged. The names in the Arduino IDE examples menu change with them.
  - `Remocon8266-4ch-control` → `Remocon8266-button-controller` (it also controls devices other than relays)
  - `iris-button-control` → `iris-button-controller`
  - `iris-basic-control` → `iris-basic-controller`
  - `esp8266_relay4ch` → `esp8266-relay4ch`
  - `esp8266-d1-mini` → `d1-mini-led-button`
- The notes for earlier versions below keep the names used at the time.

### 9.14.3

- **The board setting `category` is now `name` and sets the device name too.** Its value is the `$name` itself, `name:category` (for example `Living Room Light:1F`). `:1F` keeps the sketch's name; without `:` there is no category. Empty uses the sketch default (for example `Iris Basic:HOME`). The board settings page shows it as `Name & category (name:category)`.
- A value stored under `category` is not read; set `name` again after updating if needed.
- New `remocon::formatNameSetting()` (`RemoconUi.h`) builds `$name` from the setting.
- **New examples `button-controller` and `iris-button-control`:** general controllers whose buttons control another device (a 4-channel relay board, a light, a group channel). The target CID and what each button does (b1, b2) are board settings. `button-controller` runs on a D1 mini, ESP32, ESP32-C3 or ESP32-S3 with two buttons; `iris-button-control` runs on the iris board and shows the target's channels on its four LEDs.
- `wifi-simple`: board settings `out` (an LED or relay pin following s1), `out_high` and `button` (a pin that toggles s1) make it an ESP-01 relay or button board too; it now builds for the ESP-01.

### 9.14.2

- **The board setting `group` is now `category`.** It is the app tab (remocon.kr and others) the device is listed under; the device sends `$name` as `name:category` (for example `Iris Basic:HOME`). It was renamed so it is not mistaken for a group channel; the board settings page shows it as `App tab (category)`.
- A value stored under `group` is not read. After the update the default applies (`HOME`; `iris G<group number>` for the iris group and RGB examples), so set `category` again on the [board settings page](https://test.iosignal.net/wifi/) if needed.
- The board settings table on the [test.iosignal.net](https://test.iosignal.net/) example pages explains each setting in Korean: what it does, the default and what empty means.

### 9.14.1

- **`Remocon8266-4ch-control` reads the target relay's state again.** When its buttons moved to `RemoconButtons.h` after 9.13.0, the line that passed the received `$state` on was lost: the controller stayed at `relays=----`, `link=0`, and toggles from its buttons or card sent nothing. Reinstall boards that run this example.

### 9.14.0

- **Requires IOSignal Arduino 9.14.0.** The dependency is now `IOSignal (>=9.14.0)`. 9.14.0 keeps its log texts in flash and saves RAM on the UNO (AVR): `uno-ethernet-ir` now has 822 bytes of RAM free instead of 394 and no longer stops during boot.
- **Device rules v2: devices send `$name` and `$ui` instead of `$info`.** `$name` is `name:category` (for example `Iris Basic:HOME`; the part after the first `:` is the app's category tab, none means uncategorized). `$ui` is a button list (`on,off,toggle`) or an official type such as `t=switch4.1`. A device with a screen of its own sends `p=1;b=…`. remocon.kr, the Android app and IOSignal Desktop no longer read `$info`.
- **New header `RemoconUi.h`:** `formatName()` builds `$name` (cleaning `:` and `;` out of the name) and `nameIs()` compares another device's `$name`. `RemoconBinding.h` and `RemoconButtons.h` for the button controllers are included too.
- The board setting `group` keeps its name and is now the category in `$name`. Values already stored still apply.
- `iris-basic-control` finds its target by the name in `$name` (`Iris Basic`).

### 9.13.0

- **Requires IOSignal Arduino 9.13.0.** The dependency is now `IOSignal (>=9.13.0)`.
- **The ESP examples use board settings.** Over USB, the [board settings page](https://test.iosignal.net/wifi/) (desktop Chrome or Edge) sets Wi-Fi, the server (host:port), the device key and each example's settings, and the board keeps them. No sketch edits are needed, and the boards no longer block waiting for Wi-Fi. The example pages on [test.iosignal.net](https://test.iosignal.net/) can also install the firmware directly.
- **The app group (`g=`) is the board setting `group`.** It defaults to `HOME` and is announced again at once when changed. The iris examples use `iris G<group number>` when it is empty. The UNO examples send `g=HOME`.
- **New example `esp32cam`:** an ESP32-CAM whose pictures show on its remocon.kr device card (official type `camera.1`). With the board setting `e2e_key` the pictures are encrypted end to end, and the card needs the same key. The interval and sending on or off are set from the card.
- `Remocon8266-4ch-control`: the target CID is the board setting `target`. The remote `target=` command still works. A target stored in flash by an earlier version is moved over once.
- `esp32-group-led`: the state key to follow is the board setting `index` (0: `s1`, 1: `s2`).
- The ESP-01 examples drive the LED on GPIO 1 (TX), so they turn on serial and accept board settings only when no Wi-Fi is stored or the button is held within 3 s after power on.

### 9.12.0

- **The version number now matches the public iosignal 9.12.0 package.** The dependency is now `IOSignal (>=9.12.0)`.
- Every example sends `$info` with the short keys (`n=`, `g=`, `t=`) and the type names `<name>.<major>` (`light.1`, `switch4.1` and so on). remocon.kr still reads the old form.
- `Remocon8266-4ch-control`: the card screen is sent in `$panel`, with `p=1` in `$info`. The bottom OLED line shows this board's CID. The default target `DEFAULT_TARGET` is the placeholder `the-cid`; set it to your relay board's CID. A target set remotely and kept in flash is used first.
- `esp8266_relay4ch`: the `$info` group is now `g=relays`.
- `esp32c3/esp32c3-simple` was merged into `wifi-simple`. Uncomment `LED_PIN` to drive an LED with `s1`.
- Parts, wiring, steps and a browser control panel for each example are on [test.iosignal.net](https://test.iosignal.net/) (Korean).

### 9.6.0

- **The version number now matches the public iosignal 9.6.0 package.** The dependency is now `IOSignal (>=9.6.0)`, which adds `unsubscribe()` and reconnects when a send does not go out.
- New examples: `iris-basic`, `iris-basic-control`, `iris-rgb`, `esp8266_relay4ch`, `Remocon8266-4ch-control`.
- `iris-group`, `uno-ethernet-button` and `uno-ethernet-ir` follow the device discovery and control rules (`@$info`, `key=value` state, `#search`/`#notify`). The remote key codes in `uno-ethernet-ir` changed.
- `Remocon8266`, `esp01-relay-button`, `esp8266-d1-mini`, `uno-r4-wifi-heart`, `esp32-cid-led`, `esp32-group-led`, `wifi-simple` and `esp32c3/esp32c3-simple` now follow the same rules too. The old tags (`$name`/`$ui`, `#scan/ch`, `#$switch`) are no longer used; the shared-switch channel is `#switch`. These eight were compiled and checked against the rules, not tested on boards.
- The `esp32c3-button-led` and `esp32s3-ws2812` examples were removed.
- Each example has a `README.md`.

### 9.0.0

- **The version number now matches the public iosignal package.** This release follows 4.0.0.
- The dependency is now `IOSignal (>=9.0.0)`. IOSignal Arduino 9.0.0 tells the server its receive buffer size and skips messages larger than the buffer instead of disconnecting.
- The example code is unchanged. Every example already calls `setRxBuffer()` before `begin()`.

### 4.0.0

- The dependency became `IOSignal (>=7.0.0)`. When the connection is replaced by a login with the same ID elsewhere, the device stops instead of reconnecting anonymously.

### 3.0.0

- Added board tutorials and control examples (`esp32c3-button-led`, `iris-group`, `esp32-cid-led`, `esp32-group-led` and others).

## License and references

[MIT License](LICENSE)

- [Remocon web app](https://remocon.kr)
- [IOSignal Communication Workbench](https://test.iosignal.net/) · [Source repository](https://github.com/remocons/test_iosignal_net)
- [IOSignal Arduino library](https://github.com/remocons/iosignal-arduino)
- [Arduino library specification](https://docs.arduino.cc/arduino-cli/library-specification/)

### Public board exercises

- [iris group control](examples/iris-group): Four WS2812 LEDs, GROUP/SELF buttons, DIP switches for group and device numbers.

