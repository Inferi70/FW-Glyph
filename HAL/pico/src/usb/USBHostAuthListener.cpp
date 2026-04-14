#include "usb/USBHostAuthListener.hpp"

#include "class/hid/hid.h"
#include "host/usbh.h"

#include <cstring>

#if CFG_TUH_ENABLED

#include "third_party/tinyusb_gp2040/src/class/hid/hid_host.h"

namespace {

constexpr uint16_t P5_VENDOR_ID = 0x2B81;
constexpr uint16_t P5_PRODUCT_ID = 0x0101;

constexpr uint8_t PS4_REPORT_DEFINITION = 0x03;
constexpr uint8_t PS4_REPORT_SET_AUTH_PAYLOAD = 0xF0;
constexpr uint8_t PS4_REPORT_GET_SIGNATURE_NONCE = 0xF1;
constexpr uint8_t PS4_REPORT_GET_SIGNING_STATE = 0xF2;
constexpr uint8_t PS4_REPORT_RESET_AUTH = 0xF3;

constexpr uint8_t P5_REPORT_SET_AUTH_PAYLOAD = 0xF0;
constexpr uint8_t P5_REPORT_GET_SIGNATURE_NONCE = 0xF1;
constexpr uint8_t P5_REPORT_GET_SIGNING_STATE = 0xF2;

constexpr uint8_t X360_REQUEST_GET_SERIAL = 0x81;
constexpr uint8_t X360_REQUEST_INIT_AUTH = 0x82;
constexpr uint8_t X360_REQUEST_RESPOND_CHALLENGE = 0x83;
constexpr uint8_t X360_REQUEST_AUTH_KEEPALIVE = 0x84;
constexpr uint8_t X360_REQUEST_STATE = 0x86;
constexpr uint8_t X360_REQUEST_VERIFY_AUTH = 0x87;

constexpr uint16_t X360_WVALUE_CONSOLE_DATA = 0x0003;
constexpr uint16_t X360_WVALUE_CONTROLLER_DATA = 0x005C;
constexpr uint16_t X360_WVALUE_CONTROLLER_ID = 0x005B;
constexpr uint16_t X360_WVALUE_NO_DATA = 0x0000;
constexpr uint16_t X360_WINDEX_SECURITY = 0x0301;

void xinput_vendor_complete_cb(tuh_xfer_t *xfer) {
    auto *listener = reinterpret_cast<USBHostAuthListener *>(xfer->user_data);
    if (listener == nullptr || xfer->setup == nullptr) {
        return;
    }

    listener->xinputVendorComplete(xfer->setup->bRequest, xfer->result, xfer->actual_len);
}

} // namespace

bool USBHostAuthListener::available() const {
    return _device_type != USBHostAuthDeviceType::NONE;
}

void USBHostAuthListener::setup() {
    clear();
}

USBHostAuthDeviceType USBHostAuthListener::deviceType() const {
    return _device_type;
}

bool USBHostAuthListener::busy() const {
    return _busy;
}

uint8_t USBHostAuthListener::deviceAddress() const {
    return _dev_addr;
}

uint8_t USBHostAuthListener::instance() const {
    return _instance;
}

const uint8_t *USBHostAuthListener::lastBuffer() const {
    return _last_buffer;
}

uint16_t USBHostAuthListener::lastLength() const {
    return _last_len;
}

uint8_t USBHostAuthListener::lastReportId() const {
    return _last_report_id;
}

void USBHostAuthListener::clear() {
    _device_type = USBHostAuthDeviceType::NONE;
    _dev_addr = 0xFF;
    _instance = 0xFF;
    _interface_number = 0xFF;
    _busy = false;
    _awaiting_cb = false;
    _last_report_id = 0;
    _last_len = 0;
    memset(_last_buffer, 0, sizeof(_last_buffer));
}

void USBHostAuthListener::unmount(uint8_t dev_addr) {
    if (dev_addr == _dev_addr) {
        clear();
    }
}

