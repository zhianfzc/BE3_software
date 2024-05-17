#ifndef __KL520_API_H__
#define __KL520_API_H__

#include "kl520_include.h"
#include "version.h"
#include "kl520_api_device_id.h"

///////////////
/* timer API */
///////////////
/**
 * @brief Timer initialization
 * @sample kl520_api_timer_init(PWMTIMER4,PWMTMR_100MSEC_PERIOD)
 */
BOOL kl520_api_timer_init(pwmtimer timer, u32 tick);

/**
 * @brief Timer tick reset
 */
void kl520_api_timer_tick_reset(pwmtimer timer);

/**
 * @brief Closing of timer
 */
 int kl520_api_timer_close(pwmtimer timer);
 
 
/**
 * @brief Timer interruption
 */
void kl520_api_tmr2_user(u32 *tick);
void kl520_api_tmr3_user(u32 *tick);
void kl520_api_tmr4_user(u32 *tick);
void kl520_api_tmr5_user(u32 *tick);
void kl520_api_tmr6_user(u32 *tick);

/**
 * @brief Get Display backlight, range 0-100
 */
int kl520_api_dp_get_backlight(void);
/**
 * @brief Set Display backlight, range 0-100
 */
int kl520_api_dp_set_backlight(int light);

/**
 * @brief Display the screen and brush the contents of LCD_BUFFER_ADDR onto the Display.
 */
int kl520_api_dp_fresh(void);

void kl520_api_dp_layout_enable(void);
void kl520_api_dp_layout_disable(void);
BOOL kl520_api_dp_layout_get(void);


void kl520_api_dp_five_face_enable(void);
void kl520_api_dp_five_face_disable(void);
BOOL kl520_api_dp_five_face_get(void);

///////////////////////
/* Basic Drawing API */
///////////////////////
int kl520_api_dp_set_pen_rgb565(unsigned short rgb565, unsigned int pen_width);
int kl520_api_dp_draw_line(u32 xs, u32 ys, u32 xe, u32 ye);
int kl520_api_dp_draw_bitmap(u32 org_x, u32 org_y, u32 width, u32 height, void *buf);


int kl520_api_dp_fill_rect(u32 org_x, u32 org_y, u32 width, u32 height);
int kl520_api_dp_draw_rect(int x, int y, int width, int height);
/////////////
/* LCD API */
/////////////
int kl520_api_dp_get_device_id(void);

////////////////////////
/* Display layout API */
////////////////////////
int kl520_api_dp_open(u32 width, u32 height);
u8 kl520_api_dp_init_get(void);

int kl520_api_dp_fresh_bg(u16 nColor, u8 nWidth);

void kl520_api_dp_close(void);

void kl520_api_dp_layout(void);

void kl520_pose_timer_stop(void);
void kl520_pose_timer_delete(void);

void kl520_api_dp_layout_pose(void);

void kl520_api_dp_layout_pose_with_customized_size_img(void);

void _kl520_api_dp_layout_lm(kl520_dp_draw_info *info, int flag);
int kl520_api_get_scpu_version(struct fw_misc_data *g_fw_misc);
int kl520_api_get_ncpu_version(struct fw_misc_data *g_fw_misc);
struct fw_misc_data *kl520_api_get_model_version(void);

u32 kl520_api_get_unique_id(void);
u16 kl520_api_memxfer_get_device_id(void);
int kl520_api_get_version_info(system_info* t_system_info);
int kl520_api_get_device_info(system_info* t_system_info);
void kl520_api_free_device_info(system_info* t_system_info);


osEventFlagsId_t kl520_api_get_event(void);

void kl520_api_face_notify(int face_status);

///////////////
/*  Measure execution time API */
///////////////
//        MEASURE_STAMP(E_MEASURE_FRAME_1_CAP_STR1)  \
//        MEASURE_STAMP(E_MEASURE_FRAME_1_CAP_STR2)  \
//        MEASURE_STAMP(E_MEASURE_FRAME_1_CAP_STR3)  \
//        MEASURE_STAMP(E_MEASURE_FRAME_1_CAP_STR4)  \

