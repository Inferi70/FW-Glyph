#include "usb/PlayStationAuthPassthrough.hpp"

#include "stdlib.hpp"
#include "usb/USBHostAuthPassthrough.hpp"
#include "usb/USBHostAuthListener.hpp"

#include <CRC32.h>
#include <cstring>

namespace {

constexpr uint8_t PS4_REPORT_DEFINITION = 0x03;
constexpr uint8_t PS4_REPORT_SET_AUTH_PAYLOAD = 0xF0;
constexpr uint8_t PS4_REPORT_GET_SIGNATURE_NONCE = 0xF1;
constexpr uint8_t PS4_REPORT_GET_SIGNING_STATE = 0xF2;
constexpr uint8_t PS4_REPORT_RESET_AUTH = 0xF3;

constexpr uint8_t P5_REPORT_SET_AUTH_PAYLOAD = 0xF0;
constexpr uint8_t P5_REPORT_GET_SIGNATURE_NONCE = 0xF1;
constexpr uint8_t P5_REPORT_GET_SIGNING_STATE = 0xF2;

constexpr uint8_t PS4_RESET_RESPONSE[] = {0x00, 0x38, 0x38, 0x00, 0x00, 0x00, 0x00};
constexpr uint8_t PS4_NONCE_PAGE_COUNT = 5;
constexpr uint8_t PS4_SIGNATURE_CHUNK_COUNT = 19;
constexpr uint8_t PS4_SIGNATURE_CHUNK_SIZE = 56;

inline uint16_t min_u16(uint16_t a, uint16_t b) {
    return a < b ? a : b;
}

} // namespace

PlayStationAuthPassthrough &PlayStationAuthPassthrough::instance() {
    static PlayStationAuthPassthrough passthrough;
    return passthrough;
}

void PlayStationAuthPassthrough::reset() {
    resetPS4();
    resetP5();
}

void PlayStationAuthPassthrough::resetPS4() {
    memset(_ps4_auth_buffer, 0, sizeof(_ps4_auth_buffer));
    _ps4_nonce_id = 0;
    _ps4_console_nonce_id = 1;
    _ps4_console_nonce_chunk = 0;
    _ps4_host_nonce_page = 0;
    _ps4_host_nonce_chunk = 0;
    _ps4_stage = PS4Stage::IDLE;
}

void PlayStationAuthPassthrough::resetP5() {
    memset(_p5_auth_buffer, 0, sizeof(_p5_auth_buffer));
    _p5_f1_remaining = 0;
    _p5_f2_due_ms = 0;
    _p5_stage = P5Stage::IDLE;
}

bool PlayStationAuthPassthrough::ps4DongleReady() const {
    return USBHostAuthPassthrough::instance().deviceType() == USBHostAuthDeviceType::PS4_HID;
}

bool PlayStationAuthPassthrough::p5DongleReady() const {
    return USBHostAuthPassthrough::instance().deviceType() == USBHostAuthDeviceType::P5_HID;
}

void PlayStationAuthPassthrough::process() {
    processPS4();
    processP5();
}

