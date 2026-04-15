#include "tusb_option.h"

#if CFG_TUD_ENABLED

#include "arduino/Adafruit_TinyUSB_API.h"
#include "arduino/Adafruit_USBD_CDC.h"
#include "arduino/Adafruit_USBD_Device.h"

#include "Arduino.h"

#ifndef USB_VID
#ifdef BOARD_VENDORID
#define USB_VID BOARD_VENDORID
#else
#define USB_VID 0x239a
#endif
#endif

#ifndef USB_PID
#ifdef BOARD_PRODUCTID
#define USB_PID BOARD_PRODUCTID
#else
#define USB_PID 0xcafe
#endif
#endif

#ifndef USB_MANUFACTURER
#ifdef BOARD_MANUFACTURER
#define USB_MANUFACTURER BOARD_MANUFACTURER
#else
#define USB_MANUFACTURER "Adafruit"
#endif
#endif

#ifndef USB_PRODUCT
#ifdef BOARD_NAME
#define USB_PRODUCT BOARD_NAME
#else
#define USB_PRODUCT "Unknown"
#endif
#endif

#ifndef USB_LANGUAGE
#define USB_LANGUAGE 0x0409
#endif

enum { STRID_LANGUAGE = 0, STRID_MANUFACTURER, STRID_PRODUCT, STRID_SERIAL };

Adafruit_USBD_Device TinyUSBDevice;

static uint16_t runtime_vid = USB_VID;
static uint16_t runtime_pid = USB_PID;
static uint16_t runtime_usb_version = 0x0200;
static uint16_t runtime_device_version = 0x0100;
static uint8_t runtime_device_class = 0;
static uint8_t runtime_device_subclass = 0;
static uint8_t runtime_device_protocol = 0;
static const char *runtime_manufacturer = USB_MANUFACTURER;
static const char *runtime_product = USB_PRODUCT;
static const char *runtime_serial = nullptr;

Adafruit_USBD_Device::Adafruit_USBD_Device(void) {}

void Adafruit_USBD_Device::setConfigurationBuffer(uint8_t *buf, uint32_t buflen) {
    if (buflen < _desc_cfg_maxlen) {
        return;
    }

    memcpy(buf, _desc_cfg, _desc_cfg_len);
    _desc_cfg = buf;
    _desc_cfg_maxlen = buflen;
}

void Adafruit_USBD_Device::setID(uint16_t vid, uint16_t pid) {
    runtime_vid = vid;
    runtime_pid = pid;
    _desc_device.idVendor = vid;
    _desc_device.idProduct = pid;
}

void Adafruit_USBD_Device::setVersion(uint16_t bcd) {
    runtime_usb_version = bcd;
    _desc_device.bcdUSB = bcd;
}

void Adafruit_USBD_Device::setDeviceVersion(uint16_t bcd) {
    runtime_device_version = bcd;
    _desc_device.bcdDevice = bcd;
}

void Adafruit_USBD_Device::setLanguageDescriptor(uint16_t language_id) {
    _desc_str_arr[STRID_LANGUAGE] = (const char *)((uint32_t)language_id);
}

void Adafruit_USBD_Device::setManufacturerDescriptor(const char *s) {
    runtime_manufacturer = s;
    _desc_str_arr[STRID_MANUFACTURER] = s;
}

void Adafruit_USBD_Device::setProductDescriptor(const char *s) {
    runtime_product = s;
    _desc_str_arr[STRID_PRODUCT] = s;
}

void Adafruit_USBD_Device::setSerialDescriptor(const char *s) {
    runtime_serial = s;
    _desc_str_arr[STRID_SERIAL] = s;
}

uint8_t Adafruit_USBD_Device::addStringDescriptor(const char *s) {
    if (_desc_str_count >= STRING_DESCRIPTOR_MAX || s == NULL) {
        return 0;
    }

    uint8_t index = _desc_str_count++;
    _desc_str_arr[index] = s;
    return index;
}

void Adafruit_USBD_Device::task(void) {
    tud_task();
}

bool Adafruit_USBD_Device::mounted(void) {
    return tud_mounted();
}

bool Adafruit_USBD_Device::suspended(void) {
    return tud_suspended();
}

bool Adafruit_USBD_Device::ready(void) {
    return tud_ready();
}

bool Adafruit_USBD_Device::remoteWakeup(void) {
    return tud_remote_wakeup();
}

bool Adafruit_USBD_Device::detach(void) {
    return tud_disconnect();
}

bool Adafruit_USBD_Device::attach(void) {
    return tud_connect();
}

