/*
 * Kneron Delay driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#ifdef TARGET_SCPU
#include "drivers.h"
#include "kneron_mozart.h"
#include "kdp520_pwm_timer.h"
#include "kdp520_tmr.h"
#include "delay.h"
#else
#include <stdint.h>
#include "delay.h"
#endif

extern uint32_t SystemCoreClock;

#define CYCLES_PER_LOOP     2   // Typical: SUBS (1) + BCS (1 + P)

void delay_us(unsigned int usec)
{
    uint32_t cycles_per_us, count;

    cycles_per_us = SystemCoreClock / 1000000;
    count = usec * cycles_per_us / CYCLES_PER_LOOP;
    while(count--);
}
#ifdef TARGET_SCPU
static int _delay_en = 0;
void delay_ms_enable(void)
{
    if(_delay_en == 0){
        kdp_tmr_init(TMR1, PWMTMR_1MSEC_PERIOD);
        Timer_IntEnable(TMR1);
    }
    _delay_en++;
}

void delay_ms_disable(void)
{
    if (_delay_en > 0) {
        _delay_en--;
    }
    
    if(_delay_en ==0){
        Timer_Close(TMR1);
    }    
}

void delay_ms(unsigned int msec)
{
    uint32_t tick_end;

    delay_ms_enable();

    tick_end = GetCurrentT1Tick() + msec;
    do {
        __WFE();
    } while (tick_end > GetCurrentT1Tick());
    
    delay_ms_disable();
}
#endif
