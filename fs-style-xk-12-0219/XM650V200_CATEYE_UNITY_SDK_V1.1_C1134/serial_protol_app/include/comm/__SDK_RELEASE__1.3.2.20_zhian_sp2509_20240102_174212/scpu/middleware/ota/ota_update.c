#include "ota_update.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if 1//( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
#include "kneron_mozart.h"
#include "kl520_include.h"
#include "power_manager.h"
#include "kdp_ddr_table.h"
#include "kdp_crc.h"
#include "kdp_memxfer.h"
#include "kdp_model.h"
#include "kdp520_spi.h"
#include "flash.h"
#include "com.h"
#include "com_err.h"
#include "io.h"
#include "ota.h"
#include "user_ui.h"


#ifdef USE_KDRV
/* function used to read OTA content */
typedef u32 (*FnReadData)(u32 addr, u32 img_size);
static u8 *tmp_ver_buf = NULL;
static FnReadData fn_read_data = NULL;
#endif

#define MODEL_DDR_START_ADDR        0x60000000
#define MODEL_INFO_FLASH_ADDR       0x00100000
#define MODEL_INFO_SIZE             0XE7
#define MODEL_ALL_BIN_FLASH_ADDR    0x00101000

#define POLY 0x8408

// disable compile optimization
//#pragma GCC push_options
//#pragma GCC optimize ("O0")

extern OTA_FLASHt   stOTA;


int guser_area = -1;
ota_boot_cfg_t boot_cfg_0, boot_cfg_1;
ota_user_cfg user_cfg_0, user_cfg_1;

static u8 gota_judge_pool[OTA_JUDGE_SIZE];              //index 0: SCPU, index 1: NCPU, index 2:fw_info, index 3: model, index 4: UI image


#define CRC16_CONSTANT 0x8005
int flashing;

u32 gscpu_crc = 0xFFFFFFFF;
u32 gncpu_crc = 0xFFFFFFFF;
u32 gfwinfo_crc = 0xFFFFFFFF;
u32 gmodel_crc = 0xFFFFFFFF;
u32 gui_crc = 0xFFFFFFFF;


int ota_update_sleep(enum pm_device_id dev_id)
{
    while (flashing == 1) {
        err_msg("ota_update_sleep: stop for flashing.\n");
        osThreadFlagsWait(BIT27, osFlagsWaitAll, osWaitForever);
    }
    err_msg("ota_update_sleep: ok\n");

    return 0;
}

int ota_update_deep_sleep(enum pm_device_id dev_id)
{
    while (flashing == 1) {
        err_msg("ota_update_deep_sleep: stop for flashing.\n");
        osThreadFlagsWait(BIT27, osFlagsWaitAll, osWaitForever);
    }
    err_msg("ota_update_deep_sleep: ok\n");

    return 0;
}

struct pm_s ota_update_pm = {
    .sleep          = ota_update_sleep,
    .deep_sleep     = ota_update_deep_sleep,
};
#if 0
static u16 gen_crc16(u8 *data, u32 size)
{
    u16 out = 0;
    int bits_read = 0, bit_flag, i;

    /* Sanity check: */
    if (data == NULL)
        return 0;

    while (size > 0)
    {
        bit_flag = out >> 15;

        /* Get next bit: */
        out <<= 1;
        out |= (*data >> bits_read) & 1;
        bits_read++;
        if (bits_read > 7)
        {
            bits_read = 0;
            data++;
            size--;
        }

        /* Cycle check: */
        if (bit_flag)
            out ^= CRC16_CONSTANT;

    }

    // push out the last 16 bits
    for (i = 0; i < 16; ++i) {
        bit_flag = out >> 15;
        out <<= 1;
        if (bit_flag)
            out ^= CRC16_CONSTANT;
    }

    // reverse the bits
    u16 crc = 0;
    i = 0x8000;
    int j = 0x0001;
    for (; i != 0; i >>= 1, j <<= 1) {
        if (i & out) crc |= j;
    }

    return crc;
}
#endif


#if 0
/*
Compare flash content with buffer content, the size shall be more than 4k,
     because small block (<4k) was verified already, here is to verify the
     consistency/integrity among 4k blocks
Return:
    0 - success
    -1 - fail
*/
static int ota_post_flash_verify_4kblock(u32 flash_addr, u32 size, u8 *pbuf)
{
    int remainder, loop, i, ret;

    if (size < 0x1000) {
        //dbg_msg("size is less than 4k\n");
        return -1;
    }

    loop = size / 0x1000;
    remainder = size % 0x1000;
    u8* flash_sector_check = msg_rbuf + 0x40;

    for (i = 0; i < loop; i++) {
        kdp520_flash_read_old(FLASH_NORMAL, (flash_addr + i * 0x1000) & 0xFFFFF000,
            0x1000, flash_sector_check);// read the new sector
//        ret = flash_wait_ready(300);
        if (ret == -1)
        {
            err_msg("Flash read failure, timeout\n");
            return -1;
        }

        ret = memcmp(flash_sector_check, pbuf + i * 0x1000, 0x1000);
        if (ret != 0)
        {
            err_msg("Found diff, flash failed\n");
            return -1;
        }
    }

    if (remainder != 0) {
        kdp520_flash_read_old(FLASH_NORMAL, flash_addr + loop * 0x1000, remainder, flash_sector_check);
//        ret = flash_wait_ready(300);
        if (ret == -1)
        {
            err_msg("Flash read failure, timeout\n");
            return -1;
        }

        ret = memcmp(flash_sector_check, pbuf + loop * 0x1000, size);
        if (ret != 0)
        {
            err_msg("Flash compare failed\n");
            return -1;
        }

    }

    return 0;
}


/*
Write memory data to flash, the size is 4k blocks (i.e. n*4k)

Return:
    0 - success
    -1 - fail
*/

static int ota_mem_to_flash_4k_blocks(u32 mem_addr, u32 flash_addr, u32 size)
{
    int ret;

    /* validate parameters */
    if (size % 0x1000 != 0)
    {
        err_msg("Wrong size, not n*4K");
        return -1;
    }

    if (mem_addr & 0x1f != 0)
    {
        err_msg("memory address does not align to 32bit boundary");
        return -1;
    }

    if (flash_addr & 0x1f != 0)
    {
        err_msg("flash address does not align to 32bit boundary");
        return -1;
    }

    /* erase flash sectors firstly */

    u16 sect_num = size / 0x1000;     /* sector size = 4K */
    u32 offset = 0;
    for (int sect = 0; sect < sect_num; sect++)
    {
        kdp_flash_erase_sector(flash_addr + offset);
//        ret = flash_wait_ready(300);  // wait for the erase operation to complete
        if (ret < 0) {
            //dbg_msg("Erase Flash Sector Timeout\n");
        }

        for (int i = 0; i < 0x1000; i += 0x100) {
            kdp_flash_program_data((uint32_t)(flash_addr + offset + i),
                (void *)(mem_addr + offset + i), 0x100);
//            flash_wait_ready(500);
        }

        /* read back for confirmation*/
        u8* flash_sector_check = msg_rbuf + 0x40;
        kdp520_flash_read_old(FLASH_NORMAL, flash_addr + offset, 0x1000, flash_sector_check);
//        ret = flash_wait_ready(300);
        if (memcmp((void *)(mem_addr + offset), (void *)flash_sector_check, 0x1000)) {
            err_msg("Flash readback verification fail at flash addr=%x\n",
                (flash_addr + offset));
            return -1;
        }
        offset += 0x1000;
    }

    delay_us(500 * 1000);
    return 0;
}

/*
Write memory data to flash, the size is less than 4k

Return:
    0 - success
    -1 - fail
*/
int ota_mem_to_flash_small_block(u32 mem_addr, u32 flash_addr, u32 size)
{
    int ret;
    /* validate parameters */
    if (size >= 0x1000)
    {
        err_msg("Wrong size, bigger than 4K");
        return -1;
    }

    if (mem_addr & 0x1f != 0)
    {
        err_msg("memory address does not align to 32bit boundary");
        return -1;
    }

    if (flash_addr & 0x1f != 0)
    {
        err_msg("flash address does not align to 32bit boundary");
        return -1;
    }

    /* erase flash sectors firstly */

    kdp_flash_erase_sector(flash_addr);
//    ret = flash_wait_ready(300);  // wait for the erase operation to complete
    if (ret < 0) {
        //dbg_msg("Erase Flash Sector Timeout\n");
    }

    kdp_flash_program_data(flash_addr, (void *)mem_addr, size);
//    flash_wait_ready(500);

    /* read back for confirmation*/
    u8* flash_sector_check = msg_rbuf + 0x40;
#if FLASH_CODE_OPT == YES
  	UINT32	nsize = 0;
  	nsize = 0;
  	kdp_flash_read_data( flash_addr, flash_sector_check , &nsize, size );

#else
    kdp520_flash_read_old(FLASH_NORMAL, flash_addr, size, flash_sector_check);
#endif



//    ret = flash_wait_ready(300);
    if (memcmp((void *)mem_addr, (void *)flash_sector_check, size)) {
        err_msg("Flash readback verification fail at flash addr=%x\n", flash_addr);
        return -1;
    }

    delay_us(200 * 1000);

    return 0;
}
#endif
void ota_init_partition_boot_cfg(void)
{
    int ret;
    boot_cfg_0.scpu_cfg.partition_id = 0;
    boot_cfg_0.scpu_cfg.seq = 1;
    boot_cfg_0.scpu_cfg.flag = BOOT_STATE_CONFIRMED;

    boot_cfg_0.ncpu_cfg.partition_id = 0;
    boot_cfg_0.ncpu_cfg.seq = 1;
    boot_cfg_0.ncpu_cfg.flag = BOOT_STATE_CONFIRMED;

    boot_cfg_1.scpu_cfg.partition_id = 1;
    boot_cfg_1.scpu_cfg.seq = 0;
    boot_cfg_1.scpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;

    boot_cfg_1.ncpu_cfg.partition_id = 1;
    boot_cfg_1.ncpu_cfg.seq = 0;
    boot_cfg_1.ncpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;


    kdp_memxfer_flash_sector_multi_erase( PARTITION_0_CFG_START_IN_FLASH, PARTITION_0_CFG_START_IN_FLASH );
    kdp_memxfer_flash_sector_multi_erase( PARTITION_1_CFG_START_IN_FLASH, PARTITION_1_CFG_START_IN_FLASH );

    ret = kdp_memxfer_ddr_to_flash(  (UINT32)PARTITION_0_CFG_START_IN_FLASH , (UINT32)&boot_cfg_0,sizeof(boot_cfg_0));

    if (ret == 0) {
    	dbg_msg("Error: Flash partition 0 config Timeout");
    }

    ret = kdp_memxfer_ddr_to_flash(  (UINT32)PARTITION_1_CFG_START_IN_FLASH , (UINT32)&boot_cfg_1,sizeof(boot_cfg_1));

    if (ret == 0) {
    	dbg_msg("Error: Flash partition 1 config Timeout");
    }

    kdp_memxfer_flash_to_ddr((UINT32)&boot_cfg_0, PARTITION_0_CFG_START_IN_FLASH , sizeof( boot_cfg_0) );
    kdp_memxfer_flash_to_ddr((UINT32)&boot_cfg_1, PARTITION_1_CFG_START_IN_FLASH , sizeof( boot_cfg_1) );

}

/* get active SCPU partition ID

Return:
    0 - partition 0
    1 - partition 1
    -1 - error condition (2 active partitions)
*/

int ota_get_active_scpu_partition(void)
{
    kdp_memxfer_flash_to_ddr((UINT32)&boot_cfg_0, PARTITION_0_CFG_START_IN_FLASH , sizeof( boot_cfg_0) );
    kdp_memxfer_flash_to_ddr((UINT32)&boot_cfg_1, PARTITION_1_CFG_START_IN_FLASH , sizeof( boot_cfg_1) );

    if ((boot_cfg_0.scpu_cfg.flag == 0xffffffff) && (boot_cfg_1.scpu_cfg.flag == 0xffffffff)) {
        // no config data is there, need to create them for partition 0/1
        ota_init_partition_boot_cfg();
        return 0;
    }

    if (boot_cfg_0.scpu_cfg.flag & boot_cfg_1.scpu_cfg.flag & BOOT_STATE_CONFIRMED)
    {
        if (BOOT_STATE_CONFIRMED == boot_cfg_0.scpu_cfg.flag)
            boot_cfg_1.scpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;
        else if (BOOT_STATE_CONFIRMED == boot_cfg_1.scpu_cfg.flag)
            boot_cfg_0.scpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;
        else
        {
            dbg_msg("Critical Error: 2 active SCPU boot config");
            return -1;
        }
    }

    if ((boot_cfg_0.scpu_cfg.partition_id == boot_cfg_1.scpu_cfg.partition_id)
        && (boot_cfg_0.scpu_cfg.seq == boot_cfg_1.scpu_cfg.seq))
    {
        // no config data is there, need to create them for partition 0/1
        ota_init_partition_boot_cfg();
        return 0;
    }

    if ((boot_cfg_0.scpu_cfg.flag & BOOT_STATE_CONFIRMED) == BOOT_STATE_CONFIRMED)
        return 0;

    if ((boot_cfg_1.scpu_cfg.flag & BOOT_STATE_CONFIRMED) == BOOT_STATE_CONFIRMED)
        return 1;
    return 0;
}

