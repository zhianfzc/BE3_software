#include "ota.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if 1//( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
#include "flash.h"
#include "os_tick.h"
#include "RTX_Config.h"
#include "pinmux.h"
#include "framework/framework_driver.h"
#include "kdp520_gpio.h"
#include "kdp520_spi.h"
#include "kdp520_ssp.h"
#include "kdp_ddr_table.h"
#include "kdp_memxfer.h"
#include "ota_update.h"
#include "kl520_include.h"
#include "kl520_api.h"
#include "kl520_api_device_id.h"
#include "kl520_api_snapshot.h"
#include "kl520_api_camera.h"
#include "kl520_sys.h"
#include "user_ui.h"
#include "flash.h"
#include "kdp_model.h"

#define NIR_RGB_OTA_EN                  (1)
#define PRINT_FLASH_MECHINE_DEBUG_EN    (NO)

UINT8	*gu8OTA_Rx_buffer = (UINT8 *)KDP_DDR_DRV_COM_BUS_RX0_START_ADDR;
UINT32	gu8OTA_Rx_buffer_index = 0;
UINT8	*gu8OTA_Tx_buffer = (UINT8 *)KDP_DDR_DRV_COM_BUS_TX_START_ADDR;
UINT32	gu8OTA_Tx_buffer_index = 0;

osThreadId_t    ota_tid;
osThreadId_t    com_bus_tid;

OTA_FLASHt stOTA;
extern void delay_ms(unsigned int msec);
extern ota_boot_cfg_t boot_cfg_0, boot_cfg_1;
extern FLASH_PARMATER_T st_flash_info;

extern struct   st_com_type stCom_type;

void Drv_Uart2_Tx_arrange( void * data_ptr , UINT16 len);

#if ( OTA_TIMING_DEBUG_EN == YES)
void _flash_gpio_debug(void)
{
    UINT32	data;
    //pad mux
    data = inw(SCU_EXTREG_PA_BASE + 0x184);
    data &= 0xFFFFFFF8;		//clear low 3bit
    outw(SCU_EXTREG_PA_BASE + 0x184, data | 0x3);

    data = inw(SCU_EXTREG_PA_BASE + 0x188);
    data &= 0xFFFFFFF8;		//clear low 3bit
    outw(SCU_EXTREG_PA_BASE + 0x188, data | 0x3);

    data = inw(SCU_EXTREG_PA_BASE + 0x18C);
    data &= 0xFFFFFFF8;		//clear low 3bit
    outw(SCU_EXTREG_PA_BASE + 0x18C, data | 0x3);

    data = inw(SCU_EXTREG_PA_BASE + 0x190);
    data &= 0xFFFFFFF8;		//clear low 3bit
    outw(SCU_EXTREG_PA_BASE + 0x190, data | 0x3);

    kdp520_gpio_setdir(  24, GPIO_DIR_OUTPUT );
    kdp520_gpio_setdir(  25, GPIO_DIR_OUTPUT );
    kdp520_gpio_setdir(  26, GPIO_DIR_OUTPUT );
    kdp520_gpio_setdir(  27, GPIO_DIR_OUTPUT );

    //GPO set
//	kdp520_gpio_setdata( 1<<25);
//	kdp520_gpio_setdata( 1<<24);
//	kdp520_gpio_setdata( 1<<26);
//	kdp520_gpio_setdata( 1<<27);

    //GPO clear
    kdp520_gpio_cleardata( 1<<24);
    kdp520_gpio_cleardata( 1<<25);
    kdp520_gpio_cleardata( 1<<26);
    kdp520_gpio_cleardata( 1<<27);


//
//	data = inw(SCU_EXTREG_PA_BASE + 0x158);
//	data &= 0xFFFFFFF8;		//clear low 3bit
//	outw(SCU_EXTREG_PA_BASE + 0x158, data | 0x3);
//
//	kdp520_gpio_setdir(  18, GPIO_DIR_OUTPUT );
//	while(1)
//	{
//		kdp520_gpio_setdata( 1<<18);
//		kdp520_gpio_cleardata( 1<<18);
//	}



}
#endif

void Drv_flash_erase_area( UINT32 start_address_index )
{
    if(  boot_cfg_0.scpu_cfg.flag  == BOOT_STATE_FIRST_BOOT
    &&
    boot_cfg_1.scpu_cfg.flag  == BOOT_STATE_FIRST_BOOT )
    {
        #if( OTA_LOG_EN == YES )
        dbg_msg_flash("[OTA cfg] SCPU area erase fail");
        #endif
        return ;
    }

    if(  boot_cfg_0.ncpu_cfg.flag  == BOOT_STATE_FIRST_BOOT
        &&
        boot_cfg_1.ncpu_cfg.flag  == BOOT_STATE_FIRST_BOOT )
    {
        #if( OTA_LOG_EN == YES )
        dbg_msg_flash("[OTA cfg] NCPU area erase fail");
        #endif
        return ;
    }

    if(  boot_cfg_0.scpu_cfg.flag  == BOOT_STATE_FIRST_BOOT && start_address_index==SCPU0_START_SECTOR_INDEX )
    {
        kdp_memxfer_flash_sector_multi_erase( SCPU0_START_SECTOR_INDEX*4096, SCPU0_END_SECTOR_INDEX*4096  );
    }
    else if(  boot_cfg_1.scpu_cfg.flag  == BOOT_STATE_FIRST_BOOT && start_address_index==SCPU1_START_SECTOR_INDEX )
    {
        #if( OTA_LOG_EN == YES )
        dbg_msg_flash("[OTA cfg] NCPU area erase start sec: %d. end sec: %d", SCPU1_START_SECTOR_INDEX , SCPU1_END_SECTOR_INDEX );
        #endif
        kdp_memxfer_flash_sector_multi_erase( SCPU1_START_SECTOR_INDEX*4096, SCPU1_END_SECTOR_INDEX*4096  );
    }

    if( boot_cfg_0.ncpu_cfg.flag == BOOT_STATE_FIRST_BOOT && start_address_index==NCPU0_START_SECTOR_INDEX )
    {
        kdp_memxfer_flash_sector_multi_erase( NCPU0_START_SECTOR_INDEX*4096, NCPU0_END_SECTOR_INDEX*4096 );
    }
    else if( boot_cfg_1.ncpu_cfg.flag == BOOT_STATE_FIRST_BOOT && start_address_index==NCPU1_START_SECTOR_INDEX  )
    {
        kdp_memxfer_flash_sector_multi_erase( NCPU1_START_SECTOR_INDEX*4096, NCPU1_END_SECTOR_INDEX*4096 );
    }
}

UINT16 Drv_flash_action_doing( OTA_FLASHt * stota )
{
    //#define BOOT_STATE_CONFIRMED        0x1
    //#define BOOT_STATE_FIRST_BOOT       0x2
    //#define BOOT_STATE_POST_FIRST_BOOT  0x4
    //#define BOOT_STATE_NOT_CONFIRMED    0x8
    UINT16 ret = FLASH_DRV_OK;
    #if( OTA_USER_BACKUP == YES )
    int area = 0;
    #endif

    if( stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_PROC_DONE )
    {
        if( stota->bin_type == eOTA_bin_scpu ){
            //scpu
            // if(  boot_cfg_0.scpu_cfg.flag == BOOT_STATE_FIRST_BOOT )
            // {
            //     ota_burn_in_config( 1 );
            //     #if( OTA_LOG_EN == YES )
            //     dbg_msg_flash("[OTA cfg] config scpu burn in area0");
            //     #endif
            // }
            // else if(  boot_cfg_1.scpu_cfg.flag == BOOT_STATE_FIRST_BOOT )
            // {
            //     ota_burn_in_config( 2 );
            //     #if( OTA_LOG_EN == YES )
            //     dbg_msg_flash("[OTA cfg] config scpu burn in area1");
            //     #endif
            // }
        }
        else if(  stota->bin_type == eOTA_bin_ncpu )
        {
            //ncpu
            // if(  boot_cfg_0.ncpu_cfg.flag == BOOT_STATE_FIRST_BOOT )
            // {
            //     ota_burn_in_config( 1 );
            //     #if( OTA_LOG_EN == YES )
            //     dbg_msg_flash("[OTA cfg] config ncpu burn in area1");
            //     #endif
            // }
            // else if(  boot_cfg_1.ncpu_cfg.flag == BOOT_STATE_FIRST_BOOT )
            // {
            //     ota_burn_in_config( 2 );
            //     #if( OTA_LOG_EN == YES )
            //     dbg_msg_flash("[OTA cfg] config ncpu burn in area1");
            //     #endif
            // }
        }
        else if( stota->bin_type == eOTA_bin_fw_info )
        {
            //OK do nothing
            #if( OTA_USER_BACKUP == YES)
            area = ota_user_select_wait_active_area(USER_PARTITION_FW_INFO);
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA cfg] fw_info wait-act area:%d", area );
            #endif
            #endif
        }
        else if( stota->bin_type == eOTA_bin_all_model )
        {
            //OK do nothing
            #if( OTA_USER_BACKUP == YES)
            area = ota_user_select_wait_active_area(USER_PARTITION_MODEL);
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA cfg] all model wait-act area:%d", area );
            #endif
            #endif
        }
        else if( stota->bin_type == eOTA_bin_ui_image )
        {
            //OK do nothing
            #if( OTA_USER_BACKUP == YES)
            area = ota_user_select_wait_active_area(USER_PARTITION_UI_IMG);
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA cfg] UI wait-act area:%d", area );
            #endif
            #endif
        }
        else if ( stota->bin_type == eOTA_bin_partial_model )
        {
            #if( OTA_USER_BACKUP == YES)
            area = ota_user_select_wait_active_area(USER_PARTITION_MODEL);
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA cfg] partial model wait-act area:%d", area );
            #endif
            #endif
        }
        else
        {
            ret = FLASH_STATUS_PROGRAM_FAIL;
        }
        //write boot config
        stota->target_sectors = 0;
        stota->sector_offset = 0;
        stota->sector_acc = 0xFFFF;
        stota->bin_type = eOTA_bin_null;
    }
    else if( stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_SEL_SCPU
    || stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_SEL_NCPU
    || stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_SEL_MODEL
    || stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_SEL_IMAGE_UPDATE
    || stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_SEL_FW_INFO
    || stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_SEL_MODEL_PARTIAL)
    {
        if( stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_SEL_SCPU )
        {
            ota_update_scpu_flag_status(  );	//change which one is not in the confirmed state then assigned to first_boot
            if(boot_cfg_1.scpu_cfg.flag == BOOT_STATE_FIRST_BOOT  )
            {
                #if( OTA_LOG_EN == YES )
                dbg_msg_flash("[OTA cfg] config scpu1");
                #endif
                stota->sector_offset = SCPU1_START_SECTOR_INDEX;	//select scpu 1
            }
            else
            {
                #if( OTA_LOG_EN == YES )
                dbg_msg_flash("[OTA cfg] config scpu0");
                #endif
                stota->sector_offset = SCPU0_START_SECTOR_INDEX;	//select scpu 0
            }
            stota->target_sectors = (SCPU_SIZE>>12);
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA cfg] config scpu sector size: %d", stota->target_sectors );
            #endif
            stota->sector_acc = 0;
            stota->bin_type = eOTA_bin_scpu;//scpu bin
            stota->target_bytes = SCPU_IMAGE_SIZE;		//need to update with new code
//            stota->ddr_ptr = (UINT32 *) KDP_DDR_OTA_IMAGE_BUF_START_ADDR;		//for bin file
        }
        else if( stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_SEL_NCPU )
        {
            ota_update_ncpu_flag_status();
            if(boot_cfg_1.ncpu_cfg.flag == BOOT_STATE_FIRST_BOOT  )
            {
                #if( OTA_LOG_EN == YES )
                dbg_msg_flash("[OTA cfg] config ncpu1");
                #endif
                stota->sector_offset = NCPU1_START_SECTOR_INDEX;	//select ncpu 1
            }
            else
            {
                #if( OTA_LOG_EN == YES )
                dbg_msg_flash("[OTA cfg] config ncpu0");
                #endif
                stota->sector_offset = NCPU0_START_SECTOR_INDEX;	//select ncpu 0
            }
            stota->target_sectors = ( NCPU_SIZE >> 12 );
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA cfg] config ncpu sector size: %d", stota->target_sectors );
            #endif

            stota->sector_acc = 0;
            stota->bin_type = eOTA_bin_ncpu;//ncpu bin
            stota->target_bytes = NCPU_IMAGE_SIZE;		//need to update with new code
