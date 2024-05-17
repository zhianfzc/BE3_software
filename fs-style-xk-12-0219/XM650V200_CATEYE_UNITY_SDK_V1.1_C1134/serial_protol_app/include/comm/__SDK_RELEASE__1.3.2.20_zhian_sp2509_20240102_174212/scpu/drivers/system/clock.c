#include "clock.h"
#include <string.h>
#include "base.h"
#include "scu_reg.h"
#include "scu_extreg.h"
#include "dbg.h"
#include "delay.h"
#include "board_cfg.h"

#if 1
static BOOL osc_is_30m = FALSE;
static u16 T_OSC_in = 12;
static u16 osc_div_1n2 = 0;
static u16 pll0_out = 0;
static const u16 pll0_ms = 1;
static const u16 pll0_ns = 50;
static const u16 pll0_F = 3;

void clock_mgr_set_muxsel(u32 flags)
{
    u32 val = 0;
    if (CLOCK_MUXSEL_NCPU_TRACECLK_FROM_SCPU_TRACECLK == (flags & CLOCK_MUXSEL_NCPU_TRACECLK_MASK))
        val |= BIT14;
    if (CLOCK_MUXSEL_SCPU_TRACECLK_SRC_PLL0DIV2 == (flags & CLOCK_MUXSEL_SCPU_TRACECLK_MASK))
        val |= BIT13;
    if (CLOCK_MUXSEL_CSIRX1_CLK_PLL3 == (flags & CLOCK_MUXSEL_CSIRX1_CLK_MASK))
        val |= BIT12;

    switch (flags & CLOCK_MUXSEL_CSIRX1_CLK_MASK) {
        case CLOCK_MUXSEL_NPU_CLK_PLL0:
            val |= BIT9 | BIT8;
            break;
        case CLOCK_MUXSEL_NPU_CLK_PLL5:
            val |= BIT9;
            break;
        case CLOCK_MUXSEL_NPU_CLK_PLL4:
            val |= BIT8;
            break;                   
    }
    if (CLOCK_MUXSEL_PLL4_FREF_OSC == (flags & CLOCK_MUXSEL_PLL4_MASK))
        val |= BIT6;
    if (CLOCK_MUXSEL_UART_0_IRDA_UCLK_IRDA == (flags & CLOCK_MUXSEL_UART_0_IRDA_UCLK_MASK))
        val |= BIT4;
    
    //dbg_msg("val=%x", val);
    
    outw(SCU_EXTREG_CLK_MUX_SEL, val);
} 

void clock_mgr_init() 
{
    clock_mgr_set_muxsel(CLOCK_MUXSEL_CSIRX1_CLK_PLL3 | /* CLOCK_MUXSEL_CSIRX1_CLK_PLL5 */
                         CLOCK_MUXSEL_NPU_CLK_PLL4 |
                         CLOCK_MUXSEL_PLL4_FREF_PLL0DIV |
                         CLOCK_MUXSEL_UART_0_IRDA_UCLK_UART);      
}
void clock_mgr_set_scuclkin(enum scuclkin_type type, BOOL enable) 
{
//    SCU_REG_PLL_CTRL_SET_CLKIN_MUX(type);
//    SCU_REG_PLL_CTRL_PLLEN(enable);
    u32 val = ((type << SCU_REG_PLL_CTRL_CLKIN_MUX_BIT_START) |
              ((enable) ? (SCU_REG_PLL_CTRL_PLLEN) : (0)));
    masked_outw(SCU_REG_PLL_CTRL, val, SCU_REG_PLL_CTRL_CLKIN_MUX_MASK);
    
    osc_is_30m = ((scuclkin_pll0div4 == type) ? (TRUE) : (FALSE));
    osc_div_1n2 = (!osc_is_30m) ? (T_OSC_in) : (T_OSC_in >> 1);
    pll0_out = osc_div_1n2 * pll0_ns / pll0_ms;
    //switch (pll0_F) {
        pll0_out = pll0_out >> (3 - pll0_F);
    //}

    //dbg_msg("%s osc_div_1n2=%d pll0_out=%d", __func__, osc_div_1n2, pll0_out);
}

//void clock_mgr_change_pll3_clock(u32 ms, u32 ns, u32 ps, 
//        u32 csi0_txesc, u32 csi0_csi, u32 csi0_vc0,
//        u32 csi1_txesc, u32 csi1_csi, u32 csi1_vc0)
void clock_mgr_change_pll3_clock(u16 *pll_table)
{
#if 1//def PLL3_INITED_IN_SYTEM_INIT    
    // Set the new M/N/P values.
    masked_outw(SCU_EXTREG_PLL3_SETTING, 
                ((pll_table[PLL_PS] << SCU_EXTREG_PLL3_SETTING_ps_START) | 
                 (pll_table[PLL_NS] << SCU_EXTREG_PLL3_SETTING_ns_START) | 
                 (pll_table[PLL_MS] << SCU_EXTREG_PLL3_SETTING_ms_START) | 
                 (5 << SCU_EXTREG_PLL3_SETTING_is_START) | 
                 (3 << SCU_EXTREG_PLL3_SETTING_rs_START)),
    
                (SCU_EXTREG_PLL3_SETTING_ps_MASK | SCU_EXTREG_PLL3_SETTING_ns_MASK |
                 SCU_EXTREG_PLL3_SETTING_ms_MASK | SCU_EXTREG_PLL3_SETTING_is_MASK |
                 SCU_EXTREG_PLL3_SETTING_rs_MASK));
    
    //divider
    {            
        SCU_EXTREG_CLK_DIV1_SET_csirx0_TxEscClk(pll_table[CSIRX0_TXESCCLK]);
        SCU_EXTREG_CLK_DIV1_SET_csirx0_csi(pll_table[CSIRX0_CSI]);
        SCU_EXTREG_CLK_DIV1_SET_csirx0_vc0(pll_table[CSIRX0_VC0]);

        SCU_EXTREG_CLK_DIV7_SET_csirx1_TxEscClk_pll3(pll_table[CSIRX1_TXESCCLK]);
        SCU_EXTREG_CLK_DIV7_SET_csi1_csi_pll3(pll_table[CSIRX1_CSI]);
        SCU_EXTREG_CLK_DIV7_SET_csi1_vc0_pll3(pll_table[CSIRX1_VC0]);
    }
    
#else    
    SCU_EXTREG_CLK_EN1_SET_csirx0_TxEscClk(0);
    SCU_EXTREG_CLK_EN1_SET_csirx0_vc0(0);
    SCU_EXTREG_CLK_EN1_SET_csirx0_csi(0);
    
    // Disable PLL output (clock gated)
    SCU_EXTREG_CLK_EN0_SET_pll3_out2(0);
    SCU_EXTREG_CLK_EN0_SET_pll3_out1(0);

    // Power down PLL.
    SCU_EXTREG_PLL3_SETTING_SET_en(0);
    

    // Set the new M/N/P values.
    masked_outw(SCU_EXTREG_PLL3_SETTING, 
                ((ps << SCU_EXTREG_PLL3_SETTING_ps_START) | 
                 (ns << SCU_EXTREG_PLL3_SETTING_ns_START) | 
                 (ms << SCU_EXTREG_PLL3_SETTING_ms_START) | 
                 (5 << SCU_EXTREG_PLL3_SETTING_is_START) | 
                 (3 << SCU_EXTREG_PLL3_SETTING_rs_START)),
    
                (SCU_EXTREG_PLL3_SETTING_ps_MASK | SCU_EXTREG_PLL3_SETTING_ns_MASK |
                 SCU_EXTREG_PLL3_SETTING_ms_MASK | SCU_EXTREG_PLL3_SETTING_is_MASK |
                 SCU_EXTREG_PLL3_SETTING_rs_MASK));    
    
//dbg_msg("SCU_EXTREG_PLL3_SETTING_GET_ps()=%x", SCU_EXTREG_PLL3_SETTING_GET_ps());
//dbg_msg("SCU_EXTREG_PLL3_SETTING_GET_ns()=%x", SCU_EXTREG_PLL3_SETTING_GET_ns());
//dbg_msg("SCU_EXTREG_PLL3_SETTING_GET_ms()=%x", SCU_EXTREG_PLL3_SETTING_GET_ms());
//dbg_msg("SCU_EXTREG_PLL3_SETTING_GET_is()=%x", SCU_EXTREG_PLL3_SETTING_GET_is());
//dbg_msg("SCU_EXTREG_PLL3_SETTING_GET_rs()=%x", SCU_EXTREG_PLL3_SETTING_GET_rs());
    
    // For PLL510 series, wait for 40us reset time
    delay_us(40);

    // Enable PLL.
    SCU_EXTREG_PLL3_SETTING_SET_en(1);

    // Wait for PLL locking time. (PLL110: 50us, PLL510: 350us)
    delay_us(350);
    
    //SCU_EXTREG_CLK_MUX_SEL_SET_csirx1_clk(1);

	//divider
    SCU_EXTREG_CLK_DIV1_SET_csirx0_TxEscClk(5);
    SCU_EXTREG_CLK_DIV1_SET_csirx0_csi(0x1b);
    SCU_EXTREG_CLK_DIV1_SET_csirx0_vc0(0x0d);

    //SCU_EXTREG_CLK_DIV7_SET_csirx1_TxEscClk_pll3(5);
    //SCU_EXTREG_CLK_DIV7_SET_csi1_csi_pll3(15);
    //SCU_EXTREG_CLK_DIV7_SET_csi1_vc0_pll3(4);

    // Enable PLL output.
    SCU_EXTREG_CLK_EN0_SET_pll3_out2(1);
    SCU_EXTREG_CLK_EN0_SET_pll3_out1(1);
    
    SCU_EXTREG_CLK_EN1_SET_csirx0_TxEscClk(1);
    SCU_EXTREG_CLK_EN1_SET_csirx0_vc0(1);
    SCU_EXTREG_CLK_EN1_SET_csirx0_csi(1);
#endif    
}