/* get active NCPU partition ID

Return:
    0 - partition 0
    1 - partition 1
    -1 - error condition (2 active partitions)
*/
int ota_get_active_ncpu_partition(void)
{

    kdp_memxfer_flash_to_ddr((UINT32)&boot_cfg_0, PARTITION_0_CFG_START_IN_FLASH , sizeof( boot_cfg_0) );
    kdp_memxfer_flash_to_ddr((UINT32)&boot_cfg_1, PARTITION_1_CFG_START_IN_FLASH , sizeof( boot_cfg_1) );

//    // add here
//    dbg_msg("---------[OTA config] Config read status ---------");
//
//    dbg_msg("---------[OTA config] spcu---------");
//
//    dbg_msg("[OTA config]scpu 0 flag:%d, id:%d, seq:%d", boot_cfg_0.scpu_cfg.flag
//    							 , boot_cfg_0.scpu_cfg.partition_id, boot_cfg_0.scpu_cfg.seq);
//
//    dbg_msg("[OTA config]scpu 1 flag:%d, id:%d, seq:%d", boot_cfg_1.scpu_cfg.flag
//    							 , boot_cfg_1.scpu_cfg.partition_id, boot_cfg_1.scpu_cfg.seq);
//
//    dbg_msg("---------[OTA config] npcu---------");
//
//    dbg_msg("[OTA config]ncpu 0 flag:%d, id:%d, seq:%d", boot_cfg_0.ncpu_cfg.flag
//    							 , boot_cfg_0.ncpu_cfg.partition_id, boot_cfg_0.ncpu_cfg.seq);
//
//    dbg_msg("[OTA config]ncpu 1 flag:%d, id:%d, seq:%d", boot_cfg_1.ncpu_cfg.flag
//    							 , boot_cfg_1.ncpu_cfg.partition_id, boot_cfg_1.ncpu_cfg.seq);

    if ((boot_cfg_0.ncpu_cfg.flag == 0xffffffff) && (boot_cfg_1.ncpu_cfg.flag == 0xffffffff)) {
        // no config data is there, need to create them for partition 0/1
        ota_init_partition_boot_cfg();
        return 0;
    }

    if (boot_cfg_0.ncpu_cfg.flag & boot_cfg_1.ncpu_cfg.flag & BOOT_STATE_CONFIRMED)
    {

        if (BOOT_STATE_CONFIRMED == boot_cfg_0.ncpu_cfg.flag)
            boot_cfg_1.ncpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;
        else if (BOOT_STATE_CONFIRMED == boot_cfg_1.ncpu_cfg.flag)
            boot_cfg_0.ncpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;
        else
        {
            err_msg("Critical Error: 2 active NCPU boot config\n");
            return -1;
        }
    }

    if ((boot_cfg_0.ncpu_cfg.partition_id == boot_cfg_1.ncpu_cfg.partition_id) &&
        (boot_cfg_0.ncpu_cfg.seq == boot_cfg_1.ncpu_cfg.seq))
    {
        // no config data is there, need to create them for partition 0/1
        ota_init_partition_boot_cfg();
        return 0;
    }

    if ((boot_cfg_0.ncpu_cfg.flag & BOOT_STATE_CONFIRMED) == BOOT_STATE_CONFIRMED)
        return 0;

    if ((boot_cfg_1.ncpu_cfg.flag & BOOT_STATE_CONFIRMED) == BOOT_STATE_CONFIRMED)
        return 1;
    return 0;
}

int ota_update_scpu(void)
{
    int ret = OTA_UPDATE_SUCCESS;

    #ifdef USE_KDRV
    u32 local_sum32, remote_sum32;
    u8 *pBase;
    pBase = (u8 *)KDP_DDR_OTA_FLASH_BUF_START_ADDR;        //KDP_DDR_BASE;
    memset(pBase, 0, SCPU_IMAGE_SIZE);
    ret = fn_read_data((u32)pBase, SCPU_IMAGE_SIZE);        // fn_read_data = kcomm_read,


    if( ret == SCPU_IMAGE_SIZE )
    {
        local_sum32 = kdp_gen_sum32(pBase, SCPU_IMAGE_SIZE - 4);
        remote_sum32 = *(u32 *)(pBase + SCPU_IMAGE_SIZE - 4);
        if (local_sum32 != remote_sum32) {
            ret = MSG_AUTH_FAIL;
            goto exit;
        }
    }
    else
    {
        ret = MSG_DATA_ERROR;
        goto exit;
    }
    stOTA.ddr_ptr = (u32 *)pBase;
    stOTA.ddr_ptr_index = SCPU_IMAGE_SIZE-4;
    #endif

    //step 1: get all bin files and 4 alignment  //from USB buffer
    //init OTA ptr and variables
    Drv_OTA_init( &stOTA );

    //step 2: init config files
    //2-1
    stOTA.receive_cmd.cmd_stat = (UINT16) FLASH_CMD_ACT;
    stOTA.receive_cmd.action_number = FLASH_CMD_ACT_NUM_INIT;
    if(  drv_flash_main( &stOTA ) != FLASH_DRV_OK )
    {
        ret = MSG_AUTH_FAIL;
        goto exit;
    }
    //2-2
    stOTA.receive_cmd.cmd_stat = (UINT16) FLASH_CMD_ACT;
    stOTA.receive_cmd.action_number = FLASH_CMD_ACT_NUM_SEL_SCPU;
    if(  drv_flash_main( &stOTA ) != FLASH_DRV_OK )
    {
        ret = MSG_DATA_ERROR;
        goto exit;
    }
    //step 3: setting some parameters large program
    stOTA.receive_cmd.cmd_stat = (UINT16) FLASH_CMD_ACT;
    stOTA.receive_cmd.action_number = FLASH_CMD_ACT_NUM_PROC_DONE;

    //need to check the following parameter
    stOTA.ddr_ptr_index = SCPU_IMAGE_SIZE;        //need to assign correct bin size!
    stOTA.ddr_ptr = (UINT32*)0x60000000;        //need to assign an address, 4 alignment

    if(  drv_flash_main( &stOTA ) != FLASH_DRV_OK )
    {
        ret = MSG_FLASH_FAIL;
        goto exit;
    }

exit:
    return ret;
}

int ota_update_ncpu(void)
{
    int ret = OTA_UPDATE_SUCCESS;

    #ifdef USE_KDRV
    u32 local_sum32, remote_sum32;
    u8 *pBase;
    pBase = (u8 *)KDP_DDR_OTA_FLASH_BUF_START_ADDR;        //KDP_DDR_BASE;
    memset(pBase, 0, NCPU_IMAGE_SIZE);
    ret = fn_read_data((u32)pBase, NCPU_IMAGE_SIZE);        // fn_read_data = kcomm_read,


    if( ret == NCPU_IMAGE_SIZE )
    {
        local_sum32 = kdp_gen_sum32(pBase, NCPU_IMAGE_SIZE - 4);
        remote_sum32 = *(u32 *)(pBase + NCPU_IMAGE_SIZE - 4);
        if (local_sum32 != remote_sum32) {
            ret = MSG_AUTH_FAIL;
            goto exit;
        }
    }
    else
    {
        ret = MSG_DATA_ERROR;
        goto exit;
    }
    stOTA.ddr_ptr = (u32 *)pBase;
    stOTA.ddr_ptr_index = NCPU_IMAGE_SIZE-4;
    #endif

    //step 1: get all bin files and 4 alignment  //from USB buffer
    //init OTA ptr and variables
    Drv_OTA_init( &stOTA );
    //step 2: init config files
    //2-1
    stOTA.receive_cmd.cmd_stat = (UINT16) FLASH_CMD_ACT;
    stOTA.receive_cmd.action_number = FLASH_CMD_ACT_NUM_INIT;
    if(  drv_flash_main( &stOTA ) != FLASH_DRV_OK )
    {
        ret = MSG_AUTH_FAIL;
        goto exit;
    }
    //2-2
    stOTA.receive_cmd.cmd_stat = (UINT16) FLASH_CMD_ACT;
    stOTA.receive_cmd.action_number = FLASH_CMD_ACT_NUM_SEL_NCPU;
    if(  drv_flash_main( &stOTA ) != FLASH_DRV_OK )
    {
        ret = MSG_DATA_ERROR;
        goto exit;
    }
    //step 3: setting some parameters large program
    stOTA.receive_cmd.cmd_stat = (UINT16) FLASH_CMD_ACT;
    stOTA.receive_cmd.action_number = FLASH_CMD_ACT_NUM_PROC_DONE;

    //need to check the following parameter
    stOTA.ddr_ptr_index = NCPU_IMAGE_SIZE;        //need to assign correct bin size!
    stOTA.ddr_ptr = (UINT32*)0x60000000;        //need to assign an address, 4 alignment

    if(  drv_flash_main( &stOTA ) != FLASH_DRV_OK )
    {
        ret = MSG_FLASH_FAIL;
        goto exit;
    }

exit:
    return ret;
}

int ota_update_model(u32 size)
{
    #ifdef USE_KDRV
    int ret;
    u32 sum32_download, sum32_embedded;
    u32 ddr_buf;

    ddr_buf = KDP_DDR_OTA_FLASH_BUF_START_ADDR;
    ret = fn_read_data(ddr_buf, size);

    if (ret == size) {
        sum32_embedded = *(u32 *)(ddr_buf + size - 4);
        sum32_download = kdp_gen_sum32((u8 *)ddr_buf, size - 4);
        if (sum32_embedded != sum32_download)
        {
            // ota_update_abort();
            return MSG_AUTH_FAIL;
        }
    } else {
        // ota_update_abort();
        return MSG_DATA_ERROR;
    }
    stOTA.ddr_ptr = (u32 *)ddr_buf;
    stOTA.ddr_ptr_index = size-4;
    #endif

    //step 1: get all bin files and 4 alignment  //from USB buffer
    //init OTA ptr and variables
    Drv_OTA_init( &stOTA );
    //step 2: init config files
    //2-1
    stOTA.receive_cmd.cmd_stat = (UINT16) FLASH_CMD_ACT;
    stOTA.receive_cmd.action_number = FLASH_CMD_ACT_NUM_INIT;
    if(  drv_flash_main( &stOTA ) != FLASH_DRV_OK )
    {
        return MSG_AUTH_FAIL;
    }
    //2-2
    stOTA.receive_cmd.cmd_stat = (UINT16) FLASH_CMD_ACT;
    stOTA.receive_cmd.action_number = FLASH_CMD_ACT_NUM_SEL_MODEL;
    if(  drv_flash_main( &stOTA ) != FLASH_DRV_OK )
    {
        return MSG_DATA_ERROR;

    }
    //step 3: setting some parameters large program
    stOTA.receive_cmd.cmd_stat = (UINT16) FLASH_CMD_ACT;
    stOTA.receive_cmd.action_number = FLASH_CMD_ACT_NUM_PROC_DONE;

    //need to check the following parameter
    stOTA.ddr_ptr_index = size;        //need to assign correct bin size!
    stOTA.ddr_ptr = (UINT32*)0x60000000;        //need to assign an address, 4 alignment

    if(  drv_flash_main( &stOTA ) != FLASH_DRV_OK )
    {
        return MSG_FLASH_FAIL;
    }

    return OTA_UPDATE_SUCCESS;
}


