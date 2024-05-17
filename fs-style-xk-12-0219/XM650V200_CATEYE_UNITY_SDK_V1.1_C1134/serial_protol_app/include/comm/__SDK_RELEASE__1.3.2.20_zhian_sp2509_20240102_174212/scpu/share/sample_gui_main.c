#include "sample_gui_main.h"

#ifndef GUI_VERSION_TYPE
#define GUI_V1                          (0)
#define GUI_VERSION_TYPE                (GUI_V1)
#endif

#if ( CFG_GUI_ENABLE == YES )

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF )
#include "sample_user_com_and_gui_fdr.h"
#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP )
#include "kdp_comm_and_gui_fdr.h"
#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF_USR )
#include "user_com.h"
#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP_USR )
#include "user_comm_and_gui_fdr.h"
#endif
#endif

#include <stdlib.h>
#include "kl520_include.h"
#include "framework/v2k_color.h"
#include "board_ddr_table.h"
#include "gui_base.h"
#include "gui_text.h"
#include "gui_text_btn.h"
#include "gui_keypad.h"
#include "gui_fsm.h"
#include "gui_mvc.h"
#include "sample_gui_fsm_events.h"
#include "user_ui.h"

osThreadId_t tid_gui = 0;
static u16 _display_width=0, _display_height=0;
static gui_mvc_ctrlr ctrlr_touch;
static gui_mvc_ctrlr ctrlr_com;
static gui_mvc_view view;
static gui_mvc_model model;

static void _sample_gui_thread(void *arg);
static int _gui_register_input_touch(void);
static int _gui_deregister_input_touch(void);
static int _gui_register_display(void);
static int _gui_deregister_display(void);
static int _gui_register_object(void);
static int _gui_register_input_comm(void);
static int _gui_deregister_input_comm(void);

extern int app_kl520_init(void);

extern void sample_gui_init(void)
{
    osThreadAttr_t attr = {
        .stack_size = 512,
        .attr_bits = osThreadJoinable
    }; 

    g_bBootupGuiHighPriority = TRUE;

    if (NULL == tid_gui) {
        tid_gui = osThreadNew(_sample_gui_thread, NULL, &attr);
    }
}
extern void sample_gui_deinit(void)
{
    if (tid_gui) {
        set_event(kl520_api_get_event(), KL520_APP_FLAG_TOUCH_DEINIT);
        osThreadJoin(tid_gui);
        tid_gui = 0;

        ctrlr_touch.dereg_input();
        view.dereg_display();
        model.dereg_fsm();
    }
}

static int _gui_touch_get_info_adapter(void* arg){
    return kl520_api_mouse_info_get((kl520_mouse_info*)arg);
}
static void _sample_gui_thread(void *arg)
{
    gui_mvc_ctrlr_create(&ctrlr_touch, _gui_register_input_touch, _gui_deregister_input_touch, 
    gui_touchpanel_handler, _gui_touch_get_info_adapter);
    gui_mvc_ctrlr_create(&ctrlr_com, _gui_register_input_comm, _gui_deregister_input_comm, 
    gui_comm_handler, user_com_get_data);
    gui_mvc_view_create(&view, _gui_register_display, _gui_deregister_display, _gui_register_object);
    gui_mvc_model_create(&model, ui_fsm_init, ui_fsm_deinit);

    if (0 != ctrlr_touch.reg_input())
    {
        tid_gui = 0;
        osThreadExit();
        return;
    }
    ctrlr_com.reg_input();
    view.reg_display();
    view.reg_all_gui_obj();//link input info
    model.reg_fsm(&model);//register gui fsm and starting fsm thread.
    model.attach(view.renderer);

    while(1)
    {
        #if BOARD_VERSION==3
        osDelay(30);
        #else
        osDelay(5);
        #endif

        u32 flags = osEventFlagsGet(kl520_api_get_event());
        osEventFlagsClear(kl520_api_get_event(), KL520_APP_FLAG_TOUCH_ALL|KL520_APP_FLAG_COMM);

        if (flags & KL520_APP_FLAG_TOUCH_DEINIT)
            break;
        if (flags & KL520_APP_GUI_STOP)
            continue;

        //input sensing task
        if (flags & KL520_APP_FLAG_TOUCH)
            ctrlr_touch.input_handler((void*)&ctrlr_touch);
        else if (flags & KL520_APP_FLAG_COMM)
            ctrlr_com.input_handler((void*)&ctrlr_com);
    }
    osThreadExit();
}