void clock_mgr_change_pll5_clock(u32 ms, u32 ns, u32 ps)
{
    // Disable PLL output (clock gated)

    SCU_EXTREG_CLK_EN0_SET_pll5_out2(0);
    SCU_EXTREG_CLK_EN0_SET_pll5_out1(0);

    // Power down PLL.
    SCU_EXTREG_PLL5_SETTING_SET_en(0);    

    // Set the new M/N/P values.
    masked_outw(SCU_EXTREG_PLL5_SETTING, 
                (ps << SCU_EXTREG_PLL5_SETTING_ps_START) | 
                (ns << SCU_EXTREG_PLL5_SETTING_ns_START) | 
                (ms << SCU_EXTREG_PLL5_SETTING_ms_START) | 
                (5 << SCU_EXTREG_PLL5_SETTING_is_START) | 
                (3 << SCU_EXTREG_PLL5_SETTING_rs_START),
    
                (SCU_EXTREG_PLL5_SETTING_ps_MASK | SCU_EXTREG_PLL5_SETTING_ns_MASK |
                SCU_EXTREG_PLL5_SETTING_ms_MASK | SCU_EXTREG_PLL5_SETTING_is_MASK |
                SCU_EXTREG_PLL5_SETTING_rs_MASK));

    // For PLL510 series, wait for 40us reset time
    delay_us(40);

    // Enable PLL.
    SCU_EXTREG_PLL5_SETTING_SET_en(1);

    // Wait for PLL locking time. (PLL110: 50us, PLL510: 350us)
    delay_us(350);

    // Enable PLL output.  
    SCU_EXTREG_CLK_EN0_SET_pll5_out2(1);
    SCU_EXTREG_CLK_EN0_SET_pll5_out1(1); 
}

