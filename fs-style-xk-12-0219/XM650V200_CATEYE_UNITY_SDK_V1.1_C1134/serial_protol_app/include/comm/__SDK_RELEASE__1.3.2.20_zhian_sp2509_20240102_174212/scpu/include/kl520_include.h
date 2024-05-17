#ifndef __KL520_INCLUDE_H__
#define __KL520_INCLUDE_H__


#include "types.h"
#include "board_kl520.h"
#include "kl520_sys.h"
#include "framework/event.h"
#include "framework/v2k_color.h"
#include "framework/framework_driver.h"
#include "kdp_flash_table.h"
#include "kdp520_pwm_timer.h"
#include "kdp_e2e_util.h"
#include "kdp_e2e_ctrl.h"
//#define  ANA_GAIN

//#define CFG_CONSOLE_MODE    1
#ifndef CFG_CONSOLE_MODE
#if (CFG_COM_BUS_TYPE & COM_BUS_USB) && (CFG_PRODUCTION_TEST != YES)
#define CFG_CONSOLE_MODE    1
#else
#undef CFG_CONSOLE_MODE
#endif
#endif

#define KL520_QUICK_BOOT                    (YES)
//#define KL520_API_FACE_CLOSE_WITH_DP_CLOSE

#ifdef DEV_TEST_VERSION
#define KL520_API_SHOW_BOUNDINGBOX          (YES)
#else
#define KL520_API_SHOW_BOUNDINGBOX          (NO)
#endif
#define KL520_API_SHOW_FAIL_LM              (NO)
#define KL520_API_SHOW_TOUCH_LOG            (NO)

#define DB_DRAWING_CUSTOMER_COLOR

#define CAMERA_DEVICE_RGB_IDX               (MIPI_CAM_RGB)
#define CAMERA_DEVICE_NIR_IDX               (MIPI_CAM_NIR)

#define LCD_BUFFER_ADDR                     (KDP_DDR_DRV_LCM_START_ADDR)

//#define ENHANCE_RGB_LED_SWITCH
#define LIGHT_HIGHER_THRESHOLD              (700)
#define LIGHT_LOWER_THRESHOLD               (180)

#if BOUNDINGBOX_CHECK_LM == NO
#define LCD_FDR_OVERLINE                    (0)
#else
#ifdef BOUNDINGBOX_CHECK_CENTER
    #ifdef CFG_CUSTOM_LCD_FDR_OVERLINE
    #define LCD_FDR_OVERLINE                (CFG_CUSTOM_LCD_FDR_OVERLINE)
    #else
    #define LCD_FDR_OVERLINE                (-50)
    #endif
#else
#define LCD_FDR_OVERLINE                    (30)
#endif
#endif

#define LCD_DISPLAY_HINT_BOX_MARGIN         (20)
#define LCD_DISPLAY_HINT_BOX_LEN            (30)
#define LCD_DISPLAY_HINT_BOX_PEN_WIDTH      (2)
#define LCD_DISPLAY_HINT_BOX_COLOR          (GRAY)

#define LCD_DISPLAY_FD_BOX_MARGIN           (20)
#define LCD_DISPLAY_FD_BOX_LEN              (30)
#define LCD_DISPLAY_FD_BOX_PEN_WIDTH        (2)
#define LCD_DISPLAY_FD_BOX_COLOR            (GREEN)
#define LCD_DISPLAY_FD_EXIST_COLOR          (RED)
#define LCD_DISPLAY_FD_BOX_LV_OK_COLOR      (GREEN)

#define LCD_DISPLAY_DB_BOX_MARGIN           (20)
#define LCD_DISPLAY_DB_BOX_LEN              (30)
#define LCD_DISPLAY_DB_BOX_PEN_WIDTH        (2)
#define LCD_DISPLAY_DB_BOX_OK_COLOR         (GREEN)
#define LCD_DISPLAY_DB_BOX_FAIL_COLOR       (RED)

