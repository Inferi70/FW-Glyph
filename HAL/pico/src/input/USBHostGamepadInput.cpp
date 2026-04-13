#include "input/USBHostGamepadInput.hpp"

#include "usb/TinyUSBXInputHost.h"
#include "util/state_util.hpp"

#include <climits>
#include <cstring>

namespace {

constexpr uint16_t SONY_VENDOR_ID = 0x054C;
constexpr uint16_t DS4_PRODUCT_ID = 0x09CC;
constexpr uint16_t DS4_ORG_PRODUCT_ID = 0x05C4;
constexpr uint16_t DUALSENSE_PRODUCT_ID = 0x0CE6;

constexpr uint8_t XBOX_MASK_UP = 0x01;
constexpr uint8_t XBOX_MASK_DOWN = 0x02;
constexpr uint8_t XBOX_MASK_LEFT = 0x04;
constexpr uint8_t XBOX_MASK_RIGHT = 0x08;
constexpr uint8_t XBOX_MASK_START = 0x10;
constexpr uint8_t XBOX_MASK_BACK = 0x20;
constexpr uint8_t XBOX_MASK_LS = 0x40;
constexpr uint8_t XBOX_MASK_RS = 0x80;

constexpr uint8_t XBOX_MASK_LB = 0x01;
constexpr uint8_t XBOX_MASK_RB = 0x02;
constexpr uint8_t XBOX_MASK_HOME = 0x04;
constexpr uint8_t XBOX_MASK_A = 0x10;
constexpr uint8_t XBOX_MASK_B = 0x20;
constexpr uint8_t XBOX_MASK_X = 0x40;
constexpr uint8_t XBOX_MASK_Y = 0x80;

constexpr uint8_t TRIGGER_THRESHOLD = 32;
constexpr int16_t STICK_DIGITAL_THRESHOLD = 8192;

constexpr uint8_t PS4_HAT_UP = 0x00;
constexpr uint8_t PS4_HAT_UPRIGHT = 0x01;
constexpr uint8_t PS4_HAT_RIGHT = 0x02;
constexpr uint8_t PS4_HAT_DOWNRIGHT = 0x03;
constexpr uint8_t PS4_HAT_DOWN = 0x04;
constexpr uint8_t PS4_HAT_DOWNLEFT = 0x05;
constexpr uint8_t PS4_HAT_LEFT = 0x06;
constexpr uint8_t PS4_HAT_UPLEFT = 0x07;

typedef struct __attribute__((packed)) {
    uint8_t report_id;
    uint8_t report_size;
    uint8_t buttons1;
    uint8_t buttons2;
    uint8_t lt;
    uint8_t rt;
    int16_t lx;
    int16_t ly;
    int16_t rx;
    int16_t ry;
    uint8_t reserved[6];
} xinput_host_report_t;

typedef struct __attribute__((packed)) {
    uint8_t report_id;
    uint8_t left_stick_x;
    uint8_t left_stick_y;
    uint8_t right_stick_x;
    uint8_t right_stick_y;
    uint8_t dpad : 4;
    uint16_t button_west : 1;
    uint16_t button_south : 1;
    uint16_t button_east : 1;
    uint16_t button_north : 1;
    uint16_t button_l1 : 1;
    uint16_t button_r1 : 1;
    uint16_t button_l2 : 1;
    uint16_t button_r2 : 1;
    uint16_t button_select : 1;
    uint16_t button_start : 1;
    uint16_t button_l3 : 1;
    uint16_t button_r3 : 1;
    uint16_t button_home : 1;
    uint16_t button_touchpad : 1;
    uint8_t report_counter : 6;
    uint8_t left_trigger;
    uint8_t right_trigger;
} ds4_host_report_t;

typedef struct __attribute__((packed)) {
    uint8_t report_id;
    uint8_t left_stick_x;
    uint8_t left_stick_y;
    uint8_t right_stick_x;
    uint8_t right_stick_y;
    uint8_t left_trigger;
    uint8_t right_trigger;
    uint8_t report_counter;
    uint8_t dpad : 4;
    uint16_t button_west : 1;
    uint16_t button_south : 1;
    uint16_t button_east : 1;
    uint16_t button_north : 1;
    uint16_t button_l1 : 1;
    uint16_t button_r1 : 1;
    uint16_t button_l2 : 1;
    uint16_t button_r2 : 1;
    uint16_t button_select : 1;
    uint16_t button_start : 1;
    uint16_t button_l3 : 1;
    uint16_t button_r3 : 1;
    uint16_t button_home : 1;
    uint16_t button_touchpad : 1;
    uint16_t button_mic_mute : 1;
    uint8_t misc_data[54];
} dualsense_host_report_t;

uint8_t axis_to_uint8(int16_t value, bool invert = false) {
    uint16_t shifted = static_cast<uint16_t>(value - INT16_MIN);
    uint8_t axis = static_cast<uint8_t>(shifted >> 8);
    return invert ? static_cast<uint8_t>(~axis) : axis;
}

} // namespace

