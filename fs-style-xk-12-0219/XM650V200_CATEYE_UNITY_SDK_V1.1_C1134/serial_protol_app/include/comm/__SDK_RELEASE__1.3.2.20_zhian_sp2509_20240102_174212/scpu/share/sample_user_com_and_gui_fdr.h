#ifndef __SAMPLE_USER_COM_H__
#define __SAMPLE_USER_COM_H__
#include "board_kl520.h"
#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF )
#if 1//( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )

#define COM_DEBUG_LOG_EN        (YES)

#define USER_COM_FLAG_REGISTRATION          (1<<0)
#define USER_COM_FLAG_RECOGNITION           (1<<1)
#define USER_COM_FLAG_DELETE_ALL            (1<<2)
#define USER_COM_FLAG_DELETE_ONE            (1<<3)
#define USER_COM_FLAG_POWER_OFF             (1<<4)
#define USER_COM_FLAG_FACE_CLOSE_DONE       (1<<5)
#define USER_COM_FLAG_MP                    (1<<6)
#define USER_COM_FLAG_ALL                   (USER_COM_FLAG_REGISTRATION|\
                                             USER_COM_FLAG_RECOGNITION|\
                                             USER_COM_FLAG_DELETE_ALL|\
                                             USER_COM_FLAG_DELETE_ONE|\
                                             USER_COM_FLAG_POWER_OFF|\
                                             USER_COM_FLAG_MP)

enum USER_COM_THREAD_EVENT
{
    USER_COM_THREAD_EVENT_READY             = 0,
    USER_COM_THREAD_EVENT_OVERWRITABLE      = 1,
    USER_COM_THREAD_EVENT_NON_OVERWRITABLE  = 2,
};

enum user_om
{

	host_cmd_power_on =   0x10,
	host_cmd_power_off =  0x11,
	host_cmd_register =   0x12,
	host_cmd_recognize =  0x13,
	host_cmd_delete_all = 0x14,
	host_cmd_delete_one = 0x15,
	host_cmd_enter_ota =  0x16,

	host_cmd_enable_export_fm_mode = 0x17,
	host_cmd_enable_export_db_mode = 0x18,
	host_cmd_query_db_all_mode = 0x19,
	host_cmd_query_db_one_mode = 0x1A,
	host_cmd_export_db_mode = 0x1B,
	host_cmd_export_fm_mode = 0x1C,
	host_cmd_import_db_mode = 0x1D,
    host_cmd_import_fm_mode = 0x1E,

    //snapshot
    host_cmd_enable_export_fig_mode = 0xC0,
    host_cmd_snapshot_catch = 0xC1,
    host_cmd_snapshot_status = 0xC2,
    host_cmd_export_snapshot_data = 0xC3,

    host_cmd_camera_display_ct = 0x0100,

    //Status inquiry
    host_cmd_power_on_sys_ready =  0x1001,

    //Engineering Mode
    host_cmd_eng_cali = 0xEC01,
};

extern void user_custom_comm_power_on(void);
extern void user_com_response_data(u8* p_data, u8 size);
extern void user_com_init(void);
extern void user_com_response_large_partial_data(u8* p_data, u16 size, u8 data_ready );
extern void user_com_thread_event_set(enum USER_COM_THREAD_EVENT eState);

#if ( CFG_GUI_ENABLE == YES )
extern void user_com_set_data(u16 type, u16 data, u8 data2);
extern int user_com_get_data(void* arg);
#endif
extern void user_com_event_start(u8 event);
extern void user_com_event_interrupt(void);
extern void user_com_event_power_off(void);
#else
//#include "user_com_and_gui_fdr.h"
#endif
#endif
#endif
#endif
