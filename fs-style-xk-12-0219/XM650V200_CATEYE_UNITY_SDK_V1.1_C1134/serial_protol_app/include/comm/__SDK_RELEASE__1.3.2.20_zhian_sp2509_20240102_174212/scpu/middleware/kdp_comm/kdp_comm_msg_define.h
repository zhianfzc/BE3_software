#ifndef __KDP_COMM_MSG_DEFINE_H__
#define __KDP_COMM_MSG_DEFINE_H__



//--------------------PROTOCOL
//lmm-edit  on deShiMan protocol
#define     COM_BUS_HEAD_RX                         ((UINT32)(0x9FDC))
#define     COM_BUS_HEAD_RX_1                       ((COM_BUS_HEAD_RX&0x0000FF00)>>8 )
#define     COM_BUS_HEAD_RX_2                       ((COM_BUS_HEAD_RX&0x000000FF)>>0 )

#define     COM_BUS_HEAD_TX                         ((UINT32)(0x9FDC))
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
//-----0x00-0x0F System
#define KID_DEVICE_INFO                 ( 0x00 ) //v
#define KID_GET_VERSION                 ( 0x01 ) //v get version information

#if ( CFG_PRODUCTION_TEST == NO ) || ( IGNORE_PRODUCTION_TEST == YES )
#define KID_DEMO_MODE                   ( 0x06 ) //v enter demo mode ) verify flow will skip feature comparation step.
#define KID_DEBUG_MODE                  ( 0x07 ) //v
#define KID_GET_DEBUG_INFO              ( 0x08 ) //v get size of debug information
#define KID_UPLOAD_DEBUG_INFO           ( 0x09 ) //v upload debug information
#define KID_GET_LOG_FILE                ( 0x0A ) // get log file
#define KID_UPLOAD_LOG_FILE             ( 0x0B ) // upload log file
#endif //( CFG_PRODUCTION_TEST == NO ) || ( IGNORE_PRODUCTION_TEST == YES )

#define KID_RESET                       ( 0x0E ) //v stop and clear all in-processing messages. enter standby mode
#define KID_POWERDOWN                   ( 0x0F ) //v be prepared to power off

//-----0x10-0x3F Function
#define KID_GET_STATUS                  ( 0x10 ) //v to ping the module and get the status
#define KID_VERIFY                      ( 0x11 ) //v to verify the person in front of the camera
#define KID_ENROLL                      ( 0x12 ) //v to enroll and register the persion in front of the camera
#define KID_FACE_RESET                  ( 0x13 ) //v Reset face status
#define KID_ENROLL_SINGLE               ( 0x14 )

#define KID_DEL_USER                    ( 0x1A ) //v Delete the specified user with user id
#define KID_DEL_ALL                     ( 0x1B ) //v Delete all registerred users
#if ( CFG_PRODUCTION_TEST == NO ) || ( IGNORE_PRODUCTION_TEST == YES )
#define KID_GET_USER_INFO               ( 0x1C ) //v Get user info
#define KID_GET_ALL_USER_ID             ( 0x1D ) //v get all users ID

#if ( CFG_SNAPSHOT_ENABLE == YES ) && (IMAGE_SIZE > IMAGE_16MB)
#define KID_SNAP_IMAGE                  ( 0x20 ) //v to snap a picture and save it
#define KID_GET_SAVED_IMAGE             ( 0x21 ) //v to get size of saved image
#define KID_UPLOAD_IMAGE                ( 0x22 ) //v upload images
#endif //( CFG_SNAPSHOT_ENABLE == YES )
#endif //( CFG_PRODUCTION_TEST == NO ) || ( IGNORE_PRODUCTION_TEST == YES )

#if ( CFG_FMAP_EXTRA_ENABLE == YES ) && ( CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE )
#define KID_SW_EXP_FM_MODE              ( 0x26 ) //v
#define KID_SW_EXP_DB_MODE              ( 0x27 ) //v
#if CFG_FMAP_AP_CTRL_TYPE >= EX_FM_UART_AP_CTRL_MAIN_DB // uart contrl
#define KID_EXP_FM_DATA                 ( 0x28 ) //v
#define KID_EXP_DB_DATA                 ( 0x29 ) //v
#define KID_IMP_DB_DATA                 ( 0x2A ) //v
#define KID_IMP_FM_DATA                 ( 0x2B ) //v
#define KID_IMP_IMG_DATA                ( 0x2C )
#if ( CFG_SNAPSHOT_ENABLE == YES )
#if ( CFG_FMAP_EX_FIG_ENABLE == YES )
#define KID_CATCH_IMG_MODE              ( 0x2D )
#define KID_EXP_IMG_DATA                ( 0x2E )
#endif //( CFG_FMAP_EX_FIG_ENABLE == YES )
#endif //( CFG_SNAPSHOT_ENABLE == YES )
#endif //CFG_FMAP_AP_CTRL_TYPE >= EX_FM_UART_AP_CTRL_MAIN_DB // uart contrl
#endif //( CFG_FMAP_EXTRA_ENABLE == YES ) && ( CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE )

