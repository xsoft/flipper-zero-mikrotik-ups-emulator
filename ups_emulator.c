#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*
 * Flipper Zero USB HID UPS Emulator v9 STABLE
 * Target: Unleashed 093 / API 88.9 / HW target 7
 * v9: stable MikroTik profile using the verified PowerSummary PresentStatus path only.
 *
 * Test-only USB identity: APC 051D:0002, to maximize RouterOS UPS-driver matching.
 * The HID report descriptor itself uses standard USB HID Power Device/Battery System usages.
 */

#define PACKED __attribute__((packed))

/* ---------- Minimal Flipper public API declarations ---------- */
typedef struct FuriHalUsbInterface FuriHalUsbInterface;
typedef struct Gui Gui;
typedef struct ViewPort ViewPort;
typedef struct Canvas Canvas;

extern void furi_delay_ms(uint32_t ms);
extern void* furi_record_open(const char* name);
extern void furi_record_close(const char* name);
extern FuriHalUsbInterface* furi_hal_usb_get_config(void);
extern bool furi_hal_usb_set_config(FuriHalUsbInterface* new_if, void* ctx);

extern ViewPort* view_port_alloc(void);
extern void view_port_free(ViewPort* view_port);
extern void view_port_draw_callback_set(ViewPort* view_port, void (*callback)(Canvas*, void*), void* context);
extern void view_port_input_callback_set(ViewPort* view_port, void (*callback)(void*, void*), void* context);
extern void view_port_update(ViewPort* view_port);
extern void gui_add_view_port(Gui* gui, ViewPort* view_port, int layer);
extern void gui_remove_view_port(Gui* gui, ViewPort* view_port);
extern void canvas_clear(Canvas* canvas);
extern void canvas_set_font(Canvas* canvas, int font);
extern void canvas_draw_str(Canvas* canvas, int32_t x, int32_t y, const char* str);
extern void canvas_draw_frame(Canvas* canvas, int32_t x, int32_t y, size_t width, size_t height);
extern void canvas_draw_line(Canvas* canvas, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

typedef enum {
    FuriHalPowerICCharger = 0,
    FuriHalPowerICFuelGauge = 1,
} FuriHalPowerIC;
extern uint8_t furi_hal_power_get_pct(void);
extern float furi_hal_power_get_battery_voltage(FuriHalPowerIC ic);

typedef struct NotificationApp NotificationApp;

typedef enum {
    NotificationMessageTypeVibroCompat = 0,
    NotificationMessageTypeSoundOnCompat = 1,
    NotificationMessageTypeSoundOffCompat = 2,
    NotificationMessageTypeLedRedCompat = 3,
    NotificationMessageTypeLedGreenCompat = 4,
    NotificationMessageTypeLedBlueCompat = 5,
} NotificationMessageTypeCompat;
typedef struct { uint8_t value; } NotificationMessageDataLedCompat;
typedef union {
    NotificationMessageDataLedCompat led;
    uint32_t abi_align_and_size[3];
} NotificationMessageDataCompat;
typedef struct NotificationMessage {
    NotificationMessageTypeCompat type;
    NotificationMessageDataCompat data;
} NotificationMessage;
typedef const NotificationMessage* NotificationSequence[];
extern void notification_message(NotificationApp* app, const NotificationSequence* sequence);
extern const NotificationSequence sequence_set_only_green_255;
extern const NotificationSequence sequence_blink_start_green;
extern const NotificationSequence sequence_blink_stop;
extern const NotificationSequence sequence_reset_rgb;
extern const NotificationMessage message_red_255;
extern const NotificationMessage message_green_0;
extern const NotificationMessage message_blue_255;
extern const NotificationMessage message_do_not_reset;
extern const NotificationMessage message_note_c6;
extern const NotificationMessage message_delay_100;
extern const NotificationMessage message_sound_off;

/* Flipper enums with -fshort-enums ABI */
typedef enum {
    InputKeyUp = 0,
    InputKeyDown,
    InputKeyRight,
    InputKeyLeft,
    InputKeyOk,
    InputKeyBack,
    InputKeyMAX,
} InputKey;

typedef enum {
    InputTypePress = 0,
    InputTypeRelease,
    InputTypeShort,
    InputTypeLong,
    InputTypeRepeat,
    InputTypeMAX,
} InputType;

typedef struct {
    union {
        uint32_t sequence;
        struct {
            uint32_t sequence_source : 2;
            uint32_t sequence_counter : 30;
        };
    };
    InputKey key;
    InputType type;
} InputEvent;

#define FontPrimary 0
#define FontSecondary 1
#define GuiLayerFullscreen 4

/* ---------- Minimal libusb_stm32 ABI ---------- */
typedef enum {
    usbd_fail = 0,
    usbd_ack = 1,
    usbd_nak = 2,
} usbd_respond;

typedef struct _usbd_device usbd_device;

typedef struct {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
    uint8_t data[];
} usbd_ctlreq;

typedef void (*usbd_evt_callback)(usbd_device*, uint8_t, uint8_t);
typedef void (*usbd_rqc_callback)(usbd_device*, usbd_ctlreq*);
typedef usbd_respond (*usbd_ctl_callback)(usbd_device*, usbd_ctlreq*, usbd_rqc_callback*);
typedef usbd_respond (*usbd_cfg_callback)(usbd_device*, uint8_t);
typedef usbd_respond (*usbd_dsc_callback)(usbd_ctlreq*, void**, uint16_t*);

typedef struct {
    void* data_buf;
    void* data_ptr;
    uint16_t data_count;
    uint16_t data_maxsize;
    uint8_t ep0size;
    uint8_t device_cfg;
    volatile uint8_t device_state;
    uint8_t control_state;
} usbd_status;

typedef struct usbd_driver {
    uint32_t (*getinfo)(void);
    void (*enable)(bool);
    uint8_t (*connect)(bool);
    void (*setaddr)(uint8_t);
    bool (*ep_config)(uint8_t, uint8_t, uint16_t);
    void (*ep_deconfig)(uint8_t);
    int32_t (*ep_read)(uint8_t, void*, uint16_t);
    int32_t (*ep_write)(uint8_t, const void*, uint16_t);
    void (*ep_setstall)(uint8_t, bool);
    bool (*ep_isstalled)(uint8_t);
    void (*poll)(usbd_device*, usbd_evt_callback);
    uint16_t (*frame_no)(void);
    uint16_t (*get_serialno_desc)(void*);
} usbd_driver;

struct _usbd_device {
    const usbd_driver* driver;
    usbd_ctl_callback control_callback;
    usbd_rqc_callback complete_callback;
    usbd_cfg_callback config_callback;
    usbd_dsc_callback descriptor_callback;
    usbd_evt_callback events[8];
    usbd_evt_callback endpoint[8];
    usbd_status status;
};

struct usb_device_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} PACKED;

struct usb_config_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
} PACKED;

struct usb_interface_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} PACKED;

struct usb_hid_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdHID;
    uint8_t bCountryCode;
    uint8_t bNumDescriptors;
    uint8_t bDescriptorType0;
    uint16_t wDescriptorLength0;
} PACKED;

struct usb_endpoint_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
} PACKED;

struct FuriHalUsbInterface {
    void (*init)(usbd_device* dev, FuriHalUsbInterface* intf, void* ctx);
    void (*deinit)(usbd_device* dev);
    void (*wakeup)(usbd_device* dev);
    void (*suspend)(usbd_device* dev);
    struct usb_device_descriptor* dev_descr;
    void* str_manuf_descr;
    void* str_prod_descr;
    void* str_serial_descr;
    void* cfg_descr;
};

