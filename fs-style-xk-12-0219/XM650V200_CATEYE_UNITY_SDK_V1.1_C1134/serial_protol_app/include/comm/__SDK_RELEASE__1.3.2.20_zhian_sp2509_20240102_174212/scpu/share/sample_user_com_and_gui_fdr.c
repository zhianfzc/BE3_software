#include "sample_user_com_and_gui_fdr.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF )
#include "sample_app_console.h"
#include "kl520_include.h"
#include "kl520_api_fdfr.h"

#include "board_ddr_table.h"
#include "user_ui.h"
#if ( CFG_GUI_ENABLE == YES )
#include "gui_fsm.h"
#include "sample_gui_main.h"
#include "sample_gui_fsm_events.h"
#include "sample_app_console.h"
#endif

volatile static enum USER_COM_THREAD_EVENT g_eUserComThreadEvent = USER_COM_THREAD_EVENT_READY;
static osEventFlagsId_t user_com_event_id = NULL;
static osThreadId_t tid_user_com_fdfr_thread = NULL;
volatile static u16 _user_com_data = 0;

static void _user_com_fdfr_thread(void);
static void _user_com_fdfr_thread_enable(void);
static void _user_com_fdfr_thread_disable(void);
static void _user_com_force_abort_fdfr(void);
void user_com_event_power_off(void);
void user_com_response_data(u8* p_data, u8 size);

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
#include "ota.h"
#include "board_flash_table.h"
#include "framework/event.h"
#include "kdp_memxfer.h"
#include "kdp520_gpio.h"
#include "flash.h"
#include "ota.h"
#include "kl520_com.h"
#include "kl520_api_extra_fmap.h"
#include "kl520_api_snapshot.h"
#include "user_io.h"

UINT16 	user_tx_buf_index = 0;

void user_com_response_large_partial_data(u8* p_data, u16 size, u8 data_ready );

void host_cmd_middleware_process(u8 eEventCMD)
{
#if ( CFG_GUI_ENABLE == YES )
    gui_obj_func eEventType = GUI_NULL;

    switch(eEventCMD)
    {
        case USER_COM_FLAG_REGISTRATION:
        {
            eEventType = GUI_IMG_BTN_REGISTER;
            break;
        }
        case USER_COM_FLAG_RECOGNITION:
        {
            eEventType = GUI_IMG_BTN_RECOGNIZE;
            break;
        }
        case USER_COM_FLAG_DELETE_ALL:
        {
            eEventType = GUI_IMG_BTN_DELETE;
            break;
        }
        case USER_COM_FLAG_DELETE_ONE:
        {
//            eEventType = GUI_IMG_BTN_DELETE;
            eEventType = GUI_IMG_BTN_DELETE_1;
            break;
        }
        case USER_COM_FLAG_POWER_OFF:
        {
            eEventType = GUI_POWER_OFF;
            break;
        }
    }

    if ( eEventType > GUI_NULL )
    {
        notify_user_behavior_event(eEventType, _user_com_data, GUI_FSM_SRC_USER_COM);   //GUI
    }
    else
#endif
    {
        user_com_event_start(eEventCMD);                                                //No GUI
    }
}

