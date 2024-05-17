#ifndef __KDP_E2E_SETTINGS_H__
#define __KDP_E2E_SETTINGS_H__


#include "types.h"


/* This settings data may be used at anytime and anywhere */
typedef struct kdp_e2e_settings_struct {
    /* global */
    s32 calibration_count;
    float registered_offsetX;
    float registered_offsetY;
    s32 reserved;
} kdp_e2e_settings;

u32 kdp_e2e_settings_get_start_addr(void);
u32 kdp_e2e_settings_req_free_space(u32 size);
u32 kdp_e2e_settings_get_remaining_space(void);

kdp_e2e_settings* kdp_e2e_settings_get_inst(void);
void kdp_e2e_settings_init(void);
s32 kdp_e2e_settings_delete(void);
s32 kdp_e2e_settings_read(void* data_addr, u32 data_size);
s32 kdp_e2e_settings_write(void* data_addr, u32 data_size); 

#define kdp_e2e_settings_set(p, arg, val) p->arg = val
#define kdp_e2e_settings_get(p, arg) p->arg
#define kdp_e2e_settings_set2(arg, val) kdp_e2e_settings_get_inst()->arg = val
#define kdp_e2e_settings_get2(arg) kdp_e2e_settings_get_inst()->arg


#endif
