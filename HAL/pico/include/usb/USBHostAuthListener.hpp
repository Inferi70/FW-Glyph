#ifndef _USB_USB_HOST_AUTH_LISTENER_HPP
#define _USB_USB_HOST_AUTH_LISTENER_HPP

#include "tusb.h"
#include "usb/TinyUSBHostListener.hpp"

enum class USBHostAuthDeviceType {
    NONE = 0,
    PS4_HID,
    P5_HID,
    XINPUT_360,
};

class USBHostAuthListener : public TinyUSBHostListener {
  public:
    void setup() override;
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
    void hidGetReportComplete(
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

    bool requestXInput360Serial();
    bool sendXInput360InitAuth(const uint8_t *payload, uint16_t len);
    bool sendXInput360VerifyAuth(const uint8_t *payload, uint16_t len);
    bool requestXInput360ChallengeResponse(uint16_t len);
    bool requestXInput360State();
    bool sendXInput360KeepAlive();
    void xinputVendorComplete(uint8_t request, xfer_result_t result, uint32_t actual_len);

  private:
    bool hostGetReport(uint8_t report_id, void *report, uint16_t len);
    bool hostSetReport(uint8_t report_id, void *report, uint16_t len);
    bool xinputVendorTransfer(
        tusb_dir_t dir,
        uint8_t request,
        uint16_t value,
        uint16_t len,
        const uint8_t *payload
    );
    void clear();

    USBHostAuthDeviceType _device_type = USBHostAuthDeviceType::NONE;
    uint8_t _dev_addr = 0xFF;
    uint8_t _instance = 0xFF;
    uint8_t _interface_number = 0xFF;
    bool _busy = false;
    uint8_t _last_report_id = 0;
    uint16_t _last_len = 0;
    bool _awaiting_cb = false;
    uint8_t _last_buffer[64] = {};
    tusb_control_request_t _xinput_control_request = {};
};

#endif
