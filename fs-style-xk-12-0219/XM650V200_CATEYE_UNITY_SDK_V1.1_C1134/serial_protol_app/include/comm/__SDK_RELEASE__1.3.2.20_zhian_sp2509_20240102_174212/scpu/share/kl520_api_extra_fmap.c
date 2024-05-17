#include "board_kl520.h"
#include <stdlib.h>
#include <stdarg.h>
#include "kl520_include.h"
#include "board_ddr_table.h"
#include "kl520_api_extra_fmap.h"
#include "kl520_api_fdfr.h"
#include "kl520_api_sim.h"
#include "pinmux.h"
#include "kdp_memxfer.h"
#include "kdp_memory.h"
#include "flash.h"
#include "kl520_api_device_id.h"
#include "kdp_e2e_settings.h"
//#include "kdp_comm_app.h"

#include <math.h>

#if CFG_FMAP_EXTRA_ENABLE == YES

bool extra_db_fmap_mode = FALSE;
int g_extra_catch_idx[EXTRA_FMAP_SRC_NUM]={0};
u8 g_FmapUsbBufCnt = 0;
u8 g_FmapShotCnt = 0;
u8 g_FmapReadCnt = 0;
u32 extra_fmap_add_addr[EXTRA_FMAP_SRC_NUM][EXTRA_FMAP_ADV_NUM]={0};
u32 extra_fmap_addr[EXTRA_FMAP_SRC_NUM];
static bool _g_IsEmpty_fmap = FALSE;
static eEXTRA_FMAP_TYPE _g_extra_fmap_adv_mode = EXTRA_FMAP_CLOSE_e;

#if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE

void kdp_api_db_set_last_register_id_preprocess( void )
{
    kdp_app_db_set_last_register_id_preprocess(MAX_USER, FALSE ); // must reserver one more member for recording db
}

void kdp_api_db_set_last_register_id_postprocess( void )
{
    kdp_app_db_set_last_register_id_postprocess( MAX_USER ); // must reserver one more member for recording db
}

u8 kdp_api_ap_control_set_each_db( u16 user_idx )
{
    kapp_db_user_data_t *data = kdp_e2e_get_db_data();
    data->user_id_in = user_idx + kl520_api_get_start_user_id();
    data->flash_off = FALSE;

    // total 10KByte Feature Map
    if(!kdp_app_db_check_usr_idx(user_idx)) return FALSE;
    memcpy( (void*)kdp_app_db_get_all_info_user_addr(user_idx), (void*)0x60000000, 1024*16 );

    //header ( A5, 5A, 00, 00, 01, 00, 00, 00, FF, FF, FF, FF, ID, ID, ID, ID, 05, 00, 00, 00, 05, 00, 00, 00
    kdp_app_db_set_last_register_id_preprocess(user_idx, TRUE );

    //e2e header
    kdp_e2e_db_extra_data* vars = (kdp_e2e_db_extra_data*) kdp_app_db_get_user_setting_addr( user_idx );
    //vars->user_id = data->user_id_in;
    data->reserve_db_num = 0;

    //ddr to flash
    kdp_app_db_add( data );

    if( data->user_idx < MAX_USER )
    {
        return TRUE;
    }
    return FALSE;

}


u32 kdp_api_ap_control_del_each_db( u16 user_idx )
{
    s32 appret;

    kapp_db_user_data_t *data = kdp_e2e_get_db_data();

    data->del_all = FALSE;
    data->user_id_in = user_idx + kl520_api_get_start_user_id();
    data->user_idx = user_idx;

    appret = kdp_app_db_delete(data);

    return appret;

}

#if CFG_FMAP_AP_CTRL_TYPE < EX_FM_UART_AP_CTRL_MAIN_DB

static u16 _g_user_id = 0;
volatile static u8 _g_host_result = 0;
static eUSB_CMD_FM_STATUS _g_usb_cmd_fm_status = USB_CMD_FM_NULL_e;
static int _m_timer_ap_com_timeout = 0;
static int _m_timer_ap_com_tick = 0;

inline eUSB_CMD_FM_STATUS KL520_api_ap_com_get_extra_fmap_status( void )
{
    return _g_usb_cmd_fm_status;
}

inline void KL520_api_ap_com_set_extra_fmap_status( eUSB_CMD_FM_STATUS status )
{
    _g_usb_cmd_fm_status = status;
}