/* ---------- USB constants ---------- */
#define USB_DTYPE_DEVICE 0x01
#define USB_DTYPE_CONFIGURATION 0x02
#define USB_DTYPE_INTERFACE 0x04
#define USB_DTYPE_ENDPOINT 0x05
#define USB_DTYPE_HID 0x21
#define USB_DTYPE_HID_REPORT 0x22
#define USB_CLASS_HID 0x03
#define USB_EPTYPE_INTERRUPT 0x03
#define USB_EP0_SIZE 8
#define USB_REQ_TYPE (3u << 5)
#define USB_REQ_STANDARD (0u << 5)
#define USB_REQ_CLASS (1u << 5)
#define USB_REQ_RECIPIENT 0x1Fu
#define USB_REQ_INTERFACE 1u
#define USB_STD_GET_DESCRIPTOR 0x06
#define USB_HID_GETREPORT 0x01
#define USB_HID_GETIDLE 0x02
#define USB_HID_SETREPORT 0x09
#define USB_HID_SETIDLE 0x0A
#define UPS_EP_IN 0x81
#define UPS_EP_SZ 8

/* APC-like report IDs. The layout follows common Back-UPS HID descriptors closely. */
#define RID_PS_IPRODUCT 0x01
#define RID_PS_ISERIAL 0x02
#define RID_RECHARGEABLE 0x05
#define RID_DIRECT_POWER 0x06
#define RID_PS_MFR_DATE 0x07
#define RID_PS_CONFIG_VOLTAGE 0x08
#define RID_PS_BATTERY_VOLTAGE 0x09
#define RID_PS_IMANUFACTURER 0x0A
#define RID_CAPACITY_MODE 0x0B
#define RID_CAP_RUNTIME 0x0C
#define RID_DESIGN_CAPACITY 0x0D
#define RID_FULL_CHARGE_CAPACITY 0x0E
#define RID_WARNING_CAPACITY_LIMIT 0x0F
#define RID_CAPACITY_GRANULARITY2 0x10
#define RID_REMAINING_CAPACITY_LIMIT 0x11
#define RID_CAPACITY_GRANULARITY1 0x12
#define RID_DIRECT_AC 0x13
#define RID_DIRECT_LOW 0x14
#define RID_PRESENT_STATUS 0x16
#define RID_REMAINING_TIME_LIMIT 0x17
#define RID_AUDIBLE_ALARM 0x18

#define RID_BATTERY_MFR_DATE 0x20
#define RID_BATTERY_CAPACITY 0x22
#define RID_BATTERY_RUNTIME 0x23
#define RID_BATTERY_CONFIG_VOLTAGE 0x25
#define RID_BATTERY_VOLTAGE 0x26
#define RID_BATTERY_TEMPERATURE 0x2D

#define RID_INPUT_CONFIG_VOLTAGE 0x30
#define RID_INPUT_VOLTAGE 0x31
#define RID_INPUT_FREQUENCY 0x37
#define RID_OUTPUT_CONFIG_VOLTAGE 0x38
#define RID_OUTPUT_VOLTAGE 0x39
#define RID_OUTPUT_FREQUENCY 0x3A
#define RID_OUTPUT_LOAD 0x3B
#define RID_POWERCONVERTER_LOAD 0x50

#define RID_ROOT_AUDIBLE 0x78
#define RID_ROOT_STATUS 0x7A
#define RID_ROOT_MFR_DATE 0x7B
#define RID_ROOT_IMANUFACTURER 0x7C
#define RID_ROOT_ISERIAL 0x7D
#define RID_ROOT_IPRODUCT 0x7F

typedef enum {
    UpsModeOnline = 0,
    UpsModeBattery = 1,
    UpsModeFlipper = 2,
} UpsMode;

typedef enum {
    UpsFaultOk = 0,
    UpsFaultLow = 1,
    UpsFaultReplace = 2,
    UpsFaultOverload = 3,
} UpsFault;



/* ---------- App state ---------- */
typedef struct {
    volatile bool running;
    volatile UpsMode mode;
    volatile UpsFault fault;
    volatile uint8_t sim_battery_percent;
    volatile uint8_t flipper_battery_percent;
    volatile uint16_t flipper_voltage_x10;
    volatile uint8_t led_applied;
    volatile uint8_t blink_ticks;
    volatile bool blink_on;
    volatile bool usb_configured;
    volatile bool usb_switch_ok;
    volatile bool beep_requested;
    volatile uint8_t force_status_reports;
    ViewPort* view_port;
    NotificationApp* notification;
} UpsAppState;

static UpsAppState* g_app = NULL;
static usbd_device* g_usb_dev = NULL;

/* ---------- HID Power Device report descriptor ----------
 * Standard usage pages only: Power Device (0x84) and Battery System (0x85).
 */