#define FOREACH_MEASURE_STAMP(MEASURE_STAMP) \
        MEASURE_STAMP(E_MEASURE_MAIN_BOOT)   \
        MEASURE_STAMP(E_MEASURE_CHECK_OTA) \
        MEASURE_STAMP(E_MEASURE_CHECK_OTA_DONE) \
        MEASURE_STAMP(E_MEASURE_OS_START)  \
        MEASURE_STAMP(E_MEASURE_TOUCH_INIT)  \
        MEASURE_STAMP(E_MEASURE_TOUCH_INIT_DONE)  \
        MEASURE_STAMP(E_MEASURE_LOAD_MODEL)  \
        MEASURE_STAMP(E_MEASURE_LOAD_MODEL_VAR_RESET)  \
        MEASURE_STAMP(E_MEASURE_LOAD_MODEL_FACE_INIT)  \
        MEASURE_STAMP(E_MEASURE_LOAD_MODEL_PROP_INIT)  \
        MEASURE_STAMP(E_MEASURE_LOAD_MODEL_SETTING_INIT)  \
        MEASURE_STAMP(E_MEASURE_LOAD_MODEL_DB_INIT)  \
        MEASURE_STAMP(E_MEASURE_LOAD_MODEL_END)   \
        MEASURE_STAMP(E_MEASURE_RGB_INIT)  \
        MEASURE_STAMP(E_MEASURE_RGB_INIT_DONE)  \
        MEASURE_STAMP(E_MEASURE_NIR_INIT)  \
        MEASURE_STAMP(E_MEASURE_NIR_INIT_DONE)  \
        MEASURE_STAMP(E_MEASURE_RGB_STR)  \
        MEASURE_STAMP(E_MEASURE_RGB_END)  \
        MEASURE_STAMP(E_MEASURE_NIR_STR)  \
        MEASURE_STAMP(E_MEASURE_NIR_END)  \
        MEASURE_STAMP(E_MEASURE_SYS_READY)  \
        MEASURE_STAMP(E_MEASURE_VERIFY_MSG)  \
        MEASURE_STAMP(E_MEASURE_THR_TASKS_INIT)  \
        MEASURE_STAMP(E_MEASURE_THR_TASKS_INIT_RDY)  \
        MEASURE_STAMP(E_MEASURE_THR_CONSOLE_RDY)  \
        MEASURE_STAMP(E_MEASURE_THR_OTA_RDY)  \
        MEASURE_STAMP(E_MEASURE_THR_CAM_RDY)  \
        MEASURE_STAMP(E_MEASURE_BACKLIGHT_ON)  \
        MEASURE_STAMP(E_MEASURE_API_FACE_REC_STR)  \
        MEASURE_STAMP(E_MEASURE_API_RUN_STAT_CHK)  \
        MEASURE_STAMP(E_MEASURE_THR_FDFR_INIT_RDY)  \
        MEASURE_STAMP(E_MEASURE_THR_FDFR_OPEN_RDY)  \
        MEASURE_STAMP(E_MEASURE_THR_FDFR_REC_STR)  \
        MEASURE_STAMP(E_MEASURE_NIR_ISR_01)  \
        MEASURE_STAMP(E_MEASURE_NIR_ISR_02)  \
        MEASURE_STAMP(E_MEASURE_NIR_ISR_03)  \
        MEASURE_STAMP(E_MEASURE_NIR_ISR_04)  \
        MEASURE_STAMP(E_MEASURE_NIR_ISR_05)  \
        MEASURE_STAMP(E_MEASURE_NIR_ISR_06)  \
        MEASURE_STAMP(E_MEASURE_NIR_ISR_07)  \
        MEASURE_STAMP(E_MEASURE_NIR_ISR_08)  \
        MEASURE_STAMP(E_MEASURE_NIR_ISR_09)  \
        MEASURE_STAMP(E_MEASURE_NIR_ISR_10)  \
        MEASURE_STAMP(E_MEASURE_RGB_ISR_01)  \
        MEASURE_STAMP(E_MEASURE_RGB_ISR_02)  \
        MEASURE_STAMP(E_MEASURE_RGB_ISR_03)  \
        MEASURE_STAMP(E_MEASURE_RGB_ISR_04)  \
        MEASURE_STAMP(E_MEASURE_RGB_ISR_05)  \
        MEASURE_STAMP(E_MEASURE_RGB_ISR_06)  \
        MEASURE_STAMP(E_MEASURE_RGB_ISR_07)  \
        MEASURE_STAMP(E_MEASURE_RGB_ISR_08)  \
        MEASURE_STAMP(E_MEASURE_RGB_ISR_09)  \
        MEASURE_STAMP(E_MEASURE_RGB_ISR_10)  \
        MEASURE_STAMP(E_MEASURE_FRAME_1_FACE_STR)  \
        MEASURE_STAMP(E_MEASURE_FRAME_1_FACE_END)  \
        MEASURE_STAMP(E_MEASURE_FRAME_2_FACE_STR)  \
        MEASURE_STAMP(E_MEASURE_FRAME_2_FACE_END)  \
        MEASURE_STAMP(E_MEASURE_FRAME_3_FACE_STR)  \
        MEASURE_STAMP(E_MEASURE_FRAME_3_FACE_END)  \
        MEASURE_STAMP(E_MEASURE_FRAME_4_FACE_STR)  \
        MEASURE_STAMP(E_MEASURE_FRAME_4_FACE_END)  \
        MEASURE_STAMP(E_MEASURE_FRAME_5_FACE_STR)  \
        MEASURE_STAMP(E_MEASURE_FRAME_5_FACE_END)  \
        MEASURE_STAMP(E_MEASURE_FACE_PREEXEC_END)  \
        MEASURE_STAMP(E_MEASURE_FACE_DB_COMP_STR)  \
        MEASURE_STAMP(E_MEASURE_FACE_DB_COMP_END)  \
        MEASURE_STAMP(E_MEASURE_EVENT_SEND_OK)  \
        MEASURE_STAMP(E_MEASURE_FACE_FACE_OK)  \
        MEASURE_STAMP(E_MEASURE_FACE_REC_END)  \
        MEASURE_STAMP(E_MEASURE_FACE_DB_FAIL)  \
        MEASURE_STAMP(E_MEASURE_FACE_CLOSE_STR)  \
        MEASURE_STAMP(E_MEASURE_FACE_CLOSE_END)  \
        MEASURE_STAMP(E_MEASURE_FACTORY_STR)  \
        MEASURE_STAMP(E_MEASURE_FACTORY_END)  \
        MEASURE_STAMP(E_MEASURE_LED_STR)  \
        MEASURE_STAMP(E_MEASURE_LED_END)  \
        MEASURE_STAMP(E_MEASURE_NIR_FD_STR)  \
        MEASURE_STAMP(E_MEASURE_NIR_FD_END)  \
        MEASURE_STAMP(E_MEASURE_NIR_LM_STR)  \
        MEASURE_STAMP(E_MEASURE_NIR_LM_END)  \
        MEASURE_STAMP(E_MEASURE_NIR_LM_S_STR)  \
        MEASURE_STAMP(E_MEASURE_NIR_LM_S_END)  \
        MEASURE_STAMP(E_MEASURE_NIR_QUALITY_STR)  \
        MEASURE_STAMP(E_MEASURE_NIR_QUALITY_END)  \
        MEASURE_STAMP(E_MEASURE_NIR_POSE_STR)  \
        MEASURE_STAMP(E_MEASURE_NIR_POSE_END)  \
        MEASURE_STAMP(E_MEASURE_RGB_FD_STR)  \
        MEASURE_STAMP(E_MEASURE_RGB_FD_END)  \
        MEASURE_STAMP(E_MEASURE_RGB_LM_STR)  \
        MEASURE_STAMP(E_MEASURE_RGB_LM_END)  \
        MEASURE_STAMP(E_MEASURE_RGB_LM_S_STR)  \
        MEASURE_STAMP(E_MEASURE_RGB_LM_S_END)  \
        MEASURE_STAMP(E_MEASURE_NIR_LV_STR)  \
        MEASURE_STAMP(E_MEASURE_NIR_LV_END)  \
        MEASURE_STAMP(E_MEASURE_NIR_HSN_STR)  \
        MEASURE_STAMP(E_MEASURE_NIR_HSN_END)  \
        MEASURE_STAMP(E_MEASURE_FUSE_STR)  \
        MEASURE_STAMP(E_MEASURE_FUSE_END)  \
        MEASURE_STAMP(E_MEASURE_NIR_FR_STR)  \
        MEASURE_STAMP(E_MEASURE_NIR_FR_END)  \
        MEASURE_STAMP(E_MEASURE_RESERVED0)  \
        MEASURE_STAMP(E_MEASURE_RESERVED1)  \
        MEASURE_STAMP(E_MEASURE_RESERVED2)  \
        MEASURE_STAMP(E_MEASURE_NUM)  \