int ota_update_case( u8 update_case, u32 data_addr, u32 size )
{
    #ifdef USE_KDRV
    int ret;
    u32 sum32_download, sum32_embedded;

    if (ret == size) {
        sum32_embedded = *(u32 *)(data_addr + size - 4);
        sum32_download = kdp_gen_sum32((u8 *)data_addr, size - 4);
        if (sum32_embedded != sum32_download)
        {
            // ota_update_abort();
            return OTA_AUTH_FAIL;
        }
    } else {
        // ota_update_abort();
        return OTA_DOWNLOAD_FAIL;
    }
    #endif

    dbg_msg_console("update_case = %d, data_addr = %#X, size = %#X ",update_case, data_addr,size);

    //step 1: get all bin files and 4 alignment  //from USB buffer
    //init OTA ptr and variables
    Drv_OTA_init( &stOTA );
    //step 2: init config files
    //2-1
    stOTA.receive_cmd.cmd_stat = (UINT16) FLASH_CMD_ACT;
    stOTA.receive_cmd.action_number = FLASH_CMD_ACT_NUM_INIT;

    stOTA.sector_offset = 0;
    stOTA.target_bytes = size;
    stOTA.target_sectors = stOTA.target_bytes>>12;

    if(  drv_flash_main( &stOTA ) != FLASH_DRV_OK )
    {
        return OTA_AUTH_FAIL;
    }
    //2-2
    stOTA.receive_cmd.cmd_stat = (UINT16) FLASH_CMD_ACT;
    stOTA.receive_cmd.action_number = update_case;
    if(  drv_flash_main( &stOTA ) != FLASH_DRV_OK )
    {
        return OTA_DOWNLOAD_FAIL;

    }
    //step 3: setting some parameters large program
    stOTA.receive_cmd.cmd_stat = (UINT16) FLASH_CMD_ACT;
    stOTA.receive_cmd.action_number = FLASH_CMD_ACT_NUM_PROC_DONE;

    //need to check the following parameter
    stOTA.ddr_ptr_index = size;//size-4        //need to assign correct bin size!
    stOTA.ddr_ptr = (UINT32*)data_addr;        //need to assign an address, 4 alignment

    if(  drv_flash_main( &stOTA ) != FLASH_DRV_OK )
    {
        return OTA_FLASH_FAIL;
    }

    return OTA_UPDATE_SUCCESS;
}

int ota_update_scpu_flag_status( void )
{
    UINT32	seq =0;

    if ( boot_cfg_0.scpu_cfg.flag & boot_cfg_1.scpu_cfg.flag & BOOT_STATE_CONFIRMED )
    {
    dbg_msg("OTA Error: scpu wrong partition number");
    return -1;
    }

    seq = MAX( boot_cfg_0.scpu_cfg.seq, boot_cfg_1.scpu_cfg.seq ) + 1;

    if( boot_cfg_0.scpu_cfg.flag == BOOT_STATE_CONFIRMED )
    {
        boot_cfg_1.scpu_cfg.flag = BOOT_STATE_FIRST_BOOT; // BOOT_STATE_CONFIRMED;
        boot_cfg_1.scpu_cfg.seq = seq;
    }
    else
    {
        boot_cfg_0.scpu_cfg.flag = BOOT_STATE_FIRST_BOOT; //BOOT_STATE_CONFIRMED;
        boot_cfg_0.scpu_cfg.seq = seq;
    }

    #if (OTA_CONFIG_LOG_EN==YES)
    dbg_msg("----------------------------------------");
    dbg_msg("	-config0: scpu: flag:0x%X, part:0x%X, seq:0x%X"
        , boot_cfg_0.scpu_cfg.flag
        , boot_cfg_0.scpu_cfg.partition_id
        ,boot_cfg_0.scpu_cfg.seq);
    dbg_msg("	-config1: scpu: flag:0x%X, part:0x%X, seq:0x%X"
        , boot_cfg_1.scpu_cfg.flag
        , boot_cfg_1.scpu_cfg.partition_id
        ,boot_cfg_1.scpu_cfg.seq);

    dbg_msg("	-config0: ncpu: flag:0x%X, part:0x%X, seq:0x%X"
        , boot_cfg_0.ncpu_cfg.flag
        , boot_cfg_0.ncpu_cfg.partition_id
        ,boot_cfg_0.ncpu_cfg.seq);
    dbg_msg("	-config1: ncpu: flag:0x%X, part:0x%X, seq:0x%X"
        , boot_cfg_1.ncpu_cfg.flag
        , boot_cfg_1.ncpu_cfg.partition_id
        ,boot_cfg_1.ncpu_cfg.seq);
    #endif

	return 0;
}

int ota_update_ncpu_flag_status( void )
{
    UINT32 seq;
    if ( boot_cfg_0.ncpu_cfg.flag & boot_cfg_1.ncpu_cfg.flag & BOOT_STATE_CONFIRMED )
    {
    dbg_msg("OTA Error: scpu wrong partition number");
    return -1;
    }

    seq = MAX( boot_cfg_0.ncpu_cfg.seq, boot_cfg_1.ncpu_cfg.seq ) + 1;

    if( boot_cfg_0.ncpu_cfg.flag == BOOT_STATE_CONFIRMED )
    {
        boot_cfg_1.ncpu_cfg.flag = BOOT_STATE_FIRST_BOOT;
        boot_cfg_1.ncpu_cfg.seq = seq;
    }
    else
    {
        boot_cfg_0.ncpu_cfg.flag = BOOT_STATE_FIRST_BOOT;
        boot_cfg_0.ncpu_cfg.seq = seq;
    }
    return 0;
}


int ota_update_switch_active_partition( u32 partition )
{
    UINT32 seq;
    int ret;

    if( ( partition != 1 ) && ( partition != 2 ) ) {
        dbg_msg("OTA Error: wrong partition number");
        return -1;
    }

    if ((boot_cfg_0.scpu_cfg.flag == 0xffffffff) && (boot_cfg_1.scpu_cfg.flag == 0xffffffff)) {
        // no config data is there, need to create them for partition 0/1
        ota_init_partition_boot_cfg();
        return 0;
    }

    if (boot_cfg_0.ncpu_cfg.flag & boot_cfg_1.ncpu_cfg.flag & BOOT_STATE_CONFIRMED)
    {
    	dbg_msg("OTA Critical Error: 2 active NCPU boot config");
        return -1;
    }

    if ((boot_cfg_0.ncpu_cfg.partition_id == boot_cfg_1.ncpu_cfg.partition_id)
        && (boot_cfg_0.ncpu_cfg.seq == boot_cfg_1.ncpu_cfg.seq))
    {
        // no config data is there, need to create them for partition 0/1
        ota_init_partition_boot_cfg();
        dbg_msg("OTA only one partition, cannot switch\n");
        return -1;
    }

    if ( partition == 1 )     //switch SCPU
    {
        /* determine if any update ever happened, if not, just return */
        if ( (boot_cfg_0.scpu_cfg.flag == BOOT_STATE_CONFIRMED) &&
            (boot_cfg_0.scpu_cfg.seq == 1) &&
            (boot_cfg_1.scpu_cfg.flag == BOOT_STATE_NOT_CONFIRMED) &&
            (boot_cfg_1.scpu_cfg.seq == 0) )
        {
        	dbg_msg("OTA Never have SCPU firmware updated, cannot switch");
            return -1;
        }

        if (boot_cfg_0.scpu_cfg.flag == BOOT_STATE_CONFIRMED)
        {
            boot_cfg_1.scpu_cfg.flag = BOOT_STATE_CONFIRMED;
            seq = MAX(boot_cfg_0.scpu_cfg.seq, boot_cfg_1.scpu_cfg.seq);
            boot_cfg_1.scpu_cfg.seq = seq + 1;
            boot_cfg_0.scpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;

        } else if (boot_cfg_1.scpu_cfg.flag == BOOT_STATE_CONFIRMED)
        {
            boot_cfg_0.scpu_cfg.flag = BOOT_STATE_CONFIRMED;
            seq = MAX(boot_cfg_0.scpu_cfg.seq, boot_cfg_1.scpu_cfg.seq);
            boot_cfg_0.scpu_cfg.seq = seq + 1;
            boot_cfg_1.scpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;
        }
    }

    if (partition == 2)     //switch NCPU
    {
        /* determine if any update ever happened, if not, just return */
        if ((boot_cfg_0.ncpu_cfg.flag == BOOT_STATE_CONFIRMED) &&
           (boot_cfg_0.ncpu_cfg.seq == 1) &&
           (boot_cfg_1.ncpu_cfg.flag == BOOT_STATE_NOT_CONFIRMED) &&
           (boot_cfg_1.ncpu_cfg.seq == 0))
        {
        	dbg_msg("OTA Never have NCPU firmware updated, cannot switch\n");
            return -1;
        }

        if ( boot_cfg_0.ncpu_cfg.flag == BOOT_STATE_CONFIRMED )
        {
            boot_cfg_1.ncpu_cfg.flag = BOOT_STATE_CONFIRMED;
            seq = MAX(boot_cfg_0.ncpu_cfg.seq, boot_cfg_1.ncpu_cfg.seq);
            boot_cfg_1.ncpu_cfg.seq = seq + 1;
            boot_cfg_0.ncpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;

        } else if ( boot_cfg_1.ncpu_cfg.flag == BOOT_STATE_CONFIRMED )
        {
            boot_cfg_0.ncpu_cfg.flag = BOOT_STATE_CONFIRMED;
            seq = MAX(boot_cfg_0.ncpu_cfg.seq, boot_cfg_1.ncpu_cfg.seq);
            boot_cfg_0.ncpu_cfg.seq = seq + 1;
            boot_cfg_1.ncpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;
        }
    }

    ret = kdp_memxfer_ddr_to_flash(  (UINT32)PARTITION_0_CFG_START_IN_FLASH , (UINT32)&boot_cfg_0,sizeof(boot_cfg_0));

    if ( ret == 0 )
    {
        dbg_msg("OTA Flash write fail on %x\n", PARTITION_0_CFG_START_IN_FLASH);
        return OTA_FLASH_FAIL;
    }

    ret = kdp_memxfer_ddr_to_flash(  (UINT32)PARTITION_1_CFG_START_IN_FLASH , (UINT32)&boot_cfg_1,sizeof(boot_cfg_1));

    if ( ret == 0 )
    {
        err_msg("OTA Flash write fail on %x\n", PARTITION_1_CFG_START_IN_FLASH);
        return OTA_FLASH_FAIL;
    }

//test use!!
//    kdp_flash_read_data( PARTITION_0_CFG_START_IN_FLASH, &boot_cfg_0  , sizeof(boot_cfg_0) );
//    kdp_flash_read_data( PARTITION_1_CFG_START_IN_FLASH, &boot_cfg_1  , sizeof(boot_cfg_1) );

    return OTA_UPDATE_SUCCESS;
}


void ota_burn_in_config( UINT8  partition )
{
    UINT8 ret = 0;

    if( partition & 0x1 )
    {
        if( boot_cfg_0.scpu_cfg.flag== BOOT_STATE_FIRST_BOOT || boot_cfg_0.ncpu_cfg.flag== BOOT_STATE_FIRST_BOOT )
        {
            kdp_memxfer_ddr_to_flash(  (UINT32)PARTITION_0_CFG_START_IN_FLASH , (UINT32)&boot_cfg_0,sizeof(boot_cfg_0));
            ret |= 0x01;
        }
    }

    if( partition & 0x2 )
    {
        if ( boot_cfg_1.scpu_cfg.flag== BOOT_STATE_FIRST_BOOT || boot_cfg_1.ncpu_cfg.flag== BOOT_STATE_FIRST_BOOT )
        {
            kdp_memxfer_ddr_to_flash( (UINT32)PARTITION_1_CFG_START_IN_FLASH , (UINT32)&boot_cfg_1, sizeof(boot_cfg_1));
            ret |= 0x02;
        }
    }

    kdp_memxfer_flash_to_ddr( (UINT32)&boot_cfg_0 , PARTITION_0_CFG_START_IN_FLASH, sizeof(boot_cfg_0) );
    kdp_memxfer_flash_to_ddr( (UINT32)&boot_cfg_1 , PARTITION_1_CFG_START_IN_FLASH, sizeof(boot_cfg_1) );

    dbg_msg_console("part %d update OK", partition );
}



