#ifndef _USB_TINYUSB_HOST_LISTENER_HPP
#define _USB_TINYUSB_HOST_LISTENER_HPP

#include <stdint.h>

class TinyUSBHostListener {
  public:
    virtual ~TinyUSBHostListener() {}

    virtual void mount(uint8_t dev_addr, uint16_t vid, uint16_t pid) {}
    virtual void unmount(uint8_t dev_addr) {}

    virtual void hidMount(
        uint8_t dev_addr,
        uint8_t instance,
        const uint8_t *desc_report,
        uint16_t desc_len
    ) {}

    virtual void hidUnmount(uint8_t dev_addr, uint8_t instance) {}

    virtual void hidReportReceived(
        uint8_t dev_addr,
        uint8_t instance,
        const uint8_t *report,
        uint16_t len
    ) {}
};

#endif
