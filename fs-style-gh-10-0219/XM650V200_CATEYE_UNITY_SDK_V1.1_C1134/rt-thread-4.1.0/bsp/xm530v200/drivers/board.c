/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-11-20     Bernard    the first version
 * 2018-11-22     Jesven     add rt_hw_spin_lock
 *                           add rt_hw_spin_unlock
 *                           add smp ipi init
 */

#include <rthw.h>
#include <rtthread.h>

#include "board.h"

#include <mmu.h>
#include <l2cache.h>

struct mem_desc platform_mem_desc[] = {
    {0x00000000, 0x10000000, 0x00000000, DEVICE_MEM},
    {0x10000000, 0x70000000, 0x10000000, DEVICE_MEM},
	{0x80000000, 0x90000000, 0x80000000, NORMAL_MEM}
};

const rt_uint32_t platform_mem_desc_size = sizeof(platform_mem_desc)/sizeof(platform_mem_desc[0]);

extern void rt_hw_mmu_init(void);
extern void rt_hw_init_mmu_table(struct mem_desc *mdesc, rt_uint32_t size);
extern void rt_hw_ipi_handler_install(int ipi_vector, rt_isr_handler_t ipi_isr_handler);

void idle_wfi(void)
{
    asm volatile ("wfi");
}

/**
 * This function will initialize beaglebone board
 */
void rt_hw_board_init(void)
{
    /* initialize hardware interrupt */
    rt_hw_interrupt_init();
	/* mmu init */
	rt_hw_init_mmu_table(platform_mem_desc, platform_mem_desc_size);
	rt_hw_mmu_init();
    /* initialize system heap */
    rt_system_heap_init(HEAP_BEGIN, HEAP_END);


    rt_components_board_init();
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);

    rt_thread_idle_sethook(idle_wfi);

#ifdef RT_USING_L2CACHE
	l2cache_init();
#endif
#ifdef RT_USING_SMP
    /* install IPI handle */
    rt_hw_ipi_handler_install(RT_SCHEDULE_IPI, rt_scheduler_ipi_handler);
#endif
}