void PlayStationAuthPassthrough::processPS4() {
    auto &host = USBHostAuthPassthrough::instance();
    if (!ps4DongleReady()) {
        if (_ps4_stage != PS4Stage::IDLE) {
            resetPS4();
        }
        return;
    }

    uint8_t response[64] = {};
    uint16_t response_len = 0;
    const bool response_ready = host.responseReady() && host.responseWasGet();
    const uint8_t response_report = host.responseReportId();

    if (response_ready && host.copyResponse(response, sizeof(response), &response_len)) {
        switch (_ps4_stage) {
            case PS4Stage::WAIT_RESET_AUTH:
                if (response_report == PS4_REPORT_RESET_AUTH) {
                    host.clearResponse();
                    _ps4_host_nonce_page = 0;
                    _ps4_stage = PS4Stage::SEND_NONCE_PAGE;
                    return;
                }
                break;
            case PS4Stage::WAIT_SIGNING_STATE:
                if (response_report == PS4_REPORT_GET_SIGNING_STATE) {
                    host.clearResponse();
                    if (response_len >= 3 && response[2] == 0) {
                        _ps4_host_nonce_chunk = 0;
                        _ps4_stage = PS4Stage::GET_SIGNATURE_CHUNK;
                    } else {
                        _ps4_stage = PS4Stage::GET_SIGNING_STATE;
                    }
                    return;
                }
                break;
            case PS4Stage::WAIT_SIGNATURE_CHUNK:
                if (response_report == PS4_REPORT_GET_SIGNATURE_NONCE) {
                    host.clearResponse();
                    if (response_len >= 60 && _ps4_host_nonce_chunk < PS4_SIGNATURE_CHUNK_COUNT) {
                        memcpy(
                            &_ps4_auth_buffer[_ps4_host_nonce_chunk * PS4_SIGNATURE_CHUNK_SIZE],
                            &response[4],
                            PS4_SIGNATURE_CHUNK_SIZE
                        );
                        _ps4_host_nonce_chunk++;
                    }

                    if (_ps4_host_nonce_chunk >= PS4_SIGNATURE_CHUNK_COUNT) {
                        _ps4_console_nonce_chunk = 0;
                        _ps4_stage = PS4Stage::READY_FOR_CONSOLE;
                    } else {
                        _ps4_stage = PS4Stage::GET_SIGNATURE_CHUNK;
                    }
                    return;
                }
                break;
            default:
                break;
        }
    }

    if (host.requestInFlight()) {
        return;
    }

    switch (_ps4_stage) {
        case PS4Stage::RESET_AUTH:
            if (host.submitGetReport(PS4_REPORT_RESET_AUTH, PS4_RESET_RESPONSE, sizeof(PS4_RESET_RESPONSE))) {
                _ps4_stage = PS4Stage::WAIT_RESET_AUTH;
            }
            break;
        case PS4Stage::SEND_NONCE_PAGE: {
            uint8_t packet[64] = {};
            packet[0] = PS4_REPORT_SET_AUTH_PAYLOAD;
            packet[1] = _ps4_nonce_id;
            packet[2] = _ps4_host_nonce_page;
            packet[3] = 0;

            uint8_t payload_len = 56;
            if (_ps4_host_nonce_page == (PS4_NONCE_PAGE_COUNT - 1)) {
                payload_len = 32;
                memset(&packet[4 + payload_len], 0, 24);
            }

            memcpy(
                &packet[4],
                &_ps4_auth_buffer[_ps4_host_nonce_page * PS4_SIGNATURE_CHUNK_SIZE],
                payload_len
            );

            uint32_t crc32 = CRC32::calculate(packet, 60);
            memcpy(&packet[60], &crc32, sizeof(crc32));

            if (host.submitSetReport(PS4_REPORT_SET_AUTH_PAYLOAD, packet, sizeof(packet))) {
                _ps4_stage = PS4Stage::WAIT_NONCE_PAGE;
            }
            break;
        }
        case PS4Stage::WAIT_NONCE_PAGE:
            if (!host.requestInFlight()) {
                _ps4_host_nonce_page++;
                _ps4_stage = (_ps4_host_nonce_page >= PS4_NONCE_PAGE_COUNT)
                               ? PS4Stage::GET_SIGNING_STATE
                               : PS4Stage::SEND_NONCE_PAGE;
            }
            break;
        case PS4Stage::GET_SIGNING_STATE: {
            uint8_t packet[16] = {};
            packet[0] = PS4_REPORT_GET_SIGNING_STATE;
            packet[1] = _ps4_nonce_id;
            if (host.submitGetReport(PS4_REPORT_GET_SIGNING_STATE, packet, sizeof(packet))) {
                _ps4_stage = PS4Stage::WAIT_SIGNING_STATE;
            }
            break;
        }
        case PS4Stage::GET_SIGNATURE_CHUNK: {
            uint8_t packet[64] = {};
            packet[0] = PS4_REPORT_GET_SIGNATURE_NONCE;
            packet[1] = _ps4_nonce_id;
            packet[2] = _ps4_host_nonce_chunk;
            if (host.submitGetReport(PS4_REPORT_GET_SIGNATURE_NONCE, packet, sizeof(packet))) {
                _ps4_stage = PS4Stage::WAIT_SIGNATURE_CHUNK;
            }
            break;
        }
        default:
            break;
    }
}

