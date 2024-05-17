#ifndef __KDP_E2E_FACE_H__
#define __KDP_E2E_FACE_H__


#include "board_kl520.h"
#include "types.h"
#include "ipc.h"
#include "kdp_fb_mgr.h"
#include "kdp_app_db.h"
#include "kdp_e2e_camera.h"
#include "kdp_e2e_prop.h"

#define KDP_E2E_IMG_SRC_MAX 2
#define KDP_E2E_IMG_SRC_RGB MIPI_CAM_RGB
#define KDP_E2E_IMG_SRC_NIR MIPI_CAM_NIR
#define KDP_E2E_IMG_SRC_LV  KDP_E2E_IMG_SRC_MAX
#ifndef CFG_RGB_CV_LIVENESS
#define CFG_RGB_CV_LIVENESS      (YES)
#endif

#define KDP_E2E_IMAGE_FORMAT_RGB565     (IMAGE_FORMAT_SUB128 | NPU_FORMAT_RGB565)
#define KDP_E2E_IMAGE_FORMAT_RAW8       (IMAGE_FORMAT_SUB128 | NPU_FORMAT_NIR)
#define KDP_E2E_IMAGE_FORMAT_LV_NCPU    (KDP_E2E_IMAGE_FORMAT_RAW8 | IMAGE_FORMAT_BYPASS_NPU_OP)

#define E2E_CHK_BREAK(func)     { ret = (func); if (E2E_OK != ret) break; }
#define FR_LATER                (YES)
#define USE_R1_FD_POSTPROC      (YES)
#define USE_R1_LM_POSTPROC      (YES)
#define USE_N1_FD_POSTPROC      (NO)
#define USE_N1_LM_POSTPROC      (YES)

#if (CFG_CAMERA_DUAL_1054 == YES)
#define USE_RGB_FR_IN_1054      (YES)
#else
#define USE_RGB_FR_IN_1054      (NO)
#endif

#define USE_LM_MOTION_DETECT    (YES)
#define USE_NOSE_LM_DIFF        (YES)
#define USE_AGE_GROUP           (NO)
#define USE_CHK_BAD_POSE        (YES)
#define RGB_LED_WHEN_REG        (NO)
#define CONTINUE_NIR_AEC        (YES)
#define USE_CHK_R1_MASK         (YES)
#define USE_LM_SECONDARY        (NO)
#define USE_EYE_LID_LM          (YES)
#define USE_RGB_LED_SHORTEN     (YES)
#define USE_NIR_LED_SHORTEN_RECOG (NO)
#define LED_OPEN_MEASUREMENT    (NO)
#define USE_CHK_IOU             (NO)
#define NIR_OCCULDE_ENABLE      (NO)
#define NIR_FACE_SEG_ENABLE     (YES)
#ifndef USE_FACE_QUALITY
#define USE_FACE_QUALITY        (NO)
#endif
#define RGB_FACE_QUALITY_TH_NORMAL      0.5f
#define RGB_FACE_QUALITY_TH_OTHER       0.1f

#define NIR_FACE_QUALITY_TH_NORMAL      0.2f
#define NIR_FACE_QUALITY_TH_OTHER       0.05f

#if (CFG_CAMERA_DUAL_1054 == YES || CFG_CAMERA_SINGLE_1054 == YES)
#define USE_RECOG_FR_UPDATE     (NO)
#else
#define USE_RECOG_FR_UPDATE     (YES)
#endif

#define FACE_RECOG_LED_OPEN_NO_FD  (YES)

#if (CFG_CAMERA_DUAL_1054 == YES || CFG_CAMERA_SINGLE_1054 == YES)
#define USE_FR_FACE_FLIP           (NO)
#else
#define USE_FR_FACE_FLIP           (YES)
#endif

#define NIR_LM_SCORE_FILTER
#define DUAL_FR_DIFF            (YES)
#define TMP_LM_LIST_SIZE        5
#define FD_CROP_MODE_NUM        3

typedef enum kdp_e2e_nir_fd_mode_enum
{
    FD_NIR_IMG_SIZE_MODE = 0,
    FD_NIR_CROP_MODE,
    
} nir_fd_mode;

