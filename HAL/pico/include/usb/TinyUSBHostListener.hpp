#ifndef _USB_TINYUSB_HOST_LISTENER_HPP
#define _USB_TINYUSB_HOST_LISTENER_HPP

#include <stdint.h>

class TinyUSBHostListener {
  public:
    virtual ~TinyUSBHostListener() {}

    virtual void setup() {}

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

    virtual void hidSetReportComplete(
        uint8_t dev_addr,
        uint8_t instance,
        uint8_t report_id,
        uint8_t report_type,
        uint16_t len
    ) {}

    virtual void hidGetReportComplete(
        uint8_t dev_addr,
        uint8_t instance,
        uint8_t report_id,
        uint8_t report_type,
        uint16_t len
    ) {}

    virtual void xinputMount(
        uint8_t dev_addr,
        uint8_t instance,
        uint8_t type,
        uint8_t subtype
    ) {}

    virtual void xinputUnmount(uint8_t dev_addr, uint8_t instance) {}

    virtual void xinputReportReceived(
        uint8_t dev_addr,
        uint8_t instance,
        const uint8_t *report,
        uint16_t len
    ) {}

    virtual void xinputReportSent(
        uint8_t dev_addr,
        uint8_t instance,
        const uint8_t *report,
        uint16_t len
    ) {}
};

#endif
