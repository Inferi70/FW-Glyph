#ifndef _USB_TINYUSB_HID_HPP
#define _USB_TINYUSB_HID_HPP

#include "arduino/Adafruit_USBD_Device.h"

class TinyUSBHID : public Adafruit_USBD_Interface {
  public:
    typedef uint16_t (*get_report_callback_t)(
        uint8_t report_id,
        hid_report_type_t report_type,
        uint8_t *buffer,
        uint16_t reqlen
    );
    typedef void (*set_report_callback_t)(
        uint8_t report_id,
        hid_report_type_t report_type,
        uint8_t const *buffer,
        uint16_t bufsize
    );

    enum { INVALID_INSTANCE = 0xffu };

    TinyUSBHID(
        uint8_t const *desc_report = nullptr,
        uint16_t len = 0,
        uint8_t protocol = HID_ITF_PROTOCOL_NONE,
        uint8_t interval_ms = 4,
        bool has_out_endpoint = false
    );

    void setPollInterval(uint8_t interval_ms);
    void setBootProtocol(uint8_t protocol);

    void enableOutEndpoint(bool enable);
    bool isOutEndpointEnabled(void);

    void setReportDescriptor(uint8_t const *desc_report, uint16_t len);
    void setReportCallback(get_report_callback_t get_report, set_report_callback_t set_report);

    bool begin(void);
    bool isValid(void) { return _instance != INVALID_INSTANCE; }

    bool ready(void);
    bool sendReport(uint8_t report_id, void const *report, uint8_t len);

    uint16_t getInterfaceDescriptor(uint8_t itfnum, uint8_t *buf, uint16_t bufsize) override;
    uint16_t makeItfDesc(uint8_t itfnum, uint8_t *buf, uint16_t bufsize, uint8_t ep_in, uint8_t ep_out);

  private:
    static uint8_t _instance_count;

    uint8_t _instance;
    uint8_t _interval_ms;
    uint8_t _protocol;
    bool _out_endpoint;

    uint16_t _desc_report_len;
    uint8_t const *_desc_report;

    get_report_callback_t _get_report_cb;
    set_report_callback_t _set_report_cb;

    friend uint16_t tud_hid_get_report_cb(
        uint8_t itf,
        uint8_t report_id,
        hid_report_type_t report_type,
        uint8_t *buffer,
        uint16_t reqlen
    );
    friend void tud_hid_set_report_cb(
        uint8_t itf,
        uint8_t report_id,
        hid_report_type_t report_type,
        uint8_t const *buffer,
        uint16_t bufsize
    );
    friend uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf);
};

#endif
