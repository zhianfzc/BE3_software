#ifndef __KL520_API_SIM_H__
#define __KL520_API_SIM_H__

#include "board_kl520.h"

typedef struct _KL520_SIM_CONTEXT_ {
    s32 result;
} kl520_sim_ctx;


int kl520_api_sim_state(void);
void kl520_api_sim_set_usercmd(u8 id);
u8 kl520_api_sim_get_usercmd(void);
void kl520_api_sim_set_rst(void);
void kl520_api_sim_set_fdfr(void);
u8 kl520_api_sim_get_fdfr(void);
BOOL kl520_api_sim_is_running(void);
u16 kl520_api_sim_get_count(void);
void kl520_api_sim_fdfr(kl520_sim_ctx *ctx);
void kl520_api_sim_fdfr_flow_switch(void);
void kl520_api_sim_face_add(kl520_sim_ctx *ctx, u32 face_add_type, u32 face_mode);
void kl520_api_sim_face_recognition(kl520_sim_ctx *ctx);
void kl520_api_sim_face_pre_add(kl520_sim_ctx *ctx, u32 face_add_type);


#endif
