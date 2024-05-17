#ifndef __KDP_E2E_H__
#define __KDP_E2E_H__


#define E2E_OK                      (0x00000000)
#define E2E_ERROR                   (0x80000000)
#define E2E_ERROR_N1_LUMA_RATIO     (0x40000000)
#define E2E_STATUS                  (0x20000000)
#define E2E_INVALID_CAL             (0x04000000)
#define E2E_INVALID                 (0x02000000)
#define E2E_ENVIR                   (0x01000000)
#define E2E_R1                      (0x00100000)
#define E2E_N1                      (0x00200000)
//#define E2E_N2                      (0x00400000)
#define E2E_RXNX                    (0x00800000)
#define E2E_FD                      (0x00001000)
#define E2E_LM                      (0x00002000)
#define E2E_FR                      (0x00004000)
#define E2E_CV_LV                   (0x00008000)
#define E2E_R1_LV                   (0x00010000)
#define E2E_N1_LV                   (0x00020000)
#define E2E_N1_HSN_LV               (0x00030000)
#define E2E_FU_LV                   (0x00040000)
#define E2E_N1_EYE_LV               (0x00050000)
#define E2E_AGE_GROUP               (0x00080000)
#define E2E_N1_S_LV                 (0x000F0000)
#define E2E_MODEL                   (0x00000100)
#define E2E_POSTPROC                (0x00000200)
#define E2E_BBOX                    (0x00000400)
#define E2E_DIFF                    (0x00000800)
#define E2E_MOTION_CHK              (0x00000010)
#define E2E_BAD_POSE                (0x00000020)
#define E2E_FACE_MASK               (0x00000040)
#define E2E_EYE_CLOSED              (0x00000080)
#define E2E_FACE_QUALTY             (0x00000004)
#define E2E_FACE_QUALTY_2           (0x00000008)

#define E2E_FACE_TOO_NEAR           (0x00000001)
#define E2E_FACE_TOO_FAR            (0x00000002)
#define E2E_FACE_TOO_UP             (0x00000003)
#define E2E_FACE_TOO_DOWN           (0x00000004)
#define E2E_FACE_TOO_LEFT           (0x00000005)
#define E2E_FACE_TOO_RIGHT          (0x00000006)
#define E2E_VERY_SPECIAL_CODE       (0x00ff00ff)

#define E2E_ERROR_DEGREE_MASK       (0x0000000F)
#define E2E_ERROR_DEGREE_LEVEL0     (0x00000000)
#define E2E_ERROR_DEGREE_LEVEL1     (0x00000001)
#define E2E_ERROR_DEGREE_LEVEL2     (0x00000002)


#define E2E_ERROR_FD_MODEL          (E2E_ERROR | E2E_FD     | E2E_MODEL)
#define E2E_ERROR_FD_POSTPROC       (E2E_ERROR | E2E_FD     | E2E_POSTPROC)
#define E2E_ERROR_FD_BBOX           (E2E_ERROR | E2E_FD     | E2E_BBOX)
#define E2E_ERROR_FD_FACE_MASK      (E2E_ERROR | E2E_FD     | E2E_FACE_MASK)
#define E2E_ERROR_LM_MODEL          (E2E_ERROR | E2E_LM     | E2E_MODEL)
#define E2E_ERROR_LM_POSTPROC       (E2E_ERROR | E2E_LM     | E2E_POSTPROC)
#define E2E_ERROR_LM_MOTION_CHK     (E2E_ERROR | E2E_LM     | E2E_MOTION_CHK)
#define E2E_ERROR_LM_BAD_POSE       (E2E_ERROR | E2E_LM     | E2E_BAD_POSE)
#define E2E_ERROR_LM_BBOX           (E2E_ERROR | E2E_LM     | E2E_BBOX)
#define E2E_ERROR_EYE_CLOSED        (E2E_ERROR | E2E_LM     | E2E_EYE_CLOSED)
#define E2E_ERROR_FR_MODEL          (E2E_ERROR | E2E_FR     | E2E_MODEL)
#define E2E_ERROR_FR_POSTPROC       (E2E_ERROR | E2E_FR     | E2E_POSTPROC)
#define E2E_ERROR_CV_LV_MODEL       (E2E_ERROR | E2E_CV_LV  | E2E_MODEL)
#define E2E_ERROR_R1_LV_MODEL       (E2E_ERROR | E2E_R1_LV  | E2E_MODEL)
#define E2E_ERROR_N1_LV_MODEL       (E2E_ERROR | E2E_N1_LV  | E2E_MODEL)
#define E2E_ERROR_N1_HSN_LV_MODEL   (E2E_ERROR | E2E_N1_HSN_LV  | E2E_MODEL)
#define E2E_ERROR_N1_EYE_LV_MODEL   (E2E_ERROR | E2E_N1_EYE_LV  | E2E_MODEL)
#define E2E_ERROR_FU_LV_MODEL       (E2E_ERROR | E2E_FU_LV  | E2E_MODEL)
#define E2E_ERROR_AGE_MODEL         (E2E_ERROR | E2E_AGE_GROUP  | E2E_MODEL)
#define E2E_ERROR_N1_S_LV_MODEL     (E2E_ERROR | E2E_N1_S_LV  | E2E_MODEL)


