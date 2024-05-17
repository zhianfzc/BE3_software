#include <stdio.h>
#include "base.h"
#include "io.h"
#include "Driver_Common.h"
#include "kneron_mozart.h"
#include "kdp520_tmr.h"
#include "pinmux.h"
#include "dbg.h"

#define TIMER_RegCOUNT			(0x0)
#define TIMER_LOAD				(0x4)
#define TIMER_MATCH1			(0x8)
#define TIMER_MATCH2			(0xC)
#define TIMER_CR				(0x30)
#define TIMER_INTSTATE			(0x34)
#define TIMER_INTMASK			(0x38)
#define MAX_TIMER				(3)

//#define writel(val, addr) (*(volatile unsigned int *)(addr) = (val))
//#define readl(addr) (*(volatile unsigned int *)(addr))

typedef struct
{	
	UINT32 Tm1En:1;
	UINT32 Tm1Clock:1;
	UINT32 Tm1OfEn:1;
	UINT32 Tm2En:1;
	UINT32 Tm2Clock:1;
	UINT32 Tm2OfEn:1;
	UINT32 Tm3En:1;
	UINT32 Tm3Clock:1;
	UINT32 Tm3OfEn:1;
	UINT32 Tm1UpDown:1;
	UINT32 Tm2UpDown:1;
	UINT32 Tm3UpDown:1;	
        UINT8 dummy;	//ycmo stupid work around: just avoid byte access
} TimerCRType;

typedef struct
{	
	UINT32 MTm1Match1:1;
	UINT32 MTm1Match2:1;
	UINT32 MTm1Overflow:1;
	UINT32 MTm2Match1:1;
	UINT32 MTm2Match2:1;
	UINT32 MTm2Overflow:1;
	UINT32 MTm3Match1:1;
	UINT32 MTm3Match2:1;
	UINT32 MTm3Overflow:1;
	UINT8 dummy;	//ycmo stupid work around: just avoid byte access
} TimerMaskType;

typedef struct
{	
	UINT32 Tm1Match1:1;
	UINT32 Tm1Match2:1;
	UINT32 Tm1Overflow:1;
	UINT32 Tm2Match1:1;
	UINT32 Tm2Match2:1;
	UINT32 Tm2Overflow:1;
	UINT32 Tm3Match1:1;
	UINT32 Tm3Match2:1;
	UINT32 Tm3Overflow:1;
	UINT8 dummy;	//ycmo stupid work around: just avoid byte access
} TimerIntrStateType;

typedef struct 
{
	UINT32 IntNum;		/* interrupt number */
	UINT32 Handler;	/* interrupt Routine */
	UINT32 Tick;		/* Tick Per Second */
	UINT32 Running;		/* Is timer running */       
} TimerStructType;

//Global Variable
#define timer_base TMR_FTPWMTMR010_0_PA_BASE
UINT32 gTimer_Vectors[MAX_TIMER + 1] = {0,TMR_FTTMR010_0_1_IRQ, TMR_FTTMR010_0_2_IRQ, TMR_FTTMR010_0_3_IRQ/*, TMR_FTTMR010_0_3_IRQ*/};
UINT32 gTimerBase[] ={0, TMR_FTPWMTMR010_0_PA_BASE, TMR_FTPWMTMR010_0_PA_BASE+0x10, TMR_FTPWMTMR010_0_PA_BASE+0x20 };

//#define timer_base TMR_FTPWMTMR010_1_PA_BASE
//UINT32 gTimer_Vectors[MAX_TIMER + 1] = {0,TMR_FTTMR010_1_1_IRQ, TMR_FTTMR010_1_2_IRQ, TMR_FTTMR010_1_3_IRQ/*, TMR_FTTMR010_0_3_IRQ*/};
//UINT32 gTimerBase[] ={0, TMR_FTPWMTMR010_1_PA_BASE, TMR_FTPWMTMR010_1_PA_BASE+0x10, TMR_FTPWMTMR010_1_PA_BASE+0x20 };