//            stota->ddr_ptr = (UINT32 *) KDP_DDR_OTA_IMAGE_BUF_START_ADDR;		//for bin file
        }
        else if( stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_SEL_MODEL )
        {
            #if( OTA_USER_BACKUP == YES )

            #if( OTA_USER_BACKUP_SEPERATE == YES )
            area = ota_user_check_on_going_area(USER_PARTITION_MODEL);
            if( area < 0 )
            {
                return FLASH_STATUS_MODEL_AREA_FAIL;
            }
            #else
            area = ota_user_select_inactive_area(USER_PARTITION_MODEL);
            #endif

            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA cfg] all model area:%d", area );
            #endif
            if( area == 1 ){
                stota->sector_offset = MODEL_START_SECTOR_INDEX_1;    //model
            }
            else{
                stota->sector_offset = MODEL_START_SECTOR_INDEX;    //model
            }

            #else
            stota->sector_offset = MODEL_START_SECTOR_INDEX;    //model
            #endif

            stota->sector_acc = 0;
            stota->bin_type = eOTA_bin_all_model;//model bin
            //remove        stota->target_sectors = ( MODEL_IMAGE_SIZE >> 12 );
            //remove        stota->target_bytes = MODEL_IMAGE_SIZE;
            stota->ddr_ptr = (UINT32 *)KDP_DDR_MODEL_START_ADDR;		//for bin file
        }
        else if( stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_SEL_IMAGE_UPDATE )
        {
            #if( OTA_USER_BACKUP == YES )
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA cfg] UI area:%d", area );
            #endif
            area = ota_user_select_inactive_area(USER_PARTITION_UI_IMG);
            if( area == 1 ){
                stota->sector_offset = USR_IMAGE_START_SECTOR_INDEX_1;    //model
            }
            else{
                stota->sector_offset = USR_IMAGE_START_SECTOR_INDEX;    //model
            }

            #else
            stota->sector_offset = USR_IMAGE_START_SECTOR_INDEX;    //model
            #endif
            stota->sector_acc = 0;
            stota->bin_type = eOTA_bin_ui_image;
            //stota->target_sectors = ( USR_FLASH_LAST_ADDR - USR_FLASH_SETTINGS_ADDR )>>12;
            //stota->target_bytes = ( USR_FLASH_LAST_ADDR - USR_FLASH_SETTINGS_ADDR );
            stota->ddr_ptr = (UINT32 *) KDP_DDR_MODEL_START_ADDR;   //for bin file
        }
        else if( stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_SEL_FW_INFO )
        {
            #if( OTA_USER_BACKUP == YES )
            area = ota_user_select_inactive_area(USER_PARTITION_FW_INFO);

            #if( OTA_USER_BACKUP_SEPERATE == YES )
            area = ota_user_select_inactive_area(USER_PARTITION_MODEL);
            #endif

            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA cfg] fw info area:%d", area );
            #endif
            if( area == 1 ){
                stota->sector_offset = FW_INFO_START_SECTOR_INDEX_1;
            }
            else{
                stota->sector_offset = FW_INFO_START_SECTOR_INDEX;
            }
            #else
            stota->sector_offset = FW_INFO_START_SECTOR_INDEX;
            #endif
            stota->sector_acc = 0;
            stota->bin_type = eOTA_bin_fw_info;
//            stota->target_sectors = ( KDP_FLASH_FW_INFO_SIZE >> 12 );
//            stota->target_bytes = KDP_FLASH_FW_INFO_SIZE;
            stota->ddr_ptr = (UINT32 *) KDP_DDR_OTA_IMAGE_BUF_START_ADDR;   //for bin file
        }
        else if( stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_SEL_MODEL_PARTIAL )
        {
            #if( OTA_USER_BACKUP == YES )

            #if( OTA_USER_BACKUP_SEPERATE == YES )
            area = ota_user_check_on_going_area(USER_PARTITION_MODEL);
            if( area < 0 )
            {
                return FLASH_STATUS_MODEL_AREA_FAIL;
            }
            #else
            area = ota_user_select_inactive_area(USER_PARTITION_MODEL);
            #endif

            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA cfg] partial model area:%d", area );
            #endif
            if(area == 1){
                //calculation size
                stota->sector_offset = (KDP_FLASH_ALL_MODELS_ADDR_1 + stota->second_last_start_add)>>12;    //model
            }
            else{
                //calculation size
                stota->sector_offset = (KDP_FLASH_ALL_MODELS_ADDR + stota->second_last_start_add)>>12;    //model
            }

            #else
            //calculation size
            stota->sector_offset = (KDP_FLASH_ALL_MODELS_ADDR + stota->second_last_start_add)>>12;    //model
            #endif
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA cfg] partial model start address in flash: m_start 0x%x , second_last_address:0x%x", KDP_FLASH_ALL_MODELS_ADDR , stota->second_last_start_add  );
            dbg_msg_flash("[OTA cfg] partial model start sector: x%x", stota->sector_offset );
            dbg_msg_flash("[OTA cfg] ..........." );
            #endif
            stota->sector_acc = 0;
            stota->bin_type = eOTA_bin_partial_model;//partial model bin
            stota->target_sectors = stota->model_size>>12;      //( MODEL_IMAGE_SIZE >> 12 );
            stota->target_bytes = stota->model_size;    //MODEL_IMAGE_SIZE;
            stota->ddr_ptr = (UINT32 *)KDP_DDR_MODEL_START_ADDR;        //for bin file
        }
        stota->ddr_ptr_index = 0;
        stota->CRC = 0;
        stota->pass_sector_count = 0;
    }
    else
    {
        ret = 	FLASH_DRV_NOTHING;
    }
    return ret;
}

UINT16 ota_bin_check( OTA_FLASHt * stota, UINT32 **ptr )
{
    *ptr = stota->ddr_ptr;

    //check valid size!!
    if( stota->bin_type == eOTA_bin_scpu )
    {
        if( stota->ddr_ptr_index != KDP_FLASH_FW_SCPU_SIZE )
        {
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] scpu bin size error , size:%d--", stota->ddr_ptr_index);   //20200217 add
            #endif
            stota->sn_cpu_status =  FLASH_STATUS_RECEIVE_OVER_SIZE_FAIL;
            return  FLASH_STATUS_RECEIVE_OVER_SIZE_FAIL;
            //break;
        }
        #if( OTA_LOG_EN == YES )
        dbg_msg_flash("[OTA] scpu flash bin pass"); //20200217 add
        #endif
    }
    else if( stota->bin_type == eOTA_bin_ncpu )
    {
        if( stota->ddr_ptr_index != KDP_FLASH_FW_NCPU_SIZE )
        {
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] ncpu bin size error , size:%d--", stota->ddr_ptr_index);   //20200217 add
            #endif
            stota->sn_cpu_status =  FLASH_STATUS_RECEIVE_OVER_SIZE_FAIL;
            return  FLASH_STATUS_RECEIVE_OVER_SIZE_FAIL;
            //break;
        }
        #if( OTA_LOG_EN == YES )
        dbg_msg_flash("[OTA] ncpu flash bin pass"); //20200217 add
        #endif
    }
    else if( stota->bin_type == eOTA_bin_all_model )
    {
        if( stota->ddr_ptr_index != stota->target_bytes )
        {
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] model flash bin faiel: 0x%x", stota->ddr_ptr_index );
            #endif
            stota->sn_cpu_status =  FLASH_STATUS_RECEIVE_OVER_SIZE_FAIL;
            return  FLASH_STATUS_RECEIVE_OVER_SIZE_FAIL;
        }
        #if( OTA_LOG_EN == YES )
        dbg_msg_flash("[OTA] model flash bin pass");
        #endif
    }
    else if( stota->bin_type == eOTA_bin_ui_image )
    {
        if( stota->ddr_ptr_index != stota->target_bytes )
        {
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] Image UI flash bin fail: 0x%x", stota->ddr_ptr_index );
            #endif
            stota->sn_cpu_status =  FLASH_STATUS_RECEIVE_OVER_SIZE_FAIL;
            return  FLASH_STATUS_RECEIVE_OVER_SIZE_FAIL;
        }
        //find 4 aligned address
        *ptr = (UINT32 *)KDP_DDR_MODEL_START_ADDR + stota->ddr_ptr_index;
        while( ((UINT32)(*ptr)%4) != 0 ){
            (*ptr)=(*ptr)+1;
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] read buf address: 0x%X", ptr);
            #endif
        }
        #if( OTA_LOG_EN == YES )
        dbg_msg_flash("[OTA] Image UI flash bin pass");
        #endif
    }
    else if( stota->bin_type == eOTA_bin_fw_info )
    {
        if( stota->ddr_ptr_index != stota->target_bytes )
        {
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] fw info flash bin fail: 0x%x", stota->ddr_ptr_index );
            #endif
            stota->sn_cpu_status =  FLASH_STATUS_RECEIVE_OVER_SIZE_FAIL;
            return  FLASH_STATUS_RECEIVE_OVER_SIZE_FAIL;
        }
        #if( OTA_LOG_EN == YES )
        dbg_msg_flash("[OTA] fw info flash bin pass");
        #endif
    }
    else if( stota->bin_type == eOTA_bin_partial_model )  //partial model
    {
        if( (stota->ddr_ptr_index != stota->target_bytes) || (stota->ddr_ptr_index != stota->model_size) )
        {
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] partial model flash bin fail: 0x%x", stota->ddr_ptr_index );
            dbg_msg_flash("[OTA] partial model target size: 0x%x", stota->model_size );
            #endif
            stota->sn_cpu_status =  FLASH_STATUS_RECEIVE_OVER_SIZE_FAIL;
            return  FLASH_STATUS_RECEIVE_OVER_SIZE_FAIL;
        }

        #if (OTA_CRC_CHECK_EN==NO)
        //find 4 aligned address
        *ptr = (UINT32 *)KDP_DDR_MODEL_START_ADDR + stota->ddr_ptr_index;
        while( ((UINT32)(*ptr)%4) != 0 ){
            (*ptr)=(*ptr)+1;
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] read buf address: 0x%X", ptr);
            #endif
        }
        #endif

        #if( OTA_LOG_EN == YES )
        dbg_msg_flash("[OTA] partial model flash bin pass");
        #endif
    }
    else
    {
        stota->sn_cpu_status =  FLASH_STATUS_RECEIVE_OVER_SIZE_FAIL;
        #if( OTA_LOG_EN == YES )
        dbg_msg_flash("[OTA] bin type fatal fail");
        #endif
        return  FLASH_STATUS_RECEIVE_OVER_SIZE_FAIL;
        //break;
    }

    return FLASH_STATUS_OK;
}