void PlayStationAuthPassthrough::processP5() {
    auto &host = USBHostAuthPassthrough::instance();
    if (!p5DongleReady()) {
        if (_p5_stage != P5Stage::IDLE) {
            resetP5();
        }
        return;
    }

    uint8_t response[64] = {};
    uint16_t response_len = 0;
    const bool response_ready = host.responseReady() && host.responseWasGet();
    const uint8_t response_report = host.responseReportId();

    if (response_ready && host.copyResponse(response, sizeof(response), &response_len)) {
        switch (_p5_stage) {
            case P5Stage::WAIT_GET_F1:
                if (response_report == P5_REPORT_GET_SIGNATURE_NONCE) {
                    memcpy(_p5_auth_buffer, response, min_u16(response_len, sizeof(_p5_auth_buffer)));
                    host.clearResponse();
                    _p5_stage = P5Stage::IDLE;
                    return;
                }
                break;
            case P5Stage::WAIT_GET_F2:
                if (response_report == P5_REPORT_GET_SIGNING_STATE) {
                    memcpy(_p5_auth_buffer, response, min_u16(response_len, sizeof(_p5_auth_buffer)));
                    host.clearResponse();
                    _p5_stage = P5Stage::IDLE;
                    return;
                }
                break;
            default:
                break;
        }
    }

    if (host.requestInFlight()) {
        return;
    }

    switch (_p5_stage) {
        case P5Stage::SEND_F0:
            if (host.submitSetReport(P5_REPORT_SET_AUTH_PAYLOAD, _p5_auth_buffer, sizeof(_p5_auth_buffer))) {
                _p5_stage = P5Stage::WAIT_SEND_F0;
            }
            break;
        case P5Stage::WAIT_SEND_F0:
            if (!host.requestInFlight()) {
                switch (_p5_auth_buffer[1]) {
                    case 0x01:
                        _p5_f1_remaining = 4;
                        break;
                    case 0x03:
                        _p5_f1_remaining = 1;
                        break;
                    case 0x02:
                    default:
                        _p5_f1_remaining = 0;
                        break;
                }

                if (((_p5_auth_buffer[1] == 0x01) && (_p5_auth_buffer[3] == 3)) ||
                    (_p5_auth_buffer[1] == 0x02) || (_p5_auth_buffer[1] == 0x03)) {
                    _p5_f2_due_ms = millis() + 500;
                    _p5_stage = P5Stage::DELAY_F2;
                } else {
                    _p5_stage = P5Stage::IDLE;
                }
            }
            break;
        case P5Stage::GET_F1:
            if (host.submitGetReport(P5_REPORT_GET_SIGNATURE_NONCE, _p5_auth_buffer, sizeof(_p5_auth_buffer))) {
                _p5_stage = P5Stage::WAIT_GET_F1;
                if (_p5_f1_remaining > 0) {
                    _p5_f1_remaining--;
                }
            }
            break;
        case P5Stage::DELAY_F2:
            if (millis() >= _p5_f2_due_ms) {
                _p5_stage = P5Stage::GET_F2;
            }
            break;
        case P5Stage::GET_F2:
            if (host.submitGetReport(P5_REPORT_GET_SIGNING_STATE, _p5_auth_buffer, 16)) {
                _p5_stage = P5Stage::WAIT_GET_F2;
            }
            break;
        default:
            break;
    }
}

