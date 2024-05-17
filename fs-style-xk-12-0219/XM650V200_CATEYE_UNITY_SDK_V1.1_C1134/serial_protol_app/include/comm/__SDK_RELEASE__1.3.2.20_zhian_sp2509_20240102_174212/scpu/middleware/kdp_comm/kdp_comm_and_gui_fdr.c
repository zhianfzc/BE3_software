#include "kdp_comm_and_gui_fdr.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP )
#include "board_ddr_table.h"
#include "board_flash_table.h"
#include "flash.h"
#include "ota.h"
#include "framework/event.h"
#include "kdp_memxfer.h"
#include "kdp520_gpio.h"
#include "kl520_com.h"
#include "kl520_include.h"
#include "kl520_api.h"
#include "kl520_api_extra_fmap.h"
#include "kl520_api_snapshot.h"
#include "kl520_api_camera.h"
#include "kl520_api_fdfr.h"
#include "user_ui.h"
#include "user_io.h"
#include "kdp_comm_app.h"
#include "kdp_comm_and_gui_fdr.h"
#include "kdp_comm_protoco.h"
#include "kdp_comm_md5.h"
#include "kdp_e2e_db.h"
#include "kdp_app_db.h"
#include "sample_app_console.h"
#include "power.h"

#if ( CFG_GUI_ENABLE == YES )
#include "gui_fsm.h"
#include "sample_gui_fsm_events.h"
#include "sample_app_console.h"
#endif

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
static bool g_bResponseEn = FALSE;
#endif
volatile static enum USER_COM_THREAD_EVENT g_eUserComThreadEvent = USER_COM_THREAD_EVENT_READY;
extern u8 m_curr_user_id;
extern u8 db_write;

static osEventFlagsId_t user_com_event_id = NULL;
static osEventFlagsId_t user_com_fdfr_evt = NULL;
static osThreadId_t tid_user_com_fdfr_thread = NULL;
volatile static u16 _user_com_data = 0;
volatile static msg_enroll_data msg_enroll;
osMutexId_t mutex_rsp_msg = NULL;

extern void send_reply_AesNoDataMsg(uint8_t result, uint8_t kid);

static void _user_com_fdfr_thread(void);
static void _user_com_force_abort_fdfr(void);
void user_com_event_power_off(void);
void user_com_response_data(u8* p_data, u16 size);

osEventFlagsId_t get_user_com_event_id(void)
{
    return user_com_fdfr_evt;
}

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )

bool user_com_thread_event_check_set(u32 flag)
{
    if (g_eUserComThreadEvent > USER_COM_THREAD_EVENT_READY) {
        osDelay(200); //if this command too fast but the prev one not finished.
    }
    
    if(g_eUserComThreadEvent == USER_COM_THREAD_EVENT_OTA_FLASH) return TRUE;
    
    if(g_eUserComThreadEvent == USER_COM_THREAD_EVENT_NON_OVERWRITABLE) { //reject
        dbg_msg_console("[BYPASS]");
        return TRUE;
    }
    
    enum USER_COM_THREAD_EVENT n_sta = USER_COM_THREAD_EVENT_READY; //new state
    if(flag == USER_COM_FLAG_RECOGNITION) {
        n_sta = USER_COM_THREAD_EVENT_VERIFY;
    } else if(flag == USER_COM_FLAG_REGISTRATION) {
        n_sta = USER_COM_THREAD_EVENT_ENROLL;
    } else if(flag == 0) { //demo
        //not set new state
    } else {
        n_sta = USER_COM_THREAD_EVENT_NON_OVERWRITABLE;
    }
    
    if (g_eUserComThreadEvent <= USER_COM_THREAD_EVENT_READY) { //event ready
        if(flag) g_eUserComThreadEvent = (enum USER_COM_THREAD_EVENT)n_sta;
        return FALSE;
    }
    
    // overwritable
    // if(flag == 0 || n_sta != g_eUserComThreadEvent)
    { //abort the running one
        g_bResponseEn = TRUE;
        user_com_event_interrupt();
        user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
        g_bResponseEn = FALSE;
        dbg_msg_console("[FORCE]");
        if(flag) g_eUserComThreadEvent = (enum USER_COM_THREAD_EVENT)n_sta;
    }
    
    return FALSE;
}

bool user_com_thread_event_ota_copy_to_flash(void)
{
    dbg_msg_algo ("settting ota status:%d.", g_eUserComThreadEvent);
    if (g_eUserComThreadEvent > USER_COM_THREAD_EVENT_READY) {
        osDelay(200); //if this command too fast but the prev one not finished.
    }
    
    if(g_eUserComThreadEvent == USER_COM_THREAD_EVENT_OTA_FLASH) return TRUE;
    
    for (int i = 0; i < 25; i++) {
        if(g_eUserComThreadEvent == USER_COM_THREAD_EVENT_NON_OVERWRITABLE) {
            osDelay(200); //wait
        } else {
            break;
        }
    }
    
    if(g_eUserComThreadEvent == USER_COM_THREAD_EVENT_NON_OVERWRITABLE) { //reject
        dbg_msg_console("other command is writing flash.");
        return TRUE;
    }
    
    if (g_eUserComThreadEvent <= USER_COM_THREAD_EVENT_READY) { //event ready
        g_eUserComThreadEvent = USER_COM_THREAD_EVENT_OTA_FLASH;
        return FALSE;
    }
    
    if (g_eUserComThreadEvent == USER_COM_THREAD_EVENT_VERIFY || 
            g_eUserComThreadEvent == USER_COM_THREAD_EVENT_ENROLL) {
        user_com_event_interrupt();
        g_eUserComThreadEvent = USER_COM_THREAD_EVENT_OTA_FLASH;
        return FALSE;
    }
    
    return FALSE;
}


#if ( CFG_GUI_ENABLE == YES )
static gui_fsm_src user_com_src = GUI_FSM_SRC_NULL;
#endif
//extern bool user_parser(u8 cmd);
//__WEAK bool user_parser(u8 cmd) {return FALSE;}


#if defined KID_DB_IMPORT_REQUEST || defined KID_DB_EXPORT_REQUEST
typedef struct {
    u32 addr;
    u32 size;
} s_data_info;

static s_data_info db_export_info;
static s_data_info db_import_info;
static u16 db_import_user_id;

static void update_db_export_info(u32 addr, u32 size)
{
    db_export_info.addr = addr;
    db_export_info.size = size;
}

static u32 get_db_export_addr(void)
{
    return db_export_info.addr;
}

static u32 get_db_export_size(void)
{
    return db_export_info.size;
}

static void update_db_import_info(u32 addr, u32 size, u16 user_id)
{
    db_import_info.addr = addr;
    db_import_info.size = size;
    db_import_user_id = user_id;
}

static u32 get_db_import_addr(void)
{
    return db_import_info.addr;
}

static u32 get_db_import_size(void)
{
    return db_import_info.size;
}

static u32 get_db_import_user_id(void)
{
    return db_import_user_id;
}
#endif

//KID_KN_DEVICE_INFO
static void handle_kn_device_info(void)
{
    dbg_msg_console("[%s]", "KID_KN_DEVICE_INFO");
    kn_device_info_data device_info;
    s32 ret = DSM_Uart_Get_Kn_DeviceInfo(&device_info);
    send_Get_Kn_DeviceInfo_reply_msg(ret, &device_info);
    return;
}

//KID_GET_VERSION
static void handle_get_version(void)
{
    dbg_msg_console("[%s]", "KID_GET_VERSION");
    uint8_t Version[32];
    s32 ret = DSM_Uart_Get_Version_Info(Version);
    response_get_version_info_msg(MR_SUCCESS, Version); //result:MR_SUCCESS
    return;
}

static void handle_get_version_zhian(void)
{
    dbg_msg_console("[%s]", "KID_GET_VERSION_ZA");
    uint8_t Version[32]={0};
    s32 ret = DSM_Uart_Get_Version_Info_zhian(Version);
	  osDelay(100);
    response_get_version_info_msg_zhian(MR_SUCCESS, Version); //result:MR_SUCCESS
    return;
}

static void handle_get_version_zhian_prd(void)
{
    dbg_msg_console("[%s]", "KID_GET_VERSION_ZAPRD");
    uint8_t Version[32]={0};
    s32 ret = DSM_Uart_Get_Version_Info_zhian_prd(Version);
	  osDelay(100);
    response_get_version_info_msg_zhian_prd(MR_SUCCESS, Version); //result:MR_SUCCESS
    return;
}

static void handle_get_version_zhian_hard(void)
{
    dbg_msg_console("[%s]", "KID_GET_VERSION_HARDWARE");
    uint8_t Version[32]={0};
    s32 ret = DSM_Uart_Get_Version_Info_zhian_hard(Version);
	  osDelay(100);
    response_get_version_info_msg_zhian_hard(MR_SUCCESS, Version); //result:MR_SUCCESS
    return;
}

//KID_DEMO_MODE
static void handle_demo_mode(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_DEMO_MODE");
    if(user_com_thread_event_check_set(0) == true)  return;

    uint8_t demoModeEnable = *pDataStart;

    s32 ret = DSM_Uart_StartOrStopDemoMode(demoModeEnable);
    send_DebugModeOrDemoMode_reply_msg(ret, KID_DEMO_MODE);
    return;
}

//KID_DEBUG_MODE
#ifdef KID_DEBUG_MODE
static void handle_debug_mode(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_DEBUG_MODE");

    uint8_t StartDebugMode = *pDataStart;
    s32 ret = DSM_Uart_StartDebugMode(StartDebugMode);

    //send_StartDebugMode_reply_msg(ret,StartDebugMode);
    send_DebugModeOrDemoMode_reply_msg(ret, KID_DEBUG_MODE);
}
#endif

//KID_SOFT_RESET
static void handle_soft_reset(u8* pDataStart)
{
    if ((0x5a == pDataStart[0]) && (0xa5 == pDataStart[1]))
    {
        g_bResponseEn = TRUE;
        dbg_msg_console("[%s]", "KID_SOFT_RESET");

        g_bPowerDown = TRUE;

        wait_fr_update_thread();
        
        kl520_api_face_close();

        user_io_poweroff();
        DSM_Uart_SystemReset();
    }
    else
    {
        send_soft_reset_reply_msg(MR_FAILED_INVALID_PARAM);
    }
}

//KID_POWERDOWN
static void handle_power_down(void)
{
    dbg_msg_console("[%s]", "KID_POWERDOWN");

#if (CFG_KL520_VERSION == KL520A)
    g_bStopSendMsg = TRUE;
#endif

    u32 nIntervalTime = SysWaitDelayTime();

    dbg_msg_console("power_off diff_time=%d, note_msg_time=%d", nIntervalTime, g_nSysInternalTime);
    user_com_event_power_off();
    return;
}

//KID_GET_STATUS
static void handle_get_status(void)
{
    dbg_msg_console("[%s]", "KID_GET_STATUS");

    uint8_t state = DSM_Uart_GetState(g_eUserComThreadEvent);

    send_status_reply_msg(MR_SUCCESS, state);
    return;
}