void USBHostAuthListener::hidMount(
    uint8_t dev_addr,
    uint8_t instance,
    const uint8_t *desc_report,
    uint16_t desc_len
) {
    if (available()) {
        return;
    }

    uint16_t vid = 0xFFFF;
    uint16_t pid = 0xFFFF;
    tuh_vid_pid_get(dev_addr, &vid, &pid);

    if (vid == P5_VENDOR_ID && pid == P5_PRODUCT_ID) {
        _device_type = USBHostAuthDeviceType::P5_HID;
        _dev_addr = dev_addr;
        _instance = instance;
        _interface_number = instance;
        return;
    }

    tuh_hid_report_info_t report_info[4];
    uint8_t report_count = tuh_hid_parse_report_descriptor(report_info, 4, desc_report, desc_len);
    for (uint8_t i = 0; i < report_count; i++) {
        if (report_info[i].usage_page == 0xFFF0 && report_info[i].report_id == PS4_REPORT_RESET_AUTH) {
            _device_type = USBHostAuthDeviceType::PS4_HID;
            _dev_addr = dev_addr;
            _instance = instance;
            _interface_number = instance;
            requestPS4Definition();
            return;
        }
    }
}

void USBHostAuthListener::xinputMount(uint8_t dev_addr, uint8_t instance, uint8_t type, uint8_t subtype) {
    (void) subtype;

    if (available()) {
        return;
    }

    if (type == 1) {
        _device_type = USBHostAuthDeviceType::XINPUT_360;
        _dev_addr = dev_addr;
        _instance = instance;
    }
}

void USBHostAuthListener::xinputUnmount(uint8_t dev_addr, uint8_t instance) {
    if (dev_addr == _dev_addr && instance == _instance &&
        _device_type == USBHostAuthDeviceType::XINPUT_360) {
        clear();
    }
}

void USBHostAuthListener::hidSetReportComplete(
    uint8_t dev_addr,
    uint8_t instance,
    uint8_t report_id,
    uint8_t report_type,
    uint16_t len
) {
    if (dev_addr != _dev_addr || instance != _instance) {
        return;
    }

    (void) report_type;
    _busy = false;
    _awaiting_cb = false;
    _last_report_id = report_id;
    _last_len = len;
}

void USBHostAuthListener::hidGetReportComplete(
    uint8_t dev_addr,
    uint8_t instance,
    uint8_t report_id,
    uint8_t report_type,
    uint16_t len
) {
    if (dev_addr != _dev_addr || instance != _instance) {
        return;
    }

    (void) report_type;
    _busy = false;
    _awaiting_cb = false;
    _last_report_id = report_id;
    _last_len = len;
}

bool USBHostAuthListener::hostGetReport(uint8_t report_id, void *report, uint16_t len) {
    if (!available() || _busy || _awaiting_cb || len > sizeof(_last_buffer)) {
        return false;
    }

    _busy = true;
    _awaiting_cb = true;
    _last_report_id = report_id;
    _last_len = len;
    memset(_last_buffer, 0, sizeof(_last_buffer));
    if (report != nullptr && len <= sizeof(_last_buffer)) {
        memcpy(_last_buffer, report, len);
    }

    tusb_control_request_t request = {
        .bmRequestType_bit =
            {
                .recipient = TUSB_REQ_RCPT_INTERFACE,
                .type = TUSB_REQ_TYPE_CLASS,
                .direction = TUSB_DIR_IN,
            },
        .bRequest = HID_REQ_CONTROL_GET_REPORT,
        .wValue = tu_htole16(tu_u16(HID_REPORT_TYPE_FEATURE, report_id)),
        .wIndex = tu_htole16(static_cast<uint16_t>(_interface_number)),
        .wLength = len,
    };

    xfer_result_t result = XFER_RESULT_FAILED;
    tuh_xfer_t xfer = {
        .daddr = _dev_addr,
        .ep_addr = 0,
        .setup = &request,
        .buffer = _last_buffer,
        .complete_cb = nullptr,
        .user_data = reinterpret_cast<uintptr_t>(&result),
    };

    bool ok = tuh_control_xfer(&xfer) && result == XFER_RESULT_SUCCESS;
    _busy = false;
    _awaiting_cb = false;
    _last_len = ok ? len : 0;
    return ok;
}

