/*
 * Kneron USB Communication driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#include "host_usb_com.h"

#include "kneron_mozart.h"
#include "scu_extreg.h"
//#include "kmdw_console.h"
#include "kdp520_usbd.h"
#include "board_cfg.h"


// USB I/O control
#define USB_HOST_IN 0x02  // EP - bulk-out
#define USB_HOST_OUT 0x81 // EP - bulk-in
#define USB_AUX_IN  0x04  // EP - bulk-out
#define USB_AUX_OUT 0x83  // EP - interrupt-out

#define FLAG_USB_COMM_ISR           BIT(1)
#define FLAG_USB_COMM_WRITE         BIT(2)
#define FLAG_USB_COMM_READ          BIT(3)
#define FLAG_USB_COMM_EVENT         BIT(4)

#define FLAG_USB_COMM_ALL          (FLAG_USB_COMM_EVENT)

osThreadId_t tid_usb_comm;
static osThreadId_t tid_to_notify;
static uint32_t flags_to_notify;
volatile int usb_mode = 0, usb_read_count = 0;

unsigned int FOTG200_BASE_ADDRESS = USB_FOTG210_PA_BASE;
unsigned int OTG_BASE_ADDRESS = USB_FOTG210_PA_BASE;

// endpoint 0x81, bulk-in
kdrv_usbd_endpoint_descriptor_t endp_bulkIn_81_desc =
{
    .bLength = 0x07,            // 7 bytes
    .bDescriptorType = 0x05,    // Endpoint Descriptor
    .bEndpointAddress = 0x81,   // Direction=IN EndpointID=1
    .bmAttributes = 0x02,       // TransferType = Bulk
    .wMaxPacketSize = 0x0200,   // max 512 bytes
    .bInterval = 0x00,          // never NAKs
};

// endpoint 0x02, bulk-out
kdrv_usbd_endpoint_descriptor_t endp_bulkOut_02_desc =
{
    .bLength = 0x07,            // 7 bytes
    .bDescriptorType = 0x05,    // Endpoint Descriptor
    .bEndpointAddress = 0x02,   // Direction=OUT EndpointID=2
    .bmAttributes = 0x02,       // TransferType = Bulk
    .wMaxPacketSize = 0x0200,   // max 512 bytes
    .bInterval = 0x00,          // never NAKs
};

// endpoint 0x83, interrupt-in
kdrv_usbd_endpoint_descriptor_t endp_intIn_83_desc =
{
    .bLength = 0x07,            // 7 bytes
    .bDescriptorType = 0x05,    // Endpoint Descriptor
    .bEndpointAddress = 0x83,   // Direction=IN EndpointID=3
#if 1
    .bmAttributes = 0x03,       // TransferType = Interrupt
    .wMaxPacketSize = 0x0400,   // max 1024 bytes
    .bInterval = 0x64,          // interval = 100 * frame period (125us) = 12 ms
#else
    .bmAttributes = 0x02,       // TransferType = Bulk  // changed from 0x03 Interrupt
    .wMaxPacketSize = 0x0200,   // max 512 bytes // changed from 1024 bytes
    .bInterval = 0x00,          // never NAKs
#endif
};

// endpoint 0x04, bulk-out
kdrv_usbd_endpoint_descriptor_t endp_bulkOut_04_desc =
{
    .bLength = 0x07,            // 7 bytes
    .bDescriptorType = 0x05,    // Endpoint Descriptor
    .bEndpointAddress = 0x04,   // Direction=OUT EndpointID=4
    .bmAttributes = 0x02,       // TransferType = Bulk
    .wMaxPacketSize = 0x0200,   // max 512 bytes
    .bInterval = 0x00,          // never NAKs
};

kdrv_usbd_interface_descriptor_t intf_desc =
{
    .bLength = 0x9,             // 9 bytes
    .bDescriptorType = 0x04,    // Inteface Descriptor
    .bInterfaceNumber = 0x0,    // Interface Number
    .bAlternateSetting = 0x0,
    .bNumEndpoints = 0x4,       // 4 endpoints
    .bInterfaceClass = 0xFF,    // Vendor specific
    .bInterfaceSubClass = 0x0,  // 3rd party uses 0x01
    .bInterfaceProtocol = 0x0,  // 3rd party uses 0x50
    .iInterface = 0x0,          // No String Descriptor
    .endpoint[0] = &endp_bulkIn_81_desc,
    .endpoint[1] = &endp_bulkOut_02_desc,
    .endpoint[2] = &endp_intIn_83_desc,
    .endpoint[3] = &endp_bulkOut_04_desc,
};


kdrv_usbd_prd_string_descriptor_t str_desc_mun =
{
    .bLength = 14,
    .bDescriptorType=0x03,
    .bString=CFG_USB_MANUFACTURER,
};
kdrv_usbd_prd_string_descriptor_t str_desc_prd =
{
    .bLength = 16,
    .bDescriptorType=0x03,
    .bString=CFG_USB_PRODUCT,
};
kdrv_usbd_prd_string_descriptor_t str_desc_serial =
{
    .bLength = 18,
    .bDescriptorType=0x03,
    .bString=CFG_USB_SERIAL,
};
kdrv_usbd_string_descriptor_t str_desc =
{
    .bLength = 4,
    .bDescriptorType=0x03,
    .bLanguageID=0x0409,
    .desc[0]=&str_desc_mun,
    .desc[1]=&str_desc_prd,
    .desc[2]=&str_desc_serial,
};

kdrv_usbd_config_descriptor_t confg_desc =
{
    .bLength = 0x09,            // 9 bytes
    .bDescriptorType = 0x02,    // Type: Configuration Descriptor
    .wTotalLength = 0x2E,       // 46 bytes, total bytes including config/interface/endpoint descriptors
    .bNumInterfaces = 0x1,      // Number of interfaces
    .bConfigurationValue = 0x1, // Configuration number
    .iConfiguration = 0x0,      // No String Descriptor
    .bmAttributes = 0xC0,       // Self-powered, no Remote wakeup
    .MaxPower = 0x0,            // 0 syould be ok for self-powered device
    .interface[0] = &intf_desc,
};

kdrv_usbd_device_descriptor_t dev_desc =
{
    .bLength = 0x12,                // 18 bytes
    .bDescriptorType = 0x01,        // Type : Device Descriptor
    .bcdUSB = 0x200,              // USB 2.0
    .bDeviceClass = 0x00,        // Device class, 0x0: defined by the interface descriptors
    .bDeviceSubClass = 0x00,     // Device sub-class
    .bDeviceProtocol = 0x00,     // Device protocol
    .bMaxPacketSize0 = 0x40,     // Max EP0 packet size: 64 bytes
    .idVendor = 0x3231,          // Vendor ID
    .idProduct = 0x0100,         // Product ID
    .bcdDevice = 0x0001,         // Device release number
    .iManufacturer = 0x01,        // Manufacture string index, FIXME
    .iProduct = 0x02,             // Product string index, FIXME
    .iSerialNumber = 0x03,        // Serial number string index
    .bNumConfigurations = 1,     // Number of configurations, FIXME
    .config[0] = &confg_desc,   // configuration descriptor
};

// Device Qualifier Descriptor, plz see USB 2.0 SPEC
kdrv_usbd_device_qualifier_descriptor_t dev_qual_desc =
{
    .bLength = 0xA,
    .bDescriptorType = 0x06,
    .bcdUSB = 0x200,
    .bDeviceClass = 0x0,
    .bDeviceSubClass = 0x0,
    .bDeviceProtocol = 0x0,
    .bMaxPacketSize0 = 0x40,
    .bNumConfigurations = 0x1,
    .bReserved = 0x0,
};


/* ############################
 * ##    static functions    ##
 * ############################ */