//KID_VERIFY
static void handle_kid_verify(u8* pDataStart)
{
    dbg_msg_console("KID_VERIFY");
    kl520_measure_stamp(E_MEASURE_VERIFY_MSG);

    _user_com_data = StreamsToBigEndU16(pDataStart);
    dbg_msg_com(" shudown:%d; timeout:%d ",*pDataStart,*(pDataStart+1));

    if(user_com_thread_event_check_set(USER_COM_FLAG_RECOGNITION) == TRUE)  return;

    dbg_msg_com("had recv recognize cmd");

    if (FACE_MODE_NONE != m_face_mode)
    {
        g_bResponseEn = TRUE;
        sample_face_close();
        g_eFaceDirState = 0;
        g_nFaceId = 0;
        g_bResponseEn = FALSE;
    }
    user_com_event_start(USER_COM_FLAG_RECOGNITION);
    return;
}

static void parse_enroll_msg(u8* pDataStart, u8 cmd)
{
    memset((void*)&msg_enroll,0,sizeof(msg_enroll));
    msg_enroll.admin = *pDataStart;
    memcpy((void*)msg_enroll.user_name, pDataStart+1,USER_NAME_SIZE);
    msg_enroll.cmd_id = cmd;
	

    if(cmd == KID_ENROLL_OVERWRITE || cmd == KID_ENROLL) {
        msg_enroll.face_direction = *(pDataStart+33);
        msg_enroll.timeout = *(pDataStart+34);
    }
#ifdef KID_ENROLL_SINGLE
    else if(cmd == KID_ENROLL_SINGLE) {
        msg_enroll.face_direction = KDP_FACE_DIRECTION_MIDDLE;
        msg_enroll.timeout = *(pDataStart+34);
    }
#endif
#ifdef KID_ENROLL_ITG
    else if(cmd == KID_ENROLL_ITG) {
        msg_enroll.enroll_type = pDataStart[34];
        msg_enroll.enable_duplicate = pDataStart[35];
        msg_enroll.timeout = pDataStart[36];
        if (0 == msg_enroll.enroll_type) {
            msg_enroll.face_direction = pDataStart[33];
        } else {
            msg_enroll.face_direction = KDP_FACE_DIRECTION_MIDDLE;
        }
    }
#endif
	//zcy add for test
		if(msg_enroll.timeout <=0)
			msg_enroll.timeout = ENROLL_DEFAULT_TIME;
		
    dbg_msg_console("msg_enroll:admin:0x%02x,user_name:%s,face_dir:0x%02x,timeout:0x%02x ",
        msg_enroll.admin, msg_enroll.user_name, msg_enroll.face_direction, msg_enroll.timeout);
    
    return;
}

//KID_ENROLL
static void handle_kid_enroll(u8* pDataStart, u8 cmd)
{
    dbg_msg_console("[%s]", (KID_ENROLL==cmd)?"KID_ENROLL":"KID_ENROLL_OVERWRITE");

    parse_enroll_msg(pDataStart, cmd);

    u8 uid_in_msg = 0xff;
#ifdef  KID_ENROLL_OVERWRITE
    if (KID_ENROLL_OVERWRITE == cmd)
    {
        uid_in_msg = *(pDataStart+35);
       #if 0
        if (uid_in_msg >= 0x80)
            update_user_db_offset(0x80);
        else
            reset_user_db_offset();

        if ((uid_in_msg < get_user_db_offset()) || 
            (uid_in_msg >= get_user_db_offset() + CFG_MAX_USER))
        {
            send_enroll_reply_msg(MR_FAILED_INVALID_PARAM, ((uid_in_msg >> 8) &0xFF),  (uid_in_msg &0xFF) , 0, cmd);
            return;
        }
		#endif
				 dbg_msg_console("[%s] uid_in_msg= %x", __func__, uid_in_msg);
                if(uid_in_msg < kl520_api_get_start_user_id() || uid_in_msg > kl520_api_get_start_user_id()+MAX_USER){//zcy add
                  // send_data_error_reply_msg(MR_REJECTED);
				   send_enroll_reply_msg(MR_FAILED_INVALID_PARAM, ((uid_in_msg >> 8) &0xFF),  (uid_in_msg &0xFF) , 0, cmd);
                   return;
                }
    }
    else
#endif
        uid_in_msg = 0xff;
    
    set_enroll_customize_uid(uid_in_msg);
    
#ifdef  KID_ENROLL_OVERWRITE
    dbg_msg_console("overwrite_db_uid: %#x", uid_in_msg);
#endif
		if (KID_ENROLL_OVERWRITE == cmd){
			set_enroll_overwrite_flag(0);//zcy md for stop re-enroll
		}else{
			//set_enroll_overwrite_flag(1);//zcy md for re-enroll
			set_enroll_overwrite_flag(0);
		}
		
    if(user_com_thread_event_check_set(USER_COM_FLAG_REGISTRATION) == true) return;

    KDP_Enroll_Add_Face_Type_Set(FACE_ADD_MODE_5_FACES);
   
    user_com_event_start(USER_COM_FLAG_REGISTRATION);
    return;
}

//KID_ENROLL_SINGLE
static void handle_enroll_single(u8* pDataStart, u8 cmd)
{
    dbg_msg_console("[%s]", "KID_ENROLL_SINGLE");

    parse_enroll_msg(pDataStart, cmd);
    set_enroll_customize_uid(0xff);

    if(user_com_thread_event_check_set(USER_COM_FLAG_REGISTRATION) == true) return;

    KDP_Enroll_Add_Face_Type_Set(FACE_ADD_MODE_1_FACE);
   // set_enroll_overwrite_flag(1);//zcy md for re-enroll
	set_enroll_overwrite_flag(0);
    user_com_event_start(USER_COM_FLAG_REGISTRATION);
    return;
}

//KID_ENROLL_ITG
static void handle_enroll_itg(u8* pDataStart, u8 cmd)
{
    dbg_msg_console("[%s]", "KID_ENROLL_ITG");

    parse_enroll_msg(pDataStart, cmd);
    set_enroll_customize_uid(0xff);

    dbg_msg_console("msg_enroll:enroll_type:%d, enable_duplicate:%d", 
        msg_enroll.enroll_type, msg_enroll.enable_duplicate);

    if(user_com_thread_event_check_set(USER_COM_FLAG_REGISTRATION) == true) return;

    if (0 == msg_enroll.enroll_type) {
        KDP_Enroll_Add_Face_Type_Set(FACE_ADD_MODE_5_FACES);
    } else {
        KDP_Enroll_Add_Face_Type_Set(FACE_ADD_MODE_1_FACE);
    }

    if (msg_enroll.enable_duplicate)
        set_enroll_overwrite_flag(1);
    else
        set_enroll_overwrite_flag(0);
    
    user_com_event_start(USER_COM_FLAG_REGISTRATION);
    return;
}

//KID_FACE_RESET
static void handle_face_reset(u8 cmd)
{
#ifdef KID_FACE_RESET
    if(KID_FACE_RESET == cmd) dbg_msg_console("[%s]", "KID_FACE_RESET");
#endif
#ifdef KID_RESET
    if (KID_RESET == cmd)     dbg_msg_console("[%s]", "KID_RESET");
#endif

    s32 ret = 0;
    if(kdp_e2e_db_write_lock() == 0)
    {
        if (DB_FLASH_WRITE == db_write)
        {
            u32 cnt = 200*3;
            while ((db_write != DB_FACE_ADD_IDLE) && (cnt > 0))
            {
                osDelay(5);
                cnt--;
            }

            kl520_api_face_del(2, m_curr_user_id);
            dbg_msg_console("[1]Del usr %#x", m_curr_user_id);
            send_reply_AesNoDataMsg(ret, cmd);
        }
        else
        {
            g_bResponseEn = TRUE;

            user_com_event_interrupt();
            ret = DSM_Uart_FaceReset();

            g_bResponseEn = FALSE;
            send_reply_AesNoDataMsg(ret, cmd);
        }
        kdp_e2e_db_write_unlock();
    }
    else
    {
        u32 cnt = 200*3;
        while ((db_write != DB_FACE_ADD_IDLE) && (cnt > 0))
        {
            osDelay(5);
            cnt--;
        }

        kl520_api_face_del(2, m_curr_user_id);
        dbg_msg_console("[2]Del usr %#x", m_curr_user_id);
        send_reply_AesNoDataMsg(ret, cmd);
    }
    
    return;
}

//KID_DEL_USER
static void handle_del_user(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_DEL_USER");

    _user_com_data = StreamsToBigEndU16(pDataStart);
    dbg_msg_com(" shudown:%d; timeout:%d ",*pDataStart,*(pDataStart+1));

    if(user_com_thread_event_check_set(USER_COM_FLAG_DELETE_ONE) == true)   return;

    user_com_event_start(USER_COM_FLAG_DELETE_ONE);
    return;
}

//KID_DEL_ALL
static void handle_del_all(void)
{
    dbg_msg_console("[%s]", "KID_DEL_ALL");

    if(user_com_thread_event_check_set(USER_COM_FLAG_DELETE_ALL) == true)   return;

    user_com_event_start(USER_COM_FLAG_DELETE_ALL);
    return;
}

//KID_GET_USER_INFO
static void handle_get_user(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_GET_USER_INFO");

    _user_com_data = StreamsToBigEndU16(pDataStart);
    msg_get_user_info_data userinfo;
    s32 ret = DSM_Uart_GetUserInfo(_user_com_data, &userinfo);
    response_get_user_info_msg(ret, userinfo);

    return;
}

//KID_GET_ALL_USER_ID
static void handle_get_all(void)
{
    dbg_msg_console("[%s]", "KID_GET_ALL_USER_ID");

    msg_get_all_user_id_data allUserInfo;
    s32 ret = DSM_Uart_GetAllUserInfo(&allUserInfo);
    response_get_Alluser_info_msg(ret, allUserInfo);
    return;
}

//KID_SNAP_IMAGE
static void handle_snap_image(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_SNAP_IMAGE");

    msg_snap_img_data tInfo;
    memcpy(&tInfo, pDataStart, sizeof(tInfo));
    
    if(user_com_thread_event_check_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE) == true) {
        uint8_t result = MR_FAILED_NOT_READY;
        send_snapImage_reply_msg(result);
        return;
    }
    
    s32 ret = DSM_Uart_SnapImage(tInfo);
    if(ret != MR_SUCCESS) {
        send_snapImage_reply_msg(ret);
        user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
    } else {
        user_com_event_start(USER_COM_FLAG_SNAP_IMG); //start snap image
    }
    return;
}

//KID_GET_SAVED_IMAGE
static void handle_get_saved_image(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_GET_SAVED_IMAGE");

    uint8_t image_num = *pDataStart;//image_num
    uint8_t image_size[4] = {0};
    s32 ret = DSM_Uart_SavedImage(image_num, image_size);
    send_savedImage_reply_msg(ret, image_size);
    return;
}

//KID_UPLOAD_IMAGE
static void handle_upload_image(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_UPLOAD_IMAGE");

    uint8_t *upload_image_data = NULL;

    uint32_t upload_image_offset = kl_ntohpl(pDataStart);
    uint32_t upload_image_size   = kl_ntohpl(pDataStart+4);

    s32 ret = DSM_Uart_UploadImage(upload_image_offset, &upload_image_data, &upload_image_size);
    send_uploadImage_reply_msg(ret, upload_image_data, upload_image_size);
    return;
}

