#ifndef __EVENT_H__
#define __EVENT_H__


#include <cmsis_os2.h>
#include "types.h"

#define FLAGS_VB_DONE           0xf1dd
#define FLAGS_VB_DONE_0         0xf1d0
#define FLAGS_VB_DONE_1         0xf1d1
#define FLAGS_SOURCE_READY_EVT  0x91ad


extern osEventFlagsId_t evt_source_data_ready_id;
extern osEventFlagsId_t evt_mipi0_ready_id;
extern osEventFlagsId_t evt_vb_done_id_0;
extern osEventFlagsId_t evt_vb_done_id_1;

osEventFlagsId_t create_event(void);
void set_event(osEventFlagsId_t id, unsigned int flags);
unsigned int wait_event(osEventFlagsId_t id, unsigned int flags);
unsigned int clear_event(osEventFlagsId_t id, unsigned int flags);
void delete_event(osEventFlagsId_t id);
void set_thread_event(osThreadId_t id, unsigned int flags);
void wait_thread_event(unsigned int flags);
BOOL check_thread_alive(osEventFlagsId_t id);

#endif
