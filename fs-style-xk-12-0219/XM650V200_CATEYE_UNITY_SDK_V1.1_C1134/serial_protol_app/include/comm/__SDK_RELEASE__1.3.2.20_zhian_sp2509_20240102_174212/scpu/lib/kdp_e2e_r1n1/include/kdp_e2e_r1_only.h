#ifndef __KDP_E2E_R1_ONLY_H__
#define __KDP_E2E_R1_ONLY_H__


#include "kdp_app_fid.h"

//s32 kdp_e2e_face_r1_only_postproc(void);
s32 kdp_e2e_face_r1_only_preproc(void);
s32 kdp_e2e_face_r1_ncpu_liveness(void);
s32 kdp_e2e_face_r1_invalid_score(int *lp_updated_e2e_ret);
s32 kdp_e2e_face_r1_only(void);


#endif
