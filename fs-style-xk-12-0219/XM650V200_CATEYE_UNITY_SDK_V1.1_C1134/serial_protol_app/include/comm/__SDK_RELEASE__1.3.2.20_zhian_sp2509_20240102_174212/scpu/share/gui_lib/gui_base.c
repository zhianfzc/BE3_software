#include <string.h>
#include <stdlib.h>
#include <cmsis_os2.h>
#include "gui_base.h"
#include "kl520_include.h"
#include "kdp_memory.h"
#include "framework/v2k_color.h"
#include "framework/framework_errno.h"
#include "board_ddr_table.h"
#include "gui_fsm.h"
#include "dbg.h"


#if ( CFG_GUI_ENABLE == YES )

#define GUI_APP_DEFAULT_PROCESS_TIME    (10000)

static u8 _page_present = 0;
static u8 _page_previous = 0;
static u8 _first_page_flag = 0;
static struct kdp_list* page_child_ll_start_idx;
static struct kdp_list first_page_parent_ll;
static u16 _x_offset_dp=0, _y_offset_dp=0;
static u16 _x_offset_tp=0, _y_offset_tp=0;

static s16 gui_obj_draw(void* obj_in);


extern void gui_base_set_dp_offset(u16 x, u16 y) {
    _x_offset_dp = x;
    _y_offset_dp = y;
}
extern void gui_base_get_dp_offset(u16* x, u16* y) {
    *x = _x_offset_dp;
    *y = _y_offset_dp;
}
extern void gui_base_set_tp_offset(u16 x, u16 y) {
    _x_offset_tp = x;
    _y_offset_tp = y;
}
extern void gui_base_get_tp_offset(u16* x, u16* y) {
    *x = _x_offset_tp;
    *y = _y_offset_tp;
}

extern gui_page* gui_page_create(u8 page_id)
{
    gui_page* page = (gui_page*)kdp_ddr_reserve( sizeof(gui_page) );

    if (0 == _first_page_flag) {
        _first_page_flag = 1;
        kdp_list_init(&first_page_parent_ll);
    }
    
    struct kdp_list* page_child_ll_head = (struct kdp_list*)kdp_ddr_reserve( sizeof(struct kdp_list) );
    kdp_list_init(page_child_ll_head);
    
    kdp_list_add_tail(&first_page_parent_ll, &page->parent_ll);
    kdp_list_add_tail(page_child_ll_head, &page->ll);

    page->id = page_id;
    page_child_ll_start_idx = page_child_ll_head;

    return page;
}

extern gui_page* gui_page_get_instance_by_id(u8 id)
{
    gui_page* page;
    struct kdp_list *head = &first_page_parent_ll;

    for (page = kdp_list_first_entry(head, typeof(*page), parent_ll);
         &page->parent_ll != head;
         page = kdp_list_next_entry(page, parent_ll))
    {
        if (id == page->id)
            return page;
    }

    return 0;
}
extern gui_page* gui_page_get_present_instance(void)
{
    gui_page* page = gui_page_get_instance_by_id(_page_present);
    return page;
}

extern void gui_page_set_present_index(u8 page) {
    _page_previous = _page_present;
    _page_present = page;
}
extern u8 gui_page_get_present_index(void) {
    return _page_present;
}
extern u8 gui_page_get_previous_index(void) {
    return _page_previous;
}

extern s16 gui_obj_create(gui_obj** pp_obj, gui_obj_type type)
{
    if (!*pp_obj)
        *pp_obj = (gui_obj *)kdp_ddr_reserve( sizeof(gui_obj) );

    gui_obj* obj = *pp_obj;
    if (!obj)
        return (s16)(-KDP_FRAMEWORK_ERRNO_NOMEM);

    kdp_list_add_tail(page_child_ll_start_idx, &obj->ll);

    obj->type = type;
    if (OBJ_NULL == type)
        return -1;

    if (IMAGE_BTN==type || TEXT_BTN==type)
        obj->touch_ctrl = TOUCH_ACTIVE;
    else
        obj->touch_ctrl = TOUCH_INACTIVE;

    //set default value.
    gui_obj_set_fsm_func(obj, -1);
    gui_obj_set_cb(obj, dummy_handler);
    gui_obj_set_draw_func(obj, gui_obj_draw);
    obj->on_off = 1;

    return 0;
}

extern s16 gui_obj_set_size(gui_obj* obj, u16 x, u16 y, u16 w, u16 h)
{
#if (CFG_CAMERA_ROTATE == YES)
    if ((x == 0) && (y == 0) && (w == PANEL_WIDTH) && (h == PANEL_HEIGHT))
    {
        //pass
    }
    else
    {
        COORDINATE_CAL(x, y, w, h, x, y, w, h);
        x = PANEL_WIDTH-x-w;
    }
#endif

    obj->x = x;
    obj->y = y;
    obj->w = w;
    obj->h = h;
    return 0;
}

