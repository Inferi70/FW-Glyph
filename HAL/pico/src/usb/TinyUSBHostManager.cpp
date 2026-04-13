#include "usb/TinyUSBHostManager.hpp"

#include "arduino/Adafruit_TinyUSB_API.h"
#include "tusb.h"

extern "C" {
bool TinyUSB_Port_InitHost(uint8_t rhport);
void TinyUSB_Port_DeinitHost(uint8_t rhport);
}

TinyUSBHostManager &TinyUSBHostManager::instance() {
    static TinyUSBHostManager manager;
    return manager;
}

bool TinyUSBHostManager::start() {
    if (_started) {
        return _ready;
    }

    _started = true;

#if CFG_TUH_ENABLED
    if (!TinyUSB_Port_InitHost(BOARD_TUH_RHPORT)) {
        return false;
    }

    tuh_init(BOARD_TUH_RHPORT);
    _ready = true;
#endif

    return _ready;
}

void TinyUSBHostManager::shutdown() {
#if CFG_TUH_ENABLED
    if (_ready) {
        TinyUSB_Port_DeinitHost(BOARD_TUH_RHPORT);
    }
#endif

    _ready = false;
    _started = false;
}

void TinyUSBHostManager::process() {
#if CFG_TUH_ENABLED
    if (_ready) {
        tuh_task();
    }
#endif
}

bool TinyUSBHostManager::ready() const {
    return _ready;
}

bool TinyUSBHostManager::enabled() const {
#if CFG_TUH_ENABLED
    return true;
#else
    return false;
#endif
}

bool TinyUSBHostManager::pushListener(TinyUSBHostListener *listener) {
    if (listener == nullptr || _listener_count >= MAX_LISTENERS) {
        return false;
    }

    _listeners[_listener_count++] = listener;
    return true;
}

void TinyUSBHostManager::mount(uint8_t dev_addr, uint16_t vid, uint16_t pid) {
    for (size_t i = 0; i < _listener_count; i++) {
        _listeners[i]->mount(dev_addr, vid, pid);
    }
}

void TinyUSBHostManager::unmount(uint8_t dev_addr) {
    for (size_t i = 0; i < _listener_count; i++) {
        _listeners[i]->unmount(dev_addr);
    }
}

void TinyUSBHostManager::hidMount(
    uint8_t dev_addr,
    uint8_t instance,
    const uint8_t *desc_report,
    uint16_t desc_len
) {
    for (size_t i = 0; i < _listener_count; i++) {
        _listeners[i]->hidMount(dev_addr, instance, desc_report, desc_len);
    }
}

void TinyUSBHostManager::hidUnmount(uint8_t dev_addr, uint8_t instance) {
    for (size_t i = 0; i < _listener_count; i++) {
        _listeners[i]->hidUnmount(dev_addr, instance);
    }
}

void TinyUSBHostManager::hidReportReceived(
    uint8_t dev_addr,
    uint8_t instance,
    const uint8_t *report,
    uint16_t len
) {
    for (size_t i = 0; i < _listener_count; i++) {
        _listeners[i]->hidReportReceived(dev_addr, instance, report, len);
    }
}

#if CFG_TUH_ENABLED
extern "C" {

void tuh_mount_cb(uint8_t dev_addr) {
    uint16_t vid = 0xFFFF;
    uint16_t pid = 0xFFFF;
    tuh_vid_pid_get(dev_addr, &vid, &pid);
    TinyUSBHostManager::instance().mount(dev_addr, vid, pid);
}

void tuh_umount_cb(uint8_t dev_addr) {
    TinyUSBHostManager::instance().unmount(dev_addr);
}

void tuh_hid_mount_cb(
    uint8_t dev_addr,
    uint8_t instance,
    const uint8_t *desc_report,
    uint16_t desc_len
) {
    TinyUSBHostManager::instance().hidMount(dev_addr, instance, desc_report, desc_len);
    tuh_hid_receive_report(dev_addr, instance);
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
    TinyUSBHostManager::instance().hidUnmount(dev_addr, instance);
}

void tuh_hid_report_received_cb(
    uint8_t dev_addr,
    uint8_t instance,
    const uint8_t *report,
    uint16_t len
) {
    TinyUSBHostManager::instance().hidReportReceived(dev_addr, instance, report, len);
    tuh_hid_receive_report(dev_addr, instance);
}

}
#endif
