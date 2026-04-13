import os
import subprocess

Import("env")


def option_enabled(name, default=False):
    value = env.GetProjectOption(name, "true" if default else "false")
    return str(value).lower() in ("1", "true", "yes", "on")


def skip_adafruit_hid(node):
    return None


def skip_adafruit_usbd_device(node):
    return None


def skip_adafruit_usbd_cdc(node):
    return None


def skip_adafruit_tinyusb_api(node):
    return None


def register_skip(pattern):
    env.AddBuildMiddleware(skip_adafruit_hid, pattern)


def build_tinyusb_core(enable_host=False):
    framework_dir = env.PioPlatform().get_package_dir("framework-arduinopico")
    tinyusb_src = os.path.join("$PROJECT_DIR", "third_party", "tinyusb_gp2040", "src")

    build_specs = [
        ("class_cdc", os.path.join(tinyusb_src, "class", "cdc"), "-<*> +<cdc_device.c>"),
        ("class_hid", os.path.join(tinyusb_src, "class", "hid"), "-<*> +<hid_device.c>"),
        ("common", os.path.join(tinyusb_src, "common"), "-<*> +<tusb_fifo.c>"),
        ("device", os.path.join(tinyusb_src, "device"), "-<*> +<usbd.c> +<usbd_control.c>"),
        (
            "portable_rp2040",
            os.path.join(tinyusb_src, "portable", "raspberrypi", "rp2040"),
            "-<*> +<dcd_rp2040.c> +<rp2040_usb.c>",
        ),
        ("root", tinyusb_src, "-<*> +<tusb.c>"),
    ]

    if enable_host:
        build_specs.extend(
            [
                ("class_cdc_host", os.path.join(tinyusb_src, "class", "cdc"), "-<*> +<cdc_host.c>"),
                ("class_hid_host", os.path.join(tinyusb_src, "class", "hid"), "-<*> +<hid_host.c>"),
                ("host", os.path.join(tinyusb_src, "host"), "-<*> +<usbh.c> +<hub.c>"),
                (
                    "portable_pio_usb_host",
                    os.path.join(tinyusb_src, "portable", "raspberrypi", "pio_usb"),
                    "-<*> +<hcd_pio_usb.c>",
                ),
            ]
        )

    for name, src_dir, src_filter in build_specs:
        env.BuildSources(os.path.join("$BUILD_DIR", "TinyUSBCore", name), src_dir, src_filter)


def before_build():
    subprocess.run(["git", "config", "--global", "core.longpaths", "true"])

    c_hash = subprocess.run(["git", "rev-parse", "--short", "HEAD"], capture_output=True, text=True).stdout.strip()
    proc = subprocess.run(["git", "status", "--porcelain"], capture_output=True, text=True)

    dirty = False
    if proc.stdout == None or len(proc.stdout) > 3:
        dirty = True

    if dirty:
        c_hash += "-DIRTY"
    
    version_name = "\\\"" + c_hash + "\\\""

    env.Append(CPPDEFINES=[
        ("FIRMWARE_VERSION", version_name)
    ])

    enable_usb_host = option_enabled("custom_enable_usb_host")
    tinyusb_include = os.path.join("$PROJECT_DIR", "third_party", "tinyusb_gp2040", "src")

    env.Prepend(CPPPATH=[tinyusb_include])

    if enable_usb_host:
        usb_host_dp_pin = int(env.GetProjectOption("custom_usb_host_dp_pin", "0"))
        usb_host_pinout_option = str(env.GetProjectOption("custom_usb_host_pinout", "dpdm")).lower()
        usb_host_pinout = "PIO_USB_PINOUT_DMDP" if usb_host_pinout_option == "dmdp" else "PIO_USB_PINOUT_DPDM"

        env.Append(
            CPPDEFINES=[
                ("FW_ENABLE_USB_HOST", 1),
                ("FW_USB_HOST_DP_PIN", usb_host_dp_pin),
                ("FW_USB_HOST_PINOUT", usb_host_pinout),
            ],
        )
        env.Prepend(
            CPPPATH=[
                os.path.join("$PROJECT_DIR", "third_party", "pico_pio_usb", "src"),
            ]
        )

    build_tinyusb_core(enable_usb_host)

    if enable_usb_host:
        env.BuildSources(
            os.path.join("$BUILD_DIR", "TinyUSBHostPort"),
            os.path.join("$PROJECT_DIR", "third_party", "pico_pio_usb", "src"),
            "-<*> +<interval_override.c> +<pio_usb.c> +<pio_usb_host.c> +<usb_crc.c>",
        )

    skip_patterns = [
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/hid/Adafruit_USBD_HID.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/Adafruit_TinyUSB_API.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/Adafruit_USBD_CDC.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/Adafruit_USBD_Device.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/Adafruit_USBH_Host.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/cdc/Adafruit_USBH_CDC.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/midi/Adafruit_USBD_MIDI.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/msc/Adafruit_USBD_MSC.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/msc/Adafruit_USBH_MSC.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/webusb/Adafruit_USBD_WebUSB.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/esp32/Adafruit_TinyUSB_esp32.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/nrf/Adafruit_TinyUSB_nrf.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/rp2040/Adafruit_TinyUSB_rp2040.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/samd/Adafruit_TinyUSB_samd.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/audio/audio_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/bth/bth_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/cdc/cdc_host.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/cdc/cdc_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/dfu/dfu_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/dfu/dfu_rt_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/hid/hid_host.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/hid/hid_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/midi/midi_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/msc/msc_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/msc/msc_host.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/net/ecm_rndis_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/net/ncm_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/usbtmc/usbtmc_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/vendor/vendor_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/video/video_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/common/tusb_fifo.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/device/usbd.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/device/usbd_control.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/host/hub.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/host/usbh.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/portable/analog/max3421/hcd_max3421.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/portable/microchip/samd/dcd_samd.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/portable/nordic/nrf5x/dcd_nrf5x.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/portable/raspberrypi/pio_usb/dcd_pio_usb.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/portable/raspberrypi/rp2040/dcd_rp2040.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/portable/raspberrypi/rp2040/hcd_rp2040.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/portable/raspberrypi/rp2040/rp2040_usb.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/tusb.c",
    ]

    for pattern in skip_patterns:
        env.AddBuildMiddleware(skip_adafruit_hid, pattern)

before_build()