static const uint8_t ups_report_desc[] = {
    0x05, 0x84,             /* Usage Page (Power Device) */
    0x09, 0x04,             /* Usage (UPS) */
    0xA1, 0x01,             /* Collection (Application) */

    /* -------- PowerSummary: APC-like core -------- */
    0x09, 0x24,             /* Usage (PowerSummary) */
    0xA1, 0x00,             /* Collection (Physical) */
    0x75, 0x08, 0x95, 0x01,
    0x15, 0x00, 0x26, 0xFF, 0x00,

    /* HID string references used by APC devices. */
    0x85, RID_PS_IPRODUCT, 0x09, 0xFE, 0x79, 0x02, 0xB1, 0x22,
    0x85, RID_PS_ISERIAL, 0x09, 0xFF, 0x79, 0x03, 0xB1, 0x22,

    0x05, 0x85,
    0x85, RID_RECHARGEABLE, 0x09, 0x8B, 0x25, 0x01, 0xB1, 0x22,

    /* Direct Charging + Discharging + APC status flag fallback. */
    0x85, RID_DIRECT_POWER,
    0x09, 0x44, 0x81, 0xA2, 0x09, 0x44, 0xB1, 0xA2,
    0x09, 0x45, 0x81, 0xA2, 0x09, 0x45, 0xB1, 0xA2,
    0x06, 0x86, 0xFF, 0x09, 0x60, 0x81, 0xA2, 0x09, 0x60, 0xB1, 0xA2,

    /* Manufacture date. */
    0x85, RID_PS_MFR_DATE, 0x05, 0x85, 0x09, 0x85,
    0x75, 0x10, 0x27, 0xFF, 0xFF, 0x00, 0x00, 0xB1, 0xA2,

    /* Battery voltage in hundredths of a volt, same unit encoding as APC. */
    0x05, 0x84, 0x67, 0x21, 0xD1, 0xF0, 0x00, 0x55, 0x05,
    0x85, RID_PS_CONFIG_VOLTAGE, 0x09, 0x40, 0xB1, 0x22,
    0x85, RID_PS_BATTERY_VOLTAGE, 0x09, 0x30, 0xB1, 0xA2,
    0x65, 0x00, 0x55, 0x00,

    0x85, RID_PS_IMANUFACTURER, 0x09, 0xFD, 0x75, 0x08,
    0x26, 0xFF, 0x00, 0x79, 0x01, 0xB1, 0x22,

    0x85, RID_CAPACITY_MODE, 0x05, 0x85, 0x09, 0x2C, 0x25, 0x02, 0xB1, 0x22,

    /* Remaining capacity + runtime share one report, as on many real APC units. */
    0x85, RID_CAP_RUNTIME,
    0x09, 0x66, 0x75, 0x08, 0x25, 0x64, 0x81, 0xA2, 0x09, 0x66, 0xB1, 0xA2,
    0x09, 0x68, 0x75, 0x10, 0x27, 0xFF, 0xFF, 0x00, 0x00,
    0x66, 0x01, 0x10, 0x81, 0xA2, 0x09, 0x68, 0xB1, 0xA2,
    0x65, 0x00,

    0x75, 0x08, 0x25, 0x64,
    0x85, RID_DESIGN_CAPACITY, 0x09, 0x83, 0xB1, 0x22,
    0x85, RID_FULL_CHARGE_CAPACITY, 0x09, 0x67, 0xB1, 0x22,
    0x85, RID_WARNING_CAPACITY_LIMIT, 0x09, 0x8C, 0xB1, 0x22,
    0x85, RID_CAPACITY_GRANULARITY2, 0x09, 0x8E, 0xB1, 0x22,
    0x85, RID_REMAINING_CAPACITY_LIMIT, 0x09, 0x29, 0x15, 0x01, 0xB1, 0xA2,
    0x85, RID_CAPACITY_GRANULARITY1, 0x09, 0x8D, 0x15, 0x00, 0xB1, 0x22,

    /* Direct fallback flags found on real APC descriptors. */
    0x85, RID_DIRECT_AC, 0x09, 0xD0, 0x25, 0x01, 0x81, 0xA2, 0x09, 0xD0, 0xB1, 0xA2,
    0x85, RID_DIRECT_LOW, 0x09, 0x42, 0x81, 0xA2, 0x09, 0x42, 0xB1, 0xA2,
    0x05, 0x84, 0x09, 0x69, 0x81, 0xA2, 0x09, 0x69, 0xB1, 0xA2,

    /* Exact APC PowerSummary.PresentStatus layout from a real 051D:0002 dump.
     * 11 status bits followed by 21 constant padding bits; report is 5 bytes incl. ID. */
    0x09, 0x02, 0xA1, 0x02,
    0x85, RID_PRESENT_STATUS,
    0x05, 0x85, 0x65, 0x00, 0x75, 0x01, 0x15, 0x00, 0x25, 0x01,
    0x09, 0x44, 0x81, 0xA2, 0x09, 0x44, 0xB1, 0xA2, /* 0 Charging */
    0x09, 0x45, 0x81, 0xA2, 0x09, 0x45, 0xB1, 0xA2, /* 1 Discharging */
    0x09, 0xD0, 0x81, 0xA2, 0x09, 0xD0, 0xB1, 0xA2, /* 2 AC Present */
    0x09, 0xD1, 0x81, 0xA2, 0x09, 0xD1, 0xB1, 0xA2, /* 3 Battery Present */
    0x09, 0x42, 0x81, 0xA2, 0x09, 0x42, 0xB1, 0xA2, /* 4 BelowRemainingCapacityLimit */
    0x05, 0x84, 0x09, 0x69, 0x81, 0xA2, 0x09, 0x69, 0xB1, 0xA2, /* 5 ShutdownImminent */
    0x05, 0x85, 0x09, 0x43, 0x81, 0xA2, 0x09, 0x43, 0xB1, 0xA2, /* 6 RemainingTimeLimitExpired */
    0x05, 0x84, 0x09, 0x73, 0x81, 0xA2, 0x09, 0x73, 0xB1, 0xA2, /* 7 CommunicationLost */
    0x05, 0x85, 0x09, 0x4B, 0x81, 0xA2, 0x09, 0x4B, 0xB1, 0xA2, /* 8 NeedReplacement */
    0x05, 0x84, 0x09, 0x65, 0x81, 0xA2, 0x09, 0x65, 0xB1, 0xA2, /* 9 Overload */
    0x05, 0x85, 0x09, 0xDB, 0x81, 0xA2, 0x09, 0xDB, 0xB1, 0xA2, /* 10 VoltageNotRegulated */
    0x95, 0x15, 0x81, 0x01, 0xB1, 0x01,
    0xC0,

    0x85, RID_REMAINING_TIME_LIMIT, 0x05, 0x85, 0x09, 0x2A,
    0x95, 0x01, 0x75, 0x10, 0x27, 0xFF, 0xFF, 0x00, 0x00, 0x66, 0x01, 0x10, 0xB1, 0xA2,

    0x85, RID_AUDIBLE_ALARM, 0x05, 0x84, 0x09, 0x5A,
    0x75, 0x08, 0x15, 0x01, 0x25, 0x03, 0x65, 0x00, 0xB1, 0xA2,
    0xC0,                   /* End PowerSummary */

    /* -------- Battery collection -------- */
    0x09, 0x12, 0xA1, 0x00,
    0x85, RID_BATTERY_MFR_DATE, 0x05, 0x85, 0x09, 0x85,
    0x75, 0x10, 0x15, 0x00, 0x27, 0xFF, 0xFF, 0x00, 0x00, 0xB1, 0xA2,

    0x85, RID_BATTERY_CAPACITY, 0x09, 0x66,
    0x75, 0x08, 0x25, 0x64, 0xB1, 0xA2,

    0x85, RID_BATTERY_RUNTIME, 0x09, 0x68,
    0x75, 0x10, 0x27, 0xFF, 0xFF, 0x00, 0x00, 0x66, 0x01, 0x10, 0xB1, 0xA2,

    0x05, 0x84, 0x67, 0x21, 0xD1, 0xF0, 0x00, 0x55, 0x05,
    0x85, RID_BATTERY_CONFIG_VOLTAGE, 0x09, 0x40, 0xB1, 0x22,
    0x85, RID_BATTERY_VOLTAGE, 0x09, 0x30, 0xB1, 0xA2,

    /* Temperature in 0.1 K: 2982 = 298.2 K = about 25 C. */
    0x85, RID_BATTERY_TEMPERATURE, 0x09, 0x36,
    0x75, 0x10, 0x15, 0x00, 0x27, 0xFF, 0xFF, 0x00, 0x00,
    0x55, 0x0F, 0x67, 0x01, 0x00, 0x01, 0x00, 0xB1, 0xA2,
    0x65, 0x00, 0x55, 0x00,
    0xC0,

    /* -------- Input collection -------- */
    0x05, 0x84, 0x09, 0x1A, 0xA1, 0x00,
    0x75, 0x08, 0x15, 0x00, 0x26, 0xFF, 0x00,
    0x67, 0x21, 0xD1, 0xF0, 0x00, 0x55, 0x07,
    0x85, RID_INPUT_CONFIG_VOLTAGE, 0x09, 0x40, 0xB1, 0xA2,
    0x85, RID_INPUT_VOLTAGE, 0x09, 0x30, 0x75, 0x10, 0xB1, 0xA2,
    0x65, 0x00, 0x55, 0x00,
    0x85, RID_INPUT_FREQUENCY, 0x09, 0x32, 0x75, 0x10,
    0x27, 0xFF, 0xFF, 0x00, 0x00, 0xB1, 0xA2,
    0xC0,

    /* -------- Output collection -------- */
    0x09, 0x1C, 0xA1, 0x00,
    0x75, 0x10, 0x15, 0x00, 0x27, 0xFF, 0xFF, 0x00, 0x00,
    0x85, RID_OUTPUT_CONFIG_VOLTAGE, 0x09, 0x40, 0xB1, 0xA2,
    0x85, RID_OUTPUT_VOLTAGE, 0x09, 0x30, 0xB1, 0xA2,
    0x85, RID_OUTPUT_FREQUENCY, 0x09, 0x32, 0xB1, 0xA2,
    0x75, 0x08, 0x25, 0x64,
    0x85, RID_OUTPUT_LOAD, 0x09, 0x35, 0xB1, 0xA2,
    0xC0,

    /* -------- PowerConverter load, used by many APC drivers -------- */
    0x09, 0x16, 0xA1, 0x00,
    0x85, RID_POWERCONVERTER_LOAD, 0x09, 0x35,
    0x75, 0x08, 0x15, 0x00, 0x25, 0x64, 0xB1, 0xA2,
    0xC0,

    /* Root-level APC fallbacks for metadata/status/beeper. */
    0x85, RID_ROOT_IPRODUCT, 0x09, 0xFE, 0x79, 0x02,
    0x75, 0x08, 0x15, 0x00, 0x26, 0xFF, 0x00, 0xB1, 0x22,
    0x85, RID_ROOT_ISERIAL, 0x09, 0xFF, 0x79, 0x03, 0xB1, 0x22,
    0x85, RID_ROOT_IMANUFACTURER, 0x09, 0xFD, 0x79, 0x01, 0xB1, 0x22,
    0x85, RID_ROOT_MFR_DATE, 0x05, 0x85, 0x09, 0x85,
    0x75, 0x10, 0x27, 0xFF, 0xFF, 0x00, 0x00, 0xB1, 0xA2,

    0x05, 0x84, 0x09, 0x02, 0xA1, 0x02,
    0x85, RID_ROOT_STATUS, 0x05, 0x85, 0x75, 0x01, 0x15, 0x00, 0x25, 0x01,
    0x09, 0x44, 0xB1, 0xA2, 0x09, 0x45, 0xB1, 0xA2,
    0x09, 0xD0, 0xB1, 0xA2, 0x09, 0xD1, 0xB1, 0xA2,
    0x09, 0x42, 0xB1, 0xA2,
    0x05, 0x84, 0x09, 0x69, 0xB1, 0xA2,
    0x05, 0x85, 0x09, 0x43, 0xB1, 0xA2,
    0x05, 0x84, 0x09, 0x73, 0xB1, 0xA2,
    0x05, 0x85, 0x09, 0x4B, 0xB1, 0xA2,
    0x05, 0x84, 0x09, 0x65, 0xB1, 0xA2,
    0x05, 0x85, 0x09, 0xDB, 0xB1, 0xA2,
    0x95, 0x15, 0xB1, 0x01,
    0xC0,

    0x85, RID_ROOT_AUDIBLE, 0x05, 0x84, 0x09, 0x5A,
    0x75, 0x08, 0x15, 0x01, 0x25, 0x03, 0xB1, 0xA2,

    0xC0                    /* End UPS Application */
};

