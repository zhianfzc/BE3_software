#ifndef __SHARE_H__
#define __SHARE_H__
#include "board_kl520.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_KCOMM
#include <stdint.h>
#include "model_res.h"
#include "kdp_app_dme.h"//"kapp_dme.h"

enum APP_ID {
    APP_UNKNOWN = 0,
    APP_LW3D,
    APP_SFID,
    APP_DME,
    APP_ID_MAX
};

extern u16 APP_id;

/* kai2do: move these to cmd_dme.c once DME logic in CMD_NACK is removed */
extern struct kdp_dme_cfg dme_conf;
extern dme_res *dme_od_out;
extern fd_age_gender_res *dme_agg_out;
extern uint32_t dme_raw_out;
extern uint32_t dme_out_len;

/* shared by fid/dme */
extern struct kdp_img_cfg rgb_img_config;
extern struct kdp_img_cfg nir_img_config;

void init_img_cfg(void);
#endif
#endif
