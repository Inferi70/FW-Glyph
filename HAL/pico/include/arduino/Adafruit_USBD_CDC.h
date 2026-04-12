#ifndef ADAFRUIT_USBD_CDC_H_
#define ADAFRUIT_USBD_CDC_H_

#include "Adafruit_TinyUSB_API.h"

#if defined(__cplusplus)

#include "Adafruit_USBD_Interface.h"
#include "Stream.h"

class Adafruit_USBD_CDC : public Stream, public Adafruit_USBD_Interface {
  public:
    Adafruit_USBD_CDC(void);

    static uint8_t getInstanceCount(void) { return _instance_count; }

    uint16_t getInterfaceDescriptor(uint8_t itfnum, uint8_t *buf, uint16_t bufsize) override;

    void setPins(uint8_t pin_rx, uint8_t pin_tx) {
        (void)pin_rx;
        (void)pin_tx;
    }
    void begin(uint32_t baud);
    void begin(uint32_t baud, uint8_t config);
    void end(void);

    uint32_t baud(void);
    uint8_t stopbits(void);
    uint8_t paritytype(void);
    uint8_t numbits(void);
    int dtr(void);

    int available(void) override;
    int peek(void) override;
    int read(void) override;
    size_t read(uint8_t *buffer, size_t size);
    void flush(void) override;
    size_t write(uint8_t) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    size_t write(const char *buffer, size_t size) {
        return write(reinterpret_cast<const uint8_t *>(buffer), size);
    }
    int availableForWrite(void) override;
    using Print::write;
    operator bool();

  private:
    enum { INVALID_INSTANCE = 0xffu };
    static uint8_t _instance_count;

    uint8_t _instance;

    bool isValid(void) { return _instance != INVALID_INSTANCE; }
};

#if defined(USE_TINYUSB)
extern Adafruit_USBD_CDC Serial;
#define SerialTinyUSB Serial
#endif

#ifndef SerialTinyUSB
extern Adafruit_USBD_CDC SerialTinyUSB;
#endif

#endif

#endif