typedef enum kdp_e2e_rgb_fd_mode_enum
{
    FD_RGB_NN = 0,
    FD_RGB_MAPPING,
    
} rgb_fd_mode;

typedef enum kdp_e2e_nir_led_mode_enum
{
    IR_LIGHT = 0,
    STR_LIGHT,
    IR_LIGHT_FLASH,
    NO_LED,
} nir_led_mode;

typedef enum kdp_e2e_nir_aec_enum
{
    GAIN_AEC_T = 0,
    EXP_AEC_T,

} kdp_nir_aec;

typedef enum kdp_e2e_rgb_nir_lm_enum
{
    RGB_LM_T = 0,
    NIR_LM_T,

} kdp_rgb_nir_lm;

typedef enum kdp_e2e_nir_mode_enum
{
    IR_FEW = 0,
    IR_INFRONT,
    IR_BACKLIGHT,
#if CFG_NIR_MODE2_SPLIT == YES
    IR_BACKLIGHT_EX,
#else
    IR_WINDOW_SIDELIGHT,
#endif
    IR_LOTS,

} kdp_nir_mode;

typedef enum kdp_e2e_nir_init_flag_enum
{
    AEC_STABLE = 0,
    AEC_PROBE_SURROND,
    AEC_WATI_EXP_REDUCE,
    AEC_WAIT_LED_GAIN,
    AEC_LED_OFF_STATUS,
    AEC_LED_ON_PREPROC_STATUS,
    AEC_LED_ON_FACE_FAIL_TUNE,
    AEC_FACE_STABLE,
    AEC_SEARCH_FACE_MODE,

} kdp_nir_init_flag;

typedef enum kdp_e2e_flow_mode_enum
{
    FLOW_MODE_NORMAL = 0,
    FLOW_MODE_SIM_VIRTUAL,
    FLOW_MODE_SIM_MODELS,
    FLOW_MODE_SIM_COMP_R1N1,
    FLOW_MODE_SIM_PRE_ADD,
    FLOW_MODE_SIM_COMP_R1,

} kdp_e2e_flow_mode;

typedef enum kdp_e2e_face_mode_enum
{
    FACE_MODE_NONE = 0,
    FACE_MODE_ADD,
    FACE_MODE_RECOGNITION,
    FACE_MODE_LIVENESS,
    FACE_MODE_RECOGNITION_TEST,
    FACE_MODE_COMPARE_1VS1,
    FACE_MODE_ENG_CAL,
    FACE_MODE_SNAP_IMG,
    FACE_MODE_BUSY
} kdp_e2e_face_mode;


//typedef struct kl520_camera_param_rgb_struct
//{
//    u16 rgb_init_exp_time;
//    u16 rgb_cur_exp_time;
//    u8  rgb_dark_mode;
//    u8 rgb_dark_mode_cnt; // maximum is 5
//    u8 rgb_led_flag;
//    u8 rgb_led_cur_strength;
//    u16 rgb_led_special_cnt; // maximum unsure
//    u8 rgb_lm_motion_cnt; // maximum is 5
//    u8 rgb_dark_env_history_flag;
//    u8 rgb_lv_back_light_cnt;
//    bool rgb_lv_back_light;
//    u8 rgb_face_quality;
//    u8 rgb_corner_y;
//    bool rgb_led_lv_history_flag;
//    u8 step_cnt_rgb_wait_effect;
//    u8 rgb_face_l_luma;
//    u8 rgb_face_r_luma;
//    u8 pre_gain;
//    u8 post_gain;
//    u8 global_gain;
//    u8 y_average;
//    u8 rgb_avg_luma;
//    bool rgb_roi_set;
//    u8 led_breathe_en;
//}e2e_rgb_cam_params;

//typedef struct kl520_camera_param_nir_struct
//{
//    u8 status;
//    u16 nir_cur_exp_time;
//    u8 nir_mode;
//    //u8 nir_mode;
//    u8 nir_mode_cnt; // reserved
//    u8 nir_led_flag;
//    u8 nir_led_cur_strength;
//    u8 nir_led_special_cnt; // reserved
//    u8 nir_sensor_setting_cnt;
//    u8 step_cnt_wait_led_gain;
//    u8 nir_sensor_search_cnt;
//    float nir_gain;
//    float init_nir_gain;
//    u8 nir_led_status;
//    u8 nir_skin_luma;
//    u8 nir_env_exception_cnt;
//    
//}e2e_nir_cam_params;


