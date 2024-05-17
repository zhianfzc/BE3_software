#ifndef __SYSTEM_H__
#define __SYSTEM_H__


void system_init(void);
void system_wakeup_ncpu(unsigned char wakeup_all);
void system_init_ncpu(void);
void load_ncpu_fw(int reset_flag);

#endif
