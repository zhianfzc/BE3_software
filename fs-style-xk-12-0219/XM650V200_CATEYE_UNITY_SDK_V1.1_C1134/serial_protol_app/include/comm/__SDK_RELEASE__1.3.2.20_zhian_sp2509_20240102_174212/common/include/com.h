/**
 * @file      com.h
 * @brief     Basic communcation structure
 * @copyright (c) 2018 Kneron Inc. All right reserved.
 */
#ifndef __COM_H__
#define __COM_H__

#include <stdint.h>

#if defined(__arm__)
#pragma anon_unions
#endif

#define CUSTOMER_SETTING_REMOVE_CMD

typedef enum {
    CMD_NONE = 0,
    CMD_MEM_READ,
    CMD_MEM_WRITE,
    CMD_DATA,
    CMD_ACK_NACK,
    CMD_STS_CLR,
    CMD_MEM_CLR,
    CMD_CRC_ERR,
    CMD_TEST_ECHO,
    CMD_FILE_WRITE,
    CMD_FLASH_MEM_WRITE,  // single sector flash write

    CMD_RESET = 0x20,
    CMD_SYSTEM_STATUS,
    CMD_OTA_UPDATE,
    CMD_UPDATE_MODEL,

    CMD_SNAPSHOT,       //0x24
    CMD_SNAPSHOT_CHECK, //0x25
    CMD_RECEIVE_IMAGE, //0x26
    CMD_SIM_START, //0x27
    CMD_EXTRA_MAP, //0x28
    CMD_EXTRA_MAP_CHECK, //0x29
    CMD_E2E_SET_MODE,
    CMD_E2E_FACE_ADD,
    CMD_E2E_FACE_RECOGNITION,
    CMD_E2E_FACE_RECOGNITION_TEST,
    CMD_E2E_FACE_LIVENESS,
    CMD_E2E_FACE_PRE_ADD,


    CMD_QUERY_APPS = 0x30,
    CMD_SELECT_APP,
    CMD_SET_MODE,
    CMD_SET_EVENTS,
    CMD_UPDATE,
    CMD_IMG_RESULT,  // only RESP message is used.  No CMD version is implemented
    CMD_ABORT,

	CMD_E2E_FACE_COMAPRE_1VS1,
	CMD_OPEN_FDR_THREAD,
	CMD_CLOSE_FDR_THREAD,

    CMD_SFID_START = 0x108,
    CMD_SFID_NEW_USER,
    CMD_SFID_ADD_DB,
    CMD_SFID_DELETE_DB,
    CMD_SEND_IMAGE,
    CMD_SFID_LW3D_START,
    CMD_SEND_LW3D,
    CMD_SFID_EDIT_DB,

    CMD_DME_START = 0x118,
    CMD_DME_CONFIG,
    CMD_DME_SEND_IMAGE,

    // Flash command
    CMD_FLASH_INFO = 0x1000,
    CMD_FLASH_CHIP_ERASE,
    CMD_FLASH_SECTOR_ERASE,
    CMD_FLASH_READ,
    CMD_FLASH_WRITE,

    // debug utility command
    CMD_DBG_USB_MEM_WR = 0x2000,
    CMD_DBG_SET_MODEL_TYPE = 0x2001,

    // for Camera tool
    CMD_DOWNLOAD_IMAGE_NIR = 0x2100,
    CMD_DOWNLOAD_IMAGE_RGB,
    CMD_SET_NIR_AGC,
    CMD_SET_NIR_AEC,
    CMD_SET_RGB_AGC,
    CMD_SET_RGB_AEC,
    CMD_SET_NIR_LED,
    CMD_SET_RGB_LED,
    CMD_CAM_CONNECT,
    CMD_NIR_FD_RES,
    CMD_NIR_LM_RES,
    CMD_RGB_FD_RES,
    CMD_RGB_LM_RES,
    CMD_NIR_GET_CONF,
    CMD_RGB_GET_CONF,


    //Jeff add
    CMD_SCPU_UPDATE = 0x3000,
    CMD_NCPU_UPDATE,
    CMD_MODEL_UPDATE,

    CMD_GUI_CTRL_SERIES = 0x4000,
    CMD_GUI_REGISTER,
    CMD_GUI_RECOGNIZE,
    CMD_GUI_DELETE_ALL,

    CMD_CLOUD_UPDATE_DB = 0x5000,
    CMD_FDFR_THREAD_CLOSE = 0x6000,
    CMD_EXPORT_STREAM_IMG = 0x6100,
    CMD_CAPTRUE_SENSOR = 0x6101,

} Cmd;

typedef struct {
    uint16_t preamble;
    uint16_t ctrl; /* payload_len & ctrl info */
    uint16_t cmd;
    uint16_t msg_len;
} __attribute__((packed)) MsgHdr;

typedef struct {
    uint32_t param1;
    uint32_t param2;
    uint8_t data[];
} __attribute__((packed)) CmdPram;

typedef struct {
    union {
        uint32_t error;
        uint32_t param1;
    } __attribute__((packed));
    uint32_t param2;
    uint8_t data[];
} __attribute__((packed)) RspPram;

typedef struct {
    uint32_t op_parm1;
    uint32_t op_parm2;
    uint8_t  data[];
} __attribute__((packed)) OpPram;


#define NO_ERROR        0
#define PARAM_ERR       1
#define RSP_NOT_IMPLEMENTED 0xFFFE
#define RSP_UKNOWN_CMD  0xFFFF
#define BAD_CONFIRMATION 2  // CMD_RESET RESPONSE
#define BAD_MODE         1
#define FILE_ERROR       1  // File data transfer error

#define MSG_HDR_CMD     0xA583
#define MSG_HDR_RSP     0x8A35
#define MSG_HDR_VAL     0xA553  // this is used by the pre-packet code
#define MSG_HDR_SIZE    16  // includes both MsgHdr and CmdPram addr & len
#define PKT_CRC_FLAG    0x4000

#if (HAPS_ID == 2)
#define MSG_DATA_BUF_MAX    0x890
#else
#define MSG_DATA_BUF_MAX    0x2400  // used for testing (worst case msg payload is 4096+72)
#endif

#define UPDATE_MODULE_NONE 0
#define UPDATE_MODULE_SCPU 0x07
#define UPDATE_MODULE_NCPU 0x08

#endif
