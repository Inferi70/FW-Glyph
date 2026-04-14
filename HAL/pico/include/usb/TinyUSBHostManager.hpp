#ifndef _USB_TINYUSB_HOST_MANAGER_HPP
#define _USB_TINYUSB_HOST_MANAGER_HPP

#include "usb/TinyUSBHostListener.hpp"

#include <cstddef>
#include <functional>
#include <utility>

class TinyUSBHostManager {
  public:
    static TinyUSBHostManager &instance();

    bool start();
    void shutdown();
    void process();

    bool ready() const;
    bool enabled() const;

    bool pushListener(TinyUSBHostListener *listener);

    void mountCallback(uint8_t dev_addr, uint16_t vid, uint16_t pid);
    void unmountCallback(uint8_t dev_addr);
    void hidMountCallback(uint8_t dev_addr, uint8_t instance, const uint8_t *desc_report, uint16_t desc_len);
    void hidUnmountCallback(uint8_t dev_addr, uint8_t instance);
    void hidReportReceivedCallback(uint8_t dev_addr, uint8_t instance, const uint8_t *report, uint16_t len);
    void hidSetReportCompleteCallback(
        uint8_t dev_addr,
        uint8_t instance,
        uint8_t report_id,
        uint8_t report_type,
        uint16_t len
    );
    void hidGetReportCompleteCallback(
        uint8_t dev_addr,
        uint8_t instance,
        uint8_t report_id,
        uint8_t report_type,
        uint16_t len
    );
    void xinputMountCallback(uint8_t dev_addr, uint8_t instance, uint8_t type, uint8_t subtype);
    void xinputUnmountCallback(uint8_t dev_addr, uint8_t instance);
    void xinputReportReceivedCallback(uint8_t dev_addr, uint8_t instance, const uint8_t *report, uint16_t len);
    void xinputReportSentCallback(uint8_t dev_addr, uint8_t instance, const uint8_t *report, uint16_t len);

  private:
    TinyUSBHostManager() = default;

    static constexpr size_t MAX_LISTENERS = 4;

    template<typename Fn, typename... Args>
    void forEachListener(Fn fn, Args&&... args) {
        for (size_t i = 0; i < _listener_count; i++) {
            std::invoke(fn, _listeners[i], std::forward<Args>(args)...);
        }
    }

    TinyUSBHostListener *_listeners[MAX_LISTENERS] = {};
    size_t _listener_count = 0;
    bool _started = false;
    bool _ready = false;
};

#endif