struct UpsIntfDescriptor {
    struct usb_interface_descriptor hid;
    struct usb_hid_descriptor hid_desc;
    struct usb_endpoint_descriptor hid_ep_in;
} PACKED;

struct UpsConfigDescriptor {
    struct usb_config_descriptor config;
    struct UpsIntfDescriptor intf_0;
} PACKED;

struct UsbString12 { uint8_t bLength; uint8_t bDescriptorType; uint16_t wString[12]; } PACKED;
struct UsbString25 { uint8_t bLength; uint8_t bDescriptorType; uint16_t wString[25]; } PACKED;
struct UsbString38 { uint8_t bLength; uint8_t bDescriptorType; uint16_t wString[38]; } PACKED;

static const struct UsbString25 ups_manufacturer_str = {
    .bLength = 2 + 25 * 2, .bDescriptorType = 0x03,
    .wString = {'A','m','e','r','i','c','a','n',' ','P','o','w','e','r',' ','C','o','n','v','e','r','s','i','o','n'}
};
static const struct UsbString38 ups_product_str = {
    .bLength = 2 + 35 * 2, .bDescriptorType = 0x03,
    .wString = {'B','a','c','k','-','U','P','S',' ','E','S',' ','6','5','0',' ','F','W',':','9','.','0','.','I',' ','U','S','B',' ','F','W',':','9','.','0'}
};
static const struct UsbString12 ups_serial_str = {
    .bLength = 2 + 12 * 2, .bDescriptorType = 0x03,
    .wString = {'F','Z','1','U','P','S','0','0','0','0','0','3'}
};

