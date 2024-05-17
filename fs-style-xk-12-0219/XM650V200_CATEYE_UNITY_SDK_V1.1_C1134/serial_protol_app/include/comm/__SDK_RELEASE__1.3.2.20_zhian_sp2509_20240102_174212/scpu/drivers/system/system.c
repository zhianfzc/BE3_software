#include "board_kl520.h"
#include <cmsis_os2.h>
#include <string.h>
#include "kneron_mozart.h"
#include "base.h"
#include "scu_reg.h"
#include "scu_extreg.h"
#include "power.h"
#include "clock.h"
#include "kdp520_pwm_timer.h"
#include "dbg.h"
#include "delay.h"
#include "system.h"
#include "kdp_memxfer.h"
#include "mpu.h"


enum {
    SUBSYS_NPU      = 1,
    SUBSYS_PD_NPU,
    SUBSYS_LCDC,
    SUBSYS_NCPU,
};

u32 bootup_status;
u32 warm_boot;
u32 __sys_int_flag;

#define BOOTUP_STATUS_WARM      (SCU_REG_BTUP_STS_SMR | SCU_REG_BTUP_STS_PMR2)
static const u16 pll3_setting[]=CFG_MIPI_PLL_SETTING;
//reserved function
static void sys_reset(int subsystem)
{
    switch (subsystem) {
        case SUBSYS_NPU:
            SCU_EXTREG_SWRST_SET_NPU_resetn(1);
            break;
        case SUBSYS_PD_NPU:
            SCU_EXTREG_SWRST_SET_PD_NPU_resetn(1);
            break;
        case SUBSYS_LCDC:
            SCU_EXTREG_SWRST_SET_LCDC_resetn(1);
            break;
        case SUBSYS_NCPU:
            SCU_EXTREG_CM4_NCPU_CTRL_SET_wakeup(0);
            SCU_EXTREG_CM4_NCPU_CTRL_SET_wakeup(1);    
    }
}

static void system_isr(void)
{
    u32 status;
    
    bootup_status = inw(SCU_REG_BTUP_STS);
    outw(SCU_REG_BTUP_STS, 0xffffffff);  // clear boot-up status

    status = inw(SCU_REG_INT_STS);    
    outw(SCU_REG_INT_STS, 0xffffffff); //clear sleep wakeup interrupt   

    __sys_int_flag = 0x1;

    dbg_msg("bootup_status=%x", bootup_status);
    if (bootup_status & SCU_REG_BTUP_STS_RTC_BTUPTS) {
        dbg_msg("bootup: SCU_REG_BTUP_STS_RTC_BTUPTS");
    }    
    if (bootup_status & SCU_REG_BTUP_STS_PWRBTN_STS) {
        dbg_msg("bootup: SCU_REG_BTUP_STS_PWRBTN_STS");
    }
    
    if (status & SCU_REG_INT_STS_PWRSTATE_CHG) {
        dbg_msg("SCU_REG_INT_STS_PWRSTATE_CHG");
    }
    if (status & SCU_REG_INT_STS_RTC_SEC) {
        dbg_msg("SCU_REG_INT_STS_RTC_SEC");
    }
    if (status & SCU_REG_INT_STS_RTC_PER) {
        dbg_msg("SCU_REG_INT_STS_RTC_PER");
    }
    if (status & SCU_REG_INT_STS_RTC_ALARM) {
        dbg_msg("SCU_REG_INT_STS_RTC_ALARM");
    }
    if (status & SCU_REG_INT_STS_PLL_UPDATE) {
        dbg_msg("SCU_REG_INT_STS_PLL_UPDATE");
    }
    if (status & SCU_REG_INT_STS_FCS) {
        dbg_msg("SCU_REG_INT_STS_FCS");
    }
    if (status & SCU_REG_INT_STS_BUSSPEED) {
        dbg_msg("SCU_REG_INT_STS_BUSSPEED");
    }
    if (status & SCU_REG_INT_STS_WAKEUP) {
        dbg_msg("SCU_REG_INT_STS_WAKEUP");
    }
    if (status & SCU_REG_INT_STS_PWRBTN_RISE) {
        dbg_msg("SCU_REG_INT_STS_PWRBTN_RISE");
    }
    if (status & SCU_REG_INT_STS_PWRBTN_FALL) {
        dbg_msg("SCU_REG_INT_STS_PWRBTN_FALL");
    }

    //outw(SCU_REG_BTUP_STS, bootup_status);
    outw(SCU_REG_INT_STS, status);
    NVIC_ClearPendingIRQ(SYS_SYSTEM_IRQ);
}

