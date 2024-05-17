#ifndef __KDP_E2E_CAMERA_H__
#define __KDP_E2E_CAMERA_H__


#include "board_kl520.h"
#include "types.h"
#include "ipc.h"
#include "kdp_fb_mgr.h"
#include "kdp_e2e_n1_only.h"

#define AEC_MULTI_THREAD            (NO)  //       Multithreading

#if (IMGSRC_0_TYPE == SENSOR_TYPE_SC132GS) || (IMGSRC_1_TYPE == SENSOR_TYPE_SC132GS)
#define MAX_DEFAULT_NIR_EXP_TIME    (26864)
#define NIR_DEFAULT_GAIN            (3.0f)
#define NIR_DEFAULT_FPS             (40)
#define MIN_DEFAULT_NIR_EXP_TIME    (256)
#define MAX_GAIN                    (15.0f)
#define MIN_GAIN                    (1.0f)
#define NIR_LED_WAIT_FRAME          (4)
#define RGB_LED_WAIT_FRAME          (4)
#elif (IMGSRC_0_TYPE == SENSOR_TYPE_SC035HGS) || (IMGSRC_1_TYPE == SENSOR_TYPE_SC035HGS)
#define MAX_DEFAULT_NIR_EXP_TIME    (34500)
#define NIR_DEFAULT_GAIN            (4.0f)
#define NIR_DEFAULT_FPS             (20)
#define MIN_DEFAULT_NIR_EXP_TIME    (256)
#define MAX_GAIN                    (15.0f)
#define MIN_GAIN                    (1.0f)
#define NIR_LED_WAIT_FRAME          (4)
#define RGB_LED_WAIT_FRAME          (4)
#elif (IMGSRC_1_TYPE == SENSOR_TYPE_GC1054_R) || (IMGSRC_1_TYPE == SENSOR_TYPE_GC1054_L) || (IMGSRC_0_TYPE == SENSOR_TYPE_GC1054_R) || (IMGSRC_0_TYPE == SENSOR_TYPE_GC1054_L)
#define MAX_DEFAULT_NIR_EXP_TIME    (678)
#define MIN_DEFAULT_NIR_EXP_TIME    (30)
#define NIR_DEFAULT_GAIN            (500.0f)
#define NIR_DEFAULT_FPS             (30)
#define MAX_GAIN                    (2084.0f)
#define MIN_GAIN                    (67.0f)
#define NIR_LED_WAIT_FRAME          (3)
#define RGB_LED_WAIT_FRAME          (3)
#elif (IMGSRC_0_TYPE == SENSOR_TYPE_OV02B1B_R) || (IMGSRC_1_TYPE == SENSOR_TYPE_OV02B1B_L)
#define MAX_DEFAULT_NIR_EXP_TIME    (1800)
#define MIN_DEFAULT_NIR_EXP_TIME    (10)
#define NIR_DEFAULT_GAIN            (100.0f)
#define NIR_DEFAULT_FPS             (25)
#define MAX_GAIN                    (250.0f)    //for face
#define MIN_GAIN                    (15.0f)
#define NIR_LED_WAIT_FRAME          (3)
#define RGB_LED_WAIT_FRAME          (3)
#if ( CFG_PALM_PRINT_MODE == YES )
#define PALM_DEFAULT_EXP_TIME       (100)
#define PALM_DEFAULT_GAIN           (15.0f)     //for palm print
#define PALM_MAX_GAIN               (35.0f)     //for palm print
#endif
#elif (IMGSRC_0_TYPE == SENSOR_TYPE_SP2509_R) || (IMGSRC_1_TYPE == SENSOR_TYPE_SP2509_L)
#define MAX_DEFAULT_NIR_EXP_TIME    (430)
#define MIN_DEFAULT_NIR_EXP_TIME    (2)
#define NIR_DEFAULT_GAIN            (100.0f)
#define NIR_DEFAULT_FPS             (30)
#define MAX_GAIN                    (220.0f)
#define MIN_GAIN                    (15.0f)
#define NIR_LED_WAIT_FRAME          (3)
#define RGB_LED_WAIT_FRAME          (3)
#else
#define MAX_DEFAULT_NIR_EXP_TIME    (26864)
#define NIR_DEFAULT_GAIN            (3.0f)
#define NIR_DEFAULT_FPS             (40)
#define MIN_DEFAULT_NIR_EXP_TIME    (256)
#define MAX_GAIN                    (15.0f)
#define MIN_GAIN                    (1.0f)
#define NIR_LED_WAIT_FRAME          (4)
#define RGB_LED_WAIT_FRAME          (4)
#endif

