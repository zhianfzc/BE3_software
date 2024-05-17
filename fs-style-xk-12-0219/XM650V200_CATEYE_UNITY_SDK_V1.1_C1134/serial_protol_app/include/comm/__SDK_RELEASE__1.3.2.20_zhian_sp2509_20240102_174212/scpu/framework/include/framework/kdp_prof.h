#ifndef __KDP_PROF_H__
#define __KDP_PROF_H__

#ifndef LOG_ENABLE

#define dbg_profile_start()
#define dbg_profile_stop(tag)

#else

#include <stdint.h>
#include "cmsis_os2.h"

extern uint32_t kdp_prof_start_t;
extern uint32_t kdp_prof_end_t;

#define dbg_profile_start()  kdp_prof_start_t = osKernelGetTickCount();
#define dbg_profile_stop(tag)                                      \
    do {                                                           \
        kdp_prof_end_t = osKernelGetTickCount();                   \
        profile_msg("[TIME] <%s> elapses %d ms\n",                 \
                    tag, (kdp_prof_end_t - kdp_prof_start_t));     \
    } while(0)

#endif // LOG_ENABLE

#endif
