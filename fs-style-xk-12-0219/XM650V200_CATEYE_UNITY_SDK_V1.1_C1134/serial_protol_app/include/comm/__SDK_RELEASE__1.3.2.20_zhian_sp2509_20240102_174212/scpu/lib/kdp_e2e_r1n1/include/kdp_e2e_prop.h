#ifndef __KDP_E2E_PROP_H__
#define __KDP_E2E_PROP_H__

#include "types.h"
#include "ipc.h"

#define E2E_FD_CHK_SMALL_WIDTH_DEF      (70)    // (112)
#define E2E_FD_CHK_SMALL_HEIGHT_DEF     (70)    // (112)
#define E2E_R1_LM_CHK_LOW_CONFIDENT_DEF (0.800)
#define E2E_R1_LM_CHK_BLUR_DEF          (0.0078125)//(1.0/128)
#define E2E_N1_LM_CHK_LOW_CONFIDENT_DEF (0.000)  
#define E2E_N1_LM_CHK_BLUR_DEF          (0.0078125)//(1.0/128)
#define E2E_DB_DYNAMIC_THRESHOLD_DEF    (0.77f)
#define E2E_DB_DYNAMIC_THRESHOLD_REMOTE (0.95f)
#define E2E_DB_R1_WEIGHT_DEF            (0.5)
#define E2E_DB_PALM_THRESHOLD_DEF       (0.77f)

#define E2E_RGB_NIR_LM_DIFF_THRESHOLD   (30)

#define E2E_DB_THRESHOLD_FID_DEF        (E2E_DB_DYNAMIC_THRESHOLD_DEF - 0.0f)
#define E2E_DB_THRESHOLD_FID_SELF_DEF   (0.9f)
#define E2E_DB_THRESHOLD_AGE_DIFF       (0.015f)

#define E2E_N1_LM_RECOG_THRESH_INDOOR   (0.6f)  
#define E2E_N1_LM_RECOG_THRESH_OUTDOOR  (0.1f)  

#define E2E_NIR_LV_THRESHOLD            (0.8f)
#define E2E_RGB_LV_THRESHOLD            (0.1f)
#define E2E_FUSE_LV_THRESHOLD           (0.5f)
#define E2E_OCCLUSION_THRESHOLD         (0.6f)

typedef enum kdp_e2e_prop_attribute_enum {
    PROP_ATTRIBUTE_AUTO = 0,
    PROP_ATTRIBUTE_MANUAL,
} kdp_e2e_prop_attribute;

#define KDP_E2E_PROP_STRUCT(__type) \
typedef struct s_kdp_e2e_prop_value_##__type {  \
    BOOL adopted;                               \
    kdp_e2e_prop_attribute attr;                \
    __type value;                               \
    const __type default_val;                   \
} kdp_e2e_prop_value_##__type

KDP_E2E_PROP_STRUCT(s32);
KDP_E2E_PROP_STRUCT(float);

typedef struct kdp_e2e_prop_pt_struct
{
    u32 x;
    u32 y;
} kdp_e2e_prop_pt;

typedef struct kdp_e2e_prop_rect_struct
{
    unsigned short start_x;
    unsigned short start_y;
    unsigned short end_x;
    unsigned short end_y;
} kdp_e2e_prop_rect;

