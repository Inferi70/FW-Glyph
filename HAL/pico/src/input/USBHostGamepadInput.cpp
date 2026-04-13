#include "input/USBHostGamepadInput.hpp"

#include "usb/TinyUSBXInputHost.h"
#include "util/state_util.hpp"

#include <climits>
#include <cstring>

namespace {

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

void USBHostGamepadInput::unmount(uint8_t dev_addr) {
    if (dev_addr != _dev_addr) {
        return;
    }

    _active = false;
    _dev_addr = 0;
    _instance = 0;
    _type = 0;
}

void USBHostGamepadInput::xinputMount(uint8_t dev_addr, uint8_t instance, uint8_t type, uint8_t subtype) {
    (void) subtype;

    _active = true;
    _dev_addr = dev_addr;
    _instance = instance;
    _type = type;

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