int ota_update_force_switch_active_partition( u32 partition )
{
    u32 seq;
    int ret;

    if( ( partition != 1 ) && ( partition != 2 ) ) {
        dbg_msg("OTA Error: wrong partition number");
        return -1;
    }

    if ((boot_cfg_0.scpu_cfg.flag == 0xffffffff) && (boot_cfg_1.scpu_cfg.flag == 0xffffffff)) {
        // no config data is there, need to create them for partition 0/1
        ota_init_partition_boot_cfg();
        return 0;
    }

    if (boot_cfg_0.ncpu_cfg.flag & boot_cfg_1.ncpu_cfg.flag & BOOT_STATE_CONFIRMED)
    {
        dbg_msg("OTA Critical Error: 2 active NCPU boot config");
        return -1;
    }

    if ((boot_cfg_0.ncpu_cfg.partition_id == boot_cfg_1.ncpu_cfg.partition_id)
        && (boot_cfg_0.ncpu_cfg.seq == boot_cfg_1.ncpu_cfg.seq))
    {
        // no config data is there, need to create them for partition 0/1
        ota_init_partition_boot_cfg();
        dbg_msg("OTA only one partition, cannot switch\n");
        return -1;
    }

    if ( partition == 1 )     //switch SCPU
    {
        if (boot_cfg_0.scpu_cfg.flag == BOOT_STATE_CONFIRMED)
        {
            boot_cfg_1.scpu_cfg.flag = BOOT_STATE_CONFIRMED;
            seq = MAX(boot_cfg_0.scpu_cfg.seq, boot_cfg_1.scpu_cfg.seq);
            boot_cfg_1.scpu_cfg.seq = seq + 1;
            boot_cfg_0.scpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;

        } else if (boot_cfg_1.scpu_cfg.flag == BOOT_STATE_CONFIRMED)
        {
            boot_cfg_0.scpu_cfg.flag = BOOT_STATE_CONFIRMED;
            seq = MAX(boot_cfg_0.scpu_cfg.seq, boot_cfg_1.scpu_cfg.seq);
            boot_cfg_0.scpu_cfg.seq = seq + 1;
            boot_cfg_1.scpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;
        }
    }

    if (partition == 2)     //switch NCPU
    {
        if ( boot_cfg_0.ncpu_cfg.flag == BOOT_STATE_CONFIRMED )
        {
            boot_cfg_1.ncpu_cfg.flag = BOOT_STATE_CONFIRMED;
            seq = MAX(boot_cfg_0.ncpu_cfg.seq, boot_cfg_1.ncpu_cfg.seq);
            boot_cfg_1.ncpu_cfg.seq = seq + 1;
            boot_cfg_0.ncpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;

        } else if ( boot_cfg_1.ncpu_cfg.flag == BOOT_STATE_CONFIRMED )
        {
            boot_cfg_0.ncpu_cfg.flag = BOOT_STATE_CONFIRMED;
            seq = MAX(boot_cfg_0.ncpu_cfg.seq, boot_cfg_1.ncpu_cfg.seq);
            boot_cfg_0.ncpu_cfg.seq = seq + 1;
            boot_cfg_1.ncpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;
        }
    }

    ret = kdp_memxfer_ddr_to_flash(  (UINT32)PARTITION_0_CFG_START_IN_FLASH , (UINT32)&boot_cfg_0,sizeof(boot_cfg_0));

    if ( ret == 0 )
    {
        dbg_msg("OTA Flash write fail on %x\n", PARTITION_0_CFG_START_IN_FLASH);
        return OTA_FLASH_FAIL;
    }

    ret = kdp_memxfer_ddr_to_flash(  (UINT32)PARTITION_1_CFG_START_IN_FLASH , (UINT32)&boot_cfg_1,sizeof(boot_cfg_1));

    if ( ret == 0 )
    {
        err_msg("OTA Flash write fail on %x\n", PARTITION_1_CFG_START_IN_FLASH);
        return OTA_FLASH_FAIL;
    }

    return OTA_UPDATE_SUCCESS;
}

int ota_handle_first_time_boot(void)
{
    int ret;

    #ifdef USE_KDRV
    tmp_ver_buf = (u8*)KDP_DDR_OTA_FLASH_BUF_START_ADDR;
    fn_read_data = kcomm_read;
    #endif

    kdp_memxfer_init(MEMXFER_OPS_CPU, MEMXFER_OPS_CPU);

    /* read flash to mem */
    kdp_memxfer_flash_to_ddr( (UINT32)&boot_cfg_0 , PARTITION_0_CFG_START_IN_FLASH, sizeof(boot_cfg_0) );
    kdp_memxfer_flash_to_ddr( (UINT32)&boot_cfg_1 , PARTITION_1_CFG_START_IN_FLASH, sizeof(boot_cfg_1) );

    #if (OTA_CONFIG_LOG_EN==YES)
    //add for debug
    dbg_msg("SCPU config_0 flag 0x%x seq %d \n", boot_cfg_0.scpu_cfg.flag , boot_cfg_0.scpu_cfg.seq);
    dbg_msg("SCPU config_1 flag 0x%x seq %d \n", boot_cfg_1.scpu_cfg.flag , boot_cfg_1.scpu_cfg.seq);
    dbg_msg("NCPU config_0 flag 0x%x seq %d \n", boot_cfg_0.ncpu_cfg.flag , boot_cfg_0.ncpu_cfg.seq);
    dbg_msg("NCPU config_1 flag 0x%x seq %d \n", boot_cfg_1.ncpu_cfg.flag , boot_cfg_1.ncpu_cfg.seq);
    #endif


    if ((boot_cfg_0.scpu_cfg.flag == 0xffffffff) && (boot_cfg_1.scpu_cfg.flag == 0xffffffff)) {
        ret = -1;
        goto exit;
    }

    if ((boot_cfg_0.scpu_cfg.flag == BOOT_STATE_FIRST_BOOT) ||
        (boot_cfg_0.ncpu_cfg.flag == BOOT_STATE_FIRST_BOOT) ||
        (boot_cfg_1.scpu_cfg.flag == BOOT_STATE_FIRST_BOOT) ||
        (boot_cfg_1.ncpu_cfg.flag == BOOT_STATE_FIRST_BOOT)) {
        err_msg("Error: wrong state, BOOT_STATE_FIRST_BOOT shall not be here\n");
        ret = -1;
        goto exit;
    }

    /* determine if necessary to read flash */

    if ((boot_cfg_0.scpu_cfg.flag != BOOT_STATE_POST_FIRST_BOOT) &&
        (boot_cfg_0.ncpu_cfg.flag != BOOT_STATE_POST_FIRST_BOOT) &&
        (boot_cfg_1.scpu_cfg.flag != BOOT_STATE_POST_FIRST_BOOT) &&
        (boot_cfg_1.ncpu_cfg.flag != BOOT_STATE_POST_FIRST_BOOT)) {

        ret = 0;
        goto exit;
    }

    if (boot_cfg_0.scpu_cfg.flag == BOOT_STATE_POST_FIRST_BOOT) {
        boot_cfg_0.scpu_cfg.flag = BOOT_STATE_CONFIRMED;
        if(boot_cfg_1.scpu_cfg.flag == BOOT_STATE_CONFIRMED) {
            boot_cfg_1.scpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;
        }
        err_msg("SCPU partition 0 was confirmed\n");

    }

    if (boot_cfg_0.ncpu_cfg.flag == BOOT_STATE_POST_FIRST_BOOT) {
        boot_cfg_0.ncpu_cfg.flag = BOOT_STATE_CONFIRMED;
        if(boot_cfg_1.ncpu_cfg.flag == BOOT_STATE_CONFIRMED) {
            boot_cfg_1.ncpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;
        }
        err_msg("NCPU partition 0 was confirmed\n");
    }

    if (boot_cfg_1.scpu_cfg.flag == BOOT_STATE_POST_FIRST_BOOT) {
        boot_cfg_1.scpu_cfg.flag = BOOT_STATE_CONFIRMED;
        if(boot_cfg_0.scpu_cfg.flag == BOOT_STATE_CONFIRMED) {
            boot_cfg_0.scpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;
        }
        err_msg("SCPU partition 1 was confirmed\n");
    }

    if (boot_cfg_1.ncpu_cfg.flag == BOOT_STATE_POST_FIRST_BOOT) {
        boot_cfg_1.ncpu_cfg.flag = BOOT_STATE_CONFIRMED;
        if(boot_cfg_0.ncpu_cfg.flag == BOOT_STATE_CONFIRMED) {
            boot_cfg_0.ncpu_cfg.flag = BOOT_STATE_NOT_CONFIRMED;
        }
        err_msg("NCPU partition 1 was confirmed\n");
    }

    ret = kdp_memxfer_ddr_to_flash((UINT32)PARTITION_0_CFG_START_IN_FLASH , (UINT32)&boot_cfg_0,sizeof(boot_cfg_0));

    if (ret == 0 )
    {
        err_msg("Flash write fail on %x\n", PARTITION_0_CFG_START_IN_FLASH);
        ret = OTA_FLASH_FAIL;
        goto exit;
    }

    ret = kdp_memxfer_ddr_to_flash((UINT32)PARTITION_1_CFG_START_IN_FLASH , (UINT32)&boot_cfg_1,sizeof(boot_cfg_1));
    if (ret == 0 )
    {
        err_msg("Flash write fail on %x\n", PARTITION_1_CFG_START_IN_FLASH);
        ret = OTA_FLASH_FAIL;
    } else {
        ret = 0;
    }



exit:
    #if (OTA_CONFIG_LOG_EN==YES)
    //for debug
    dbg_msg("SCPU config_0 flag 0x%x seq %d \n", boot_cfg_0.scpu_cfg.flag , boot_cfg_0.scpu_cfg.seq);
    dbg_msg("SCPU config_1 flag 0x%x seq %d \n", boot_cfg_1.scpu_cfg.flag , boot_cfg_1.scpu_cfg.seq);
    dbg_msg("NCPU config_0 flag 0x%x seq %d \n", boot_cfg_0.ncpu_cfg.flag , boot_cfg_0.ncpu_cfg.seq);
    dbg_msg("NCPU config_1 flag 0x%x seq %d \n", boot_cfg_1.ncpu_cfg.flag , boot_cfg_1.ncpu_cfg.seq);
    #endif

    power_manager_register(PM_DEVICE_OTA_UPDATE, &ota_update_pm);

    return ret;
}

// restore optimization
//#pragma GCC pop_options



void ota_update_show_config(void)
{
    boot_cfg_0.ncpu_cfg.flag =0xFF;
    boot_cfg_1.ncpu_cfg.flag =0xFF;

    kdp_memxfer_flash_to_ddr((UINT32)&boot_cfg_0, PARTITION_0_CFG_START_IN_FLASH , sizeof( boot_cfg_0) );
    kdp_memxfer_flash_to_ddr((UINT32)&boot_cfg_1, PARTITION_1_CFG_START_IN_FLASH , sizeof( boot_cfg_1) );
    dbg_msg_console("SCPU config_0 flag 0x%x seq %d \n", boot_cfg_0.scpu_cfg.flag , boot_cfg_0.scpu_cfg.seq);
    dbg_msg_console("SCPU config_1 flag 0x%x seq %d \n", boot_cfg_1.scpu_cfg.flag , boot_cfg_1.scpu_cfg.seq);
    dbg_msg_console("NCPU config_0 flag 0x%x seq %d \n", boot_cfg_0.ncpu_cfg.flag , boot_cfg_0.ncpu_cfg.seq);
    dbg_msg_console("NCPU config_1 flag 0x%x seq %d \n", boot_cfg_1.ncpu_cfg.flag , boot_cfg_1.ncpu_cfg.seq);
    dbg_msg_console("============================== \n");
}

int ota_get_scpu_flag_status( void )
{
    int ret = -1;
 
    boot_cfg_0.ncpu_cfg.flag =0xFF;
    boot_cfg_1.ncpu_cfg.flag =0xFF;

    kdp_memxfer_flash_to_ddr((UINT32)&boot_cfg_0, PARTITION_0_CFG_START_IN_FLASH , sizeof( boot_cfg_0) );
    kdp_memxfer_flash_to_ddr((UINT32)&boot_cfg_1, PARTITION_1_CFG_START_IN_FLASH , sizeof( boot_cfg_1) );

    if( boot_cfg_0.scpu_cfg.flag == USER_STATE_ACTIVE )
    {
        ret = 0;
    }
    
    if( boot_cfg_1.scpu_cfg.flag == USER_STATE_ACTIVE )
    {
        ret = 1;
    }
    
    return ret;
}


void ota_update_check_scpu_ota(u8 index)
{
    int area;
    u32 ncrc = 0;
    area = ota_get_active_scpu_partition();
    if( area == 0 )
    {
        if(  boot_cfg_1.scpu_cfg.flag == BOOT_STATE_POST_FIRST_BOOT  )
        {
            #if(OTA_BOOT_CONFIG_LOG==YES)
            dbg_msg_flash("scpu length : %d ", KDP_FLASH_FW_SCPU_SIZE-20 );
            #endif
            //check SCPU CRC
            kdp_memxfer_flash_to_ddr( (u32)KDP_DDR_MODEL_START_ADDR, KDP_FLASH_FW_SCPU1_ADDR , (KDP_FLASH_FW_SCPU_SIZE-20) );
            ncrc = ota_crc32( (u8 *)KDP_DDR_MODEL_START_ADDR, (KDP_FLASH_FW_SCPU_SIZE-20) );
            #if(OTA_BOOT_CONFIG_LOG==YES)
            dbg_msg_flash("scpu area1: 0x%x , crc_ans:0x%x ",ncrc, gscpu_crc );
            #endif

            if( ncrc == gscpu_crc )
            {
                gota_judge_pool[index] = 0x80;
                gota_judge_pool[index] += OTA_STAT_BOOT_JUDGE_OK;
            }
            else
            {
                gota_judge_pool[index] = area << 7;
                gota_judge_pool[index] += OTA_STAT_BOOT_JUDGE_CRC_FAIL;
            }
        }
        else if(  boot_cfg_1.scpu_cfg.flag == BOOT_STATE_FIRST_BOOT )
        {
            gota_judge_pool[index] = area<<7;
            gota_judge_pool[index] += OTA_STAT_BOOT_JUDGE_OTA_FAIL;
        }
        else
        {
            gota_judge_pool[index] = area<<7;
            gota_judge_pool[index] += OTA_STAT_NOTHING;
            #if(OTA_BOOT_CONFIG_LOG==YES)
            dbg_msg_flash("scpu active area: %d",area);
            #endif
        }
    }
    else if( area == 1 )
    {

        if(  boot_cfg_0.scpu_cfg.flag == BOOT_STATE_POST_FIRST_BOOT )
        {
            //check SCPU CRC
            kdp_memxfer_flash_to_ddr( (u32)KDP_DDR_MODEL_START_ADDR, KDP_FLASH_FW_SCPU_ADDR , KDP_FLASH_FW_SCPU_SIZE-20 );
            ncrc = ota_crc32( (u8 *)KDP_DDR_MODEL_START_ADDR, KDP_FLASH_FW_SCPU_SIZE-20 );
            //check SCPU CRC
            if( ncrc == gscpu_crc )
            {
                gota_judge_pool[index] = 0x00;
                gota_judge_pool[index] += OTA_STAT_BOOT_JUDGE_OK;
            }
            else
            {
                gota_judge_pool[index] = area << 7;
                gota_judge_pool[index] += OTA_STAT_BOOT_JUDGE_CRC_FAIL;
            }
        }
        else if(  boot_cfg_0.scpu_cfg.flag == BOOT_STATE_FIRST_BOOT )
        {
            gota_judge_pool[index] = area<<7;
            gota_judge_pool[index] += OTA_STAT_BOOT_JUDGE_OTA_FAIL;
        }
        else
        {
            gota_judge_pool[index] = area<<7;
            gota_judge_pool[index] += OTA_STAT_NOTHING;
            #if(OTA_BOOT_CONFIG_LOG==YES)
            dbg_msg_flash("scpu active area: %d",area);
            #endif
        }

    }
}

