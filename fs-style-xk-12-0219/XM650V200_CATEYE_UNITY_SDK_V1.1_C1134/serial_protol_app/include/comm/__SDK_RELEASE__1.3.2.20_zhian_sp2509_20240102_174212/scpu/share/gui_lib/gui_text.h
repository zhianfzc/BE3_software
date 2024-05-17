#pragma once

#include "types.h"
#include "gui_base.h"


typedef struct _gui_text_area
{
    gui_obj gui_base;
    u16 text_spacing;
} gui_text_area;


extern s16 gui_text_create(gui_text_area** pp_text, gui_obj_type type);
extern s16 gui_text_set_text_spacing(gui_text_area* text, u16 space);
extern s16 gui_text_draw(void* obj_in);
extern s16 gui_text_add(s8 key_value);
extern s16 gui_text_delete(void);
extern s16 gui_text_delete_all(void);
extern u8 gui_text_get_max_len(void);
extern u8 gui_text_get_str(s8* str);
extern gui_text_area* gui_text_area_create_and_set_all_para(u8* name, u16 x, u16 y, u16 w, u16 h, u32 image_addr,
        obj_cb_fun draw_func, u16 color, u16 text_space, u8 on_off);
