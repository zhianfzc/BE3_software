#include "kdp_comm_app.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP )
#include "kneron_mozart_ext.h"
#include "io.h"
#include "board_ddr_table.h"
#include "pinmux.h"
#include "framework/event.h"
#include "framework/v2k.h"
#include "framework/v2k_color.h"
#include "framework/v2k_image.h"
#include "framework/framework_driver.h"
#include "media/display/display.h"
#include "media/display/video_renderer.h"
#include "drivers.h"
#include "ddr.h"
#include "rtc.h"
#include "system.h"
#include "power.h"
#include "flash.h"
#include "touch.h"
#include "ota.h"
#include "ota_update.h"
#include "power_manager.h"
#include "kdp520_ssp.h"
#include "kdp520_spi.h"
#include "kdp520_pwm_timer.h"
#include "kdp_uart.h"
#include "kdp_com.h"
#include "kdp_memxfer.h"
#include "kdp_e2e_db.h"
#include "kdp_model.h"
#include "kdp_system.h"
#include "kdp_memory.h"
#include "kl520_api.h"
#include "kl520_api_fdfr.h"
#include "kl520_api_device_id.h"
#include "kl520_api_ssp.h"
#include "kl520_api_snapshot.h"
#include "kl520_api_extra_fmap.h"
#include "kl520_api_sim.h"
#include "kl520_api_camera.h"

#if ( CFG_GUI_ENABLE == YES )
#include "sample_gui_main.h"
#include "sample_gui_fsm_events.h"
#endif
#include "user_io.h"
#include "user_ui.h"
#include "sample_app_console.h"
#include "sample_app_touch.h"

#include "kdp_host_com.h"
#include "kdp_comm_protoco.h"
#include "kdp_comm_md5.h"
#include "kdp_comm_and_gui_fdr.h"

#define OTA_START_PROCESS_FLAG (4)

#define OTA_STATUS_IDLE     (0)
#define OTA_STATUS_START    (1)
#define OTA_STATUS_HEADER   (2)
#define OTA_STATUS_PACKET   (3)
#define OTA_STATUS_FINAL    (4)

#if (CFG_E2E_REC_NOTE == YES)
u16 skip_face_note = 0;
#endif

extern u8 db_write;
extern osThreadId_t tid_fdfr_update_fr;

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK )
ota_update_info_t ota_update_info;
imp_exp_mass_data_info_t g_tImpExpMassDataPkgInfo;

uint32_t g_nSysReadyTime = 0;
uint32_t g_nAutoPowerOffCnt = 0;
uint32_t g_nSysInternalTime = 0;

u16 g_nFaceId = 0;
u8  g_eFaceDirState = 0;

BOOL g_bStopSendMsg = FALSE;
BOOL g_bUserDemoMode = FALSE;
BOOL g_bAutoPowerOff = FALSE;
BOOL g_bKID_SetKey  = FALSE;
u8 g_nCheckSum_Error = 0;
kl520_face_add_mode g_eEnrollAddFaceTypeMode = FACE_ADD_MODE_5_FACES;
#endif

extern u8 g_snap_img_cnt;
u8 g_nEncryptionMode = NO_ENCRYPTION;
volatile static bool use_debug_key = false;

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
#if ( ENCRYPTION_MODE&AES_ENCRYPTION ) || ( ENCRYPTION_MODE&XOR_ENCRYPTION )
uint8_t key_num[16] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
#endif
#endif

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK )
extern uint32_t SysWaitDelayTime(void)
{
    u32 nIntervalTime =  osKernelGetTickCount() - g_nSysInternalTime;

    if ( nIntervalTime <= SYS_INTERNAL_TIME_INTERVAL )
    {
        nIntervalTime = SYS_INTERNAL_TIME_INTERVAL - nIntervalTime;
        // dbg_msg_console("_time=%d", nIntervalTime);
        osDelay(nIntervalTime);
        //nIntervalTime = SYS_INTERNAL_TIME_INTERVAL - nIntervalTime;
    }

    return nIntervalTime;
}

void wait_fr_update_thread(void)
{
    u16 cnt = 0;
    while(tid_fdfr_update_fr != NULL && cnt < 100) {
        osDelay(50);
        cnt++;
        dbg_msg_console("RESET WAIT %d", cnt);	// avoid restart when is updating DB
    }
}
#endif

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x00-0x0F System
#ifdef KID_DEVICE_INFO
extern uint8_t DSM_Uart_GetDeviceInfo(device_info_data* device_info)
{
    uint8_t ret = 0;

    struct fw_misc_data ver_info;
    //TODO : get fw version and kl520 id.
    device_info->sn = kl520_api_get_unique_id();
    device_info->boot_version = kdp_sys_get_spl_version();

    dbg_msg_console("SN: 0x%X", device_info->sn);
    dbg_msg_console("boot_version: %X", device_info->boot_version);

    kl520_api_get_scpu_version(&ver_info);
    memcpy(device_info->cpu_version, ver_info.version, sizeof(device_info->cpu_version));

    kl520_api_get_ncpu_version(&ver_info);
    memcpy(device_info->npu_version, ver_info.version, sizeof(device_info->npu_version));

    {
        struct kdp_model_s **model_infos = NULL;
        kdp_model_load_model(-1);
        uint8_t model_count = kdp_model_get_model_count();
        dbg_msg_console("model_count:%d",model_count);
        if (!model_infos)
            model_infos = (struct kdp_model_s**)kdp_ddr_reserve(sizeof(struct kdp_model_s*) * model_count);

        for (int i = 0; i < model_count; ++i)
        {
            model_infos[i] = (struct kdp_model_s*)kdp_model_get_model_info(i);
            switch(model_infos[i]->model_type)
            {
                case KNERON_FD_MASK_MBSSD_200_200_3://KNERON_FDSSD
                case KNERON_FD_ROTATE:
                {
                    device_info->fd_version = model_infos[i]->model_version;
                    dbg_msg_console("(%d). fd_version and mask_version:0x%x and 0x%x",i, model_infos[i]->model_version,device_info->fd_version);
                    break;
                }

                case KNERON_FR_VGG10:
                {
                    device_info->fr_version = model_infos[i]->model_version;
                    dbg_msg_console("(%d). fr_version:0x%x ", i, device_info->fr_version);
                    break;
                }

                case UPHOTON_LIVENESS:
                {
                    device_info->live_version = model_infos[i]->model_version;
                    dbg_msg_console("(%d). live_version:0x%x ", i, device_info->live_version);
                    break;
                }

                case KNERON_EYELID_DETECTION_ONET_48_48_3://KNERON_LM_EYE_LID
                {
                    device_info->eye_mode_version = model_infos[i]->model_version;
                    dbg_msg_console("(%d). eye_mode_version:0x%x ", i, device_info->eye_mode_version);
                    break;
                }

                case KNERON_NIR_OCCLUSION_RES18_112_112_3://KNERON_NIR_OCCLUDE
                {
                    device_info->mask_version = model_infos[i]->model_version;
                    dbg_msg_console("(%d). mask_version:0x%x ", i, device_info->mask_version);
                    break;
                }

                default:
                {
                    dbg_msg_console("(%d). Null", i);
                    break;
                }
            }
        }
    }

    device_info->cmos_id_0 = kl520_api_camera_get_id(CAMERA_DEVICE_RGB_IDX);
    device_info->cmos_id_1 = kl520_api_camera_get_id(CAMERA_DEVICE_NIR_IDX);
    device_info->flash_id = kl520_api_memxfer_get_device_id();
    dbg_msg_console("cmos_id:0x%X, cmos_id:0x%X, flash_id:0x%X ",device_info->cmos_id_0, device_info->cmos_id_1, device_info->flash_id);

#if ( UART_PROTOCOL_VERSION >= 0x0200 )
    device_info->display_id = kl520_api_dp_get_device_id();
    device_info->touch_id = kl520_api_touch_get_device_id();
    device_info->protocol_version = kdp_comm_get_protocol_version();
    dbg_msg_console("device_id:0x%X, touch_id:0x%X",device_info->display_id, device_info->touch_id);
    dbg_msg_console("Protocol version : 0x%X", device_info->protocol_version);
#endif
    return ret;

//    device_info->led_id = (tlc59116_read_reg(TLC59116_SUB_ADDR1)<<8) + tlc59116_read_reg(TLC59116_SUB_ADDR2);
//    dbg_msg_console("device_id:0x%04x",device_info->led_id);
//    dbg_msg_console("TLC59116 addr1:0x%02x, addr2:0x%02x, addr3:0x%02x, ALLCALLADR:0x%02x", tlc59116_read_reg(TLC59116_SUB_ADDR1),tlc59116_read_reg(TLC59116_SUB_ADDR2),tlc59116_read_reg(TLC59116_SUB_ADDR3),tlc59116_read_reg(TLC59116_ALLCALLADR));
}
#endif

#if ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36515 )
extern u8 aw36515_get_id(void);
#endif
uint8_t DSM_Uart_Get_Kn_DeviceInfo(kn_device_info_data* device_info)
{
    uint8_t ret = 0;
    
    memset(device_info, 0, sizeof(kn_device_info_data));

    struct fw_misc_data ver_info;
    //TODO : get fw version and kl520 id.
    device_info->sn = kl520_api_get_unique_id();
    device_info->boot_version = kdp_sys_get_spl_version();

    dbg_msg_console("SN: 0x%X", device_info->sn);
    dbg_msg_console("boot_version: %X", device_info->boot_version);

    kl520_api_get_scpu_version(&ver_info);
    memcpy(device_info->cpu_version, ver_info.version, sizeof(device_info->cpu_version));

    kl520_api_get_ncpu_version(&ver_info);
    memcpy(device_info->npu_version, ver_info.version, sizeof(device_info->npu_version));

    {
        struct kdp_model_s model_info_s;
        struct kdp_model_s *p_model_info_s = &model_info_s;
        kdp_model_load_model(-1);
        uint8_t model_count = kdp_model_get_model_count();
        dbg_msg_console("model_count:%d",model_count);
        
        if ((sizeof(device_info->models)/sizeof(model_info)) < model_count)
        {
            dbg_msg_console("[Err]device_info->models size %d(%d) is not enough", sizeof(device_info->models)/sizeof(model_info), model_count);
        }

        for (int i = 0; i < model_count; ++i)
        {
            p_model_info_s = (struct kdp_model_s*)kdp_model_get_model_info(i);
            device_info->models[i].model_type = p_model_info_s->model_type;
            device_info->models[i].model_version = p_model_info_s->model_version;
            dbg_msg_console("(%d). model %d version: 0x%x", i, device_info->models[i].model_type, device_info->models[i].model_version);
        }
    }

    device_info->sensor_id_0 = kl520_api_camera_get_id(CAMERA_DEVICE_RGB_IDX);
    device_info->sensor_id_1 = kl520_api_camera_get_id(CAMERA_DEVICE_NIR_IDX);
    device_info->flash_id = kl520_api_memxfer_get_device_id();
    device_info->display_id = kl520_api_dp_get_device_id();
    
#if ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36515 )
    device_info->led_id = (u16)aw36515_get_id();
#endif
    
#if ( CFG_PANEL_TYPE != NULL )
//    if(t_system_info->device_id_3.id == DEVICE_NOT_INIT)
//    {
//        kl520_api_dp_open(DISPLAY_WIDTH, DISPLAY_HEIGHT);
//        t_system_info->device_id_3.id = kl520_api_dp_get_device_id();
//        kl520_api_dp_close();
//    }
#endif
    device_info->touch_id = kl520_api_touch_get_device_id();
#if ( CFG_TOUCH_ENABLE == YES )
//    if(t_system_info->device_id_4.id == DEVICE_NOT_INIT)
//    {
//        kl520_api_touch_open();
//        t_system_info->device_id_4.id = kl520_api_touch_get_device_id();
//        kl520_api_touch_stop();
//    }
#endif

    dbg_msg_console("sensor_id_0:0x%X, sensor_id_1:0x%X, flash_id:0x%X ",device_info->sensor_id_0, device_info->sensor_id_1, device_info->flash_id);

    return ret;
}

extern uint8_t DSM_Uart_Get_Version_Info(uint8_t Version[32])
{
    uint8_t ret = 0;

    struct fw_misc_data scpu_ver, ncpu_ver;

    kl520_api_get_scpu_version(&scpu_ver);
    kl520_api_get_ncpu_version(&ncpu_ver);
    
    char str_ver[32] = {0, };

#ifdef CFG_KL520_VERSION
#ifdef CFG_MODULE_NAME
    sprintf(str_ver, "%s_V%d.%02d.%03d", CFG_MODULE_NAME, scpu_ver.version[1], scpu_ver.version[2], scpu_ver.version[3]);
#else
   sprintf(str_ver, "ZF-BP3-X-25-V%d.%d.%d.%d", 
                            scpu_ver.version[0], scpu_ver.version[1], scpu_ver.version[2], scpu_ver.version[3]
                            );
#endif
#else
    sprintf(str_ver, "ZF-BP3-SV%d.%d.%d.%d-NV%d.%d.%d.%d", 
                            scpu_ver.version[0], scpu_ver.version[1], scpu_ver.version[2], scpu_ver.version[3],
                            ncpu_ver.version[0], ncpu_ver.version[1], ncpu_ver.version[2], ncpu_ver.version[3]);
#endif

    memcpy(Version, str_ver, 32);

    return ret;
}

