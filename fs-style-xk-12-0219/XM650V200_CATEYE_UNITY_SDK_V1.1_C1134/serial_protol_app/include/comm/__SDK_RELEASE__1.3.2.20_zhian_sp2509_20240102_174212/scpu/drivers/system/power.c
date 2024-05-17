/*
 * Kneron Power Manager Base driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
 
#include "power.h"
#include "base.h"
#include "scu_reg.h"
#include "scu_extreg.h"
#include "dbg.h"
#include "delay.h"


#define POWER_DOMAIN_WORKING_NONE       0x00000000
#define POWER_DOMAIN_WORKING_DEFAULT    0x00000001
#define POWER_DOMAIN_WORKING_NPU        0x00000002
#define POWER_DOMAIN_WORKING_DDR        0x00000004
#define POWER_DOMAIN_WORKING_ALL        (POWER_DOMAIN_WORKING_DEFAULT | POWER_DOMAIN_WORKING_NPU | POWER_DOMAIN_WORKING_DDR)

#define POWER_DOMAIN_SOFTOFF_DEFAULT    0x00000010
#define POWER_DOMAIN_SOFTOFF_NPU        0x00000020
#define POWER_DOMAIN_SOFTOFF_DDR        0x00000040

#define POWER_DOMAIN_SOFTOFF_MASK       (POWER_DOMAIN_SOFTOFF_DEFAULT | \
                                         POWER_DOMAIN_SOFTOFF_NPU | \
                                         POWER_DOMAIN_SOFTOFF_DDR)

#define PWR_CTRL_SOFTOFF_MASK   (SCU_REG_PWR_CTRL_PWRUP_UPDATE | \
                                SCU_REG_PWR_CTRL_PWRUP_CTRL_MASK | \
                                SCU_REG_PWR_CTRL_PWRDN_CTRL_MASK)

#define PWR_CTRL_SOFTOFF_DEEP_RETENTION     (SCU_REG_PWR_CTRL_PWRUP_UPDATE | \
                                            SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DDRCK | \
                                            SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DEFAULT | \
                                            SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_DDRCK)

#define PWR_CTRL_SOFTOFF_RTC_MODE           (SCU_REG_PWR_CTRL_PWRUP_UPDATE | \
                                            SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DEFAULT)

enum power_mgr_mode __power_mgr_mode = POWER_MGR_MODE_RTC;

void power_mgr_sw_reset(void)
{
#if 1
    err_msg("Set watchdog reset\n");

    outw(WDT_FTWDT010_PA_BASE+0x0C, 0);
    outw(WDT_FTWDT010_PA_BASE+0x04, 1000);
    outw(WDT_FTWDT010_PA_BASE+0x0C, 0x03); // system reset
    outw(WDT_FTWDT010_PA_BASE+0x08, 0x5AB9);

    __WFI();
#else	
    masked_outw(SCU_REG_PWR_MOD, SCU_REG_PWR_MOD_SW_RST, SCU_REG_PWR_MOD_SW_RST);
#endif		
}

void power_mgr_set_domain_2(u32 types, BOOL enable)
{
    volatile u32 val = 0;
    volatile u32 mask = SCU_REG_PWR_CTRL_PWRUP_CTRL_MASK | SCU_REG_PWR_CTRL_PWRUP_UPDATE;
    if (types & POWER_DOMAIN_WORKING_DEFAULT)
        val |= SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DEFAULT;
    if (types & POWER_DOMAIN_WORKING_NPU)
        val |= SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NPU;
    if (types & POWER_DOMAIN_WORKING_DDR)
        val |= SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DDRCK;
    if (types & POWER_DOMAIN_SOFTOFF_DEFAULT)
        val |= SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_DEFAULT;
    if (types & POWER_DOMAIN_SOFTOFF_NPU)
        val |= SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_NPU;
    if (types & POWER_DOMAIN_SOFTOFF_DDR)
        val |= SCU_REG_PWR_CTRL_PWRDN_CTRL_DOMAIN_DDRCK;

    val |= (enable) ? (SCU_REG_PWR_CTRL_PWRUP_UPDATE) : (0);
    //dbg_msg("power_mgr_set_domain2 val=%x", val);
    //dbg_msg("power_mgr_set_domain2 mask=%x", mask);
    masked_outw(SCU_REG_PWR_CTRL, val, mask);
    {
        int i = 50000;
        while (--i);
    }
}

static void _power_mgr_wait_domain_ready(u32 types) 
{
    volatile u32 val = 0;
    volatile u32 vsssts = SCU_REG_PWR_VCCSTS_PWR_READY_DOMAIN_DEFAULT;
    volatile u32 miscpwr = SCU_EXTREG_MISC_PWR_RESET_RELEASE_DOMAIN_DEFAULT;
	
    if (types & POWER_DOMAIN_WORKING_DEFAULT) {
        vsssts |= SCU_REG_PWR_VCCSTS_PWR_READY_DOMAIN_DEFAULT;
        miscpwr |= SCU_EXTREG_MISC_PWR_RESET_RELEASE_DOMAIN_DEFAULT;
    }
    if (types & POWER_DOMAIN_WORKING_NPU) {
        vsssts |= SCU_REG_PWR_VCCSTS_PWR_READY_DOMAIN_NPU;
        miscpwr |= SCU_EXTREG_MISC_PWR_RESET_RELEASE_DOMAIN_NPU;
    }
    if (types & POWER_DOMAIN_WORKING_DDR) {
        vsssts |= SCU_REG_PWR_VCCSTS_PWR_READY_DOMAIN_DDRCK;
        miscpwr |= SCU_EXTREG_MISC_PWR_RESET_RELEASE_DOMAIN_DDRCK;
    }
    //dbg_msg("power_mgr_wait_domain_ready2 vsssts=%x", vsssts);
    //dbg_msg("power_mgr_wait_domain_ready2 miscpwr=%x", miscpwr);

    do {
        val = inw(SCU_REG_PWR_VCCSTS);
    } while((val & SCU_REG_PWR_VCCSTS_PWR_READY_DOMAIN_MASK) != vsssts);
    
    do {
        val = inw(SCU_EXTREG_MISC);
    } while((val & SCU_EXTREG_MISC_PWR_RESET_RELEASE_DOMAIN_MASK) != miscpwr);
}

void power_mgr_set_mode(enum power_mgr_mode mode) 
{
    BOOL update_flag = TRUE;
    u32 ctrl_types;
    u32 wait_types;
    if (mode != __power_mgr_mode) {
        switch (mode) {
        case POWER_MGR_MODE_RTC:
            ctrl_types = POWER_DOMAIN_WORKING_NONE;
            break;
        case POWER_MGR_MODE_ALWAYSON:
            ctrl_types = POWER_DOMAIN_WORKING_DEFAULT;
            break;
        case POWER_MGR_MODE_FULL:
            ctrl_types = POWER_DOMAIN_WORKING_DEFAULT | POWER_DOMAIN_WORKING_NPU | POWER_DOMAIN_WORKING_DDR;
            break;
        case POWER_MGR_MODE_RETENTION:
            ctrl_types = POWER_DOMAIN_WORKING_DEFAULT | POWER_DOMAIN_WORKING_DDR | POWER_DOMAIN_SOFTOFF_NPU;
            break;
        case POWER_MGR_MODE_DEEP_RETENTION:
            ctrl_types = POWER_DOMAIN_WORKING_DEFAULT | POWER_DOMAIN_WORKING_DDR | POWER_DOMAIN_SOFTOFF_NPU;
            update_flag = FALSE;
            break;
        default:;
        }
        
        __power_mgr_mode = mode;
        power_mgr_set_domain_2(ctrl_types, update_flag);
        if (update_flag) {
            wait_types = ctrl_types & ~POWER_DOMAIN_SOFTOFF_MASK;
            _power_mgr_wait_domain_ready(wait_types);
        }

        if (mode == POWER_MGR_MODE_DEEP_RETENTION) {
            outw(SCU_REG_BTUP_CTRL, SCU_REG_BTUP_CTRL_RTC_BU_EN | // send power button output signal
                                    SCU_REG_BTUP_CTRL_PWRBTN_EN |
                                    SCU_REG_BTUP_CTRL_GPO_OUT);

            outw(SCU_REG_BTUP_STS, 0xffffffff);  // clear boot-up status
            outw(SCU_REG_INT_STS, 0xffffffff); // clear all interrupt status

            SCU_REG_PWR_MOD_SET_SOFTOFF(1);
            __WFI();
        }
    }

}

void power_mgr_set_domain(u32 domain, int enable)
{
    u32 val, mask, vcc_ready, misc_pwr, wait_state;

    switch (domain) {
        case POWER_DOMAIN_DDRCK:
            mask = SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DDRCK;
            vcc_ready = SCU_REG_PWR_VCCSTS_PWR_READY_DOMAIN_DDRCK;
            misc_pwr = SCU_EXTREG_MISC_PWR_RESET_RELEASE_DOMAIN_DDRCK;
            break;
        case POWER_DOMAIN_NPU:
            mask = SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_NPU;
            vcc_ready = SCU_REG_PWR_VCCSTS_PWR_READY_DOMAIN_NPU;
            misc_pwr = SCU_EXTREG_MISC_PWR_RESET_RELEASE_DOMAIN_NPU;
            break;
        case POWER_DOMAIN_DEFAULT:
            mask = SCU_REG_PWR_CTRL_PWRUP_CTRL_DOMAIN_DEFAULT;
            vcc_ready = SCU_REG_PWR_VCCSTS_PWR_READY_DOMAIN_DEFAULT;
            misc_pwr = SCU_EXTREG_MISC_PWR_RESET_RELEASE_DOMAIN_DEFAULT;
            break;
        default:
            return;
    }

    val = enable? mask : 0;
    val |= SCU_REG_PWR_CTRL_PWRUP_UPDATE;
    mask |= SCU_REG_PWR_CTRL_PWRUP_UPDATE;

    masked_outw(SCU_REG_PWR_CTRL, val, mask);
    delay_us(500);

    /* Wait for VCC change */
    wait_state = enable? vcc_ready : 0;
    do {
        val = inw(SCU_REG_PWR_VCCSTS);
    } while((val & vcc_ready) != wait_state);

    /* Wait for power reset change */
    wait_state = enable? misc_pwr : 0;
    do {
        val = inw(SCU_EXTREG_MISC);
    } while((val & misc_pwr) != wait_state);
}