static struct usb_device_descriptor ups_device_desc = {
    .bLength = sizeof(struct usb_device_descriptor),
    .bDescriptorType = USB_DTYPE_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = USB_EP0_SIZE,
    .idVendor = 0x051D, /* APC */
    .idProduct = 0x0002, /* Common APC USB UPS PID; test emulation only */
    .bcdDevice = 0x0900,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

static const struct UpsConfigDescriptor ups_cfg_desc = {
    .config = {
        .bLength = sizeof(struct usb_config_descriptor),
        .bDescriptorType = USB_DTYPE_CONFIGURATION,
        .wTotalLength = sizeof(struct UpsConfigDescriptor),
        .bNumInterfaces = 1,
        .bConfigurationValue = 1,
        .iConfiguration = 0,
        .bmAttributes = 0xC0, /* reserved + self powered, as Flipper stock HID */
        .bMaxPower = 250,    /* 500 mA in 2 mA units */
    },
    .intf_0 = {
        .hid = {
            .bLength = sizeof(struct usb_interface_descriptor),
            .bDescriptorType = USB_DTYPE_INTERFACE,
            .bInterfaceNumber = 0,
            .bAlternateSetting = 0,
            .bNumEndpoints = 1,
            .bInterfaceClass = USB_CLASS_HID,
            .bInterfaceSubClass = 0,
            .bInterfaceProtocol = 0,
            .iInterface = 0,
        },
        .hid_desc = {
            .bLength = sizeof(struct usb_hid_descriptor),
            .bDescriptorType = USB_DTYPE_HID,
            .bcdHID = 0x0111,
            .bCountryCode = 0,
            .bNumDescriptors = 1,
            .bDescriptorType0 = USB_DTYPE_HID_REPORT,
            .wDescriptorLength0 = sizeof(ups_report_desc),
        },
        .hid_ep_in = {
            .bLength = sizeof(struct usb_endpoint_descriptor),
            .bDescriptorType = USB_DTYPE_ENDPOINT,
            .bEndpointAddress = UPS_EP_IN,
            .bmAttributes = USB_EPTYPE_INTERRUPT,
            .wMaxPacketSize = UPS_EP_SZ,
            .bInterval = 10,
        },
    },
};

/* ---------- Reports ---------- */
typedef struct { uint8_t id; uint8_t value; } PACKED Report8;
typedef struct { uint8_t id; uint16_t value; } PACKED Report16;
typedef struct { uint8_t id; uint32_t value; } PACKED Report32;
typedef struct { uint8_t id; uint8_t a; uint8_t b; } PACKED Report2x8;
typedef struct { uint8_t id; uint8_t a; uint8_t b; uint8_t c; } PACKED Report3x8;
typedef struct { uint8_t id; uint8_t capacity; uint16_t runtime; } PACKED ReportCapacityRuntime;

/* 2026-09-24 in HID battery date encoding: (year-1980)<<9 | month<<5 | day. */
#define UPS_MFR_DATE 0x5D38u

static Report8 rpt_ps_product = {RID_PS_IPRODUCT, 2};
static Report8 rpt_ps_serial = {RID_PS_ISERIAL, 3};
static Report8 rpt_ps_manufacturer = {RID_PS_IMANUFACTURER, 1};
static Report8 rpt_rechargeable = {RID_RECHARGEABLE, 1};
static Report3x8 rpt_direct_power = {RID_DIRECT_POWER, 1, 0, 8};
static Report16 rpt_ps_mfr_date = {RID_PS_MFR_DATE, UPS_MFR_DATE};
static Report16 rpt_ps_config_voltage = {RID_PS_CONFIG_VOLTAGE, 1200};
static Report16 rpt_ps_battery_voltage = {RID_PS_BATTERY_VOLTAGE, 1360};
static Report8 rpt_capacity_mode = {RID_CAPACITY_MODE, 2};
static ReportCapacityRuntime rpt_cap_runtime = {RID_CAP_RUNTIME, 80, 480};
static Report8 rpt_design_capacity = {RID_DESIGN_CAPACITY, 100};
static Report8 rpt_full_capacity = {RID_FULL_CHARGE_CAPACITY, 100};
static Report8 rpt_warning_limit = {RID_WARNING_CAPACITY_LIMIT, 20};
static Report8 rpt_capacity_granularity2 = {RID_CAPACITY_GRANULARITY2, 1};
static Report8 rpt_remaining_limit = {RID_REMAINING_CAPACITY_LIMIT, 10};
static Report8 rpt_capacity_granularity1 = {RID_CAPACITY_GRANULARITY1, 1};
static Report8 rpt_direct_ac = {RID_DIRECT_AC, 1};
static Report2x8 rpt_direct_low = {RID_DIRECT_LOW, 0, 0};
static Report32 rpt_status = {RID_PRESENT_STATUS, 0};
static Report16 rpt_remaining_time_limit = {RID_REMAINING_TIME_LIMIT, 120};
static Report8 rpt_audible = {RID_AUDIBLE_ALARM, 2};

static Report16 rpt_batt_mfr_date = {RID_BATTERY_MFR_DATE, UPS_MFR_DATE};
static Report8 rpt_batt_capacity = {RID_BATTERY_CAPACITY, 80};
static Report16 rpt_batt_runtime = {RID_BATTERY_RUNTIME, 480};
static Report16 rpt_batt_config_voltage = {RID_BATTERY_CONFIG_VOLTAGE, 1200};
static Report16 rpt_batt_voltage = {RID_BATTERY_VOLTAGE, 1360};
static Report16 rpt_batt_temperature = {RID_BATTERY_TEMPERATURE, 2982};

static Report8 rpt_input_config_voltage = {RID_INPUT_CONFIG_VOLTAGE, 230};
static Report16 rpt_input_voltage = {RID_INPUT_VOLTAGE, 230};
static Report16 rpt_input_frequency = {RID_INPUT_FREQUENCY, 50};
static Report16 rpt_output_config_voltage = {RID_OUTPUT_CONFIG_VOLTAGE, 230};
static Report16 rpt_output_voltage = {RID_OUTPUT_VOLTAGE, 230};
static Report16 rpt_output_frequency = {RID_OUTPUT_FREQUENCY, 50};
static Report8 rpt_output_load = {RID_OUTPUT_LOAD, 5};
static Report8 rpt_converter_load = {RID_POWERCONVERTER_LOAD, 5};

static Report8 rpt_root_product = {RID_ROOT_IPRODUCT, 2};
static Report8 rpt_root_serial = {RID_ROOT_ISERIAL, 3};
static Report8 rpt_root_manufacturer = {RID_ROOT_IMANUFACTURER, 1};
static Report16 rpt_root_mfr_date = {RID_ROOT_MFR_DATE, UPS_MFR_DATE};
static Report32 rpt_root_status = {RID_ROOT_STATUS, 0};
static Report8 rpt_root_audible = {RID_ROOT_AUDIBLE, 2};
static uint8_t rpt_idle = 0;

static uint8_t ups_effective_percent(const UpsAppState* s) {
    return s->mode == UpsModeFlipper ? s->flipper_battery_percent : s->sim_battery_percent;
}

static uint16_t ups_effective_voltage_x10(const UpsAppState* s) {
    if(s->mode == UpsModeFlipper) return s->flipper_voltage_x10;
    return s->mode == UpsModeBattery ? 122u : 136u;
}

static uint16_t ups_effective_voltage_x100(const UpsAppState* s) {
    return (uint16_t)(ups_effective_voltage_x10(s) * 10u);
}

static uint16_t ups_config_voltage_x100(const UpsAppState* s) {
    return s->mode == UpsModeFlipper ? 370u : 1200u;
}

static bool ups_is_low(const UpsAppState* s) { return s->fault == UpsFaultLow; }
static bool ups_is_replace(const UpsAppState* s) { return s->fault == UpsFaultReplace; }
static bool ups_is_overload(const UpsAppState* s) { return s->fault == UpsFaultOverload; }

static uint32_t ups_status_bits(const UpsAppState* s) {
    uint32_t bits = 0;
    uint8_t p = ups_effective_percent(s);
    bool battery_mode = s->mode == UpsModeBattery;
    bool ac_present = !battery_mode;

    /* Exact APC report 0x16 / 0x7A bit positions. */
    if(ac_present && p < 100) bits |= (1u << 0); /* Charging */
    if(battery_mode) bits |= (1u << 1);          /* Discharging */
    if(ac_present) bits |= (1u << 2);            /* AC Present */
    bits |= (1u << 3);                           /* Battery Present */
    if(ups_is_low(s)) {
        bits |= (1u << 4);                       /* BelowRemainingCapacityLimit */
        bits |= (1u << 5);                       /* ShutdownImminent */
        bits |= (1u << 6);                       /* RemainingTimeLimitExpired */
    }
    if(ups_is_replace(s)) bits |= (1u << 8);     /* NeedReplacement */
    if(ups_is_overload(s)) bits |= (1u << 9);    /* Overload */
    return bits;
}

static uint8_t ups_apc_status_flag(const UpsAppState* s) {
    /* APC's proprietary ff860060 report is 8 on normal mains operation and 16
     * has been observed with DISCHRG. For debug we also OR the classic APC
     * status bits for overload/low/replace to see whether RouterOS interprets them. */
    uint8_t q = (s->mode == UpsModeBattery) ? 0x10u : 0x08u;
    if(ups_is_overload(s)) q |= 0x20u;
    if(ups_is_low(s)) q |= 0x40u;
    if(ups_is_replace(s)) q |= 0x80u;
    return q;
}

static void ups_refresh_reports(void) {
    if(!g_app) return;
    uint8_t p = ups_effective_percent(g_app);
    uint16_t runtime = (uint16_t)p * 6u; /* 100% = 10 minutes */
    uint16_t bv100 = ups_effective_voltage_x100(g_app);
    uint16_t cv100 = ups_config_voltage_x100(g_app);
    bool battery_mode = g_app->mode == UpsModeBattery;
    bool ac_present = !battery_mode;
    bool charging = ac_present && p < 100;
    bool discharging = battery_mode;
    uint32_t status = ups_status_bits(g_app);

    rpt_direct_power.a = charging ? 1u : 0u;
    rpt_direct_power.b = discharging ? 1u : 0u;
    rpt_direct_power.c = ups_apc_status_flag(g_app);
    rpt_direct_ac.value = ac_present ? 1u : 0u;
    rpt_direct_low.a = ups_is_low(g_app) ? 1u : 0u;
    rpt_direct_low.b = ups_is_low(g_app) ? 1u : 0u;

    rpt_status.value = status;
    rpt_root_status.value = status;

    rpt_cap_runtime.capacity = p;
    rpt_cap_runtime.runtime = runtime;
    rpt_batt_capacity.value = p;
    rpt_batt_runtime.value = runtime;

    rpt_ps_config_voltage.value = cv100;
    rpt_ps_battery_voltage.value = bv100;
    rpt_batt_config_voltage.value = cv100;
    rpt_batt_voltage.value = bv100;

    /* AC side values. ON FLIPPER shows the USB-powered 5 V environment. */
    if(g_app->mode == UpsModeFlipper) {
        rpt_input_config_voltage.value = 5;
        rpt_input_voltage.value = 5;
        rpt_output_config_voltage.value = 5;
        rpt_output_voltage.value = 5;
        rpt_input_frequency.value = 0;
        rpt_output_frequency.value = 0;
        rpt_output_load.value = 5;
        rpt_converter_load.value = 5;
    } else {
        rpt_input_config_voltage.value = 230;
        rpt_input_voltage.value = battery_mode ? 0u : 230u;
        rpt_output_config_voltage.value = 230;
        rpt_output_voltage.value = 230;
        rpt_input_frequency.value = 50;
        rpt_output_frequency.value = 50;
        uint8_t load = ups_is_overload(g_app) ? 100u : 5u;
        rpt_output_load.value = load;
        rpt_converter_load.value = load;
    }
}

static void ups_send_report(const void* data, uint16_t size) {
    if(g_usb_dev && g_app && g_app->usb_configured) {
        (void)g_usb_dev->driver->ep_write(UPS_EP_IN, data, size);
    }
}

/*
 * Stable v9 report scheduler.
 * RouterOS v7 was verified to interpret PowerSummary PresentStatus (RID 0x16)
 * for On Line, On Battery, Low Battery, Replace Battery and Overload.
 * Keep endpoint cadence identical to the known-good v8 P16 test: one report
 * every 250 ms, alternating status and numeric telemetry.
 */
static void ups_send_next_dynamic_report(void) {
    static uint8_t slot = 0;
    ups_refresh_reports();

    /* After a state/fault change, prioritise the verified PresentStatus report. */
    if(g_app && g_app->force_status_reports) {
        ups_send_report(&rpt_status, sizeof(rpt_status));
        g_app->force_status_reports--;
        return;
    }

    if((slot & 1u) == 0u) {
        ups_send_report(&rpt_status, sizeof(rpt_status));
    } else {
        switch((slot >> 1u) & 3u) {
        case 0: ups_send_report(&rpt_cap_runtime, sizeof(rpt_cap_runtime)); break;
        case 1: ups_send_report(&rpt_batt_voltage, sizeof(rpt_batt_voltage)); break;
        case 2: ups_send_report(&rpt_output_load, sizeof(rpt_output_load)); break;
        default: ups_send_report(&rpt_converter_load, sizeof(rpt_converter_load)); break;
        }
    }
    slot = (uint8_t)((slot + 1u) & 7u);
}


/* ---------- USB callbacks ---------- */
static usbd_respond ups_ep_config(usbd_device* dev, uint8_t cfg) {
    if(cfg == 0) {
        dev->driver->ep_deconfig(UPS_EP_IN);
        dev->endpoint[UPS_EP_IN & 7u] = NULL;
        if(g_app) g_app->usb_configured = false;
        return usbd_ack;
    }
    if(cfg == 1) {
        if(!dev->driver->ep_config(UPS_EP_IN, USB_EPTYPE_INTERRUPT, UPS_EP_SZ)) return usbd_fail;
        dev->endpoint[UPS_EP_IN & 7u] = NULL;
        (void)dev->driver->ep_write(UPS_EP_IN, NULL, 0);
        if(g_app) g_app->usb_configured = true;
        return usbd_ack;
    }
    return usbd_fail;
}

static void ups_get_report_ptr(uint8_t id, void** ptr, uint16_t* size) {
    ups_refresh_reports();
    *ptr = NULL;
    *size = 0;
    switch(id) {
    case RID_PS_IPRODUCT: *ptr=&rpt_ps_product; *size=sizeof(rpt_ps_product); break;
    case RID_PS_ISERIAL: *ptr=&rpt_ps_serial; *size=sizeof(rpt_ps_serial); break;
    case RID_RECHARGEABLE: *ptr=&rpt_rechargeable; *size=sizeof(rpt_rechargeable); break;
    case RID_DIRECT_POWER: *ptr=&rpt_direct_power; *size=sizeof(rpt_direct_power); break;
    case RID_PS_MFR_DATE: *ptr=&rpt_ps_mfr_date; *size=sizeof(rpt_ps_mfr_date); break;
    case RID_PS_CONFIG_VOLTAGE: *ptr=&rpt_ps_config_voltage; *size=sizeof(rpt_ps_config_voltage); break;
    case RID_PS_BATTERY_VOLTAGE: *ptr=&rpt_ps_battery_voltage; *size=sizeof(rpt_ps_battery_voltage); break;
    case RID_PS_IMANUFACTURER: *ptr=&rpt_ps_manufacturer; *size=sizeof(rpt_ps_manufacturer); break;
    case RID_CAPACITY_MODE: *ptr=&rpt_capacity_mode; *size=sizeof(rpt_capacity_mode); break;
    case RID_CAP_RUNTIME: *ptr=&rpt_cap_runtime; *size=sizeof(rpt_cap_runtime); break;
    case RID_DESIGN_CAPACITY: *ptr=&rpt_design_capacity; *size=sizeof(rpt_design_capacity); break;
    case RID_FULL_CHARGE_CAPACITY: *ptr=&rpt_full_capacity; *size=sizeof(rpt_full_capacity); break;
    case RID_WARNING_CAPACITY_LIMIT: *ptr=&rpt_warning_limit; *size=sizeof(rpt_warning_limit); break;
    case RID_CAPACITY_GRANULARITY2: *ptr=&rpt_capacity_granularity2; *size=sizeof(rpt_capacity_granularity2); break;
    case RID_REMAINING_CAPACITY_LIMIT: *ptr=&rpt_remaining_limit; *size=sizeof(rpt_remaining_limit); break;
    case RID_CAPACITY_GRANULARITY1: *ptr=&rpt_capacity_granularity1; *size=sizeof(rpt_capacity_granularity1); break;
    case RID_DIRECT_AC: *ptr=&rpt_direct_ac; *size=sizeof(rpt_direct_ac); break;
    case RID_DIRECT_LOW: *ptr=&rpt_direct_low; *size=sizeof(rpt_direct_low); break;
    case RID_PRESENT_STATUS: *ptr=&rpt_status; *size=sizeof(rpt_status); break;
    case RID_REMAINING_TIME_LIMIT: *ptr=&rpt_remaining_time_limit; *size=sizeof(rpt_remaining_time_limit); break;
    case RID_AUDIBLE_ALARM: *ptr=&rpt_audible; *size=sizeof(rpt_audible); break;

    case RID_BATTERY_MFR_DATE: *ptr=&rpt_batt_mfr_date; *size=sizeof(rpt_batt_mfr_date); break;
    case RID_BATTERY_CAPACITY: *ptr=&rpt_batt_capacity; *size=sizeof(rpt_batt_capacity); break;
    case RID_BATTERY_RUNTIME: *ptr=&rpt_batt_runtime; *size=sizeof(rpt_batt_runtime); break;
    case RID_BATTERY_CONFIG_VOLTAGE: *ptr=&rpt_batt_config_voltage; *size=sizeof(rpt_batt_config_voltage); break;
    case RID_BATTERY_VOLTAGE: *ptr=&rpt_batt_voltage; *size=sizeof(rpt_batt_voltage); break;
    case RID_BATTERY_TEMPERATURE: *ptr=&rpt_batt_temperature; *size=sizeof(rpt_batt_temperature); break;

    case RID_INPUT_CONFIG_VOLTAGE: *ptr=&rpt_input_config_voltage; *size=sizeof(rpt_input_config_voltage); break;
    case RID_INPUT_VOLTAGE: *ptr=&rpt_input_voltage; *size=sizeof(rpt_input_voltage); break;
    case RID_INPUT_FREQUENCY: *ptr=&rpt_input_frequency; *size=sizeof(rpt_input_frequency); break;
    case RID_OUTPUT_CONFIG_VOLTAGE: *ptr=&rpt_output_config_voltage; *size=sizeof(rpt_output_config_voltage); break;
    case RID_OUTPUT_VOLTAGE: *ptr=&rpt_output_voltage; *size=sizeof(rpt_output_voltage); break;
    case RID_OUTPUT_FREQUENCY: *ptr=&rpt_output_frequency; *size=sizeof(rpt_output_frequency); break;
    case RID_OUTPUT_LOAD: *ptr=&rpt_output_load; *size=sizeof(rpt_output_load); break;
    case RID_POWERCONVERTER_LOAD: *ptr=&rpt_converter_load; *size=sizeof(rpt_converter_load); break;

    case RID_ROOT_IPRODUCT: *ptr=&rpt_root_product; *size=sizeof(rpt_root_product); break;
    case RID_ROOT_ISERIAL: *ptr=&rpt_root_serial; *size=sizeof(rpt_root_serial); break;
    case RID_ROOT_IMANUFACTURER: *ptr=&rpt_root_manufacturer; *size=sizeof(rpt_root_manufacturer); break;
    case RID_ROOT_MFR_DATE: *ptr=&rpt_root_mfr_date; *size=sizeof(rpt_root_mfr_date); break;
    case RID_ROOT_STATUS: *ptr=&rpt_root_status; *size=sizeof(rpt_root_status); break;
    case RID_ROOT_AUDIBLE: *ptr=&rpt_root_audible; *size=sizeof(rpt_root_audible); break;
    default: break;
    }
}

static usbd_respond ups_control(usbd_device* dev, usbd_ctlreq* req, usbd_rqc_callback* callback) {
    (void)callback;
    uint8_t recipient_type = req->bmRequestType & (USB_REQ_RECIPIENT | USB_REQ_TYPE);

    if(recipient_type == (USB_REQ_INTERFACE | USB_REQ_CLASS) && req->wIndex == 0) {
        switch(req->bRequest) {
        case USB_HID_GETREPORT: {
            void* ptr;
            uint16_t size;
            uint8_t report_id = (uint8_t)(req->wValue & 0xFFu);
            ups_get_report_ptr(report_id, &ptr, &size);
            if(!ptr) return usbd_fail;
            dev->status.data_ptr = ptr;
            dev->status.data_count = size;
            return usbd_ack;
        }
        case USB_HID_SETREPORT: {
            uint8_t report_id = (uint8_t)(req->wValue & 0xFFu);
            /* RouterOS Beep writes AudibleAlarmControl. Defer sound to the main loop. */
            if((report_id == RID_AUDIBLE_ALARM || report_id == RID_ROOT_AUDIBLE) && g_app) g_app->beep_requested = true;
            return usbd_ack;
        }
        case USB_HID_SETIDLE:
            return usbd_ack;
        case USB_HID_GETIDLE:
            dev->status.data_ptr = &rpt_idle;
            dev->status.data_count = 1;
            return usbd_ack;
        default:
            return usbd_fail;
        }
    }

    if(recipient_type == (USB_REQ_INTERFACE | USB_REQ_STANDARD) && req->wIndex == 0 &&
       req->bRequest == USB_STD_GET_DESCRIPTOR) {
        switch(req->wValue >> 8) {
        case USB_DTYPE_HID:
            dev->status.data_ptr = (void*)&ups_cfg_desc.intf_0.hid_desc;
            dev->status.data_count = sizeof(ups_cfg_desc.intf_0.hid_desc);
            return usbd_ack;
        case USB_DTYPE_HID_REPORT:
            dev->status.data_ptr = (void*)ups_report_desc;
            dev->status.data_count = sizeof(ups_report_desc);
            return usbd_ack;
        default:
            return usbd_fail;
        }
    }

    return usbd_fail;
}

static void ups_usb_init(usbd_device* dev, FuriHalUsbInterface* intf, void* ctx) {
    (void)intf;
    g_usb_dev = dev;
    g_app = (UpsAppState*)ctx;
    dev->config_callback = ups_ep_config;
    dev->control_callback = ups_control;
    dev->driver->connect(true);
}

static void ups_usb_deinit(usbd_device* dev) {
    dev->config_callback = NULL;
    dev->control_callback = NULL;
    if(g_app) g_app->usb_configured = false;
    g_usb_dev = NULL;
}

static void ups_usb_wakeup(usbd_device* dev) { (void)dev; }
static void ups_usb_suspend(usbd_device* dev) { (void)dev; }

static FuriHalUsbInterface usb_ups = {
    .init = ups_usb_init,
    .deinit = ups_usb_deinit,
    .wakeup = ups_usb_wakeup,
    .suspend = ups_usb_suspend,
    .dev_descr = &ups_device_desc,
    .str_manuf_descr = (void*)&ups_manufacturer_str,
    .str_prod_descr = (void*)&ups_product_str,
    .str_serial_descr = (void*)&ups_serial_str,
    .cfg_descr = (void*)&ups_cfg_desc,
};

/* ---------- UI / LEDs / sound ---------- */

/* Dim green is deliberately non-zero so the system red charging LED never shines through. */
static const NotificationMessage ups_led_red_0 = {
    .type = NotificationMessageTypeLedRedCompat,
    .data.led = {.value = 0},
};
static const NotificationMessage ups_led_green_16 = {
    .type = NotificationMessageTypeLedGreenCompat,
    .data.led = {.value = 16},
};
static const NotificationMessage ups_led_blue_0 = {
    .type = NotificationMessageTypeLedBlueCompat,
    .data.led = {.value = 0},
};
static const NotificationSequence ups_dim_green_sequence = {
    &ups_led_red_0,
    &ups_led_green_16,
    &ups_led_blue_0,
    &message_do_not_reset,
    NULL,
};

static const NotificationSequence ups_magenta_sequence = {
    &message_red_255,
    &message_green_0,
    &message_blue_255,
    &message_do_not_reset,
    NULL,
};

static const NotificationSequence ups_red_sequence = {
    &message_red_255,
    &message_green_0,
    &message_do_not_reset,
    NULL,
};

static const NotificationSequence ups_beep_sequence = {
    &message_note_c6,
    &message_delay_100,
    &message_sound_off,
    NULL,
};

enum {
    UpsLedUnknown = 0,
    UpsLedOff = 1,
    UpsLedRed = 2,
    UpsLedGreen = 3,
    UpsLedGreenDim = 4,
    UpsLedMagenta = 5,
};

static void ups_set_led(UpsAppState* s, uint8_t led) {
    if(!s || !s->notification || s->led_applied == led) return;
    s->led_applied = led;
    notification_message(s->notification, &sequence_blink_stop);
    if(led == UpsLedOff) notification_message(s->notification, &sequence_reset_rgb);
    else if(led == UpsLedRed) notification_message(s->notification, &ups_red_sequence);
    else if(led == UpsLedGreen) notification_message(s->notification, &sequence_set_only_green_255);
    else if(led == UpsLedGreenDim) notification_message(s->notification, &ups_dim_green_sequence);
    else if(led == UpsLedMagenta) notification_message(s->notification, &ups_magenta_sequence);
}

static void ups_update_led(UpsAppState* s) {
    if(!s || !s->notification) return;
    if(!s->usb_switch_ok) {
        ups_set_led(s, UpsLedRed);
        return;
    }
    if(!s->usb_configured) {
        ups_set_led(s, UpsLedOff);
        return;
    }

    if(s->mode == UpsModeOnline) {
        ups_set_led(s, UpsLedGreen);
    } else if(s->mode == UpsModeFlipper) {
        ups_set_led(s, UpsLedMagenta);
    } else {
        /* BATTERY: 1 second = ten 100 ms slots. Battery percentage is represented
         * as strong-green duty cycle, with dim green for the remainder.
         * 90% => 900 ms strong + 100 ms dim; 20% => 200 ms strong + 800 ms dim. */
        uint8_t p = ups_effective_percent(s);
        uint8_t full_slots = (uint8_t)((p + 5u) / 10u);
        if(full_slots > 10u) full_slots = 10u;
        uint8_t phase = (uint8_t)(s->blink_ticks % 10u);
        ups_set_led(s, phase < full_slots ? UpsLedGreen : UpsLedGreenDim);
        s->blink_ticks = (uint8_t)((phase + 1u) % 10u);
    }
}

static void percent_string(char out[5], uint8_t value) {
    if(value >= 100) {
        out[0] = '1'; out[1] = '0'; out[2] = '0'; out[3] = '%'; out[4] = '\0';
    } else if(value >= 10) {
        out[0] = (char)('0' + value / 10);
        out[1] = (char)('0' + value % 10);
        out[2] = '%'; out[3] = '\0'; out[4] = '\0';
    } else {
        out[0] = (char)('0' + value);
        out[1] = '%'; out[2] = '\0'; out[3] = '\0'; out[4] = '\0';
    }
}

static void voltage_string(char out[7], uint16_t value_x10) {
    uint16_t whole = value_x10 / 10u;
    uint8_t frac = (uint8_t)(value_x10 % 10u);
    uint8_t i = 0;
    if(whole >= 10u) out[i++] = (char)('0' + ((whole / 10u) % 10u));
    out[i++] = (char)('0' + (whole % 10u));
    out[i++] = '.';
    out[i++] = (char)('0' + frac);
    out[i++] = 'V';
    out[i] = '\0';
}

static void runtime_string(char out[7], uint8_t p) {
    uint16_t sec = (uint16_t)p * 6u;
    uint8_t min = (uint8_t)(sec / 60u);
    uint8_t rem = (uint8_t)(sec % 60u);
    uint8_t i = 0;
    if(min >= 10) out[i++] = (char)('0' + min / 10u);
    out[i++] = (char)('0' + min % 10u);
    out[i++] = ':';
    out[i++] = (char)('0' + rem / 10u);
    out[i++] = (char)('0' + rem % 10u);
    out[i] = '\0';
}

static void draw_left_arrow(Canvas* c, int32_t x, int32_t y) {
    canvas_draw_line(c, x + 5, y, x, y);
    canvas_draw_line(c, x, y, x + 2, y - 2);
    canvas_draw_line(c, x, y, x + 2, y + 2);
}

static void draw_right_arrow(Canvas* c, int32_t x, int32_t y) {
    canvas_draw_line(c, x, y, x + 5, y);
    canvas_draw_line(c, x + 5, y, x + 3, y - 2);
    canvas_draw_line(c, x + 5, y, x + 3, y + 2);
}

static void draw_up_arrow(Canvas* c, int32_t x, int32_t y) {
    canvas_draw_line(c, x, y + 4, x, y - 1);
    canvas_draw_line(c, x, y - 1, x - 2, y + 1);
    canvas_draw_line(c, x, y - 1, x + 2, y + 1);
}

static void draw_down_arrow(Canvas* c, int32_t x, int32_t y) {
    canvas_draw_line(c, x, y - 1, x, y + 4);
    canvas_draw_line(c, x, y + 4, x - 2, y + 2);
    canvas_draw_line(c, x, y + 4, x + 2, y + 2);
}

static const char* ups_mode_name(UpsMode mode) {
    if(mode == UpsModeBattery) return "BATTERY";
    if(mode == UpsModeFlipper) return "ON FLIPPER";
    return "ONLINE";
}

static const char* ups_fault_name(UpsFault fault) {
    if(fault == UpsFaultLow) return "LOW";
    if(fault == UpsFaultReplace) return "REPL";
    if(fault == UpsFaultOverload) return "OVRL";
    return "OK";
}



static void draw_battery_icon(Canvas* c, int32_t x, int32_t y, UpsMode mode) {
    canvas_draw_frame(c, x, y, 15, 8);
    canvas_draw_line(c, x + 15, y + 2, x + 15, y + 5);

    if(mode == UpsModeFlipper) {
        /* Tiny F centered inside the battery. */
        canvas_draw_line(c, x + 5, y + 2, x + 5, y + 6);
        canvas_draw_line(c, x + 5, y + 2, x + 9, y + 2);
        canvas_draw_line(c, x + 5, y + 4, x + 8, y + 4);
        return;
    }

    uint8_t bars = mode == UpsModeOnline ? 10u : 5u;
    for(uint8_t i = 0; i < bars; i++) {
        canvas_draw_line(c, x + 2 + i, y + 2, x + 2 + i, y + 5);
    }
}

static void ups_draw(Canvas* canvas, void* context) {
    UpsAppState* s = (UpsAppState*)context;
    uint8_t p = ups_effective_percent(s);
    uint16_t vx10 = ups_effective_voltage_x10(s);
    char pct[5];
    char volt[7];
    char runtime[7];
    percent_string(pct, p);
    voltage_string(volt, vx10);
    runtime_string(runtime, p);

    canvas_clear(canvas);
    canvas_draw_frame(canvas, 0, 0, 128, 64);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 4, 10, "UPS Emulator");
    draw_battery_icon(canvas, 108, 2, s->mode);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 4, 20,
        s->usb_switch_ok ? (s->usb_configured ? "USB: connected" : "USB: waiting host") : "USB: CONFIG FAILED");

    draw_left_arrow(canvas, 4, 31);
    draw_right_arrow(canvas, 119, 31);
    canvas_draw_str(canvas, 14, 34, "State:");
    canvas_draw_str(canvas, 47, 34, ups_mode_name(s->mode));

    canvas_draw_str(canvas, 4, 45, "Battery:");
    canvas_draw_str(canvas, 44, 45, pct);
    canvas_draw_str(canvas, 70, 45, volt);
    if(s->mode != UpsModeFlipper) {
        draw_up_arrow(canvas, 120, 40);
        draw_down_arrow(canvas, 120, 47);
    }

    canvas_draw_str(canvas, 4, 56, "Runtime:");
    canvas_draw_str(canvas, 45, 56, runtime);
    canvas_draw_str(canvas, 80, 56, ups_fault_name(s->fault));

    canvas_draw_str(canvas, 4, 63, "OK=Status");
    canvas_draw_str(canvas, 75, 63, "Back=Exit");
}

