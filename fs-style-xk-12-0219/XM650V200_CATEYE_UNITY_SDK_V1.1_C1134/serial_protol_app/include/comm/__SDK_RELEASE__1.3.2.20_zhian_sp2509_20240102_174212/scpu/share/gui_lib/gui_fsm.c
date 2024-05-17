#include <stdio.h>
#include <string.h>
#include "gui_fsm.h"
#include "sample_gui_fsm_events.h"
#include "framework/event.h"
#include "cmsis_os2.h"
#include "kl520_include.h"
#include "dbg.h"


#if ( CFG_GUI_ENABLE == YES )

#define MAX_EVENT_QUEUE     (12)
#define MAX_MSG_SIZE        (sizeof(user_behavior_data))

static osMessageQueueId_t _event_queue = NULL;

static void _push_event(user_behavior_data data);
static void _run(void *param);
static void _get_event(user_behavior_data* a);
static user_behavior_ui_interface _ui_fsm =
{
    .task_run = _run,
    .push_event = _push_event,
    .get_event = _get_event,
    .is_run = 0,
    .main_state = 0
};


extern void user_interface_fsm_thread(void *arg)
{
    //start the finite state machine.
    if (0 == _ui_fsm.is_run)
        _ui_fsm.task_run( (void *)0 );//into _run while loop

    while(1);/* should never get here */
}

static void _push_event(user_behavior_data data) {
    osMessageQueuePut(_event_queue, &data, NULL, 0);
}
static void _get_event(user_behavior_data* a) {
    osMessageQueueGet(_event_queue, a, NULL, osWaitForever);
}

static void _run(void *param)
{
    user_behavior_data event;

    _event_queue = osMessageQueueNew(MAX_EVENT_QUEUE, MAX_MSG_SIZE, NULL);

    _ui_fsm.is_run = 1;
    _ui_fsm.main_state = 0;
    notify_user_behavior_event((user_behavior_type)0, 0, 0);

    while (1)
    {
        _ui_fsm.get_event(&event);
        _ui_fsm.main_state = run_main_state_machine(_ui_fsm.main_state, &event);

        osDelay(50);
    }
}

extern void notify_user_behavior_event(user_behavior_type type,unsigned short data, u8 input_src)
{
    user_behavior_data i;
    i.type = type;
    i.data = data;
    i.data2 = input_src;
    _ui_fsm.push_event(i);
}

extern u8 ui_get_main_state(void){
    return _ui_fsm.main_state;
}

#endif