//void Reset_Handler(void) 
void reset_handler(void)
{
    bootup_status = inw(SCU_REG_BTUP_STS);
    outw(SCU_REG_BTUP_STS, 0xffffffff);  // clear boot-up status
    outw(SCU_REG_BTUP_CTRL, SCU_REG_BTUP_CTRL_RTC_BU_EN | // RTC wakeup allowed
                            SCU_REG_BTUP_CTRL_PWRBTN_EN | // send power button output signal
                            SCU_REG_BTUP_CTRL_GPO_1_OUT |
                            SCU_REG_BTUP_CTRL_GPO_OUT);

    NVIC_ClearPendingIRQ((IRQn_Type)SYS_SYSTEM_IRQ);
    NVIC_EnableIRQ((IRQn_Type)SYS_SYSTEM_IRQ);

    outw(SCU_REG_INT_STS, 0xffffffff); // clear all interrupt status
    outw(SCU_REG_INT_EN, 0xffffffff); // enable all interrupts during boot

    //can't directly write to the PLL control pins, it needs to use the 
    //FCS or PLL_UPDATE command that contained the power-mode register
    clock_mgr_set_scuclkin(scuclkin_pll0div3, TRUE);
    
    power_mgr_ops(POWER_MGR_OPS_FCS);
    __WFI();
    do{
    }while((__sys_int_flag)!= 0x1);
}