static void ups_input(void* input_event_void, void* context) {
    InputEvent* event = (InputEvent*)input_event_void;
    UpsAppState* s = (UpsAppState*)context;

    if(event->type != InputTypeShort && event->type != InputTypeRepeat) return;
    /* Fault cycling is deliberately short-OK only. */
    if(event->key == InputKeyOk && event->type != InputTypeShort) return;

    UpsMode old_mode = s->mode;
    switch(event->key) {
    case InputKeyLeft:
        if(s->mode == UpsModeOnline) s->mode = UpsModeFlipper;
        else s->mode = (UpsMode)((int)s->mode - 1);
        break;
    case InputKeyRight:
        if(s->mode == UpsModeFlipper) s->mode = UpsModeOnline;
        else s->mode = (UpsMode)((int)s->mode + 1);
        break;
    case InputKeyUp:
        if(s->mode != UpsModeFlipper) {
            if(s->sim_battery_percent <= 95) s->sim_battery_percent += 5;
            else s->sim_battery_percent = 100;
        }
        break;
    case InputKeyDown:
        if(s->mode != UpsModeFlipper) {
            if(s->sim_battery_percent >= 5) s->sim_battery_percent -= 5;
            else s->sim_battery_percent = 0;
        }
        break;
    case InputKeyOk:
        s->fault = (UpsFault)(((int)s->fault + 1) & 3);
        break;
    case InputKeyBack:
        s->running = false;
        break;
    default:
        break;
    }

    if(s->mode != old_mode) {
        s->force_status_reports = 20;
        s->blink_ticks = 0;
        s->blink_on = true;
        s->led_applied = UpsLedUnknown;
    } else if(event->key == InputKeyUp || event->key == InputKeyDown || event->key == InputKeyOk) {
        s->force_status_reports = 20;
    }

    if(s->view_port) view_port_update(s->view_port);
}

