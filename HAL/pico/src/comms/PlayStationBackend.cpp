#include "comms/PlayStationBackend.hpp"

#include "CRC32.h"
#include "core/state.hpp"
#include "usb/PlayStationAuthPassthrough.hpp"
#include "usb/TinyUSBRuntime.hpp"

#include <cstring>

namespace {

constexpr uint16_t kPs4VendorId = 0x1532;
constexpr uint16_t kPs4ProductId = 0x0401;
constexpr uint8_t kPs4ControllerType = 0x00;
constexpr uint8_t kPs5ControllerType = 0x07;

constexpr uint8_t kReportIdInput = 0x01;
constexpr uint8_t kReportIdFeatureCalibration = 0x02;
constexpr uint8_t kReportIdFeatureDefinition = 0x03;
constexpr uint8_t kReportIdFeatureOutputState = 0x05;
constexpr uint8_t kReportIdFeatureMac = 0x12;
constexpr uint8_t kReportIdFeatureUsbBtControl = 0x14;
constexpr uint8_t kReportIdFeatureVersion = 0xA3;
constexpr uint8_t kReportIdFeatureAuthF0 = 0xF0;
constexpr uint8_t kReportIdFeatureAuthF1 = 0xF1;
constexpr uint8_t kReportIdFeatureAuthF2 = 0xF2;
constexpr uint8_t kReportIdFeatureAuthF3 = 0xF3;

constexpr uint8_t kPs4Calibration[] = {
    0xfe, 0xff, 0x0e, 0x00, 0x04, 0x00, 0xd4, 0x22,
    0x2a, 0xdd, 0xbb, 0x22, 0x5e, 0xdd, 0x81, 0x22,
    0x84, 0xdd, 0x1c, 0x02, 0x1c, 0x02, 0x85, 0x1f,
    0xb0, 0xe0, 0xc6, 0x20, 0xb5, 0xe0, 0xb1, 0x20,
    0x83, 0xdf, 0x0c, 0x00,
};

constexpr uint8_t kPs4MacAddress[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x08, 0x25, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

constexpr uint8_t kPs4VersionDate[] = {
    0x4a, 0x75, 0x6e, 0x20, 0x20, 0x39, 0x20, 0x32,
    0x30, 0x31, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x31, 0x32, 0x3a, 0x33, 0x36, 0x3a, 0x34, 0x31,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x08, 0xb4, 0x01, 0x00, 0x00, 0x00,
    0x07, 0xa0, 0x10, 0x20, 0x00, 0xa0, 0x02, 0x00,
};

constexpr uint8_t kPs4ReportDescriptor[] = {
    0x05, 0x01, 0x09, 0x05, 0xA1, 0x01, 0x85, 0x01,
    0x09, 0x30, 0x09, 0x31, 0x09, 0x32, 0x09, 0x35,
    0x15, 0x00, 0x26, 0xFF, 0x00, 0x75, 0x08, 0x95,
    0x04, 0x81, 0x02, 0x09, 0x39, 0x15, 0x00, 0x25,
    0x07, 0x35, 0x00, 0x46, 0x3B, 0x01, 0x65, 0x14,
    0x75, 0x04, 0x95, 0x01, 0x81, 0x42, 0x65, 0x00,
    0x05, 0x09, 0x19, 0x01, 0x29, 0x0E, 0x15, 0x00,
    0x25, 0x01, 0x75, 0x01, 0x95, 0x0E, 0x81, 0x02,
    0x06, 0x00, 0xFF, 0x09, 0x20, 0x75, 0x06, 0x95,
    0x01, 0x81, 0x02, 0x05, 0x01, 0x09, 0x33, 0x09,
    0x34, 0x15, 0x00, 0x26, 0xFF, 0x00, 0x75, 0x08,
    0x95, 0x02, 0x81, 0x02, 0x06, 0x00, 0xFF, 0x09,
    0x21, 0x95, 0x36, 0x81, 0x02, 0x85, 0x05, 0x09,
    0x22, 0x95, 0x1F, 0x91, 0x02, 0x85, 0x03, 0x0A,
    0x21, 0x27, 0x95, 0x2F, 0xB1, 0x02, 0x85, 0x02,
    0x09, 0x24, 0x95, 0x24, 0xB1, 0x02, 0x85, 0x08,
    0x09, 0x25, 0x95, 0x03, 0xB1, 0x02, 0x85, 0x10,
    0x09, 0x26, 0x95, 0x04, 0xB1, 0x02, 0x85, 0x11,
    0x09, 0x27, 0x95, 0x02, 0xB1, 0x02, 0x85, 0x12,
    0x06, 0x02, 0xFF, 0x09, 0x21, 0x95, 0x0F, 0xB1,
    0x02, 0x85, 0x13, 0x09, 0x22, 0x95, 0x16, 0xB1,
    0x02, 0x85, 0x14, 0x06, 0x05, 0xFF, 0x09, 0x20,
    0x95, 0x10, 0xB1, 0x02, 0x85, 0x15, 0x09, 0x21,
    0x95, 0x2C, 0xB1, 0x02, 0x06, 0x80, 0xFF, 0x85,
    0x80, 0x09, 0x20, 0x95, 0x06, 0xB1, 0x02, 0x85,
    0x81, 0x09, 0x21, 0x95, 0x06, 0xB1, 0x02, 0x85,
    0x82, 0x09, 0x22, 0x95, 0x05, 0xB1, 0x02, 0x85,
    0x83, 0x09, 0x23, 0x95, 0x01, 0xB1, 0x02, 0x85,
    0x84, 0x09, 0x24, 0x95, 0x04, 0xB1, 0x02, 0x85,
    0x85, 0x09, 0x25, 0x95, 0x06, 0xB1, 0x02, 0x85,
    0x86, 0x09, 0x26, 0x95, 0x06, 0xB1, 0x02, 0x85,
    0x87, 0x09, 0x27, 0x95, 0x23, 0xB1, 0x02, 0x85,
    0x88, 0x09, 0x28, 0x95, 0x22, 0xB1, 0x02, 0x85,
    0x89, 0x09, 0x29, 0x95, 0x02, 0xB1, 0x02, 0x85,
    0x90, 0x09, 0x30, 0x95, 0x05, 0xB1, 0x02, 0x85,
    0x91, 0x09, 0x31, 0x95, 0x03, 0xB1, 0x02, 0x85,
    0x92, 0x09, 0x32, 0x95, 0x03, 0xB1, 0x02, 0x85,
    0x93, 0x09, 0x33, 0x95, 0x0C, 0xB1, 0x02, 0x85,
    0xA0, 0x09, 0x40, 0x95, 0x06, 0xB1, 0x02, 0x85,
    0xA1, 0x09, 0x41, 0x95, 0x01, 0xB1, 0x02, 0x85,
    0xA2, 0x09, 0x42, 0x95, 0x01, 0xB1, 0x02, 0x85,
    0xA3, 0x09, 0x43, 0x95, 0x30, 0xB1, 0x02, 0x85,
    0xA4, 0x09, 0x44, 0x95, 0x0D, 0xB1, 0x02, 0x85,
    0xA5, 0x09, 0x45, 0x95, 0x15, 0xB1, 0x02, 0x85,
    0xA6, 0x09, 0x46, 0x95, 0x15, 0xB1, 0x02, 0x85,
    0xA7, 0x09, 0x4A, 0x95, 0x01, 0xB1, 0x02, 0x85,
    0xA8, 0x09, 0x4B, 0x95, 0x01, 0xB1, 0x02, 0x85,
    0xA9, 0x09, 0x4C, 0x95, 0x08, 0xB1, 0x02, 0x85,
    0xAA, 0x09, 0x4E, 0x95, 0x01, 0xB1, 0x02, 0x85,
    0xAB, 0x09, 0x4F, 0x95, 0x39, 0xB1, 0x02, 0x85,
    0xAC, 0x09, 0x50, 0x95, 0x39, 0xB1, 0x02, 0x85,
    0xAD, 0x09, 0x51, 0x95, 0x0B, 0xB1, 0x02, 0x85,
    0xAE, 0x09, 0x52, 0x95, 0x01, 0xB1, 0x02, 0x85,
    0xAF, 0x09, 0x53, 0x95, 0x02, 0xB1, 0x02, 0x85,
    0xB0, 0x09, 0x54, 0x95, 0x3F, 0xB1, 0x02, 0xC0,
    0x06, 0xF0, 0xFF, 0x09, 0x40, 0xA1, 0x01, 0x85,
    0xF0, 0x09, 0x47, 0x95, 0x3F, 0xB1, 0x02, 0x85,
    0xF1, 0x09, 0x48, 0x95, 0x3F, 0xB1, 0x02, 0x85,
    0xF2, 0x09, 0x49, 0x95, 0x0F, 0xB1, 0x02, 0x85,
    0xF3, 0x0A, 0x01, 0x47, 0x95, 0x07, 0xB1, 0x02,
    0xC0,
};

template <typename T> uint16_t copyResponse(uint8_t *buffer, uint16_t reqlen, const T &src) {
    uint16_t response_len = sizeof(T);
    if (reqlen < response_len) {
        return 0;
    }

    memcpy(buffer, &src, response_len);
    return response_len;
}

template <size_t N> uint16_t copyResponse(uint8_t *buffer, uint16_t reqlen, const uint8_t (&src)[N]) {
    if (reqlen < N) {
        return 0;
    }

    memcpy(buffer, src, N);
    return N;
}

} // namespace

PlayStationBackend *PlayStationBackend::_active_backend = nullptr;

PlayStationBackend::PlayStationBackend(
    PlayStationBackendMode mode,
    InputState &inputs,
    InputSource **input_sources,
    size_t input_source_count
)
    : CommunicationBackend(inputs, input_sources, input_source_count),
      _mode(mode),
      _hid(kPs4ReportDescriptor, sizeof(kPs4ReportDescriptor), HID_ITF_PROTOCOL_NONE, 1, true) {
    _active_backend = this;

    _controller_config.featureValue = 0xEF;
    _controller_config.imuConfig.gyroRange = 0x0008;
    _controller_config.imuConfig.gyroResPerDegDenom = 0x003D;
    _controller_config.imuConfig.gyroResPerDegNumer = 0x03E8;
    _controller_config.imuConfig.accelRange = 0x0004;
    _controller_config.imuConfig.accelResPerG = 0x7FFF;
    _controller_config.controllerType = (_mode == PlayStationBackendMode::PS4) ? kPs4ControllerType : kPs5ControllerType;

    usb_runtime::setManufacturer("Open Stick Community");
    usb_runtime::setProduct((_mode == PlayStationBackendMode::PS4) ? "GP2040-CE (PS4)" : "GP2040-CE (PS5)");
    usb_runtime::setSerial("1.0");
    usb_runtime::setDeviceId(kPs4VendorId, kPs4ProductId);

    _hid.setReportCallback(&PlayStationBackend::getReportCallback, &PlayStationBackend::setReportCallback);
    _hid.begin();
}

PlayStationBackend::~PlayStationBackend() {
    if (_active_backend == this) {
        _active_backend = nullptr;
    }
}

CommunicationBackendId PlayStationBackend::BackendId() {
    return (_mode == PlayStationBackendMode::PS4) ? COMMS_BACKEND_PASSTHROUGH_PS4
                                                  : COMMS_BACKEND_PASSTHROUGH_PS5;
}

void PlayStationBackend::SendReport() {
    ScanInputs(InputScanSpeed::SLOW);
    ScanInputs(InputScanSpeed::MEDIUM);

    while (!_hid.ready()) {
        tight_loop_contents();
    }

    ScanInputs(InputScanSpeed::FAST);
    UpdateOutputs();
    fillInputReport();
    _hid.sendReport(kReportIdInput, _report, sizeof(_report));
}

uint16_t PlayStationBackend::getReportCallback(
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t *buffer,
    uint16_t reqlen
) {
    return _active_backend ? _active_backend->getReport(report_id, report_type, buffer, reqlen) : 0;
}

void PlayStationBackend::setReportCallback(
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t const *buffer,
    uint16_t bufsize
) {
    if (_active_backend) {
        _active_backend->setReport(report_id, report_type, buffer, bufsize);
    }
}

uint16_t PlayStationBackend::getReport(
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t *buffer,
    uint16_t reqlen
) {
    if (buffer == nullptr) {
        return 0;
    }

    if (report_type != HID_REPORT_TYPE_FEATURE) {
        fillInputReport();
        if (reqlen < sizeof(_report)) {
            return 0;
        }
        memcpy(buffer, _report, sizeof(_report));
        return sizeof(_report);
    }

    switch (report_id) {
        case kReportIdFeatureCalibration:
            return copyResponse(buffer, reqlen, kPs4Calibration);
        case kReportIdFeatureDefinition:
            return copyResponse(buffer, reqlen, _controller_config);
        case kReportIdFeatureMac:
            return copyResponse(buffer, reqlen, kPs4MacAddress);
        case kReportIdFeatureVersion:
            return copyResponse(buffer, reqlen, kPs4VersionDate);
        case kReportIdFeatureAuthF1:
            return (_mode == PlayStationBackendMode::PS4)
                ? PlayStationAuthPassthrough::instance().ps4GetReport(report_id, buffer, reqlen)
                : PlayStationAuthPassthrough::instance().p5GetReport(report_id, buffer, reqlen);
        case kReportIdFeatureAuthF2:
            return (_mode == PlayStationBackendMode::PS4)
                ? PlayStationAuthPassthrough::instance().ps4GetReport(report_id, buffer, reqlen)
                : PlayStationAuthPassthrough::instance().p5GetReport(report_id, buffer, reqlen);
        case kReportIdFeatureAuthF3:
            return PlayStationAuthPassthrough::instance().ps4GetReport(report_id, buffer, reqlen);
        default:
            return 0;
    }
}

void PlayStationBackend::setReport(
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t const *buffer,
    uint16_t bufsize
) {
    if (buffer == nullptr) {
        return;
    }

    if (report_type == HID_REPORT_TYPE_OUTPUT) {
        if (report_id == 0 || report_id == kReportIdFeatureOutputState) {
            uint16_t copy_len = bufsize < sizeof(_last_features) ? bufsize : sizeof(_last_features);
            memcpy(_last_features, buffer, copy_len);
        }
        return;
    }

    if (report_type != HID_REPORT_TYPE_FEATURE) {
        return;
    }

    switch (report_id) {
        case kReportIdFeatureAuthF0:
            if (_mode == PlayStationBackendMode::PS4) {
                PlayStationAuthPassthrough::instance().ps4SetReport(report_id, buffer, bufsize);
            } else {
                PlayStationAuthPassthrough::instance().p5SetReport(report_id, buffer, bufsize);
            }
            break;
        case kReportIdFeatureUsbBtControl:
        default:
            break;
    }
}

void PlayStationBackend::fillInputReport() {
    memset(_report, 0, sizeof(_report));

    _report[0] = _outputs.leftStickX;
    _report[1] = 255 - _outputs.leftStickY;
    _report[2] = _outputs.rightStickX;
    _report[3] = 255 - _outputs.rightStickY;

    uint8_t hat = getHatPosition(_outputs.dpadLeft, _outputs.dpadRight, _outputs.dpadDown, _outputs.dpadUp);
    uint16_t buttons = 0;
    buttons |= _outputs.x ? (1u << 0) : 0;                // square
    buttons |= _outputs.a ? (1u << 1) : 0;                // cross
    buttons |= _outputs.b ? (1u << 2) : 0;                // circle
    buttons |= _outputs.y ? (1u << 3) : 0;                // triangle
    buttons |= _outputs.buttonL ? (1u << 4) : 0;          // L1
    buttons |= _outputs.buttonR ? (1u << 5) : 0;          // R1
    buttons |= _outputs.triggerLDigital ? (1u << 6) : 0;  // L2
    buttons |= _outputs.triggerRDigital ? (1u << 7) : 0;  // R2
    buttons |= _outputs.select ? (1u << 8) : 0;           // share
    buttons |= _outputs.start ? (1u << 9) : 0;            // options
    buttons |= _outputs.leftStickClick ? (1u << 10) : 0;
    buttons |= _outputs.rightStickClick ? (1u << 11) : 0;
    buttons |= _outputs.home ? (1u << 12) : 0;            // PS
    buttons |= _outputs.capture ? (1u << 13) : 0;         // touchpad click

    _report[4] = (hat & 0x0F) | static_cast<uint8_t>((buttons & 0x0F) << 4);
    _report[5] = static_cast<uint8_t>((buttons >> 4) & 0xFF);
    _report[6] = static_cast<uint8_t>(((buttons >> 12) & 0x03) | ((_report_counter & 0x3F) << 2));
    _report[7] = _outputs.triggerLDigital ? 0xFF : _outputs.triggerLAnalog;
    _report[8] = _outputs.triggerRDigital ? 0xFF : _outputs.triggerRAnalog;

    _report_counter = (_report_counter + 1) & 0x3F;
}

uint8_t PlayStationBackend::getHatPosition(bool left, bool right, bool down, bool up) {
    if (up && !down) {
        if (right && !left) {
            return 0x01;
        }
        if (left && !right) {
            return 0x07;
        }
        return 0x00;
    }

    if (down && !up) {
        if (right && !left) {
            return 0x03;
        }
        if (left && !right) {
            return 0x05;
        }
        return 0x04;
    }

    if (right && !left) {
        return 0x02;
    }

    if (left && !right) {
        return 0x06;
    }

    return 0x0F;
}