static void clk_control(enum clk clk, int enable)
{
    switch (clk) {
        case CLK_PLL1:
            SCU_EXTREG_PLL1_SETTING_SET_en(enable);
            break;
        case CLK_PLL2:
            SCU_EXTREG_PLL2_SETTING_SET_en(enable);
            break;
        case CLK_PLL3:
            SCU_EXTREG_PLL3_SETTING_SET_en(enable);
            break;
        case CLK_PLL4:
            SCU_EXTREG_PLL4_SETTING_SET_en(enable);
            break;
        case CLK_PLL5:
            SCU_EXTREG_PLL5_SETTING_SET_en(enable);
            break;
        case CLK_PLL1_OUT:
            SCU_EXTREG_CLK_EN0_SET_pll1_out(enable);
            break;
        case CLK_PLL2_OUT:
            SCU_EXTREG_CLK_EN0_SET_pll2_out(enable);
            break;
        case CLK_PLL3_OUT1:
            SCU_EXTREG_CLK_EN0_SET_pll3_out1(enable);
            break;
        case CLK_PLL3_OUT2:
            SCU_EXTREG_CLK_EN0_SET_pll3_out2(enable);
            break;
        case CLK_PLL4_OUT:
            SCU_EXTREG_CLK_EN0_SET_pll4_out1(enable);
            break;
        case CLK_PLL5_OUT1:
            SCU_EXTREG_CLK_EN0_SET_pll5_out1(enable);
            break;
        case CLK_PLL5_OUT2:
            SCU_EXTREG_CLK_EN0_SET_pll5_out2(enable);
            break;

        case CLK_SCPU_TRACE:
            SCU_EXTREG_CLK_EN0_SET_scpu_traceclk(enable);
            break;

        case CLK_NCPU:
            SCU_EXTREG_CLK_EN0_SET_ncpu_fclk_src(enable);
            break;
        case CLK_NCPU_TRACE:
            SCU_EXTREG_CLK_EN0_SET_ncpu_traceclk(enable);
            break;

        case CLK_NPU:
            SCU_EXTREG_CLK_EN1_SET_npu(enable);
            break;

        case CLK_PLL4_FREF_PLL0:
            SCU_EXTREG_CLK_EN0_SET_pll4_fref_pll0(enable);
            break;
        case CLK_FCS_PLL2:
            SCU_REG_PLL2_CTRL_SET_PLL2EN(enable);
            break;
        case CLK_FCS_DLL:
            SCU_REG_DLL_CTRL_SET_DLLEN(enable);
            break;

        case CLK_SPI_CLK:
            SCU_EXTREG_CLK_EN1_SET_spi_clk(enable);
            break;
        case CLK_ADC_CLK:
            SCU_EXTREG_CLK_EN1_SET_adcclk(enable);
            break;
        case CLK_WDT_EXT_CLK:
            SCU_EXTREG_CLK_EN1_SET_wdt_extclk(enable);
            break;
        case CLK_SD_CLK:
            SCU_EXTREG_CLK_EN1_SET_sdclk(enable);
            break;
        case CLK_MIPI_TXHSPLLREF_CLK:
            SCU_EXTREG_CLK_EN1_SET_TxHsPllRefClk(enable);
            break;
        case CLK_MIPI_TX_ESC_CLK:
            SCU_EXTREG_CLK_EN1_SET_tx_EscClk(enable);
            break;
        case CLK_MIPI_CSITX_DSI_CLK:
            SCU_EXTREG_CLK_EN1_SET_csitx_dsi(enable);
            break;
        case CLK_MIPI_CSITX_CSI_CLK:
            SCU_EXTREG_CLK_EN1_SET_csitx_csi(enable);
            break;
        case CLK_MIPI_CSIRX1_TXESC_CLK:
            SCU_EXTREG_CLK_EN1_SET_csirx1_TxEscClk(enable);
            break;
        case CLK_MIPI_CSIRX1_CSI_CLK:
            SCU_EXTREG_CLK_EN1_SET_csirx1_csi(enable);
            break;
        case CLK_MIPI_CSIRX1_VC0_CLK:
            SCU_EXTREG_CLK_EN1_SET_csirx1_vc0(enable);
            break;
        case CLK_MIPI_CSIRX0_TXESC_CLK:
            SCU_EXTREG_CLK_EN1_SET_csirx0_TxEscClk(enable);
            break;
        case CLK_MIPI_CSIRX0_CSI_CLK:
            SCU_EXTREG_CLK_EN1_SET_csirx0_csi(enable);
            break;
        case CLK_MIPI_CSIRX0_VC0_CLK:
            SCU_EXTREG_CLK_EN1_SET_csirx0_vc0(enable);
            break;
        case CLK_LC_SCALER:
            SCU_EXTREG_CLK_EN1_SET_LC_SCALER(enable);
            break;
        case CLK_LC_CLK:
            SCU_EXTREG_CLK_EN1_SET_LC_CLK(enable);
            break;
        case CLK_TMR1_EXTCLK3:
            SCU_EXTREG_CLK_EN2_SET_tmr1_extclk3(enable);
            break;
        case CLK_TMR1_EXTCLK2:
            SCU_EXTREG_CLK_EN2_SET_tmr1_extclk2(enable);
            break;
        case CLK_TMR1_EXTCLK1:
            SCU_EXTREG_CLK_EN2_SET_tmr1_extclk1(enable);
            break;
        case CLK_TMR0_EXTCLK3:
            SCU_EXTREG_CLK_EN2_SET_tmr0_extclk3(enable);
            break;
        case CLK_TMR0_EXTCLK2:
            SCU_EXTREG_CLK_EN2_SET_tmr0_extclk2(enable);
            break;
        case CLK_TMR0_EXTCLK1:
            SCU_EXTREG_CLK_EN2_SET_tmr0_extclk1(enable);
            break;
        case CLK_PWM_EXTCLK6:
            SCU_EXTREG_CLK_EN2_SET_pwm_extclk6(enable);
            break;
        case CLK_PWM_EXTCLK5:
            SCU_EXTREG_CLK_EN2_SET_pwm_extclk5(enable);
            break;
        case CLK_PWM_EXTCLK4:
            SCU_EXTREG_CLK_EN2_SET_pwm_extclk4(enable);
            break;
        case CLK_PWM_EXTCLK3:
            SCU_EXTREG_CLK_EN2_SET_pwm_extclk3(enable);
            break;
        case CLK_PWM_EXTCLK2:
            SCU_EXTREG_CLK_EN2_SET_pwm_extclk2(enable);
            break;
        case CLK_PWM_EXTCLK1:
            SCU_EXTREG_CLK_EN2_SET_pwm_extclk1(enable);
            break;
        case CLK_UART1_3_FREF:
            SCU_EXTREG_CLK_EN2_SET_uart1_3_fref(enable);
            break;
        case CLK_UART1_2_FREF:
            SCU_EXTREG_CLK_EN2_SET_uart1_2_fref(enable);
            break;
        case CLK_UART1_1_FREF:
            SCU_EXTREG_CLK_EN2_SET_uart1_1_fref(enable);
            break;
        case CLK_UART1_0_FREF:
            SCU_EXTREG_CLK_EN2_SET_uart1_0_fref(enable);
            break;
        case CLK_UART0_FREF:
            SCU_EXTREG_CLK_EN2_SET_uart0_fref(enable);
            break;
        case CLK_SSP1_1_SSPCLK:
            SCU_EXTREG_CLK_EN2_SET_ssp1_1_sspclk(enable);
            break;
        case CLK_SSP1_0_SSPCLK:
            SCU_EXTREG_CLK_EN2_SET_ssp1_0_sspclk(enable);
            break;
        case CLK_SSP0_1_SSPCLK:
            SCU_EXTREG_CLK_EN2_SET_ssp0_1_sspclk(enable);
            break;
        case CLK_SSP0_0_SSPCLK:
            SCU_EXTREG_CLK_EN2_SET_ssp0_0_sspclk(enable);
            break;
    }
}

void clk_enable(enum clk clk)
{
    clk_control(clk, 1);
}

void clk_disable(enum clk clk)
{
    clk_control(clk, 0);
}

#else
/* PLL 3/4/5
VCOOUT = FREF * (ns / ms)
CKOUT1 = VCOOUT / ps / 2
CKOUT2 = VCOOUT / ps / 8
if FREF = 12MHz,
CKOUT1 = 12 * 120 /2 = 720
CKOUT2 = 12 * 120 /8 = 180
csirx0_vc0_pclk = 720 / (0x1d+1) = 720/30=24
csirx0_csi_pclk = 720 / (0x1d+1) = 720/30=24
csirx0_TxEscClk = 180 / (0x4+1) = 36
*/ 

//#define DEBUG_PLL_CLOCK

#ifdef DEBUG_PLL_CLOCK 
#define DEBUG_CLOCK(__node, __clock_val) { \
        if (clock_val) { \
            dbg_msg("%s name=%s is_enabled=%d ns=%u ms=%u ps=%u div=%u %x", \
            __func__, node->name, node->is_enabled, \
            __clock_val->u.pll.ns, \
            __clock_val->u.pll.ms, \
            __clock_val->u.pll.ps, \
            __clock_val->u.pll.div, \
            (*((u32*)__clock_val))); \
        } else { \
            dbg_msg("%s name=%s is_enabled=%d %x", \
            __func__, node->name, node->is_enabled, (*((u32*)__clock_val))); \
        } \
    }
#else
#define DEBUG_CLOCK(__node, __clock_val)
#endif 
#define _clock_ref_add(__node) {  ++__node->ref_cnt; }
#define _clock_ref_sub(__node) {  if (__node->ref_cnt > 0) --__node->ref_cnt; }

static BOOL osc_is_30m = FALSE;
static u16 T_OSC_in = 12;
static u16 osc_div_1n2 = 0;
static u16 pll0_out = 0;
static const u16 pll0_ms = 1;
static const u16 pll0_ns = 50;
static const u16 pll0_F = 3;

/* pll1 nodes */
struct clock_node clock_node_pll1 = { .name = "pll1" };
struct clock_node clock_node_pll1_out = { .name = "pll1o" };
/* pll2 nodes */
struct clock_node clock_node_pll2 = { .name = "pll2" };
struct clock_node clock_node_pll2_out = { .name = "pll2o" };
/* pll3 nodes */
struct clock_node clock_node_pll3 = { .name = "pll3" };
struct clock_node clock_node_pll3_out1 = { .name = "pll3o1" };
struct clock_node clock_node_pll3_out2 = {.name = "pll3o2" };
struct clock_node clock_node_csirx0_hs_csi = { .name = "pll3csirx0csi" };
struct clock_node clock_node_csirx0_hs_vc0 = { .name = "pll3csirx0vc0" };
struct clock_node clock_node_csirx0_lp = { .name = "pll3csirx0lp" };
/* pll4 nodes */
struct clock_node clock_node_pll4 = { .name = "pll4" };
struct clock_node clock_node_pll4_out1 = { .name = "pll4o1" };
struct clock_node clock_node_pll4_fref_pll0 = { .name = "pll4frefpll0" };
/* pll5 nodes */
struct clock_node clock_node_pll5 = { .name = "pll5" };
struct clock_node clock_node_pll5_out1 = { .name = "pll5o1" };
struct clock_node clock_node_pll5_out2 = { .name = "pll5o2" };
struct clock_node clock_node_lcdc = { .name = "pll5lcclk" };



