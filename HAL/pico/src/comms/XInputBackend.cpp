#include "comms/XInputBackend.hpp"

#include "core/CommunicationBackend.hpp"
#include "core/state.hpp"
#include "usb/TinyUSBRuntime.hpp"
#include "hardware/timer.h"

XInputBackend::XInputBackend(
    InputState &inputs,
    InputSource **input_sources,
    size_t input_source_count
)
    : CommunicationBackend(inputs, input_sources, input_source_count),
      _xinput() {
    Serial.end();

    usb_runtime::setManufacturer("Microsoft Corporation");
    usb_runtime::setProduct("Controller");
    usb_runtime::setDeviceId(0x045E, 0x028E);
    usb_runtime::setDeviceVersion(0x0200);
    usb_runtime::setDeviceRelease(0x0114);
    usb_runtime::setDeviceClassCodes(0xFF, 0xFF, 0xFF);

    _xinput.begin();
    Serial.begin(115200);

    absolute_time_t start_time = get_absolute_time();
    while (!_xinput.ready()) {
        if ((get_absolute_time() - start_time) > 1000 * 250) {
            break;
        }
        tight_loop_contents();
    }
}

CommunicationBackendId XInputBackend::BackendId() {
    return COMMS_BACKEND_XINPUT;
}

void XInputBackend::SendReport() {
    //we get stalled if the computer doesn't like us
    //if we get stalled, the screen gets starved for inputs and we can't do shit
    //so we have a timeout
    absolute_time_t start_time = get_absolute_time();
    bool timeout = false;
    
    ScanInputs(InputScanSpeed::SLOW);
    ScanInputs(InputScanSpeed::MEDIUM);

    while (!_xinput.ready()) {
        if((get_absolute_time() - start_time)  > 1000 * 50) {
            timeout = true;
            break;
        }
    }

    ScanInputs(InputScanSpeed::FAST);

    UpdateOutputs();

    // Digital outputs
    _report.a = _outputs.a;
    _report.b = _outputs.b;
    _report.x = _outputs.x;
    _report.y = _outputs.y;
    _report.lb = _outputs.buttonL;
    _report.rb = _outputs.buttonR;
    _report.lt = _outputs.triggerLDigital ? 255 : _outputs.triggerLAnalog;
    _report.rt = _outputs.triggerRDigital ? 255 : _outputs.triggerRAnalog;
    _report.start = _outputs.start;
    _report.back = _outputs.select;
    _report.home = _outputs.home;
    _report.dpad_up = _outputs.dpadUp;
    _report.dpad_down = _outputs.dpadDown;
    _report.dpad_left = _outputs.dpadLeft;
    _report.dpad_right = _outputs.dpadRight;
    _report.ls = _outputs.leftStickClick;
    _report.rs = _outputs.rightStickClick;

    _report.lx = (_outputs.leftStickX - 128) * 65535 / 255 + 128;
    _report.ly = (_outputs.leftStickY - 128) * 65535 / 255 + 128;
    _report.rx = (_outputs.rightStickX - 128) * 65535 / 255 + 128;
    _report.ry = (_outputs.rightStickY - 128) * 65535 / 255 + 128;

    if(timeout) return;

    _xinput.sendReport(&_report);
}
