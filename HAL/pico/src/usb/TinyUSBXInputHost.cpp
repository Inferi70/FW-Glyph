#include "tusb_option.h"

#if (CFG_TUH_ENABLED && CFG_TUH_XINPUT)

#include "host/usbh.h"
#include "host/usbh_pvt.h"
#include "usb/TinyUSBXInputHost.h"

typedef struct {
    uint8_t itf_num;
    uint8_t ep_in;
    uint8_t ep_out;
    uint8_t type;
    uint8_t subtype;
    uint16_t epin_size;
    uint16_t epout_size;
    uint8_t epin_buf[CFG_TUH_XINPUT_EPIN_BUFSIZE];
    uint8_t epout_buf[CFG_TUH_XINPUT_EPOUT_BUFSIZE];
} xinputh_interface_t;

typedef struct {
    uint8_t inst_count;
    xinputh_interface_t instances[CFG_TUH_XINPUT];
} xinputh_device_t;

static xinputh_device_t _xinputh_dev[CFG_TUH_DEVICE_MAX];

#define XINPUT_DESC_TYPE_RESERVED 0x21

typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t flags;
    uint8_t reserved;
    uint8_t subtype;
    uint8_t reserved2;
    uint8_t bEndpointAddressIn;
    uint8_t bMaxDataSizeIn;
    uint8_t reserved3[5];
    uint8_t bEndpointAddressOut;
    uint8_t bMaxDataSizeOut;
    uint8_t reserved4[2];
} __attribute__((packed)) xinput_id_descriptor_t;

TU_ATTR_ALWAYS_INLINE static inline xinputh_device_t *get_dev(uint8_t dev_addr) {
    return &_xinputh_dev[dev_addr - 1];
}

TU_ATTR_ALWAYS_INLINE static inline xinputh_interface_t *get_instance(uint8_t dev_addr, uint8_t instance) {
    return &_xinputh_dev[dev_addr - 1].instances[instance];
}

static uint8_t get_instance_id_by_itfnum(uint8_t dev_addr, uint8_t itf) {
    for (uint8_t inst = 0; inst < CFG_TUH_XINPUT; inst++) {
        xinputh_interface_t *hid = get_instance(dev_addr, inst);
        if (hid->itf_num == itf) {
            return inst;
        }
    }
    return 0xff;
}

static uint8_t get_instance_id_by_epaddr(uint8_t dev_addr, uint8_t ep_addr) {
    for (uint8_t inst = 0; inst < CFG_TUH_XINPUT; inst++) {
        xinputh_interface_t *hid = get_instance(dev_addr, inst);
        if ((ep_addr == hid->ep_in) || (ep_addr == hid->ep_out)) {
            return inst;
        }
    }
    return 0xff;
}

uint8_t tuh_xinput_instance_count(uint8_t dev_addr) {
    return get_dev(dev_addr)->inst_count;
}

bool tuh_xinput_mounted(uint8_t dev_addr, uint8_t instance) {
    if (get_dev(dev_addr)->inst_count < instance) {
        return false;
    }

    xinputh_interface_t *hid_itf = get_instance(dev_addr, instance);
    return (hid_itf->ep_in != 0) || (hid_itf->ep_out != 0);
}

bool tuh_xinput_receive_report(uint8_t dev_addr, uint8_t instance) {
    xinputh_interface_t *xid_itf = get_instance(dev_addr, instance);
    TU_VERIFY(usbh_edpt_claim(dev_addr, xid_itf->ep_in));

    if (!usbh_edpt_xfer(dev_addr, xid_itf->ep_in, xid_itf->epin_buf, xid_itf->epin_size)) {
        usbh_edpt_release(dev_addr, xid_itf->ep_in);
        return false;
    }

    return true;
}