extern s16 gui_obj_set_cb(gui_obj* obj, obj_cb_fun callback) {
    obj->obj_cb = callback;
    return 0;
}
extern s16 gui_obj_set_color(gui_obj* obj, u16 color) {
    obj->color = color;
    return 0;
}
extern s16 gui_obj_set_image(gui_obj* obj, u32 addr) {
    obj->image_addr = addr;
    return 0;
}
extern s16 gui_obj_set_fsm_func(gui_obj* obj, s16 func) {
    obj->fsm_func = func;
    return 0;
}
extern s16 gui_obj_set_draw_func(gui_obj* obj, obj_cb_fun draw_func) {
    obj->draw = draw_func;
    return 0;
}
extern s16 gui_obj_set_name(gui_obj* obj, u8* name) {
    memcpy(obj->name, name, GUI_OBJ_NAME_LEN);
    return 0;
}
extern s16 gui_obj_set_on_off(gui_obj* obj, u8 on_off) {
    obj->on_off = on_off;
    return 0;
}

static s16 gui_obj_draw(void* obj_in)
{
    gui_obj* obj = (gui_obj*)obj_in;

    if (BACKGROUND == obj->type) {
        kl520_api_dp_set_pen_rgb565(obj->color, 2);
        kl520_api_dp_fill_rect(obj->x+_x_offset_dp, obj->y+_y_offset_dp, obj->w, obj->h);
    }
    else
        kl520_api_dp_draw_bitmap(obj->x+_x_offset_dp, obj->y+_y_offset_dp, obj->w, obj->h, (void *)obj->image_addr);

    return 0;
}

extern s16 dummy_handler(void* obj_in) {
    return 0;
}
extern s16 gui_img_btn_default_handler(void* obj_in) {
    gui_obj* obj = (gui_obj*)obj_in;
    //acknowledge touch event to fsm.
    notify_user_behavior_event((user_behavior_type)obj->fsm_func, GUI_APP_DEFAULT_PROCESS_TIME, 1);
    return 0;
}
extern s16 gui_img_btn_fdr_delete_all_handler(void* obj_in) {
    gui_obj* obj = (gui_obj*)obj_in;
    notify_user_behavior_event((user_behavior_type)obj->fsm_func, 0, 1);
    return 0;
}
extern s16 gui_img_btn_fdr_delete_one_handler(void* obj_in) {
    gui_obj* obj = (gui_obj*)obj_in;
    notify_user_behavior_event((user_behavior_type)obj->fsm_func, 1, 1);
    return 0;
}

extern gui_obj* gui_bg_create_and_set_all_para(u8* name, u16 x, u16 y, u16 w, u16 h, u16 color, u8 on_off)
{
    gui_obj* img_bg = 0;
    gui_obj_create(&img_bg, BACKGROUND);
    gui_obj_set_size(img_bg, x, y, w, h);
    gui_obj_set_color(img_bg, color);
    gui_obj_set_name(img_bg, name);
    gui_obj_set_on_off(img_bg, on_off);

    return img_bg;
}
extern gui_obj* gui_img_create_and_set_all_para(u8* name, u16 x, u16 y, u16 w, u16 h, u32 image_addr, u8 on_off)
{
    gui_obj* img = 0;
    gui_obj_create(&img, IMAGE);
    gui_obj_set_size(img, x, y, w, h);
    gui_obj_set_image(img, image_addr);
    gui_obj_set_name(img, name);
    gui_obj_set_on_off(img, on_off);

    return img;
}

extern gui_obj* gui_img_btn_create_and_set_all_para(u8* name, u16 x, u16 y, u16 w, u16 h, u32 image_addr,
        obj_cb_fun callback, u16 func, u8 on_off)
{
    gui_obj* img_btn = 0;
    gui_obj_create(&img_btn, IMAGE_BTN);
    gui_obj_set_size(img_btn, x, y, w, h);
    gui_obj_set_image(img_btn, image_addr);
    gui_obj_set_cb(img_btn, callback);
    gui_obj_set_fsm_func(img_btn, func);
    gui_obj_set_name(img_btn, name);
    gui_obj_set_on_off(img_btn, on_off);

    return img_btn;
}

extern s16 gui_obj_set_on_off_by_name(u8* name, u8 on_off) {
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
        if (0 == strncmp((const char*)p_obj->name, (const char*)name, GUI_OBJ_NAME_LEN)) {
            p_obj->on_off = on_off;
            return 0;
        }
    }
    
    return -1;
}

#endif
