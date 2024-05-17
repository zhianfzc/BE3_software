#ifndef __KDP_E2E_N1_ONLY_H__
#define __KDP_E2E_N1_ONLY_H__

#include "kdp_app_fid.h"

#define E2E_N1_ONLY_STRUCTURE_ENABLE        (CFG_E2E_STRUCT_LIGHT)
#define E2E_N1_TWO_STAGE_LED_MODE           (CFG_E2E_NIR_TWO_STAGE_LIGHT)
#define E2E_N1_REC_STR_LED_CLOSE_EARLY      (YES)
#if (CFG_SENSOR_TYPE == SENSOR_TYPE_GC1054_GC1054) || \
    (CFG_SENSOR_TYPE == SENSOR_TYPE_GC02M1_GC1054) || \
    (CFG_SENSOR_TYPE == SENSOR_TYPE_OV02B1B_OV02B1B) || \
    (CFG_SENSOR_TYPE == SENSOR_TYPE_SP2509_SP2509)
#define E2E_RGB_USE_RAW8                    (YES)
#else
#define E2E_RGB_USE_RAW8                    (NO)
#endif

s32 kdp_e2e_face_n1_only(void);
s32 kdp_e2e_face_n1_only_preproc(void);
s32 kdp_e2e_face_n1_only_environmental_adaptation(void);
s32 kdp_e2e_face_n1_only_stucture_light_live(void);
s32 kdp_e2e_face_n1_only_liveness(void);
s32 kdp_e2e_face_n1_only_invalid_score(int *lp_updated_e2e_ret);
#if (E2E_N1_ONLY_STRUCTURE_ENABLE == YES)
s32 kdp_e2e_n1_only_ir_st_diff_same_id_compare(void);
#endif

#endif
