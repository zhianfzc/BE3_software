#ifndef __POWER_H__
#define __POWER_H__


#include "types.h"

/* Power Domains */
#define POWER_DOMAIN_DEFAULT        1
#define POWER_DOMAIN_NPU            2
#define POWER_DOMAIN_DDRCK          3

enum power_mgr_ops {
    POWER_MGR_OPS_FCS = 0,
    POWER_MGR_OPS_CHANGE_BUS_SPEED,
    POWER_MGR_OPS_PLL_UPDATE,
    POWER_MGR_OPS_SLEEPING,    
};

enum power_mgr_mode {
    POWER_MGR_MODE_RTC = 0,          //rtc
    POWER_MGR_MODE_ALWAYSON,         //rtc + default 
    POWER_MGR_MODE_FULL,             //rtc + default + ddr + npu
    POWER_MGR_MODE_RETENTION,        //rtc + default + ddr(self-refresh)
    POWER_MGR_MODE_DEEP_RETENTION    //rtc + ddr(self-refresh)
};

void power_mgr_ops(enum power_mgr_ops ops);
void power_mgr_set_mode(enum power_mgr_mode mode);
//low level power manager api
void power_mgr_set_domain(u32 domain, int enable);
void power_mgr_set_domain_2(u32 types, BOOL enable);
void power_mgr_sw_reset(void);
void power_mgr_softoff(enum power_mgr_mode mode);
//#define DEBUG_POWER_MGR
#ifndef DEBUG_POWER_MGR
void power_mgr_test(void);
#endif


#endif
