#ifndef __KDP_E2E_UTIL_H__
#define __KDP_E2E_UTIL_H__

#include "kdp_app_fid.h"
#include "kdp_e2e_db.h"
#include "kdp_e2e_face.h"
#define LL_LUMA                             (NO)
#define BOUNDINGBOX_CHECK_LM                (YES)
#if CFG_AI_TYPE == AI_TYPE_N1 || CFG_LW3D_TYPE != CFG_LW3D_NORMAL
#define NORMAL_FACE_ANGLE_CHECK             (NO)
#else
#define NORMAL_FACE_ANGLE_CHECK             (YES)
#endif
#if  BOUNDINGBOX_CHECK_LM == NO
#define BOUNDINGBOX_CHECK_CENTER
#endif

#define FACE_MOVE_X_THRESH      (40)//30)
#define FACE_MOVE_Y_THRESH      (40)//30)
#define FACE_NOT_MOVE_CNT       (3)
#if (IMGSRC_0_TYPE == SENSOR_TYPE_SC035HGS) || (IMGSRC_1_TYPE == SENSOR_TYPE_SC035HGS)
#define FACE_DIST_MAX_THRESH    (80)
#else
#if CFG_CAMERA_DUAL_1054 == 1 || CFG_CAMERA_SINGLE_1054 == 1
#define FACE_DIST_MAX_THRESH    (120)
#else
#define FACE_DIST_MAX_THRESH    (80)
#endif
#endif
#define FACE_DIST_MIN_THRESH    (10)
#ifndef USE_XGB_DYNAMIC_THRESH
#define USE_XGB_DYNAMIC_THRESH  (NO)
#endif

#if BOUNDINGBOX_CHECK_LM == YES
s32 kdp_e2e_util_chk_r1_lm(
    struct kdp_img_cfg *img_cfg,
    struct landmark_result_s *r1_lm);
s32 kdp_e2e_util_chk_n1_lm(
    struct landmark_result_s *n1_lm);    
#endif

//int abs_n(int a);

s16 kdp_e2e_get_avg_eye_range(struct landmark_result_s *lm);    
    
void kdp_e2e_get_mapping_bbox_from_nir(int xywh[4], float coe);
    
//s32 kdp_e2e_util_calc_ratio(float curr_luma_ratio, float db_luma_ratio, float cur_luma_ref, float db_luma_ref);

//s32 kdp_e2e_util_calc_ref_range(float id_dual_fr_diff);

s32 kdp_e2e_util_r1_chk_bbox(struct facedet_result_s *r1_fd);

s32 kdp_e2e_util_chk_position(struct landmark_result_s *lm, s32 width, s32 height, s32 offset_x, s32 offset_y, s32 e2e_cam );

float kdp_e2e_iou_fd_by_mapping(struct facedet_result_s *r1_fd, struct landmark_result_s *r1_lm);

s32 kdp_e2e_util_chk_bbox_by_fd(struct facedet_result_s *r1_fd, struct facedet_result_s *n1_fd);

s32 kdp_e2e_util_n1_chk_bbox_by_lm(struct landmark_result_s *r1_lm, struct landmark_result_s *n1_lm);

s32 kdp_e2e_util_r1_chk_bad_pose(struct landmark_result_s *r1_lm);

s32 kdp_e2e_util_r1_chk_mask(struct facedet_result_s *r1_fd);
s32 kdp_e2e_util_n1_chk_mask(struct facedet_result_s *n1_fd);

s32 kdp_e2e_util_n1_chk_bad_pose(struct landmark_result_s *n1_lm);

//s32 kdp_e2e_util_chk_head_pose(u8 reg_idx, u8 force_flag, u8* face_reg_thresh, s8 *lp_pose_val);

//void kdp_e2e_util_measure_camera_distance(struct landmark_result_s *r1_lm, struct landmark_result_s *n1_lm);

u16 kdp_e2e_util_get_person_to_camera_distance(void);

int kdp_e2e_util_get_person_position(s32 *x, s32 *y, s32 *w, s32 *h);

int kdp_e2e_util_get_eye_range(struct landmark_result_s *lm);
int kdp_e2e_util_uint_subtract_abs(uint32_t a, uint32_t b, int abs_flag);

void kdp_e2e_util_calc_offset_from_nir_rgb_xy(
    float* registered_offset_x, float* registered_offset_y, float nir_nose_x, float nir_nose_y, 
    float rgb_x, float rgb_y);

void kdp_e2e_util_predict_xy_from_offset_xy(
    float registered_offset_x, float registered_offset_y, float* predicted_x, float* predicted_y, 
    float lm_x, float lm_y, int eye_range);

float kdp_e2e_util_calc_predict_after_led_y(float rgb_eye_range, float init_exp_tile);
float kdp_e2e_eye_open_ratio(struct eye_lid_lm_result_s* p_lms);

int kdp_e2e_check_face_move(struct facedet_result_s* fd);
//s32 kdp_e2e_util_chk_bad_pose(struct landmark_result_s *p_lm, u32 check_hp_type);

s32 kdp_e2e_util_face_add_pose_chk(float yaw, float pitch, float roll, float normal_score, u32 check_hp_type);
s32 kdp_e2e_util_face_recog_pose_chk(float yaw, float pitch, float roll);

int kdp_e2e_check_face_position(void);
    
//void kdp_e2e_face_status_get(int32_t width, uint8_t *nir_img, struct landmark_result_s *lm, float *get_feat);  
//float kdp_get_e2e_dynamic_th(kdp_e2e_db_extra_data *vars_db, kdp_e2e_face_variables *vars_cur);    
#endif
