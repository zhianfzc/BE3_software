/*
 * Kneron Application FD/FR APIs
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef __KDP_APP_FID_H_
#define __KDP_APP_FID_H_

#include <stdint.h>
#include "types.h"
#include "ipc.h"
#include "kdp_app.h"

// To use new FDR. If using old FDR (FDSmallbox+Onet+VGG10), comment this Macro
#define FDSSD_ONETBLUR_VGG

/* ==================
 *  FID application
 *      kdpAppFID.c
 * ================== */
//enum kapp_fid_status_code_e {
//    KAPP_FDR_WRONG_USAGE = 0x100,    ///== KDP_APP_FID_CODES
//    KAPP_FDR_NO_FACE,                /// error: no face
//    KAPP_FDR_BAD_POSE,               /// error: base face pose
//    KAPP_FDR_NO_ALIVE,               /// error: not alive
//    KAPP_FDR_FR_FAIL,                /// error: FR failed
//    KAPP_LM_LOW_CONFIDENT,           /// error: LM low confident
//    KAPP_LM_BLUR,                    /// error: LM blur
//    KAPP_SMALL_FACE,                 /// error: face too small
//    KAPP_RGB_Y_DARK
//};

//enum kapp_fid_op_e{
//    KAPP_FID_OP_EXTRACT,             /// extract feature map only
//    KAPP_FID_OP_INFERENCE,           /// do inference (do DB comparison)
//    KAPP_FID_OP_REGISTER             /// do register (insert DB temp data)
//};

// data structure
//---------------------------------
//typedef struct kdp_app_fid_inout_s {
//    enum kapp_fid_op_e op;               ///operation mode
//    uint16_t face_id;                       //1-5: faceId (1-5)
//    uint16_t user_id;                       ///as input: user id got from Host
//                                            ///as output: user id returned by KL520
//    float thresh_fid[KAPP_IMG_SRC_MAX];  ///threshold value for face recognition
//} kdp_app_fid_inout_t;

/** @brief face detectin result
 *
 * facedet_result_s is defined in ipc.h
 * */
//typedef struct facedet_result_s kdp_app_face_det_res_t;

/** @brief landmark result
 *
 * landmark_result_s is defined in ipc.h
 * */
//typedef struct landmark_result_s kdp_app_face_lm_res_t;

/** @brief face recognition result
 *
 * fr_result_s defined in ipc.h
 * */
//typedef struct fr_result_s kdp_app_face_fmap_t;

//typedef struct kdp_app_image_buffer_desc_s {
//   uint32_t buf_count;                  ///image buffer count
//   uint32_t alignment_in_bit;           ///alignment size in bit
//   uint32_t rgb_buf_base_addr;          ///rgb image buffer address
//   uint32_t rgb_img_size;               ///rgb image size
//   uint32_t nir_buf_base_addr;          ///nir image buffer address
//   uint32_t nir_img_size;               ///nir image size
//} kdp_app_image_buffer_desc_t;

/**
 * @brief do face detection
 * @param [out] p_out_p: detected face bounding box
 * @param [in] img_cfg img config
 * @return OK or Err code
 */
//int32_t kdp_app_fd(struct facedet_result_s *p_out_p/*out*/,
//                   struct kdp_img_cfg *img_cfg);


/**
 * @brief do face landmark
 * @param [out] p_out_p detected face landmarks
 * @param [in] p_fd_out_p detected face bounding box
 * @param [in] img_cfg img config
 * @return OK or Err code
 */
//int32_t kdp_app_lm(struct landmark_result_s *p_out_p/*out*/,
//                   struct facedet_result_s *p_fd_out_p,
//                   struct kdp_img_cfg *img_cfg);

/**
 * @brief do nir liveness
 * @param [out] p_out_p detected nir liveness
 * @param [in] p_lm_out_p detected face landmarks
 * @param [in] img_cfg img config
 * @return OK or Err code
 */
int32_t kdp_app_nir_lv(struct lv_result_s *p_out_p/*out*/,
                   //struct facedet_result_s  *p_fd_out_p,
                   struct landmark_result_s   *p_lm_out_p,
                   struct kdp_img_cfg *p_nir_img_setting_p);
                   //int img_src);

int32_t kdp_app_nir_eye_lv(struct lv_result_s *p_out_p/*out*/,
                   //struct facedet_result_s  *p_fd_out_p,
                   struct landmark_result_s   *p_lm_out_p,
                   struct kdp_img_cfg *p_nir_img_setting_p);

int32_t kdp_app_fuse_nir_lv(struct lv_result_s *p_out_p/*out*/,
                   //struct facedet_result_s  *p_fd_out_p,
                   struct landmark_result_s   *p_lm_out_p,
                   struct kdp_img_cfg *p_nir_img_setting_p);
                   //int img_src);
