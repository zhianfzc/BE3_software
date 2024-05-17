#ifndef __KDP_COMM_PROTOCO_H__
#define __KDP_COMM_PROTOCO_H__

#include "board_kl520.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP )
#include "kl520_com.h"
#include "kdp_comm_and_gui_fdr.h"
#include "kdp_comm_aes.h"
#include "kdp_comm.h"
#include "common.h"
#include "version.h"

#include "drivers.h"
#include "kdp_uart.h"
#include "kl520_com.h"
#include <stdint.h>
#include <stdarg.h>


#define  DEVICE_ID_NUM                      (20)
//#define BYTE_ALIGN   __attribute__ ((packed))
//#pragma pack(1)
#define KEY_SIZE  							(16)  //must be 16bytes
#define USER_NAME_SIZE   					(32)
#define VERSION_INFO_BUFFER_SIZE 			(32)
#define MAX_USER_COUNTS  					(MAX_USER)

#define KL_BYTE_ORDER                       (1) //little endian
#define NET_BYTE_ORDER                      (0) //big endian

#pragma pack(1)

/* communication message ID definitions*/
// Module to Host (m->h)
#define MSG_HEAD                    ((COM_BUS_HEAD_TX_2<<8)|(COM_BUS_HEAD_TX_1))//(0xAAEF)
//#define MID_REPLY                   (0x00)     // request(command) reply message, success with data or fail with reason
//#define MID_NOTE                    (0x01)    // note to host e.g. the position or angle of the face
//#define MID_IMAGE                   (0X02)    // send image to host
// Host to Module (h->m)
//0x10-0x2F
//#define MID_RESET                   (0x10)     // stop and clear all in-processing messages. enter standby mode
//#define MID_GETSTATUS               (0x11) // to ping the module and get the status
//#define MID_VERIFY                  (0x12)    // to verify the person in front of the camera
//#define MID_ENROLL                  (0x13)    // to enroll and register the persion in front of the camera
//#define MID_SNAPIMAGE               (0x16) // to snap a picture and save it
//#define MID_GETSAVEDIMAGE           (0x17) // to get size of saved image
//#define MID_UPLOADIMAGE             (0x18)   // upload images
//#define MID_DELUSER                 (0x20)   // Delete the specified user with user id
//#define MID_DELALL                  (0x21)   // Delete all registerred users
//#define MID_GETUSERINFO             (0x22)   // Get user info
//#define MID_FACERESET               (0x23)     // Reset face status
//#define MID_GET_ALL_USERID          (0x24)  // get all users ID
//
////0x30-0x4F
//#define MID_GET_VERSION             (0x30)   // get version information
//#define MID_WRITE_SN                (0x31)      // write sn to board
//#define MID_START_OTA               (0x40)     // ask the module to enter OTA mode
//#define MID_STOP_OTA                (0x41)      // ask the module to exit OTA mode
//#define MID_GET_OTA_STATUS          (0x42)// query the current ota status
//#define MID_OTA_HEADER              (0x43)    // the ota header data
//#define MID_OTA_PACKET              (0x44)    // the data packet, carries real firmware data
//
////0x50-0x6F
//#define MID_INIT_ENCRYPTION         (0x50) // initialize encrypted communication
//#define MID_CONFIG_BAUDRATE         (0x51) // config uart baudrate
//#define MID_SET_RELEASE_ENC_KEY     (0x52) // set release encrypted key(Warning!!!:Once set, the KEY will not be able to modify)
//#define MID_SET_DEBUG_ENC_KEY       (0x53) // set debug encrypted key
//#define MID_SET_INTERACTIVATE       (0x54)
//#define MID_GET_LOGFILE             (0x60) // get log file
//#define MID_UPLOAD_LOGFILE          (0x61) // upload log file
//
////0x70-0x8F
//#define MID_START_LIVE_DETECT       (0x80) // start liveness_detect ZF-demo protoco.
//
////production testing protoco.
//#define DEVICE_INFO                 (0x70)
//#define FIRST_FRAME_TIME            (0x71)
//#define TURN_ON_STRUCT_LED          (0x72)
//#define TURN_OFF_STRUCT_LED         (0x73)
//#define TURN_ON_LED                 (0x74)
//#define TURN_OFF_LED                (0x75)
//#define START_SNAP_PICS             (0x76)
//#define STOP_SNAP_PICS              (0x77)
////production testing protoco end.
//
////0x90-0xAF
//#define MID_SW_EXP_FM_MODE          (0xA0)
//#define MID_SW_EXP_DB_MODE          (0xA1)
//#define MID_EXP_FM_DATA             (0xA4)
//#define MID_EXP_DB_DATA             (0xA5)
//#define MID_IMP_DB_DATA             (0xA8)
//#define MID_DEL_ONE_USR             (0xAE)
//#define MID_DEL_ALL_USR             (0xAF)
//
////0xB0-0xCF
//#define MID_FACTORY_TEST            (0xC0)  // factory test
//#define MID_DDR_TEST                (0xC1)     // DDR test
//
////0xD0-0xEF
//#define MID_SET_THRESHOLD_LEVEL     (0xD4) // Set threshold level
//#define MID_POWERDOWN               (0xED) // be prepared to power off
//#define MID_DEBUG_MODE              (0xF0)
//#define MID_GET_DEBUG_INFO          (0xF1)    // get size of debug information
//#define MID_UPLOAD_DEBUG_INFO       (0xF2)   // upload debug information
//#define MID_DEMOMODE                (0xFE)  // enter demo mode, verify flow will skip feature comparation step.
//#define MID_MAX                     (0xFF)       // reserved
#define KERNEL_LOG 0
#define APP_LOG 	 1