static int _gui_register_display(void)
{
    //just set w/h here, and using 'open renderer' in fsm.
    //TODO : link w/h from system.
    _display_width = DISPLAY_WIDTH;
    _display_height = DISPLAY_HEIGHT;

    u16 x = 0;  //display origin x
    u16 y = 0;  //display origin y  

    gui_base_set_dp_offset(x, y);
    
    return 0;
}
static int _gui_deregister_display(void) { return 0; }

static int _gui_register_input_touch(void)
{
#if CFG_TOUCH_ENABLE == YES
    if (0 != app_kl520_init())
    {
        return -1;
    }
#endif
    u16 x = 0;
    u16 y = 0;
    gui_base_set_tp_offset(x, y);
    return 0;
}
static int _gui_deregister_input_touch(void) {
#if CFG_TOUCH_ENABLE == YES
    kl520_api_touch_stop();
#endif

    return 0;
}

static int _gui_register_input_comm(void){ return 0; }
static int _gui_deregister_input_comm(void){ return 0; }

static void _gui_keypad_create(void)
{
    obj_cb_fun cb = gui_keypad_handler;
    u16 func = (u16)GUI_KEYPAD_BTN;

    gui_key_create_and_set_all_para("k_1", 0, 56, 66, 66, USR_DDR_IMG_DIGIT_1_ADDR, cb, func, '1', 1);
    gui_key_create_and_set_all_para("k_2", 66, 56, 66, 66, USR_DDR_IMG_DIGIT_2_ADDR, cb, func, '2', 1);
    gui_key_create_and_set_all_para("k_3", 132, 56, 66, 66, USR_DDR_IMG_DIGIT_3_ADDR, cb, func, '3', 1);
    gui_key_create_and_set_all_para("k_4", 0, 122, 66, 66, USR_DDR_IMG_DIGIT_4_ADDR, cb, func, '4', 1);
    gui_key_create_and_set_all_para("k_5", 66, 122, 66, 66, USR_DDR_IMG_DIGIT_5_ADDR, cb, func, '5', 1);
    gui_key_create_and_set_all_para("k_6", 132, 122, 66, 66, USR_DDR_IMG_DIGIT_6_ADDR, cb, func, '6', 1);
    gui_key_create_and_set_all_para("k_7", 0, 188, 66, 66, USR_DDR_IMG_DIGIT_7_ADDR, cb, func, '7', 1);
    gui_key_create_and_set_all_para("k_8", 66, 188, 66, 66, USR_DDR_IMG_DIGIT_8_ADDR, cb, func, '8', 1);
    gui_key_create_and_set_all_para("k_9", 132, 188, 66, 66, USR_DDR_IMG_DIGIT_9_ADDR, cb, func, '9', 1);
    gui_key_create_and_set_all_para("k_0", 0, 254, 66, 66, USR_DDR_IMG_DIGIT_0_ADDR, cb, func, '0', 1);
    gui_key_create_and_set_all_para("k_x", 66, 254, 66, 66, USR_DDR_IMG_ICON_FAIL_ADDR, cb, func, 'x', 1);
    gui_key_create_and_set_all_para("k_o", 132, 254, 66, 66, USR_DDR_IMG_ICON_SUCCESS_ADDR, cb, func, 'o', 1);
}