/**
 * @brief do nir liveness by head shoulder and neck
 * @param [out] p_out_p detected nir liveness
 * @param [in] p_lm_out_p detected face landmarks
 * @param [in] img_cfg img config
 * @return OK or Err code
 */
int32_t kdp_app_nir_hsn_lv(struct lv_result_s *p_out_p/*out*/,
                   //struct facedet_result_s  *p_fd_out_p,
                   struct landmark_result_s   *p_lm_out_p,
                   struct kdp_img_cfg *p_nir_img_setting_p);
                   //int img_src);
                   
/**
 * @brief do nir structure light liveness
 * @param [out] p_out_p detected nir liveness
 * @param [in] p_lm_out_p detected face landmarks
 * @param [in] img_cfg img config
 * @return OK or Err code
 */
//int32_t kdp_app_nir_sl_lv(struct lv_result_s *p_out_p/*out*/,
//                   struct facedet_result_s  *p_nir_fd,
//                   struct landmark_result_s   *p_lm_out_p,
//                   struct kdp_img_cfg *p_nir_img_setting_p);              
                   
/**
 * @brief do rgb liveness
 * @param [out] p_out_p detected rgb liveness
 * @param [in] p_lm_out_p detected face landmarks
 * @param [in] img_cfg img config
 * @return OK or Err code
 */
//int32_t kdp_app_rgb_lv(struct lv_result_s *p_out_p/*out*/,
//                   //struct facedet_result_s  *p_fd_out_p,
//                   struct landmark_result_s   *p_lm_out_p,
//                   struct kdp_img_cfg *p_rgb_img_setting_p);
//                   //int img_src);

/**
 * @brief extract face embedding
 * @param [out] p_out_p extracted face embedding
 * @param [in] p_fd_out_p detected face bounding box
 * @param [in] p_lm_out_p detected face landmarks
 * @param [in] img_cfg img config
 * @return OK or Err code
 */
//int32_t kdp_app_fr(struct fr_result_s *p_out_p/*out*/,
//                   struct facedet_result_s  *p_fd_out_p,
//                   struct landmark_result_s   *p_lm_out_p,
//                   struct kdp_img_cfg *img_cfg);


//int32_t kdp_model_age_group(struct age_group_result_s *age, /*output*/
//                     struct kdp_img_cfg *img_cfg,
//                     struct facedet_result_s *fd_bbox);

/**
 * @brief init necessary data, face data base
 * @param[in]  reserve_db_num : reserve more user_db unit in ddr
 * @return OK
 */
//int32_t kdp_app_init( uint16_t reserve_db_num );


/**
 * @brief (Beta) get 5 pose score for a landmark set
 * @param[out] pose : 5 pose score
 * @param[in]  landmark : 5 points landmark, 10 int value
 */
//void kdp_app_get_pose_value(uint32_t* landmarks, float *poses);


/**
 * @brief (Beta) check if it is a bad pose for a landmark set
 * @param[in]  landmark : 5 points landmark, 10 int value
 * @return True for bad pose, False otherwise
 */
//bool kdp_app_check_bad_pose(uint32_t *landmarks);

//int kdp_app_check_head_pose(struct landmark_result_s* landmarks,
//                            uint8_t reg_idx, uint8_t force_flag,
//                            uint8_t* face_reg_thresh);

//int kdp_app_rgb_occlusion_lv(struct kdp_img_cfg *img_cfg,
//                     struct facedet_result_s *fd_bbox,
//                     struct landmark_result_s   *p_lm,
//                     struct face_occlude_result_s *out, int model_type);  // output)

int kdp_app_nir_face_seg_mask(struct kdp_img_cfg *img_cfg,
                     struct facedet_result_s *fd_bbox,
                     struct landmark_result_s   *p_lm,
                     struct face_occlude_result_s *out, int model_type);  // output)
										 
int32_t kdp_model_eye_lid_2(struct landmark_result_s *lm, /*output*/
                     struct kdp_img_cfg *img_cfg,
                     struct lv_result_s *eye_lid,
                     int e_id);										 
/**
 * @brief (Beta) check if it is a good normal face for a landmark set
 * @param[in]  landmark : 5 points landmark, 10 int value
 * @param[in]  thresh : angle ratio threshold
 * @return True for good normal face
 */
//bool kdp_app_check_angle_ratio(uint32_t *landmarks, float thresh);
//int32_t kapp_face_seg(float output_seg_map[144],float landmark[10], float score[5], struct kdp_img_cfg *img_cfg);
//#define kapp_fid_inout_t kdp_app_fid_inout_t
//#define kapp_init kdp_app_init
//#define kapp_image_buffer_desc_t kdp_app_image_buffer_desc_t
//#define kapp_fdr kdp_app_fdr
//#define kapp_lw3d kdp_app_lw3d

#endif