#define LCD_DISPLAY_FD_OK_PEN_WIDTH         (2)
#define LCD_DISPLAY_FD_OK_COLOR             (GREEN)

#define LCD_DISPLAY_DB_OK_PEN_WIDTH         (1)
#define LCD_DISPLAY_DB_OK_COLOR             (GREEN)

#define LCD_DISPLAY_LM_BOX_PEN_WIDTH        (1)
#define LCD_DISPLAY_LM_OK_PEN_WIDTH         (4)
#define LCD_DISPLAY_LM_OK_COLOR             (GBLUE)

#define LCD_DISPLAY_FDR_FAIL_PEN_WIDTH      (4)
#define LCD_DISPLAY_FDR_FAIL_COLOR          (YELLOW)

#define KL520_APP_FLAG_FDFR_OK              (1<<0)
#define KL520_APP_FLAG_FDFR_ERR             (1<<1)
#define KL520_APP_FLAG_FDFR_TIMEOUT         (1<<2)
#define KL520_APP_FLAG_FDFR                 (KL520_APP_FLAG_FDFR_OK | KL520_APP_FLAG_FDFR_TIMEOUT | KL520_APP_FLAG_FDFR_ERR)
#define KL520_APP_FLAG_TOUCH                (1<<3)
#define KL520_APP_FLAG_TOUCH_DEINIT         (1<<4)
#define KL520_APP_FLAG_TOUCH_ALL            (KL520_APP_FLAG_TOUCH | KL520_APP_FLAG_TOUCH_DEINIT)
#define KL520_APP_FLAG_ACTION               (1<<5)
#define KL520_APP_FLAG_ACTION_TOUCH         (1<<6)
#define KL520_DEVICE_FLAG_OK                (1<<7)
#define KL520_DEVICE_FLAG_ERR               (1<<8)
#define KL520_APP_GUI_STOP                  (1<<9)
#define KL520_APP_FLAG_COMM                 (1<<10)
#define KL520_APP_FLAG_COMM_DONE            (1<<11)
#define KL520_APP_THREAD_ERR                (1<<12)
#define KL520_APP_FLAG_ALL                  (0X000FFFFF)

#define FLAGS_API_FDFR_ADD_EVT              0x00000200
#define FLAGS_API_FDFR_RECOGNITION_EVT      0x00000400
#define FLAGS_API_FDFR_RECOGNITION_TEST_EVT 0x00000800
#define FLAGS_API_FDFR_CLOSE_EVT            0x00001000
#define FLAGS_API_FDFR_LIVENESS_EVT         0x00002000
#define FLAGS_API_FDFR_ABORT_EVT            0x00004000
#define FLAGS_API_FDFR_SNAPIMG_EVT          0x00008000
#define FLAGS_API_FDFR_COMPARE_1VS1_EVT     0x00010000
#define FLAGS_API_FDFR_ALL_EVT              0x0000FF00

#define KL520_CAMERA_SKIP_MS                (0)
#define KL520_FDFR_SKIP_MS                  (0)
#define KL520_FDFR_KEEP_MS                  (300)

#define KL520_FACE_SCORE_MIN                (-1)
#define KL520_FACE_SCORE_MAX                (80)
/* Possible return value of kl520_api_face_get_result() */
#define KL520_FACE_OK                       (0)
#define KL520_FACE_DB_OK                    (1)
#define KL520_FACE_FAIL                     (2)
#define KL520_FACE_DB_FAIL                  (3)
#define KL520_FACE_NOFACE                   (4)
#define KL520_FACE_NOFACE_AND_TIMEOUT       (5)
#define KL520_FACE_TIMEOUT                  (6)
#define KL520_FACE_EXIST                    (7)
#define KL520_FACE_EMPTY                    (8)
#define KL520_FACE_FULL                     (9)
#define KL520_FACE_BADPOSE                  (10)

#define KL520_FACE_INVALID                  (11)