inline void kdp_api_ap_com_set_user_id( u8 user_id )
{
    _g_user_id = (u16)user_id;
}

inline u16 kdp_api_ap_com_get_usrt_id( void )
{
    return _g_user_id;
}

inline void kdp_api_ap_com_set_host_result( u8 action )
{
    _g_host_result = action;
}

inline u8 kdp_api_ap_com_get_host_result( void )
{
    return _g_host_result;
}

inline void kl520_api_ap_com_set_timeout_and_start( int time_out_s )
{
    kl520_api_timer_init(PWMTIMER3, PWMTMR_1000MSEC_PERIOD);
    _m_timer_ap_com_tick = kdp_current_t3_tick();
    _m_timer_ap_com_timeout = time_out_s;

}

bool kl520_api_ap_com_chk_timeout_and_stop( void )
{
    bool ret = FALSE;

    int _nShowimerCur = -1;

    u32 curr_tick = kdp_current_t3_tick();

    _nShowimerCur = curr_tick - _m_timer_ap_com_tick;

    if ( _nShowimerCur >= _m_timer_ap_com_timeout )
    {
        ret = TRUE;
        kl520_api_timer_close(PWMTIMER3);
    }

    return ret;

}

u8 kdp_api_ap_com_check_host_command_type( u32 check )
{
    if(check == RSP_REGISTER_RET_CMD
    || check == RSP_DELETE_ALL_RET_CMD || check == RSP_DELETE_ONE_RET_CMD
#if CFG_FMAP_AP_CTRL_TYPE == EX_FM_USB_AP_CTRL_ALL
    || check == RSP_RECOGNIZE_RET_CMD
#endif
      )
    {
        return TRUE;
    }

    return FALSE;
}

u8 kdp_api_ap_com_wait_host_cmd_result( void )
{
	while( kdp_api_ap_com_get_host_result() == 0 /*_api_fdfr_chk_timeout(m_face_recognition_timeout) != TRUE*/ )
	{
		//dbg_msg_com("host rsp: %d", kdp_api_ap_com_get_host_result() );
		osDelay(10);
	} // wait timeout or response from comparison host info
	kdp_api_ap_com_set_host_result(0);

    return (u8)kdp_api_ap_com_get_usrt_id();
}

u8 kdp_api_ap_com_wait_host_db_result( void )
{
    u8 ret = 0xFF;
    u16 user_id;

    while( kdp_api_ap_com_get_host_result() == 0 /*_api_fdfr_chk_timeout(m_face_recognition_timeout) != TRUE*/ )
    {
        //dbg_msg_com("host rsp: %d", kdp_api_ap_com_get_host_result() );
        osDelay(10);
    } // wait timeout or response from comparison host info
    kdp_api_ap_com_set_host_result(0);

    user_id = kdp_api_ap_com_get_usrt_id() ;
    if( user_id == 255 )
    {
        dbg_msg_com("host db cmd: -1", );
        return ret;
    }

    return (u8)user_id;
}

#if CFG_FMAP_AP_CTRL_TYPE == EX_FM_USB_AP_CTRL_ALL

s32 kdp_api_ap_com_wait_host_compare_result( u16 *lp_user_id )
{
    s32 ret = E2E_ERROR;

    while( kdp_api_ap_com_get_host_result() == 0 /*_api_fdfr_chk_timeout(m_face_recognition_timeout) != TRUE*/ )
    {
        //dbg_msg_com("host rsp: %d", kdp_api_ap_com_get_host_result() );
        osDelay(10);
    } // wait timeout or response from comparison host info

    kdp_api_ap_com_set_host_result(0);
    if( kdp_api_ap_com_get_usrt_id() == 255 )
    {
    dbg_msg_com("host fr cmd: -1", );
        return ret;
    }

    *lp_user_id = kdp_api_ap_com_get_usrt_id();;
    ret = E2E_OK;
    dbg_msg_com("host cmd: %d", *lp_user_id);

    return ret;

}

#endif //#if CFG_FMAP_AP_CTRL_TYPE == EX_FM_USB_AP_CTRL_ALL

#endif //#if CFG_FMAP_AP_CTRL_TYPE < EX_FM_UART_AP_CTRL_MAIN_DB

#endif // #if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE


void KL520_api_set_extra_fmap_adv_mode( eEXTRA_FMAP_TYPE type )
{
    _g_extra_fmap_adv_mode = type;
}