/* communication message ID definitions end */

/* message result code */
//#define MR_SUCCESS     0     // success
//#define MR_REJECTED    1     // module rejected this command
//#define MR_ABORTED     2     // algo aborted
//#define v MR_FAILED4_CAMERA  4 // camera open failed
//#define v MR_FAILED4_UNKNOWNREASON  5 // UNKNOWN_ERROR
//#define v MR_FAILED4_INVALIDPARAM  6  // invalid param
//#define v MR_FAILED4_NOMEMORY  7      // no enough memory
//#define MR_FAILED4_UNKNOWNUSER  8   // no user enrolled
//#define x MR_FAILED4_MAXUSER  9       // exceed maximum user number
//#define v MR_FAILED4_FACEENROLLED  10 // this face has been enrolled
//#define v MR_FAILED4_LIVENESSCHECK  12// liveness check failed
//#define v MR_FAILED4_TIMEOUT  13      // exceed the time limit
//#define x MR_FAILED4_AUTHORIZATION  14// authorization failed
//#define x MR_FAILED4_CAMERAFOV  15    // camera fov test failed
//#define x MR_FAILED4_CAMERAQUA  16    // camera quality test failed
//#define x MR_FAILED4_CAMERASTRU  17   // camera structure test failed
//#define x MR_FAILED4_BOOT_TIMEOUT  18 // boot up timeout
//#define x MR_FAILED4_READ_FILE   19    // read file failed
//#define x MR_FAILED4_WRITE_FILE  20   // write file failed
//#define x MR_FAILED4_NO_ENCRYPT  21   // encrypt must be set


//Pelase discuss
//#define v MR_FAILED4_STOREERR  22      // Flash store fail
//#define v MR_FAILED4_NOIMG  23      // Image not in flash
//#define v MR_FAILED4_NOIMGIDX  24      // Image not in flash
//#define v MR_FAILED4_IMGOVERFLOW  25      // Image not in flash


/* message result code end */
//#define ONE_USER_INFO  0
//#define ALL_USER_INFO  1
/* module state */
//typedef uint8_t s_mstate;

//#define MS_STANDBY  0; // IDLE, waiting for commands
//#define MS_BUSY  1;    // in working of processing commands
//#define MS_ERROR  2;   // in error state. can't work properly
//#define MS_INVALID  3; // not initialized
//#define MS_OTA  4;     // in ota state

/* module state end */

// a general message adapter
//typedef struct {
//    uint8_t kid;       // the message id
//    uint8_t size_heb;  // high eight bits
//    uint8_t size_leb;  // low eight bits
//    uint8_t data[0];
//} s_msg;

/* message data definitions */
/* module -> host note */
/* msg note id */
//typedef uint8_t s_note_id;

//#define NID_READY  0;        // module ready for handling request (command)
//#define NID_FACE_STATE  1;   // the detected face status description
//#define NID_UNKNOWN_ERROR 2; // unknown error
//#define NID_OTA_DONE  3;     // OTA upgrading processing done

/* msg note id end */