void ota_update_check_ncpu_ota(u8 index)
{
    int area;
    u32 ncrc = 0;


    area = ota_get_active_ncpu_partition();

    if( area == 0 )
    {
        if(  boot_cfg_1.ncpu_cfg.flag == BOOT_STATE_POST_FIRST_BOOT  )
        {
            //check NCPU CRC
            kdp_memxfer_flash_to_ddr( (u32)KDP_DDR_MODEL_START_ADDR, KDP_FLASH_FW_NCPU1_ADDR , KDP_FLASH_FW_NCPU_SIZE );
            ncrc = ota_crc32( (u8 *)KDP_DDR_MODEL_START_ADDR, KDP_FLASH_FW_NCPU_SIZE );
            if( ncrc == gncpu_crc )
            {
                gota_judge_pool[index] = 0x80;
                gota_judge_pool[index] += OTA_STAT_BOOT_JUDGE_OK;
            }
            else
            {
                gota_judge_pool[index] = area << 7;
                gota_judge_pool[index] += OTA_STAT_BOOT_JUDGE_CRC_FAIL;
            }
        }
        else if(  boot_cfg_1.ncpu_cfg.flag == BOOT_STATE_FIRST_BOOT )
        {
            gota_judge_pool[index] = area<<7;
            gota_judge_pool[index] += OTA_STAT_BOOT_JUDGE_OTA_FAIL;
        }
        else
        {
            gota_judge_pool[index] = area<<7;
            gota_judge_pool[index] += OTA_STAT_NOTHING;
            #if(OTA_BOOT_CONFIG_LOG==YES)
            dbg_msg_flash("ncpu active area: %d",area);
            #endif
        }
     }
     else if( area == 1 )
     {
         if(  boot_cfg_0.ncpu_cfg.flag == BOOT_STATE_POST_FIRST_BOOT )
         {
             kdp_memxfer_flash_to_ddr( (u32)KDP_DDR_MODEL_START_ADDR, KDP_FLASH_FW_NCPU_ADDR , KDP_FLASH_FW_NCPU_SIZE );
             ncrc = ota_crc32( (u8 *)KDP_DDR_MODEL_START_ADDR, KDP_FLASH_FW_NCPU_SIZE );
            if( ncrc == gncpu_crc )
            {
                gota_judge_pool[index] = 0x00;
                gota_judge_pool[index] += OTA_STAT_BOOT_JUDGE_OK;
            }
            else
            {
                gota_judge_pool[index] = area << 7;
                gota_judge_pool[index] += OTA_STAT_BOOT_JUDGE_CRC_FAIL;
            }
         }
         else if(  boot_cfg_0.ncpu_cfg.flag == BOOT_STATE_FIRST_BOOT )
         {
             gota_judge_pool[index] = area<<7;
             gota_judge_pool[index] += OTA_STAT_BOOT_JUDGE_OTA_FAIL;
         }
         else
         {
             gota_judge_pool[index] = area<<7;
             gota_judge_pool[index] += OTA_STAT_NOTHING;
            #if(OTA_BOOT_CONFIG_LOG==YES)
            dbg_msg_flash("ncpu active area: %d",area);
            #endif
         }
     }
}




#if (OTA_USER_BACKUP == YES )

int ota_update_user_judge( u32 *flag_0, u32 *flag_1 ,u8 npartition )
{
    u32 crc_result =0 ;
    u8 active_area = 0xFF;
    u32 address_0 = 0, address_1 = 0;
    u32 bin_size;
    u32 crc_ans;

    if( npartition == USER_PARTITION_FW_INFO)
    {
        address_0 = KDP_FLASH_FW_INFO_ADDR;
        address_1 = KDP_FLASH_FW_INFO_ADDR+KDP_FLASH_FW_INFO_OFFSET_1;
        bin_size = KDP_FLASH_FW_INFO_SIZE;
        crc_ans = gfwinfo_crc;
    }
    else if( npartition == USER_PARTITION_MODEL)
    {
        address_0 = KDP_FLASH_ALL_MODELS_ADDR;
        address_1 = KDP_FLASH_ALL_MODELS_ADDR+KDP_FLASH_ALL_MODEL_OFFSET_1;
        bin_size = KDP_FLASH_ALL_MODELS_SIZE;
        crc_ans = gmodel_crc;
    }
    else if( npartition == USER_PARTITION_UI_IMG )
    {
        address_0 = USR_FLASH_SETTINGS_ADDR;
        address_1 = USR_FLASH_SETTINGS_ADDR+KDP_FLASH_USER_OFFSET_1;
        bin_size = USR_FLASH_LAST_ADDR - USR_FLASH_SETTINGS_ADDR;
        crc_ans = gui_crc;
    }
    else
    {
        return -1;
    }

    #if(OTA_BOOT_CONFIG_LOG==YES)
    dbg_msg_flash("[user_judge] partition: %d", npartition );
    #endif
    if( (*flag_1 == USER_STATE_ACTIVE) && (*flag_0 == USER_STATE_INACTIVE ) )
    {
        #if(OTA_BOOT_CONFIG_LOG==YES)
        dbg_msg_flash("[user_judge] good area: %d", 1 );
        #endif
        gota_judge_pool[npartition] = 0x80;
        //pass
        return 1;
    }
    else if( (*flag_0 == USER_STATE_ACTIVE) && (*flag_1 == USER_STATE_INACTIVE ) )
    {
        #if(OTA_BOOT_CONFIG_LOG==YES)
        dbg_msg_flash("[user_judge] good area: %d", 0 );
        #endif
        gota_judge_pool[npartition] = 0x00;
        //pass
        return 1;
    }
    else
    {
        active_area = ota_user_get_active_area_partial( npartition );

        #if(OTA_BOOT_CONFIG_LOG==YES)
        dbg_msg_flash("[user_judge] area judge doing, previous area: %d", active_area );
        #endif

        if( active_area == 0 )
        {
            if( *flag_1 == USER_STATE_ON_GOING )
            {
                gota_judge_pool[npartition] = active_area << 7;
                gota_judge_pool[npartition] += OTA_STAT_BOOT_JUDGE_OTA_FAIL;
                #if(OTA_BOOT_CONFIG_LOG==YES)
                dbg_msg_flash("[user_judge] area judge doing, previous area: %d, new area flag error", (int)active_area );
                #endif
            }
            else
            {
                #if(OTA_BOOT_CONFIG_LOG==YES)
                dbg_msg_flash("[user_judge] CRC check start 1" );
                #endif
                kdp_memxfer_flash_to_ddr( (u32)KDP_DDR_MODEL_START_ADDR, address_1 , bin_size );
                crc_result = ota_crc32((u8 *)KDP_DDR_MODEL_START_ADDR, bin_size );
                if(  crc_result == crc_ans )
                {
                    gota_judge_pool[npartition] = active_area << 7;
                    gota_judge_pool[npartition] += OTA_STAT_BOOT_JUDGE_OK;
                    #if(OTA_BOOT_CONFIG_LOG==YES)
                    dbg_msg_flash("[user_judge] area judge doing, previous area: %d, new area crc pass", (int)active_area );
                    #endif
                }
                else
                {
                    gota_judge_pool[npartition] = active_area << 7;
                    gota_judge_pool[npartition] += OTA_STAT_BOOT_JUDGE_CRC_FAIL;
                    #if(OTA_BOOT_CONFIG_LOG==YES)
                    dbg_msg_flash("[user_judge] area judge doing, previous area: %d, new area crc fail", (int)active_area );
                    #endif
                }
            }
        }
        else if( active_area == 1 )
        {
            if( *flag_0 == USER_STATE_ON_GOING )
            {
                gota_judge_pool[npartition] = active_area << 7;
                gota_judge_pool[npartition] += OTA_STAT_BOOT_JUDGE_OTA_FAIL;
                #if(OTA_BOOT_CONFIG_LOG==YES)
                dbg_msg_flash("[user_judge] area judge doing, previous area: %d, new area flag error", (int)active_area );
                #endif
            }
            else
            {
                #if(OTA_BOOT_CONFIG_LOG==YES)
                dbg_msg_flash("[user_judge] CRC check start 0" );
                #endif
                //check CRC
                kdp_memxfer_flash_to_ddr( (u32)KDP_DDR_MODEL_START_ADDR, address_0 , bin_size );
                crc_result = ota_crc32((u8 *)KDP_DDR_MODEL_START_ADDR, bin_size );
                if(  crc_result == crc_ans )
                {
                    gota_judge_pool[npartition] = active_area << 7;
                    gota_judge_pool[npartition] += OTA_STAT_BOOT_JUDGE_OK;
                    #if(OTA_BOOT_CONFIG_LOG==YES)
                    dbg_msg_flash("[user_judge] area judge doing, previous area: %d, new area crc pass", (int)active_area );
                    #endif
                }
                else
                {

                    gota_judge_pool[npartition] = active_area << 7;
                    gota_judge_pool[npartition] += OTA_STAT_BOOT_JUDGE_CRC_FAIL;
                    #if(OTA_BOOT_CONFIG_LOG==YES)
                    dbg_msg_flash("[user_judge] area judge doing, previous area: %d, new area crc fail", (int)active_area );
                    #endif
                }
            }
        }
    }
    return 1;
}


extern void power_mgr_sw_reset(void);
void ota_update_pool_check(void)
{
    int i;
    u32 result = 0xFF;
    u8 error_cnt = 0;
    u8 area = 0xFF;

    #if(OTA_BOOT_CONFIG_LOG==YES)
    dbg_msg_flash( "[pool check] begin" );
    #endif

    //scan error flag
    for( i=0; i < OTA_JUDGE_SIZE; i++  )
    {
        result = gota_judge_pool[i]&0x0F;
        if( (result == OTA_STAT_BOOT_JUDGE_OTA_FAIL) || (result == OTA_STAT_BOOT_JUDGE_CRC_FAIL) )
        {
            error_cnt++;
        }
    }
    #if(OTA_BOOT_CONFIG_LOG==YES)
    dbg_msg_flash( "[pool check] error count: %d", error_cnt );
    #endif
    if( error_cnt == 0  )
    {
        #if(OTA_BOOT_CONFIG_LOG==YES)
        dbg_msg_flash( "[pool check] all pass" );
        #endif
        //do nothing
        return;
    }
    for( i= (OTA_JUDGE_SIZE-1); i >=0 ; i-- )
    {
        result = gota_judge_pool[i]&0x0F;
        area = ( (gota_judge_pool[i]&0x80)>0 )?1:0;

        if( i == USER_PARTITION_SCPU )
        {
            if( error_cnt > 0 )
            {
                //watchdog reset !
                #if(OTA_BOOT_CONFIG_LOG==YES)
                dbg_msg_flash( "[pool check] scpu check error and reboot" );
                #endif
                power_mgr_sw_reset();
            }
        }
        else if( i == USER_PARTITION_NCPU )
        {
            continue;
        }
        else if( i == USER_PARTITION_FW_INFO )
        {
            if(result != OTA_STAT_NOTHING)
            {
                ota_user_config_init_partial( USER_PARTITION_FW_INFO, area );
            }
        }
        else if( i == USER_PARTITION_MODEL )
        {
            if(result != OTA_STAT_NOTHING)
            {
                ota_user_config_init_partial( USER_PARTITION_MODEL, area );
            }
        }
        else if( i == USER_PARTITION_UI_IMG )
        {
            if(result != OTA_STAT_NOTHING)
            {
                ota_user_config_init_partial( USER_PARTITION_UI_IMG, area );
            }
        }
        else
        {
            #if(OTA_BOOT_CONFIG_LOG==YES)
            dbg_msg_flash( "[pool check] error partition");
            #endif
        }
    }
}



