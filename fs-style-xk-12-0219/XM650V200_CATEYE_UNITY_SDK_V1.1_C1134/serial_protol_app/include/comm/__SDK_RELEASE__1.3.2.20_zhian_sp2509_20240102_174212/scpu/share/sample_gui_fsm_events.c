#include "sample_gui_fsm_events.h"

#ifndef GUI_VERSION_TYPE
#define GUI_V1                          (0)
#define GUI_VERSION_TYPE                (GUI_V1)
#endif

#if ( CFG_GUI_ENABLE == YES )
#include "pinmux.h"
#include "kdp520_gpio.h"
#include "kl520_include.h"
#include "sample_gui_main.h"
#include "sample_app_console.h"
#include "gui_fsm.h"
#include "gui_text.h"
#include "gui_text_btn.h"
#include "board_ddr_table.h"
#include "framework/event.h"
#include "framework/v2k_image.h"
#include "board_ddr_table.h"
#include "board_flash_table.h"
#include "kdp_memxfer.h"
#include "flash.h"
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF ) || ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF_USR )
#include "sample_user_com_and_gui_fdr.h"
#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP )
#include "kdp_comm_and_gui_fdr.h"
#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP_USR )
#include "user_comm_and_gui_fdr.h"
#endif

#include "common.h"
#include "kl520_api_fdfr.h"

#if ( GUI_VERSION_TYPE == GUI_V1)
#define MAX_EVENT_SIZE      (11)
#define MAX_STATE_SIZE      (5)
#define MAX_TEXT_BTN_SIZE   (5)

#elif ( GUI_VERSION_TYPE == GUI_V2)
#define MAX_EVENT_SIZE      (13)
#define MAX_STATE_SIZE      (8)
#define MAX_TEXT_BTN_SIZE   (5)
#endif

osThreadId_t tid_ui_fsm;
static gui_mvc_model* _model_ptr;
static const s8 _password_default[MAX_PASSWORD_LEN] = "123456";
static const s8 _password_empty[MAX_PASSWORD_LEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
static s8 _password_saved[MAX_PASSWORD_LEN];
static u8 _password_saved_len = 0;

static signed char _user_ui_gui_go_to_main_page(void);
static signed char _user_ui_gui_go_to_keypad_page(void);
static void _user_ui_gui_save_password(s8* password, u8 length);
static u8 _user_ui_gui_get_password(s8* password);



#if ( GUI_VERSION_TYPE == GUI_V1)
#elif ( GUI_VERSION_TYPE == GUI_V2)
typedef enum _e_keypad_mode {
    KEYPAD_MODE_REG = 0,
    KEYPAD_MODE_REC = 1,
} e_keypad_mode;
static signed char _page_to_state_idx[MAX_STATE_SIZE] = {STATE_1, STATE_2, STATE_3, STATE_4, STATE_5, STATE_6, STATE_7, REMAIN_STATE};
static e_keypad_mode _keypad_mode = KEYPAD_MODE_REG;
static signed char _user_ui_gui_go_to_previous_page(void);
static signed char _user_ui_gui_go_to_manage_page(void);
static signed char _user_ui_gui_go_to_password_page(void);
#endif


int gui_obj_sele_draw_two_img(u8* sImgName0, u8* sImgName1, u8 nRet)
{
    int ret = -1;
    if (0 == tid_gui)   return ret;

    if (0 == nRet)
        ret = gui_obj_set_on_off_by_name(sImgName0, 1);
    else
        ret = gui_obj_set_on_off_by_name(sImgName1, 1);

    _model_ptr->update_model();
    osDelay(DISPLAY_RESULT_HOLD_TIME);
    gui_obj_set_on_off_by_name(sImgName0, 0);
    gui_obj_set_on_off_by_name(sImgName1, 0);
    return ret;
}

void ui_fsm_init(void* model)
{
    osThreadAttr_t attr = {
        .stack_size = 1024,
        .priority = osPriorityNormal
    };

    _model_ptr = (gui_mvc_model*) model;

    tid_ui_fsm = osThreadNew(user_interface_fsm_thread, NULL, &attr);
    
    _password_saved_len = _user_ui_gui_get_password(_password_saved);
}
int ui_fsm_deinit(void){ return 0; }

static signed char _a_null(user_behavior_data *event){
    dbg_msg_err("wrong fsm state\n");
    return REMAIN_STATE;
}

static signed char _a_0(user_behavior_data *event){
    kl520_api_cam_disp_ctrl(API_CTRL_DISP_OPEN, NULL, PERMANENT_GUI);
    return _user_ui_gui_go_to_main_page();
}

#if ( GUI_VERSION_TYPE == GUI_V1)
static signed char _a_1(user_behavior_data *event){
    return _user_ui_gui_go_to_keypad_page();
}
#elif ( GUI_VERSION_TYPE == GUI_V2)
static signed char _a_pw_page(user_behavior_data *event){
    return _user_ui_gui_go_to_password_page();
}

static signed char _a_keypad_reg(user_behavior_data *event){
    _keypad_mode = KEYPAD_MODE_REG;
    return _user_ui_gui_go_to_keypad_page();
}

static signed char _a_keypad_rec(user_behavior_data *event){
    _keypad_mode = KEYPAD_MODE_REC;
    return _user_ui_gui_go_to_keypad_page();
}

static signed char _a_pw_del(user_behavior_data *event){
    strncpy(_password_saved, _password_default, MAX_PASSWORD_LEN);
    _password_saved_len = MAX_PASSWORD_LEN;
    gui_obj_sele_draw_two_img("deS", "deF", 0);
    _model_ptr->update_model();
    return REMAIN_STATE;
}

static signed char _a_home(user_behavior_data *event){
    return _user_ui_gui_go_to_main_page();
}
#endif

static signed char _a_rec_0(user_behavior_data *event)
{
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF ) || ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF_USR )
    if (event->data2 == GUI_FSM_SRC_TOUCH)
    {
        kl520_api_hmi_ctrl_state_set(CTRL_GUI);
    }
    else
    {
        kl520_api_hmi_ctrl_state_set(CTRL_CMD);
    }
