#ifndef __XM530V200V200_H__
#define __XM530V200V200_H__

#include <rtdef.h>
#include "armv7.h"


#define XM_TIMER01_BASE			0x100C0000
#define XM_TIMER23_BASE			0x100D0000
#define XM_UART0_BASE			0x10030000
#define XM_UART1_BASE			0x10040000
#define XM_GMAC_BASE			0x10010000
#define XM_GPIO_BASE       		0x10020000
#define XM_SYS_CTRL_BASE  		0x20000000
#define XM_DMAC_BASE			0x20020000
#define XM_SDIO0_BASE      		0x50000000
#define XM_SDIO1_BASE      		0x50020000
#define XM_USB0_BASE			0x50300000

#define XM_GIC_CPU_BASE			0x20200100  /* Generic interrupt controller CPU interface */
#define XM_GIC_DIST_BASE       	0x20201000  /* Generic interrupt controller distributor */

#define XM_GIC_IRQ_START	(32)

#define IRQ_XM_TIMER01   	(XM_GIC_IRQ_START + 4)    /* 33 */
#define IRQ_XM_TIMER23   	(XM_GIC_IRQ_START + 5)
#define IRQ_XM_UART0		(XM_GIC_IRQ_START + 0)
#define IRQ_XM_UART1		(XM_GIC_IRQ_START + 1)
#define IRQ_XM_UART2		(XM_GIC_IRQ_START + 2)
#define IRQ_XM_GMAC			(XM_GIC_IRQ_START + 3)
#define IRQ_XM_DMAC			(XM_GIC_IRQ_START + 10)
#define IRQ_XM_SDIO0		(XM_GIC_IRQ_START + 41)
#define IRQ_XM_SDIO1		(XM_GIC_IRQ_START + 42)
#define IRQ_XM_USB0			(XM_GIC_IRQ_START + 44)

#define ARM_GIC_NR_IRQS		(XM_GIC_IRQ_START + 96)

#define MAX_HANDLERS 	ARM_GIC_NR_IRQS
#define GIC_IRQ_START	0
#define GIC_ACK_INTID_MASK              0x000003ff   
/* the basic constants and interfaces needed by gic */
rt_inline rt_uint32_t platform_get_gic_dist_base(void)
{
    return XM_GIC_DIST_BASE;
}

rt_inline rt_uint32_t platform_get_gic_cpu_base(void)
{
    return XM_GIC_CPU_BASE;
}
/* only one GIC available */
#define ARM_GIC_MAX_NR      1

#define XM_SYS_SOFT_RSTEN           0x20000080
#define XM_SYS_SOFT_RST             0x20000084

#define REG_SC_REMAP        0x0204

#define REG_SYS_SOFT_RSTEN  0x80  
#define REG_SYS_SOFT_RST    0x84
#define REG_CPU1_SOFT_RST   0x8C

#define GPIO_MUX1_EN       	(1 << 1)
#define GPIO_MUX2_EN       	(1 << 2)
#define GPIO_MUX3_EN       	(1 << 3)
#define GPIO_OUT_EN        	(1 << 10)
#define GPIO_OUT_HIGH      	(1 << 11)
#define GPIO_OUT_LOW       	(0 << 11)
#define GPIO_IN_EN         	(1 << 12)
#define GPIO_IN_DATA       	(1 << 13)
#define GPIO_DRIVE_2MA      (0 << 6)
#define GPIO_DRIVE_4MA      (1 << 6)
#define GPIO_DRIVE_8MA      (2 << 6)
#define GPIO_DRIVE_12MA     (3 << 6)

#define TIMER0_LOAD              (XM_TIMER01_BASE + 0x00)
#define TIMER0_VALUE             (XM_TIMER01_BASE + 0x04)
#define TIMER0_CTRL              (XM_TIMER01_BASE + 0x08)
#define TIMER0_INTCLR            (XM_TIMER01_BASE + 0x0c)
#define TIMER0_RIS               (XM_TIMER01_BASE + 0x10)
#define TIMER0_MIS               (XM_TIMER01_BASE + 0x14)
#define TIMER0_BGLOAD            (XM_TIMER01_BASE + 0x18)
#define TIMER1_LOAD              (XM_TIMER01_BASE + 0x20)
#define TIMER1_VALUE             (XM_TIMER01_BASE + 0x24)
#define TIMER1_CTRL              (XM_TIMER01_BASE + 0x28)
#define TIMER1_INTCLR            (XM_TIMER01_BASE + 0x2c)
#define TIMER1_RIS               (XM_TIMER01_BASE + 0x30)
#define TIMER1_MIS               (XM_TIMER01_BASE + 0x34)
#define TIMER1_BGLOAD            (XM_TIMER01_BASE + 0x38)
#define TIMER2_LOAD              (XM_TIMER23_BASE + 0x00)
#define TIMER2_VALUE             (XM_TIMER23_BASE + 0x04)
#define TIMER2_CTRL              (XM_TIMER23_BASE + 0x08)
#define TIMER2_INTCLR            (XM_TIMER23_BASE + 0x0c)
#define TIMER2_RIS               (XM_TIMER23_BASE + 0x10)
#define TIMER2_MIS               (XM_TIMER23_BASE + 0x14)
#define TIMER2_BGLOAD            (XM_TIMER23_BASE + 0x18)
#define TIMER_CTRL_ONESHOT      (1 << 0)
#define TIMER_CTRL_32BIT        (1 << 1)
#define TIMER_CTRL_DIV1         (0 << 2)
#define TIMER_CTRL_DIV16        (1 << 2)
#define TIMER_CTRL_DIV256       (2 << 2)
#define TIMER_CTRL_IE           (1 << 5)        /* Interrupt Enable (versatile only) */
#define TIMER_CTRL_PERIODIC     (1 << 6)
#define TIMER_CTRL_ENABLE       (1 << 7)


#define PLL_PLLA_CTRL                0x20000008     
#define PLL_PLLA_FRAC                0x2000000C     
#define PLL_CPUCLK_CTRL              0x20000030


#define BIT(nr)         (1UL << (nr))
#define GT_COUNTER0 0x20200200
#define GT_COUNTER1 0x20200204
#define GT_CONTROL  0x20200208

#define GT_CONTROL_TIMER_ENABLE     BIT(0)  /* this bit is NOT banked */
#define GT_CONTROL_COMP_ENABLE      BIT(1)  /* banked */
#define GT_CONTROL_IRQ_ENABLE       BIT(2)  /* banked */
#define GT_CONTROL_AUTO_INC     BIT(3)  /* banked */




#endif
