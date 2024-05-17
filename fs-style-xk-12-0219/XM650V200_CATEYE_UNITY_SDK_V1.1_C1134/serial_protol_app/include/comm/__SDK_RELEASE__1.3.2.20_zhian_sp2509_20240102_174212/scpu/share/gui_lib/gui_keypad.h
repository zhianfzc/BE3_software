#pragma once

#include "types.h"
#include "gui_base.h"


typedef struct _gui_key
{
    gui_obj gui_base;
    s8 key_value;
} gui_key;


extern s16 gui_keypad_handler(void* obj_in);
extern gui_key* gui_key_create_and_set_all_para(u8* name, u16 x, u16 y, u16 w, u16 h, u32 image_addr,
        obj_cb_fun callback, u16 func, s8 key_value, u8 on_off);