#endif
    gui_page_set_present_index((u8)PAGE_IDX_2);
    user_com_set_data(event->type, event->data, event->data2);
    user_com_event_start(USER_COM_FLAG_RECOGNITION);
    return STATE_3;
}
static signed char _a_rec_1(user_behavior_data *event)
{
    user_gui_fsm_recognition_done(event->data, event->data2);
    return _user_ui_gui_go_to_main_page();
}

static signed char _a_reg_0(user_behavior_data *event)
{
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF ) || ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF_USR )
    if (event->data2 == GUI_FSM_SRC_TOUCH)
    {
        kl520_api_hmi_ctrl_state_set(CTRL_GUI);
    }
    else
    {
        kl520_api_hmi_ctrl_state_set(CTRL_CMD);
    }
#endif
    gui_page_set_present_index((u8)PAGE_IDX_2);
    user_com_set_data(event->type, event->data, event->data2);
    user_com_event_start(USER_COM_FLAG_REGISTRATION);
    return STATE_3;
}
static signed char _a_reg_1(user_behavior_data *event)
{
    user_gui_fsm_registraion_done(event->data, event->data2);
    u8 total_id_num, face_status[MAX_USER] = {0};
    kl520_api_face_query_all(&total_id_num, face_status);


    if (event->data2 == GUI_FSM_SRC_TOUCH)
    {
#if ( GUI_VERSION_TYPE == GUI_V1)
        u8 res = event->data & 0xFF;

        if (1 == total_id_num && res == 0) //first one
            return _user_ui_gui_go_to_keypad_page();
        else
            return _user_ui_gui_go_to_main_page();
#elif ( GUI_VERSION_TYPE == GUI_V2)
        return _user_ui_gui_go_to_manage_page();
#endif
    }
    else
    {
        return _user_ui_gui_go_to_main_page();
    }
}