static int _clock_set_pll1(struct clock_node *node, struct clock_value *clock_val) 
{
    int ret = 0;

    DEBUG_CLOCK(node, clock_val);

    if (clock_val) {
        SCU_EXTREG_PLL1_SETTING_SET_en(clock_val->enable);
    } else {
        SCU_EXTREG_PLL1_SETTING_SET_en(0);
    }
    
    delay_us(1000);
            
    return ret;
}

static int _clock_set_pll1_out(struct clock_node *node, struct clock_value *clock_val) 
{
    int ret = 0;

    DEBUG_CLOCK(node, clock_val);

    if (clock_val) {
        SCU_EXTREG_CLK_EN0_SET_pll1_out(clock_val->enable);
    } else {
        SCU_EXTREG_CLK_EN0_SET_pll1_out(0);
    }
    
    delay_us(1000);
    
    return ret;
}

static int _clock_set_pll2(struct clock_node *node, struct clock_value *clock_val) 
{
    int ret = 0;

    DEBUG_CLOCK(node, clock_val);

    if (clock_val) {
        SCU_EXTREG_PLL2_SETTING_SET_en(clock_val->enable);
    } else {
        SCU_EXTREG_PLL2_SETTING_SET_en(0);
    }
    
    delay_us(1000);
    
    return ret;
}

static int _clock_set_pll2_out(struct clock_node *node, struct clock_value *clock_val) 
{
    int ret = 0;

    DEBUG_CLOCK(node, clock_val);

    if (clock_val) {
        SCU_EXTREG_CLK_EN0_SET_pll2_out(clock_val->enable);
    } else {
        SCU_EXTREG_CLK_EN0_SET_pll2_out(0);
    }
    
    delay_us(1000);
    
    return ret;
}

static int _clock_set_pll3(struct clock_node *node, struct clock_value *clock_val)
{
    int ret = 0;

    DEBUG_CLOCK(node, clock_val);

    if (clock_val) {
        if ((clock_val->ms > 0) && (clock_val->ns > 0) && (clock_val->ps)) {
            SCU_EXTREG_PLL3_SETTING_SET_en(1);
            delay_us(1000);
            SCU_EXTREG_PLL3_SETTING_SET_ms(clock_val->ms);
            SCU_EXTREG_PLL3_SETTING_SET_ns(clock_val->ns);
            SCU_EXTREG_PLL3_SETTING_SET_ps(clock_val->ps);
        }
    } else {
        SCU_EXTREG_PLL3_SETTING_SET_en(0);
    }
   
    delay_us(1000);
    
    return ret;
}

static int _clock_set_pll3_out1(struct clock_node *node, struct clock_value *clock_val) 
{
    int ret = 0;

    DEBUG_CLOCK(node, clock_val);

    if (clock_val) {
        SCU_EXTREG_CLK_EN0_SET_pll3_out1(clock_val->enable);    
        delay_us(1000);
        SCU_EXTREG_CLK_EN0_SET_pll3_out1(1);
    } else {
        SCU_EXTREG_CLK_EN0_SET_pll3_out1(0);
    }
    
    delay_us(1000);
    
    return ret;
}

static int _clock_set_pll3_out2(struct clock_node *node, struct clock_value *clock_val) 
{
    int ret = 0;

    DEBUG_CLOCK(node, clock_val);

    if (clock_val) {
        SCU_EXTREG_CLK_EN0_SET_pll3_out2(clock_val->enable);
    } else {
        SCU_EXTREG_CLK_EN0_SET_pll3_out2(0);
    }
    
    delay_us(1000);
    
    return ret;
}

static int _clock_set_csirx0_hs_csi(struct clock_node *node, struct clock_value *clock_val) 
{
    int ret = 0;

    DEBUG_CLOCK(node, clock_val);

    if (clock_val) {   
        SCU_EXTREG_CLK_DIV1_SET_csirx0_csi(clock_val->div);
        delay_us(1000);
        SCU_EXTREG_CLK_EN1_SET_csirx0_csi(1);
    } else {
        SCU_EXTREG_CLK_EN1_SET_csirx0_csi(0);
    }

    delay_us(1000);

    return ret;
}

static int _clock_set_csirx0_hs_vc0(struct clock_node *node, struct clock_value *clock_val) 
{
    int ret = 0;

    DEBUG_CLOCK(node, clock_val);

    if (clock_val) {
        SCU_EXTREG_CLK_DIV1_SET_csirx0_vc0(clock_val->div);
        delay_us(1000);
        SCU_EXTREG_CLK_EN1_SET_csirx0_vc0(1);
    } else {
        SCU_EXTREG_CLK_EN1_SET_csirx0_vc0(0);
    }

    delay_us(1000);

    return ret;
}

static int _clock_set_csirx0_lp(struct clock_node *node, struct clock_value *clock_val) 
{
    int ret = 0;

    DEBUG_CLOCK(node, clock_val);

    if (clock_val) {
        SCU_EXTREG_CLK_DIV1_SET_csirx0_TxEscClk(clock_val->div);
        delay_us(1000);
        SCU_EXTREG_CLK_EN1_SET_csirx0_TxEscClk(1);
    } else {
        SCU_EXTREG_CLK_EN1_SET_csirx0_TxEscClk(0);
    }
    
    delay_us(1000);
    
    return ret;
}

static int _clock_set_pll4(struct clock_node *node, struct clock_value *clock_val) 
{
    int ret = 0;

    DEBUG_CLOCK(node, clock_val);

    if (clock_val) {
        SCU_EXTREG_PLL4_SETTING_SET_en(clock_val->enable);
    } else {
        SCU_EXTREG_PLL4_SETTING_SET_en(0);
    }
    
    delay_us(1000);
    
    return ret;
}

static int _clock_set_pll4_out1(struct clock_node *node, struct clock_value *clock_val) 
{
    int ret = 0;

    DEBUG_CLOCK(node, clock_val);

    if (clock_val) {
        SCU_EXTREG_CLK_EN0_SET_pll4_out1(clock_val->enable);
    } else {
        SCU_EXTREG_CLK_EN0_SET_pll4_out1(0);
    }
    
    delay_us(1000);
    
    return ret;
}

static int _clock_set_pll4_fref_pll0(struct clock_node *node, struct clock_value *clock_val) 
{
    int ret = 0;

    DEBUG_CLOCK(node, clock_val);

    if (clock_val) {
        SCU_EXTREG_CLK_EN0_SET_pll4_fref_pll0(clock_val->enable);
    } else {
        SCU_EXTREG_CLK_EN0_SET_pll4_fref_pll0(0);
    }

    delay_us(1000);

    return ret;
}

static int _clock_set_pll5(struct clock_node *node, struct clock_value *clock_val) 
{
    int ret = 0;

    DEBUG_CLOCK(node, clock_val);

    if (clock_val) {
        if ((clock_val->ms > 0) && (clock_val->ns > 0) && (clock_val->ps))
        {
            SCU_EXTREG_PLL5_SETTING_SET_en(1);
            delay_us(1000);
            SCU_EXTREG_PLL5_SETTING_SET_ms(clock_val->ms);
            SCU_EXTREG_PLL5_SETTING_SET_ns(clock_val->ns);
            SCU_EXTREG_PLL5_SETTING_SET_ps(clock_val->ps);
        }
    } else {
        SCU_EXTREG_PLL5_SETTING_SET_en(0);
    }
    
    delay_us(1000);

    return ret;
}

