#pragma once

#include "types.h"
#include "kl520_include.h"
#include "framework/mutex.h"


#define GUI_TP_RANGE        (50)
#define MAX_OBS_NUM         (1)//only one view(display)

typedef int (*fp)(void);
typedef int (*input_fp)(void*);

typedef void (*reg_fsm)(void*);
typedef fp update_view;
typedef void (*attach)(update_view);
typedef fp update_model;
typedef fp model_notify_view;

typedef struct _gui_mvc_ctrlr
{
    fp reg_input;
    fp dereg_input;
    input_fp get_data;
    input_fp input_handler;
} gui_mvc_ctrlr;

typedef struct _gui_mvc_view
{
    fp reg_display;
    fp dereg_display;
    fp reg_all_gui_obj;
    fp renderer;
    struct mutex lock;
} gui_mvc_view;

typedef struct _gui_mvc_model
{
    reg_fsm reg_fsm;
    fp dereg_fsm;

    //observer part :
    update_view observer[MAX_OBS_NUM];
    u8 observer_num;
    update_view update_view;
    attach attach;
    model_notify_view model_notify_view;
    update_model update_model;

} gui_mvc_model;

//tp related
typedef enum _gui_tp_state
{
    GUI_TP_STATE_NONE = 0,
    GUI_TP_STATE_PRESS,
    GUI_TP_STATE_LONG_PRESS,
    GUI_TP_STATE_SLIDE_R,
    GUI_TP_STATE_SLIDE_L,
} gui_tp_state;
typedef struct _gui_tp_status
{
    u16 x;
    u16 y;
    u8 cnt;
    u8 slide_dir;
} gui_tp_status;


extern s16 gui_mvc_ctrlr_create(gui_mvc_ctrlr* ctrlr, fp reg_input, fp dereg_input, input_fp handler, input_fp get_data);
extern s16 gui_mvc_view_create(gui_mvc_view* view, fp reg_display, fp dereg_display, fp reg_all_gui_obj);
extern s16 gui_mvc_model_create(gui_mvc_model* model, reg_fsm reg_fsm, fp dereg_fsm);
extern int gui_touchpanel_handler(void*);
extern int gui_comm_handler(void*);
