#ifndef __KDP_COMM_APP_H__
#define __KDP_COMM_APP_H__

#include "board_kl520.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP )
#include "kl520_com.h"
#include <stdlib.h>
#include "kl520_include.h"
#include "kdp_comm_protoco.h"
#include "kl520_api_extra_fmap.h"

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK )
#define IGNORE_PRODUCTION_TEST              (NO)

#if defined(CFG_KL520_VERSION) && (CFG_KL520_VERSION > KL520A)
#define ENABLE_AUTO_POWER_OFF_MODE          (NO)

#else

#if (CFG_PRODUCTION_TEST == YES) //lmm-edit
#define ENABLE_AUTO_POWER_OFF_MODE          (NO)
#elif (ENCRYPTION_MODE > 0)
#define ENABLE_AUTO_POWER_OFF_MODE          (YES)
#else
#define ENABLE_AUTO_POWER_OFF_MODE          (NO)   //(YES)
#endif

#endif

#define POWER_OFF_DELAY_TIME_CNT            (1500)
#define AUTO_POWER_OFF_TIME_CNT             (1000)
#define SYS_INTERNAL_TIME_INTERVAL          (100)  //100ms
#define SYS_INTERNAL_TIME_NOTE_INTERVAL     (100)  //100ms

enum kdp_imp_mode
{
    KDP_IMP_FM_RGB                      = 0,
    KDP_IMP_FM_RGB_TO_NIR               = 1,
    KDP_IMP_FM_RGB_NIR                  = 2,
    KDP_IMP_FM_NIR_TO_NIR               = 6,

//    KDP_IMP_IMG_RGB                     = 3,
//    KDP_IMP_IMG_NIR                     = 4,
//    KDP_IMP_IMG_INF                     = 5,
};

enum kdp_catch_img_type
{
    KDP_CATCH_CLOSE                     = 0,
    KDP_CATCH_RGB_IMG                   = 0x005A,
    KDP_CATCH_NIR_IMG                   = 0x00A5,
    KDP_CATCH_BOTH_CAM                  = 0x00AA,
};

#if ( UART_PROTOCOL_VERSION >= 0x201)
enum kdp_ota_mode
{
    //bit[7]:SCPU, bit[3]:UI, bit[2]:NCPU, bit[1]:INFO, bit[0]:MODEL
    KDP_OTA_VERSION                     = 0x00,
    KDP_OTA_SCPU                        = 0x80,
    KDP_OTA_SCPU_NCPU                   = 0x87, //scpu + ncpu + model + info
    KDP_OTA_SCPU_UI                     = 0x88, //scpu + ui
};
#endif

typedef struct
{
    msg_mass_data_header header_info;
    u32 ddr_addr_idx;
    u32 total_update_size;
    u16 total_pid;
    u16 packet_id;
    u8 md5_crc[32];
    u8 ui_update;
    u8 ncpu_update;     //ncpu update flag;
    u8 scpu_update;     //scpu update flag;
    u8 model_update;    //model update flag;
    u8 ota_process;
    u8 ota_status;
    u32 ota_ddr_start_addr;
    u8 ota_mode; //mode 0: safe mode.  mode 1: ext mode
}ota_update_info_t;

enum kdp_ota_type
{
    KDP_OTA_TYPE_SCPU                   = 1,
    KDP_OTA_TYPE_NCPU                   = 2,
    KDP_OTA_TYPE_MODEL                  = 3,
};

#define KDP_OTA_MAX_PART                3

struct __attribute__((packed)) ota_part_header
{
    u8 type;
    u32 length;
    u8 version[4];
    u32 addr;
};

struct __attribute__((packed)) ota_header_info
{
    u8 sync_word[2];
    u32 total_size;
    u8 md5[32];
    struct ota_part_header part_bin[KDP_OTA_MAX_PART];
};

extern ota_update_info_t ota_update_info;
extern imp_exp_mass_data_info_t g_tImpExpMassDataPkgInfo;


