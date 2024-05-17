#pragma once

#include "types.h"
#include "framework/utils.h" 
#include "framework/kdp_list.h"


#define GUI_OBJ_NAME_LEN        (3)

#define COORDINATE_CAL(x, y, w, h, a, b, c, d) {\
    int tmp = 0;\
    tmp = x;\
    a = y;\
    b = tmp;\
    tmp = w;\
    c = h;\
    d = tmp;\
}

typedef enum _gui_obj_type
{
    OBJ_NULL = -1,
    IMAGE = 0,
    IMAGE_BTN,
    TEXT_BTN,
    TEXT_AREA,
    BACKGROUND,
} gui_obj_type;

typedef enum _gui_obj_touch
{
    TOUCH_INACTIVE = 0,
    TOUCH_ACTIVE,
} gui_obj_touch;

typedef s16 (*obj_cb_fun)(void* obj_in);
typedef struct _gui_obj
{
    struct kdp_list ll;
    u8 name[GUI_OBJ_NAME_LEN];
    gui_obj_type type;
    u16 fsm_func;
    u8 touch_ctrl;
    u16 x;
    u16 y;
    u16 w;
    u16 h;
    u32 image_addr;
    u16 color;
    obj_cb_fun obj_cb;
    obj_cb_fun draw;
    u8 on_off;
} gui_obj;

typedef struct _gui_page
{
    u8 id;
    struct kdp_list parent_ll;
    struct kdp_list ll;
} gui_page;


extern void gui_base_set_dp_offset(u16 x, u16 y);
extern void gui_base_get_dp_offset(u16* x, u16* y);
extern void gui_base_set_tp_offset(u16 x, u16 y);
extern void gui_base_get_tp_offset(u16* x, u16* y);
extern gui_page* gui_page_create(u8 page_id);
extern void gui_page_set_present_index(u8 page);
extern u8 gui_page_get_present_index(void);
extern u8 gui_page_get_previous_index(void);
extern gui_page* gui_page_get_instance_by_id(u8 id);
extern gui_page* gui_page_get_present_instance(void);

extern s16 gui_obj_create(gui_obj** pp_obj, gui_obj_type type);
extern s16 gui_obj_set_size(gui_obj* obj, u16 x, u16 y, u16 w, u16 h);
extern s16 gui_obj_set_cb(gui_obj* obj, obj_cb_fun callback);
extern s16 gui_obj_set_color(gui_obj* obj, u16 color);
extern s16 gui_obj_set_image(gui_obj* obj, u32 addr);
extern s16 gui_obj_set_fsm_func(gui_obj* obj, s16 func);
extern s16 gui_obj_set_draw_func(gui_obj* obj, obj_cb_fun draw_func);
extern s16 gui_obj_set_name(gui_obj* obj, u8* name);
extern s16 gui_obj_set_on_off(gui_obj* obj, u8 on_off);
extern s16 dummy_handler(void* obj_in);
extern s16 gui_img_btn_default_handler(void* obj_in);
extern s16 gui_img_btn_fdr_delete_all_handler(void* obj_in);
extern s16 gui_img_btn_fdr_delete_one_handler(void* obj_in);

extern gui_obj* gui_bg_create_and_set_all_para(u8* name, u16 x, u16 y, u16 w, u16 h, u16 color, u8 on_off);
extern gui_obj* gui_img_btn_create_and_set_all_para(u8* name, u16 x, u16 y, u16 w, u16 h, u32 image_addr,
        obj_cb_fun callback, u16 func, u8 on_off);
extern gui_obj* gui_img_create_and_set_all_para(u8* name, u16 x, u16 y, u16 w, u16 h, u32 image_addr, u8 on_off);
extern s16 gui_obj_set_on_off_by_name(u8* name, u8 on_off);
