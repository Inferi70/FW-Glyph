#include "usb/XboxAuthPassthrough.hpp"

#include "Arduino.h"

#include <cstring>

namespace {

constexpr uint8_t X360_REQUEST_GET_SERIAL = 0x81;
constexpr uint8_t X360_REQUEST_INIT_AUTH = 0x82;
constexpr uint8_t X360_REQUEST_RESPOND_CHALLENGE = 0x83;
constexpr uint8_t X360_REQUEST_AUTH_KEEPALIVE = 0x84;
constexpr uint8_t X360_REQUEST_STATE = 0x86;
constexpr uint8_t X360_REQUEST_VERIFY_AUTH = 0x87;

constexpr uint16_t X360_STATE_IN_PROGRESS = 1;
constexpr uint16_t X360_STATE_COMPLETE = 2;

constexpr uint16_t X360_AUTHLEN_CONSOLE_INIT = 34;
constexpr uint16_t X360_AUTHLEN_DONGLE_SERIAL = 29;
constexpr uint16_t X360_AUTHLEN_DONGLE_INIT = 46;
constexpr uint16_t X360_AUTHLEN_CHALLENGE = 22;

constexpr uint32_t X360_POLL_INTERVAL_MS = 100;
constexpr uint8_t X360_POLL_RETRY_LIMIT = 60;

} // namespace

XboxAuthPassthrough &XboxAuthPassthrough::instance() {
    static XboxAuthPassthrough passthrough;
    return passthrough;
}

void XboxAuthPassthrough::attach(USBHostAuthListener *listener) {
    _listener = listener;
    reset();
}

bool XboxAuthPassthrough::hostReady() const {
    return _listener != nullptr && _listener->deviceType() == USBHostAuthDeviceType::XINPUT_360;
}

bool XboxAuthPassthrough::dongleReady() const {
    return hostReady() && _has_serial;
}

void XboxAuthPassthrough::clearSession() {
    _pending_len = 0;
    _response_len = 0;
    _control_len = 0;
    _poll_due_ms = 0;
    _control_request = 0;
    _poll_count = 0;
    _pending_command = PendingCommand::NONE;
    _active_command = PendingCommand::NONE;
    _stage = Stage::IDLE;
    _response_ready = false;
    memset(_response_buffer, 0, sizeof(_response_buffer));
    memset(_control_buffer, 0, sizeof(_control_buffer));
}

void XboxAuthPassthrough::reset() {
    memset(_console_initial_auth, 0, sizeof(_console_initial_auth));
    memset(_dongle_serial, 0, sizeof(_dongle_serial));
    _has_serial = false;
    _has_init_auth = false;
    clearSession();
}

void XboxAuthPassthrough::process() {
    if (!hostReady()) {
        if (_stage != Stage::IDLE || _has_serial || _has_init_auth) {
            reset();
        }
        return;
    }

    if (_listener->busy()) {
        return;
    }

    switch (_stage) {
        case Stage::WAIT_FETCH_SERIAL:
            if (_listener->lastLength() == X360_AUTHLEN_DONGLE_SERIAL) {
                memcpy(_dongle_serial, _listener->lastBuffer(), X360_AUTHLEN_DONGLE_SERIAL);
                _has_serial = true;
            }
            _stage = Stage::IDLE;
            break;
        case Stage::WAIT_SEND_TO_DONGLE:
            _poll_due_ms = millis() + X360_POLL_INTERVAL_MS;
            _poll_count = 0;
            _stage = Stage::POLL_STATE;
            break;
        case Stage::WAIT_POLL_STATE:
            if (_listener->lastLength() >= 1 && _listener->lastBuffer()[0] == X360_STATE_COMPLETE) {
                _stage = Stage::GET_RESPONSE;
            } else if (++_poll_count >= X360_POLL_RETRY_LIMIT) {
                clearSession();
            } else {
                _poll_due_ms = millis() + X360_POLL_INTERVAL_MS;
                _stage = Stage::POLL_STATE;
            }
            break;
        case Stage::WAIT_GET_RESPONSE:
            if (_listener->lastLength() > 0 && _listener->lastLength() <= sizeof(_response_buffer)) {
                _response_len = _listener->lastLength();
                memcpy(_response_buffer, _listener->lastBuffer(), _response_len);
                _response_ready = true;
                if (_active_command == PendingCommand::INIT_AUTH) {
                    _listener->sendXInput360KeepAlive();
                }
            }
            _active_command = PendingCommand::NONE;
            _stage = Stage::IDLE;
            break;
        default:
            break;
    }

    if (_stage != Stage::IDLE && _stage != Stage::POLL_STATE && _stage != Stage::GET_RESPONSE &&
        _stage != Stage::FETCH_SERIAL) {
        return;
    }

    if (!_has_serial) {
        if (_stage == Stage::IDLE && _listener->requestXInput360Serial()) {
            _stage = Stage::WAIT_FETCH_SERIAL;
        }
        return;
    }

    switch (_stage) {
        case Stage::FETCH_SERIAL:
        case Stage::IDLE:
            if (_pending_command == PendingCommand::INIT_AUTH) {
                if (_listener->sendXInput360InitAuth(_control_buffer, _pending_len)) {
                    _active_command = _pending_command;
                    _pending_command = PendingCommand::NONE;
                    _stage = Stage::WAIT_SEND_TO_DONGLE;
                }
            } else if (_pending_command == PendingCommand::VERIFY_AUTH) {
                if (_listener->sendXInput360VerifyAuth(_control_buffer, _pending_len)) {
                    _active_command = _pending_command;
                    _pending_command = PendingCommand::NONE;
                    _stage = Stage::WAIT_SEND_TO_DONGLE;
                }
            }
            break;
        case Stage::POLL_STATE:
            if (millis() >= _poll_due_ms && _listener->requestXInput360State()) {
                _stage = Stage::WAIT_POLL_STATE;
            }
            break;
        case Stage::GET_RESPONSE:
            if (_listener->requestXInput360ChallengeResponse(_response_len)) {
                _stage = Stage::WAIT_GET_RESPONSE;
            }
            break;
        default:
            break;
    }
}