bool tuh_xinput_send_report(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len) {
    xinputh_interface_t *xid_itf = get_instance(dev_addr, instance);

    TU_ASSERT(len <= xid_itf->epout_size);

    if (!tuh_ready(dev_addr) || xid_itf->ep_out == 0 || usbh_edpt_busy(dev_addr, xid_itf->ep_out)) {
        return false;
    }

    TU_VERIFY(usbh_edpt_claim(dev_addr, xid_itf->ep_out));
    memcpy(xid_itf->epout_buf, report, len);

    if (!usbh_edpt_xfer(dev_addr, xid_itf->ep_out, xid_itf->epout_buf, len)) {
        usbh_edpt_release(dev_addr, xid_itf->ep_out);
        return false;
    }

    return true;
}

bool tuh_xinput_ready(uint8_t dev_addr, uint8_t instance) {
    TU_VERIFY(tuh_xinput_mounted(dev_addr, instance));
    xinputh_interface_t *hid_itf = get_instance(dev_addr, instance);
    return !usbh_edpt_busy(dev_addr, hid_itf->ep_in);
}

void tuh_xinput_wait_for_tx(uint8_t dev_addr, uint8_t instance) {
    if (!tuh_xinput_mounted(dev_addr, instance)) {
        return;
    }

    xinputh_interface_t *hid_itf = get_instance(dev_addr, instance);
    while (usbh_edpt_busy(dev_addr, hid_itf->ep_out)) {
        tuh_task();
    }
}

bool xinputh_init(void) {
    tu_memclr(_xinputh_dev, sizeof(_xinputh_dev));
    return true;
}

bool xinputh_xfer_cb(uint8_t dev_addr, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes) {
    uint8_t const dir = tu_edpt_dir(ep_addr);
    uint8_t const instance = get_instance_id_by_epaddr(dev_addr, ep_addr);
    xinputh_interface_t *xinput_itf = get_instance(dev_addr, instance);

    if (result != XFER_RESULT_SUCCESS) {
        usbh_edpt_xfer(dev_addr, xinput_itf->ep_in, xinput_itf->epin_buf, xinput_itf->epin_size);
        return false;
    }

    if (dir == TUSB_DIR_IN) {
        tuh_xinput_report_received_cb(dev_addr, instance, xinput_itf->epin_buf, (uint16_t)xferred_bytes);
        usbh_edpt_xfer(dev_addr, xinput_itf->ep_in, xinput_itf->epin_buf, xinput_itf->epin_size);
    } else if (tuh_xinput_report_sent_cb) {
        tuh_xinput_report_sent_cb(dev_addr, instance, xinput_itf->epout_buf, xferred_bytes);
    }

    return true;
}

void xinputh_close(uint8_t dev_addr) {
    TU_VERIFY(dev_addr <= CFG_TUH_DEVICE_MAX, );
    xinputh_device_t *hid_dev = get_dev(dev_addr);

    if (tuh_xinput_umount_cb) {
        for (uint8_t inst = 0; inst < hid_dev->inst_count; inst++) {
            tuh_xinput_umount_cb(dev_addr, inst);
        }
    }

    tu_memclr(hid_dev, sizeof(xinputh_device_t));
}

