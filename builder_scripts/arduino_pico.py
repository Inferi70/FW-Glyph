import subprocess

Import("env")


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

    skip_patterns = [
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/hid/Adafruit_USBD_HID.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/Adafruit_USBD_Device.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/Adafruit_USBD_CDC.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/Adafruit_TinyUSB_API.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/Adafruit_USBH_Host.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/cdc/Adafruit_USBH_CDC.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/midi/Adafruit_USBD_MIDI.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/msc/Adafruit_USBD_MSC.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/msc/Adafruit_USBH_MSC.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/webusb/Adafruit_USBD_WebUSB.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/esp32/Adafruit_TinyUSB_esp32.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/nrf/Adafruit_TinyUSB_nrf.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/samd/Adafruit_TinyUSB_samd.cpp",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/audio/audio_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/bth/bth_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/cdc/cdc_host.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/dfu/dfu_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/dfu/dfu_rt_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/hid/hid_host.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/midi/midi_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/msc/msc_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/msc/msc_host.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/net/ecm_rndis_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/net/ncm_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/usbtmc/usbtmc_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/vendor/vendor_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/class/video/video_device.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/host/hub.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/host/usbh.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/portable/analog/max3421/hcd_max3421.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/portable/microchip/samd/dcd_samd.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/portable/nordic/nrf5x/dcd_nrf5x.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/portable/raspberrypi/pio_usb/dcd_pio_usb.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c",
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/portable/raspberrypi/rp2040/hcd_rp2040.c",
    ]

    for pattern in skip_patterns:
        env.AddBuildMiddleware(skip_adafruit_hid, pattern)

before_build()