void ota_update_get_crc(void)
{
    u8  area =0;

    if( boot_cfg_1.scpu_cfg.flag == BOOT_STATE_POST_FIRST_BOOT   )
    {
        area = 1;
    }
    else if( boot_cfg_0.scpu_cfg.flag == BOOT_STATE_POST_FIRST_BOOT   )
    {
        area = 0;
    }
    else if( boot_cfg_0.scpu_cfg.flag == BOOT_STATE_CONFIRMED )
    {
        area = 0;
    }
    else if( boot_cfg_1.scpu_cfg.flag == BOOT_STATE_CONFIRMED )
    {
        area = 1;
    }
    else
    {
        area = 0;
        dbg_msg_flash("no correct area" );
    }


    dbg_msg_flash("-----area 0x%x", area );
    if( area == 0  )
    {
        kdp_memxfer_flash_to_ddr( (u32)KDP_DDR_MODEL_START_ADDR, KDP_FLASH_FW_SCPU_ADDR , KDP_FLASH_FW_SCPU_SIZE );
    }
    else
    {
        kdp_memxfer_flash_to_ddr( (u32)KDP_DDR_MODEL_START_ADDR, KDP_FLASH_FW_SCPU1_ADDR , KDP_FLASH_FW_SCPU_SIZE );
    }

    gscpu_crc = *((u32 *)(KDP_DDR_MODEL_START_ADDR + KDP_FLASH_FW_SCPU_SIZE - SCPU_CRC_START + SCPU_CRC_OFFSET ));
    gncpu_crc = *((u32 *)(KDP_DDR_MODEL_START_ADDR + KDP_FLASH_FW_SCPU_SIZE - SCPU_CRC_START + NCPU_CRC_OFFSET ));
    gfwinfo_crc = *((u32 *)(KDP_DDR_MODEL_START_ADDR + KDP_FLASH_FW_SCPU_SIZE - SCPU_CRC_START + FW_INFO_CRC_OFFSET ));
    gmodel_crc = *((u32 *)(KDP_DDR_MODEL_START_ADDR + KDP_FLASH_FW_SCPU_SIZE - SCPU_CRC_START + MODEL_CRC_OFFSET ));
    gui_crc = *((u32 *)(KDP_DDR_MODEL_START_ADDR + KDP_FLASH_FW_SCPU_SIZE - SCPU_CRC_START + UI_CRC_OFFSET ));

//    #if( OTA_BOOT_CONFIG_LOG == YES )
//    dbg_msg_flash("gscpu_crc 0x%x", gscpu_crc );
//    dbg_msg_flash("gncpu_crc 0x%x", gncpu_crc );
//    dbg_msg_flash("gfwinfo_crc 0x%x", gfwinfo_crc );
//    dbg_msg_flash("gmodel_crc 0x%x", gmodel_crc );
//    dbg_msg_flash("gui_crc 0x%x", gui_crc );
//    #endif
}


void ota_update_boot_judge(void)
{
    int i;

    dbg_msg_console("=boot judge start=");

    kdp_memxfer_init(MEMXFER_OPS_CPU, MEMXFER_OPS_CPU);
    ota_get_active_scpu_partition();
    ota_user_read_cfg();

    //get crc data
    ota_update_get_crc();

    #if(OTA_BOOT_CONFIG_LOG==YES)
    ota_user_debug_show();
    #endif
    //initial buffer pool
    for( i=0; i < OTA_JUDGE_SIZE; i++ )   { gota_judge_pool[i]= 0x0F; }

    //step 1:   UI check
    ota_update_user_judge( &user_cfg_0.ui_img.flag, &user_cfg_1.ui_img.flag, USER_PARTITION_UI_IMG);
    //step 2-1:   fw_info check
    ota_update_user_judge( &user_cfg_0.fw_info.flag, &user_cfg_1.fw_info.flag, USER_PARTITION_FW_INFO );
    //step 2-2:   model check
    ota_update_user_judge( &user_cfg_0.model.flag, &user_cfg_1.model.flag, USER_PARTITION_MODEL );
    //step 3:   NCPU
    ota_update_check_ncpu_ota(USER_PARTITION_NCPU);
    //step 4:   SCPU
    ota_update_check_scpu_ota(USER_PARTITION_SCPU);

    #if(OTA_BOOT_CONFIG_LOG==YES)
    for( i = 0; i < OTA_JUDGE_SIZE; i++  )
    {
        dbg_msg_flash("[data pool] %d, 0x%x", i, gota_judge_pool[i] );
    }
    #endif
    ota_update_pool_check();

//    #if(OTA_BOOT_CONFIG_LOG==YES)
//    dbg_msg_console("=judge end=");
//    #endif

}


void ota_update_user_cfg(void)
{
    kdp_memxfer_flash_sector_multi_erase( KDP_FLASH_USER_CFG0_ADDR, KDP_FLASH_USER_CFG0_ADDR );
    kdp_memxfer_flash_sector_multi_erase( KDP_FLASH_USER_CFG1_ADDR, KDP_FLASH_USER_CFG1_ADDR );
    kdp_memxfer_ddr_to_flash( (UINT32)KDP_FLASH_USER_CFG0_ADDR, (UINT32)&user_cfg_0, sizeof(user_cfg_0));
    kdp_memxfer_ddr_to_flash( (UINT32)KDP_FLASH_USER_CFG1_ADDR, (UINT32)&user_cfg_1, sizeof(user_cfg_1));
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_0,  KDP_FLASH_USER_CFG0_ADDR , sizeof( user_cfg_0) );
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_1,  KDP_FLASH_USER_CFG1_ADDR , sizeof( user_cfg_1) );

}

void ota_user_debug_show(void)
{
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_0,  KDP_FLASH_USER_CFG0_ADDR , sizeof( user_cfg_0) );
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_1,  KDP_FLASH_USER_CFG1_ADDR , sizeof( user_cfg_1) );
    #if(OTA_BOOT_CONFIG_LOG==YES)
    dbg_msg_console("fw_infow config_0 ID 0x%x seq: %d, flag: %d", user_cfg_0.fw_info.partition_id, user_cfg_0.fw_info.seq , user_cfg_0.fw_info.flag );
    dbg_msg_console("model config_0    ID 0x%x seq: %d, flag: %d", user_cfg_0.model.partition_id, user_cfg_0.model.seq , user_cfg_0.model.flag );
    dbg_msg_console("ui_img config_0   ID 0x%x seq: %d, flag: %d", user_cfg_0.ui_img.partition_id, user_cfg_0.ui_img.seq , user_cfg_0.ui_img.flag );
    dbg_msg_console("fw_infow config_1 ID 0x%x seq: %d, flag: %d", user_cfg_1.fw_info.partition_id, user_cfg_1.fw_info.seq , user_cfg_1.fw_info.flag );
    dbg_msg_console("model config_1    ID 0x%x seq: %d, flag: %d", user_cfg_1.model.partition_id, user_cfg_1.model.seq , user_cfg_1.model.flag );
    dbg_msg_console("ui_img config_1   ID 0x%x seq: %d, flag: %d", user_cfg_1.ui_img.partition_id, user_cfg_1.ui_img.seq , user_cfg_1.ui_img.flag );
    dbg_msg_console("----------------------------" );
    #endif
}

void ota_user_read_cfg(void)
{
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_0,  KDP_FLASH_USER_CFG0_ADDR , sizeof( user_cfg_0) );
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_1,  KDP_FLASH_USER_CFG1_ADDR , sizeof( user_cfg_1) );
}

void ota_user_config_init(void)
{
    user_cfg_0.fw_info.partition_id = 2;
    user_cfg_0.model.partition_id   = 3;
    user_cfg_0.ui_img.partition_id  = 4;
    user_cfg_0.fw_info.seq = 1;
    user_cfg_0.model.seq   = 1;
    user_cfg_0.ui_img.seq  = 1;

    user_cfg_0.fw_info.flag = USER_STATE_ACTIVE;
    user_cfg_0.model.flag   = USER_STATE_ACTIVE;
    user_cfg_0.ui_img.flag  = USER_STATE_ACTIVE;

    user_cfg_1.fw_info.partition_id = 2;
    user_cfg_1.model.partition_id   = 3;
    user_cfg_1.ui_img.partition_id  = 4;

    user_cfg_1.fw_info.seq = 0;
    user_cfg_1.model.seq   = 0;
    user_cfg_1.ui_img.seq  = 0;

    user_cfg_1.fw_info.flag = USER_STATE_INACTIVE;
    user_cfg_1.model.flag   = USER_STATE_INACTIVE;
    user_cfg_1.ui_img.flag  = USER_STATE_INACTIVE;

    //erase
    kdp_memxfer_flash_sector_multi_erase( KDP_FLASH_USER_CFG0_ADDR, KDP_FLASH_USER_CFG0_ADDR );
    kdp_memxfer_flash_sector_multi_erase( KDP_FLASH_USER_CFG1_ADDR, KDP_FLASH_USER_CFG1_ADDR );
    kdp_memxfer_ddr_to_flash( (UINT32)KDP_FLASH_USER_CFG0_ADDR, (UINT32)&user_cfg_0, sizeof(user_cfg_0));
    kdp_memxfer_ddr_to_flash( (UINT32)KDP_FLASH_USER_CFG1_ADDR, (UINT32)&user_cfg_1, sizeof(user_cfg_1));
    user_cfg_0.fw_info.flag = 0x7887;
    user_cfg_0.model.flag = 0x7887;
    user_cfg_0.ui_img.flag = 0x7887;
    user_cfg_1.fw_info.flag = 0x7887;
    user_cfg_1.model.flag = 0x7887;
    user_cfg_1.ui_img.flag = 0x7887;
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_0,  KDP_FLASH_USER_CFG0_ADDR , sizeof( user_cfg_0) );
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_1,  KDP_FLASH_USER_CFG1_ADDR , sizeof( user_cfg_1) );
    ota_user_debug_show();

}

void ota_user_config_init_partial( u8 partition , u8 idx)
{

    if( partition == USER_PARTITION_FW_INFO )
    {
        if( idx == 0 )
        {
            user_cfg_0.fw_info.flag = USER_STATE_ACTIVE;
            user_cfg_1.fw_info.flag = USER_STATE_INACTIVE;
        }
        else if( idx == 1 )
        {
            user_cfg_1.fw_info.flag = USER_STATE_ACTIVE;
            user_cfg_0.fw_info.flag = USER_STATE_INACTIVE;
        }
    }
    if( partition == USER_PARTITION_MODEL )
    {
        if( idx == 0 )
        {
            user_cfg_0.model.flag = USER_STATE_ACTIVE;
            user_cfg_1.model.flag = USER_STATE_INACTIVE;
        }
        else if( idx == 1 )
        {
            user_cfg_1.model.flag = USER_STATE_ACTIVE;
            user_cfg_0.model.flag = USER_STATE_INACTIVE;
        }
    }
    if( partition == USER_PARTITION_UI_IMG )
    {
        if( idx == 0 )
        {
            user_cfg_0.ui_img.flag = USER_STATE_ACTIVE;
            user_cfg_1.ui_img.flag = USER_STATE_INACTIVE;
        }
        else if( idx == 1 )
        {
            user_cfg_1.ui_img.flag = USER_STATE_ACTIVE;
            user_cfg_0.ui_img.flag = USER_STATE_INACTIVE;
        }
    }
    ota_update_user_cfg();
    ota_user_debug_show();
}


int ota_user_get_active_area_partial( u8 partition )
{
    if( partition == USER_PARTITION_FW_INFO )
    {
        if( user_cfg_0.fw_info.flag == USER_STATE_ACTIVE )
        {
            return 0;
        }
        else if( user_cfg_1.fw_info.flag == USER_STATE_ACTIVE )
        {
            return 1;
        }
        else
        {
            return ( ( -1 ) *USER_PARTITION_FW_INFO );
        }
    }
    else if( partition == USER_PARTITION_MODEL )
    {
        if( user_cfg_0.model.flag == USER_STATE_ACTIVE )
        {
            return 0;
        }
        else if( user_cfg_1.model.flag == USER_STATE_ACTIVE )
        {
            return 1;
        }
        else
        {
            return ( ( -1 ) *USER_PARTITION_MODEL );
        }
    }
    else if( partition == USER_PARTITION_UI_IMG )
    {
        if( user_cfg_0.ui_img.flag == USER_STATE_ACTIVE )
        {
            return 0;
        }
        else if( user_cfg_1.ui_img.flag == USER_STATE_ACTIVE )
        {
            return 1;
        }
        else
        {
            return ( ( -1 ) *USER_PARTITION_UI_IMG );
        }
    }
    return -1;
}