typedef struct kdp_e2e_face_variables_struct {
    u8 pre_add;            //Fixed 1bytes

    struct frame_info info;
    s32 buf_idx[2];
    u32 user_id;
    char user_name[MAX_LEN_USERNAME];
    u8 admin;

    kdp_nir_init_flag init_tile_flag;
    u8 init_tile;
    u8 init_exp_tile;
    float target_tile_max;

    /* These variables will be collected as sub-structures classified as camera sensor related */
    u16 rgb_init_exp_time;
    u16 rgb_cur_exp_time;
    u8 rgb_dark_mode;
    u8 rgb_dark_mode_cnt; // maximum is 5
    u8 rgb_led_flag;
    u8 rgb_led_cur_strength;
    u16 rgb_led_special_cnt; // maximum unsure
    u8 rgb_lm_motion_cnt; // maximum is 5
    u8 rgb_dark_env_history_flag;
    u8 rgb_lv_back_light_cnt;
    bool rgb_lv_back_light;
    u8 rgb_face_quality;
    u8 rgb_corner_y;
    bool rgb_led_lv_history_flag;
    u8 step_cnt_rgb_wait_effect;
    u8 rgb_face_l_luma;
    u8 rgb_face_r_luma;
    u8 pre_gain;
    u8 post_gain;
    u8 global_gain;
    u8 y_average;

    u32 nir_cur_exp_time;
    kdp_nir_mode nir_mode;
    //u8 nir_mode;
    u8 nir_mode_cnt; // reserved
    u8 nir_led_flag;
    u8 nir_led_cur_strength;
    u8 nir_led_special_cnt; // reserved
    u8 nir_lm_motion_cnt;
    u8 nir_sensor_setting_cnt;
    u8 step_cnt_wait_led_gain;
    u8 nir_face_quality;
    u16 nir_sensor_search_cnt;
    float effect_2d;
    u8 nir_led_on_tile;
    u8 distance_from_led_on;
    u8 nir_skin_luma;

    float nir_gain;
    float init_nir_gain;
    float nir_lv_cnn_real_score;
    float nir_lv_cnn_face_real_score;
    float rgb_lv_cnn_real_score;
    float fuse_lv_real_score;
    float nir_lv_cnn_diff;
    float rgb_lv_cnn_diff;
    float fuse_lv_diff;
    float nir_luma_ratio;
    float nir_lv_hsn_neck_score;
    float nir_lv_hsn_edge_score;
    u32 face_nomal_dark_cnt;
    u32 face_low_score_cnt;
    s32 lm_diff;
    float rgb_nir_fr_diff;
    float *additional_params_0;
    float *additional_params_1;
    u8  rgb_age_group;
    u8  rgb_avg_luma;    
    float org_rgb_dist;
    float org_nir_dist;
    
    float eye_left_ratio[2];
    float eye_right_ratio[2];
    u8 nir_env_exception_cnt;
    u8 n1r1_sim_status;
    u8 nir_led_status;
    u8 ir_led_flow_ready;
    u8 st_led_flow_ready;
    u8 rgb_fd_status;
    
    u8 rgb_no_fd_nn_cnt;
    float sl_lv_cnn_real_score;    
    
    float id_ref_c;
    float id_dual_fr_diff;

    bool rgb_roi_set;
    u8 led_breathe_en;
    u8 nir_fd_status;
    float str_gain;
    u32 str_exp_time;
    u8 nir_fd_crop_mode_flag;    //not used any more
    u8 str_skin_luma;
    u8 flash_skin_luma;
    u32 nir_dynamic_max_exp_time;
    u8 nir_fps;

    u16 rec_cnt;
    u16 min_fm_num;
    u16 rec_fm_hist[MAX_FID];

#if (AEC_MULTI_THREAD == YES) 
    e2e_rgb_cam_params rgb_cam_params;
    e2e_nir_cam_params nir_cam_params;
#endif

    u8 n1_face_available;
    u8 r1_face_available;

    float rgb_gain;
    
    float fs_feature[17];
    float darkness;
    bool n1_inference_ready;
    bool r1_inference_ready;
    u32  recog_err_not_return;
    cam_image_state rgb_img_ready;
    u8   n1_face_nok_cnt;
    bool occ_for_aec;
    u8   tile_sat_cnt;
    u8   rgb_nir_fr_flag;
    float face_pose_cur_val;
    float face_pose_best_val;
    u8   _is_fm_import;
    u32   norm_variance;
    u16  n1_fd_x;
    u16  n1_fd_y;
    u16  n1_fd_w;
    u16  n1_fd_h;
    float lm_length_ratio[5];
    u8   beard_flag;
} kdp_e2e_face_variables;