void kl520_api_set_extra_fmap(  int ret )
{
    if ( ret == KL520_FACE_OK ) {
        _g_IsEmpty_fmap = FALSE;
    }
    else {
        _g_IsEmpty_fmap = TRUE;
    }

    //dbg_msg_console("[fmap] face id status: %d ", _g_IsEmpty_fmap );
}

// jim : push data to snapshot_buf_addr
int kl520_api_extract_fr_map(int cam_idx )
{

    if(kl520_api_fdfr_exist_thread())
    {
        if( g_extra_catch_idx[cam_idx] == EXTRA_FMAP_CLEAR)
            return -1;
    }

    //dbg_msg_console("[fmap] push data to buf: %d , %d, %d", g_extra_catch_idx[MIPI_CAM_NIR], g_extra_catch_idx[MIPI_CAM_RGB],_g_IsEmpty_fmap );

    if( _g_IsEmpty_fmap == TRUE )
    {
        if(cam_idx == MIPI_CAM_NIR ) {   /*NIR*/
            memset((void *)extra_fmap_addr[MIPI_CAM_NIR], 0x5A, KDP_DDR_TEST_NIR_FR_SIZE);
        }
        else if(cam_idx == MIPI_CAM_RGB ){  /*RGB*/
            memset((void *)extra_fmap_addr[MIPI_CAM_RGB], 0xA5, KDP_DDR_TEST_RGB_FR_SIZE );
        }
    }
    else // ( _g_IsEmpty_fmap == FALSE )
    {
        if(cam_idx == MIPI_CAM_NIR ) {   /*NIR*/
            memcpy((void *)extra_fmap_addr[MIPI_CAM_NIR], (void*)kdp_e2e_get_n1_fr(), KDP_DDR_TEST_NIR_FR_SIZE);
        }
        else if(cam_idx == MIPI_CAM_RGB){  /*RGB*/
            memcpy((void *)extra_fmap_addr[MIPI_CAM_RGB], (void*)kdp_e2e_get_r1_fr(), KDP_DDR_TEST_RGB_FR_SIZE );
        }
    }

    if(kl520_api_fdfr_exist_thread())
        g_extra_catch_idx[cam_idx] = EXTRA_FMAP_UPDATE;

    //dbg_msg_console("[fmap] catch status: %d ", g_extra_catch_idx[cam_idx] );

    return 0;
}


int kl520_api_extra_fmap_fdfr_catch(int cam_idx)
{
    if(kl520_api_fdfr_exist_thread())
    {
        if( (g_extra_catch_idx[MIPI_CAM_NIR] > EXTRA_FMAP_CLEAR && g_extra_catch_idx[MIPI_CAM_RGB] > EXTRA_FMAP_CLEAR ) )
        {
            return 0;   //catch buffer not ready
        }
    }

    //dbg_msg_console("[fmap] read to catch frame: %d, %d, %d", g_extra_catch_idx[MIPI_CAM_RGB], g_extra_catch_idx[MIPI_CAM_NIR], _g_IsEmpty_fmap );

    g_extra_catch_idx[MIPI_CAM_NIR] = EXTRA_FMAP_START;
    g_extra_catch_idx[MIPI_CAM_RGB] = EXTRA_FMAP_START;
    _g_IsEmpty_fmap = TRUE;
    return 1;

}

int kl520_api_extra_fmap_fdfr_reset(void)
{
    if(kl520_api_fdfr_exist_thread())
    {
        if( g_extra_catch_idx[MIPI_CAM_RGB] < EXTRA_FMAP_UPDATE && g_extra_catch_idx[MIPI_CAM_NIR] < EXTRA_FMAP_UPDATE )
            return 0;
    }

    //dbg_msg_console("[fmap] done to send data and do reset frame: %d, %d, %d", g_extra_catch_idx[MIPI_CAM_RGB], g_extra_catch_idx[MIPI_CAM_NIR], _g_IsEmpty_fmap );

    g_extra_catch_idx[MIPI_CAM_RGB] = EXTRA_FMAP_CLEAR;
    g_extra_catch_idx[MIPI_CAM_NIR] = EXTRA_FMAP_CLEAR;
    _g_IsEmpty_fmap = TRUE;
    return 1;
}

