#ifndef __USER_COMM_MSG_DEFINE_H__
#define __USER_COMM_MSG_DEFINE_H__


//--------------------PROTOCOL
#define     COM_BUS_HEAD_RX                         ((UINT32)(0xEFAA))
#define     COM_BUS_HEAD_RX_1                       ((COM_BUS_HEAD_RX&0x0000FF00)>>8 )
#define     COM_BUS_HEAD_RX_2                       ((COM_BUS_HEAD_RX&0x000000FF)>>0 )

#define     COM_BUS_HEAD_TX                         ((UINT32)(0xEFAA))
#define     COM_BUS_HEAD_TX_1                       ((COM_BUS_HEAD_TX&0x0000FF00)>>8 )
#define     COM_BUS_HEAD_TX_2                       ((COM_BUS_HEAD_TX&0x000000FF)>>0 )

#define     COM_BUS_TAIL                            (0x7887)
#define     COM_BUS_TAIL_1                          (COM_BUS_TAIL&0xFF)
#define     COM_BUS_TAIL_2                          ((COM_BUS_TAIL>>8)&0xFF)

#define     COM_BUS_GET_DATA_HEAD_TX                (0xC410C410)

//error code
#define     COM_BUS_HEAD_CHECK_ERROR                (0xEEE0)
#define     COM_BUS_TAIL_CHECK_ERROR                (0xEEE1)
#define     COM_BUS_PACKAGE_SIZE_ERROR              (0xEEE2)
#define     COM_BUS_CHECK_SUM_ERROR                 (0xEEE3)
#define     COM_BUS_DATA_SIZE_ERROR                 (0xEEE4)
#define     COM_BUS_ENCRYPTION_ERROR                (0xEEE5)
#define     COM_BUS_PACKET_OK                       (0x66)

// Host to Module                   // KID: kneron ID
	//Register and recognition
#define KID_RESET 0x10                     //v stop and clear all in-processing messages. enter standby mode
#define KID_GET_STATUS 0x11                //v to ping the module and get the status
#define KID_VERIFY 0x12                    //v to verify the person in front of the camera
#define KID_ENROLL 0x13                    //v to enroll and register the persion in front of the camera

#define KID_ENROLL_OVERWRITE 0x14

#if ( CFG_PRODUCTION_TEST == NO ) || ( IGNORE_PRODUCTION_TEST == YES )

	//Snapshot
//#define KID_SNAP_IMAGE 0x16                //v to snap a picture and save it
//#define KID_GET_SAVED_IMAGE 0x17           //v to get size of saved image
//#define KID_UPLOAD_IMAGE 0x18              //v upload images

#define KID_ENROLL_SINGLE 0x1D

	//User info
#define KID_DEL_USER 0x20                  //v Delete the specified user with user id
#define KID_DEL_ALL 0x21                   //v Delete all registerred users
#define KID_GET_USER_INFO 0x22             //v Get user info
#define KID_FACE_RESET 0x23                //v Reset face status
#define KID_GET_ALL_USER_ID 0x24           //v get all users ID

#define KID_ENROLL_ITG 0x26
#endif

	//Version info
#define KID_GET_VERSION 0x30               //v get version information
#define KID_GET_VERSION_ZA 0x6a               //zcy v get version information
#define KID_GET_VERSION_ZAPRD 0x3a               //zcy v get version information
#define KID_GET_VERSION_HARDWARE 0x3b               //zcy v get version information

#if ( CFG_PRODUCTION_TEST == NO ) || ( IGNORE_PRODUCTION_TEST == YES )
	//OTA
#define KID_START_OTA 0x40                 //v ask the module to enter OTA mode
#define KID_STOP_OTA 0x41                  //v ask the module to exit OTA mode
#define KID_GET_OTA_STATUS 0x42            //v query the current ota status
#define KID_OTA_HEADER 0x43                //v the ota header data
#define KID_OTA_PACKET 0x44                //v the data packet, carries real firmware data
#endif

	//Encryption