static signed char _a_fdr_int(user_behavior_data *event)
{
    gui_page_set_present_index((u8)PAGE_IDX_0);
    user_com_event_interrupt();
    notify_user_behavior_event(event->type, event->data, event->data2);
    return STATE_1;
}
static signed char _a_fdr_int_ret(user_behavior_data *event)
{
    user_com_event_interrupt();
    kl520_api_cam_disp_ctrl(API_CTRL_DISP_OPEN, NULL, PERMANENT_NULL);

    gui_page_set_present_index((u8)PAGE_IDX_0);

    _model_ptr->update_model();
    return STATE_1;
}
static signed char _a_fdr_int_key(user_behavior_data *event)
{
    user_com_event_interrupt();
    kl520_api_cam_disp_ctrl(API_CTRL_DISP_OPEN, NULL, PERMANENT_NULL);

    gui_page_set_present_index((u8)PAGE_IDX_0);

    return _user_ui_gui_go_to_keypad_page();
}

static u8 _del_one_page = 0;
void user_gui_face_query_all_users(void)
{
    u8 total_id_num;
    u8 face_status[MAX_USER] = {0};
    kl520_api_face_query_all(&total_id_num, face_status);


    dbg_msg_err("db number: %d, user id: 0x%X\n", total_id_num, kl520_api_get_start_user_id());

    u8 offset = _del_one_page*MAX_TEXT_BTN_SIZE;
    u8 name[3] = "d__";
    for(int i=0; i<MAX_TEXT_BTN_SIZE; i++) {
        name[2] = '1'+i;
        if (kl520_api_get_start_user_id()<=face_status[i+offset] && MAX_USER+kl520_api_get_start_user_id()>=face_status[i+offset]) {
            gui_obj_set_on_off_by_name(name, 1);
            gui_text_btn_set_text_by_name(name, face_status[i+offset]-kl520_api_get_start_user_id()+1);
            gui_text_btn_set_data_by_name(name, face_status[i+offset]);
        }
        else
            gui_obj_set_on_off_by_name(name, 0);
    }
}
static signed char _a_del(user_behavior_data *event)
{
    int ret = -1;

#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF ) || ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF_USR )
    if (0 == event->data) {
        ret = uart_sample_face_del_all();
        //strncpy(_password_saved, _password_default, MAX_PASSWORD_LEN);
    }
    else
#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP ) || ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP_USR )
    if (0 == event->data)
    {
        user_com_set_data(event->type, event->data, event->data2);
        user_com_event_start(USER_COM_FLAG_DELETE_ALL);
        return REMAIN_STATE;
    }
    else 
#endif
    {
        if (event->data2 == GUI_FSM_SRC_USER_COM)//user_com
        {
            if (event->data2 == GUI_FSM_SRC_TOUCH)
            {
                user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);
            }
            ret = uart_sample_face_del_user(event->data);
        }
        else
        {//touch
            gui_page_set_present_index((u8)PAGE_IDX_3);
            user_gui_face_query_all_users();
            _model_ptr->update_model();
            return STATE_4;
        }
    }

    gui_obj_sele_draw_two_img("deS", "deF", ret);

    _model_ptr->update_model();

    if (event->data2 == GUI_FSM_SRC_USER_COM)
    {
        user_com_response_data((u8*)&ret, 1);
    }

    user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);

    return REMAIN_STATE;
}
static signed char _a_del_one(user_behavior_data *event)
{
    fsm_state eRetPage = REMAIN_STATE;
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF ) || ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF_USR )
    int ret = -1;

    user_com_thread_event_set(USER_COM_THREAD_EVENT_NON_OVERWRITABLE);

    if ( ( event->data2 == GUI_FSM_SRC_USER_COM ) && ( event->data < 0x80 ) )
    {
        //Fail
        ret = 1;
    }
    else
    {
        ret = uart_sample_face_del_user(event->data);
    }

    gui_page_set_present_index((u8)PAGE_IDX_0);
    gui_obj_sele_draw_two_img("deS", "deF", ret);

    if ( event->data2 == GUI_FSM_SRC_TOUCH )
    {
        gui_page_set_present_index((u8)PAGE_IDX_3);
        user_gui_face_query_all_users();
    }

    _model_ptr->update_model();

    if ( event->data2 == GUI_FSM_SRC_USER_COM )
    {
        user_com_response_data((u8*)&ret, 1);
        eRetPage = STATE_1;
    }

    user_com_thread_event_set(USER_COM_THREAD_EVENT_READY);