static int _clock_set_pll5_out1(struct clock_node *node, struct clock_value *clock_val) 
{
    int ret = 0;

    DEBUG_CLOCK(node, clock_val);

    if (clock_val) {
        SCU_EXTREG_CLK_EN0_SET_pll5_out1(clock_val->enable);
    } else {
        SCU_EXTREG_CLK_EN0_SET_pll5_out1(0);
    }

    delay_us(1000);

    return ret;
}

static int _clock_set_pll5_out2(struct clock_node *node, struct clock_value *clock_val) 
{
    int ret = 0;

    DEBUG_CLOCK(node, clock_val);

    if (clock_val) {
        SCU_EXTREG_CLK_EN0_SET_pll5_out2(clock_val->enable);
    } else {
        SCU_EXTREG_CLK_EN0_SET_pll5_out2(0);
    }

    delay_us(1000);

    return ret;
}

static int _clock_set_lcdc(struct clock_node *node, struct clock_value *clock_val) 
{
    int ret = 0;

    DEBUG_CLOCK(node, clock_val);

    if (clock_val) {
        SCU_EXTREG_SWRST_SET_LCDC_resetn(1);
        delay_us(1000);
    // outw(SCU_EXTREG_SWRST_MASK1, 
    //      SCU_EXTREG_SWRST_MASK1_AResetn_u_FTLCDC210 |
    //      SCU_EXTREG_SWRST_MASK1_PRESETn_u_FTLCDC210 |
    //      SCU_EXTREG_SWRST_MASK1_TV_RSTn_FTLCDC210 |
    //      SCU_EXTREG_SWRST_MASK1_LC_SCALER_RSTn_FTLCDC210 |
    //      SCU_EXTREG_SWRST_MASK1_LC_RSTn_FTLCDC210);

        SCU_EXTREG_CLK_EN1_SET_LC_CLK(1);
        SCU_EXTREG_CLK_EN1_SET_LC_SCALER(1);

    } else {
        SCU_EXTREG_CLK_EN1_SET_LC_SCALER(0);
        SCU_EXTREG_CLK_EN1_SET_LC_CLK(0);
    }
    
    delay_us(1000);
    
    return ret;
}

inline struct clock_node * _find_child_head(struct clock_node *node) 
{
    struct clock_node *p = node; 
    while (p->child_next) p = p->child_next; 
    return p;
}

struct clock_node * _find_sibling_is_enabled(struct clock_node *node) 
{
    struct clock_node *p = NULL; 
    struct clock_node *next = node->child_head;
    while (next)
    {
        if (next->is_enabled) {
            p = next;
            break;
        }
        next = next->child_next;
    }

    return p;
}

static void _clock_node_register(struct clock_node *node, struct clock_node *parent, fn_set set) 
{
    node->is_enabled = FALSE;
    node->child_head = NULL;
    node->parent = parent;
    node->set = set;
    if (parent) {
        struct clock_node *child = parent->child_head;

        if (!parent->child_head) {
            parent->child_head = node;
            parent->child_next = node;
        } else 
            (_find_child_head(parent))->child_next = node;

        child = parent->child_head;
        while (child) {
            child = child->child_next;
        } 
    } 
}

void clock_mgr_init() 
{
    _clock_node_register(&clock_node_pll1, NULL, _clock_set_pll1);
    _clock_node_register(&clock_node_pll1_out, &clock_node_pll1, _clock_set_pll1_out);
    
    _clock_node_register(&clock_node_pll2, NULL, _clock_set_pll2);
    _clock_node_register(&clock_node_pll2_out, &clock_node_pll2, _clock_set_pll2_out);

    _clock_node_register(&clock_node_pll3, NULL, _clock_set_pll3);
    _clock_node_register(&clock_node_pll3_out1, &clock_node_pll3, _clock_set_pll3_out1);
    _clock_node_register(&clock_node_pll3_out2, &clock_node_pll3, _clock_set_pll3_out2);
    _clock_node_register(&clock_node_csirx0_hs_csi, &clock_node_pll3_out1, _clock_set_csirx0_hs_csi);
    _clock_node_register(&clock_node_csirx0_hs_vc0, &clock_node_pll3_out1, _clock_set_csirx0_hs_vc0);
    _clock_node_register(&clock_node_csirx0_lp, &clock_node_pll3_out2, _clock_set_csirx0_lp);
    
    _clock_node_register(&clock_node_pll4, NULL, _clock_set_pll4);
    _clock_node_register(&clock_node_pll4_out1, &clock_node_pll4, _clock_set_pll4_out1);
    _clock_node_register(&clock_node_pll4_fref_pll0, &clock_node_pll4, _clock_set_pll4_fref_pll0);

    _clock_node_register(&clock_node_pll5, NULL, _clock_set_pll5);
    _clock_node_register(&clock_node_pll5_out1, &clock_node_pll5, _clock_set_pll5_out1);
    _clock_node_register(&clock_node_pll5_out2, &clock_node_pll5, _clock_set_pll5_out2);
    _clock_node_register(&clock_node_lcdc, &clock_node_pll5_out1, _clock_set_lcdc);
    // for debug 
    // //clock_mgr_open(&clock_node_pll1_out);
    // //clock_mgr_close(&clock_node_pll1_out);
    // struct clock_value val;
    // val.u.enable = 1;

    // clock_mgr_open(&clock_node_csirx0_hs_csi, &val);
    // clock_mgr_open(&clock_node_csirx0_hs_vc0, &val);
    // //clock_mgr_open(&clock_node_csirx0_lp, &val);
    
    // //clock_mgr_close(&clock_node_csirx0_lp);
    // clock_mgr_close(&clock_node_csirx0_hs_vc0);
    // clock_mgr_close(&clock_node_csirx0_hs_csi);

    clock_mgr_set_muxsel(CLOCK_MUXSEL_CSIRX1_CLK_PLL3 | /* CLOCK_MUXSEL_CSIRX1_CLK_PLL5 */
                         CLOCK_MUXSEL_NPU_CLK_PLL4 |
                         CLOCK_MUXSEL_PLL4_FREF_PLL0DIV |
                         CLOCK_MUXSEL_UART_0_IRDA_UCLK_UART);      
}

void clock_mgr_open(struct clock_node *node, struct clock_value *clock_val) 
{
    //dbg_msg("%s node->name=%s", __func__, node->name);    
    if (node) {
        if (node->parent) {
            clock_mgr_open(node->parent, clock_val);
        }

        if (FALSE == node->is_enabled) {
            node->set(node, clock_val);
            node->is_enabled = TRUE;
        }
    }
}

void clock_mgr_close(struct clock_node *node) 
{
    //dbg_msg("%s node->name=%s", __func__, node->name);        
    if (node) {
        struct clock_node *sibling;
        node->set(node, NULL);
        node->is_enabled = FALSE;
    
        sibling = _find_sibling_is_enabled(node->parent);
        if (!sibling)
            clock_mgr_close(node->parent);   
    }
}