/* ---------- FAP entry ---------- */
int32_t ups_emulator_app(void* p) {
    (void)p;

    UpsAppState state = {
        .running = true,
        .mode = UpsModeOnline,
        .fault = UpsFaultOk,
        .sim_battery_percent = 80,
        .flipper_battery_percent = 0,
        .flipper_voltage_x10 = 0,
        .led_applied = UpsLedUnknown,
        .blink_ticks = 0,
        .blink_on = true,
        .usb_configured = false,
        .usb_switch_ok = false,
        .beep_requested = false,
        .force_status_reports = 20,
        .view_port = NULL,
        .notification = NULL,
    };
    g_app = &state;

    state.flipper_battery_percent = furi_hal_power_get_pct();
    float initial_v = furi_hal_power_get_battery_voltage(FuriHalPowerICFuelGauge);
    state.flipper_voltage_x10 = (uint16_t)(initial_v * 10.0f + 0.5f);

    Gui* gui = (Gui*)furi_record_open("gui");
    state.notification = (NotificationApp*)furi_record_open("notification");
    ViewPort* vp = view_port_alloc();
    state.view_port = vp;
    view_port_draw_callback_set(vp, ups_draw, &state);
    view_port_input_callback_set(vp, ups_input, &state);
    gui_add_view_port(gui, vp, GuiLayerFullscreen);

    /* No LED before USB state is known. */
    if(state.notification) notification_message(state.notification, &sequence_reset_rgb);

    FuriHalUsbInterface* old_usb = furi_hal_usb_get_config();
    state.usb_switch_ok = furi_hal_usb_set_config(&usb_ups, &state);
    state.led_applied = UpsLedUnknown;
    ups_update_led(&state);
    view_port_update(vp);

    uint8_t refresh_div = 0;
    uint8_t led_div = 0;
    uint8_t usb_div = 0;
    while(state.running) {
        /* Real Flipper telemetry once per second. */
        if(++refresh_div >= 20u) {
            refresh_div = 0;
            state.flipper_battery_percent = furi_hal_power_get_pct();
            float v = furi_hal_power_get_battery_voltage(FuriHalPowerICFuelGauge);
            state.flipper_voltage_x10 = (uint16_t)(v * 10.0f + 0.5f);
            if(state.mode == UpsModeFlipper && state.view_port) view_port_update(state.view_port);
        }

        if(state.beep_requested) {
            state.beep_requested = false;
            if(state.notification) notification_message(state.notification, &ups_beep_sequence);
        }

        /* LED state/duty cycle at 100 ms resolution. */
        if(++led_div >= 2u) {
            led_div = 0;
            ups_update_led(&state);
        }

        /* Preserve v5 USB event cadence exactly: one report every 250 ms. */
        if(++usb_div >= 5u) {
            usb_div = 0;
            if(state.usb_switch_ok && state.usb_configured) ups_send_next_dynamic_report();
        }

        furi_delay_ms(50);
    }

    if(state.usb_switch_ok && old_usb) {
        (void)furi_hal_usb_set_config(old_usb, NULL);
    }

    if(state.notification) {
        notification_message(state.notification, &sequence_blink_stop);
        notification_message(state.notification, &sequence_reset_rgb);
    }

    gui_remove_view_port(gui, vp);
    view_port_free(vp);
    if(state.notification) furi_record_close("notification");
    furi_record_close("gui");
    g_app = NULL;
    return 0;
}
