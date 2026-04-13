#include "usb/USBHostAuthPassthrough.hpp"

#include <cstring>

namespace {

constexpr uint8_t PS4_REPORT_DEFINITION = 0x03;
constexpr uint8_t PS4_REPORT_SET_AUTH_PAYLOAD = 0xF0;
constexpr uint8_t PS4_REPORT_GET_SIGNATURE_NONCE = 0xF1;
constexpr uint8_t PS4_REPORT_GET_SIGNING_STATE = 0xF2;
constexpr uint8_t PS4_REPORT_RESET_AUTH = 0xF3;

constexpr uint8_t P5_REPORT_SET_AUTH_PAYLOAD = 0xF0;
constexpr uint8_t P5_REPORT_GET_SIGNATURE_NONCE = 0xF1;
constexpr uint8_t P5_REPORT_GET_SIGNING_STATE = 0xF2;

} // namespace

USBHostAuthPassthrough &USBHostAuthPassthrough::instance() {
    static USBHostAuthPassthrough passthrough;
    return passthrough;
}

void USBHostAuthPassthrough::attach(USBHostAuthListener *listener) {
    _listener = listener;
    reset();
}

void USBHostAuthPassthrough::reset() {
    _request_report_id = 0;
    _request_len = 0;
    memset(_request_buffer, 0, sizeof(_request_buffer));
    _request_type = RequestType::NONE;
    _request_pending = false;
    _request_in_flight = false;

    _response_ready = false;
    _response_was_get = false;
    _response_report_id = 0;
    _response_len = 0;
    memset(_response_buffer, 0, sizeof(_response_buffer));
}

bool USBHostAuthPassthrough::ready() const {
    return _listener != nullptr && _listener->available();
}

USBHostAuthDeviceType USBHostAuthPassthrough::deviceType() const {
    return _listener != nullptr ? _listener->deviceType() : USBHostAuthDeviceType::NONE;
}

bool USBHostAuthPassthrough::submitGetReport(uint8_t report_id, const uint8_t *payload, uint16_t len) {
    if (_listener == nullptr || len > sizeof(_request_buffer) || _request_pending || _request_in_flight) {
        return false;
    }

    _request_report_id = report_id;
    _request_len = len;
    _request_type = RequestType::GET;
    _request_pending = true;
    _response_ready = false;
    _response_was_get = false;
    _response_report_id = 0;
    _response_len = 0;
    memset(_request_buffer, 0, sizeof(_request_buffer));
    if (payload != nullptr && len > 0) {
        memcpy(_request_buffer, payload, len);
    }
    return true;
}

bool USBHostAuthPassthrough::submitSetReport(uint8_t report_id, const uint8_t *payload, uint16_t len) {
    if (_listener == nullptr || len > sizeof(_request_buffer) || _request_pending || _request_in_flight) {
        return false;
    }

    _request_report_id = report_id;
    _request_len = len;
    _request_type = RequestType::SET;
    _request_pending = true;
    _response_ready = false;
    _response_was_get = false;
    _response_report_id = 0;
    _response_len = 0;
    memset(_request_buffer, 0, sizeof(_request_buffer));
    if (payload != nullptr && len > 0) {
        memcpy(_request_buffer, payload, len);
    }
    return true;
}

bool USBHostAuthPassthrough::requestInFlight() const {
    return _request_pending || _request_in_flight;
}

bool USBHostAuthPassthrough::responseReady() const {
    return _response_ready;
}

bool USBHostAuthPassthrough::responseWasGet() const {
    return _response_was_get;
}

uint8_t USBHostAuthPassthrough::responseReportId() const {
    return _response_report_id;
}

uint16_t USBHostAuthPassthrough::responseLength() const {
    return _response_len;
}

bool USBHostAuthPassthrough::copyResponse(uint8_t *dst, uint16_t max_len, uint16_t *actual_len) const {
    if (!_response_ready || dst == nullptr || max_len < _response_len) {
        return false;
    }

    memcpy(dst, _response_buffer, _response_len);
    if (actual_len != nullptr) {
        *actual_len = _response_len;
    }
    return true;
}

