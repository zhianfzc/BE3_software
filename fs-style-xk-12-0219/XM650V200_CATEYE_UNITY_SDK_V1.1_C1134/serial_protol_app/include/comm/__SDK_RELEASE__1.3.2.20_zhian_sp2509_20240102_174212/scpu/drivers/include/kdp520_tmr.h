#ifndef __KDP520_TIMER_H
#define __KDP520_TIMER_H

#include <stdint.h>
#include "types.h"
#include "kdp520_pwm_timer.h"
#include "kneron_mozart.h"

#define TMRCTRL_START         (1 << 1)
#define TMRCTRL_UPDATE        (1 << 2)
#define TMRCTRL_AUTORELOAD    (1 << 4)
#define TMRCTRL_INTEN         (1 << 5)

#define TIMEOUT_COUNTER         (0xF0000000)
#define UPPER_COUNT             (0x5)
#define UP                      (1)
#define DOWN                    (2)
#define INTERNAL                (1)
#define EXTERNAL                (2)
#define PCLK                    (0)
#define EXTCLK                  (1)

typedef enum {
    TMR1=1,
    TMR2=2,
    TMR3=3, 
} tmr;


/*  -------------------------------------------------------------------------------
 *   API
 *  -------------------------------------------------------------------------------
 */
 
//this routines will export to upper ap or test program
BOOL Timer_Init(tmr, UINT32, UINT32);
INT32 Timer_IOCtrl(timeriotype, tmr, UINT32);

UINT32 Timer_Counter(tmr, UINT32);
INT32 Write_MR(tmr, int, int, int);
INT32 Write_CR(tmr, UINT32, UINT32);

//void Timer_Isr(UINT32);

void TickTimer1(void);
void TickTimer2(void);
void TickTimer3(void);

void Tick_Timer1_One_Shot(void);
void Tick_Timer2_One_Shot(void);
void Tick_Timer3_One_Shot(void);

UINT32 GetCurrentT1Tick(void);
UINT32 GetCurrentT2Tick(void);
UINT32 GetCurrentT3Tick(void);

void Timer_AutoReloadValue(tmr, UINT32);
void Timer_MatchValue1(tmr, UINT32);
void Timer_MatchValue2(tmr, UINT32);

INT32 Timer_Enable(tmr);
INT32 Timer_Disable(tmr);
INT32 Timer_Close(tmr);
INT32 Timer_IntEnable(tmr);
void ResetAllTimer(void);

BOOL kdp_tmr_init(tmr timer, u32 tick);

#endif //__KDP520_TIMER_H
