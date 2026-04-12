#ifndef ADAFRUIT_TINYUSB_H_
#define ADAFRUIT_TINYUSB_H_

#if !defined(USE_TINYUSB) &&                                                   \
    (defined(ARDUINO_ARCH_SAMD) ||                                             \
     (defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_ARCH_MBED)))
#error TinyUSB is not selected, please select it in "Tools->Menu->USB Stack"
#endif

#include "tusb_option.h"

#if CFG_TUD_ENABLED

#include "arduino/Adafruit_USBD_Device.h"

#if CFG_TUD_CDC
#include "arduino/Adafruit_USBD_CDC.h"
#endif

#if CFG_TUD_HID
#include "arduino/hid/Adafruit_USBD_HID.h"
#endif

#if CFG_TUD_MIDI
#include "arduino/midi/Adafruit_USBD_MIDI.h"
#endif

#if CFG_TUD_MSC
#include "arduino/msc/Adafruit_USBD_MSC.h"
#endif

#if CFG_TUD_VENDOR
#include "arduino/webusb/Adafruit_USBD_WebUSB.h"
#endif

void TinyUSB_Device_Init(uint8_t rhport);

#endif

#if CFG_TUH_ENABLED

#include "arduino/Adafruit_USBH_Host.h"

#if CFG_TUH_CDC
#include "arduino/cdc/Adafruit_USBH_CDC.h"
#endif

#if CFG_TUH_MSC
#include "arduino/msc/Adafruit_USBH_MSC.h"
#endif

#endif

#endif