#define KL520_FACE_TOO_FAR                  (12)
#define KL520_FACE_TOO_NEAR                 (13)
#define KL520_FACE_WAIT_DONT_MOVE           (14)
#define KL520_FACE_LIVENESS_OK              (15)    // only for drawing
#define KL520_FACE_DETECTED                 (16)
#define KL520_FACE_MASK                     (17)
#define KL520_FACE_EYE_CLOSE_STATUS_OPEN_EYE    (18)        //KL520_FACE_EYE_CLOSED_CONT
#define KL520_FACE_EYE_CLOSED                   (19)
#define KL520_FACE_EYE_CLOSE_UNKNOW_STATUS      (20)
#define KL520_FACE_TOOUP                    (21)
#define KL520_FACE_TOODOWN                  (22)
#define KL520_FACE_TOOLEFT                  (23)
#define KL520_FACE_TOORIGHT                 (24)
#define KL520_FACE_LOW_QUALITY              (25)
#define KL520_FACE_FACE_TO                  (0xFD)	//lmm-edit

#define KL520_FACE_SEND_NEXT_IMAGE          (26)
#define KL520_FACE_CALLIB_FAIL              (27)
#define KL520_FACE_ATTACK                   (30)

#if CFG_FMAP_NO_COMP_ENABLE == YES
#define KL520_DB_COMP_FAIL_ALLOWED_MAX      (1)
#else
#define KL520_DB_COMP_FAIL_ALLOWED_MAX      (1)
#endif

#if (CFG_CAMERA_DUAL_1054 == 1 || CFG_CAMERA_SINGLE_1054 == 1)
#define NORMAL_POSE_SCORE0  (0.98f)
#define NORMAL_POSE_SCORE1  (0.85f)
#define NORMAL_POSE_YAW     (8)
#define NORMAL_POSE_ROLL    (12)
#define NORMAL_POSE_UD0     (15)
#define NORMAL_POSE_UD1     (-15)
#define MAX_LR_POSE    70
#define MAX_UD_POSE    70
#if CFG_ZHIAN == 1
#define MIN_LR_POSE    12
#else
#define MIN_LR_POSE    6
#endif
#define MIN_UD_POSE    1
#else
#define NORMAL_POSE_SCORE0  (0.98f)
#define NORMAL_POSE_SCORE1  (0.85f)
#define NORMAL_POSE_YAW     (12)
#define NORMAL_POSE_ROLL    (12)
#define NORMAL_POSE_UD0     (30)
#define NORMAL_POSE_UD1     (-30)
#define MAX_LR_POSE    70
#define MAX_UD_POSE    70
#define MIN_LR_POSE    12
#define MIN_UD_POSE    1
#endif

extern const float reg_normal_score0;
extern const float reg_normal_score1;
extern const s8 reg_normal_pitch0;
extern const s8 reg_normal_pitch1;
extern const s8 reg_normal_yaw;
extern const s8 reg_normal_roll;

#if CFG_AI_TYPE == AI_TYPE_N1
#define KL520_THRESH_NORMAL_MAX_UD          (15)
#define KL520_THRESH_NORMAL_MAX_LR          (10)//8)
#define KL520_THRESH_LEFT_MIN               (60)
#define KL520_THRESH_LEFT_MAX               (90)
#define KL520_THRESH_RIGHT_MIN              (60)
#define KL520_THRESH_RIGHT_MAX              (90)
#else
#define KL520_THRESH_NORMAL_MAX_UD          (15)
#define KL520_THRESH_NORMAL_MAX_LR          (8)//8)
#define KL520_THRESH_LEFT_MIN               (60)
#define KL520_THRESH_LEFT_MAX               (75)
#define KL520_THRESH_RIGHT_MIN              (60)
#define KL520_THRESH_RIGHT_MAX              (75)
#endif
#define KL520_THRESH_NORMAL_ANGLE_RATIO     (120)
#ifdef HEAD_POSE_CHECK_PERCENT
#if CFG_AI_TYPE != AI_TYPE_N1
#define KL520_THRESH_UP_MIN                 (8)
#define KL520_THRESH_DOWN_MIN               (8)
#else
#define KL520_THRESH_UP_MIN                 (4)
#define KL520_THRESH_DOWN_MIN               (4)
#endif
#define KL520_THRESH_UP_MAX                 (30)
#define KL520_THRESH_DOWN_MAX               (30)
#else
#define KL520_THRESH_UP_MIN                 (50)
#define KL520_THRESH_UP_MAX                 (65)
#define KL520_THRESH_DOWN_MIN               (50)
#define KL520_THRESH_DOWN_MAX               (65)
#endif