//KID_CONFIG_BAUDRATE
static void handle_config_baudrate(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_CONFIG_BAUDRATE");

    uint8_t baudrate_index = *pDataStart;
    s32 ret = DSM_Uart_ConfigBaudrate(baudrate_index);
    //send_ConfigBaurate_reply_msg(ret);
    return;
}

//KID_SET_THRESHOLD_LEVEL
static void handle_threshold_level(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_SET_THRESHOLD_LEVEL");

    uint8_t verify_level = *pDataStart;
    uint8_t live_level 	 = *(pDataStart+1);
    s32 ret = DSM_Uart_SET_THRESHOLD_LEVEL(verify_level,live_level);
    if (MR_SUCCESS == ret)
    {
        kl520_customer_info Cusinfo;
        kl520_api_customer_get(&Cusinfo);
        Cusinfo.verify_threshold = verify_level;
        Cusinfo.live_threshold = live_level;
        kl520_api_customer_write(&Cusinfo);
    }

    send_AlgThreshold_level_reply_msg(ret);
    return;
}

//KID_SW_BOOT_PART
static void handle_sw_boot_part(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_SW_BOOT_PART");

    uint8_t nSwPart = *pDataStart;
    uint8_t nCurPart = 0xFF;
    s32 ret = DSM_Uart_SwitchPart(nSwPart, &nCurPart);

    send_SwitchPart_reply_msg(ret, nCurPart);

#if (CFG_KL520_VERSION == KL520A)
    if ( ret == MR_SUCCESS )
    {
        osDelay(10);
        user_com_event_power_off();
    }
#endif
    return;
}

//KID_GET_CUR_PART
static void handle_get_cur_part(void)
{
    dbg_msg_console("[%s]", "KID_GET_CUR_PART");

    uint8_t nCurPart = 0xFF;
    s32 ret = DSM_Uart_Get_Cur_Part(&nCurPart);

    send_GetCurPart_reply_msg(ret, nCurPart);
    return;
}

//KID_MP_CALIBRATION
static void handle_mp_calibration(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_MP_CALIBRATION");

    user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);

    u8 eMode = *pDataStart;
    s32 ret = KDP_Uart_MP_Calibration(eMode);

    send_MpCalibration_reply_msg(ret);
    db_write = DB_FACE_ADD_IDLE;

    user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
    return;
}

//KID_INIT_ENCRYPTION
static void handle_init_encryption(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_INIT_ENCRYPTION");
         
    msg_init_encryption_data encryption;
    uint8_t device_id[DEVICE_ID_NUM] = {0};
    memcpy(&encryption,pDataStart,sizeof(encryption));
    s32 ret = DSM_Uart_Init_Encryption(encryption, device_id);
			osDelay(10);//zcy add for delay 10ms for hongyan-xinhongjia-2022-11-14
    send_InitEncryption_reply_msg(ret, device_id);

    return;
}

//KID_SET_RELEASE_ENC_KEY
static void handle_set_release_key(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_SET_RELEASE_ENC_KEY");

    msg_enc_key_number_data key_number;
    memcpy(&key_number,pDataStart,sizeof(key_number));
    s32 ret = DSM_Uart_SetReleaseEncKey(key_number);
    send_SetReleaseOrDebugEncKey_reply_msg(ret, KID_SET_RELEASE_ENC_KEY);

    return;
}

//KID_SET_DEBUG_ENC_KEY
static void handle_set_debug_key(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_SET_DEBUG_ENC_KEY");

    msg_enc_key_number_data key_number2;
    memcpy(key_number2.enc_key_number,pDataStart,sizeof(key_number2.enc_key_number));
    s32 ret = DSM_Uart_SetDebugEncKey(key_number2);
    send_SetReleaseOrDebugEncKey_reply_msg(ret, KID_SET_DEBUG_ENC_KEY);

    return;
}

//KID_USER_ROTATE_180
#ifdef KID_USER_ROTATE_180
static void handle_user_rotate_sensor(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_USER_ROTATE_180");

    u8 user_rotate_enable = *pDataStart;

    kl520_customer_info Cusinfo;
    kl520_api_customer_get(&Cusinfo);

    if (user_rotate_enable)
        user_rotate_enable = ROTATE_180_ENABLE;
    else
        user_rotate_enable = ROTATE_180_DISABLE;

    if (Cusinfo.user_rotate_180 !=user_rotate_enable)
    {
        dbg_msg_console("Save rotate setting to flash");
        Cusinfo.user_rotate_180 =user_rotate_enable;
        kl520_api_customer_write(&Cusinfo);
    }

    send_reply_AesNoDataMsg(MR_SUCCESS, KID_USER_ROTATE_180);
    
    return;
}
#endif

#ifdef KID_START_OTA
//KID_START_OTA
static void handle_start_ota(u8* pDataStart)
{
    uint8_t v_primary   = *pDataStart;
    uint8_t v_secondary = *(pDataStart+1);
    uint8_t v_revision  = *(pDataStart+2);
    //dbg_msg_console("v_primary:0x%02x, v_secondary:%02x, v_revision:%02x ",v_primary,v_secondary,v_revision);
    s32 ret = DSM_Uart_StartOta(v_primary,v_secondary,v_revision);
    send_StartOrStopOta_reply_msg(ret,KID_START_OTA);

    return;
}

//KID_STOP_OTA
static void handle_stop_ota(void)
{
    dbg_msg_console("[%s]", "KID_STOP_OTA");

    s32 ret = DSM_Uart_StopOta();
    send_StartOrStopOta_reply_msg(ret,KID_STOP_OTA);
    
    power_mgr_sw_reset();
    return;
}

//KID_GET_OTA_STATUS
static void handle_get_ota_status(void)
{
    dbg_msg_console("[%s]", "KID_GET_OTA_STATUS");

    uint8_t ota_status1;
    uint8_t next_pid_e1[2];
    s32 ret = DSM_Uart_GetOtaStatus(&ota_status1,next_pid_e1);
    send_OtaStatus_reply_msg(ret, ota_status1, next_pid_e1);
    return;
}

//KID_OTA_HEADER
static void handle_ota_header(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_OTA_HEADER");

    msg_mass_data_header otaheader={0};

    memcpy(&otaheader, pDataStart, (sizeof(otaheader)-sizeof(otaheader.ext_data)));
    s32 ret = DSM_Uart_OtaHeader(&otaheader);
    send_OtaHeaderOrPacket_reply_msg(ret, KID_OTA_HEADER);

    return;
}

//KID_OTA_PACKET
static void handle_ota_packet(u8* pDataStart)
{
    msg_mass_data_pkg otapacket;

    memcpy(&otapacket, pDataStart, sizeof(otapacket.nPkgIdx)+sizeof(otapacket.nPkgSize));
    otapacket.pDataHeader = (u8*) (pDataStart + sizeof(otapacket.nPkgIdx)+sizeof(otapacket.nPkgSize));

    s32 ret = DSM_Uart_OtaPacket(otapacket);

    send_OtaHeaderOrPacket_reply_msg(ret, KID_OTA_PACKET);
    // all data receive done and start ota process
    return;
}
#endif

#ifdef KID_DB_EXPORT_REQUEST
//KID_DB_EXPORT_REQUEST
static void handle_db_export(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_DB_EXPORT_REQUEST");

    u16 user_id = (pDataStart[0] << 8) | pDataStart[1];

    u16 u_idx = kdp_app_db_find_exist_id(user_id);
    if(u_idx == MAX_USER)
    {
        send_db_export_reply_msg(MR_FAILED_INVALID_PARAM, 0, 0, 0);
    }
    else
    {
        uint32_t db_addr = kdp_app_db_get_all_info_user_addr(u_idx);
        uint32_t total_size = kdp_app_db_get_all_info_user_size();
        update_db_export_info(db_addr, total_size);
        dbg_msg_console("user_id:%d, total_size:%d", user_id, total_size);

        u8 md5_crc[32], md5[16];
        md5_main((unsigned char*)db_addr, md5, total_size);
        md5_covers(md5 ,md5_crc);
        dbg_msg_nocrlf("md5: ");
        for (size_t i = 0; i < sizeof(md5_crc); i++)
        {
            dbg_msg_nocrlf("%c", md5_crc[i]);
        }
        dbg_msg_nocrlf("\n");

        send_db_export_reply_msg(MR_SUCCESS, user_id, total_size, md5_crc);
    }
    
    return;
}

//KID_DB_IMPORT_REQUEST
static void handle_db_import(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_DB_IMPORT_REQUEST");

    u8 result = MR_SUCCESS;

    uint16_t user_id = (pDataStart[0] << 8) | pDataStart[1];
    uint32_t size = (pDataStart[2] << 24) | (pDataStart[3] << 16) | (pDataStart[4] << 8) | pDataStart[5];
    uint8_t import_mode = pDataStart[6];

    update_db_import_info(0, 0, 0xff);

    int32_t ret = kdp_app_db_import_request(&user_id, size, import_mode);
    uint32_t db_addr;

    switch (ret)
    {
    case 0:
        db_addr = KDP_DDR_TEST_EXTRA_DB_ADDR;
        dbg_msg_console("db_addr:0x%08x, size:%d, user_id:%d", db_addr, size, user_id);
        update_db_import_info(db_addr, size, user_id);
        break;
    
    case -1:
        // No space
        result = MR_FAILED_MAX_USER;
        break;
    
    case -2:
        // MR_FAILED_FACE_ENROLLED
        result = MR_FAILED_FACE_ENROLLED;
        break;
    
    case -3:
    case -4:
        // MR_FAILED_INVALID_PARAM
        result = MR_FAILED_INVALID_PARAM;
        break;

    default:
        break;
    }

    send_db_import_request_reply_msg(result, user_id);
    return;
}


//KID_UPLOAD_DATA
static void handle_upload_data(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_UPLOAD_DATA");

    uint32_t offset = (pDataStart[0] << 24) | (pDataStart[1] << 16) | (pDataStart[2] << 8) | pDataStart[3];
    uint32_t size = (pDataStart[4] << 24) | (pDataStart[5] << 16) | (pDataStart[6] << 8) | pDataStart[7];
    u8 request_cmd = pDataStart[8];

    dbg_msg_console("offset:%d, size:%d, request_cmd:0x%02x", offset, size, request_cmd);

    uint32_t data_addr = 0;
    uint32_t data_size = 0;

    switch (request_cmd)
    {
#ifdef KID_DB_EXPORT_REQUEST
    case KID_DB_EXPORT_REQUEST:
        data_addr = get_db_export_addr();
        data_size = get_db_export_size();
        break;
#endif
    
    default:
        dbg_msg_console("request_cmd %d is invalid", request_cmd);
        break;
    }

    if ((data_addr) && (offset < data_size))
    {
        uint32_t upload_addr;
        upload_addr = data_addr + offset;
        if ((offset + size) > data_size)
            size = data_size - offset;

        send_upload_data_reply_msg(MR_SUCCESS, upload_addr, size);
    }
    else
    {
        dbg_msg_console("");
        send_upload_data_reply_msg(MR_FAILED_INVALID_PARAM, 0, 0);
    }
    
    return;
}

