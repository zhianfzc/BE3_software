#pragma once
#include "board_kl520.h"
#include "user_ui.h"

#if ( CFG_GUI_ENABLE == YES )
#include "sample_gui_main.h"
#include "gui_base.h"
#include "gui_mvc.h"
#include "gui_fsm.h"
#include "kl520_api_camera.h"


#define MAX_PASSWORD_LEN    (6)

typedef enum _fsm_state
{
    REMAIN_STATE = -1,
    STATE_0 = 0,
    STATE_1,
    STATE_2,
    STATE_3,
    STATE_4,
    STATE_5,
    STATE_6,
    STATE_7,
} fsm_state;

typedef enum _gui_obj_func
{
    GUI_NULL = -1,
    //touch related obj :
    GUI_IMG_BTN_GO_TO_KEY = 0,
    GUI_IMG_BTN_RECOGNIZE,
    GUI_IMG_BTN_REGISTER,
    GUI_IMG_BTN_DELETE,
    GUI_IMG_BTN_RETURN,
    GUI_IMG_BTN_HOME,
    GUI_KEYPAD_BTN,
    GUI_IMG_BTN_LIVENESS,
    GUI_RECOGNIZE_RET,
    GUI_REGISTER_RET,
    GUI_POWER_OFF,
    GUI_IMG_BTN_DELETE_1,
    GUI_IMG_BTN_REGISTER_PAGE,
    //obj with no touch :
    GUI_BG_0 = 128,
} gui_obj_func;

typedef enum _gui_fsm_src {
    GUI_FSM_SRC_NULL = 0,
    GUI_FSM_SRC_TOUCH = 1,
    GUI_FSM_SRC_USER_COM = 2,
} gui_fsm_src;

typedef struct _user_flash_data
{
    u8 password[MAX_PASSWORD_LEN];
    u8 length;
} user_flash_data;

extern void ui_fsm_init(void* model);
extern int ui_fsm_deinit(void);
extern char run_main_state_machine(char state,user_behavior_data *event);
extern void user_gui_update_renderer(void);
extern void user_gui_fsm_recognition_done(u32 data, u32 data2);
extern void user_gui_fsm_registraion_done(u32 data, u32 data2);
extern void user_gui_face_query_all_users(void);
extern int gui_obj_sele_draw_two_img(u8* sImgName0, u8* sImgName1, u8 nRet);
#endif
