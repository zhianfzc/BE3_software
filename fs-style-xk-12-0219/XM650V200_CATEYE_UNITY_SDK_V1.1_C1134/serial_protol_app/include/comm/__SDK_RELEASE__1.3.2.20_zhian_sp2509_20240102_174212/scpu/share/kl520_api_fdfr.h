#ifndef __KL520_API_FDFR_H__
#define __KL520_API_FDFR_H__

#include "board_kl520.h"

#include "framework/v2k_image.h"
#include "board_cfg.h"
#include "kl520_include.h"
#include "kdp_e2e_face.h"
#include "kl520_api_camera.h"

enum kl520_fdfr_status_code_e{
    FDFR_STATUS_OK              = 0,
    FDFR_STATUS_IDLE            = 1,
    FDFR_STATUS_COMP_OK         = 2,
    FDFR_STATUS_COMP_FAIL       = 3,
    FDFR_STATUS_EXIST           = 4,
    FDFR_STATUS_NOFACE          = 5,
    FDFR_STATUS_FULL            = 6,
    FDFR_STATUS_OPENED          = 7,

    FDFR_STATUS_NEXT            = 10,
    FDFR_STATUS_DETECTED        = 11,
    FDFR_STATUS_BAD_POSE        = 12,
    FDFR_STATUS_TOO_FAR         = 13,
    FDFR_STATUS_TOO_NEAR        = 14,
    FDFR_STATUS_WAIT_DONT_MOVE  = 15,
    FDFR_STATUS_INVALID         = 16,
    FDFR_STATUS_MASK            = 17,
    FDFR_STATUS_EYE_CLOSE_STATUS_OPEN_EYE   = 18,
    FDFR_STATUS_EYE_CLOSED                  = 19,
    FDFR_STATUS_EYE_CLOSE_UNKNOW_STATUS     = 20,
    FDFR_STATUS_TOOUP           = 22,
    FDFR_STATUS_TOODOWN         = 23,
    FDFR_STATUS_TOOLEFT         = 24,
    FDFR_STATUS_TOORIGHT        = 25,
    FDFR_STATUS_LOW_QUALITY     = 26,
    FDFR_STATUS_CALLIB_FAIL     = 27,
    FDFR_STATUS_NORMAL          = 28,
    FDFR_STATUS_EYE_CLOSE_STATUS_CLOSE_EYE = 29,
    FDFR_STATUS_ATTACK          = 30,
    FDFR_STATUS_EMPTY           = 31,
    FDFR_STATUS_ERROR           = -1,
    FDFR_STATUS_TIMEOUT         = -2,
    FDFR_STATUS_NOFACE_AND_TIMEOUT  = -3,
    
};
#ifdef CFG_AI_3D_EXTEND_RANGE
//#define FACE_DIST_MAX_THRESH    (200)
//#define FACE_DIST_MIN_THRESH    (1)
#else
//#define FACE_DIST_MAX_THRESH    (100)   //74
//#define FACE_DIST_MIN_THRESH    (20)
#endif

#define FACE_BAD_POSE_CNT       (2)
#define FACE_BAD_POSE_LED_CNT   (2)
#define FACE_ATTACK_CNT         (10)

#define USE_FDFR_DRAWING_TIMER
#define DB_OFFSET_CMD          NO

#if (CFG_PANEL_TYPE == NULL) && ((CFG_AI_TYPE == AI_TYPE_N1R1) || CFG_AI_TYPE == AI_TYPE_N1)
#define FACE_POSTITON_UP_THRESH     (NIR_IMG_SOURCE_H / 20)
#define FACE_POSTITON_DOWN_THRESH   (NIR_IMG_SOURCE_H - NIR_IMG_SOURCE_H / 20)
#define FACE_POSTITON_LEFT_THRESH   (NIR_IMG_SOURCE_W / 20)
#define FACE_POSTITON_RIGHT_THRESH  (NIR_IMG_SOURCE_W - NIR_IMG_SOURCE_W / 20)
#else
#define FACE_POSTITON_UP_THRESH     (DISPLAY_HEIGHT/8)
#define FACE_POSTITON_DOWN_THRESH   (DISPLAY_HEIGHT - DISPLAY_HEIGHT/8)
#define FACE_POSTITON_LEFT_THRESH   (DISPLAY_WIDTH/8)
#define FACE_POSTITON_RIGHT_THRESH  (DISPLAY_WIDTH - DISPLAY_WIDTH/8)
#endif

#define FDFR_COM_EVENT_CLOSED    (1)

extern kdp_e2e_face_mode m_face_mode;

extern BOOL g_bRecognitionMandatoryFlag;
extern u32 face_reg_sts;

//int kl520_api_fdfr_facemode_get(void);
void kl520_api_fdfr_set_flow_mode(BOOL );
void kl520_api_fdfr_model_init(void);
s32 kl520_api_face_preexecute_stage1(void);
s32 kl520_api_face_preexecute_stage2(void);
s32 kl520_api_face_preexecute_stage3(void);
s32 kl520_api_face_preexecute_colse(void);
s32 kl520_api_face_preempt_init(void);
void kl520_api_tasks_init(void);
void kl520_api_tasks_init_wait_ready(void);
int kl520_api_fdfr_exist_thread(void);
int kl520_api_fdfr_element(void);
void kl520_api_fdfr_terminate_thread(void);

extern bool b_en_aec_only;
extern uint8_t m_curr_face_id;
extern kl520_dp_draw_info dp_draw_info;
inline void _api_fdfr_set_event(u32 flags_api_fdfr_state, u32 kl520_app_flag_state, bool CLR);

extern void api_fdfr_face_recognition_set_mandatory_event(void);

BOOL kl520_api_ui_fsm_dp_layout_get(void);

void kl520_api_fdfr_init_thrd(void);
void kl520_api_fdfr_start(void);
void kl520_api_fdfr_stop(void);

u8 kl520_is_fdfr_abort(void);
void kl520_set_fdfr_abort(u8 flag);
u8 kl520_fdfr_opened(void);

u8 kl520_api_get_start_user_id(void);

u8 is_enroll_customize_uid(void);
void set_enroll_customize_uid(u8 uid);
u8 get_enroll_customize_uid(void);
void set_enroll_overwrite_flag(u8 flag);
u8 get_enroll_overwrite_flag(void);

#ifdef CUSTOMIZE_DB_OFFSET
u8 get_user_db_offset(void);
void load_user_db_offset(void);
void update_user_db_offset(u8 offset);
void reset_user_db_offset(void);
#endif

void kl520_customer_info_init(void);

void switch_palm_mode(u8 mode);

#endif