u32 kl520_api_extra_fmap_ddr_addr(int cam_idx)
{
    if(cam_idx == MIPI_CAM_NIR){   /*NIR*/
        return KDP_DDR_TEST_EXTRA_NIR_ADDR;
    }
    else if(cam_idx == MIPI_CAM_RGB){   /*RGB*/
        return KDP_DDR_TEST_EXTRA_RGB_ADDR;
    }
    else{
        return KDP_DDR_TEST_EXTRA_RGB_ADDR;
    }
}

u32 kl520_api_extra_db_map_addr( void )
{

    if(kl520_api_fdfr_exist_thread())
    {
    // to-do
    // check register flow done? and db in ddr memory // jim
    }
    #if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE
    u16 reserve_db_num = 1;
    #else
    u16 reserve_db_num = 0;
    #endif
    // jim : change memory to face id ddr addr ( use rgb memory )
    // db size = 10KB + 6KB( db_header )
    memcpy((void *)KDP_DDR_TEST_EXTRA_DB_ADDR, (void *)kdp_app_db_get_all_info_user_addr( MAX_USER+reserve_db_num-1 ), KDP_DDR_TEST_USER_DB_SIZE );
    return KDP_DDR_TEST_EXTRA_DB_ADDR;   //buffer not ready
}


u32 kl520_api_extra_fmap_addr( void )
{
    #define MAX_CHECK_FMAP_FRAME_CNT (5)

    static u8 frame_cnt = 0;

    if(kl520_api_fdfr_exist_thread())
    {
        if( g_extra_catch_idx[MIPI_CAM_NIR] != EXTRA_FMAP_UPDATE || g_extra_catch_idx[MIPI_CAM_RGB] != EXTRA_FMAP_UPDATE ) {
            return 0;   //catch buffer not ready, double check fmap get
        }
        else if( _g_IsEmpty_fmap == TRUE && frame_cnt < MAX_CHECK_FMAP_FRAME_CNT )
        {
            //dbg_msg_console("[fmap] send data to usb: %d , %d, %d", g_extra_catch_idx[MIPI_CAM_NIR], g_extra_catch_idx[MIPI_CAM_RGB],_g_IsEmpty_fmap );
            frame_cnt++;
            return 0;   //catch buffer not ready, double check fmap get
        }
    }
    //dbg_msg_console("[fmap] send data to usb: %d , %d, %d", g_extra_catch_idx[MIPI_CAM_NIR], g_extra_catch_idx[MIPI_CAM_RGB],_g_IsEmpty_fmap );

    //dbg_msg_console("[fmap] done to send data to usb after %d frame", frame_cnt );
    //|| !kl520_api_fdfr_exist_thread() // && kl520_api_sim_state() == 0
    if( frame_cnt >= MAX_CHECK_FMAP_FRAME_CNT ) {
        memset((void *)KDP_DDR_TEST_EXTRA_NIR_ADDR, 0x5A, KDP_DDR_TEST_NIR_FR_SIZE);    /*NIR*/
        memset((void *)KDP_DDR_TEST_EXTRA_RGB_ADDR, 0xA5, KDP_DDR_TEST_RGB_FR_SIZE);    /*RGB*/
        //dbg_msg_console("[Snapshot]adv_load nir...cam_idx:%d, face_idx:%d",cam_idx,face_idx);
    }
    else {
        memcpy((void *)KDP_DDR_TEST_EXTRA_NIR_ADDR, (void *)extra_fmap_addr[MIPI_CAM_NIR], KDP_DDR_TEST_NIR_FR_SIZE);    /*NIR*/
        memcpy((void *)KDP_DDR_TEST_EXTRA_RGB_ADDR, (void *)extra_fmap_addr[MIPI_CAM_RGB], KDP_DDR_TEST_RGB_FR_SIZE);    /*RGB*/
        //dbg_msg_console("[Snapshot]adv_load nir...cam_idx:%d, face_idx:%d",cam_idx,face_idx);
    }
    frame_cnt = 0;

    kl520_api_extra_fmap_fdfr_reset();

    return KDP_DDR_TEST_EXTRA_RGB_ADDR;
}


void kl520_api_extra_fmap_adv_chk(void)
{
    //if(extra_db_fmap_mode == TRUE)
    {
        kl520_api_extra_fmap_fdfr_catch(MIPI_CAM_NIR);
        kl520_api_extra_fmap_fdfr_catch(MIPI_CAM_RGB);
    }
}

