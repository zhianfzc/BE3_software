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

#ifndef __KDRV_STATUS_H__
#define __KDRV_STATUS_H__

#include <stdbool.h> // for type 'bool', ture and false.
typedef enum
{
    KDRV_STATUS_OK = 0,                       /**< driver status OK */
    KDRV_STATUS_ERROR,                        /**< driver status error */
    KDRV_STATUS_INVALID_PARAM,                /**< driver invalid parameters */
    KDRV_STATUS_I2C_BUS_BUSY,                 /**< I2C : bus is busy */
    KDRV_STATUS_I2C_DEVICE_NACK,              /**< I2C : slave device does not send ACK */
    KDRV_STATUS_I2C_TIMEOUT,                  /**< I2C : transfer timeout */
    KDRV_STATUS_USBD_INVALID_ENDPOINT,        /**< USBD : invalid endpoint */
    KDRV_STATUS_USBD_TRANSFER_TIMEOUT,        /**< USBD : transfer timeout */
    KDRV_STATUS_USBD_INVALID_TRANSFER,        /**< USBD : invalid transfer operation */
    KDRV_STATUS_USBD_TRANSFER_IN_PROGRESS,    /**< USBD : transfer is in progress */
    KDRV_STATUS_GDMA_ERROR_NO_RESOURCE,       /**< GDMA : DMA channel is not available */
    KDRV_STATUS_TIMER_ID_NOT_IN_USED,         /**< Timer: TIMER Id NOT in used */
    KDRV_STATUS_TIMER_ID_IN_USED,             /**< Timer: TIMER Id in used */
    KDRV_STATUS_TIMER_ID_NOT_AVAILABLE,       /**< Timer: TIMER Id is NOT available */
    KDRV_STATUS_TIMER_INVALID_TIMER_ID,       /**< Timer: TIMER Id is invalid */
    KDRV_STATUS_UART_TX_RX_BUSY,              /**< UART: TX or RX is busy */
    KDRV_STATUS_UART_TIMEOUT,                 /**< UART: timeout */
} kdrv_status_t;

#endif
