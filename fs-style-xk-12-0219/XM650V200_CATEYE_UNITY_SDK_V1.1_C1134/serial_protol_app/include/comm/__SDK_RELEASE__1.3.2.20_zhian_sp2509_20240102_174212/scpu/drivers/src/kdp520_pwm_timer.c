#include <cmsis_os2.h>
#include <string.h> 
#include "base.h"
#include "io.h"
#include "Driver_Common.h"
#include "kneron_mozart.h"
#include "kdp520_pwm_timer.h"
#include "pinmux.h"
#include "delay.h"
#include "dbg.h"
#include "media/display/lcm.h"

#define TMR_CTRL_CLK_SRC        BIT0
#define TMR_CTRL_START          BIT1
#define TMR_CTRL_UPDATE         BIT2
#define TMR_CTRL_OUT_INV        BIT3
#define TMR_CTRL_AUTO_LOAD      BIT4
#define TMR_CTRL_INT_EN         BIT5
#define TMR_CTRL_INT_MODE       BIT6
#define TMR_CTRL_DMA_EN         BIT7
#define TMR_CTRL_PWM_EN         BIT8

static UINT32 TimerBase[MAX_PWM_TIMER+1] ={0, PWM_FTPWMTMR010_PA_BASE+0x10, PWM_FTPWMTMR010_PA_BASE+0x20,PWM_FTPWMTMR010_PA_BASE+0x30,PWM_FTPWMTMR010_PA_BASE+0x40,PWM_FTPWMTMR010_PA_BASE+0x50,PWM_FTPWMTMR010_PA_BASE+0x60/*,PWM_FTPWMTMR010_PA_BASE+0x70*/};
static UINT32 CNTBBase[MAX_PWM_TIMER+1] ={0, PWM_FTPWMTMR010_PA_BASE+0x14, PWM_FTPWMTMR010_PA_BASE+0x24,PWM_FTPWMTMR010_PA_BASE+0x34,PWM_FTPWMTMR010_PA_BASE+0x44,PWM_FTPWMTMR010_PA_BASE+0x54,PWM_FTPWMTMR010_PA_BASE+0x64/*,PWM_FTPWMTMR010_PA_BASE+0x74*/};
static UINT32 CMPBBase[MAX_PWM_TIMER+1] ={0, PWM_FTPWMTMR010_PA_BASE+0x18, PWM_FTPWMTMR010_PA_BASE+0x28,PWM_FTPWMTMR010_PA_BASE+0x38,PWM_FTPWMTMR010_PA_BASE+0x48,PWM_FTPWMTMR010_PA_BASE+0x58,PWM_FTPWMTMR010_PA_BASE+0x68/*,PWM_FTPWMTMR010_PA_BASE+0x78*/};

//kdp_timer_control *timer_control[MAX_PWM_TIMER+1];
static kdp_timer_struct ftimer[MAX_PWM_TIMER+1];
kdp_pwm_CNTB* PWMCNTB[MAX_PWM_TIMER+1];
kdp_pwm_CMPB* PWMCMPB[MAX_PWM_TIMER+1];

UINT32 t1_tick = 0, t2_tick = 0, t3_tick = 0, t4_tick = 0, t5_tick = 0, t6_tick = 0;

//funtion prototype
INT32 _get_timer_tick(pwmtimer timer);
INT32 _set_timer_tick(UINT32 timer,UINT32 clk_tick);
INT32 _set_timer_clk_source(pwmtimer timer,UINT32 clk);
//INT32 Timer_ConnectIsr(UINT32 timer,PrHandler handler);
void _print_pwm_reg(pwmtimer timer);

typedef void (*FNTimerISR)(void);

extern void kl520_api_tmr2_user(u32 *tick);
extern void kl520_api_tmr3_user(u32 *tick);
extern void kl520_api_tmr4_user(u32 *tick);
extern void kl520_api_tmr5_user(u32 *tick);
extern void kl520_api_tmr6_user(u32 *tick);