#if (MEASURE_RECOGNITION == YES)
#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum _KL520_MEASURE_STAMP_E {
    FOREACH_MEASURE_STAMP(GENERATE_ENUM)
}measure_stamp_e;

static const char *MEASURE_STAMP_STRING[] = {
    FOREACH_MEASURE_STAMP(GENERATE_STRING)
};
#endif

#define NUM_OF_MEASURE (E_MEASURE_NUM)

void kl520_measure_init(void);
#if (MEASURE_RECOGNITION == YES)
void kl520_measure_stamp(measure_stamp_e eStamp);
#else 
#define kl520_measure_stamp(a) //
#endif
void kl520_measure_info(void);

/**
 * @brief set user log level
 * \param[in]       cpu_id  0 : scpu, 1 : ncpu
 * \param[in]       level   refer to dbg.h for definition of levels
 * \return          NULL
*/
void kl520_api_log_set_user_level(u32 cpu_id, u8 level);

void kl520_api_set_rgb_led_level(BOOL reset, s32 level);


int kl520_engineering_calibration(u8 type, u32 *args);

void kl520_api_ota_switch_SCPU(void);
void kl520_api_ota_switch_NCPU(void);
int kl520_api_model_count(void);
int kl520_api_model_version(uint8_t idx);


///////////////
/*  Customer Info API */
///////////////
typedef struct _KL520_CUSTOMER_INFO_
{
    u8  nCusInfo0;
    u16 nCusInfo1;
    u32 nCusInfo2;
    u64 nCusInfo3;
    u8 nReleaseKey[16];
    u8 eye_mode_en;
    u8 verify_threshold;
    u8 live_threshold;
    u16 exp_time;
    u8 gain1;
    u8 gain2;
    u8 user_rotate_180;
    u8 user_db_offset;
} kl520_customer_info;

int kl520_api_customer_get(kl520_customer_info *cusinfo);
int kl520_api_customer_write(kl520_customer_info *cusinfo);
BOOL kl520_api_customer_chk_key_exist(u8* ptr, u8 nLen);
int kl520_api_customer_clr(void);
u32 kl520_api_customer_info(void);

#endif
