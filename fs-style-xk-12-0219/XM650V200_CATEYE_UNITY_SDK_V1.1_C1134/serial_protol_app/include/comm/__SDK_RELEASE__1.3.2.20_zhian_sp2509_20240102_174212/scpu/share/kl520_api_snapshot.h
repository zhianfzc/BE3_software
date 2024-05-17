#ifndef __KL520_API_SNAPSHOT_H__
#define __KL520_API_SNAPSHOT_H__

#include "kdp_e2e.h"
#include "kdp_e2e_util.h"
#include "kdp_e2e_ctrl.h"
#include "kdp_e2e_r1n1.h"
#include "kdp_e2e_face.h"
#include "kdp_e2e_prop.h"
#include "kdp_e2e_db.h"

#define SNAPSHOT_ADV_NUM    (3)
#define SNAPSHOT_SRC_NUM    (3) //2(mipi),1(info)
#define MIPI_CAM_INF        (IMGSRC_NUM) //2, virtual

#define SNAP_SHOT_EVENT_READY  0x40

extern bool snapshot_adv_mode;
extern u8 snapshot_adv_select;

enum FACE_SIM_PARAM_IDX
{
    SIM_CFG_BOARD_PARAMS_0_ID =     0,
    SIM_INIT_TILE_FLAG =            1,
    SIM_INIT_EXP_TILE =             2,
    SIM_NIR_MODE =                  3,
    SIM_INIT_NIR_GAIN =             4,
    SIM_NIR_GAIN =                  5,
    SIM_NIR_CUR_EXP_TIME =          6,
    SIM_RGB_LED_FLAG =              7,
    SIM_RGB_CUR_EXP_TIME =          8,
    SIM_REGISTERED_OFFSETX =        13,
    SIM_REGISTERED_OFFSETY =        14,
    SIM_RGB_LED_LV_HISTORY_FLAG =   29,
    SIM_INIT_TILE =                 37,
};

//API declare
void kl520_api_snapshot_adv_init(int idx, uint32_t buf_size);
int kl520_api_save_to_snap_ddr(int cam_idx, u32 addr, int size);
int kl520_api_save_snap_img_addr(int cam_idx, int face_idx, int off_w, int off_h);
#if CFG_USB_SIMTOOL == 1
void kl520_api_sim_write_json(void);
#endif
void kl520_api_snapshot_dma(u32 writeAddr, u32 readAddr, u32 len);

#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
void kl520_api_snapshot_adv_select(u8 idx);
void kl520_api_snapshot_adv_mode(void);
void kl520_api_snapshot_adv_shot_cont(u32 ret);

void kl520_api_snapshot_adv_shot_5face(void);
void kl520_api_snapshot_adv_shot_after_fdfr_element(void);

#if CFG_USB_SIMTOOL == 1
void kl520_api_snapshot_inf_decoder(void);
#endif
u32 kl520_api_snapshot_ddr_addr(int cam_idx);

#endif

#if CFG_SNAPSHOT_ENABLE == 1
void kl520_api_snapshot_init(void );
int kl520_api_snapshot_fdfr_cam(int cam_idx, u32 addr);
u32 kl520_api_snapshot_addr(int cam_idx);

#endif

///////////////
/* Snapshot API */
///////////////
#if CFG_SNAPSHOT_ENABLE == 2

extern u8 g_SnapUsbBufCnt;
extern u8 g_SnapShotCnt;
extern u8 g_SnapReadCnt;

typedef struct _KL520_SNAPSHOT_INFO_
{
    u8  nSanpIdx;
    u32 nRecIdx;

    u32 nInfoAddr;
    u32 nInfoLen;

    u32 nDataAddr;
    u32 nDataLen;
    u32 nLength;

    u16 nWidth;
    u16 nHeight;
    u64 nTime;
    u8  nResult;

} kl520_snapshot_info;

void kl520_api_snapshot_init(void );
int kl520_api_snapshot_fdfr_cam(int cam_idx, u32 addr);
u32 kl520_api_snapshot_addr(int cam_idx);
int kl520_api_snapshot_fdfr_catch(int cam_idx);
void kl520_api_snapshot_adv_chk(void);