#define FACE_STATE_NORMAL      					(0)  // normal state, the face is available
#define FACE_STATE_NOFACE      					(1)  // no face detected
#define FACE_STATE_TOOUP       					(2)  // face is too up side
#define FACE_STATE_TOODOWN     					(3)  // face is too down side
#define FACE_STATE_TOOLEFT     					(4)  // face is too left side
#define FACE_STATE_TOORIGHT    					(5)  // face is too right side
#define FACE_STATE_TOOFAR      					(6)  // face is too far
#define FACE_STATE_TOOCLOSE    					(7)  // face is too near
#define FACE_STATE_EYEBROW_OCCLUSION  			(8)  // eyebrow occlusion
#define FACE_STATE_EYE_OCCLUSION      			(9)  // eye occlusion
#define FACE_STATE_FACE_OCCLUSION     			(10) // face occlusion
#define FACE_STATE_DIRECTION_ERROR    			(11) // face direction error
#define FACE_STATE_EYE_CLOSE_STATUS_OPEN_EYE  	(12) // eye close time out
#define FACE_STATE_EYE_CLOSE_STATUS           	(13) // confirm eye close status
#define FACE_STATE_EYE_CLOSE_UNKNOW_STATUS    	(14) // eye close unknow status

/* logfile type */

#define LOG_FILE_KERNEL  					(0) // kernel log
#define LOG_FILE_SENSELOCK_APP  			(1) // senselock app log
#define LOG_FILE_FACE_MODULE  				(2) // face module log

#define MSG_HEAD_SIZE                       (2) //Head
#define MSG_CMD_SIZE                        (1) //CMD
#define MSG_LEN_SIZE                        (2) //Len
#define MSG_CHK_SIZE                        (1)
#define MSG_HEAD_BIG_SIZE                   (MSG_HEAD_SIZE+MSG_CMD_SIZE)
#define MSG_CMD_BIG_SIZE                    (MSG_CMD_SIZE+MSG_LEN_SIZE)
#define MSG_ALL_SIZE                        (MSG_HEAD_SIZE+MSG_CMD_SIZE+MSG_LEN_SIZE+MSG_CHK_SIZE)

#define MSG_APPEND_1_SIZE                   (1) //NID
#define MSG_APPEND_2_SIZE                   (2) //MID + Result
#define MSG_APPEND_3_SIZE                   (3) //MID + Result + Status

#define MSG_AES_HEAD_SIZE                   (2) //Head
#define MSG_AES_DATA_SIZE                   (2) //Size
#define MSG_AES_CHK_SIZE                    (1)
#define MSG_AES_HEAD_BIG_SIZE               (MSG_AES_HEAD_SIZE+MSG_AES_DATA_SIZE)
#define MSG_AES_HEAD_TAIL_SIZE              (MSG_AES_HEAD_BIG_SIZE+MSG_AES_CHK_SIZE)

#define ST_FACE_MODULE_STATUS_UNLOCK_OK                 (200)
#define ST_FACE_MODULE_STATUS_UNLOCK_WITH_EYES_CLOSE    (204)

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// Common Structure
typedef struct {
    uint16_t head;
    uint8_t cmd;
    uint16_t size;
} msg_base;

typedef struct {
    msg_base msg_head;
    uint8_t nid;
    //uint8_t data;
    uint8_t check;
} s_msg_note_ready;

typedef struct {
    msg_base msg_head;
    uint8_t nid;
    uint8_t OkOrFail;
    uint8_t check;
} s_msg_note_mass_data;

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    //uint8_t  data; N/A no data
    uint8_t  check;
} s_msg_reply_no_data;

typedef struct {
    uint8_t nPkgIdx[2];
    uint8_t nPkgSize[2];
    uint8_t* pDataHeader;
} msg_mass_data_pkg;                                    // KID_EXP_FM_DATA/KID_EXP_DB_DATA/KID_IMP_DB_DATA

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    uint8_t  check;
} s_msg_reply_mass_data;

/////////////////////////////////////////Import message
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x00-0x0F System
typedef struct {
    uint32_t sn;
    uint32_t boot_version;
    uint8_t cpu_version[4];
    uint8_t npu_version[4];
    uint32_t fd_version;
    uint32_t fr_version;
    uint32_t live_version;
    uint32_t eye_mode_version;
    uint32_t mask_version;
    uint16_t cmos_id_0;
    uint16_t cmos_id_1;
    uint16_t flash_id;
    uint16_t led_id;
#if ( UART_PROTOCOL_VERSION >= 0x0200 )
    uint16_t display_id;
    uint16_t touch_id;
    uint16_t protocol_version;
#else
    uint16_t reserve;
#endif
} device_info_data;