UINT32 drv_read_all_model_crc(void)
{
    UINT32  crc_r;
    kdp_memxfer_flash_to_ddr((uint32_t)&crc_r, MODEL_MGR_FLASH_ADDR_MODEL_INFO+kdp_crc_offset_in_fwinfo()+kdp_get_fwinfo_offset() , 4);
    return crc_r;
}

UINT32 drv_read_each_model_crc(uint8_t idx)
{
    UINT32  crc_r;
    kdp_memxfer_flash_to_ddr((uint32_t)&crc_r,
                             MODEL_MGR_FLASH_ADDR_MODEL_INFO+kdp_crc_offset_in_fwinfo()+kdp_get_fwinfo_offset() + idx*4+4
                             , 4);
    return crc_r;
}

//**********************************************************
//**********************************************************
//**************flash parser coding*************************
//**********************************************************
//**********************************************************

UINT16 drv_flash_main( OTA_FLASHt * stota  )
{
    UINT32  addr;
    UINT32  i, ntemp_index=0 , ntemp_index2=0 ;
    UINT32  *ptr;

#if ( (CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2) && (NIR_RGB_OTA_EN == 1) )
    UINT16  ntime_out = 0;
    UINT16  ntime_out_target = 5;
    UINT16  ntime_out_interval = 100;		//1000ms is 1s
#endif

    UINT16  ret = 0xFF;
    UINT16  end_sector = 0;
    #if ( OTA_CRC_CHECK_EN==YES )
    UINT32  crc_result;
    UINT32  crc_target;
    #endif
    #if(OTA_USER_BACKUP==YES)
    UINT8   temp;
    #endif

    addr = stota->receive_cmd.sector_index * st_flash_info.sector_size_Bytes;

    switch( stota->receive_cmd.cmd_stat )
    {
        case eflash_init:

            //action number check
            #if ( NIR_RGB_OTA_EN == 1 )
            if(  stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_INIT
                    || stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_SW_VERSION )
            {
    			if( kdp_memxfer_init(MEMXFER_OPS_CPU, MEMXFER_OPS_CPU) != 1)
    			{
    				return FLASH_DRV_FAIL;
    			}
    			else
    			{
    			    kl520_api_face_close();
    			}

                //read boot config
                // dbg_msg_console("ota_get_active_scpu_partition");
                // ota_get_active_scpu_partition();
                // ota_get_active_ncpu_partition();
            }
            else if( stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_SEL_MODEL_PARTIAL )
            {
                //do nothing
            }
            else if( stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_PROC_DONE )
            {
                    goto FLASH_Large_process;
            }

            if(  stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_NIR )
            {
                //call active
                //kl520_api_snapshot_fdfr_catch(MIPI_CAM_NIR);
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
                osDelay(100);
                kl520_api_face_close();
#endif
            }
            else if(  stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_RGB )
            {
                //call active
                //kl520_api_snapshot_fdfr_catch(MIPI_CAM_RGB);
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
                osDelay(100);
                kl520_api_face_close();
#endif
            }


            #else
            if(  stota->receive_cmd.action_number != 0x01 )		return FLASH_DRV_FAIL;
            #endif

            break;
FLASH_Large_process:
        case eflash_large_program:
            ntemp_index =0;
            ntemp_index2 = KDP_DDR_TEST_RGB_IMG_SIZE;

            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] receive bin size, size:%d--", stota->ddr_ptr_index);
            #endif

            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] flash bin type %d--", stota->bin_type);
            #endif

            //check valid size!!
            if ( ota_bin_check(stota , &ptr ) == FLASH_STATUS_RECEIVE_OVER_SIZE_FAIL )
            {
                return FLASH_STATUS_RECEIVE_OVER_SIZE_FAIL;
            }

            //erase area
            Drv_flash_erase_area( stota->sector_offset );
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] Start Program --");
            dbg_msg_flash("[OTA] dst: %#X, src: %#X, size: %#X --", (UINT32)stota->sector_offset<<12 ,(UINT32)stota->ddr_ptr, stota->target_bytes );
            #endif

            //program area
            if( kdp_memxfer_ddr_to_flash((UINT32)stota->sector_offset<<12 ,(UINT32)stota->ddr_ptr, stota->target_bytes ) == 0 )
            {
                #if( OTA_LOG_EN == YES )
                dbg_msg_flash("[OTA] Large Program fail");
                #endif
                return FLASH_STATUS_PROGRAM_FAIL;
            }

            #if ( OTA_CRC_CHECK_EN == YES )

            if( stota->bin_type == eOTA_bin_ui_image )   //no check
            {
                //no data compare
                goto final_check;
            }
            else if( stota->bin_type == eOTA_bin_all_model || stota->bin_type == eOTA_bin_partial_model )    //partial model
            {
                kdp_model_reload_model_info(FALSE);
                #if ( OTA_USER_BACKUP == YES )
                //step1 check wait area
                if( ota_get_wait_active_area( USER_PARTITION_FW_INFO ) == 1 )   //sync with fw_info
                {
                    #if( OTA_LOG_EN == YES )
                    dbg_msg_flash("[OTA fw info] set 1");
                    #endif
                    temp =0;    //pre area
                    kdp_set_model_offset(KDP_FLASH_ALL_MODEL_OFFSET_1);
                    kdp_set_fwinfo_offset(KDP_FLASH_FW_INFO_OFFSET_1);
                }
                else
                {
                    #if( OTA_LOG_EN == YES )
                    dbg_msg_flash("[OTA fw info] set 0");
                    #endif
                    temp =1;
                    kdp_set_model_offset(0);
                    kdp_set_fwinfo_offset(0);
                }
                //step2 set to this area
                #if ( OTA_LOG_EN == YES )
                dbg_msg_flash("[fw reload] load info offest : 0x%x", kdp_get_fwinfo_offset() );
                dbg_msg_flash("[fw reload] model offest : 0x%x", kdp_get_model_offset() );
                #endif
                //step3 roll back to another area
                kdp_model_info_reload();
                //crc check
                 stota->ddr_ptr_index = kdp_clc_all_model_size();
                 #if( OTA_LOG_EN == YES )
                 dbg_msg_flash("[OTA] model start address 0x%x", stota->sector_offset );
                 #endif


                #else
                //crc check
                 stota->ddr_ptr_index = kdp_clc_all_model_size();
                 #if( OTA_LOG_EN == YES )
                 dbg_msg_flash("[OTA] model start address 0x%x", stota->sector_offset );
                 #endif
                //step3 roll back to another area
                kdp_model_info_reload();
                #endif

                #if( OTA_USER_BACKUP == YES )
                stota->sector_offset = MODEL_START_SECTOR_INDEX + (kdp_get_model_offset()>>12);        //for partial mode , read from model start

                #else
                stota->sector_offset = MODEL_START_SECTOR_INDEX;        //for partial mode , read from model start
                #endif
            }
            else
            {
                //data compare
            }

            #else
            if( stota->bin_type == eOTA_bin_all_model /*|| stota->bin_type == eOTA_bin_ui_image*/ || stota->bin_type == eOTA_bin_partial_model )   //no check
            {
                goto final_check;
            }
            #endif

            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] temp address: 0x%x", ptr);
            dbg_msg_flash("[OTA]read size: %d", stota->ddr_ptr_index );

            dbg_msg_flash("[OTA]start read sector address: 0x%x", (stota->sector_offset *4096) );
            #endif

            *(ptr+0) = 0x12;
            *(ptr+1) = 0xFF;
            *(ptr+2) = 0xFF;
            //read back and check
            if( kdp_memxfer_flash_to_ddr( (UINT32)ptr ,(UINT32)stota->sector_offset << 12, stota->ddr_ptr_index ) != 0 )
            {
                #if( OTA_LOG_EN == YES )
                dbg_msg_flash("[OTA] Large Read fail");
                #endif
                return	FLASH_STATUS_PROGRAM_FAIL;
            }

            #if ( OTA_CRC_CHECK_EN == YES )

            if( stota->bin_type == eOTA_bin_all_model || stota->bin_type == eOTA_bin_partial_model )    //partial model
            {
                crc_result =  ota_crc32( (UINT8 *)ptr, stota->ddr_ptr_index );
                crc_target =  drv_read_all_model_crc();
                #if( OTA_LOG_EN == YES )
                dbg_msg_flash("[OTA] start CRC check");
                dbg_msg_flash("CRC result is 0x%x",crc_result );
                #endif

                if( crc_result != crc_target )
                {
                    dbg_msg_flash("CRC error is 0x%x, target: 0x%x", crc_result, crc_target );
                    return  FLASH_STATUS_CRC_FAIL;
                }
                dbg_msg_flash("CRC check Done");
                goto final_check;
            }
            #endif

            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] Compare large data --");
            #endif
            //error code and doing data compare or CRC check
            #if ( OTA_CRC_CHECK_EN == YES )
            if(stota->bin_type == eOTA_bin_all_model  ||  stota->bin_type == eOTA_bin_partial_model )    //all model and partial model
            {

            }
            else
            {
                for( i =0; i < (stota->ddr_ptr_index>>2); i++ )
                {
                    #if( OTA_LOG_EN == YES )
                    if(0)//( (i%4096) ==0)
                    {
                        dbg_msg_flash("[OTA] Large compare: %d rx: 0x%x, org: 0x%x,", i,*(ptr+i), *(stota->ddr_ptr+i));
                    }
                    #endif

                    if( *(ptr+i) != *(stota->ddr_ptr+i) )
                    {

                        #if( OTA_LOG_EN == YES )
                        dbg_msg_flash("[OTA] Large compare fail");
                        #endif

                        return  FLASH_STATUS_COMPARE_FAIL;
                    }
                    stota->pass_sector_count+=4;
                }
            }



            #else
            for( i =0; i < (stota->ddr_ptr_index>>2); i++ )
            {
                #if( OTA_LOG_EN == YES )
//                if( (i%4096) ==0)
//                {
//                    dbg_msg_flash("[OTA] Large compare: %d rx: 0x%x, org: 0x%x,", i,*(ptr+i), *(stota->ddr_ptr+i));
//                }
                #endif

                if( *(ptr+i) != *(stota->ddr_ptr+i) )
                {

                    #if( OTA_LOG_EN == YES )
                    dbg_msg_flash("[OTA] Large compare fail");
                    #endif

                    return  FLASH_STATUS_COMPARE_FAIL;
                }
                stota->pass_sector_count+=4;
            }
            #endif

            stota->pass_sector_count >>= 12;
            if( (stota->pass_sector_count) !=  ( stota->target_sectors) )
            {
                #if( OTA_LOG_EN == YES )
                dbg_msg_flash("[OTA] Pass count error 2: %d, %d", stota->pass_sector_count, stota->target_sectors );
                #endif
                return	FLASH_STATUS_COMPARE_FAIL;
            }
            final_check:
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] Large program procedure Done --");
            #endif
            stota->sn_cpu_status = 0;
            break;
        case eflash_store_DDR:
            //need to verify first
            ntemp_index = stota->receive_cmd.data_size>>2;
            ntemp_index2 = stota->ddr_ptr_index>>2;

            ptr = (UINT32 *)(stota->receive_cmd.ptr + stota->receive_cmd.data_offset);

            for( i = 0; i < ntemp_index ; i++ )
            {
                *( stota->ddr_ptr + ntemp_index2+i ) = *( ptr+ i );
            }
            stota->ddr_ptr_index += (  ntemp_index * 4 );
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] ntemp_index2+ntemp_index * 4: %d",  ntemp_index * 4);
            #endif
            break;
        case eflash_program:
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] Program ing, sector: %d", stota->receive_cmd.sector_index );
            #endif

            if( kdp_memxfer_ddr_to_flash( (UINT32)addr, (UINT32)stota->receive_cmd.ptr+stota->receive_cmd.data_offset,
                                                stota->receive_cmd.data_size) == 0 )
            {
            #if( OTA_LOG_EN == YES )
                dbg_msg_flash("[OTA] Program fail");
            #endif
                return FLASH_DRV_FAIL;
            }

            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] Program Done");
            #endif
            break;
        case eflash_read:
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] Read ing");
            #endif
            #if 1   //( CFG_COM_BUS_TYPE == COM_BUS_TYPE_SSP1 )
                *stota->send_response.ptr_index = 0;		//clear buf index
                *stota->receive_cmd.ptr_index = 0;			//clear buf index

                if ( kdp_memxfer_flash_to_ddr( (UINT32)stota->receive_cmd.ptr ,(UINT32)addr,  stota->receive_cmd.data_size ) != 0 )
                {
                    #if( OTA_LOG_EN == YES )
                    dbg_msg_flash("[OTA] Read fail");
                    #endif
                    return FLASH_DRV_FAIL;
                }
                stota->send_response.data_size = stota->receive_cmd.data_size;
                *stota->send_response.ptr_index = stota->receive_cmd.data_size;
                #if( OTA_LOG_EN == YES )
                dbg_msg_flash("[OTA] Read size : %d", stota->receive_cmd.data_size);
                dbg_msg_flash("[OTA] Read Done");
                #endif
            #else
