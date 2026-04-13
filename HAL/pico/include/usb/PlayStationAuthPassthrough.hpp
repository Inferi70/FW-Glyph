#ifndef _USB_PLAYSTATION_AUTH_PASSTHROUGH_HPP
#define _USB_PLAYSTATION_AUTH_PASSTHROUGH_HPP

#include <stdint.h>

class PlayStationAuthPassthrough {
  public:
    static PlayStationAuthPassthrough &instance();

    void reset();
    void process();

    uint16_t ps4GetReport(uint8_t report_id, uint8_t *buffer, uint16_t reqlen);
    void ps4SetReport(uint8_t report_id, uint8_t const *buffer, uint16_t bufsize);

    uint16_t p5GetReport(uint8_t report_id, uint8_t *buffer, uint16_t reqlen);
    void p5SetReport(uint8_t report_id, uint8_t const *buffer, uint16_t bufsize);

    bool ps4DongleReady() const;
    bool p5DongleReady() const;

  private:
    enum class PS4Stage : uint8_t {
        IDLE = 0,
        RESET_AUTH,
        WAIT_RESET_AUTH,
        SEND_NONCE_PAGE,
        WAIT_NONCE_PAGE,
        GET_SIGNING_STATE,
        WAIT_SIGNING_STATE,
        GET_SIGNATURE_CHUNK,
        WAIT_SIGNATURE_CHUNK,
        READY_FOR_CONSOLE,
    };

    enum class P5Stage : uint8_t {
        IDLE = 0,
        SEND_F0,
        WAIT_SEND_F0,
        GET_F1,
        WAIT_GET_F1,
        DELAY_F2,
        GET_F2,
        WAIT_GET_F2,
    };

    PlayStationAuthPassthrough() = default;

    void resetPS4();
    void resetP5();

    void processPS4();
    void processP5();

    uint8_t _ps4_auth_buffer[1064] = {};
    uint8_t _ps4_nonce_id = 0;
    uint8_t _ps4_console_nonce_id = 1;
    uint8_t _ps4_console_nonce_chunk = 0;
    uint8_t _ps4_host_nonce_page = 0;
    uint8_t _ps4_host_nonce_chunk = 0;
    PS4Stage _ps4_stage = PS4Stage::IDLE;

    uint8_t _p5_auth_buffer[64] = {};
    uint8_t _p5_f1_remaining = 0;
    uint32_t _p5_f2_due_ms = 0;
    P5Stage _p5_stage = P5Stage::IDLE;
};

#endif
