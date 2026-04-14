#ifndef _INPUT_USB_HOST_GAMEPAD_INPUT_HPP
#define _INPUT_USB_HOST_GAMEPAD_INPUT_HPP

#include "core/InputSource.hpp"
#include "usb/TinyUSBHostListener.hpp"

class USBHostGamepadInput : public InputSource, public TinyUSBHostListener {
  public:
    void setup() override;
    InputScanSpeed ScanSpeed() override;
    void UpdateInputs(InputState &inputs) override;

    void mount(uint8_t dev_addr, uint16_t vid, uint16_t pid) override;
    void unmount(uint8_t dev_addr) override;
    void hidMount(uint8_t dev_addr, uint8_t instance, const uint8_t *desc_report, uint16_t desc_len) override;
    void hidUnmount(uint8_t dev_addr, uint8_t instance) override;
    void hidReportReceived(
        uint8_t dev_addr,
        uint8_t instance,
        const uint8_t *report,
        uint16_t len
    ) override;
    void xinputMount(uint8_t dev_addr, uint8_t instance, uint8_t type, uint8_t subtype) override;
    void xinputUnmount(uint8_t dev_addr, uint8_t instance) override;
    void xinputReportReceived(
        uint8_t dev_addr,
        uint8_t instance,
        const uint8_t *report,
        uint16_t len
    ) override;

  private:
    enum class SourceType : uint8_t {
        NONE = 0,
        XINPUT,
        DS4_HID,
        DUALSENSE_HID,
        SWITCH_PRO_HID,
    };

    enum HostedButtonMask : uint32_t {
        HOST_BTN_DPAD_UP = 1u << 0,
        HOST_BTN_DPAD_DOWN = 1u << 1,
        HOST_BTN_DPAD_LEFT = 1u << 2,
        HOST_BTN_DPAD_RIGHT = 1u << 3,
        HOST_BTN_START = 1u << 4,
        HOST_BTN_BACK = 1u << 5,
        HOST_BTN_LS = 1u << 6,
        HOST_BTN_RS = 1u << 7,
        HOST_BTN_LB = 1u << 8,
        HOST_BTN_RB = 1u << 9,
        HOST_BTN_HOME = 1u << 10,
        HOST_BTN_A = 1u << 11,
        HOST_BTN_B = 1u << 12,
        HOST_BTN_X = 1u << 13,
        HOST_BTN_Y = 1u << 14,
    };

    struct HostedGamepadState {
        uint32_t buttons = 0;
        uint8_t lt = 0;
        uint8_t rt = 0;
        uint8_t lx = 128;
        uint8_t ly = 128;
        uint8_t rx = 128;
        uint8_t ry = 128;
    };

    void clearMappedInputs(InputState &inputs);
    void applyNormalizedState(InputState &inputs);
    void resetState();
    void applyDs4Report(const uint8_t *report, uint16_t len);
    void applyDualSenseReport(const uint8_t *report, uint16_t len);
    void applySwitchProReport(const uint8_t *report, uint16_t len);
    void startSwitchProInit();
    void handleSwitchProInitReport(const uint8_t *report, uint16_t len);
    bool hostSendReport(uint8_t report_id, const void *report, uint16_t len);
    uint8_t nextSwitchReportCounter();
    void setDpadFromHat(uint8_t hat);
    void clearButtons(uint32_t mask);
    void setButtons(uint32_t mask, bool enabled);
    bool buttonPressed(HostedButtonMask mask) const;

    bool _active = false;
    bool _applied_last_update = false;
    SourceType _source = SourceType::NONE;
    uint8_t _dev_addr = 0;
    uint8_t _instance = 0;
    uint8_t _type = 0;
    uint16_t _last_vid = 0;
    uint16_t _last_pid = 0;
    bool _switch_pro_ready = false;
    uint8_t _switch_report_counter = 0;
    HostedGamepadState _state = {};
};

#endif
