#ifdef USE_KDRV
#include "drivers.h"
#include "kdrv_clock.h"
#else
#ifndef __CLOCK_H__
#define __CLOCK_H__


#include <types.h>

enum clk {
    CLK_PLL1            = 1,
    CLK_PLL1_OUT,
    CLK_PLL2,
    CLK_PLL2_OUT,
    CLK_PLL3,
    CLK_PLL3_OUT1,
    CLK_PLL3_OUT2,
    CLK_PLL4,
    CLK_PLL4_OUT,
    CLK_PLL5,
    CLK_PLL5_OUT1,
    CLK_PLL5_OUT2,

    CLK_FCS_PLL2        = 20,
    CLK_FCS_DLL,
    CLK_PLL4_FREF_PLL0,

    CLK_BUS_SAHB        = 30,
    CLK_BUS_NAHB,
    CLK_BUS_PAHB1,
    CLK_BUS_PAHB2,
    CLK_BUS_APB0,
    CLK_BUS_APB1,

    CLK_SCPU            = 50,
    CLK_SCPU_TRACE,

    CLK_NCPU            = 60,
    CLK_NCPU_TRACE,
    CLK_NPU,

    /* Peripheral clocks */
    CLK_SPI_CLK         = 100,
    CLK_ADC_CLK,
    CLK_WDT_EXT_CLK,
    CLK_SD_CLK,
    CLK_MIPI_TXHSPLLREF_CLK,
    CLK_MIPI_TX_ESC_CLK,
    CLK_MIPI_CSITX_DSI_CLK,
    CLK_MIPI_CSITX_CSI_CLK,
    CLK_MIPI_CSIRX1_TXESC_CLK,
    CLK_MIPI_CSIRX1_CSI_CLK,
    CLK_MIPI_CSIRX1_VC0_CLK,
    CLK_MIPI_CSIRX0_TXESC_CLK,
    CLK_MIPI_CSIRX0_CSI_CLK,
    CLK_MIPI_CSIRX0_VC0_CLK,
    CLK_LC_SCALER,
    CLK_LC_CLK,
    CLK_TMR1_EXTCLK3,
    CLK_TMR1_EXTCLK2,
    CLK_TMR1_EXTCLK1,
    CLK_TMR0_EXTCLK3,
    CLK_TMR0_EXTCLK2,
    CLK_TMR0_EXTCLK1,
    CLK_PWM_EXTCLK6,
    CLK_PWM_EXTCLK5,
    CLK_PWM_EXTCLK4,
    CLK_PWM_EXTCLK3,
    CLK_PWM_EXTCLK2,
    CLK_PWM_EXTCLK1,
    CLK_UART1_3_FREF,
    CLK_UART1_2_FREF,
    CLK_UART1_1_FREF,
    CLK_UART1_0_FREF,
    CLK_UART0_FREF,
    CLK_SSP1_1_SSPCLK,
    CLK_SSP1_0_SSPCLK,
    CLK_SSP0_1_SSPCLK,
    CLK_SSP0_0_SSPCLK,
};