bool XboxAuthPassthrough::handleVendorControl(
    uint8_t rhport,
    uint8_t stage,
    const tusb_control_request_t *request
) {
    if (request == nullptr || request->bmRequestType_bit.type != TUSB_REQ_TYPE_VENDOR) {
        return false;
    }

    if (request->bmRequestType_bit.direction == TUSB_DIR_IN) {
        if (stage != CONTROL_STAGE_SETUP) {
            return true;
        }

        switch (request->bRequest) {
            case X360_REQUEST_GET_SERIAL:
                if (!dongleReady()) {
                    return false;
                }
                return tud_control_xfer(rhport, request, _dongle_serial, X360_AUTHLEN_DONGLE_SERIAL);
            case X360_REQUEST_RESPOND_CHALLENGE:
                if (!_response_ready) {
                    return false;
                }
                if (!tud_control_xfer(rhport, request, _response_buffer, _response_len)) {
                    return false;
                }
                _response_ready = false;
                return true;
            case X360_REQUEST_AUTH_KEEPALIVE:
                return tud_control_xfer(rhport, request, nullptr, 0);
            case X360_REQUEST_STATE: {
                uint16_t state = _response_ready ? X360_STATE_COMPLETE : X360_STATE_IN_PROGRESS;
                memcpy(_control_buffer, &state, sizeof(state));
                return tud_control_xfer(rhport, request, _control_buffer, sizeof(state));
            }
            default:
                return false;
        }
    }

    if (stage == CONTROL_STAGE_SETUP) {
        switch (request->bRequest) {
            case X360_REQUEST_INIT_AUTH:
            case X360_REQUEST_VERIFY_AUTH:
                if (request->wLength > sizeof(_control_buffer)) {
                    return false;
                }
                _control_request = request->bRequest;
                _control_len = request->wLength;
                return tud_control_xfer(rhport, request, _control_buffer, request->wLength);
            default:
                return false;
        }
    }

    if (stage != CONTROL_STAGE_DATA) {
        return true;
    }

    switch (_control_request) {
        case X360_REQUEST_INIT_AUTH:
            if (_control_len != X360_AUTHLEN_CONSOLE_INIT) {
                return false;
            }
            memcpy(_console_initial_auth, _control_buffer, _control_len);
            _has_init_auth = true;
            _pending_len = _control_len;
            _response_len = X360_AUTHLEN_DONGLE_INIT;
            _pending_command = PendingCommand::INIT_AUTH;
            _response_ready = false;
            break;
        case X360_REQUEST_VERIFY_AUTH:
            if (_control_len != X360_AUTHLEN_CHALLENGE) {
                return false;
            }
            _pending_len = _control_len;
            _response_len = X360_AUTHLEN_CHALLENGE;
            _pending_command = PendingCommand::VERIFY_AUTH;
            _response_ready = false;
            break;
        default:
            return false;
    }

    return true;
}