void kl520_api_extra_fmap_adv_init(int idx, uint32_t buf_size)
{
    int i;

    for(i=0;i<EXTRA_FMAP_ADV_NUM;i++){

        if(extra_fmap_add_addr[idx][i] == 0){
            extra_fmap_add_addr[idx][i] = kdp_ddr_reserve(buf_size);
            dbg_msg_console("[fmap]adv_init addr: 0x%d, idx:%d, buf_size:%d", extra_fmap_add_addr[idx][i], idx, buf_size);
        }
    }
}

void kl520_api_extra_fmap_mode( bool active )
{
    extra_db_fmap_mode = active;
    kl520_api_extra_fmap_fdfr_reset();

    if(extra_db_fmap_mode == TRUE){
        dbg_msg_console("[fmap] read Frame FR");
    }
    else{
        dbg_msg_console("[fmap] read DB FR");
    }
}

bool kl520_api_get_extra_fmap_mode( void )
{
    return extra_db_fmap_mode;
}


void kl520_api_extra_fmap_adv_shot(int idx, u8 face_idx)
{
    if(face_idx >= EXTRA_FMAP_ADV_NUM){
        return;
    }

    if(extra_db_fmap_mode == FALSE){
        return;
    }

    if(idx == MIPI_CAM_NIR && extra_fmap_add_addr[idx][face_idx] != 0){   /*NIR*/
        memcpy((void *)extra_fmap_add_addr[idx][face_idx], (void *)extra_fmap_addr[MIPI_CAM_NIR], KDP_DDR_TEST_NIR_FR_SIZE);
        //dbg_msg_console("[fmap]adv_shot nir...idx:%d, face_idx:%d",idx,face_idx);
    }
    else if(idx == MIPI_CAM_RGB && extra_fmap_add_addr[idx][face_idx] != 0){  /*RGB*/
        memcpy((void *)extra_fmap_add_addr[idx][face_idx], (void *)extra_fmap_addr[MIPI_CAM_RGB], KDP_DDR_TEST_RGB_FR_SIZE);
        //dbg_msg_console("[fmap]adv_shot rgb...idx:%d, face_idx:%d",idx,face_idx);
    }
    else
    {
        dbg_msg("[fmap]adv_shot err");
    }
}

void kl520_api_extra_fmap_adv_shot_save( int ret )
{
    kl520_api_set_extra_fmap(  ret );
    kl520_api_extract_fr_map(MIPI_CAM_RGB); // push rgb fr rst
    kl520_api_extract_fr_map(MIPI_CAM_NIR); // push nir fr rst

    if( extra_db_fmap_mode == FALSE )
        return;

    if(g_FmapUsbBufCnt < (EXTRA_FMAP_ADV_NUM) && _g_extra_fmap_adv_mode != EXTRA_FMAP_CLOSE_e && ( _g_IsEmpty_fmap == FALSE || _g_extra_fmap_adv_mode == EXTRA_FMAP_BYPASS_e ) )
    {
        kl520_api_extra_fmap_adv_shot(MIPI_CAM_NIR,g_FmapShotCnt);
        kl520_api_extra_fmap_adv_shot(MIPI_CAM_RGB,g_FmapShotCnt);

        g_FmapUsbBufCnt++;
        dbg_msg_console("[fmap]g_SnapShotCnt :%d, g_SnapUsbBufCnt:%d",g_FmapShotCnt, g_FmapUsbBufCnt);
        if(g_FmapUsbBufCnt == (EXTRA_FMAP_ADV_NUM))
            dbg_msg_console("[fmap]snapshot buffer FULL ");

        g_FmapShotCnt++;
        g_FmapShotCnt = g_FmapShotCnt % EXTRA_FMAP_ADV_NUM;

        kl520_api_extra_fmap_fdfr_reset();
    }

}

