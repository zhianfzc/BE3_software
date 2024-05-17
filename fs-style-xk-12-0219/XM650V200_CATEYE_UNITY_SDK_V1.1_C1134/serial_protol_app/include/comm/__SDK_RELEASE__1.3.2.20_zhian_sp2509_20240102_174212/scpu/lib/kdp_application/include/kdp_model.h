#ifndef __MODEL_MGR_H__
#define __MODEL_MGR_H__

#include <stdint.h>
#include "ipc.h"
#include "base.h"
#include "kdp_app.h"
#include "kdp_com.h"
#include "model_type.h"

#define KDP_MODEL_ALL_MODELS  -1

/* flash memory table
 *    BOOT related       :  192K(reserved)      0x0         - 0x2FFFF
 *    FLASH_ADDR_HEAD    :                      0x30000
 *    FID database       :  512K(reserved)      0x80000     - 0xFFFFF
 *    fw_info.bin        :    4K(reserved)      0x100000    - 0x100FFF  : real use: depend on model count
 *      - model count : 4 bytes
 *      - models      :sizeof(struct kdp_model_s)*count
 *      - ddr end addr:4 bytes
 *
 *    bin files for models: (4K aligned)        0x101000     - ??       : real use: depend model size
 *      - cmd.bin + weight.bin + setup.bin
 *    FLASH_ADDR_END                            0x101000 + model size   -> + 4K alignment
 */

/* size of fw_info.bin & all_models.bin */
#define MODEL_MGR_SIZE_MODEL_INFO  224      // (232 - 4 -4 )   todo: must specify the size
#define MODEL_MGR_SIZE_MODEL_POOL  3572576  // todo: musth specify the size

#define MODEL_MGR_FLASH_ADDR_HEAD               0x00143000
#define MODEL_MGR_FLASH_ADDR_MODEL_COUNT        (MODEL_MGR_FLASH_ADDR_HEAD)
#define MODEL_MGR_FLASH_ADDR_MODEL_INFO         (MODEL_MGR_FLASH_ADDR_MODEL_COUNT + 4)
#define MODEL_MGR_FLASH_ADDR_MODEL_DDR_END_ADDR (MODEL_MGR_FLASH_ADDR_MODEL_INFO  + MODEL_MGR_SIZE_MODEL_INFO)
#define MODEL_MGR_FLASH_ADDR_MODEL_POOL         0x00144000

/* These crc values will ONLY change if model is changed in FLASH */
#define MODEL_INFO_CRC16_VALUE  0x5341
#define MODEL_INFO_SUM32_VALUE  0x00eee074

#ifndef FLASH_MODEL_HAS_CRC_CHECK
#define MODEL0_FD_CRC16_VALUE   0x939f   /* original: 0x4318 */
#define MODEL1_LM_CRC16_VALUE   0x1cd0   /* original: 0xfce0 */
#define MODEL3_FR_CRC16_VALUE   0x5df0   /* original: 0xb359 */
#define MODEL0_FD_SUM32_VALUE   0xf9e571d8
#define MODEL1_LM_SUM32_VALUE   0x3f96c4fc
#define MODEL3_FR_SUM32_VALUE   0xbc998f45
#endif

enum kdp_model_status_code_e {
    KDP_MODEL_OK = 0,
    KDP_MODEL_UNKNOWN_ERR,
    KDP_MODEL_ABORT,
    KDP_MODEL_FDR_WRONG_USAGE = 0x100, // KDP_MODEL_FID_CODES
    KDP_MODEL_FDR_NO_FACE,
    KDP_MODEL_FDR_BAD_POSE,
    KDP_MODEL_FDR_NO_ALIVE,
    KDP_MODEL_FDR_FR_FAIL,
    KDP_MODEL_SIZE_SMALL,
    KDP_MODEL_FDR_NO_LM
};

enum kdp_model_rc {
    // 0 - 9 is reserved for ncpu return
    // defined in ipc.h
    // IMAGE_STATE_INACTIVE == 0
    // IMAGE_STATE_ACTIVE == 1
    // IMAGE_STATE_NPU_DONE == 2
    // IMAGE_STATE_DONE == 3

    KDP_MODEL_RUN_RC_ABORT = 10,
    KDP_MODEL_RUN_RC_ERROR = 11,
    KDP_MODEL_RUN_RC_END
};

/* ############################
 * ##    Public Functions    ##
 * ############################ */
int kdp_model_get_raw_img_idx(int ipc_com_idx);
struct kdp_img_raw_s* kdp_model_get_raw_img(const struct kdp_img_cfg *img_cfg);
struct kdp_img_raw_s* kdp_model_get_raw_img_by_idx(int idx);
/**
 * @brief Get raw image config
 * @param idx image index
 * @return raw image config
 */