//				dbg_msg_flash("   [OTA] flash read ");
                *stota->send_response.ptr_index = 0;		//clear buf index

                //if( kdp_memxfer_flash_to_ddr(stota->send_response.ptr, add , stota->receive_cmd.data_size ) != 0 )
                if(kdp_flash_read_data( addr , stota->send_response.ptr , stota->receive_cmd.data_size ) == 0 )
                {
                    return FLASH_DRV_FAIL;
                }
                stota->send_response.data_size = stota->receive_cmd.data_size;
                *stota->send_response.ptr_index = stota->receive_cmd.data_size;
                //dbg_msg_flash("   [OTA] flash done ");
            #endif
        break;
        case eflash_erase:
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] erase ing");
            #endif
            end_sector = stota->receive_cmd.data_offset;

            if ( kdp_memxfer_flash_sector_multi_erase( stota->receive_cmd.sector_index*4096, end_sector*4096 ) != 0 )
            {
                #if( OTA_LOG_EN == YES )
                dbg_msg_flash("[OTA] erase fail");
                #endif
                return FLASH_DRV_FAIL;
            }
            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("Done");
            #endif
            break;
        case eflash_ui_image_info:
            if(0)//( stota->sector_offset != USR_IMAGE_START_SECTOR_INDEX)
            {
                return FLASH_DRV_FAIL;
            }
        break;
#if ( (CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2) && (NIR_RGB_OTA_EN == 1) )
        case eflash_NIR:
            if( (stota->receive_cmd.packet_ddr_ptr = kl520_api_snapshot_addr(0) ) == 0 )
            {
                osDelay(ntime_out_interval);
                //call wait
                ntime_out++;
                osDelay( ntime_out_interval );
                if( ntime_out > ntime_out_target )
                {
                    return FLASH_DRV_FAIL;
                }
            }
            *stota->send_response.ptr_index = stota->receive_cmd.data_size;
            break;

        case eflash_RGB:
        case eflash_ALL:
            if( (stota->receive_cmd.packet_ddr_ptr = kl520_api_snapshot_addr(1) )== 0)
            {
                osDelay(ntime_out_interval);
                //call wait
                ntime_out++;
                osDelay( ntime_out_interval );
                if( ntime_out > ntime_out_target )
                {
                    return FLASH_DRV_FAIL;
                }
            }
            *stota->send_response.ptr_index = stota->receive_cmd.data_size;
            break;
#endif
        case eflash_model_size:
            //do nothing here

            #if( OTA_USER_BACKUP == YES )
            //step1 check wait area
            if( ota_get_wait_active_area( USER_PARTITION_FW_INFO ) == 1 )
            {
                #if( OTA_LOG_EN == YES )
                dbg_msg_flash("[OTA fw info] set 1"); //....ok
                #endif
                temp =0;    //pre area
                kdp_set_fwinfo_offset( KDP_FLASH_FW_INFO_OFFSET_1 );
            }
            else
            {
                #if( OTA_LOG_EN == YES )
                dbg_msg_flash("[OTA fw info] set 0");
                #endif
                temp =1;
                kdp_set_fwinfo_offset(0);
            }
            //step2 set to this area

            #if ( OTA_LOG_EN == YES )
            dbg_msg_flash("[fw reload] load info offest : 0x%x", kdp_get_fwinfo_offset() );
            #endif

            //step3 roll back to another area
            #endif
            kdp_model_info_reload();

            #if( OTA_USER_BACKUP == YES )
            kdp_set_fwinfo_offset( temp* KDP_FLASH_FW_INFO_OFFSET_1 ); //roll back
            #endif

            if( kdp_model_info_get( &stota->second_last_start_add, &stota->last_start_add, &stota->model_size ) == 0 )
            {
                ret =   FLASH_STATUS_READ_CONFIG_FAIL;
            }

            #if( OTA_LOG_EN == YES )
            dbg_msg_flash("[OTA] part model size: %d", stota->model_size );
            dbg_msg_flash("[OTA] part model ofsset: 0x%X", stota->second_last_start_add );
            #endif


            break;
        case eflash_idle:

            break;
        default:

            break;
    }



    if( stota->receive_cmd.cmd_stat == FLASH_CMD_ACT )
    {
        ret = Drv_flash_action_doing( stota );
        if( ret != FLASH_DRV_NOTHING )
        {
            return ret;
        }
    }


    return FLASH_DRV_OK;


}



void Drv_OTA_init(OTA_FLASHt * stota)
{
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )

    stota->receive_cmd.ptr = 	(UINT8 *)driver_ssp_ctx.Rx_buffer;
    stota->receive_cmd.ptr_index = (UINT32 *)driver_ssp_ctx.Rx_buffer_index;		//&gu8OTA_Rx_buffer_index;
    *stota->receive_cmd.ptr_index = 0;

    stota->send_response.ptr = 	(UINT8 *)driver_ssp_ctx.Tx_buffer;   //gu8OTA_Tx_buffer;
    stota->send_response.ptr_index = (UINT32 *)driver_ssp_ctx.Tx_buffer_index;	//&gu8OTA_Tx_buffer_index;
    *stota->send_response.ptr_index = 0;

    stota->send_response.buf_max_size = driver_ssp_ctx.buffer_max_size;

#else
    stota->receive_cmd.ptr = gu8OTA_Rx_buffer;
    stota->receive_cmd.ptr_index = &gu8OTA_Rx_buffer_index;
    *stota->receive_cmd.ptr_index = 0;

    stota->send_response.ptr = gu8OTA_Rx_buffer;
    stota->send_response.ptr_index = &gu8OTA_Tx_buffer_index;
    *stota->send_response.ptr_index = 0;
#endif

    #if (  CFG_OTA_FLASH_BUF_ENABLE == YES)
    stota->temp_buffer = (UINT32 *)KDP_DDR_OTA_FLASH_BUF_START_ADDR;
    stota->temp_buffer_index = 0;
    stota->temp_buffer_size = KDP_DDR_OTA_FLASH_BUF_RESERVED;
    #endif

    stota->target_sectors = 0;
    stota->pass_sector_count = 0;
    stota->sector_offset = 0;
    stota->bin_type = eOTA_bin_null;
    stota->target_bytes = 0;

#if 1
    stota->ddr_ptr = (UINT32 *) KDP_DDR_OTA_IMAGE_BUF_START_ADDR;		//for bin file
#else
    stota->ddr_ptr = (UINT32 *)kdp_ddr_reserve(16*1024*1024);
#endif
    stota->ddr_ptr_index = 0;

}

UINT32	Drv_utility_checksum( UINT8 * buf, UINT16 start, UINT16 end )
{
    UINT32 ntemp = 0;
#if 1
    UINT32 i ;
    for( i = start ; i < end; i++  )
    {
        ntemp += *(buf+i);
    }
#else

    do
    {
        ntemp += *(buf+ (end--) );
    } while( (int)end >= start );

#endif

    return	ntemp;
}

