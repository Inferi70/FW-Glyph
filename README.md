# Glyph Firmware

*Glyph's firmware is based on HayBox, please consider supporting/sponsoring it [here](https://github.com/JonnyHaystack/HayBox)*

## Glyph Links
For remapping your Glyph: [Configurator](https://limitlabs.com/pages/glyph-configurator)

Glyph resources: [Resources](https://limitlabs.com/pages/glyph-resources)

User manual: [Manual](https://cdn.shopify.com/s/files/1/0926/5597/6818/files/Glyph_Manual_v1.0.pdf?v=1775239160)

Order today through [Satisfye.com](https://www.satisfye.com/collections/glyph), our manufacturing and logistics partner for Glyph.

## TinyUSB Rewrite Status

The `tiny-usb-rewrite` branch removes the project's dependence on Adafruit's TinyUSB wrapper layer for the RP2040 build and moves the firmware onto repo-owned glue plus vendored TinyUSB sources aligned with GP2040-CE.

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

## New Features In This Branch

### USB Host Build

Host support is enabled through the dedicated `glyph_mk6_usb_host` environment.

- Default device build:
  `pio run -e glyph_mk6`
- Host-enabled build:
  `pio run -e glyph_mk6_usb_host`

The host-enabled build uses:

- vendored TinyUSB from `third_party/tinyusb_gp2040/src`
- vendored `pico_pio_usb` from `third_party/pico_pio_usb`
- repo-owned host glue in `HAL/pico/src/usb/`

Relevant config knobs:

- `platformio.ini`
  - `custom_enable_usb_host = false` in the normal device build
- `config/glyph/env.ini`
  - `glyph_mk6_usb_host` sets `custom_enable_usb_host = true`

### Hosted Controller Input

The host build can now ingest controllers connected to the controller's USB host port and feed them into FW-Glyph's normal input pipeline.

Supported hosted controller families:

- XInput
- DualShock 4
- DualSense
- Switch Pro
- Generic HID gamepads and joysticks that follow the common gamepad report layout

How hosted input is handled:

- `TinyUSBHostManager` starts TinyUSB host mode and fans out callbacks to listeners.
- `USBHostGamepadInput` listens for HID/XInput host reports.
- Reports normalize into one shared hosted gamepad state.
- That shared state is translated into normal FW-Glyph `InputState`.
- FW-Glyph's existing profile remaps, SOCD handling, and game-mode logic then run as usual.

Hosted default mappings:

- hosted `L3 -> LT1`
- hosted `R3 -> LT2`
- those line up with FW-Glyph's common `modX/modY` usage

If a developer wants different hosted defaults, change the centralized mapping table in:

- [USBHostGamepadInput.cpp](/home/inferi/code/FW-Glyph/HAL/pico/src/input/USBHostGamepadInput.cpp)

This is not yet a separate user-facing host remap UI. Normal FW-Glyph profile remaps still apply after hosted input is translated.

### PlayStation Passthrough

This branch now has device-side PlayStation passthrough backends plus host-side auth support.

Backends:

- `COMMS_BACKEND_PASSTHROUGH_PS4`
- `COMMS_BACKEND_PASSTHROUGH_PS5`

Key pieces:

- `PlayStationBackend`
- `USBHostAuthListener`
- `USBHostAuthPassthrough`
- `PlayStationAuthPassthrough`

How it works:

1. FW-Glyph presents itself as a PS4 or PS5-family HID device.
2. Console auth/report requests are handled by `PlayStationBackend`.
3. `PlayStationBackend` forwards auth traffic into `PlayStationAuthPassthrough`.
4. `PlayStationAuthPassthrough` drives the attached host auth device through `USBHostAuthPassthrough` and `USBHostAuthListener`.

What is ready:

- firmware layers are present end-to-end
- PS4/PS5 modes are selectable from the on-device menu
- both normal and host builds compile

What still needs hardware validation:

- real PS4 console behavior
- real PS5 console behavior
- auth device compatibility edge cases

### Xbox Auth And XInput

The XInput device side is now closer to GP2040-CE.

- `XInputBackend` uses a GP2040-style 4-interface Xbox 360 layout
- Xbox auth is handled on the XInput path, not a separate backend
- `XboxAuthPassthrough` routes vendor auth traffic to an attached Xbox auth-capable host device

What is ready:

- PC XInput behavior is working again
- Xbox auth transport is wired in firmware

What still needs hardware validation:

- real Xbox console auth behavior
- whether the current 4-interface shape needs more fidelity tweaks for specific hosts

### On-Device Mode Selection

The menu/UI now exposes the new PlayStation passthrough backends in addition to the original FW-Glyph USB modes.

Available USB-facing families in this branch:

- XInput
- DInput
- Nintendo Switch
- PS4 passthrough
- PS5 passthrough

Selection does not require an auth dongle.

- You can select PS4 or PS5 mode from the device menu with nothing attached to the host port.
- Auth devices matter for real console acceptance, not for choosing the mode.

### GP2040-Style Structure

This branch intentionally moved several pieces closer to GP2040-CE's structure:

- vendored TinyUSB source tree
- vendored `pico_pio_usb`
- host manager/listener model
- repo-owned XInput host glue
- repo-owned auth listeners and passthrough layers
- GP2040-style 4-interface Xbox 360 descriptor shape

The project still keeps repo-owned integration code where FW-Glyph behavior diverges from GP2040-CE:

- device backends
- hosted-input-to-FW-Glyph mapping
- auth session policy
- menu/config integration

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