extern uint8_t DSM_Uart_Get_Version_Info_zhian(uint8_t Version[32])
{
    uint8_t ret = 0;

    struct fw_misc_data scpu_ver, ncpu_ver;

    kl520_api_get_scpu_version(&scpu_ver);
   // kl520_api_get_ncpu_version(&ncpu_ver);
    
    char str_ver[32] = {0, };

#ifdef CFG_KL520_VERSION
    //zcy mod K520 to ZF-BP3  add ver date
		#if 1 //baikang
    sprintf(str_ver, "BH%d%d%d", 
                            
                            scpu_ver.version[0], scpu_ver.version[1], scpu_ver.version[2]
               );
		#endif
		#if 0 //chengxiangtong
		 sprintf(str_ver, "CH%d%d%d", 
                            
                            scpu_ver.version[0], scpu_ver.version[1], scpu_ver.version[2]
               );
		#endif
		#if 0 //xing chen guang 
		 sprintf(str_ver, "XH%d%d%d", 
                            
                            scpu_ver.version[0], scpu_ver.version[1], scpu_ver.version[2]
               );
		#endif
#else
    sprintf(str_ver, "ZFBP3-SV%d.%d.%d.%d-NV%d.%d.%d.%d", 
                            scpu_ver.version[0], scpu_ver.version[1], scpu_ver.version[2], scpu_ver.version[3],
                            ncpu_ver.version[0], ncpu_ver.version[1], ncpu_ver.version[2], ncpu_ver.version[3]);
#endif

    memcpy(Version, str_ver, 32);

    return ret;
}

extern uint8_t DSM_Uart_Get_Version_Info_zhian_prd(uint8_t Version[32])
{
    uint8_t ret = 0;

    struct fw_misc_data scpu_ver, ncpu_ver;

    kl520_api_get_scpu_version(&scpu_ver);
   // kl520_api_get_ncpu_version(&ncpu_ver);
    
    char str_ver[32] = {0, };


//zcy mod K520 to ZF-BP2  add ver date
    sprintf(str_ver, "ZF-BP3-X-25-%s-V%d.%d.%d.%d-(%d)", 
                            "B",
                            scpu_ver.version[0], scpu_ver.version[1], scpu_ver.version[2], scpu_ver.version[3],
                            scpu_ver.date);



    memcpy(Version, str_ver, 32);

    return ret;
}

extern uint8_t DSM_Uart_Get_Version_Info_zhian_hard(uint8_t Version[32])
{
    uint8_t ret = 0;

    struct fw_misc_data scpu_ver, ncpu_ver;

    kl520_api_get_scpu_version(&scpu_ver);
   // kl520_api_get_ncpu_version(&ncpu_ver);
    
    char str_ver[32] = {0, };


//zcy mod K520 to ZF-BP2  add ver date
    sprintf(str_ver, "BP3-V%d.%d", 
                            1, 1 );
                          



    memcpy(Version, str_ver, 32);

    return ret;
}

extern uint8_t DSM_Uart_StartDebugMode(uint8_t StartDebugMode)
{
    uint8_t ret = 0;
    //wait KL to add specific fun.
  return ret;
}

#ifdef KID_GET_DEBUG_INFO
extern uint8_t DSM_Uart_GET_DebugInfo(uint8_t debug_file_size[4])
{
    uint8_t ret = 0;
    //wait KL to add specific fun.
  return ret;
}
#endif

#ifdef KID_UPLOAD_DEBUG_INFO
extern uint8_t DSM_Uart_UploadDebugInfo(uint8_t upload_debug_info_offset[4], uint8_t upload_debug_info_size[4])
{
    uint8_t ret = 0;
    //wait KL to add specific fun.
  return ret;
}
#endif

#ifdef KID_GET_LOG_FILE
extern uint8_t DSM_Uart_GetLogFile(uint8_t log_type, uint8_t log_size[4])//lmm-add
{
    uint8_t ret = 0;
    //wait KL to add specific fun.
  return ret;
}
#endif

#ifdef KID_UPLOAD_LOG_FILE
extern uint8_t DSM_Uart_UploadLogFile(uint8_t* logdata,msg_upload_logfile_data logData)//lmm-add
{
    uint8_t ret = 0;
    //wait KL to add specific fun.
  return ret;
}
#endif


extern uint8_t DSM_Uart_SystemReset(void)//lmm-add
{
    uint8_t ret = 0;
    //wait KL to add specific fun.
    kdrv_power_sw_reset();
    return ret;
}

//extern uint8_t DSM_Uart_Get_Status_Info(uint8_t* ota_status, uint8_t next_pid_e[2])//lmm-add
//{
//    uint8_t ret = 0;
//    //wait KL to add specific fun.
//  return ret;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x10-0x3F Function
extern uint8_t KDP_Enroll_Add_Face_Type_Set(kl520_face_add_mode eType)
{
    if ( eType != FACE_ADD_MODE_1_FACE )
    {
        eType = FACE_ADD_MODE_5_FACES;
    }

    g_eEnrollAddFaceTypeMode = eType;

    if (FACE_ADD_MODE_1_FACE == g_eEnrollAddFaceTypeMode)
        g_eFaceDirState = 0;

    return MR_SUCCESS;
}

extern uint8_t DSM_Uart_GetState(uint8_t fdfr_state)
{
    uint8_t ret;

    if (fdfr_state == USER_COM_THREAD_EVENT_READY)
    {
        ret=SYS_IDLE;
    }
    else if (fdfr_state >= USER_COM_THREAD_EVENT_ENROLL)
    {
        ret=SYS_BUSY;
    }
    else if (fdfr_state == USER_COM_THREAD_EVENT_NON_OVERWRITABLE)
    {
        ret=SYS_BUSY;
    }
    else
    {
        ret=SYS_INVALID;
    }

    return ret;
}

extern uint8_t DSM_Uart_FaceReset(void)//lmm-add
{
    uint8_t ret = 0;

    g_eFaceDirState = 0;
    g_nFaceId = 0;

    //wait KL to add specific fun.
//    if(kl520_api_fdfr_exist_thread() == 1){
//        dbg_msg_console("DSM_Uart_FaceReset kl520_api_fdfr_exist_thread");
//        kl520_api_fdfr_terminate_thread();
//        kl520_api_dp_five_face_disable();
//    }
//    kdp_e2e_db_abort_reg();
    return ret;
}

extern uint8_t DSM_Uart_GetUserInfo(uint16_t userID,msg_get_user_info_data* userinfo) //lmm-add
{
    if (!kdp_app_db_check_user_id(userID))
        return MR_FAILED_INVALID_PARAM;

    uint8_t ret = MR_FAILED_UNKNOWN_USER;
    //wait KL to add specific fun.
    int i = 0;
//    u8 total_id_num;
//    u8 exist_cnt = 0;

    kdp_e2e_db_extra_data tmp;
    kdp_e2e_db_extra_data *pvars = &tmp;
    kdp_e2e_face_variables *vars = kdp_e2e_get_face_variables();
    u16 valid_fm0 = 0, valid_fm1 = 0, type = 0;

    memset(userinfo, 0, sizeof(msg_get_user_info_data));

    for ( i = 0; i < MAX_USER; i++ )
    {
        memset(&tmp, 0, sizeof(tmp));
        kdp_e2e_db_extra_read(i, &tmp, sizeof(tmp));
        s32 r1 = kdp_e2e_db_get_user_info_by_idx(i, &valid_fm0, &valid_fm1, &type);
        if(r1 != E2E_OK) continue;

        if ( 0 < valid_fm0 && TYPE_VALID == type && kdp_app_db_get_user_id(i) == userID )
        {
            userinfo->admin = pvars->admin;
            memcpy( userinfo->user_name, pvars->user_name, USER_NAME_SIZE );
            userinfo->user_id_heb=((kdp_app_db_get_user_id(i) >> 8) &0xFF);
            userinfo->user_id_leb=((kdp_app_db_get_user_id(i) >> 0) &0xFF);
            ret = MR_SUCCESS;
            dbg_msg_engineering("DSM_Uart_GetUserInfo, face_id=0x%x, Admin=%d, UseName=%s", pvars->user_id , pvars->admin, pvars->user_name);
            break;
        }
    }

    return ret;
}

extern uint8_t DSM_Uart_GetAllUserInfo(msg_get_all_user_id_data* allUserInfo) //lmm-add
{
    uint8_t ret = 0;
    int i = 0;
    u8 total_id_num;
    u8 face_status[MAX_USER];

    //u8 BigToSmallEnd = 1; //0:no change  1:BigToSmallEndien
    memset(&allUserInfo->users_id[0], 0, MAX_USER_COUNTS*2);
    kl520_api_face_query_all(&total_id_num, &face_status[0]);
    if (total_id_num > MAX_USER_COUNTS)
        total_id_num = MAX_USER_COUNTS;

    for (i = 0; i < total_id_num; i++)
    {
        //allUserInfo->users_id[i*2+BigToSmallEnd] = face_status[i];
        u16 id = ShortType_BigToSmallEnd(face_status[i]);
        memcpy(&allUserInfo->users_id[i*2], &id, sizeof(id));
        //allUserInfo->users_id[0+i*2] = ((id >>0)&0xFF);
        //allUserInfo->users_id[1+i*2] = ((id >>8)&0xFF);
        dbg_msg_engineering("User ID: 0x%04x", face_status[i]);
    }
    dbg_msg_engineering("Num Face DB:%d", total_id_num);
    allUserInfo->user_counts = total_id_num;

    return ret;
}

#if ( CFG_PRODUCTION_TEST == NO ) || ( IGNORE_PRODUCTION_TEST == YES )

msg_snap_img_data g_tSnapImgInfo = {0};
BOOL g_bUploadImageAllow = FALSE;
uint8_t SNAP_IMG_X_STRIDE = 2;
uint8_t SNAP_IMG_Y_STRIDE = 2;
#define SNAP_IMG_MAX_ID          50
uint32_t cur_image_size = 0;

extern uint8_t DSM_Uart_SnapImage(msg_snap_img_data tInfo)
{
    dbg_msg_algo ("snap img:%d,%d.", tInfo.nStrNum, tInfo.nImgCnt);
#ifdef KID_SNAP_IMAGE
    uint8_t cap_cnt = tInfo.nImgCnt ;
    if(cap_cnt == 0 || cap_cnt > SNAPSHOT_ADV_NUM) return MR_FAILED_INVALID_PARAM;
    if(tInfo.nStrNum > SNAP_IMG_MAX_ID || tInfo.nStrNum == 0) return MR_FAILED_INVALID_PARAM;
#endif
#ifdef KID_KN_SNAP_IMAGE
    uint8_t x_stride = tInfo.nImgCnt;
    uint8_t y_stride = tInfo.nStrNum;
    if(x_stride < 1 || x_stride > 10) return MR_FAILED_INVALID_PARAM;
    if(y_stride < 1 || y_stride > 10) return MR_FAILED_INVALID_PARAM;
    SNAP_IMG_X_STRIDE = x_stride;
    SNAP_IMG_Y_STRIDE = y_stride;
    tInfo.nImgCnt = 1;
    tInfo.nStrNum = 0; //hardcode
#endif

    memcpy(&g_tSnapImgInfo, &tInfo, sizeof(tInfo));
    return MR_SUCCESS;
}

extern uint8_t DSM_Uart_SavedImage(uint8_t image_num, uint8_t image_size[4])
{
    uint8_t ret = MR_SUCCESS;
    u32 img_size = 0;

#ifdef KID_GET_SAVED_IMAGE
    int start_num = g_tSnapImgInfo.nStrNum;
    if(start_num > SNAP_IMG_MAX_ID / 2) {
        start_num -= SNAP_IMG_MAX_ID / 2;
        if(image_num <= SNAP_IMG_MAX_ID / 2) return MR_FAILED_INVALID_PARAM;

        image_num -= SNAP_IMG_MAX_ID / 2;
    }
    
    if(image_num == 0 || image_num > SNAP_IMG_MAX_ID / 2) return MR_FAILED_INVALID_PARAM;

    int real_idx = 0;
    if(image_num < start_num) {
        real_idx = image_num + (SNAP_IMG_MAX_ID / 2 - start_num);
    } else {
        real_idx = image_num - start_num;
    }

    dbg_msg_algo ("saving snap image:%d,%d,%d", image_num, real_idx, g_tSnapImgInfo.nStrNum);
    if(real_idx >= g_tSnapImgInfo.nImgCnt) {
        return MR_FAILED_INVALID_PARAM;
    }

    g_bUploadImageAllow = TRUE;
    int s_ret = 0;

    if(g_tSnapImgInfo.nStrNum > SNAP_IMG_MAX_ID / 2) { //RGB
        img_size = KDP_DDR_TEST_RGB_IMG_SIZE / (SNAP_IMG_X_STRIDE * SNAP_IMG_Y_STRIDE);
        s_ret = kl520_api_save_snap_img_addr(MIPI_CAM_RGB, real_idx, SNAP_IMG_X_STRIDE, SNAP_IMG_Y_STRIDE);
    } else { //NIR
        img_size = KDP_DDR_TEST_NIR_IMG_SIZE / (SNAP_IMG_X_STRIDE * SNAP_IMG_Y_STRIDE);
        s_ret = kl520_api_save_snap_img_addr(MIPI_CAM_NIR, real_idx, SNAP_IMG_X_STRIDE, SNAP_IMG_Y_STRIDE);
    }
    
    if(s_ret < 0) return MR_FAILED_STORE_ERR;

#endif
#ifdef KID_KN_GET_SAVED_IMAGE
    
    if(image_num != 0 && image_num != 1) return MR_FAILED_INVALID_PARAM;
    g_bUploadImageAllow = TRUE;

    int s_ret = kl520_api_save_snap_img_addr(image_num, 0, SNAP_IMG_X_STRIDE, SNAP_IMG_Y_STRIDE);
    if(s_ret < 0) return MR_FAILED_STORE_ERR;
    img_size = s_ret;
#endif

    u32 tmp = kl_htonl(img_size);
    memcpy(image_size, &tmp, sizeof(tmp));
    
    cur_image_size = img_size;
    dbg_msg_console("image_num:%d, cur_image_size:%d",image_num, cur_image_size);

    return ret;
}

