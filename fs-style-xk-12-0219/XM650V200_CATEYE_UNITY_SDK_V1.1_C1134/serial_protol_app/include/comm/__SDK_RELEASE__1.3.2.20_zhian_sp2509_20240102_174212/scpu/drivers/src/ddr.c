/* -------------------------------------------------------------------------- 
 * @file ddr.c 
 *      DDR init function for different module
 *---------------------------------------------------------------------------*/

#include "kneron_mozart.h" 
#include "base.h"
#include "types.h"
#include "io.h"
#include "ddr.h"
#include "scu_reg.h"
#include "scu_extreg.h"
#include "system.h"
#include "delay.h"
#include "dbg.h"

#define DDR_BASE                                        DDRC_FTDDR3030_PA_BASE
#define DDR_REG_MCSR                                    (DDR_BASE + 0x0004)

/* Memory Controller State Control (Offset: 0x0004) */
#define DDR_REG_MCSR_EXIT_SELF_REFRESH                  BIT3
#define DDR_REG_MCSR_SELF_REFRESH                       BIT2
#define DDR_REG_MCSR_INIT_CMD                           BIT0

#ifdef KL520

#define DDR_DORMANT_BOOTUP      (SCU_REG_BTUP_STS_RTC_BTUPTS | SCU_REG_BTUP_STS_SMR)

extern u32 bootup_status;

void ddr_self_refresh_enter(void)
{
    masked_outw(DDR_REG_MCSR, DDR_REG_MCSR_SELF_REFRESH, DDR_REG_MCSR_SELF_REFRESH);
}

void ddr_self_refresh_exit(void)
{
    masked_outw(DDR_REG_MCSR, DDR_REG_MCSR_EXIT_SELF_REFRESH, DDR_REG_MCSR_EXIT_SELF_REFRESH);
}

void ddr_wakeup(void)
{   
    delay_us(10);
    masked_outw(SCU_EXTREG_DDR_CTRL,
                (SCU_EXTREG_DDR_CTRL_wakeup | SCU_EXTREG_DDR_CTRL_SELFBIAS),
                (SCU_EXTREG_DDR_CTRL_wakeup | SCU_EXTREG_DDR_CTRL_SELFBIAS));    
    delay_us(10);
}

void ddr_init(enum ddr_init_mode mode)
{       
    int val;

	if((bootup_status & DDR_DORMANT_BOOTUP) == DDR_DORMANT_BOOTUP)
		mode = DDR_INIT_ALL_EXIT_SELF_REFRESH;

    ddr_wakeup();
    if (DDR_INIT_WAKEUP_ONLY == mode)
        return;

    outw(DDR_BASE + 0x100 , 0x000206C3);
    outw(DDR_BASE + 0x104 , 0x0000ff00);
    outw(DDR_BASE + 0x108 , 0x000000ff);

    outw(DDR_BASE + 0x04 , 0x00000000);
    outw(DDR_BASE + 0x08 , 0x00041c70);
    outw(DDR_BASE + 0x0C , 0x00000018);

    outw(DDR_BASE + 0x10 , 0x60007272);
    outw(DDR_BASE + 0x14 , 0x170D0F0B);
    outw(DDR_BASE + 0x18 , 0x21611323);
    outw(DDR_BASE + 0x1C , 0x00002041);

    outw(DDR_BASE + 0x20 , 0x00000f02);
    outw(DDR_BASE + 0x24 , 0x77777777);
    outw(DDR_BASE + 0x130 , 0x77777777);
	outw(DDR_BASE + 0x28 , /*0x00000a29*/1);

    outw(DDR_BASE + 0x30 , 0x90000000);
    outw(DDR_BASE + 0x34 , 0x03030303);
    outw(DDR_BASE + 0x38 , 0x03030303);
    outw(DDR_BASE + 0x3C , 0x00030011);
    //old setting
    //outw(DDR_BASE + 0x48 , 0x00000002);
//    outw(DDR_BASE + 0x48 , 0x00005401);
    outw(DDR_BASE + 0x48 , 0x00005502);     // bufferable for each ch   (RD change 2023.4.26)
                                            // [9:8]    CH0_hprot_sel_AXI
                                            // [11:10]  CH1_hprot_sel_SAHB
                                            // [13:12]  CH2_hprot_sel_NAHB
                                            // [15:14]  CH3_hprot_sel_PAHB
    
	outw(DDR_BASE + 0x70 , /*0x000000ff*/0x0);
    //outw(DDR_BASE + 0x74 , 0x44444444);
//    outw(DDR_BASE + 0x78 , 0xEEEEEEEE);
    outw(DDR_BASE + 0x78 , 0x77777777);         // RD change 2023.4.26
    outw(DDR_BASE + 0xA0 , 0x03080303);         // RD change 2023.4.26

    outw(DDR_BASE + 0xA8 , 0x0000D000);
    outw(DDR_BASE + 0xAC , 0x00000100);

    outw(DDR_BASE + 0x10C , 0x00000000);
    outw(DDR_BASE + 0x110 , 0x00000000);
    outw(DDR_BASE + 0x114 , 0x00000000);
    outw(DDR_BASE + 0x11C , 0x00000000);
    outw(DDR_BASE + 0x138 , 0x00000005);
    
    delay_us(50);

    outw(DDR_BASE + 0x00 , 0x080b9d03);
    
    if (DDR_INIT_ALL == mode)
        outw(DDR_REG_MCSR, DDR_REG_MCSR_INIT_CMD);
    else if (DDR_INIT_ALL_EXIT_SELF_REFRESH == mode)
        outw(DDR_REG_MCSR, DDR_REG_MCSR_INIT_CMD | DDR_REG_MCSR_EXIT_SELF_REFRESH);
    
    do {
        val = inw(DDR_BASE + 0x04);
    } while ((val & 0x100) == 0);
    
	// Reset Dphy after FTDDR3030 init OK
    SCU_EXTREG_DDR_CTRL_SET_Dphy_resetn(1);
    delay_us(30);
    SCU_EXTREG_DDR_CTRL_SET_Dphy_resetn(0);
    delay_us(500);
}