void clock_mgr_set_scuclkin(enum scuclkin_type type, BOOL enable) 
{
//    SCU_REG_PLL_CTRL_SET_CLKIN_MUX(type);
//    SCU_REG_PLL_CTRL_PLLEN(enable);
    u32 val = ((type << SCU_REG_PLL_CTRL_CLKIN_MUX_BIT_START) |
              ((enable) ? (SCU_REG_PLL_CTRL_PLLEN) : (0)));
    masked_outw(SCU_REG_PLL_CTRL, val, SCU_REG_PLL_CTRL_CLKIN_MUX_MASK);
    
    osc_is_30m = ((scuclkin_pll0div4 == type) ? (TRUE) : (FALSE));
    osc_div_1n2 = (!osc_is_30m) ? (T_OSC_in) : (T_OSC_in >> 1);
    pll0_out = osc_div_1n2 * pll0_ns / pll0_ms;
    //switch (pll0_F) {
        pll0_out = pll0_out >> (3 - pll0_F);
    //}

    //dbg_msg("%s osc_div_1n2=%d pll0_out=%d", __func__, osc_div_1n2, pll0_out);
}

void clock_mgr_set_muxsel(u32 flags)
{
    u32 val = 0;
    if (CLOCK_MUXSEL_NCPU_TRACECLK_FROM_SCPU_TRACECLK == (flags & CLOCK_MUXSEL_NCPU_TRACECLK_MASK))
        val |= BIT14;
    if (CLOCK_MUXSEL_SCPU_TRACECLK_SRC_PLL0DIV2 == (flags & CLOCK_MUXSEL_SCPU_TRACECLK_MASK))
        val |= BIT13;
    if (CLOCK_MUXSEL_CSIRX1_CLK_PLL3 == (flags & CLOCK_MUXSEL_CSIRX1_CLK_MASK))
        val |= BIT12;

    switch (flags & CLOCK_MUXSEL_CSIRX1_CLK_MASK) {
        case CLOCK_MUXSEL_NPU_CLK_PLL0:
            val |= BIT9 | BIT8;
            break;
        case CLOCK_MUXSEL_NPU_CLK_PLL5:
            val |= BIT9;
            break;
        case CLOCK_MUXSEL_NPU_CLK_PLL4:
            val |= BIT8;
            break;                   
    }
    if (CLOCK_MUXSEL_PLL4_FREF_OSC == (flags & CLOCK_MUXSEL_PLL4_MASK))
        val |= BIT6;
    if (CLOCK_MUXSEL_UART_0_IRDA_UCLK_IRDA == (flags & CLOCK_MUXSEL_UART_0_IRDA_UCLK_MASK))
        val |= BIT4;
    
    //dbg_msg("val=%x", val);
    
    outw(SCU_EXTREG_CLK_MUX_SEL, val);
}

u32  clock_mgr_calculate_clockout(enum pll_id id, u16 ms, u16 ns, u16 F_ps)
{
    u32 fref = 0;
    u32 ckout = 0;

    switch (id) {
    //case pll_0:
    case pll_1:
    case pll_2:
        fref = T_OSC_in;
        ckout = (fref * ns / ms) >> (3 - F_ps);
        break;
    case pll_3:
    case pll_5:
        fref = osc_div_1n2;
        ckout = (fref * ns / ms) / F_ps;
        break;
    case pll_4:
        if (0 == SCU_EXTREG_CLK_MUX_SEL_GET_pll4_fref()) {
            fref = (osc_div_1n2 * pll0_ns / pll0_ms) >> (3 - pll0_F) / (1 + SCU_EXTREG_CLK_DIV0_GET_pll4_fref_pll0());
        } else
            fref = osc_div_1n2;
        ckout = (fref * ns / ms) / F_ps;
        break;
    default:;
    }
    
    return ckout;
}

void clock_mgr_open_pll1(void)
{
#if 1
    struct clock_value clock_val;    
    memset(&clock_val, 0, sizeof(clock_val));
    clock_val.enable = 1;
    clock_mgr_open(&clock_node_pll1_out, &clock_val);
#else        
    SCU_EXTREG_PLL1_SETTING_SET_en(1);
    delay_us(1000);
    SCU_EXTREG_CLK_EN0_SET_pll1_out(1);
    delay_us(1000);
#endif
}

void clock_mgr_open_pll2(void)
{
    //PLL2 en - ddr3  
#if 1
    struct clock_value clock_val;     
    memset(&clock_val, 0, sizeof(clock_val));
    clock_val.enable = 1;
    clock_mgr_open(&clock_node_pll2_out, &clock_val);
#else        
    SCU_EXTREG_PLL2_SETTING_SET_en(1); 
    delay_us(1000);
    SCU_EXTREG_CLK_EN0_SET_pll2_out(1); 
    delay_us(1000);
#endif
}

void clock_mgr_open_pll3(void)
{
    {
        //u32 clk;

        struct clock_value clock_val_hs;
        memset(&clock_val_hs, 0, sizeof(clock_val_hs));

            clock_val_hs.ns = 268;//283
            clock_val_hs.ms = 2;
            clock_val_hs.ps = 2;
            clock_val_hs.div = 27;//6;

        //clk = clock_mgr_calculate_clockout(pll_3, clock_val_hs.ms, clock_val_hs.ns, clock_val_hs.ps);

        clock_mgr_open(&clock_node_csirx0_hs_csi, &clock_val_hs);

            clock_val_hs.ns = 268;//283
            clock_val_hs.ms = 2;
            clock_val_hs.ps = 2;
            clock_val_hs.div = 13;//29;
        clock_mgr_open(&clock_node_csirx0_hs_vc0, &clock_val_hs);
    }
    {
        struct clock_value clock_val_lp;
        memset(&clock_val_lp, 0, sizeof(clock_val_lp));      
        clock_val_lp.ns = 268;//283
        clock_val_lp.ms = 2;
        clock_val_lp.ps = 2;
        clock_val_lp.div = 5;//4;

        clock_mgr_open(&clock_node_csirx0_lp, &clock_val_lp);
    }    
}

void clock_mgr_open_pll4(void)
{   
#if 1
    struct clock_value clock_val;
	memset(&clock_val, 0, sizeof(clock_val));
	clock_val.enable = 1;
	clock_mgr_open(&clock_node_pll4_fref_pll0, &clock_val);		
	clock_mgr_open(&clock_node_pll4_out1, &clock_val);
#else
    SCU_EXTREG_PLL4_SETTING_SET_en(1);
    delay_us(1000);
    SCU_EXTREG_CLK_EN0_SET_pll4_fref_pll0(1);
    delay_us(1000);
    SCU_EXTREG_CLK_EN0_SET_pll4_out1(1);
    delay_us(1000);
#endif    
}

void clock_mgr_open_pll5(void)
{
#if 0
#else    
//    SCU_EXTREG_PLL5_SETTING_SET_en(1);
//    SCU_EXTREG_CLK_EN0_SET_pll5_out1(1);
//    SCU_EXTREG_CLK_EN0_SET_pll5_out2(1);
#endif    
}

void clock_mgr_close_pll1(void)
{
}

void clock_mgr_close_pll2(void)
{

}

void clock_mgr_close_pll3(void)
{

}

void clock_mgr_close_pll4(void)
{
#if 0
    clock_mgr_close(&clock_node_pll4_fref_pll0);
    clock_mgr_close(&clock_node_pll4_out1);
#else
    SCU_EXTREG_CLK_EN0_SET_pll4_out1(0);
    delay_us(1000);
    SCU_EXTREG_PLL4_SETTING_SET_en(0);
    delay_us(1000);
    SCU_EXTREG_CLK_EN0_SET_pll4_fref_pll0(0);
    delay_us(1000);
#endif
}

void clock_mgr_close_pll5(void)
{

}