int ota_user_check_on_going_area( u32 partition )
{
    //inactive to on-going
    if( partition == USER_PARTITION_FW_INFO ){

        if( user_cfg_0.fw_info.flag == USER_STATE_ON_GOING ){
            return 0;
        }
        else if( user_cfg_1.fw_info.flag == USER_STATE_ON_GOING ){
            return 1;
        }
        else{
            return -2;
        }
    }
    else if( partition == USER_PARTITION_MODEL ){

        if( user_cfg_0.model.flag == USER_STATE_ON_GOING ){
            return 0;
        }
        else if( user_cfg_1.model.flag == USER_STATE_ON_GOING ){
            return 1;
        }
        else{
            return -3;
        }
    }
    else if( partition == USER_PARTITION_UI_IMG ){

        if( user_cfg_0.ui_img.flag == USER_STATE_ON_GOING ){
            return 0;
        }
        else if( user_cfg_1.ui_img.flag == USER_STATE_ON_GOING ){
            return 1;
        }
        else{
            return -4;
        }
    }
    else{
        //error parameter input
        return -1;
    }
}



//from inactive to on-going state
int ota_user_select_inactive_area( u32 partition )
{
    int area = 0;
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_0,  KDP_FLASH_USER_CFG0_ADDR , sizeof( user_cfg_0) );
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_1,  KDP_FLASH_USER_CFG1_ADDR , sizeof( user_cfg_1) );

    //inactive to on-going
    if( partition == USER_PARTITION_FW_INFO ){
        if( user_cfg_0.fw_info.flag == USER_STATE_ACTIVE  && user_cfg_1.fw_info.flag != USER_STATE_ACTIVE ){
            user_cfg_1.fw_info.flag = USER_STATE_ON_GOING;
            area = 1;
        }
        else if( user_cfg_1.fw_info.flag == USER_STATE_ACTIVE && user_cfg_0.fw_info.flag != USER_STATE_ACTIVE ){
            user_cfg_0.fw_info.flag = USER_STATE_ON_GOING;
            area = 0;
        }
        else{
            return -2;
        }
    }
    else if( partition == USER_PARTITION_MODEL ){
        if( user_cfg_0.model.flag == USER_STATE_ACTIVE && user_cfg_1.model.flag != USER_STATE_ACTIVE ){
            user_cfg_1.model.flag = USER_STATE_ON_GOING;
            area = 1;
        }
        else if( user_cfg_1.model.flag == USER_STATE_ACTIVE && user_cfg_0.model.flag != USER_STATE_ACTIVE ){
            user_cfg_0.model.flag = USER_STATE_ON_GOING;
            area = 0;
        }
        else{
            return -3;
        }
    }
    else if( partition == USER_PARTITION_UI_IMG ){
        if( user_cfg_0.ui_img.flag == USER_STATE_ACTIVE && user_cfg_1.ui_img.flag != USER_STATE_ACTIVE){
            user_cfg_1.ui_img.flag = USER_STATE_ON_GOING;
            area = 1;
        }
        else if( user_cfg_1.ui_img.flag == USER_STATE_ACTIVE && user_cfg_0.ui_img.flag != USER_STATE_ACTIVE ){
            user_cfg_0.ui_img.flag = USER_STATE_ON_GOING;
            area = 0;
        }
        else{
            return -4;
        }
    }
    else{
        //error parameter input
        return -1;
    }

    ota_update_user_cfg();
    ota_user_debug_show();

    return area;
}

int ota_user_select_wait_active_area( u32 partition )
{
    int ret ;
    //inactive to on-going
    if( partition == USER_PARTITION_FW_INFO ){

        if( user_cfg_0.fw_info.flag == USER_STATE_ON_GOING  /*&& user_cfg_1.fw_info.flag == USER_STATE_ACTIVE*/ )
        {
            user_cfg_0.fw_info.flag = USER_STATE_WAIT_ACTIVE;
            ret = 0;
        }
        else if( user_cfg_1.fw_info.flag == USER_STATE_ON_GOING /* && user_cfg_0.fw_info.flag == USER_STATE_ACTIVE*/ )
        {
            user_cfg_1.fw_info.flag = USER_STATE_WAIT_ACTIVE;
            ret = 1;
        }
        else
        {
            return -2;
        }

    }
    else if( partition == USER_PARTITION_MODEL ){
        if( user_cfg_0.model.flag == USER_STATE_ON_GOING  /*&& user_cfg_1.model.flag == USER_STATE_ACTIVE*/ )
        {
            user_cfg_0.model.flag = USER_STATE_WAIT_ACTIVE;
            ret = 0;
        }
        else if( user_cfg_1.model.flag == USER_STATE_ON_GOING /* && user_cfg_0.model.flag == USER_STATE_ACTIVE*/ )
        {
            user_cfg_1.model.flag = USER_STATE_WAIT_ACTIVE;
            ret = 1;
        }
        else
        {
            return -3;
        }
    }
    else if( partition == USER_PARTITION_UI_IMG ){

        if( user_cfg_0.ui_img.flag == USER_STATE_ON_GOING  /*&& user_cfg_1.ui_img.flag == USER_STATE_ACTIVE*/ )
        {
            user_cfg_0.ui_img.flag = USER_STATE_WAIT_ACTIVE;
            ret = 0;
        }
        else if( user_cfg_1.ui_img.flag == USER_STATE_ON_GOING  /*&& user_cfg_0.ui_img.flag == USER_STATE_ACTIVE*/ )
        {
            user_cfg_1.ui_img.flag = USER_STATE_WAIT_ACTIVE;
            ret = 1;
        }
        else
        {
            return -4;
        }
    }
    else{
        return -1;
    }

    ota_update_user_cfg();
    ota_user_debug_show();

    return ret;
}






#if ( OTA_USER_BACKUP_SEPERATE == YES )


int ota_user_get_active_area( uint8_t idx )
{
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_0,  KDP_FLASH_USER_CFG0_ADDR , sizeof( user_cfg_0) );
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_1,  KDP_FLASH_USER_CFG1_ADDR , sizeof( user_cfg_1) );

    if( idx == USER_PARTITION_FW_INFO )
    {
        //check model and fw_info simultaneously

        if( user_cfg_0.fw_info.flag == USER_STATE_ACTIVE
            && user_cfg_0.model.flag == USER_STATE_ACTIVE )
        {

            return 0;
        }
        else if( user_cfg_1.fw_info.flag ==  USER_STATE_ACTIVE
            && user_cfg_1.model.flag == USER_STATE_ACTIVE )
        {
            return 1;
        }
        else
        {
            return -3;
        }
    }
    else if( idx == USER_PARTITION_MODEL )
    {
        //no need check model, because model and fw_info are checked together
        return -2;
    }
    else if( idx == USER_PARTITION_UI_IMG )
    {
        if( user_cfg_0.ui_img.flag == USER_STATE_ACTIVE ){
            return 0;
        }
        else if( user_cfg_1.ui_img.flag == USER_STATE_ACTIVE ){
            return 1;
        }
        else
        {
            return -4;
        }
    }
    else
    {
        return -1;
    }
}

int ota_user_area_boot_check( void )
{
    //select which area
    //step1: find current active area 0 or 1
    //step2: find another area which are all in wait active then switch area
    //       if not all area are all in the wait active then do not switch area
    int act_area ;
    int ret = 0;

    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_0,  KDP_FLASH_USER_CFG0_ADDR , sizeof( user_cfg_0) );
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_1,  KDP_FLASH_USER_CFG1_ADDR , sizeof( user_cfg_1) );


    /*************************
        UI image
    *************************/
    act_area = ota_user_get_active_area(USER_PARTITION_UI_IMG);
    #if(OTA_BOOT_CONFIG_LOG==YES)
    dbg_msg_flash("UI 1 boot switch :%d", act_area );
    #endif
    if( act_area == 0 )
    {
        #if(OTA_BOOT_CONFIG_LOG==YES)
        dbg_msg_flash("user 0 UI flag :%d", user_cfg_0.ui_img.flag );
        dbg_msg_flash("user 1 UI flag :%d", user_cfg_1.ui_img.flag );
        #endif
        if ( user_cfg_0.ui_img.flag == USER_STATE_ACTIVE
        && user_cfg_1.ui_img.flag == USER_STATE_INACTIVE )
        {
            #if(OTA_BOOT_CONFIG_LOG==YES)
            dbg_msg_flash("UI 0 boot use org" );
            #endif
            ret = 0;
            guser_area = 0;
            user_set_ui_offset( guser_area*USR_FLASH_SETTINGS_OFFSET_1  );
        }
        else if ( user_cfg_1.ui_img.flag == USER_STATE_WAIT_ACTIVE )
        {
            dbg_msg_console("UI 0 switch doing");
            user_cfg_1.ui_img.flag = USER_STATE_ACTIVE;
            user_cfg_0.ui_img.flag = USER_STATE_INACTIVE;
            ret = 1;
            guser_area = 1;
            ota_update_user_cfg();
            user_set_ui_offset( guser_area*USR_FLASH_SETTINGS_OFFSET_1  );
        }
        else
        {
            dbg_msg_console("UI 0 check fail and no switch");
            user_cfg_1.fw_info.flag = USER_STATE_INACTIVE;
            user_cfg_1.model.flag = USER_STATE_INACTIVE;
            user_cfg_1.ui_img.flag = USER_STATE_INACTIVE;
            ret = 0;
            guser_area = 0;
            ota_update_user_cfg();
            user_set_ui_offset( guser_area*USR_FLASH_SETTINGS_OFFSET_1  );
        }
    }
    else if( act_area == 1 )
    {
        if ( user_cfg_1.ui_img.flag == USER_STATE_ACTIVE
        && user_cfg_0.ui_img.flag == USER_STATE_INACTIVE )
        {
            #if(OTA_BOOT_CONFIG_LOG==YES)
            dbg_msg_flash("UI 1 boot use org" );
            #endif
            guser_area = 1;
            ret = 1;
            user_set_ui_offset( guser_area*USR_FLASH_SETTINGS_OFFSET_1  );
        }
        else if ( user_cfg_0.ui_img.flag == USER_STATE_WAIT_ACTIVE )
        {
            dbg_msg_console("UI 1 switch doing");
            user_cfg_0.ui_img.flag = USER_STATE_ACTIVE;
            user_cfg_1.ui_img.flag = USER_STATE_INACTIVE;
            ret = 0;
            guser_area = 0;
            ota_update_user_cfg();
            user_set_ui_offset( guser_area*USR_FLASH_SETTINGS_OFFSET_1  );
        }
        else
        {
            dbg_msg_console("UI 1 check fail and no switch");
            user_cfg_0.ui_img.flag = USER_STATE_INACTIVE;
            ret = 1;
            guser_area = 1;
            ota_update_user_cfg();
            user_set_ui_offset( guser_area*USR_FLASH_SETTINGS_OFFSET_1  );
        }
    }
    else
    {
        return -1;
    }

    /*************************
        fw_info and model
    *************************/
    act_area = ota_user_get_active_area(USER_PARTITION_FW_INFO);
    #if(OTA_BOOT_CONFIG_LOG==YES)
    dbg_msg_flash("fw+model boot switch :%d", act_area );
    #endif
    if( act_area == 0 )
    {
      if ( user_cfg_0.fw_info.flag == USER_STATE_ACTIVE
      && user_cfg_1.fw_info.flag == USER_STATE_INACTIVE
      && user_cfg_0.model.flag == USER_STATE_ACTIVE
      && user_cfg_1.model.flag == USER_STATE_INACTIVE
      )
      {
        #if(OTA_BOOT_CONFIG_LOG==YES)
        dbg_msg_flash("fw + model 0 boot use org" );
        #endif
        ret = 0;
        guser_area = 0;
        kdp_set_model_offset( guser_area*KDP_FLASH_ALL_MODEL_OFFSET_1  );
        kdp_set_fwinfo_offset( guser_area*KDP_FLASH_FW_INFO_OFFSET_1  );
      }
      else if ( user_cfg_1.model.flag == USER_STATE_WAIT_ACTIVE
             && user_cfg_1.fw_info.flag == USER_STATE_WAIT_ACTIVE )
      {
          dbg_msg_console("fw + model 0 switch doing");
          user_cfg_1.model.flag = USER_STATE_ACTIVE;
          user_cfg_0.model.flag = USER_STATE_INACTIVE;
          user_cfg_1.fw_info.flag = USER_STATE_ACTIVE;
          user_cfg_0.fw_info.flag = USER_STATE_INACTIVE;
          ret = 1;
          guser_area = 1;
          ota_update_user_cfg();
          kdp_set_model_offset( guser_area*KDP_FLASH_ALL_MODEL_OFFSET_1  );
          kdp_set_fwinfo_offset( guser_area*KDP_FLASH_FW_INFO_OFFSET_1  );
      }
      else
      {
          dbg_msg_console("fw + model 0 check fail and no switch");
          user_cfg_1.fw_info.flag = USER_STATE_INACTIVE;
          user_cfg_1.model.flag = USER_STATE_INACTIVE;
          ret = 0;
          guser_area = 0;
          ota_update_user_cfg();
          kdp_set_model_offset( guser_area*KDP_FLASH_ALL_MODEL_OFFSET_1  );
          kdp_set_fwinfo_offset( guser_area*KDP_FLASH_FW_INFO_OFFSET_1  );
      }
    }
    else if( act_area == 1 )
    {
      if ( user_cfg_1.fw_info.flag == USER_STATE_ACTIVE
      && user_cfg_0.fw_info.flag == USER_STATE_INACTIVE
      && user_cfg_1.model.flag == USER_STATE_ACTIVE
      && user_cfg_0.model.flag == USER_STATE_INACTIVE )
      {
        #if(OTA_BOOT_CONFIG_LOG==YES)
        dbg_msg_flash("fw + model 1 boot use org" );
        #endif
        ret = 1;
        guser_area = 1;
        kdp_set_model_offset( guser_area*KDP_FLASH_ALL_MODEL_OFFSET_1  );
        kdp_set_fwinfo_offset( guser_area*KDP_FLASH_FW_INFO_OFFSET_1  );
      }
      else if( user_cfg_0.model.flag == USER_STATE_WAIT_ACTIVE
                && user_cfg_0.fw_info.flag == USER_STATE_WAIT_ACTIVE )
      {
          dbg_msg_console("fw + model 1 switch doing");
          user_cfg_0.model.flag = USER_STATE_ACTIVE;
          user_cfg_1.model.flag = USER_STATE_INACTIVE;
          user_cfg_0.fw_info.flag = USER_STATE_ACTIVE;
          user_cfg_1.fw_info.flag = USER_STATE_INACTIVE;
          ret = 0;
          guser_area = 0;
          ota_update_user_cfg();
          kdp_set_model_offset( guser_area*KDP_FLASH_ALL_MODEL_OFFSET_1  );
          kdp_set_fwinfo_offset( guser_area*KDP_FLASH_FW_INFO_OFFSET_1  );
      }
      else
      {
          dbg_msg_console("fw + model 1 check fail and no switch");

          user_cfg_0.fw_info.flag = USER_STATE_INACTIVE;
          user_cfg_0.model.flag = USER_STATE_INACTIVE;
          ret = 1;
          guser_area = 1;
          ota_update_user_cfg();
          kdp_set_model_offset( guser_area*KDP_FLASH_ALL_MODEL_OFFSET_1  );
          kdp_set_fwinfo_offset( guser_area*KDP_FLASH_FW_INFO_OFFSET_1  );
      }
    }
    else
    {
      return -1;
    }