TimerStructType gFtimer[MAX_TIMER+1];
UINT32 gT1_Tick = 0,gT2_Tick = 0, gT3_Tick = 0;

//funtion prototype
void Timer_AutoReloadValue(tmr, UINT32);
void  ResetAllTimer(void);
INT32 GetTick(tmr);
INT32 SetTick(tmr, UINT32);
INT32 SetClkSource(tmr, UINT32);
INT32 Timer_SetIsr(tmr, UINT32);
INT32 Clear_IntrStateRegister(tmr);
INT32 Timer_IOCtrl(timeriotype, tmr, UINT32);


typedef void (*FNTmrISR)(void);

// ============  Function Definition  =================
void Timer_AutoReloadValue(tmr timer, UINT32 value)
{
    //writel(value, (UINT32*)(uintptr_t)(gTimerBase[timer] + TIMER_LOAD));
    outw((UINT32*)(uintptr_t)(gTimerBase[timer] + TIMER_LOAD), value);
}

void ResetAllTimer(void)
{
    /* Start-up routine to initialise the timers to a known state */
    //UINT32 i;

    //reset all timer to default value
    //for (i = 1; i <= MAX_TIMER; i++)
    //    Timer_Disable(i);
        Timer_Disable(TMR1);
        Timer_Disable(TMR2);
        Timer_Disable(TMR3);
}

INT32 GetTick(tmr timer)
{
    UINT32 cur_tick;

    volatile TimerStructType *ctimer = &gFtimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;

    cur_tick=ctimer->Tick;

    return cur_tick;
}

INT32 SetTick(tmr timer,UINT32 clk_tick)
{
    volatile TimerStructType *ctimer = &gFtimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;

    ctimer->Tick=clk_tick;

    return TRUE;
}

INT32 SetClkSource(tmr timer,UINT32 clk)
{
    volatile TimerCRType *TimerControl=(TimerCRType *)(timer_base + TIMER_CR);


    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;

    switch(timer)
    {
    case 1:
        TimerControl->Tm1Clock=clk;
        break;
    case 2:
        TimerControl->Tm2Clock=clk;
        break;
    case 3:
        TimerControl->Tm3Clock=clk;
        break;

    default:
        break;
  }

   return TRUE;
}

INT32 Timer_SetIsr(tmr timer, UINT32 handler)
{
    /* Routine to initialise install requested timer. Stops the timer. */
    TimerStructType *ctimer=&gFtimer[timer];
    UINT32 i;

    i = gTimer_Vectors[timer];

    //fLib_CloseIRQ(i);

    //if (!fLib_ConnectIRQ(i, handler))
    //    return FALSE;
	
		NVIC_SetVector((IRQn_Type) i, handler);

    ctimer->Handler = handler;
    ctimer->IntNum = i;     /* INT number */

    return timer;
}

INT32 Clear_IntrStateRegister(tmr timer)
{
/*
When a interrupt whose interrupt status register = 7(overflow+match1+match2),
the first comparison(TimerIntrState->Tm1Overflow == 1) turns out true.
It seems we only clear the Tm1Overflow bit(TimerIntrState->Tm1Overflow=1;),
but actually we also clear the Tm1Match1 & Tm1Match2.
This is tricky. With this trick, we can clear intrstatus register this way.
*/
    volatile TimerIntrStateType *TimerIntrState = (TimerIntrStateType *)(timer_base + TIMER_INTSTATE);
    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;

    switch(timer)
    {
        case 1:
            if (TimerIntrState->Tm1Overflow == 1)
                TimerIntrState->Tm1Overflow=1;
            else if (TimerIntrState->Tm1Match1 == 1)
                TimerIntrState->Tm1Match1=1;
            else if (TimerIntrState->Tm1Match2 == 1)
                TimerIntrState->Tm1Match2=1;
            break;
        case 2:
        if (TimerIntrState->Tm2Overflow == 1)
                TimerIntrState->Tm2Overflow=1;
        else if (TimerIntrState->Tm2Match1 == 1)
                TimerIntrState->Tm2Match1=1;
        else if (TimerIntrState->Tm2Match2 == 1)
                TimerIntrState->Tm2Match2=1;
        break;
        
        case 3:
            if (TimerIntrState->Tm3Overflow == 1)
                TimerIntrState->Tm3Overflow=1;
            else if (TimerIntrState->Tm3Match1 == 1)
                TimerIntrState->Tm3Match1=1;
            else if (TimerIntrState->Tm3Match2 == 1)
                TimerIntrState->Tm3Match2=1;
        break;

    default:
        break;

    }

    return TRUE;
}