void user_lwcom_parser( struct st_com_type *st_com )
{
    u8* pDataStart;
    int ret = 0;

#if( COM_DEBUG_LOG_EN == YES)
    dbg_msg_user("[com_bus] normal parser API ");
#endif

    if (g_eUserComThreadEvent <= USER_COM_THREAD_EVENT_READY)
    {

    }
    else if (g_eUserComThreadEvent == USER_COM_THREAD_EVENT_OVERWRITABLE)
    {
        user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
        _user_com_force_abort_fdfr();
        _user_com_fdfr_thread_disable();
    }
    else if (g_eUserComThreadEvent == USER_COM_THREAD_EVENT_NON_OVERWRITABLE)
    {
        return;
    }

    user_tx_buf_index = 0;

    switch(st_com->cmd)
    {
        case host_cmd_power_off:
        {
#ifdef CUSTOMER_PARSER_POWER_OFF_HS_SHUTDOWN
            user_com_event_interrupt();
            osDelay(10);
            user_com_event_power_off();
#else
            _user_com_data = 0;
            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            host_cmd_middleware_process(USER_COM_FLAG_POWER_OFF);
#endif
            break;
        }

        case host_cmd_register:
        {
            pDataStart = (st_com->rx_buffer + st_com->data_start_index);
            _user_com_data = ((*pDataStart)<<8) + (*(pDataStart+1));
            user_com_thread_event_set(USER_COM_THREAD_EVENT_OVERWRITABLE);
            host_cmd_middleware_process(USER_COM_FLAG_REGISTRATION);
            break;
        }

        case host_cmd_recognize:
        {
            pDataStart = (st_com->rx_buffer + st_com->data_start_index);
            _user_com_data = ((*pDataStart)<<8) + (*(pDataStart+1));
            user_com_thread_event_set(USER_COM_THREAD_EVENT_OVERWRITABLE);
            host_cmd_middleware_process(USER_COM_FLAG_RECOGNITION);
            break;
        }

        case host_cmd_delete_all:
        {
            _user_com_data = 0;
            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            host_cmd_middleware_process(USER_COM_FLAG_DELETE_ALL);
            break;
        }

        case host_cmd_delete_one:
        {
        	_user_com_data = ( *(st_com->rx_buffer + st_com->data_start_index) );
//        	_user_com_data += kl520_api_get_start_user_id();    //user id in DDR
        	user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            host_cmd_middleware_process(USER_COM_FLAG_DELETE_ONE);
            break;
        }

        case host_cmd_enter_ota:
        {
//            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            kneron_lwcom_set_parameter( st_com, 0x1234, 0x00 );
            kneron_lwcom_packet_response_brief_w_tx_buffer( st_com, st_com->host_number, 0x7855 );
            kl520_com_response( st_com );
            ota_thread_event_set();
            osThreadTerminate(com_bus_tid);
            break;
        }

        case host_cmd_eng_cali:
        {
            _user_com_data = 3000;
            user_com_thread_event_set(USER_COM_THREAD_EVENT_OVERWRITABLE);
            host_cmd_middleware_process(USER_COM_FLAG_MP);
            break;
        }

#if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_EXTRA_ENABLE == YES
        case host_cmd_enable_export_fm_mode:
        {
            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            eEXTRA_FMAP_TYPE e_mode_idx = (eEXTRA_FMAP_TYPE) *(st_com->rx_buffer + st_com->data_start_index);
            ret = kl520_api_ap_com_db_enable_fm_extra_mode( e_mode_idx );
            user_com_response_data( (u8*)&ret, 1);
            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }

        case host_cmd_enable_export_db_mode:
        {
            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            u8 mode_idx = *(st_com->rx_buffer + st_com->data_start_index);
            ret = kl520_api_ap_com_db_enable_export_db_mode( mode_idx );
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
            user_com_response_data((u8*)&ret, 1);
            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }

        case host_cmd_query_db_all_mode:
        {
            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            const u8 table_size = ( (MAX_USER + 7) >> 3);
            u8 bin_table[table_size] = { 0 };
            u16 db_valid_cnt = kl520_api_ap_com_db_query_db_all_mode( bin_table );
            user_com_response_large_partial_data( (u8*)&db_valid_cnt, 2, FALSE );
            user_com_response_large_partial_data( bin_table, table_size, TRUE );
            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }

        case host_cmd_query_db_one_mode:
        {
            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            pDataStart = (st_com->rx_buffer + st_com->data_start_index);
            u16 user_id = ((*pDataStart)<<8) + (*(pDataStart+1));
            ret = kl520_api_ap_com_db_query_db_one_mode( user_id );
            user_com_response_data((u8*)&ret, 2);
            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }

#if CFG_FMAP_AP_CTRL_TYPE >= EX_FM_UART_AP_CTRL_MAIN_DB // uart contrl
        case host_cmd_export_db_mode:
        {
            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            pDataStart = (st_com->rx_buffer + st_com->data_start_index);
            u16 sector_idx = ((*pDataStart)<<8) + (*(pDataStart+1));
            kl520_api_ap_com_db_export_db_mode( sector_idx );
            user_com_response_large_partial_data( (u8*)&sector_idx, 2, FALSE );
            user_com_response_large_partial_data( (u8*)KDP_DDR_TEST_EXTRA_DB_ADDR + (sector_idx*4096), 4096, TRUE );
            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }

        case host_cmd_export_fm_mode:
        {
            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            kl520_api_ap_com_db_catch_fm_mode();
            user_com_response_large_partial_data( (u8*)KDP_DDR_TEST_EXTRA_RGB_ADDR, 2048, TRUE );
            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }

        case host_cmd_import_db_mode:
        {
            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            pDataStart = (st_com->rx_buffer + st_com->data_start_index);
            u16 user_idx = ((*pDataStart)<<8) + (*(pDataStart+1));
            u16 packet_idx = ( *(pDataStart+2) << 8 ) + ( *(pDataStart+3) );
            u8* packet_data_input = (st_com->rx_buffer + st_com->data_start_index + 4 );
            ret = kl520_api_ap_com_db_import_db_mode( user_idx, packet_idx, packet_data_input );
            user_com_response_data((u8*)&ret, 1);
            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }

        case host_cmd_import_fm_mode:
        {
//            dbg_msg_console("----------[%s]", "host_cmd_import_fm_mode");
            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            pDataStart = (st_com->rx_buffer + st_com->data_start_index);
            u16 packet_idx = ( *(pDataStart) << 8 ) + ( *(pDataStart+1) );
            ret = kl520_api_ap_com_import_fm_mode( packet_idx, 1024, 0, 1024, pDataStart+2, FALSE );
            user_com_response_data((u8*)&ret, 1);
            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }
#if CFG_FMAP_EX_FIG_ENABLE == YES
        case host_cmd_enable_export_fig_mode:
        {
//            dbg_msg_console("----------[%s]", "host_cmd_enable_export_fig_mode");
            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            u8 mode_idx = *(st_com->rx_buffer + st_com->data_start_index);
            kl520_api_snapshot_adv_select(0); // disable snapshot mode
            kl520_api_snapshot_adv_mode();

//            dbg_msg_console("----------[%d]", mode_idx);
            if( mode_idx == 1)
            {
                kl520_api_snapshot_adv_init(MIPI_CAM_NIR, KDP_DDR_TEST_NIR_IMG_SIZE);
                kl520_api_snapshot_adv_init(MIPI_CAM_RGB, KDP_DDR_TEST_RGB_IMG_SIZE);
                kl520_api_snapshot_adv_init(MIPI_CAM_INF, KDP_DDR_TEST_INF_IMG_SIZE);
                kl520_api_snapshot_adv_select(8); // recognize image
                kl520_api_snapshot_adv_mode();

                api_fdfr_face_recognition_set_mandatory_event();
            }

            user_com_response_data((u8*)&snapshot_adv_mode, 1);
            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }

        case host_cmd_snapshot_catch:
        {
//            dbg_msg_console("----------[%s]", "host_cmd_snapshot_catch");
            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            pDataStart = (st_com->rx_buffer + st_com->data_start_index);
            u16 src_type = ((*pDataStart)<<8) + (*(pDataStart+1));
//            dbg_msg_console("----------[0x%02X]", src_type);
            ret = kl520_api_ap_com_snapshot_catch( src_type );
            user_com_response_data((u8*)&ret, 1);
            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }

        case host_cmd_snapshot_status: // check avaiable snapshot count
        {
            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            user_com_response_data((u8*)&g_SnapUsbBufCnt, 1);
            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }

        case host_cmd_export_snapshot_data: // check avaiable snapshot count
        {
//            dbg_msg_console("----------[%s]", "host_cmd_export_snapshot_data");
            user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            pDataStart = (st_com->rx_buffer + st_com->data_start_index);
            u16 src_type = ((*pDataStart)<<8) + (*(pDataStart+1));
            u16 src_idx = ((*(pDataStart+2))<<8) + (*(pDataStart+3));
            u16 src_size = ((*(pDataStart+4))<<8) + (*(pDataStart+5));
//            dbg_msg_console("----------[0x%02X, %d, %d]", src_type, src_idx, src_size);
            ret = NO;
            //sector index feedback first
            user_com_response_large_partial_data( (u8*)&src_idx, 2, FALSE );
            //data feedback second
            if( src_type == 0x5A || src_type == 0xAA || src_type == 0x015A )
                user_com_response_large_partial_data( (u8*)KDP_DDR_TEST_RGB_IMG_ADDR + (src_idx*4096), src_size, TRUE );
            else if( src_type == 0xA5 )
                user_com_response_large_partial_data( (u8*)KDP_DDR_TEST_NIR_IMG_ADDR + (src_idx*4096), src_size, TRUE );
            else
                user_com_response_large_partial_data( (u8*)ret, 1, TRUE );
            user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);
            break;
        }
#endif // #if CFG_FMAP_EX_FIG_ENABLE == YES
#endif // #if CFG_FMAP_AP_CTRL_TYPE >= EX_FM_UART_AP_CTRL_MAIN_DB // uart contrl
#endif // #if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_EXTRA_ENABLE == YES

        case host_cmd_power_on_sys_ready:
        {
            ret = 1;
            user_com_response_data((u8*)&ret, 1);
            break;
        }

        default:
            break;
    }
}

