#include "usb/TinyUSBXInput.hpp"
#include "usb/XboxAuthPassthrough.hpp"
#include "device/usbd_pvt.h"
#include "tusb_option.h"

enum {
    XINPUT_DESC_TYPE_RESERVED = 0x21,
    XINPUT_SECURITY_DESC_TYPE_RESERVED = 0x41,
};

static void xinput_init(void);
void xinput_reset(uint8_t rhport);
static bool xinput_control_xfer_callback(
    uint8_t rhport,
    uint8_t stage,
    const tusb_control_request_t *request
);
bool xinput_vendor_control_xfer_cb(
    uint8_t rhport,
    uint8_t stage,
    const tusb_control_request_t *request
);

static usbd_class_driver_t const xinput_driver = {
#if CFG_TUSB_DEBUG >= 2
    .name = "XINPUT",
#endif
    .init = xinput_init,
    .reset = xinput_reset,
    .open = xinput_open,
    .control_xfer_cb = xinput_control_xfer_callback,
    .xfer_cb = xinput_xfer_callback,
    .sof = nullptr
};

static TinyUSBXInput *xinput_dev = nullptr;

TinyUSBXInput::TinyUSBXInput(uint8_t interval_ms) {
    _interval_ms = interval_ms;
}

uint16_t TinyUSBXInput::getInterfaceDescriptor(uint8_t itfnum, uint8_t *buf, uint16_t bufsize) {
    const uint8_t security_stridx = TinyUSBDevice.addStringDescriptor(XINPUT_SECURITY_STRING);
    const uint8_t desc[] = {
        TUD_XINPUT_DESCRIPTOR(
            itfnum,
            security_stridx,
            XINPUT_EPOUT,
            XINPUT_EPIN,
            XINPUT_EPSIZE,
            _interval_ms
        )
    };
    const uint16_t len = sizeof(desc);

    if (bufsize < len) {
        return 0;
    }

    memcpy(buf, desc, len);
    return len;
}

bool TinyUSBXInput::begin(void) {
    xinput_dev = this;

    if (!TinyUSBDevice.addInterface(*this)) {
        return false;
    }

    TinyUSBDevice.setVersion(0x0200);
    return true;
}

bool TinyUSBXInput::ready(void) {
    return tud_xinput_ready();
}

bool TinyUSBXInput::sendReport(xinput_report_t *report) {
    return send_xinput_report(report);
}

bool tud_xinput_ready() {
    return xinput_dev && xinput_dev->_endpoint_in && tud_ready() &&
           !usbd_edpt_busy(TUD_OPT_RHPORT, xinput_dev->_endpoint_in);
}

void receive_xinput_report(void) {
    if (xinput_dev && xinput_dev->_endpoint_out && tud_ready() &&
        !usbd_edpt_busy(TUD_OPT_RHPORT, xinput_dev->_endpoint_out)) {
        usbd_edpt_claim(TUD_OPT_RHPORT, xinput_dev->_endpoint_out);
        usbd_edpt_xfer(
            TUD_OPT_RHPORT,
            xinput_dev->_endpoint_out,
            xinput_dev->_xinput_out_buffer,
            XINPUT_EPSIZE
        );
        usbd_edpt_release(TUD_OPT_RHPORT, xinput_dev->_endpoint_out);
    }
}

bool send_xinput_report(xinput_report_t *report) {
    bool sent = false;
    if (tud_xinput_ready()) {
        usbd_edpt_claim(TUD_OPT_RHPORT, xinput_dev->_endpoint_in);
        usbd_edpt_xfer(
            TUD_OPT_RHPORT,
            xinput_dev->_endpoint_in,
            reinterpret_cast<uint8_t *>(report),
            sizeof(xinput_report_t)
        );
        usbd_edpt_release(TUD_OPT_RHPORT, xinput_dev->_endpoint_in);
        sent = true;
    }

    return sent;
}

static void xinput_init(void) {}

void xinput_reset(uint8_t rhport) {
    (void)rhport;
}