#define KID_INIT_ENCRYPTION 0x50           //v initialize encrypted communication
#define KID_CONFIG_BAUDRATE 0x51           //v config uart baudrate
#define KID_SET_RELEASE_ENC_KEY 0x52       //v set release encrypted key(Warning!!!:Once set, the KEY will not be able to modify)
#define KID_SET_DEBUG_ENC_KEY 0x53         //v set debug encrypted key

#if ( CFG_PRODUCTION_TEST == NO ) || ( IGNORE_PRODUCTION_TEST == YES )
	//Log
//#define KID_GET_LOG_FILE 0x60              // get log file
//#define KID_UPLOAD_LOG_FILE 0x61           // upload log file

#if ( CFG_FMAP_EXTRA_ENABLE == YES ) && ( CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE )
#define KID_SW_EXP_FM_MODE 0x70
#define KID_SW_EXP_DB_MODE 0x71
#if CFG_FMAP_AP_CTRL_TYPE >= EX_FM_UART_AP_CTRL_MAIN_DB // uart contrl
#define KID_EXP_FM_DATA 0x72
#define KID_EXP_DB_DATA 0x73
#define KID_IMP_DB_DATA 0x74

#define KID_IMP_IMG_DATA 0x76
#endif //CFG_FMAP_AP_CTRL_TYPE >= EX_FM_UART_AP_CTRL_MAIN_DB // uart contrl
#endif //( CFG_FMAP_EXTRA_ENABLE == YES ) && ( CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE )
#if ( CFG_PRODUCTION_TEST == NO ) || ( IGNORE_PRODUCTION_TEST == YES )
#define KID_IMP_FM_DATA 0x75
//#define KID_SET_EXP_MASS_DATA_HEADER 0x78  //v
#define KID_SET_IMP_MASS_DATA_HEADER 0x79
#endif

//#define KID_EXP_ALL_DB 0x7A
//#define KID_IMP_ALL_DB 0x7B
#endif

#if ( CFG_PRODUCTION_TEST == NO ) || ( IGNORE_PRODUCTION_TEST == YES )
#define KID_DB_EXPORT_REQUEST   0x7C
#define KID_UPLOAD_DATA         0x7D
#define KID_DB_IMPORT_REQUEST   0x7E
#define KID_DOWNLOAD_DATA       0x7F
#endif

//-----0xA0-0xAF Unit control
#if ( CFG_PRODUCTION_TEST == YES ) || ( IGNORE_PRODUCTION_TEST == YES )
//#define KID_TURN_ON_CAMERA              ( 0x80 ) //
//#define KID_TURN_OFF_CAMERA             ( 0x81 ) //
//#define KID_TURN_ON_VIS_LED             ( 0x82 ) //Visible
//#define KID_TURN_OFF_VIS_LED            ( 0x83 ) //
#define KID_TURN_ON_IR_LED              ( 0x84 ) //
#define KID_TURN_OFF_IR_LED             ( 0x85 ) //
//#define KID_TURN_ON_STRUCT_LED          ( 0x86 ) //
//#define KID_TURN_OFF_STRUCT_LED         ( 0x87 ) //
//#define KID_TURN_ON_PANEL               ( 0x88 )
//#define KID_TURN_OFF_PANEL              ( 0x89 )
#define KID_SET_EXP_TIME                ( 0x8A )
#define KID_SET_GAIN                    ( 0x8B )
#endif

    //MP
#define KID_SW_BOOT_PART 0xAA
#define KID_GET_CUR_PART 0xAB

#if ( CFG_PRODUCTION_TEST == NO ) || ( IGNORE_PRODUCTION_TEST == YES )
#define KID_KN_SNAP_IMAGE 0xAC
#define KID_KN_GET_SAVED_IMAGE 0xAD
#define KID_KN_UPLOAD_IMAGE 0xAE
#endif

// user cmd
#define KID_SOFT_RESET   0xCA
#define KID_KN_DEVICE_INFO  0xCB
#define KID_MP_CALIBRATION  0xCC
#define KID_USER_ROTATE_180 0xCD