static kl520_com_user_ops _user_com_ops = {
    .packet_analyze              = kneron_lwcom_packet_analyze,
    .packet_response_w_tx_buffer = kneron_lwcom_packet_response_w_tx_buffer,
    .parser                      = user_lwcom_parser
};

void user_com_init(void)
{
    kl520_com_reg_user_ops(&_user_com_ops);
}

//void user_com_power_on_response_ready(void)
//{
//    user_tx_buf_index = 0;
//    stCom_type.cmd = host_cmd_power_on_sys_ready;
//    user_tx_buf[user_tx_buf_index++] = 0x01;
//    kneron_lwcom_packet_response_w_tx_buffer( &stCom_type, user_tx_buf, user_tx_buf_index );
//    kl520_com_response( &stCom_type );
//}

void user_com_response_large_partial_data(u8* p_data, u16 size, u8 data_ready )
{
    if (NULL == p_data)
        return;

    if (0 < size)
    {
        memcpy( (void*)(KDP_DDR_OTA_IMAGE_BUF_START_ADDR+user_tx_buf_index), p_data, size);
    }
    user_tx_buf_index+=size;

    if( data_ready > 0 )
    {
        kneron_lwcom_packet_response_w_tx_buffer( &stCom_type, (UINT8*)KDP_DDR_OTA_IMAGE_BUF_START_ADDR, user_tx_buf_index );
        kl520_com_response( &stCom_type );
    }
}
#endif