uint16_t PlayStationAuthPassthrough::ps4GetReport(uint8_t report_id, uint8_t *buffer, uint16_t reqlen) {
    if (buffer == nullptr) {
        return 0;
    }

    uint8_t data[64] = {};
    switch (report_id) {
        case PS4_REPORT_GET_SIGNATURE_NONCE: {
            if (reqlen < 63) {
                return 0;
            }

            data[0] = PS4_REPORT_GET_SIGNATURE_NONCE;
            data[1] = _ps4_console_nonce_id;
            data[2] = _ps4_console_nonce_chunk;
            memcpy(&data[4], &_ps4_auth_buffer[_ps4_console_nonce_chunk * PS4_SIGNATURE_CHUNK_SIZE], PS4_SIGNATURE_CHUNK_SIZE);

            uint32_t crc32 = CRC32::calculate(data, 60);
            memcpy(&data[60], &crc32, sizeof(crc32));
            memcpy(buffer, &data[1], 63);

            _ps4_console_nonce_chunk++;
            if (_ps4_console_nonce_chunk >= PS4_SIGNATURE_CHUNK_COUNT) {
                _ps4_stage = PS4Stage::IDLE;
                _ps4_console_nonce_chunk = 0;
            }
            return 63;
        }
        case PS4_REPORT_GET_SIGNING_STATE: {
            if (reqlen < 15) {
                return 0;
            }

            data[0] = PS4_REPORT_GET_SIGNING_STATE;
            data[1] = _ps4_console_nonce_id;
            data[2] = (_ps4_stage == PS4Stage::READY_FOR_CONSOLE) ? 0 : 16;
            uint32_t crc32 = CRC32::calculate(data, 12);
            memcpy(&data[12], &crc32, sizeof(crc32));
            memcpy(buffer, &data[1], 15);
            return 15;
        }
        case PS4_REPORT_RESET_AUTH: {
            uint16_t response_len = min_u16(reqlen, static_cast<uint16_t>(sizeof(PS4_RESET_RESPONSE)));
            memcpy(buffer, PS4_RESET_RESPONSE, response_len);
            resetPS4();
            return response_len;
        }
        default:
            return 0;
    }
}

void PlayStationAuthPassthrough::ps4SetReport(uint8_t report_id, uint8_t const *buffer, uint16_t bufsize) {
    if (report_id != PS4_REPORT_SET_AUTH_PAYLOAD || buffer == nullptr || bufsize != 63) {
        return;
    }

    uint8_t send_buffer[64] = {};
    send_buffer[0] = report_id;
    memcpy(&send_buffer[1], buffer, bufsize);

    uint32_t incoming_crc = 0;
    memcpy(&incoming_crc, &send_buffer[60], sizeof(incoming_crc));
    if (CRC32::calculate(send_buffer, 60) != incoming_crc) {
        return;
    }

    uint8_t nonce_id = buffer[0];
    uint8_t nonce_page = buffer[1];
    if (nonce_page >= PS4_NONCE_PAGE_COUNT) {
        return;
    }

    if (nonce_page == 4) {
        memcpy(&_ps4_auth_buffer[nonce_page * PS4_SIGNATURE_CHUNK_SIZE], &send_buffer[4], 32);
        _ps4_nonce_id = nonce_id;
        _ps4_stage = PS4Stage::RESET_AUTH;
    } else {
        memcpy(&_ps4_auth_buffer[nonce_page * PS4_SIGNATURE_CHUNK_SIZE], &send_buffer[4], PS4_SIGNATURE_CHUNK_SIZE);
    }

    if (nonce_page == 0) {
        _ps4_console_nonce_id = nonce_id;
    } else if (nonce_id != _ps4_console_nonce_id) {
        resetPS4();
    }
}

uint16_t PlayStationAuthPassthrough::p5GetReport(uint8_t report_id, uint8_t *buffer, uint16_t reqlen) {
    if (buffer == nullptr) {
        return 0;
    }

    switch (report_id) {
        case P5_REPORT_GET_SIGNATURE_NONCE:
            memcpy(buffer, _p5_auth_buffer + 1, min_u16(63, reqlen));
            if (_p5_stage == P5Stage::IDLE) {
                _p5_stage = (_p5_f1_remaining > 0) ? P5Stage::GET_F1 : P5Stage::IDLE;
            }
            return min_u16(63, reqlen);
        case P5_REPORT_GET_SIGNING_STATE:
            memcpy(buffer, _p5_auth_buffer + 1, min_u16(15, reqlen));
            if (_p5_stage == P5Stage::IDLE) {
                _p5_stage = (_p5_f1_remaining > 0) ? P5Stage::GET_F1 : P5Stage::IDLE;
            }
            return min_u16(15, reqlen);
        default:
            return 0;
    }
}

void PlayStationAuthPassthrough::p5SetReport(uint8_t report_id, uint8_t const *buffer, uint16_t bufsize) {
    if (report_id != P5_REPORT_SET_AUTH_PAYLOAD || buffer == nullptr || bufsize != 63) {
        return;
    }

    if (_p5_stage != P5Stage::IDLE) {
        return;
    }

    _p5_auth_buffer[0] = report_id;
    memcpy(_p5_auth_buffer + 1, buffer, bufsize);
    _p5_stage = P5Stage::SEND_F0;
}
