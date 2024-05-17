#include "kdp_memxfer.h"
#include <string.h>
#include <cmsis_os2.h>
#include "types.h"
#include "io.h"
#include "framework/utils.h"
#include "scu_extreg.h"
#include "kdp520_spi.h"
#include "kdp520_dma.h"
#include "flash.h"
#include "ota_update.h"
#include "dbg.h"


osMutexId_t flash_memx_lock = NULL;
static u16 flash_device_id = 0;
extern u16 kdp520_memxfer_get_flash_device_id(void)
{
    return flash_device_id;
}

int kdp_memxfer_init(u8 flash_mode, u8 mem_mode)
{
    kdp_status_t    ret ;
    if(flash_memx_lock == NULL)
        flash_memx_lock = osMutexNew(NULL);

    osMutexAcquire(flash_memx_lock, osWaitForever);

    kdp_flash_initialize();
    ret = kdp_flash_get_info();

    flash_device_id = kdp_flash_get_id();
    osMutexRelease(flash_memx_lock);
    return ret;
}

int kdp_memxfer_flash_to_ddr(u32 dst, u32 src, size_t bytes)
{

#if 0 || FLASH_READ_ARRANGE_EN == 1 //FLASH_CODE_OPT == YES
    osMutexAcquire(flash_memx_lock, osWaitForever);
    if( kdp_flash_read_data( src,  (UINT32 *)dst ,bytes ) == KDP_STATUS_OK )
    {
        osMutexRelease(flash_memx_lock);
        return 0;
    }
    else
    {
        osMutexRelease(flash_memx_lock);
        return -1;
    }
#else
#endif

}

extern void kdp_ddr_to_flash_dma_copy(UINT32 src_addr, UINT32 dst_addr, UINT32 size);
int kdp_memxfer_ddr_to_flash(u32 dst, u32 src, size_t bytes)
{
    int ret = 0;
    osMutexAcquire(flash_memx_lock, osWaitForever);
    kdp_flash_erase_multi_sectors( dst, dst+bytes );
    if( src != 0 )
    {
        ret = (int)kdp_flash_program_data( dst, (UINT8*)src , bytes );
        //kdp_ddr_to_flash_dma_copy(src, dst, bytes);
    }
    osMutexRelease(flash_memx_lock);
    return ret ;
}

#define FLASH_SECTOR_SIZE 4096
int kdp_memxfer_ddr_to_flash_sector(u32 dst, u32 src, size_t bytes)
{
    int ret = 0;
    if(bytes > FLASH_SECTOR_SIZE) return KDP_STATUS_ERROR;

    u32 start_dst = ( dst / FLASH_SECTOR_SIZE ) * FLASH_SECTOR_SIZE;
    u32 start_src = src - (dst - start_dst);
    //dbg_msg_algo ("flashing %x,%x by %x,%x.", dst, src, start_dst, start_src);
    
    osMutexAcquire(flash_memx_lock, osWaitForever);
    kdp_flash_erase_4k( start_dst );
    if( src != 0 )
    {
        ret = (int)kdp_flash_program_data( start_dst, (UINT8*)start_src , FLASH_SECTOR_SIZE );
    }
    osMutexRelease(flash_memx_lock);
    return ret ;
}

int kdp_memxfer_ddr_to_flash_wo_erase(u32 dst, u32 src, size_t bytes)
{
    int ret = 0;
    osMutexAcquire(flash_memx_lock, osWaitForever);
    if( src != 0 )
    {
        ret = (int)kdp_flash_program_data( dst, (UINT8*)src , bytes );
    }
    osMutexRelease(flash_memx_lock);
    return ret ;
}

/**
 * @brief flash 4k sector erase
 */
int kdp_memxfer_flash_sector_erase4k(u32 addr)
{
    osMutexAcquire(flash_memx_lock, osWaitForever);

    kdp_flash_erase_4k(addr);

    osMutexRelease(flash_memx_lock);

    return 0;
}

/**
 * @brief flash 64k sector erase
 */
int kdp_memxfer_flash_sector_erase64k(u32 addr)
{
    osMutexAcquire(flash_memx_lock, osWaitForever);

    kdp_flash_erase_64k(addr);

    osMutexRelease(flash_memx_lock);

    return 0;
}

int kdp_memxfer_flash_sector_multi_erase( u32 start_add, u32 end_add )
{
    osMutexAcquire(flash_memx_lock, osWaitForever);
    kdp_flash_erase_multi_sectors( start_add, end_add );
    osMutexRelease(flash_memx_lock);
    return 0;
}

/**  
 * @brief load ncpu firmware code from flash to niram 
 */
int kdp_memxfer_flash_to_niram(void)
{
    int ota_active_sts;

    ota_active_sts = ota_get_active_ncpu_partition();
    /* stop ncpu, then load code from flash to NiRAM */
    
    if (ota_active_sts == 0) {
        kdp_memxfer_flash_to_ddr((u32)NCPU_START_ADDRESS, 
                NCPU_PARTITION0_START_IN_FLASH, NCPU_IMAGE_SIZE);
    } else {
        kdp_memxfer_flash_to_ddr((u32)NCPU_START_ADDRESS, 
                NCPU_PARTITION1_START_IN_FLASH, NCPU_IMAGE_SIZE);
    }


    return 0;
}

// const struct s_kdp_memxfer kdp_memxfer_module = {
//     kdp_memxfer_init,
//     kdp_memxfer_flash_to_ddr,
//     kdp_memxfer_ddr_to_flash,
//     #ifdef USE_KDRV
//     NULL,
//     NULL,
//     NULL,
//     #else
//     kdp_memxfer_flash_to_ddr_dma,
//     kdp_memxfer_ddr_to_flash_dma,
//     kdp_memxfer_ddr_to_ddr,
//     #endif
//     kdp_memxfer_flash_sector_erase64k,
//     kdp_memxfer_flash_sector_multi_erase,
//     kdp_memxfer_flash_to_niram,
// };