bool USBHostAuthListener::hostSetReport(uint8_t report_id, void *report, uint16_t len) {
    if (!available() || _busy || _awaiting_cb) {
        return false;
    }

    _busy = true;
    _awaiting_cb = true;
    _last_report_id = report_id;
    _last_len = len;

    memset(_last_buffer, 0, sizeof(_last_buffer));
    if (report != nullptr && len <= sizeof(_last_buffer)) {
        memcpy(_last_buffer, report, len);
    }

    bool ok = tuh_hid_set_report(_dev_addr, _instance, report_id, HID_REPORT_TYPE_FEATURE, _last_buffer, len);
    if (!ok) {
        _busy = false;
        _awaiting_cb = false;
    }
    return ok;
}

bool USBHostAuthListener::requestPS4Definition() {
    if (_device_type != USBHostAuthDeviceType::PS4_HID) {
        return false;
    }

    _last_buffer[0] = PS4_REPORT_DEFINITION;
    return hostGetReport(PS4_REPORT_DEFINITION, _last_buffer, 48);
}

bool USBHostAuthListener::requestPS4ResetAuth() {
    if (_device_type != USBHostAuthDeviceType::PS4_HID) {
        return false;
    }

    _last_buffer[0] = PS4_REPORT_RESET_AUTH;
    _last_buffer[1] = 0x38;
    _last_buffer[2] = 0x38;
    return hostGetReport(PS4_REPORT_RESET_AUTH, _last_buffer, 8);
}

bool USBHostAuthListener::requestPS4SigningState() {
    if (_device_type != USBHostAuthDeviceType::PS4_HID) {
        return false;
    }

    _last_buffer[0] = PS4_REPORT_GET_SIGNING_STATE;
    return hostGetReport(PS4_REPORT_GET_SIGNING_STATE, _last_buffer, 16);
}

bool USBHostAuthListener::requestPS4SignatureNonce(uint8_t nonce_id, uint8_t nonce_chunk) {
    if (_device_type != USBHostAuthDeviceType::PS4_HID) {
        return false;
    }

    _last_buffer[0] = PS4_REPORT_GET_SIGNATURE_NONCE;
    _last_buffer[1] = nonce_id;
    _last_buffer[2] = nonce_chunk;
    return hostGetReport(PS4_REPORT_GET_SIGNATURE_NONCE, _last_buffer, 64);
}

bool USBHostAuthListener::sendPS4AuthPayload(const uint8_t *payload, uint16_t len) {
    if (_device_type != USBHostAuthDeviceType::PS4_HID || payload == nullptr || len > sizeof(_last_buffer)) {
        return false;
    }

    memcpy(_last_buffer, payload, len);
    return hostSetReport(PS4_REPORT_SET_AUTH_PAYLOAD, _last_buffer, len);
}

bool USBHostAuthListener::requestP5SignatureNonce(uint16_t len) {
    if (_device_type != USBHostAuthDeviceType::P5_HID || len > sizeof(_last_buffer)) {
        return false;
    }

    return hostGetReport(P5_REPORT_GET_SIGNATURE_NONCE, _last_buffer, len);
}

bool USBHostAuthListener::requestP5SigningState(uint16_t len) {
    if (_device_type != USBHostAuthDeviceType::P5_HID || len > sizeof(_last_buffer)) {
        return false;
    }

    return hostGetReport(P5_REPORT_GET_SIGNING_STATE, _last_buffer, len);
}

bool USBHostAuthListener::sendP5AuthPayload(const uint8_t *payload, uint16_t len) {
    if (_device_type != USBHostAuthDeviceType::P5_HID || payload == nullptr || len > sizeof(_last_buffer)) {
        return false;
    }

    memcpy(_last_buffer, payload, len);
    return hostSetReport(P5_REPORT_SET_AUTH_PAYLOAD, _last_buffer, len);
}

bool USBHostAuthListener::requestXInput360Serial() {
    if (_device_type != USBHostAuthDeviceType::XINPUT_360) {
        return false;
    }

    return xinputVendorTransfer(
        TUSB_DIR_IN,
        X360_REQUEST_GET_SERIAL,
        TU_U16(X360_WVALUE_CONTROLLER_ID, static_cast<uint8_t>(29 - 6)),
        29,
        nullptr
    );
}