UINT8 drv_ota_packet_analyze( OTA_FLASHt * stota )
{
    UINT32	i = 0;
    UINT8		*ptr = stota->receive_cmd.ptr;
    UINT32	*ptr_indedx = stota->receive_cmd.ptr_index;
    UINT32	nhead_index = 0xFFFFFFFF;
    UINT32	ncheck_sum = 0;


#if( OTA_TIMING_DEBUG_EN == YES )
    kdp520_gpio_setdata( 1<<24);
    kdp520_gpio_cleardata( 1<<24);
    kdp520_gpio_setdata( 1<<24);
    kdp520_gpio_cleardata( 1<<24);
#endif

#if( OTA_LOG_EN == YES )
    //dbg_msg_flash("   [OTA] Analyze 1..");
    //dbg_msg_flash("[OTA] Packet Rx size: %d", *ptr_indedx );
#endif

    stota->receive_cmd.busy_flag = 1;		//flash controller is busy

    //-----------------------
    //packet tail  check
    //-----------------------
    if(	(*(ptr + (*ptr_indedx) -1)  !=  FLASH_TAIL_1) || ( *(ptr + (*ptr_indedx) -2 )  != FLASH_TAIL_2 ) )
    {
        return PACKET_TAIL_CHECK_ERROR;
    }

    //-----------------------
    //-----	head check ------
    //-----------------------
    for( i = 0; i < ( *ptr_indedx-4 ); i++ )
    {
        if(	 (*(ptr + i + 0 ) == FLASH_HEAD_RX_1) && (*(ptr +i +  1 ) == FLASH_HEAD_RX_2 )
            && (*(ptr +i +  2 ) == FLASH_HEAD_RX_3) && (*(ptr +i +  3 ) == FLASH_HEAD_RX_4 )	)
        {
            nhead_index = i;
            *(ptr + i + 0 ) = 0;
            *(ptr +i +  1 ) = 0;
            break;
        }
    }
    if( nhead_index == 0xFFFFFFFF  )
    {
        return PACKET_HEAD_CHECK_ERROR;
    }

    #if( OTA_TIMING_DEBUG_EN == YES )
    kdp520_gpio_setdata( 1<<24);
    kdp520_gpio_cleardata( 1<<24);
    kdp520_gpio_setdata( 1<<24);
    kdp520_gpio_cleardata( 1<<24);
    kdp520_gpio_setdata( 1<<24);
    kdp520_gpio_cleardata( 1<<24);
    kdp520_gpio_setdata( 1<<24);
    kdp520_gpio_cleardata( 1<<24);
    #endif

    stota->receive_cmd.head = FLASH_HEAD_RX;							//receive data from device
    stota->receive_cmd.host_number = *(ptr+nhead_index+4)<<8 | *(ptr+nhead_index+5);
    stota->receive_cmd.cmd_stat = 	*(ptr+nhead_index+6)<<8  |  *(ptr+nhead_index+7) ;

    stota->receive_cmd.action_number = 0;

    if( stota->receive_cmd.cmd_stat == FLASH_PROGRAM_ACT )
    {
        stota->receive_cmd.sector_index= ( *( ptr+nhead_index+8) << 8 ) | *(ptr+nhead_index + 9);
        stota->receive_cmd.action_number = 0;
        stota->receive_cmd.data_size=	( *( ptr+nhead_index+10) << 8 ) | *(ptr+nhead_index+11);

        stota->receive_cmd.data_offset = nhead_index+12;

        ncheck_sum = Drv_utility_checksum( ptr, nhead_index+6, *ptr_indedx - 6 );
    }
    else if( stota->receive_cmd.cmd_stat == FLASH_DATA_TO_DDR_ACT )		// add
    {

        stota->receive_cmd.sector_index= ( *( ptr+nhead_index+8) << 8 ) | *(ptr+nhead_index + 9);
        stota->receive_cmd.action_number = 0;
        stota->receive_cmd.data_size=	( *( ptr+nhead_index+10) << 8 ) | *(ptr+nhead_index+11);

        stota->receive_cmd.data_offset = nhead_index+12;

        ncheck_sum = Drv_utility_checksum( ptr, nhead_index+6, *ptr_indedx - 6 );

    }

#if( NIR_RGB_OTA_EN==1 )
    else if( stota->receive_cmd.cmd_stat == FLASH_READ_ACT ||
             stota->receive_cmd.cmd_stat == FLASH_READ_NIR_ACT ||
             stota->receive_cmd.cmd_stat == FLASH_READ_ALL_ACT ||
             stota->receive_cmd.cmd_stat == FLASH_READ_RGB_ACT )
#else
        else if( stota->receive_cmd.cmd_stat == FLASH_READ_ACT )
#endif
    {
            //dbg_msg_flash("   [OTA] Analyze read..");

        #if( NIR_RGB_OTA_EN==1 )
        stota->receive_cmd.action_number = *(ptr+nhead_index+8) ;
        #endif
        stota->receive_cmd.sector_index =	( *( ptr+nhead_index+8) << 8 ) | *(ptr+nhead_index + 9);
        stota->receive_cmd.data_size=	( *( ptr+nhead_index+10) << 8 ) | *(ptr+nhead_index+11);
        stota->receive_cmd.data_offset = 0;
        ncheck_sum = Drv_utility_checksum( ptr, nhead_index+6, *ptr_indedx - 6 );
    }
    else if( stota->receive_cmd.cmd_stat == FLASH_ERASE_SECTOR_ACT )
    {
        stota->receive_cmd.sector_index =	( *( ptr+nhead_index+8) << 8 ) | *(ptr+nhead_index + 9);			//start sector
        stota->receive_cmd.data_offset =	( *( ptr+nhead_index+10) << 8 ) | *(ptr+nhead_index + 11);		//end sector

        stota->receive_cmd.data_size=	0;
        ncheck_sum = Drv_utility_checksum( ptr, nhead_index+6, *ptr_indedx - 6 );
    }
    else if( stota->receive_cmd.cmd_stat == FLASH_UI_MODEL_INFO )
    {
        stota->sector_offset = 0;
        stota->target_bytes = ( *( ptr+nhead_index+12) << 24 ) | (*( ptr+nhead_index+13) << 16 )| (*( ptr+nhead_index+14) << 8 )| (*( ptr+nhead_index+15) << 0 );
        stota->target_sectors = stota->target_bytes>>12;
        #if( OTA_LOG_EN == YES )
        dbg_msg_flash("[OTA cfg] 0xFE target_sectors : %d", stota->target_sectors );
        #endif
        ncheck_sum = Drv_utility_checksum( ptr, nhead_index+6, *ptr_indedx - 6 );
    }
    else if ( stota->receive_cmd.cmd_stat == FLASH_MODEL_SIZE )
    {
        if(  (*ptr_indedx-6) <= (nhead_index+6) )
        {
            return PACKET_TAIL_CHECK_ERROR;
        }
        ncheck_sum = Drv_utility_checksum( ptr, nhead_index+6, *ptr_indedx - 6 );
    }
    else	// if( stota->receive_cmd.cmd_stat == FLASH_CMD_ACT )
    {
        stota->receive_cmd.action_number = *( ptr+nhead_index + 8 );
        //action
#if CFG_SNAPSHOT_ENABLE == 2
        #if( NIR_RGB_OTA_EN==1 )
        if(  stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_NIR )
        {
            //call NIR capture
            kl520_api_snapshot_fdfr_catch(MIPI_CAM_NIR);
        }
        else if(  stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_RGB )
        {
            //call RGB capture
            kl520_api_snapshot_fdfr_catch(MIPI_CAM_RGB);
        }
        #endif
#endif
        stota->receive_cmd.sector_index = 0;
        stota->receive_cmd.data_size=	0;
        ncheck_sum = Drv_utility_checksum( ptr, nhead_index+6, *ptr_indedx - 6 );
        stota->receive_cmd.data_offset = 0;
    }

    stota->receive_cmd.check_sum = ( *( ptr+ (*ptr_indedx) -6 ) << 24 ) | (*(ptr+nhead_index+ (*ptr_indedx) -5 )<<16 ) |
                                   ( *( ptr+ (*ptr_indedx) -4 ) << 8 ) | *(ptr+nhead_index+ (*ptr_indedx) -3 );

    stota->receive_cmd.tail = FLASH_TAIL;

    //check sum error
    if( stota->receive_cmd.check_sum != ncheck_sum  )
    {
        return PACKET_HEAD_CHECK_ERROR;
    }
    return FLASH_PACKET_OK;	//exist good data
}

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
void Drv_Uart2_Tx_arrange( void * data_ptr , UINT16 len)
{
#if ( ( CFG_COM_BUS_TYPE&COM_BUS_UART0 ) == COM_BUS_UART0 )
    kdp_uart_dev_id uart_dev = UART0_DEV;
#elif ( ( CFG_COM_BUS_TYPE&COM_BUS_UART1 ) == COM_BUS_UART1 )
    kdp_uart_dev_id uart_dev = UART1_DEV;
#elif ( ( CFG_COM_BUS_TYPE&COM_BUS_UART2 ) == COM_BUS_UART2 )
    kdp_uart_dev_id uart_dev = UART2_DEV;
#elif ( ( CFG_COM_BUS_TYPE&COM_BUS_UART3 ) == COM_BUS_UART3 )
    kdp_uart_dev_id uart_dev = UART3_DEV;
#elif ( ( CFG_COM_BUS_TYPE&COM_BUS_UART4 ) == COM_BUS_UART4 )
    kdp_uart_dev_id uart_dev = UART4_DEV;
#else
    kdp_uart_dev_id uart_dev = NULL;
#endif

    if( len == 1 )
    {
        kdp_uart_write(uart_dev, (UINT8 *)data_ptr, 1 );
    }
    else if( len == 2 )
    {
        kdp_uart_write(uart_dev, ((UINT8 *)data_ptr)+1, 1 );
        kdp_uart_write(uart_dev, ((UINT8 *)data_ptr)+0, 1 );
    }
    else if( len == 4 )
    {
        kdp_uart_write(uart_dev, ((UINT8 *)data_ptr)+3, 1 );
        kdp_uart_write(uart_dev, ((UINT8 *)data_ptr)+2, 1 );
        kdp_uart_write(uart_dev, ((UINT8 *)data_ptr)+1, 1 );
        kdp_uart_write(uart_dev, ((UINT8 *)data_ptr)+0, 1 );
    }
    else
    {
        //error
    }

}

