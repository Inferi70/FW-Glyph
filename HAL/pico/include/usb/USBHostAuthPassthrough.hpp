#ifndef _USB_USB_HOST_AUTH_PASSTHROUGH_HPP
#define _USB_USB_HOST_AUTH_PASSTHROUGH_HPP

#include "usb/USBHostAuthListener.hpp"

#include <stdint.h>

class USBHostAuthPassthrough {
  public:
    static USBHostAuthPassthrough &instance();

    void attach(USBHostAuthListener *listener);
    void reset();
    void process();

    bool ready() const;
    USBHostAuthDeviceType deviceType() const;

    bool submitGetReport(uint8_t report_id, const uint8_t *payload, uint16_t len);
    bool submitSetReport(uint8_t report_id, const uint8_t *payload, uint16_t len);

    bool requestInFlight() const;
    bool responseReady() const;
    bool responseWasGet() const;
    uint8_t responseReportId() const;
    uint16_t responseLength() const;
    bool copyResponse(uint8_t *dst, uint16_t max_len, uint16_t *actual_len = nullptr) const;
    void clearResponse();

  private:
    enum class RequestType : uint8_t {
        NONE = 0,
        GET,
        SET,
    };

    USBHostAuthPassthrough() = default;

    bool dispatchPS4Get(uint8_t report_id, const uint8_t *payload, uint16_t len);
    bool dispatchPS4Set(uint8_t report_id, const uint8_t *payload, uint16_t len);
    bool dispatchP5Get(uint8_t report_id, const uint8_t *payload, uint16_t len);
    bool dispatchP5Set(uint8_t report_id, const uint8_t *payload, uint16_t len);

    USBHostAuthListener *_listener = nullptr;
    uint8_t _request_report_id = 0;
    uint16_t _request_len = 0;
    uint8_t _request_buffer[64] = {};
    RequestType _request_type = RequestType::NONE;
    bool _request_pending = false;
    bool _request_in_flight = false;

    bool _response_ready = false;
    bool _response_was_get = false;
    uint8_t _response_report_id = 0;
    uint16_t _response_len = 0;
    uint8_t _response_buffer[64] = {};
};

#endif
