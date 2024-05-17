/********************************************************************
 * Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 ********************************************************************/

/**@addtogroup  KDRV_USBD
 * @{
 * @brief       Kneron USB device mode driver
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef __KDRV_USBD_H__
#define __KDRV_USBD_H__

#pragma anon_unions
#include "kdp520_usbd_status.h"

#define MAX_USBD_CONFIG 1    /**< maximum number of configuration descriptor */
#define MAX_USBD_INTERFACE 1 /**< maximum number of interface descriptor */
#define MAX_USBD_ENDPOINT 4  /**< maximum number of endpoint descriptor */

/** 8-byte setup packet struct */
typedef struct __attribute__((__packed__))
{
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} kdrv_usbd_setup_packet_t;

/** Endpoint descriptor */
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
} kdrv_usbd_endpoint_descriptor_t;

/** Interface descriptor */
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;

    kdrv_usbd_endpoint_descriptor_t *endpoint[MAX_USBD_ENDPOINT];

} kdrv_usbd_interface_descriptor_t;

/** Configuration descriptor */
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t MaxPower;

    kdrv_usbd_interface_descriptor_t *interface[MAX_USBD_INTERFACE];

} kdrv_usbd_config_descriptor_t;

/** Device descriptor */
typedef struct __attribute__((__packed__))
{
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

    kdrv_usbd_config_descriptor_t *config[MAX_USBD_CONFIG];

} kdrv_usbd_device_descriptor_t;

/** Device qualifier descriptor */
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint8_t bNumConfigurations;
    uint8_t bReserved;
} kdrv_usbd_device_qualifier_descriptor_t;

/** Device string descriptor */
typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bString[32];
} kdrv_usbd_prd_string_descriptor_t;

typedef struct __attribute__((__packed__))
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bLanguageID;
    kdrv_usbd_prd_string_descriptor_t* desc[3];
    
} kdrv_usbd_string_descriptor_t;

/** Connection speed */
typedef enum
{
    KDRV_USBD_HIGH_SPEED,
    KDRV_USBD_FULL_SPEED /**< not supported yet */
} kdrv_usbd_speed_t;

/** USB event name type */
typedef enum
{
    KDRV_USBD_EVENT_BUS_RESET = 1,
    KDRV_USBD_EVENT_BUS_SUSPEND,
    KDRV_USBD_EVENT_BUS_RESUME,
    KDRV_USBD_EVENT_SETUP_PACKET,
    KDRV_USBD_EVENT_DEV_CONFIGURED,
    KDRV_USBD_EVENT_TRANSFER_BUF_FULL,
    KDRV_USBD_EVENT_TRANSFER_DONE,
    KDRV_USBD_EVENT_TRANSFER_OUT,
    KDRV_USBD_EVENT_TRANSFER_TERMINATED,
    KDRV_USBD_EVENT_DMA_ERROR,
} kdrv_usbd_event_name_t;

/** USB event, it includes kdrv_usbd_event_name_t and related data */
typedef struct
{
    kdrv_usbd_event_name_t ename;
    union {
        kdrv_usbd_setup_packet_t setup;
        struct
        {
            uint32_t data1;
            uint32_t data2;
        };
    };

} kdrv_usbd_event_t;

/** Code for response to host in control transfer */
typedef enum
{
    KDRV_USBD_RESPOND_OK,   /** send ACK in the status stage */
    KDRV_USBD_RESPOND_ERROR /** send STALL in the status stage */
} kdrv_usbd_status_respond_t;