void system_init(void)
{
    delay_ms_enable();
    
    NVIC_SetVector((IRQn_Type)SYS_SYSTEM_IRQ, (u32)system_isr);
    
    reset_handler();
    
    if ((bootup_status & BOOTUP_STATUS_WARM) == BOOTUP_STATUS_WARM)
        warm_boot = 1;
    else
        warm_boot = 0;

    clock_mgr_init();

    /* Default power domain is already on */
    {
//OLD_VER
        power_mgr_set_mode(POWER_MGR_MODE_ALWAYSON);
        delay_us(10);
        SCU_EXTREG_CLK_EN0_SET_pll4_fref_pll0(1);
        delay_us(30);
        SCU_EXTREG_PLL4_SETTING_SET_en(1);
        delay_us(30);
        SCU_EXTREG_CLK_EN0_SET_pll4_out1(1);
//NEW
/*        //power_mgr_set_mode(POWER_MGR_MODE_ALWAYSON);
        //clock_mgr_open_pll4(); // npu
        delay_us(10);
        clk_enable(CLK_PLL4_FREF_PLL0);
        delay_us(30);
        clk_enable(CLK_PLL4);
        delay_us(30);
        clk_enable(CLK_PLL4_OUT); 
*/
    }	

    
//OLD_VER

//NEW
//    /* Turn on NPU power domain */
//    power_mgr_set_domain(POWER_DOMAIN_NPU, 1);
//    if (warm_boot) {
//        /* TODO: reload ncpu fw to NiRAM */
//    }


    {
        //OLD VER
        power_mgr_set_mode(POWER_MGR_MODE_FULL);
        {   // PLL1
            SCU_EXTREG_PLL1_SETTING_SET_en(1);
            delay_us(10);
            SCU_EXTREG_CLK_EN0_SET_pll1_out(1);
            delay_us(30);
        }
        {   // PLL2
            SCU_EXTREG_PLL2_SETTING_SET_en(1);
            delay_us(10);
            SCU_EXTREG_CLK_EN0_SET_pll2_out(1);
        }

        //NEW
//        /* Turn on DDR power domain for cold boot */
//        power_mgr_set_domain(POWER_DOMAIN_DDRCK, 1);
//
//        {   // PLL1
//            clk_enable(CLK_PLL1);
//            delay_us(10);
//            clk_enable(CLK_PLL1_OUT);
//            delay_us(30);
//        }
//
//        {   // PLL2
//            clk_enable(CLK_PLL2);
//            delay_us(10);
//            clk_enable(CLK_PLL2_OUT);
//        }

        //OLD
/*        {   // PLL3
            SCU_EXTREG_PLL3_SETTING_SET_en(1);
                 //set m, n, p value

            #if (V2K_ENABLE_TYPE == V2K_ENABLE_HMX2056) || \
                (V2K_ENABLE_TYPE == V2K_ENABLE_OV9286) || \
                (V2K_ENABLE_TYPE == V2K_ENABLE_HMX2056_OV9286)
                clock_mgr_change_pll3_clock(2, 268, 2, 5, 27, 13, 5, 15, 4);
            #elif (V2K_ENABLE_TYPE == V2K_ENABLE_HMXRICA)
                clock_mgr_change_pll3_clock(1, 210, 3, 5, 15, 3, 2, 31, 7);
            #elif (V2K_ENABLE_TYPE == V2K_ENABLE_GC2145)
                clock_mgr_change_pll3_clock(2, 242, 2, 4, 11, 5, 4, 7, 1);
            #elif (V2K_ENABLE_TYPE == V2K_ENABLE_SC132GS)
                clock_mgr_change_pll3_clock(2, 242, 2, 4, 7, 1, 4, 11, 5);
            #elif (V2K_ENABLE_TYPE == V2K_ENABLE_GC2145_SC132GS)
                clock_mgr_change_pll3_clock(2, 242, 2, 4, 11, 5, 4, 7, 1);
            #elif (V2K_ENABLE_TYPE == V2K_ENABLE_SC132GS_GC2145)
                clock_mgr_change_pll3_clock(2, 242, 2, 4, 7, 1, 4, 11, 5);
            #endif

            delay_us(10);
            SCU_EXTREG_CLK_EN0_SET_pll3_out1(1);
            SCU_EXTREG_CLK_EN0_SET_pll3_out2(1);
        }*/

        //NEW
        {   // PLL3     
            clk_enable(CLK_PLL3);
            //set m, n, p value

            clock_mgr_change_pll3_clock((u16*)&pll3_setting[0]);
       
        
            delay_us(10);
            clk_enable(CLK_PLL3_OUT1);
            clk_enable(CLK_PLL3_OUT2);
        }

        //OLD
        {   // PLL5
                SCU_EXTREG_PLL5_SETTING_SET_en(1);

            clock_mgr_change_pll5_clock(1, 0x63, 2);
            //clock_mgr_change_pll5_clock(1, 0x54, 5);

            delay_us(10);
            SCU_EXTREG_CLK_EN0_SET_pll5_out1(1);
            SCU_EXTREG_CLK_EN0_SET_pll5_out2(1);
        }

        //NEW
 /*       {   // PLL5
            clk_enable(CLK_PLL5);

            clock_mgr_change_pll5_clock(1, 0x63, 2);
            //clock_mgr_change_pll5_clock(1, 0x54, 5);

            delay_us(10);
            clk_enable(CLK_PLL5_OUT1);
            clk_enable(CLK_PLL5_OUT2);
        }  */
    }

//OLD_VER
    {   // fcs
        delay_us(10);
        SCU_EXTREG_CLK_EN0_SET_pll4_out1(0);
        delay_us(30);
        SCU_EXTREG_PLL4_SETTING_SET_en(0);
        delay_us(30);
        SCU_EXTREG_CLK_EN0_SET_pll4_fref_pll0(0);

        NVIC_ClearPendingIRQ((IRQn_Type)SYS_SYSTEM_IRQ);
        NVIC_EnableIRQ((IRQn_Type)SYS_SYSTEM_IRQ);

        outw(SCU_REG_INT_STS, 0xffffffff); // clear all interrupt status

        SCU_REG_PLL2_CTRL_SET_PLL2EN(1);
        SCU_REG_DLL_CTRL_SET_DLLEN(1);
        clock_mgr_set_scuclkin(scuclkin_pll0div3, FALSE);
        power_mgr_ops(POWER_MGR_OPS_FCS);

        __WFI();
        do{
        }while((__sys_int_flag)!= 0x1);

        delay_us(10);
        SCU_EXTREG_CLK_EN0_SET_pll4_fref_pll0(1);
        delay_us(30);
        SCU_EXTREG_PLL4_SETTING_SET_en(1);
        delay_us(30);
        SCU_EXTREG_CLK_EN0_SET_pll4_out1(1);
    }
//NEW VER
    /*
    {   // fcs
        delay_us(10);
        clk_enable(CLK_PLL4_FREF_PLL0);
        delay_us(30);
        clk_enable(CLK_PLL4);
        delay_us(30);
        clk_enable(CLK_PLL4_OUT);

        NVIC_ClearPendingIRQ((IRQn_Type)SYS_SYSTEM_IRQ);
        NVIC_EnableIRQ((IRQn_Type)SYS_SYSTEM_IRQ);
        
        outw(SCU_REG_INT_STS, 0xffffffff); // clear all interrupt status

        clk_enable(CLK_FCS_PLL2);
        clk_enable(CLK_FCS_DLL);
        clock_mgr_set_scuclkin(scuclkin_pll0div3, FALSE);
        power_mgr_ops(POWER_MGR_OPS_FCS);
            
        __WFI();
        do{
        }while((__sys_int_flag)!= 0x1);
    }
*/

    delay_ms_disable();
    masked_outw(SCU_REG_APBCLKG, 0, SCU_REG_APBCLKG_PCLK_EN_I2C0_PCLK);
}

