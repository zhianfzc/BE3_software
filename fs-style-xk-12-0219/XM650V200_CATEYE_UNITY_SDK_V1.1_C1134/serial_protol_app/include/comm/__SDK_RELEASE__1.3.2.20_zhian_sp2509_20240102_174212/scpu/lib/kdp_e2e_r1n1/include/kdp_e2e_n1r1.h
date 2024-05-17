#ifndef __KDP_E2E_N1R1_H__
#define __KDP_E2E_N1R1_H__

#include "kdp_app_fid.h"
#define USE_N1R1_LED_ON_OFF_SWITCH    (NO)

s32 kdp_e2e_face_n1r1_environmental_adaptation(struct facedet_result_s *n1_fd, struct landmark_result_s *n1_lm);
//s32 kdp_e2e_face_n1r1_ncpu_liveness(void);
//s32 kdp_e2e_face_n1r1_invalid_score(int *lp_updated_e2e_ret);
s32 kdp_e2e_face_n1r1(void);
//void kdp_e2e_face_n1r1_get_luma_ratio(void);
s32 kdp_e2e_palm_r1(void);
#endif
