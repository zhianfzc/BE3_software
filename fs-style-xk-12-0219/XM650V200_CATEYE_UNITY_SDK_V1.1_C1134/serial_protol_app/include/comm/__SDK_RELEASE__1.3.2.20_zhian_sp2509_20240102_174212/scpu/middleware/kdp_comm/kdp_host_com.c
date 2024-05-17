 /*
 * Kneron Host Communication driver
 *
 * Copyright (C) 2018 Kneron, Inc. All rights reserved.
 *
 */
#include "kdp_host_com.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK )
#if ( CFG_COM_USB_PROT_TYPE == COM_USB_PROT_KDP )
#include "Kdp_memxfer.h"
#include "kdp_comm_app.h"
#include "kdp_comm.h"
#include "host_msg.h"

#include "Com_err.h"
#include "com.h"
#include "kneron_mozart.h"
#include "host_usb_com.h"
#include "kdp520_usbd_status.h"
#include "framework/event.h"
#include "os_tick.h"
#include "ota_update.h"
#include "kdp_ddr_table.h"
#include "usr_ddr_img_table.h"
#include "ipc.h"
#include "io.h"
#include "power.h"
#include "Power_manager.h"
#include "flash.h"
#include "sample_app_console.h"
#include "kl520_api_snapshot.h"
#include "kl520_api_fdfr.h"


// SFID prototype
u32 kcomm_read(u32 address, u32 image_size);

osThreadId_t tid_host_comm;

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

/**
 * @brief cmd_parser(), uart/usb message parser
 */
__WEAK u8 user_cmd_parser(u8 *buf, int len){ return 0;}