#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP ) || ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP_USR )
    user_com_set_data(event->type, event->data, event->data2);
    user_com_event_start(USER_COM_FLAG_DELETE_ONE);
#endif

    return eRetPage;
}
static signed char _a_d1_next(user_behavior_data *event) {
    u8 total_id_num;
    u8 face_status[MAX_USER] = {0};
    kl520_api_face_query_all(&total_id_num, face_status);
    s16 diff = total_id_num - (_del_one_page+1)*MAX_TEXT_BTN_SIZE;
    if (0<diff) {
        _del_one_page++;
        user_gui_face_query_all_users();
        _model_ptr->update_model();
    }
    return REMAIN_STATE;
}
static signed char _a_d1_pre(user_behavior_data *event) {
    u8 total_id_num;
    u8 face_status[MAX_USER] = {0};
    kl520_api_face_query_all(&total_id_num, face_status);
    s16 diff = (_del_one_page+1)*MAX_TEXT_BTN_SIZE - total_id_num;
    if (0<diff && 0<_del_one_page) {
        _del_one_page--;
        user_gui_face_query_all_users();
        _model_ptr->update_model();
    }
    return REMAIN_STATE;
}

static signed char _a_ret(user_behavior_data *event){
#if ( GUI_VERSION_TYPE == GUI_V1)
    _del_one_page = 0;
    gui_text_delete_all();
    return _user_ui_gui_go_to_main_page();

#elif ( GUI_VERSION_TYPE == GUI_V2)
    return _user_ui_gui_go_to_previous_page();
#endif
}

#if ( GUI_VERSION_TYPE == GUI_V1)
static signed char _a_6(user_behavior_data *event)
{
    s8 password_present[7];
    u8 max_len = gui_text_get_max_len();
    u8 len = gui_text_get_str(password_present);
    //dbg_msg_console("str = %s\n", password_present);

    if ('o' == event->data)
    {
        //save password
        if (0 == strncmp(_password_saved, _password_default, sizeof(_password_default))) {
            memset(_password_saved, 0, sizeof(_password_saved));
            strncpy(_password_saved, password_present, len);
            _user_ui_gui_save_password(_password_saved, len);
            _password_saved_len = len;
            //return to main page
            gui_text_delete_all();
            return _user_ui_gui_go_to_main_page();
        }
        //verify password
        else {
            if ( 0 == strncmp(_password_saved, password_present, _password_saved_len) &&
                len == _password_saved_len )
                gui_obj_sele_draw_two_img("rcS", "rcF", 0);
            else
                gui_obj_sele_draw_two_img("rcS", "rcF", 1);
        }
    }
    _model_ptr->update_model();

    return REMAIN_STATE;
}

#elif ( GUI_VERSION_TYPE == GUI_V2)
static signed char _a_pw_proc(user_behavior_data *event)
{
    s8 password_present[7];
    u8 max_len = gui_text_get_max_len();
    u8 len = gui_text_get_str(password_present);
    //dbg_msg_console("str = %s\n", password_present);

    if ('o' == event->data) {
        //save password
        if (KEYPAD_MODE_REG == _keypad_mode) {
            if (0 == strncmp(_password_saved, _password_default, sizeof(_password_default))) {
                memset(_password_saved, 0, sizeof(_password_saved));
                strncpy(_password_saved, password_present, len);
                _user_ui_gui_save_password(_password_saved, len);
                _password_saved_len = len;
                gui_obj_sele_draw_two_img("reS", "reF", 0);
            }
            else
            {
                gui_obj_sele_draw_two_img("reS", "reF", 1);
            }

            return _user_ui_gui_go_to_password_page();
        }
        //verify password
        if (KEYPAD_MODE_REC == _keypad_mode) {
            if ( 0 == strncmp(_password_saved, password_present, _password_saved_len) &&
                len == _password_saved_len )
            {
                gui_obj_sele_draw_two_img("rcS", "rcF", 0);
            }
            else
            {
                gui_obj_sele_draw_two_img("rcS", "rcF", 1);
            }

            return _user_ui_gui_go_to_password_page();
        }
    }
    _model_ptr->update_model();

    return REMAIN_STATE;
}
#endif