uint16_t xinput_open(
    uint8_t rhport,
    const tusb_desc_interface_t *itf_descriptor,
    uint16_t max_length
) {
    uint16_t driver_length = 0;

    if (itf_descriptor->bInterfaceClass != TUSB_CLASS_VENDOR_SPECIFIC) {
        return 0;
    }

    if (itf_descriptor->bInterfaceSubClass == 0x5D &&
        (itf_descriptor->bInterfaceProtocol == 0x01 || itf_descriptor->bInterfaceProtocol == 0x02 ||
         itf_descriptor->bInterfaceProtocol == 0x03)) {
        driver_length = sizeof(tusb_desc_interface_t) +
                        (itf_descriptor->bNumEndpoints * sizeof(tusb_desc_endpoint_t));
        TU_VERIFY(max_length >= driver_length, 0);

        auto const *descriptor = reinterpret_cast<uint8_t const *>(itf_descriptor);
        descriptor = tu_desc_next(descriptor);
        TU_VERIFY(descriptor[1] == XINPUT_DESC_TYPE_RESERVED, 0);
        driver_length += descriptor[0];
        descriptor = tu_desc_next(descriptor);

        if (itf_descriptor->bInterfaceProtocol == 0x01) {
            TU_ASSERT(usbd_open_edpt_pair(
                rhport,
                descriptor,
                itf_descriptor->bNumEndpoints,
                TUSB_XFER_INTERRUPT,
                &xinput_dev->_endpoint_out,
                &xinput_dev->_endpoint_in
            ), 0);
        } else {
            for (uint8_t i = 0; i < itf_descriptor->bNumEndpoints; i++) {
                auto const *endpoint = reinterpret_cast<tusb_desc_endpoint_t const *>(descriptor);
                TU_ASSERT(endpoint->bDescriptorType == TUSB_DESC_ENDPOINT, 0);
                TU_ASSERT(usbd_edpt_open(rhport, endpoint), 0);
                descriptor = tu_desc_next(descriptor);
            }
        }

        return driver_length;
    }

    if (itf_descriptor->bInterfaceSubClass == 0xFD && itf_descriptor->bInterfaceProtocol == 0x13) {
        driver_length = sizeof(tusb_desc_interface_t);
        TU_VERIFY(max_length >= driver_length, 0);

        auto const *descriptor = reinterpret_cast<uint8_t const *>(itf_descriptor);
        descriptor = tu_desc_next(descriptor);
        TU_VERIFY(descriptor[1] == XINPUT_SECURITY_DESC_TYPE_RESERVED, 0);
        driver_length += descriptor[0];
        return driver_length;
    }

    return 0;
}

static bool xinput_control_xfer_callback(
    uint8_t rhport,
    uint8_t stage,
    const tusb_control_request_t *request
) {
    (void)rhport;
    (void)stage;
    (void)request;
    return true;
}

bool xinput_xfer_callback(
    uint8_t rhport,
    uint8_t ep_addr,
    xfer_result_t result,
    uint32_t xferred_bytes
) {
    (void)rhport;
    if (ep_addr == xinput_dev->_endpoint_out) {
        usbd_edpt_xfer(
            TUD_OPT_RHPORT,
            xinput_dev->_endpoint_out,
            xinput_dev->_xinput_out_buffer,
            XINPUT_EPSIZE
        );
    }
    return true;
}

bool xinput_vendor_control_xfer_cb(
    uint8_t rhport,
    uint8_t stage,
    const tusb_control_request_t *request
) {
    if (XboxAuthPassthrough::instance().handleVendorControl(rhport, stage, request)) {
        return true;
    }

    if (!xinput_dev) {
        return false;
    }

    if (stage != CONTROL_STAGE_SETUP) {
        return true;
    }

    return false;
}

extern "C" const usbd_class_driver_t *usbd_app_driver_get_cb(uint8_t *driver_count) {
    *driver_count = 1;
    return &xinput_driver;
}

extern "C" const uint8_t *tud_descriptor_bos_cb(void) {
    return nullptr;
}

extern "C" bool tud_vendor_control_xfer_cb(
    uint8_t rhport,
    uint8_t stage,
    const tusb_control_request_t *request
) {
    return xinput_vendor_control_xfer_cb(rhport, stage, request);
}