InputScanSpeed USBHostGamepadInput::ScanSpeed() {
    return InputScanSpeed::FAST;
}

void USBHostGamepadInput::UpdateInputs(InputState &inputs) {
    if (_active) {
        clearMappedInputs(inputs);
        applyXInputState(inputs);
        _applied_last_update = true;
        return;
    }

    if (_applied_last_update) {
        clearMappedInputs(inputs);
        _applied_last_update = false;
    }
}

void USBHostGamepadInput::mount(uint8_t dev_addr, uint16_t vid, uint16_t pid) {
    _last_vid = 0;
    _last_pid = 0;

    if (!_active || _dev_addr == dev_addr) {
        _last_vid = vid;
        _last_pid = pid;
    }
}

void USBHostGamepadInput::unmount(uint8_t dev_addr) {
    if (dev_addr != _dev_addr) {
        return;
    }

    _active = false;
    _source = SourceType::NONE;
    _dev_addr = 0;
    _instance = 0;
    _type = 0;
    _last_vid = 0;
    _last_pid = 0;
}

void USBHostGamepadInput::hidMount(
    uint8_t dev_addr,
    uint8_t instance,
    const uint8_t *desc_report,
    uint16_t desc_len
) {
    (void) desc_report;
    (void) desc_len;

    if (_active) {
        return;
    }

    if (_last_vid != SONY_VENDOR_ID ||
        (_last_pid != DS4_PRODUCT_ID && _last_pid != DS4_ORG_PRODUCT_ID && _last_pid != DUALSENSE_PRODUCT_ID)) {
        return;
    }

    _active = true;
    _source = (_last_pid == DUALSENSE_PRODUCT_ID) ? SourceType::DUALSENSE_HID : SourceType::DS4_HID;
    _dev_addr = dev_addr;
    _instance = instance;
    _type = 0;
    resetState();
}

void USBHostGamepadInput::hidUnmount(uint8_t dev_addr, uint8_t instance) {
    if (!_active || (_source != SourceType::DS4_HID && _source != SourceType::DUALSENSE_HID) ||
        dev_addr != _dev_addr || instance != _instance) {
        return;
    }

    _active = false;
    _source = SourceType::NONE;
    _dev_addr = 0;
    _instance = 0;
}

void USBHostGamepadInput::hidReportReceived(
    uint8_t dev_addr,
    uint8_t instance,
    const uint8_t *report,
    uint16_t len
) {
    if (!_active || (_source != SourceType::DS4_HID && _source != SourceType::DUALSENSE_HID) ||
        dev_addr != _dev_addr || instance != _instance) {
        return;
    }

    if (_source == SourceType::DUALSENSE_HID) {
        applyDualSenseReport(report, len);
    } else {
        applyDs4Report(report, len);
    }
}

void USBHostGamepadInput::xinputMount(uint8_t dev_addr, uint8_t instance, uint8_t type, uint8_t subtype) {
    (void) subtype;

    _active = true;
    _source = SourceType::XINPUT;
    _dev_addr = dev_addr;
    _instance = instance;
    _type = type;
    resetState();

    #if CFG_TUH_ENABLED && CFG_TUH_XINPUT
    if (_type == XINPUT_HOST_XBOX360 || _type == XINPUT_HOST_XBOXONE) {
        tuh_xinput_receive_report(dev_addr, instance);
    }
    #endif
}

void USBHostGamepadInput::xinputUnmount(uint8_t dev_addr, uint8_t instance) {
    if (dev_addr != _dev_addr || instance != _instance) {
        return;
    }

    _active = false;
    _source = SourceType::NONE;
    _dev_addr = 0;
    _instance = 0;
    _type = 0;
}

