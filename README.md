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
- Adafruit TinyUSB core C build path:
  replaced with a repo-vendored TinyUSB source tree aligned with GP2040-CE.

### What Was Wrapped First

Before the lower-level rewrite, `TUCompositeHID` was first reduced to a narrow wrapper surface:

- `TUCompositeHID::begin`
- `TUCompositeHID::ready`
- `TUCompositeHID::sendReport`

That let the HID transport be swapped underneath without changing every caller at once.

### Current USB Build Layout

For `pio run -e glyph_mk6`:

- HID, XInput, CDC/API glue, RP2040 port glue, and device runtime are repo-owned.
- TinyUSB core C sources are built from:
  `third_party/tinyusb_gp2040/src`
- The old Adafruit TinyUSB wrapper and port sources are skipped in the build script.

For `pio run -e glyph_mk6_usb_host`:

- Repo-owned host manager, host listeners, XInput host class glue, and RP2040 host port glue are used.
- Vendored `pico_pio_usb` provides the RP2040 host transport.
- TinyUSB host-side C sources are built from the same vendored GP2040-aligned tree:
  `third_party/tinyusb_gp2040/src`

### USB Host Direction

The branch also now has the first GP2040-CE-style host scaffolding:

- `InputSourceManager` exists as the seam between raw inputs and communication backends.
- `TinyUSBHostManager` owns TinyUSB host startup, task pumping, and listener fan-out.
- `glyph_mk6_usb_host` is a dedicated host-enabled build using vendored `pico_pio_usb`.
- Host-enabled TinyUSB now builds HID host and XInput host support.
- The XInput host class driver is repo-owned and closely ported from GP2040-CE's `xinput_host`.

Current status:

- Host builds pass.
- Device builds still pass.
- Repo-owned host input plumbing now includes a first host-backed input source.
- XInput host reports are translated into FW-Glyph raw input slots through `USBHostGamepadInput`.
- DS4-class HID reports are now also translated into FW-Glyph raw input slots through the same host input bridge.
- DualSense HID reports are now also translated into FW-Glyph raw input slots through the same host input bridge.
- Switch Pro HID reports are now also translated into FW-Glyph raw input slots through the same host input bridge.
- The current host bridge still uses fixed default raw-button mappings rather than a normalized per-device configuration layer.
- Generic HID fallback parsing is still the next expansion area.
- Repo-owned host auth detection and transport now exist through `USBHostAuthListener`.
- The current auth listener can detect PS4-style HID auth devices, P5 auth devices, and XInput 360-class host devices.
- PS4/P5 feature-report auth requests are now routed through TinyUSB host HID get/set report helpers in the host-enabled build.
- `USBHostAuthPassthrough` now provides a repo-owned bridge for console-facing auth drivers:
  it queues PS4/P5 feature-report requests, drives the host dongle, and buffers responses back.
- `PlayStationAuthPassthrough` now provides a repo-owned PS4/P5 auth session layer on top of that bridge:
  it owns the console-side PS4/P5 auth report flow and sequences the host dongle requests.
- `COMMS_BACKEND_PASSTHROUGH_PS4` and `COMMS_BACKEND_PASSTHROUGH_PS5` now exist as PS4-family HID device backends.
- Those backends use repo-owned HID get/set report callbacks and consume `PlayStationAuthPassthrough`.
- End-to-end passthrough is now wired in firmware, but it still needs real hardware validation on PS4/PS5 hosts.

### XInput Notes

- The final XInput fix was not a descriptor tweak. The real issue was callback timing.
- `framework-arduinopico` calls `TinyUSB_Device_Init(0)` during startup.
- XInput app-driver/BOS/vendor callbacks therefore need to exist at USB init time.
- The working final state keeps these callbacks as direct TinyUSB globals in `TinyUSBXInput.cpp`:
  - `usbd_app_driver_get_cb`
  - `tud_descriptor_bos_cb`
  - `tud_vendor_control_xfer_cb`
- The earlier runtime registration approach via `TinyUSBRuntime` was too late for TinyUSB startup and caused Linux `SET_CONFIGURATION` failure (`can't set config #1, error -32`).
- Current XInput endpoint intervals are:
  - IN `0x81`: `bInterval = 1`
  - OUT `0x01`: `bInterval = 1`

### What Still Appears In PlatformIO

PlatformIO may still print `Adafruit TinyUSB Library` in the dependency graph.

That is a discovery/LDF artifact from the framework package layout, not an indication that Adafruit wrapper or core code is still being compiled. The actual TinyUSB objects now come from `.pio/build/.../TinyUSBCore/...` and are built from the repo-vendored TinyUSB tree.

### Build Size Impact

Observed during this rewrite series on `glyph_mk6`:

- Earlier passing state in this branch:
  Flash `372944` bytes, RAM `51264` bytes.
- Current direct-TinyUSB state:
  Flash `368608` bytes, RAM `50296` bytes.

Net change from the observed branch baseline:

- Flash reduced by `4336` bytes.
- RAM reduced by `968` bytes.

### Major Commits In This Migration

- `2cdc594` `TUCompositeHID wrapper rewrite`
- `9764e28` `Vendor XInput and add USB runtime`
- `7e51444` `Own TinyUSB HID callbacks`
- `3a08382` `Own TinyUSB device runtime`
- `fd3ecbd` `Own TinyUSB device headers`
- `43312d6` `Own TinyUSB CDC layer`
- `8249b17` `Trim bundled TinyUSB sources`
- `e41ccf6` `Own TinyUSB RP2040 port glue`
- `71656da` `Document TinyUSB rewrite status`
- `2c396d3` `Finalize working repo-owned XInput on direct TinyUSB`

---
This project is licensed under the GNU GPL Version 3 - see the [LICENSE](LICENSE) file for details