//-----0x40-0x6F Unit control
#if ( CFG_PRODUCTION_TEST == YES ) || ( IGNORE_PRODUCTION_TEST == YES )
#define KID_TURN_ON_CAMERA              ( 0x40 ) //
#define KID_TURN_OFF_CAMERA             ( 0x41 ) //
#define KID_TURN_ON_VIS_LED             ( 0x42 ) //Visible
#define KID_TURN_OFF_VIS_LED            ( 0x43 ) //
#define KID_TURN_ON_IR_LED              ( 0x44 ) //
#define KID_TURN_OFF_IR_LED             ( 0x45 ) //
#define KID_TURN_ON_STRUCT_LED          ( 0x46 ) //
#define KID_TURN_OFF_STRUCT_LED         ( 0x47 ) //
#define KID_TURN_ON_PANEL               ( 0x48 )
#define KID_TURN_OFF_PANEL              ( 0x49 )
#endif

#define KID_CONFIG_BAUDRATE             ( 0x50 ) //v config uart baudrate

#if ( CFG_PRODUCTION_TEST == NO ) || ( IGNORE_PRODUCTION_TEST == YES )
#define KID_SET_EXP_MASS_DATA_HEADER    ( 0x51 ) //v
#define KID_SET_IMP_MASS_DATA_HEADER    ( 0x52 )
#define KID_GET_MASS_DATA_STATUS        ( 0x53 )
#endif

#define KID_SET_THRESHOLD_LEVEL         ( 0x54 ) //v Set threshold level


//-----0x70-0x7F MP
#define KID_SYS_INIT_READY_TIME         ( 0x70 )
#define KID_SW_BOOT_PART                ( 0x71 )
#define KID_GET_CUR_PART                ( 0x72 )
#if ( KDP_REMOTE_CTRL_BY_TYPE == YES )
#if ( CFG_PRODUCTION_TEST == NO ) || ( IGNORE_PRODUCTION_TEST == YES )
#define KID_EXP_ALL_DB                  ( 0x73 ) // Upload DB form DDR
#define KID_IMP_ALL_DB                  ( 0x74 ) // Download DB to flash
#define KID_EXP_FLASH                   ( 0x75 ) //
#endif
#endif
#define KID_MP_CALIBRATION              ( 0x76 )


//-----0x80-0xEF unused


//-----0xF0-0xF7 Key
#define KID_INIT_ENCRYPTION             ( 0xF0 ) //v initialize encrypted communication
#define KID_SET_RELEASE_ENC_KEY         ( 0xF1 ) //v set release encrypted key(Warning!!!:Once set ) the KEY will not be able to modify)
#define KID_SET_DEBUG_ENC_KEY           ( 0xF2 ) //v set debug encrypted key
#define KID_SET_INTER_ACTIVATE          ( 0xF3 ) //v

//-----0xF8-0xFF OTA
#if ( CFG_PRODUCTION_TEST == NO ) || ( IGNORE_PRODUCTION_TEST == YES )
#define KID_START_OTA                   ( 0xF8 ) //v ask the module to enter OTA mode
#define KID_STOP_OTA                    ( 0xF9 ) //v ask the module to exit OTA mode
#define KID_GET_OTA_STATUS              ( 0xFA ) //v query the current ota status
#define KID_OTA_HEADER                  ( 0xFB ) //v the ota header data
#define KID_OTA_PACKET                  ( 0xFC ) //v the data packet ) carries real firmware data
#endif


#define CMD_REPLY_RESULT_MACRO(CMD_TYPE)    \
    CMD_TYPE(MR_SUCCESS, 0x00) \
    CMD_TYPE(MR_REJECTED, 0x01) \
    CMD_TYPE(MR_ABORTED, 0x02) \
    CMD_TYPE(MR_CONTIUNOUS, 0x03) \
    CMD_TYPE(MR_FAILED_INVALID_CMD, 0x04) \
    CMD_TYPE(MR_FAILED_INVALID_PARAM, 0x05) \
    CMD_TYPE(MR_FAILED_TIME_OUT, 0x06) \
    CMD_TYPE(MR_FAILED_OOM, 0x07) \
    CMD_TYPE(MR_FAILED_NOT_READY, 0x08) \
    CMD_TYPE(MR_SUCCESS_BUT_DEL_USER, 0x09) \
    CMD_TYPE(MR_FAILED_UNKNOWN_REASON, 0x0F) \
    CMD_TYPE(MR_FAILED_UNKNOWN_USER, 0x10) \
    CMD_TYPE(MR_FAILED_EXISTED_USER, 0x11) \
    CMD_TYPE(MR_FAILED_MAX_USER, 0x12) \
    CMD_TYPE(MR_FAILED_FACE_ENROLLED, 0x13) \
    CMD_TYPE(MR_FAILED_LIVENESS_CHECK, 0x14) \
    CMD_TYPE(MR_FAILED_IDX_OVERFLOW, 0x20) \
    CMD_TYPE(MR_FAILED_BUF_OVERFLOW, 0x21) \
    CMD_TYPE(MR_FAILED_NO_IMG, 0x22) \
    CMD_TYPE(MR_FAILED_NO_IDX, 0x23) \
    CMD_TYPE(MR_FAILED_MASS_DATA_HEAD_EMPTY, 0x30) \
    CMD_TYPE(MR_FAILED_MASS_DATA_DB_ABNORMAL, 0x31) \
    CMD_TYPE(MR_FAILED_MASS_DATA_FM_ABNORMAL, 0x32) \
    CMD_TYPE(MR_FAILED_DEV_OPEN_FAIL, 0x40) \
    CMD_TYPE(MR_FAILED_STORE_ERR, 0x48) \
    CMD_TYPE(MR_FAILED_AUTHORIZATION, 0x50) \
    CMD_TYPE(MR_FAILED_NO_ENCRYPT, 0x51) \

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

#endif    //__KDP_COMM_MSG_DEFINE_H__