void clock_mgr_change_pll3_clock(u32 ms, u32 ns, u32 ps, 
        u32 csi0_txesc, u32 csi0_csi, u32 csi0_vc0,
        u32 csi1_txesc, u32 csi1_csi, u32 csi1_vc0)
{
#if 1//def PLL3_INITED_IN_SYTEM_INIT    
    // Set the new M/N/P values.
    masked_outw(SCU_EXTREG_PLL3_SETTING, 
                ((ps << SCU_EXTREG_PLL3_SETTING_ps_START) | 
                 (ns << SCU_EXTREG_PLL3_SETTING_ns_START) | 
                 (ms << SCU_EXTREG_PLL3_SETTING_ms_START) | 
                 (5 << SCU_EXTREG_PLL3_SETTING_is_START) | 
                 (3 << SCU_EXTREG_PLL3_SETTING_rs_START)),
    
                (SCU_EXTREG_PLL3_SETTING_ps_MASK | SCU_EXTREG_PLL3_SETTING_ns_MASK |
                 SCU_EXTREG_PLL3_SETTING_ms_MASK | SCU_EXTREG_PLL3_SETTING_is_MASK |
                 SCU_EXTREG_PLL3_SETTING_rs_MASK));
    
    //divider
    {            
        SCU_EXTREG_CLK_DIV1_SET_csirx0_TxEscClk(csi0_txesc);
        SCU_EXTREG_CLK_DIV1_SET_csirx0_csi(csi0_csi);
        SCU_EXTREG_CLK_DIV1_SET_csirx0_vc0(csi0_vc0);

        SCU_EXTREG_CLK_DIV7_SET_csirx1_TxEscClk_pll3(csi1_txesc);
        SCU_EXTREG_CLK_DIV7_SET_csi1_csi_pll3(csi1_csi);
        SCU_EXTREG_CLK_DIV7_SET_csi1_vc0_pll3(csi1_vc0);
    }
    
#else    
    SCU_EXTREG_CLK_EN1_SET_csirx0_TxEscClk(0);
    SCU_EXTREG_CLK_EN1_SET_csirx0_vc0(0);
    SCU_EXTREG_CLK_EN1_SET_csirx0_csi(0);
    
    // Disable PLL output (clock gated)
    SCU_EXTREG_CLK_EN0_SET_pll3_out2(0);
    SCU_EXTREG_CLK_EN0_SET_pll3_out1(0);

    // Power down PLL.
    SCU_EXTREG_PLL3_SETTING_SET_en(0);
    

    // Set the new M/N/P values.
    masked_outw(SCU_EXTREG_PLL3_SETTING, 
                ((ps << SCU_EXTREG_PLL3_SETTING_ps_START) | 
                 (ns << SCU_EXTREG_PLL3_SETTING_ns_START) | 
                 (ms << SCU_EXTREG_PLL3_SETTING_ms_START) | 
                 (5 << SCU_EXTREG_PLL3_SETTING_is_START) | 
                 (3 << SCU_EXTREG_PLL3_SETTING_rs_START)),
    
                (SCU_EXTREG_PLL3_SETTING_ps_MASK | SCU_EXTREG_PLL3_SETTING_ns_MASK |
                 SCU_EXTREG_PLL3_SETTING_ms_MASK | SCU_EXTREG_PLL3_SETTING_is_MASK |
                 SCU_EXTREG_PLL3_SETTING_rs_MASK));    
    
//dbg_msg("SCU_EXTREG_PLL3_SETTING_GET_ps()=%x", SCU_EXTREG_PLL3_SETTING_GET_ps());
//dbg_msg("SCU_EXTREG_PLL3_SETTING_GET_ns()=%x", SCU_EXTREG_PLL3_SETTING_GET_ns());
//dbg_msg("SCU_EXTREG_PLL3_SETTING_GET_ms()=%x", SCU_EXTREG_PLL3_SETTING_GET_ms());
//dbg_msg("SCU_EXTREG_PLL3_SETTING_GET_is()=%x", SCU_EXTREG_PLL3_SETTING_GET_is());
//dbg_msg("SCU_EXTREG_PLL3_SETTING_GET_rs()=%x", SCU_EXTREG_PLL3_SETTING_GET_rs());
    
    // For PLL510 series, wait for 40us reset time
    delay_us(40);

    // Enable PLL.
    SCU_EXTREG_PLL3_SETTING_SET_en(1);

    // Wait for PLL locking time. (PLL110: 50us, PLL510: 350us)
    delay_us(350);
    
    //SCU_EXTREG_CLK_MUX_SEL_SET_csirx1_clk(1);

	//divider
    SCU_EXTREG_CLK_DIV1_SET_csirx0_TxEscClk(5);
    SCU_EXTREG_CLK_DIV1_SET_csirx0_csi(0x1b);
    SCU_EXTREG_CLK_DIV1_SET_csirx0_vc0(0x0d);

    //SCU_EXTREG_CLK_DIV7_SET_csirx1_TxEscClk_pll3(5);
    //SCU_EXTREG_CLK_DIV7_SET_csi1_csi_pll3(15);
    //SCU_EXTREG_CLK_DIV7_SET_csi1_vc0_pll3(4);

    // Enable PLL output.
    SCU_EXTREG_CLK_EN0_SET_pll3_out2(1);
    SCU_EXTREG_CLK_EN0_SET_pll3_out1(1);
    
    SCU_EXTREG_CLK_EN1_SET_csirx0_TxEscClk(1);
    SCU_EXTREG_CLK_EN1_SET_csirx0_vc0(1);
    SCU_EXTREG_CLK_EN1_SET_csirx0_csi(1);
#endif    
}

void clock_mgr_change_pll5_clock(u32 ms, u32 ns, u32 ps)
{
    // Disable PLL output (clock gated)

    SCU_EXTREG_CLK_EN0_SET_pll5_out2(0);
    SCU_EXTREG_CLK_EN0_SET_pll5_out1(0);

    // Power down PLL.
    SCU_EXTREG_PLL5_SETTING_SET_en(0);    

    // Set the new M/N/P values.
    masked_outw(SCU_EXTREG_PLL5_SETTING, 
                (ps << SCU_EXTREG_PLL5_SETTING_ps_START) | 
                (ns << SCU_EXTREG_PLL5_SETTING_ns_START) | 
                (ms << SCU_EXTREG_PLL5_SETTING_ms_START) | 
                (5 << SCU_EXTREG_PLL5_SETTING_is_START) | 
                (3 << SCU_EXTREG_PLL5_SETTING_rs_START),
    
                (SCU_EXTREG_PLL5_SETTING_ps_MASK | SCU_EXTREG_PLL5_SETTING_ns_MASK |
                SCU_EXTREG_PLL5_SETTING_ms_MASK | SCU_EXTREG_PLL5_SETTING_is_MASK |
                SCU_EXTREG_PLL5_SETTING_rs_MASK));

    // For PLL510 series, wait for 40us reset time
    delay_us(40);

    // Enable PLL.
    SCU_EXTREG_PLL5_SETTING_SET_en(1);

    // Wait for PLL locking time. (PLL110: 50us, PLL510: 350us)
    delay_us(350);

    // Enable PLL output.  
    SCU_EXTREG_CLK_EN0_SET_pll5_out2(1);
    SCU_EXTREG_CLK_EN0_SET_pll5_out1(1); 
}