extern uint8_t DSM_Uart_UploadImage(uint32_t upload_image_offset, uint8_t** upload_image_data, uint32_t* image_size)
{
    uint8_t ret = MR_SUCCESS;
    u32 img_addr = KDP_DDR_TEST_RGB_IMG_ADDR;

    if((upload_image_offset + *image_size) >= cur_image_size) {
        dbg_msg_console("upload image end");
        *image_size = cur_image_size - upload_image_offset;
    }

    if ( g_bUploadImageAllow ) {
        if ( *image_size != 0) {
            img_addr += upload_image_offset;
            *upload_image_data = (UINT8 *)img_addr;
        } else {
            ret = MR_FAILED_BUF_OVERFLOW;
        }
    } else {
        ret = MR_FAILED_NO_IDX;
    }

    dbg_msg_algo("uploading img:%x, from:%d, size:%d.", img_addr, upload_image_offset, *image_size);
    return ret;
}

#if ( CFG_SNAPSHOT_ADVANCED == 1)

osThreadId_t tid_tasks_napimage = NULL;

u8 SnapImageResultReplaceToM2H(u8 eState)
{
    if ( eState == SNAPIMG_SUCCESS )
    {
        eState = MR_SUCCESS;
    }
    else if ( eState == SNAPIMG_IDX_OVERFLOW )
    {
        eState = MR_FAILED_IDX_OVERFLOW;
    }
    else if ( eState == SNAPIMG_CAM_NULL )
    {
        eState = MR_FAILED_DEV_OPEN_FAIL;
    }
    else
    {
        eState = MR_FAILED_STORE_ERR;
    }

    return eState;
}

void _tasks_snapimage_thread(void *arg)
{
    dbg_msg_console("[%s] is start", __func__);
    dbg_msg_console(" image_counts:%d; start_number:%d ", g_tSnapImgInfo.nImgCnt, g_tSnapImgInfo.nStrNum);

    uint8_t result = SNAPIMG_SUCCESS;

    if ( g_tSnapImgInfo.nStrNum+g_tSnapImgInfo.nImgCnt >=CFG_SNAPSHOT_NUMS )
    {
        result = SNAPIMG_IDX_OVERFLOW;
    }
    else
    {
        result = kl520_api_snapshot_record_img_early(g_tSnapImgInfo.nStrNum, g_tSnapImgInfo.nImgCnt, NULL, NULL);
    }

    result = SnapImageResultReplaceToM2H(result);

    tid_tasks_napimage = NULL;
    dbg_msg_console("[%s] is end", __func__);
    send_snapImage_reply_msg(result);//MR_SUCCESS
    osThreadExit();
}

#endif
#endif

u8 KDP_imp_mass_data_result_middleware_process(u8 nRet)
{
    if ( nRet == IMP_SUCCESS )
    {
        nRet = MR_SUCCESS;
    }
    else if ( nRet == IMP_CONTIUNOUS )
    {
        nRet = MR_CONTIUNOUS;
    }
    else if ( nRet == IMP_FAIL_IDX_EXISTED_USER )
    {
        nRet = MR_FAILED_EXISTED_USER;
    }
    else if ( nRet == IMP_FAIL_DB_ERR )
    {
        nRet = MR_FAILED_MASS_DATA_DB_ABNORMAL;
    }
    else if ( nRet == IMP_FAIL_FM_ERR )
    {
        nRet = MR_FAILED_MASS_DATA_FM_ABNORMAL;
    }

    return nRet;
}


void KDP_imp_fm_inejct_data(void)
{
    if ( g_tImpExpMassDataPkgInfo.nReadyType == DATA_READY_TYPE_IMP_FM )
    {
#if ( CFG_LW3D_TYPE == CFG_LW3D_NORMAL )
        if ( g_tImpExpMassDataPkgInfo.nExtData == KDP_IMP_FM_RGB )
        {
            kl520_api_ap_com_import_fm_r1_inject();
        }
        else if ( g_tImpExpMassDataPkgInfo.nExtData == KDP_IMP_FM_NIR )
#else
        if ( ( g_tImpExpMassDataPkgInfo.nExtData == KDP_IMP_FM_RGB_TO_NIR ) || 
            ( g_tImpExpMassDataPkgInfo.nExtData == KDP_IMP_FM_NIR_TO_NIR ) )
#endif
        {
            kl520_api_ap_com_import_fm_n1_inject();
        }
        else if ( g_tImpExpMassDataPkgInfo.nExtData == KDP_IMP_FM_RGB_NIR )
        {
            kl520_api_ap_com_import_fm_r1n1_inject();
        }
    }
}


void KDP_clr_mass_data_header(void)
{
    memset(&g_tImpExpMassDataPkgInfo, 0, sizeof(g_tImpExpMassDataPkgInfo));
}

#ifdef KID_SET_EXP_MASS_DATA_HEADER
#define FLASH_READ_BLOCK_SIZE       (4096*4)
void KDP_exp_mass_data(msg_mass_data_pkg tInfo, u8 eCMD)
{
    imp_exp_mass_data_info_t* ptr = &g_tImpExpMassDataPkgInfo;
    u32 nOfsIdx  = 0;
    u32 nPkgIdx  = StreamsToBigEndU16(&tInfo.nPkgIdx[0]);
    u16 nPkgSize = StreamsToBigEndU16(&tInfo.nPkgSize[0]);
    u32 nAddr = 0;
    u8  nMrState = MR_SUCCESS;

    if ( ( ptr->eType & MASS_DATA_MODE_MASK ) == MASS_DATA_EXP_MASK )  //Export mode
    {
        dbg_msg_console("|PkgIdx<TotPkgNum|PkgSize|");
        dbg_msg_console("|[%5d<%8d]|%7d|", nPkgIdx, ptr->nTotPkgNum, nPkgSize);

        if ( ptr->nReadyType == DATA_READY_TYPE_NULL )
        {
            nMrState = MR_FAILED_NOT_READY;
        }
        else if ( ( nPkgIdx <= ptr->nPkgCnt ) && ( ( ptr->nPkgCnt-nPkgIdx ) <= 1 ) &&
             ( ( ( nPkgIdx != ( ptr->nTotPkgNum-1 ) ) && ( nPkgSize == ptr->nStdPkgSize ) ) ||
               ( ( nPkgIdx == ( ptr->nTotPkgNum-1 ) ) && ( nPkgSize == ( ptr->nTotPkgSize - nPkgIdx*ptr->nStdPkgSize ) ) ) ) )
        {
#if CFG_FMAP_AP_CTRL_TYPE >= EX_FM_UART_AP_CTRL_MAIN_DB
#ifdef KID_EXP_FM_DATA
			if ( ( eCMD == KID_EXP_FM_DATA ) && ( ptr->eType == MASS_DATA_EXP_FM ) )
			{
				if ( nPkgIdx == 0 )
				{
					kl520_api_ap_com_db_catch_fm_mode();
				}
			}
			else
#endif
#ifdef KID_EXP_DB_DATA
			if ( ( eCMD == KID_EXP_DB_DATA ) && ( ptr->eType == MASS_DATA_EXP_DB ) )
			{
				if ( nPkgIdx == 0 )
				{
					kl520_api_ap_com_db_export_db_mode((u16)nPkgIdx);
				}
			}
			else
#endif
#if ( CFG_FMAP_EX_FIG_ENABLE == YES )
#ifdef KID_EXP_IMG_DATA
            if ( ( eCMD == KID_EXP_IMG_DATA ) && ( ptr->eType == MASS_DATA_EXP_IMG ) )
            {
                //Bypass
            }
			else
#endif
#endif
#endif
#ifdef KID_EXP_ALL_DB
			if ( ( eCMD == KID_EXP_ALL_DB ) && ( ptr->eType == MASS_DATA_EXP_ALL_DB ) )
			{
                //Bypass
			}
			else
#endif
#ifdef KID_EXP_FLASH
			if ( ( eCMD == KID_EXP_FLASH ) && ( ptr->eType == MASS_DATA_EXP_FW ) )
			{
                nOfsIdx = nPkgIdx/ptr->nModNum;

			    if ( ( nPkgIdx%ptr->nModNum ) == 0 )
			    {
			        nAddr = (nOfsIdx)*(FLASH_READ_BLOCK_SIZE);

	                if ( ( nPkgIdx+ptr->nModNum ) <= ptr->nTotPkgNum )
	                {
	                    kdp_memxfer_flash_to_ddr( ptr->nBuffAddr, nAddr, FLASH_READ_BLOCK_SIZE );
	                }
	                else
	                {
	                    kdp_memxfer_flash_to_ddr( ptr->nBuffAddr, nAddr, (ptr->nTotPkgNum-nPkgIdx)*ptr->nStdPkgSize );
	                }
			    }

                nOfsIdx *= ptr->nModNum;
			}
			else
#endif
			{
				nMrState = MR_FAILED_INVALID_CMD;
			}

			nAddr = ptr->nBuffAddr + ( ( nPkgIdx - nOfsIdx ) * ptr->nStdPkgSize );
        }
        else if ( ( nPkgIdx == ptr->nTotPkgNum ) && ( nPkgSize == 0 ) && ( ptr->bFinal == TRUE ) )
        {
            KDP_clr_mass_data_header();
        }
        else
        {
            nMrState = MR_FAILED_INVALID_PARAM;
        }
    }
    else
    {
        nMrState = MR_FAILED_MASS_DATA_HEAD_EMPTY;
    }

    if ( nMrState == MR_SUCCESS )
    {
        send_exp_mass_data_reply_msg(nMrState, eCMD, nAddr, nPkgSize);

        if ( nPkgIdx == ptr->nPkgCnt )
        {
            ptr->nPkgCnt++;

            if ( ptr->nPkgCnt == ptr->nTotPkgNum )
            {
                ptr->bFinal = TRUE;
            }
        }
    }
    else
    {
        send_exp_mass_data_reply_msg(nMrState, eCMD, 0, 0);
    }
}
#endif

#ifdef KID_IMP_FM_DATA
void KDP_imp_mass_data(msg_mass_data_pkg* tInfo, u8 eCMD)
{
    imp_exp_mass_data_info_t* ptr = &g_tImpExpMassDataPkgInfo;
    u32 nPkgIdx  = StreamsToBigEndU16(&tInfo->nPkgIdx[0]);
    u16 nPkgSize = StreamsToBigEndU16(&tInfo->nPkgSize[0]);
    u8  nMrState = MR_SUCCESS;

    if ( ( ptr->eType & MASS_DATA_MODE_MASK ) == MASS_DATA_IMP_MASK ) { //Import mode
        dbg_msg_console("pkt idx:%d, total num:%d, pkt size:%d.", nPkgIdx, ptr->nTotPkgNum, nPkgSize);

        if ( ptr->bFinal == TRUE )
        {
            nMrState = MR_FAILED_EXISTED_USER;
        } else if ( ( nPkgIdx <= ptr->nPkgCnt ) &&
                  ( ( ( nPkgIdx != ( ptr->nTotPkgNum-1 ) ) && ( nPkgSize == ptr->nStdPkgSize ) ) ||
                    ( ( nPkgIdx == ( ptr->nTotPkgNum-1 ) ) && ( nPkgSize == ( ptr->nTotPkgSize - nPkgIdx*ptr->nStdPkgSize ) ) ) ) ) {
            if ( ( eCMD == KID_IMP_FM_DATA ) && ( ptr->eType == MASS_DATA_IMP_FM ) )
            {
                nMrState = kl520_api_ap_com_import_fm_mode_split(nPkgIdx, nPkgSize, ( ptr->nTotPkgNum-1 ), ptr->nStdPkgSize, 
                    ptr->nTotPkgSize, tInfo->pDataHeader, 0);
                nMrState = KDP_imp_mass_data_result_middleware_process(nMrState);
            } else {
                nMrState = MR_FAILED_INVALID_CMD;
            }
        } else {
            nMrState = MR_FAILED_INVALID_PARAM;
        }
    } else {
        nMrState = MR_FAILED_MASS_DATA_HEAD_EMPTY;
    }

    send_exp_mass_data_reply_msg(nMrState, eCMD, 0, 0);

    if ( ( nMrState == MR_SUCCESS ) || ( nMrState == MR_CONTIUNOUS ) ) {
        if ( nPkgIdx == ptr->nPkgCnt ) {
            ptr->nPkgCnt++;

            if ( ptr->nPkgCnt == ptr->nTotPkgNum ) {
                ptr->bFinal = TRUE;
            }
        }

        if ( ptr->bFinal == TRUE ) {
            if ( eCMD == KID_IMP_FM_DATA ) {
                ptr->nReadyType = DATA_READY_TYPE_IMP_FM;
            } else {
                KDP_clr_mass_data_header();
            }
            send_exp_mass_data_done_note_msg(MR_SUCCESS);
        }
    }
}
#endif