void ddr_init_chip_0429(void)
{   
    //DDR test code, fail
    int val;
    int i= 50000;
    outw(DDR_BASE + 0x100 , 0x000206C3);
    outw(DDR_BASE + 0x104 , 0x0000ff00);
    outw(DDR_BASE + 0x108 , 0x000000ff);

    outw(DDR_BASE + 0x04 , 0x00000000);
    outw(DDR_BASE + 0x08 , 0x00041c70);
    outw(DDR_BASE + 0x0C , 0x00000018);

    outw(DDR_BASE + 0x10 , 0x60007272);
    outw(DDR_BASE + 0x14 , 0x170D0F0B);
    outw(DDR_BASE + 0x18 , 0x21611323);
    outw(DDR_BASE + 0x1C , 0x00002041);

    outw(DDR_BASE + 0x20 , 0x00000f02);
    outw(DDR_BASE + 0x24 , 0x77777777);
    outw(DDR_BASE + 0x130 , 0x77777777);
    outw(DDR_BASE + 0x28 , 0x00000001);

    outw(DDR_BASE + 0x30 , 0x90000000);
    outw(DDR_BASE + 0x34 , 0x03030303);
    outw(DDR_BASE + 0x38 , 0x03030303);
    outw(DDR_BASE + 0x3C , 0x00030011);

    outw(DDR_BASE + 0x48 , 0x00000002);

    outw(DDR_BASE + 0x70 , 0x00000000);
    //outw(DDR_BASE + 0x74 , 0x44444444);
    outw(DDR_BASE + 0x78 , 0xEEEEEEEE);

    outw(DDR_BASE + 0xA8 , 0x0000D000);
    outw(DDR_BASE + 0xAC , 0x00000100);

    outw(DDR_BASE + 0x10C , 0x00000000);
    outw(DDR_BASE + 0x110 , 0x00000000);
    outw(DDR_BASE + 0x114 , 0x00000000);
    outw(DDR_BASE + 0x11C , 0x00000000);
    outw(DDR_BASE + 0x138 , 0x00000005);

    i = 5000;
    while(i--);

    outw(DDR_BASE + 0x00 , 0x080b9d03);
    outw(DDR_BASE + 0x04 , 0x00000001);
    do {
        val = inw(DDR_BASE + 0x04);
    } while ((val & 0x100) == 0);
}


#endif