typedef struct {
    uint32_t    model_type;     // defined in model_type.h
    uint32_t    model_version;  // Model version
}model_info;

typedef struct {
    uint32_t sn;
    uint32_t boot_version;
    uint8_t cpu_version[4];
    uint8_t npu_version[4];
    uint16_t sensor_id_0;
    uint16_t sensor_id_1;
    uint16_t flash_id;
    uint16_t led_id;
    uint16_t display_id;
    uint16_t touch_id;
    model_info models[15];
} kn_device_info_data;

// upload logfile
typedef struct {
    uint8_t upload_logfile_offset[4]; // upload logfile offset, int -> [o1 o2 o3 o4]
    uint8_t upload_logfile_size[4];   // uploade logfile size, int -> [s1 s2 s3 s4]
} msg_upload_logfile_data;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x10-0x3F Function
// enroll user
typedef struct {
    uint8_t admin; // the user will be set to admin
    uint8_t user_name[USER_NAME_SIZE];
    uint8_t face_direction;
    uint8_t enroll_type;    // 0: 5 faces; 1: 1 face;
    uint8_t enable_duplicate;
    uint8_t timeout; // timeout, unit second default 10s
    uint8_t cmd_id;
} msg_enroll_data;                                      // KID_ENROLL

typedef struct {
    uint8_t user_id_heb;
    uint8_t user_id_leb;
    uint8_t user_name[USER_NAME_SIZE];
    uint8_t admin;
} msg_get_user_info_data;                               // KID_GET_USER_INFO

typedef struct {
    uint8_t user_counts;      // number of enrolled users
    uint8_t users_id[MAX_USER_COUNTS*2];   //use 2 bytes to save a user ID and save high eight bits firstly
} msg_get_all_user_id_data;                             // KID_GET_ALL_USER_ID


typedef struct {
    uint8_t nImgCnt;
    uint8_t nStrNum;
} msg_snap_img_data;;

typedef struct {
    uint8_t eType;
} msg_sw_exp_mode_data;

typedef struct {
    uint8_t nUserIdx[2];
} msg_query_db_one_mode_data;

typedef struct {
    uint8_t nUserCnt[2];
    uint8_t nTable[( (MAX_USER + 7) >> 3)];
} msg_query_db_all_mode_data;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x40-0x6F Unit control
typedef struct {
    uint8_t fsize_b[4];  // OTA FW file size int -> [b1, b2, b3, b4]
    uint8_t num_pkt[4];  // number packet to be divided for transferring, int -> [b1, b2, b3, b4]
    uint8_t pkt_size[2]; // raw data size of single packet
    uint8_t md5_sum[32]; // md5 check sum
    uint8_t mode_type;
    uint8_t ext_data[2];
} msg_mass_data_header;

typedef struct
{
//    msg_mass_data_header tHeaderInfo;
    u16 nStdPkgSize;
    u32 nTotPkgNum;
    u32 nTotPkgSize;
    u32 nPkgCnt;
    mass_data_type eType;
    u16 nExtData;

    u32 nBuffAddr;
    u16 nModNum;
    u8  nReadyType;
//    BOOL bMassRx;
    BOOL bFinal;
}imp_exp_mass_data_info_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x70-0x7F MP

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0xF0-0xF7 Key
// MID_INIT_ENCRYPTION data
typedef struct {
    uint8_t seed[4]; // random data as a seed
    uint8_t mode;    // reserved for selecting encrytion mode
    uint8_t crttime[4];
} msg_init_encryption_data;                             // KID_INIT_ENCRYPTION

typedef struct {
    uint8_t enc_key_number[16];
} msg_enc_key_number_data;                              // KID_SET_RELEASE_ENC_KEY

typedef struct {
    uint8_t range_left_min;
    uint8_t range_left_max;
    uint8_t range_right_min;
    uint8_t range_right_max;
    uint8_t range_up_min;
    uint8_t range_up_max;
    uint8_t range_down_min;
    uint8_t range_down_max;
} msg_interactivate_param;                            // KID_SET_INTER_ACTIVATE

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0xF8-0xFF OTA
// ota packet MID_OTA_PACKET
//typedef struct {
//    uint8_t  pid[2];   // the packet id
//    uint8_t  psize[2]; // the size of this package
//    uint8_t* data;  // data 0 start
//} s_msg_otapacket_data;                             // KID_OTA_PACKET