static int _gui_register_object(void)
{
    obj_cb_fun cb = gui_img_btn_default_handler;

    //set init page
    gui_page_set_present_index((u8)PAGE_IDX_0);

    //page 0 :
    gui_page_create((u8)PAGE_IDX_0);
    //background
    gui_bg_create_and_set_all_para("bg0", 0, 0, _display_width, _display_height, BLACK, 1);
    //buttons
#if ( GUI_VERSION_TYPE == GUI_V1)
    gui_img_btn_create_and_set_all_para("key", 0, 0, 120, 48, USR_DDR_IMG_BTN_PW_E_ADDR, cb, (u16)GUI_IMG_BTN_GO_TO_KEY, 1);
    gui_img_btn_create_and_set_all_para("rcg", 0, 320-48, 120, 48, USR_DDR_IMG_BTN_FACE_E_ADDR, cb, (u16)GUI_IMG_BTN_RECOGNIZE, 1);
    gui_img_btn_create_and_set_all_para("reg", 240-120, 320-48, 120, 48, USR_DDR_IMG_BTN_GOON_E_ADDR, cb, (u16)GUI_IMG_BTN_REGISTER, 1);
    gui_img_btn_create_and_set_all_para("de1", (240-66)/4, (320-66)/2, 66, 66, USR_DDR_IMG_ICON_DEL_ID_ADDR, gui_img_btn_fdr_delete_one_handler, (u16)GUI_IMG_BTN_DELETE, 1);
    gui_img_btn_create_and_set_all_para("deA", (240-66)/4*3, (320-66)/2, 66, 66, USR_DDR_IMG_ICON_FAIL_ADDR, gui_img_btn_fdr_delete_all_handler, (u16)GUI_IMG_BTN_DELETE, 1);

#elif ( GUI_VERSION_TYPE == GUI_V2)
    gui_img_btn_create_and_set_all_para("reg", 20, 10, 90, 110, USR_DDR_IMG_FACE_DATA_ADDR, cb, (u16)GUI_IMG_BTN_REGISTER_PAGE, 1);
    gui_img_btn_create_and_set_all_para("rcg", 20+90+20, 10, 90, 110, USR_DDR_IMG_FACE_ADDR, cb, (u16)GUI_IMG_BTN_RECOGNIZE, 1);
    gui_img_btn_create_and_set_all_para("pwd", 20, 10+110+10, 90, 110, USR_DDR_IMG_PASSWORD_ADDR, cb, (u16)GUI_IMG_BTN_GO_TO_KEY, 1);
    gui_img_btn_create_and_set_all_para("set", 20+90+20, 10+110+10, 90, 110, USR_DDR_IMG_SETTING_ADDR, cb, (u16)GUI_IMG_BTN_RETURN, 1);
    gui_img_btn_create_and_set_all_para("pow", (240-70)/2, (320-70-10), 70, 70, USR_DDR_IMG_POWER_ADDR, cb, (u16)GUI_POWER_OFF, 1);
#endif
    //pictures
    gui_img_create_and_set_all_para("rcS", 0, 0, _display_width, _display_height, USR_DDR_IMG_RECOGNIZE_SUCCESSED_ADDR, 0);
    gui_img_create_and_set_all_para("rcF", 0, 0, _display_width, _display_height, USR_DDR_IMG_RECOGNIZE_FAILED_ADDR, 0);
    gui_img_create_and_set_all_para("reS", 0, 0, _display_width, _display_height, USR_DDR_IMG_REGISTER_SUCCESSED_ADDR, 0);
    gui_img_create_and_set_all_para("reF", 0, 0, _display_width, _display_height, USR_DDR_IMG_REGISTER_FAILED_ADDR, 0);
    gui_img_create_and_set_all_para("deS", 0, 0, _display_width, _display_height, USR_DDR_IMG_DELETE_SUCCESSED_ADDR, 0);
    gui_img_create_and_set_all_para("deF", 0, 0, _display_width, _display_height, USR_DDR_IMG_DELETE_FAILED_ADDR, 0);

    //page 1 :
    gui_page_create((u8)PAGE_IDX_1);
    //background
    gui_bg_create_and_set_all_para("bg0", 0, 0, _display_width, _display_height, BLACK, 1);
    //buttons
#if (CFG_CAMERA_ROTATE == YES)
    gui_img_btn_create_and_set_all_para("rtn", 240-42, 0, 33, 17, USR_DDR_IMG_ICON_ARROW_DOWN_ADDR, cb, GUI_IMG_BTN_RETURN, 1);
#else
    gui_img_btn_create_and_set_all_para("rtn", 240-42, 0, 33, 17, USR_DDR_IMG_ICON_ARROW_RIGHT_ADDR, cb, GUI_IMG_BTN_RETURN, 1);
#endif
    //keypad
    _gui_keypad_create();
    //text area
    gui_text_area_create_and_set_all_para("txt", 0, 0, 66, 66, USR_DDR_IMG_DIGIT_STAR_ADDR, gui_text_draw, BLACK, 22, 1);
    //pictures
    gui_img_create_and_set_all_para("rcS", 0, 0, _display_width, _display_height, USR_DDR_IMG_RECOGNIZE_SUCCESSED_ADDR, 0);
    gui_img_create_and_set_all_para("rcF", 0, 0, _display_width, _display_height, USR_DDR_IMG_RECOGNIZE_FAILED_ADDR, 0);
#if ( GUI_VERSION_TYPE == GUI_V1)
#elif ( GUI_VERSION_TYPE == GUI_V2)
    gui_img_create_and_set_all_para("reS", 0, 0, _display_width, _display_height, USR_DDR_IMG_REGISTER_SUCCESSED_ADDR, 0);
    gui_img_create_and_set_all_para("reF", 0, 0, _display_width, _display_height, USR_DDR_IMG_REGISTER_FAILED_ADDR, 0);
#endif

    //page 2
    gui_page_create((u8)PAGE_IDX_2);
#if (CFG_CAMERA_ROTATE == YES)
    gui_img_btn_create_and_set_all_para("rtn", 240-42, 0, 33, 17, USR_DDR_IMG_ICON_ARROW_DOWN_ADDR, cb, GUI_IMG_BTN_RETURN, 1);
#else
    gui_img_btn_create_and_set_all_para("rtn", 240-42, 0, 33, 17, USR_DDR_IMG_ICON_ARROW_RIGHT_ADDR, cb, GUI_IMG_BTN_RETURN, 1);
#endif
    gui_img_btn_create_and_set_all_para("key", 0, 0, 66, 66, USR_DDR_IMG_DIGIT_0_ADDR, cb, GUI_IMG_BTN_GO_TO_KEY, 1);

    //page 3 - deletion
    gui_page_create((u8)PAGE_IDX_3);
    gui_bg_create_and_set_all_para("bg0", 0, 0, _display_width, _display_height, BLACK, 1);

#if ( GUI_VERSION_TYPE == GUI_V1)
#if (CFG_CAMERA_ROTATE == YES)
    gui_img_btn_create_and_set_all_para("rtn", 240-42, 0, 33, 17, USR_DDR_IMG_ICON_ARROW_DOWN_ADDR, cb, GUI_IMG_BTN_RETURN, 1);
    gui_img_btn_create_and_set_all_para("nxt", 240-42, 320-17, 33, 17, USR_DDR_IMG_ICON_ARROW_DOWN_ADDR, cb, GUI_IMG_BTN_GO_TO_KEY, 1);
    gui_img_btn_create_and_set_all_para("pre", 0, 320-17, 33, 17, USR_DDR_IMG_ICON_ARROW_UP_ADDR, cb, GUI_KEYPAD_BTN, 1);
#else
    gui_img_btn_create_and_set_all_para("rtn", 240-42, 0, 33, 17, USR_DDR_IMG_ICON_ARROW_RIGHT_ADDR, cb, GUI_IMG_BTN_RETURN, 1);
    gui_img_btn_create_and_set_all_para("nxt", 240-42, 320-17, 33, 17, USR_DDR_IMG_ICON_ARROW_RIGHT_ADDR, cb, GUI_IMG_BTN_GO_TO_KEY, 1);
    gui_img_btn_create_and_set_all_para("pre", 0, 320-17, 33, 17, USR_DDR_IMG_ICON_ARROW_LEFT_ADDR, cb, GUI_KEYPAD_BTN, 1);
#endif

#elif ( GUI_VERSION_TYPE == GUI_V2)
    gui_img_btn_create_and_set_all_para("mre", 0, 320-30, 40, 30, USR_DDR_IMG_RETURN_ADDR, cb, (u16)GUI_IMG_BTN_RETURN, 1);
    gui_img_btn_create_and_set_all_para("mho", 240-40, 320-30, 40, 30, USR_DDR_IMG_HOME_ADDR, cb, (u16)GUI_IMG_BTN_HOME, 1);

#if (CFG_CAMERA_ROTATE == YES)
    gui_img_btn_create_and_set_all_para("nxt", 120+10, 320-17, 33, 17, USR_DDR_IMG_ICON_ARROW_DOWN_ADDR, cb, GUI_IMG_BTN_GO_TO_KEY, 1);
    gui_img_btn_create_and_set_all_para("pre", 120-42-10, 320-17, 33, 17, USR_DDR_IMG_ICON_ARROW_UP_ADDR, cb, GUI_KEYPAD_BTN, 1);
#else
    gui_img_btn_create_and_set_all_para("nxt", 120+10, 320-17, 33, 17, USR_DDR_IMG_ICON_ARROW_RIGHT_ADDR, cb, GUI_IMG_BTN_GO_TO_KEY, 1);
    gui_img_btn_create_and_set_all_para("pre", 120-42-10, 320-17, 33, 17, USR_DDR_IMG_ICON_ARROW_LEFT_ADDR, cb, GUI_KEYPAD_BTN, 1);
#endif
#endif

    //text btn
    gui_text_btn_create_and_set_all_para("d_1", 240/2-66/2, 0, 66, 66, 1, gui_text_btn_fdr_delete_id_handler, GUI_IMG_BTN_DELETE_1, 1);
    gui_text_btn_create_and_set_all_para("d_2", 240/2-66/2, 50, 66, 66, 2, gui_text_btn_fdr_delete_id_handler, GUI_IMG_BTN_DELETE_1, 1);
    gui_text_btn_create_and_set_all_para("d_3", 240/2-66/2, 100, 66, 66, 3, gui_text_btn_fdr_delete_id_handler, GUI_IMG_BTN_DELETE_1, 1);
    gui_text_btn_create_and_set_all_para("d_4", 240/2-66/2, 150, 66, 66, 4, gui_text_btn_fdr_delete_id_handler, GUI_IMG_BTN_DELETE_1, 1);
    gui_text_btn_create_and_set_all_para("d_5", 240/2-66/2, 200, 66, 66, 5, gui_text_btn_fdr_delete_id_handler, GUI_IMG_BTN_DELETE_1, 1);

#if ( GUI_VERSION_TYPE == GUI_V1)
#elif ( GUI_VERSION_TYPE == GUI_V2)    //page 4 - management
    gui_page_create((u8)PAGE_IDX_4);
    gui_bg_create_and_set_all_para("bg0", 0, 0, _display_width, _display_height, BLACK, 1);
    gui_img_btn_create_and_set_all_para("reg", 20, 10, 200, 50, USR_DDR_IMG_F_REG_NEW_ADDR, cb, (u16)GUI_IMG_BTN_REGISTER, 1);
    gui_img_btn_create_and_set_all_para("did", 20, 10+50+10, 200, 50, USR_DDR_IMG_F_DEL_ONE_ID_ADDR, gui_img_btn_fdr_delete_one_handler, (u16)GUI_IMG_BTN_DELETE, 1);
    //gui_img_btn_create_and_set_all_para("dfc", 20, 10+50+10+50+10, 200, 50, USR_DDR_IMG_F_DEL_ONE_FACE_ADDR, cb, (u16)GUI_IMG_BTN_DELETE, 1);
    gui_img_btn_create_and_set_all_para("del", 20, 10+50+10+50+10, 200, 50, USR_DDR_IMG_F_DEL_ALL_ADDR, gui_img_btn_fdr_delete_all_handler, (u16)GUI_IMG_BTN_DELETE, 1);
    //gui_img_btn_create_and_set_all_para("del", 20, 10+50+10+50+10+50+10, 200, 50, USR_DDR_IMG_F_DEL_ALL_ADDR, gui_img_btn_fdr_delete_all_handler, (u16)GUI_IMG_BTN_DELETE, 1);
    gui_img_btn_create_and_set_all_para("mre", 0, 320-30, 40, 30, USR_DDR_IMG_RETURN_ADDR, cb, (u16)GUI_IMG_BTN_HOME, 1);
    gui_img_btn_create_and_set_all_para("mho", 240-40, 320-30, 40, 30, USR_DDR_IMG_HOME_ADDR, cb, (u16)GUI_IMG_BTN_HOME, 1);
    gui_img_create_and_set_all_para("deS", 0, 0, _display_width, _display_height, USR_DDR_IMG_DELETE_SUCCESSED_ADDR, 0);
    gui_img_create_and_set_all_para("deF", 0, 0, _display_width, _display_height, USR_DDR_IMG_DELETE_FAILED_ADDR, 0);


    //page 5 - password management
    gui_page_create((u8)PAGE_IDX_5);
    gui_bg_create_and_set_all_para("bg0", 0, 0, _display_width, _display_height, BLACK, 1);
    gui_img_btn_create_and_set_all_para("pwa", 20, 10, 200, 50, USR_DDR_IMG_PW_ADD_ADDR, cb, (u16)GUI_IMG_BTN_REGISTER, 1);
    gui_img_btn_create_and_set_all_para("pwd", 20, 10+50+10, 200, 50, USR_DDR_IMG_PW_DEL_ADDR, cb, (u16)GUI_IMG_BTN_DELETE, 1);
    gui_img_btn_create_and_set_all_para("pwu", 20, 10+50+10+50+10, 200, 50, USR_DDR_IMG_PW_UNLOCK_ADDR, cb, (u16)GUI_IMG_BTN_RECOGNIZE, 1);
    gui_img_btn_create_and_set_all_para("mre", 0, 320-30, 40, 30, USR_DDR_IMG_RETURN_ADDR, cb, (u16)GUI_IMG_BTN_HOME, 1);
    gui_img_btn_create_and_set_all_para("mho", 240-40, 320-30, 40, 30, USR_DDR_IMG_HOME_ADDR, cb, (u16)GUI_IMG_BTN_HOME, 1);
    //pictures
    gui_img_create_and_set_all_para("deS", 0, 0, _display_width, _display_height, USR_DDR_IMG_DELETE_SUCCESSED_ADDR, 0);
    gui_img_create_and_set_all_para("deF", 0, 0, _display_width, _display_height, USR_DDR_IMG_DELETE_FAILED_ADDR, 0);


    //page 6 - system
    gui_page_create((u8)PAGE_IDX_6);
    gui_bg_create_and_set_all_para("bg0", 0, 0, _display_width, _display_height, BLACK, 1);
    gui_img_btn_create_and_set_all_para("mre", 0, 320-30, 40, 30, USR_DDR_IMG_RETURN_ADDR, cb, (u16)GUI_IMG_BTN_HOME, 1);
    gui_img_btn_create_and_set_all_para("mho", 240-40, 320-30, 40, 30, USR_DDR_IMG_HOME_ADDR, cb, (u16)GUI_IMG_BTN_HOME, 1);
    gui_img_create_and_set_all_para("sys", (240-110)/2, 0, 110, 30, USR_DDR_IMG_SYS_ICON_ADDR, 1);
    //UUID
    gui_img_create_and_set_all_para("sid", 0, 30+5, 240, 20, USR_DDR_IMG_SYS_UUID_ADDR, 1);
    //gui_text_area_create_and_set_all_para("txt", 0, 35-15, 66, 66, USR_DDR_IMG_DIGIT_STAR_ADDR, gui_text_draw, BLACK, 22, 1);
    gui_text_only_create_and_set_all_para("t01", 0, 35+6, 66, 66, gui_text_only_draw_uuid, 1);
    //fw version
    gui_img_create_and_set_all_para("sfw", 0, 35+20+40+5, 240, 20, USR_DDR_IMG_SYS_FW_INFO_ADDR, 1);
    gui_text_only_create_and_set_all_para("t01", 0, 100+6, 66, 66, gui_text_only_draw_fw_version, 1);
    //number of people registered
    gui_img_create_and_set_all_para("spn", 0, 100+20+40+5, 240, 40, USR_DDR_IMG_SYS_NUM_ADDR, 1);
    gui_text_only_create_and_set_all_para("t01", 0, 165+26, 66, 66, gui_text_only_draw_reg_num, 1);
#endif
    return 0;
}

static u8 gui_app_on_off = 1;
extern void gui_app_stop(void) {
    if (0 == tid_gui)   return;
    
    if (1 == gui_app_on_off) {
        osEventFlagsSet(kl520_api_get_event(), KL520_APP_GUI_STOP);
        gui_app_on_off = 0;
        osDelay(5);
    }
}
extern void gui_app_proceed(void) {
    if (0 == tid_gui)   return;

    if (0 == gui_app_on_off) {
        osDelay(5);
        osEventFlagsClear(kl520_api_get_event(), KL520_APP_GUI_STOP);
        gui_app_on_off = 1;
    }
}
extern u8 gui_app_get_status(void) {
    return gui_app_on_off;
}

#endif
