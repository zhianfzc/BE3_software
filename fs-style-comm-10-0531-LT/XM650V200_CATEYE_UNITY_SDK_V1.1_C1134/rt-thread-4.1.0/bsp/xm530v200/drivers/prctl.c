/*
 * Copyright (C) 2018 Alibaba Group Holding Limited
 */

#include <rthw.h>
#include <rtthread.h>
#include <string.h>
#include <stdarg.h>
#include "prctl.h"


int prctl(int option, ...)
{
    va_list       ap;
    unsigned long arg;
    char        * p_str;
	rt_base_t level;  
	rt_thread_t  thread;

    if (option == PR_SET_NAME) {
        /* set ap to the start stack address of argument list */
        va_start(ap, option);

        /*
         * checkout the first argument from argument list, and force it
         * to  unsigned long, the ap address will be moved at same time.
         */

        arg = va_arg(ap, unsigned long);

        va_end(ap);

        p_str = (char *)arg;
        if (NULL == p_str) {
            return 1;
        }

		thread = rt_thread_self();

		level =  rt_hw_interrupt_disable();
		strncpy(thread->name, p_str, RT_NAME_MAX);
		rt_hw_interrupt_enable(level);    

        return 0;
    } else {
        return 1;
    }
}