#define KL520_MIDDLE_LINE                   (70)
#define KL520_MIDDLE_LINE1                  (20)
#define KL520_HINT_ARROW_LEN                (20)
#define KL520_HINT_ARROW                    (10)
#define LCD_DISPLAY_ARROW_COLOR             (0xFEA0)   //GOLD


#define KL520_DEVICE_OK                     (0)
#define KL520_DEVICE_ERR_NIR_ID             (1<<0)
#define KL520_DEVICE_ERR_RGB_ID             (1<<1)
#define KL520_DEVICE_ERR_FLASH_ID           (1<<2)
#define KL520_DEVICE_ERR_TOUCH_ID           (1<<3)
#define KL520_DEVICE_ERR_LCM_ID             (1<<4)
#define KL520_DEVICE_ERR_IOEXTENDER_ID      (1<<5)

#define KL520_API_FACE_NORMAL_RST_TIMEOUT   (YES)

#define KL520_DEFAULT_TIMEOUT_MS            (10)
#define KL520_DEFAULT_ADD_TIMEOUT_S         (30)
#define KL520_DEFAULT_REC_TIMEOUT_S         (30)
#define KL520_DEFAULT_LIVENESS_TIMEOUT_S    (30)

#define KL520_DEFAULT_DB_THR_ENVIR_DIFF_0   (E2E_DB_DYNAMIC_THRESHOLD_DEF)  //environment 0
#define KL520_DEFAULT_DB_THR_ENVIR_DIFF_1   (E2E_DB_DYNAMIC_THRESHOLD_DEF)  //environment 1
#define KL520_DEFAULT_DB_THR_ENVIR_DIFF_2   (E2E_DB_DYNAMIC_THRESHOLD_DEF)  //environment 2

#if CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP
#ifndef KL520_FACE_ADD_BMP
#define KL520_FACE_ADD_BMP  1
#endif
#endif

#if( CFG_LW3D_TYPE == CFG_LW3D_940 )
#else
#define HEAD_POSE_LR_FIRST          YES
#endif

#define KDP_CHK_BREAK(func)     { ret = (func); if (0 != ret) break; }

#define ROTATE_180_ENABLE   (0x96)
#define ROTATE_180_DISABLE  (0xFF)

extern bool g_bPowerDown;

// jim : new db add mode
typedef enum _KL520_FACE_DB_ADD_MODE_
{
    FACE_ADD_MODE_IN_DB = 0,
    FACE_ADD_MODE_NO_DB

} kl520_face_db_add_mode;

typedef enum _KL520_FACE_ADD_MODE_
{
    FACE_ADD_MODE_DEFAULT = 0,
    FACE_ADD_MODE_1_FACE,
    FACE_ADD_MODE_5_FACES

} kl520_face_add_mode;

typedef enum _KL520_FACE_ADD_TYPE_
{
    FACE_ADD_TYPE_NORMAL = 0,
    FACE_ADD_TYPE_LEFT,
    FACE_ADD_TYPE_RIGHT,
    FACE_ADD_TYPE_UP,
    FACE_ADD_TYPE_DOWN,

} kl520_face_add_type;