////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////Export message

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
} s_msg_rsp_base;

typedef struct {
    msg_base msg_head;
    uint8_t  result;
    uint8_t  status[2];
    uint8_t  check;
} s_msg_reply_communication_abnormal;

//-----0x00-0x0F System
typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    device_info_data device_info;
    uint8_t  check;
} s_msg_reply_device_info_data;                         // KID_DEVICE_INFO

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    kn_device_info_data device_info;
    uint8_t  check;
} s_msg_reply_kn_device_info_data;                      // KID_KN_DEVICE_INFO

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    uint8_t version_infos[VERSION_INFO_BUFFER_SIZE];
    uint8_t check;
} s_msg_reply_version_data;                             // KID_GET_VERSION

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    uint8_t  debug_file_size[4]; // the debug file size
    uint8_t  check;
} s_msg_reply_get_debug_info_data;                      // KID_GET_DEBUG_INFO

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    uint8_t  upload_debug_info_offset[4]; // the debug file size
    uint8_t  upload_debug_info_size[4];
    uint8_t  check;
} s_msg_reply_upload_debug_info_data;                   // KID_UPLOAD_DEBUG_INFO

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    uint8_t  log_size[4]; // the log size
    uint8_t  check;
} s_msg_reply_get_log_data;                             // KID_GET_LOG_FILE

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    uint8_t* log_data; // the log size
    uint8_t  check;
} s_msg_reply_upload_log_data;                          // KID_UPLOAD_LOG_FILE

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x10-0x3F Function
typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    uint8_t status;
    uint8_t check;
} s_msg_reply_getstatus_data;                           // KID_GET_STATUS

/* message reply data definitions */
typedef struct {
    uint8_t user_id_heb;
    uint8_t user_id_leb;
    uint8_t user_name[USER_NAME_SIZE];
    uint8_t admin;
    uint8_t unlockStatus;
} msg_verify_data;                              // KID_VERIFY

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    msg_verify_data reply_data;
    uint8_t check;
} s_msg_reply_verify_data;                              // KID_VERIFY

typedef struct {
    uint8_t user_id_heb;
    uint8_t user_id_leb;
    uint8_t face_direction;
} msg_reply_enroll_data;                              // KID_ENROLL

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    msg_reply_enroll_data reply_data;
    uint8_t check;
} s_msg_reply_enroll_data;                              // KID_ENROLL

typedef struct {
    int16_t state; // corresponding to FACE_STATE_*
    // position
    int16_t left;  // in pixel
    int16_t top;
    int16_t right;
    int16_t bottom;
    // pose
    int16_t yaw;   // up and down in vertical orientation
    int16_t pitch; // right or left turned in horizontal orientation
    int16_t roll;  // slope
} s_msg_note_data_face;

typedef struct {
    msg_base msg_head;
    uint8_t nid;
    s_msg_note_data_face msg_data;
    uint8_t check;
} s_msg_note_data_EAR_face;                             // KID_ENROLL //Enroll And Register

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    uint8_t user_id_heb;
    uint8_t user_id_leb;
    uint8_t check;
} s_msg_reply_delete_one_data;                          // KID_DEL_USER

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    msg_get_user_info_data user_info;
    uint8_t check;
} s_msg_reply_get_user_info;                            // KID_GET_USER_INFO

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    msg_get_all_user_id_data AllUserInfo;
    uint8_t check;
} s_msg_reply_get_all_user_info;                        // KID_GET_ALL_USER_ID

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    uint8_t  image_size[4];   // image size int -> [s1, s2, s3, s4]
    uint8_t  check;
} s_msg_reply_get_saved_image_data;                     // KID_GET_SAVED_IMAGE