/**
 * @brief           USB device mode driver initialization
 *
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_initialize(void);

/**
 * @brief           USB device mode driver uninitialization
 *
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_uninitialize(void);

/**
 * @brief           reset device and then it can be re-enumerated by host
 *
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_reset_device(void);

/**
 * @brief           configure device manufacturer, product , serial number
 * @param[in]       dev_desc              device string descriptor
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_set_string_descriptor(
    kdrv_usbd_string_descriptor_t *dev_str_desc);

/**
 * @brief           configure device descriptor including configuration, interface and all endpoints descriptors
 *
 * @param[in]       speed                 speed want to run, now support only High-Speed
 * @param[in]       dev_desc              user crated device descriptor, this must be kept during device enumeration
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_set_device_descriptor(
    kdrv_usbd_speed_t speed,
    kdrv_usbd_device_descriptor_t *dev_desc);

/**
 * @brief           configure device qualifier descriptor, this is optional
 *
 * @param[in]       speed                 speed want to run, now support only High-Speed
 * @param[in]       dev_qual_desc         user crated device qualifier descriptor, this must be kept during device enumeration
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_set_device_qualifier_descriptor(
    kdrv_usbd_speed_t speed,
    kdrv_usbd_device_qualifier_descriptor_t *dev_qual_desc);

/**
 * @brief           register user thread ID and thread flag for notifications including events or transfer completion/errors
 *
 * @param[in]       tid                   CMSIS-RTOS v2 thread ID
 * @param[in]       tflag                 user defined thread flag to be notified by osThreadFlagsSet()
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_register_thread_notification(osThreadId_t tid, uint32_t tflag);

/**
 * @brief           set enable/disabale of USB device mode, host can enumerate this device only if device is enabled
 *
 * @param[in]       enable                true to enable, false to disable
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_set_enable(bool enable);

/**
 * @brief           check if device is enumerated and configured by a host
 *
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern bool kdrv_usbd_is_dev_configured(void);

// get an usb event from internal event queue

/**
 * @brief           get a usbd event, this is a blocking function for sync mode usage of USBD APIs
 *
 * @param[in]       uevent                usbd event to be notified of
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_get_event(kdrv_usbd_event_t *uevent);

/**
 * @brief           Control-IN transfer, send data to host through the control endpont
 * @details         for a user-defined vendor reqeust & control IN & wLength > 0,
 *                  user should use this function to send data to host,
 *                  or respond an error via kdrv_usbd_control_respond(KDRV_USBD_RESPOND_ERROR) to claim STALL
 * 
 * @param[in]       buf                   data to be sent to host
 * @param[in]       size                  number of bytes to be transfered
 * @param[in]       timeout_ms            timeout in millisecond
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_control_send(
    uint8_t *buf,
    uint32_t size,
    uint32_t timeout_ms);

/**
 * @brief           Control-OUT transfer, receive data from host through the control endpont
 * @details         for a user-defined vendor reqeust & control OUT & wLength > 0,
 *                  user should use this function to receive data from host,
 *                  or respond an error via kdrv_usbd_control_respond(KDRV_USBD_RESPOND_ERROR) to claim STALL
 * 
 * @param[out]      buf                   buffer for receiving data
 * @param[in]       size                  buffer length
 * @param[in]       timeout_ms            timeout in millisecond
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_control_receive(
    uint8_t *buf,
    uint32_t *size,
    uint32_t timeout_ms);

// to report status for a user-defined vendor request

/**
 * @brief           respond to host through control transfer in the status stage
 * @details         this function is used as response function to report status for a user-defined vendor request
 * 
 * @param[in]       status                status
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_control_respond(kdrv_usbd_status_respond_t status);

/**
 * @brief           reset specified endpoint
 * 
 * @param[in]       status                status
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_reset_endpoint(uint32_t endpoint);

/**
 * @brief           Bulk-IN transfser, send data to host through a bulk-in endpont in blocking mode
 * 
 * @param[in]       endpoint              a bulk-in endpoint address, should be the value from bEndpointAddress
 * @param[in]       buf                   data to be sent to host
 * @param[in]       txLen                 number of bytes to be transfered
 * @param[in]       timeout_ms            timeout in millisecond
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_bulk_send(
    uint32_t endpoint,
    uint32_t *buf,
    uint32_t txLen,
    uint32_t timeout_ms);

/**
 * @brief           Bulk-IN transfser, send data to host through a bulk-in endpont in non-blocking mode
 * @details         user can commit a buffer for Bulk In transfer, and then wait for KDRV_USBD_EVENT_TRANSFER_DONE 
 *                  to be notified that the transfer is done or some error code if failed.
 *                  This function works with kdrv_usbd_get_event().
 * 
 * @param[in]       endpoint              a bulk-in endpoint address, should be the value from bEndpointAddress
 * @param[in]       buf                   data to be sent to host
 * @param[in]       txLen                 number of bytes to be transfered
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_bulk_send_async(
    uint32_t endpoint, // should be the value from bEndpointAddress
    uint32_t *buf,     // memory addres to be read from
    uint32_t txLen);   // transfer length

/**
 * @brief           Bulk-OUT transfser, receive data from the host through a bulk-out endpoint in blocking mode
 * 
 * @param[in]       endpoint              a bulk-out endpoint address, should be the value from bEndpointAddress
 * @param[out]      buf                   buffer for receiving data
 * @param[in,out]   blen                  buffer length for input, actual transfered length for output
 * @param[in]       timeout_ms            timeout in millisecond
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_bulk_receive(
    uint32_t endpoint,
    uint32_t *buf,
    uint32_t *blen,
    uint32_t timeout_ms);

/**
 * @brief           Bulk-OUT transfser, receive data from the host through a bulk-out endpoint in non-blocking mode
 * @details         this works with kdrv_usbd_get_event(), when receiving a 'KDRV_USBD_EVENT_TRANSFER_OUT' event,
 *                  user should commit a buffer for Bulk Out transfer through this function.
 *                  when transfer is done by usbd, eihter a 'KDRV_USBD_EVENT_TRANSFER_DONE' or 'KDRV_USBD_EVENT_TRANSFER_BUF_FULL' event
 *                  will be sent to user.
 * 
 * @param[in]       endpoint              a bulk-out endpoint address, should be the value from bEndpointAddress
 * @param[in]       buf                   buffer for receiving data
 * @param[in]       blen                  buffer length
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_bulk_receive_async(
    uint32_t endpoint,
    uint32_t *buf,
    uint32_t blen
);

/**
 * @brief           Interrupt-IN transfer in blocking mode
 * @details         Immediately write data to the FIFO buffer for periodic interrupt-in transfer.
 *                  Note even while the old data is not yet read by host, this function will overwrite it.
 * 
 * @param[in]       endpoint              a interrupt-in endpoint address, should be the value from bEndpointAddress
 * @param[in]       buf                   data to be sent to host
 * @param[in]       txLen                 transfer length, shoudl be less then MaxPacketSize
 * @param[in]       timeout_ms            timeout in millisecond
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_interrupt_send(
    uint32_t endpoint,
    uint32_t *buf,
    uint32_t txLen,
    uint32_t timeout_ms);

/**
 * @brief           Interrupt-OUT transfer in blocking mode
 * 
 * @param[in]       endpoint              a interrupt-out endpoint address, should be the value from bEndpointAddress
 * @param[out]      buf                   buffer for receiving data
 * @param[in,out]   rxLen                 buffer length for input, actual transfered length for output, should be less than MaxPacketSize
 * @param[in]       timeout_ms            timeout in millisecond
 * @return          kdrv_status_t         see @ref kdrv_status_t
 */
extern kdrv_status_t kdrv_usbd_interrupt_receive(
    uint32_t endpoint, // should be the value from bEndpointAddress
    uint32_t *buf,
    uint32_t *rxLen,
    uint32_t timeout_ms);

#endif