#ifdef KID_SW_EXP_FM_MODE
uint8_t KDP_export_fm_mode( eEXTRA_FMAP_TYPE mode_idx )
{
    uint8_t ret = 0;

    if ( mode_idx >= EXTRA_FMAP_SIZE_e )
    {
        ret = MR_FAILED_INVALID_PARAM;
    }
    else
    {
        kl520_api_ap_com_db_enable_fm_extra_mode( mode_idx );
        ret = MR_SUCCESS;
    }

    return ret;
}
#endif

#ifdef KID_SW_EXP_DB_MODE
uint8_t KDP_export_db_mode( eEXPORT_DB_TYPE mode_idx )
{
    uint8_t ret = 0;

    if ( mode_idx >= EXPORT_DB_MODE_DB_SIZE )
    {
        ret = MR_FAILED_INVALID_PARAM;
    }
    else
    {
        kl520_api_ap_com_db_enable_export_db_mode( mode_idx );
        ret = MR_SUCCESS;
    }

    return ret;
}
#endif

#if CFG_SNAPSHOT_ENABLE == 2
u8 KDP_catch_image_mode(u16 nSrcType)
{
    u8  nRet = MR_SUCCESS;

    if ( g_tImpExpMassDataPkgInfo.eType == MASS_DATA_EXP_IMG )
    {
        nRet = kl520_api_ap_com_snapshot_catch(nSrcType);

        if ( nRet == 1 )
        {
            g_tImpExpMassDataPkgInfo.nReadyType = DATA_READY_TYPE_EXP_IMG;
            nRet = MR_SUCCESS;
        }
        else
        {
            nRet = MR_FAILED_INVALID_PARAM;
        }
    }
    else
    {
        nRet = MR_REJECTED;
    }

    return nRet;
}
#endif

//u32 KDP_get_customer_exp_img_size(u16 nType)
//{
//    if ( nType == KDP_CATCH_RGB_IMG )
//    {
//#if ( CUSTOMER_CATCH_EXP_IMG_SCALE_DOWN_DIV > 1 ) && ( CUSTOMER_CATCH_EXP_IMG_SCALE_DOWN_DIV%2 == 0 )
//        return (RGB_IMG_SOURCE_W/CUSTOMER_CATCH_EXP_IMG_SCALE_DOWN_DIV)*(RGB_IMG_SOURCE_H/CUSTOMER_CATCH_EXP_IMG_SCALE_DOWN_DIV)*2;
//#else
//        return (KDP_DDR_TEST_RGB_IMG_SIZE);
//#endif
//    }
//    else if ( nType == KDP_CATCH_NIR_IMG )
//    {
//#if ( CUSTOMER_CATCH_EXP_IMG_SCALE_DOWN_DIV > 1 ) && ( CUSTOMER_CATCH_EXP_IMG_SCALE_DOWN_DIV%2 == 0 )
//        return (NIR_IMG_SOURCE_W/CUSTOMER_CATCH_EXP_IMG_SCALE_DOWN_DIV)*(NIR_IMG_SOURCE_H/CUSTOMER_CATCH_EXP_IMG_SCALE_DOWN_DIV);
//#else
//        return (KDP_DDR_TEST_NIR_IMG_SIZE);
//#endif
//    }
//    else
//    {
//        return 0;
//    }
//}

//this func only handle imp feature map data
u8 KDP_set_mass_data_header(msg_mass_data_header* tHeader)
{
    uint8_t mode = tHeader->mode_type;
    uint16_t ext_data = StreamsToBigEndU16(&(tHeader->ext_data[0]));
    
    if(ext_data == KDP_IMP_FM_RGB) ext_data = KDP_IMP_FM_RGB_TO_NIR;
    
    if( (mode != MASS_DATA_IMP_FM) || 
        (ext_data != KDP_IMP_FM_RGB_TO_NIR && ext_data != KDP_IMP_FM_NIR_TO_NIR) ) {
            return MR_FAILED_INVALID_PARAM;
    }

    KDP_clr_mass_data_header();
    imp_exp_mass_data_info_t* ptr = &g_tImpExpMassDataPkgInfo;

    //memcpy(&(ptr->tHeaderInfo), tHeader, sizeof(msg_mass_data_header));

    ptr->nStdPkgSize = StreamsToBigEndU16(&tHeader->pkt_size[0]);
    ptr->nTotPkgNum  = StreamsToBigEndU32(&tHeader->num_pkt[0]);
    ptr->nTotPkgSize = StreamsToBigEndU32(&tHeader->fsize_b[0]);
    ptr->eType       = (mass_data_type)mode;
    ptr->nExtData    = ext_data;

    dbg_msg_console("pkt size:%d, num pkt:%d, total size:%d, type:%d, ext:%d.", 
        ptr->nStdPkgSize, ptr->nTotPkgNum, ptr->nTotPkgSize, ptr->eType, ptr->nExtData);

    return MR_SUCCESS;
}

