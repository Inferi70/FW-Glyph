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

    env.AddBuildMiddleware(
        skip_adafruit_hid,
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/hid/Adafruit_USBD_HID.cpp"
    )
    env.AddBuildMiddleware(
        skip_adafruit_usbd_device,
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/Adafruit_USBD_Device.cpp"
    )
    env.AddBuildMiddleware(
        skip_adafruit_usbd_cdc,
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/Adafruit_USBD_CDC.cpp"
    )
    env.AddBuildMiddleware(
        skip_adafruit_tinyusb_api,
        "*framework-arduinopico/libraries/Adafruit_TinyUSB_Arduino/src/arduino/Adafruit_TinyUSB_API.cpp"
    )

before_build()
