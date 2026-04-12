#ifndef ADAFRUIT_TINYUSB_API_H_
#define ADAFRUIT_TINYUSB_API_H_

#include <stdbool.h>
#include <stdint.h>

#define TINYUSB_API_VERSION 20000

#ifdef __cplusplus
extern "C" {
#endif

void TinyUSB_Device_Init(uint8_t rhport) __attribute__((weak));
void TinyUSB_Device_Task(void) __attribute__((weak));
void TinyUSB_Device_FlushCDC(void) __attribute__((weak));

#ifdef __cplusplus
}
#endif

void TinyUSB_Port_EnterDFU(void);
void TinyUSB_Port_InitDevice(uint8_t rhport);
uint8_t TinyUSB_Port_GetSerialNumber(uint8_t serial_id[16]);

#endif
