# MoonPixel

A Wi-Fi moon lamp: an ESP32-C3 SuperMini drives a 2×2 WS2812 LED panel inside a 3D-printed
base, under a moon-shaped diffuser. You control it from a phone web app it serves itself.

Original hardware design and starter code by
[muztahiddurjoy](https://github.com/muztahiddurjoy/moonpixel). This fork rewrites the firmware.

## Features

- **13 effects:** Moon, Moon glow, Colour, Breathe, Candle, Sunset, Ember, Rainbow, Pastel,
  Ocean, Aurora, Twilight and Gradient (colours circling the moon). All animations are smooth
  and tuned so that they stay visible through the diffuser.
- **Phone and desktop web app:** colour wheel, swatches, brightness and speed sliders. The layout
  is one column on phones and two columns on tablets and laptops. Add it to your iPhone home
  screen, or to the Mac Dock with Safari's **File → Add to Dock**, to get a full-screen app with
  its own moon icon. It's built for a weak Wi-Fi link: taps
  apply at once, requests are queued, and lost requests are retried.
- **Wi-Fi that copes with a weak signal:**
  - It stores up to 3 networks and joins the strongest one it can hear.
  - Long-range mode (802.11b only) works at weaker signals than the normal mode.
  - Transmit power adjusts automatically.
  - It reconnects on its own after a dropout.
- **No-flicker LED driver:** the LED data goes out through SPI + DMA, so Wi-Fi activity can't
  corrupt a frame.
- **Optional Bluetooth LE control.** It's off by default, because on the C3 it slows Wi-Fi.

## Hardware

| Part | Notes |
|---|---|
| ESP32-C3 SuperMini | Powered and programmed over USB-C |
| 2×2 WS2812 5050 panel | Plain RGB, GRB colour order |
| 330 Ω resistor | In series with the LED data line |
| 3D-printed base | `hardware/cad/` (STL, STEP, SolidCraft) |

**Wiring**

| LED panel | SuperMini |
|---|---|
| VCC | 5V (VBUS) |
| GND | GND |
| DIN | GPIO 10, through the 330 Ω resistor |

> **Tip:** The SuperMini's Wi-Fi antenna is the end opposite the USB-C port. Keep it out from
> under the LED panel and wires. Covering it noticeably shortens the Wi-Fi range.

## Build and flash (macOS)

```bash
brew install platformio
cd firmware
pio run -t upload        # build + flash
pio device monitor       # serial log, 115200 baud
```

**If the case is sealed and you can't reach the BOOT button,** and `pio run -t upload` fails
with "No serial data received", put the chip into download mode over USB and flash at a lower
speed. Run this from `firmware/`, after `pio run`:

```bash
python3 ~/.platformio/packages/tool-esptoolpy/esptool.py --chip esp32c3 \
  --port $(ls /dev/cu.usbmodem* | head -1) --baud 115200 \
  --before usb_reset --after hard_reset --no-stub \
  write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB \
  0x10000 .pio/build/esp32c3_supermini/firmware.bin
```

This writes only the program, so saved Wi-Fi networks and lamp settings are kept. On some
setups you have to use PlatformIO's own Python instead of `python3`.

## First-time setup

1. Power the lamp. With no saved network, it opens a hotspot called **MoonPixel-Setup**.
2. Join that hotspot and open **http://192.168.4.1**.
3. Open **Wi-Fi & settings**, tap **Scan**, pick your network, enter its password and tap
   **Save network**.
4. The lamp joins your Wi-Fi and the hotspot switches off. Find the lamp's address in your
   router's client list, or in the serial log. To make the address permanent, reserve it on
   your router.
5. On iPhone, open that address in Safari, then tap **Share → Add to Home Screen**. On a Mac, use
   Safari's **File → Add to Dock**.

To get the setup hotspot back later, use **Forget all Wi-Fi** on the page, or send `hotspot`
over Bluetooth.

## Controls

**Web app:** everything is on the page. The power button is at the top, followed by the colour
wheel, the effect tiles and the sliders. Wi-Fi, transmit power, long-range mode, Bluetooth and
diagnostics are under **Wi-Fi & settings**. `/diag` gives a full plain-text diagnostic.

**Bluetooth LE:** turn it on under settings first. In a BLE app such as nRF Connect, write
UTF-8 text to characteristic `4d4f4f4e-5049-5845-4c00-000000000002`:

```
on | off | toggle | mode <name> | color FF8800 | bri 1-255 | spd 1-10
wifi Network|Password | forget | hotspot | reboot
```

Mode names are `moon glow solid breathe candle sunset ember rainbow pastel ocean aurora
twilight gradient`. You can send several commands at once by separating them with `;`.
Characteristic `…0003` (read/notify) reports the current state.

**USB serial** (115200 baud):

| Command | What it does |
|---|---|
| `d` | Full diagnostics |
| `scan` | Wi-Fi scan |
| `ping [ip]` | Network test |
| `set <key> <value>` | Same keys as the web API |
| `raw R G B W [led]` | Drive the LED channels directly |
| `pattern` | One LED each in red, green, blue and white |
| `pin` | LED data-line idle level |

## Project layout

```
firmware/              PlatformIO project (Arduino core 2.0.17, pinned)
  src/main.cpp         modes, Wi-Fi, web server, Bluetooth, serial commands
  src/led_driver.h     WS2812 driver over SPI + DMA
  src/web_page.h       the phone web app (source)
  tools/gen_web.py     gzips the web app before every build -> web_page_gz.h
  tools/gen_icons.py   draws the app icon/favicon before every build -> web_icons.h
hardware/cad/          3D-printable base (STL), STEP and SolidCraft sources
```

## Troubleshooting

- **The page is slow or times out.** The signal is weak where the lamp sits. Check the dBm
  figure under settings: better than -70 is good, worse than -78 is unreliable. Keep
  long-range mode on and Bluetooth off, and keep the antenna clear of the LED panel.
- **The colours are wrong, or a dim glow stays on when the lamp is off.** Run `pattern` over
  USB serial. The firmware is set up for WS2812 panels in GRB order (`cfg.ledt` in `main.cpp`).
- **The lamp won't join a network.** Network names are case-sensitive, and the C3 only sees
  2.4 GHz networks. Check `d` over serial for the disconnect reason.