void PWMTMR1_IRQHandler(void)
{
    //internal reserved
    kdp_timer_int_clear(PWMTIMER1);
    ++t1_tick;
}

void PWMTMR2_IRQHandler(void)
{
    kdp_timer_int_clear(PWMTIMER2);
    ++t2_tick;
}

void PWMTMR3_IRQHandler(void)
{
    kl520_api_tmr3_user(&t3_tick);   
    kdp_timer_int_clear(PWMTIMER3);
    ++t3_tick;
}

void PWMTMR4_IRQHandler(void)
{
    kl520_api_tmr4_user(&t4_tick);
    kdp_timer_int_clear(PWMTIMER4);
    ++t4_tick;
}

void PWMTMR5_IRQHandler(void)
{
    kl520_api_tmr5_user(&t5_tick);
    kdp_timer_int_clear(PWMTIMER5);
    ++t5_tick;
}

void PWMTMR6_IRQHandler(void)
{
    kl520_api_tmr6_user(&t6_tick);
    kdp_timer_int_clear(PWMTIMER6);
    ++t6_tick;
}


/* Routine to disable a timer and free-up the associated IRQ */
INT32 kdp_timer_close(pwmtimer timer)
{
    u32 timer_irq;

    if (timer == 0 || timer > MAX_PWM_TIMER)
        return FALSE;

    if(!kdp_timer_disable(timer))
        return FALSE;   /* Stop the timer first */

    switch(timer)
    {
        case PWMTIMER1:
            timer_irq = PWM_FTPWMTMR010_1_IRQ;
            t1_tick = 0;
        break;
        case PWMTIMER2:
            timer_irq = PWM_FTPWMTMR010_2_IRQ;
            t2_tick = 0;
        break;
        case PWMTIMER3:
            timer_irq = PWM_FTPWMTMR010_3_IRQ;
            t3_tick = 0;
        break;
        case PWMTIMER4:
            timer_irq = PWM_FTPWMTMR010_4_IRQ;
            t4_tick = 0;
        break;
        case PWMTIMER5:
            timer_irq = PWM_FTPWMTMR010_5_IRQ;
            t5_tick = 0;
        break;
        case PWMTIMER6:
            timer_irq = PWM_FTPWMTMR010_6_IRQ;
            t6_tick = 0;
        break;
        default:
            return FALSE;
    }	
        
    NVIC_DisableIRQ((IRQn_Type)timer_irq);
        
    return TRUE;
}


INT32 kdp_timer_ioctrl(timeriotype iotype, pwmtimer timer, UINT32 tick)
{
    switch(iotype)
    {
        case IO_TIMER_RESETALL:
            kdp_timer_resetall();
            break;
        case IO_TIMER_GETTICK:
            return _get_timer_tick(timer);
         // break;
        case IO_TIMER_SETTICK:
            return _set_timer_tick(timer,tick);
         // break;
        case IO_TIMER_SETCLKSRC:
            return _set_timer_clk_source(timer,tick);
         // break;
        default:
            return FALSE;
    }

    return TRUE;
}