//KID_DOWNLOAD_DATA
static void handle_download_data(u8* pDataStart)
{
    uint32_t pkg_offset = (pDataStart[0] << 24) | (pDataStart[1] << 16) | (pDataStart[2] << 8) | pDataStart[3];
    uint32_t pkg_size = (pDataStart[4] << 24) | (pDataStart[5] << 16) | (pDataStart[6] << 8) | pDataStart[7];
    uint8_t data_type = pDataStart[8];

    dbg_msg_console("[%s] offset:%d, size:%d, data_type:0x%02x", "KID_DOWNLOAD_DATA", pkg_offset, pkg_size, data_type);

    uint32_t addr, size;
    u8 result = MR_SUCCESS;
    u16 user_id;
    uint32_t ret;

    switch (data_type)
    {
#ifdef KID_DB_IMPORT_REQUEST
    case KID_DB_IMPORT_REQUEST:
        addr = get_db_import_addr();
        size = get_db_import_size();
        break;
#endif
    
    default:
        break;
    }

    if ((addr) && ((pkg_offset + pkg_size) <= size))
    {
        memcpy((void *)(addr + pkg_offset), &pDataStart[9], pkg_size);
        if ((pkg_offset + pkg_size) == size)
        {
            switch (data_type)
            {
#ifdef KID_DB_IMPORT_REQUEST
            case KID_DB_IMPORT_REQUEST:
                user_id = get_db_import_user_id();
                ret = kdp_app_db_import(user_id, (u8 *)addr, size);
                if(0 == ret)
                {
                    update_db_import_info(0, 0, 0xff);
                    result = MR_SUCCESS;
                }
                else
                    result = MR_FAILED_INVALID_PARAM;
                dbg_msg_console("db_import ret:%d", ret);
                break;
#endif
            
            default:
                break;
            }
            
        }

        send_reply_AesNoDataMsg(result, KID_DOWNLOAD_DATA);
    }
    else
    {
        // MR_FAILED_INVALID_PARAM
        send_reply_AesNoDataMsg(MR_FAILED_INVALID_PARAM, KID_DOWNLOAD_DATA);
    }
    return;
}
#endif

//KID_SET_IMP_MASS_DATA_HEADER
#ifdef KID_SET_IMP_MASS_DATA_HEADER
static void handle_import_mass_data_header(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_SET_IMP_MASS_DATA_HEADER");

    msg_mass_data_header tImpHeader = {0};

    memcpy(&tImpHeader, pDataStart, sizeof(tImpHeader));
    s32 ret = KDP_set_mass_data_header(&tImpHeader);

    send_SetMassDataHeader_reply_msg(ret, KID_SET_IMP_MASS_DATA_HEADER);
    return;
}
#endif

//KID_IMP_FM_DATA
#ifdef KID_IMP_FM_DATA
static void handle_import_fm_data(u8* pDataStart)
{
    dbg_msg_console("[%s]", "KID_IMP_FM_DATA");

    user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
    msg_mass_data_pkg tInfo;

    memcpy(&tInfo, pDataStart, sizeof(tInfo.nPkgIdx)+sizeof(tInfo.nPkgSize));
    tInfo.pDataHeader = (u8*) (pDataStart + sizeof(tInfo.nPkgIdx)+sizeof(tInfo.nPkgSize));

    KDP_imp_mass_data(&tInfo, KID_IMP_FM_DATA);

    user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
    return;
}
#endif


void user_lwcom_parser( struct st_com_type *st_com )  //lmm-edit  parser func.
{
    //s32 ret = 0;
    u8 cmd = st_com->cmd;
    u8* pDataStart = st_com->parser_buffer + st_com->data_start_index;

#ifdef GET_HOST_CAMERA_STATUS
    if(GET_HOST_CAMERA_STATUS())
    {
        send_reply_AesNoDataMsg(MR_FAILED_CATEYE_RUNNING, cmd);
        return;
    }
#endif

    wait_fr_update_thread();
    
#if ( CFG_GUI_ENABLE == YES )
    user_com_src = GUI_FSM_SRC_USER_COM;
#endif

    switch( cmd )
    {
#ifdef KID_KN_DEVICE_INFO
        case KID_KN_DEVICE_INFO:   //v//DEVICE_INFO:
            handle_kn_device_info();
            break;
#endif

#ifdef KID_GET_VERSION
        case KID_GET_VERSION:   //v//MID_GET_VERSION:
            handle_get_version();
            break;
#endif
#ifdef KID_GET_VERSION_ZA
				case KID_GET_VERSION_ZA:
					handle_get_version_zhian();
					break;
#endif
#ifdef KID_GET_VERSION_ZAPRD
				case KID_GET_VERSION_ZAPRD:
					handle_get_version_zhian_prd();
					break;
#endif
				
#ifdef KID_GET_VERSION_HARDWARE
				case KID_GET_VERSION_HARDWARE:
					handle_get_version_zhian_hard();
					break;
#endif

#ifdef KID_DEMO_MODE
        case KID_DEMO_MODE:  //MID_DEMOMODE:
            handle_demo_mode(pDataStart);
            break;
#endif

#ifdef KID_DEBUG_MODE
        case KID_DEBUG_MODE:    //MID_DEBUG_MODE:
            handle_debug_mode(pDataStart);
            break;
#endif

#ifdef KID_SOFT_RESET
        case KID_SOFT_RESET:         //MID_RESET:
            handle_soft_reset(pDataStart);
            break;
#endif

#ifdef KID_POWERDOWN
        case KID_POWERDOWN:     //MID_POWERDOWN:                //debug ok
            handle_power_down();
            break;
#endif

#ifdef KID_GET_STATUS
        case KID_GET_STATUS:    //MID_GETSTATUS:
            handle_get_status();
            break;
#endif

#ifdef KID_VERIFY
        case KID_VERIFY:        //MID_VERIFY:  //lmm-debug
            handle_kid_verify(pDataStart);
            break;
#endif

#if defined KID_ENROLL || defined KID_ENROLL_OVERWRITE
#ifdef KID_ENROLL_OVERWRITE
        case KID_ENROLL_OVERWRITE:
#endif
#ifdef KID_ENROLL
        case KID_ENROLL:
#endif
            handle_kid_enroll(pDataStart, cmd);
            break;
#endif

#ifdef KID_ENROLL_SINGLE
        case KID_ENROLL_SINGLE:
            handle_enroll_single(pDataStart, cmd);
            break;
#endif

#ifdef KID_ENROLL_ITG
        case KID_ENROLL_ITG:
            handle_enroll_itg(pDataStart, cmd);
            break;
#endif

#if defined(KID_FACE_RESET) ||  defined(KID_RESET)
#ifdef KID_RESET
        case KID_RESET:
#endif
#ifdef KID_FACE_RESET
        case KID_FACE_RESET:    //MID_FACERESET:
#endif
            handle_face_reset(cmd);
            break;
#endif

#ifdef KID_DEL_USER
        case KID_DEL_USER:          //MID_DELUSER:
            handle_del_user(pDataStart);
            break;
#endif

#ifdef KID_DEL_ALL
        case KID_DEL_ALL:           //MID_DELALL:
            handle_del_all();
            break;
#endif

#ifdef KID_GET_USER_INFO
        case KID_GET_USER_INFO:     //MID_GETUSERINFO:  //debug ok
            handle_get_user(pDataStart);
            break;
#endif

#ifdef KID_GET_ALL_USER_ID
        case KID_GET_ALL_USER_ID:   //MID_GET_ALL_USERID:	//debug ok
            handle_get_all();
            break;
#endif

#if defined KID_SNAP_IMAGE || defined KID_KN_SNAP_IMAGE
#ifdef KID_SNAP_IMAGE
        case KID_SNAP_IMAGE:
#endif
#ifdef KID_KN_SNAP_IMAGE
        case KID_KN_SNAP_IMAGE:
#endif
            handle_snap_image(pDataStart);
            break;
#endif

#if defined KID_GET_SAVED_IMAGE || defined KID_KN_GET_SAVED_IMAGE
#ifdef KID_GET_SAVED_IMAGE
        case KID_GET_SAVED_IMAGE:
#endif
#ifdef KID_KN_GET_SAVED_IMAGE
        case KID_KN_GET_SAVED_IMAGE:
#endif
            handle_get_saved_image(pDataStart);
            break;
#endif

#if defined KID_UPLOAD_IMAGE || defined KID_KN_UPLOAD_IMAGE
#ifdef KID_UPLOAD_IMAGE
        case KID_UPLOAD_IMAGE:
#endif
#ifdef KID_KN_UPLOAD_IMAGE
        case KID_KN_UPLOAD_IMAGE:
#endif
            handle_upload_image(pDataStart);
            break;
#endif

#ifdef KID_CONFIG_BAUDRATE
        case KID_CONFIG_BAUDRATE:   //MID_CONFIG_BAUDRATE:
            handle_config_baudrate(pDataStart);
            break;
#endif

#ifdef KID_SET_THRESHOLD_LEVEL
        case KID_SET_THRESHOLD_LEVEL:   //MID_SET_THRESHOLD_LEVEL:
            handle_threshold_level(pDataStart);
            break;
#endif

#ifdef KID_SW_BOOT_PART
        case KID_SW_BOOT_PART:
            handle_sw_boot_part(pDataStart);
            break;
#endif

#ifdef KID_GET_CUR_PART
        case KID_GET_CUR_PART:
            handle_get_cur_part();
            break;
#endif

#ifdef KID_MP_CALIBRATION
        case KID_MP_CALIBRATION:
            handle_mp_calibration(pDataStart);
            break;
#endif

#if ( ENCRYPTION_MODE&AES_ENCRYPTION ) || ( ENCRYPTION_MODE&XOR_ENCRYPTION )
#ifdef KID_INIT_ENCRYPTION
        case KID_INIT_ENCRYPTION:   //MID_INIT_ENCRYPTION:
            handle_init_encryption(pDataStart);
            break;
#endif

#ifdef KID_SET_RELEASE_ENC_KEY
        case KID_SET_RELEASE_ENC_KEY:   //MID_SET_RELEASE_ENC_KEY:
            handle_set_release_key(pDataStart);
            break;
#endif

#ifdef KID_SET_DEBUG_ENC_KEY
        case KID_SET_DEBUG_ENC_KEY: //MID_SET_DEBUG_ENC_KEY:
            handle_set_debug_key(pDataStart);
            break;
#endif
#endif

#ifdef KID_USER_ROTATE_180
        case KID_USER_ROTATE_180:
            handle_user_rotate_sensor(pDataStart);
            break;
#endif

//-----0xF8-0xFF OTA
#ifdef KID_START_OTA
        case KID_START_OTA:         //MID_START_OTA
            handle_start_ota(pDataStart);
            break;
#endif

#ifdef KID_STOP_OTA
        case KID_STOP_OTA:          //MID_STOP_OTA:
            handle_stop_ota();
            break;
#endif

#ifdef KID_GET_OTA_STATUS
        case KID_GET_OTA_STATUS:    //MID_GET_OTA_STATUS:
            handle_get_ota_status();
            break;
#endif

#ifdef KID_OTA_HEADER
        case KID_OTA_HEADER:    //MID_OTA_HEADER:
            handle_ota_header(pDataStart);
            break;
#endif

#ifdef KID_OTA_PACKET
        case KID_OTA_PACKET:    //MID_OTA_PACKET:   //Lucien
            handle_ota_packet(pDataStart);
            break;
#endif

#ifdef KID_DB_EXPORT_REQUEST
        case KID_DB_EXPORT_REQUEST:
            handle_db_export(pDataStart);
            break;
#endif

#ifdef KID_DB_IMPORT_REQUEST
        case KID_DB_IMPORT_REQUEST:
            handle_db_import(pDataStart);
            break;
#endif

#ifdef KID_UPLOAD_DATA
        case KID_UPLOAD_DATA:
            handle_upload_data(pDataStart);
            break;
#endif

#ifdef KID_DOWNLOAD_DATA
        case KID_DOWNLOAD_DATA:
            handle_download_data(pDataStart);
            break;
#endif

#ifdef KID_SET_IMP_MASS_DATA_HEADER
        case KID_SET_IMP_MASS_DATA_HEADER:
            handle_import_mass_data_header(pDataStart);
            break;
#endif

#ifdef KID_IMP_FM_DATA
        case KID_IMP_FM_DATA:
            handle_import_fm_data(pDataStart);
            break;
#endif

#ifdef KID_DEVICE_INFO
        case KID_DEVICE_INFO:   //v//DEVICE_INFO:
        {
            dbg_msg_console("[%s]", "KID_DEVICE_INFO");

            device_info_data device_info;
            ret = DSM_Uart_GetDeviceInfo(&device_info);
            send_GetDeviceInfo_reply_msg(ret, device_info);
            //free(device_info);
            break;
        }
#endif

#ifdef KID_GET_DEBUG_INFO
        case KID_GET_DEBUG_INFO:    //MID_GET_DEBUG_INFO:
        {
            dbg_msg_console("[%s]", "KID_GET_DEBUG_INFO");

            //pDataStart = st_com->parser_buffer + st_com->data_start_index;
            uint8_t debug_file_size[4]={0};
            ret = DSM_Uart_GET_DebugInfo(debug_file_size);//wait lmm to edit
            send_GetDebugInfo_reply_msg(ret,debug_file_size);
            break;
        }
#endif

#ifdef KID_UPLOAD_DEBUG_INFO
        case KID_UPLOAD_DEBUG_INFO: //MID_UPLOAD_DEBUG_INFO:
        {
            dbg_msg_console("[%s]", "KID_UPLOAD_DEBUG_INFO");

            pDataStart = st_com->parser_buffer + st_com->data_start_index;
            uint8_t upload_debug_info_offset[4], upload_debug_info_size[4];
            memcpy(upload_debug_info_offset,pDataStart,sizeof(upload_debug_info_offset));
            memcpy(upload_debug_info_size,pDataStart+4,sizeof(upload_debug_info_size));
            ret = DSM_Uart_UploadDebugInfo(upload_debug_info_offset, upload_debug_info_size);
            send_UploadDebugInfo_reply_msg(ret, upload_debug_info_offset, upload_debug_info_size);
            break;
        }
#endif

#ifdef KID_GET_LOG_FILE
        case KID_GET_LOG_FILE:       //MID_GET_LOGFILE:
        {
            dbg_msg_console("[%s]", "KID_GET_LOG_FILE");

            pDataStart = st_com->parser_buffer + st_com->data_start_index;
            uint8_t log_type = *pDataStart;
            uint8_t log_size[4];
            ret = DSM_Uart_GetLogFile(log_type,log_size);
            send_GetLogFile_reply_msg(ret, log_size);
            //free(log_size);
            break;
        }
#endif

#ifdef KID_UPLOAD_LOG_FILE
        case KID_UPLOAD_LOG_FILE:    //MID_UPLOAD_LOGFILE:
        {
            dbg_msg_console("[%s]", "KID_UPLOAD_LOG_FILE");

            uint8_t* logdata = NULL;
            msg_upload_logfile_data logData;
            pDataStart = st_com->parser_buffer + st_com->data_start_index;
            memcpy(&logData,pDataStart,sizeof(logData));
            uint16_t log_size2 = StreamsToBigEndU16(&logData.upload_logfile_size[0]);
            ret = DSM_Uart_UploadLogFile(logdata,logData);
            send_UploadLogFile_reply_msg(ret, logdata, log_size2);
            break;
        }
#endif

#ifdef KID_SW_EXP_FM_MODE
        case KID_SW_EXP_FM_MODE:        //MID_SW_EXP_FM_MODE:
        {
            dbg_msg_console("[%s]", "KID_SW_EXP_FM_MODE");

            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);

            msg_sw_exp_mode_data tInfo;
            pDataStart = st_com->parser_buffer + st_com->data_start_index;
            memcpy(&tInfo, pDataStart, sizeof(tInfo));

            ret = KDP_export_fm_mode( (eEXTRA_FMAP_TYPE)tInfo.eType );
            send_switch_exp_fm_mode_reply_msg(MR_SUCCESS);
            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }
#endif

#ifdef KID_SW_EXP_DB_MODE
        case KID_SW_EXP_DB_MODE:        //MID_SW_EXP_DB_MODE:
        {
            dbg_msg_console("[%s]", "KID_SW_EXP_DB_MODE");

            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);

            msg_sw_exp_mode_data tInfo;
            pDataStart = st_com->parser_buffer + st_com->data_start_index;
            memcpy(&tInfo, pDataStart, sizeof(tInfo));

            ret = KDP_export_db_mode( (eEXPORT_DB_TYPE)tInfo.eType );
#if CFG_FMAP_EX_FIG_ENABLE == YES && CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_AP_CTRL_TYPE < EX_FM_UART_AP_CTRL_MAIN_DB
            if( snapshot_adv_mode == FALSE && mode_idx == 2)
            {
                kl520_api_snapshot_adv_init(MIPI_CAM_NIR, KDP_DDR_TEST_NIR_IMG_SIZE);
                kl520_api_snapshot_adv_init(MIPI_CAM_RGB, KDP_DDR_TEST_RGB_IMG_SIZE);
                kl520_api_snapshot_adv_init(MIPI_CAM_INF, KDP_DDR_TEST_INF_IMG_SIZE);
                kl520_api_snapshot_adv_select(3); // five face log mode
                kl520_api_snapshot_adv_mode();
            }
#endif //#if CFG_FMAP_EX_FIG_ENABLE == YES && CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_AP_CTRL_TYPE < EX_FM_UART_AP_CTRL_MAIN_DB
            send_switch_exp_db_mode_reply_msg(MR_SUCCESS);
            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }
