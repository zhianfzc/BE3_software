#ifndef __DDR_H__
#define __DDR_H__


enum ddr_init_mode {
    DDR_INIT_WAKEUP_ONLY = 0,
    DDR_INIT_ALL,
    DDR_INIT_ALL_EXIT_SELF_REFRESH,
};

#ifdef KL520
void ddr_wakeup(void);
void ddr_init(enum ddr_init_mode mode);
void ddr_init_chip_0429(void);
void ddr_self_refresh_enter(void);
void ddr_self_refresh_exit(void);
#endif

void ddr_init_v35_mipi(unsigned char ncpu_wakeup);


#endif
