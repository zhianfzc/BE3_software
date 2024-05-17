/*
 * Kneron Power Manager driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
 
#include <string.h>
#include "cmsis_os2.h"
#include "os_tick.h"
#include "RTX_Config.h"
#include "power_manager.h"
#include "scu_reg.h"
#include "power.h"
#include "clock.h"
#include "system.h"
#include "rtc.h"
#include "dbg.h"
#include "base.h"

#define REDUCE_CODE_SIZE

#define FLAG_SYSTEM_RESET       BIT0
#define FLAG_SYSTEM_NAP         BIT1
#define FLAG_SYSTEM_NAP2        BIT2
#define FLAG_SYSTEM_SLEEP       BIT3
#define FLAG_SYSTEM_DEEP_SLEEP  BIT4
#define FLAG_SYSTEM_TIMER       BIT5
#define FLAG_SYSTEM_SHUTDOWN    BIT6
#define FLAG_SYSTEM_ERROR       BIT8
#define FLAG_SYSTEM_PWRBTN_FALL BIT16
#define FLAG_SYSTEM_PWRBTN_RISE BIT17

#define FLAGS_ALL       (FLAG_SYSTEM_RESET | FLAG_SYSTEM_SHUTDOWN \
                        | FLAG_SYSTEM_NAP | FLAG_SYSTEM_NAP2 \
                        | FLAG_SYSTEM_SLEEP | FLAG_SYSTEM_DEEP_SLEEP \
                        | FLAG_SYSTEM_TIMER | FLAG_SYSTEM_ERROR \
                        | FLAG_SYSTEM_PWRBTN_FALL | FLAG_SYSTEM_PWRBTN_RISE)

#define PERIOD_PRINT        (3 * OS_TICK_FREQ)                  // 3 secs
#define PERIOD_COUNT        (PERIOD_PRINT + PERIOD_PRINT/100)   // add 1% margin

/* Inactivity timers in seconds */
#define NAP_TIME_1              30
#define NAP_TIME_2              60

osThreadId_t    power_tid;
uint32_t cpu_idle_counter = 0;

uint32_t idle_entry_time_in_secs;
uint32_t sleep_state;

struct pm_device_func_s {
    enum pm_device_id   dev_id;
    int                 inuse;
    struct pm_s         pm;
} pm_dev_fns[PM_DEVICE_MAX];

//static void scu_system_isr(void)
//{
//    static int pwr_button_wakeup = 1;   // = 1 for cold boot by PWR button
//    uint32_t status;

//    status = inw(SCU_REG_INT_STS);

//    if (status & SCU_INT_RTC_ALARM)
//        osThreadFlagsSet(power_tid, FLAG_SYSTEM_NAP);
//    if (status & SCU_INT_RTC_PERIODIC)
//        osThreadFlagsSet(power_tid, FLAG_SYSTEM_TIMER);
//    if (status & SCU_INT_PWRBTN_FALL) {
//        if (pwr_button_wakeup) {
//            pwr_button_wakeup = 0;
//        } else {
//            osThreadFlagsSet(power_tid, FLAG_SYSTEM_PWRBTN_FALL);
//        }
//    }
//    if (status & SCU_INT_PWRBTN_RISE) {
//        if (sleep_state == 1) {
//            pwr_button_wakeup = 1;
//        } else {
//            pwr_button_wakeup = 0;
//            osThreadFlagsSet(power_tid, FLAG_SYSTEM_PWRBTN_RISE);
//        }
//    }

//    outw(SCU_REG_INT_STS, status);
//    NVIC_ClearPendingIRQ(SYS_SYSTEM_IRQ);
//}

//static void scu_system_init(void)
//{
//    NVIC_DisableIRQ(SYS_SYSTEM_IRQ);

//    rtc_init(NULL, NULL);
//    outw(SCU_REG_INT_STS, 0xffffffff);  // Clear all old ones

//    /* Enable PWR button interrupt and wakeup */
//    outw(SCU_REG_INT_EN, SCU_INT_PWRBTN_FALL | SCU_INT_PWRBTN_RISE | SCU_INT_WAKEUP);

