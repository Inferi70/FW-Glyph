#ifndef _USB_TINYUSB_XINPUT_HPP
#define _USB_TINYUSB_XINPUT_HPP

#include "arduino/Adafruit_USBD_Device.h"
#include "device/usbd_pvt.h"

#define XINPUT_SUBCLASS_DEFAULT 0x5D
#define XINPUT_PROTOCOL_DEFAULT 1

#define XINPUT_EPOUT 0x02
#define XINPUT_EPIN 0x81
#define XINPUT_EPSIZE 32
#define XINPUT_AUDIO_IN_EPIN 0x83
#define XINPUT_AUDIO_OUT_EPOUT 0x04
#define XINPUT_AUDIO_AUX_IN_EPIN 0x85
#define XINPUT_AUDIO_AUX_OUT_EPOUT 0x06
#define XINPUT_PLUGIN_EPIN 0x86

#define TUD_XINPUT_DESC_LEN 144

#define XINPUT_SECURITY_STRING "Xbox Security Method 3, Version 1.00, \xA9 2005 Microsoft Corporation. All rights reserved."

#define TUD_XINPUT_DESCRIPTOR(_itfnum, _stridx, _epout, _epin, _epsize, _ep_interval) \
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 2, TUSB_CLASS_VENDOR_SPECIFIC, 0x5D, 0x01, 0, \
  17, HID_DESC_TYPE_HID, U16_TO_U8S_LE(0x0100), 0x01, 0x25, _epin, 0x14, 0x00, 0x00, 0x00, 0x00, 0x13, _epout, 0x08, 0x00, 0x00, \
  7, TUSB_DESC_ENDPOINT, _epin, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_epsize), _ep_interval, \
  7, TUSB_DESC_ENDPOINT, _epout, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_epsize), 8, \
  9, TUSB_DESC_INTERFACE, (uint8_t)((_itfnum) + 1), 0, 4, TUSB_CLASS_VENDOR_SPECIFIC, 0x5D, 0x03, 0, \
  27, HID_DESC_TYPE_HID, U16_TO_U8S_LE(0x0100), 0x01, 0x01, XINPUT_AUDIO_IN_EPIN, 0x40, 0x01, XINPUT_AUDIO_OUT_EPOUT, 0x20, 0x16, XINPUT_AUDIO_AUX_IN_EPIN, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, XINPUT_AUDIO_AUX_OUT_EPOUT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
  7, TUSB_DESC_ENDPOINT, XINPUT_AUDIO_IN_EPIN, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(0x20), 2, \
  7, TUSB_DESC_ENDPOINT, XINPUT_AUDIO_OUT_EPOUT, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(0x20), 4, \
  7, TUSB_DESC_ENDPOINT, XINPUT_AUDIO_AUX_IN_EPIN, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(0x20), 0x40, \
  7, TUSB_DESC_ENDPOINT, XINPUT_AUDIO_AUX_OUT_EPOUT, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(0x20), 0x10, \
  9, TUSB_DESC_INTERFACE, (uint8_t)((_itfnum) + 2), 0, 1, TUSB_CLASS_VENDOR_SPECIFIC, 0x5D, 0x02, 0, \
  9, HID_DESC_TYPE_HID, U16_TO_U8S_LE(0x0100), 0x01, 0x22, XINPUT_PLUGIN_EPIN, 0x03, 0x00, \
  7, TUSB_DESC_ENDPOINT, XINPUT_PLUGIN_EPIN, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(0x20), 0x10, \
  9, TUSB_DESC_INTERFACE, (uint8_t)((_itfnum) + 3), 0, 0, TUSB_CLASS_VENDOR_SPECIFIC, 0xFD, 0x13, _stridx, \
  6, 0x41, 0x00, 0x01, 0x01, 0x03

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
