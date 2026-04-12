#include "usb/TinyUSBHID.hpp"

#if CFG_TUD_ENABLED && CFG_TUD_HID

#define TINYUSB_HID_EPOUT 0x00
#define TINYUSB_HID_EPIN 0x80

static TinyUSBHID *hid_instances[CFG_TUD_HID] = {0};

uint8_t TinyUSBHID::_instance_count = 0;

TinyUSBHID::TinyUSBHID(
    uint8_t const *desc_report,
    uint16_t len,
    uint8_t protocol,
    uint8_t interval_ms,
    bool has_out_endpoint
) {
    _instance = INVALID_INSTANCE;
    _interval_ms = interval_ms;
    _protocol = protocol;
    _out_endpoint = has_out_endpoint;
    _desc_report = desc_report;
    _desc_report_len = len;
    _get_report_cb = nullptr;
    _set_report_cb = nullptr;
}

void TinyUSBHID::setPollInterval(uint8_t interval_ms) {
    _interval_ms = interval_ms;
}

void TinyUSBHID::setBootProtocol(uint8_t protocol) {
    _protocol = protocol;
}

bool TinyUSBHID::isOutEndpointEnabled(void) {
    return _out_endpoint;
}

void TinyUSBHID::enableOutEndpoint(bool enable) {
    _out_endpoint = enable;
}

void TinyUSBHID::setReportDescriptor(uint8_t const *desc_report, uint16_t len) {
    _desc_report = desc_report;
    _desc_report_len = len;
}

void TinyUSBHID::setReportCallback(get_report_callback_t get_report, set_report_callback_t set_report) {
    _get_report_cb = get_report;
    _set_report_cb = set_report;
}

uint16_t TinyUSBHID::makeItfDesc(
    uint8_t itfnum,
    uint8_t *buf,
    uint16_t bufsize,
    uint8_t ep_in,
    uint8_t ep_out
) {
    if (!_desc_report_len) {
        return 0;
    }

    uint8_t const desc_inout[] = {
        TUD_HID_INOUT_DESCRIPTOR(
            itfnum,
            0,
            _protocol,
            _desc_report_len,
            ep_in,
            ep_out,
            CFG_TUD_HID_EP_BUFSIZE,
            _interval_ms
        )
    };
    uint8_t const desc_in_only[] = {
        TUD_HID_DESCRIPTOR(
            itfnum,
            0,
            _protocol,
            _desc_report_len,
            ep_in,
            CFG_TUD_HID_EP_BUFSIZE,
            _interval_ms
        )
    };

    uint8_t const *desc = _out_endpoint ? desc_inout : desc_in_only;
    uint16_t len = _out_endpoint ? sizeof(desc_inout) : sizeof(desc_in_only);

    if (buf) {
        if (bufsize < len) {
            return 0;
        }

        memcpy(buf, desc, len);
    }

    return len;
}

uint16_t TinyUSBHID::getInterfaceDescriptor(uint8_t itfnum, uint8_t *buf, uint16_t bufsize) {
    return makeItfDesc(itfnum, buf, bufsize, TINYUSB_HID_EPIN, TINYUSB_HID_EPOUT);
}

bool TinyUSBHID::begin(void) {
    if (isValid()) {
        return true;
    }

    if (_instance_count >= CFG_TUD_HID) {
        return false;
    }

    if (!TinyUSBDevice.addInterface(*this)) {
        return false;
    }

    _instance = _instance_count++;
    hid_instances[_instance] = this;

    return true;
}

bool TinyUSBHID::ready(void) {
    return tud_hid_n_ready(_instance);
}

bool TinyUSBHID::sendReport(uint8_t report_id, void const *report, uint8_t len) {
    return tud_hid_n_report(_instance, report_id, report, len);
}

extern "C" {

uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf) {
    TinyUSBHID *hid = hid_instances[itf];

    if (!hid) {
        return nullptr;
    }

    return hid->_desc_report;
}

uint16_t tud_hid_get_report_cb(
    uint8_t itf,
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t *buffer,
    uint16_t reqlen
) {
    TinyUSBHID *hid = hid_instances[itf];

    if (!(hid && hid->_get_report_cb)) {
        return 0;
    }

    return hid->_get_report_cb(report_id, report_type, buffer, reqlen);
}

void tud_hid_set_report_cb(
    uint8_t itf,
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t const *buffer,
    uint16_t bufsize
) {
    TinyUSBHID *hid = hid_instances[itf];

    if (!(hid && hid->_set_report_cb)) {
        return;
    }

    hid->_set_report_cb(report_id, report_type, buffer, bufsize);
}

}

#endif
