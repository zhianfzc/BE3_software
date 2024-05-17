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

/* History:
 *  Version 2.00
 *    Renamed driver NOR -> Flash (more generic)
 *    Non-blocking operation
 *    Added Events, Status and Capabilities
 *    Linked Flash information (GetInfo)
 *  Version 1.11
 *    Changed prefix ARM_DRV -> ARM_DRIVER
 *  Version 1.10
 *    Namespace prefix ARM_ added
 *  Version 1.00
 *    Initial release
 */

/**@addtogroup  KDEV_FLASH  KDEV_FLASH
 * @{
 * @brief       Kneron flash device
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */
#ifndef __KDEV_FLASH_H
#define __KDEV_FLASH_H

#include "Driver_Common.h"
#include "kdrv_SPI020.h"
#include "kdev_status.h"

#define SPI020_SECTOR_SIZE		4096
#define SPI020_BLOCK_64SIZE		65536

/**
* @brief Flash Sector index struct
*/
typedef struct {
  uint32_t start;                       /**< Sector Start address */
  uint32_t end;                         /**< Sector End address (start+size-1) */
}kdev_flash_sector_t;

/**
* @brief Flash information struct
*/
typedef struct {
  kdev_flash_sector_t *sector_info;        /**< Sector layout information (NULL=Uniform sectors) */
  uint32_t            sector_count;       /**< Number of sectors */
  uint32_t            sector_size;        /**< Uniform sector size in bytes (0=sector_info used) */
  uint32_t            page_size;          /**< Optimal programming page size in bytes */
  uint32_t            program_unit;       /**< Smallest programmable unit in bytes */
  uint8_t             erased_value;       /**< Contents of erased memory (usually 0xFF) */
  uint32_t            flash_size;
} kdev_flash_info_t;    

/**
* @brief Flash Status struct
*/
typedef struct {
  uint32_t busy  : 1;                   /**< Flash busy flag */
  uint32_t error : 1;                   /**< Read/Program/Erase error flag (cleared on start of next operation) */
} kdev_flash_status_t;


extern uint32_t  kdev_flash_probe(spi_flash_t *flash);

// Function documentation
/**
* @fn          kdev_status_t kdev_flash_initialize (void)
* @brief       Initialize spi flash interface include hardware setting, get flash information and set to 4byte address 
*              if flash size is bigger than 16Mbytes
* @param[in]   N/A
* @return      @ref kdrv_status_t
*
* @note        This API MUST be called before using the Read/write APIs for spi flash.
*/
kdev_status_t kdev_flash_initialize(void);//ARM_Flash_SignalEvent_t cb_event);
/**
* @fn          kdrv_status_t kdev_flash_uninitialize(void)
* @brief       Uinitialize the spi flash interface.
* @return      @ref kdrv_status_t
*/
kdev_status_t kdev_flash_uninitialize(void);
/**
* @fn          kdev_status_t kdev_flash_power_control(ARM_POWER_STATE state)
* @brief       Power handling for spi flasg.
* @param[in]   state  Power state
* @return      @ref kdev_status_t
*/
kdev_status_t kdev_flash_power_control(ARM_POWER_STATE state);
/**
* @fn          kdev_status_t kdev_flash_readdata(uint32_t addr, void *data, uint32_t cnt)
* @brief       Read data from specific index of spi flash.
* @param[in]   addr  Data address.
* @param[out]  data  Pointer to a buffer storing the data read from Flash.
* @param[in]   cnt   Number of data items to read.
* @return      number of data items read or @ref kdev_status_t
*/
kdev_status_t kdev_flash_readdata(uint32_t addr, void *data, uint32_t cnt);
/**
* @fn          kdev_status_t kdev_flash_programdata (uint32_t addr, const void *data, uint32_t cnt)
* @brief       Program data to specific index in spi flash
* @param[in]   addr  Data address.
* @param[in]   data  Pointer to a buffer containing the data to be programmed to Flash.
* @param[in]   cnt   Number of data items to program.
* @return      number of data items programmed or @ref kdev_status_t
*/
kdev_status_t kdev_flash_programdata (uint32_t addr, const void *data, uint32_t cnt);
/**
* @fn          kdev_status_t kdev_flash_erase_sector(uint32_t addr)
* @brief       Erase Flash by Sector(4k bytes).
* @param[in]   addr  Sector address
* @return      @ref kdev_status_t
*/
kdev_status_t kdev_flash_erase_sector(uint32_t addr);
/**
* @fn          kdev_status_t kdev_flash_erase_multi_sector(uint16_t start_addr, uint16_t end_addr)
* @brief       Erase multiple Flash Sectors(continuously).
* @param[in]   addr  Sector start address
* @param[in]   addr  Sector end address
* @return      @ref kdev_status_t
*/
kdev_status_t kdev_flash_erase_multi_sector(uint16_t start_addr, uint16_t end_addr);
/**
* @fn          kdev_status_t kdev_flash_erase_chip(void)
* @brief       Erase whole Flash at once.
               Optional function for faster full chip erase.
* @return      @ref kdev_status_t
*/
kdev_status_t kdev_flash_erase_chip(void);
/**
* @fn          kdev_flash_status_t kdev_flash_get_status(void)
* @brief       Get Flash status.
* @return      Flash status @ref kdev_flash_status_t
*/
kdev_flash_status_t kdev_flash_get_status(void);
/**
* @fn          kdev_flash_info_t * kdev_flash_get_info(void)
* @brief       Get Flash information.
* @return      Pointer to Flash information @ref kdev_flash_info_t
*/
kdev_flash_info_t * kdev_flash_get_info(void);
#endif /* __KDEV_FLASH_H */
/** @}*/