//    ota_user_debug_show();

    return ret;
}

#else

int ota_user_get_active_area( void )
{
    int ret;

    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_0,  KDP_FLASH_USER_CFG0_ADDR , sizeof( user_cfg_0) );
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_1,  KDP_FLASH_USER_CFG1_ADDR , sizeof( user_cfg_1) );

    if ( user_cfg_0.fw_info.flag == USER_STATE_ACTIVE
        && user_cfg_0.model.flag == USER_STATE_ACTIVE
        && user_cfg_0.ui_img.flag == USER_STATE_ACTIVE)
    {
        ret = 0;
    }
    else if ( user_cfg_1.fw_info.flag == USER_STATE_ACTIVE
        && user_cfg_1.model.flag == USER_STATE_ACTIVE
        && user_cfg_1.ui_img.flag == USER_STATE_ACTIVE)
    {
        ret = 1;
    }
    else
    {
        return -1;
    }
    return ret;
}

int ota_user_area_boot_check( void )
{
    //select which area
    //step1: find current active area 0 or 1
    //step2: find another area which are all in wait active then switch area
    //       if not all area are all in the wait active then do not switch area
    int act_area ;
    int ret = 0;

    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_0,  KDP_FLASH_USER_CFG0_ADDR , sizeof( user_cfg_0) );
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_1,  KDP_FLASH_USER_CFG1_ADDR , sizeof( user_cfg_1) );

    act_area = ota_user_get_active_area();

    dbg_msg_console("area:%d", act_area);
    if( act_area == 0 )
    {
        if ( user_cfg_0.fw_info.flag == USER_STATE_ACTIVE
            && user_cfg_0.model.flag == USER_STATE_ACTIVE
            && user_cfg_0.ui_img.flag == USER_STATE_ACTIVE
            && user_cfg_1.fw_info.flag == USER_STATE_INACTIVE
            && user_cfg_1.model.flag == USER_STATE_INACTIVE
            && user_cfg_1.ui_img.flag == USER_STATE_INACTIVE )
        {
            ret = 0;
            guser_area = 0;
            goto check_final;
        }
        else if ( user_cfg_1.fw_info.flag == USER_STATE_WAIT_ACTIVE
          && user_cfg_1.model.flag == USER_STATE_WAIT_ACTIVE
          && user_cfg_1.ui_img.flag == USER_STATE_WAIT_ACTIVE )
        {
            dbg_msg_console("switch doing");
            user_cfg_1.fw_info.flag = USER_STATE_ACTIVE;
            user_cfg_1.model.flag = USER_STATE_ACTIVE;
            user_cfg_1.ui_img.flag = USER_STATE_ACTIVE;
            user_cfg_0.fw_info.flag = USER_STATE_INACTIVE;
            user_cfg_0.model.flag = USER_STATE_INACTIVE;
            user_cfg_0.ui_img.flag = USER_STATE_INACTIVE;
            ret = 1;
            guser_area = 1;
        }
        else
        {
            dbg_msg_console("check fail and no switch");
            user_cfg_1.fw_info.flag = USER_STATE_INACTIVE;
            user_cfg_1.model.flag = USER_STATE_INACTIVE;
            user_cfg_1.ui_img.flag = USER_STATE_INACTIVE;
            ret = 0;
            guser_area = 0;
        }
    }
    else if( act_area == 1 )
    {
        if ( user_cfg_1.fw_info.flag == USER_STATE_ACTIVE
            && user_cfg_1.model.flag == USER_STATE_ACTIVE
            && user_cfg_1.ui_img.flag == USER_STATE_ACTIVE
            && user_cfg_0.fw_info.flag == USER_STATE_INACTIVE
            && user_cfg_0.model.flag == USER_STATE_INACTIVE
            && user_cfg_0.ui_img.flag == USER_STATE_INACTIVE )
        {
            guser_area = 1;
            ret = 1;
            goto check_final;
//            {
//                user_cfg_0.fw_info.flag = USER_STATE_INACTIVE;
//                user_cfg_0.model.flag = USER_STATE_INACTIVE;
//                user_cfg_0.ui_img.flag = USER_STATE_INACTIVE;
//            }
        }
        else if ( user_cfg_0.fw_info.flag == USER_STATE_WAIT_ACTIVE
          && user_cfg_0.model.flag == USER_STATE_WAIT_ACTIVE
          && user_cfg_0.ui_img.flag == USER_STATE_WAIT_ACTIVE )
        {
            dbg_msg_console("switch doing");
            user_cfg_0.fw_info.flag = USER_STATE_ACTIVE;
            user_cfg_0.model.flag = USER_STATE_ACTIVE;
            user_cfg_0.ui_img.flag = USER_STATE_ACTIVE;
            user_cfg_1.fw_info.flag = USER_STATE_INACTIVE;
            user_cfg_1.model.flag = USER_STATE_INACTIVE;
            user_cfg_1.ui_img.flag = USER_STATE_INACTIVE;
            ret = 0;
            guser_area = 0;
        }
        else
        {
            dbg_msg_console("check fail and no switch");
            user_cfg_0.fw_info.flag = USER_STATE_INACTIVE;
            user_cfg_0.model.flag = USER_STATE_INACTIVE;
            user_cfg_0.ui_img.flag = USER_STATE_INACTIVE;
            ret = 1;
            guser_area = 1;
        }
    }
    else
    {
        return -1;
    }

    kdp_memxfer_flash_sector_multi_erase( KDP_FLASH_USER_CFG0_ADDR, KDP_FLASH_USER_CFG0_ADDR );
    kdp_memxfer_flash_sector_multi_erase( KDP_FLASH_USER_CFG1_ADDR, KDP_FLASH_USER_CFG1_ADDR );
    kdp_memxfer_ddr_to_flash( (UINT32)KDP_FLASH_USER_CFG0_ADDR, (UINT32)&user_cfg_0, sizeof(user_cfg_0));
    kdp_memxfer_ddr_to_flash( (UINT32)KDP_FLASH_USER_CFG1_ADDR, (UINT32)&user_cfg_1, sizeof(user_cfg_1));
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_0,  KDP_FLASH_USER_CFG0_ADDR , sizeof( user_cfg_0) );
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_1,  KDP_FLASH_USER_CFG1_ADDR , sizeof( user_cfg_1) );
//
//    dbg_msg_console("USR_FLASH_SETTINGS_OFFSET_1 0x:%x", guser_area*USR_FLASH_SETTINGS_OFFSET_1);
//    dbg_msg_console("KDP_FLASH_ALL_MODEL_OFFSET_1 0x:%x", guser_area*KDP_FLASH_ALL_MODEL_OFFSET_1);
//    dbg_msg_console("KDP_FLASH_FW_INFO_OFFSET_1 0x:%x", guser_area*KDP_FLASH_FW_INFO_OFFSET_1);
check_final:
    user_set_ui_offset( guser_area*USR_FLASH_SETTINGS_OFFSET_1  );
    kdp_set_model_offset( guser_area*KDP_FLASH_ALL_MODEL_OFFSET_1  );
    kdp_set_fwinfo_offset( guser_area*KDP_FLASH_FW_INFO_OFFSET_1  );

    ota_user_debug_show();

    return ret;
}



#endif










//user_cfg_1.fw_info.flag
int ota_get_wait_active_area(u32 partition )
{

    if( partition == USER_PARTITION_UI_IMG )
    {
        if( user_cfg_1.ui_img.flag == USER_STATE_WAIT_ACTIVE )
        {
            return 1;
        }
        else if( user_cfg_0.ui_img.flag == USER_STATE_WAIT_ACTIVE )
        {
            return 0;
        }
    }
    if( partition == USER_PARTITION_FW_INFO )
    {
        if( user_cfg_1.fw_info.flag == USER_STATE_WAIT_ACTIVE )
        {
            return 1;
        }
        else if( user_cfg_0.fw_info.flag == USER_STATE_WAIT_ACTIVE )
        {
            return 0;
        }
    }
    if( partition == USER_PARTITION_MODEL )
    {
        if( user_cfg_1.model.flag == USER_STATE_WAIT_ACTIVE )
        {
            return 1;
        }
        else if( user_cfg_0.model.flag == USER_STATE_WAIT_ACTIVE )
        {
            return 0;
        }
    }
    return -1;
}

u32 ota_user_current_area(void)
{
    return guser_area;
}


void ota_user_dummy_changes_status(void)
{

    user_cfg_1.fw_info.flag = USER_STATE_INACTIVE;


    kdp_memxfer_flash_sector_multi_erase( KDP_FLASH_USER_CFG0_ADDR, KDP_FLASH_USER_CFG0_ADDR );
    kdp_memxfer_flash_sector_multi_erase( KDP_FLASH_USER_CFG1_ADDR, KDP_FLASH_USER_CFG1_ADDR );
    kdp_memxfer_ddr_to_flash( (UINT32)KDP_FLASH_USER_CFG0_ADDR, (UINT32)&user_cfg_0, sizeof(user_cfg_0));
    kdp_memxfer_ddr_to_flash( (UINT32)KDP_FLASH_USER_CFG1_ADDR, (UINT32)&user_cfg_1, sizeof(user_cfg_1));
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_0,  KDP_FLASH_USER_CFG0_ADDR , sizeof( user_cfg_0) );
    kdp_memxfer_flash_to_ddr((UINT32)&user_cfg_1,  KDP_FLASH_USER_CFG1_ADDR , sizeof( user_cfg_1) );

    ota_user_debug_show();

}
#endif

//---------------
//---CRC check---
//---------------
const u32 crc32_tab[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};


u32 ota_crc32( u8 *buf, size_t size)
{
    const u8 *p = buf;
    u32 crc;
    crc = ~0U;
    while (size--){
        crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ ~0U;
}


UINT32 ota_get_model_crc(void)
{
    UINT32  crc;
    kdp_memxfer_flash_to_ddr( (UINT32)&crc, KDP_FLASH_FW_INFO_SIZE , 4 );
    return 0x00;
}
#endif
#endif