void Adafruit_USBD_Device::clearConfiguration(void) {
    tusb_desc_device_t const desc_dev = {.bLength = sizeof(tusb_desc_device_t),
                                         .bDescriptorType = TUSB_DESC_DEVICE,
                                         .bcdUSB = runtime_usb_version,
                                         .bDeviceClass = runtime_device_class,
                                         .bDeviceSubClass = runtime_device_subclass,
                                         .bDeviceProtocol = runtime_device_protocol,
                                         .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
                                         .idVendor = runtime_vid,
                                         .idProduct = runtime_pid,
                                         .bcdDevice = runtime_device_version,
                                         .iManufacturer = STRID_MANUFACTURER,
                                         .iProduct = STRID_PRODUCT,
                                         .iSerialNumber = STRID_SERIAL,
                                         .bNumConfigurations = 0x01};

    _desc_device = desc_dev;

    uint8_t const dev_cfg[sizeof(tusb_desc_configuration_t)] = {
        TUD_CONFIG_DESCRIPTOR(
            1,
            0,
            0,
            sizeof(tusb_desc_configuration_t),
            TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP | TU_BIT(7),
            100
        ),
    };

    memcpy(_desc_cfg_buffer, dev_cfg, sizeof(tusb_desc_configuration_t));
    _desc_cfg = _desc_cfg_buffer;
    _desc_cfg_maxlen = sizeof(_desc_cfg_buffer);
    _desc_cfg_len = sizeof(tusb_desc_configuration_t);

    _itf_count = 0;
    _epin_count = _epout_count = 1;

    memset(_desc_str_arr, 0, sizeof(_desc_str_arr));
    _desc_str_arr[STRID_LANGUAGE] = (const char *)((uint32_t)USB_LANGUAGE);
    _desc_str_arr[STRID_MANUFACTURER] = runtime_manufacturer;
    _desc_str_arr[STRID_PRODUCT] = runtime_product;
    _desc_str_arr[STRID_SERIAL] = runtime_serial;
    _desc_str_count = 4;
}

bool Adafruit_USBD_Device::addInterface(Adafruit_USBD_Interface &itf) {
    uint8_t *desc = _desc_cfg + _desc_cfg_len;
    uint16_t const len =
        itf.getInterfaceDescriptor(_itf_count, desc, _desc_cfg_maxlen - _desc_cfg_len);
    uint8_t *desc_end = desc + len;
    const char *desc_str = itf.getStringDescriptor();

    if (!len) {
        return false;
    }

    while (desc < desc_end) {
        if (tu_desc_type(desc) == TUSB_DESC_INTERFACE) {
            tusb_desc_interface_t *desc_itf = (tusb_desc_interface_t *)desc;
            if (desc_itf->bAlternateSetting == 0) {
                _itf_count++;
                if (desc_str && (_desc_str_count < STRING_DESCRIPTOR_MAX)) {
                    _desc_str_arr[_desc_str_count] = desc_str;
                    desc_itf->iInterface = _desc_str_count;
                    _desc_str_count++;
                    desc_str = NULL;
                }
            }
        } else if (tu_desc_type(desc) == TUSB_DESC_ENDPOINT) {
            tusb_desc_endpoint_t *desc_ep = (tusb_desc_endpoint_t *)desc;
            uint8_t const dir = desc_ep->bEndpointAddress & 0x80;
            uint8_t ep_num = desc_ep->bEndpointAddress & 0x0F;

            // Keep explicitly assigned endpoint numbers intact for composite
            // descriptors that must reference fixed addresses internally.
            if (dir) {
                if (ep_num == 0) {
                    ep_num = _epin_count++;
                } else if (_epin_count <= ep_num) {
                    _epin_count = ep_num + 1;
                }
            } else {
                if (ep_num == 0) {
                    ep_num = _epout_count++;
                } else if (_epout_count <= ep_num) {
                    _epout_count = ep_num + 1;
                }
            }

            desc_ep->bEndpointAddress = dir | ep_num;
        }

        if (desc[0] == 0) {
            return false;
        }

        desc += tu_desc_len(desc);
    }

    _desc_cfg_len += len;

    tusb_desc_configuration_t *config = (tusb_desc_configuration_t *)_desc_cfg;
    config->wTotalLength = _desc_cfg_len;
    config->bNumInterfaces = _itf_count;

    return true;
}

bool Adafruit_USBD_Device::begin(uint8_t rhport) {
    clearConfiguration();

    if (runtime_device_class == 0 && runtime_device_subclass == 0 &&
        runtime_device_protocol == 0) {
        _desc_device.bDeviceClass = TUSB_CLASS_MISC;
        _desc_device.bDeviceSubClass = MISC_SUBCLASS_COMMON;
        _desc_device.bDeviceProtocol = MISC_PROTOCOL_IAD;
    }

    SerialTinyUSB.begin(115200);
    TinyUSB_Port_InitDevice(rhport);

    return true;
}

extern "C" void TinyUSBDevice_SetDeviceClassCodes(
    uint8_t device_class,
    uint8_t device_subclass,
    uint8_t device_protocol
) {
    runtime_device_class = device_class;
    runtime_device_subclass = device_subclass;
    runtime_device_protocol = device_protocol;
}

static int strcpy_utf16(const char *s, uint16_t *buf, int bufsize);

