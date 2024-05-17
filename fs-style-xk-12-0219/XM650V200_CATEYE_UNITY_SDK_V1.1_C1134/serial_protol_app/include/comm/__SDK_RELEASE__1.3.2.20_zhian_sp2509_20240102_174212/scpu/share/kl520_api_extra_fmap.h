#ifndef __KL520_API_EXTRA_MAP_H__
#define __KL520_API_EXTRA_MAP_H__

#include "kdp_e2e.h"
#include "kdp_e2e_util.h"
#include "kdp_e2e_ctrl.h"
#include "kdp_e2e_r1n1.h"
#include "kdp_e2e_face.h"
#include "kdp_e2e_prop.h"
#include "kdp_e2e_db.h"

typedef enum {
    IMP_SUCCESS = 0,
    IMP_CONTIUNOUS,
    IMP_FAIL_IDX_EXISTED_USER,
    IMP_FAIL_DB_ERR,
    IMP_FAIL_FM_ERR,
}eIMP_RSP_TYPE;

#if CFG_FMAP_EXTRA_ENABLE == YES

#define EXTRA_FMAP_CLEAR 0
#define EXTRA_FMAP_START 1
#define EXTRA_FMAP_UPDATE 2

#define TEST_CASE_EXTRA 2


//#define CFG_FMAP_EXTRA_ENABLE TEST_CASE_EXTRA


#define EXTRA_FMAP_ADV
#define EXTRA_FMAP_ADV_NUM    (6)
#define EXTRA_FMAP_SRC_NUM    (2) //2(mipi),1(info)

/*
#define KDP_DDR_TEST_RGB_FR_SIZE           0x00000400 // 1K
#define KDP_DDR_TEST_NIR_FR_SIZE           0x00000400 // 1K
#define KDP_DDR_TEST_USER_DB_SIZE			0x00004000 // 16K

#define KDP_DDR_TEST_EXTRA_RGB_ADDR			KDP_DDR_TEST_START_ADDR
#define KDP_DDR_TEST_EXTRA_NIR_ADDR			(KDP_DDR_TEST_EXTRA_RGB_ADDR + KDP_DDR_TEST_RGB_FR_SIZE)
#define KDP_DDR_TEST_EXTRA_DB_ADDR			(KDP_DDR_TEST_EXTRA_NIR_ADDR + KDP_DDR_TEST_NIR_FR_SIZE)
*/

#define RSP_REGISTER_RET_CMD    0x00001055
#define RSP_RECOGNIZE_RET_CMD   0x00002055
#define RSP_DELETE_ALL_RET_CMD  0x00003055
#define RSP_DELETE_ONE_RET_CMD  0x00004055


typedef enum {
    EXTRA_FMAP_CLOSE_e = 0,
    EXTRA_FMAP_BYPASS_e,
    EXTRA_FMAP_FR_e,
    EXTRA_FMAP_SIZE_e
}eEXTRA_FMAP_TYPE;

typedef enum {
    EXPORT_DB_MODE_DB_IN = 0,
    EXPORT_DB_MODE_DB_OUT,
    EXPORT_DB_MODE_DB_IMG_OUT,
    EXPORT_DB_MODE_DB_SIZE
}eEXPORT_DB_TYPE;

typedef enum {
    USB_CMD_FM_NULL_e = 0,
    USB_CMD_FM_REGISTER_e,
    USB_CMD_FM_RECOGNIZE_e,
    USB_CMD_FM_DEL_ALL_e,
    USB_CMD_FM_DEL_USER_e,
    USB_CMD_FM_RX_DB_e,
    USB_CMD_FM_SIZE_e
}eUSB_CMD_FM_STATUS;


///////////////
/*  Customer Info API */
///////////////

extern bool extra_db_fmap_mode;
extern u8 g_FmapUsbBufCnt;
extern u8 g_FmapShotCnt;
extern u8 g_FmapReadCnt;

void KL520_api_set_extra_fmap_adv_mode( eEXTRA_FMAP_TYPE type );
int kl520_api_extra_fmap_fdfr_catch(int cam_idx);