static signed char _a_off(user_behavior_data *event)
{
    user_com_event_interrupt();
    osDelay(10);
    user_com_event_power_off();
    return REMAIN_STATE;
}

#if ( GUI_VERSION_TYPE == GUI_V1)
static signed char (*_action[MAX_EVENT_SIZE][MAX_STATE_SIZE])(user_behavior_data *event) =
{
    //bg       //main       //keypad    //fdr_int
    { _a_0,    _a_1,        _a_null,    _a_fdr_int_key, _a_d1_next },
    { _a_null, _a_rec_0,    _a_null,    _a_fdr_int,     _a_null },
    { _a_null, _a_reg_0,    _a_null,    _a_fdr_int,     _a_null },
    { _a_null, _a_del,      _a_null,    _a_fdr_int,     _a_null },
    { _a_null, _a_null,     _a_ret,     _a_fdr_int_ret, _a_ret },
    { _a_null, _a_null,     _a_6,       _a_null,        _a_d1_pre },
    { _a_null, _a_null,     _a_null,    _a_null,        _a_null },
    { _a_null, _a_null,     _a_null,    _a_rec_1,       _a_null },
    { _a_null, _a_null,     _a_null,    _a_reg_1,       _a_null },
    { _a_off,  _a_off,      _a_off,     _a_off,         _a_off },
    { _a_null, _a_del_one,     _a_null,    _a_null,        _a_del_one },
};

#elif ( GUI_VERSION_TYPE == GUI_V2)
static signed char _a_mng(user_behavior_data *event) {
    gui_page_set_present_index((u8)PAGE_IDX_4);
    _model_ptr->update_model();
    return STATE_5;
}
static signed char _a_sys(user_behavior_data *event) {
    gui_page_set_present_index((u8)PAGE_IDX_6);
    _model_ptr->update_model();
    return STATE_7;
}

static signed char (*_action[MAX_EVENT_SIZE][MAX_STATE_SIZE])(user_behavior_data *event) =
{
    //bg       //main      //keypad    //fdr_int       //delete_one //manage    //password     //system
    { _a_0,    _a_pw_page, _a_null,    _a_fdr_int_key, _a_d1_next, _a_null,     _a_null,       _a_null },
    { _a_null, _a_rec_0,   _a_null,    _a_fdr_int,     _a_null,    _a_null,     _a_keypad_rec, _a_null },
    { _a_null, _a_reg_0,   _a_null,    _a_fdr_int,     _a_null,    _a_reg_0,    _a_keypad_reg, _a_null },
    { _a_null, _a_del,     _a_null,    _a_fdr_int,     _a_null,    _a_del,      _a_pw_del,     _a_null },
    { _a_null, _a_sys,     _a_ret,     _a_fdr_int_ret, _a_ret,     _a_ret,      _a_null,       _a_ret },
    { _a_null, _a_null,    _a_pw_proc, _a_null,        _a_d1_pre,  _a_null,     _a_null,       _a_null },
    { _a_null, _a_null,    _a_null,    _a_null,        _a_null,    _a_null,     _a_null,       _a_null },
    { _a_null, _a_null,    _a_null,    _a_rec_1,       _a_null,    _a_null,     _a_null,       _a_null },
    { _a_null, _a_null,    _a_null,    _a_reg_1,       _a_null,    _a_null,     _a_null,       _a_null },
    { _a_off,  _a_off,     _a_off,     _a_off,         _a_off,     _a_null,     _a_null,       _a_null },
    { _a_null, _a_null,    _a_null,    _a_null,        _a_del_one, _a_null,     _a_null,       _a_null },
    { _a_null, _a_mng,     _a_null,    _a_null,        _a_null,    _a_null,     _a_null,       _a_null },
    { _a_null, _a_null,    _a_null,    _a_home,        _a_home,    _a_home,     _a_home,       _a_home },
};
#endif


