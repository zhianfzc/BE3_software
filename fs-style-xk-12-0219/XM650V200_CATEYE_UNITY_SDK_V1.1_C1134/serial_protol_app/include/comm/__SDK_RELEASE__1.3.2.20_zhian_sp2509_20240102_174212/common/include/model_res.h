#ifndef __MODEL_RES_H__
#define __MODEL_RES_H__

/* These header defines structures shared by scpu/ncpu/host_lib */

#define LAND_MARK_POINTS       5
#define EYE_LID_LM_POINTS      7
#define FR_FEATURE_MAP_SIZE    512
#define LV_R_SIZE              1
#define LV_SCORE_SIZE          2
#define DUAL_LAND_MARK_POINTS  10
#define DME_OBJECT_MAX         80
#define IMAGENET_TOP_MAX       5
#define HAND_KEY_POINTS        7

/* Yolo Result */
struct bounding_box_s {
    float x1;      // top-left corner: x
    float y1;      // top-left corner: y
    float x2;      // bottom-right corner: x
    float y2;      // bottom-right corner: y
    float score;   // probability score
    int32_t class_num; // class # (of many) with highest probability
};

struct yolo_result_s {
    uint32_t class_count;            // total class count
    uint32_t box_count;              // boxes of all classes
    struct bounding_box_s boxes[1];  // box_count
};

struct age_gender_result_s {
    uint32_t age;
    uint8_t ismale;
};

struct imagenet_result_s {
    int32_t   index; // index of the class
    float score; // probability score of the class
};

struct facedet_result_s {
    int32_t len;
    int32_t xywh[4]; // 4 values for X, Y, W, H
    float xywh_fl[4]; // 4 values for X, Y, W, H
    float score;     //prob score
    int32_t class_num; //class
};

struct landmark_result_s {
    struct {
        uint32_t x;
        uint32_t y;
        float    x_f;
        float    y_f;
    } marks[LAND_MARK_POINTS];
    float score;
    float blur;
};

struct hand_kp_result_s {
    struct {
        float    x_f;
        float    y_f;
    } marks[HAND_KEY_POINTS];
    float score;
};

struct eye_lid_lm_result_s {
    struct {
        uint32_t x;
        uint32_t y;
    } marks[EYE_LID_LM_POINTS];
    float score;
};

struct face_occlude_result_s {
    float yaw;
    float pitch;
    float roll;
    float occ;
    float seg_res[7];
};

struct age_group_result_s {
    int32_t age;
};

struct face_quality_result_s {
    float face_score;
};

struct fr_result_s {
    float feature_map[FR_FEATURE_MAP_SIZE];
};

/* by larry lai */

struct lv_result_s{
    int32_t  real[LV_R_SIZE];
    float    score[LV_SCORE_SIZE];
    _Bool    wb_result;
    float    nir_luma_ratio;
    uint8_t  rgb_quality;
    uint8_t  rgb_corner_y;
    float    effect_2d;
    uint8_t  cal_nir_led_on_tile;
    uint8_t  cal_distance;
    float    id_ref_c;   
};

struct dual_landmarks_s {
    struct {
        uint32_t x;
        uint32_t y;
    } marks[DUAL_LAND_MARK_POINTS];
};

typedef struct {
    struct bounding_box_s fd_res;
    struct age_gender_result_s ag_res;
} fd_age_gender_res;

typedef struct {
    uint32_t class_count; // total class count
    uint32_t box_count;   // boxes of all classes
    struct bounding_box_s boxes[DME_OBJECT_MAX]; // box information
} dme_res;

#endif
