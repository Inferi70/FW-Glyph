#ifndef _USB_TINYUSB_RUNTIME_HPP
#define _USB_TINYUSB_RUNTIME_HPP

#include "arduino/Adafruit_USBD_Device.h"
#include "device/usbd_pvt.h"

namespace usb_runtime {

    using app_driver_getter_t = const usbd_class_driver_t *(*)(uint8_t *driver_count);
    using bos_descriptor_getter_t = const uint8_t *(*)();
    using vendor_control_xfer_cb_t =
        bool (*)(uint8_t rhport, uint8_t stage, const tusb_control_request_t *request);

    void setDeviceId(uint16_t vid, uint16_t pid);
    void setDeviceVersion(uint16_t bcd);
    void setManufacturer(const char *manufacturer);
    void setProduct(const char *product);
    void setSerial(const char *serial);

    void registerAppDriverGetter(app_driver_getter_t getter);
    void registerBosDescriptorGetter(bos_descriptor_getter_t getter);
    void registerVendorControlXfer(vendor_control_xfer_cb_t callback);
}

#endif
