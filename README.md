# Glyph Firmware

*Glyph's firmware is based on HayBox, please consider supporting/sponsoring it [here](https://github.com/JonnyHaystack/HayBox)*

## Glyph Links
For remapping your Glyph: [Configurator](https://limitlabs.com/pages/glyph-configurator)

Glyph resources: [Resources](https://limitlabs.com/pages/glyph-resources)

User manual: [Manual](https://cdn.shopify.com/s/files/1/0926/5597/6818/files/Glyph_Manual_v1.0.pdf?v=1775239160)

Order today through [Satisfye.com](https://www.satisfye.com/collections/glyph), our manufacturing and logistics partner for Glyph.

## TinyUSB Rewrite Status

The `tiny-usb-rewrite` branch removes the project's dependence on Adafruit's TinyUSB wrapper layer for the RP2040 build and moves the firmware onto repo-owned glue plus direct TinyUSB core sources.

### What Was Replaced

- External `Adafruit_TinyUSB_XInput` package:
  replaced with repo-owned `TinyUSBXInput`.
- Adafruit runtime callback ownership:
  replaced with repo-owned `TinyUSBRuntime`.
- Adafruit HID wrapper ownership:
  replaced with repo-owned `TinyUSBHID`.
- Adafruit USB device runtime implementation:
  replaced with repo-owned `TinyUSBDevice.cpp`.
- Adafruit USB device/interface headers:
  shadowed with repo-owned headers under `HAL/pico/include/arduino/`.
- Adafruit CDC/API layer:
  replaced with repo-owned `TinyUSBCDC.cpp`, `TinyUSBAPI.cpp`, and matching headers.
- Adafruit RP2040 TinyUSB port glue:
  replaced with repo-owned `TinyUSBPortRP2040.cpp` and a repo-owned `Adafruit_TinyUSB.h` umbrella header.

### What Was Wrapped First

Before the lower-level rewrite, `TUCompositeHID` was first reduced to a narrow wrapper surface:

- `TUCompositeHID::begin`
- `TUCompositeHID::ready`
- `TUCompositeHID::sendReport`

That let the HID transport be swapped underneath without changing every caller at once.

### Current USB Build Layout

For `pio run -e glyph_mk6`:

- HID, XInput, USB runtime routing, CDC/API glue, RP2040 port glue, and device runtime are repo-owned.
- TinyUSB core C sources are built directly from:
  `framework-arduinopico/pico-sdk/lib/tinyusb/src`
- The old Adafruit TinyUSB wrapper and port sources are skipped in the build script.

### What Still Appears In PlatformIO

PlatformIO may still print `Adafruit TinyUSB Library` in the dependency graph.

That is a discovery/LDF artifact from the framework package layout, not an indication that Adafruit wrapper code is still being compiled. The actual TinyUSB objects now come from `.pio/build/glyph_mk6/TinyUSBCore/...`.

### Build Size Impact

Observed during this rewrite series on `glyph_mk6`:

- Earlier passing state in this branch:
  Flash `372944` bytes, RAM `51264` bytes.
- Current direct-TinyUSB state:
  Flash `368624` bytes, RAM `50308` bytes.

Net change from the observed branch baseline:

- Flash reduced by `4320` bytes.
- RAM reduced by `956` bytes.

### Major Commits In This Migration

- `2cdc594` `TUCompositeHID wrapper rewrite`
- `9764e28` `Vendor XInput and add USB runtime`
- `7e51444` `Own TinyUSB HID callbacks`
- `3a08382` `Own TinyUSB device runtime`
- `fd3ecbd` `Own TinyUSB device headers`
- `43312d6` `Own TinyUSB CDC layer`
- `8249b17` `Trim bundled TinyUSB sources`
- `e41ccf6` `Own TinyUSB RP2040 port glue`
- `d6aee70` `Build TinyUSB core directly`

---
This project is licensed under the GNU GPL Version 3 - see the [LICENSE](LICENSE) file for details
