#include "usb/TinyUSBRuntime.hpp"

namespace usb_runtime {
    static app_driver_getter_t app_driver_getter = nullptr;
    static bos_descriptor_getter_t bos_descriptor_getter = nullptr;
    static vendor_control_xfer_cb_t vendor_control_xfer = nullptr;

    void setDeviceId(uint16_t vid, uint16_t pid) {
        TinyUSBDevice.setID(vid, pid);
    }

    void setDeviceVersion(uint16_t bcd) {
        TinyUSBDevice.setVersion(bcd);
    }

    void setManufacturer(const char *manufacturer) {
        TinyUSBDevice.setManufacturerDescriptor(manufacturer);
    }

    void setProduct(const char *product) {
        TinyUSBDevice.setProductDescriptor(product);
    }

    void setSerial(const char *serial) {
        TinyUSBDevice.setSerialDescriptor(serial);
    }

    void registerAppDriverGetter(app_driver_getter_t getter) {
        app_driver_getter = getter;
    }

    void registerBosDescriptorGetter(bos_descriptor_getter_t getter) {
        bos_descriptor_getter = getter;
    }

    void registerVendorControlXfer(vendor_control_xfer_cb_t callback) {
        vendor_control_xfer = callback;
    }
}

extern "C" const usbd_class_driver_t *usbd_app_driver_get_cb(uint8_t *driver_count) {
    if (usb_runtime::app_driver_getter == nullptr) {
        *driver_count = 0;
        return nullptr;
    }
    return usb_runtime::app_driver_getter(driver_count);
}

extern "C" const uint8_t *tud_descriptor_bos_cb(void) {
    if (usb_runtime::bos_descriptor_getter == nullptr) {
        return nullptr;
    }
    return usb_runtime::bos_descriptor_getter();
}

extern "C" bool tud_vendor_control_xfer_cb(
    uint8_t rhport,
    uint8_t stage,
    const tusb_control_request_t *request
) {
    if (usb_runtime::vendor_control_xfer == nullptr) {
        return false;
    }
    return usb_runtime::vendor_control_xfer(rhport, stage, request);
}
