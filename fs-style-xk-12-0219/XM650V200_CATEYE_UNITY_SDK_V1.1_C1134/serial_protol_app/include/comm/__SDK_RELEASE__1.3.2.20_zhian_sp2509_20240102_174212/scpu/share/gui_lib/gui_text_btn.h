#pragma once

#include "types.h"
#include "gui_base.h"


typedef struct _gui_text_btn
{
    gui_obj gui_base;
    u16 btn_name;
    u16 data;
} gui_text_btn;


extern s16 gui_text_btn_create(gui_text_btn** pp_text, gui_obj_type type);
extern s16 gui_text_btn_draw(void* obj_in);
extern gui_text_btn* gui_text_btn_create_and_set_all_para(u8* name, u16 x, u16 y, u16 w, u16 h,
        u16 btn_name, obj_cb_fun callback, s16 func, u8 on_off);
extern gui_text_btn* gui_text_only_create_and_set_all_para(u8* name, u16 x, u16 y, u16 w, u16 h,
        obj_cb_fun draw_func, u8 on_off);
extern s16 gui_text_only_draw_uuid(void* obj_in);
extern s16 gui_text_only_draw_fw_version(void* obj_in);
extern s16 gui_text_only_draw_reg_num(void* obj_in);
extern s16 gui_text_btn_set_text_by_name(u8* name, u16 btn_name);
extern s16 gui_text_btn_set_data_by_name(u8* name, u16 data);
extern s16 gui_text_btn_fdr_delete_id_handler(void* obj_in);
