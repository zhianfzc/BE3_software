/* Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
*
* The information contained herein is property of Kneron, Inc.
* Terms and conditions of usage are described in detail in Kneron
* STANDARD SOFTWARE LICENSE AGREEMENT.
*
* Licensees are granted free, non-transferable use of the information.
* NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
* from the file.
*/

/**@addtogroup  KDRV_SPIF  KDRV_SPIF
 * @{
 * @brief       Kneron spi flash driver
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef __KDRV_SPIF_H__
#define __KDRV_SPIF_H__

#include "base.h"
#include "kdrv_status.h"

/*-----------------------------------------------------------------------
 *                          Puclic flash driver API 
 *-----------------------------------------------------------------------*/
/**
* @brief        Initialize spi flash include hardware setting, operation frequency, and flash status check.
*
* @param[in]    N/A
* @return       N/A
*
* @note        This API MUST be called before using the Read/write APIs for spi flash.
*/
void kdrv_spif_initialize(void);
/**
* @brief        Initialize spi flash for memxfer include hardware setting, operation frequency, and flash status check.
*
* @param[in]   flash_mode   flash operating mode
*              mem_mode     memory operating mode 
* @return      N/A
*/
void kdrv_spif_memxfer_initialize(uint8_t flash_mode, uint8_t mem_mode);
/**
* @brief       Uninitialize spi flash and clear related variables
*
* @param[in]   N/A
* @return      @ref kdrv_status_t
*/
kdrv_status_t kdrv_spif_uninitialize(void);

/**
* @brief        set spi communication commands including read/write by 3/4bytes address, dummy byte size, operation mode, etc
*
* @param[in]    cmd0 ~ 3
* @return       N/A
*/
void kdrv_spif_set_commands(uint32_t cmd0, uint32_t cmd1, uint32_t cmd2, uint32_t cmd3);
/**
* @brief        Check status bit to wait until command completed
*
* @param[in]    N/A
* @return       N/A
*/
void kdrv_spif_wait_command_complete(void);
/**
* @brief        Wait until the RX FIFO is full so ready to read 
*
* @param[in]    N/A
* @return       N/A
*/
void kdrv_spif_wait_rx_full(void);
/**
* @brief        Wait until the TX FIFO is empty so ready to write
*
* @param[in]    N/A
* @return       N/A
*/
void kdrv_spif_wait_tx_empty(void);
/**
* @brief        Check the RX FIFO size, unit in byte
*
* @param[in]    N/A
* @return       >0          RX FIFO depth
*/
uint32_t kdrv_spif_rxfifo_depth(void);
/**
* @brief        Check the TX FIFO size, unit in byte
*
* @param[in]    N/A
* @return       >0          TX FIFO depth
*/
uint32_t kdrv_spif_txfifo_depth(void);
/**
* @brief        read data from specific index in spi flash
*
* @param[in]    *buf        buffer for the data read from flash 
*               length      data size
* @return       N/A
*/
void kdrv_spif_read_data(/*uint8_t*/uint32_t *buf, uint32_t length);
/**
* @brief        write data to specific index in spi flash
*
* @param[in]    *buf        buffer for the data to write to flash 
*               length      data size
* @return       N/A
*/
void kdrv_spif_write_data(uint8_t *buf, uint32_t length);
/**
* @brief        read Rx FIFO data
*
* @param[in]    *buf_word       buffer for the data read from flash
*               *buf_word_index start from specific flash index 
*               target_byte     data size
* @return       N/A
*/
void kdrv_spif_read_Rx_FIFO( uint32_t *buf_word, uint16_t *buf_word_index, uint32_t target_byte );
/**
* @brief        check status till the progress is done and ready for next step
*
* @param[in]    N/A
* @return       N/A
*/
void kdrv_spif_check_status_till_ready_2(void);
/**
* @brief        wait command completed and check status till it's ready
*
* @param[in]    N/A
* @return       N/A
*/
void kdrv_spif_check_status_till_ready(void);
/**
* @brief        wait quad read command completed and check status till ready
*
* @param[in]    N/A
* @return       N/A
*/
void kdrv_spif_check_quad_status_till_ready(void);

#endif/* __KDRV_SPIF_H__ */
/** @}*/