#if ( KDP_BIT_CTRL_MODE == YES )
typedef enum _KL520_BIT_CTRL_FACE_ADD_
{
    BIT_CTRL_FACE_ADD_NORMAL    = 0x01,
    BIT_CTRL_FACE_ADD_RIGHT     = 0x02,
    BIT_CTRL_FACE_ADD_LEFT      = 0x04,
    BIT_CTRL_FACE_ADD_DOWN      = 0x08,
    BIT_CTRL_FACE_ADD_UP        = 0x10,

    BIT_CTRL_FACE_ADD_2FACE_LR  = BIT_CTRL_FACE_ADD_LEFT|BIT_CTRL_FACE_ADD_RIGHT,
    BIT_CTRL_FACE_ADD_2FACE_UD  = BIT_CTRL_FACE_ADD_UP|BIT_CTRL_FACE_ADD_DOWN,
    BIT_CTRL_FACE_ADD_4FACE     = BIT_CTRL_FACE_ADD_2FACE_UD|BIT_CTRL_FACE_ADD_2FACE_LR,

    BIT_CTRL_FACE_ADD_FACE_MASK = 0x1F,

} kl520_bit_ctrl_face_add;
#endif

typedef enum _KL520_MOUSE_STATE_
{
    MOUSE_NONE,
    MOUSE_DOWN,
    MOUSE_MOVE,
    MOUSE_UP,

} kl520_mouse_state;

typedef struct _KL520_MOUSE_INFO_
{
    kl520_mouse_state state;
    short x;
    short y;

} kl520_mouse_info;

typedef struct _KL520_DP_POINT_
{
    u32 x;
    u32 y;
} kl520_dp_point;

typedef struct _KL520_DP_RECT_
{
    unsigned short start_x;
    unsigned short start_y;
    unsigned short end_x;
    unsigned short end_y;
} kl520_dp_rect;


typedef struct _KL520_DP_DRAW_INFO_
{
    kl520_dp_rect r1_rc;
    kl520_dp_rect n1_rc;
    kl520_dp_point r1_pt_array[LAND_MARK_POINTS];
    kl520_dp_point n1_pt_array[LAND_MARK_POINTS];
    
    float r1_lm_score;
    float n1_lm_score;
    
    s32 e2e_ret;

    u8 e2e_dist_type;
    u16 e2e_dist;

    u8 e2e_pos_type;
    kl520_dp_point  e2e_pos;

    u8 e2e_eye_type;
    u8 e2e_mask_type;


} kl520_dp_draw_info;


////////////////
/* Camera API */
////////////////
u16 kl520_api_camera_open(unsigned int cam_idx);
int kl520_api_camera_close(unsigned int cam_idx);
int kl520_api_camera_start(unsigned int cam_idx);
int kl520_api_camera_stop(unsigned int cam_idx);
int kl520_api_camera_get_id(unsigned int cam_idx);


///////////////
/* Touch API */
///////////////
/**
 * @brief After receiving the touch event evt_touch_id, the touch information can be obtained through this interface.
 */
int kl520_api_mouse_info_get(kl520_mouse_info * info);

//////////////////////
/* Face-related API */
/////////////////////
/*
 * @brief
*/

void kl520_api_face_set_add_mode(kl520_face_add_mode mode);
kl520_face_add_mode kl520_api_face_get_add_mode(void);
void kl520_api_face_set_db_add_mode(kl520_face_db_add_mode mode);
/**
 * @brief Face capture
 * The parameter represents the area that camera can display and face type
 * The remaining areas are left to the UI
 * If successful, the event 'evt_face_result_id' is returned asynchronously and the ID of the face can be obtained
 * via kl520_api_face_get_result.
*/

int kl520_api_face_add(short x, short y, short w, short h, kl520_face_add_type type);

#if ( KL520_FACE_ADD_BMP == YES )
int kl520_api_face_add_ex(short x, short y, short w, short h, u8 f_bmp);
#endif

/**
 * @brief When the application receives evt_face_result_id, it calls the API to get the result.
 * Face ID can be obtained if face record is successful.
 * If face test and verify function is used, the paramter face_id is empty.
*/
int kl520_api_face_get_result(u8* face_id);

/**
 * @brief Delete all information about user ID
 * If the parameter del_ctrl is 1, all user_id in database will be deleted.
 * Otherwise, the specified user_id will be deleted
*/
int kl520_api_face_del(u8 del_ctrl, u8 user_id);

