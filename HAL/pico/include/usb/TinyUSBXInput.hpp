#ifndef _USB_TINYUSB_XINPUT_HPP
#define _USB_TINYUSB_XINPUT_HPP

#include "arduino/Adafruit_USBD_Device.h"
#include "device/usbd_pvt.h"

#define XINPUT_SUBCLASS_DEFAULT 0x5D
#define XINPUT_PROTOCOL_DEFAULT 1

#define XINPUT_EPOUT 0x01
#define XINPUT_EPIN 0x81
#define XINPUT_EPSIZE 32

#define TUD_XINPUT_DESC_LEN (9 + 16 + 7 + 7)

#define TUD_XINPUT_DESCRIPTOR(_itfnum, _stridx, _epout, _epin, _epsize, _ep_interval) \
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 2, TUSB_CLASS_VENDOR_SPECIFIC, XINPUT_SUBCLASS_DEFAULT, XINPUT_PROTOCOL_DEFAULT, _stridx, \
  16, HID_DESC_TYPE_HID, U16_TO_U8S_LE(0x0110), 0x01, 0x24, 0x81, 0x14, 0x03, 0x00, 0x03, 0x13, 0x01, 0x00, 0x03, 0, \
  7, TUSB_DESC_ENDPOINT, _epin, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_epsize), _ep_interval, \
  7, TUSB_DESC_ENDPOINT, _epout, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_epsize), 1

typedef struct __attribute((packed, aligned(1))) {
    uint8_t report_id;
    uint8_t report_size;

    bool dpad_up : 1;
    bool dpad_down : 1;
    bool dpad_left : 1;
    bool dpad_right : 1;
    bool start : 1;
    bool back : 1;
    bool ls : 1;
    bool rs : 1;

    bool lb : 1;
    bool rb : 1;
    bool home : 1;
    bool _reserved0 : 1;
    bool a : 1;
    bool b : 1;
    bool x : 1;
    bool y : 1;

    uint8_t lt;
    uint8_t rt;
    int16_t lx;
    int16_t ly;
    int16_t rx;
    int16_t ry;
    uint8_t _reserved1[6];
} xinput_report_t;

bool tud_xinput_ready();
void receive_xinput_report(void);
bool send_xinput_report(xinput_report_t *report);
uint16_t xinput_open(
    uint8_t rhport,
    const tusb_desc_interface_t *itf_descriptor,
    uint16_t max_length
);
bool xinput_xfer_callback(
    uint8_t rhport,
    uint8_t ep_addr,
    xfer_result_t result,
    uint32_t xferred_bytes
);
bool xinput_vendor_control_xfer_cb(
    uint8_t rhport,
    uint8_t stage,
    const tusb_control_request_t *request
);

class TinyUSBXInput : public Adafruit_USBD_Interface {
  public:
    TinyUSBXInput(uint8_t interval_ms = 1);

    bool begin(void);
    bool ready(void);
    bool sendReport(xinput_report_t *report);
    uint16_t getInterfaceDescriptor(uint8_t itfnum, uint8_t *buf, uint16_t bufsize) override;

  private:
    uint8_t _interval_ms;
    uint8_t _endpoint_in = 0;
    uint8_t _endpoint_out = 0;
    uint8_t _xinput_out_buffer[XINPUT_EPSIZE] = {};

    friend bool tud_xinput_ready();
    friend void receive_xinput_report(void);
    friend bool send_xinput_report(xinput_report_t *report);
    friend uint16_t xinput_open(
        uint8_t rhport,
        const tusb_desc_interface_t *itf_descriptor,
        uint16_t max_length
    );
    friend bool xinput_xfer_callback(
        uint8_t rhport,
        uint8_t ep_addr,
        xfer_result_t result,
        uint32_t xferred_bytes
    );
    friend const usbd_class_driver_t *usbd_app_driver_get_cb(uint8_t *driver_count);
    friend bool xinput_vendor_control_xfer_cb(
        uint8_t rhport,
        uint8_t stage,
        const tusb_control_request_t *request
    );
};

#endif