/* Routine to start the specified timer & enable the interrupt */
BOOL kdp_timer_init(pwmtimer timer, u32 tick)
{
    kdp_timer_struct *ctimer=&ftimer[timer];
    UINT32 timer_irq;
    FNTimerISR isr;

    if (timer == 0 || timer > MAX_PWM_TIMER)
    return FALSE;

    switch(timer)
    {
        case PWMTIMER1:
            t1_tick = 0;
            timer_irq = PWM_FTPWMTMR010_1_IRQ;		
            isr = PWMTMR1_IRQHandler;
        break;
        case PWMTIMER2:
            t2_tick = 0;
            timer_irq = PWM_FTPWMTMR010_2_IRQ;		
            isr = PWMTMR2_IRQHandler;
        break;
        case PWMTIMER3:
            t3_tick = 0;
            timer_irq = PWM_FTPWMTMR010_3_IRQ;		
            isr = PWMTMR3_IRQHandler;
        break;
        case PWMTIMER4:
            t4_tick = 0;
            timer_irq = PWM_FTPWMTMR010_4_IRQ;		
            isr = PWMTMR4_IRQHandler;
        break;
        case PWMTIMER5:
            t5_tick = 0;
            timer_irq = PWM_FTPWMTMR010_5_IRQ;		
            isr = PWMTMR5_IRQHandler;
        break;
        case PWMTIMER6:
            t6_tick = 0;
            timer_irq = PWM_FTPWMTMR010_6_IRQ;		
            isr = PWMTMR6_IRQHandler;
        break;
        default:
            return FALSE;
    }

    kdp_timer_close(timer);

    /* Set the timer tick */
    if(!kdp_timer_ioctrl(IO_TIMER_SETTICK,timer,tick))
        return FALSE;

    kdp_timer_autoreloadvalue(timer,ctimer->Tick);
    /*enable auto and int bit */
    kdp_timer_autoreloadenable(timer);

    kdp_timer_int_clear(timer);
    if (!kdp_timer_int_enable(timer))
        return FALSE;

    NVIC_SetVector((IRQn_Type)timer_irq, (uint32_t)isr);    
    NVIC_EnableIRQ((IRQn_Type)timer_irq);

    /* Start the timer ticking */
    if(!kdp_timer_enable(timer))
        return FALSE;
        
    return TRUE;
}

INT32 kdp_timer_autoreloadenable(pwmtimer timer)
{
    //timer_struct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_PWM_TIMER))
        return FALSE;

    masked_outw(TimerBase[timer], TMR_CTRL_AUTO_LOAD, TMR_CTRL_AUTO_LOAD);
    return TRUE;
}


INT32 kdp_timer_counter(pwmtimer timer)
{
    return inw(TimerBase[timer] + TIMER_CNTO);
}


INT32 kdp_timer_int_enable(pwmtimer timer)
{
    if ((timer == 0) || (timer > MAX_PWM_TIMER))
        return FALSE;

    masked_outw(TimerBase[timer], TMR_CTRL_INT_EN, TMR_CTRL_INT_EN);
    return TRUE;
}

INT32 kdp_timer_int_disable(UINT32 timer)
{
    //timer_struct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_PWM_TIMER))
        return FALSE;

    masked_outw(TimerBase[timer], 0, TMR_CTRL_INT_EN);

    return TRUE;
}


INT32 kdp_timer_intmode_enable(UINT32 timer,UINT32 mode)
{
    //timer_struct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_PWM_TIMER))
        return FALSE;

    masked_outw(TimerBase[timer], mode << 6, TMR_CTRL_INT_MODE);

    return TRUE;
}

INT32 _timer_dma_enable(UINT32 timer)
{

    //timer_struct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_PWM_TIMER))
        return FALSE;

    masked_outw(TimerBase[timer], TMR_CTRL_DMA_EN, TMR_CTRL_DMA_EN);

    return TRUE;
}

/* This routine starts the specified timer hardware. */
INT32 kdp_timer_enable(pwmtimer timer)
{
    kdp_timer_struct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_PWM_TIMER))
        return FALSE;

    if(ctimer->Running==TRUE)
        return FALSE;

    masked_outw(TimerBase[timer], (TMR_CTRL_UPDATE | TMR_CTRL_START), (TMR_CTRL_UPDATE | TMR_CTRL_START));

    //set the timer status =true
    ctimer->Running=TRUE;

    return TRUE;
}


/* This routine stops the specified timer hardware. */
INT32 kdp_timer_disable(pwmtimer timer)
{
    kdp_timer_struct *ctimer=&ftimer[timer];

    if ((timer == 0) || (timer > MAX_PWM_TIMER))
        return FALSE;

    /* Disable the Control register bit */
    outw(TimerBase[timer], 0);

     //set the timer status=false
    ctimer->Running=FALSE;

    return TRUE;
}