extern char run_main_state_machine(char state,user_behavior_data *event)
{
    int i;
    signed char ret;

    i = -1;
    switch (event->type)
    {
        case GUI_IMG_BTN_GO_TO_KEY:       i = 0;    break;
        case GUI_IMG_BTN_RECOGNIZE:       i = 1;    break;
        case GUI_IMG_BTN_REGISTER:        i = 2;    break;
        case GUI_IMG_BTN_DELETE:          i = 3;    break;
        case GUI_IMG_BTN_RETURN:          i = 4;    break;
        case GUI_KEYPAD_BTN:              i = 5;    break;
        case GUI_IMG_BTN_LIVENESS:        i = 6;    break;
        case GUI_RECOGNIZE_RET:           i = 7;    break;
        case GUI_REGISTER_RET:            i = 8;    break;
        case GUI_POWER_OFF:               i = 9;    break;
        case GUI_IMG_BTN_DELETE_1:        i = 10;   break;
#if ( GUI_VERSION_TYPE == GUI_V1)
#elif ( GUI_VERSION_TYPE == GUI_V2)
        case GUI_IMG_BTN_REGISTER_PAGE:   i = 11;   break;
        case GUI_IMG_BTN_HOME:            i = 12;    break;
#endif
        default:
            break;
    }

    if (i != -1)
    {
        ret = (*_action[i][state])(event);
        if (ret != -1)
            return ret;
    }
    return state;
}

#if ( GUI_VERSION_TYPE == GUI_V1)

#elif ( GUI_VERSION_TYPE == GUI_V2)
static void _user_ui_gui_switch_page_common_process(u8 page_idx) {
    _del_one_page = 0;
    gui_text_delete_all();
    gui_page_set_present_index(page_idx);
    _model_ptr->update_model();
}
static signed char _user_ui_gui_go_to_previous_page(void) {
    u8 page_idx = gui_page_get_previous_index();
    _user_ui_gui_switch_page_common_process(page_idx);
    return _page_to_state_idx[page_idx];
}
#endif

static signed char _user_ui_gui_go_to_main_page(void) {
#if ( GUI_VERSION_TYPE == GUI_V1)
    gui_page_set_present_index((u8)PAGE_IDX_0);
    _model_ptr->update_model();
    g_bBootupGuiHighPriority = FALSE;
    return STATE_1;

#elif ( GUI_VERSION_TYPE == GUI_V2)
    _user_ui_gui_switch_page_common_process((u8)PAGE_IDX_0);
    g_bBootupGuiHighPriority = FALSE;
    return _page_to_state_idx[PAGE_IDX_0];
#endif
}
static signed char _user_ui_gui_go_to_keypad_page(void) {
#if ( GUI_VERSION_TYPE == GUI_V1)
    gui_page_set_present_index((u8)PAGE_IDX_1);
    _model_ptr->update_model();
    return STATE_2;

#elif ( GUI_VERSION_TYPE == GUI_V2)
    _user_ui_gui_switch_page_common_process((u8)PAGE_IDX_1);
    return _page_to_state_idx[PAGE_IDX_1];
#endif
}

#if ( GUI_VERSION_TYPE == GUI_V1)
#elif ( GUI_VERSION_TYPE == GUI_V2)
static signed char _user_ui_gui_go_to_manage_page(void) {
    _user_ui_gui_switch_page_common_process((u8)PAGE_IDX_4);
    return _page_to_state_idx[PAGE_IDX_4];
}
static signed char _user_ui_gui_go_to_password_page(void) {
    _user_ui_gui_switch_page_common_process((u8)PAGE_IDX_5);
    return _page_to_state_idx[PAGE_IDX_5];
}
#endif

