#ifndef _USB_XBOX_AUTH_PASSTHROUGH_HPP
#define _USB_XBOX_AUTH_PASSTHROUGH_HPP

#include "tusb.h"
#include "usb/USBHostAuthListener.hpp"

#include <stdint.h>

class XboxAuthPassthrough {
  public:
    static XboxAuthPassthrough &instance();

    void attach(USBHostAuthListener *listener);
    void reset();
    void process();

    bool dongleReady() const;
    bool handleVendorControl(uint8_t rhport, uint8_t stage, const tusb_control_request_t *request);

  private:
    enum class PendingCommand : uint8_t {
        NONE = 0,
        INIT_AUTH,
        VERIFY_AUTH,
    };

    enum class Stage : uint8_t {
        IDLE = 0,
        FETCH_SERIAL,
        WAIT_FETCH_SERIAL,
        SEND_TO_DONGLE,
        WAIT_SEND_TO_DONGLE,
        POLL_STATE,
        WAIT_POLL_STATE,
        GET_RESPONSE,
        WAIT_GET_RESPONSE,
    };

    XboxAuthPassthrough() = default;

    bool hostReady() const;
    void clearSession();

    USBHostAuthListener *_listener = nullptr;

    uint8_t _console_initial_auth[34] = {};
    uint8_t _dongle_serial[29] = {};
    uint8_t _response_buffer[46] = {};
    uint8_t _control_buffer[64] = {};

    uint16_t _response_len = 0;
    uint16_t _pending_len = 0;
    uint16_t _control_len = 0;
    uint32_t _poll_due_ms = 0;
    uint8_t _control_request = 0;
    uint8_t _poll_count = 0;
    PendingCommand _pending_command = PendingCommand::NONE;
    PendingCommand _active_command = PendingCommand::NONE;
    Stage _stage = Stage::IDLE;
    bool _has_serial = false;
    bool _has_init_auth = false;
    bool _response_ready = false;
};

#endif
