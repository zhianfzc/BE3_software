#include <string.h>
#include <stdlib.h>
#include <cmsis_os2.h>
#include "gui_base.h"
#include "kl520_include.h"
#include "kdp_memory.h"
#include "framework/v2k_color.h"
#include "framework/framework_errno.h"
#include "board_ddr_table.h"
#include "gui_text_btn.h"
#include "gui_fsm.h"
#include "dbg.h"
#include "kl520_api_device_id.h"
#include "common.h"


#if ( CFG_GUI_ENABLE == YES )

#define GUI_TEXT_BTN_MAX_NUM    (4)
#define GUI_TEXT_BTN_SPACE      (23)

static u32 num_table[10] = {USR_DDR_IMG_DIGIT_0_ADDR, USR_DDR_IMG_DIGIT_1_ADDR, USR_DDR_IMG_DIGIT_2_ADDR, 
    USR_DDR_IMG_DIGIT_3_ADDR, USR_DDR_IMG_DIGIT_4_ADDR, USR_DDR_IMG_DIGIT_5_ADDR, USR_DDR_IMG_DIGIT_6_ADDR,
    USR_DDR_IMG_DIGIT_7_ADDR, USR_DDR_IMG_DIGIT_8_ADDR, USR_DDR_IMG_DIGIT_9_ADDR};


extern s16 gui_text_btn_create(gui_text_btn** pp_text, gui_obj_type type)
{
    if (!*pp_text)
        *pp_text = (gui_text_btn *)kdp_ddr_reserve( sizeof(gui_text_btn) );

    gui_text_btn* text = *pp_text;
    if (!text)
        return (s16)(-KDP_FRAMEWORK_ERRNO_NOMEM);

    gui_obj* p_obj = &text->gui_base;
    gui_obj_create(&p_obj, type);
    gui_obj_set_draw_func(p_obj, gui_text_btn_draw);

    return 0;
}

extern s16 gui_text_btn_draw(void* obj_in)
{
    gui_obj* obj = (gui_obj*)obj_in;
    gui_text_btn* text = container_of(obj, gui_text_btn, gui_base);
    u16 btn_name = text->btn_name;
    u16 space = GUI_TEXT_BTN_SPACE;
    u8 space_num = 0;

    while (0 != btn_name) {
        u8 tmp = btn_name%10;
        btn_name /= 10;
        kl520_api_dp_draw_bitmap((obj->x)-(space_num*space), obj->y, 
            obj->w, obj->h, (void *)num_table[tmp]);
        space_num++;
    }

    return 0;
}

extern s16 gui_text_btn_set_text(gui_text_btn* text_btn, u16 btn_name) {
    text_btn->btn_name = btn_name;
    return 0;
}
extern s16 gui_text_btn_set_text_by_name(u8* name, u16 btn_name) {
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
            gui_text_btn* p_text_btn = container_of(p_obj, gui_text_btn, gui_base);
            p_text_btn->btn_name = btn_name;
            return 0;
        }
    }
    
    return -1;
}
extern s16 gui_text_btn_set_data_by_name(u8* name, u16 data) {
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
            gui_text_btn* p_text_btn = container_of(p_obj, gui_text_btn, gui_base);
            p_text_btn->data = data;
            return 0;
        }
    }
    return -1;
}

extern s16 gui_text_btn_fdr_delete_id_handler(void* obj_in) {
    gui_obj* obj = (gui_obj*)obj_in;
    gui_text_btn* p_text_btn = container_of(obj, gui_text_btn, gui_base);

    u16 id = p_text_btn->data;
    notify_user_behavior_event((user_behavior_type)obj->fsm_func, id, 1);
    return 0;
}

extern gui_text_btn* gui_text_btn_create_and_set_all_para(u8* name, u16 x, u16 y, u16 w, u16 h,
        u16 btn_name, obj_cb_fun callback, s16 func, u8 on_off)
{
    gui_text_btn* text_btn = 0;
    gui_text_btn_create(&text_btn, TEXT_BTN);
    gui_text_btn_set_text(text_btn, btn_name);
    gui_obj_set_name(&text_btn->gui_base, name);
    gui_obj_set_size(&text_btn->gui_base, x, y, w, h);
    gui_obj_set_color(&text_btn->gui_base, BLACK);
    gui_obj_set_cb(&text_btn->gui_base, callback);
    gui_obj_set_fsm_func(&text_btn->gui_base, func);
    gui_obj_set_on_off(&text_btn->gui_base, on_off);

    return text_btn;
}