#define E2E_ERROR_R1_FD_MODEL       (E2E_R1 | E2E_ERROR_FD_MODEL)
#define E2E_ERROR_R1_FD_POSTPROC    (E2E_R1 | E2E_ERROR_FD_POSTPROC)
#define E2E_ERROR_R1_FD_BBOX        (E2E_R1 | E2E_ERROR_FD_BBOX)
#define E2E_ERROR_R1_LM_MODEL       (E2E_R1 | E2E_ERROR_LM_MODEL)
#define E2E_ERROR_R1_LM_POSTPROC    (E2E_R1 | E2E_ERROR_LM_POSTPROC)
#define E2E_ERROR_R1_LM_MOTION_CHK  (E2E_R1 | E2E_ERROR_LM_MOTION_CHK)
#define E2E_ERROR_R1_LM_BAD_POSE    (E2E_R1 | E2E_ERROR_LM_BAD_POSE)
#define E2E_ERROR_R1_LM_BBOX        (E2E_R1 | E2E_ERROR_LM_BBOX)
#define E2E_ERROR_R1_FR_MODEL       (E2E_R1 | E2E_ERROR_FR_MODEL)
#define E2E_ERROR_R1_FR_POSTPROC    (E2E_R1 | E2E_ERROR_FR_POSTPROC)
#define E2E_ERROR_R1_FACE_MASK      (E2E_R1 | E2E_ERROR_FD_FACE_MASK)
#define E2E_ERROR_R1_EYE_CLOSED     (E2E_R1 | E2E_ERROR_EYE_CLOSED)
#define E2E_ERROR_R1_FACE_QUALTY    (E2E_R1 | E2E_ERROR | E2E_FACE_QUALTY)
#define E2E_ERROR_R1_FACE_QUALTY_2  (E2E_R1 | E2E_ERROR | E2E_FACE_QUALTY_2)

#define E2E_ERROR_N1_FD_MODEL       (E2E_N1 | E2E_ERROR_FD_MODEL)
#define E2E_ERROR_N1_FD_POSTPROC    (E2E_N1 | E2E_ERROR_FD_POSTPROC)
#define E2E_ERROR_N1_FD_BBOX        (E2E_N1 | E2E_ERROR_FD_BBOX)
#define E2E_ERROR_N1_LM_MODEL       (E2E_N1 | E2E_ERROR_LM_MODEL)
#define E2E_ERROR_N1_LM_POSTPROC    (E2E_N1 | E2E_ERROR_LM_POSTPROC)
#define E2E_ERROR_N1_LM_MOTION_CHK  (E2E_N1 | E2E_ERROR_LM_MOTION_CHK)
#define E2E_ERROR_N1_LM_BAD_POSE    (E2E_N1 | E2E_ERROR_LM_BAD_POSE)
#define E2E_ERROR_N1_LM_BBOX        (E2E_N1 | E2E_ERROR_LM_BBOX)
#define E2E_ERROR_N1_FR_MODEL       (E2E_N1 | E2E_ERROR_FR_MODEL)
#define E2E_ERROR_N1_FR_POSTPROC    (E2E_N1 | E2E_ERROR_FR_POSTPROC)
#define E2E_ERROR_N1_FACE_MASK      (E2E_N1 | E2E_ERROR_FD_FACE_MASK)
#define E2E_ERROR_N1_EYE_CLOSED     (E2E_N1 | E2E_ERROR_EYE_CLOSED)
#define E2E_ERROR_N1_FACE_QUALTY    (E2E_N1 | E2E_ERROR | E2E_FACE_QUALTY)
#define E2E_ERROR_N1_FACE_QUALTY_2  (E2E_N1 | E2E_ERROR | E2E_FACE_QUALTY_2)

#define E2E_ERROR_LM_DIFF           (E2E_ERROR | E2E_DIFF | E2E_LM)
#define E2E_ERROR_FR_DIFF           (E2E_ERROR | E2E_DIFF | E2E_FR)
#define E2E_ERROR_RXNX_POSTPROC     (E2E_ERROR | E2E_RXNX | E2E_POSTPROC)
#define E2E_STATUS_FR_MODEL_FLIP_0  (E2E_STATUS | E2E_MODEL | E2E_FR | 0)
#define E2E_STATUS_FR_MODEL_FLIP_1  (E2E_STATUS | E2E_MODEL | E2E_FR | 1)

struct cfg_board_params_0 {
    float offset_x;
    float offset_y;
    float x_disparity_scaling;
    float x_scaling;
    float y_scaling;
    float rgb_nir_pos_diff_x;
    float rgb_nir_pos_diff_y;
    float rgb_fd_width_to_dist_ratio;
    float rgb_nir_width_ratio;
    float rgb_nir_height_ratio;
    float rgb_eye_range_scale;
    float d_offset;
};

struct cfg_board_params_1 {
    float rgb_eye_range_multiplier;
    float rgb_eye_range_offset;
};


#endif