void power_mgr_softoff(enum power_mgr_mode mode)
{
    u32 val;

    switch (mode) {
        case POWER_MGR_MODE_DEEP_RETENTION:
            val = PWR_CTRL_SOFTOFF_DEEP_RETENTION;
            break;
        case POWER_MGR_MODE_RTC:
            val = PWR_CTRL_SOFTOFF_RTC_MODE;
            break;
        default:
            return;
    }

    masked_outw(SCU_REG_PWR_CTRL, val, PWR_CTRL_SOFTOFF_MASK);
    SCU_REG_PWR_MOD_SET_SOFTOFF(1);
}

static void _power_mgr_ops_fcs()
{
    masked_outw(SCU_REG_PWR_MOD, 
                SCU_REG_PWR_MOD_SELFR_CMD_OFF | //scu will NOT issue the self-refresh command to DDR/SDR (in FCS)
                SCU_REG_PWR_MOD_FCS_PLL_RSTn |  //Keep PLL active in FCS
                SCU_REG_PWR_MOD_FCS,            //enter FCS
    
                SCU_REG_PWR_MOD_SELFR_CMD_OFF | SCU_REG_PWR_MOD_FCS_PLL2_RSTn |
                SCU_REG_PWR_MOD_FCS_DLL_RSTn | SCU_REG_PWR_MOD_FCS_PLL_RSTn |
                SCU_REG_PWR_MOD_FCS | SCU_REG_PWR_MOD_BUS_SPEED);
}

