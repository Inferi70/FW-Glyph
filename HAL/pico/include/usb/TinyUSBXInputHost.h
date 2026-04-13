#ifndef _USB_TINYUSB_XINPUT_HOST_H_
#define _USB_TINYUSB_XINPUT_HOST_H_

#include "tusb.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CFG_TUH_XINPUT_EPIN_BUFSIZE
#define CFG_TUH_XINPUT_EPIN_BUFSIZE 64
#endif

#ifndef CFG_TUH_XINPUT_EPOUT_BUFSIZE
#define CFG_TUH_XINPUT_EPOUT_BUFSIZE 64
#endif

typedef enum {
    XINPUT_HOST_UNKNOWN = 0,
    XINPUT_HOST_XBOX360,
    XINPUT_HOST_XBOXONE,
} xinput_host_type_t;

uint8_t tuh_xinput_instance_count(uint8_t dev_addr);
bool tuh_xinput_mounted(uint8_t dev_addr, uint8_t instance);
bool tuh_xinput_receive_report(uint8_t dev_addr, uint8_t instance);
void tuh_xinput_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t type, uint8_t subtype);
bool tuh_xinput_ready(uint8_t dev_addr, uint8_t instance);
bool tuh_xinput_send_report(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len);
void tuh_xinput_wait_for_tx(uint8_t dev_addr, uint8_t instance);
TU_ATTR_WEAK void tuh_xinput_umount_cb(uint8_t dev_addr, uint8_t instance);
void tuh_xinput_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len);
TU_ATTR_WEAK void tuh_xinput_report_sent_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len);

bool xinputh_init(void);
bool xinputh_open(uint8_t rhport, uint8_t dev_addr, tusb_desc_interface_t const *desc_itf, uint16_t max_len);
bool xinputh_set_config(uint8_t dev_addr, uint8_t itf_num);
bool xinputh_xfer_cb(uint8_t dev_addr, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes);
void xinputh_close(uint8_t dev_addr);

#ifdef __cplusplus
}
#endif

#endif