extern uint32_t g_nSysReadyTime;
extern uint32_t g_nAutoPowerOffCnt;
extern uint32_t g_nSysInternalTime;

extern u16 g_nFaceId;
extern u8 g_eFaceDirState;

extern BOOL g_bStopSendMsg;
extern BOOL g_bUserDemoMode;
extern BOOL g_bAutoPowerOff;
extern BOOL g_bKID_SetKey;
extern u8 g_nCheckSum_Error;
extern kl520_face_add_mode g_eEnrollAddFaceTypeMode;

extern u8 g_nEncryptionMode;
#endif

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x00-0x0F System
extern uint8_t DSM_Uart_GetDeviceInfo(device_info_data* device_info); //lmm-add
extern uint8_t DSM_Uart_Get_Version_Info(uint8_t Version[32]);                                          //lmm-add
extern uint8_t DSM_Uart_Get_Version_Info_zhian(uint8_t Version[32]);  //zcy
extern uint8_t DSM_Uart_Get_Version_Info_zhian_prd(uint8_t Version[32]);  //zcy
extern uint8_t DSM_Uart_Get_Version_Info_zhian_hard(uint8_t Version[32]);  //zcy
extern uint8_t DSM_Uart_StartDebugMode(uint8_t StartDebugMode);//lmm-add
extern uint8_t DSM_Uart_GET_DebugInfo(uint8_t debug_file_size[4]);//lmm-add
extern uint8_t DSM_Uart_UploadDebugInfo(uint8_t upload_debug_info_offset[4], uint8_t upload_debug_info_size[4]);//lmm-add
extern uint8_t DSM_Uart_GetLogFile(uint8_t log_type, uint8_t log_size[4]);//lmm-add
extern uint8_t DSM_Uart_UploadLogFile(uint8_t* logdata, msg_upload_logfile_data logData);//lmm-add
extern uint8_t DSM_Uart_SystemReset(void);//lmm-add
//extern uint8_t DSM_Uart_Get_Status_Info(uint8_t* ota_status, uint8_t next_pid_e[2]);//lmm-add

uint8_t DSM_Uart_Get_Kn_DeviceInfo(kn_device_info_data* device_info);

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x10-0x3F Function
extern uint8_t KDP_Enroll_Add_Face_Type_Set(kl520_face_add_mode eType);
extern uint8_t DSM_Uart_GetState(uint8_t fdfr_state);           //tien-add
extern uint8_t DSM_Uart_FaceReset(void); //lmm-add
extern uint8_t DSM_Uart_GetUserInfo(uint16_t userID,msg_get_user_info_data* userinfo); //lmm-add
extern uint8_t DSM_Uart_GetAllUserInfo(msg_get_all_user_id_data* allUserInfo);   //lmm-add

extern uint8_t DSM_Uart_SnapImage(msg_snap_img_data tInfo);
extern uint8_t DSM_Uart_SavedImage(uint8_t image_num, uint8_t* image_size);                 //lmm-add
extern uint8_t DSM_Uart_UploadImage(uint32_t upload_image_offset, uint8_t** upload_image_data, uint32_t* image_size);//lmm-add

#if CFG_FMAP_EXTRA_ENABLE == YES
extern uint8_t KDP_export_fm_mode( eEXTRA_FMAP_TYPE mode_idx );
extern uint8_t KDP_export_db_mode( eEXPORT_DB_TYPE mode_idx );
#endif
extern void KDP_imp_fm_inejct_data(void);
extern void KDP_exp_mass_data(msg_mass_data_pkg tInfo, u8 eCMD);
extern void KDP_imp_mass_data(msg_mass_data_pkg* tInfo, u8 eCMD);
extern u8 KDP_catch_image_mode(u16 nSrcType);
extern u8 KDP_set_mass_data_header(msg_mass_data_header* tHeader);
extern void KDP_clr_mass_data_header(void);
extern uint8_t DSM_Uart_StartOrStopDemoMode(uint8_t Enable);//lmm-add

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x40-0x6F Unit control
extern uint8_t DSM_Uart_TurnOnOffCamera(u8 nIdx, u8 nCtrl);
extern uint8_t DSM_Uart_TurnOnOffVisLed(BOOL bSw);
extern uint8_t DSM_Uart_TurnOnOffIrLed(BOOL bSw);
extern uint8_t DSM_Uart_TurnOnOffStructLed(BOOL bSw);
extern uint8_t DSM_Uart_TurnOnOffPanel(u8 nCtrl);

