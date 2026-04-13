#ifndef _USB_USB_HOST_AUTH_LISTENER_HPP
#define _USB_USB_HOST_AUTH_LISTENER_HPP

#include "usb/TinyUSBHostListener.hpp"
#include "host/usbh.h"

enum class USBHostAuthDeviceType {
    NONE = 0,
    PS4_HID,
    P5_HID,
    XINPUT_360,
};

class USBHostAuthListener : public TinyUSBHostListener {
  public:
    bool available() const;
    USBHostAuthDeviceType deviceType() const;
    bool busy() const;
    uint8_t deviceAddress() const;
    uint8_t instance() const;
    const uint8_t *lastBuffer() const;
    uint16_t lastLength() const;
    uint8_t lastReportId() const;

    void unmount(uint8_t dev_addr) override;
    void hidMount(uint8_t dev_addr, uint8_t instance, const uint8_t *desc_report, uint16_t desc_len) override;
    void xinputMount(uint8_t dev_addr, uint8_t instance, uint8_t type, uint8_t subtype) override;
    void xinputUnmount(uint8_t dev_addr, uint8_t instance) override;
    void hidSetReportComplete(
        uint8_t dev_addr,
        uint8_t instance,
        uint8_t report_id,
        uint8_t report_type,
        uint16_t len
    ) override;

    bool requestPS4Definition();
    bool requestPS4ResetAuth();
    bool requestPS4SigningState();
    bool requestPS4SignatureNonce(uint8_t nonce_id, uint8_t nonce_chunk);
    bool sendPS4AuthPayload(const uint8_t *payload, uint16_t len);

    bool requestP5SignatureNonce(uint16_t len = 64);
    bool requestP5SigningState(uint16_t len = 16);
    bool sendP5AuthPayload(const uint8_t *payload, uint16_t len);

  private:
    static void hidGetReportCompleteCallback(tuh_xfer_t *xfer);
    bool hidGetReport(uint8_t report_id, void *report, uint16_t len);
    bool hidSetReport(uint8_t report_id, void *report, uint16_t len);
    void onHidGetReportComplete(tuh_xfer_t *xfer);
    void clear();

    USBHostAuthDeviceType _device_type = USBHostAuthDeviceType::NONE;
    uint8_t _dev_addr = 0xFF;
    uint8_t _instance = 0xFF;
    uint8_t _interface_number = 0xFF;
    bool _busy = false;
    uint8_t _last_report_id = 0;
    uint16_t _last_len = 0;
    uint8_t _last_buffer[64] = {};
    tusb_control_request_t _last_request = {};
};

#endif