#if ( CFG_PRODUCTION_TEST == YES ) || ( IGNORE_PRODUCTION_TEST == YES )
#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
#define KID_SNAPSHOT_MODE   0xCE
#endif
#endif
    
#if ((CFG_COM_BUS_TYPE&COM_BUS_USB_MASK) != COM_BUS_USB_MASK)
	//Other
#define KID_SET_THRESHOLD_LEVEL 0xD4       //v Set threshold level
#define KID_DEBUG_MODE 0xF0                //v
//#define KID_GET_DEBUG_INFO 0xF1            //v get size of debug information
//#define KID_UPLOAD_DEBUG_INFO 0xF2         //v upload debug information

#endif

#define KID_DEMO_MODE 0xFE                 //v enter demo mode, verify flow will skip feature comparation step.
#define KID_POWERDOWN 0xED                 //v be prepared to power off

#define CMD_REPLY_RESULT_MACRO(CMD_TYPE)    \
    CMD_TYPE(MR_SUCCESS, 0x00) \
    CMD_TYPE(MR_REJECTED, 0x01) \
    CMD_TYPE(MR_ABORTED, 0x02) \
    CMD_TYPE(MR_FAILED_DEV_OPEN_FAIL, 0x04) \
    CMD_TYPE(MR_FAILED_UNKNOWN_REASON, 0x05) \
    CMD_TYPE(MR_FAILED_INVALID_PARAM, 0x06) \
    CMD_TYPE(MR_FAILED_OOM, 0x07) \
    CMD_TYPE(MR_FAILED_UNKNOWN_USER, 0x08) \
    CMD_TYPE(MR_FAILED_MAX_USER, 0x09) \
    CMD_TYPE(MR_FAILED_FACE_ENROLLED, 0x0A) \
    CMD_TYPE(MR_FAILED_LIVENESS_CHECK, 0x0C) \
    CMD_TYPE(MR_FAILED_TIME_OUT, 0x0D) \
    CMD_TYPE(MR_FAILED_AUTHORIZATION, 0x0E) \
    CMD_TYPE(MR_FAILED_READ_FILE, 0x13) \
    CMD_TYPE(MR_FAILED_WRITE_FILE, 0x14) \
    CMD_TYPE(MR_FAILED_NO_ENCRYPT, 0x15) \
    CMD_TYPE(MR_FAILED_STORE_ERR, 0x16) \
    CMD_TYPE(MR_FAILED_NO_IMG, 0x17) \
    CMD_TYPE(MR_FAILED_NO_IDX, 0x18) \
    CMD_TYPE(MR_FAILED_BUF_OVERFLOW, 0x19) \
    CMD_TYPE(MR_FAILED_MASS_DATA_HEAD_EMPTY, 0x30) \
    CMD_TYPE(MR_FAILED_MASS_DATA_DB_ABNORMAL, 0x31) \
    CMD_TYPE(MR_FAILED_MASS_DATA_FM_ABNORMAL, 0x32) \
    CMD_TYPE(MR_FAILED_NOT_READY, 0x33) \
    CMD_TYPE(MR_FAILED_EXISTED_USER, 0x34) \
    CMD_TYPE(MR_FAILED_IDX_OVERFLOW, 0x35) \
    CMD_TYPE(MR_CONTIUNOUS, 0x36) \
    CMD_TYPE(MR_SUCCESS_BUT_DEL_USER, 0x40) \
    CMD_TYPE(MR_FAILED_INVALID_CMD, 0xFF) \

#define TO_ENUM(result, value) result=value,
#define TO_STR(result, value)  {case result: return #result; break;}

enum uart_reply_result {
    CMD_REPLY_RESULT_MACRO(TO_ENUM)
};

char *_str_uart_reply(enum uart_reply_result result);

#pragma diag_suppress 111
inline char *_str_uart_reply(enum uart_reply_result result)
{
    switch (result)
    {
        CMD_REPLY_RESULT_MACRO(TO_STR)
    default:
        break;
    }
    
    return "";
}
#pragma diag_warning 111


#endif    //__USER_COMM_MSG_DEFINE_H__