void user_custom_comm_power_on(void)
{
//    user_tx_buf_index = 0;
//    kneron_lwcom_set_parameter(&stCom_type, 0x8989 , host_cmd_power_on);
//    kneron_lwcom_packet_response_w_tx_buffer( &stCom_type, user_tx_buf, user_tx_buf_index );
//    kl520_com_response( &stCom_type );
    dbg_msg_console("----------[%s]", "host_cmd_power_on");
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
    UINT8  nPacketResult = 0;
    kneron_lwcom_set_parameter(&stCom_type, 0x8989 , host_cmd_power_on);
    kneron_lwcom_packet_response_w_tx_buffer( &stCom_type, &nPacketResult, 0 );
    kl520_com_response( &stCom_type );
#endif
}

void user_com_event_power_off(void)
{
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
    UINT8  nPacketResult = 0;
    kneron_lwcom_packet_response_w_tx_buffer( &stCom_type, &nPacketResult, sizeof(nPacketResult) );
    kl520_com_response( &stCom_type );
    osDelay(100);
#endif
    kl520_api_poweroff();
}

void user_com_response_data(u8* p_data, u8 size)
{
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
    if (NULL == p_data)
        return;

    kneron_lwcom_packet_response_w_tx_buffer( &stCom_type, p_data, size );
    kl520_com_response( &stCom_type );
#endif
}

