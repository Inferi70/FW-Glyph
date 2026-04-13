#ifndef _USB_TINYUSB_HOST_MANAGER_HPP
#define _USB_TINYUSB_HOST_MANAGER_HPP

#include "usb/TinyUSBHostListener.hpp"

#include <cstddef>

class TinyUSBHostManager {
  public:
    static TinyUSBHostManager &instance();

    bool start();
    void shutdown();
    void process();

    bool ready() const;
    bool enabled() const;

    bool pushListener(TinyUSBHostListener *listener);

    void mount(uint8_t dev_addr, uint16_t vid, uint16_t pid);
    void unmount(uint8_t dev_addr);
    void hidMount(uint8_t dev_addr, uint8_t instance, const uint8_t *desc_report, uint16_t desc_len);
    void hidUnmount(uint8_t dev_addr, uint8_t instance);
    void hidReportReceived(uint8_t dev_addr, uint8_t instance, const uint8_t *report, uint16_t len);

  private:
    TinyUSBHostManager() = default;

    static constexpr size_t MAX_LISTENERS = 4;

    TinyUSBHostListener *_listeners[MAX_LISTENERS] = {};
    size_t _listener_count = 0;
    bool _started = false;
    bool _ready = false;
};

#endif