static void clk_control(enum clk clk, int enable)
{
    switch (clk) {
        case CLK_PLL1:
            SCU_EXTREG_PLL1_SETTING_SET_en(enable);
            break;
        case CLK_PLL2:
            SCU_EXTREG_PLL2_SETTING_SET_en(enable);
            break;
        case CLK_PLL3:
            SCU_EXTREG_PLL3_SETTING_SET_en(enable);
            break;
        case CLK_PLL4:
            SCU_EXTREG_PLL4_SETTING_SET_en(enable);
            break;
        case CLK_PLL5:
            SCU_EXTREG_PLL5_SETTING_SET_en(enable);
            break;
        case CLK_PLL1_OUT:
            SCU_EXTREG_CLK_EN0_SET_pll1_out(enable);
            break;
        case CLK_PLL2_OUT:
            SCU_EXTREG_CLK_EN0_SET_pll2_out(enable);
            break;
        case CLK_PLL3_OUT1:
            SCU_EXTREG_CLK_EN0_SET_pll3_out1(enable);
            break;
        case CLK_PLL3_OUT2:
            SCU_EXTREG_CLK_EN0_SET_pll3_out2(enable);
            break;
        case CLK_PLL4_OUT:
            SCU_EXTREG_CLK_EN0_SET_pll4_out1(enable);
            break;
        case CLK_PLL5_OUT1:
            SCU_EXTREG_CLK_EN0_SET_pll5_out1(enable);
            break;
        case CLK_PLL5_OUT2:
            SCU_EXTREG_CLK_EN0_SET_pll5_out2(enable);
            break;

        case CLK_SCPU_TRACE:
            SCU_EXTREG_CLK_EN0_SET_scpu_traceclk(enable);
            break;

        case CLK_NCPU:
            SCU_EXTREG_CLK_EN0_SET_ncpu_fclk_src(enable);
            break;
        case CLK_NCPU_TRACE:
            SCU_EXTREG_CLK_EN0_SET_ncpu_traceclk(enable);
            break;

        case CLK_NPU:
            SCU_EXTREG_CLK_EN1_SET_npu(enable);
            break;

        case CLK_PLL4_FREF_PLL0:
            SCU_EXTREG_CLK_EN0_SET_pll4_fref_pll0(enable);
            break;
        case CLK_FCS_PLL2:
            SCU_REG_PLL2_CTRL_SET_PLL2EN(enable);
            break;
        case CLK_FCS_DLL:
            SCU_REG_DLL_CTRL_SET_DLLEN(enable);
            break;

        case CLK_SPI_CLK:
            SCU_EXTREG_CLK_EN1_SET_spi_clk(enable);
            break;
        case CLK_ADC_CLK:
            SCU_EXTREG_CLK_EN1_SET_adcclk(enable);
            break;
        case CLK_WDT_EXT_CLK:
            SCU_EXTREG_CLK_EN1_SET_wdt_extclk(enable);
            break;
        case CLK_SD_CLK:
            SCU_EXTREG_CLK_EN1_SET_sdclk(enable);
            break;
        case CLK_MIPI_TXHSPLLREF_CLK:
            SCU_EXTREG_CLK_EN1_SET_TxHsPllRefClk(enable);
            break;
        case CLK_MIPI_TX_ESC_CLK:
            SCU_EXTREG_CLK_EN1_SET_tx_EscClk(enable);
            break;
        case CLK_MIPI_CSITX_DSI_CLK:
            SCU_EXTREG_CLK_EN1_SET_csitx_dsi(enable);
            break;
        case CLK_MIPI_CSITX_CSI_CLK:
            SCU_EXTREG_CLK_EN1_SET_csitx_csi(enable);
            break;
        case CLK_MIPI_CSIRX1_TXESC_CLK:
            SCU_EXTREG_CLK_EN1_SET_csirx1_TxEscClk(enable);
            break;
        case CLK_MIPI_CSIRX1_CSI_CLK:
            SCU_EXTREG_CLK_EN1_SET_csirx1_csi(enable);
            break;
        case CLK_MIPI_CSIRX1_VC0_CLK:
            SCU_EXTREG_CLK_EN1_SET_csirx1_vc0(enable);
            break;
        case CLK_MIPI_CSIRX0_TXESC_CLK:
            SCU_EXTREG_CLK_EN1_SET_csirx0_TxEscClk(enable);
            break;
        case CLK_MIPI_CSIRX0_CSI_CLK:
            SCU_EXTREG_CLK_EN1_SET_csirx0_csi(enable);
            break;
        case CLK_MIPI_CSIRX0_VC0_CLK:
            SCU_EXTREG_CLK_EN1_SET_csirx0_vc0(enable);
            break;
        case CLK_LC_SCALER:
            SCU_EXTREG_CLK_EN1_SET_LC_SCALER(enable);
            break;
        case CLK_LC_CLK:
            SCU_EXTREG_CLK_EN1_SET_LC_CLK(enable);
            break;
        case CLK_TMR1_EXTCLK3:
            SCU_EXTREG_CLK_EN2_SET_tmr1_extclk3(enable);
            break;
        case CLK_TMR1_EXTCLK2:
            SCU_EXTREG_CLK_EN2_SET_tmr1_extclk2(enable);
            break;
        case CLK_TMR1_EXTCLK1:
            SCU_EXTREG_CLK_EN2_SET_tmr1_extclk1(enable);
            break;
        case CLK_TMR0_EXTCLK3:
            SCU_EXTREG_CLK_EN2_SET_tmr0_extclk3(enable);
            break;
        case CLK_TMR0_EXTCLK2:
            SCU_EXTREG_CLK_EN2_SET_tmr0_extclk2(enable);
            break;
        case CLK_TMR0_EXTCLK1:
            SCU_EXTREG_CLK_EN2_SET_tmr0_extclk1(enable);
            break;
        case CLK_PWM_EXTCLK6:
            SCU_EXTREG_CLK_EN2_SET_pwm_extclk6(enable);
            break;
        case CLK_PWM_EXTCLK5:
            SCU_EXTREG_CLK_EN2_SET_pwm_extclk5(enable);
            break;
        case CLK_PWM_EXTCLK4:
            SCU_EXTREG_CLK_EN2_SET_pwm_extclk4(enable);
            break;
        case CLK_PWM_EXTCLK3:
            SCU_EXTREG_CLK_EN2_SET_pwm_extclk3(enable);
            break;
        case CLK_PWM_EXTCLK2:
            SCU_EXTREG_CLK_EN2_SET_pwm_extclk2(enable);
            break;
        case CLK_PWM_EXTCLK1:
            SCU_EXTREG_CLK_EN2_SET_pwm_extclk1(enable);
            break;
        case CLK_UART1_3_FREF:
            SCU_EXTREG_CLK_EN2_SET_uart1_3_fref(enable);
            break;
        case CLK_UART1_2_FREF:
            SCU_EXTREG_CLK_EN2_SET_uart1_2_fref(enable);
            break;
        case CLK_UART1_1_FREF:
            SCU_EXTREG_CLK_EN2_SET_uart1_1_fref(enable);
            break;
        case CLK_UART1_0_FREF:
            SCU_EXTREG_CLK_EN2_SET_uart1_0_fref(enable);
            break;
        case CLK_UART0_FREF:
            SCU_EXTREG_CLK_EN2_SET_uart0_fref(enable);
            break;
        case CLK_SSP1_1_SSPCLK:
            SCU_EXTREG_CLK_EN2_SET_ssp1_1_sspclk(enable);
            break;
        case CLK_SSP1_0_SSPCLK:
            SCU_EXTREG_CLK_EN2_SET_ssp1_0_sspclk(enable);
            break;
        case CLK_SSP0_1_SSPCLK:
            SCU_EXTREG_CLK_EN2_SET_ssp0_1_sspclk(enable);
            break;
        case CLK_SSP0_0_SSPCLK:
            SCU_EXTREG_CLK_EN2_SET_ssp0_0_sspclk(enable);
            break;
    }
}

void clk_enable(enum clk clk)
{
    clk_control(clk, 1);
}

void clk_disable(enum clk clk)
{
    clk_control(clk, 0);
}

#endif