static void usb_done_notify(void)
{
    osThreadFlagsSet(tid_to_notify, flags_to_notify);
}

static void usb_init_dev(void)
{
    kdrv_usbd_initialize();

    kdrv_usbd_set_device_descriptor(KDRV_USBD_HIGH_SPEED, &dev_desc);
    kdrv_usbd_set_string_descriptor(&str_desc);
    kdrv_usbd_set_device_qualifier_descriptor(KDRV_USBD_HIGH_SPEED, &dev_qual_desc);
    kdrv_usbd_set_enable(true);
    kdrv_usbd_register_thread_notification(tid_usb_comm, FLAG_USB_COMM_EVENT);
}

static void usb_comm_thread(void *argument)
{
    int32_t flags;
    kdrv_usbd_event_t usbevent;
    kdrv_status_t usb_status;

    while (1) {
        flags = osThreadFlagsWait(FLAG_USB_COMM_EVENT, osFlagsWaitAny, osWaitForever);
        osThreadFlagsClear(flags);

        if (flags & FLAG_USB_COMM_EVENT) {
            while(KDRV_STATUS_OK == kdrv_usbd_get_event(&usbevent)) {  // process all current events
                dbg_msg("[usb] event = [%d] %x %d, mode %d\r\n", usbevent.ename, usbevent.data1, usbevent.data2, usb_mode);

                switch (usb_mode)
                {
                    case 0:  // init
                        if (usbevent.ename  ==  KDRV_USBD_EVENT_DEV_CONFIGURED) usb_mode = 1;  // after init, go to idle
                        break;
                    case 1:  // idle
                        if (usbevent.ename  ==  KDRV_USBD_EVENT_TRANSFER_OUT) {
                            usb_mode = 2;  // go to message input mode
                            usb_status = kdrv_usbd_bulk_receive_async(USB_HOST_IN, (u32*) msg_rbuf, MSG_DATA_BUF_MAX + sizeof(MsgHdr)+ sizeof(RspPram) + 4);
                            if (usb_status != KDRV_STATUS_OK) {
                                err_msg("[usb] receive cmd ERROR %d\r\n", usb_status);
                                kdrv_usbd_reset_endpoint(USB_HOST_IN);
                                usb_mode = 1;  // stay at idle
                            }
                        }
						//patch: usb queue not in control
                        if (usbevent.ename == KDRV_USBD_EVENT_TRANSFER_DONE) {
                            if (usbevent.data1 == USB_HOST_OUT) {  //done with reply message
                            	usb_done_notify();
                                usb_mode = 1;  // ready to send reply
                            }
                        }
                        break;
                    case 2:  // message in
                        if (usbevent.ename == KDRV_USBD_EVENT_TRANSFER_DONE) {
                            if (usbevent.data1 == USB_HOST_IN) {  // done with command message
                                if (usbevent.data2) {
                                    usb_done_notify();
                                    usb_mode = 3;  // ready to send reply
                                }
                                else { // empty msg
                                    usb_mode = 1;  // ignore the message ang back to idle
                                }
                            }
                        }
                        break;
					case 3:  // message out
						if (usbevent.ename == KDRV_USBD_EVENT_TRANSFER_DONE) {
							if (usbevent.data1 == USB_HOST_OUT) { // done with reply message
								usb_mode = 1;  // back to idle
							} else {
								err_msg("USB mode 3, EP = %d\n", usbevent.data1);
								usb_mode = 1;  // back to idle
							}
							break;
						}
						err_msg("Mode 3 exception, EVNT = %d [%d]\n", usbevent.ename, usbevent.data1);
						if (usbevent.ename	==	KDRV_USBD_EVENT_TRANSFER_OUT) {  // a new command from host ?
							kdrv_status_t status;
							//status = kdrv_usbd_reset_endpoint(USB_HOST_IN);  // terminate current outgoing message
							status = KDRV_STATUS_OK;
							if (status == KDRV_STATUS_OK) {
								usb_status = kdrv_usbd_bulk_receive_async(USB_HOST_IN, (u32*) msg_rbuf, MSG_DATA_BUF_MAX + sizeof(MsgHdr)+ sizeof(RspPram) + 4);
								if (usb_status == KDRV_STATUS_OK)
									usb_mode = 2;  // go receive the incoming message
								else
									usb_mode = 1;
							}
							else {
								err_msg("[usb_com] Error in usb reset [%d]\r\n", status);
								usb_mode = 1;
							}
						}
						if (usbevent.ename	==	KDRV_USBD_EVENT_DEV_CONFIGURED)
							usb_mode = 1;  // got an init, go to idle
						break;

                    case 4:  // ack out  >>>> NOTE: data I/O is separated into two modes to avoid race conditions
                        if (usbevent.ename == KDRV_USBD_EVENT_TRANSFER_DONE) {  // ack tx done
                            if (usbevent.data1 == USB_HOST_OUT) { // done with ack output
                                usb_done_notify();
                            }
                        }
                        break;
                    case 5:  // image in
                        if (usbevent.ename == KDRV_USBD_EVENT_TRANSFER_DONE) {  // image rcv done
                            if (usbevent.data1 == USB_HOST_IN) {  // done with image in
                                usb_read_count = usbevent.data2;
                                if (usbevent.data2 == 0) {
                                    err_msg("[usb] ERROR, no data received.\r\n");
                                    usb_mode = 2;
                                }
                                usb_done_notify();
                            }
                        }
                        break;
                    default:  // ignore
                        break;
                }
            }
        }
    }
}