void kl520_api_extra_fmap_adv_load(int cam_idx, u8 face_idx)
{

    if(face_idx >= EXTRA_FMAP_ADV_NUM)
    {
        return;
    }

    if(cam_idx == MIPI_CAM_NIR && extra_fmap_add_addr[cam_idx][face_idx] != 0){   /*NIR*/
        memcpy((void *)KDP_DDR_TEST_EXTRA_NIR_ADDR, (void *)extra_fmap_add_addr[cam_idx][face_idx], KDP_DDR_TEST_NIR_FR_SIZE); //
        //dbg_msg_console("[Snapshot]adv_load nir...cam_idx:%d, face_idx:%d",cam_idx,face_idx);
    }
    else if(cam_idx == MIPI_CAM_RGB && extra_fmap_add_addr[cam_idx][face_idx] != 0){  /*RGB*/
        memcpy((void *)KDP_DDR_TEST_EXTRA_RGB_ADDR, (void *)extra_fmap_add_addr[cam_idx][face_idx], KDP_DDR_TEST_RGB_FR_SIZE);
        //dbg_msg_console("[Snapshot]adv_load rgb...cam_idx:%d, face_idx:%d",cam_idx,face_idx);
    }
    else
    {
        dbg_msg_console("[fmap]adv_load err");
    }
}

void kl520_api_extra_fmap_init(void)
{
    extra_fmap_addr[MIPI_CAM_NIR] = kdp_ddr_reserve(KDP_DDR_TEST_NIR_FR_SIZE);
    extra_fmap_addr[MIPI_CAM_RGB] = kdp_ddr_reserve(KDP_DDR_TEST_RGB_FR_SIZE);
    kl520_api_extra_fmap_close();
}

void kl520_api_extra_fmap_close(void)
{
    extra_db_fmap_mode = FALSE;
    _g_IsEmpty_fmap = TRUE;
    _g_extra_fmap_adv_mode = EXTRA_FMAP_CLOSE_e;

    g_FmapUsbBufCnt = 0;
    g_FmapShotCnt = 0;
    g_FmapReadCnt = 0;
    #if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_AP_CTRL_TYPE < EX_FM_UART_AP_CTRL_MAIN_DB
    _g_usb_cmd_fm_status = USB_CMD_FM_NULL_e;
    _g_host_result = 0;
    #endif
    kl520_api_extra_fmap_fdfr_reset();
}

#if 0
void debug_loop_value( int size, u8* paddr, int type )
{
    u8* pu8addr = paddr;
    u16* pu16addr = (u16*)paddr;
    u32* pu32appr = (u32*)paddr;
    int i;

    for( i = 0 ; i < size; i++  )
    {
        if( type == 1 )
            dbg_msg_console( "val[%d]", i, pu8addr[i] );
        if( type == 2 )
            dbg_msg_console( "val[%d]", i, pu16addr[i] );
        if( type == 4 )
            dbg_msg_console( "val[%d]", i, pu32appr[i] );
    }
}
#endif

#if ( CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE )
/* ==================================================================
*
* db control in com bus
*
================================================================== */
u16 kl520_api_ap_com_db_enable_fm_extra_mode( eEXTRA_FMAP_TYPE mode_idx )
{
    kl520_api_extra_fmap_adv_init(MIPI_CAM_NIR, KDP_DDR_TEST_RGB_FR_SIZE);
    kl520_api_extra_fmap_adv_init(MIPI_CAM_RGB, KDP_DDR_TEST_NIR_FR_SIZE);

    if( mode_idx == EXTRA_FMAP_BYPASS_e || mode_idx == EXTRA_FMAP_FR_e )
    {
        kl520_api_extra_fmap_mode(TRUE);
        KL520_api_set_extra_fmap_adv_mode( mode_idx );
    }
    else
    {
        kl520_api_extra_fmap_close();
        return 0;
    }

    return 1;
}

u16 kl520_api_ap_com_db_enable_export_db_mode( u8 mode_idx )
{
    if( mode_idx == EXPORT_DB_MODE_DB_OUT )
    {
        kl520_api_face_set_db_add_mode( FACE_ADD_MODE_NO_DB );
    }
#if CFG_FMAP_EX_FIG_ENABLE == YES && CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_AP_CTRL_TYPE < EX_FM_UART_AP_CTRL_MAIN_DB
    else if( mode_idx == EXPORT_DB_MODE_DB_IMG_OUT )
    {
        kl520_api_face_set_db_add_mode( FACE_ADD_MODE_NO_DB );
        KL520_api_ap_com_set_extra_fmap_status( USB_CMD_FM_RX_DB_e );
    }
#endif
    else
    {
        kl520_api_face_set_db_add_mode( FACE_ADD_MODE_IN_DB );
        return 0;
    }
    kl520_api_extra_fmap_mode(FALSE);

    return 1;
}