struct kdp_img_raw_s* kdp_model_get_raw_img_by_idx(int idx);

/**
 * @brief Init model functionality
 */
void kdp_ipc_init(BOOL log_enable);

/**
 * @brief A wrapper of load_model
 * @param model_index_p: model info index,
 *                    0-n: info_index of model to load
 *                    -1 means to load all models
 * @return always 0
 */
int32_t kdp_model_load_model(int8_t model_info_index);

/**
 * @brief A wrapper of load_model_info
 * @param [in] is_model_from_ddr: if model is from ddr/host command
 */
void kdp_model_reload_model_info(bool from_ddr);

/**
 * @brief Refresh all models
 */
void kdp_model_refresh_models(void);

/**
 * @brief Output model_info of specified index
 * @param[in] idx_p the index of programmed models
 * @return model_info defined in ipc.h
 */
struct kdp_model_s* kdp_model_get_model_info(int idx_p);

/**
 * @brief Specify output address for model run in ncpu/npu
          !!! must be called after kmdw_model_config_model()
 * @return always 0
 */
int32_t kmdw_model_config_result(osEventFlagsId_t result_evt, uint32_t result_evt_flag);

//int32_t kdp_model_run(uint32_t model_type);

/**
 * @brief Config model image
 * @param img_cfg image config
 * @param crop_box image crop config
 * @param pad_values image padding config
 * @param ext_param extra param
 */
void kdp_model_config_img(struct kdp_img_cfg *img_cfg, struct kdp_crop_box_s *crop_box,
                          struct kdp_pad_value_s *pad_values, void *ext_param);

int32_t kdp_model_config_liveness_img(struct kdp_img_cfg *p_kdp_image_p,
                                      struct kdp_img_cfg *p_rgb_kdp_image_p,
                                      struct kdp_crop_box_s *p_kdp_crop_box_p,
                                      struct dual_landmarks_s *p_ext_pararms_p,
                                      struct dual_landmarks_s *p_ext_pararms_2_p,
                                      struct nir_camera_tune_s *p_nir_tune_p );

/**
 * @brief Run model
 * @param tag model tag
 * @param output model output
 * @param model_type model type
 * @param dme DME mode
 * @return kmdw_model_rc
 */
int kdp_model_run(const char *tag, void *output, uint32_t model_type, bool dme);
int kdp_model_run_ex(const char *tag, void *output,
                      uint32_t model_type, bool model_from_ddr,
                      struct kdp_img_cfg *img_cfg,
                      struct kdp_crop_box_s *crop_bbox,
                      struct kdp_pad_value_s *pad_values,	 
                      void *ext_param);

int32_t kdp_model_fd(struct facedet_result_s *out,
                     struct kdp_img_cfg *img_cfg);

int32_t kdp_model_crop_fd(struct facedet_result_s *out, int32_t* xywh, 
                     struct kdp_img_cfg *img_cfg);

int32_t kdp_model_lm(struct landmark_result_s *out, /*output*/
                     struct kdp_img_cfg *img_cfg,
                     struct facedet_result_s *fd_bbox);

int32_t kdp_model_face_pose(struct face_occlude_result_s *out, /*output*/
                     struct kdp_img_cfg *img_cfg,
                     struct landmark_result_s *lm);

int32_t kdp_model_lm_s(struct landmark_result_s *out, /*output*/
                     struct kdp_img_cfg *img_cfg,
                     struct facedet_result_s *fd_bbox);

int32_t kdp_model_eye_lid(struct landmark_result_s *lm, /*output*/
                     struct kdp_img_cfg *img_cfg,
                     struct eye_lid_lm_result_s *eye_lid, int eid);

int32_t kdp_model_fr(struct fr_result_s *out, /*output*/
                     struct kdp_img_cfg *img_cfg,
                     struct facedet_result_s *fd_bbox,
                     struct landmark_result_s *lm_pts, int flip_face);
                     
int32_t kdp_model_face_quality(struct face_quality_result_s *out, /*output*/
                     struct kdp_img_cfg *img_cfg,
                     struct facedet_result_s *fd_bbox,
                     struct landmark_result_s *lm_pts);

int32_t kdp_model_cv_lv(struct lv_result_s *p_lv_output_p, /*output*/
                     struct kdp_img_cfg *p_nir_img_setting_p,
                     struct kdp_img_cfg *p_rgb_img_setting_p,
                     struct dual_landmarks_s *p_lm_pts_p, // dual landmarks
                     struct dual_landmarks_s *p_lm_pts_2_p, // second dual landmarks
                     struct nir_camera_tune_s *p_nir_tune_p,
                     uint32_t model_type_p);