#endif

#ifdef KID_EXP_FM_DATA
        case KID_EXP_FM_DATA:       //MID_EXP_FM_DATA:  //Lucien
        {
            dbg_msg_console("[%s]", "KID_EXP_FM_DATA");

            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);

            msg_mass_data_pkg tInfo;
            pDataStart = st_com->parser_buffer + st_com->data_start_index;
            memcpy(&tInfo, pDataStart, sizeof(tInfo.nPkgIdx)+sizeof(tInfo.nPkgSize));

            KDP_exp_mass_data(tInfo, KID_EXP_FM_DATA);

            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }
#endif

#ifdef KID_EXP_DB_DATA
        case KID_EXP_DB_DATA:       //MID_EXP_DB_DATA:
        {
            dbg_msg_console("[%s]", "KID_EXP_DB_DATA");

            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            msg_mass_data_pkg tInfo;
            pDataStart = st_com->parser_buffer + st_com->data_start_index;
            memcpy(&tInfo, pDataStart, sizeof(tInfo.nPkgIdx)+sizeof(tInfo.nPkgSize));

            KDP_exp_mass_data(tInfo, KID_EXP_DB_DATA);

            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }
#endif

#ifdef KID_IMP_DB_DATA
        case KID_IMP_DB_DATA:       //MID_IMP_DB_DATA:
        {
            dbg_msg_console("[%s]", "KID_IMP_DB_DATA");

            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            msg_mass_data_pkg tInfo;
            pDataStart = st_com->parser_buffer + st_com->data_start_index;
            memcpy(&tInfo, pDataStart, sizeof(tInfo.nPkgIdx)+sizeof(tInfo.nPkgSize));
            tInfo.pDataHeader = (u8*) (pDataStart + sizeof(tInfo.nPkgIdx)+sizeof(tInfo.nPkgSize));

            KDP_imp_mass_data(tInfo, KID_IMP_DB_DATA);

            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }
#endif


#ifdef KID_IMP_IMG_DATA
        case KID_IMP_IMG_DATA:       //MID_IMP_DB_DATA:
        {
            dbg_msg_console("[%s]", "KID_IMP_IMG_DATA");

            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            msg_mass_data_pkg tInfo;
            pDataStart = st_com->parser_buffer + st_com->data_start_index;
            memcpy(&tInfo, pDataStart, sizeof(tInfo.nPkgIdx)+sizeof(tInfo.nPkgSize));
            tInfo.pDataHeader = (u8*) (pDataStart + sizeof(tInfo.nPkgIdx)+sizeof(tInfo.nPkgSize));

            KDP_imp_mass_data(tInfo, KID_IMP_IMG_DATA);

            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }
#endif

#ifdef KID_CATCH_IMG_MODE
        case KID_CATCH_IMG_MODE:
        {
            dbg_msg_console("[%s]", "KID_CATCH_IMG_MODE");

            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);

            pDataStart = st_com->parser_buffer + st_com->data_start_index;
            uint16_t nSrcType = StreamsToBigEndU16(pDataStart);

            ret = KDP_catch_image_mode(nSrcType);

            send_catch_image_mode_reply_msg(ret);
            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }
#endif

#ifdef KID_EXP_IMG_DATA
        case KID_EXP_IMG_DATA:
        {
            dbg_msg_console("[%s]", "KID_EXP_IMG_DATA");

            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            msg_mass_data_pkg tInfo;
            pDataStart = st_com->parser_buffer + st_com->data_start_index;
            memcpy(&tInfo, pDataStart, sizeof(tInfo.nPkgIdx)+sizeof(tInfo.nPkgSize));
            tInfo.pDataHeader = (u8*) (pDataStart + sizeof(tInfo.nPkgIdx)+sizeof(tInfo.nPkgSize));

            KDP_exp_mass_data(tInfo, KID_EXP_IMG_DATA);

            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }
#endif

//-----0x40-0x6F Unit control
#ifdef KID_TURN_ON_CAMERA
        case KID_TURN_ON_CAMERA:    //TURN_ON_CAMERA:
        {
            dbg_msg_console("[%s]", "KID_TURN_ON_CAMERA");

            pDataStart = st_com->parser_buffer + st_com->data_start_index;
            u8 nIdx = *pDataStart;
            ret = DSM_Uart_TurnOnOffCamera(nIdx, API_CTRL_CAM_EN);
            send_Response_result_reply_msg(ret, cmd);
            break;
        }
#endif