u16 kl520_api_ap_com_db_query_db_all_mode( u8* bin_table )
{
    u16 db_valid_cnt = 0, idx;
    u8 unit;

    for( idx = 0; idx < MAX_USER; idx++ )
    {
        if( kdp_app_db_slot_is_used( idx ) )
        {
            unit = idx >> 3;
            bin_table[unit] |=  ( 0x01 << ( idx & 0xFF ) );
            db_valid_cnt++;
        }
    }
    return db_valid_cnt;
}

u16 kl520_api_ap_com_db_query_db_one_mode( u16 user_id )
{
    u16 ret = (u32)kdp_app_db_find_exist_id(user_id);
    if( ret >= MAX_USER )
    {
        return 0xFFFF;
    }
    return ret;
}

void kl520_api_ap_com_db_export_db_mode( u16 sector_idx )
{
    if( sector_idx == 0 )
    {
        while(kl520_api_extra_db_map_addr()==0) {osDelay(10);}
    }
    return;
}

void kl520_api_ap_com_db_catch_fm_mode( void )
{
    kl520_api_extra_fmap_fdfr_reset();
    while(kl520_api_extra_fmap_fdfr_catch(MIPI_CAM_RGB)==0)
    {osDelay(10);    }
    while(kl520_api_extra_fmap_addr()==0)
    {osDelay(10);}
}

u16 kl520_api_ap_com_db_import_db_mode( u16 user_idx, u16 packet_idx, u8* packet_data_in )
{
    u16 ret = IMP_FAIL_IDX_EXISTED_USER;

    if( kdp_app_db_slot_is_used( user_idx ) == FALSE )
    {
        memcpy( (void*)(0x60000000+packet_idx*4096), (void*)packet_data_in, 4096 );
        ret = IMP_CONTIUNOUS;

        if( packet_idx == 3 && *((u32*)0x60000000) == 0xdeadbeef ) // (1024*16/4096)-1 = 3
        {
            kdp_api_ap_control_set_each_db( user_idx );
            ret = IMP_SUCCESS;
        }
    }
    return ret;
}

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP ) || ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP_USR )
u16 kl520_api_ap_com_db_import_db_mode_split(u16 nUserIdx, u16 nPkgIdx, u16 nPkgSize, u16 nTotPkgNum, u16 nStdPkgSize, u8* pDataAddr)
{
    u16 ret = IMP_FAIL_IDX_EXISTED_USER;

    if( kdp_app_db_slot_is_used(nUserIdx) == FALSE )
    {
        memcpy( (void*)(KDP_DDR_MEM_START+nPkgIdx*nStdPkgSize), (void*)pDataAddr, nPkgSize );
        ret = IMP_CONTIUNOUS;

        dbg_msg_console("0x%x", *((u32*)KDP_DDR_MEM_START));
        if( nPkgIdx == nTotPkgNum && *((u32*)KDP_DDR_MEM_START) == 0xdeadbeef ) // (1024*16/4096)-1 = 3
        {
            if ( kdp_app_imp_db_chk() == TRUE )
            {
                kdp_api_ap_control_set_each_db(nUserIdx);
                ret = IMP_SUCCESS;
            }
            else
            {
                ret = IMP_FAIL_DB_ERR;
            }
        }
    }
    return ret;
}




#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF ) || ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF_USR )
BOOL g_bImpFmDataReady = FALSE;

u16 kl520_api_ap_com_import_fm_mode(u16 nPkgIdx, u16 nPkgSize, u16 nTotPkgNum, u16 nStdPkgSize, u8* pDataAddr, BOOL nAfterChk)
{
    memcpy( (void*)(KDP_DDR_MEM_START+nPkgIdx*nStdPkgSize), (void*)pDataAddr, nPkgSize );

//    if ( nPkgIdx == nTotPkgNum )
    {
        if ( kdp_app_imp_fm_chk(nAfterChk) == TRUE )
        {
            g_bImpFmDataReady = TRUE;
            return IMP_SUCCESS;
        }
        else
        {
            return IMP_FAIL_FM_ERR;
        }
    }
//    else
//    {
//        return IMP_CONTIUNOUS;
//    }
}
#endif


#endif
#endif //CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE

#endif

static void NormalizeEmbedding_scpu(float *x, int emb_size)
{
    float sum_x = 0.;

    int i = 0;
    for(i = 0; i < emb_size; i++) {
        //sum_x += powf(x[i], 2);
        sum_x += x[i] * x[i];
    }
    sum_x = sqrtf(sum_x);

    for(i = 0; i < emb_size; i++) {
        x[i] /= sum_x;
    }
}