void kl520_api_ap_com_snapshot_offset_merge( void );

u32 kl520_api_snapshot_db(void);
u32 kl520_api_snapshot_falshIdx(void);

int kl520_api_snapshot_record(u8 idx, u64 time, u8 result);
int kl520_api_snapshot_show(u8 idx);
int kl520_api_snapshot_delete(BOOL all ,u8 idx);

int kl520_api_snapshot_record_img(u8 nRecIdx, u8 nNumCnt, u64 time, u8 result);
int kl520_api_snapshot_record_img_early(u8 nRecIdx, u8 nNumCnt, u64 time, u8 result);
int kl520_api_snapshot_show_img(u8 nRecIdx, u8 nNumCnt);
int kl520_api_snapshot_del_img(u8 nDelIdx, u8 nNumCnt);




int kl520_api_snapshot_get(u8 idx, kl520_snapshot_info *snapinfo);
u32 kl520_api_snapshot_data_addr(kl520_snapshot_info *snapinfo);
u8 kl520_api_snapshot_recidx(void);
int kl520_api_snapshot_enable(bool enable);
void kl520_api_snapshot_shot(u32 disp_addr);


enum SNAPIMG_STATE
{
    SNAPIMG_SUCCESS = 0,
    SNAPIMG_IDX_OVERFLOW,
    SNAPIMG_BUF_OVERFLOW,
    SNAPIMG_IMG_NULL,
    SNAPIMG_DEL_FAIL,
    SNAPIMG_WD_FAIL,
    SNAPIMG_CAM_NULL,
    SNAPIMG_TIMEOUT,
    SNAPIMG_STATE_NUM,
};


u32 kl520_api_host_com_snpahot_status( void );
u32 kl520_api_ap_com_snapshot_catch( u16 src_type );
u32 kl520_api_ap_com_snapshot_catch_addr( u16 src_type );


/*sim*/
#if (CFG_SNAPSHOT_ADVANCED == 1)
void kl520_api_snapshot_fdfr_save(void);
#endif

void kl520_api_snapshot_adv_load(int cam_idx, u8 face_idx);

#endif


#if CFG_USB_EXPORT_STREAM_IMG == YES
typedef enum {
    STRAM_IMAGE_RGB_640x480_e = 0,
    STRAM_IMAGE_RGB_320x480_e,
    STRAM_IMAGE_RGB_320x240_e,
    STRAM_IMAGE_NIR_480x640_e,
    STRAM_IMAGE_BOTH_CAMERA_e,
    STRAM_IMAGE_DISPALY_e,
    STRAM_IMAGE_TOTAL_SIZE_e
}eSTREAM_IMAGE_EXPORT_SRC;

s8 kl520_api_export_stream_get_image_crtl( void );
eSTREAM_IMAGE_EXPORT_SRC kl520_api_export_stream_get_image_export_crtl( void );
void kl520_api_export_stream_get_info( u32 para, s8 *cam_idx, eSTREAM_IMAGE_EXPORT_SRC *export_mode );
void kl520_api_export_stream_set_image_crtl( s8 in, eSTREAM_IMAGE_EXPORT_SRC export_src_idx );
u32 kl520_api_export_stream_image_ddr_addr(eSTREAM_IMAGE_EXPORT_SRC cam_idx);
int kl520_api_export_stream_image_catch( void );
u32 kl520_api_export_stream_image_addr(eSTREAM_IMAGE_EXPORT_SRC cam_idx);
int kl520_api_export_stream_image(int cam_idx, u32 addr, eSTREAM_IMAGE_EXPORT_SRC export_src_idx);
int kl520_api_export_stream_image_ready(void);

#if CFG_USB_EXPORT_LIVENESS_RET == YES
void kl520_api_usb_set_export_liveness_ret( u8 in );
u32 kl520_api_export_stream_get_fd_box( void );
#endif

#endif //CFG_USB_EXPORT_STREAM_IMG


#endif //__KL520_API_SNAPSHOT_H__