void Drv_Uart_Tx_send( OTA_FLASHt * stota )
{
    UINT16	i = 0;
    UINT16	ntemp = 0;
#if( NIR_RGB_OTA_EN==1 )
    UINT32	noffset =0;
#endif


    #if( PRINT_FLASH_MECHINE_DEBUG_EN == YES )
    kdp_printf("------\r\n");
    kdp_printf("head 0x%x \r\n", stota->send_response.head);
    kdp_printf("host number 0x%x \r\n", stota->send_response.host_number);
    #endif


    Drv_Uart2_Tx_arrange( &stota->send_response.head, sizeof(stota->send_response.head) );
    Drv_Uart2_Tx_arrange( &stota->send_response.host_number, sizeof(stota->send_response.host_number) );


    if(  stota->send_response.status == FLASH_STATUS_FAIL )
    {
        #if( PRINT_FLASH_MECHINE_DEBUG_EN == YES )
        kdp_printf("Status 0xEEEE\r\n");
        kdp_printf("Tail 0x7887\r\n");
        #endif

        Drv_Uart2_Tx_arrange( &stota->send_response.status, sizeof(stota->send_response.status) );
        Drv_Uart2_Tx_arrange( &stota->send_response.tail, sizeof(stota->send_response.tail) );

        return;
    }

    //normal status
    if(   stota->send_response.cmd_stat == FLASH_PROGRAM_ACT )
    {
            //send program data
        #if( PRINT_FLASH_MECHINE_DEBUG_EN == YES )
        kdp_printf("program response 0x7855\r\n");
        #endif

        Drv_Uart2_Tx_arrange( &stota->send_response.status, sizeof(stota->send_response.status) );

    }

    else if(  stota->send_response.cmd_stat == FLASH_READ_ACT )
    {
        Drv_Uart2_Tx_arrange( &stota->send_response.status, sizeof(stota->send_response.status) );
        Drv_Uart2_Tx_arrange( &stota->send_response.sector_index, sizeof(stota->send_response.sector_index) );
        Drv_Uart2_Tx_arrange( &stota->send_response.data_size, sizeof(stota->send_response.data_size) );

        //dbg_msg_flash("   [OTA] Tx send Tx_size.. pre 0x%x", *stota->send_response.ptr_index);
        //dbg_msg_flash("   [OTA] Data size  0x%x", stota->send_response.data_size );

        //send read data
        //a lot of data
        *stota->send_response.ptr_index = 0 ;

        for(  i=0 ; i< stota->send_response.data_size; i++  )
        {
            ntemp = *( ( (UINT8 *)stota->send_response.ptr ) + i );
            Drv_Uart2_Tx_arrange( (UINT8 *)&ntemp, 1);
            *stota->send_response.ptr_index = *stota->send_response.ptr_index + 1 ;

            if( (i%255 ==0) && (i>0) )
            {
                delay_ms(1);
            }

            #if  0//( PRINT_FLASH_MECHINE_DEBUG_EN == YES )
            //-----debug use--------
            kdp_printf("0x%X ", ntemp );
            if( i%16 ==0 && i >0 )
            {
                kdp_printf("\r\n");
            }
            #endif
        }

        //dbg_msg_flash("   [OTA] Tx send Tx_size.. after 0x%x", *stota->send_response.ptr_index);
        //send checksum
        Drv_Uart2_Tx_arrange( &stota->send_response.check_sum, sizeof(stota->send_response.check_sum) );

    }
    #if( NIR_RGB_OTA_EN==1 )
    else if(  stota->send_response.cmd_stat == FLASH_READ_NIR_ACT ||  stota->send_response.cmd_stat == FLASH_READ_RGB_ACT ||  stota->send_response.cmd_stat == FLASH_READ_ALL_ACT  )
    {
        Drv_Uart2_Tx_arrange( &stota->send_response.status, sizeof(stota->send_response.status) );
        Drv_Uart2_Tx_arrange( &stota->send_response.sector_index, sizeof(stota->send_response.sector_index) );
        Drv_Uart2_Tx_arrange( &stota->send_response.data_size, sizeof(stota->send_response.data_size) );

        noffset =stota->send_response.sector_index*4096;

//        dbg_msg_flash("[OTA] RGB res doing!! %d", *stota->send_response.ptr_index );

            //send read data
            //a lot of data
        for(  i=0 ; i< *stota->send_response.ptr_index; i++  )
        {
            //dbg_msg_flash("       [OTA] RGB address 0x:%x ", (UINT32)(stota->send_response.packet_ddr_ptr+ noffset + i ));
            ntemp = *( (UINT8 *)stota->send_response.packet_ddr_ptr+ noffset + i );
            Drv_Uart2_Tx_arrange( (UINT8 *)&ntemp, 1);

            if( (i%255 ==0) && (i>0) )
            {
                delay_ms(1);
            }

            #if  0//( PRINT_FLASH_MECHINE_DEBUG_EN == YES )
            //-----debug use--------
            kdp_printf("0x%X ", ntemp );
            if( i%16 ==0 && i >0 )
            {
                kdp_printf("\r\n");
            }
            #endif
        }
        //send checksum
        Drv_Uart2_Tx_arrange( &stota->send_response.check_sum, sizeof(stota->send_response.check_sum) );

    }
    #endif
    else if( stota->send_response.cmd_stat == FLASH_ERASE_SECTOR_ACT  )
    {
        #if( PRINT_FLASH_MECHINE_DEBUG_EN == YES )
        kdp_printf("erase response 0x7855\r\n");
        #endif
        Drv_Uart2_Tx_arrange( &stota->send_response.status, sizeof(stota->send_response.status) );

    }
    else if( stota->send_response.cmd_stat == FLASH_CMD_ACT )
    {
        #if( PRINT_FLASH_MECHINE_DEBUG_EN == YES )
        kdp_printf("cmd action response 0x7855\r\n");
        #endif
        Drv_Uart2_Tx_arrange( &stota->send_response.status, sizeof(stota->send_response.status) );

    }
    else if( stota->send_response.cmd_stat == FLASH_DATA_TO_DDR_ACT  || stota->send_response.cmd_stat == FLASH_UI_MODEL_INFO )
    {
        Drv_Uart2_Tx_arrange( &stota->send_response.status, sizeof(stota->send_response.status) );
    }
    else if( stota->send_response.cmd_stat == FLASH_MODEL_SIZE )
    {
        Drv_Uart2_Tx_arrange( &stota->send_response.status, sizeof(stota->send_response.status) );
        Drv_Uart2_Tx_arrange( &stota->second_last_start_add, sizeof(stota->second_last_start_add) );
        Drv_Uart2_Tx_arrange( &stota->model_size, sizeof(stota->model_size) );
        Drv_Uart2_Tx_arrange( &stota->send_response.check_sum, sizeof(stota->send_response.check_sum) );
    }


    #if( PRINT_FLASH_MECHINE_DEBUG_EN == YES )
    kdp_printf("Tail 0x7887\r\n");
    #endif
    Drv_Uart2_Tx_arrange( &stota->send_response.tail, sizeof(stota->send_response.tail) );
    return;

}

#else
void Drv_Uart2_Tx_arrange( void * data_ptr , UINT16 len) {}
void Drv_Uart_Tx_send( OTA_FLASHt * stota ) {}
#endif


#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )

void drv_ssp_spi_write_continue( struct st_ssp_spi *stspi, UINT8 *src, UINT16 nlen )
{
    UINT16 i = 0;

    if(  nlen ==1 )
    {
        *( stspi->Tx_buffer + *stspi->Tx_buffer_index ) = *( src  );
        *stspi->Tx_buffer_index = *stspi->Tx_buffer_index + 1;
    }
    else if(  nlen == 2 )
    {
        *( stspi->Tx_buffer + *stspi->Tx_buffer_index ) = *( src + 1 );
        *stspi->Tx_buffer_index = *stspi->Tx_buffer_index + 1;
        *( stspi->Tx_buffer + *stspi->Tx_buffer_index ) = *( src  );
        *stspi->Tx_buffer_index = *stspi->Tx_buffer_index + 1;
    }
    else if(  nlen == 4 )
    {
        *( stspi->Tx_buffer + *stspi->Tx_buffer_index ) = *( src +3);
        *stspi->Tx_buffer_index = *stspi->Tx_buffer_index + 1;

        *( stspi->Tx_buffer + *stspi->Tx_buffer_index ) = *( src + 2);
        *stspi->Tx_buffer_index = *stspi->Tx_buffer_index + 1;

        *( stspi->Tx_buffer + *stspi->Tx_buffer_index ) = *( src + 1);
        *stspi->Tx_buffer_index = *stspi->Tx_buffer_index + 1;

        *( stspi->Tx_buffer + *stspi->Tx_buffer_index ) = *( src + 0);
        *stspi->Tx_buffer_index = *stspi->Tx_buffer_index + 1;
    }
    else
    {
        for( i = 0; i < nlen; i++ )
        {
            *( stspi->Tx_buffer + *stspi->Tx_buffer_index ) = *( src + i );
            *stspi->Tx_buffer_index = *stspi->Tx_buffer_index + 1;
        }

    }


    if( *(volatile UINT32 *)stspi->Tx_buffer_index > stspi->buffer_max_size )
    {
        while(1);		//for protect
    }


}

void Drv_SPI_Slave_Tx( OTA_FLASHt * stota , struct st_ssp_spi *stspi )
{
    UINT32	ndata =0;
    UINT32 nbyte_count =0;
    #if( NIR_RGB_OTA_EN==1 )
    UINT32 noffset = 0;
    #endif

    *stota->send_response.ptr_index = 0;

    /************
     * Need to clc
     * Calculate the second parts of packet total Bytes
     ************/

    //head + host number + tail
    nbyte_count += ( sizeof(stota->send_response.head) + sizeof(stota->send_response.host_number)
                                    + sizeof(stota->send_response.tail) );
    //status
    nbyte_count += sizeof( stota->send_response.status );

#if( NIR_RGB_OTA_EN==1 )
    if( (stota->send_response.status  == FLASH_STATUS_OK ) &&( stota->send_response.cmd_stat == FLASH_READ_ACT || stota->send_response.cmd_stat  == FLASH_READ_NIR_ACT ||
         stota->send_response.cmd_stat  == FLASH_READ_RGB_ACT ) )
#else
    if( stota->send_response.cmd_stat == FLASH_READ_ACT   && stota->send_response.status  == FLASH_STATUS_OK  )
#endif

    {
        //start sector + rr data size + data size field
        nbyte_count += ( sizeof(stota->send_response.sector_index ) + sizeof( stota->send_response.data_size ) + stota->send_response.data_size );
        //checksum
        nbyte_count += ( sizeof(stota->send_response.check_sum )  );
    }
    else if( (stota->send_response.status  == FLASH_STATUS_OK ) && ( stota->send_response.cmd_stat==FLASH_UI_MODEL_INFO))
    {
        //do extra data need to be send
    }
    else if( stota->send_response.status == FLASH_STATUS_OK && stota->send_response.cmd_stat == FLASH_MODEL_SIZE )
    {

        nbyte_count += ( sizeof(stota->second_last_start_add )  );
        nbyte_count += ( sizeof(stota->model_size ) );
        nbyte_count += ( sizeof(stota->send_response.check_sum )  );
    }

    if( stota->send_response.cmd_stat == FLASH_CMD_ACT
            &&  stota->send_response.action_number == FLASH_CMD_ACT_NUM_SW_VERSION
            && stota->send_response.status  == FLASH_STATUS_OK  )
    {
        nbyte_count += (stota->temp_buffer_index*4);

        //checksum
        nbyte_count += ( sizeof(stota->send_response.check_sum )  );
    }


    //1. calculate data size for packet size for SPI protocol

    ndata = FLASH_GET_DATA_HEAD_TX;
    drv_ssp_spi_write_continue( stspi, (UINT8 *)&ndata , sizeof(ndata) );		//part1: head

    ndata = nbyte_count;
    drv_ssp_spi_write_continue( stspi, (UINT8 *)&ndata , sizeof(ndata) );		//part1: Data numbers

    ndata = (nbyte_count&0xFF000000)>>24;
    ndata += (nbyte_count&0x00FF0000)>>16;
    ndata += (nbyte_count&0x0000FF00)>>8;
    ndata += (nbyte_count&0x000000FF)>>0;
    drv_ssp_spi_write_continue( stspi, (UINT8 *)&ndata , sizeof(ndata) );		//part1: checksums

    ndata = FLASH_TAIL  ;
    drv_ssp_spi_write_continue( stspi, (UINT8 *)&ndata , 2 );		//part1: tail



    //part 2

    //head
    drv_ssp_spi_write_continue( stspi, (UINT8 *)&stota->send_response.head, sizeof(stota->send_response.head) );
    drv_ssp_spi_write_continue( stspi, (UINT8 *)&stota->send_response.host_number, sizeof(stota->send_response.host_number) );
    drv_ssp_spi_write_continue( stspi, (UINT8 *)&stota->send_response.status, sizeof(stota->send_response.status) );

    if( stota->send_response.cmd_stat == FLASH_READ_ACT  && stota->send_response.status  == FLASH_STATUS_OK  )
    {
        drv_ssp_spi_write_continue( stspi, (UINT8 *)&stota->send_response.sector_index, sizeof(stota->send_response.sector_index) );
        drv_ssp_spi_write_continue( stspi, (UINT8 *)&stota->send_response.data_size, sizeof(stota->send_response.data_size) );
        //drv_ssp_spi_write_continue( stspi, (UINT8 *)stota->send_response.ptr, stota->send_response.data_size );
        drv_ssp_spi_write_continue( stspi, (UINT8 *)stota->receive_cmd.ptr, stota->receive_cmd.data_size );		//share memory
        drv_ssp_spi_write_continue( stspi, (UINT8 *)&stota->send_response.check_sum, sizeof( stota->send_response.check_sum ) );
    }
    #if( NIR_RGB_OTA_EN==1 )
    else if(  //stota->send_response.cmd_stat  == FLASH_CMD_ACT &&
                        (stota->send_response.cmd_stat == FLASH_READ_NIR_ACT  || stota->send_response.cmd_stat == FLASH_READ_RGB_ACT) )
    {
        drv_ssp_spi_write_continue( stspi, (UINT8 *)&stota->send_response.sector_index, sizeof(stota->send_response.sector_index) );
        drv_ssp_spi_write_continue( stspi, (UINT8 *)&stota->send_response.data_size, sizeof(stota->send_response.data_size) );
        noffset =stota->send_response.sector_index*4096;


        drv_ssp_spi_write_continue( stspi , (UINT8 *)stota->send_response.packet_ddr_ptr + noffset , stota->send_response.data_size );

        drv_ssp_spi_write_continue( stspi, (UINT8 *)&stota->send_response.check_sum, sizeof( stota->send_response.check_sum ) );
    }
    #endif
    else if(  stota->send_response.cmd_stat == FLASH_MODEL_SIZE  && stota->send_response.status == FLASH_STATUS_OK  )
    {
        //offset field
        drv_ssp_spi_write_continue( stspi, (UINT8 *)&stota->second_last_start_add, sizeof(stota->second_last_start_add) );
        //data size field
        drv_ssp_spi_write_continue( stspi, (UINT8 *)&stota->model_size, sizeof(stota->model_size) );
        //checksum
        drv_ssp_spi_write_continue( stspi, (UINT8 *)&stota->send_response.check_sum, sizeof( stota->send_response.check_sum ) );
    }
    else
    {
        //future work
    }

    if( stota->send_response.cmd_stat == FLASH_CMD_ACT
            &&  stota->send_response.action_number == FLASH_CMD_ACT_NUM_SW_VERSION
            && stota->send_response.status  == FLASH_STATUS_OK  )
    {
        drv_ssp_spi_write_continue( stspi, (UINT8 *)stota->temp_buffer, 4 );
        drv_ssp_spi_write_continue( stspi, (UINT8 *)(stota->temp_buffer+1), 4 );
        drv_ssp_spi_write_continue( stspi, (UINT8 *)(stota->temp_buffer+2), 4 );
        drv_ssp_spi_write_continue( stspi, (UINT8 *)(stota->temp_buffer+3), 4 );
        drv_ssp_spi_write_continue( stspi, (UINT8 *)(stota->temp_buffer+4), 4 );
        stota->temp_buffer_index = 0;
    }
    drv_ssp_spi_write_continue( stspi, (UINT8 *)&stota->send_response.tail, sizeof(stota->send_response.tail) );

}

