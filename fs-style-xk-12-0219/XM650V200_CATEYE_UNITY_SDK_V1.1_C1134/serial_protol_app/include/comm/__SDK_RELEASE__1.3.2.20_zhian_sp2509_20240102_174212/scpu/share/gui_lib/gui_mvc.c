#include <cmsis_os2.h>
#include <stdio.h>
#include <stdlib.h>
#include "gui_mvc.h"
#include "gui_fsm.h"
#include "gui_base.h"
#include "dbg.h"


#if ( CFG_GUI_ENABLE == YES )
#define GUI_TOUCH_PRESS_THR         (2)
#define GUI_TOUCH_LONG_PRESS_THR    (30)
#define GUI_TOUCH_RANGE_THR         (20)
#define GUI_TOUCH_SLIDE_RANGE_X     (80)

static gui_mvc_model* _model_ptr;
static gui_mvc_view* _view_ptr;

static int _gui_display_handler(void);
static void _attach_observer(update_view update_view);
static int _model_notify_view(void);
static int _update_model(void);

extern s16 gui_mvc_ctrlr_create(gui_mvc_ctrlr* ctrlr, fp reg_input, fp dereg_input, input_fp handler, input_fp get_data)
{
    ctrlr->input_handler = handler;
    ctrlr->reg_input = reg_input;
    ctrlr->dereg_input = dereg_input;
    ctrlr->get_data = get_data;

    return 0;
}

static gui_tp_status _status = {0};
static gui_tp_state tp_state = GUI_TP_STATE_NONE;
static int _gui_tp_state_processing(kl520_mouse_info info)
{
    switch(info.state) {
        case MOUSE_DOWN:
            _status.x = info.x;
            _status.y = info.y;
            //work around for quickly continuous touch(no NONE event).
            _status.cnt = 0;
            _status.slide_dir = 0;
            tp_state = GUI_TP_STATE_NONE;
            break;
        case MOUSE_MOVE:
            if ( GUI_TOUCH_RANGE_THR > abs(_status.x - info.x) &&
             GUI_TOUCH_RANGE_THR > abs(_status.y - info.y)) 
            {
                if ( GUI_TOUCH_LONG_PRESS_THR >= _status.cnt )
                    _status.cnt++;
            }
            else if (GUI_TOUCH_SLIDE_RANGE_X <= abs(_status.x - info.x)) {
                _status.cnt = 0;
                if (info.x > _status.x) _status.slide_dir = 1;
                if (info.x < _status.x) _status.slide_dir = 2;
            }
            break;
        case MOUSE_UP:
            if (GUI_TOUCH_LONG_PRESS_THR <= _status.cnt) tp_state = GUI_TP_STATE_LONG_PRESS;
            else if (GUI_TOUCH_PRESS_THR <= _status.cnt) tp_state = GUI_TP_STATE_PRESS;
            else if ( 1 == _status.slide_dir ) tp_state = GUI_TP_STATE_SLIDE_R;
            else if ( 2 == _status.slide_dir ) tp_state = GUI_TP_STATE_SLIDE_L;
            break;
        default :
            _status.x = 0;
            _status.y = 0;
            _status.cnt = 0;
            _status.slide_dir = 0;
            tp_state = GUI_TP_STATE_NONE;
            break;
    }
    
    switch(tp_state)
    {
        case GUI_TP_STATE_PRESS:
            dbg_msg_touch("PRESS, x=%d, y=%d", info.x, info.y);
            return tp_state;
        case GUI_TP_STATE_LONG_PRESS:
            dbg_msg_touch("LONG_PRESS, x=%d, y=%d", info.x, info.y);
            return tp_state;
        case GUI_TP_STATE_SLIDE_R:
            dbg_msg_touch("SLIDE_R, x=%d, y=%d", info.x, info.y);
            return -1;
        case GUI_TP_STATE_SLIDE_L:
            dbg_msg_touch("SLIDE_L, x=%d, y=%d", info.x, info.y);
            return -1;
        default:
            return -1;
    }
}
extern int gui_touchpanel_handler(void* input_ptr)
{
    u16 x_offset = 0, y_offset = 0;
    gui_base_get_tp_offset(&x_offset, &y_offset);

    //get data
    gui_mvc_ctrlr* ctrlr_ptr = input_ptr;
    kl520_mouse_info info = {MOUSE_NONE, 0, 0};
    ctrlr_ptr->get_data(&info);
    dbg_msg_touch("touch event:%d, x=%d, y=%d", info.state, info.x, info.y);

    //parse tp data
    if (-1 == _gui_tp_state_processing(info))
        return -1;

    //iteration
    struct kdp_list *head;
    gui_page* page = gui_page_get_present_instance();
    if (0 == page)
        return -1;
    else
        head = &page->ll;//change page content(ll head) here.

    gui_obj* p_obj;
    for (p_obj = kdp_list_first_entry(head, typeof(*p_obj), ll);
         &p_obj->ll != head->prev;
         p_obj = kdp_list_next_entry(p_obj, ll))
    {
        if (0 == p_obj->on_off) {continue;}
        if (TOUCH_INACTIVE == p_obj->touch_ctrl) {continue;}


//        dbg_msg_touch("x: %d, w: %d, y: %d, h: %d",    p_obj->x + p_obj->w/2 - GUI_TP_RANGE,
//                                                    p_obj->x + p_obj->w/2 + GUI_TP_RANGE,
//                                                    p_obj->y + p_obj->h/2 - GUI_TP_RANGE,
//                                                    p_obj->y + p_obj->h/2 + GUI_TP_RANGE);
//        if ( info.x+x_offset > (p_obj->x - GUI_TP_RANGE) &&
//             info.x+x_offset < (p_obj->x + p_obj->w + GUI_TP_RANGE) &&
//             info.y+y_offset > (p_obj->y - GUI_TP_RANGE) &&
//             info.y+y_offset < (p_obj->y + p_obj->h + GUI_TP_RANGE) )

        if ( info.x+x_offset > (p_obj->x + p_obj->w/2 - GUI_TP_RANGE) &&
             info.x+x_offset < (p_obj->x + p_obj->w/2 + GUI_TP_RANGE) &&
             info.y+y_offset > (p_obj->y + p_obj->h/2 - GUI_TP_RANGE) &&
             info.y+y_offset < (p_obj->y + p_obj->h/2 + GUI_TP_RANGE) )
        {
            p_obj->obj_cb(p_obj);
            return 0;
        }
    }

    return -1;
}
extern int gui_comm_handler(void* input_ptr)
{
    gui_mvc_ctrlr* ctrlr_ptr = input_ptr;

    //get data
    user_behavior_data info = {0xFF, 0xFF};
    ctrlr_ptr->get_data(&info);
    dbg_msg_console("event:%d, data=%d\n", info.type, info.data);

    struct kdp_list *head;
    gui_page* page = gui_page_get_present_instance();
    if (0 == page)
        return -1;
    else
        head = &page->ll;

    gui_obj* p_obj;
    for (p_obj = kdp_list_first_entry(head, typeof(*p_obj), ll);
         &p_obj->ll != head->prev;
         p_obj = kdp_list_next_entry(p_obj, ll))
    {
        if (0 == p_obj->on_off){continue;}

        if (p_obj->fsm_func == info.type) {
            p_obj->obj_cb(p_obj);
            return 0;
        }
    }

    return -1;
}

