/*
 * File      : cpu.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2013, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-07-20     Bernard      first version
 */

#include <rthw.h>
#include <rtthread.h>
#include <finsh.h>
#include "board.h"

/**
 * reset cpu by dog's time-out
 *
 */
void rt_hw_cpu_reset()
{
	writel(1, XM_SYS_SOFT_RSTEN);
	writel(0xca110000, XM_SYS_SOFT_RST);
    while (1);  /* loop forever and wait for reset to happen */

    /* NEVER REACHED */
}


MSH_CMD_EXPORT_ALIAS(rt_hw_cpu_reset, reset, restart the system);
MSH_CMD_EXPORT_ALIAS(rt_hw_cpu_reset, reboot, restart the system);
