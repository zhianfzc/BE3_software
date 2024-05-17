#include <string.h>
#include <stdlib.h>
#include <cmsis_os2.h>
#include "gui_base.h"
#include "kl520_include.h"
#include "kdp_memory.h"
#include "framework/v2k_color.h"
#include "framework/framework_errno.h"
#include "board_ddr_table.h"
#include "gui_text.h"
#include "gui_fsm.h"
#include "dbg.h"
#include "kl520_api_device_id.h"


#if ( CFG_GUI_ENABLE == YES )

#define GUI_TEXT_MAX_NUM    (6)

static s8 _present_text_num = 0;
static s8 _present_text_str[GUI_TEXT_MAX_NUM] = {};


extern s16 gui_text_create(gui_text_area** pp_text, gui_obj_type type)
{
    if (!*pp_text)
        *pp_text = (gui_text_area *)kdp_ddr_reserve( sizeof(gui_text_area) );

    gui_text_area* text = *pp_text;
    if (!text)
        return (s16)(-KDP_FRAMEWORK_ERRNO_NOMEM);

    gui_obj* p_obj = &text->gui_base;
    gui_obj_create(&p_obj, type);

    return 0;
}

extern s16 gui_text_set_text_spacing(gui_text_area* text, u16 space) {
    text->text_spacing = space;
    return 0;
}

extern s16 gui_text_draw(void* obj_in)
{
    gui_obj* obj = (gui_obj*)obj_in;
    gui_text_area* text = container_of(obj, gui_text_area, gui_base);
    u16 space = text->text_spacing;
    u8 i = 0;
    u16 x_offset = 0, y_offset = 0;
    gui_base_get_dp_offset(&x_offset, &y_offset);

    for (i=0; i<_present_text_num; i++) {
#if (CFG_CAMERA_ROTATE == YES)
        kl520_api_dp_draw_bitmap((obj->x)+x_offset, obj->y+(i*space)+y_offset, obj->w, obj->h, (void *)obj->image_addr);
#else
        kl520_api_dp_draw_bitmap((obj->x)+(i*space)+x_offset, obj->y+y_offset, obj->w, obj->h, (void *)obj->image_addr);
#endif
    }
    kl520_api_dp_set_pen_rgb565(obj->color, 2);
#if (CFG_CAMERA_ROTATE == YES)
    kl520_api_dp_fill_rect((obj->x)+x_offset, obj->y+((i+1)*space)+y_offset, obj->w, obj->h);
#else
    kl520_api_dp_fill_rect((obj->x)+((i+1)*space)+x_offset, obj->y+y_offset, obj->w, obj->h);
#endif

    return 0;
}
/*extern s16 gui_text_area0_handler(void* obj_in) {
    gui_obj* obj = (gui_obj*)obj_in;
    gui_text_area* text = container_of(obj, gui_text_area, gui_base);
    u16 space = text->text_spacing;
    u8 i = 0;
    u16 x_offset = 0, y_offset = 0;
    gui_base_get_dp_offset(&x_offset, &y_offset);

    int ret = 0;
    system_info t_sys_info = { 0 };
    ret = kl520_api_get_device_info(&t_sys_info);
    t_sys_info.unique_id;
    t_sys_info.fw_scpu_version.date;
    
    if ('0'<=key_value && '9'>=key_value)
    gui_text_add();
    
    for (i=0; i<_present_text_num; i++) {
        kl520_api_dp_draw_bitmap((obj->x)+(i*space)+x_offset, obj->y+y_offset, obj->w, obj->h, (void *)obj->image_addr);
    }
    kl520_api_dp_set_pen_rgb565(obj->color, 2);
    kl520_api_dp_fill_rect((obj->x)+((i+1)*space)+x_offset, obj->y+y_offset, obj->w, obj->h);

    return 0;
}*/

extern s16 gui_text_add(s8 key_value)
{
    if (GUI_TEXT_MAX_NUM > _present_text_num) {
        _present_text_num++;
        s16 idx = _present_text_num-1;
        if (0 <= idx)
            _present_text_str[idx] = key_value;
    }

    return 0;
}

extern s16 gui_text_delete(void) {
    s16 idx = _present_text_num-1;
    if (0 < _present_text_num) {
        _present_text_num--;
        _present_text_str[idx] = 0;
        return 1;
    }
    return 0;
}
extern s16 gui_text_delete_all(void) {
    while ( gui_text_delete() ) {}
    return 0;
}
extern u8 gui_text_get_max_len(void) {
    return GUI_TEXT_MAX_NUM;
}
extern u8 gui_text_get_str(s8* str) {
    strcpy(str, _present_text_str);
    return _present_text_num;
}

extern gui_text_area* gui_text_area_create_and_set_all_para(u8* name, u16 x, u16 y, u16 w, u16 h, u32 image_addr,
        obj_cb_fun draw_func, u16 color, u16 text_space, u8 on_off)
{
    gui_text_area* text_area = 0;
    gui_text_create(&text_area, TEXT_AREA);
    gui_text_set_text_spacing(text_area, text_space);
    gui_obj_set_size(&text_area->gui_base, x, y, w, h);
    gui_obj_set_image(&text_area->gui_base, image_addr);
    gui_obj_set_color(&text_area->gui_base, color);
    gui_obj_set_draw_func(&text_area->gui_base, draw_func);
    gui_obj_set_name(&text_area->gui_base, name);
    gui_obj_set_on_off(&text_area->gui_base, on_off);

    return text_area;
}

#endif