int cmd_parser(u8 *buf, int len)
{
    MsgHdr *msghdr;
    CmdPram *msgcmd_arg;
    RspPram *msgrsp_arg;
    OpPram *opcmdpram;
    u8 *pdata = NULL;
    u32 *pwdata = NULL;
    u8 rstatus = 0;
    int wlen;
#if ( CFG_GUI_ENABLE == YES )
    //user_behavior_data ret_data;
    int data = 0;
#endif

    msghdr = (MsgHdr*)buf;
    msgcmd_arg = (CmdPram*)(buf + sizeof(MsgHdr));
    msgrsp_arg = (RspPram*)(buf + sizeof(MsgHdr));  // use the same rcv buffer to format responses
    opcmdpram = (OpPram*)msgcmd_arg;  // used to parse operational commands

    dbg_msg("[Host_com] cmd_parser[%d]: cmd %d [addr, len]=[0x%x, %d]\n", len, msghdr->cmd, msgcmd_arg->param1, msgcmd_arg->param2);

    pdata = (u8*)(buf + (sizeof(MsgHdr) + sizeof(CmdPram)));  // points to stuff beyond error and byte count
    pwdata = (u32*)(buf + (sizeof(MsgHdr) + sizeof(CmdPram)));
    wlen = (msgcmd_arg->param2 % 4) == 0 ? msgcmd_arg->param2 / 4 : msgcmd_arg->param2 / 4 + 1;  // last bytes is treated as word

    switch (msghdr->cmd) {
    // Test Commands
#ifdef KDP_CMD_MEM_WRITE    
        case KDP_CMD_MEM_WRITE:
        {
#if MSG_MEM_BYTE_WRITE
        /* NPU memory(0x30000000) doest not support byte R/W */
            if ((msgcmd_arg->param1 >= NPU_PA_BASE) && (msghdr->param1 < 0x40000000))
#else
            if (1)
#endif
            {
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
            kcomm_send_rsp(KDP_CMD_MEM_WRITE, NULL, 0);
            break;            
        }
#endif
#ifdef KDP_CMD_MEM_READ
        case KDP_CMD_MEM_READ:
        {
#if MSG_MEM_BYTE_WRITE
        /* NPU memory(0x30000000) doest not support byte R/W */
            if ((msgcmd_arg->param1 >= NPU_PA_BASE) && (msghdr->param1 < 0x40000000))
#else
            if (1) 
#endif
            {
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
            kcomm_send_rsp(KDP_CMD_MEM_READ, (u8 *)pdata, msgcmd_arg->param2);
            break;
        }
#endif        
#ifdef KDP_CMD_ACK_NACK        
        case KDP_CMD_ACK_NACK:
        {
            dbg_msg("[Host_com] Received ACK/NACK\r\n");
            // Send results back to host
            if (msgcmd_arg->param1) {
                kcomm_send_no_rsp();
                break;  // if NACK, ignore the command and no reply
            }
            break;
        }
#endif        
#ifdef KDP_CMD_TEST_ECHO 
        case KDP_CMD_TEST_ECHO:
        {
            // msg bytes echo'ed is unchanged
            msgrsp_arg->error = NO_ERROR;
            kcomm_send_rsp(KDP_CMD_TEST_ECHO, (u8 *)pdata, msgcmd_arg->param2);
            break;
        }
#endif
#ifdef KDP_CMD_MEM_CLR
        case KDP_CMD_MEM_CLR:
        {
            memset((void *)msgcmd_arg->param1, 0, msgcmd_arg->param2);
            // msg clear size is unchanged
            msgrsp_arg->error = NO_ERROR;
            kcomm_send_rsp(KDP_CMD_MEM_CLR, NULL, 0);
            break;
        }
#endif
#ifdef KDP_CMD_FILE_WRITE
        case KDP_CMD_FILE_WRITE:
        {
            u32 bytes_read;
            bytes_read = kcomm_read(msgcmd_arg->param1, msgcmd_arg->param2);
            if (bytes_read == msgcmd_arg->param2)
                msgrsp_arg->error = NO_ERROR;
            else
                msgrsp_arg->error = FILE_ERROR;
            msgrsp_arg->param2 = bytes_read;
            kcomm_send_rsp(KDP_CMD_FILE_WRITE, NULL, 0);
            break;
        }
#endif
#ifdef KDP_CMD_FLASH_MEM_WRITE
        case KDP_CMD_FLASH_MEM_WRITE:
        {            
            #define FLASH_CAPACITY_PAGE_SIZE 0x2000000 // 32M

            u32 srt = osKernelGetTickCount();
            u32 status, program_addr;
            u8 last_page_flag;
            last_page_flag = (u8)( ( opcmdpram->op_parm1 & 0xFF000000 ) >> 24 );
            program_addr = FLASH_CAPACITY_PAGE_SIZE * ( ( opcmdpram->op_parm1 & 0x00FF0000 ) >> 16 );
            status = opcmdpram->op_parm1 & 0xFFFF;

            //dbg_msg("[flash programe] start with status=%#x, size=%#x, crc=%#X, page=%#x, last=%d !!!", status, opcmdpram->op_parm2, msgrsp_arg->param2, program_addr, last_page_flag );

            msgrsp_arg->error = kl520_api_com_flash_program(status, opcmdpram->op_parm2, program_addr, &msgrsp_arg->param2 );

            //dbg_msg("[flash program]crc:%#x", msgrsp_arg->param2);
            //dbg_msg("[flash program]usb time:%05d (ms)",osKernelGetTickCount() - srt);
            
            kcomm_send_rsp(KDP_CMD_FLASH_MEM_WRITE, NULL, 0);
            memset( (void*)(KDP_DDR_HEAP_HEAD_FOR_MALLOC_END+1), 0x00, FLASH_CAPACITY_PAGE_SIZE);
            if ( msgrsp_arg->error == NO_ERROR && last_page_flag == 1 ) {           
                osDelay(5000);
                power_mgr_sw_reset();
            }

        break;
        }
#endif
        // Operational Commands
#ifdef KDP_CMD_RESET        
        case KDP_CMD_RESET:
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
            kcomm_send_rsp(KDP_CMD_RESET, NULL, 0);
            if (reset_flag) {
                kcomm_wait();  // wait till reply message is done
                // ncpu fw needs update to support waking up from sleep
    //                if (reset_flag == 1) power_manager_sleep();
                if (reset_flag == 2) power_mgr_sw_reset();
                if (reset_flag == 3) power_manager_shutdown();
            }
            break;
        }
#endif
#ifdef KDP_CMD_OTA_UPDATE
        case KDP_CMD_OTA_UPDATE:
        {
            uint32_t ota_case = opcmdpram->op_parm1;
            uint32_t ota_size = opcmdpram->op_parm2;
            msgrsp_arg->error = ota_update_case( ota_case, 0x60000000, ota_size );

            kcomm_send_rsp(KDP_CMD_OTA_UPDATE, NULL, 0);

            if ( msgrsp_arg->error == SUCCESS && ota_case != 0x0B ) {
                //delay_us(50*1000);
                osDelay(5000);
                power_mgr_sw_reset();
            }
            break;
        }
#endif
#ifdef KDP_CMD_SNAPSHOT
        case KDP_CMD_SNAPSHOT:
        {
            u32 srt = osKernelGetTickCount();
            if( kl520_api_ap_com_snapshot_catch( opcmdpram->op_parm1 ) == 1 )
            {
                // msg byte read size is unchanged
                msgrsp_arg->error = NO_ERROR;
                kcomm_write((u8 *) kl520_api_ap_com_snapshot_catch_addr( opcmdpram->op_parm1 ), msgcmd_arg->param2, WMODE_DEF);
            }
            else
            {
                msgrsp_arg->error = RSP_UKNOWN_CMD;
                kcomm_send_rsp(KDP_CMD_SNAPSHOT, NULL, 0);
            }
            break;
        }
#endif
#ifdef KDP_CMD_SNAPSHOT_CHECK
        case KDP_CMD_SNAPSHOT_CHECK: // check avaiable snapshot count
        {
            msgrsp_arg->error = NO_ERROR;   // use "error" parameter for device ID
            msgrsp_arg->param2 = kl520_api_host_com_snpahot_status();
            kcomm_send_rsp(KDP_CMD_SNAPSHOT_CHECK, (u8*)msgrsp_arg->param2, 0);
            break;
        }
#endif
#ifdef KDP_CMD_RECEIVE_IMAGE
        case KDP_CMD_RECEIVE_IMAGE:
        {
            //dbg_msg("Received cmd: CMD_DME_SEND_IMAGE %d\n",ret);
            //Msg for the length of raw results

            if(msgrsp_arg->param1 == 0x1111)
            {
                kcomm_read(KDP_DDR_TEST_RGB_IMG_ADDR, msgcmd_arg->param2);
                dbg_msg_console("tgt %x,len=%d\n",KDP_DDR_TEST_RGB_IMG_ADDR,msgcmd_arg->param2);
            }
            else if(msgrsp_arg->param1 == 0x2222)
            {
                kcomm_read(KDP_DDR_TEST_NIR_IMG_ADDR, msgcmd_arg->param2);
            }
            else if(msgrsp_arg->param1 == 0x3333)
            {
                kcomm_read(KDP_DDR_TEST_INF_IMG_ADDR, msgcmd_arg->param2);
            }
            else if(msgrsp_arg->param1 == 0x4444)
            {
                u8 OnOff = (msgrsp_arg->param2 >> 30) & 0x3;
                u16 img_width = (msgrsp_arg->param2 >> 15) & 0x7FFF;
                u16 img_height = (msgrsp_arg->param2 >> 0) & 0x7FFF;
                if( OnOff == 1 ) kdp_e2e_config_image(KDP_E2E_IMG_SRC_RGB, img_width, img_height, 3, KDP_E2E_IMAGE_FORMAT_RGB565, 0, 0);
                else kdp_e2e_config_image(KDP_E2E_IMG_SRC_RGB, RGB_IMG_SOURCE_W, RGB_IMG_SOURCE_H, 3, KDP_E2E_IMAGE_FORMAT_RGB565, 0, 0);
            }
            else if(msgrsp_arg->param1 == 0x5555)
            {
                u8 OnOff = (msgrsp_arg->param2 >> 30) & 0x3;
                u16 img_width = (msgrsp_arg->param2 >> 15) & 0x7FFF;
                u16 img_height = (msgrsp_arg->param2 >> 0) & 0x7FFF;
                if( OnOff == 1 ) kdp_e2e_config_image(KDP_E2E_IMG_SRC_NIR, img_width, img_height, 1, KDP_E2E_IMAGE_FORMAT_RAW8, 0, 1);
                else kdp_e2e_config_image(KDP_E2E_IMG_SRC_NIR, NIR_IMG_SOURCE_W, NIR_IMG_SOURCE_H, 1, KDP_E2E_IMAGE_FORMAT_RAW8, 0, 1);
            }           
            else if(msgrsp_arg->param1 == 0xFFFF)
            {        
                struct kdp_img_cfg *img_cfg = kdp_e2e_get_img_cfg(KDP_E2E_IMG_SRC_RGB);
                msgrsp_arg->param1 = ((( kdp_app_get_pixel_size(img_cfg->image_format) & 0x3 ) << 30) | (( img_cfg->image_col & 0x7FFF ) << 15) | (( img_cfg->image_row & 0x7FFF ) << 0 ) );

#if CFG_ONE_SHOT_MODE == YES       
                img_cfg = kdp_e2e_get_img_cfg(KDP_E2E_IMG_SRC_NIR);
                msgrsp_arg->param1 = msgrsp_arg->param2 =  ((( kdp_app_get_pixel_size(img_cfg->image_format) & 0x3 ) << 30) | (( img_cfg->image_col & 0x7FFF ) << 15) | (( img_cfg->image_row & 0x7FFF ) << 0 ) );
#else
				img_cfg = kdp_e2e_get_img_cfg(KDP_E2E_IMG_SRC_NIR);
                msgrsp_arg->param2 =  ((( kdp_app_get_pixel_size(img_cfg->image_format) & 0x3 ) << 30) | (( img_cfg->image_col & 0x7FFF ) << 15) | (( img_cfg->image_row & 0x7FFF ) << 0 ) );                				
#endif

            }

            kcomm_send_rsp(KDP_CMD_RECEIVE_IMAGE, (u8*)msgrsp_arg->error, 0);
            break;
        }
#endif
#ifdef KDP_CMD_FDFR_THREAD_CLOSE
        case KDP_CMD_FDFR_THREAD_CLOSE:
        {
            if( kl520_api_fdfr_exist_thread() )
            {
                sample_face_close();
            }
            kcomm_send_rsp(KDP_CMD_FDFR_THREAD_CLOSE, NULL, 0);
            break;
        }
#endif
#ifdef KDP_CMD_EXPORT_STREAM_IMG
        case KDP_CMD_EXPORT_STREAM_IMG:
        {
            u32 srt = osKernelGetTickCount();

            u32 iamge_addr = 0;
            s8 cam_idx = MIPI_CAM_RGB;

            eSTREAM_IMAGE_EXPORT_SRC export_mode = STRAM_IMAGE_BOTH_CAMERA_e;

            if( (msgcmd_arg->param1 & 0xFFFF) == 0xFFFF )
            {
                msgrsp_arg->error = NO_ERROR;
            
                msgrsp_arg->param1 = ((( (uint32_t)2 & 0x3 ) << 30) | (( DISPLAY_WIDTH & 0x7FFF ) << 15) | (( DISPLAY_HEIGHT & 0x7FFF ) << 0 ) );
            
                kcomm_send_rsp(KDP_CMD_EXPORT_STREAM_IMG, (u8*)msgrsp_arg->error, 0);
            }
            else
            {
                kl520_api_export_stream_get_info( msgrsp_arg->param1, &cam_idx, &export_mode);

                kl520_api_export_stream_set_image_crtl( cam_idx, export_mode );

                while(kl520_api_export_stream_image_catch()==0) {osDelay(10);}

                while( (iamge_addr = kl520_api_export_stream_image_addr( export_mode ))==0) {osDelay(10);}

                pwdata=(void*)iamge_addr;

                // msg byte read size is unchanged
                msgrsp_arg->error = NO_ERROR;

                kcomm_write((u8 *) pwdata, msgcmd_arg->param2, WMODE_DEF);

            }
            break;
        }  
#endif

        default:
        {
            if( user_cmd_parser(buf, len) == 0 )
            {
                msgrsp_arg->error = RSP_UKNOWN_CMD;
                msgrsp_arg->param2 = 0;
                kcomm_send_rsp(msghdr->cmd, NULL, 0);
            }            
            break;
        }
    }
    return rstatus;
}

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
            rsp->error = KDP_CMD_CRC_ERR;
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
    uint32_t flags;
    int len;

    //init kdp_app
    //kdp_app_init();

    //ota_handle_first_time_boot();

    //reserve DDR memory for image buffer

    // register PM_DEVICE_HOST_COM for sleep & wakeup
    //power_manager_register(PM_DEVICE_HOST_COM, &pm_host_com_device);

    for (;;) {
        flags = osThreadFlagsWait(FLAG_COMM_USB_DONE | FLAG_COMM_UART_DONE | FLAG_COMM_TIMER, osFlagsWaitAny, osWaitForever);
        osThreadFlagsClear(flags);

        if (flags & FLAG_COMM_TIMER) {
            if (len > 0) {
                dbg_msg("Get uart_len = %d\n", len);
                check_msg(msg_rbuf, len);
            }
        }

        if (flags & FLAG_COMM_USB_DONE) {
            osThreadSetPriority(tid_host_comm, osPriorityAboveNormal2);
            check_msg(msg_rbuf, msg_rbuf[2]+4);  // process command
            osThreadSetPriority(tid_host_comm, osPriorityNormal);
        }
    }
}

void user_host_com_init(void)
{
    osThreadAttr_t attr = {
        .stack_size = 1024
    };

    if(msg_rbuf == NULL)
    {
        msg_rbuf = (u8*)kdp_ddr_reserve(MSG_DATA_BUF_MAX + sizeof(MsgHdr)+ sizeof(RspPram) + 4 + 4);//4096+72 bytes
    }
    if(msg_tbuf == NULL)
    {
        msg_tbuf = (u8*)kdp_ddr_reserve(MSG_DATA_BUF_MAX + sizeof(MsgHdr)+ sizeof(RspPram) + 4 + 4);//4096+72 bytes
    }

    tid_host_comm = osThreadNew(host_comm_thread, NULL, &attr);
    kcomm_msg_init(tid_host_comm);
}
#endif
#endif
#endif
