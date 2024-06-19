#ifndef __TIMER_H_
#define __TIMER_H_


void udelay(unsigned long usec);
void mdelay(unsigned long msec);
unsigned long get_timer(unsigned long base);
void clk_get_pts(unsigned long long *pts);

#endif
