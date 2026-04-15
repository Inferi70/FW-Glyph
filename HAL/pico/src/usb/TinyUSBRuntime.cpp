#include "usb/TinyUSBRuntime.hpp"

extern "C" void TinyUSBDevice_SetDeviceClassCodes(
    uint8_t device_class,
    uint8_t device_subclass,
    uint8_t device_protocol
);

namespace usb_runtime {
    void setDeviceId(uint16_t vid, uint16_t pid) {
        TinyUSBDevice.setID(vid, pid);
    }

    void setDeviceVersion(uint16_t bcd) {
        TinyUSBDevice.setVersion(bcd);
    }

    void setDeviceRelease(uint16_t bcd) {
        TinyUSBDevice.setDeviceVersion(bcd);
    }

    void setDeviceClassCodes(uint8_t device_class, uint8_t device_subclass, uint8_t device_protocol) {
        TinyUSBDevice_SetDeviceClassCodes(device_class, device_subclass, device_protocol);
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

}