extern uint8_t kn_uart_set_exposure_time( s_msg_exp_time tInfo );
extern u8 kn_uart_set_gain(u8 cam_index, u16 gain);

//extern uint8_t DSM_Uart_TurnOnStructLed(void);//lmm-add
//extern uint8_t DSM_Uart_TurnOffStructLed(void);//lmm-add
//extern uint8_t DSM_Uart_TurnOnLed(void);//lmm-add
//extern uint8_t DSM_Uart_TurnOffLed(void);//lmm-add
extern uint8_t DSM_Uart_SET_THRESHOLD_LEVEL(uint8_t verify_level,uint8_t live_level);
extern uint8_t DSM_Uart_ConfigBaudrate(uint8_t baudrate_index);//lmm-add

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x70-0x7F MP
extern uint8_t DSM_Uart_SysInitReadyTime(uint8_t time[4]);
extern uint8_t DSM_Uart_SwitchPart(uint8_t nSwPart, uint8_t* nCurPart);
extern uint8_t DSM_Uart_Get_Cur_Part(uint8_t* nCurPart);
extern uint8_t KDP_Uart_MP_Calibration(u8 eMode);
extern uint8_t DSM_Uart_StartSnapPics(void);//lmm-add
extern uint8_t DSM_Uart_StopSnapPics(void);//lmm-add

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0xF0-0xF7 Key
extern uint8_t DSM_Uart_Init_Encryption(msg_init_encryption_data encryption, uint8_t device_id[DEVICE_ID_NUM]);//lmm-add
extern uint8_t DSM_Uart_SetReleaseEncKey(msg_enc_key_number_data key_number);//lmm-add
extern uint8_t DSM_Uart_SetDebugEncKey(msg_enc_key_number_data key_number);//lmm-add
extern uint8_t DSM_Uart_SET_Interactivate(msg_interactivate_param interactivate_param);//lmm-add

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0xF8-0xFF OTA
extern uint8_t DSM_Uart_StartOta(uint8_t v_primary, uint8_t v_secondary,uint8_t v_revision);//lmm-add
extern uint8_t DSM_Uart_GetOtaStatus(uint8_t* ota_status, uint8_t next_pid_e[2]);//lmm-add
extern uint8_t DSM_Uart_OtaHeader(msg_mass_data_header* otaheader);//lmm-add
extern uint8_t DSM_Uart_OtaPacket(msg_mass_data_pkg otapacket);//lmm-add
extern uint8_t Ota_final_packet( void );
#endif

extern uint16_t uart_face_add_timeout(u8_t admin, char *user_name, u8_t face_type, uint16_t time_out_ms, uint8_t cmd_id);
extern uint16_t uart_face_recognition_timeout(u8 pd_rightaway, uint16_t time_out_ms);   //power down rightaway
extern uint8_t  uart_face_snap_image(void);
u8 _uart_face_del_some_users(u8 face_id0, u8 face_id1);
void wait_fr_update_thread(void);

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK )
extern uint32_t SysWaitDelayTime(void);
extern uint8_t get_mass_data_status(void);
extern uint8_t get_ota_data_status( void );
extern uint8_t DSM_Uart_StopOta(void);//lmm-add
extern uint8_t OtaProcess_run( u8 bypass );
#endif

#if (CFG_E2E_REC_NOTE == YES)
extern void kl520_face_recognition_note(void);
#endif

#endif
#endif
#endif    //__KDP_COMM_APP_H__
