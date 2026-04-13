#ifndef _INPUT_USB_HOST_GAMEPAD_INPUT_HPP
#define _INPUT_USB_HOST_GAMEPAD_INPUT_HPP

#include "core/InputSource.hpp"
#include "usb/TinyUSBHostListener.hpp"

class USBHostGamepadInput : public InputSource, public TinyUSBHostListener {
  public:
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
    };

    void clearMappedInputs(InputState &inputs);
    void applyXInputState(InputState &inputs);
    void resetState();
    void applyDs4Report(const uint8_t *report, uint16_t len);
    void setDpadFromHat(uint8_t hat);

    bool _active = false;
    bool _applied_last_update = false;
    SourceType _source = SourceType::NONE;
    uint8_t _dev_addr = 0;
    uint8_t _instance = 0;
    uint8_t _type = 0;
    uint16_t _last_vid = 0;
    uint16_t _last_pid = 0;

    bool _dpad_up = false;
    bool _dpad_down = false;
    bool _dpad_left = false;
    bool _dpad_right = false;
    bool _start = false;
    bool _back = false;
    bool _ls = false;
    bool _rs = false;
    bool _lb = false;
    bool _rb = false;
    bool _home = false;
    bool _a = false;
    bool _b = false;
    bool _x = false;
    bool _y = false;
    uint8_t _lt = 0;
    uint8_t _rt = 0;
    uint8_t _lx = 128;
    uint8_t _ly = 128;
    uint8_t _rx = 128;
    uint8_t _ry = 128;
};

#endif