#if ( CFG_PALM_PRINT_MODE == YES )
#define BRIGHTNESS_TARGET           ( 150 )
#define BRIGHTNESS_TARGET_RANGE     ( 10 )
#endif

#if (CFG_SENSOR_0_TYPE == SENSOR_TYPE_GC02M1_R || CFG_SENSOR_0_TYPE == SENSOR_TYPE_GC02M1_L) || \
    (CFG_SENSOR_1_TYPE == SENSOR_TYPE_GC02M1_R || CFG_SENSOR_1_TYPE == SENSOR_TYPE_GC02M1_L) || \
    (CFG_SENSOR_TYPE == SENSOR_TYPE_GC2145_GC1054)
#define MAX_DEFAULT_RGB_EXP_TIME    (638)
#define RGB_DEFAULT_GAIN            (500.0f)
#define RGB_DEFAULT_FPS             (30)
#define MIN_DEFAULT_RGB_EXP_TIME    (20)
#define MAX_RGB_GAIN                (1000.0f)
#define MIN_RGB_GAIN                (67.0f)
#define RGB_CATEYE_WAIT_FRAME          (4)
#endif

#if (E2E_N1_ONLY_STRUCTURE_ENABLE == YES)
    #define STR_550A                    (YES)
    #if (STR_550A == YES)
    #define MAX_STR_EXP_TIME            (10800)
    #else
    #define MAX_STR_EXP_TIME            (3230)    
    #endif
#else
    #define MAX_STR_EXP_TIME            (MAX_DEFAULT_NIR_EXP_TIME)
#endif
#define RGB_LED_DEFAULT_DIM_ENV_STRENGTH  (CFG_E2E_RGB_LED_DEFAULT_DIM_ENV_STRENGTH)
#define RGB_LED_DEFAULT_DARK_ENV_STRENGTH       (CFG_E2E_RGB_LED_DEFAULT_DARK_ENV_STRENGTH)
#define DEFAULT_RGB_LED_STRENGTH    (CFG_E2E_RGB_LED_STRENGTH)
#define DEFAULT_NIR_LED_STRENGTH    (CFG_E2E_NIR_LED_STRENGTH)

typedef enum CAM_IMAGE_STATE_e
{
    CAM_IMAGE_STATE_NULL = 0,
    CAM_IMAGE_STATE_ISR,
    CAM_IMAGE_STATE_RUN,  
} cam_image_state;


void an_nir_gain_setting(float *nir_gain);
void an_rgb_gain_setting(float *rgb_gain);

s32 nir_continuous_gain_tune(int WIDTH, uint8_t *Img, int lm_type, int aec_type);

void nir_waiting_led_gain_ready(int WIDTH, uint8_t *Img);

void nir_reduce_exposure(int WIDTH, uint8_t *Img);
    
void nir_predict_gain_led_instantly(int WIDTH, uint8_t *Img, float *nir_gain, int eye_range);

void Surrond(int WIDTH, uint8_t *Img, u8 tile_env);

void camera_sensor_rst(void);

bool nir_sensor_delay_fdfr(void);
void nir_sensor_wait_effect(void);
void nir_sensor_rst(u8 wait_cnt);

#define DUAL_IDENTICAL_CAM_CLOSED    0
#define DUAL_IDENTICAL_CAM_INITED    1
#define DUAL_IDENTICAL_CAM_AEC       2
#define DUAL_IDENTICAL_CAM_RUN       3
#define DUAL_IDENTICAL_CAM_SLEEP     4

#if CFG_ONE_SHOT_MODE == YES
s32 kdp_e2e_set_dual_cam_state(u8 cam_sts);
u8  kdp_e2e_get_dual_cam_state(void);
#else
#define kdp_e2e_set_dual_cam_state(cam_sts) //
#define kdp_e2e_get_dual_cam_state(void) //
#endif

bool rgb_sensor_delay_fdfr(void);
void rgb_sensor_wait_effect(void);
void rgb_sensor_rst(u8 wait_cnt);

u32 kdp_e2e_rgb_aec_clip_bound(void);
float kdp_e2e_rgb_gain_clip_bound(void);

