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
constexpr uint16_t NINTENDO_VENDOR_ID = 0x057E;
constexpr uint16_t SWITCH_PRO_PRODUCT_ID = 0x2009;

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

constexpr uint8_t SWITCH_REPORT_OUTPUT_30 = 0x30;
constexpr uint8_t SWITCH_REPORT_CONFIGURATION = 0x80;
constexpr uint8_t SWITCH_REPORT_USB_INPUT_81 = 0x81;
constexpr uint8_t SWITCH_SUBCMD_IDENTIFY = 0x01;
constexpr uint8_t SWITCH_SUBCMD_HANDSHAKE = 0x02;
constexpr uint8_t SWITCH_SUBCMD_DISABLE_USB_TIMEOUT = 0x04;

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

typedef struct __attribute__((packed)) {
    uint8_t data[3];

    uint16_t getX() const {
        return static_cast<uint16_t>(data[0]) | ((data[1] & 0x0F) << 8);
    }

    uint16_t getY() const {
        return static_cast<uint16_t>(data[1] >> 4) | (static_cast<uint16_t>(data[2]) << 4);
    }
} switch_analog_t;

typedef struct __attribute__((packed)) {
    uint8_t connection_info : 4;
    uint8_t battery_level : 4;

    uint8_t button_y : 1;
    uint8_t button_x : 1;
    uint8_t button_b : 1;
    uint8_t button_a : 1;
    uint8_t button_right_sr : 1;
    uint8_t button_right_sl : 1;
    uint8_t button_r : 1;
    uint8_t button_zr : 1;

    uint8_t button_minus : 1;
    uint8_t button_plus : 1;
    uint8_t button_thumb_r : 1;
    uint8_t button_thumb_l : 1;
    uint8_t button_home : 1;
    uint8_t button_capture : 1;
    uint8_t dummy : 1;
    uint8_t charging_grip : 1;

    uint8_t dpad_down : 1;
    uint8_t dpad_up : 1;
    uint8_t dpad_right : 1;
    uint8_t dpad_left : 1;
    uint8_t button_left_sl : 1;
    uint8_t button_left_sr : 1;
    uint8_t button_l : 1;
    uint8_t button_zl : 1;

    switch_analog_t left_stick;
    switch_analog_t right_stick;
} switch_input_report_t;

typedef struct __attribute__((packed)) {
    uint8_t report_id;
    uint8_t timestamp;
    switch_input_report_t inputs;
    uint8_t rumble_report;
    uint8_t imu_data[36];
    uint8_t padding[15];
} switch_pro_report_t;

typedef struct __attribute__((packed)) {
    uint8_t command;
    uint8_t counter;
    uint8_t rumble_l[4];
    uint8_t rumble_r[4];
    uint8_t subcommand;
    uint8_t subcommand_args[3];
} switch_pro_host_report_t;

constexpr uint8_t SWITCH_INIT_REPORT[10] = { SWITCH_REPORT_CONFIGURATION, SWITCH_SUBCMD_IDENTIFY };

struct HostedGlyphMapping {
    Button dpad_left;
    Button dpad_right;
    Button dpad_down;
    Button dpad_up;
    Button face_a;
    Button face_b;
    Button face_x;
    Button face_y;
    Button shoulder_l;
    Button shoulder_r;
    Button trigger_l;
    Button trigger_r;
    Button start;
    Button back;
    Button home;
    Button mod_x;
    Button mod_y;
    Button right_stick_click;
};

// Default hosted-controller layout into FW-Glyph raw inputs.
// Change these targets if you want a different hosted default without touching parser code.
constexpr HostedGlyphMapping DEFAULT_HOSTED_GLYPH_MAPPING = {
    .dpad_left = BTN_LF3,
    .dpad_right = BTN_LF1,
    .dpad_down = BTN_LF2,
    .dpad_up = BTN_RF4,
    .face_a = BTN_RF1,
    .face_b = BTN_RF2,
    .face_x = BTN_RF5,
    .face_y = BTN_RF6,
    .shoulder_l = BTN_RF8,
    .shoulder_r = BTN_RF7,
    .trigger_l = BTN_RF4,
    .trigger_r = BTN_RF3,
    .start = BTN_MB7,
    .back = BTN_MB6,
    .home = BTN_MB5,
    .mod_x = BTN_LT1,
    .mod_y = BTN_LT2,
    .right_stick_click = BTN_RT1,
};

