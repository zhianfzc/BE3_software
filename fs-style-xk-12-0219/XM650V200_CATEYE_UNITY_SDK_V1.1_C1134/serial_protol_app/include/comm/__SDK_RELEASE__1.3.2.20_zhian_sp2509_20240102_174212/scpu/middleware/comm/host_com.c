/*
 * Kneron Host Communication driver
 *
 * Copyright (C) 2018 Kneron, Inc. All rights reserved.
 *
 */
#include "host_com.h"

#include <stdint.h>
#include <errno.h>
#include "os_tick.h"
#include "kdp_memxfer.h"
#include "ota_update.h"
#include "com.h"
#include "Com_err.h"
#include "kneron_mozart.h"
#include "kdp_app.h"
#include "kdp_app_fid.h"
#include "kdp_app_db.h"
#include "kdp_app_dme.h"
#include "kdp_app_age_gender.h"
#include "host_usb_com.h"
#include "kdp_memory.h"
#include "kdp520_usbd_status.h"
#include "framework/event.h"
#include "kdp_ddr_table.h"
#include "ipc.h"
//#include "kdrv_clock.h" // for delay_us
#include "power_manager.h"
#include "kl520_api.h"
#include "kl520_api_fdfr.h"
#include "kl520_api_snapshot.h"
#include "kl520_api_extra_fmap.h"
#include "kl520_api_device_id.h"
#include "kl520_api_sim.h"
#include "kl520_api_camera.h"
#include "io.h"
#include "power.h"
#include "flash.h"
#include "ota.h"

#include "sample_app_console.h"

#if ( CFG_GUI_ENABLE == YES )
#include "sample_user_com_and_gui_fdr.h"
#include "sample_app_console.h"
#include "gui_fsm.h"
#include "sample_gui_fsm_events.h"
#endif

#ifdef USE_KDRV
extern void kdrv_mpu_niram_enable(void);
extern void kdrv_mpu_niram_disable(void);
#endif

// error code for cmd parser
#define NO_ERROR        0
//#define NO_MATCH        1
//#define RSP_NOT_IMPLEMENTED 0xFFFE
#define RSP_UKNOWN_CMD  0xFFFF
#define BAD_CONFIRMATION 2  // CMD_RESET RESPONSE
#define BAD_MODE         1
#define FILE_ERROR       1  // File data transfer error

//#define PRODUCT_TEST_FDR_THRD

extern int32_t kcomm_read(u32 address, u32 image_size);
osThreadId_t tid_host_comm;

#if CFG_SNAPSHOT_ENABLE == 1
extern osMutexId_t snap_sts_mtx;
extern osEventFlagsId_t snapshot_evt;
extern u8  snapshot_status[SNAPSHOT_SRC_NUM];
#endif

#ifndef CUSTOMER_SETTING_REMOVE_CMD
static u16 host_com_app_id = 0;  // 1 -> 3D (LW3D), 2 -> 2D, 3 -> DME
static u16 host_com_sys_sts = 0; // system status
#endif

//enum APP_ID {
//    APP_UNKNOWN = 0,
//    APP_LW3D,
//    APP_SFID,
//    APP_DME,
//    APP_ID_MAX,
//};

#if CFG_USB_PROGRAME_FW_ENABLE == YES
u32 cmd_crc_check( u32 crc, u8* buf, u32 nlen )
{
    u32 i;
    for( i = 0; i < nlen; i++ )
    {
        crc += *(buf+i);
    }
    return crc;
}

u32 kl520_api_com_flash_program( u32 status, u32 size, u32 program_addr, u32 *crc )
{
    u32 ret = FILE_ERROR;

    if( kl520_api_fdfr_exist_thread() )
    {
        return ret;
    }

    if ( status != 0xA55A && status != 0x5AA5 )
    {
        dbg_msg("[flash program] command fail: %#08x", status);
        return ret;
    }
    
    kdp_flash_set_protect_bypass(1);
    kdp_flash_erase_chip();

    if( kdp_memxfer_ddr_to_flash_wo_erase( program_addr, 0x60000000, size) == 1 )
    //if( kdp_memxfer_ddr_to_flash( program_addr, 0x60000000, size) == 1 )
    {
        ret = NO_ERROR;
        dbg_msg("[flash program] program ok");
    }
    else
    {
        dbg_msg("[flash program] program fail");
    }

    kdp_flash_set_protect_bypass(0);
    
    if( ret == NO_ERROR && status == 0x5AA5 )
    {
        #define VERIFY_SECTION_SIZE 4096
        u32 verify_addr = 0x60000000 + size;
        int major_idx = size >> 12;
        int minor_idx = size & 0x0FFF;
        int idx;

        dbg_msg("[flash program] verify strat");

        *crc = 0;
        for( idx = 0; idx < major_idx; idx++ )
        {
            if ( kdp_memxfer_flash_to_ddr( verify_addr, program_addr+idx*VERIFY_SECTION_SIZE, VERIFY_SECTION_SIZE ) == 0)
            {
                *crc = cmd_crc_check( *crc, (u8*)verify_addr, VERIFY_SECTION_SIZE );

                if( memcmp( (void*)verify_addr, (void*)(0x60000000+VERIFY_SECTION_SIZE*idx), VERIFY_SECTION_SIZE ) )
                {
                    ret = FILE_ERROR;
                    dbg_msg("[flash program] verify fail, fail section: %d", idx);
                    return ret;
                }
            }
            else
            {
                dbg_msg("[flash program] read fail");
                ret = FILE_ERROR;
                return ret;
            }

        }
        if( minor_idx > 0 && idx == major_idx )
        {
            if ( kdp_memxfer_flash_to_ddr( verify_addr, major_idx*VERIFY_SECTION_SIZE, minor_idx ) == 0)
            {
                *crc = cmd_crc_check( *crc, (u8*)verify_addr, VERIFY_SECTION_SIZE );
                if( memcmp( (void*)verify_addr, (void*)(0x60000000+VERIFY_SECTION_SIZE*major_idx), minor_idx ) )
                {
                    ret = FILE_ERROR;
                    dbg_msg("[flash program] verify fail, fail section: %d", major_idx);
                }
            }
            else
            {
                dbg_msg("[flash program] read fail");
                ret = FILE_ERROR;
            }
        }
    }

    dbg_msg("[flash program]crc:%#x",crc);

    return ret;

}

#endif

#if CFG_SNAPSHOT_ENABLE == 1