//    /* Enable nap alarm interrupt */
//    uint32_t nap_time = NAP_TIME_1;
//    rtc_alarm_enable(ALARM_IN_SECS, &nap_time, NULL);
//    masked_outw(SCU_REG_INT_EN, SCU_INT_RTC_ALARM, SCU_INT_RTC_ALARM);

//    NVIC_SetVector(SYS_SYSTEM_IRQ, (uint32_t)scu_system_isr);
//    NVIC_EnableIRQ(SYS_SYSTEM_IRQ);
//}

#define MMFAR 0xE000ED34
#define FLAG_WAIT_FOREVER 0x40000000
static void scpu_wait_reset(void);

static void scpu_wait_reset(void)
{
    osThreadId_t calling_tid = osThreadGetId();
    if ((calling_tid == 0) || (calling_tid == power_tid)){  // no os or if power mgmnt thread is in trouble
#if 0
            power_mgr_sw_reset();
#else
            for (;;);
#endif
    }
    else  // let power mgmnt thread handles the reset
        osThreadFlagsWait((u32)calling_tid , FLAG_WAIT_FOREVER, osWaitForever);
}

register unsigned int _msp __asm("msp");
register unsigned int _psp __asm("psp");
register unsigned int _lr  __asm("lr");
static unsigned int stack, pc;

static void scpu_hard_fault(void)
{
    if (_lr & 4) {
        stack = _psp;
        pc = stack + 24;
    }
    else {
        stack = _msp;
        pc = stack + 40;
    }

    err_msg("scpu: hard fault @ %08X, PC = %08X, LR = %08X, SP = %08X\n", *(u32*)MMFAR,
        *(u32*)pc, *(u32*)(pc-4), (u32)pc+8);
    if (power_tid)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_ERROR);
    *(u32*)pc = (u32)&scpu_wait_reset;  // modify stack to go to the wait forever loop
}

static void scpu_mem_mnmt(void)
{
    if (_lr & 4) {
        stack = _psp;
        pc = stack + 24;
    }
    else {
        stack = _msp;
        pc = stack + 40;
    }

    err_msg("scpu: memory fault @ %08X, PC = %08X, LR = %08X, SP = %08X\n", *(u32*)MMFAR,
        *(u32*)pc, *(u32*)(pc-4), (u32)pc+8);
    if (power_tid)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_ERROR);
    *(u32*)pc = (u32)&scpu_wait_reset;  // modify stack to go to the wait forever loop
}

static void scpu_bus_fault(void)
{
    err_msg("scpu: scpu_bus_fault !\n");
    if (power_tid)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_ERROR);
}

static void scpu_usage_fault(void)
{
    err_msg("scpu: scpu_usage_fault !\n");
    if (power_tid)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_ERROR);
}

void power_manager_error_notify(uint32_t code, void *object_id)
{
    err_msg("scpu: exception: code=%d, object_id=0x%p\n", code, object_id);
    if (power_tid)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_ERROR);
}

void power_manager_reset(void)
{
    if (power_tid)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_RESET);
}

void power_manager_sleep(void)
{
    if (power_tid)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_SLEEP);
}

void power_manager_deep_sleep(void)
{
    if (power_tid)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_DEEP_SLEEP);
}

void power_manager_shutdown(void)
{
    if (power_tid)
        osThreadFlagsSet(power_tid, FLAG_SYSTEM_SHUTDOWN);
}

#ifndef REDUCE_CODE_SIZE
static void power_mgr_cpu_usage(void)
{
    static uint32_t last_record=0, diff;

    diff = (cpu_idle_counter - last_record);
    last_record = cpu_idle_counter;

    if (diff > PERIOD_COUNT)
        diff = PERIOD_COUNT;

    info_msg("#%04d cpu loading %d %%\n", ++print_count,
            (PERIOD_COUNT - diff) * 100 / PERIOD_COUNT);
}
#endif