/* Routine to disable a timer and free-up the associated IRQ */
INT32 Timer_Close(tmr timer)
{
    UINT32 i;

    if (timer == 0 || timer > MAX_TIMER)
        return FALSE;

    if(!Timer_Disable(timer))
        return FALSE;   /* Stop the timer first */

    i = gTimer_Vectors[timer];  /* then release the IRQ */
    
		//if (!fLib_CloseIRQ(i))
        //return FALSE;
		NVIC_DisableIRQ((IRQn_Type)i);

    return TRUE;
}


INT32 Timer_IOCtrl(timeriotype IoType,tmr timer,UINT32 tick)
{

    switch(IoType)
    {
        case IO_TIMER_RESETALL:
            ResetAllTimer();
        break;
        
        case IO_TIMER_GETTICK:
            GetTick(timer);
        break;
        
        case IO_TIMER_SETTICK:
            SetTick(timer,tick);
        break;
        
        case IO_TIMER_SETCLKSRC:
            SetClkSource(timer,tick);
        break;
        
        default:
        return FALSE;
    }

    return TRUE;
}

BOOL Timer_Init(tmr timer, UINT32 tick, UINT32 handler)
{
    /* Routine to start the specified timer & enable the interrupt */
    int intNum = gTimer_Vectors[timer];
	//dbg_msg("%s timer=%d,irq=%d\n",__func__, timer, intNum);
    switch(timer)
    {
        case TMR1:
        gT1_Tick = 0;
        break;
        
        case TMR2:
        gT2_Tick = 0;
        break;
        
        case TMR3:
        gT3_Tick = 0;
        break;

        default:
        break;
    }
 
    if (timer == 0 || timer > MAX_TIMER)
        return FALSE;

    /* Set the timer tick */
    if(!Timer_IOCtrl(IO_TIMER_SETTICK, timer, tick))
        return FALSE;

    Clear_IntrStateRegister(timer);

    //connect timer ISR
    if(!Timer_SetIsr(timer, handler))
        return FALSE;

    /* Install timer interrupt routine */
    //fLib_SetIRQmode((UINT32)intNum, EDGE);
    //fLib_EnableIRQ((UINT32)intNum);
		NVIC_EnableIRQ((IRQn_Type)intNum);

    if (!Timer_IntEnable(timer))
	return FALSE;

    /* Start the timer ticking */
    if(!Timer_Enable(timer))
        return FALSE;

    return TRUE;
}


/* This routine starts the specified timer hardware. */
INT32 Timer_IntEnable(tmr timer)
{
    volatile TimerMaskType *TimerMask = (TimerMaskType *)(timer_base + TIMER_INTMASK);

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;

    TimerMask->MTm1Overflow=0;
    TimerMask->MTm1Match1=1;
    TimerMask->MTm1Match2=1;
    TimerMask->MTm2Overflow=0;
    TimerMask->MTm2Match1=0;
    TimerMask->MTm2Match2=0;
    TimerMask->MTm3Overflow=0;
    TimerMask->MTm3Match1=0;
    TimerMask->MTm3Match2=0;

    //enable timer
    switch(timer)
    {
    case 1:
        TimerMask->MTm1Overflow=0;
        TimerMask->MTm1Match1=0;
        TimerMask->MTm1Match2=0;
        break;
    case 2:
        TimerMask->MTm2Overflow=0;
        TimerMask->MTm2Match1=0;
        TimerMask->MTm2Match2=0;
        break;
    case 3:
        TimerMask->MTm3Overflow=0;
        TimerMask->MTm3Match1=0;
        TimerMask->MTm3Match2=0;
        break;

    default:
        break;

    }

    return TRUE;
}

