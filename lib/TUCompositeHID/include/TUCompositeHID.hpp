#ifndef _TUCOMPOSITE_TUCOMPOSITE_HPP
#define _TUCOMPOSITE_TUCOMPOSITE_HPP

#include <Adafruit_TinyUSB.h>
#include <Arduino.h>

namespace TUCompositeHID {
    bool addDescriptor(uint8_t *descriptor, size_t descriptor_len);
    void begin();
    bool ready();
    bool sendReport(uint8_t report_id, void const *report, size_t len);
}

#endif
