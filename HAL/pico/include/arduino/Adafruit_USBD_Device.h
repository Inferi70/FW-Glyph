#ifndef ADAFRUIT_USBD_DEVICE_H_
#define ADAFRUIT_USBD_DEVICE_H_

#include "Adafruit_USBD_Interface.h"
#include "tusb.h"

class Adafruit_USBD_Device {
  private:
    enum { STRING_DESCRIPTOR_MAX = 12 };

    tusb_desc_device_t _desc_device __attribute__((aligned(4)));

    uint8_t *_desc_cfg;
    uint8_t _desc_cfg_buffer[256];
    uint16_t _desc_cfg_len;
    uint16_t _desc_cfg_maxlen;

    uint8_t _itf_count;
    uint8_t _epin_count;
    uint8_t _epout_count;

    const char *_desc_str_arr[STRING_DESCRIPTOR_MAX];
    uint8_t _desc_str_count;
    uint16_t _desc_str[32 + 1];

  public:
    Adafruit_USBD_Device(void);

    void setID(uint16_t vid, uint16_t pid);
    void setVersion(uint16_t bcd);
    void setDeviceVersion(uint16_t bcd);

    bool addInterface(Adafruit_USBD_Interface &itf);
    void clearConfiguration(void);
    void setConfigurationBuffer(uint8_t *buf, uint32_t buflen);

    void setLanguageDescriptor(uint16_t language_id);
    void setManufacturerDescriptor(const char *s);
    void setProductDescriptor(const char *s);
    void setSerialDescriptor(const char *s);
    uint8_t getSerialDescriptor(uint16_t *serial_utf16);
    uint8_t addStringDescriptor(const char *s);

    bool begin(uint8_t rhport = 0);
    void task(void);
    bool detach(void);
    bool attach(void);
    bool mounted(void);
    bool suspended(void);
    bool ready(void);
    bool remoteWakeup(void);

  private:
    uint16_t const *descriptor_string_cb(uint8_t index, uint16_t langid);

    friend uint8_t const *tud_descriptor_device_cb(void);
    friend uint8_t const *tud_descriptor_configuration_cb(uint8_t index);
    friend uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid);
};

extern Adafruit_USBD_Device TinyUSBDevice;

#ifdef USE_TINYUSB
#define USBDevice TinyUSBDevice
#endif

#endif