u32 kdp_e2e_nir_aec_clip_bound(void);
float kdp_e2e_nir_gain_clip_bound(void);

u32 kdp_e2e_str_aec_clip_bound(void);
float kdp_e2e_str_gain_clip_bound(void);


//bool kdp_e2e_cam_delay_fdfr(u8 cam_idx);
//void kdp_e2e_cam_wait_effect(u8 cam_idx);
//void kdp_e2e_cam_rst(u8 cam_idx, u8 wait_cnt);

#if ((CFG_AI_TYPE == AI_TYPE_N1)|| (CFG_AI_TYPE == AI_TYPE_N1R1))
void nir_aec_tune_for_fd_search(void);
s32 nir_only_surrond(int nir_width, uint8_t *Img, u8 tile_env);
#endif

#if ( CFG_PALM_PRINT_MODE == YES )
s32 nir_palm_print_continuous_aec(int nir_width, uint8_t *Img, u8 tile_env);
#endif

#if (AEC_MULTI_THREAD == YES)
s32 kdp_e2e_n1_cam_block_check(e2e_nir_cam_params* nir_cam_param, struct frame_info* info);
void kdp_e2e_n1_led_off_proc(e2e_nir_cam_params* nir_cam_params, struct frame_info* info);
void kdp_e2e_n1_led_on_preproc(e2e_nir_cam_params* nir_cam_params, struct frame_info* info);
void n1_aec_fd_search(e2e_nir_cam_params* nir_cam_params, struct frame_info* info);
void lock_vars_n1_aec(e2e_nir_cam_params* nir_cam_params);
void unlock_vars_n1_aec(e2e_nir_cam_params* nir_cam_params);
void kdp_nir_aec_thread(u32 buf_addr);
void kdp_rgb_aec_thread(u32 buf_addr);
void lock_vars_r1_aec(e2e_rgb_cam_params* rgb_cam_params);
void unlock_vars_r1_aec(e2e_rgb_cam_params* rgb_cam_params);
void get_r1_face_region_luma(int32_t WIDTH, uint16_t *pImageRgb, struct landmark_result_s *rgb_p_lm, u8 *Y_left, u8 *Y_right, u8 *back_light_flag);
#else
void kdp_e2e_camera_r1_set_aec_roi_preproc(void);
void kdp_e2e_camera_r1_set_aec_roi(struct facedet_result_s *r1_fd);
void kdp_e2e_camera_r1_get_lux(u16 *lp_rgb_exp_time, u8 *lp_rgb_dark_mode);
#endif

void aec_e2e_multiply_factor_set(float luma_target, float luma_curr);
void kdp_e2e_camera_r1_set_aec_roi_n1r1(struct facedet_result_s *r1_fd);
void kdp_e2e_camera_r1_set_aec_roi_from_nir(struct facedet_result_s *n1_fd, struct landmark_result_s *n1_lm);

void kdp_e2e_aec_rgb_sync_to_nir_aec(void);
void cateye_rgb_aec_process(int nir_width, uint8_t *nir_img, struct facedet_result_s *fd, u8 luma_tgt);
u8 luma_avg_roi(int nir_width, uint8_t *nir_img, struct facedet_result_s *fd);
#endif


#if (AEC_MULTI_THREAD == YES)
typedef struct kl520_camera_param_rgb_struct
{
    u8 status;
    u16 rgb_init_exp_time;
    u16 rgb_cur_exp_time;
    u8  rgb_dark_mode;
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
    u8 rgb_avg_luma;
    bool rgb_roi_set;
    u8 led_breathe_en;
    bool is_lock; 
}e2e_rgb_cam_params;

typedef struct kl520_camera_param_nir_struct
{
    u8 status;
    u16 nir_cur_exp_time;
    u8 nir_mode;
    //u8 nir_mode;
    u8 nir_mode_cnt; // reserved
    u8 nir_led_flag;
    u8 nir_led_cur_strength;
    u8 nir_led_special_cnt; // reserved
    u8 nir_sensor_setting_cnt;
    u8 step_cnt_wait_led_gain;
    u8 nir_sensor_search_cnt;
    float nir_gain;
    float init_nir_gain;
    u8 nir_led_status;
    u8 nir_skin_luma;
    u8 nir_env_exception_cnt;
    bool is_lock; 
}e2e_nir_cam_params;
#endif