void USBHostGamepadInput::xinputReportReceived(
    uint8_t dev_addr,
    uint8_t instance,
    const uint8_t *report,
    uint16_t len
) {
    if (!_active || dev_addr != _dev_addr || instance != _instance) {
        return;
    }

    if (_source != SourceType::XINPUT) {
        return;
    }

    if (_type != XINPUT_HOST_XBOX360 || len < sizeof(xinput_host_report_t)) {
        #if CFG_TUH_ENABLED && CFG_TUH_XINPUT
        tuh_xinput_receive_report(dev_addr, instance);
        #endif
        return;
    }

    xinput_host_report_t xinput_report = {};
    memcpy(&xinput_report, report, sizeof(xinput_report));

    _dpad_up = xinput_report.buttons1 & XBOX_MASK_UP;
    _dpad_down = xinput_report.buttons1 & XBOX_MASK_DOWN;
    _dpad_left = xinput_report.buttons1 & XBOX_MASK_LEFT;
    _dpad_right = xinput_report.buttons1 & XBOX_MASK_RIGHT;
    _start = xinput_report.buttons1 & XBOX_MASK_START;
    _back = xinput_report.buttons1 & XBOX_MASK_BACK;
    _ls = xinput_report.buttons1 & XBOX_MASK_LS;
    _rs = xinput_report.buttons1 & XBOX_MASK_RS;
    _lb = xinput_report.buttons2 & XBOX_MASK_LB;
    _rb = xinput_report.buttons2 & XBOX_MASK_RB;
    _home = xinput_report.buttons2 & XBOX_MASK_HOME;
    _a = xinput_report.buttons2 & XBOX_MASK_A;
    _b = xinput_report.buttons2 & XBOX_MASK_B;
    _x = xinput_report.buttons2 & XBOX_MASK_X;
    _y = xinput_report.buttons2 & XBOX_MASK_Y;
    _lt = xinput_report.lt;
    _rt = xinput_report.rt;
    _lx = axis_to_uint8(xinput_report.lx);
    _ly = axis_to_uint8(xinput_report.ly, true);
    _rx = axis_to_uint8(xinput_report.rx);
    _ry = axis_to_uint8(xinput_report.ry, true);

    #if CFG_TUH_ENABLED && CFG_TUH_XINPUT
    tuh_xinput_receive_report(dev_addr, instance);
    #endif
}

void USBHostGamepadInput::resetState() {
    _dpad_up = false;
    _dpad_down = false;
    _dpad_left = false;
    _dpad_right = false;
    _start = false;
    _back = false;
    _ls = false;
    _rs = false;
    _lb = false;
    _rb = false;
    _home = false;
    _a = false;
    _b = false;
    _x = false;
    _y = false;
    _lt = 0;
    _rt = 0;
    _lx = 128;
    _ly = 128;
    _rx = 128;
    _ry = 128;
}

void USBHostGamepadInput::setDpadFromHat(uint8_t hat) {
    _dpad_up = false;
    _dpad_down = false;
    _dpad_left = false;
    _dpad_right = false;

    switch (hat) {
        case PS4_HAT_UP:
            _dpad_up = true;
            break;
        case PS4_HAT_UPRIGHT:
            _dpad_up = true;
            _dpad_right = true;
            break;
        case PS4_HAT_RIGHT:
            _dpad_right = true;
            break;
        case PS4_HAT_DOWNRIGHT:
            _dpad_down = true;
            _dpad_right = true;
            break;
        case PS4_HAT_DOWN:
            _dpad_down = true;
            break;
        case PS4_HAT_DOWNLEFT:
            _dpad_down = true;
            _dpad_left = true;
            break;
        case PS4_HAT_LEFT:
            _dpad_left = true;
            break;
        case PS4_HAT_UPLEFT:
            _dpad_up = true;
            _dpad_left = true;
            break;
        default:
            break;
    }
}

void USBHostGamepadInput::applyDs4Report(const uint8_t *report, uint16_t len) {
    if (len < sizeof(ds4_host_report_t)) {
        return;
    }

    ds4_host_report_t ds4_report = {};
    memcpy(&ds4_report, report, sizeof(ds4_report));

    if (ds4_report.report_id != 0x01) {
        return;
    }

    setDpadFromHat(ds4_report.dpad);
    _start = ds4_report.button_start;
    _back = ds4_report.button_select;
    _ls = ds4_report.button_l3;
    _rs = ds4_report.button_r3;
    _lb = ds4_report.button_l1;
    _rb = ds4_report.button_r1;
    _home = ds4_report.button_home;
    _a = ds4_report.button_south;
    _b = ds4_report.button_east;
    _x = ds4_report.button_west;
    _y = ds4_report.button_north;
    _lt = ds4_report.left_trigger;
    _rt = ds4_report.right_trigger;
    _lx = ds4_report.left_stick_x;
    _ly = static_cast<uint8_t>(~ds4_report.left_stick_y);
    _rx = ds4_report.right_stick_x;
    _ry = static_cast<uint8_t>(~ds4_report.right_stick_y);
}