u16 kl520_api_ap_com_import_fm_mode_split(u16 nPkgIdx, u16 nPkgSize, u16 nTotPkgNum, u16 nStdPkgSize, 
    u16 nTotPkgSize, u8* pDataAddr, BOOL nAfterChk)
{
    if (nTotPkgSize == (FM_SIZE_BYTE/4)) //fixed
    {
        memcpy( (void*)(KDP_DDR_TEST_EXTRA_NIR_ADDR+nPkgIdx*nStdPkgSize), (void*)pDataAddr, nPkgSize );

        if (( nPkgIdx == nTotPkgNum ) && (nTotPkgSize == (nPkgIdx*nStdPkgSize+nPkgSize)))
        {
            int div = 128;
            u32 scale_u32 = 0x3f8a1a17;
            float scale = *(float *)(&scale_u32);

            float *p_fr = (float *)KDP_DDR_MEM_START;
            int8_t *p_src = (int8_t *)KDP_DDR_TEST_EXTRA_NIR_ADDR;

            for (int i = 0; i < nTotPkgSize; i++)
            {
                p_fr[i] = ((float)(p_src[i]) / div) / scale;
            }

            NormalizeEmbedding_scpu(p_fr, nTotPkgSize);

            if (kdp_app_imp_fm_chk_int8((u32)(&p_fr[0])))
                return IMP_SUCCESS;
            else
                return IMP_FAIL_FM_ERR;
        }
        else
        {
            return IMP_CONTIUNOUS;
        }
    } else { //float
        memcpy( (void*)(KDP_DDR_MEM_START+nPkgIdx*nStdPkgSize), (void*)pDataAddr, nPkgSize );

        if ( nPkgIdx == nTotPkgNum )
        {
            if ( kdp_app_imp_fm_chk(nAfterChk) == TRUE )
            {
                return IMP_SUCCESS;
            }
            else
            {
                return IMP_FAIL_FM_ERR;
            }
        }
        else
        {
            return IMP_CONTIUNOUS;
        }
    }
}

void kl520_api_ap_com_import_fm_r1_inject(void)
{
    memcpy((void*)kdp_e2e_get_r1_fr(), (void*)KDP_DDR_MEM_START, KDP_DDR_TEST_RGB_FR_SIZE);
}

void kl520_api_ap_com_import_fm_n1_inject(void)
{
    memcpy((void*)kdp_e2e_get_n1_fr(), (void*)KDP_DDR_MEM_START, KDP_DDR_TEST_NIR_FR_SIZE);
}

void kl520_api_ap_com_import_fm_r1n1_inject(void)
{
    memcpy((void*)kdp_e2e_get_r1_fr(), (void*)KDP_DDR_MEM_START, KDP_DDR_TEST_RGB_FR_SIZE);
    memcpy((void*)kdp_e2e_get_n1_fr(), (void*)(KDP_DDR_MEM_START+KDP_DDR_TEST_RGB_FR_SIZE), KDP_DDR_TEST_NIR_FR_SIZE);
}

//u8 kl520_api_ap_com_import_all_db_mode_split(u16 nPkgIdx, u16 nPkgSize, u16 nTotPkgNum, u16 nStdPkgSize, u8* pDataAddr, u32 nTotalSize)
//{
//	extern uint32_t s_p_kdp_app_db;
//	u8 ret = IMP_CONTIUNOUS;

//	dbg_msg_console( "[all db split] download to db_ddr: %#X + download_offset : %#X,  with size: %x", s_p_kdp_app_db, nPkgIdx*nPkgSize, nStdPkgSize );
//	memcpy( (void*)(s_p_kdp_app_db+nPkgIdx*nStdPkgSize), (void*)pDataAddr, nPkgSize );

//	if( nPkgIdx == nTotPkgNum )
//	{
//        kdp_app_update_all_user_id();

//		dbg_msg_console("### DownloadFaceLib2Flash func coming in ###,    nTotalSize=%d", nTotalSize);
//		kdp_app_all_db_move_to_flash( nTotalSize );
//		dbg_msg_console("### DownloadFaceLib2Flash complete ###");
//		ret = IMP_SUCCESS;
//	}
//    return ret;
//}