// upload image
typedef struct {
    msg_base msg_head;
//    uint8_t  kid;
//    uint8_t  result;
//    uint8_t* upload_image_data; // upload image offset, int -> [o1 o2 o3 o4]
    uint8_t  check;
} s_msg_reply_upload_image_data;                        // KID_UPLOAD_IMAGE

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    msg_query_db_one_mode_data tUserInfo;
    uint8_t  check;
} s_msg_reply_query_db_one_mode_data;

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    msg_query_db_all_mode_data tUserInfo;
    uint8_t  check;
} s_msg_reply_query_db_all_mode_data;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x40-0x6F Unit control

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x70-0x7F MP
typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    uint8_t  time[4];
    uint8_t  check;
} s_msg_reply_fisrt_frame_time;

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    uint8_t  current_part;
    uint8_t  check;
} s_msg_reply_switch_part;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0xF0-0xF7 Key
typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    uint8_t  device_id[DEVICE_ID_NUM]; // the unique ID of this device, string type
    uint8_t  check;
} s_msg_reply_init_encryption_data;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0xF8-0xFF OTA
// ota header MID_OTA_HEADER
// REPLY MID_GET_OTA_STATUS
typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    uint8_t  ota_status;        // current ota status
    uint8_t  next_pid_e[2];  // expected next pid, [b0,b1]
    uint8_t  check;
} s_msg_reply_get_ota_status_data;                      // KID_GET_OTA_STATUS

#ifdef KID_DB_EXPORT_REQUEST

typedef struct {
    uint8_t user_id_heb;
    uint8_t user_id_leb;
    uint8_t total_size[4];
    uint8_t md5[32];
} msg_db_export_request;

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    msg_db_export_request data;
    uint8_t check;
} s_msg_reply_db_export_request;
#endif

#ifdef KID_UPLOAD_DATA
typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    uint8_t  check;
} s_msg_reply_upload_data; 
#endif

#ifdef KID_DB_IMPORT_REQUEST
typedef struct {
    uint8_t user_id_heb;
    uint8_t user_id_leb;
} msg_reply_db_import_request;

typedef struct {
    msg_base msg_head;
    uint8_t  kid;
    uint8_t  result;
    msg_reply_db_import_request reply_data;
    uint8_t check;
} s_msg_reply_db_import_request;
#endif

//typedef struct {
//    msg_base msg_head;
//    uint8_t kid;
//    uint8_t result;
//    uint8_t data;
//    uint8_t check;
//} Msg_Delete_User;

//typedef struct {
//    msg_base msg_head;
//    uint8_t kid;
//    uint8_t result;
//    uint8_t data;
//    uint8_t check;
//} msg_reply_power_down;
/* module -> host note end */

//typedef struct {
//    uint8_t pd_rightaway; // power down right away after verifying
//    uint8_t timeout; // timeout, unit second, default 10s
//} s_msg_verify_data;

// delete user
//typedef struct {
//    uint8_t user_id_heb; // high eight bits of user_id to be deleted
//    uint8_t user_id_leb; // low eight bits pf user_id to be deleted
//} s_msg_deluser_data;

// get user info
//typedef struct {
//    uint8_t user_id_heb; // high eight bits of user_id to get info
//    uint8_t user_id_leb; // low eight bits of user_id to get info
//} s_msg_getuserinfo_data;

// MID_INIT_ENCRYPTION reply
//typedef struct {
//    uint8_t seed[4]; // random data as a seed
//    uint8_t mode;    // reserved for selecting encrytion mode
//    uint8_t crttime[4]; // current time - this is the interface that module sync time with host
//    uint8_t enable_network;// enable network
//    uint8_t waiting_time;  // waiting time after process media business
//    uint8_t reserved[5]; // reservie some field
//} s_msg_init_data;


//// start ota data MID_START_OTA
//typedef struct {
//    uint8_t v_primary;   // primary version number
//    uint8_t v_secondary; // secondary version number
//    uint8_t v_revision;  // revision number
//} s_msg_startota_data;

// demo data
//typedef struct {
//    uint8_t enable; // enable demo or not
//} s_msg_demomode_data;

// snap image data
//typedef struct {
//    uint8_t image_counts; // number of stored images
//    uint8_t start_number; // start number of stored images
//} s_msg_snap_image_data;

// get saved image data
//typedef struct {
//    uint8_t image_number; // number of saved image
//} s_msg_get_saved_image_data;

// DDR test
//typedef struct {
//    uint8_t ddr_test_counts; // number of DDR testing
//} s_msg_ddr_test_data;

//typedef struct {
//    uint8_t time_heb;      // high eight bits of factory test time
//    uint8_t time_leb;      // low eight bits of facetory test time
//} s_msg_reply_factory_test_data;

//typedef struct {
//    uint8_t  ota_result;//0 :is sucess; 1: is faile;
//} s_note_ota_result;

//typedef struct {
//    uint8_t baudrate_index;//1: is 115200 (115200*1); 2 is 230400 (115200*2); 3 is 460800 (115200*4); 4 is 1500000
//} s_msg_config_baudrate;