int32_t kdp_model_fuse_lv(struct lv_result_s *p_lv_output_p, /*output*/
                     struct kdp_img_cfg *p_nir_img_setting_p,
                     struct kdp_img_cfg *p_rgb_img_setting_p,
                     struct dual_landmarks_s *p_lm_pts_p, // dual landmarks
                     struct nir_camera_tune_s *p_nir_tune_p,
                     uint32_t model_type_p);

/**
 * @brief  do nir liveness
 */										 
int32_t kdp_model_nir_lv(struct lv_result_s *out, /*output*/
                     struct kdp_img_cfg *img_cfg,
                     //struct facedet_result_s *fd_bbox,
                     struct landmark_result_s *lm_pts, // not use now
                     uint32_t model_type);

int32_t kdp_model_nir_eye_lv(struct lv_result_s *out,
                     struct kdp_img_cfg *img_cfg,
                     struct landmark_result_s *lm_pts,
                     uint32_t model_type);
/**
 * @brief  do nir hsn liveness
 */										 
int32_t kdp_model_nir_hsn_lv(struct lv_result_s *out, /*output*/
                     struct kdp_img_cfg *img_cfg,
                     //struct facedet_result_s *fd_bbox,
                     struct landmark_result_s *lm_pts, // not use now
                     uint32_t model_type);
/**
 * @brief  do nir sl liveness
 */										 
int32_t kdp_model_nir_sl_lv(struct lv_result_s *out, /*output*/
                     struct kdp_img_cfg *img_cfg,
                     struct facedet_result_s  *pfd,
                     struct landmark_result_s *lm_pts, // not use now
                     uint32_t model_type);    

int32_t kdp_model_fuse_nir_lv(struct lv_result_s *out, /*output*/
                     struct kdp_img_cfg *img_cfg,
                     //struct facedet_result_s *fd_bbox,
                     struct landmark_result_s *lm_pts, // not use now
                     uint32_t model_type);

int32_t kdp_model_hand(dme_res *out,
                     struct kdp_img_cfg *img_cfg);
                     
int32_t kdp_model_hand_kp(struct hand_kp_result_s *out, /*output*/
                     struct kdp_img_cfg *img_cfg,
                     struct facedet_result_s *fd_bbox);
int32_t kdp_model_palm_lv(struct lv_result_s *out, /*output*/
                     struct kdp_img_cfg *img_cfg,
                     struct facedet_result_s *fd_bbox);

int32_t kdp_model_palm_print_fm(struct fr_result_s *out, /*output*/
                     struct kdp_img_cfg *img_cfg,
                     struct hand_kp_result_s *lm_pts);
/**
 * @brief Abort model execution
 */
void kdp_model_abort(void);

#if DEBUG

/**
 * @brief Dump model debug info
 */
void kdp_model_dump_model_info(void);

#endif

void kdp_model_init_addr( u32 head, u32 model_count, u32 model_info, u32 ddr_end_addr , u32 model_pool);
void kdp_model_show_info(void);
int kdp_model_version(u8 idex);
int kdp_model_get_model_count(void);
int kdp_model_info_get( uint32_t *second_last_start, uint32_t *last_start ,uint32_t *model_size );
void kdp_model_info_reload(void);
void kdp_model_info_clear(void);
void kdp_model_info_reload_test(void);
uint8_t kdp_model_load_flag( uint8_t model_index_p);

int32_t kdp_model_config_result_addr(int model_slot_idx, void *addr_p);
int32_t kdp_model_config_model(uint32_t model_type, bool model_from_ddr);
int32_t kdp_model_config_virtual_model(uint32_t model_type);
void kdp_model_show_info(void);
u8 kdp_all_model_version(void);
int kdp_clc_all_model_size(void);
int kdp_crc_offset_in_fwinfo(void);
void kdp_set_model_offset( u32 offset);
u32 kdp_get_model_offset(void);
void kdp_set_fwinfo_offset( u32 offset);
u32 kdp_get_fwinfo_offset(void);

u8 kdp_is_abort_flag(void);
void kdp_set_abort_flag(u8 flag);

u8 kdp_is_palm_mode(void);
void kdp_set_palm_mode(u8 pm);

#define DB_DEFAULT_RGB_TO_NIR_RATIO     85
void kdp_set_rgb_to_nir_ratio(u8 ratio);
u8   kdp_get_rgb_to_nir_ratio(void);

#endif