INT32 Write_MR(tmr timer, int match1, int match2, int overflow)
{
	volatile TimerMaskType *TimerMask = (TimerMaskType *)(timer_base + TIMER_INTMASK);
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	TimerMask -> MTm1Overflow = 0;
	TimerMask -> MTm1Match1 = 0;
	TimerMask -> MTm1Match2 = 0;
	TimerMask -> MTm2Overflow = 0;
	TimerMask -> MTm2Match1 = 0;
	TimerMask -> MTm2Match2 = 0;
	TimerMask -> MTm3Overflow = 0;
	TimerMask -> MTm3Match1 = 0;
	TimerMask -> MTm3Match2 = 0;
    
	switch(timer)
	{
		case 1:
			TimerMask -> MTm1Overflow = overflow;
			TimerMask -> MTm1Match1 = match1;
			TimerMask -> MTm1Match2 = match2;
			break;
		case 2:
			TimerMask -> MTm2Overflow = overflow;
			TimerMask -> MTm2Match1 = match1;
			TimerMask -> MTm2Match2 = match2;
			break;
		case 3:
			TimerMask -> MTm3Overflow = overflow;
			TimerMask -> MTm3Match1 = match1;
			TimerMask -> MTm3Match2 = match2;
			break;

		default:
			break;
	}

	return TRUE;
}

INT32 Write_CR(tmr timer, UINT32 source, UINT32 upDown)
{
	volatile TimerCRType *TimerControl=(TimerCRType *)(timer_base + TIMER_CR);
	if ((timer == 0) || (timer > MAX_TIMER))
		return FALSE;

	switch(timer)
	{
		case TMR1:
			TimerControl -> Tm1Clock = source;
			TimerControl -> Tm1OfEn = 1;
			TimerControl -> Tm1UpDown = upDown;
			TimerControl -> Tm1En = 1;
			break;
		case TMR2:
			TimerControl -> Tm2Clock = source;
			TimerControl -> Tm2OfEn = 1;
			TimerControl -> Tm2UpDown = upDown;
			TimerControl -> Tm2En = 1;
			break;
		case TMR3:
			TimerControl -> Tm3Clock = source;
			TimerControl -> Tm3OfEn = 1;
			TimerControl -> Tm3UpDown = upDown;
			TimerControl -> Tm3En = 1;
			break;
		default:
			break;
	}

	return TRUE;
}

INT32 Timer_Enable(tmr timer)
{
    /* This routine starts the specified timer hardware. */
    TimerStructType *ctimer=&gFtimer[timer];

    if ((timer == 0) || (timer > MAX_TIMER))
        return FALSE;

    if(ctimer->Running == TRUE)
    {
//        printf("Timer is running.\r\n");
	return FALSE;
    }

    ctimer->Running=TRUE;
    return TRUE;
}


INT32 Timer_Disable(tmr timer)
{
    /* This routine stops the specified timer hardware. */
    volatile TimerCRType *TimerControl=(TimerCRType *)(timer_base + TIMER_CR);

    TimerStructType *ctimer=&gFtimer[timer];


    if ((timer == 0) || (timer > MAX_TIMER))
    {
        return FALSE;
    }

    /* Disable the Control register bit */
    switch(timer)
    {
    case TMR1:
        TimerControl->Tm1En=0;
        TimerControl->Tm1OfEn=0;
        break;
    case TMR2:
        TimerControl->Tm2En=0;
        TimerControl->Tm2OfEn=0;
        break;
    case TMR3:
        TimerControl->Tm3En=0;
        TimerControl->Tm3OfEn=0;
        break;

    default:
        break;
    }

    //set the timer status=false
    ctimer->Running=FALSE;

    return TRUE;
}