int kl520_api_extra_fmap_fdfr_cam(int cam_idx, u32 addr);
u32 kl520_api_extra_fmap_ddr_addr(int cam_idx);
u32 kl520_api_extra_db_map_addr( void );
void kl520_api_extra_fmap_adv_chk(void);
void kl520_api_extra_fmap_adv_init(int idx, uint32_t buf_size);
void kl520_api_extra_fmap_mode(bool active);
bool kl520_api_get_extra_fmap_mode( void );
void kl520_api_extra_fmap_adv_shot(int idx, u8 face_idx);
void kl520_api_extra_fmap_adv_shot_save( int ret );
int kl520_api_extract_fr_map( int cam_idx );
void kl520_api_extra_fmap_adv_load(int cam_idx, u8 face_idx);
int kl520_api_extra_fmap_fdfr_reset(void);
u32 kl520_api_extra_fmap_addr(void);
void kl520_api_extra_fmap_init(void );
void kl520_api_extra_fmap_close(void);

#if ( CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE )
void kdp_api_db_set_last_register_id_preprocess( void );
void kdp_api_db_set_last_register_id_postprocess( void );
u8 kdp_api_ap_control_set_each_db( u16 user_idx );
u32 kdp_api_ap_control_del_each_db( u16 user_idx );
#if CFG_FMAP_AP_CTRL_TYPE < EX_FM_UART_AP_CTRL_MAIN_DB
eUSB_CMD_FM_STATUS KL520_api_ap_com_get_extra_fmap_status( void );
void KL520_api_ap_com_set_extra_fmap_status( eUSB_CMD_FM_STATUS status );
void kdp_api_ap_com_set_user_id( u8 user_id );
u16 kdp_api_ap_com_get_usrt_id( void );
void kdp_api_ap_com_set_host_result( u8 action );
u8 kdp_api_ap_com_get_host_result( void );
void kl520_api_ap_com_set_timeout_and_start( int time_out_s );
bool kl520_api_ap_com_chk_timeout_and_stop( void );
u8 kdp_api_ap_com_check_host_command_type( u32 check );
u8 kdp_api_ap_com_wait_host_cmd_result( void );
u8 kdp_api_ap_com_wait_host_db_result( void );
#if CFG_FMAP_AP_CTRL_TYPE == EX_FM_USB_AP_CTRL_ALL
s32 kdp_api_ap_com_wait_host_compare_result( u16 *lp_user_id );
#endif //#if CFG_FMAP_AP_CTRL_TYPE == EX_FM_USB_AP_CTRL_ALL
#endif //#if CFG_FMAP_AP_CTRL_TYPE < EX_FM_UART_AP_CTRL_MAIN_DB
#endif // #if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE


#if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE
u16 kl520_api_ap_com_db_enable_fm_extra_mode( eEXTRA_FMAP_TYPE mode_idx );
u16 kl520_api_ap_com_db_enable_export_db_mode( u8 mode_idx );
u16 kl520_api_ap_com_db_query_db_all_mode( u8* bin_table );
u16 kl520_api_ap_com_db_query_db_one_mode( u16 user_id );
extern void kl520_api_ap_com_db_catch_fm_mode( void );
extern void kl520_api_ap_com_db_export_db_mode( u16 sector_idx );



#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF ) || ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF_USR )
extern BOOL g_bImpFmDataReady;

u16 kl520_api_ap_com_db_import_db_mode( u16 user_idx, u16 packet_idx, u8* packet_data_in );
u16 kl520_api_ap_com_import_fm_mode(u16 nPkgIdx, u16 nPkgSize, u16 nTotPkgNum, u16 nStdPkgSize, u8* pDataAddr, BOOL nAfterChk);

#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP ) || ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP_USR )
extern u16 kl520_api_ap_com_db_import_db_mode_split(u16 nUserIdx, u16 nPkgIdx, u16 nPkgSize, u16 nTotPkgNum, u16 nStdPkgSize, u8* pDataAddr);

#endif

extern u8 kl520_api_ap_com_import_all_db_mode_split(u16 nPkgIdx, u16 nPkgSize, u16 nTotPkgNum, u16 nStdPkgSize, u8* pDataAddr, u32 nTotalSize);


#endif //#if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE



#endif

extern void kl520_api_ap_com_import_fm_r1_inject(void);
extern void kl520_api_ap_com_import_fm_n1_inject(void);
//extern void kl520_api_ap_com_import_fm_n1_inject_int8(void);
extern void kl520_api_ap_com_import_fm_r1n1_inject(void);
extern u16 kl520_api_ap_com_import_fm_mode_split(u16 nPkgIdx, u16 nPkgSize, u16 nTotPkgNum, u16 nStdPkgSize, 
    u16 nTotPkgSize, u8* pDataAddr, BOOL nAfterChk);

#endif