/* ############################
 * ##    public functions    ##
 * ############################ */

void usb_idle(void)
{
    usb_mode = 1;
}

int usb_get_mode(void)
{
    return usb_mode;
}

void usb_set_notify_flag(osThreadId_t tid, uint32_t flags)
{
    tid_to_notify = tid;
    flags_to_notify = flags;
}

/* send usb msg out
*  mode = 0 -> regular response message, send immediately
*  mode = 1 -> ack packet, send immediately
*  mode = 2 -> FR data, send only after response message is done
*/
int usb_com_write(uint8_t *BufferPtr, uint32_t BufferLen, uint32_t flags, uint32_t mode)
{
    usb_set_notify_flag(osThreadGetId(), flags);

    if (mode == 1) {
        usb_mode = 4;  // tell usb to wait for data tx done before next step
    } else if (mode == 2) { // alternate data mode ?
        if (usb_mode == 3) {  // still tx msg ?
            usb_mode = 4;
            if (usb_mode == 4) { // debounce in case of context switching
                osThreadFlagsWait(flags, osFlagsWaitAll, osWaitForever);
            }
        }
        usb_mode = 3;
    }

    kdrv_status_t status = kdrv_usbd_bulk_send_async(USB_HOST_OUT, (u32*) BufferPtr, BufferLen);
    if (status != KDRV_STATUS_OK) {
        err_msg("[usb_com] Error in usb_com write [%d]\r\n", status);
        return -1;  // error
    }

    return 0;
}

