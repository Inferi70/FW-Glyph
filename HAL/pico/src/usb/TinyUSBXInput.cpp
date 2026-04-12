#include "usb/TinyUSBXInput.hpp"
#include "device/usbd_pvt.h"
#include "tusb_option.h"

enum {
    VENDOR_REQUEST_MICROSOFT = 1,
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

#define BOS_TOTAL_LEN (TUD_BOS_DESC_LEN + TUD_BOS_MICROSOFT_OS_DESC_LEN)
#define MS_OS_20_DESC_LEN 0xB2

const uint8_t desc_bos[] = {
    TUD_BOS_DESCRIPTOR(BOS_TOTAL_LEN, 1),
    TUD_BOS_MS_OS_20_DESCRIPTOR(MS_OS_20_DESC_LEN, VENDOR_REQUEST_MICROSOFT)
};

static TinyUSBXInput *xinput_dev = nullptr;

uint8_t desc_ms_os_20[MS_OS_20_DESC_LEN] = {
    U16_TO_U8S_LE(0x000A), U16_TO_U8S_LE(MS_OS_20_SET_HEADER_DESCRIPTOR),
    U32_TO_U8S_LE(0x06030000), U16_TO_U8S_LE(MS_OS_20_DESC_LEN),
    U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_CONFIGURATION),
    0, 0, U16_TO_U8S_LE(MS_OS_20_DESC_LEN - 0x0A),
    U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_FUNCTION),
    0, 0, U16_TO_U8S_LE(MS_OS_20_DESC_LEN - 0x0A - 0x08),
    U16_TO_U8S_LE(0x0014), U16_TO_U8S_LE(MS_OS_20_FEATURE_COMPATBLE_ID), 'X',
    'U', 'S', 'B', '2', '0', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00,
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN - 0x0A - 0x08 - 0x08 - 0x14),
    U16_TO_U8S_LE(MS_OS_20_FEATURE_REG_PROPERTY), U16_TO_U8S_LE(0x0007),
    U16_TO_U8S_LE(0x002A),
    'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00,
    'n', 0x00, 't', 0x00, 'e', 0x00, 'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00,
    'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00,
    0x00,
    U16_TO_U8S_LE(0x0050),
    '{', 0x00, '8', 0x00, 'D', 0x00, '9', 0x00, '0', 0x00, '8', 0x00, '4', 0x00,
    '2', 0x00, 'C', 0x00, '-', 0x00, '1', 0x00, '5', 0x00, '9', 0x00, '4', 0x00,
    '-', 0x00, '4', 0x00, '1', 0x00, 'C', 0x00, 'E', 0x00, '-', 0x00, 'A', 0x00,
    'A', 0x00, '3', 0x00, 'F', 0x00, '-', 0x00, '6', 0x00, '2', 0x00, 'D', 0x00,
    '4', 0x00, '6', 0x00, '4', 0x00, 'E', 0x00, '1', 0x00, 'B', 0x00, 'E', 0x00,
    '7', 0x00, '9', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00
};

TinyUSBXInput::TinyUSBXInput(uint8_t interval_ms) {
    _interval_ms = interval_ms;
}

uint16_t TinyUSBXInput::getInterfaceDescriptor(uint8_t itfnum, uint8_t *buf, uint16_t bufsize) {
    const uint8_t desc[] = {
        TUD_XINPUT_DESCRIPTOR(itfnum, 0, XINPUT_EPOUT, XINPUT_EPIN, XINPUT_EPSIZE, _interval_ms)
    };
    const uint16_t len = sizeof(desc);

    if (bufsize < len) {
        return 0;
    }

    memcpy(buf, desc, len);
    desc_ms_os_20[0x0a + 0x08 + 4] = itfnum;
    return len;
}

bool TinyUSBXInput::begin(void) {
    xinput_dev = this;

    if (!TinyUSBDevice.addInterface(*this)) {
        return false;
    }

    TinyUSBDevice.setVersion(0x0210);
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
    if (itf_descriptor->bInterfaceClass != TUSB_CLASS_VENDOR_SPECIFIC ||
        itf_descriptor->bInterfaceSubClass != XINPUT_SUBCLASS_DEFAULT ||
        itf_descriptor->bInterfaceProtocol != XINPUT_PROTOCOL_DEFAULT) {
        return false;
    }

    uint16_t driver_length = sizeof(tusb_desc_interface_t) +
                             (itf_descriptor->bNumEndpoints * sizeof(tusb_desc_endpoint_t)) + 16;

    TU_VERIFY(max_length >= driver_length, 0);

    const uint8_t *current_descriptor = tu_desc_next(itf_descriptor);
    uint8_t found_endpoints = 0;
    while ((found_endpoints < itf_descriptor->bNumEndpoints) && (driver_length <= max_length)) {
        const tusb_desc_endpoint_t *endpoint_descriptor =
            reinterpret_cast<const tusb_desc_endpoint_t *>(current_descriptor);
        if (TUSB_DESC_ENDPOINT == tu_desc_type(endpoint_descriptor)) {
            TU_ASSERT(usbd_edpt_open(rhport, endpoint_descriptor));

            if (tu_edpt_dir(endpoint_descriptor->bEndpointAddress) == TUSB_DIR_IN) {
                xinput_dev->_endpoint_in = endpoint_descriptor->bEndpointAddress;
            } else {
                xinput_dev->_endpoint_out = endpoint_descriptor->bEndpointAddress;
            }

            ++found_endpoints;
        }

        current_descriptor = tu_desc_next(current_descriptor);
    }
    return driver_length;
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
    if (!xinput_dev) {
        return false;
    }

    if (stage != CONTROL_STAGE_SETUP) {
        return true;
    }

    switch (request->bmRequestType_bit.type) {
        case TUSB_REQ_TYPE_VENDOR:
            switch (request->bRequest) {
                case VENDOR_REQUEST_MICROSOFT:
                    if (request->wIndex == 7) {
                        uint16_t total_len;
                        memcpy(&total_len, desc_ms_os_20 + 8, 2);
                        return tud_control_xfer(rhport, request, (void *)desc_ms_os_20, total_len);
                    } else {
                        return false;
                    }
                default:
                    break;
            }
            break;
        default:
            return false;
    }

    return true;
}

extern "C" const usbd_class_driver_t *usbd_app_driver_get_cb(uint8_t *driver_count) {
    *driver_count = 1;
    return &xinput_driver;
}

extern "C" const uint8_t *tud_descriptor_bos_cb(void) {
    return desc_bos;
}

extern "C" bool tud_vendor_control_xfer_cb(
    uint8_t rhport,
    uint8_t stage,
    const tusb_control_request_t *request
) {
    return xinput_vendor_control_xfer_cb(rhport, stage, request);
}