enum pll_setting {
    PLL_MS=0,
    PLL_NS,
    PLL_PS,
    CSIRX0_TXESCCLK,
    CSIRX0_CSI,
    CSIRX0_VC0,
    CSIRX1_TXESCCLK,
    CSIRX1_CSI,
    CSIRX1_VC0
};
#define CLOCK_MUXSEL_NCPU_TRACECLK_DEFAULT              0x10000000
#define CLOCK_MUXSEL_NCPU_TRACECLK_FROM_SCPU_TRACECLK   0x20000000
#define CLOCK_MUXSEL_NCPU_TRACECLK_MASK                 0x30000000
#define CLOCK_MUXSEL_SCPU_TRACECLK_SRC_PLL0DIV3         0x01000000
#define CLOCK_MUXSEL_SCPU_TRACECLK_SRC_PLL0DIV2         0x02000000
#define CLOCK_MUXSEL_SCPU_TRACECLK_MASK                 0x03000000
#define CLOCK_MUXSEL_CSIRX1_CLK_PLL5                    0x00100000
#define CLOCK_MUXSEL_CSIRX1_CLK_PLL3                    0x00200000
#define CLOCK_MUXSEL_CSIRX1_CLK_MASK                    0x00300000
#define CLOCK_MUXSEL_NPU_CLK_PLL4                       0x00010000
#define CLOCK_MUXSEL_NPU_CLK_PLL5                       0x00020000
#define CLOCK_MUXSEL_NPU_CLK_PLL0                       0x00040000
#define CLOCK_MUXSEL_NPU_CLK_MASK                       0x00070000
#define CLOCK_MUXSEL_PLL4_FREF_PLL0DIV                  0x00001000
#define CLOCK_MUXSEL_PLL4_FREF_OSC                      0x00002000
#define CLOCK_MUXSEL_PLL4_MASK                          0x00003000
#define CLOCK_MUXSEL_UART_0_IRDA_UCLK_UART              0x00000100
#define CLOCK_MUXSEL_UART_0_IRDA_UCLK_IRDA              0x00000200
#define CLOCK_MUXSEL_UART_0_IRDA_UCLK_MASK              0x00000300

#define PLL3_INITED_IN_SYTEM_INIT

//enum clock_mux_selection {
//    ncpu_traceclk_default = 0x10000000,
//    ncpu_traceclk_from_scpu_traceclk = 0x20000000,
//    scpu_traceclk_src_pll0div3 = 0x01000000,
//    scpu_traceclk_src_pll0div2 = 0x02000000,    
//};

enum pll_id {
    /* pll_0 = 0, */
    pll_1 = 0,
    pll_2,
    pll_3,
    pll_4,
    pll_5
};

enum scuclkin_type {
    scuclkin_osc = 0,
    scuclkin_rtcosc,
    scuclkin_pll0div3,
    scuclkin_pll0div4,
};

struct clock_value {
    u16 ms;
    u16 ns;
    u16 ps;
    u8 div;
    u8 enable;
};

struct clock_list {
    struct clock_list *next;
};

struct clock_node;
typedef int (*fn_set)(struct clock_node *, struct clock_value *);
struct clock_node {
    struct clock_node *parent;
    struct clock_node *child_head;
    struct clock_node *child_next;
    fn_set set;//int (*set)(struct clock_node *, struct clock_value *);
    u8 is_enabled;
    char name[15];

};


extern struct clock_node clock_node_pll1_out;

void clock_mgr_init(void);
void clock_mgr_open(struct clock_node *node, struct clock_value *clock_val);
void clock_mgr_close(struct clock_node *node);
void clock_mgr_set_scuclkin(enum scuclkin_type type, BOOL enable);
void clock_mgr_set_muxsel(u32 flags);
u32  clock_mgr_calculate_clockout(enum pll_id id, u16 ms, u16 ns, u16 F_ps);

void clock_mgr_open_pll1(void);
void clock_mgr_open_pll2(void);
void clock_mgr_open_pll3(void);
void clock_mgr_open_pll4(void);
void clock_mgr_open_pll5(void);

void clock_mgr_close_pll1(void);
void clock_mgr_close_pll2(void);
void clock_mgr_close_pll4(void);

//void clock_mgr_change_pll3_clock(u32 ms, u32 ns, u32 ps, 
//        u32 csi0_txesc, u32 csi0_csi, u32 csi0_vc0,
//        u32 csi1_txesc, u32 csi1_csi, u32 csi1_vc0);
void clock_mgr_change_pll3_clock(u16 *pll_table);
void clock_mgr_change_pll5_clock(u32 ms, u32 ns, u32 ps);

void debug_pll_clock(void);

void clk_enable(enum clk clk);
void clk_disable(enum clk clk);

#endif
#endif // end of #ifdef USE_KDRV