void system_wakeup_ncpu(unsigned char wakeup_all)
{
    if (1 == wakeup_all) {
//OLD VER
        SCU_EXTREG_CLK_EN0_SET_scpu_traceclk(1);
        SCU_EXTREG_CLK_EN0_SET_ncpu_fclk_src(1);
        SCU_EXTREG_CLK_EN1_SET_npu(1);
        /* reset adc, usb, ddr3, lcdc, lcm, dpi, csirx0, csirx1, csitx, dsitx,
                 mipi_rx0_phy, mipi_rx1_phy */
        SCU_EXTREG_SWRST_SET_PD_NPU_resetn(0);
        SCU_EXTREG_SWRST_SET_NPU_resetn(0);
        delay_us(100);
        SCU_EXTREG_SWRST_SET_PD_NPU_resetn(1);
        SCU_EXTREG_SWRST_SET_NPU_resetn(1);
//NEW VER
        /*
        clk_enable(CLK_SCPU_TRACE);
        clk_enable(CLK_NCPU);
        clk_enable(CLK_NPU);
        sys_reset(SUBSYS_PD_NPU);
        sys_reset(SUBSYS_NPU);
        */
    }
    
    SCU_EXTREG_CM4_NCPU_CTRL_SET_wakeup(1);
}

void system_init_ncpu(void)
{
    clk_enable(CLK_SCPU_TRACE);
    clk_enable(CLK_NCPU);
    clk_enable(CLK_NPU);
    delay_us(100);
    
    sys_reset(SUBSYS_PD_NPU);
    memset((void *)NiRAM_MEM_BASE, 0x0, 0x10000);
    sys_reset(SUBSYS_NPU);
}

/**
*   flag = 0, just launch ncpu
*   flag <> 0, load and launch ncpu
*   flag < 0, not using mpu
*/
void load_ncpu_fw(int reset_flag)
{
    
    SCU_EXTREG_CM4_NCPU_CTRL_SET_wakeup(0);  // stop ncpu first
    if (reset_flag) {
        if (reset_flag > 0)
            mpu_niram_enable();
        kdp_memxfer_init(MEMXFER_OPS_DMA, MEMXFER_OPS_DMA);
        kdp_memxfer_flash_to_niram();
        if (reset_flag > 0)
            mpu_niram_disable();
    }
    SCU_EXTREG_CM4_NCPU_CTRL_SET_wakeup(1);  // restart ncpu
}
