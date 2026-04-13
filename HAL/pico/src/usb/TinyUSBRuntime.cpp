#include "usb/TinyUSBRuntime.hpp"

namespace usb_runtime {
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

}