extern s16 gui_mvc_view_create(gui_mvc_view* view, fp reg_display, fp dereg_display, fp reg_all_gui_obj)
{
    _view_ptr = view;
    view->reg_display = reg_display;
    view->dereg_display = dereg_display;
    view->reg_all_gui_obj = reg_all_gui_obj;
    view->renderer = _gui_display_handler;
    mutex_create(&view->lock);

    return 0;
}

static int _gui_display_handler(void)
{
    mutex_lock(&_view_ptr->lock);

    struct kdp_list *head;
    gui_page* page = gui_page_get_present_instance();
    if (0 == page)
        return 0;
    else
        head = &page->ll;//change page content(ll head) here.

    //add obj to canvas from ll.
    //TODO : need to find that why " &p_obj->ll != head; " is not working.
    gui_obj* p_obj;
    for (p_obj = kdp_list_first_entry(head, typeof(*p_obj), ll);
         &p_obj->ll != head->prev;
         p_obj = kdp_list_next_entry(p_obj, ll))
    {
        if (0 == p_obj->on_off) {continue;}
        p_obj->draw(p_obj);
    }

    //lcm fresh
    kl520_api_dp_fresh();

    mutex_unlock(&_view_ptr->lock);
    
    return 0;
}

extern s16 gui_mvc_model_create(gui_mvc_model* model, reg_fsm reg_fsm, fp dereg_fsm)
{
    _model_ptr = model;
    model->reg_fsm = reg_fsm;
    model->dereg_fsm = dereg_fsm;
    model->attach = _attach_observer;
    model->model_notify_view = _model_notify_view;
    model->update_model = _update_model;
    model->observer_num = 0;

    return 0;
}

static void _attach_observer(update_view update_view)
{
    if (MAX_OBS_NUM > _model_ptr->observer_num) {
        _model_ptr->observer[_model_ptr->observer_num] = update_view;
        _model_ptr->observer_num++;
    }
}
static int _model_notify_view(void)
{
    for (u8 i=0; i<_model_ptr->observer_num; i++)
        _model_ptr->observer[i]();
    return 0;
}
static int _update_model(void)
{
    _model_ptr->model_notify_view();
    return 0;
}

#endif