void power_mgr_ops(enum power_mgr_ops ops)
{   
    switch (ops) {    
    case POWER_MGR_OPS_FCS:
        _power_mgr_ops_fcs();
        break;
    case POWER_MGR_OPS_CHANGE_BUS_SPEED:
        break;
    case POWER_MGR_OPS_PLL_UPDATE:
        break;    
    case POWER_MGR_OPS_SLEEPING:
        //stop cpu 
        //stop ddr
        //send self-refresh command to ddr
        //wait ack
        break;
    default:;
    }
}

#ifdef DEBUG_POWER_MGR
#include "kdp520_pwm_timer.h"
extern void delay_ms(unsigned int msec);
void power_mgr_test(void)
{
    char ch;
    int data;

#if 0
    delay_ms(200);
    dbg_msg("switch to RTC mode!");
    power_mgr_set_mode(POWER_MGR_MODE_RTC);
    ch = kdp_getchar(DEBUG_CONSOLE);
    delay_ms(200);
#endif
    // dbg_msg("switch to DEFAULT mode!");
    // power_mgr_set_mode(POWER_MGR_MODE_ALWAYSON);
    // dbg_msg(" over");
    // ch = kdp_getchar(DEBUG_CONSOLE);	
    // delay_ms(200);
    
    // dbg_msg("switch to FULL FUNCTION!");
    // power_mgr_set_mode(POWER_MGR_MODE_FULL);
    // ddr_wakeup();
    // system_wakeup_ncpu(0, 0);
    // ddr_init(DDR_INIT_ALL_EXIT_SELF_REFRESH);
    // dbg_msg(" over");
    // ch = kdp_getchar(DEBUG_CONSOLE);
    
    // delay_ms(200);
    // dbg_msg("switch to Retention mode!");
    // power_mgr_set_mode(POWER_MGR_MODE_RETENTION);

    // dbg_msg(" over");
    // ch = kdp_getchar(DEBUG_CONSOLE);

    // dbg_msg("switch to FULL FUNCTION!");
    // power_mgr_set_mode(POWER_MGR_MODE_FULL);
    // ddr_wakeup();
    // system_wakeup_ncpu(0, 0);
    // ddr_init(DDR_INIT_ALL_EXIT_SELF_REFRESH);
    dbg_msg(" over");

    delay_ms(2000);
    dbg_msg("switch to DEEP RETENTION mode!");
    delay_ms(2000);
    power_mgr_set_mode(POWER_MGR_MODE_DEEP_RETENTION);
    ch = kdp_getchar(DEBUG_CONSOLE);

}
#endif