/* This routine starts the specified timer hardware. */
INT32 kdp_timer_int_clear(pwmtimer timer)
{
    int value;

        if ((timer == 0) || (timer > MAX_PWM_TIMER))
    {
       return FALSE;
    }

    value=1<<(timer-1);
    outw(PWM_FTPWMTMR010_PA_BASE + TIMER_INTSTAT, value);

    return TRUE;
}

void kdp_timer_cmpvalue(pwmtimer timer, UINT32 value)
{
    outw(TimerBase[timer] + TIMER_COMPARE, value);
}

INT32 _set_timer_clk_source(pwmtimer timer,UINT32 clk)
{
    if ((timer == 0) || (timer > MAX_PWM_TIMER))
        return FALSE;

    masked_outw(TimerBase[timer], clk, TMR_CTRL_CLK_SRC);

    return TRUE;
}

#if 0
/* Routine to initialise install requested timer. Stops the timer. */
INT32 Timer_ConnectIsr(UINT32 timer,PrHandler handler)
{
    timer_struct *ctimer=&ftimer[timer];
    UINT32 i;

    i = fLib_Timer_Vectors[timer];

    if (request_irq(i, handler, IRQF_SHARED | IRQF_DISABLED, "timer", 0) < 0)
    {
        return FALSE;
    }

    ctimer->Handler = handler;
    ctimer->IntNum = i;     /* INT number */

    return timer;
}
#endif

UINT32 kdp_current_t1_tick(void)
{
    return t1_tick;
}

UINT32 kdp_current_t2_tick(void)
{
    return t2_tick;
}

UINT32 kdp_current_t3_tick(void)
{
    return t3_tick;
}

UINT32 kdp_current_t4_tick(void)
{
    return t4_tick;
}

UINT32 kdp_current_t5_tick(void)
{
    return t5_tick;
}

UINT32 kdp_current_t6_tick(void)
{
    return t6_tick;
}

void kdp_timer_tick_reset(pwmtimer timer)
{
    if ((timer == 0) || (timer > MAX_PWM_TIMER))
    {
        return;
    }

    switch(timer)
    {
        case PWMTIMER1:
            t1_tick = 0;
        break;
        case PWMTIMER2:
            t2_tick = 0;
        break;
        case PWMTIMER3:
            t3_tick = 0;
        break;
        case PWMTIMER4:
            t4_tick = 0;
        break;
        case PWMTIMER5:
            t5_tick = 0;
        break;
        case PWMTIMER6:
            t6_tick = 0;
        break;
        default:
            return;
    }   

    return;
}

/////////////////////////////////////////////////////
//
//  Only for detail function call subroutine
//
/////////////////////////////////////////////////////


/* Start-up routine to initialise the timers to a known state */
void kdp_timer_resetall(void)
{
    UINT32 i;

    //reset all timer to default value
    for (i = 1; i <= MAX_PWM_TIMER; i++)
        kdp_timer_disable((pwmtimer)i);

}

INT32 _get_timer_tick(pwmtimer timer)
{
    UINT32 cur_tick;

    volatile kdp_timer_struct *ctimer = &ftimer[timer];

    if ((timer == 0) || (timer > MAX_PWM_TIMER))
        return FALSE;

    cur_tick=ctimer->Tick;

    return cur_tick;
}

INT32 _set_timer_tick(UINT32 timer,UINT32 clk_tick)
{
    volatile kdp_timer_struct *ctimer = &ftimer[timer];

    if ((timer == 0) || (timer > MAX_PWM_TIMER))
        return FALSE;

    ctimer->Tick=clk_tick;

    return TRUE;
}