extern uint8_t DSM_Uart_StartOrStopDemoMode(uint8_t Enable)
{
    uint8_t ret = 0;

    if( Enable == TRUE )
    {
        g_bUserDemoMode = TRUE;
        dbg_msg_console("Enable DemoMode");
    }
    else
    {
        g_bUserDemoMode = FALSE;
        dbg_msg_console("Disable DemoMode");
    }

    //wait KL to add specific fun.
  return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x40-0x6F Unit control
#ifdef KID_TURN_OFF_CAMERA
extern uint8_t DSM_Uart_TurnOnOffCamera(u8 nIdx, u8 nCtrl)
{
    uint8_t ret = MR_FAILED_INVALID_PARAM;

    if ( nIdx < IMGSRC_NUM )
    {
        kl520_api_hmi_ctrl_state_set(CTRL_CMD);
        ret = kl520_api_cam_disp_ctrl(nCtrl, nIdx, PERMANENT_NULL);
        kl520_api_hmi_ctrl_state_reset(CTRL_COMM);
    }

    return ret;
}
#endif

#ifdef KID_TURN_ON_VIS_LED
extern uint8_t DSM_Uart_TurnOnOffVisLed(BOOL bSw)
{
    uint8_t ret = 0;

    if ( bSw == TRUE )
    {
        u16 level = CFG_E2E_RGB_LED_STRENGTH;
        rgb_led_open((u16)level);
    }
    else
    {
        rgb_led_close();
    }

    return ret;
}
#endif

#ifdef KID_TURN_ON_IR_LED
extern uint8_t DSM_Uart_TurnOnOffIrLed(BOOL bSw)
{
    uint8_t ret = 0;

    if ( bSw == TRUE )
    {
        u16 level = CFG_E2E_NIR_LED_STRENGTH;
        nir_led_open(level);
    }
    else
    {
        nir_led_close();
    }

    return ret;
}
#endif

#ifdef KID_TURN_ON_STRUCT_LED
extern uint8_t DSM_Uart_TurnOnOffStructLed(BOOL bSw)
{
    uint8_t ret = 0;

    if ( bSw == TRUE )
    {
//        u16 level = CFG_E2E_NIR_LED_STRENGTH;
//        nir_led_open(level);
    }
    else
    {
//        nir_led_close();
    }

    return ret;
}
#endif

#ifdef KID_TURN_ON_PANEL
extern uint8_t DSM_Uart_TurnOnOffPanel(u8 nCtrl)
{
    uint8_t ret = 0;

    kl520_api_hmi_ctrl_state_set(CTRL_CMD);
    ret = kl520_api_cam_disp_ctrl(nCtrl, NULL, PERMANENT_NULL);
    kl520_api_hmi_ctrl_state_reset(CTRL_COMM);

    return ret;
}
#endif

#ifdef KID_SET_EXP_TIME
uint8_t kn_uart_set_exposure_time( s_msg_exp_time tInfo )
{
    uint8_t ret = 0;
    
    u8 nCamIdx  = tInfo.cam_idx;
    u32 nExpTime = ( StreamsToBigEndU16(&tInfo.exposure_time[0]) << 16 ) | StreamsToBigEndU16(&tInfo.exposure_time[2]);
    dbg_msg_console("[exposure] cam = %d, exp = 0x%x", nCamIdx, nExpTime);
    
    if (kl520_api_set_exposure_only( nCamIdx, nExpTime ))
        ret = MR_REJECTED;
    else
        ret = MR_SUCCESS;
    
    return ret;
}
#endif

#ifdef KID_SET_GAIN
u8 kn_uart_set_gain(u8 cam_index, u16 gain)
{
    uint8_t ret = 0;

    if (gain > (u16)MAX_GAIN)
        gain = MAX_GAIN;
    else if (gain < (u16)MIN_GAIN)
        gain = MIN_GAIN;

    u8 gain_h = (gain >> 8) & 0xff;
    u8 gain_l = gain & 0xff;
    
    dbg_msg_console("[set gain] cam = %d, gain_h:0x%02x, gain_l:0x%02x", cam_index, gain_h, gain_l);
    if (kdp_camera_set_gain(cam_index, gain_h, gain_l))
        ret = MR_REJECTED;
    else
        ret = MR_SUCCESS;

    if (cam_index == MIPI_CAM_NIR)
        nir_sensor_rst((u8)NIR_LED_WAIT_FRAME);
    else
        rgb_sensor_rst((u8)RGB_LED_WAIT_FRAME);
    
    return ret;
}
#endif

extern uint8_t DSM_Uart_SET_THRESHOLD_LEVEL(uint8_t verify_level,uint8_t live_level)
{
    if( (verify_level > 4) || 
        (live_level < 50) || 
        (live_level > 95))
        return MR_FAILED_INVALID_PARAM;

    kdp_e2e_set_fr_threshold_level(verify_level);
    
    kdp_set_rgb_to_nir_ratio(live_level);

    dbg_msg_console("Set fr th: %d, lv th: %d", verify_level, live_level);

    return MR_SUCCESS;
}

extern uint8_t DSM_Uart_ConfigBaudrate(uint8_t baudrate_index)//lmm-add
{
    uint8_t ret = 0;
    uint8_t kl520_idx = 0;
//    if(rate == 0)     {baudrate = BAUD_115200;}
//    else if(rate == 1){baudrate = BAUD_921600;}
//    else if(rate == 2){baudrate = BAUD_460800;}
//    else if(rate == 3){baudrate = BAUD_230400;}
//    else if(rate == 4){baudrate = BAUD_1500000;}

 //wait KL to add specific fun.
 //1:115200
 //2:230400
 //3:460800
 //4:1500000
    if(baudrate_index == 1) {       kl520_idx = 0; ret = MR_SUCCESS;   dbg_msg_console("BAUD_115200"); }//115200
    else if(baudrate_index == 2){   kl520_idx = 3; ret = MR_SUCCESS;   dbg_msg_console("BAUD_234000"); }//234000
    else if(baudrate_index == 3){   kl520_idx = 2; ret = MR_SUCCESS;   dbg_msg_console("BAUD_460800"); }//460800
    else if(baudrate_index == 4){   kl520_idx = 4; ret = MR_REJECTED;   dbg_msg_console("BAUD_1500000"); }//1500000
    else if(baudrate_index == 5){   kl520_idx = 1; ret = MR_SUCCESS;   dbg_msg_console("BAUD_921600"); }//921600
    else{                                          ret = MR_REJECTED;  dbg_msg_console("Not supported BAUD"); }


    if(ret == MR_SUCCESS){
        send_ConfigBaurate_reply_msg(MR_SUCCESS);
        osDelay(50);//delay 50ms
        kl520_com_reconfig_baud_rate(kl520_idx);
        dbg_msg_engineering("baudrate index:%d Pass",baudrate_index);
    }
    else{
        send_ConfigBaurate_reply_msg(MR_REJECTED);
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x70-0x7F MP
#ifdef KID_SYS_INIT_READY_TIME
extern uint8_t DSM_Uart_SysInitReadyTime(uint8_t time[4])
{
    uint8_t ret = MR_SUCCESS;

    uint32_t TempT = IntType_BigToSmallEnd(g_nSysReadyTime);
    memcpy(time,&TempT,sizeof(time));

    dbg_msg_console("%s = %d", __func__, g_nSysReadyTime);
  return ret;
}
#endif

#define SCPU_PART_A (0)
#define SCPU_PART_B (1)

static void SwitchScpu(void)
{
    ota_update_show_config();
    kl520_api_ota_switch_SCPU();
    ota_update_show_config();
}

uint8_t DSM_Uart_SwitchPart(uint8_t nSwPart, uint8_t* nCurPart)
{
    uint8_t ret = MR_SUCCESS;
    *nCurPart = (u8)ota_get_scpu_flag_status();

    dbg_msg_console("Current/Switch part: %d/%d",*nCurPart, nSwPart);

    if ( nSwPart == SCPU_PART_B )
    {
        if( *nCurPart == SCPU_PART_A )
        {
            SwitchScpu();   //Stop in this function...
        }
        else
        {
            dbg_msg_console("Already in B");
            ret = MR_REJECTED;
        }
    }
    else if( nSwPart == SCPU_PART_A )    //  B(1) to A(0)
    {
        if( *nCurPart == SCPU_PART_B )
        {
            SwitchScpu();   //Stop in this function...
        }
        else
        {
            dbg_msg_console("Already in A");
            ret = MR_REJECTED;
        }
    }
    else
    {
        dbg_msg_console("Invalid parameter");
        ret = MR_FAILED_INVALID_PARAM;
    }

    return ret;
}

uint8_t DSM_Uart_Get_Cur_Part(uint8_t* nCurPart)
{
    uint8_t ret = MR_SUCCESS;

    *nCurPart = (u8)ota_get_scpu_flag_status();

    if( *nCurPart > 1 )
    {
        ret = MR_REJECTED;
    }
    else
    {
        dbg_msg_console("Current SCPU in %d part.", *nCurPart);
    }
    return ret;
}

#ifdef KID_MP_CALIBRATION
uint8_t KDP_Uart_MP_Calibration(u8 eMode)
{
    uint8_t ret = MR_SUCCESS;

    u8 tm = 10; //default 10
    if(eMode != 0) tm = eMode; //mp timeout value
    ret = kl520_engineering_calibration(tm, NULL);

    if ( ret != MR_SUCCESS )
    {
        ret = MR_REJECTED;
    }

    return ret;
}
#endif

//extern uint8_t DSM_Uart_StartSnapPics(void)//lmm-add
//{
//    uint8_t ret = 0;
//    //wait KL to add specific fun.
//  return ret;
//}
//extern uint8_t DSM_Uart_StopSnapPics(void)//lmm-add
//{
//    uint8_t ret = 0;
//    //wait KL to add specific fun.
//  return ret;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0xF0-0xF7 Key
#if ( ENCRYPTION_MODE&AES_ENCRYPTION ) || ( ENCRYPTION_MODE&XOR_ENCRYPTION )
extern uint8_t DSM_Uart_Init_Encryption(msg_init_encryption_data encryption,uint8_t device_id[DEVICE_ID_NUM])
{
#if 0
    //read releaseKey opt.
    kl520_customer_info Cusinfo;
    kl520_api_customer_get(&Cusinfo);
    if(strcmp(Cusinfo.nReleaseKey,"0") || Cusinfo.nReleaseKey!=NULL)
    {

            memcpy(key_num, &Cusinfo.nReleaseKey[0], KEY_SIZE);
            if(!strcmp(key_num,"0") || key_num==NULL)
            {
                memset(key_num,0x0f,KEY_SIZE);
            }
            int i = 0;
            for(i=0;i<KEY_SIZE;i++)
            {
                dbg_msg_console("nReleaseKey[%d]=0x%02x",i,Cusinfo.nReleaseKey[i]);
                dbg_msg_console("key_num[%d]=0x%02x",i,key_num[i]);
            }
    }

#else
    kl520_customer_info Cusinfo;
    kl520_api_customer_get(&Cusinfo);
    if (( kl520_api_customer_chk_key_exist(&Cusinfo.nReleaseKey[0], KEY_SIZE) == TRUE ) && (!use_debug_key))
    {
//        dbg_msg_console("Load Key...");
        memcpy(key_num, &Cusinfo.nReleaseKey[0], KEY_SIZE);
    }
    else if ( kl520_api_customer_chk_key_exist(&key_num[0], KEY_SIZE) == FALSE )
    {
//        dbg_msg_console("No Key...");
        return MR_REJECTED;
    }
#endif

    system_info t_sys_info = { 0 };
    //ret = kl520_api_get_device_info(&t_sys_info);
    t_sys_info.unique_id = kdp_sys_get_unique_id();
    dbg_msg_com("unique_id : %x", t_sys_info.unique_id);
    memset(device_id, 0,DEVICE_ID_NUM);
    uint32_t real_device_id = IntType_BigToSmallEnd(t_sys_info.unique_id);
    memcpy(device_id, &real_device_id, sizeof(t_sys_info.unique_id) );

    uint8_t i = 0;
    memset(device_id+sizeof(t_sys_info.unique_id), 0, DEVICE_ID_NUM-sizeof(t_sys_info.unique_id));
    uint8_t key_md5[KEY_SIZE] = "0";
    uint8_t str_md5[KEY_SIZE*2] = "0";
    md5_main( encryption.seed, key_md5, ARRAY_SIZE(encryption.seed) );
    md5_HexDevide2DoubleHex( str_md5, key_md5, KEY_SIZE );
    memset(debug_key,0,KEY_SIZE);
    for(i=0;i<KEY_SIZE;i++)
    {
        debug_key[i] = str_md5[key_num[i]];
        if((0x0a<=debug_key[i] && debug_key[i]<=0x0f) || (0x0A<=debug_key[i] && debug_key[i]<=0x0F) )//|| ('a'<=debug_key[i] && debug_key[i]<='f') || ('A'<=debug_key[i] && debug_key[i]<='Z'))
        {
            debug_key[i] = debug_key[i] + 87;
        }
        else if(debug_key[i]<=0x09)
        {
            debug_key[i] = debug_key[i] + 0x30;
        }
        else
            dbg_msg_console("not recognized char:0x%02x",debug_key[i]);
    }

    g_nEncryptionMode = (XOR_ENCRYPTION == encryption.mode)?XOR_ENCRYPTION:AES_ENCRYPTION;

    dbg_msg_com("debug_key=%s",debug_key);

    return MR_SUCCESS;
}

extern uint8_t DSM_Uart_SetReleaseEncKey(msg_enc_key_number_data key_number)
{
    uint8_t ret = 0;
    //wait KL to add specific fun.
    memcpy((void*)key_num, (void*)key_number.enc_key_number, sizeof(key_number.enc_key_number));
    kl520_customer_info Cusinfo;
    kl520_api_customer_get(&Cusinfo);
    if (memcmp(Cusinfo.nReleaseKey, key_num,KEY_SIZE))
    {
        dbg_msg_console("Save release key to flash");
        memcpy(Cusinfo.nReleaseKey, key_num,KEY_SIZE);
        kl520_api_customer_write(&Cusinfo);
    }
    use_debug_key = false;
    return ret;
}

extern uint8_t DSM_Uart_SetDebugEncKey(msg_enc_key_number_data key_number)
{
    uint8_t ret = 0;
    //wait KL to add specific fun.
    memcpy(key_num, key_number.enc_key_number, sizeof(key_number.enc_key_number));
    use_debug_key = true;
    return ret;
}

#ifdef KID_SET_INTER_ACTIVATE
extern uint8_t DSM_Uart_SET_Interactivate(msg_interactivate_param interactivate_param)
{
    uint8_t ret = 0;
    //wait KL to add specific fun.
    return ret;
}
#endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0xF8-0xFF OTA
extern uint8_t DSM_Uart_StartOta(uint8_t v_primary, uint8_t v_secondary,uint8_t v_revision)//lmm-add
{
    //read version from flash
    struct fw_misc_data fw_misc;
    kl520_api_get_scpu_version(&fw_misc);

    //u8 cur_release_v = fw_misc.version[0];
    u8 cur_v_primary = fw_misc.version[1];
    u8 cur_v_secondary = fw_misc.version[2];
    u8 cur_v_revision = fw_misc.version[3];

    //check version
    dbg_msg_console("Current version: %d.%d.%d", cur_v_primary, cur_v_secondary, cur_v_revision);
    dbg_msg_console("Ota version:     %d.%d.%d", v_primary, v_secondary, v_revision);
    if ((v_secondary == cur_v_secondary) && (v_primary == cur_v_primary) && (v_revision == cur_v_revision))
    {
        dbg_msg_console ("version is same!");
        return MR_FAILED_INVALID_PARAM;
    }
    
#if ( UART_PROTOCOL_VERSION >= 0x201 )
    //2: check update mode or version
    if ( v_primary == KDP_OTA_VERSION )
    {
        ota_update_info.scpu_update = v_revision == cur_v_revision? NO : YES;//v_revision;
        ota_update_info.ncpu_update = v_secondary == cur_v_secondary? NO : YES;//v_secondary;
        ota_update_info.ui_update = NO;//v_primary;

        if ( ota_update_info.scpu_update == NO )  //same version or update ncpu only
        {
            return MR_FAILED_INVALID_PARAM;
        }
    }
    else if ( v_primary == KDP_OTA_SCPU )
    {
        ota_update_info.scpu_update = YES;
        ota_update_info.ncpu_update = NO;
        ota_update_info.ui_update = NO;
    }
    else if ( v_primary == KDP_OTA_SCPU_NCPU )
    {
        ota_update_info.scpu_update = YES;
        ota_update_info.ncpu_update = YES;
        ota_update_info.ui_update = NO;
    }
    else if ( v_primary == KDP_OTA_SCPU_UI )
    {
        ota_update_info.scpu_update = YES;
        ota_update_info.ncpu_update = NO;
        ota_update_info.ui_update = YES;
    }
    else
    {
        return MR_FAILED_INVALID_PARAM;
    }
#else
    ota_update_info.scpu_update = NO;
    ota_update_info.ncpu_update = NO;
    ota_update_info.ui_update = NO;
#endif

    ota_update_info.ota_process = OTA_START_PROCESS_FLAG; // dsm ota start
    ota_update_info.packet_id = 0;
    ota_update_info.ddr_addr_idx = 0;
    ota_update_info.ota_status = OTA_STATUS_START;
    
    g_nEncryptionMode = NO_ENCRYPTION;

    return MR_SUCCESS;
}

extern uint8_t DSM_Uart_GetOtaStatus(uint8_t* ota_status, uint8_t next_pid_e[2])
{
    //uint8_t ret = 0;
    //wait KL to add specific fun.

    if( ota_update_info.ota_process == OTA_START_PROCESS_FLAG )
    {
        next_pid_e[0] = ( (ota_update_info.packet_id & 0xFF00) >> 8) ;
        next_pid_e[1] = ( (ota_update_info.packet_id & 0x00FF) >> 0) ;
    }
    else
    {
        next_pid_e[0] = next_pid_e[1] = 0;
    }
    dbg_msg_console("[%s] packet_id: %d", __func__, ota_update_info.packet_id );

    *ota_status = ota_update_info.ota_process;

    return MR_SUCCESS;
}

extern bool user_com_thread_event_ota_copy_to_flash(void);
extern uint8_t DSM_Uart_OtaHeader(msg_mass_data_header* otaheader)//lmm-add
{
    if ((ota_update_info.ota_process != OTA_START_PROCESS_FLAG) || \
        (ota_update_info.ota_status != OTA_STATUS_START))
    {
        return MR_REJECTED;
    }

    memcpy( &ota_update_info.header_info, (void*)otaheader, sizeof(msg_mass_data_header) );

    u16 num_pkt = (ota_update_info.header_info.num_pkt[2] << 8) + (ota_update_info.header_info.num_pkt[3]);
    if((ota_update_info.header_info.num_pkt[0] != 0) || \
        (ota_update_info.header_info.num_pkt[1] != 0) || \
        (num_pkt == 0))
    {
        return MR_FAILED_INVALID_PARAM;
    }

    u32 fsize = (((u32)ota_update_info.header_info.fsize_b[0]) << 24) + (((u32)ota_update_info.header_info.fsize_b[1]) << 16) \
                    + (((u32)ota_update_info.header_info.fsize_b[2]) << 8) + ((u32)ota_update_info.header_info.fsize_b[3]);

    u16 pkt_size = ( ota_update_info.header_info.pkt_size[0] << 8 ) + ota_update_info.header_info.pkt_size[1];

    if (fsize <= 1024)
        return MR_FAILED_INVALID_PARAM;
    
    ota_update_info.total_pid = num_pkt;
    ota_update_info.total_update_size = fsize;
    
    u32 tmp_addr = kdp_ddr_reserve(fsize);
    if(tmp_addr == 0) {
        //ddr mem not enough
        ota_update_info.ota_ddr_start_addr = OTA_START_DDR_ADDR;
        ota_update_info.ota_mode = 0;
    } else {
        //ota_update_info.ota_ddr_start_addr = OTA_START_DDR_ADDR;
        ota_update_info.ota_mode = 1;
        ota_update_info.ota_ddr_start_addr = tmp_addr;
    }
    
    dbg_msg_console ("ota ddr mem addr:%x", ota_update_info.ota_ddr_start_addr);
    //dbg_msg_console("[%s]", __func__ );
    dbg_msg_console("fsize: 0x%X , num_pkt: 0x%X , p_size: 0x%X", fsize, num_pkt, pkt_size);

    dbg_msg_nocrlf("md5:");
    for (int i = 0; i < 32; i++) dbg_msg_nocrlf (" %02X", ota_update_info.header_info.md5_sum[i]);
    dbg_msg_console("\n");
    
    //set to ota state
    if (ota_update_info.ota_mode == 0) {
        bool fl = user_com_thread_event_ota_copy_to_flash();
        if(fl == TRUE) {
            //status is not correct
            return MR_REJECTED;
        }
    }

    ota_update_info.ota_status = OTA_STATUS_HEADER;

    return MR_SUCCESS;
}

extern uint8_t DSM_Uart_OtaPacket(msg_mass_data_pkg otapacket)//lmm-add
{
    if ((ota_update_info.ota_status != OTA_STATUS_HEADER) && \
        (ota_update_info.ota_status != OTA_STATUS_PACKET))
    {
        return MR_REJECTED;
    }

    uint16_t pid = (((u16)otapacket.nPkgIdx[0]) << 8) + otapacket.nPkgIdx[1];
    dbg_msg_console("ota pkt id :%d", pid);

    if( ota_update_info.packet_id != pid || ota_update_info.total_pid < pid )
    {
        return MR_FAILED_INVALID_PARAM;
    }

    uint16_t buf_size = (((u16)otapacket.nPkgSize[0]) << 8) + otapacket.nPkgSize[1];
    if( ota_update_info.total_update_size < ota_update_info.ddr_addr_idx + buf_size )
    {
        return MR_FAILED_INVALID_PARAM;
    }

    memcpy( (void*)(ota_update_info.ota_ddr_start_addr + ota_update_info.ddr_addr_idx), 
        (void*)otapacket.pDataHeader, buf_size );

    ota_update_info.ddr_addr_idx+=buf_size;
    ota_update_info.packet_id++;

    ota_update_info.ota_status = OTA_STATUS_PACKET;

    if( Ota_final_packet() == 1 )
    {
        ota_update_info.ota_status = OTA_STATUS_FINAL;
        dbg_msg_console("Get all ota packets...");
        
        //set to ota state
        if ( ota_update_info.ota_mode == 1) {
            bool fl = user_com_thread_event_ota_copy_to_flash();
            if(fl == TRUE) {
                //status is not correct
                return MR_REJECTED;
            }
        }
        //start ota copy to flash...
        user_com_event_start(USER_COM_FLAG_OTA_PROCESS);

    }

    return MR_SUCCESS;
}

extern uint8_t Ota_final_packet( void )
{
    return (ota_update_info.ddr_addr_idx == ota_update_info.total_update_size);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----ENROLL
void _api_face_note(u8 ret)
{
//    static u32 _gface_note_time = 0xFFFF;
//    if(osKernelGetTickCount()-_gface_note_time > MSG_INTERNAL_TIME_NOTE)
    {
        s_msg_note_data_face msg_note;
        u8 nid = NID_UNKNOWN_ERROR;
        s32 msg_x, msg_y, msg_w, msg_h;
        u8 en_note_msg = FALSE;
        memset(&msg_note,0,sizeof(msg_note));

        if(ret == KL520_FACE_TOO_FAR){              msg_note.state = FACE_STATE_TOOFAR;             en_note_msg = TRUE; }
        else if(ret == KL520_FACE_TOO_NEAR){        msg_note.state = FACE_STATE_TOOCLOSE;           en_note_msg = TRUE; }
        else if(ret == KL520_FACE_TOOUP){           msg_note.state = FACE_STATE_TOOUP;              en_note_msg = TRUE; }
        else if(ret == KL520_FACE_TOODOWN){         msg_note.state = FACE_STATE_TOODOWN;            en_note_msg = TRUE; }
	      else if(ret == KL520_FACE_TOOLEFT){         msg_note.state = FACE_STATE_TOORIGHT;            en_note_msg = TRUE; }//zcy mod left to rignht
        else if(ret == KL520_FACE_TOORIGHT){        msg_note.state = FACE_STATE_TOOLEFT;           en_note_msg = TRUE; } //zcy mod right to left
        else if(ret == KL520_FACE_MASK){            msg_note.state = FACE_STATE_FACE_OCCLUSION;     en_note_msg = TRUE; }
        else if(ret == KL520_FACE_EYE_CLOSE_STATUS_OPEN_EYE){   msg_note.state = FACE_STATE_EYE_CLOSE_STATUS_OPEN_EYE;  en_note_msg = TRUE;}
        else if(ret == KL520_FACE_EYE_CLOSED){                  msg_note.state = FACE_STATE_EYE_CLOSE_STATUS;           en_note_msg = TRUE;}
        else if(ret == KL520_FACE_EYE_CLOSE_UNKNOW_STATUS){     msg_note.state = FACE_STATE_EYE_CLOSE_UNKNOW_STATUS;    en_note_msg = TRUE;}
        else if(ret == KL520_FACE_NOFACE){          msg_note.state = FACE_STATE_NOFACE;             en_note_msg = TRUE; }
        else if(ret == KL520_FACE_OK){              msg_note.state = FACE_STATE_NORMAL;             en_note_msg = FALSE;}
        else if(ret == KL520_FACE_BADPOSE){         msg_note.state = FACE_STATE_DIRECTION_ERROR;    en_note_msg = TRUE;}
        else if(ret == KL520_FACE_WAIT_DONT_MOVE){  msg_note.state = FACE_STATE_NORMAL;             en_note_msg = FALSE;}
        else if(ret == KL520_FACE_FAIL){                                                            en_note_msg = FALSE;}
        else if(ret == KL520_FACE_INVALID){                                                         en_note_msg = FALSE;}
        else{  dbg_msg_console("[%s], ret=0x%x",__func__, ret);                                     en_note_msg = FALSE;}

        if(en_note_msg == TRUE)
        {
            nid = NID_FACE_STATE;
            if( kdp_e2e_util_get_person_position(&msg_x, &msg_y, &msg_w, &msg_h) == 1)
            {
                struct video_input_params params;
                kdp_video_renderer_get_params(&params);
                int dp_w = (int)params.dp_out_w;
                int dp_h = (int)params.dp_out_h;
                int img_w = (int)params.dp_area_w;
                int img_h = (int)params.dp_area_h;

                msg_x = msg_x * dp_w / img_w;
                msg_y = msg_y * dp_h / img_h;
                msg_w = msg_w * dp_w / img_w;
                msg_h = msg_h * dp_h / img_h;

                dbg_msg_algo("[FACE_POS] (msg_x, msg_y)= (%d, %d)", msg_x, msg_y);

                msg_note.top = msg_y ;
                msg_note.bottom = msg_y + msg_h;
                msg_note.left = msg_x;
                msg_note.right = msg_x + msg_w;

                msg_note.pitch = 0;
                msg_note.yaw = 0;
                msg_note.roll = 0;
            }

            if ( ( g_bStopSendMsg == FALSE ) && ( (osKernelGetTickCount()-g_nSysInternalTime) > SYS_INTERNAL_TIME_NOTE_INTERVAL ) )
            {
                send_EnrollOrVerify_note_msg(msg_note ,nid);
                g_nSysInternalTime = osKernelGetTickCount();
                //dbg_msg_console("[%s], osKernelGetTickCount()=%d",__func__, osKernelGetTickCount());
//                _gface_note_time = osKernelGetTickCount();
            }
        }
    }
}

//Refer to kl520_api_face_add_internal
int _face_add_internal(short x, short y, short w, short h, u8 type)
{
    int ret = KL520_FACE_OK;
    int face_ret = 0;
    
    u8 f_bmp = type;

    u8 flag = 1;
    for(int i = 0; i < 8000; i++) {
        if(flag) {
            face_ret = kl520_api_face_add_ex(x, y, w, h, f_bmp | 0x80);
            flag = 0;
        } else {
            face_ret = kl520_api_face_add_ex(x, y, w, h, f_bmp);
        }
        if(face_ret != 0) {
            ret = KL520_FACE_FAIL;
            dbg_msg_console ("face ret err...");
        } else {
            ret = kl520_api_add_wait_and_get();
        }
        _api_face_note(ret);

        if((ret < KL520_FACE_BADPOSE || ret >= KL520_FACE_ATTACK) && (ret != KL520_FACE_NOFACE))
        {
#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
            kl520_api_snapshot_adv_shot_5face();
#endif
            // kl520_api_reset_hmi_external_interface();
            osDelay(1);
            break;
        }
        else if(ret == KL520_FACE_CALLIB_FAIL)
        {
            // kl520_api_reset_hmi_external_interface();
            break;
        }
        //for bad pose, retry
        //dbg_msg_console("get bad pose for registration, retrying:%d", i+1);
        osDelay(1);

        //in case of abort
        if (kl520_is_fdfr_abort() != 0) {
            ret = KL520_FACE_FAIL;
            break;
        }
    }
#if (CFG_LED_CTRL_ENHANCE == 1)
    kdp_e2e_nir_led_flag_off();
    if(ret == KL520_FACE_OK)
    {
        osDelay(100);
    }
#endif
    
    if(ret != KL520_FACE_OK && is_enroll_customize_uid()) {
        // if oms overwrite fails, need restore
        //restore the FLASH to DDR DB.
        kdp_app_db_flash_to_ddr(get_enroll_customize_uid());
    }
    
    return ret;
}

//Refer to uart_sample_face_add_timeout
extern uint16_t uart_face_add_timeout(u8_t admin, char *user_name, u8_t face_type, uint16_t time_out_ms, uint8_t cmd_id)
{
    kdp_e2e_face_variables* vars = kdp_e2e_get_face_variables();
    db_write = DB_FACE_ADD_GOING;

    u16 ret = 0xFF0F;
    u8 result = MR_SUCCESS;
    u8 bmp=0;
		if((KDP_FACE_DIRECTION_UNDEFINE <  face_type ) && (KDP_FACE_DIRECTION_MASK> face_type))
		{
			if(face_type == KDP_FACE_DIRECTION_MIDDLE )
			{
                bmp = 1 << FACE_ADD_TYPE_NORMAL;
                g_eFaceDirState = 0;
                face_reg_sts = 0;
			}
			else
			{
					if(face_type & KDP_FACE_DIRECTION_RIGHT) bmp |= 1<<FACE_ADD_TYPE_RIGHT;
					if(face_type & KDP_FACE_DIRECTION_LEFT)  bmp |= 1<<FACE_ADD_TYPE_LEFT;
					if(face_type & KDP_FACE_DIRECTION_DOWN)  bmp |= 1<<FACE_ADD_TYPE_DOWN;
					if(face_type & KDP_FACE_DIRECTION_UP)    bmp |= 1<<FACE_ADD_TYPE_UP;
			}
		
			if(bmp == (1<<FACE_ADD_TYPE_NORMAL))
			{
					vars->admin = admin;
					memcpy(vars->user_name, user_name, MAX_LEN_USERNAME);
			}
		}
		else
		{
				g_eFaceDirState = 0;    
				g_nFaceId = 0xFF;
				ret = 0xFF05; // PARAMETER ERROR
		}


    if ( ( g_tImpExpMassDataPkgInfo.nReadyType == DATA_READY_TYPE_IMP_FM ) ||
         ( g_tImpExpMassDataPkgInfo.nReadyType == DATA_READY_TYPE_IMP_IMG ) )
    {
        kl520_api_face_close();
        kl520_api_face_set_add_mode(FACE_ADD_MODE_1_FACE);
        kdp_e2e_prop_set2(flow_mode, FLOW_MODE_SIM_PRE_ADD);

#if (CFG_AI_TYPE == AI_TYPE_N1R1)
        vars->rgb_led_flag = TRUE;
        vars->nir_led_flag = TRUE; //for sim, always on.

        if (vars->rgb_led_flag == FALSE && vars->rgb_led_lv_history_flag == FALSE)
            vars->rgb_led_flag = FALSE;
        else
            vars->rgb_led_flag = TRUE;
#endif

        vars->pre_add = AI_TYPE_PR1; //default rgb to nir
        if(g_tImpExpMassDataPkgInfo.nExtData == KDP_IMP_FM_NIR_TO_NIR) { //nir to nir
            vars->pre_add += 0x80;
        }
        dbg_msg_algo("[%s] pre_add type = %d", __func__, vars->pre_add);

        kl520_api_face_add(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, FACE_ADD_TYPE_NORMAL);

        ret = kl520_api_add_wait_and_get();
        kdp_e2e_prop_set2(flow_mode, FLOW_MODE_NORMAL);
        kl520_api_face_close();
        vars->pre_add = 0;
				//zcy add for import fm set stat FACE_ADD_MODE_5_FACES return id
				g_eFaceDirState = KDP_FACE_DIRECTION_MASK;

    }
    else
    {
        kl520_api_face_add_set_timeout(time_out_ms);

#ifdef KID_ENROLL_SINGLE
        if ( g_eEnrollAddFaceTypeMode == FACE_ADD_MODE_1_FACE )
        {
            kl520_api_face_set_add_mode(FACE_ADD_MODE_1_FACE);
        }
        else
#endif
        {
            kl520_api_dp_five_face_enable();
            kl520_api_face_set_add_mode(FACE_ADD_MODE_5_FACES);
        }

        if(bmp)
        {
            ret = _face_add_internal(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, (kl520_face_add_type)bmp);
        }

    }

    if (tid_abort_thread != 0)
    {
        sample_force_abort_disable();
    }

    if (KL520_FACE_OK == ret)
    {
        if (0xFF != kl520_api_face_get_curr_face_id())
        {
            g_nFaceId = (u8)kl520_api_face_get_curr_face_id();
            kl520_api_face_set_curr_face_id(0xFF);
        }

        ret = ret + ((g_nFaceId&0xFF)<<8);
        result = MR_SUCCESS;

        extern u8 face_succ_index;
        switch(face_succ_index)
        {	
            case FACE_ADD_TYPE_NORMAL: //middle
                g_eFaceDirState |= KDP_FACE_DIRECTION_MIDDLE;
                break;
            case FACE_ADD_TYPE_LEFT: //left
                g_eFaceDirState |= KDP_FACE_DIRECTION_LEFT;
                break;
            case FACE_ADD_TYPE_RIGHT: //right
                g_eFaceDirState |= KDP_FACE_DIRECTION_RIGHT;
                break;
            case FACE_ADD_TYPE_UP: //up
                g_eFaceDirState |= KDP_FACE_DIRECTION_UP;
                break;
            case FACE_ADD_TYPE_DOWN: //down
                g_eFaceDirState |= KDP_FACE_DIRECTION_DOWN;
                break;
        }

    }
    else if (KL520_FACE_EXIST == ret){ 
			ret = 0xFF02; 
			result = MR_FAILED_FACE_ENROLLED; 
			g_eFaceDirState = KDP_FACE_DIRECTION_MASK;
			 
			g_nFaceId = (u8)kl520_api_face_get_curr_face_id(); //zcy add for return user id 
			dbg_msg_console("[%s] g_nFaceId= %x and g_eFaceDirState %x", __func__, g_nFaceId,g_eFaceDirState);
	  }
    else if (KL520_FACE_TIMEOUT == ret){ ret = 0xFF03; result = MR_FAILED_TIME_OUT; }
    else if (KL520_FACE_FULL == ret){ ret = 0xFF01; result = MR_FAILED_MAX_USER; }
    else if (KL520_FACE_ATTACK == ret){ ret = 0xFF05; result = MR_FAILED_LIVENESS_CHECK; }
    else if (0xff05 == ret) {result = MR_FAILED_INVALID_PARAM;}
    else{ ret = 0xFF0F; result = MR_ABORTED; }

    SysWaitDelayTime();
    if ( g_bStopSendMsg == FALSE )
    {
        u16 user_id;
        if ((g_eEnrollAddFaceTypeMode == FACE_ADD_MODE_5_FACES) && 
            (g_eFaceDirState != KDP_FACE_DIRECTION_MASK) && 
            ( g_tImpExpMassDataPkgInfo.nReadyType != DATA_READY_TYPE_IMP_FM ) && 
            ( g_tImpExpMassDataPkgInfo.nReadyType != DATA_READY_TYPE_IMP_IMG ) )
            user_id = 0xFFFF;
        else
            user_id = g_nFaceId;
        
        if (((g_eEnrollAddFaceTypeMode == FACE_ADD_MODE_5_FACES) && (g_eFaceDirState == KDP_FACE_DIRECTION_MASK)) ||
            ((g_eEnrollAddFaceTypeMode == FACE_ADD_MODE_1_FACE) /*&& (g_eFaceDirState == KDP_FACE_DIRECTION_MIDDLE)*/))
        {
            user_id = g_nFaceId;
            g_eFaceDirState = KDP_FACE_DIRECTION_MASK;
        }
        dbg_msg_console("[%s] userid= %x g_eEnrollAddFaceTypeMode %d", __func__, user_id,g_eEnrollAddFaceTypeMode);
        send_enroll_reply_msg(result, ((user_id >> 8) & 0xFF), ((user_id >> 0) & 0xFF), g_eFaceDirState, cmd_id);
    }
    db_write = DB_FACE_ADD_IDLE;
    return ret;
}
//-----ENROLL

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----VERIFY
//Refer to uart_sample_face_recognition_timeout

extern uint16_t uart_face_recognition_timeout(u8 pd_rightaway, uint16_t time_out_ms)
{
    u16 ret = 0;
    u8 face_id = 0;
    u32 events = 0;
    u16 input = 0;
//    system_info t_sys_info = { 0 };
    u8 result = MR_SUCCESS;
    char buf[USER_NAME_SIZE]={'L','I','V','E','N','E','S','S'};
    msg_verify_data s_msg;
    memset(&(s_msg), 0, sizeof(s_msg));
    s_msg.user_id_heb = 0xFF;
    s_msg.user_id_leb = 0xFF;
#if (CFG_E2E_REC_NOTE == YES)
    skip_face_note = 0;
#endif

    if(time_out_ms == 0) { //if time out is 0.
        result = MR_FAILED_INVALID_PARAM;
        send_verify_reply_msg(result, s_msg);
        return result;
    }
    
    if (g_tImpExpMassDataPkgInfo.eType == MASS_DATA_IMP_FM)
    {
        osDelay(1000);
        result = MR_REJECTED;
        send_verify_reply_msg(result, s_msg);
        return result;
    }

    int face_ret = 0;
    if ( g_bUserDemoMode == TRUE )
    {
        dbg_msg_console("[Demo]Verify");
        kl520_api_face_liveness_set_timeout(time_out_ms);
        face_ret = kl520_api_face_liveness(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    }
    else
    {
        kl520_api_face_recognition_set_timeout(time_out_ms);
        face_ret = kl520_api_face_recognition(0, 0, DISPLAY_WIDTH, (input == 0)?DISPLAY_HEIGHT:input);
    }

    do{
//        kl520_face_recognition_note();
#if KL520_REC_EYEMODE == YES
        events = osEventFlagsWait( kl520_api_get_event(), KL520_APP_FLAG_FDFR | KL520_DEVICE_FLAG_ERR, osFlagsWaitAny, 100); // 100ms
        if( events == 0xfffffffe ) {
            if( get_eye_mode_status() == 1 ) {
                kl520_face_recognition_note();
            }
            //clear_event( kl520_api_get_event(), KL520_APP_FLAG_FDFR | KL520_DEVICE_FLAG_ERR );
            continue;

        }
#else
        if(face_ret != -1) {
            events = wait_event(kl520_api_get_event(), KL520_APP_FLAG_FDFR | KL520_DEVICE_FLAG_ERR);
        }
#endif
//        kl520_face_note_Terminate();
//        dbg_msg_console("sample_face_recognition events, events=%d", events);


        if(events == KL520_DEVICE_FLAG_ERR){    result = MR_FAILED_DEV_OPEN_FAIL;     ret = 0xFF02;   break;}
        else
        {
            if(face_ret != -1) {
                ret = kl520_api_face_get_result(&face_id);
            } else {
                ret = KL520_FACE_FAIL;
            }
            //dbg_msg_console("kl520_api_face_get_result, ret =%d", ret );

#ifdef DB_DRAWING_CUSTOMER_COLOR
            kl520_api_face_notify((KL520_FACE_OK == ret)?KL520_FACE_DB_OK:ret);
#endif

            if ( ( KL520_FACE_OK == ret ) ||
                 ( KL520_FACE_EYE_CLOSE_STATUS_OPEN_EYE == ret ) ||
                 ( KL520_FACE_EYE_CLOSED == ret ) ||
                 ( KL520_FACE_EYE_CLOSE_UNKNOW_STATUS == ret ) )
            {
                kdp_e2e_face_variables *vars_cur = kdp_e2e_get_face_variables();
                s_msg.admin = vars_cur->admin;
//                s_msg.unlockStatus = 0;
                s_msg.user_id_heb = ((face_id >> 8) &0xFF);
                s_msg.user_id_leb = ((face_id >> 0) &0xFF);

//                if(vars_cur->eye_left_ratio < 0.20f && vars_cur->eye_right_ratio < 0.20f){ s_msg.unlockStatus = ST_FACE_MODULE_STATUS_UNLOCK_WITH_EYES_CLOSE;}
//                else{ s_msg.unlockStatus = ST_FACE_MODULE_STATUS_UNLOCK_OK; }
                dbg_msg_algo("vars_cur->eye_left_ratio=%f, vars_cur->eye_right_ratio=%f", vars_cur->eye_left_ratio, vars_cur->eye_right_ratio);

                if ( g_bUserDemoMode == TRUE )
                {
                    memcpy( s_msg.user_name, &buf, USER_NAME_SIZE );
                }
                else{
                    memcpy( s_msg.user_name, vars_cur->user_name, USER_NAME_SIZE );
                }

                dbg_msg_engineering("KL520_FACE_OK, face_id=0x%x, Admin=%d, UseName=%s", face_id, s_msg->admin, s_msg.user_name);

                result = MR_SUCCESS;
                break;
            }
            else
            {
                if(( g_bUserDemoMode == FALSE )&&(KL520_FACE_DB_FAIL == ret || KL520_FACE_EMPTY == ret)) { result = MR_FAILED_UNKNOWN_USER;ret = 0xFF01;break;}
                else if (KL520_FACE_TIMEOUT == ret)             { result = MR_FAILED_TIME_OUT;          ret = 0xFF03;   break;}
                else if (KL520_FACE_NOFACE_AND_TIMEOUT == ret)  { result = MR_FAILED_TIME_OUT;          ret = 0xFF04;   break;}
                else if (KL520_FACE_INVALID == ret)             { result = MR_FAILED_LIVENESS_CHECK;    ret = 0xFF05;   break;}
                else if (KL520_FACE_EYE_CLOSED == ret || KL520_FACE_FAIL == ret) { result = MR_ABORTED; ret = 0xFF06;   break;}
                else if (KL520_FACE_NOFACE == ret)              { result = MR_FAILED_UNKNOWN_REASON;    dbg_msg_console("event , KL520_FACE_NOFACE and continue"); }  //continue fdfr
                else if (KL520_FACE_TOO_FAR <= ret && KL520_FACE_LOW_QUALITY >= ret) { dbg_msg_console("event , ret=%d", ret); }   //continue fdfr
                else
                {
                    dbg_msg_console("face_result: %d", ret);
                    result = MR_FAILED_UNKNOWN_REASON;
                    ret = 0xFF07;
                    break;
                }
            }
        }
        osDelay(1);
    }while(1);

#if (CFG_E2E_REC_NOTE == YES)
    skip_face_note = 0;
#endif
    if(dp_draw_info.e2e_eye_type == FDFR_STATUS_EYE_CLOSED)                     { s_msg.unlockStatus = ST_FACE_MODULE_STATUS_UNLOCK_WITH_EYES_CLOSE; }  //
    else if(dp_draw_info.e2e_eye_type == FDFR_STATUS_EYE_CLOSE_UNKNOW_STATUS)   { s_msg.unlockStatus = ST_FACE_MODULE_STATUS_UNLOCK_OK; }
    else                                                                        { s_msg.unlockStatus = ST_FACE_MODULE_STATUS_UNLOCK_OK; }

    SysWaitDelayTime();

#if (MEASURE_RECOGNITION == YES)
    kl520_measure_info();
#endif

    dbg_msg_console("sample_face_recognition END, ret=0x%x", ret);

#ifdef CFG_GUI_RECOG_CLOSE_TEST
#if (0==CFG_GUI_RECOG_CLOSE_TEST)
    sample_face_close();
#endif
#else

    wait_fr_update_thread();

    if ( g_bStopSendMsg == FALSE ) {
        send_verify_reply_msg(result, s_msg);
#if (MEASURE_RECOGNITION == YES)
        kl520_measure_stamp(E_MEASURE_FACE_FACE_OK);
#endif
    }
    db_write = DB_FACE_ADD_IDLE;

    sample_face_close();
#endif

    return ret;
}
//-----VERIFY

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
#if defined KID_SNAP_IMAGE || defined KID_KN_SNAP_IMAGE
extern uint8_t uart_face_snap_image(void)
{
    //u8 start_cnt = 0;//g_tSnapImgInfo.nStrNum;
    u8 img_cnt = g_tSnapImgInfo.nImgCnt;
    if(img_cnt > SNAPSHOT_ADV_NUM) img_cnt = SNAPSHOT_ADV_NUM;

    u8 ret = KL520_FACE_OK;
    u8 face_id = 0;
    
    kl520_api_snapshot_adv_init(MIPI_CAM_RGB, KDP_DDR_TEST_RGB_IMG_SIZE);
    kl520_api_snapshot_adv_init(MIPI_CAM_NIR, KDP_DDR_TEST_NIR_IMG_SIZE);

    for ( u8 i = 0; i < img_cnt; i++ )
    {
        u8 nIdx = i;
        g_snap_img_cnt = nIdx;
        
        int face_ret = kl520_api_snap_image(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);

        if(face_ret == -1) {
            dbg_msg_console("[SnapImage] Camera is not ready", nIdx);
            return MR_FAILED_DEV_OPEN_FAIL;
        }

        u32 events = wait_event(kl520_api_get_event(), KL520_APP_FLAG_FDFR | KL520_DEVICE_FLAG_ERR);
        
        if(events == KL520_DEVICE_FLAG_ERR){
            dbg_msg_console("[SnapImage] Camera is not ready", nIdx);
            return MR_FAILED_DEV_OPEN_FAIL;
        } else {
            ret = kl520_api_face_get_result(&face_id);
        }

        sample_face_close();
        
        if ( ret == KL520_FACE_OK ) {
            dbg_msg_console("[SnapImage] got %d image succeeded", nIdx+1);
        } else {
            dbg_msg_console("[SnapImage] got %d image, failed", nIdx+1);
            return MR_FAILED_STORE_ERR;
        }
        osDelay(200);
    }
    return MR_SUCCESS;
}
#endif
#endif

u8 _uart_face_del_some_users(u8 face_id0, u8 face_id1)
{
    u8 ret = 0;
    u8 ret_count = 0;
    u8 id0 = (face_id0<face_id1)?face_id0:face_id1;
    u8 id1 = (face_id0<face_id1)?face_id1:face_id0;
    
    for(u16 i=id0; i<(id1+1); i++)
    {
        if (MAX_USER > kdp_app_db_find_exist_id(i))
        {
            ret = kl520_api_face_del(2, (u8)i);
            if (0 == ret)
            {
                ret_count++;
            }
        }
    }
    
    if(ret_count > 0)
        return 0;
    else
        return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----OTA
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK )
uint8_t get_mass_data_status( void )
{
    return ota_update_info.ota_process;
}

uint8_t get_ota_data_status( void )
{
    return ota_update_info.ota_process;
}

extern uint8_t DSM_Uart_StopOta(void)
{
    uint8_t ret = MR_SUCCESS;
    
    if (ota_update_info.ota_status == OTA_STATUS_FINAL)
    {
        ret = MR_REJECTED;
    }
    else
    {
        ota_update_info.ota_process = NO;
        ota_update_info.packet_id = 0;
        ota_update_info.ddr_addr_idx = 0;
        ota_update_info.total_pid = 0;
        ota_update_info.total_update_size = 0;
        ota_update_info.ota_status = OTA_STATUS_IDLE;
    }
    
    return ret;
}

#if ( ENCRYPTION_MODE&AES_ENCRYPTION )
extern uint8_t Ota_MD5_checksum( u32 addr )
{
    u8 md5_crc[32], md5[16];
    u8 i;

    md5_main( (void*)addr , (void*)md5, ota_update_info.total_update_size );
    md5_covers( md5 ,md5_crc );

    dbg_msg_nocrlf("md5 32:");
    for (i = 0; i < 32; i++) dbg_msg_nocrlf (" %02X", md5_crc[i]);
    dbg_msg_console("\n");    

    for (i = 0; i < 32; i++) {
        if( md5_crc[i] != ota_update_info.header_info.md5_sum[i] ) {
            dbg_msg_console("[%s] FAIL: [%d]: %d", __func__, i, md5_crc[i] );
            return NO;
        }
    }
    dbg_msg_console("[%s] Checksum OK", __func__);

    return YES;
}
#endif

uint8_t ota_bin_double_check(struct ota_header_info *info)
{
    u8 md5_crc[32], md5[16];
    u8 i;
    u8 special_str[] = {'K','N','E','R','O','N',' ','I','P','O'};
    u8 final_str[42] = {0,};

    //u32 ota_bin_addr = (u32)info;

    md5_main( (void*)(info->part_bin), (void*)md5, ota_update_info.total_update_size - 38 );
    md5_covers( md5 ,md5_crc );

    memcpy(&final_str[0], &special_str[0], 10);
    memcpy(&final_str[10], &md5_crc[0], 32);

    md5_main((void*)final_str, (void*)md5, sizeof(final_str));
    md5_covers(md5 ,md5_crc);

    dbg_msg_nocrlf("md5 32:");
    for (i = 0; i < 32; i++) dbg_msg_nocrlf (" %02X", md5_crc[i]);
    dbg_msg_console("\n");   

    for (i = 0; i < 32; i++) {
        if( md5_crc[i] != info->md5[i] ) {
            dbg_msg_console("[%s] FAIL: [%d]: %02x", __func__, i, md5_crc[i] );
            return NO;
        }
    }
    dbg_msg_console("[%s]Checksum ok", __func__);

    return YES;
}

void ota_check_info(struct ota_header_info *info, struct ota_part_header *scpu_header, 
        struct ota_part_header *ncpu_header, struct ota_part_header* model_header)
{
    //dbg_msg_console("sync word: %02x %02x", info->sync_word[0], info->sync_word[1]);
    //dbg_msg_console("total_size: %d", info->total_size);

    for (size_t i = 0; i < KDP_OTA_MAX_PART; i++)
    {
        if ((KDP_OTA_TYPE_SCPU > info->part_bin[i].type) || (info->part_bin[i].type > KDP_OTA_TYPE_MODEL))
            break;

        dbg_msg_console("type: %d, length: %d. addr: 0x%08x", info->part_bin[i].type,
            info->part_bin[i].length, info->part_bin[i].addr);
//        dbg_msg_console("version: %02x.%02x.%02x.%02x", info->part_bin[i].version[0], 
//            info->part_bin[i].version[1], info->part_bin[i].version[2], info->part_bin[i].version[3]);

        u32 ota_version = StreamsToBigEndU32(info->part_bin[i].version);
        dbg_msg_console("ota_version:   %08x", ota_version);

        u32 local_version;
        struct fw_misc_data scpu_version;
        struct fw_misc_data ncpu_version;
        struct fw_misc_data *model_version;
        switch (info->part_bin[i].type)
        {
        case KDP_OTA_TYPE_SCPU:
            memcpy(scpu_header, &info->part_bin[i], sizeof(struct ota_part_header));
            
            kl520_api_get_scpu_version(&scpu_version);
            local_version = StreamsToBigEndU32(scpu_version.version);
            dbg_msg_console("local_version: %08x", local_version);
            if (ota_version != local_version)
            {
                ota_update_info.scpu_update = YES;
            }
            else
            {
                ota_update_info.scpu_update = NO;
            }
            break;

        case KDP_OTA_TYPE_NCPU:
            memcpy(ncpu_header, &info->part_bin[i], sizeof(struct ota_part_header));
            
            kl520_api_get_ncpu_version(&ncpu_version);
            local_version = StreamsToBigEndU32(ncpu_version.version);
            dbg_msg_console("local_version: %08x", local_version);
            if (ota_version != local_version)
            {
                ota_update_info.ncpu_update = YES;
            }
            else
            {
                ota_update_info.ncpu_update = NO;
            }
            break;

        case KDP_OTA_TYPE_MODEL:
            memcpy(model_header, &info->part_bin[i], sizeof(struct ota_part_header));

            model_version = kl520_api_get_model_version();
            local_version = StreamsToBigEndU32(model_version->version);
            dbg_msg_console("local_version: %08x", local_version);
            if (ota_version != local_version)
            {
                ota_update_info.model_update = YES;
            }
            else
            {
                ota_update_info.model_update = NO;
            }
            break;

        default:
            break;
        }
    }
}

#define OTA_IDLE 2
#define OTA_FAIL 1
#define OTA_DONE 0

u8 ota_auto_update(u32 addr_start)
{
    int ret = 0;
    struct ota_header_info *ota_header = (struct ota_header_info *)addr_start;
    if(!ota_bin_double_check(ota_header)) {
        dbg_msg_console("bin file check fail");
        return OTA_FAIL;
    }

    struct ota_part_header ota_scpu_header, ota_ncpu_header, ota_model_header;
    ota_check_info(ota_header, &ota_scpu_header, &ota_ncpu_header, &ota_model_header);
    dbg_msg_console("scpu_update:%d", ota_update_info.scpu_update);
    dbg_msg_console("ncpu_update:%d", ota_update_info.ncpu_update);
    dbg_msg_console("model_update:%d", ota_update_info.model_update);

    if( ota_update_info.scpu_update == YES )
    {
        //scpu
        ret = ota_update_case( FLASH_CMD_ACT_NUM_SEL_SCPU, addr_start + ota_scpu_header.addr, ota_scpu_header.length );
        if( ret != OTA_UPDATE_SUCCESS )
        {
            dbg_msg_console("scpu ota fail, ret:%d", ret);
            return OTA_FAIL;
        }
        dbg_msg_console("scpu ota ok, ret:%d", ret);
    }
    if ( ota_update_info.ncpu_update == YES )
    {
        //ncpu
        ret = ota_update_case( FLASH_CMD_ACT_NUM_SEL_NCPU, addr_start + ota_ncpu_header.addr, ota_ncpu_header.length );
        if( ret != OTA_UPDATE_SUCCESS )
        {
            dbg_msg_console("ncpu ota fail, ret:%d", ret);
            return OTA_FAIL;
        }
        dbg_msg_console("ncpu ota ok, ret:%d", ret);
    }
    
    if ( ota_update_info.model_update == YES) 
    {
        kdp_flash_set_protect_bypass(1);
        //fw info
        int info_size = KDP_FLASH_ALL_MODELS_ADDR - KDP_FLASH_FW_INFO_ADDR;
        ret = ota_update_case( FLASH_CMD_ACT_NUM_SEL_FW_INFO, addr_start + ota_model_header.addr, info_size );
        if( ret != OTA_UPDATE_SUCCESS )
        {
            dbg_msg_console("fw info ota fail, ret:%d", ret);
            kdp_flash_set_protect_bypass(0);
            return OTA_FAIL;
        }
        
        ret = ota_update_case( FLASH_CMD_ACT_NUM_SEL_MODEL, addr_start + ota_model_header.addr + info_size, 
                ota_model_header.length - info_size );
        if( ret != OTA_UPDATE_SUCCESS )
        {
            dbg_msg_console("model ota fail, ret:%d", ret);
            kdp_flash_set_protect_bypass(0);
            return OTA_FAIL;
        }
        kdp_flash_set_protect_bypass(0);
    }

    ota_burn_in_config(0x3);


#if 0
    if ( ota_update_info.ui_update == YES )
    {
        u32 nUiSize = USR_FLASH_LAST_ADDR - KDP_FLASH_LAST_ADDR;

        if ( ota_update_case( FLASH_CMD_ACT_NUM_SEL_IMAGE_UPDATE, ota_buf_start, nUiSize ) != OTA_UPDATE_SUCCESS )
        {
            return OTA_FAIL;
        }
        ota_buf_start += nUiSize;
        total_update_size -= nUiSize;
    }
#endif
    
    return OTA_DONE;
}

uint8_t OtaProcess_run( u8 bypass )
{   
    if(bypass != 0) return OTA_IDLE;

    u32 total_update_size = ota_update_info.total_update_size;
    if( ota_update_info.ddr_addr_idx != total_update_size ) {
        dbg_msg_console("ota size mismatch:%d,%d.", total_update_size, 
            ota_update_info.ddr_addr_idx);
        return OTA_FAIL;
    }

#if ( ENCRYPTION_MODE&AES_ENCRYPTION )
    if( Ota_MD5_checksum(ota_update_info.ota_ddr_start_addr) != YES ) {
        dbg_msg_console("ota md5 failed.");
        return OTA_FAIL;
    }
    else
#else
        //if( bypass == TRUE)
#endif
    {
        uint32_t ota_buf_start = ota_update_info.ota_ddr_start_addr;
        int ret = ota_auto_update(ota_buf_start);
        if (ret != OTA_DONE) {
            dbg_msg_console("ota not succeeded.");
            return OTA_FAIL;
        } else {
            dbg_msg_console("ota done.");
            return OTA_DONE;
        }
    }
}
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----Other
//osThreadId_t tid_notes_face_recognition = NULL;
//void _tasks_notes_face_recognition_thread(void *arg)
//{
//    u8 face_id = 0;
//    _api_face_note(kl520_api_face_get_result(&face_id));
//    tid_notes_face_recognition = NULL;
//    osThreadExit();
//}

#if (CFG_E2E_REC_NOTE == YES)
void kl520_face_recognition_note(void)
{
    if(skip_face_note < 2){     skip_face_note++;   return;}

//#if 0
//    if( tid_notes_face_recognition == NULL)
//    {
//        osThreadAttr_t attr = {
//            .stack_size = 1024,
//            .priority = osPriorityNormal
//        };
//        tid_notes_face_recognition = osThreadNew(_tasks_notes_face_recognition_thread, NULL, &attr);
//    }
//#endif
    {
        u8 face_id = 0;
        _api_face_note(kl520_api_face_get_result(&face_id));

    }
}
#endif

//void kl520_face_note_Terminate(void)
//{
//    if( tid_notes_face_recognition != NULL)
//    {
//        osThreadTerminate(tid_notes_face_recognition);
//        tid_notes_face_recognition = NULL;
//    }
//}
#endif
#endif
