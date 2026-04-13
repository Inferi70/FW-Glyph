#ifndef _INPUT_USB_HOST_GAMEPAD_INPUT_HPP
#define _INPUT_USB_HOST_GAMEPAD_INPUT_HPP

#include "core/InputSource.hpp"
#include "usb/TinyUSBHostListener.hpp"

class USBHostGamepadInput : public InputSource, public TinyUSBHostListener {
  public:
    InputScanSpeed ScanSpeed() override;
    void UpdateInputs(InputState &inputs) override;

    void unmount(uint8_t dev_addr) override;
    void xinputMount(uint8_t dev_addr, uint8_t instance, uint8_t type, uint8_t subtype) override;
    void xinputUnmount(uint8_t dev_addr, uint8_t instance) override;
    void xinputReportReceived(
        uint8_t dev_addr,
        uint8_t instance,
        const uint8_t *report,
        uint16_t len
    ) override;

  private:
    void clearMappedInputs(InputState &inputs);
    void applyXInputState(InputState &inputs);

    bool _active = false;
    bool _applied_last_update = false;
    uint8_t _dev_addr = 0;
    uint8_t _instance = 0;
    uint8_t _type = 0;

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