uint8_t Adafruit_USBD_Device::getSerialDescriptor(uint16_t *serial_utf16) {
    if (!_desc_str_arr[STRID_SERIAL]) {
        uint8_t serial_id[16] __attribute__((aligned(4)));
        uint8_t const serial_len = TinyUSB_Port_GetSerialNumber(serial_id);

        for (uint8_t i = 0; i < serial_len; i++) {
            for (uint8_t j = 0; j < 2; j++) {
                const char nibble_to_hex[16] = {
                    '0', '1', '2', '3', '4', '5', '6', '7',
                    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
                };

                uint8_t nibble = (serial_id[i] >> (j * 4)) & 0xf;
                serial_utf16[1 + i * 2 + (1 - j)] = nibble_to_hex[nibble];
            }
        }

        return 2 * serial_len;
    }

    return strcpy_utf16(_desc_str_arr[STRID_SERIAL], serial_utf16 + 1, 32);
}

uint16_t const *Adafruit_USBD_Device::descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;

    uint8_t chr_count;

    switch (index) {
        case STRID_LANGUAGE:
            _desc_str[1] = ((uint16_t)((uint32_t)_desc_str_arr[STRID_LANGUAGE]));
            chr_count = 1;
            break;

        case STRID_SERIAL:
            chr_count = getSerialDescriptor(_desc_str);
            break;

        default:
            if (index >= _desc_str_count) {
                return NULL;
            }

            chr_count = strcpy_utf16(_desc_str_arr[index], _desc_str + 1, 32);
            break;
    }

    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    return _desc_str;
}

extern "C" {

uint8_t const *tud_descriptor_device_cb(void) {
    return (uint8_t const *)&TinyUSBDevice._desc_device;
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return TinyUSBDevice._desc_cfg;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    return TinyUSBDevice.descriptor_string_cb(index, langid);
}

}

constexpr static inline bool isInvalidUtf8Octet(uint8_t t) {
    return (t == 0xc0) || (t == 0xC1) || (t >= 0xF5);
}

static int8_t utf8Codepoint(const uint8_t *utf8, uint32_t *codepointp) {
    const uint32_t CODEPOINT_LOWEST_SURROGATE_HALF = 0xD800;
    const uint32_t CODEPOINT_HIGHEST_SURROGATE_HALF = 0xDFFF;

    *codepointp = 0xFFFD;
    uint32_t codepoint;
    int len;

    if (isInvalidUtf8Octet(utf8[0])) {
        return -1;
    }

    if (utf8[0] < 0x80) {
        len = 1;
        codepoint = utf8[0];
    } else if ((utf8[0] & 0xe0) == 0xc0) {
        len = 2;
        codepoint = utf8[0] & 0x1f;
    } else if ((utf8[0] & 0xf0) == 0xe0) {
        len = 3;
        codepoint = utf8[0] & 0x0f;
    } else if ((utf8[0] & 0xf8) == 0xf0) {
        len = 4;
        codepoint = utf8[0] & 0x07;
    } else {
        return -1;
    }

    for (int i = 1; i < len; i++) {
        if ((utf8[i] & 0xc0) != 0x80) {
            return -1;
        }
        codepoint <<= 6;
        codepoint |= utf8[i] & 0x3f;
    }

    if ((len == 1) && (codepoint > 0x00007F)) {
        return -1;
    } else if ((len == 2) && ((codepoint < 0x000080) || (codepoint > 0x0007FF))) {
        return -1;
    } else if ((len == 3) && ((codepoint < 0x000800) || (codepoint > 0x00FFFF))) {
        return -1;
    } else if ((len == 4) && ((codepoint < 0x010000) || (codepoint > 0x10FFFF))) {
        return -1;
    }

    if ((codepoint >= CODEPOINT_LOWEST_SURROGATE_HALF) &&
        (codepoint <= CODEPOINT_HIGHEST_SURROGATE_HALF)) {
        return -1;
    }

    *codepointp = codepoint;
    return len;
}

static int strcpy_utf16(const char *s, uint16_t *buf, int bufsize) {
    int i = 0;
    int buflen = 0;

    while (s[i] != 0) {
        uint32_t codepoint;
        int8_t utf8len = utf8Codepoint((const uint8_t *)s + i, &codepoint);

        if (utf8len < 0) {
            i++;
            continue;
        }

        i += utf8len;

        if (codepoint <= 0xffff) {
            if (buflen == bufsize) {
                break;
            }

            buf[buflen++] = codepoint;
        } else {
            if (buflen + 1 >= bufsize) {
                break;
            }

            codepoint -= 0x10000;
            buf[buflen++] = (codepoint >> 10) + 0xd800;
            buf[buflen++] = (codepoint & 0x3ff) + 0xdc00;
        }
    }

    return buflen;
}

#if CFG_TUD_DFU_RUNTIME
void tud_dfu_runtime_reboot_to_dfu_cb(void) {}
#endif

#endif
