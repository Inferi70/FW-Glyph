#include "tusb_option.h"

#if CFG_TUD_CDC && !defined(ARDUINO_ARCH_ESP32)

#include "Arduino.h"
#include "arduino/Adafruit_TinyUSB_API.h"
#include "arduino/Adafruit_USBD_CDC.h"
#include "arduino/Adafruit_USBD_Device.h"

#ifndef TINYUSB_API_VERSION
#define TINYUSB_API_VERSION 0
#endif

Adafruit_USBD_CDC SerialTinyUSB;

uint8_t Adafruit_USBD_CDC::_instance_count = 0;
Adafruit_USBD_CDC::Adafruit_USBD_CDC(void) {
    _instance = INVALID_INSTANCE;
}

#if CFG_TUD_ENABLED

#define EPOUT 0x00
#define EPIN 0x80

uint16_t Adafruit_USBD_CDC::getInterfaceDescriptor(uint8_t itfnum, uint8_t *buf, uint16_t bufsize) {
    uint8_t desc[] = {TUD_CDC_DESCRIPTOR(itfnum, 0, EPIN, 8, EPOUT, EPIN, 64)};
    uint16_t const len = sizeof(desc);

    if (bufsize < len) {
        return 0;
    }

    memcpy(buf, desc, len);
    return len;
}

void Adafruit_USBD_CDC::begin(uint32_t baud) {
    (void)baud;

    if (isValid()) {
        return;
    }

    if (!(_instance_count < CFG_TUD_CDC)) {
        return;
    }

    _instance = _instance_count++;
    this->setStringDescriptor("TinyUSB Serial");
    TinyUSBDevice.addInterface(*this);
}

void Adafruit_USBD_CDC::begin(uint32_t baud, uint8_t config) {
    (void)config;
    this->begin(baud);
}

void Adafruit_USBD_CDC::end(void) {
    TinyUSBDevice.clearConfiguration();
    _instance_count = 0;
    _instance = INVALID_INSTANCE;
}

uint32_t Adafruit_USBD_CDC::baud(void) {
    if (!isValid()) {
        return 0;
    }

    cdc_line_coding_t coding;
    tud_cdc_n_get_line_coding(_instance, &coding);
    return coding.bit_rate;
}

uint8_t Adafruit_USBD_CDC::stopbits(void) {
    if (!isValid()) {
        return 0;
    }

    cdc_line_coding_t coding;
    tud_cdc_n_get_line_coding(_instance, &coding);
    return coding.stop_bits;
}

uint8_t Adafruit_USBD_CDC::paritytype(void) {
    if (!isValid()) {
        return 0;
    }

    cdc_line_coding_t coding;
    tud_cdc_n_get_line_coding(_instance, &coding);
    return coding.parity;
}

uint8_t Adafruit_USBD_CDC::numbits(void) {
    if (!isValid()) {
        return 0;
    }

    cdc_line_coding_t coding;
    tud_cdc_n_get_line_coding(_instance, &coding);
    return coding.data_bits;
}

int Adafruit_USBD_CDC::dtr(void) {
    if (!isValid()) {
        return 0;
    }

    return tud_cdc_n_connected(_instance);
}

Adafruit_USBD_CDC::operator bool() {
    if (!isValid()) {
        return false;
    }

    bool ret = tud_cdc_n_connected(_instance);
    if (!ret) {
        yield();
    }
    return ret;
}

int Adafruit_USBD_CDC::available(void) {
    if (!isValid()) {
        return 0;
    }

    uint32_t count = tud_cdc_n_available(_instance);
    if (!count) {
        yield();
    }

    return count;
}

int Adafruit_USBD_CDC::peek(void) {
    if (!isValid()) {
        return -1;
    }

    uint8_t ch;
    return tud_cdc_n_peek(_instance, &ch) ? (int)ch : -1;
}

int Adafruit_USBD_CDC::read(void) {
    if (!isValid()) {
        return -1;
    }
    return (int)tud_cdc_n_read_char(_instance);
}

size_t Adafruit_USBD_CDC::read(uint8_t *buffer, size_t size) {
    if (!isValid()) {
        return 0;
    }

    return tud_cdc_n_read(_instance, buffer, size);
}

void Adafruit_USBD_CDC::flush(void) {
    if (!isValid()) {
        return;
    }

    tud_cdc_n_write_flush(_instance);
}

size_t Adafruit_USBD_CDC::write(uint8_t ch) {
    return write(&ch, 1);
}

size_t Adafruit_USBD_CDC::write(const uint8_t *buffer, size_t size) {
    if (!isValid()) {
        return 0;
    }

    size_t remain = size;
    while (remain && tud_cdc_n_connected(_instance)) {
        size_t wrcount = tud_cdc_n_write(_instance, buffer, remain);
        remain -= wrcount;
        buffer += wrcount;

        if (remain) {
            yield();
        }
    }

    return size - remain;
}

int Adafruit_USBD_CDC::availableForWrite(void) {
    if (!isValid()) {
        return 0;
    }
    return tud_cdc_n_write_available(_instance);
}

extern "C" {

void tud_cdc_line_state_cb(uint8_t instance, bool dtr, bool rts) {
    (void)rts;

    if (!dtr && instance == 0) {
        cdc_line_coding_t coding;
        tud_cdc_get_line_coding(&coding);

        if (coding.bit_rate == 1200) {
            TinyUSB_Port_EnterDFU();
        }
    }
}

}

#endif
#endif