typedef struct {
    uint16_t head;   //
    uint16_t body_size;
    uint8_t* body_data;
    uint8_t  checknum;
} Msg_AesEncryptData;

typedef struct {
    uint8_t cam_idx;
    uint8_t exposure_time[4];
} s_msg_exp_time;

#if ( ENCRYPTION_MODE&AES_ENCRYPTION ) || ( ENCRYPTION_MODE&XOR_ENCRYPTION )
extern uint8_t debug_key[KEY_SIZE];

void PkgDecrypt(uint8_t *bytes, int length, const uint8_t *key, int stride, uint8_t *out);
#endif

u16 kl_ntohs(u16 val);
u32 kl_ntohl(u32 val);
u16 kl_ntohps(u8* p);
u32 kl_ntohpl(u8* p);
u16 kl_htons(u16 val);
u32 kl_htonl(u32 val);

u8 checksum_cal(u8* p_data, u16 IdxS, u16 IdxE);
extern u16 StreamsToBigEndU16(u8* pData);
extern u32 StreamsToBigEndU32(u8* pData);
uint16_t ShortType_BigToSmallEnd(uint16_t value);
uint32_t IntType_BigToSmallEnd(uint32_t value);
void send_reply_NoDataMsg(uint8_t result,uint8_t mid);                      // MR_SUCCESS; mid:reply type msg
void send_system_ready_note_msg(void);
void send_communication_abnormal_reply_msg(uint8_t result, uint16_t status);
void send_heartbeat_msg(void);

#if ( UART_PROTOCOL_VERSION >= 0x0200 )
extern u16 kdp_comm_get_protocol_version(void);
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x00-0x0F System
void send_GetDeviceInfo_reply_msg(uint8_t result,device_info_data device_info);
void send_Get_Kn_DeviceInfo_reply_msg(uint8_t result,kn_device_info_data *device_info);
void response_get_version_info_msg(uint8_t result, uint8_t Version[32]); //result:MR_SUCCESS
void response_get_version_info_msg_zhian(uint8_t result, uint8_t Version[32]); //zcy
void response_get_version_info_msg_zhian_prd(uint8_t result, uint8_t Version[32]); //zcy
void response_get_version_info_msg_zhian_hard(uint8_t result, uint8_t Version[32]); //zcy
void send_DebugModeOrDemoMode_reply_msg(uint8_t result, uint8_t kid);       // MR_SUCCESS; MID_OTA_HEADER:MID_OTA_PACKET
void send_GetDebugInfo_reply_msg(uint8_t result, uint8_t debug_file_size[4]);
void send_UploadDebugInfo_reply_msg(uint8_t result, uint8_t upload_debug_info_offset[4], uint8_t upload_debug_info_size[4]);
void send_GetLogFile_reply_msg(uint8_t result, uint8_t log_size[4]);
void send_UploadLogFile_reply_msg(uint8_t result, uint8_t* logdata, uint16_t log_size); //log_size up to 4Kbytes
void send_Reset_reply_msg(uint8_t result);                                              // MR_SUCCESS;
void send_soft_reset_reply_msg(uint8_t result);
void send_power_off_reply_msg(void);                                                        //result:MR_SUCCESS
void send_data_error_reply_msg(uint8_t result);

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x10-0x3F Function
void send_status_reply_msg(uint8_t result,uint8_t status);              // MR_SUCCESS;
void send_verify_reply_msg(uint8_t result,msg_verify_data userinfo);
void send_enroll_reply_msg(uint8_t result, uint8_t user_id_heb, uint8_t user_id_leb,uint8_t face_direction, uint8_t cmd_id);
void send_EnrollOrVerify_note_msg(s_msg_note_data_face face_info,uint8_t nid);//nid alg result  NID_FACE_STATE(1):success  NID_UNKNOWNERROR(2):error;verify-NID_EYE_STATE(4)
void send_FaceReset_reply_msg(uint8_t result);                                      // MR_SUCCESS;
void response_delete_one_msg(uint8_t result, uint16_t user_id);
void response_delete_msg(uint8_t result, uint8_t OneOrAll);//
void response_get_user_info_msg(uint8_t result, msg_get_user_info_data userinfo);  //result:MR_SUCCESS
void response_get_Alluser_info_msg(uint8_t result, msg_get_all_user_id_data P_allUserInfo);  //result:MR_SUCCESS