bool xinputh_open(uint8_t rhport, uint8_t dev_addr, tusb_desc_interface_t const *desc_itf, uint16_t max_len) {
    (void)rhport;
    TU_VERIFY(
        TUSB_CLASS_VENDOR_SPECIFIC == desc_itf->bInterfaceClass || TUSB_CLASS_HID == desc_itf->bInterfaceClass,
        0
    );

    xinputh_interface_t *p_xinput = nullptr;
    for (uint8_t i = 0; i < CFG_TUH_XINPUT; i++) {
        xinputh_interface_t *xid_itf = get_instance(dev_addr, i);
        if (xid_itf->ep_in == 0 && xid_itf->ep_out == 0) {
            p_xinput = xid_itf;
            break;
        }
    }
    TU_VERIFY(p_xinput, 0);

    uint8_t const *p_desc = (uint8_t const *)desc_itf;
    uint16_t pos = 0;

    if (desc_itf->bInterfaceSubClass == 0x5D &&
        (desc_itf->bInterfaceProtocol == 0x01 || desc_itf->bInterfaceProtocol == 0x03 ||
         desc_itf->bInterfaceProtocol == 0x02)) {
        uint8_t ep = 0;
        xinput_id_descriptor_t const *x_desc = nullptr;
        while (ep < desc_itf->bNumEndpoints && pos < max_len) {
            uint8_t desc_type = tu_desc_type(p_desc);
            if (desc_type == TUSB_DESC_ENDPOINT) {
                tusb_desc_endpoint_t const *ep_desc = (tusb_desc_endpoint_t const *)p_desc;
                TU_ASSERT(tuh_edpt_open(dev_addr, ep_desc));
                if (tu_edpt_dir(ep_desc->bEndpointAddress) == TUSB_DIR_OUT) {
                    p_xinput->ep_out = ep_desc->bEndpointAddress;
                    p_xinput->epout_size = tu_edpt_packet_size(ep_desc);
                } else {
                    p_xinput->ep_in = ep_desc->bEndpointAddress;
                    p_xinput->epin_size = tu_edpt_packet_size(ep_desc);
                }
                ep++;
            } else if (desc_type == XINPUT_DESC_TYPE_RESERVED) {
                TU_ASSERT(ep == 0, 0);
                x_desc = (xinput_id_descriptor_t const *)p_desc;
            }

            pos += tu_desc_len(p_desc);
            p_desc = tu_desc_next(p_desc);
        }
        TU_ASSERT(x_desc, 0);

        p_xinput->itf_num = desc_itf->bInterfaceNumber;
        p_xinput->type = XINPUT_HOST_XBOX360;
        if (desc_itf->bInterfaceProtocol == TUSB_DESC_DEVICE) {
            p_xinput->subtype = x_desc->subtype;
            usbh_edpt_xfer(dev_addr, p_xinput->ep_in, p_xinput->epin_buf, p_xinput->epin_size);
        }
        return true;
    } else if (
        desc_itf->bInterfaceSubClass == 0x47 && desc_itf->bInterfaceProtocol == 0xD0 &&
        desc_itf->bNumEndpoints
    ) {
        uint8_t ep = 0;
        while (ep < desc_itf->bNumEndpoints && pos < max_len) {
            uint8_t desc_type = tu_desc_type(p_desc);
            if (desc_type == TUSB_DESC_ENDPOINT) {
                tusb_desc_endpoint_t const *ep_desc = (tusb_desc_endpoint_t const *)p_desc;
                TU_ASSERT(tuh_edpt_open(dev_addr, ep_desc));
                if (ep_desc->bEndpointAddress & 0x80) {
                    p_xinput->ep_in = ep_desc->bEndpointAddress;
                    p_xinput->epin_size = tu_edpt_packet_size(ep_desc);
                } else {
                    p_xinput->ep_out = ep_desc->bEndpointAddress;
                    p_xinput->epout_size = tu_edpt_packet_size(ep_desc);
                }
                ep++;
            }

            pos += tu_desc_len(p_desc);
            p_desc = tu_desc_next(p_desc);
        }

        p_xinput->itf_num = desc_itf->bInterfaceNumber;
        p_xinput->type = XINPUT_HOST_XBOXONE;
        get_dev(dev_addr)->inst_count++;
        usbh_edpt_xfer(dev_addr, p_xinput->ep_in, p_xinput->epin_buf, p_xinput->epin_size);
        return true;
    }

    return false;
}

static void config_driver_mount_complete(uint8_t dev_addr, uint8_t instance) {
    xinputh_interface_t *xid_itf = get_instance(dev_addr, instance);
    tuh_xinput_mount_cb(dev_addr, instance, xid_itf->type, xid_itf->subtype);
    usbh_driver_set_config_complete(dev_addr, xid_itf->itf_num);
}

bool xinputh_set_config(uint8_t dev_addr, uint8_t itf_num) {
    uint8_t instance = get_instance_id_by_itfnum(dev_addr, itf_num);
    config_driver_mount_complete(dev_addr, instance);
    return true;
}

#endif