void USBHostAuthPassthrough::clearResponse() {
    _response_ready = false;
    _response_was_get = false;
    _response_report_id = 0;
    _response_len = 0;
    memset(_response_buffer, 0, sizeof(_response_buffer));
}

bool USBHostAuthPassthrough::dispatchPS4Get(uint8_t report_id, const uint8_t *payload, uint16_t len) {
    switch (report_id) {
        case PS4_REPORT_DEFINITION:
            return _listener->requestPS4Definition();
        case PS4_REPORT_RESET_AUTH:
            return _listener->requestPS4ResetAuth();
        case PS4_REPORT_GET_SIGNING_STATE:
            return _listener->requestPS4SigningState();
        case PS4_REPORT_GET_SIGNATURE_NONCE:
            if (len < 3) {
                return false;
            }
            return _listener->requestPS4SignatureNonce(payload[1], payload[2]);
        default:
            return false;
    }
}

bool USBHostAuthPassthrough::dispatchPS4Set(uint8_t report_id, const uint8_t *payload, uint16_t len) {
    if (report_id != PS4_REPORT_SET_AUTH_PAYLOAD || payload == nullptr) {
        return false;
    }
    return _listener->sendPS4AuthPayload(payload, len);
}

bool USBHostAuthPassthrough::dispatchP5Get(uint8_t report_id, const uint8_t *payload, uint16_t len) {
    (void) payload;

    switch (report_id) {
        case P5_REPORT_GET_SIGNATURE_NONCE:
            return _listener->requestP5SignatureNonce(len);
        case P5_REPORT_GET_SIGNING_STATE:
            return _listener->requestP5SigningState(len);
        default:
            return false;
    }
}

bool USBHostAuthPassthrough::dispatchP5Set(uint8_t report_id, const uint8_t *payload, uint16_t len) {
    if (report_id != P5_REPORT_SET_AUTH_PAYLOAD || payload == nullptr) {
        return false;
    }
    return _listener->sendP5AuthPayload(payload, len);
}

void USBHostAuthPassthrough::process() {
    if (_listener == nullptr) {
        reset();
        return;
    }

    if (!_listener->available()) {
        reset();
        return;
    }

    if (_request_in_flight) {
        if (_listener->busy()) {
            return;
        }

        _request_in_flight = false;
        if (_request_type == RequestType::GET &&
            _listener->lastReportId() == _request_report_id &&
            _listener->lastLength() <= sizeof(_response_buffer)) {
            _response_ready = true;
            _response_was_get = true;
            _response_report_id = _listener->lastReportId();
            _response_len = _listener->lastLength();
            memset(_response_buffer, 0, sizeof(_response_buffer));
            if (_response_len > 0) {
                memcpy(_response_buffer, _listener->lastBuffer(), _response_len);
            }
        }

        _request_type = RequestType::NONE;
        _request_report_id = 0;
        _request_len = 0;
        memset(_request_buffer, 0, sizeof(_request_buffer));
        return;
    }

    if (!_request_pending) {
        return;
    }

    bool dispatched = false;
    switch (_listener->deviceType()) {
        case USBHostAuthDeviceType::PS4_HID:
            dispatched = (_request_type == RequestType::GET)
                           ? dispatchPS4Get(_request_report_id, _request_buffer, _request_len)
                           : dispatchPS4Set(_request_report_id, _request_buffer, _request_len);
            break;
        case USBHostAuthDeviceType::P5_HID:
            dispatched = (_request_type == RequestType::GET)
                           ? dispatchP5Get(_request_report_id, _request_buffer, _request_len)
                           : dispatchP5Set(_request_report_id, _request_buffer, _request_len);
            break;
        default:
            dispatched = false;
            break;
    }

    if (!dispatched) {
        _request_pending = false;
        _request_type = RequestType::NONE;
        _request_report_id = 0;
        _request_len = 0;
        memset(_request_buffer, 0, sizeof(_request_buffer));
        return;
    }

    _request_pending = false;
    _request_in_flight = true;
}