void send_snapImage_reply_msg(uint8_t result);                                      // MR_SUCCESS;
void send_savedImage_reply_msg(uint8_t result,uint8_t size[4]);     // MR_SUCCESS; size:number of saved image
void send_uploadImage_reply_msg(uint8_t result,uint8_t* upload_image_data,uint16_t upload_image_size);

#if ( CFG_FMAP_EXTRA_ENABLE == YES ) && ( CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE )
extern void send_switch_exp_fm_mode_reply_msg(uint8_t result);
extern void send_switch_exp_db_mode_reply_msg(uint8_t result);
extern void send_catch_image_mode_reply_msg(uint8_t result);
//extern void send_query_db_one_mode_reply_msg(uint16_t nRet);
//extern void send_query_db_all_mode_reply_msg(msg_query_db_all_mode_data tInfo);
#endif
extern void send_exp_mass_data_reply_msg(uint8_t result, u8 eCMD, u32 nAddr, u16 nUploadSize);
extern void send_exp_mass_data_done_note_msg(uint8_t nResult);
void send_MpCalibration_reply_msg(uint8_t result);
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x40-0x6F Unit control
void send_Response_result_reply_msg(uint8_t result, uint8_t kid);
//void send_TurnOnCamera_reply_msg(uint8_t result);
//void send_TurnOffCamera_reply_msg(uint8_t result);
//void send_TurnOnStructLed_reply_msg(uint8_t result);
//void send_TurnOffStructLed_reply_msg(uint8_t result);
//void send_TurnOnLed_reply_msg(uint8_t result);
//void send_TurnOffLed_reply_msg(uint8_t result);
void send_SysInitReadyTime_reply_msg(uint8_t result, uint8_t time[4]);
void send_SetMassDataHeader_reply_msg(uint8_t result, uint8_t kid);
void send_AlgThreshold_level_reply_msg(uint8_t result);
void send_ConfigBaurate_reply_msg(uint8_t result);                              // MR_SUCCESS; MID_OTA_HEADER:MID_OTA_PACKET

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x70-0x7F MP
void send_SwitchPart_reply_msg(uint8_t result, uint8_t nCurPart);
void send_GetCurPart_reply_msg(uint8_t result, uint8_t nCurPart);
void send_StartSnapPics_reply_msg(uint8_t result);
void send_StopSnapPics_reply_msg(uint8_t result);

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0xF0-0xF7 Key
void send_InitEncryption_reply_msg(uint8_t result, uint8_t device_id[DEVICE_ID_NUM]);
void send_SetReleaseOrDebugEncKey_reply_msg(uint8_t result,uint8_t ReleaseOrDebugEncKey);   // MR_SUCCESS; MID_OTA_HEADER:MID_OTA_PACKET
void send_SetInteractivate_reply_msg(uint8_t result,uint8_t mid);// MR_SUCCESS; MID_OTA_HEADER:MID_OTA_PACKET
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0xF8-0xFF OTA
void send_StartOrStopOta_reply_msg(uint8_t result,uint8_t StartOrStopOTA);
void send_OtaStatus_reply_msg(uint8_t result,uint8_t ota_status,uint8_t next_pid_e[2]);     // MR_SUCCESS;
void send_OtaHeaderOrPacket_reply_msg(uint8_t result,uint8_t HeaderOrPacket);                       // MR_SUCCESS; MID_OTA_HEADER:MID_OTA_PACKET
void send_OtaDone_Note_msg(uint8_t OtaResult);  //0:OTA sucess,1:OTA fail

#ifdef KID_DB_EXPORT_REQUEST
void send_db_export_reply_msg(uint8_t result, u16 user_id, u32 total_size, u8 *md5);
void update_db_export_info(u32 addr, u32 size);
u32 get_db_export_addr(void);
u32 get_db_export_size(void);
#endif

#ifdef KID_UPLOAD_DATA
void send_upload_data_reply_msg(u8 result, u32 addr, u32 size);
#endif

#ifdef KID_DB_IMPORT_REQUEST
void send_db_import_request_reply_msg(uint8_t result, uint16_t user_id);
void update_db_import_info(u32 addr, u32 size, u16 user_id);
u32 get_db_import_addr(void);
u32 get_db_import_size(void);
u32 get_db_import_user_id(void);
#endif

#pragma pack()

#endif
#endif
#endif    //__KDP_COMM_PROTOCO_H__
