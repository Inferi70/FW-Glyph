#ifndef _COMMS_PLAYSTATIONBACKEND_HPP
#define _COMMS_PLAYSTATIONBACKEND_HPP

#include "core/CommunicationBackend.hpp"
#include "usb/TinyUSBHID.hpp"

enum class PlayStationBackendMode : uint8_t {
    PS4 = 0,
    PS5 = 1,
};

class PlayStationBackend : public CommunicationBackend {
  public:
    PlayStationBackend(
        PlayStationBackendMode mode,
        InputState &inputs,
        InputSource **input_sources,
        size_t input_source_count
    );
    ~PlayStationBackend();

    CommunicationBackendId BackendId() override;
    void SendReport() override;

  private:
    struct __attribute__((packed)) PS4IMUConfig {
        uint16_t gyroRange = 0;
        uint16_t gyroResPerDegDenom = 0;
        uint16_t gyroResPerDegNumer = 0;
        uint16_t accelRange = 0;
        uint16_t accelResPerG = 0;
    };

    struct __attribute__((packed)) PS4ControllerConfig {
        uint16_t hidUsage = 0x2127;
        uint8_t mystery0 = 0x04;
        uint8_t featureValue = 0xEF;

        uint8_t controllerType = 0;
        uint8_t touchpadParam[2] = {0x2c, 0x56};
        PS4IMUConfig imuConfig;
        uint16_t magicID = 0x0d0d;
        uint8_t mystery1[4] = {0x00, 0x00, 0x00, 0x00};
        uint8_t wheelParam[3] = {0x0d, 0x84, 0x03};
        uint8_t mystery2[21] = {0x00};
    };

    static uint16_t getReportCallback(
        uint8_t report_id,
        hid_report_type_t report_type,
        uint8_t *buffer,
        uint16_t reqlen
    );
    static void setReportCallback(
        uint8_t report_id,
        hid_report_type_t report_type,
        uint8_t const *buffer,
        uint16_t bufsize
    );

    uint16_t getReport(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen);
    void setReport(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize);

    void fillInputReport();
    static uint8_t getHatPosition(bool left, bool right, bool down, bool up);

    static PlayStationBackend *_active_backend;

    PlayStationBackendMode _mode;
    TinyUSBHID _hid;
    PS4ControllerConfig _controller_config = {};
    uint8_t _report[63] = {};
    uint8_t _last_features[31] = {};
    uint8_t _report_counter = 0;

    static_assert(sizeof(PS4ControllerConfig) == 47, "PS4 controller config must match HID feature size");
};

#endif