#endif

void Drv_OTA_Get_Version( OTA_FLASHt * stota )
{
    UINT32 nchecksum=0;
    system_info sys_info = { 0 };
    kl520_api_get_device_info( &sys_info );

    stota->send_response.cmd_stat = stota->receive_cmd.cmd_stat;
    stota->send_response.action_number = stota->receive_cmd.action_number;

    stota->temp_buffer_index = 0;


    *( (UINT32 *)(stota->temp_buffer+stota->temp_buffer_index)  ) = sys_info.unique_id;
    stota->temp_buffer_index++;

//	dbg_msg_flash( "Uniqe ID....0x%x", sys_info.unique_id );

    *( (UINT32 *)(stota->temp_buffer+stota->temp_buffer_index) ) = sys_info.spl_version;
    stota->temp_buffer_index++;

//	dbg_msg_flash( "SPL ID....0x%x", sys_info.spl_version );

    *( (UINT32 *)(stota->temp_buffer+stota->temp_buffer_index)  ) = sys_info.fw_scpu_version.date;
    stota->temp_buffer_index++;

//	dbg_msg_flash( "SCPU ID....0x%x", sys_info.fw_scpu_version );


    *( (UINT32 *)(stota->temp_buffer+stota->temp_buffer_index)  ) = sys_info.fw_ncpu_version.date;
    stota->temp_buffer_index++;

    //	dbg_msg_flash( "NCPU ID....0x%x", sys_info.fw_ncpu_version );

    //model
    *( (UINT32 *)(stota->temp_buffer+stota->temp_buffer_index)  ) = 0;
    stota->temp_buffer_index++;

//	dbg_msg_flash( "NCPU ID....0x%x", sys_info.fw_ncpu_version );

    nchecksum = Drv_utility_checksum( (UINT8 *)stota->temp_buffer , 0
                                                                            , stota->temp_buffer_index*sizeof(stota->temp_buffer_index) );

    nchecksum+= ((stota->send_response.status&0xFF) + (stota->send_response.status>>8)&0xFF );
    stota->send_response.check_sum = nchecksum;

    kl520_api_free_device_info(&sys_info);
}

void Drv_OTA_packet_response( OTA_FLASHt * stota, UINT16 status )
{

    UINT8	*ptr = stota->send_response.ptr;
    UINT32	*ptr_indedx = stota->send_response.ptr_index;

    UINT8	ncmd = stota->receive_cmd.cmd_stat;
    UINT32	nchecksum = 0;

    stota->send_response.head = ~( FLASH_HEAD_RX );
    stota->send_response.host_number = stota->receive_cmd.host_number;
    stota->send_response.tail = FLASH_TAIL;

    #if( NIR_RGB_OTA_EN==1 )
    stota->send_response.action_number = stota->receive_cmd.action_number;
    #endif


    //--- error message
    if( status == PACKET_HEAD_CHECK_ERROR || status ==PACKET_TAIL_CHECK_ERROR || status == FLASH_DRV_FAIL )
    {
        stota->send_response.status = FLASH_STATUS_FAIL;
    }
    else if( status >=FLASH_STATUS_COMPARE_FAIL && status <= FLASH_STATUS_CRC_FAIL )
    {
        stota->send_response.status  = status;

        dbg_msg_flash("CRC error 0x%x", status );
    }
    else if( status == FLASH_DRV_PROGRAM_FAIL )
    {
        stota->send_response.status = FLASH_STATUS_PROGRAM_FAIL;		//if program fail

    }
    else
    {
        //--- program or read
        stota->send_response.status = FLASH_STATUS_OK;
        stota->send_response.cmd_stat = ncmd;

        if( ncmd == FLASH_PROGRAM_ACT || ncmd == FLASH_CMD_ACT || ncmd == FLASH_ERASE_SECTOR_ACT || ncmd == FLASH_UI_MODEL_INFO )
        {
            stota->send_response.sector_index = 0;
            stota->send_response.data_size = 0;
            stota->send_response.check_sum = 0;

        }
        if( ncmd == FLASH_MODEL_SIZE )
        {
            nchecksum = ((stota->send_response.status&0xFF) + (stota->send_response.status>>8)&0xFF );
            nchecksum += ((stota->second_last_start_add>>24)&0xFF)+((stota->second_last_start_add>>16)&0xFF)
                            +((stota->second_last_start_add>>8)&0xFF)+((stota->second_last_start_add>>0)&0xFF);
            nchecksum += ((stota->model_size>>24)&0xFF)+((stota->model_size>>16)&0xFF)
                            +((stota->model_size>>8)&0xFF)+((stota->model_size>>0)&0xFF);
            stota->send_response.check_sum = nchecksum;

        }

        if( ncmd == FLASH_READ_ACT )
        {

            stota->send_response.sector_index = stota->receive_cmd.sector_index;
            stota->send_response.data_size = *stota->send_response.ptr_index;


            //dbg_msg_flash("[OTA] Read checksum 1, size: %d",stota->send_response.data_size  );
//			nchecksum = Drv_utility_checksum( ptr , 0, *ptr_indedx );
            ptr = stota->receive_cmd.ptr;
            nchecksum = Drv_utility_checksum( ptr , 0, stota->send_response.data_size );

            //dbg_msg_flash("[OTA] Read checksum 2");

            nchecksum+= ( (stota->send_response.data_size&0xFF)  +  (stota->send_response.data_size>>8)&0xFF );
            nchecksum+= ( (stota->send_response.sector_index&0xFF) + (stota->send_response.sector_index>>8)&0xFF );
            nchecksum+= ((stota->send_response.status&0xFF) + (stota->send_response.status>>8)&0xFF );
            stota->send_response.check_sum = nchecksum;
        }

#if( NIR_RGB_OTA_EN==1 )
        if( ncmd == FLASH_READ_RGB_ACT || ncmd == FLASH_READ_NIR_ACT || ncmd == FLASH_READ_ALL_ACT)
        {
            stota->send_response.sector_index = stota->receive_cmd.sector_index;
            stota->send_response.data_size = *stota->send_response.ptr_index;
            stota->send_response.packet_ddr_ptr = stota->receive_cmd.packet_ddr_ptr;

            if(ncmd == FLASH_READ_RGB_ACT){
                stota->send_response.packet_ddr_ptr = KDP_DDR_TEST_RGB_IMG_ADDR;
            }
            else if(ncmd == FLASH_READ_NIR_ACT){
                stota->send_response.packet_ddr_ptr = KDP_DDR_TEST_NIR_IMG_ADDR;
            }
            else if(ncmd == FLASH_READ_ALL_ACT){
                stota->send_response.packet_ddr_ptr = KDP_DDR_TEST_RGB_IMG_ADDR;    //KDP_DDR_TEST_RGB_IMG_ADDR + KDP_DDR_TEST_NIR_IMG_ADDR + KDP_DDR_TEST_INF_IMG_ADDR
            }
            else{
                stota->send_response.packet_ddr_ptr = KDP_DDR_TEST_RGB_IMG_ADDR;    //Init packet_ddr_ptr
            }

            nchecksum = Drv_utility_checksum( (UINT8 *)stota->send_response.packet_ddr_ptr + stota->send_response.sector_index*4096			//packet_ddr_ptr+dector_index*4096 = real_address
                                                            , 0, *ptr_indedx );

            nchecksum+= ( (stota->send_response.data_size&0xFF)  +  (stota->send_response.data_size>>8)&0xFF );
            nchecksum+= ( (stota->send_response.sector_index&0xFF) + (stota->send_response.sector_index>>8)&0xFF );
            nchecksum+= ((stota->send_response.status&0xFF) + (stota->send_response.status>>8)&0xFF );
            stota->send_response.check_sum = nchecksum;
        }
#endif

        if( ( stota->receive_cmd.cmd_stat == FLASH_CMD_ACT ) && ( stota->receive_cmd.action_number == FLASH_CMD_ACT_NUM_SW_VERSION ) )
        {
            Drv_OTA_Get_Version( stota );
        }

    }

    //-------------- packet data into Uart FIFO ----------------------
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
    Drv_SPI_Slave_Tx( stota , &driver_ssp_ctx );
    kdp_ssp_pre_write_to_fifo( &driver_ssp_ctx, 5 );
#else
    Drv_Uart_Tx_send(stota);
#endif

    return;

}