__NO_RETURN void power_manager_cpu_idle(void)
{
    uint32_t tick_start, tick_end, tick_idle;

    while(1) {
        rtc_get_date_time_in_secs(&idle_entry_time_in_secs);
        tick_start = osKernelGetTickCount();
        __WFI();
        tick_end = osKernelGetTickCount();
        tick_idle = tick_end - tick_start;
        cpu_idle_counter += tick_idle;
    }
}

#ifndef REDUCE_CODE_SIZE
static void power_manager_do_nap(void)
{
    int i;

    for (i = PM_DEVICE_MAX - 1; i >= 0; i--) {
        if (pm_dev_fns[i].inuse && pm_dev_fns[i].pm.nap)
        {
            if(pm_dev_fns[i].pm.nap(pm_dev_fns[i].dev_id) < 0)
            {
                err_msg("Can't take a nap\n");
                return;
            }
        }
    }
    err_msg("Take a nap\n");
    /* Disable npu/ncpu clocks */
    rtc_get_date_time_in_secs(&idle_entry_time_in_secs);
    clk_disable(CLK_NPU);
    clk_disable(CLK_NCPU);
    __WFI();
    clk_enable(CLK_NCPU);
    clk_enable(CLK_NPU);

    for (i = 0; i < PM_DEVICE_MAX; i++) {
        if (pm_dev_fns[i].inuse && pm_dev_fns[i].pm.wakeup_nap)
            pm_dev_fns[i].pm.wakeup_nap(pm_dev_fns[i].dev_id);
    }
}

static void power_manager_do_deep_nap(void)
{
    int i;

    for (i = PM_DEVICE_MAX - 1; i >= 0; i--) {
        if (pm_dev_fns[i].inuse && pm_dev_fns[i].pm.deep_nap)
        {
            if(pm_dev_fns[i].pm.deep_nap(pm_dev_fns[i].dev_id) < 0)
            {
                err_msg("Can't take a deep nap\n");
                return;
            }
        }
    }
    err_msg("Take a deep nap\n");
    /* Disable npu/ncpu clocks + DDR self refresh */
    rtc_get_date_time_in_secs(&idle_entry_time_in_secs);
    clk_disable(CLK_NPU);
    clk_disable(CLK_NCPU);
    ddr_self_refresh_enter();
    __WFI();
    ddr_self_refresh_exit();
    clk_enable(CLK_NCPU);
    clk_enable(CLK_NPU);

    for (i = 0; i < PM_DEVICE_MAX; i++) {
        if (pm_dev_fns[i].inuse && pm_dev_fns[i].pm.wakeup_deep_nap)
            pm_dev_fns[i].pm.wakeup_deep_nap(pm_dev_fns[i].dev_id);
    }
}

static void power_manager_do_sleep(void)
{
    int i;

    for (i = PM_DEVICE_MAX - 1; i >= 0; i--) {
        if (pm_dev_fns[i].inuse && pm_dev_fns[i].pm.sleep)
        {
            if(pm_dev_fns[i].pm.sleep(pm_dev_fns[i].dev_id) < 0)
            {
                err_msg("Can't sleep\n");
                return;
            }
        }
    }
    err_msg("!!! sleep\n");

    rtc_get_date_time_in_secs(&idle_entry_time_in_secs);
    sleep_state = 1;

    /* Retention: NPU power domain off */
    clk_disable(CLK_NPU);
    clk_disable(CLK_NCPU);
    ddr_self_refresh_enter();
    power_mgr_set_domain(POWER_DOMAIN_NPU, 0);
    __WFI();
    power_mgr_set_domain(POWER_DOMAIN_NPU, 1);
    ddr_self_refresh_exit();
    clk_enable(CLK_NCPU);
    clk_enable(CLK_NPU);
    /* TODO: reload ncpu fw to NiRAM */
    system_wakeup_ncpu(1);

    sleep_state = 0;
    err_msg("!!! sleep -> wakeup: (TODO: reload NCPU FW)\n");

    for (i = 0; i < PM_DEVICE_MAX; i++) {
        if (pm_dev_fns[i].inuse && pm_dev_fns[i].pm.wakeup_sleep)
            pm_dev_fns[i].pm.wakeup_sleep(pm_dev_fns[i].dev_id);
    }
}