#ifdef KID_TURN_OFF_CAMERA
        case KID_TURN_OFF_CAMERA:   //TURN_OFF_CAMERA:
        {
            dbg_msg_console("[%s]", "KID_TURN_OFF_CAMERA");

            pDataStart = st_com->parser_buffer + st_com->data_start_index;
            u8 nIdx = *pDataStart;
            ret = DSM_Uart_TurnOnOffCamera(nIdx, API_CTRL_CAM_DIS);
            send_Response_result_reply_msg(ret, cmd);
            break;
        }
#endif

#ifdef KID_TURN_ON_VIS_LED
        case KID_TURN_ON_VIS_LED:   //TURN_ON_LED:
        {
            dbg_msg_console("[%s]", "KID_TURN_ON_VIS_LED");

            ret = DSM_Uart_TurnOnOffVisLed(TRUE);
            send_Response_result_reply_msg(ret, cmd);
            break;
        }
#endif

#ifdef KID_TURN_OFF_VIS_LED
        case KID_TURN_OFF_VIS_LED:  //TURN_OFF_LED:
        {
            dbg_msg_console("[%s]", "KID_TURN_OFF_VIS_LED");

            ret = DSM_Uart_TurnOnOffVisLed(FALSE);
            send_Response_result_reply_msg(ret, cmd);
            break;
        }
#endif

#ifdef KID_TURN_ON_IR_LED
        case KID_TURN_ON_IR_LED:   //TURN_ON_LED:
        {
            dbg_msg_console("[%s]", "KID_TURN_ON_IR_LED");

            s32 ret = DSM_Uart_TurnOnOffIrLed(TRUE);
            send_Response_result_reply_msg(ret, cmd);
            break;
        }
#endif

#ifdef KID_TURN_OFF_IR_LED
        case KID_TURN_OFF_IR_LED:  //TURN_OFF_LED:
        {
            dbg_msg_console("[%s]", "KID_TURN_OFF_IR_LED");

            s32 ret = DSM_Uart_TurnOnOffIrLed(FALSE);
            send_Response_result_reply_msg(ret, cmd);
            break;
        }
#endif

#ifdef KID_TURN_ON_STRUCT_LED
        case KID_TURN_ON_STRUCT_LED:    //TURN_ON_STRUCT:
        {
            dbg_msg_console("[%s]", "KID_TURN_ON_STRUCT_LED");

            ret = DSM_Uart_TurnOnOffStructLed(TRUE);
            send_Response_result_reply_msg(ret, cmd);
            break;
        }
#endif

#ifdef KID_TURN_OFF_STRUCT_LED
        case KID_TURN_OFF_STRUCT_LED:   //TURN_OFF_STRUCT:
        {
            dbg_msg_console("[%s]", "KID_TURN_OFF_STRUCT_LED");

            ret = DSM_Uart_TurnOnOffStructLed(FALSE);
            send_Response_result_reply_msg(ret, cmd);
            break;
        }
#endif

#ifdef KID_TURN_ON_PANEL
        case KID_TURN_ON_PANEL:
        {
            dbg_msg_console("[%s]", "KID_TURN_ON_PANEL");

            ret = DSM_Uart_TurnOnOffPanel(API_CTRL_DISP_OPEN);
            send_Response_result_reply_msg(ret, cmd);
            break;
        }
#endif

#ifdef KID_TURN_OFF_PANEL
        case KID_TURN_OFF_PANEL:
        {
            dbg_msg_console("[%s]", "KID_TURN_OFF_PANEL");

            ret = DSM_Uart_TurnOnOffPanel(API_CTRL_DISP_CLOS);
            send_Response_result_reply_msg(ret, cmd);
            break;
        }
#endif

#ifdef KID_SET_EXP_TIME
        case KID_SET_EXP_TIME:   //TURN_ON_LED:
        {
            dbg_msg_console("[%s]", "KID_SET_EXP_TIME");

            s_msg_exp_time tInfo;
            pDataStart = st_com->parser_buffer + st_com->data_start_index;
            memcpy(&tInfo, pDataStart, sizeof(tInfo));
            
            s32 ret = kn_uart_set_exposure_time(tInfo);
            send_Response_result_reply_msg(ret, cmd);
            
            break;
        }
#endif

#ifdef KID_SET_GAIN
        case KID_SET_GAIN:   //TURN_ON_LED:
        {
            dbg_msg_console("[%s]", "KID_SET_GAIN");

            pDataStart = st_com->parser_buffer + st_com->data_start_index;

            u8 cam_index = *pDataStart;
            u8 gain_h = *(pDataStart+1);
            u8 gain_l = *(pDataStart+2);
            u32 gain = (gain_h << 8) | gain_l;
            
            u8 result = kn_uart_set_gain(cam_index, gain);

            send_Response_result_reply_msg(result, cmd);
            break;
        }
#endif

#ifdef KID_SET_EXP_MASS_DATA_HEADER
        case KID_SET_EXP_MASS_DATA_HEADER:
        {
            dbg_msg_console("[%s]", "KID_SET_EXP_MASS_DATA_HEADER");

            msg_mass_data_header tExpHeader = {0};
            pDataStart = st_com->parser_buffer + st_com->data_start_index;

            memcpy(&tExpHeader, pDataStart, sizeof(tExpHeader));
            ret = KDP_set_mass_data_header(&tExpHeader);

            send_SetMassDataHeader_reply_msg(ret, KID_SET_EXP_MASS_DATA_HEADER);
            break;
        }
#endif


#ifdef KID_GET_MASS_DATA_STATUS
        case KID_GET_MASS_DATA_STATUS:
        {
            dbg_msg_console("[%s]", "KID_GET_MASS_DATA_STATUS");
//            uint8_t ota_status1;
//            uint8_t next_pid_e1[2];
//            ret = DSM_Uart_GetOtaStatus(&ota_status1,next_pid_e1);
//            send_OtaStatus_reply_msg(ret, ota_status1, next_pid_e1);
            break;
        }
#endif

#ifdef KID_SYS_INIT_READY_TIME
        case KID_SYS_INIT_READY_TIME:
        {
            dbg_msg_console("[%s]", "KID_SYS_INIT_READY_TIME");

            uint8_t time[4];
            ret = DSM_Uart_SysInitReadyTime(time);
            send_SysInitReadyTime_reply_msg(ret, time);
            //free(device_info);
            break;
        }
#endif

#ifdef KID_EXP_ALL_DB
        case KID_EXP_ALL_DB:
        {
            dbg_msg_console("[%s]", "KID_EXP_ALL_DB");
            msg_mass_data_pkg tInfo;

            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            pDataStart = st_com->parser_buffer + st_com->data_start_index;
            memcpy(&tInfo, pDataStart, sizeof(tInfo.nPkgIdx)+sizeof(tInfo.nPkgSize));
            KDP_exp_mass_data(tInfo, KID_EXP_ALL_DB);
            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }
#endif

#ifdef KID_IMP_ALL_DB
        case KID_IMP_ALL_DB:
        {
            dbg_msg_console("[%s]", "KID_IMP_ALL_DB");
            msg_mass_data_pkg tInfo;

            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            pDataStart = st_com->parser_buffer + st_com->data_start_index;
            memcpy(&tInfo, pDataStart, sizeof(tInfo.nPkgIdx)+sizeof(tInfo.nPkgSize));
            tInfo.pDataHeader = (u8*) (pDataStart + sizeof(tInfo.nPkgIdx)+sizeof(tInfo.nPkgSize));
            KDP_imp_mass_data(tInfo, KID_IMP_ALL_DB);
            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }
#endif

#ifdef KID_EXP_FLASH
        case KID_EXP_FLASH:
        {
            dbg_msg_console("[%s]", "KID_EXP_FLASH");
            msg_mass_data_pkg tInfo;

            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            pDataStart = st_com->parser_buffer + st_com->data_start_index;
            memcpy(&tInfo, pDataStart, sizeof(tInfo.nPkgIdx)+sizeof(tInfo.nPkgSize));
            KDP_exp_mass_data(tInfo, KID_EXP_FLASH);
            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }
#endif

#ifdef KID_SET_INTER_ACTIVATE
        case KID_SET_INTER_ACTIVATE: //MID_SET_INTERACTIVATE:
        {
            dbg_msg_console("[%s]", "KID_SET_INTER_ACTIVATE");

            pDataStart = st_com->parser_buffer + st_com->data_start_index;
            msg_interactivate_param interactivate_param;
            memcpy(&interactivate_param,pDataStart,sizeof(interactivate_param));
            ret = DSM_Uart_SET_Interactivate(interactivate_param);
            send_SetInteractivate_reply_msg(ret, KID_SET_INTER_ACTIVATE);
            //free(&interactivate_param);
            break;
        }
#endif

#ifdef KID_SNAPSHOT_MODE
        case KID_SNAPSHOT_MODE:
        {
            pDataStart = st_com->parser_buffer + st_com->data_start_index;
        
            u8 snapshot_mode = pDataStart[0];
        
            dbg_msg_console("[%s] mode: %d", "KID_SNAPSHOT_MODE", snapshot_mode);

            if ((5 == snapshot_mode) || (6 == snapshot_mode))
            {
                send_reply_AesNoDataMsg(MR_FAILED_INVALID_PARAM, KID_SNAPSHOT_MODE);
            }
            else
            {
                sample_snapshot_auto_usb_mode(snapshot_mode);

                send_reply_AesNoDataMsg(MR_SUCCESS, KID_SNAPSHOT_MODE);
            }

            break;
        }
#endif

        default:
        {
            //if (FALSE == user_parser(cmd))
            {
                dbg_msg_console("[%s]", "Unknown command");
                send_Response_result_reply_msg(MR_REJECTED, cmd);
            }
            break;
        }
    }
}

static kl520_com_user_ops _user_com_ops = {
    .packet_analyze              = kneron_lwcom_packet_analyze,
    .parser                      = user_lwcom_parser
};

void user_com_init(void)
{
    kl520_com_reg_user_ops(&_user_com_ops);
}
#endif

static int check_face_enroll_seq(uint8_t _face_enroll_status, u8 _face_direction)
{
    if(_face_direction == KDP_FACE_DIRECTION_UNDEFINE || (_face_direction > KDP_FACE_DIRECTION_MASK))
    {
        return KDP_CHK_FACE_DIR_ERROR;
    }
    
    if(MAX_FID == 1 && _face_direction != KDP_FACE_DIRECTION_MIDDLE) {
        return KDP_CHK_FACE_DIR_ERROR;
    }

    if(_face_direction & KDP_FACE_DIRECTION_UDLR)
    { //left right up or down any one
        if((_face_enroll_status & 0x01) == 0)
        { //middle is not finished yet
            dbg_msg_console("middle face must be enrolled first, 0x%x", _face_direction);
            return KDP_CHK_FACE_DIR_ERROR; //
        }
    }

    if ( ( _face_direction & _face_enroll_status) ==_face_direction )
    {
        dbg_msg_console("face already enrolled. 0x%x", _face_direction);
        return KDP_CHK_FACE_DIR_EXISTED;
    }

    return KDP_CHK_FACE_DIR_NORMAL;
}

u8 user_com_GetOtaStatus(void)
{
    return get_mass_data_status();
}

#ifdef KID_POWERDOWN
void user_com_event_power_off(void)
{
    send_power_off_reply_msg();
#if CFG_KL520_VERSION == KL520A
    osDelay(50);
    kl520_api_poweroff();
#endif
}
#endif

