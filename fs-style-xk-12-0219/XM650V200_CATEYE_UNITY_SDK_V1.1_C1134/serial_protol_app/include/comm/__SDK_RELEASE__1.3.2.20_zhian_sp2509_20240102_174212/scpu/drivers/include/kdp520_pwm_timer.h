#ifndef __KDP520_PWM_TIMER_H
#define __KDP520_PWM_TIMER_H


#include <stdint.h>
#include "types.h"
#include "kneron_mozart.h"


#define HZ			                (100)				/// how many tick each sec

#define MAX_PWM_TIMER		        (6)

#define TIMER_INTSTAT				(0x0)
//#define TIMER_CR				    (0x0)
#define TIMER_LOAD					(0x4)
#define TIMER_COMPARE				(0x8)
#define TIMER_CNTO					(0xc)


typedef enum {
    PWMTIMER1=1,
    PWMTIMER2=2,
    PWMTIMER3=3, 
    PWMTIMER4=4,
    PWMTIMER5=5,
    PWMTIMER6=6,
} pwmtimer;


typedef struct 
{	
	UINT32 TmSrc:1;	/* bit 0 */
	UINT32 TmStart:1;
	UINT32 TmUpdate:1;
	UINT32 TmOutInv:1;
	UINT32 TmAutoLoad:1;
	UINT32 TmIntEn:1;
	UINT32 TmIntMode:1;
	UINT32 TmDmaEn:1;	
	UINT32 TmPwmEn:1;	/* bit 8 */
	UINT32 Reserved:15;	/* bit 9~23 */
	UINT32 TmDeadZone:8;	/* bit 24~31 */
}kdp_timer_control ;
typedef struct
{	
			UINT32 Counter;
}kdp_pwm_CNTB;

typedef struct
{
	UINT32 CompareBuffer;
}kdp_pwm_CMPB; //CMPB

typedef struct 
{
    UINT32 IntNum;			/* interrupt number */   
    UINT32 Tick;				/* Tick Per Second */   
    UINT32 Running;			/* Is timer running */       
}kdp_timer_struct;


#define APB_CLK APB_CLOCK
//--------------------------------------------------
// Timer tick
//--------------------------------------------------
#define PWMTMR_5000MSEC_PERIOD      (UINT32)(APB_CLK*5)
#define PWMTMR_1000MSEC_PERIOD		(UINT32)(APB_CLK)
#define PWMTMR_100MSEC_PERIOD			(UINT32)(APB_CLK/10)
#define PWMTMR_20MSEC_PERIOD			(UINT32)(APB_CLK/50)
#define PWMTMR_15MSEC_PERIOD			(UINT32)(((APB_CLK/100)*3)/2)
#define PWMTMR_10MSEC_PERIOD			(UINT32)(APB_CLK/100)
#define PWMTMR_1MSEC_PERIOD				(UINT32)(APB_CLK/1000)
#define PWMTMR_01MSEC_PERIOD			(UINT32)(APB_CLK/10000)

typedef enum Timer_IoType
{
	IO_TIMER_RESETALL,
	IO_TIMER_GETTICK,
	IO_TIMER_SETTICK,
	IO_TIMER_SETCLKSRC
}timeriotype;


/*PWM function*/ 
/**
 * enum pwm_polarity - polarity of a PWM signal
 * @PWM_POLARITY_NORMAL: a high signal for the duration of the duty-
 * cycle, followed by a low signal for the remainder of the pulse
 * period
 * @PWM_POLARITY_INVERSED: a low signal for the duration of the duty-
 * cycle, followed by a high signal for the remainder of the pulse
 * period
 */
typedef enum {
	PWM_POLARITY_NORMAL = 0,
	PWM_POLARITY_INVERSED,
}pwmpolarity;

extern BOOL kdp_pwm_config(pwmtimer timer, pwmpolarity polarity, int duty_ms, int period_ms);
extern BOOL kdp_pwm_enable(pwmtimer timer);
extern BOOL kdp_pwm_disable(pwmtimer timer);
extern BOOL kdp_pwm_start(pwmtimer timer);
extern BOOL kdp_pwm_stop(pwmtimer timer);

extern BOOL kdp_timer_init(pwmtimer timer, UINT32 tick);
extern INT32 kdp_timer_ioctrl(timeriotype IoType,pwmtimer timer,UINT32 tick);
extern INT32 kdp_timer_counter(pwmtimer timer);
extern INT32 kdp_timer_autoreloadenable(pwmtimer timer);
extern void kdp_timer_autoreloadvalue(pwmtimer timer, UINT32 value);
extern void kdp_timer_cmpvalue(pwmtimer timer, UINT32 value);
extern INT32 kdp_timer_enable(pwmtimer timer);
extern INT32 kdp_timer_disable(pwmtimer timer);
extern INT32 kdp_timer_close(pwmtimer timer);
extern void kdp_timer_resetall(void);

extern INT32 kdp_timer_int_enable(pwmtimer timer);
extern INT32 kdp_timer_int_disable(UINT32 timer);
extern INT32 kdp_timer_int_clear(pwmtimer timer);
extern INT32 kdp_timer_intmode_enable(UINT32 timer,UINT32 mode);
extern void kdp_timer_tick_reset(pwmtimer timer);


extern UINT32 kdp_current_t1_tick(void);
extern UINT32 kdp_current_t2_tick(void);
extern UINT32 kdp_current_t3_tick(void);
extern UINT32 kdp_current_t4_tick(void);
extern UINT32 kdp_current_t5_tick(void);
extern UINT32 kdp_current_t6_tick(void);

extern void PWMTMR1_IRQHandler(void);
extern void PWMTMR2_IRQHandler(void);
extern void PWMTMR3_IRQHandler(void);
extern void PWMTMR4_IRQHandler(void);
extern void PWMTMR5_IRQHandler(void);
extern void PWMTMR6_IRQHandler(void);


#endif //__KDP520_PWM_TIMER_H