void USBHostGamepadInput::applyDualSenseReport(const uint8_t *report, uint16_t len) {
    if (len < sizeof(dualsense_host_report_t)) {
        return;
    }

    dualsense_host_report_t dualsense_report = {};
    memcpy(&dualsense_report, report, sizeof(dualsense_report));

    if (dualsense_report.report_id != 0x01) {
        return;
    }

    setDpadFromHat(dualsense_report.dpad);
    _start = dualsense_report.button_start;
    _back = dualsense_report.button_select;
    _ls = dualsense_report.button_l3;
    _rs = dualsense_report.button_r3;
    _lb = dualsense_report.button_l1;
    _rb = dualsense_report.button_r1;
    _home = dualsense_report.button_home;
    _a = dualsense_report.button_south;
    _b = dualsense_report.button_east;
    _x = dualsense_report.button_west;
    _y = dualsense_report.button_north;
    _lt = dualsense_report.left_trigger;
    _rt = dualsense_report.right_trigger;
    _lx = dualsense_report.left_stick_x;
    _ly = static_cast<uint8_t>(~dualsense_report.left_stick_y);
    _rx = dualsense_report.right_stick_x;
    _ry = static_cast<uint8_t>(~dualsense_report.right_stick_y);
}

void USBHostGamepadInput::clearMappedInputs(InputState &inputs) {
    set_button(inputs.buttons, BTN_LF3, false);
    set_button(inputs.buttons, BTN_LF1, false);
    set_button(inputs.buttons, BTN_LF2, false);
    set_button(inputs.buttons, BTN_LT1, false);
    set_button(inputs.buttons, BTN_RF1, false);
    set_button(inputs.buttons, BTN_RF2, false);
    set_button(inputs.buttons, BTN_RF3, false);
    set_button(inputs.buttons, BTN_RF4, false);
    set_button(inputs.buttons, BTN_RF5, false);
    set_button(inputs.buttons, BTN_RF6, false);
    set_button(inputs.buttons, BTN_RF7, false);
    set_button(inputs.buttons, BTN_RF8, false);
    set_button(inputs.buttons, BTN_LT2, false);
    set_button(inputs.buttons, BTN_RT1, false);
    set_button(inputs.buttons, BTN_RT2, false);
    set_button(inputs.buttons, BTN_RT3, false);
    set_button(inputs.buttons, BTN_RT4, false);
    set_button(inputs.buttons, BTN_RT5, false);
    set_button(inputs.buttons, BTN_MB4, false);
    set_button(inputs.buttons, BTN_MB5, false);
    set_button(inputs.buttons, BTN_MB6, false);
    set_button(inputs.buttons, BTN_MB7, false);

    inputs.nunchuk_connected = false;
    inputs.nunchuk_c = false;
    inputs.nunchuk_z = false;
}

void USBHostGamepadInput::applyXInputState(InputState &inputs) {
    set_button(inputs.buttons, BTN_LF3, _dpad_left);
    set_button(inputs.buttons, BTN_LF1, _dpad_right);
    set_button(inputs.buttons, BTN_LF2, _dpad_down);
    set_button(inputs.buttons, BTN_LT1, _dpad_up);

    set_button(inputs.buttons, BTN_RF1, _a);
    set_button(inputs.buttons, BTN_RF2, _b);
    set_button(inputs.buttons, BTN_RF5, _x);
    set_button(inputs.buttons, BTN_RF6, _y);
    set_button(inputs.buttons, BTN_RF8, _lb);
    set_button(inputs.buttons, BTN_RF7, _rb);
    set_button(inputs.buttons, BTN_RF4, _lt > TRIGGER_THRESHOLD);
    set_button(inputs.buttons, BTN_RF3, _rt > TRIGGER_THRESHOLD);

    set_button(inputs.buttons, BTN_MB7, _start);
    set_button(inputs.buttons, BTN_MB6, _back);
    set_button(inputs.buttons, BTN_MB5, _home);
    set_button(inputs.buttons, BTN_MB4, false);
    set_button(inputs.buttons, BTN_LT2, _ls);
    set_button(inputs.buttons, BTN_RT1, _rs);

    set_button(inputs.buttons, BTN_RT3, _rx < (128 - (STICK_DIGITAL_THRESHOLD >> 8)));
    set_button(inputs.buttons, BTN_RT5, _rx > (128 + (STICK_DIGITAL_THRESHOLD >> 8)));
    set_button(inputs.buttons, BTN_RT2, _ry > (128 + (STICK_DIGITAL_THRESHOLD >> 8)));
    set_button(inputs.buttons, BTN_RT4, _ry < (128 - (STICK_DIGITAL_THRESHOLD >> 8)));

    inputs.nunchuk_connected = true;
    inputs.nunchuk_x = static_cast<int8_t>(_lx);
    inputs.nunchuk_y = static_cast<int8_t>(_ly);
}