extern u32 snap_shot_buffer_addr;
static u8* wait_snap_shot_buffer(void)
{
    u8* pwdata = (void*)KDP_DDR_TEST_RGB_IMG_ADDR;
    u32 buf_data = snap_shot_buffer_addr;
    uint32_t flags = 0;
    bool waited = FALSE;

    do {
        osMutexAcquire(snap_sts_mtx, osWaitForever); //protect
        //check buffer ready
        if(snapshot_status[MIPI_CAM_NIR] == 1 && snapshot_status[MIPI_CAM_RGB] == 1 
            && snapshot_status[MIPI_CAM_INF] == 1) {
            //dbg_msg_algo ("img ready, setting flag...");
            snapshot_status[MIPI_CAM_RGB] = 88; //transferring flag
            osMutexRelease(snap_sts_mtx);

            //copy buf to here
            int tot_size = KDP_DDR_TEST_NIR_IMG_SIZE + KDP_DDR_TEST_RGB_IMG_SIZE + KDP_DDR_TEST_INF_IMG_SIZE;
            kl520_api_snapshot_dma(KDP_DDR_TEST_RGB_IMG_ADDR, buf_data, tot_size);
            return pwdata; //ready
        }

        osMutexRelease(snap_sts_mtx);

        if(waited) { //already waited once
            break;
        }

        //wait once
        //dbg_msg_algo ("waiting for buffer img ready....");
        //flags = osEventFlagsWait(snapshot_evt, SNAP_SHOT_EVENT_READY, osFlagsWaitAny, osWaitForever);
        flags = osEventFlagsWait(snapshot_evt, SNAP_SHOT_EVENT_READY, osFlagsWaitAny, 5000);
        if(flags != SNAP_SHOT_EVENT_READY) { //wait error
            break;
        }
        waited = TRUE;

    } while(1);

    return NULL;
}

static void snapshot_img_transfer_complt(void)
{
    //static int cnt;
    osMutexAcquire(snap_sts_mtx, osWaitForever); //protect
    
    osEventFlagsClear(snapshot_evt, SNAP_SHOT_EVENT_READY);

    snapshot_status[MIPI_CAM_RGB] = 0;
    snapshot_status[MIPI_CAM_NIR] = 0;
    snapshot_status[MIPI_CAM_INF] = 0;
    
    osMutexRelease(snap_sts_mtx);
    //dbg_msg_algo ("img transfer complted:%d.", ++cnt);
    return;
}

#endif