static void power_manager_do_deep_sleep(void)
{
    int i;

    for (i = PM_DEVICE_MAX - 1; i >= 0; i--) {
        if (pm_dev_fns[i].inuse && pm_dev_fns[i].pm.deep_sleep)
        {
            if(pm_dev_fns[i].pm.deep_sleep(pm_dev_fns[i].dev_id) < 0)
            {
                err_msg("Can't deep sleep\n");
                return;
            }
        }
    }

    err_msg("!!! deep sleep\n\n");

    /* Deep Retention: NPU+Default domain off */
    power_mgr_softoff(POWER_MGR_MODE_DEEP_RETENTION);
    __WFI();

    err_msg("!!! deep sleep failed!\n");

    /* TODO: resume hear */
    for (i = 0; i < PM_DEVICE_MAX; i++) {
        if (pm_dev_fns[i].inuse && pm_dev_fns[i].pm.wakeup_deep_sleep)
            pm_dev_fns[i].pm.wakeup_deep_sleep(pm_dev_fns[i].dev_id);
    }
}
#endif

static void power_manager_do_shutdown(void)
{
    power_mgr_set_domain(POWER_DOMAIN_NPU, 0);

    err_msg("!!! shutdown ...\n\n");

    /* Disable alarm */
    rtc_alarm_disable(ALARM_IN_SECS);

    /* Power off everything except RTC */
    power_mgr_softoff(POWER_MGR_MODE_RTC);
    __WFI();

    err_msg("!!! shutdown failed!\n");
    for (;;);
}

//#define PRINT_CPU_USAGE

void power_manager_thread(void *arg)
{
    uint32_t status, timeout;
    uint32_t current_time, elapsed_time, nap_time, pwrbtn_press_time, pwrbtn_release_time;
    uint32_t last_power_manager_time;

    /* Init system/power/rtc control on SCU */
    // scu_system_init();

#ifdef PRINT_CPU_USAGE
    timeout = PERIOD_PRINT;
#else
    timeout = osWaitForever;
#endif

    /* init last_power_manager_time */
    rtc_get_date_time_in_secs(&last_power_manager_time);

    while(1)
    {
        status = osThreadFlagsWait(FLAGS_ALL, osFlagsWaitAny, timeout);

        /* update last_power_manager_time */
        rtc_get_date_time_in_secs(&last_power_manager_time);

#ifndef REDUCE_CODE_SIZE
        if (status == osFlagsErrorTimeout) {
            power_mgr_cpu_usage();
            continue;
        }

        if (status & FLAG_SYSTEM_SLEEP) {
            power_manager_do_sleep();
        }

        if (status & FLAG_SYSTEM_DEEP_SLEEP) {
            power_manager_do_deep_sleep();
        }
#endif

        if (status & FLAG_SYSTEM_RESET) {
            err_msg("!!! reset\r\n");
            // will not come back
            power_mgr_sw_reset();
        }

        if (status & FLAG_SYSTEM_SHUTDOWN) {
            // will not come back
            power_manager_do_shutdown();
        }

        if (status & FLAG_SYSTEM_NAP) {
            rtc_get_date_time_in_secs(&current_time);

            if (last_power_manager_time < idle_entry_time_in_secs)
                // idle time is newer
                elapsed_time = current_time - idle_entry_time_in_secs;
            else
                // last idle is old = not idle much or hijacked
                elapsed_time = 0;

            /* set next alarm */
            if (elapsed_time < NAP_TIME_1) {
                nap_time = NAP_TIME_1;
                rtc_alarm_enable(ALARM_IN_SECS, &nap_time, NULL);
            }
#ifndef REDUCE_CODE_SIZE
            else if (elapsed_time < NAP_TIME_2) {
                rtc_current_time_info();
                err_msg("Idle: %d seconds -> nap\n", elapsed_time);
                /* Set longer nap time */
                nap_time = NAP_TIME_2;
                rtc_alarm_enable(ALARM_IN_SECS, &nap_time, NULL);
                /* Take nap */
                power_manager_do_nap();
                /* regular nap time upon wakeup */
                nap_time = NAP_TIME_1;
                rtc_alarm_enable(ALARM_IN_SECS, &nap_time, NULL);
            } else {
                rtc_current_time_info();
                err_msg("Idle: %d seconds -> deep nap\n", elapsed_time);
                /* Set even longer nap time */
                nap_time = NAP_TIME_2 * 10;
                rtc_alarm_enable(ALARM_IN_SECS, &nap_time, NULL);
                /* Take deep nap */
                power_manager_do_deep_nap();
                /* regular nap time upon wakeup */
                nap_time = NAP_TIME_1;
                rtc_alarm_enable(ALARM_IN_SECS, &nap_time, NULL);
            }
#endif
        }

        if (status & FLAG_SYSTEM_TIMER) {
            rtc_current_time_info();

            rtc_get_date_time_in_secs(&current_time);
            elapsed_time = current_time - idle_entry_time_in_secs;

            //dbg_msg("Idle: %d\n", elapsed_time);
        }

        if (status & FLAG_SYSTEM_ERROR) {
            err_msg("!!! scpu: error\n");
#if 1
            power_mgr_sw_reset();
#else
            // for debug
            for (;;);
#endif
        }

        if (status & FLAG_SYSTEM_PWRBTN_FALL) {
            rtc_get_date_time_in_secs(&pwrbtn_release_time);
            elapsed_time = pwrbtn_release_time - pwrbtn_press_time;
            err_msg("!!! PWR Button pressed for %d seconds:\n", elapsed_time);
            if (elapsed_time > 6)
                power_manager_do_shutdown();
        }

        if (status & FLAG_SYSTEM_PWRBTN_RISE) {
            rtc_get_date_time_in_secs(&pwrbtn_press_time);
            err_msg("!!! PWR Button Press&Hold 7+ seconds for shutdown (RTC mode)\n");
        }
    }
}