void kdp_timer_autoreloadvalue(pwmtimer timer, UINT32 value)
{
    outw(TimerBase[timer] + TIMER_LOAD, value);
}

// End of file - timer.c


// Start of PWM function
/* Routine to start the specified timer & enable the PWM function */
BOOL kdp_pwm_config(pwmtimer timer, pwmpolarity polarity, int duty, int period)
{
    //UINT32 val;
    unsigned int freq;
	unsigned int ratio_period = 0;
	unsigned int ratio_duty = 0;	
	unsigned int tmp;
    kdp_timer_control setting;
    if (timer == 0 || timer > MAX_PWM_TIMER)
        return FALSE;

    //_print_pwm_reg(timer);
        
    if(timer == PWMTIMER6)
    {
        //val = PINMUX_LC_DATA12_GET();
        //dbg_msg("LC_DATA12_GET = 0x%4x",val);
        PINMUX_LC_DATA12_SET(PINMUX_LC_DATA12_PWM5);
        //val = PINMUX_LC_DATA12_GET();
        //dbg_msg("LC_DATA12_GET = 0x%4x",val);
    }
 
    PWMCNTB[timer] = (kdp_pwm_CNTB *) (CNTBBase[timer]);
    PWMCMPB[timer] = (kdp_pwm_CMPB *) (CMPBBase[timer]);
    
    memset(&setting,0,4);

    if(period)                /*	ratio_period = 1000000000 / period_ns;*/
	{
		tmp = 1000000000;
		tmp = tmp/period;
		ratio_period = tmp;
	}
	
	if(duty)                /*	ratio_duty = 1000000000 / duty_ns; */
	{	
		tmp = 1000000000;
		tmp = tmp/duty;
		ratio_duty = tmp;
	}
    
    if(ratio_period)
	{	
		freq = APB_CLK / ratio_period;
		PWMCNTB[timer]->Counter = freq ;
	}
	
	if(ratio_duty)
	{	
		freq = APB_CLK / ratio_duty;
		PWMCMPB[timer]->CompareBuffer = freq ;
	}
    else
    {
        PWMCMPB[timer]->CompareBuffer = 1 ;
    }
    setting.TmStart = 0;
    setting.TmUpdate = 1;
    setting.TmOutInv = polarity;
    setting.TmAutoLoad = 1;
    memcpy((void *)TimerBase[timer],&setting,4);

    kdp_pwm_start(timer);

    if ( duty == 0 )
    {
        delay_us(1000);
        kdp_pwm_stop(timer);
    }

    return TRUE;
}

BOOL kdp_pwm_start(pwmtimer timer)
{
    masked_outw(TimerBase[timer], TMR_CTRL_START, TMR_CTRL_START);

    return TRUE;
}

BOOL kdp_pwm_stop(pwmtimer timer)
{
    masked_outw(TimerBase[timer], 0, TMR_CTRL_START);

    return TRUE;
}

BOOL kdp_pwm_enable(pwmtimer timer)
{
    masked_outw(TimerBase[timer], TMR_CTRL_PWM_EN, TMR_CTRL_PWM_EN);

    return TRUE;
}

BOOL kdp_pwm_disable(pwmtimer timer)
{
    masked_outw(TimerBase[timer], 0, TMR_CTRL_PWM_EN);
        
    return TRUE;
}

void _print_pwm_reg(pwmtimer timer)
{
#if 0
    UINT32 val = 0;
    
    val = inw(TimerBase[timer]);
    //dbg_msg("T%d_CTRL = 0x%4x", timer, val);
    val = inw(CNTBBase[timer]);
    //dbg_msg("T%d_CNTB = 0x%4x", timer, val);
    val = inw(CMPBBase[timer]);
    //dbg_msg("T%d_CMPB = 0x%4x", timer, val);
    val = inw(CMPBBase[timer] + 0x4);
    //dbg_msg("T%d_CNTO = 0x%4x", timer, val);
#endif
}

// End of PWM function