typedef struct kdp_e2e_prop_struct {
    BOOL liveness_en;
    BOOL motion_detect_en;
    BOOL nose_lm_diff_en;
    s32 check_bad_pose;
    BOOL nir_aec_continuous_tune_en;
    BOOL rgb_led_when_reg_en;
    BOOL face_check_movement_en;
    BOOL face_check_position;
    u32 flow_mode;
    u32 face_mode;
    kdp_e2e_prop_value_s32 fd_chk_small_width;
    kdp_e2e_prop_value_s32 fd_chk_small_height;
    kdp_e2e_prop_value_float r1_lm_chk_low_confident;
    kdp_e2e_prop_value_float r1_lm_chk_blur;
    kdp_e2e_prop_value_float n1_lm_chk_low_confident;
    kdp_e2e_prop_value_float n1_lm_chk_blur;
    kdp_e2e_prop_value_float dynamic_threshold;
    kdp_e2e_prop_value_float r1_weight;
    kdp_e2e_prop_value_float db_threshold_normal;
    kdp_e2e_prop_value_float db_threshold_self;
    kdp_e2e_prop_value_float db_threshold_age_diff;
    kdp_e2e_prop_value_float db_threshold_remote;    
    kdp_e2e_prop_value_float nir_lv_threshold;
    kdp_e2e_prop_value_float rgb_lv_threshold;
    kdp_e2e_prop_value_float fuse_lv_threshold;
    kdp_e2e_prop_value_float n1_occlusion_threshold;
    kdp_e2e_prop_value_s32 rgb_nir_lm_diff_threshold;
    s32 dst_win_width;
    s32 dst_win_height;
    s32 r1_offset_x;
    s32 r1_offset_y;
    s32 n1_dst_win_width;
    s32 n1_dst_win_height;
    s32 n1_offset_x;
    s32 n1_offset_y;
    s32 bbox_tolerance_lines;
    
    s32 invalid_score_min;
    s32 invalid_score_max;
    s16 rgb_led_enhance_cnt;
    s8  rgb_led_enhance_step;
    s8  rgb_led_enhance_min;
    s8  rgb_led_enhance_max;

    kdp_e2e_prop_rect r1_rc;
    kdp_e2e_prop_rect n1_rc;
    kdp_e2e_prop_pt r1_pt_array[LAND_MARK_POINTS];
    kdp_e2e_prop_pt n1_pt_array[LAND_MARK_POINTS];
    kdp_e2e_prop_value_s32 r1_led_value;
    BOOL age_group_en;

} kdp_e2e_prop;


kdp_e2e_prop* kdp_e2e_prop_get_inst(void);
void kdp_e2e_prop_init(void);
void kdp_e2e_prop_enable_liveness(BOOL enable);
void kdp_e2e_prop_update_db_comp_params(void);
void kdp_e2e_prop_r1_update_rect(int x, int y, int w, int h);
void kdp_e2e_prop_n1_update_rect(int x, int y, int w, int h);
void kdp_e2e_prop_r1_update_pt_array(u32 *pt_array);
void kdp_e2e_prop_n1_update_pt_array(u32 *pt_array);

#define KDP_E2E_PROP_DECLARE(__member) void kdp_e2e_prop_reset_##__member(void)
KDP_E2E_PROP_DECLARE(r1_led_value);

#define kdp_e2e_prop_reset_value(p, arg) { p->arg.adopted = FALSE; p->arg.attr = PROP_ATTRIBUTE_AUTO; p->arg.value = p->arg.default_val; } 
#define kdp_e2e_prop_get(p, arg) p->arg
#define kdp_e2e_prop_set(p, arg, __val) p->arg = __val
#define kdp_e2e_prop_get_value(p, arg) p->arg.value
#define kdp_e2e_prop_set_manual_value(p, arg, __val) { \
    p->arg.attr = PROP_ATTRIBUTE_MANUAL; \
    if (p->arg.value != __val) { p->arg.adopted = FALSE; p->arg.value = __val; } }
#define kdp_e2e_prop_set_auto_value(p, arg, __val) { \
    if ((p->arg.value != __val) && (PROP_ATTRIBUTE_AUTO == p->arg.attr)) \
        { p->arg.adopted = FALSE; p->arg.value = __val; } }

#define kdp_e2e_prop_set_adopted(p, arg) p->arg.adopted = TRUE
#define kdp_e2e_prop_is_adopted(p, arg) (p->arg.adopted == TRUE)
#define kdp_e2e_prop_is_manual(p, arg) (PROP_ATTRIBUTE_MANUAL == p->arg.attr)
#define kdp_e2e_prop_set_auto(p, arg) p->arg.attr = PROP_ATTRIBUTE_AUTO

#define kdp_e2e_prop_get2(arg) kdp_e2e_prop_get_inst()->arg
#define kdp_e2e_prop_set2(arg, __val) kdp_e2e_prop_get_inst()->arg = __val
#define kdp_e2e_prop_get2_value(arg) (kdp_e2e_prop_get_inst()->arg.value)


#endif