extern u32 last_hb_tick;
void user_com_response_data(u8* p_data, u16 size)  //p_data cmd,size,data
{
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
    if (g_bResponseEn)
    {
        return;
    }

    if (NULL == p_data)
    {
        dbg_msg_com("NULL == p_data");
        return;
    }
    if (0 < size)
    {
        g_bAutoPowerOff = FALSE;
        g_nAutoPowerOffCnt = 0;

//        osThreadId_t thread =  osThreadGetId();
//        osPriority_t CurPriority = osThreadGetPriority (thread);
//
//        if(CurPriority != osPriorityAboveNormal5)
//            osThreadSetPriority(thread, osPriorityAboveNormal5);

        if( p_data[0]!=COM_BUS_HEAD_TX_1 || p_data[1]!=COM_BUS_HEAD_TX_2 )
        {
                        //p_data[0]=0xef;   p_data[1]=0xaa;
            //dbg_msg_console("$$222$$$$ p_data[0]:0x%02x,p_data[1]:0x%02x,p_data[2]:0x%02x $$222$$$",p_data[0],p_data[1],p_data[2]);
        }
        
        if(mutex_rsp_msg) osMutexAcquire(mutex_rsp_msg, osWaitForever);
#ifdef DEV_PKT_LOG_DETAIL
        dbg_msg_nocrlf("%d -- Tx: ", osKernelGetTickCount());
        for(u8 i=0; i<size; i++) {
            dbg_msg_nocrlf("%02x ", p_data[i]);
        }
        dbg_msg_nocrlf("\r\n");
#endif

        if( kdp_uart_write( stCom_type.uart_port, p_data, size) == UART_API_ERROR )
        {
            dbg_msg_console("Response Uart Tx fail");
        }
        last_hb_tick = osKernelGetTickCount();
        if(mutex_rsp_msg) osMutexRelease(mutex_rsp_msg);
        //dbg_msg_console("Response end");
        //kl520_com_response( &stCom_type );//lmm-edit
    }
    else
    {
        dbg_msg_console("size =< 0");
    }
#endif
}

#if ( CFG_GUI_ENABLE == YES )
static user_behavior_data fsm_data = {0xFF, 0xFF, 0xFF};
volatile static u16 _user_com_data2 = 0;
extern void user_com_set_data(u16 type, u16 data, u8 data2)
{
    fsm_data.type = type;
    fsm_data.data = data;
    fsm_data.data2 = data2;
    _user_com_data = data;
    _user_com_data2 = data2;

    user_com_src = GUI_FSM_SRC_TOUCH;
}
extern int user_com_get_data(void* arg)
{
    user_behavior_data* adapted_data = (user_behavior_data*)arg;
    adapted_data->type = fsm_data.type;
    adapted_data->data = fsm_data.data;
    adapted_data->data2 = fsm_data.data2;
    return 0;
}
#else
void kl520_api_dp_draw_two_img(u32 nImgAddr0, u32 nImgAddr1, u8 nRet)
{
#if ( CFG_PANEL_TYPE != PANEL_NULL )
    int ret;

    do
    {
        if (nRet == 0)
        {
            KDP_CHK_BREAK(kl520_api_dp_draw_bitmap(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, (void *)nImgAddr0));
        }
        else
        {
            KDP_CHK_BREAK(kl520_api_dp_draw_bitmap(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, (void *)nImgAddr1));
        }
        KDP_CHK_BREAK(kl520_api_dp_fresh());

        osDelay(DISPLAY_RESULT_HOLD_TIME);
        KDP_CHK_BREAK(kl520_api_dp_fresh_bg(BLACK, 2));
    } while (0);
#endif
}
#endif

void user_com_thread_event_set(enum USER_COM_THREAD_EVENT eState)
{
    g_eUserComThreadEvent = eState;
}

enum USER_COM_THREAD_EVENT user_com_thread_event_get(void)
{
    return g_eUserComThreadEvent;
}

#ifdef KID_DEL_ALL
static void _handle_user_com_event_delete_all(void)
{
    u16 ret = 0;
    {
#if ( CFG_FMAP_EXTRA_ENABLE == YES ) && ( CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE ) && (CFG_FMAP_AP_CTRL_TYPE == EX_FM_USB_AP_CTRL_ALL )
        KL520_api_ap_com_set_extra_fmap_status( USB_CMD_FM_DEL_ALL_e );
        while( KL520_api_ap_com_get_extra_fmap_status() != USB_CMD_FM_NULL_e ){ osDelay(10); }
        ret = kdp_api_ap_com_wait_host_cmd_result();
#else
        ret = uart_sample_face_del_all();
#ifdef CUSTOMIZE_DB_OFFSET
        reset_user_db_offset();
#endif
        dbg_msg_console(" uart_sample_face_del_all over");
        ret =0;
#endif //CFG_FMAP_AP_CTRL_TYPE == EX_FM_USB_AP_CTRL_ALL && CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_EXTRA_ENABLE == YES


        kl520_api_cam_disp_close_perm_state_chk();
        kl520_api_disp_open_chk();

#if (CFG_PANEL_TYPE > PANEL_NULL)
#if ( CFG_GUI_ENABLE == YES )
#if ( CFG_COM_URT_PROT_TYPE != COM_UART_PROT_KDP )
        gui_page_set_present_index((u8)PAGE_IDX_0);
#endif
        gui_obj_sele_draw_two_img("deS", "deF", (ret&0xFF));
        user_gui_update_renderer();
#else
        kl520_api_dp_draw_two_img(USR_DDR_IMG_DELETE_SUCCESSED_ADDR, USR_DDR_IMG_DELETE_FAILED_ADDR, (ret&0xFF));
#endif
#endif
        kl520_api_cam_disp_state_rst();

        response_delete_msg(ret, KID_DEL_ALL);//lmm-edit
    }
}
#endif

#ifdef KID_DEL_USER
static void _handle_user_com_event_delete_user(void)
{
    u16 ret = 0;
    {
#if ( CFG_FMAP_EXTRA_ENABLE == YES ) && ( CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE ) && (CFG_FMAP_AP_CTRL_TYPE == EX_FM_USB_AP_CTRL_ALL )
        kdp_api_ap_com_set_user_id( _user_com_data );
        KL520_api_ap_com_set_extra_fmap_status( USB_CMD_FM_DEL_USER_e );
        while( KL520_api_ap_com_get_extra_fmap_status() != USB_CMD_FM_NULL_e ){ osDelay(10); }
        ret = kdp_api_ap_com_wait_host_cmd_result();
#else
        if (_user_com_data < 0x0100)
        {
            if (kdp_app_db_check_user_id(_user_com_data))
                ret = uart_sample_face_del_user(_user_com_data);
            else
                ret = MR_FAILED_INVALID_PARAM;
        }
        else
        {
            u8 id0 = _user_com_data & 0xff;
            u8 id1 = (_user_com_data >> 8) & 0xff;
            
            dbg_msg_console("del some users [%d:%d]", id0, id1);
            ret = _uart_face_del_some_users(id0, id1);
            
#ifdef CUSTOMIZE_DB_OFFSET
            if (((148 == id0) && (128 == id1)) ||
                ((148 == id1) && (128 == id0)))
                reset_user_db_offset();
#endif
        }
        
        if (1 == ret)
            ret = MR_FAILED_UNKNOWN_USER;
        
#endif //CFG_FMAP_AP_CTRL_TYPE == EX_FM_USB_AP_CTRL_ALL && CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_EXTRA_ENABLE == YES

        kl520_api_cam_disp_close_perm_state_chk();
        kl520_api_disp_open_chk();

#if (CFG_PANEL_TYPE > PANEL_NULL)
#if ( CFG_GUI_ENABLE == YES )
        gui_page_set_present_index((u8)PAGE_IDX_0);
        gui_obj_sele_draw_two_img("deS", "deF", (ret&0xFF));
        if (GUI_FSM_SRC_TOUCH == user_com_src)
        {
            gui_page_set_present_index((u8)PAGE_IDX_3);
            user_gui_face_query_all_users();
        }
        user_gui_update_renderer();
#else
        kl520_api_dp_draw_two_img(USR_DDR_IMG_DELETE_SUCCESSED_ADDR, USR_DDR_IMG_DELETE_FAILED_ADDR, (ret&0xFF));
#endif
#endif
        kl520_api_cam_disp_state_rst();

        response_delete_one_msg(ret, _user_com_data);
    }
}
#endif

static void _handle_face_enroll_correct_direction(void)
{
    u16 ret = 0;
    uint8_t thd_flag = 0;

#if ( CFG_FMAP_EXTRA_ENABLE == YES ) && ( CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE )
    kdp_api_db_set_last_register_id_preprocess();
#endif

    ret = uart_face_add_timeout(msg_enroll.admin, (char *) msg_enroll.user_name, msg_enroll.face_direction, msg_enroll.timeout, msg_enroll.cmd_id);

    if ((ret & 0xFF) != 0) { //if error happened.
        //if (msg_enroll.face_direction == KDP_FACE_DIRECTION_MIDDLE) { thd_flag = 1; }//need exit thread
        if(0xFF03 == ret)
            thd_flag = 2;
        else
            thd_flag = 1;
    }
    else
    {
        if ( ( ( g_eEnrollAddFaceTypeMode == FACE_ADD_MODE_5_FACES ) && ( g_eFaceDirState == KDP_FACE_DIRECTION_MASK ) ) ||\
             ( g_eEnrollAddFaceTypeMode == FACE_ADD_MODE_1_FACE  ) )
        { //the last one
            dbg_msg_console("face enroll finished.");
            thd_flag = 1;
        }
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
        if ( ( g_tImpExpMassDataPkgInfo.nReadyType == DATA_READY_TYPE_IMP_FM ) ||
                  ( g_tImpExpMassDataPkgInfo.nReadyType == DATA_READY_TYPE_IMP_IMG ) )
        {
            dbg_msg_console("face fm import finished.");
            thd_flag = 1;
        }
#endif
    }

    if( thd_flag == 1) {
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
        if ( ( g_tImpExpMassDataPkgInfo.nReadyType == DATA_READY_TYPE_IMP_FM ) ||
                  ( g_tImpExpMassDataPkgInfo.nReadyType == DATA_READY_TYPE_IMP_IMG ) )
        {
            KDP_clr_mass_data_header();
        }
#endif

        if (g_eUserComThreadEvent <= USER_COM_THREAD_EVENT_READY)
        {
            // Interrupt
            sample_face_close();
            kl520_api_cam_disp_state_rst();
        }
        else
        {
            #if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_AP_CTRL_TYPE < EX_FM_UART_AP_CTRL_MAIN_DB && CFG_FMAP_EXTRA_ENABLE == YES
            KL520_api_ap_com_set_extra_fmap_status( USB_CMD_FM_REGISTER_e );
            kl520_api_ap_com_set_timeout_and_start( 3 );
            while( KL520_api_ap_com_get_extra_fmap_status() != USB_CMD_FM_NULL_e && kl520_api_ap_com_chk_timeout_and_stop() == FALSE ) { osDelay(10); }
            KL520_api_ap_com_set_extra_fmap_status( USB_CMD_FM_NULL_e );
            {
                u8 db_ret = kdp_api_ap_com_wait_host_db_result();
                if ( 0xFF != db_ret ) {
                    ret = KL520_FACE_OK + ((db_ret&0xFF)<<8);
                }
                else
                {
                    ret = 0xFF02;
                }
            }
            #endif //CFG_FMAP_AP_CTRL_TYPE == EX_FM_USB_AP_CTRL_ALL && CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_EXTRA_ENABLE == YES

#if ( CFG_FMAP_EXTRA_ENABLE == YES ) && ( CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE )
            kdp_api_db_set_last_register_id_postprocess();

            if ( (ret&0xFF) == 0 )
            {
                g_tImpExpMassDataPkgInfo.nReadyType = DATA_READY_TYPE_EXP_DB;
            }
#endif

            //==================================================================================================================================
            //to-do: At all uart control module, only show wait system feedback instead of successed or failed case, and not colse display.
            //==================================================================================================================================
            sample_face_close();
            kl520_api_cam_disp_close_perm_state_chk();
            kl520_api_disp_open_chk();

#if (CFG_PANEL_TYPE > PANEL_NULL)
#if ( CFG_GUI_ENABLE == YES )
            gui_page_set_present_index((u8)PAGE_IDX_0);
            gui_obj_sele_draw_two_img("reS", "reF", (ret&0xFF));
            user_gui_update_renderer();
#else
            kl520_api_dp_draw_two_img(USR_DDR_IMG_REGISTER_SUCCESSED_ADDR, USR_DDR_IMG_REGISTER_FAILED_ADDR, (ret&0xFF));
#endif
#endif
            kl520_api_cam_disp_state_rst();
        }

//                face_enroll_status = 0; //clear it
        g_eFaceDirState = 0;
        g_nFaceId = 0;
    }
    else{
        user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
        dbg_msg_com("tien-edit next_wait_fdfr");
    }
}

