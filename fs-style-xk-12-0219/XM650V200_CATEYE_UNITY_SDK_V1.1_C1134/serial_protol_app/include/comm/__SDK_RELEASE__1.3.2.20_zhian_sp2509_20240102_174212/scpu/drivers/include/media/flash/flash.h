#ifndef __FLASH_H__
#define __FLASH_H__


#include "types.h"
#include "board_kl520.h"
#include "kdp_flash_def.h"
#include "usr_flash_img_table.h"


#if (FLASH_VENDOR_SELECT==GD25Q256D)
#include "GD25Q256D.h"
#elif (FLASH_VENDOR_SELECT==GD25S512MD)
#include "GD25S512MD.h"
#elif (FLASH_VENDOR_SELECT==W25Q256JV)
#include "W25Q256JV.h"
#elif (FLASH_VENDOR_SELECT==W25M512JV)
#include "W25M512JV.h"
#endif





#define FLASH_TYPE_NOR_FLASH        (0)
#define FLASH_TYPE_NAND_FLASH       (1)
#define FLASH_TYPE                  (FLASH_TYPE_NOR_FLASH)


//#if ( FLASH_TYPE == FLASH_TYPE_NOR_FLASH)
//#define GD25Q256D                   (0)
//#define GD25S512MD                  (1)
//#define W25Q256JV                   (2)
//#define W25M512JV                   (3)
//#else
//
//#error"Please select correct flash type"
//
//#endif
//
//#define FLASH_VENDOR_SELECT         (GD25S512MD)




#define FLASH_WORKING_EN            (1)
#define FLASH_READ_ARRANGE_EN       (1)
#define FLASH_PROG_ARRANGE_EN       (1)
#define FLASH_QUAD_EN               (YES)
#define SPI_QUAD_MODE
#define SPI_BUS_SPEED_100MHZ        (0x01)
#define SPI_BUS_SPEED_50MHZ         (0x02)
#define SPI_BUS_SPEED_25MHZ         (0x04)
#define SPI_BUS_SPEED               (SPI_BUS_SPEED_100MHZ)






//void kdp_flash_register_device( struct kdp_dev_flash *flash_struct );
kdp_status_t kdp_flash_initialize(void);
int kdp_flash_uninitialize(void);
kdp_status_t kdp_flash_read_data(UINT32 addr, void *data, UINT32 target_Bytes);
kdp_status_t kdp_flash_program_data(UINT32 addr, UINT8 *data, UINT32 send_bytes);
kdp_status_t kdp_flash_erase_sector(UINT32 address);
kdp_status_t kdp_flash_erase_multi_sectors(UINT32 nstart_index,  UINT32 nend_index);
kdp_status_t kdp_flash_erase_chip(void);
kdp_status_t kdp_flash_get_status(void);
kdp_status_t kdp_flash_get_info(void);
UINT8 kdp_flash_get_id(void);
void kdp_flash_erase_4k(u32 offset);
void kdp_flash_erase_64k(u32 offset);
void kdp_flash_dma_read_stop(void);
void kdp_flash_dma_write_stop(void);
void kdp_flash_set_protect_bypass(UINT8 bypass);


typedef struct kdp_dev_flash
{
    kdp_status_t        (*initial)             (void);
    kdp_status_t        (*program)             ( UINT32 addr, UINT8 *data, UINT32 send_bytes);
    kdp_status_t        (*read)                (uint32_t addr, void *data, uint32_t target_Bytes);
    kdp_status_t        (*erasemultisector)    (UINT32 nstart_add,  UINT32 nend_add);
    kdp_status_t        (*erase4KB)            (UINT32 address);
    kdp_status_t        (*erase64KB)           (UINT32 offset);
    kdp_status_t        (*eraseallchip)        (void);
    kdp_status_t        (*get_info)            (void);
    UINT8               (*get_id)              (void);
    kdp_status_t        (*GetStatus)           (void);
    void                (*set_bypass)          (UINT8 bypass);
}FLASH_DEV;

#endif