static int handle_host_com_general(u8 *buf, int len)
{
    u8 rstatus = 0;
    MsgHdr* msghdr = (MsgHdr*)buf;
    RspPram* msgrsp_arg = (RspPram*)(buf + sizeof(MsgHdr));  // use the same rcv buffer to format responses

#if CFG_SNAPSHOT_ENABLE == 2 || CFG_SNAPSHOT_ENABLE == 1
    CmdPram* msgcmd_arg = (CmdPram*)(buf + sizeof(MsgHdr));
    OpPram* opcmdpram = (OpPram*)msgcmd_arg;  // used to parse operational commands
    u32* pwdata = (u32*)(buf + (sizeof(MsgHdr) + sizeof(CmdPram)));
    dbg_msg("[Host_com] cmd_parser[%d]: cmd %d [addr, len]=[0x%x, %d]\n", len, msghdr->cmd, msgcmd_arg->param1, msgcmd_arg->param2);
#endif

    switch (msghdr->cmd) {

#if CFG_SNAPSHOT_ENABLE == 2
        case CMD_SNAPSHOT:
        {
            u32 srt = osKernelGetTickCount();
            dbg_msg("[Host_com] get snapshot cmd!");
            if(opcmdpram->op_parm1 == 0x5A)
            {
                while(kl520_api_snapshot_fdfr_catch(MIPI_CAM_RGB)==0)
                {osDelay(10);}
                while(kl520_api_snapshot_addr(MIPI_CAM_RGB)==0)
                {osDelay(10);}
                pwdata=(void*)KDP_DDR_TEST_RGB_IMG_ADDR;
                dbg_msg("--> RGB %d ",msgcmd_arg->param2);
            }
            else if(opcmdpram->op_parm1 == 0xA5)
            {
                while(kl520_api_snapshot_fdfr_catch(MIPI_CAM_NIR)==0)
                {osDelay(10);}
                while(kl520_api_snapshot_addr(MIPI_CAM_NIR)==0)
                {osDelay(10);}
                pwdata=(void*)KDP_DDR_TEST_NIR_IMG_ADDR;
                dbg_msg("--> NIR%d ",msgcmd_arg->param2);
            }
            else if(opcmdpram->op_parm1 == 0xAA)
            {
                if(snapshot_adv_mode == TRUE)
                {
                    if(g_SnapUsbBufCnt > 0)
                    {
                        kl520_api_snapshot_adv_load(MIPI_CAM_NIR, g_SnapReadCnt);
                        kl520_api_snapshot_adv_load(MIPI_CAM_RGB, g_SnapReadCnt);
                        kl520_api_snapshot_adv_load(MIPI_CAM_INF, g_SnapReadCnt);

                        if(g_SnapUsbBufCnt>0)
                            g_SnapUsbBufCnt--;

                        dbg_msg_com("[Snapshot]g_SnapReadCnt :%d, g_SnapUsbBufCnt:%d",g_SnapReadCnt, g_SnapUsbBufCnt);

                        g_SnapReadCnt++;
                        g_SnapReadCnt = g_SnapReadCnt%SNAPSHOT_ADV_NUM;

                        if(g_SnapUsbBufCnt == 0)
                            dbg_msg_com("[Snapshot]Buffer NULL");
                    }
                }
                else
                {
                    while(kl520_api_snapshot_fdfr_catch(MIPI_CAM_RGB)==0)
                    {osDelay(10);    }
                    while(kl520_api_snapshot_addr(MIPI_CAM_RGB)==0)
                    {osDelay(10);}
                }
                kl520_api_ap_com_snapshot_offset_merge();
                
                pwdata=(void*)KDP_DDR_TEST_RGB_IMG_ADDR;
                dbg_msg("--> NIR+RGB+INFO%d ",msgcmd_arg->param2);
            }
            else
            {
                //dbg_msg("--> unknow");
            }


            // msg byte read size is unchanged
            msgrsp_arg->error = NO_ERROR;

            kcomm_write((u8 *) pwdata, msgcmd_arg->param2, WMODE_DEF);
            dbg_msg_com("[snapshot]usb time:%05d (ms)",osKernelGetTickCount() - srt);


            break;
        }
        case CMD_SNAPSHOT_CHECK: // check avaiable snapshot count
        {
            msgrsp_arg->error = NO_ERROR;   // use "error" parameter for device ID
            #if CFG_FMAP_EX_FIG_ENABLE == YES && CFG_FMAP_EXTRA_ENABLE == YES && CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_AP_CTRL_TYPE < EX_FM_UART_AP_CTRL_MAIN_DB
            msgrsp_arg->param2 = (u32)g_SnapUsbBufCnt | (KL520_api_ap_com_get_extra_fmap_status() << 16);  // and "len" parameter for firmware version
            #else
            msgrsp_arg->param2 = (u32)g_SnapUsbBufCnt;  // and "len" parameter for firmware version
            #endif
            kcomm_send_rsp(CMD_SNAPSHOT_CHECK, (u8*)msgrsp_arg->param2, 0);
            break;
        }
#endif

#if CFG_SNAPSHOT_ENABLE == 1
        case CMD_SNAPSHOT: // check avaiable snapshot count
        {
            pwdata = (void*)wait_snap_shot_buffer();

            if (pwdata == NULL) {
                kcomm_send_rsp(CMD_SNAPSHOT, NULL, 0);
            } else {
                //dbg_msg_algo ("img ready, sending via usb....");
                kcomm_write((u8 *) pwdata, msgcmd_arg->param2, WMODE_DEF);
                snapshot_img_transfer_complt();
            }

            break;
        }
#endif

#if CFG_USB_SIMTOOL == 1
        case CMD_RECEIVE_IMAGE:
        {
            //dbg_msg("Received cmd: CMD_DME_SEND_IMAGE %d\n",ret);
            //Msg for the length of raw results

            int ret = NO_ERROR; 
            if(msgrsp_arg->param1 == 0x1111)
            {
                ret = kcomm_read(KDP_DDR_TEST_RGB_IMG_ADDR, msgcmd_arg->param2);
                dbg_msg_console("tgt %x,len=%d\n",KDP_DDR_TEST_RGB_IMG_ADDR,msgcmd_arg->param2);
            }
            else if(msgrsp_arg->param1 == 0x2222)
            {
                ret = kcomm_read(KDP_DDR_TEST_NIR_IMG_ADDR, msgcmd_arg->param2);
            }
            else if(msgrsp_arg->param1 == 0x3333)
            {
                ret = kcomm_read(KDP_DDR_TEST_INF_IMG_ADDR, msgcmd_arg->param2);
            }
            else if(msgrsp_arg->param1 == 0x4444)
            {
                u8 OnOff = (msgrsp_arg->param2 >> 30) & 0x3;
                u16 img_width = (msgrsp_arg->param2 >> 15) & 0x7FFF;
                u16 img_height = (msgrsp_arg->param2 >> 0) & 0x7FFF;
#if CFG_CAMERA_DUAL_1054 == 1 || CFG_CAMERA_SINGLE_1054 == 1
                if( OnOff == 1 ) kdp_e2e_config_image(KDP_E2E_IMG_SRC_RGB, img_width, img_height, 1, KDP_E2E_IMAGE_FORMAT_RAW8, 0, 0);
                else kdp_e2e_config_image(KDP_E2E_IMG_SRC_RGB, RGB_IMG_SOURCE_W, RGB_IMG_SOURCE_H, 1, KDP_E2E_IMAGE_FORMAT_RAW8, 0, 0);
#else
                if( OnOff == 1 ) kdp_e2e_config_image(KDP_E2E_IMG_SRC_RGB, img_width, img_height, 3, KDP_E2E_IMAGE_FORMAT_RGB565, 0, 0);
                else kdp_e2e_config_image(KDP_E2E_IMG_SRC_RGB, RGB_IMG_SOURCE_W, RGB_IMG_SOURCE_H, 3, KDP_E2E_IMAGE_FORMAT_RGB565, 0, 0);
#endif
            }
            else if(msgrsp_arg->param1 == 0x5555)
            {
                u8 OnOff = (msgrsp_arg->param2 >> 30) & 0x3;
                u16 img_width = (msgrsp_arg->param2 >> 15) & 0x7FFF;
                u16 img_height = (msgrsp_arg->param2 >> 0) & 0x7FFF;
                if( OnOff == 1 ) kdp_e2e_config_image(KDP_E2E_IMG_SRC_NIR, img_width, img_height, 1, KDP_E2E_IMAGE_FORMAT_RAW8, 0, 1);
                else kdp_e2e_config_image(KDP_E2E_IMG_SRC_NIR, NIR_IMG_SOURCE_W, NIR_IMG_SOURCE_H, 1, KDP_E2E_IMAGE_FORMAT_RAW8, 0, 1);
            }           
            else if(msgrsp_arg->param1 == 0x6666)
            {
                memset((void*)KDP_DDR_TEST_IMG_NAME_ADDR, 0, KDP_DDR_TEST_IMG_NAME_SIZE);
                ret = kcomm_read(KDP_DDR_TEST_IMG_NAME_ADDR, msgcmd_arg->param2);
            }
            else if(msgrsp_arg->param1 == 0xFFFF)
            {        
                struct kdp_img_cfg *img_cfg = kdp_e2e_get_img_cfg(KDP_E2E_IMG_SRC_RGB);
                msgrsp_arg->param1 = ((( kdp_app_get_pixel_size(img_cfg->image_format) & 0x3 ) << 30) | (( img_cfg->image_col & 0x7FFF ) << 15) | (( img_cfg->image_row & 0x7FFF ) << 0 ) );
                
                img_cfg = kdp_e2e_get_img_cfg(KDP_E2E_IMG_SRC_NIR);
                msgrsp_arg->param2 =  ((( kdp_app_get_pixel_size(img_cfg->image_format) & 0x3 ) << 30) | (( img_cfg->image_col & 0x7FFF ) << 15) | (( img_cfg->image_row & 0x7FFF ) << 0 ) );

                #if CFG_AI_TYPE == AI_TYPE_N1
                msgrsp_arg->param1 = msgrsp_arg->param2;
                #elif CFG_AI_TYPE == AI_TYPE_R1 
                msgrsp_arg->param2 = msgrsp_arg->param1;
                #endif
            }

            if(ret == NO_ERROR) msgrsp_arg->error = NO_ERROR;
            else msgrsp_arg->error = PARAM_ERR;

            kcomm_send_rsp(CMD_RECEIVE_IMAGE, (u8*)msgrsp_arg->error, 0);
            break;
        }
        case CMD_SIM_START:
        case CMD_E2E_FACE_RECOGNITION_TEST:
        {
            dbg_msg_console("\n Simulation start: %s", (char*)KDP_DDR_TEST_IMG_NAME_ADDR);
            kl520_sim_ctx ctx = {0};
            kl520_api_sim_fdfr(&ctx);
            msgrsp_arg->error = NO_ERROR;   // use "error" parameter for device ID
            msgrsp_arg->param2 = ctx.result;//(u32)0x55AA;  // and "len" parameter for firmware version
            u8 *buf = (u8 *)KDP_DDR_TEST_INF_IMG_ADDR;
            int size = KDP_DDR_TEST_INF_IMG_SIZE;
            kcomm_send_rsp(CMD_SIM_START, buf, size);
            dbg_msg_console("\n Simulation end: %s", (char*)KDP_DDR_TEST_IMG_NAME_ADDR);
            break;
        }
        case CMD_E2E_FACE_ADD:
        {
            dbg_msg_console("\n Simulation start: %s", (char*)KDP_DDR_TEST_IMG_NAME_ADDR);
#if CFG_SNAPSHOT_ENABLE == 2
            kl520_api_snapshot_fdfr_catch(KAPP_IMG_SRC_NIR);
#endif

            kl520_sim_ctx ctx = {0};
            kl520_api_sim_face_add(&ctx, msgrsp_arg->param1, msgrsp_arg->param2);
            msgrsp_arg->error = NO_ERROR;   // use "error" parameter for device ID
            msgrsp_arg->param2 = ctx.result;//(u32)0x55AA;  // and "len" parameter for firmware version

            u8 *buf = (u8 *)KDP_DDR_TEST_INF_IMG_ADDR;
            int size = KDP_DDR_TEST_INF_IMG_SIZE;
            kcomm_send_rsp(CMD_E2E_FACE_ADD, buf, size);
            dbg_msg_console("\n Simulation end: %s", (char*)KDP_DDR_TEST_IMG_NAME_ADDR);
            break;
        }
        case CMD_E2E_FACE_PRE_ADD:
        {
            kl520_sim_ctx ctx = {0};
            kl520_api_sim_face_pre_add(&ctx, msgrsp_arg->param1);
            msgrsp_arg->error = NO_ERROR;   // use "error" parameter for device ID
            msgrsp_arg->param2 = ctx.result;//(u32)0x55AA;  // and "len" parameter for firmware version
            kcomm_send_rsp(CMD_E2E_FACE_PRE_ADD, (u8*)msgrsp_arg->param2, 0);
            break;
        }
        case CMD_E2E_FACE_RECOGNITION:
        {
            dbg_msg_console("\n Simulation start: %s", (char*)KDP_DDR_TEST_IMG_NAME_ADDR);
#if CFG_SNAPSHOT_ENABLE == 2
            kl520_api_snapshot_fdfr_catch(KAPP_IMG_SRC_NIR);
#endif

            kl520_sim_ctx ctx = {0};
            kl520_api_sim_face_recognition(&ctx);
            msgrsp_arg->error = NO_ERROR;   // use "error" parameter for device ID
            msgrsp_arg->param2 = ctx.result;//(u32)0x55AA;  // and "len" parameter for firmware version

            u8 *buf = (u8 *)KDP_DDR_TEST_INF_IMG_ADDR;
            int size = KDP_DDR_TEST_INF_IMG_SIZE;
            kcomm_send_rsp(CMD_E2E_FACE_RECOGNITION, buf, size);
            dbg_msg_console("\n Simulation end: %s", (char*)KDP_DDR_TEST_IMG_NAME_ADDR);
            break;
        }
#endif

#ifdef PRODUCT_TEST_FDR_THRD
        case CMD_OPEN_FDR_THREAD:
        {
            dbg_msg_console("CMD_OPEN_FDR_THREAD");
            kl520_api_fdfr_init_thrd();
            if(msgrsp_arg->param1 == 0x1)
            {
                u8 adv_mode = (u8)msgrsp_arg->param2;
                sample_snapshot_auto_usb_mode(adv_mode);
            }

            s32 ret = 0;
            system_info t_sys_info = { 0 };
            ret = kl520_api_face_recognition_test(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
            if(ret !=0)
            {
                ret = kl520_api_get_device_info(&t_sys_info);
                dbg_msg_err("sample_face_recognition_test device error = %x", ret);
                kl520_api_free_device_info(&t_sys_info);
            }
            
            msgrsp_arg->error = NO_ERROR;   // use "error" parameter for device ID
            msgrsp_arg->param2 = (u32)ret;
            kcomm_send_rsp(CMD_OPEN_FDR_THREAD, (u8*)msgrsp_arg->param2, 0);
            break;
        }
        case CMD_CLOSE_FDR_THREAD:
        {
            kl520_measure_stamp(E_MEASURE_FACE_CLOSE_STR);
            kl520_api_face_close();
            kl520_measure_stamp(E_MEASURE_FACE_CLOSE_END);

            #if CFG_FMAP_EXTRA_ENABLE == YES
            kl520_api_extra_fmap_close();
            #endif

            kl520_api_sim_set_rst();

            sample_snapshot_auto_usb_mode(0);

            msgrsp_arg->error = NO_ERROR;   // use "error" parameter for device ID
            msgrsp_arg->param2 = 0;
            kcomm_send_rsp(CMD_CLOSE_FDR_THREAD, (u8*)msgrsp_arg->param2, 0);
            break;
        }
#endif

#if CFG_COMPARE_1VS1 == YES
        case CMD_E2E_FACE_COMAPRE_1VS1:
        {
            kl520_sim_ctx ctx = {0};
            ctx.result = opcmdpram->op_parm1;
            kl520_api_sim_2user_comapre(&ctx);
            msgrsp_arg->error = NO_ERROR;   // use "error" parameter for device ID
            msgrsp_arg->param2 = ctx.result;//(u32)0x55AA;  // and "len" parameter for firmware version
            kcomm_send_rsp(CMD_E2E_FACE_COMAPRE_1VS1, (u8*)msgrsp_arg->param2, 0);
            break;
        }
#endif

#ifndef CUSTOMER_SETTING_REMOVE_CMD
#if CFG_FMAP_EXTRA_ENABLE == YES
        case CMD_EXTRA_MAP:
        {
            u32 srt = osKernelGetTickCount();
            dbg_msg("[Host_com] get fmap cmd!");

            if(opcmdpram->op_parm1 == 0x5A )
            {
                while(kl520_api_extra_db_map_addr()==0)
                {osDelay(10);}
                pwdata=(void*)KDP_DDR_TEST_EXTRA_DB_ADDR;
                dbg_msg("--> DB %d ",msgcmd_arg->param2);
            }
            else if(opcmdpram->op_parm1 == 0xA5)
            {
                kl520_api_extra_fmap_fdfr_reset();
                while(kl520_api_extra_fmap_fdfr_catch(MIPI_CAM_RGB)==0)
                {osDelay(10);    }
                while(kl520_api_extra_fmap_addr()==0)
                {osDelay(10);}

                pwdata=(void*)KDP_DDR_TEST_EXTRA_RGB_ADDR;
                dbg_msg("--> FR %d ",msgcmd_arg->param2);
            }
            else if(opcmdpram->op_parm1 == 0xAA)
            {
                if(g_FmapUsbBufCnt > 0)
                {
                    kl520_api_extra_fmap_adv_load(MIPI_CAM_NIR, g_FmapReadCnt);
                    kl520_api_extra_fmap_adv_load(MIPI_CAM_RGB, g_FmapReadCnt);

                    if(g_FmapUsbBufCnt>0)
                        g_FmapUsbBufCnt--;

                    dbg_msg_com("[fmap]g_SnapReadCnt :%d, g_SnapUsbBufCnt:%d",g_FmapReadCnt, g_FmapUsbBufCnt);

                    g_FmapReadCnt++;
                    g_FmapReadCnt = g_FmapReadCnt % EXTRA_FMAP_ADV_NUM;

                    if(g_FmapUsbBufCnt == 0)
                        dbg_msg_com("[fmap]Buffer NULL");
                }
                dbg_msg("--> FR LOOP %d ",msgcmd_arg->param2);
                pwdata=(void*)KDP_DDR_TEST_EXTRA_RGB_ADDR;
            }
			#if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_AP_CTRL_TYPE < EX_FM_UART_AP_CTRL_MAIN_DB && CFG_FMAP_EXTRA_ENABLE == YES
            else if( kdp_api_ap_com_check_host_command_type( opcmdpram->op_parm1 ) )
			{
				dbg_msg_com("[fmap] command: 0x%x, ret: %d", opcmdpram->op_parm1, (opcmdpram->op_parm2 & 0x00FF) );
				KL520_api_ap_com_set_extra_fmap_status( USB_CMD_FM_NULL_e );
				kdp_api_ap_com_set_user_id( (u8)(opcmdpram->op_parm2 & 0x00FF) );
				kdp_api_ap_com_set_host_result(1);
			}
			#endif
            else
            {
                //dbg_msg("--> unknow");
            }

            // msg byte read size is unchanged
			#if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_AP_CTRL_TYPE < EX_FM_UART_AP_CTRL_MAIN_DB && CFG_FMAP_EXTRA_ENABLE == YES
			if( kdp_api_ap_com_check_host_command_type( opcmdpram->op_parm1 ) )
			{
				dbg_msg_com("[rsp] info :%d", (opcmdpram->op_parm2 & 0x00FF) );
				kcomm_send_rsp(CMD_EXTRA_MAP, (u8*)opcmdpram->op_parm2, 0);
			}
			else
			#endif
            {
       			msgrsp_arg->error = NO_ERROR;
            	kcomm_write((u8 *) pwdata, msgcmd_arg->param2, WMODE_DEF);
			}
            dbg_msg_com("[fmap]usb time:%05d (ms)",osKernelGetTickCount() - srt);
            break;
        }

		case CMD_EXTRA_MAP_CHECK: // check avaiable snapshot count
		{
			msgrsp_arg->error = NO_ERROR;	// use "error" parameter for device ID
			#if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_AP_CTRL_TYPE < EX_FM_UART_AP_CTRL_MAIN_DB && CFG_FMAP_EXTRA_ENABLE == YES
			msgrsp_arg->param2 = (u32)( g_FmapUsbBufCnt | (KL520_api_ap_com_get_extra_fmap_status() << 16) | ((u8)kdp_api_ap_com_get_usrt_id() << 24) );	// and "len" parameter for firmware version
			#else
			msgrsp_arg->param2 = (u32)( g_FmapUsbBufCnt );	// and "len" parameter for firmware version
			#endif
			//dbg_msg_console("[fmap] status :0x%x", (msgrsp_arg->param2) );

			kcomm_send_rsp(CMD_EXTRA_MAP_CHECK, (u8*)msgrsp_arg->param2, 0);
			break;
		}
#endif
#endif
        // Test Commands
#ifndef CUSTOMER_SETTING_REMOVE_CMD
        case CMD_MEM_WRITE:
#if MSG_MEM_BYTE_WRITE
        /* NPU memory(0x30000000) doest not support byte R/W */
            if ((msgcmd_arg->param1 >= NPU_PA_BASE) && (msghdr->param1 < 0x40000000)) {
#else
            if (1) {
#endif
                for (int i = 0; i < wlen; i++) {
                    outw(msgcmd_arg->param1 + i * 4, (u32) *(pwdata + i));
                }
            }
            else {
                for (int i = 0; i < msgcmd_arg->param2; i++) {
                    outb(msgcmd_arg->param1 + i, *(pdata + i));
                }
            }
            // msg byte write size is unchanged
            msgrsp_arg->error = NO_ERROR;
            kcomm_send_rsp(CMD_MEM_WRITE, NULL, 0);
            break;

        case CMD_MEM_READ:
#if MSG_MEM_BYTE_WRITE
        /* NPU memory(0x30000000) doest not support byte R/W */
            if ((msgcmd_arg->param1 >= NPU_PA_BASE) && (msghdr->param1 < 0x40000000)) {
#else
            if (1) {
#endif
                for (int i = 0; i < wlen; i++) {
                    *(pwdata + i) = inw(msgcmd_arg->param1 + 4 * i);
                }
            }
            else {
                for (int i = 0; i < msgcmd_arg->param2; i++) {
                    *(pdata + i) = inb(msgcmd_arg->param1 + i);
                }
            }
            // msg byte read size is unchanged
            msgrsp_arg->error = NO_ERROR;
            kcomm_send_rsp(CMD_MEM_READ, (u8 *)pdata, msgcmd_arg->param2);
            break;

        case CMD_ACK_NACK:
        {
            dbg_msg("[Host_com] Received ACK/NACK\r\n");
            // Send results back to host
            if (msgcmd_arg->param1) {
                kcomm_send_no_rsp();
                break;  // if NACK, ignore the command and no reply
            }

            // We don't enforce Source ID check yet, since only DME is using this
#if 0
            if (dme_conf.img_conf.image_format & IMAGE_FORMAT_RAW_OUTPUT){
                kcomm_write((u8 *) p_raw_out_s, final_len, WMODE_DEF);
            } else if (dme_conf.img_conf.image_format & IMAGE_FORMAT_MODEL_AGE_GENDER) {
                kcomm_write((u8 *) p_fd_age_gender_out_s, final_len, WMODE_DEF);
            } else {
                kcomm_write((u8 *) p_od_out_s, final_len, WMODE_DEF);
            }
#endif
            break;
        }

        case CMD_TEST_ECHO:
            // msg bytes echo'ed is unchanged
            msgrsp_arg->error = NO_ERROR;
            kcomm_send_rsp(CMD_TEST_ECHO, (u8 *)pdata, msgcmd_arg->param2);
            break;

        case CMD_MEM_CLR:
            memset((void *)msgcmd_arg->param1, 0, msgcmd_arg->param2);
            // msg clear size is unchanged
            msgrsp_arg->error = NO_ERROR;
            kcomm_send_rsp(CMD_MEM_CLR, NULL, 0);
            break;

        case CMD_FILE_WRITE:
        {
            u32 bytes_read;
            bytes_read = kcomm_read(msgcmd_arg->param1, msgcmd_arg->param2);
            if (bytes_read == msgcmd_arg->param2)
                msgrsp_arg->error = NO_ERROR;
            else
                msgrsp_arg->error = FILE_ERROR;
            msgrsp_arg->param2 = bytes_read;
            kcomm_send_rsp(CMD_FILE_WRITE, NULL, 0);
            break;
        }
        case CMD_FLASH_MEM_WRITE:
        {
#if CFG_USB_PROGRAME_FW_ENABLE == YES
            
#define FLASH_CAPACITY_PAGE_SIZE 0x2000000 // 32M

            u32 srt = osKernelGetTickCount();
            u32 status, program_addr;
            u8 last_page_flag;
            last_page_flag = (u8)( ( opcmdpram->op_parm1 & 0xFF000000 ) >> 24 );
            program_addr = FLASH_CAPACITY_PAGE_SIZE * ( ( opcmdpram->op_parm1 & 0x00FF0000 ) >> 16 );
            status = opcmdpram->op_parm1 & 0xFFFF;

            dbg_msg("[flash programe] start with status=%#x, size=%#x, crc=%#X, page=%#x, last=%d !!!", status, opcmdpram->op_parm2, msgrsp_arg->param2, program_addr, last_page_flag );

            msgrsp_arg->error = kl520_api_com_flash_program(status, opcmdpram->op_parm2, program_addr, &msgrsp_arg->param2 );

            dbg_msg("[flash program]crc:%#x", msgrsp_arg->param2);
            dbg_msg("[flash program]usb time:%05d (ms)",osKernelGetTickCount() - srt);
            kcomm_send_rsp(CMD_FLASH_MEM_WRITE, NULL, 0);
            memset( (void*)(KDP_DDR_HEAP_HEAD_FOR_MALLOC_END+1), 0x00, FLASH_CAPACITY_PAGE_SIZE);
            if ( msgrsp_arg->error == NO_ERROR && last_page_flag == 1 ) {
            //delay_us(50*1000);
                osDelay(5000);
                power_mgr_sw_reset();
            }

        break;
#endif

        }

        case CMD_FLASH_READ:
        {
            u32 flash_addr = opcmdpram->op_parm1;
            u32 read_size = opcmdpram->op_parm2;
            dbg_msg_console("flash_addr:0x%08x, read_size:%d", flash_addr, read_size);
            kdp_memxfer_flash_to_ddr(KDP_DDR_MEM_START, flash_addr, read_size);
            dbg_msg_console("Read finish");
            pwdata=(void*)KDP_DDR_MEM_START;

            msgrsp_arg->error = NO_ERROR;
        
            kcomm_write((u8 *) pwdata, read_size, WMODE_DEF);
            dbg_msg_console("kcomm_write finish");

            break;
        }

        // Operational Commands
        case CMD_RESET:
        {
            u32 reset_mode, reset_flag = 0;  // reset flag: 1->sleep, 2->reset, 3->RTC

            reset_mode = opcmdpram->op_parm1;
#if ( USB_HOST == YES )
            if ((reset_mode < 2) || (reset_mode == 255)  || (reset_mode == 256)
#else
            if (((reset_mode < 5) && (reset_mode != 2)) || (reset_mode == 255) || (reset_mode == 256)
#endif
                || ((reset_mode >> 16) == 0x1000) || ((reset_mode >> 16) == 0x2000))
            {
                if ((reset_mode + opcmdpram->op_parm2) == 0xFFFFFFFF) {
                    if (reset_mode == 3) reset_flag = 1;    // go to sleep
                    if (reset_mode == 255) reset_flag = 2;  // reset
                    if (reset_mode == 256) reset_flag = 3;  // power down
                    if ((reset_mode >> 16) == 0x1000) {
                         log_set_level_scpu(reset_mode & 0x000F);  // set scpu debug output level
                        log_set_level_scpu((reset_mode & 0x0F00)>> 8);  // set ncpu debug output level

                        // Disable OS tick for power but keep it for profile/debug
                        if ((reset_mode & 0x000F) >= LOG_PROFILE)
                            OS_Tick_Enable();
                        else
                            OS_Tick_Disable();
                    }
                    if ((reset_mode >> 16) == 0x2000) {
                        // switch active boot image, bit 0 -> scpu, bit 1 -> ncpu

                        int ret = ota_update_switch_active_partition(reset_mode & 0x3);
                        msgrsp_arg->error = ret;

                    }
                    msgrsp_arg->error = NO_ERROR;  // not doing anything else for now
                }
                else
                    msgrsp_arg->error = BAD_CONFIRMATION;
            }
            else
            {
                msgrsp_arg->error = BAD_MODE;
            }
            msgrsp_arg->param2 = 0;
            kcomm_send_rsp(CMD_RESET, NULL, 0);
            if (reset_flag) {
                kcomm_wait();  // wait till reply message is done
                // ncpu fw needs update to support waking up from sleep
//                if (reset_flag == 1) power_manager_sleep();
                if (reset_flag == 2) power_mgr_sw_reset();
                if (reset_flag == 3) power_manager_shutdown();
            }
            break;
        }

        case CMD_SYSTEM_STATUS:
        {
            system_info sys_info = { 0 };
            kl520_api_get_device_info(&sys_info);
            u16* status;
            msgrsp_arg->error = 0x520;   // use "error" parameter for device ID
            msgrsp_arg->param2 = (u32)sys_info.fw_scpu_version.version;  // and "len" parameter for firmware version
            status = (u16*)msgrsp_arg->data;
            *status++ = host_com_sys_sts;
            *status = host_com_app_id;
            kcomm_send_rsp(CMD_SYSTEM_STATUS, msgrsp_arg->data, 4);
            kl520_api_free_device_info(&sys_info);
            break;
        }

        case CMD_OTA_UPDATE:
        {
            uint32_t ota_case = opcmdpram->op_parm1;
            uint32_t ota_size = opcmdpram->op_parm2;
            msgrsp_arg->error = ota_update_case( ota_case, 0x60000000, ota_size );

            kcomm_send_rsp(CMD_OTA_UPDATE, NULL, 0);

            if ( msgrsp_arg->error == SUCCESS && ota_case != 0x0B ) {
                //delay_us(50*1000);
                osDelay(5000);
                power_mgr_sw_reset();
            }
            break;
        }

        case CMD_QUERY_APPS:
        case CMD_SELECT_APP:
        case CMD_SET_MODE:
        case CMD_SET_EVENTS:
        case CMD_UPDATE:
        case CMD_ABORT:
#endif

    default:
        msgrsp_arg->error = RSP_UKNOWN_CMD;
        msgrsp_arg->param2 = 0;
        kcomm_send_rsp(msghdr->cmd, NULL, 0);
        break;
    }
    return rstatus;
}

static int cmd_parser(u8 *buf, int len)
{
    MsgHdr *msghdr;
    CmdPram *msgcmd_arg;
    RspPram *msgrsp_arg;
    OpPram *opcmdpram;
    u8 rstatus = 0;

    msghdr = (MsgHdr*)buf;
    msgcmd_arg = (CmdPram*)(buf + sizeof(MsgHdr));
    msgrsp_arg = (RspPram*)(buf + sizeof(MsgHdr));  // use the same rcv buffer to format responses
    opcmdpram = (OpPram*)msgcmd_arg;  // used to parse operational commands

    dbg_msg("[Host_com] cmd_parser[%d]: cmd %d [addr, len]=[0x%x, %d]\n", len, msghdr->cmd, msgcmd_arg->param1, msgcmd_arg->param2);

    if ((msghdr->cmd < 0x100) || (CMD_FLASH_READ == msghdr->cmd))  // build-in commands
    {
        handle_host_com_general(buf, len);
    }
    else if( CMD_CAPTRUE_SENSOR == msghdr->cmd )
    {
        u32 srt = osKernelGetTickCount();
        int cam_idx = opcmdpram->op_parm1;
        if((0 != cam_idx) && (1 != cam_idx))
            cam_idx = 0;
        
        if ( kl520_api_cam_state_get(cam_idx) != KDP_DEVICE_CAMERA_RUNNING )
        {
            msgrsp_arg->error = PARAM_ERR;
        
            kcomm_send_rsp(CMD_CAPTRUE_SENSOR, (u8*)msgrsp_arg->error, 0);
        }

        int buf_idx;
        u32 buf_addr = kdp_fb_mgr_next_read(cam_idx, &buf_idx);

        msgrsp_arg->error = NO_ERROR;

        kcomm_write((u8 *) buf_addr, msgcmd_arg->param2, WMODE_DEF);
        kdp_fb_mgr_free_read_buf(cam_idx);
        dbg_msg_com("[snapshot]usb time:%05d (ms)",osKernelGetTickCount() - srt);
    }
    else if( CMD_FDFR_THREAD_CLOSE == msghdr->cmd )
    {
        if( kl520_api_fdfr_exist_thread() )
        {
            sample_face_close();
        }
        kcomm_send_rsp(CMD_FDFR_THREAD_CLOSE, NULL, 0);
    }
#if ( CFG_GUI_ENABLE == YES )
    else if( CMD_GUI_CTRL_SERIES == ((msghdr->cmd)&CMD_GUI_CTRL_SERIES) )
    //else if( CMD_GUI_CTRL_SERIES <= msghdr->cmd )
    {
        int data = opcmdpram->op_parm1;
        switch ( msghdr->cmd )
        {
            case CMD_GUI_REGISTER:
                user_com_set_data(GUI_IMG_BTN_REGISTER, data, GUI_FSM_SRC_USER_COM);
                set_event(kl520_api_get_event(), KL520_APP_FLAG_COMM);
                wait_event(kl520_api_get_event(), KL520_APP_FLAG_COMM_DONE);
                break;
            case CMD_GUI_RECOGNIZE:
                user_com_set_data(GUI_IMG_BTN_RECOGNIZE, data, GUI_FSM_SRC_USER_COM);
                set_event(kl520_api_get_event(), KL520_APP_FLAG_COMM);
                wait_event(kl520_api_get_event(), KL520_APP_FLAG_COMM_DONE);
                break;
            case CMD_GUI_DELETE_ALL:
                user_com_set_data(GUI_IMG_BTN_DELETE, 0, GUI_FSM_SRC_USER_COM);
                set_event(kl520_api_get_event(), KL520_APP_FLAG_COMM);
                break;
            default:
                return 0;
        }

    }
#endif
#if (CFG_USB_CLOUD_DB_UPDATE == YES && CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_AP_CTRL_TYPE < EX_FM_UART_AP_CTRL_MAIN_DB )
    else if( CMD_CLOUD_UPDATE_DB == (msghdr->cmd&CMD_CLOUD_UPDATE_DB)  )
    //else if( CMD_GUI_CTRL_SERIES <= msghdr->cmd )
    {
        u32 user_idx = msgcmd_arg->param2;
        u16 ret = 1;

        if     ( msgcmd_arg->param1 == 0x00000001 ) // add
        {
            kdp_api_ap_control_set_each_db( (u16)user_idx );
            ret = 0;
        }
        else if( msgcmd_arg->param1 == 0x00000002 ) // delete
        {
            kdp_api_ap_control_del_each_db( (u16)user_idx );
            ret = 0;
        }
        else if( msgcmd_arg->param1 == 0x00000003 ) // modify
        {
            kdp_api_ap_control_del_each_db( (u16)user_idx );
            kdp_api_ap_control_set_each_db( (u16)user_idx );
            ret = 0;
        }
        else if( msgcmd_arg->param1 == 0x00000004 ) // search
        {
            const u8 table_size = 2 + ( (MAX_USER + 7) >> 3);
            u8 bin_table[table_size] = { 0 };
            u16 db_valid_cnt = kl520_api_ap_com_db_query_db_all_mode( &bin_table[2] );
            bin_table[0] = (u8)(db_valid_cnt&0x00FF);
            bin_table[1] = (u8)((db_valid_cnt>>8) & 0x00FF);
            kcomm_write( bin_table , sizeof(bin_table), WMODE_DEF);

        }
        else if( msgcmd_arg->param1 == 0x00000005 ) // search
        {
            u16 user_id = (u16)msgcmd_arg->param2;
            ret = kl520_api_ap_com_db_query_db_one_mode( user_id );
        }
        else
        {
            // do-nothing
        }

        if( msgcmd_arg->param1 == 0x00000004 )
        {

        }
        else
        {
            msgrsp_arg->error = ret;
            kcomm_send_rsp(CMD_CLOUD_UPDATE_DB, NULL, 0);
        }

    }
#endif


#if CFG_USB_EXPORT_STREAM_IMG == YES
    else if( CMD_EXPORT_STREAM_IMG == msghdr->cmd )
    {
        u32 srt = osKernelGetTickCount();
        dbg_msg("[Host_com] get stream cmd!");

        u32 iamge_addr = 0;
        s8 cam_idx = MIPI_CAM_RGB;

        eSTREAM_IMAGE_EXPORT_SRC export_mode = STRAM_IMAGE_BOTH_CAMERA_e;

        dbg_msg("[Host_com] stream start!!!!");

#if CFG_USB_EXPORT_LIVENESS_RET == YES

        if( (msgcmd_arg->param1 & 0x00FF) == 0x0055 )
        {
            if(msgcmd_arg->param1 == 0x0055 )
            {
                msgrsp_arg->error = NO_ERROR;
                kcomm_write((u8 *) kl520_api_export_stream_get_fd_box(), msgcmd_arg->param2, WMODE_DEF);
            }
        }
        else
#endif
        if( (msgcmd_arg->param1 & 0xFFFF) == 0xFFFF )
        {
            msgrsp_arg->error = NO_ERROR;
        
            msgrsp_arg->param1 = ((( (uint32_t)2 & 0x3 ) << 30) | (( DISPLAY_WIDTH & 0x7FFF ) << 15) | (( DISPLAY_HEIGHT & 0x7FFF ) << 0 ) );
        
            kcomm_send_rsp(CMD_EXPORT_STREAM_IMG, (u8*)msgrsp_arg->error, 0);
        }
        else
        {
            kl520_api_export_stream_get_info( msgrsp_arg->param1, &cam_idx, &export_mode);
            //dbg_msg("[Host_com] stream get idx: %d with 0x%08x!!!!", cam_idx, (msgcmd_arg->param1 & 0x00AA) );

            kl520_api_export_stream_set_image_crtl( cam_idx, export_mode );

            while(kl520_api_export_stream_image_catch()==0) {osDelay(10);}

            while( (iamge_addr = kl520_api_export_stream_image_addr( export_mode ))==0) {osDelay(10);}

            pwdata=(void*)iamge_addr;

            // msg byte read size is unchanged
            msgrsp_arg->error = NO_ERROR;

            kcomm_write((u8 *) pwdata, msgcmd_arg->param2, WMODE_DEF);

        }

        dbg_msg("[snapshot]usb time:%05d (ms)",osKernelGetTickCount() - srt);
    }

#endif // CFG_USB_EXPORT_STREAM_IMG == YES

    else
    {
        msgrsp_arg->error = RSP_UKNOWN_CMD;
        msgrsp_arg->param2 = 0;
        kcomm_send_rsp(msghdr->cmd, NULL, 0);
    }
    return rstatus;
}

#ifndef CUSTOMER_SETTING_REMOVE_CMD
static void device_sleep(void)
{
    host_com_sys_sts = 1;
//    trace_msg("[Host_com] Going to sleep....\r\n");
}

static void device_wakeup(void)
{
    host_com_sys_sts = 0;
//    trace_msg("[Host_com] Waking up....\r\n");
}

struct pm_s pm_host_com_device =
{
    .nap = 0,
    .wakeup_nap = 0,
    .deep_nap = 0,
    .wakeup_deep_nap = 0,
    .sleep = (pm_call) device_sleep,
    .wakeup_sleep = (pm_call) device_wakeup,
    .deep_sleep = (pm_call) device_sleep,
    .wakeup_deep_sleep = (pm_call) device_wakeup,
};
#endif

/*
 * @brief check_msg(), process the incoming uart message and checks the packet format
 * @param buf buffer for uart read
 */
static int check_msg(u8 *buf, int size)
{
    MsgHdr *hdr = (MsgHdr*)msg_rbuf;

    if (hdr->preamble != MSG_HDR_CMD) {
        err_msg("BAD Preamble %d\r\n", size);
        return -1;
    }

    if (hdr->ctrl & PKT_CRC_FLAG) {
        int length = hdr->ctrl & 0x01FFF;
        u32 crc16 = gen_crc16(buf, length);

        if (crc16 != (buf[length+0]+(buf[length+1]<<8))) {
            // send CRC ERROR packet
            err_msg("BAD CRC [%d] crc16=%x : %x\r\n", length, crc16, buf[length+3]+(buf[length+3]<<8));

            u16 rsp_cmd = hdr->cmd | 0x8000;
            hdr = (MsgHdr*) msg_tbuf;
            hdr->cmd = rsp_cmd;
            hdr->msg_len = 8;

            RspPram *rsp = (RspPram*) (msg_tbuf + sizeof(MsgHdr));
            rsp->error = CMD_CRC_ERR;
            rsp->param2 = 0;

            kcomm_write_msg(msg_tbuf, sizeof(MsgHdr)+sizeof(RspPram), 0);  // send crc error packet, without crc

            return -1;
        }
    }
    cmd_parser(buf, size);  // if there are more than one command, we only process the first one
    return size;
}

static void host_comm_thread(void *argument)
{
#ifndef CUSTOMER_SETTING_REMOVE_CMD
    power_manager_register(PM_DEVICE_HOST_COM, &pm_host_com_device);
#endif
    for (;;) {
        uint32_t flags = osThreadFlagsWait(FLAG_COMM_USB_DONE | FLAG_COMM_UART_DONE | FLAG_COMM_TIMER, osFlagsWaitAny, osWaitForever);
        //osThreadFlagsClear(flags);

#if ( USB_HOST == YES )
        if (flags & FLAG_COMM_USB_DONE)
#else
        if (flags & FLAG_COMM_UART_DONE)
#endif
        {
            osThreadSetPriority(tid_host_comm, osPriorityAboveNormal2);
            check_msg(msg_rbuf, msg_rbuf[2]+4);  // process command
            osThreadSetPriority(tid_host_comm, osPriorityNormal);
        }
    }
}

void host_com_init(void)
{
    if(msg_rbuf == NULL)
    {
        msg_rbuf = (u8*)kdp_ddr_reserve(MSG_DATA_BUF_MAX + sizeof(MsgHdr)+ sizeof(RspPram) + 4 + 4);//4096+72 bytes
    }
    if(msg_tbuf == NULL)
    {
        msg_tbuf = (u8*)kdp_ddr_reserve(MSG_DATA_BUF_MAX + sizeof(MsgHdr)+ sizeof(RspPram) + 4 + 4);//4096+72 bytes
    }

    osThreadAttr_t attr = {
        .stack_size = 1536
    };

    tid_host_comm = osThreadNew(host_comm_thread, NULL, &attr);
    kcomm_msg_init(tid_host_comm);
}