#if ( CFG_GUI_ENABLE == YES )
volatile static u16 _user_com_data2 = 0;
static user_behavior_data fsm_data = {0xFF, 0xFF, 0xFF};
extern void user_com_set_data(u16 type, u16 data, u8 data2)
{
    fsm_data.type = type;
    fsm_data.data = data;
    fsm_data.data2 = data2;
    _user_com_data = data;
    _user_com_data2 = data2;
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
    if (nRet == 0)
        kl520_api_dp_draw_bitmap(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, (void *)nImgAddr0);
    else
        kl520_api_dp_draw_bitmap(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, (void *)nImgAddr1);
    kl520_api_dp_fresh();

    osDelay(DISPLAY_RESULT_HOLD_TIME);
    kl520_api_dp_fresh_bg(BLACK, 2);
#endif
}
#endif

void user_com_thread_event_set(enum USER_COM_THREAD_EVENT eState)
{
    g_eUserComThreadEvent = eState;
}

static void _user_com_fdfr_thread(void)
{
    u16 ret = 0;
    u32 flags = wait_event(user_com_event_id, USER_COM_FLAG_ALL);
    clear_event(user_com_event_id, flags);

    if ( USER_COM_FLAG_REGISTRATION == (flags & USER_COM_FLAG_REGISTRATION) )
    {
        if ( _user_com_data > 0 )
        {
#if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_EXTRA_ENABLE == YES
            kdp_api_db_set_last_register_id_preprocess();
#endif

            ret = uart_sample_face_add_timeout(_user_com_data);

            if (g_eUserComThreadEvent <= USER_COM_THREAD_EVENT_READY)
            {
                // Interrupt
                kl520_api_cam_disp_state_rst();
            }
            else
            {
#if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_EXTRA_ENABLE == YES
                kdp_api_db_set_last_register_id_postprocess();
#endif

#if ( CFG_GUI_ENABLE == YES )
                notify_user_behavior_event(GUI_REGISTER_RET, ret, _user_com_data2);

#else
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

                //==================================================================================================================================
                //to-do: At all uart control module, only show wait system feedback instead of successed or failed case, and not colse display.
                //==================================================================================================================================
                kl520_api_cam_disp_close_perm_state_chk();
                kl520_api_disp_open_chk();
                kl520_api_dp_draw_two_img(USR_DDR_IMG_REGISTER_SUCCESSED_ADDR, USR_DDR_IMG_REGISTER_FAILED_ADDR, (ret&0xFF));
                kl520_api_cam_disp_state_rst();
                user_com_response_data((u8*)&ret, sizeof(ret));

#endif
            }
        }
        else
        {
            ret = 0xEDFD;
            user_com_response_data((u8*)&ret, sizeof(ret));
        }

        set_event(user_com_event_id, USER_COM_FLAG_FACE_CLOSE_DONE);
    }
    else if ( USER_COM_FLAG_RECOGNITION == (flags & USER_COM_FLAG_RECOGNITION) )
    {
        if ( _user_com_data > 0 )
        {
#if CFG_FMAP_AP_CTRL_TYPE == EX_FM_USB_AP_CTRL_ALL && CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_EXTRA_ENABLE == YES
            KL520_api_ap_com_set_extra_fmap_status( USB_CMD_FM_RECOGNIZE_e );
            ret = uart_sample_face_recognition_timeout(_user_com_data);
            kl520_api_ap_com_set_timeout_and_start( 3 );
            while( KL520_api_ap_com_get_extra_fmap_status() != USB_CMD_FM_NULL_e && kl520_api_ap_com_chk_timeout_and_stop() == FALSE ){ osDelay(10); }
            KL520_api_ap_com_set_extra_fmap_status( USB_CMD_FM_NULL_e );
#else
            ret = uart_sample_face_recognition_timeout(_user_com_data);
#endif //CFG_FMAP_AP_CTRL_TYPE == EX_FM_USB_AP_CTRL_ALL && CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_EXTRA_ENABLE == YES

            if (g_eUserComThreadEvent <= USER_COM_THREAD_EVENT_READY)
            {
                // Interrupt
                kl520_api_cam_disp_state_rst();
            }
            else
            {
#if ( CFG_GUI_ENABLE == YES )
                notify_user_behavior_event(GUI_RECOGNIZE_RET, ret, _user_com_data2);            //GUI

#else
                //==================================================================================================================================
                //to-do: At all uart control module, only show wait system feedback instead of successed or failed case, and not colse display.
                //==================================================================================================================================
                kl520_api_cam_disp_close_perm_state_chk();
                kl520_api_disp_open_chk();
                kl520_api_dp_draw_two_img(USR_DDR_IMG_RECOGNIZE_SUCCESSED_ADDR, USR_DDR_IMG_RECOGNIZE_FAILED_ADDR, (ret&0xFF));
                kl520_api_cam_disp_state_rst();
                user_com_response_data((u8*)&ret, sizeof(ret));
#endif
            }
        }
        else
        {
            ret = 0xCDBD;
            user_com_response_data((u8*)&ret, sizeof(ret));
        }

        g_bRecognitionMandatoryFlag = FALSE;

        set_event(user_com_event_id, USER_COM_FLAG_FACE_CLOSE_DONE);
    }
#if ( CFG_GUI_ENABLE == YES )
#else
    else if ( USER_COM_FLAG_DELETE_ALL == (flags & USER_COM_FLAG_DELETE_ALL) )
    {
#if CFG_FMAP_AP_CTRL_TYPE == EX_FM_USB_AP_CTRL_ALL && CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_EXTRA_ENABLE == YES
        KL520_api_ap_com_set_extra_fmap_status( USB_CMD_FM_DEL_ALL_e );
        while( KL520_api_ap_com_get_extra_fmap_status() != USB_CMD_FM_NULL_e ){ osDelay(10); }
        ret = kdp_api_ap_com_wait_host_cmd_result();
#else
        ret = uart_sample_face_del_all();
#endif //CFG_FMAP_AP_CTRL_TYPE == EX_FM_USB_AP_CTRL_ALL && CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_EXTRA_ENABLE == YES

        kl520_api_cam_disp_close_perm_state_chk();
        kl520_api_disp_open_chk();
        kl520_api_dp_draw_two_img(USR_DDR_IMG_DELETE_SUCCESSED_ADDR, USR_DDR_IMG_DELETE_FAILED_ADDR, (ret&0xFF));
        kl520_api_cam_disp_state_rst();
        user_com_response_data((u8*)&ret, 1);
    }
    else if ( USER_COM_FLAG_DELETE_ONE == (flags & USER_COM_FLAG_DELETE_ONE) )
    {
#if CFG_FMAP_AP_CTRL_TYPE == EX_FM_USB_AP_CTRL_ALL && CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_EXTRA_ENABLE == YES
        kdp_api_ap_com_set_user_id( _user_com_data );
        KL520_api_ap_com_set_extra_fmap_status( USB_CMD_FM_DEL_USER_e );
        while( KL520_api_ap_com_get_extra_fmap_status() != USB_CMD_FM_NULL_e ){ osDelay(10); }
        ret = kdp_api_ap_com_wait_host_cmd_result();
#else
        ret = uart_sample_face_del_user(_user_com_data);
#endif //CFG_FMAP_AP_CTRL_TYPE == EX_FM_USB_AP_CTRL_ALL && CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_EXTRA_ENABLE == YES


        kl520_api_cam_disp_close_perm_state_chk();
        kl520_api_disp_open_chk();
        kl520_api_dp_draw_two_img(USR_DDR_IMG_DELETE_SUCCESSED_ADDR, USR_DDR_IMG_DELETE_FAILED_ADDR, (ret&0xFF));
        kl520_api_cam_disp_state_rst();
        user_com_response_data((u8*)&ret, 1);
    }
#endif
    else if ( USER_COM_FLAG_POWER_OFF == (flags & USER_COM_FLAG_POWER_OFF) )
    {
        user_com_event_power_off();
    }
    else if ( USER_COM_FLAG_MP == (flags & USER_COM_FLAG_MP) )
    {
        if ( _user_com_data > 0 )
        {
#if ( CFG_GUI_ENABLE == YES )
            gui_app_stop();
#endif
            ret = uart_sample_face_mp_timeout(_user_com_data);

            if (g_eUserComThreadEvent <= USER_COM_THREAD_EVENT_READY)
            {
                // Interrupt
                kl520_api_cam_disp_state_rst();
            }
            else
            {
                user_com_response_data((u8*)&ret, sizeof(ret));
            }

#if ( CFG_GUI_ENABLE == YES )
            gui_app_proceed();
            user_gui_update_renderer();
#endif
        }
        else
        {
            ret = 0xEFEF;
            user_com_response_data((u8*)&ret, sizeof(ret));
        }

        set_event(user_com_event_id, USER_COM_FLAG_FACE_CLOSE_DONE);
    }

    user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);

    tid_user_com_fdfr_thread = NULL;
    delete_event(user_com_event_id);
    user_com_event_id = NULL;
    osThreadExit();
}


