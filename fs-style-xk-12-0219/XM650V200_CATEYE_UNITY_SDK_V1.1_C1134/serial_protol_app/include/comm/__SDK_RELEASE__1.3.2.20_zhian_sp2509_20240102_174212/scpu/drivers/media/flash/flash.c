#include <string.h>
#include "types.h"
#include "flash.h"
#include "base.h"
#include "pinmux.h"
#include "delay.h"
#include "framework/framework_driver.h"
#include "kdp520_flash.h"
#include "kdp_memxfer.h"
#include "kl520_include.h"
//#include "sample_app_console.h"

#if (FLASH_VENDOR_SELECT==GD25Q256D)
#include "GD25Q256D.h"
#elif (FLASH_VENDOR_SELECT==GD25S512MD)
#include "GD25S512MD.h"
#elif (FLASH_VENDOR_SELECT==W25Q256JV)
#include "W25Q256JV.h"
#elif (FLASH_VENDOR_SELECT==W25M512JV)
#include "W25M512JV.h"
#endif







//-----------------------------------------------------
static struct kdp_dev_flash *m_flash_handle = NULL;
FLASH_PARMATER_T    st_flash_info;
extern FLASH_DEV    flash_vendor;


void kdp_flash_register_device( struct kdp_dev_flash *stflash )
{
    m_flash_handle = stflash;
}


kdp_status_t kdp_flash_initialize(void)
{
    if( m_flash_handle == NULL )
        kdp_flash_register_device( &flash_vendor );

    return m_flash_handle->initial();
}

//int kdp_flash_uninitialize(void)
//{
//    return kdp520_flash_driver.Uninitialize();
//}

kdp_status_t kdp_flash_read_data(UINT32 addr, void *data, UINT32 target_Bytes)
{
    return m_flash_handle->read(addr, data, target_Bytes);
}

kdp_status_t kdp_flash_program_data(UINT32 addr, UINT8 *data, UINT32 send_bytes)
{
    return m_flash_handle->program(addr, data, send_bytes);
}

kdp_status_t kdp_flash_erase_sector(UINT32 address)
{
    return m_flash_handle->erase4KB(address);
}

kdp_status_t kdp_flash_erase_multi_sectors(UINT32 nstart_add,  UINT32 nend_add)
{
    return m_flash_handle->erasemultisector(nstart_add, nend_add);
}

kdp_status_t kdp_flash_erase_chip(void)
{
    return m_flash_handle->eraseallchip();
}

kdp_status_t kdp_flash_get_status(void)
{
    return m_flash_handle->GetStatus();
}

kdp_status_t kdp_flash_get_info(void)
{
    return m_flash_handle->get_info();
}

void kdp_flash_set_protect_bypass(UINT8 bypass)
{
    return m_flash_handle->set_bypass(bypass);
}

UINT8 kdp_flash_get_id(void)
{
    return m_flash_handle->get_id();
}

void kdp_flash_erase_4k(u32 offset)
{
    m_flash_handle->erase4KB(offset);
}

void kdp_flash_erase_64k(u32 offset)
{
    m_flash_handle->erase64KB(offset);
}

void kdp_flash_dma_read_stop(void)
{
    kdp520_flash_dma_read_stop();
}

void kdp_flash_dma_write_stop(void)
{
    kdp520_flash_dma_write_stop();
}


