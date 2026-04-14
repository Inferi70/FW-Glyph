#include "usb/TinyUSBHostManager.hpp"

#include "arduino/Adafruit_TinyUSB_API.h"
#include "tusb.h"

extern "C" {
bool TinyUSB_Port_InitHost(uint8_t rhport);
void TinyUSB_Port_DeinitHost(uint8_t rhport);
}

#if CFG_TUH_ENABLED && CFG_TUH_XINPUT
#include "third_party/tinyusb_gp2040/src/host/usbh_pvt.h"
#include "usb/TinyUSBXInputHost.h"

static bool xinputh_driver_init() {
    return xinputh_init();
}

static usbh_class_driver_t driver_host[] = {{
#if CFG_TUSB_DEBUG >= 2
    .name = "XInput_Host_HID",
#endif
    .init = xinputh_driver_init,
    .open = xinputh_open,
    .set_config = xinputh_set_config,
    .xfer_cb = xinputh_xfer_cb,
    .close = xinputh_close,
}};

extern "C" usbh_class_driver_t const *usbh_app_driver_get_cb(uint8_t *driver_count) {
    *driver_count = 1;
    return driver_host;
}
#endif

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

    listener->setup();
    _listeners[_listener_count++] = listener;
    return true;
}

void TinyUSBHostManager::mountCallback(uint8_t dev_addr, uint16_t vid, uint16_t pid) {
    forEachListener(&TinyUSBHostListener::mount, dev_addr, vid, pid);
}

void TinyUSBHostManager::unmountCallback(uint8_t dev_addr) {
    forEachListener(&TinyUSBHostListener::unmount, dev_addr);
}

void TinyUSBHostManager::hidMountCallback(
    uint8_t dev_addr,
    uint8_t instance,
    const uint8_t *desc_report,
    uint16_t desc_len
) {
    forEachListener(&TinyUSBHostListener::hidMount, dev_addr, instance, desc_report, desc_len);
}

void TinyUSBHostManager::hidUnmountCallback(uint8_t dev_addr, uint8_t instance) {
    forEachListener(&TinyUSBHostListener::hidUnmount, dev_addr, instance);
}

void TinyUSBHostManager::hidReportReceivedCallback(
    uint8_t dev_addr,
    uint8_t instance,
    const uint8_t *report,
    uint16_t len
) {
    forEachListener(&TinyUSBHostListener::hidReportReceived, dev_addr, instance, report, len);
}

void TinyUSBHostManager::hidSetReportCompleteCallback(
    uint8_t dev_addr,
    uint8_t instance,
    uint8_t report_id,
    uint8_t report_type,
    uint16_t len
) {
    forEachListener(
        &TinyUSBHostListener::hidSetReportComplete,
        dev_addr,
        instance,
        report_id,
        report_type,
        len
    );
}

void TinyUSBHostManager::hidGetReportCompleteCallback(
    uint8_t dev_addr,
    uint8_t instance,
    uint8_t report_id,
    uint8_t report_type,
    uint16_t len
) {
    forEachListener(
        &TinyUSBHostListener::hidGetReportComplete,
        dev_addr,
        instance,
        report_id,
        report_type,
        len
    );
}

void TinyUSBHostManager::xinputMountCallback(uint8_t dev_addr, uint8_t instance, uint8_t type, uint8_t subtype) {
    forEachListener(&TinyUSBHostListener::xinputMount, dev_addr, instance, type, subtype);
}

void TinyUSBHostManager::xinputUnmountCallback(uint8_t dev_addr, uint8_t instance) {
    forEachListener(&TinyUSBHostListener::xinputUnmount, dev_addr, instance);
}

void TinyUSBHostManager::xinputReportReceivedCallback(
    uint8_t dev_addr,
    uint8_t instance,
    const uint8_t *report,
    uint16_t len
) {
    forEachListener(&TinyUSBHostListener::xinputReportReceived, dev_addr, instance, report, len);
}

void TinyUSBHostManager::xinputReportSentCallback(
    uint8_t dev_addr,
    uint8_t instance,
    const uint8_t *report,
    uint16_t len
) {
    forEachListener(&TinyUSBHostListener::xinputReportSent, dev_addr, instance, report, len);
}

#if CFG_TUH_ENABLED
extern "C" {

void tuh_mount_cb(uint8_t dev_addr) {
    uint16_t vid = 0xFFFF;
    uint16_t pid = 0xFFFF;
    tuh_vid_pid_get(dev_addr, &vid, &pid);
    TinyUSBHostManager::instance().mountCallback(dev_addr, vid, pid);
}

void tuh_umount_cb(uint8_t dev_addr) {
    TinyUSBHostManager::instance().unmountCallback(dev_addr);
}

void tuh_hid_mount_cb(
    uint8_t dev_addr,
    uint8_t instance,
    const uint8_t *desc_report,
    uint16_t desc_len
) {
    TinyUSBHostManager::instance().hidMountCallback(dev_addr, instance, desc_report, desc_len);
    tuh_hid_receive_report(dev_addr, instance);
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
    TinyUSBHostManager::instance().hidUnmountCallback(dev_addr, instance);
}

void tuh_hid_report_received_cb(
    uint8_t dev_addr,
    uint8_t instance,
    const uint8_t *report,
    uint16_t len
) {
    TinyUSBHostManager::instance().hidReportReceivedCallback(dev_addr, instance, report, len);
    tuh_hid_receive_report(dev_addr, instance);
}

void tuh_hid_set_report_complete_cb(
    uint8_t dev_addr,
    uint8_t instance,
    uint8_t report_id,
    uint8_t report_type,
    uint16_t len
) {
    if (len != 0) {
        TinyUSBHostManager::instance().hidSetReportCompleteCallback(
            dev_addr,
            instance,
            report_id,
            report_type,
            len
        );
    }
}

void tuh_hid_get_report_complete_cb(
    uint8_t dev_addr,
    uint8_t instance,
    uint8_t report_id,
    uint8_t report_type,
    uint16_t len
) {
    if (len != 0) {
        TinyUSBHostManager::instance().hidGetReportCompleteCallback(
            dev_addr,
            instance,
            report_id,
            report_type,
            len
        );
    }
}

void tuh_xinput_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t type, uint8_t subtype) {
    TinyUSBHostManager::instance().xinputMountCallback(dev_addr, instance, type, subtype);
}

void tuh_xinput_umount_cb(uint8_t dev_addr, uint8_t instance) {
    TinyUSBHostManager::instance().xinputUnmountCallback(dev_addr, instance);
}

void tuh_xinput_report_received_cb(
    uint8_t dev_addr,
    uint8_t instance,
    const uint8_t *report,
    uint16_t len
) {
    TinyUSBHostManager::instance().xinputReportReceivedCallback(dev_addr, instance, report, len);
}

void tuh_xinput_report_sent_cb(
    uint8_t dev_addr,
    uint8_t instance,
    const uint8_t *report,
    uint16_t len
) {
    TinyUSBHostManager::instance().xinputReportSentCallback(dev_addr, instance, report, len);
}

}
#endif