static void _user_ui_gui_save_password(s8* password, u8 length)
{
    user_flash_data data;
    data.length = length;
    memcpy(data.password, password, length);
    u8 size = (sizeof(user_flash_data)%4) ? ((sizeof(user_flash_data)/4+1)*4) : sizeof(user_flash_data);
    memcpy((void*)USR_DDR_SETTINGS_ADDR, &data, size);
    kdp_memxfer_ddr_to_flash(USR_FLASH_SETTINGS_ADDR, USR_DDR_SETTINGS_ADDR, size);
}
static u8 _user_ui_gui_get_password(s8* password)
{
    user_flash_data data;
    u8 len = MAX_PASSWORD_LEN;
    u8 size = (sizeof(user_flash_data)%4) ? ((sizeof(user_flash_data)/4+1)*4) : sizeof(user_flash_data);
    memcpy((void*)&data, (void*)USR_DDR_SETTINGS_ADDR, size);

    if (!strncmp(_password_empty, (const s8*)data.password, MAX_PASSWORD_LEN)) {
        //save default password to flash.
        memcpy(password, _password_default, sizeof(_password_default));
        _user_ui_gui_save_password(password, sizeof(_password_default));
    }
    else {
        //just get data
        len = data.length;
        if (MAX_PASSWORD_LEN < len)
            len = MAX_PASSWORD_LEN;
        memcpy(password, data.password, len);
    }

    return len;
}

extern void user_gui_update_renderer(void)
{
    if (0 == tid_gui)   return;
    
    if (gui_app_get_status())
        _model_ptr->update_model();
}

extern void user_gui_fsm_recognition_done(u32 data, u32 data2)
{
    gui_page_set_present_index((u8)PAGE_IDX_0);
    kl520_api_cam_disp_ctrl(API_CTRL_DISP_OPEN, NULL, PERMANENT_NULL);

    u16 ret = data&0xFFFF;
    gui_obj_sele_draw_two_img("rcS", "rcF", (ret&0xFF));

#ifdef CFG_GUI_RECOG_CLOSE_TEST
    #if (0==CFG_GUI_RECOG_CLOSE_TEST)
    _model_ptr->update_model();
    #else
    sample_close_video_renderer();
    _model_ptr->update_model();
    sample_face_close();
    sample_open_video_renderer();
    #endif
#endif

    if (data2 == GUI_FSM_SRC_USER_COM)
    {
#ifdef CUSTOMER_RECOG_PACKAGE_ADD_FACE_L2_DATA
#if ( CUSTOMER_RECOG_PACKAGE_ADD_FACE_L2_DATA == YES ) && ( ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF ) || ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF_USR ) )
        u32 nTemp = g_nFaceL2*1000000;
        nTemp = (((nTemp>>24)&0xFF)<<0)|(((nTemp>>16)&0xFF)<<8)|(((nTemp>>8)&0xFF)<<16)|(((nTemp>>0)&0xFF)<<24);
        user_com_response_large_partial_data( (u8*)&ret, sizeof(ret), FALSE );
        user_com_response_large_partial_data( (u8*)&nTemp, sizeof(nTemp), TRUE );
#else
        user_com_response_data((u8*)&ret, sizeof(ret));
#endif
#else
        user_com_response_data((u8*)&ret, sizeof(ret));
#endif

    }
}

extern void user_gui_fsm_registraion_done(u32 data, u32 data2)
{
    gui_page_set_present_index((u8)PAGE_IDX_0);
    kl520_api_cam_disp_ctrl(API_CTRL_DISP_OPEN, NULL, PERMANENT_NULL);

    u16 ret = data&0xFFFF;
    gui_obj_sele_draw_two_img("reS", "reF", (ret&0xFF));

    if (data2 == GUI_FSM_SRC_USER_COM)
        user_com_response_data((u8*)&ret, sizeof(ret));
}

#endif