static void _user_com_fdfr_thread_enable(void)
{
    if (NULL == user_com_event_id)
        user_com_event_id = create_event();

    if (NULL == tid_user_com_fdfr_thread){
        osThreadAttr_t attr = {
            .stack_size = 1024
        };
        tid_user_com_fdfr_thread = osThreadNew((osThreadFunc_t)_user_com_fdfr_thread, NULL, &attr);
    }
}

static void _user_com_fdfr_thread_disable(void)
{
    osStatus_t status;

    u32 flags = wait_event(user_com_event_id, USER_COM_FLAG_FACE_CLOSE_DONE);
    clear_event(user_com_event_id, USER_COM_FLAG_FACE_CLOSE_DONE);
    osEventFlagsClear(kl520_api_get_event(), KL520_APP_FLAG_FDFR);

    if (tid_user_com_fdfr_thread) {
        status = osThreadTerminate(tid_user_com_fdfr_thread);
        tid_user_com_fdfr_thread = NULL;
    }
    if(status != osOK)
        dbg_msg_console("[%s] thread id of force abort is 0x%x, status = %d", __func__, tid_user_com_fdfr_thread, status);
    if (0 != user_com_event_id) {
        delete_event(user_com_event_id);
        user_com_event_id = NULL;
    }
}

static void _user_com_force_abort_fdfr(void)
{
//    int mode = kl520_api_fdfr_facemode_get();
    if (FACE_MODE_ADD == m_face_mode || FACE_MODE_RECOGNITION == m_face_mode)
    {
        sample_face_close();
        set_event(kl520_api_get_event(), KL520_APP_FLAG_FDFR_OK);
    }
}

void user_com_event_start(u8 event) {
    _user_com_fdfr_thread_enable();
    set_event(user_com_event_id, event);
}

void user_com_event_interrupt(void) {
    _user_com_force_abort_fdfr();
    _user_com_fdfr_thread_disable();
}
#endif
#endif