static void _handle_user_com_event_face_enroll(void)
{
#if ( CFG_GUI_ENABLE == YES )
    if (GUI_FSM_SRC_TOUCH == user_com_src)
    {
        kl520_api_hmi_ctrl_state_set(CTRL_GUI);

        u16 ret = uart_sample_face_add_timeout(_user_com_data);
        notify_user_behavior_event(GUI_REGISTER_RET, ret, _user_com_data2);
        return;
    }
#endif
    {
        int check_enroll_flag = check_face_enroll_seq(g_eFaceDirState, msg_enroll.face_direction);

        dbg_msg_com("g_eFaceDirState=0x%x, msg_enroll.face_direction=0x%x", g_eFaceDirState,  msg_enroll.face_direction);
        dbg_msg_com("check_enroll_flag=%d", check_enroll_flag);

        kl520_api_hmi_ctrl_state_set(CTRL_CMD);

        if ( check_enroll_flag == KDP_CHK_FACE_DIR_NORMAL )//(msg_enroll.user_name[0] != NULL) &&
        {
            _handle_face_enroll_correct_direction();
        }
        else if(check_enroll_flag == KDP_CHK_FACE_DIR_EXISTED){
            send_enroll_reply_msg(MR_SUCCESS, ((g_nFaceId >> 8) &0xFF),  ((g_nFaceId >> 0) &0xFF) , g_eFaceDirState, msg_enroll.cmd_id);
            //user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
        }
        else{
            send_enroll_reply_msg(MR_FAILED_INVALID_PARAM, ((g_nFaceId >> 8) &0xFF),  ((g_nFaceId >> 0) &0xFF) , g_eFaceDirState, msg_enroll.cmd_id);
                sample_face_close();
                g_eFaceDirState = 0;
                g_nFaceId = 0;
            //user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
        }
    }
}

static void _handle_user_com_event_face_recog(void)
{
    
    uint8_t reg_timeout;
    {  // wait to edit for powerdown
#if ( CFG_GUI_ENABLE == YES )
        if (GUI_FSM_SRC_TOUCH == user_com_src)
        {
            kl520_api_hmi_ctrl_state_set(CTRL_GUI);

            ret = uart_face_recognition_timeout(0, _user_com_data/1000);
            notify_user_behavior_event(GUI_RECOGNIZE_RET, ret, _user_com_data2);
            set_event(user_com_event_id, USER_COM_FLAG_FACE_CLOSE_DONE);
        }
        else
#endif
        {
            kl520_api_hmi_ctrl_state_set(CTRL_CMD);
            reg_timeout = _user_com_data & 0x00ff;
					
						//zcy add for test
						if(reg_timeout <=0)
							reg_timeout = VERIFY_DEFAULT_TIME;

            //calling api to do recognition
            uart_face_recognition_timeout(0, reg_timeout);

#if ( CFG_FMAP_EXTRA_ENABLE == YES ) && ( CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE )
            g_tImpExpMassDataPkgInfo.nReadyType = DATA_READY_TYPE_EXP_FM;
#endif

            kl520_api_cam_disp_close_perm_state_chk();
            kl520_api_disp_open_chk();

#if (CFG_PANEL_TYPE > PANEL_NULL)
#if ( CFG_GUI_ENABLE == YES )
            gui_page_set_present_index((u8)PAGE_IDX_0);
            gui_obj_sele_draw_two_img("rcS", "rcF", (ret&0xFF));
            user_gui_update_renderer();
#else
            kl520_api_dp_draw_two_img(USR_DDR_IMG_RECOGNIZE_SUCCESSED_ADDR, USR_DDR_IMG_RECOGNIZE_FAILED_ADDR, (ret&0xFF));
#endif
#endif

            kl520_api_cam_disp_state_rst();

            g_bRecognitionMandatoryFlag = FALSE;

#if (CFG_KL520_VERSION == KL520A)
            uint8_t is_shutdown = ( _user_com_data & 0xff00 ) >> 8;
            if(ret == MR_SUCCESS && is_shutdown) { //if shutdown is requested.
                dbg_msg_console("power off after recognition succeeded...");
                user_com_event_power_off();
            }
#endif
        }
    }
}

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
#if defined KID_SNAP_IMAGE || defined KID_KN_SNAP_IMAGE
static void _handle_user_com_event_snap_img(void)
{
    u8 ret = uart_face_snap_image();
    send_snapImage_reply_msg(ret);
    return;
}
#endif

static void _handle_user_com_event_ota_process(void)
{
    // all data receive done and start ota process
    static  uint8_t  flag_first_ota = 0;
    if(!flag_first_ota)
    {
        flag_first_ota = 1;
        uint8_t run_ota = OtaProcess_run( 0 );
        dbg_msg_console("### run_ota:%x ###", run_ota);
        if( run_ota < 2 )
        {
            send_OtaDone_Note_msg( run_ota );
            dbg_msg_console("### send_OtaDone_Note_msg ###");
            osDelay(5);
            DSM_Uart_StopOta();

            g_nAutoPowerOffCnt = 0;
            g_bAutoPowerOff = FALSE;
        } else {
            dbg_msg_console("### ota idle ###");
            DSM_Uart_StopOta();
        }

        flag_first_ota = 0;//reuse ota

        power_mgr_sw_reset();
    }
}
#endif

static void _user_com_fdfr_thread(void)
{
    u32 flags;
    while(1) {
        flags = wait_event(user_com_event_id, USER_COM_FLAG_ALL);

        if ( USER_COM_FLAG_DELETE_ALL == (flags & USER_COM_FLAG_DELETE_ALL) )
        {
#ifdef KID_DEL_ALL
            _handle_user_com_event_delete_all();
#endif
        }
#ifdef KID_DEL_USER
        else if ( USER_COM_FLAG_DELETE_ONE == (flags & USER_COM_FLAG_DELETE_ONE) ) 
        {
            _handle_user_com_event_delete_user();
        }
#endif
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
        else if ( USER_COM_FLAG_REGISTRATION == (flags & USER_COM_FLAG_REGISTRATION) ) 
        {
            _handle_user_com_event_face_enroll();
        }
#if defined KID_SNAP_IMAGE || defined KID_KN_SNAP_IMAGE
        else if ( USER_COM_FLAG_SNAP_IMG == (flags & USER_COM_FLAG_SNAP_IMG) )
        {
            _handle_user_com_event_snap_img();
        }
#endif        
        else if ( USER_COM_FLAG_RECOGNITION == (flags & USER_COM_FLAG_RECOGNITION) )
        {
            _handle_user_com_event_face_recog();
        }
    
        else if ( USER_COM_FLAG_OTA_PROCESS == (flags & USER_COM_FLAG_OTA_PROCESS) )
        {
            _handle_user_com_event_ota_process();
        }
#endif

        user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
        if(kl520_is_fdfr_abort() != 0) { //abort is ongoing
            osEventFlagsSet(user_com_fdfr_evt, USER_COM_EVENT_CMD_READY);
        }
    }
//    dbg_msg_console("tid_user_com_fdfr_thread thread exiting....");
//    tid_user_com_fdfr_thread = NULL;
//    osThreadExit();
}

static void _wait_user_com_thread_event(void)
{
    if ( user_com_thread_event_get() == USER_COM_THREAD_EVENT_ENROLL || 
        user_com_thread_event_get() == USER_COM_THREAD_EVENT_VERIFY ) //if in enroll or verify
    {
        osEventFlagsWait(user_com_fdfr_evt, USER_COM_EVENT_CMD_READY, osFlagsWaitAny, 5000);
    }
}

static void _user_com_force_abort_fdfr(void)
{
    if ( user_com_thread_event_get() == USER_COM_THREAD_EVENT_ENROLL || 
        user_com_thread_event_get() == USER_COM_THREAD_EVENT_VERIFY ) //if in enroll or verify
    {
        if(kl520_fdfr_opened() == 0) {
            osEventFlagsWait(user_com_fdfr_evt, USER_COM_EVENT_FDR_OPENED | USER_COM_EVENT_CMD_READY, \
                osFlagsWaitAny, 5000);
        }
        
        sample_face_close();
        set_event(kl520_api_get_event(), KL520_APP_FLAG_FDFR_ERR);
    }
    else
    {
        sample_face_close();
    }
}

void user_com_event_start(u32 event) {
    set_event(user_com_event_id, event);
}
void user_com_event_interrupt(void) {
    //only one thread call this func, so no need mutex
    if(kl520_is_fdfr_abort() == 1) return; 

    osEventFlagsClear(user_com_fdfr_evt, USER_COM_EVENT_CMD_READY | USER_COM_EVENT_FDR_OPENED);
    kl520_set_fdfr_abort(1); //set abort flag

    _user_com_force_abort_fdfr();
    _wait_user_com_thread_event();

    kl520_set_fdfr_abort(0); //clear abort flag
}

void init_user_com_thread(void)
{
    if (NULL == user_com_event_id){
        user_com_event_id = create_event();
    }
    if (NULL == user_com_fdfr_evt) {
        user_com_fdfr_evt = osEventFlagsNew(NULL);
    }
    if (NULL == tid_user_com_fdfr_thread){
        osThreadAttr_t attr = {
            .stack_size = 1536
        };
        tid_user_com_fdfr_thread = osThreadNew((osThreadFunc_t)_user_com_fdfr_thread, NULL, &attr);
    }    
}

#endif
#endif