/**
 * @brief Query the face ID is exist or not
*/
int kl520_api_face_query(u8 face_id);

/**
 * @brief Query all face ID in db are exist or not
*/
int kl520_api_face_query_all(u8* total_id_num, u8* face_id);

u8 kl520_api_face_query_first_avail_user_idx(void);
u8 kl520_api_face_query_user_idx(u8 user_id);
/**
 * @brief Set the face capture timeout time in seconds
*/
int kl520_api_face_add_set_timeout(int timeout);
/**
 * @brief Get the face capture timeout time in seconds
*/
int kl520_api_face_add_get_timeout(void);

/**
 * @brief Face recognition
 * The parameter represents the area that camera can display
 * The remaining areas are left to the UI
 * If successful, the event 'evt_face_result_id' is returned asynchronously and the ID of the face can be obtained
 * via kl520_api_face_get_result.
*/
int kl520_api_face_recognition(short x, short y, short w, short h);

/**
 * @brief Set the face recognition timeout time in seconds
*/
int kl520_api_face_recognition_set_timeout(int timeout);
/**
 * @brief Get the face recognition timeout time in seconds
*/
int kl520_api_face_recognition_get_timeout(void);


/**
 * @brief Face liveness
 * The parameter represents the area that camera can display
 * The remaining areas are left to the UI
 * If successful, the event 'evt_face_result_id' is returned asynchronously and the ID of the face can be obtained
 * via kl520_api_face_get_result.
*/
int kl520_api_face_liveness(short x, short y, short w, short h);

//snap image
int kl520_api_snap_image(short x, short y, short w, short h);

/**
 * @brief Set the face liveness timeout time in seconds
*/
int kl520_api_face_liveness_set_timeout(int timeout);
/**
 * @brief Get the face liveness timeout time in seconds
*/
int kl520_api_face_liveness_get_timeout(void);

void kl520_api_face_set_curr_face_id(u8 curr_face_id);
u8 kl520_api_face_get_curr_face_id(void);

/**
 * @brief Handle events during kl520_api_face_add_internal
*/
int kl520_api_add_wait_and_get(void);

/**
 * @brief The internal process of registering faces
*/

int kl520_api_face_add_internal(short x, short y, short w, short h, kl520_face_add_type type);

#if ( KL520_FACE_ADD_BMP == YES )
int kl520_api_face_add_internal_ex(short x, short y, short w, short h, u8 f_bmp);
#endif

/**
 * @brief Face liveness Test mode
 * Through evt_face_result_id notification, the application receives the event and calls kl520_face_get_result to get the result.
*/
int kl520_api_face_recognition_test(short x, short y, short w, short h);

/**
 * @brief Turn off Face Related Functions
*/
void kl520_api_face_close(void);


u16 kl520_api_touch_get_device_id(void);

#if ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36515 ) || ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36404 )
void kl520_api_led_register_hook(u32 cam_idx, fn_led_open fn_open, fn_led_close fn_close, fn_led_light_mode fn_mode);
#else
void kl520_api_led_register_hook(u32 cam_idx, fn_led_open fn_open, fn_led_close fn_close);
#endif
void kl520_api_light_sensor_register_hook(fn_strength_get fn_get);
void kl520_api_camera_register_hook(u32 cam_idx, fn_power_hook fn_power_on, fn_power_hook fn_power_off);
void kl520_api_touch_register_hook(fn_power_hook fn_power_on, fn_power_hook fn_power_off);
int kl520_api_touch_open(void);
int kl520_api_touch_start(void);
int kl520_api_touch_stop(void);
BOOL kl520_api_touch_is_started(void);
void kl520_api_touch_set_x_range_max(int x_range_max);
void kl520_api_touch_set_y_range_max(int y_range_max);
void kl520_api_touch_set_x_axis_inverse(int on_off);
void kl520_api_touch_set_y_axis_inverse(int on_off);


#endif
