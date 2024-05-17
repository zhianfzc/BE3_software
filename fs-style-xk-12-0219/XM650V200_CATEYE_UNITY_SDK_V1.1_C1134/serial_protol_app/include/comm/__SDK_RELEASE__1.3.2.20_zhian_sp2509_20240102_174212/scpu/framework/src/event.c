#include "framework/event.h"
#include "dbg.h"

osEventFlagsId_t evt_source_data_ready_id;
osEventFlagsId_t evt_mipi0_ready_id;
osEventFlagsId_t evt_vb_done_id_0;
osEventFlagsId_t evt_vb_done_id_1;

osEventFlagsId_t create_event(void) {
    return osEventFlagsNew(NULL);
}

void set_event(osEventFlagsId_t id, unsigned int flags) {
    osEventFlagsSet(id, flags);
}

unsigned int wait_event(osEventFlagsId_t id, unsigned int flags) {   
    return osEventFlagsWait(id, flags, osFlagsWaitAny, osWaitForever);
}

unsigned int clear_event(osEventFlagsId_t id, unsigned int flags)
{
    return flags;
    //return osEventFlagsClear(id, flags);
}

void delete_event(osEventFlagsId_t id) {
    osEventFlagsDelete(id);
}

void set_thread_event(osThreadId_t id, unsigned int flags) {
    osThreadFlagsSet(id, flags);
}

void wait_thread_event(unsigned int flags) {
    osThreadFlagsWait(flags, osFlagsWaitAny, osWaitForever);
}

BOOL check_thread_alive(osEventFlagsId_t id)
{
    BOOL ret = TRUE;
    osThreadState_t state = osThreadGetState(id);

    if ( ( state == osThreadTerminated ) || ( state == osThreadError ) )
    {
        ret = FALSE;
    }

    return ret;
}

