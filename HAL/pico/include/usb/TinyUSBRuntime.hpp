#ifndef _USB_TINYUSB_RUNTIME_HPP
#define _USB_TINYUSB_RUNTIME_HPP

#include "arduino/Adafruit_USBD_Device.h"

namespace usb_runtime {

    void setDeviceId(uint16_t vid, uint16_t pid);
    void setDeviceVersion(uint16_t bcd);
    void setManufacturer(const char *manufacturer);
    void setProduct(const char *product);
    void setSerial(const char *serial);
}

#endif
