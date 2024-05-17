#ifndef __KDP_COMM_AND_GUI_FDR_H__
#define __KDP_COMM_AND_GUI_FDR_H__

#include "board_kl520.h"

//zcy add for default time
#define VERIFY_DEFAULT_TIME  5
#define ENROLL_DEFAULT_TIME  9


#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP )
#include "kl520_com.h"

#if (CFG_COM_UART_MSG == COM_UART_MSG_KDP)
#include "kdp_comm_msg_define.h"
#else
#include "user_comm_msg_define.h"
#endif

#define COM_DEBUG_LOG_EN        (NO)

#define USER_COM_FLAG_REGISTRATION          (1<<0)
#define USER_COM_FLAG_RECOGNITION           (1<<1)
#define USER_COM_FLAG_DELETE_ALL            (1<<2)
#define USER_COM_FLAG_DELETE_ONE            (1<<3)
#define USER_COM_FLAG_POWER_OFF             (1<<4)
#define USER_COM_FLAG_FACE_CLOSE_DONE       (1<<5)
#define USER_COM_FLAG_SNAP_IMG              (1<<6)
#define USER_COM_FLAG_FACE_RESET            (1<<7)
#define USER_COM_FLAG_MP                    (1<<8)
#define USER_COM_FLAG_OTA_PROCESS           (1<<9)
#define USER_COM_FLAG_DOWNLIB_PROCESS       (1<<10)
#define USER_COM_FLAG_ALL                   (USER_COM_FLAG_REGISTRATION|\
                                             USER_COM_FLAG_RECOGNITION|\
                                             USER_COM_FLAG_DELETE_ALL|\
                                             USER_COM_FLAG_DELETE_ONE|\
                                             USER_COM_FLAG_POWER_OFF|\
                                             USER_COM_FLAG_SNAP_IMG|\
                                             USER_COM_FLAG_FACE_RESET|\
                                             USER_COM_FLAG_MP|\
                                             USER_COM_FLAG_OTA_PROCESS|\
                                             USER_COM_FLAG_DOWNLIB_PROCESS)

#define USER_COM_EVENT_CMD_READY            (1)
#define USER_COM_EVENT_FDR_OPENED           (2)

enum USER_COM_THREAD_EVENT
{
    USER_COM_THREAD_EVENT_READY             = 0,
    USER_COM_THREAD_EVENT_OVERWRITABLE      = 1,
    USER_COM_THREAD_EVENT_NON_OVERWRITABLE  = 2,
    USER_COM_THREAD_EVENT_OTA_FLASH         = 3,
    USER_COM_THREAD_EVENT_ENROLL            = 0x80, //overwritable starts from 0x80
    USER_COM_THREAD_EVENT_VERIFY            = 0x81,
};

enum
{
    SYS_IDLE = 0,
    SYS_BUSY = 1,
    SYS_ERROR = 2,
    SYS_INVALID = 3,
};

// Mass data type
typedef enum
{
	MASS_DATA_NULL					= 0x00,

	//OTA
    MASS_DATA_OTA_MASK              = 0x10,
	MASS_DATA_OTA					= 0x10,

	//EXPORT
    MASS_DATA_EXP_MASK              = 0x20,
	MASS_DATA_EXP_DB				= 0x21,
	MASS_DATA_EXP_FM				= 0x22,
    MASS_DATA_EXP_IMG               = 0x23, //for recognition
	MASS_DATA_EXP_ALL_DB			= 0x24,
    MASS_DATA_EXP_FW                = 0x2A,

	//IMPORT
    MASS_DATA_IMP_MASK              = 0x30,
	MASS_DATA_IMP_DB				= 0x31,
	MASS_DATA_IMP_FM				= 0x32,
    MASS_DATA_IMP_IMG               = 0x33, //for register
    MASS_DATA_IMP_ALL_DB            = 0x34,
    MASS_DATA_IMP_FW                = 0x3A,

	MASS_DATA_MODE_MASK             = 0xF0,
} mass_data_type;

enum data_ready_type
{
    DATA_READY_TYPE_NULL            = 0x00,
    DATA_READY_TYPE_EXP_DB          = 0x01,
    DATA_READY_TYPE_EXP_FM          = 0x02,
    DATA_READY_TYPE_EXP_IMG         = 0x03,
	DATA_READY_TYPE_EXP_ALL_DB      = 0x04,
    DATA_READY_TYPE_EXP_FLASH       = 0x05,
    DATA_READY_TYPE_IMP_DB          = 0x10,
    DATA_READY_TYPE_IMP_FM          = 0x20,
    DATA_READY_TYPE_IMP_IMG         = 0x30,
	DATA_READY_TYPE_IMP_ALL_DB      = 0x40,
};

// Module to Host
enum kdp_com_m2h
{
    KID_REPLY                       = 0x00, // request(command) reply message, success with data or fail with reason
    KID_NOTE                        = 0x01, // note to host e.g. the position or angle of the face
    KID_IMAGE                       = 0x02, // send image to host
    KID_ERROR                       = 0x03
};

enum kdp_com_m2h_type
{
    NID_READY                       = 0x00, // module ready for handling request (command)
    NID_FACE_STATE                  = 0x01, // the detected face status description
    NID_UNKNOWN_ERROR               = 0x02, // unknown error
    NID_OTA_DONE                    = 0x03, // OTA upgrading processing done
    NID_MASS_DATA_DONE              = 0x04, // Mass data transmission done
    NID_CATEYE_RUNNING              = 0xF0,
};

enum kdp_face_dir
{
    KDP_FACE_DIRECTION_UNDEFINE     = 0x00,
    KDP_FACE_DIRECTION_MIDDLE       = 0x01,
    KDP_FACE_DIRECTION_RIGHT        = 0x02,
    KDP_FACE_DIRECTION_LEFT         = 0x04,
    KDP_FACE_DIRECTION_DOWN         = 0x08,
    KDP_FACE_DIRECTION_UP           = 0x10,

    KDP_FACE_DIRECTION_LR           = KDP_FACE_DIRECTION_LEFT|KDP_FACE_DIRECTION_RIGHT,
    KDP_FACE_DIRECTION_UD           = KDP_FACE_DIRECTION_UP|KDP_FACE_DIRECTION_DOWN,
    KDP_FACE_DIRECTION_UDLR         = KDP_FACE_DIRECTION_UD|KDP_FACE_DIRECTION_LR,
    KDP_FACE_DIRECTION_MASK         = 0x1F,
};

enum kdp_chk_face_dir
{
    KDP_CHK_FACE_DIR_NORMAL         = 0x00,
    KDP_CHK_FACE_DIR_ERROR          = 0x01,
    KDP_CHK_FACE_DIR_EXISTED        = 0x02,
};

extern u8 user_com_GetOtaStatus(void);
extern void user_com_response_data(u8* p_data, u16 size);
extern void user_com_init(void);
extern void user_com_thread_event_set(enum USER_COM_THREAD_EVENT eState);
extern enum USER_COM_THREAD_EVENT user_com_thread_event_get(void);

#if ( CFG_GUI_ENABLE == YES )
extern void user_com_set_data(u16 type, u16 data, u8 data2);
extern int user_com_get_data(void* arg);
#endif
extern void user_com_event_start(u32 event);
extern void user_com_event_interrupt(void);
extern void user_com_event_power_off(void);

void init_user_com_thread(void);

#endif
#endif
#endif    //__KDP_COMM_AND_GUI_FDR_H__

