#ifndef __KDP_E2E_R1N1_H__
#define __KDP_E2E_R1N1_H__


#include "board_cfg.h"
#include "kdp_app_fid.h"


void kdp_e2e_reset(void);

s32 kdp_e2e_face_r1n1_environmental_adaptation(
    struct facedet_result_s *r1_fd, struct landmark_result_s *r1_lm);

s32 kdp_e2e_face_r1n1_ncpu_liveness(void);

s32 kdp_e2e_face_r1n1_invalid_score(int *lp_updated_e2e_ret);
s32 kdp_e2e_face_r1n1(void);

#endif
