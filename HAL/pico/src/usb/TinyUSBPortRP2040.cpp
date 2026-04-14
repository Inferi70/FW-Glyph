#include "tusb_option.h"

#if defined ARDUINO_ARCH_RP2040 && CFG_TUD_ENABLED

#include "Arduino.h"

extern "C" {
#include "hardware/flash.h"
#include "hardware/irq.h"
#include "pico/bootrom.h"
#include "pico/mutex.h"
#include "pico/time.h"
}

#include "arduino/Adafruit_TinyUSB_API.h"
#include "tusb.h"

#if CFG_TUH_ENABLED
#include "third_party/tinyusb_gp2040/src/host/usbh.h"
#include "pio_usb.h"
#endif

#if (PICO_SDK_VERSION_MAJOR * 100 + PICO_SDK_VERSION_MINOR) < 104
#define USB_TASK_IRQ 31
#else
static unsigned int USB_TASK_IRQ;
#endif

#ifdef ARDUINO_ARCH_MBED
#define get_unique_id(_serial) flash_get_unique_id(_serial)
#else
#include "pico/unique_id.h"
#define get_unique_id(_serial) pico_get_unique_board_id((pico_unique_board_id_t *)_serial);
#endif

mutex_t __usb_mutex;

static void usb_task_irq(void) {
    if (mutex_try_enter(&__usb_mutex, NULL)) {
        tud_task();
        mutex_exit(&__usb_mutex);
    }
}

#ifndef PICO_SHARED_IRQ_HANDLER_LOWEST_ORDER_PRIORITY
#define PICO_SHARED_IRQ_HANDLER_LOWEST_ORDER_PRIORITY 0x00
#endif

static void usb_task_trigger_irq(void) {
    irq_set_pending(USB_TASK_IRQ);
}

void TinyUSB_Port_InitDevice(uint8_t rhport) {
    mutex_init(&__usb_mutex);

    tud_init(rhport);

#if (PICO_SDK_VERSION_MAJOR * 100 + PICO_SDK_VERSION_MINOR) >= 104
    USB_TASK_IRQ = user_irq_claim_unused(true);
#endif
    irq_set_exclusive_handler(USB_TASK_IRQ, usb_task_irq);
    irq_set_enabled(USB_TASK_IRQ, true);

    irq_add_shared_handler(
        USBCTRL_IRQ,
        usb_task_trigger_irq,
        PICO_SHARED_IRQ_HANDLER_LOWEST_ORDER_PRIORITY
    );
}

void TinyUSB_Port_EnterDFU(void) {
    reset_usb_boot(0, 0);
    while (1) {
    }
}

uint8_t TinyUSB_Port_GetSerialNumber(uint8_t serial_id[16]) {
    get_unique_id(serial_id);
    return FLASH_UNIQUE_ID_SIZE_BYTES;
}

extern "C" {

void TinyUSB_Device_Task(void) {
    if (mutex_try_enter(&__usb_mutex, NULL)) {
        tud_task();
        mutex_exit(&__usb_mutex);
    }
}

bool TinyUSB_Port_InitHost(uint8_t rhport) {
    (void)rhport;
#if CFG_TUH_ENABLED
    static pio_usb_configuration_t host_cfg = PIO_USB_DEFAULT_CONFIG;

#ifdef FW_USB_HOST_DP_PIN
    host_cfg.pin_dp = FW_USB_HOST_DP_PIN;
#endif

#ifdef FW_USB_HOST_PINOUT
    host_cfg.pinout = static_cast<PIO_USB_PINOUT>(FW_USB_HOST_PINOUT);
#endif

    return tuh_configure(rhport, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &host_cfg);
#else
    return false;
#endif
}

void TinyUSB_Port_DeinitHost(uint8_t rhport) {
    (void)rhport;
#if CFG_TUH_ENABLED
    pio_usb_host_stop();
#endif
}

}

#endif