uint8_t axis_to_uint8(int16_t value, bool invert = false) {
    uint16_t shifted = static_cast<uint16_t>(value - INT16_MIN);
    uint8_t axis = static_cast<uint8_t>(shifted >> 8);
    return invert ? static_cast<uint8_t>(~axis) : axis;
}

} // namespace

InputScanSpeed USBHostGamepadInput::ScanSpeed() {
    return InputScanSpeed::FAST;
}

void USBHostGamepadInput::setup() {
    _active = false;
    _applied_last_update = false;
    _source = SourceType::NONE;
    _dev_addr = 0;
    _instance = 0;
    _type = 0;
    _last_vid = 0;
    _last_pid = 0;
    resetState();
}

void USBHostGamepadInput::UpdateInputs(InputState &inputs) {
    if (_active) {
        clearMappedInputs(inputs);
        applyNormalizedState(inputs);
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
        if (_last_vid != NINTENDO_VENDOR_ID || _last_pid != SWITCH_PRO_PRODUCT_ID) {
            return;
        }
    }

    _active = true;
    if (_last_vid == NINTENDO_VENDOR_ID && _last_pid == SWITCH_PRO_PRODUCT_ID) {
        _source = SourceType::SWITCH_PRO_HID;
    } else {
        _source = (_last_pid == DUALSENSE_PRODUCT_ID) ? SourceType::DUALSENSE_HID : SourceType::DS4_HID;
    }
    _dev_addr = dev_addr;
    _instance = instance;
    _type = 0;
    resetState();

    if (_source == SourceType::SWITCH_PRO_HID) {
        startSwitchProInit();
    }
}