int usb_com_read(uint8_t *BufferPtr, uint32_t BufferLen, uint32_t flags, uint32_t mode)
{
    kdrv_status_t status;
    int os_flag;

    usb_set_notify_flag(osThreadGetId(), flags);

    if (mode) {  // waiting for previous tx complete ?
        //osThreadFlagsWait(flags, osFlagsWaitAll, osWaitForever);
        os_flag = osThreadFlagsWait(flags, osFlagsWaitAll, 1000);  // wait for max 1 second
        if (os_flag < 0) {
            err_msg("[usb_com] Data upload timed out\r\n");
            status = kdrv_usbd_reset_endpoint(USB_HOST_OUT);
            usb_mode = 1;

            if (status != KDRV_STATUS_OK) {
                err_msg("[usb_com] Error in usb reset [%d]\r\n", status);
            }

            return -1;  // signal error
        }
        usb_mode++;  // go to the receive phase
    }

    status = kdrv_usbd_bulk_receive_async(USB_HOST_IN, (u32*) BufferPtr, BufferLen);
    if (status != KDRV_STATUS_OK) {
        err_msg("[usb_com] Error in usb_com read [%d]\r\n", status);
        return -1;
    }

    os_flag = osThreadFlagsWait(flags, osFlagsWaitAll, 5000);  // now wait for data rcv complete
    if (os_flag < 0) {
        err_msg("[usb_com] Data download timed out\r\n");
        usb_mode = 1;  // usb timed out, data transfer taking too long
        status = kdrv_usbd_reset_endpoint(USB_HOST_IN);
        if (status != KDRV_STATUS_OK) {
            err_msg("[usb_com] Error in usb reset [%d]\r\n", status);
        }
        return -1;
    }

    usb_mode -= 2;  // back to previous state

    return usb_read_count;
}

void usb_init(void)
{
    osThreadAttr_t attr = {
        .stack_size = 512
    };

    tid_usb_comm = osThreadNew(usb_comm_thread, NULL, &attr);
    usb_init_dev();
}