/* Registration APIs */
int power_manager_register(enum pm_device_id dev_id, struct pm_s *pm_p)
{
    int i;

    if (dev_id >= PM_DEVICE_MAX || pm_p == NULL)
        return -1;

    for (i = 0; i < PM_DEVICE_MAX; i++) {
        if (pm_dev_fns[i].inuse == 0) {
            memcpy(&pm_dev_fns[i].pm, pm_p, sizeof(struct pm_s));
            pm_dev_fns[i].dev_id = dev_id;
            pm_dev_fns[i].inuse = 1;
            break;
        }
    }

    return 0;
}

void power_manager_unregister(enum pm_device_id dev_id, struct pm_s *pm_p)
{
    int i;

    if (dev_id >= PM_DEVICE_MAX || pm_p == NULL)
        return;

    for (i = 0; i < PM_DEVICE_MAX; i++) {
        if (pm_dev_fns[i].dev_id == dev_id && pm_dev_fns[i].pm.sleep == pm_p->sleep && pm_dev_fns[i].inuse) {
            memset(&pm_dev_fns[i].pm, 0, sizeof(struct pm_s));
            pm_dev_fns[i].dev_id = PM_DEVICE_NONE;
            pm_dev_fns[i].inuse = 0;
            return;
        }
    }
}

void power_manager_init(void)
{
    osThreadAttr_t attr = {
        .stack_size = 512,
        .priority = osPriorityRealtime7
    };

    power_tid = osThreadNew(power_manager_thread, NULL, &attr);

    NVIC_SetVector(HardFault_IRQn, (uint32_t)scpu_hard_fault);
    NVIC_SetVector(MemoryManagement_IRQn, (uint32_t)scpu_mem_mnmt);
    NVIC_SetVector(BusFault_IRQn, (uint32_t)scpu_bus_fault);
    NVIC_SetVector(UsageFault_IRQn, (uint32_t)scpu_usage_fault);
}