UINT32 Timer_Counter(tmr timer, UINT32 value)
{
    if (value)
        outw((UINT32*)(uintptr_t)(gTimerBase[timer] + TIMER_RegCOUNT), value);

    return readl((UINT32*)(uintptr_t)(gTimerBase[timer] + TIMER_RegCOUNT));
}

void Timer_MatchValue1(tmr timer, UINT32 value)
{
    outw((UINT32*)(uintptr_t)(gTimerBase[timer] + TIMER_MATCH1), value);
}

void Timer_MatchValue2(tmr timer, UINT32 value)
{
    outw((UINT32*)(uintptr_t)(gTimerBase[timer] + TIMER_MATCH2), value);
}

/* The reason why we cannot merge TickTimer1 2 3 into TickIsr() is that
 * timer isr only receives a variable "IRQ num", but the timer
 * num is also required to determine which gT_Tick to ++. So we have to
 * have a isr for each timer.
*/

void TickTimer1()
{
	//fLib_ClearIRQ(TMR_FTTMR010_0_IRQ);
	gT1_Tick++;
	Clear_IntrStateRegister(TMR1);
    //dbg_msg("gT1_Tick=%d", gT1_Tick);
}

void TickTimer2()
{
    //fLib_ClearIRQ(TMR_FTTMR010_0_1_IRQ);
    gT2_Tick++;
	Clear_IntrStateRegister(TMR2);
    //dbg_msg("gT2_Tick=%d", gT2_Tick);
}

void TickTimer3()
{
	//fLib_ClearIRQ(TMR_FTTMR010_0_2_IRQ);
	gT3_Tick++;
	Clear_IntrStateRegister(TMR3);
    //dbg_msg("gT3_Tick=%d", gT3_Tick);
}

/* The following three isr are for one shot test */

void Tick_Timer1_One_Shot()
{
    gT1_Tick++;
    Timer_Counter(TMR1, 0xffffffff/2);
    //fLib_ClearIRQ(TMR_FTTMR010_0_IRQ);
    Clear_IntrStateRegister(TMR1);
}

void Tick_Timer2_One_Shot()
{
    gT2_Tick++;
    Timer_Counter(TMR2, 0xffffffff/2); 
    //fLib_ClearIRQ(TMR_FTTMR010_0_1_IRQ);
    Clear_IntrStateRegister(TMR2);
}

void Tick_Timer3_One_Shot()
{
    gT3_Tick++;
    Timer_Counter(TMR3, 0xffffffff/2); 
    //fLib_ClearIRQ(TMR_FTTMR010_0_2_IRQ);
    Clear_IntrStateRegister(TMR3);
}

UINT32 GetCurrentT1Tick(void)
{
    return gT1_Tick;
}

UINT32 GetCurrentT2Tick(void)
{
    return gT2_Tick;
}

UINT32 GetCurrentT3Tick(void)
{
    return gT3_Tick;
}

BOOL kdp_tmr_init(tmr timer, u32 tick)
{
    UINT32 tmr_irq;
    FNTmrISR isr;
    
    if (timer == 0 || timer > MAX_TIMER)
        return FALSE;

    switch(timer)
    {
        case TMR1:
            gT1_Tick = 0;
            tmr_irq = TMR_FTTMR010_0_1_IRQ;		
            isr = TickTimer1;
        break;
        case TMR2:
            gT2_Tick = 0;
            tmr_irq = TMR_FTTMR010_0_2_IRQ;		
            isr = TickTimer2;
        break;
        case TMR3:
            gT3_Tick = 0;
            tmr_irq = TMR_FTTMR010_0_3_IRQ;		
            isr = TickTimer3;
        break;
        default:
            return FALSE;
    }

    Timer_Init(timer, 1, (UINT32)isr);
    Timer_Counter(timer, 0);
    Timer_AutoReloadValue(timer, tick);
    Write_CR(timer, PCLK, DOWN);
    
    NVIC_SetVector((IRQn_Type)tmr_irq, (uint32_t)isr);    
    NVIC_EnableIRQ((IRQn_Type)tmr_irq);    
    
    
    return TRUE;
}