void USBHostGamepadInput::hidUnmount(uint8_t dev_addr, uint8_t instance) {
    if (!_active || (_source != SourceType::DS4_HID && _source != SourceType::DUALSENSE_HID &&
                     _source != SourceType::SWITCH_PRO_HID) ||
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
    if (!_active || (_source != SourceType::DS4_HID && _source != SourceType::DUALSENSE_HID &&
                     _source != SourceType::SWITCH_PRO_HID) ||
        dev_addr != _dev_addr || instance != _instance) {
        return;
    }

    if (_source == SourceType::SWITCH_PRO_HID) {
        applySwitchProReport(report, len);
    } else if (_source == SourceType::DUALSENSE_HID) {
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

    setButtons(HOST_BTN_DPAD_UP, xinput_report.buttons1 & XBOX_MASK_UP);
    setButtons(HOST_BTN_DPAD_DOWN, xinput_report.buttons1 & XBOX_MASK_DOWN);
    setButtons(HOST_BTN_DPAD_LEFT, xinput_report.buttons1 & XBOX_MASK_LEFT);
    setButtons(HOST_BTN_DPAD_RIGHT, xinput_report.buttons1 & XBOX_MASK_RIGHT);
    setButtons(HOST_BTN_START, xinput_report.buttons1 & XBOX_MASK_START);
    setButtons(HOST_BTN_BACK, xinput_report.buttons1 & XBOX_MASK_BACK);
    setButtons(HOST_BTN_LS, xinput_report.buttons1 & XBOX_MASK_LS);
    setButtons(HOST_BTN_RS, xinput_report.buttons1 & XBOX_MASK_RS);
    setButtons(HOST_BTN_LB, xinput_report.buttons2 & XBOX_MASK_LB);
    setButtons(HOST_BTN_RB, xinput_report.buttons2 & XBOX_MASK_RB);
    setButtons(HOST_BTN_HOME, xinput_report.buttons2 & XBOX_MASK_HOME);
    setButtons(HOST_BTN_A, xinput_report.buttons2 & XBOX_MASK_A);
    setButtons(HOST_BTN_B, xinput_report.buttons2 & XBOX_MASK_B);
    setButtons(HOST_BTN_X, xinput_report.buttons2 & XBOX_MASK_X);
    setButtons(HOST_BTN_Y, xinput_report.buttons2 & XBOX_MASK_Y);
    _state.lt = xinput_report.lt;
    _state.rt = xinput_report.rt;
    _state.lx = axis_to_uint8(xinput_report.lx);
    _state.ly = axis_to_uint8(xinput_report.ly, true);
    _state.rx = axis_to_uint8(xinput_report.rx);
    _state.ry = axis_to_uint8(xinput_report.ry, true);

    #if CFG_TUH_ENABLED && CFG_TUH_XINPUT
    tuh_xinput_receive_report(dev_addr, instance);
    #endif
}

void USBHostGamepadInput::resetState() {
    _switch_pro_ready = false;
    _switch_report_counter = 0;
    _state = {};
}

void USBHostGamepadInput::setDpadFromHat(uint8_t hat) {
    clearButtons(HOST_BTN_DPAD_UP | HOST_BTN_DPAD_DOWN | HOST_BTN_DPAD_LEFT | HOST_BTN_DPAD_RIGHT);

    switch (hat) {
        case PS4_HAT_UP:
            setButtons(HOST_BTN_DPAD_UP, true);
            break;
        case PS4_HAT_UPRIGHT:
            setButtons(HOST_BTN_DPAD_UP | HOST_BTN_DPAD_RIGHT, true);
            break;
        case PS4_HAT_RIGHT:
            setButtons(HOST_BTN_DPAD_RIGHT, true);
            break;
        case PS4_HAT_DOWNRIGHT:
            setButtons(HOST_BTN_DPAD_DOWN | HOST_BTN_DPAD_RIGHT, true);
            break;
        case PS4_HAT_DOWN:
            setButtons(HOST_BTN_DPAD_DOWN, true);
            break;
        case PS4_HAT_DOWNLEFT:
            setButtons(HOST_BTN_DPAD_DOWN | HOST_BTN_DPAD_LEFT, true);
            break;
        case PS4_HAT_LEFT:
            setButtons(HOST_BTN_DPAD_LEFT, true);
            break;
        case PS4_HAT_UPLEFT:
            setButtons(HOST_BTN_DPAD_UP | HOST_BTN_DPAD_LEFT, true);
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
    setButtons(HOST_BTN_START, ds4_report.button_start);
    setButtons(HOST_BTN_BACK, ds4_report.button_select);
    setButtons(HOST_BTN_LS, ds4_report.button_l3);
    setButtons(HOST_BTN_RS, ds4_report.button_r3);
    setButtons(HOST_BTN_LB, ds4_report.button_l1);
    setButtons(HOST_BTN_RB, ds4_report.button_r1);
    setButtons(HOST_BTN_HOME, ds4_report.button_home);
    setButtons(HOST_BTN_A, ds4_report.button_south);
    setButtons(HOST_BTN_B, ds4_report.button_east);
    setButtons(HOST_BTN_X, ds4_report.button_west);
    setButtons(HOST_BTN_Y, ds4_report.button_north);
    _state.lt = ds4_report.left_trigger;
    _state.rt = ds4_report.right_trigger;
    _state.lx = ds4_report.left_stick_x;
    _state.ly = static_cast<uint8_t>(~ds4_report.left_stick_y);
    _state.rx = ds4_report.right_stick_x;
    _state.ry = static_cast<uint8_t>(~ds4_report.right_stick_y);
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
    setButtons(HOST_BTN_START, dualsense_report.button_start);
    setButtons(HOST_BTN_BACK, dualsense_report.button_select);
    setButtons(HOST_BTN_LS, dualsense_report.button_l3);
    setButtons(HOST_BTN_RS, dualsense_report.button_r3);
    setButtons(HOST_BTN_LB, dualsense_report.button_l1);
    setButtons(HOST_BTN_RB, dualsense_report.button_r1);
    setButtons(HOST_BTN_HOME, dualsense_report.button_home);
    setButtons(HOST_BTN_A, dualsense_report.button_south);
    setButtons(HOST_BTN_B, dualsense_report.button_east);
    setButtons(HOST_BTN_X, dualsense_report.button_west);
    setButtons(HOST_BTN_Y, dualsense_report.button_north);
    _state.lt = dualsense_report.left_trigger;
    _state.rt = dualsense_report.right_trigger;
    _state.lx = dualsense_report.left_stick_x;
    _state.ly = static_cast<uint8_t>(~dualsense_report.left_stick_y);
    _state.rx = dualsense_report.right_stick_x;
    _state.ry = static_cast<uint8_t>(~dualsense_report.right_stick_y);
}

void USBHostGamepadInput::startSwitchProInit() {
    (void) hostSendReport(0, SWITCH_INIT_REPORT, sizeof(SWITCH_INIT_REPORT));
}

uint8_t USBHostGamepadInput::nextSwitchReportCounter() {
    _switch_report_counter = static_cast<uint8_t>(_switch_report_counter + 1);
    return _switch_report_counter;
}

void USBHostGamepadInput::handleSwitchProInitReport(const uint8_t *report, uint16_t len) {
    switch_pro_host_report_t out_report = {
        .command = SWITCH_REPORT_CONFIGURATION,
        .counter = 0,
        .rumble_l = {0x00, 0x01, 0x40, 0x40},
        .rumble_r = {0x00, 0x01, 0x40, 0x40},
        .subcommand = 0,
        .subcommand_args = {0x00, 0x00, 0x00},
    };

    if (len < 2 || report[0] != SWITCH_REPORT_USB_INPUT_81) {
        (void) hostSendReport(0, SWITCH_INIT_REPORT, sizeof(SWITCH_INIT_REPORT));
        return;
    }

    if (report[1] == SWITCH_SUBCMD_IDENTIFY) {
        out_report.counter = SWITCH_SUBCMD_HANDSHAKE;
        (void) nextSwitchReportCounter();
        (void) hostSendReport(0, &out_report, 10);
        return;
    }

    if (report[1] == SWITCH_SUBCMD_HANDSHAKE || report[0] == SWITCH_REPORT_OUTPUT_30) {
        out_report.counter = SWITCH_SUBCMD_DISABLE_USB_TIMEOUT;
        (void) nextSwitchReportCounter();
        (void) hostSendReport(0, &out_report, 10);
        _switch_pro_ready = true;
    }
}

bool USBHostGamepadInput::hostSendReport(uint8_t report_id, const void *report, uint16_t len) {
#if CFG_TUH_ENABLED
    return tuh_hid_send_report(_dev_addr, _instance, report_id, report, len);
#else
    (void) report_id;
    (void) report;
    (void) len;
    return false;
#endif
}

void USBHostGamepadInput::applySwitchProReport(const uint8_t *report, uint16_t len) {
    if (!_switch_pro_ready) {
        handleSwitchProInitReport(report, len);
        return;
    }

    if (len < sizeof(switch_pro_report_t) || report[0] != SWITCH_REPORT_OUTPUT_30) {
        return;
    }

    switch_pro_report_t switch_report = {};
    memcpy(&switch_report, report, sizeof(switch_report));

    setButtons(HOST_BTN_DPAD_UP, switch_report.inputs.dpad_up);
    setButtons(HOST_BTN_DPAD_DOWN, switch_report.inputs.dpad_down);
    setButtons(HOST_BTN_DPAD_LEFT, switch_report.inputs.dpad_left);
    setButtons(HOST_BTN_DPAD_RIGHT, switch_report.inputs.dpad_right);
    setButtons(HOST_BTN_START, switch_report.inputs.button_plus);
    setButtons(HOST_BTN_BACK, switch_report.inputs.button_minus);
    setButtons(HOST_BTN_LS, switch_report.inputs.button_thumb_l);
    setButtons(HOST_BTN_RS, switch_report.inputs.button_thumb_r);
    setButtons(HOST_BTN_LB, switch_report.inputs.button_l);
    setButtons(HOST_BTN_RB, switch_report.inputs.button_r);
    setButtons(HOST_BTN_HOME, switch_report.inputs.button_home);
    setButtons(HOST_BTN_A, switch_report.inputs.button_b);
    setButtons(HOST_BTN_B, switch_report.inputs.button_a);
    setButtons(HOST_BTN_X, switch_report.inputs.button_y);
    setButtons(HOST_BTN_Y, switch_report.inputs.button_x);
    _state.lt = switch_report.inputs.button_zl ? 0xFF : 0x00;
    _state.rt = switch_report.inputs.button_zr ? 0xFF : 0x00;
    _state.lx = static_cast<uint8_t>(switch_report.inputs.left_stick.getX() >> 4);
    _state.ly = static_cast<uint8_t>(~(switch_report.inputs.left_stick.getY() >> 4));
    _state.rx = static_cast<uint8_t>(switch_report.inputs.right_stick.getX() >> 4);
    _state.ry = static_cast<uint8_t>(~(switch_report.inputs.right_stick.getY() >> 4));
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

void USBHostGamepadInput::clearButtons(uint32_t mask) {
    _state.buttons &= ~mask;
}

void USBHostGamepadInput::setButtons(uint32_t mask, bool enabled) {
    if (enabled) {
        _state.buttons |= mask;
    } else {
        clearButtons(mask);
    }
}

bool USBHostGamepadInput::buttonPressed(HostedButtonMask mask) const {
    return (_state.buttons & static_cast<uint32_t>(mask)) != 0;
}

void USBHostGamepadInput::applyNormalizedState(InputState &inputs) {
    set_button(inputs.buttons, DEFAULT_HOSTED_GLYPH_MAPPING.dpad_left, buttonPressed(HOST_BTN_DPAD_LEFT));
    set_button(inputs.buttons, DEFAULT_HOSTED_GLYPH_MAPPING.dpad_right, buttonPressed(HOST_BTN_DPAD_RIGHT));
    set_button(inputs.buttons, DEFAULT_HOSTED_GLYPH_MAPPING.dpad_down, buttonPressed(HOST_BTN_DPAD_DOWN));
    set_button(inputs.buttons, DEFAULT_HOSTED_GLYPH_MAPPING.dpad_up, buttonPressed(HOST_BTN_DPAD_UP));

    set_button(inputs.buttons, DEFAULT_HOSTED_GLYPH_MAPPING.face_a, buttonPressed(HOST_BTN_A));
    set_button(inputs.buttons, DEFAULT_HOSTED_GLYPH_MAPPING.face_b, buttonPressed(HOST_BTN_B));
    set_button(inputs.buttons, DEFAULT_HOSTED_GLYPH_MAPPING.face_x, buttonPressed(HOST_BTN_X));
    set_button(inputs.buttons, DEFAULT_HOSTED_GLYPH_MAPPING.face_y, buttonPressed(HOST_BTN_Y));
    set_button(inputs.buttons, DEFAULT_HOSTED_GLYPH_MAPPING.shoulder_l, buttonPressed(HOST_BTN_LB));
    set_button(inputs.buttons, DEFAULT_HOSTED_GLYPH_MAPPING.shoulder_r, buttonPressed(HOST_BTN_RB));
    set_button(inputs.buttons, DEFAULT_HOSTED_GLYPH_MAPPING.trigger_l, _state.lt > TRIGGER_THRESHOLD);
    set_button(inputs.buttons, DEFAULT_HOSTED_GLYPH_MAPPING.trigger_r, _state.rt > TRIGGER_THRESHOLD);

    set_button(inputs.buttons, DEFAULT_HOSTED_GLYPH_MAPPING.start, buttonPressed(HOST_BTN_START));
    set_button(inputs.buttons, DEFAULT_HOSTED_GLYPH_MAPPING.back, buttonPressed(HOST_BTN_BACK));
    set_button(inputs.buttons, DEFAULT_HOSTED_GLYPH_MAPPING.home, buttonPressed(HOST_BTN_HOME));
    set_button(inputs.buttons, BTN_MB4, false);
    set_button(inputs.buttons, DEFAULT_HOSTED_GLYPH_MAPPING.mod_x, buttonPressed(HOST_BTN_LS));
    set_button(inputs.buttons, DEFAULT_HOSTED_GLYPH_MAPPING.mod_y, buttonPressed(HOST_BTN_RS));
    set_button(inputs.buttons, DEFAULT_HOSTED_GLYPH_MAPPING.right_stick_click, false);

    set_button(inputs.buttons, BTN_RT3, _state.rx < (128 - (STICK_DIGITAL_THRESHOLD >> 8)));
    set_button(inputs.buttons, BTN_RT5, _state.rx > (128 + (STICK_DIGITAL_THRESHOLD >> 8)));
    set_button(inputs.buttons, BTN_RT2, _state.ry > (128 + (STICK_DIGITAL_THRESHOLD >> 8)));
    set_button(inputs.buttons, BTN_RT4, _state.ry < (128 - (STICK_DIGITAL_THRESHOLD >> 8)));

    inputs.nunchuk_connected = true;
    inputs.nunchuk_x = static_cast<int8_t>(_state.lx);
    inputs.nunchuk_y = static_cast<int8_t>(_state.ly);
}