extern s16 gui_text_only_draw_uuid(void* obj_in) {
    gui_obj* obj = (gui_obj*)obj_in;
    gui_text_btn* text = container_of(obj, gui_text_btn, gui_base);
    u16 space = GUI_TEXT_BTN_SPACE;

    int btn_name = kl520_api_get_unique_id();
    char str[9] = {0};
    sprintf(str, "%x", btn_name);
    
    for (int i=0; i<8; i++) {
        if ('0'>str[i] || '9'<str[i]) return -1;
#if (CFG_CAMERA_ROTATE == YES)
        kl520_api_dp_draw_bitmap((obj->x), obj->y+(i*space),
            obj->w, obj->h, (void *)num_table[str[i]-'0']);
#else
        kl520_api_dp_draw_bitmap((obj->x)+(i*space), obj->y,
            obj->w, obj->h, (void *)num_table[str[i]-'0']);
#endif
    }
    return 0;
}
extern s16 gui_text_only_draw_fw_version(void* obj_in) {
    gui_obj* obj = (gui_obj*)obj_in;
    gui_text_btn* text = container_of(obj, gui_text_btn, gui_base);
    u16 space = GUI_TEXT_BTN_SPACE;

    struct fw_misc_data scpu_fw_info;
    kl520_api_get_scpu_version(&scpu_fw_info);
    
    int space_num = 0;
    for (int i=0; i<4; i++) {
        unsigned char btn_name = scpu_fw_info.version[i];
        char str[4] = {0};
        sprintf(str, "%d", btn_name);
        
        int j=0;
        while ('\0' != str[j]) {
            if ('0'>str[j] || '9'<str[j]) return -1;
#if (CFG_CAMERA_ROTATE == YES)
            kl520_api_dp_draw_bitmap((obj->x), obj->y+(space_num*(space/2)),
                obj->w, obj->h, (void *)num_table[str[j]-'0']);
#else
            kl520_api_dp_draw_bitmap((obj->x)+(space_num*(space/2)), obj->y,
                obj->w, obj->h, (void *)num_table[str[j]-'0']);
#endif
            space_num+=2;
            j++;
        }
        space_num++;//space for child version
    }
    return 0;
}
extern s16 gui_text_only_draw_reg_num(void* obj_in) {
    gui_obj* obj = (gui_obj*)obj_in;
    gui_text_btn* text = container_of(obj, gui_text_btn, gui_base);
    u16 space = GUI_TEXT_BTN_SPACE;
    
    u8 total_id_num;
    u8 face_status[MAX_USER];
    kl520_api_face_query_all(&total_id_num, &face_status[0]);

    int btn_name = total_id_num;
    char str[9] = {0};
    sprintf(str, "%x", btn_name);
    
    int j=0, space_num=0;
    while ('\0' != str[j]) {
        if ('0'>str[j] || '9'<str[j]) return -1;
#if (CFG_CAMERA_ROTATE == YES)
        kl520_api_dp_draw_bitmap((obj->x), obj->y+(space_num*space),
            obj->w, obj->h, (void *)num_table[str[j]-'0']);
#else
        kl520_api_dp_draw_bitmap((obj->x)+(space_num*space), obj->y,
            obj->w, obj->h, (void *)num_table[str[j]-'0']);
#endif
        space_num++;
        j++;
    }
    return 0;
}
extern gui_text_btn* gui_text_only_create_and_set_all_para(u8* name, u16 x, u16 y, u16 w, u16 h,
        obj_cb_fun draw_func, u8 on_off)
{
    gui_text_btn* text_btn = 0;
    gui_text_btn_create(&text_btn, TEXT_BTN);
    gui_obj_set_name(&text_btn->gui_base, name);
    gui_obj_set_size(&text_btn->gui_base, x, y, w, h);
    gui_obj_set_color(&text_btn->gui_base, BLACK);
    gui_obj_set_draw_func(&text_btn->gui_base, draw_func);
    gui_obj_set_on_off(&text_btn->gui_base, on_off);

    return text_btn;
}

#endif
