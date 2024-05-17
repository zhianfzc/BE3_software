#pragma once

#include "types.h"
#include "gui_base.h"


typedef int user_behavior_type;
typedef struct _user_behavior_data
{
    user_behavior_type type;
    u32 data;
    u32 data2;
} user_behavior_data;

typedef struct _user_behavior_ui_interface
{
    void (*task_run)(void *param);
    void (*push_event)(user_behavior_data data);
    void (*get_event)(user_behavior_data* a);
    unsigned char is_run;
    char main_state;
} user_behavior_ui_interface;

extern void user_interface_fsm_thread(void *arg);
extern void notify_user_behavior_event(user_behavior_type type,unsigned short data, u8 input_src);
extern u8 ui_get_main_state(void);