bool USBHostAuthListener::sendXInput360InitAuth(const uint8_t *payload, uint16_t len) {
    if (_device_type != USBHostAuthDeviceType::XINPUT_360 || payload == nullptr) {
        return false;
    }

    return xinputVendorTransfer(TUSB_DIR_OUT, X360_REQUEST_INIT_AUTH, X360_WVALUE_CONSOLE_DATA, len, payload);
}

bool USBHostAuthListener::sendXInput360VerifyAuth(const uint8_t *payload, uint16_t len) {
    if (_device_type != USBHostAuthDeviceType::XINPUT_360 || payload == nullptr) {
        return false;
    }

    return xinputVendorTransfer(TUSB_DIR_OUT, X360_REQUEST_VERIFY_AUTH, X360_WVALUE_CONSOLE_DATA, len, payload);
}

bool USBHostAuthListener::requestXInput360ChallengeResponse(uint16_t len) {
    if (_device_type != USBHostAuthDeviceType::XINPUT_360) {
        return false;
    }

    return xinputVendorTransfer(
        TUSB_DIR_IN,
        X360_REQUEST_RESPOND_CHALLENGE,
        TU_U16(X360_WVALUE_CONTROLLER_DATA, static_cast<uint8_t>(len - 6)),
        len,
        nullptr
    );
}

bool USBHostAuthListener::requestXInput360State() {
    if (_device_type != USBHostAuthDeviceType::XINPUT_360) {
        return false;
    }

    return xinputVendorTransfer(TUSB_DIR_IN, X360_REQUEST_STATE, X360_WVALUE_NO_DATA, 2, nullptr);
}

bool USBHostAuthListener::sendXInput360KeepAlive() {
    if (_device_type != USBHostAuthDeviceType::XINPUT_360) {
        return false;
    }

    return xinputVendorTransfer(TUSB_DIR_IN, X360_REQUEST_AUTH_KEEPALIVE, X360_WVALUE_CONSOLE_DATA, 0, nullptr);
}

bool USBHostAuthListener::xinputVendorTransfer(
    tusb_dir_t dir,
    uint8_t request,
    uint16_t value,
    uint16_t len,
    const uint8_t *payload
) {
    if (_device_type != USBHostAuthDeviceType::XINPUT_360 || !tuh_ready(_dev_addr) || _busy || _awaiting_cb ||
        len > sizeof(_last_buffer)) {
        return false;
    }

    _busy = true;
    _awaiting_cb = true;
    _last_report_id = request;
    _last_len = 0;
    memset(_last_buffer, 0, sizeof(_last_buffer));
    if (payload != nullptr && len > 0) {
        memcpy(_last_buffer, payload, len);
    }

    _xinput_control_request = {
        .bmRequestType_bit =
            {
                .recipient = TUSB_REQ_RCPT_INTERFACE,
                .type = TUSB_REQ_TYPE_VENDOR,
                .direction = dir,
            },
        .bRequest = request,
        .wValue = value,
        .wIndex = X360_WINDEX_SECURITY,
        .wLength = len,
    };

    tuh_xfer_t xfer = {
        .daddr = _dev_addr,
        .ep_addr = 0,
        .setup = &_xinput_control_request,
        .buffer = _last_buffer,
        .complete_cb = xinput_vendor_complete_cb,
        .user_data = reinterpret_cast<uintptr_t>(this),
    };

    if (!tuh_control_xfer(&xfer)) {
        _busy = false;
        _awaiting_cb = false;
        _last_len = 0;
        return false;
    }

    return true;
}

void USBHostAuthListener::xinputVendorComplete(uint8_t request, xfer_result_t result, uint32_t actual_len) {
    _busy = false;
    _awaiting_cb = false;
    _last_report_id = request;
    _last_len = (result == XFER_RESULT_SUCCESS) ? static_cast<uint16_t>(actual_len) : 0;
}

#else

void USBHostAuthListener::setup() {}

bool USBHostAuthListener::available() const {
    return false;
}

USBHostAuthDeviceType USBHostAuthListener::deviceType() const {
    return USBHostAuthDeviceType::NONE;
}

bool USBHostAuthListener::busy() const {
    return false;
}

uint8_t USBHostAuthListener::deviceAddress() const {
    return 0xFF;
}