typedef struct kdp_e2e_face_model_res_struct {
    struct facedet_result_s  m_r1_fd __attribute__ ((aligned (16)));
    struct facedet_result_s  m_r1_fd_aec __attribute__ ((aligned (16)));
    struct landmark_result_s m_r1_lm __attribute__ ((aligned (16)));
    struct landmark_result_s m_r1_lm_s __attribute__ ((aligned (16)));
    struct landmark_result_s m_r1_lm_aec __attribute__ ((aligned (16)));
    struct fr_result_s       m_r1_fr __attribute__ ((aligned (16)));
    struct fr_result_s       m_r1_fr_flip __attribute__ ((aligned (16)));
    struct facedet_result_s  m_n1_fd __attribute__ ((aligned (16)));
    struct facedet_result_s  m_n1_fd_aec __attribute__ ((aligned (16)));
    struct landmark_result_s m_n1_lm __attribute__ ((aligned (16)));
    struct landmark_result_s m_n1_lm_s __attribute__ ((aligned (16)));
    struct landmark_result_s m_n1_lm_aec __attribute__ ((aligned (16)));
    struct age_group_result_s m_n1_age_group __attribute__ ((aligned (16)));
    struct face_quality_result_s m_face_quality_res __attribute__ ((aligned (16)));
    struct fr_result_s       m_n1_fr __attribute__ ((aligned (16)));
    struct fr_result_s       m_n1_fr_flip __attribute__ ((aligned (16)));
#if EYE_LID_2 == NO
    struct eye_lid_lm_result_s m_eye_lid_res __attribute__ ((aligned (16)));
#else
    struct lv_result_s m_eye_lid_res __attribute__ ((aligned (16)));
#endif
    struct face_occlude_result_s m_face_occ_res __attribute__ ((aligned (16)));

#ifdef FDSSD_ONETBLUR_VGG
    dme_res                  m_od __attribute__ ((aligned (16)));
#endif
    struct lv_result_s       m_lv __attribute__ ((aligned (16)));
	struct dual_landmarks_s  m_dual_lm __attribute__ ((aligned (16)));
	struct dual_landmarks_s  m_dual_lm_2 __attribute__ ((aligned (16)));
	struct nir_camera_tune_s m_nir_tune __attribute__ ((aligned (16)));
} kdp_e2e_face_model_result;


struct facedet_result_s*    kdp_e2e_get_r1_fd(void);
struct landmark_result_s*   kdp_e2e_get_r1_lm(void);
struct landmark_result_s*   kdp_e2e_get_r1_lm_s(void);
struct fr_result_s*         kdp_e2e_get_r1_fr(void);

struct facedet_result_s*    kdp_e2e_get_n1_fd(void);
struct landmark_result_s*   kdp_e2e_get_n1_lm(void);
struct landmark_result_s*   kdp_e2e_get_n1_lm_s(void);
struct fr_result_s*         kdp_e2e_get_n1_fr(void);

struct lv_result_s*         kdp_e2e_get_r1n1_lv(void);
struct dual_landmarks_s*    kdp_e2e_get_r1n1_dlm(void);
struct dual_landmarks_s*    kdp_e2e_get_r1n1_dlm_2(void);
struct nir_camera_tune_s*   kdp_e2e_get_r1n1_tune(void);
kdp_e2e_face_variables*     kdp_e2e_get_face_variables(void);
struct face_occlude_result_s*    kdp_e2e_get_r1n1_occ(void);
struct face_quality_result_s*      kdp_e2e_get_face_quality(void);

