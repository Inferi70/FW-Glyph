#include "tusb_option.h"

#if CFG_TUD_ENABLED && !defined(ARDUINO_ARCH_ESP32)

#include "Adafruit_TinyUSB.h"
#include "Arduino.h"

extern "C" {

void TinyUSB_Device_Init(uint8_t rhport) {
    TinyUSBDevice.begin(rhport);
}

void TinyUSB_Device_FlushCDC(void) {
    uint8_t const cdc_instance = Adafruit_USBD_CDC::getInstanceCount();
    for (uint8_t instance = 0; instance < cdc_instance; instance++) {
        tud_cdc_n_write_flush(instance);
    }
}

}

#endif