uint8_t USBHostAuthListener::instance() const {
    return 0xFF;
}

const uint8_t *USBHostAuthListener::lastBuffer() const {
    return _last_buffer;
}

uint16_t USBHostAuthListener::lastLength() const {
    return 0;
}

uint8_t USBHostAuthListener::lastReportId() const {
    return 0;
}

void USBHostAuthListener::unmount(uint8_t dev_addr) {
    (void) dev_addr;
}

void USBHostAuthListener::hidMount(
    uint8_t dev_addr,
    uint8_t instance,
    const uint8_t *desc_report,
    uint16_t desc_len
) {
    (void) dev_addr;
    (void) instance;
    (void) desc_report;
    (void) desc_len;
}

void USBHostAuthListener::xinputMount(uint8_t dev_addr, uint8_t instance, uint8_t type, uint8_t subtype) {
    (void) dev_addr;
    (void) instance;
    (void) type;
    (void) subtype;
}

void USBHostAuthListener::xinputUnmount(uint8_t dev_addr, uint8_t instance) {
    (void) dev_addr;
    (void) instance;
}

void USBHostAuthListener::hidSetReportComplete(
    uint8_t dev_addr,
    uint8_t instance,
    uint8_t report_id,
    uint8_t report_type,
    uint16_t len
) {
    (void) dev_addr;
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) len;
}

void USBHostAuthListener::hidGetReportComplete(
    uint8_t dev_addr,
    uint8_t instance,
    uint8_t report_id,
    uint8_t report_type,
    uint16_t len
) {
    (void) dev_addr;
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) len;
}

bool USBHostAuthListener::requestPS4Definition() {
    return false;
}

bool USBHostAuthListener::requestPS4ResetAuth() {
    return false;
}

bool USBHostAuthListener::requestPS4SigningState() {
    return false;
}

bool USBHostAuthListener::requestPS4SignatureNonce(uint8_t nonce_id, uint8_t nonce_chunk) {
    (void) nonce_id;
    (void) nonce_chunk;
    return false;
}

bool USBHostAuthListener::sendPS4AuthPayload(const uint8_t *payload, uint16_t len) {
    (void) payload;
    (void) len;
    return false;
}

bool USBHostAuthListener::requestP5SignatureNonce(uint16_t len) {
    (void) len;
    return false;
}

bool USBHostAuthListener::requestP5SigningState(uint16_t len) {
    (void) len;
    return false;
}

bool USBHostAuthListener::sendP5AuthPayload(const uint8_t *payload, uint16_t len) {
    (void) payload;
    (void) len;
    return false;
}

bool USBHostAuthListener::requestXInput360Serial() {
    return false;
}

bool USBHostAuthListener::sendXInput360InitAuth(const uint8_t *payload, uint16_t len) {
    (void) payload;
    (void) len;
    return false;
}

bool USBHostAuthListener::sendXInput360VerifyAuth(const uint8_t *payload, uint16_t len) {
    (void) payload;
    (void) len;
    return false;
}

bool USBHostAuthListener::requestXInput360ChallengeResponse(uint16_t len) {
    (void) len;
    return false;
}

bool USBHostAuthListener::requestXInput360State() {
    return false;
}

bool USBHostAuthListener::sendXInput360KeepAlive() {
    return false;
}

bool USBHostAuthListener::hostGetReport(uint8_t report_id, void *report, uint16_t len) {
    (void) report_id;
    (void) report;
    (void) len;
    return false;
}

bool USBHostAuthListener::hostSetReport(uint8_t report_id, void *report, uint16_t len) {
    (void) report_id;
    (void) report;
    (void) len;
    return false;
}

bool USBHostAuthListener::xinputVendorTransfer(
    tusb_dir_t dir,
    uint8_t request,
    uint16_t value,
    uint16_t len,
    const uint8_t *payload
) {
    (void) dir;
    (void) request;
    (void) value;
    (void) len;
    (void) payload;
    return false;
}

void USBHostAuthListener::xinputVendorComplete(uint8_t request, xfer_result_t result, uint32_t actual_len) {
    (void) request;
    (void) result;
    (void) actual_len;
}

void USBHostAuthListener::clear() {}

#endif