void                        kdp_e2e_rst_r1_fd(void);
void                        kdp_e2e_rst_r1_lm(void);
void                        kdp_e2e_rst_r1_fr(void);
void                        kdp_e2e_rst_n1_fd(void);
void                        kdp_e2e_rst_n1_lm(void);
void                        kdp_e2e_rst_n1_fr(void);


s32 kdp_e2e_face_n1_fd_lm_combo(
    struct kdp_img_cfg *img_cfg,
    struct facedet_result_s *fd,
    struct landmark_result_s *lm_out);

void kdp_e2e_face_rst_variables(void);
void kdp_e2e_face_get_frame_info(int cam_idx, struct frame_info *info);
void kdp_e2e_face_switch_board_params(int , int );
///s32 kdp_e2e_face_r1_get_bbox(int *x, int *y, int *w,int *h);
//s32 kdp_e2e_face_n1_get_bbox(int *x, int *y, int *w,int *h);
s32 kdp_e2e_face_init(void);
BOOL kdp_e2e_face_init_done(void);
s32 kdp_e2e_face_r1n1_preproc(void);
s32 kdp_e2e_face_n1r1_preproc(void);

s32 kdp_e2e_face_r1(void);
s32 kdp_e2e_face_n1(void);
s32 kdp_e2e_face_r1n1_postproc(void);
s32 kdp_e2e_face_n1r1_postproc(int flip_face);
s32 kdp_e2e_face_r1_only_postproc(void);
s32 kdp_e2e_face_n1_only_postproc(void);
s32 kdp_e2e_face_cat_eye(void);

s32 kdp_e2e_palm_preproc(void);
s32 kdp_e2e_palm_proc(void);
s32 kdp_e2e_palm_post_proc(void);

s32 kdp_e2e_face_n1_fd_lm(struct kdp_img_cfg *nir_img_cfg, struct facedet_result_s *n1_fd, struct landmark_result_s *n1_lm);
s32 kdp_e2e_face_r1_fd_lm(struct kdp_img_cfg *rgb_img_cfg, struct facedet_result_s *r1_fd, struct landmark_result_s *r1_lm);
s32 kdp_e2e_face_n1_quality(struct kdp_img_cfg *nir_img_cfg, kdp_e2e_prop* prop, kdp_e2e_face_variables *vars, struct facedet_result_s *n1_fd, struct landmark_result_s *n1_lm, struct landmark_result_s *n1_lm_s);
s32 kdp_e2e_face_r1_quality(struct kdp_img_cfg *rgb_img_cfg, kdp_e2e_prop* prop, kdp_e2e_face_variables *vars, struct facedet_result_s *r1_fd, struct landmark_result_s *r1_lm, struct landmark_result_s *r1_lm_s);
s32 kdp_e2e_n1_r1_interactive_check(void);

/**
 * @brief To config image setting
 *
 * @param [in] src_index src img index (no need to align with cam src)
 * @param [in] col column size
 * @param [in] row row size
 * @param [in] ch channel size
 * @param [in] fmt propietory image format
 * @param [in] mem_addr image DDR address
 * @param [in] img_buf_idx raw image buffer index
 */
void kdp_e2e_config_image(int src_idx, s32 col, s32 row, s32 ch,
                          u32 fmt, u32 addr, int img_buf_idx);

struct kdp_img_cfg* kdp_e2e_get_img_cfg(int img_src);
/**
 * @brief to update image memory address
 *
 * !!! MUST be called after kdp_app_config_image()
 * @param [in] cam_index cam0 or cam1
 * @param [in] image_memory_address new image memory address
 * @return n/a
 */
void kdp_e2e_set_img_mem_addr(int img_src_idx, s32 img_mem_addr);
/**
 * @brief output image memory address of specified source
 * @param [in] camera image source (RGB or NIR)
 * @return image memory address being used currently
 */
uint32_t kdp_e2e_get_img_mem_addr(int img_src_idx);

uint32_t kdp_e2e_get_img_mem_len(int img_src_idx);

#if ( CFG_PALM_PRINT_MODE == YES )
void kdp_e2e_bg_init_done( BOOL flag );
#endif
#endif
