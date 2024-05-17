#include <string.h>
#include <stdlib.h>
#include <cmsis_os2.h>
#include "gui_base.h"
#include "kl520_include.h"
#include "kdp_memory.h"
#include "framework/v2k_color.h"
#include "framework/framework_errno.h"
#include "board_ddr_table.h"
#include "gui_keypad.h"
#include "gui_text.h"
#include "gui_fsm.h"
#include "dbg.h"


#if ( CFG_GUI_ENABLE == YES )

extern s16 gui_key_create(gui_key** pp_key, gui_obj_type type)
{
    if (!*pp_key)
        *pp_key = (gui_key *)kdp_ddr_reserve( sizeof(gui_key) );

    gui_key* key = *pp_key;
    if (!key)
        return (s16)(-KDP_FRAMEWORK_ERRNO_NOMEM);

    gui_obj* p_obj = &key->gui_base;
    gui_obj_create(&p_obj, type);

    return 0;
}
extern s16 gui_key_set_value(gui_key* pp_key, s8 key_value)
{
    pp_key->key_value = key_value;
    return 0;
}

extern s16 gui_keypad_handler(void* obj_in) {
    gui_obj* obj = (gui_obj*)obj_in;
    gui_key* key = container_of(obj, gui_key, gui_base);
    s8 key_value = key->key_value;

    //operate text area.
    if ('x' == key_value)
        gui_text_delete();
    else if ('0'<=key_value && '9'>=key_value)
        gui_text_add(key_value);

    //acknowledge touch event to fsm.
    notify_user_behavior_event((user_behavior_type)obj->fsm_func, key_value, 0);

    return 0;
}

extern gui_key* gui_key_create_and_set_all_para(u8* name, u16 x, u16 y, u16 w, u16 h, u32 image_addr,
        obj_cb_fun callback, u16 func, s8 key_value, u8 on_off)
{
    gui_key* key = 0;
    gui_key_create(&key, IMAGE_BTN);
    gui_obj_set_size(&key->gui_base, x, y, w, h);
    gui_obj_set_image(&key->gui_base, image_addr);
    gui_obj_set_cb(&key->gui_base, callback);
    gui_obj_set_fsm_func(&key->gui_base, func);
    gui_key_set_value(key, key_value);
    gui_obj_set_name(&key->gui_base, name);
    gui_obj_set_on_off(&key->gui_base, on_off);

    return key;
}

#endif
