#ifndef ADAFRUIT_USBD_INTERFACE_H_
#define ADAFRUIT_USBD_INTERFACE_H_

#include <stddef.h>
#include <stdint.h>

class Adafruit_USBD_Interface {
  protected:
    const char *_desc_str;

  public:
    Adafruit_USBD_Interface(void) { _desc_str = nullptr; }

    virtual uint16_t getInterfaceDescriptor(uint8_t itfnum, uint8_t *buf, uint16_t bufsize) = 0;

    void setStringDescriptor(const char *str) { _desc_str = str; }
    const char *getStringDescriptor(void) { return _desc_str; }
};

#endif