void Drv_OTA_Set_Flow( OTA_FLASHt * _stOTA, enum eOTA_flow _eOTA  )
{
    _stOTA->flow = _eOTA;
}

#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )


void Drv_OTA_Clear_all_sw_buffer(void)
{
        kdp_ssp_clear_rxhw( driver_ssp_ctx.reg_base_address );
        kdp_ssp_clear_txhw( driver_ssp_ctx.reg_base_address );
        kdp_ssp_clear_rx_buf_index( &driver_ssp_ctx );
        kdp_ssp_clear_tx_buf_index( &driver_ssp_ctx );
        kdp_ssp_clear_tx_current_buf_index(&driver_ssp_ctx);
        kdp_ssp_clear_tx_done_flag(&driver_ssp_ctx);
}
void Drv_OTA_Traffic_disable( void )
{
//	kdp_ssp_clear_rxhw( driver_ssp_ctx.reg_base_address );
//	kdp_ssp_clear_txhw( driver_ssp_ctx.reg_base_address );
//	kdp_ssp_clear_rx_buf_index( &driver_ssp_ctx );
//	kdp_ssp_clear_tx_buf_index( &driver_ssp_ctx );
//	kdp_ssp_clear_tx_current_buf_index(&driver_ssp_ctx);
    kdp_ssp_clear_tx_done_flag(&driver_ssp_ctx);
    kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_disable );
}


void Drv_OTA_Clear_buffer( void )
{
        kdp_ssp_clear_rx_buf_index( &driver_ssp_ctx );
        kdp_ssp_clear_tx_buf_index( &driver_ssp_ctx );
}



void Drv_OTA_Traffic_enable( void )
{
    kdp_ssp_clear_rxhw( driver_ssp_ctx.reg_base_address );
//	kdp_ssp_clear_txhw( driver_ssp_ctx.reg_base_address );
//	kdp_ssp_clear_rx_buf_index( &driver_ssp_ctx );
//	kdp_ssp_clear_tx_buf_index( &driver_ssp_ctx );

    kdp_ssp_clear_tx_done_flag(&driver_ssp_ctx);
    kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_enable );
}


#endif

void Drv_OTA_main( void )
{
    UINT32  countinuous_empty_count = 0;
    UINT32  countinuous_empty_target = 10;

    static UINT16   nstat;
    static UINT16   ptr_rx_count = 0;
    static UINT16 nretry_acc = 0;
    UINT16 nretry_acc_target =  3;

#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
    UINT16 response_TO = 0;
    UINT16 response_TO_target = 100;
#else
    UINT16 pre_count = 0;
#endif

    osThreadFlagsWait( FLAGS_FLASH_START, osFlagsWaitAny, osWaitForever);

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
#if (UART_INIT_AFTER_OTA == YES)
#ifdef COM_BUS_RESPONSE_REQUEST_PIN
    kl520_com_init(KL520_COM_HAS_ADDITIONAL_IO);
#else
    kl520_com_init(KL520_COM_NORMAL);
#endif
#endif
#endif
#endif

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
    kdp_uart_read( stCom_type.uart_port, gu8OTA_Rx_buffer, KDP_DDR_DRV_COM_BUS_RESERVED );                        //read case
#endif

    #if( OTA_LOG_EN == YES )
    dbg_msg_flash("[OTA] flash machine Start-- !!");
    #endif

    kl520_measure_stamp(E_MEASURE_THR_OTA_RDY);
    while(1)
    {
        //dbg_msg_flash("[OTA] Start !!");
        if( *stOTA.receive_cmd.ptr_index == 0  )
        {
            countinuous_empty_count++;
            if( countinuous_empty_count > countinuous_empty_target ){
                countinuous_empty_count = countinuous_empty_target;
                    osDelay( 10 );
            }
            else{
                osDelay( 2 );
            }
        }
        else
        {
            countinuous_empty_count = 0;
        }

        switch( stOTA.flow )
        {
            case eOTA_init:
                Drv_OTA_init( &stOTA );
                stOTA.flow = eOTA_idle;
                break;
            case eOTA_packet_analyze:
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
                //SPI read data check
                if( *stOTA.receive_cmd.ptr_index > 0  )
                {
                    while( kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_rx ) == e_spi_ret_rxbusy )
                    {
                        delay_us(200);
                    }
                    #if( OTA_TIMING_DEBUG_EN == YES )
                    //Rx check done
                    kdp520_gpio_setdata( 1<<27);
                    kdp520_gpio_cleardata( 1<<27);
                    kdp520_gpio_setdata( 1<<27);
                    kdp520_gpio_cleardata( 1<<27);
                    #endif

                    kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_disable );
                    #if ( OTA_LOG_EN == YES )
                    //dbg_msg_flash("[OTA] check slave Rx busy OK");
                    #endif
                }
                else
                {
                    #if ( OTA_LOG_EN == YES )
                    //dbg_msg_flash("[OTA] check slave Rx no data 2");
                    #endif
                    continue;
                }
#else
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
                *stOTA.receive_cmd.ptr_index = kdp_uart_GetRxCount( stCom_type.dev_id );
#endif
                if(  *stOTA.receive_cmd.ptr_index ==  0  )
                {
                    break;//    stOTA.flow = eOTA_idle; //sleep or others!!
                }
                else        //add
                {
                    if(  pre_count != *stOTA.receive_cmd.ptr_index )
                    {
                        pre_count = *stOTA.receive_cmd.ptr_index;
                        delay_ms(1);
                        continue;
                    }
                }
#endif

                if(  ( nstat = drv_ota_packet_analyze( &stOTA ) ) == FLASH_PACKET_OK )
                {
                    #if( OTA_LOG_EN == YES )
                    dbg_msg_flash("[OTA] Packet head and tail check OK");
                    #endif

                    nstat = drv_flash_main( &stOTA );
                    stOTA.receive_cmd.busy_flag = 0;
                    stOTA.flow = eOTA_response_doing;
                }
                else
                {
                    stOTA.receive_cmd.busy_flag = 0;
                    if( nstat ==  PACKET_HEAD_CHECK_ERROR )
                    {
                      #if( OTA_LOG_EN == YES )
                        dbg_msg_flash("[OTA] Packet head error");
                        #endif
                        //goto report error
                        stOTA.flow = eOTA_response_doing;
                    }
                    if( nstat ==  PACKET_TAIL_CHECK_ERROR  )
                    {
                        if( ptr_rx_count > 0  &&  ptr_rx_count == *stOTA.receive_cmd.ptr_index )
                        {
                            nretry_acc ++;
                            delay_ms(1);
                            if( nretry_acc >= nretry_acc_target )
                            {
                                #if( OTA_LOG_EN == YES )
                                dbg_msg_flash("[OTA] Packet tail error timeout, ignore this packet! ");
                                #endif
                                nretry_acc = 0;
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
                                //stOTA.flow = eOTA_idle;
                                stOTA.flow = eOTA_response_doing;
#else
                                stOTA.flow = eOTA_response_doing;
#endif
                            }
                            else
                            {
                                //stOTA.flow = eOTA_packet_analyze;		//any good case
                            }
                        }
                        ptr_rx_count = *stOTA.receive_cmd.ptr_index;
                    }
                }
                break;
            case	eOTA_response_doing:
              #if( OTA_LOG_EN == YES )
                dbg_msg_flash("[OTA] Packet response doing");
                #endif

#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
                kdp_ssp_clear_rxhw( driver_ssp_ctx.reg_base_address );
                kdp_ssp_clear_txhw( driver_ssp_ctx.reg_base_address );
#endif

                Drv_OTA_packet_response( &stOTA, nstat ) ;

#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )

                Drv_OTA_Traffic_enable();

#ifdef COM_BUS_RESPONSE_REQUEST_PIN
                if (stCom_type.flags == KL520_COM_HAS_ADDITIONAL_IO)
                {
                    kdp_slave_request_active();
                }
#endif

                while( kdp_ssp_get_tx_done_flag( &driver_ssp_ctx ) == 0 )
                {
                    delay_ms(30);
                    response_TO ++;
                    if(response_TO > response_TO_target)
                    {
                        response_TO = 0;
                        #if( OTA_LOG_EN == YES )
                        dbg_msg_flash("[OTA] Packet SPI Rx TimeOut");
                        #endif
                        break;
                    }
                }
                response_TO = 0;
                #if( OTA_LOG_EN == YES )
                dbg_msg_flash("[OTA] Packet response Pre-done");
                #endif
                while( kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_rx ) == e_spi_ret_rxbusy )
                {
                        delay_us(30);
                }

                Drv_OTA_Clear_all_sw_buffer();
#endif

#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
#if( OTA_LOG_EN == YES )
                dbg_msg_flash("[OTA] Packet response done");
#ifdef COM_BUS_RESPONSE_REQUEST_PIN
                if (stCom_type.flags == KL520_COM_HAS_ADDITIONAL_IO)
                {
                    kdp_slave_request_inactive();
                }
#endif
#endif
#endif

                stOTA.flow = eOTA_idle;
                //break;

            case eOTA_idle:
                nretry_acc = 0;

#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )

#elif ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
                kdp_uart_read( stCom_type.uart_port, gu8OTA_Rx_buffer, KDP_DDR_DRV_COM_BUS_RESERVED );                        //read case
#endif
                *stOTA.receive_cmd.ptr_index = 0 ;
                *stOTA.send_response.ptr_index = 0;
                stOTA.flow = eOTA_packet_analyze;
                #if( OTA_TIMING_DEBUG_EN == YES )
                dbg_msg_flash("[OTA] wait!!");
                #endif

                #if( OTA_TIMING_DEBUG_EN == YES )
                kdp520_gpio_setdata( 1<<25 );
                kdp520_gpio_cleardata( 1<<25 );
                kdp520_gpio_setdata( 1<<25 );
                kdp520_gpio_cleardata( 1<<25 );
                #endif
                break;
            case eOTA_error_report:
                stOTA.flow = eOTA_idle;
                break;
            default:
                break;
        }
    }
}


extern OTA_FLASHt	stOTA;


void Drv_OTA_Thread( void )
{
    #if ( OTA_TIMING_DEBUG_EN == YES)
    _flash_gpio_debug();
    #endif

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
#ifdef COM_BUS_RESPONSE_REQUEST_PIN
    kl520_com_init(KL520_COM_HAS_ADDITIONAL_IO);
#else
    kl520_com_init(KL520_COM_NORMAL);
#endif
#endif
#endif

    #if (CFG_OTA_EN == YES)
    osThreadAttr_t attr = {
        .stack_size = 512
    };
    stOTA.flow = eOTA_init;
    ota_tid = osThreadNew( (osThreadFunc_t)Drv_OTA_main, NULL, &attr );
    #endif

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
    osThreadAttr_t attr_com = {
        .stack_size = 1792
    };
    com_bus_tid = osThreadNew( (osThreadFunc_t)kl520_com_thread, NULL, &attr_com );
#endif
#endif
}

void ota_thread_event_set(void)
{
    set_thread_event( ota_tid , FLAGS_FLASH_START);
}
#endif
#endif
