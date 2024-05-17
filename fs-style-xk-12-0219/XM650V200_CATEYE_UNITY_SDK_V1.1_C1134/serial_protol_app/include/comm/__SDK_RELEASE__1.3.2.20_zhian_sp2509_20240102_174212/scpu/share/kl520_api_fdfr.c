#include "kl520_api_fdfr.h"
#include "kdp520_dma.h"
#include "io.h"
#include "pinmux.h"
#include "framework/init.h"
#include "framework/v2k_image.h"
#include "framework/framework_driver.h"
#include "framework/event.h"
#include "media/display/video_renderer.h"
#include "kdp520_i2c.h"
#include "kdp520_pwm_timer.h"
#include "kdp_camera.h"
#include "kdp_memory.h"
#include "kdp_fb_mgr.h"
#include "kdp_memxfer.h"
#include "kdp_model.h"
#include "kdp_e2e.h"
#include "kdp_e2e_settings.h"
#include "kdp_e2e_camera.h"
#include "kdp_e2e_util.h"
#include "kdp_e2e_ctrl.h"
#include "kdp_e2e_r1n1.h"
#include "kdp_e2e_n1r1.h"
#include "kdp_e2e_n1_only.h"
#include "kdp_e2e_r1_only.h"
#include "kdp_e2e_face.h"
#include "kdp_e2e_prop.h"
#include "kdp_e2e_db.h"
#include "kl520_include.h"
#include "kl520_api.h"
#include "kl520_sys.h" // engineering mode
#include "kl520_api_fdfr.h"
#include "kl520_api_snapshot.h"
#include "kl520_api_extra_fmap.h" // jim
#include "kl520_api_sim.h"
#include "kl520_api_device_id.h"
#include "user_io.h" // will be removed in next version
#include "board_ddr_table.h"

#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP )
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK )
#include "kdp_comm_app.h"
#endif
#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP_USR )
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK )
#include "user_comm_app.h"
#endif
#endif

#if (CFG_E2E_REC_NOTE == YES)
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF )
#undef CFG_E2E_REC_NOTE
#define CFG_E2E_REC_NOTE    (NO)
#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP )
#include "kdp_comm_app.h"
#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF_USR )
#include "user_app.h"
#endif
#endif

//#define LIVENSS_ON_FACE_ADD_TOO
#define FACE_REG_CHECK_EYE
#define FACE_REG_CHECK_POSITION             (CFG_E2E_CHECK_POSITION)
#define SUPPORT_5_FACES
#define FLAGS_API_FDFR_OPEN_EVT             0x00000100

#define FLAGS_FDFR_ACK                      0x00010000

#define USE_SCORE_JUDGEMENT                 YES
//#define CYCLE_ADD_ENABLE
#ifdef CYCLE_ADD_ENABLE
//#define AUTO_DELETE_USER_ENABLE
#else
#define COMPARE_BEFORE_ADD_DISABLE  //zcy note close the tag can re-enroll
#endif

#define SKIP_SOME_BAD_POSE_EVENT

#define FDFR_THREAD_PRIORITY_DYNAMIC_ADJUSTMENT
#define FDFR_KEEP_MS_WHEN_FACE_CLOSED

#define kl520_api_face_get_reserve_db_num() (m_face_db_add_mode == FACE_ADD_MODE_IN_DB? 0 : 1)

#define FDFR_BREAK_NUM              (100)


#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) && (CFG_FMAP_AP_CTRL_TYPE >= EX_FM_UART_AP_CTRL_MAIN_DB)

#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF ) || ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF_USR )
#define HOST_DEV_COM_PROTO_TYPE 1
#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP ) || ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP_USR )
#define HOST_DEV_COM_PROTO_TYPE 2
#else 
#define HOST_DEV_COM_PROTO_TYPE 3
#endif

#else
#define HOST_DEV_COM_PROTO_TYPE 0
#endif

u8 rgb_sensor_index = CFR_CAM_RGB;
u8 nir_sensor_index = CFR_CAM_NIR;
u8 sensor_0_mirror = CFG_SENSOR_0_FMT_MIRROR;
u8 sensor_0_flip = CFG_SENSOR_0_FMT_FLIP;
u8 sensor_1_mirror = CFG_SENSOR_1_FMT_MIRROR;
u8 sensor_1_flip = CFG_SENSOR_1_FMT_FLIP;

const float reg_normal_score0 = NORMAL_POSE_SCORE0;
const float reg_normal_score1 = NORMAL_POSE_SCORE1;
const s8 reg_normal_pitch0 = NORMAL_POSE_UD0;
const s8 reg_normal_pitch1 = (s8)(NORMAL_POSE_UD1);
const s8 reg_normal_yaw = NORMAL_POSE_YAW;
const s8 reg_normal_roll = NORMAL_POSE_ROLL;

typedef struct sample_db_data_struct {
    u16 db_user_id;
} sample_db_data;
//static struct sample_db_data_struct INITDATA m_sample_db[MAX_USER] = { 0 };

static kl520_face_add_mode m_face_add_mode = FACE_ADD_MODE_DEFAULT;
static kl520_face_db_add_mode m_face_db_add_mode = FACE_ADD_MODE_IN_DB;
u8 m_curr_user_id = 0xFF;
static u32 m_fdfr_tick = 0;
static u32 m_db_comp_fail_cnt = 0;
static BOOL m_face_appear = FALSE;
uint8_t m_curr_face_id = 0xFF;
u32 nir_normal_addr = 0;
static u8 _enroll_customize_uid = 0xff;
static u8 _enroll_overwrite_flag = 0;
static osMutexId_t mutex_face_close = NULL;

BOOL m_fdr_model_inited = FALSE;
kdp_e2e_flow_mode m_flow_mode = FLOW_MODE_NORMAL;
kdp_e2e_face_mode m_face_mode = FACE_MODE_NONE;
BOOL g_bRecognitionMandatoryFlag = FALSE;

#ifdef DEV_TEST_VERSION
int _face_recog_count = 0;
int _face_succ_count = 0;
#endif

osMutexId_t _mutex_fdfr = NULL;
int         _fdr_opened = 0;

struct api_fdfr_context {
    enum kl520_fdfr_status_code_e state;
    osThreadId_t tid;
    u8           reg_idx;
};

int m_face_add_timeout = KL520_DEFAULT_ADD_TIMEOUT_S;
int m_face_recognition_timeout = KL520_DEFAULT_REC_TIMEOUT_S;
int m_face_liveness_timeout = KL520_DEFAULT_LIVENESS_TIMEOUT_S;

osThreadId_t tid_fdfr_update_fr = NULL;

//u8 total_db_num;

kl520_dp_draw_info dp_draw_info = { 0 };

struct api_fdfr_context m_api_fdfr_ctx = { FDFR_STATUS_IDLE, NULL, 0xff };
static osPriority_t m_old_thread_priority = osPriorityNormal;
osTimerId_t kl520_fdfr_drawing_timer = 0;
int kl520_fdfr_drawing_timer_flag = KL520_FACE_FAIL;

extern void kl520_api_dp_layout_enable(void);
extern void kl520_api_dp_layout_disable(void);
//extern BOOL kl520_api_dp_layout_get(void);

static void api_fdfr_thread(void *argument);
//osThreadId_t tid_app_aec_r1 = NULL;
//osThreadId_t tid_app_aec_n1 = NULL;

//extern void kl520_api_aec_r1_element(void *argument);
//extern void kl520_api_aec_n1_element(void *argument);

//int kl520_api_fdfr_facemode_get()
//{
//    return m_face_mode;
//}

s32 vote_score = 0;

#if ( KL520_FACE_ADD_BMP == YES )
u8 face_add_bitmap = 0x1e;
u8 face_succ_index;
#endif

bool b_en_aec_only = false;
int  face_non_move_cnt = -1;
uint8_t face_attack_cnt = 0;

uint8_t  bad_pose_cnt = 0;

static osEventFlagsId_t _fdr_event_id = NULL;
__WEAK extern osEventFlagsId_t get_user_com_event_id(void) {return NULL;}

BOOL ui_fsm_dp_layout_en = FALSE;
void kl520_api_ui_fsm_dp_layout_enable(void)
{
    ui_fsm_dp_layout_en = TRUE;
}

void kl520_api_ui_fsm_dp_layout_disable(void)
{
    ui_fsm_dp_layout_en = FALSE;
}

u8 kl520_is_fdfr_abort(void)
{
    return kdp_is_abort_flag();
}

void kl520_set_fdfr_abort(u8 flag)
{
    kdp_set_abort_flag(flag);
}

u8 kl520_fdfr_opened(void) 
{
    return _fdr_opened;
}

u8 is_enroll_customize_uid(void)
{
    if(_enroll_customize_uid == 0xff) return 0;
    else return 1;
}

void set_enroll_customize_uid(u8 uid)
{
    _enroll_customize_uid = uid;
}

u8 get_enroll_customize_uid(void)
{
    return _enroll_customize_uid;
}

void set_enroll_overwrite_flag(u8 flag)
{
    _enroll_overwrite_flag = flag;
}

u8 get_enroll_overwrite_flag(void)
{
    return _enroll_overwrite_flag;
}

#ifdef CUSTOMIZE_DB_OFFSET
static u8 user_db_offset = CUSTOMIZE_DB_OFFSET;

u8 get_user_db_offset(void)
{
    return user_db_offset;
}

void load_user_db_offset(void)
{
    kl520_customer_info Cusinfo;
    kl520_api_customer_get(&Cusinfo);
    
    user_db_offset = Cusinfo.user_db_offset;
#if (DB_OFFSET_CMD == YES)
#else
    if (0x80 == Cusinfo.user_db_offset)
        user_db_offset = 0x80;
    else
        user_db_offset = CUSTOMIZE_DB_OFFSET;
#endif
    
    dbg_msg_console("[%s] user_db_offset:%x", __func__, user_db_offset);
}

void update_user_db_offset(u8 offset)
{
    kl520_customer_info Cusinfo;
    kl520_api_customer_get(&Cusinfo);
    
    if (offset != Cusinfo.user_db_offset)
    {
        user_db_offset = offset;
        Cusinfo.user_db_offset = user_db_offset;
        kl520_api_customer_write(&Cusinfo);
    }
    dbg_msg_console("Update user_db_offset to 0x%02x", user_db_offset);
}

void reset_user_db_offset(void)
{
    update_user_db_offset(CUSTOMIZE_DB_OFFSET);
}
#endif

void kl520_customer_info_init(void)
{
    kl520_customer_info Cusinfo;
    kl520_api_customer_get(&Cusinfo);
    
#ifdef CUSTOMIZE_DB_OFFSET
    user_db_offset = Cusinfo.user_db_offset;
#if (DB_OFFSET_CMD == YES)
#else
    if (0x80 == Cusinfo.user_db_offset)
        user_db_offset = 0x80;
    else
        user_db_offset = CUSTOMIZE_DB_OFFSET;
#endif
    dbg_msg_console("[%s] user_db_offset:%x", __func__, user_db_offset);
#endif
    
    // verify_threshold
    if (Cusinfo.verify_threshold > 4)
    {
        if (Cusinfo.verify_threshold > 4)
            Cusinfo.verify_threshold = DB_DEFAULT_FR_THRESHOLD_LEVEL;

        kl520_api_customer_write(&Cusinfo);
    }
    kdp_e2e_set_fr_threshold_level(Cusinfo.verify_threshold);
    dbg_msg_console("Fr th: %d", Cusinfo.verify_threshold);

    
    // live_threshold
    if ((Cusinfo.live_threshold < 50) || (Cusinfo.live_threshold > 95))
    {
        Cusinfo.live_threshold = DB_DEFAULT_RGB_TO_NIR_RATIO;

        kl520_api_customer_write(&Cusinfo);
    }
    kdp_set_rgb_to_nir_ratio(Cusinfo.live_threshold);
    dbg_msg_console("lv th: %d", Cusinfo.live_threshold);
    
}

u8 _kl520_app_calc_db_uid(u8 db_idx)
{
#ifdef CUSTOMIZE_DB_OFFSET
#ifdef CUSTOMIZE_OFFSET_FUNC
    //call customer's specific map func
#else
    return db_idx + user_db_offset;
#endif
#else
    return db_idx + KDP_DB_DEFAULT_OFFSET;
#endif
}

u8  kdp_app_db_get_db_idx(u8 user_id)
{
#ifdef CUSTOMIZE_DB_OFFSET
#ifdef CUSTOMIZE_OFFSET_FUNC
    //call customer's specific map func
#else
    return user_id - user_db_offset;
#endif
#else
    return user_id - KDP_DB_DEFAULT_OFFSET;
#endif
}
            
BOOL kl520_api_ui_fsm_dp_layout_get(void)
{
    return ui_fsm_dp_layout_en;
}

extern osThreadId_t tid_rgb_led_gradually;
static void kl520_api_face_recognition_variable_reset(void)
{
#if (CFG_LED_CTRL_ENHANCE == 0)
    nir_led_close();
#endif

#if (CFG_AI_TYPE == AI_TYPE_R1N1 || CFG_AI_TYPE == AI_TYPE_R1)
    if(tid_rgb_led_gradually)
    {
        osThreadJoin(tid_rgb_led_gradually);
        tid_rgb_led_gradually = 0;
    }
#endif
    
    rgb_led_close();
#if (CFG_LED_CTRL_ENHANCE == 1)
    kdp_e2e_nir_led_flag_off();
#endif
    kdp_e2e_reset();
    vote_score = 0;
}

void kl520_api_fdfr_set_flow_mode(BOOL sim_enabled)
{
    if (sim_enabled)
        kdp_e2e_prop_set2(flow_mode, FLOW_MODE_SIM_MODELS);
    else
        kdp_e2e_prop_set2(flow_mode, FLOW_MODE_NORMAL);
}

void kl520_api_fdfr_model_init(void)
{
    if (!m_fdr_model_inited) {
        
        kl520_customer_info_init();
        
        kdp_model_init_addr(    MODEL_MGR_FLASH_ADDR_HEAD,
                                MODEL_MGR_FLASH_ADDR_MODEL_COUNT,
                                MODEL_MGR_FLASH_ADDR_MODEL_INFO,
                                MODEL_MGR_FLASH_ADDR_MODEL_DDR_END_ADDR,
                                MODEL_MGR_FLASH_ADDR_MODEL_POOL);

        kl520_measure_stamp(E_MEASURE_LOAD_MODEL);
        kl520_api_face_recognition_variable_reset();
        kl520_measure_stamp(E_MEASURE_LOAD_MODEL_VAR_RESET);
        kdp_e2e_face_init();
        kl520_measure_stamp(E_MEASURE_LOAD_MODEL_FACE_INIT);
        kdp_e2e_prop_init();
        kl520_measure_stamp(E_MEASURE_LOAD_MODEL_PROP_INIT);
        kdp_e2e_prop_set2(dst_win_width, DISPLAY_RGB_WIDTH);
        kdp_e2e_prop_set2(dst_win_height, DISPLAY_RGB_HEIGHT);
        kdp_e2e_prop_set2(r1_offset_x, DISPLAY_RGB_X_OFFSET);
        kdp_e2e_prop_set2(r1_offset_y, DISPLAY_RGB_Y_OFFSET);
        kdp_e2e_prop_set2(bbox_tolerance_lines, LCD_FDR_OVERLINE);
        kdp_e2e_prop_set2(n1_offset_x, DISPLAY_NIR_X_OFFSET);
        kdp_e2e_prop_set2(n1_offset_y, DISPLAY_NIR_Y_OFFSET);
        kdp_e2e_prop_set2(n1_dst_win_width, DISPLAY_NIR_WIDTH);
        kdp_e2e_prop_set2(n1_dst_win_height, DISPLAY_NIR_HEIGHT);
        kdp_e2e_prop_set2(invalid_score_min, KL520_FACE_SCORE_MIN);
        kdp_e2e_prop_set2(invalid_score_max, KL520_FACE_SCORE_MAX);
        kdp_e2e_prop_set2(rgb_led_enhance_cnt, CFG_E2E_RGB_LED_STRONG_ENHANCE_COUNT);
        kdp_e2e_prop_set2(rgb_led_enhance_step, CFG_E2E_RGB_LED_STRONG_ENHANCE_STRONG_STEP);
        kdp_e2e_prop_set2(rgb_led_enhance_min, CFG_E2E_RGB_LED_STRONG_ENHANCE_STRONG_MIN);
        kdp_e2e_prop_set2(rgb_led_enhance_max, CFG_E2E_RGB_LED_STRONG_ENHANCE_STRONG_MAX);
        
        kdp_e2e_settings_init();
        kl520_measure_stamp(E_MEASURE_LOAD_MODEL_SETTING_INIT);
     
        #if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE
        kdp_e2e_db_init( 1 );
        #else
        kdp_e2e_db_init( 0 );
        #endif

//        for (int i = 0; i < ARRAY_SIZE(m_sample_db); ++i) {
//            m_sample_db[i].db_user_id = kl520_api_get_start_user_id() + i;
//        }
        kl520_measure_stamp(E_MEASURE_LOAD_MODEL_DB_INIT);

        m_fdr_model_inited = TRUE;
        kl520_measure_stamp(E_MEASURE_LOAD_MODEL_END);

        /*user setting*/
        //kdp_e2e_prop *prop = kdp_e2e_prop_get_inst();
        //kdp_e2e_prop_set_manual_value(prop, nir_lv_threshold, 1.0);
        //*FD,DB

    }
}
 
s32 kl520_api_face_preexecute_stage1(void)
{
    s32 ret = 0;

#if (CFG_AI_TYPE == AI_TYPE_R1)
    kl520_measure_stamp(E_MEASURE_RGB_INIT);
    ret = kl520_api_cam_disp_ctrl(API_CTRL_CAM_OPEN, CAMERA_DEVICE_RGB_IDX, PERMANENT_NULL);
    kl520_measure_stamp(E_MEASURE_RGB_INIT_DONE);
    if(ret != 0) return ret;
#elif (CFG_AI_TYPE == AI_TYPE_R1N1)
    kl520_measure_stamp(E_MEASURE_RGB_INIT);
    ret = kl520_api_cam_disp_ctrl(API_CTRL_CAM_OPEN, CAMERA_DEVICE_RGB_IDX, PERMANENT_NULL);
    kl520_measure_stamp(E_MEASURE_RGB_INIT_DONE);
    if(ret != 0) return ret;
    osDelay(1);
    kl520_measure_stamp(E_MEASURE_NIR_INIT);
    ret = kl520_api_cam_disp_ctrl(API_CTRL_CAM_OPEN, CAMERA_DEVICE_NIR_IDX, PERMANENT_NULL);
    kl520_measure_stamp(E_MEASURE_NIR_INIT_DONE);
    if(ret != 0) return ret;
#elif (CFG_AI_TYPE == AI_TYPE_N1)
    kl520_measure_stamp(E_MEASURE_NIR_INIT);
    ret = kl520_api_cam_disp_ctrl(API_CTRL_CAM_OPEN, CAMERA_DEVICE_NIR_IDX, PERMANENT_NULL);
    kl520_measure_stamp(E_MEASURE_NIR_INIT_DONE);
    if(ret != 0) return ret;
#elif (CFG_AI_TYPE == AI_TYPE_N1R1)

#if (CFG_SENSOR_0_TYPE >= 0)
#if (CFG_PALM_PRINT_MODE == 1)
    if ((!kdp_is_palm_mode() && (MIPI_CAM_NIR == 0)) || \
        (kdp_is_palm_mode() && (MIPI_CAM_RGB == 0)))
#endif
    {
        kl520_measure_stamp((MIPI_CAM_NIR==0)?E_MEASURE_NIR_INIT:E_MEASURE_RGB_INIT);
        ret = kl520_api_cam_disp_ctrl(API_CTRL_CAM_OPEN, 0, PERMANENT_NULL);
        kl520_measure_stamp((MIPI_CAM_NIR==0)?E_MEASURE_NIR_INIT_DONE:E_MEASURE_RGB_INIT_DONE);
        if(ret != 0) return ret;
    }

#endif
      

    osDelay(5);

#if (CFG_SENSOR_1_TYPE >= 0)
#if (CFG_PALM_PRINT_MODE == 1)
    if ((!kdp_is_palm_mode() && (MIPI_CAM_NIR == 1)) || \
        (kdp_is_palm_mode() && (MIPI_CAM_RGB == 1)))
#endif
    {
        kl520_measure_stamp((MIPI_CAM_RGB==1)?E_MEASURE_RGB_INIT:E_MEASURE_NIR_INIT);
        ret = kl520_api_cam_disp_ctrl(API_CTRL_CAM_OPEN, 1, PERMANENT_NULL);
        kl520_measure_stamp((MIPI_CAM_RGB==1)?E_MEASURE_RGB_INIT_DONE:E_MEASURE_NIR_INIT_DONE);
        if(ret != 0) return ret;
    }
#endif
    
#endif

    return ret;
}

s32 kl520_api_face_preexecute_stage2(void)
{
    s32 ret = 0;

#if (CFG_AI_TYPE == AI_TYPE_R1)
    kl520_measure_stamp(E_MEASURE_RGB_STR);
    ret = kl520_api_cam_disp_ctrl(API_CTRL_CAM_STAR, CAMERA_DEVICE_RGB_IDX, PERMANENT_NULL);
    if(ret != 0) return ret;
    kl520_measure_stamp(E_MEASURE_RGB_END);
#elif (CFG_AI_TYPE == AI_TYPE_R1N1)
    kl520_measure_stamp(E_MEASURE_RGB_STR);
    ret = kl520_api_cam_disp_ctrl(API_CTRL_CAM_STAR, CAMERA_DEVICE_RGB_IDX, PERMANENT_NULL);
    if(ret != 0) return ret;
    kl520_measure_stamp(E_MEASURE_RGB_END);
    kl520_measure_stamp(E_MEASURE_NIR_STR);
    ret = kl520_api_cam_disp_ctrl(API_CTRL_CAM_STAR, CAMERA_DEVICE_NIR_IDX, PERMANENT_NULL);
    if(ret != 0) return ret;
    kl520_measure_stamp(E_MEASURE_NIR_END);
#elif (CFG_AI_TYPE == AI_TYPE_N1)
    kl520_measure_stamp(E_MEASURE_NIR_STR);
    ret = kl520_api_cam_disp_ctrl(API_CTRL_CAM_STAR, CAMERA_DEVICE_NIR_IDX, PERMANENT_NULL);
    if(ret != 0) return ret;
    kl520_measure_stamp(E_MEASURE_NIR_END);
#elif (CFG_AI_TYPE == AI_TYPE_N1R1)

#if (CFG_SENSOR_0_TYPE >= 0)
#if (CFG_PALM_PRINT_MODE == 1)
    if ((!kdp_is_palm_mode() && (MIPI_CAM_NIR == 0)) || \
        (kdp_is_palm_mode() && (MIPI_CAM_RGB == 0)))
#endif
    {
        kl520_measure_stamp((MIPI_CAM_NIR==0)?E_MEASURE_NIR_STR:E_MEASURE_RGB_INIT);
        ret = kl520_api_cam_disp_ctrl(API_CTRL_CAM_STAR, 0, PERMANENT_NULL);
        if(ret != 0) return ret;
        kl520_measure_stamp((MIPI_CAM_NIR==0)?E_MEASURE_NIR_END:E_MEASURE_RGB_END);
    }
#endif
#if (CFG_SENSOR_1_TYPE >= 0)
#if (CFG_PALM_PRINT_MODE == 1)
    if ((!kdp_is_palm_mode() && (MIPI_CAM_NIR == 1)) || \
        (kdp_is_palm_mode() && (MIPI_CAM_RGB == 1)))
#endif
    {
        kl520_measure_stamp((MIPI_CAM_RGB==1)?E_MEASURE_RGB_STR:E_MEASURE_NIR_STR);
        ret = kl520_api_cam_disp_ctrl(API_CTRL_CAM_STAR, 1, PERMANENT_NULL);
        if(ret != 0) return ret;
        kl520_measure_stamp((MIPI_CAM_RGB==1)?E_MEASURE_RGB_END:E_MEASURE_NIR_END);
    }
#endif
    
#endif

    kl520_measure_stamp(E_MEASURE_FACE_PREEXEC_END);

    return ret;
}

s32 kl520_api_face_preexecute_stage3(void)
{
    s32 ret = E2E_OK;
    
    return ret;
}

s32 kl520_api_face_preexecute_colse(void)
{
    s32 ret = 0;

#if (CFG_AI_TYPE == AI_TYPE_R1)

#elif (CFG_AI_TYPE == AI_TYPE_R1N1)

#elif (CFG_AI_TYPE == AI_TYPE_N1)
  if(m_face_mode == FACE_MODE_NONE){
        m_face_mode = FACE_MODE_RECOGNITION;    //temp 
        kl520_api_face_close();
  }
#elif (CFG_AI_TYPE == AI_TYPE_N1R1)

#endif
 
    return ret;
}

#if (CFG_E2E_NIR_TWO_STAGE_LIGHT == YES)
void kl520_api_nir_reserve(int cam_idx, u32 addr)
{
    kdp_e2e_face_variables *vars = kdp_e2e_get_face_variables();
    
    if (nir_normal_addr == 0)
        nir_normal_addr = kdp_ddr_reserve(KDP_DDR_TEST_NIR_IMG_SIZE);

    if( vars->nir_led_status == IR_LIGHT )
    {
        kl520_api_img_reserv_dma(nir_normal_addr, addr, KDP_DDR_TEST_NIR_IMG_SIZE);       
        kdp_e2e_set_img_mem_addr(MIPI_CAM_RGB, nir_normal_addr);
    }
}
#endif


s32 kl520_api_face_preempt_init(void)
{
#define NUM_REPEAT_CAM_INIT (100)
    s32 ret = 0;
    u8 repeat = 0;
    while(1)
    {
        ret = kl520_api_face_preexecute_stage1();

        if (0 == ret)
        {
            ret = kl520_api_face_preexecute_stage2();
        }

        if(ret == 0 || repeat > NUM_REPEAT_CAM_INIT){
            dbg_msg_engineering("[%s], ret =%d, repeat =%d",__func__, ret, repeat);
            break;
        }
        else
            repeat++;

        osDelay(2);
    }

    return ret;
}

s32 _kl520_api_face_preexecute(short x, short y, short w, short h)
{
    s32 ret = 0;

    if(kdp_e2e_prop_get2(flow_mode) < FLOW_MODE_SIM_VIRTUAL)
        ret = kl520_api_face_preexecute_stage1();

    if (0 == ret)
    {
        if(kdp_e2e_prop_get2(flow_mode) < FLOW_MODE_SIM_VIRTUAL)
            ret = kl520_api_face_preexecute_stage2();

        if (0 == ret)
        {
//            ret = kl520_api_dp_open((u32)w, (u32)h);
            kl520_api_disp_resolution_set(w, h);
//            ret = kl520_api_disp_open_chk();
            ret = kl520_api_cam_disp_ctrl(API_CTRL_DISP_OPEN, NULL, PERMANENT_NULL);

            if(ret != 0)
            {
                return 0;
            }

            kl520_api_dp_layout_enable();
            kl520_api_ui_fsm_dp_layout_enable();
        }
    }
#if CFG_ONE_SHOT_MODE == YES
    if(kdp_e2e_get_dual_cam_state() == DUAL_IDENTICAL_CAM_CLOSED)
        kdp_e2e_set_dual_cam_state(DUAL_IDENTICAL_CAM_INITED);
#endif
    return ret;
}
#if (KL520_QUICK_BOOT == NO)
osEventFlagsId_t evt_face_op_ack_id = NULL;
static osEventFlagsId_t _kl520_api_fdfr_get_ack_event(void)
{
    if (NULL == evt_face_op_ack_id)
        evt_face_op_ack_id = create_event();

    return evt_face_op_ack_id;
}
#endif

static BOOL _kl520_api_fdfr_try_create_thread(void)
{
    if(kl520_is_fdfr_abort() != 0) { //abort is ongoing
        return false;
    } else {
        kl520_api_fdfr_start();
        return true;
    }
}

void kl520_api_fdfr_terminate_thread(void)
{
    kl520_api_fdfr_stop();
}

void kl520_api_fdfr_init_thrd(void)
{
    if(_mutex_fdfr) return; //already inited.

    _mutex_fdfr = osMutexNew(NULL);

    if (NULL == m_api_fdfr_ctx.tid) {
        osThreadAttr_t attr = {
            .stack_size = 2560,
            .attr_bits = osThreadJoinable
        };

    #ifdef FDFR_THREAD_PRIORITY_DYNAMIC_ADJUSTMENT
        attr.priority = osPriorityNormal;
    #else
        attr.priority = osPriorityNormal2;
    #endif

        osEventFlagsClear(kl520_api_get_event(), KL520_APP_FLAG_FDFR);

        m_api_fdfr_ctx.tid = osThreadNew(api_fdfr_thread, (void*)&m_api_fdfr_ctx, &attr);
        if (m_api_fdfr_ctx.tid) {
            m_old_thread_priority = osThreadGetPriority(m_api_fdfr_ctx.tid);
        }
    }
}

void kl520_api_fdfr_start(void)
{
    if(osMutexAcquire(_mutex_fdfr, 5000) != osOK) return;

    osEventFlagsClear(kl520_api_get_event(), KL520_APP_FLAG_FDFR);

    if (m_api_fdfr_ctx.tid) {
        kl520_measure_stamp(E_MEASURE_THR_FDFR_OPEN_RDY);
        set_thread_event(m_api_fdfr_ctx.tid, FLAGS_API_FDFR_OPEN_EVT);
        osDelay(1);
    }
    
    osMutexRelease(_mutex_fdfr);
}

void kl520_api_fdfr_stop(void)
{
    if(m_api_fdfr_ctx.state == FDFR_STATUS_IDLE) return; //already closed.
    
    if (NULL == _fdr_event_id) {
        _fdr_event_id = osEventFlagsNew(NULL);
    }

    if(osMutexAcquire(_mutex_fdfr, 5000) != osOK) return;

    if (m_api_fdfr_ctx.tid) {
        osEventFlagsClear(_fdr_event_id, FDFR_COM_EVENT_CLOSED);

        set_thread_event(m_api_fdfr_ctx.tid, FLAGS_API_FDFR_CLOSE_EVT);
        
        osEventFlagsWait(_fdr_event_id, FDFR_COM_EVENT_CLOSED, osFlagsWaitAny, 5000);
        
        m_api_fdfr_ctx.state = FDFR_STATUS_IDLE;
        m_api_fdfr_ctx.reg_idx = 0xff;
    }
    osMutexRelease(_mutex_fdfr);
}

int kl520_api_fdfr_exist_thread(void)
{
    if(m_api_fdfr_ctx.tid != 0 && m_api_fdfr_ctx.state != FDFR_STATUS_IDLE)
    {
        return 1;
    }

    //dbg_msg_console("fdfr NO=%d",m_api_fdfr_ctx.tid);
    return 0;
}

static u32 _api_fdfr_check_dist()
{
    u32 _fdfr_state = FDFR_STATUS_NORMAL;
    u16 dist = kdp_e2e_util_get_person_to_camera_distance();

    if(dist > FACE_DIST_MAX_THRESH )
        _fdfr_state = FDFR_STATUS_TOO_FAR;
    else if(dist < FACE_DIST_MIN_THRESH) {
        _fdfr_state = FDFR_STATUS_TOO_NEAR;
    }

    return _fdfr_state;
}

u32 _api_fdfr_check_eye(void)
{
    u32 _fdfr_state = FDFR_STATUS_NORMAL;
    kdp_e2e_face_variables *vars_cur = kdp_e2e_get_face_variables();

#define  EYE_CLOSE_RATIO     (0.5f)
#define  EYE_OPEN_RATIO      (0.5f)
#define  EYE_CLOSE_OPEN_RATIO     ((EYE_CLOSE_RATIO+EYE_OPEN_RATIO)/2)
    float eye_min = MIN(vars_cur->eye_left_ratio[0], vars_cur->eye_right_ratio[0]);
    float eye_max = MAX(vars_cur->eye_left_ratio[0], vars_cur->eye_right_ratio[0]);
#if (CFG_CONSOLE_MODE!=0)
    dbg_msg_console("[%d,%d]vars_cur->eye_left_ratio=%f, vars_cur->eye_right_ratio=%f", dp_draw_info.e2e_pos.x, dp_draw_info.e2e_pos.y, vars_cur->eye_left_ratio[0], vars_cur->eye_right_ratio[0]);
#endif
    if( eye_min <= EYE_CLOSE_RATIO)
    {
    		#if (CFG_CONSOLE_MODE!=0)            
        dbg_msg_console("eye_min=%f \033[31m[close]\033[m", eye_min);
    		#endif
        _fdfr_state = FDFR_STATUS_EYE_CLOSED;
    }
    else if(eye_min > EYE_OPEN_RATIO)
    {
    		#if (CFG_CONSOLE_MODE!=0)
        dbg_msg_console("eye_min=%f  \033[32m[open]\033[m", eye_min);
    		#endif
    		_fdfr_state = FDFR_STATUS_NORMAL;
    }
    else if(eye_max >= EYE_CLOSE_OPEN_RATIO)
    {
    		#if (CFG_CONSOLE_MODE!=0)
        dbg_msg_console("eye_min=%f \033[33m[close_open]\033[m", eye_min);
    		#endif
    		 _fdfr_state = FDFR_STATUS_EYE_CLOSE_UNKNOW_STATUS;//FDFR_STATUS_EYE_CLOSE_STATUS_OPEN_EYE;//FDFR_STATUS_EYE_CLOSE_UNKNOW_STATUS;
    }
    else
    {
    		 _fdfr_state = FDFR_STATUS_EYE_CLOSE_UNKNOW_STATUS;//FDFR_STATUS_EYE_CLOSE_STATUS_CLOSE_EYE;
    }
      
    //dbg_msg_console("[%d,%d]vars_cur->eye_left_ratio=%f, vars_cur->eye_right_ratio=%f", dp_draw_info.e2e_pos.x, dp_draw_info.e2e_pos.y, vars_cur->eye_left_ratio, vars_cur->eye_right_ratio);
    return _fdfr_state;
}

u32 _api_fdfr_ckeck_position_xy(s32 x, s32 y, s32 w, s32 h)
{
    u32 _fdfr_state = FDFR_STATUS_NORMAL;
    
#if (CFG_PANEL_TYPE > PANEL_NULL)
    struct video_input_params params;
    kdp_video_renderer_get_params(&params);
    int dp_w = (int)params.dp_out_w;
    int dp_h = (int)params.dp_out_h;
    int img_w = (int)params.dp_area_w;
    int img_h = (int)params.dp_area_h;

    x = x * dp_w / img_w;
    y = y * dp_h / img_h;
#endif

    dbg_msg_algo("[FACE_POS] (x, y, w, h)= (%d, %d, %d, %d)", x, y, w, h);

    if( y < FACE_POSTITON_UP_THRESH )
        _fdfr_state = FDFR_STATUS_TOOUP;
    else if((y + h) > FACE_POSTITON_DOWN_THRESH)
        _fdfr_state = FDFR_STATUS_TOODOWN;
    else if(x < FACE_POSTITON_LEFT_THRESH)
        _fdfr_state = FDFR_STATUS_TOOLEFT;
    else if((x + w) > FACE_POSTITON_RIGHT_THRESH)
        _fdfr_state = FDFR_STATUS_TOORIGHT;


#if (CFG_CAMERA_ROTATE == 1)
    switch (_fdfr_state)
    {
        case FDFR_STATUS_TOOLEFT:
            _fdfr_state = FDFR_STATUS_TOODOWN;
            dbg_msg_algo("[FDFR_STATUS_TOODOWN");
            break;
        case FDFR_STATUS_TOORIGHT:
            _fdfr_state = FDFR_STATUS_TOOUP;
            dbg_msg_algo("[FDFR_STATUS_TOOUP");
            break;
        case FDFR_STATUS_TOOUP:
            _fdfr_state = FDFR_STATUS_TOOLEFT;
            dbg_msg_algo("[FDFR_STATUS_TOOLEFT");
            break;
        case FDFR_STATUS_TOODOWN:
            _fdfr_state = FDFR_STATUS_TOORIGHT;
            dbg_msg_algo("[FDFR_STATUS_TOORIGHT");
            break;
        default:
            break;
    }
#endif

    return _fdfr_state;
}

static void _kl520_api_util_collection(kl520_dp_draw_info *lp_draw_info, u32 e2e_ret)
{
    s32 x, y, w, h;
    kdp_e2e_prop *prop = kdp_e2e_prop_get_inst();
    memcpy(&lp_draw_info->r1_rc, &prop->r1_rc, sizeof(prop->r1_rc));
    memcpy(&lp_draw_info->n1_rc, &prop->n1_rc, sizeof(prop->n1_rc));
    memcpy(&lp_draw_info->r1_pt_array, &prop->r1_pt_array, sizeof(prop->r1_pt_array));
    memcpy(&lp_draw_info->n1_pt_array, &prop->n1_pt_array, sizeof(prop->n1_pt_array));
    lp_draw_info->e2e_ret = e2e_ret;
    
    lp_draw_info->r1_lm_score = kdp_e2e_get_r1_lm()->score;
    lp_draw_info->n1_lm_score = kdp_e2e_get_n1_lm()->score;

    if( kdp_e2e_util_get_person_position(&x, &y, &w, &h) == 1) {
        lp_draw_info->e2e_pos.x = (u32)x;
        lp_draw_info->e2e_pos.y = (u32)y;
#if (FACE_REG_CHECK_POSITION == YES)
        lp_draw_info->e2e_pos_type = _api_fdfr_ckeck_position_xy(x, y, w, h);
#endif
#if (CFG_E2E_REC_NOTE == YES) || (KL520_REC_EYEMODE == YES)
        lp_draw_info->e2e_eye_type = _api_fdfr_check_eye();
#endif
    }
    else {
        lp_draw_info->e2e_pos.x = 0;
        lp_draw_info->e2e_pos.y = 0;
        lp_draw_info->e2e_pos_type = FDFR_STATUS_NORMAL;
        lp_draw_info->e2e_eye_type = FDFR_STATUS_NORMAL;
    }

    //mask
    if(E2E_ERROR_R1_FACE_MASK == e2e_ret || E2E_ERROR_N1_FACE_MASK == e2e_ret) {
        lp_draw_info->e2e_mask_type = FDFR_STATUS_MASK;
    }
    else {
        lp_draw_info->e2e_mask_type = FDFR_STATUS_NORMAL;
    }
}

BOOL _decide_liveness_enable(void)
{
    if ((kdp_e2e_prop_get2(flow_mode) > FLOW_MODE_SIM_MODELS))
        return FALSE;
    
    if (m_face_mode == FACE_MODE_BUSY) return TRUE; //in case of face close...
        
    if ((m_face_add_mode == FACE_ADD_MODE_5_FACES && m_api_fdfr_ctx.reg_idx == 0) ||
        (m_face_add_mode == FACE_ADD_MODE_1_FACE) ||
#if KL520_REC_EYEMODE == YES
        (m_face_mode == FACE_MODE_RECOGNITION && get_eye_mode_status() == 0 ) ||
#else
        (m_face_mode == FACE_MODE_RECOGNITION) ||
#endif
        (m_face_mode == FACE_MODE_LIVENESS)    ||
        (m_face_mode == FACE_MODE_RECOGNITION_TEST) ||
        (m_face_mode == FACE_MODE_COMPARE_1VS1)    ||
        (m_face_mode == FACE_MODE_ENG_CAL))
        return TRUE;

    return FALSE;
}

BOOL _decide_age_group_enable(void)
{
    if ((kdp_e2e_prop_get2(flow_mode) > FLOW_MODE_SIM_VIRTUAL))
        return TRUE;

    if(FACE_MODE_ADD != m_face_mode) return FALSE;
    
    if ((m_face_add_mode == FACE_ADD_MODE_5_FACES && m_api_fdfr_ctx.reg_idx == 0) ||
        (m_face_add_mode == FACE_ADD_MODE_1_FACE))
        return TRUE;

    return FALSE;
}

BOOL _decide_motion_detect_enable(void)
{
    if ((kdp_e2e_prop_get2(flow_mode) > FLOW_MODE_SIM_VIRTUAL))
        return FALSE;
    if( (m_face_add_mode == FACE_ADD_MODE_1_FACE) ||
        (m_face_mode == FACE_MODE_RECOGNITION)    ||
        (m_face_mode == FACE_MODE_LIVENESS)       ||
        (m_face_mode == FACE_MODE_COMPARE_1VS1)    ||
        (m_face_mode == FACE_MODE_RECOGNITION_TEST))
        return TRUE;

    return FALSE;
}

BOOL _decide_nose_lm_diff_detect_enable(void)
{
    if ((kdp_e2e_prop_get2(flow_mode) > FLOW_MODE_SIM_MODELS))
        return FALSE;

    if ((m_face_mode == FACE_MODE_RECOGNITION) ||
        (m_face_mode == FACE_MODE_LIVENESS)    ||
        (m_face_mode == FACE_MODE_COMPARE_1VS1)    ||
        (m_face_mode == FACE_MODE_RECOGNITION_TEST))
        return TRUE;

    return FALSE;
}

s32 _decide_check_bad_pose_enable(void)
{
    if ((m_face_mode == FACE_MODE_RECOGNITION) ||
        (m_face_mode == FACE_MODE_LIVENESS)    ||
        (m_face_mode == FACE_MODE_COMPARE_1VS1)    ||
        (m_face_mode == FACE_MODE_RECOGNITION_TEST))
        return 1;


    if (m_face_mode == FACE_MODE_ADD)
    {
        if (m_face_add_mode == FACE_ADD_MODE_5_FACES) {
            if( m_api_fdfr_ctx.reg_idx == 0 )  return 2; //face reg
    #if ( KL520_FACE_ADD_BMP == YES )
            else {
                s32 bp = 0x40 | face_add_bitmap;
                return bp;
            }
    #else
            else return 3;
    #endif
        }
        if (m_face_add_mode == FACE_ADD_MODE_1_FACE)
            return 2;
    }

    if ((kdp_e2e_prop_get2(flow_mode) > FLOW_MODE_SIM_VIRTUAL))
        return 0;
    
    return 0;
}

BOOL _decide_nir_aec_continuous_tune_enable(void)
{
    if ((kdp_e2e_prop_get2(flow_mode) > FLOW_MODE_SIM_VIRTUAL))
        return FALSE;

    if ((m_face_add_mode == FACE_ADD_MODE_5_FACES) ||
        (m_face_add_mode == FACE_ADD_MODE_1_FACE)  ||
        (m_face_mode == FACE_MODE_RECOGNITION)     ||
        (m_face_mode == FACE_MODE_LIVENESS)        ||
        (m_face_mode == FACE_MODE_COMPARE_1VS1)    ||
        (m_face_mode == FACE_MODE_RECOGNITION_TEST))
        return TRUE;

    return FALSE;
}

BOOL _decide_rgb_led_when_reg_enable(void)
{
    if ((kdp_e2e_prop_get2(flow_mode) > FLOW_MODE_SIM_VIRTUAL))
        return FALSE;

    if (m_face_mode == FACE_MODE_ADD)
        return TRUE;

    return FALSE;
}

BOOL _decide_face_check_move_enable(void)
{
    if ((kdp_e2e_prop_get2(flow_mode) > FLOW_MODE_SIM_VIRTUAL))
        return FALSE;

    if (m_face_mode == FACE_MODE_ADD) 
    {
        if ((m_face_add_mode == FACE_ADD_MODE_5_FACES && m_api_fdfr_ctx.reg_idx == 0)
            || (m_face_add_mode == FACE_ADD_MODE_1_FACE))
            return TRUE;
    }

    return FALSE;
}

BOOL _decide_face_check_position_enable(void)
{
    if ((kdp_e2e_prop_get2(flow_mode) > FLOW_MODE_SIM_VIRTUAL))
        return FALSE;

    if (m_face_mode == FACE_MODE_ADD)
        return TRUE;

    return FALSE;
}

static int _kl520_api_fdfr_only(s32 *e2e_result)
{
    s32 ret = KL520_FACE_FAIL;
    s32 e2e_ret = E2E_OK;
    dbg_msg_algo ("\n\n\nstart fdfr only....\n");

    kdp_e2e_prop_set2(liveness_en, _decide_liveness_enable());
    kdp_e2e_prop_set2(age_group_en, _decide_age_group_enable());
    kdp_e2e_prop_set2(motion_detect_en, _decide_motion_detect_enable());
    kdp_e2e_prop_set2(nose_lm_diff_en, _decide_nose_lm_diff_detect_enable());
    kdp_e2e_prop_set2(check_bad_pose, _decide_check_bad_pose_enable());
    kdp_e2e_prop_set2(nir_aec_continuous_tune_en, _decide_nir_aec_continuous_tune_enable());
    kdp_e2e_prop_set2(rgb_led_when_reg_en, _decide_rgb_led_when_reg_enable());
    kdp_e2e_prop_set2(face_check_movement_en, _decide_face_check_move_enable());
    kdp_e2e_prop_set2(face_check_position, _decide_face_check_position_enable());

#if CFG_AI_TYPE == AI_TYPE_N1 && CFG_E2E_STRUCT_LIGHT == YES
    kdp_e2e_face_variables *vars = kdp_e2e_get_face_variables();
    nir_led_mode led_status = (nir_led_mode)vars->nir_led_status;
#endif


    if( kdp_e2e_prop_get2(flow_mode) == FLOW_MODE_SIM_PRE_ADD)
    {
#if ( CFG_LW3D_TYPE == CFG_LW3D_NORMAL && CFG_AI_TYPE != AI_TYPE_N1 )
        e2e_ret = kdp_e2e_face_r1_only();
#endif
    }
    else
    {
    
#if CFG_AI_TYPE == AI_TYPE_R1
    e2e_ret = kdp_e2e_face_r1_only();
#elif CFG_AI_TYPE == AI_TYPE_R1N1
    e2e_ret = kdp_e2e_face_r1n1();
#elif CFG_AI_TYPE == AI_TYPE_N1

    if(b_en_aec_only == true)
    {
        e2e_ret = kdp_e2e_ctrl_aec();
    }
    else
    {
        e2e_ret = kdp_e2e_face_n1_only();
    }     
#elif CFG_AI_TYPE == AI_TYPE_N1R1
#if CFG_PALM_PRINT_MODE == 1
    if(kdp_is_palm_mode())
    {
        kdp_e2e_aec_brightness_stats_set_block( BRIGHTNESS_STATS_X_BLOCK_SIZE, BRIGHTNESS_STATS_Y_BLOCK_SIZE, BRIGHTNESS_STATS_STEP );
        e2e_ret = kdp_e2e_palm_r1();
    }
    else
#endif
    {
        e2e_ret = kdp_e2e_face_n1r1();
    }
#endif

    }

    //dbg_msg_console("e2e_ret=%x", e2e_ret);

    *e2e_result = e2e_ret;

//#if (KL520_API_SHOW_BOUNDINGBOX == YES)
    if (E2E_OK != e2e_ret)
    {
        if ((m_face_mode == FACE_MODE_RECOGNITION) || (m_face_mode == FACE_MODE_LIVENESS))
            _kl520_api_util_collection(&dp_draw_info, e2e_ret);
    }
//#endif
    //dbg_msg_console("e2e_ret = 0x%x" , e2e_ret);
#if (USE_SCORE_JUDGEMENT == YES && CFG_AI_TYPE != AI_TYPE_R1 /*&& CFG_AI_TYPE != AI_TYPE_N1*/)
    if (E2E_OK == e2e_ret) {
        vote_score = 0;
        ret = KL520_FACE_OK;
    }
    else if(E2E_ERROR_R1_FACE_MASK == e2e_ret || E2E_ERROR_N1_FACE_MASK == e2e_ret){
        dbg_msg_algo("KL520_FACE_MASK");
        ret = KL520_FACE_MASK;
    }
    else if(E2E_ERROR_R1_EYE_CLOSED == e2e_ret || E2E_ERROR_N1_EYE_CLOSED == e2e_ret){
        dbg_msg_algo("KL520_FACE_EYE_CLOSED");
        ret = KL520_FACE_EYE_CLOSED;
    }
    else if(E2E_ERROR_R1_FACE_QUALTY == e2e_ret || E2E_ERROR_N1_FACE_QUALTY == e2e_ret || 
            E2E_ERROR_R1_FACE_QUALTY_2 == e2e_ret || E2E_ERROR_N1_FACE_QUALTY_2 == e2e_ret ) {
        dbg_msg_algo("E2E_ERROR_R1_FACE_QUALTY");
        ret = KL520_FACE_LOW_QUALITY;
    }
    else if(E2E_ERROR_N1_LM_MOTION_CHK == e2e_ret && _decide_face_check_move_enable()) {
        dbg_msg_algo("E2E_ERROR_N1_LM_MOTION_CHK");
        ret = KL520_FACE_WAIT_DONT_MOVE;
    }
    else if(e2e_ret == E2E_FACE_TOO_FAR || e2e_ret == E2E_FACE_TOO_NEAR) {
        dbg_msg_algo("E2E_ERROR_FACE_DIST");
        if(e2e_ret == E2E_FACE_TOO_FAR) ret = KL520_FACE_TOO_FAR;
        else ret = KL520_FACE_TOO_NEAR;
    }
    else if(e2e_ret >= E2E_FACE_TOO_UP && e2e_ret <= E2E_FACE_TOO_RIGHT) {
        dbg_msg_algo("E2E_ERROR_FACE_POSITION");
        ret = KL520_FACE_TOOUP + (e2e_ret - E2E_FACE_TOO_UP);
    }
    else if(E2E_ERROR_R1_LM_BAD_POSE == e2e_ret && _decide_check_bad_pose_enable() > 1) {
        dbg_msg_algo("E2E_ERROR_R1_LM_BAD_POSE");
        ret = KL520_FACE_BADPOSE;
    }
    else {
        if (m_face_mode == FACE_MODE_RECOGNITION || m_face_mode == FACE_MODE_ENG_CAL || m_face_mode == FACE_MODE_LIVENESS) {
            
#if CFG_AI_TYPE == AI_TYPE_R1
            s32 tmp_vote_score = kdp_e2e_face_r1n1_invalid_score(&e2e_ret);
#elif CFG_AI_TYPE == AI_TYPE_R1N1
            s32 tmp_vote_score = kdp_e2e_face_r1n1_invalid_score(&e2e_ret);
#elif CFG_AI_TYPE == AI_TYPE_N1
            s32 tmp_vote_score = kdp_e2e_face_n1_only_invalid_score(&e2e_ret); 
#elif CFG_AI_TYPE == AI_TYPE_N1R1
            s32 tmp_vote_score = kdp_e2e_face_r1n1_invalid_score(&e2e_ret);
#endif
            
            dbg_msg_algo("tmp_vote_score=%d", tmp_vote_score);
            if (tmp_vote_score   & E2E_INVALID) {
                ret = KL520_FACE_DETECTED;
            }
            else {
                vote_score += tmp_vote_score;
                dbg_msg_algo("vote_score=%d", vote_score);
            }

            if (vote_score >= KL520_FACE_SCORE_MAX) {
                
                if(m_face_mode == FACE_MODE_ENG_CAL) { ret = KL520_FACE_CALLIB_FAIL;}
                else{ ret = KL520_FACE_INVALID;}
            }
        } else if (m_face_mode == FACE_MODE_ADD) {
            if( (e2e_ret == E2E_ERROR_N1_LV_MODEL) || 
                (e2e_ret == E2E_ERROR_N1_HSN_LV_MODEL) || 
                (e2e_ret == E2E_ERROR_N1_EYE_LV_MODEL) || 
                (e2e_ret == E2E_ERROR_FU_LV_MODEL) ||
                (e2e_ret == E2E_ERROR_CV_LV_MODEL) ) {
                dbg_msg_algo("E2E_ERROR_FACE_ATTACK");
                ret = KL520_FACE_ATTACK;
            }
            else if ((e2e_ret == E2E_ERROR_R1_FD_MODEL)
                    || (e2e_ret == E2E_ERROR_N1_FD_MODEL))
            {
                ret = KL520_FACE_NOFACE;
            }
            
        }
#if (E2E_N1_ONLY_STRUCTURE_ENABLE == YES)
        else if (m_face_mode == FACE_MODE_LIVENESS) {
            if (e2e_ret == E2E_ERROR_N1_S_LV_MODEL) { ret = KL520_FACE_INVALID;}
        }
#endif
    }
    if (E2E_OK != e2e_ret)
        dbg_msg_console("e2e_ret: 0x%08x", e2e_ret);
    else
        dbg_msg_algo("e2e_ret: 0x%08x", e2e_ret);
    
#else
    if (E2E_OK == e2e_ret) {
        ret = KL520_FACE_OK;
    }
#endif

    if(kl520_api_sim_is_running() == TRUE)
    {
        ret = e2e_ret;
    }

#if CFG_AI_TYPE == AI_TYPE_N1 && CFG_E2E_STRUCT_LIGHT == YES
    if( ( led_status == IR_LIGHT && E2E_ENVIR != e2e_ret && get_usb_catch_image_log() == 1 ) \
     || ( led_status == STR_LIGHT && E2E_OK != e2e_ret && get_usb_catch_image_log() == 2 ) )
        kl520_api_snapshot_adv_shot_after_fdfr_element(); // liveness failed or IR failed
#endif

#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
    dp_draw_info.e2e_ret = e2e_ret;
    kl520_api_snapshot_adv_shot_cont(e2e_ret);
#endif

    return ret;
}

static int _kl520_api_fdfr_del_user(u8 del_ctrl, u8 user_id)
{
    int ret = KL520_FACE_FAIL;

    kl520_api_fdfr_model_init();

    if (E2E_OK == kdp_e2e_db_delete(del_ctrl, (u16)user_id)) {
        ret = KL520_FACE_OK;
    }
    else {
    }

    return ret;
}

static int _kl520_api_fdfr_query_user(u8 user_id)
{
    int ret = KL520_FACE_FAIL;
    u16 valid_fm0 = 0, valid_fm1 = 0, type = 0;

    kl520_api_fdfr_model_init();

    s32 r1 = kdp_e2e_db_get_user_info((u16)user_id, &valid_fm0, &valid_fm1, &type);
    if(r1 != E2E_OK) return KL520_FACE_EMPTY;
    //dbg_msg_api("[query_user] : OK!, valid fm0=%u valid fm1=%u type=%u",valid_fm0, valid_fm1, type);
    if(0 < valid_fm0 && TYPE_INVALID != type)
        ret = KL520_FACE_EXIST;
    else
        ret = KL520_FACE_EMPTY;
    //dbg_msg_api("query ret = %d", ret);
    return ret;
}

 static int _kl520_api_fdfr_query_user_is_added(u16 db_idx)
{
    int ret = -1;
    u16 valid_fm0 = 0, valid_fm1 = 0, type = 0;

    kl520_api_fdfr_model_init();

    //kdp_e2e_db_get_user_info((u16)user_id, &valid_fm0, &valid_fm1, &type);
    //dbg_msg_api("[query_user] : OK!, valid fm0=%u valid fm1=%u type=%u",valid_fm0, valid_fm1, type);
    
    s32 r1 = kdp_e2e_db_get_user_info_by_idx(db_idx, &valid_fm0, &valid_fm1, &type);
    if(r1 != E2E_OK) return KL520_FACE_FAIL;

    if(0 < valid_fm0 && TYPE_VALID == type)
        ret = KL520_FACE_EXIST;
    else
        ret = KL520_FACE_EMPTY;
    //dbg_msg_api("query ret = %d", ret);
    return ret;
}


static int _kl520_api_fdfr_query_all(u8* total_id_num, u8* face_id)
{
    int ret = KL520_FACE_OK;
    int idx = 0;
    u8 exist_cnt = 0;
    u16 valid_fm0 = 0, valid_fm1 = 0, type = 0;
    s32 e2e_ret;

    kl520_api_fdfr_model_init();

    for(idx=0; idx<MAX_USER; idx++)
    {
        e2e_ret = kdp_e2e_db_get_user_info_by_idx(idx, &valid_fm0, &valid_fm1, &type);
        if(e2e_ret != E2E_OK) continue;

        //dbg_msg_api("[query_user] : OK!, valid fm0=%u valid fm1=%u type=%u",valid_fm0, valid_fm1, type);
        if(0 < valid_fm0 && TYPE_VALID == type)
        {
            face_id[exist_cnt] = kdp_app_db_get_user_id(idx);
            dbg_msg_api("face_id[%d] = %d", exist_cnt, face_id[exist_cnt]);
            exist_cnt++;
        }

        if(E2E_OK != e2e_ret) {
            ret = KL520_FACE_FAIL;
            break;
        }
    }
    *total_id_num = exist_cnt;

    return ret;
}

///////////////////////////
/* kl520_api_fdfr series */
///////////////////////////
static int _kl520_api_fdfr_add(u8 user_id, u8 fmap_idx)
{
    int ret = KL520_FACE_OK;

    #if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE
    int reserve_db_num = kl520_api_face_get_reserve_db_num();
    #else
    int reserve_db_num = 0;
    #endif

    int e2e_ret1 = kdp_e2e_db_register((u16)user_id, (u16)fmap_idx, reserve_db_num);

    if(e2e_ret1 != E2E_OK) {
        dbg_msg_api("db register failed:%d.", e2e_ret1);
        return KL520_FACE_FAIL;
    }

	BOOL db_wo_flash = FALSE;
	if( m_face_db_add_mode == FACE_ADD_MODE_NO_DB )	db_wo_flash = TRUE;

    if (FACE_ADD_MODE_1_FACE == m_face_add_mode) {

        int e2e_ret2 = kdp_e2e_db_add(user_id, db_wo_flash, reserve_db_num );

        dbg_msg_api("[face_add_mode:%u] user_id=%x e2e_ret1=%d e2e_ret=%d", m_face_add_mode, user_id, e2e_ret1, e2e_ret2);
        if (E2E_OK == e2e_ret2) {
            dbg_msg_algo("[registration] : -- OK!");
            ret = KL520_FACE_OK;
        }
        else {
            dbg_msg_api("[registration] : -- ERROR!, e2e_ret2=%x", e2e_ret2);
            ret = KL520_FACE_FAIL;
        }
    }
    else if (FACE_ADD_MODE_5_FACES == m_face_add_mode) {
        if ( fmap_idx-1 == FACE_ADD_TYPE_DOWN )
        {

            int e2e_ret2 = kdp_e2e_db_add(user_id, db_wo_flash, reserve_db_num );

            dbg_msg_algo("[face_add_mode:%u] user_id=%x e2e_ret1=%d e2e_ret2=%d", m_face_add_mode, user_id, e2e_ret1, e2e_ret2);
            if (E2E_OK == e2e_ret2) {
                dbg_msg_algo("[registration] : -- OK!");
                ret = KL520_FACE_OK;
            }
            else {
                dbg_msg_api("[registration] : -- ERROR!, e2e_ret2=%x", e2e_ret2);
                ret = KL520_FACE_FAIL;
            }
        }
    }
    else{
        dbg_msg_console("[registration] : -- ERROR!, ");
    }

    return ret;
}

//static int _kl520_api_fr_fmap_update_db(u16 u_idx, u8 fmap_idx)
//{
//    int ret = KL520_FACE_OK;

//#if (CFG_LW3D_TYPE == CFG_LW3D_NORMAL)
//    //update rgb entry
//    uint32_t rgb_fdr_addr = (u32)kdp_e2e_get_r1_fr();
//    kdp_app_db_fmap_update(rgb_fdr_addr, u_idx, fmap_idx, FR_TO_DB_IDX_0);
//    kdp_app_db_save_fmap_to_flash(u_idx, fmap_idx, FR_TO_DB_IDX_0);
//#endif

//    //update nir entry
//    uint32_t nir_fdr_addr = (u32)kdp_e2e_get_n1_fr();
//    kdp_app_db_fmap_update(nir_fdr_addr, u_idx, fmap_idx, FR_TO_DB_IDX_1);
//    kdp_app_db_save_fmap_to_flash(u_idx, fmap_idx, FR_TO_DB_IDX_1);
//    return ret;
//}

#if 0
static int _kl520_api_fdfr_update(u8 user_id, u8 fmap_idx, float coef)
{
    int ret = KL520_FACE_OK;

    int reserve_db_num = 0;

    int e2e_ret1 = kdp_e2e_db_recog_update((u16)user_id, (u16)fmap_idx, reserve_db_num, coef);

    if(e2e_ret1 != E2E_OK) {
        dbg_msg_api("db register failed:%d.", e2e_ret1);
        return KL520_FACE_FAIL;
    }

    BOOL db_wo_flash = FALSE;
    
    if( m_face_db_add_mode == FACE_ADD_MODE_NO_DB ) db_wo_flash = TRUE;

    int e2e_ret2 = kdp_e2e_db_update(user_id, db_wo_flash, reserve_db_num );

    dbg_msg_api("[face_add_mode:%u] user_id=%x e2e_ret1=%d e2e_ret2=%d", m_face_add_mode, user_id, e2e_ret1, e2e_ret2);
    if (E2E_OK == e2e_ret2) {
        dbg_msg_api("[registration] : -- OK!");
        ret = KL520_FACE_OK;
    }
    else {
        dbg_msg_api("[registration] : -- ERROR!, e2e_ret2=%x", e2e_ret2);
        ret = KL520_FACE_FAIL;
    }
    return ret;
}
#endif

#if (MEASURE_RECOGNITION == YES)
int no_rgb_source_cnt = 0;
int ccnt = 1;
static void _kl520_measure_stamp_fb_mgr_rgb_next_inf(void)
{
    ++no_rgb_source_cnt;
    //dbg_msg_api("no_rgb_source_cnt=%d", no_rgb_source_cnt);
//    switch (no_rgb_source_cnt) {
//        case 1: kl520_measure_stamp(E_MEASURE_R1_NO_SRC_CNT_01); break;
//        case 2: kl520_measure_stamp(E_MEASURE_R1_NO_SRC_CNT_02); break;
//        case 3: kl520_measure_stamp(E_MEASURE_R1_NO_SRC_CNT_03); break;
//        case 4: kl520_measure_stamp(E_MEASURE_R1_NO_SRC_CNT_04); break;
//        case 5: kl520_measure_stamp(E_MEASURE_R1_NO_SRC_CNT_05); break;
//    }
}
#endif

static BOOL _api_fdfr_cam_capture(int cam_idx)
{
    if (((0 == cam_idx)?CFG_SENSOR_0_TYPE:CFG_SENSOR_1_TYPE) == SENSOR_TYPE_NULL)
        return TRUE;

    unsigned int frame_addr = 0;
    kdp_e2e_face_variables *vars = kdp_e2e_get_face_variables();

#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
    if(kl520_api_sim_is_running() == TRUE)
    {
        frame_addr = kl520_api_snapshot_ddr_addr(cam_idx);
    }
    else
#endif
    {
        frame_addr = kdp_fb_mgr_next_inf(cam_idx, &vars->buf_idx[cam_idx]);
    }
#if (FB_TILE_RECODE == YES)
    if(kdp_fb_mgr_tile_delay(cam_idx, vars->buf_idx[cam_idx]) == TRUE){
        dbg_msg_console("Tile is not reday");
        return FALSE;
    }
#endif

    if (0 == frame_addr){
#if (MEASURE_RECOGNITION == YES)
        if(CAMERA_DEVICE_RGB_IDX == cam_idx)
            _kl520_measure_stamp_fb_mgr_rgb_next_inf();
#endif
        return FALSE;
    }

    kdp_e2e_set_img_mem_addr(cam_idx, frame_addr);

#if (CFG_E2E_NIR_TWO_STAGE_LIGHT == YES)
    kl520_api_nir_reserve(cam_idx, frame_addr);
#endif    
    
#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
    kl520_api_snapshot_fdfr_cam(cam_idx, frame_addr);
#endif

    return TRUE;
}
#if CFG_ONE_SHOT_MODE == YES
extern BOOL skip_all_led;
#endif
static BOOL _api_fdfr_thread_data_capture(void)
{
#if CFG_SNAPSHOT_ENABLE == 2
    kl520_api_snapshot_adv_chk();
#endif

#if (CFG_FMAP_EXTRA_ENABLE == YES)
    kl520_api_extra_fmap_adv_chk();
#endif

#if (CFG_AI_TYPE == AI_TYPE_R1)
    if(_api_fdfr_cam_capture(CAMERA_DEVICE_RGB_IDX) == FALSE)
        return FALSE;
#elif (CFG_AI_TYPE == AI_TYPE_R1N1)
    if(_api_fdfr_cam_capture(CAMERA_DEVICE_RGB_IDX) == FALSE)
        return FALSE;
    if(_api_fdfr_cam_capture(CAMERA_DEVICE_NIR_IDX) == FALSE)
        return FALSE;
#elif (CFG_AI_TYPE == AI_TYPE_N1)
    if(_api_fdfr_cam_capture(CAMERA_DEVICE_NIR_IDX) == FALSE)
        return FALSE;
#elif (CFG_AI_TYPE == AI_TYPE_N1R1)
#if CFG_PALM_PRINT_MODE == 1
    if(kdp_is_palm_mode()) {
        if(_api_fdfr_cam_capture(CAMERA_DEVICE_RGB_IDX) == FALSE)
            return FALSE;
    } 
    else 
#endif
    {
        if(_api_fdfr_cam_capture(CAMERA_DEVICE_NIR_IDX) == FALSE)
            return FALSE;
#if (CFG_CAMERA_SINGLE_1054 == NO)
        if( kdp_e2e_get_face_variables()->init_tile_flag == AEC_STABLE
        || kdp_e2e_get_face_variables()->rgb_img_ready != CAM_IMAGE_STATE_NULL )
        {
            if(_api_fdfr_cam_capture(CAMERA_DEVICE_RGB_IDX) == FALSE)
                return FALSE;

            kdp_e2e_get_face_variables()->rgb_img_ready = CAM_IMAGE_STATE_RUN;
        }
#endif
    }

#endif

    return TRUE;
}

static void _api_fdfr_cam_prepare(int cam_idx)
{
    kdp_e2e_face_variables *vars = kdp_e2e_get_face_variables();
    if(kl520_api_sim_is_running() == FALSE) {
        kdp_fb_mgr_inf_done(cam_idx, vars->buf_idx[cam_idx]);
    }
}

static void _api_fdfr_thread_data_prepare(void)
{

#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
    kl520_api_sim_set_fdfr();
    //kl520_api_snapshot_fdfr_inf(0);
#endif

#if (CFG_AI_TYPE == AI_TYPE_R1)
    _api_fdfr_cam_prepare(CAMERA_DEVICE_RGB_IDX);
#elif (CFG_AI_TYPE == AI_TYPE_R1N1)
    _api_fdfr_cam_prepare(CAMERA_DEVICE_RGB_IDX);
    _api_fdfr_cam_prepare(CAMERA_DEVICE_NIR_IDX);
#elif (CFG_AI_TYPE == AI_TYPE_N1)
    _api_fdfr_cam_prepare(CAMERA_DEVICE_NIR_IDX);
#elif (CFG_AI_TYPE == AI_TYPE_N1R1)
    _api_fdfr_cam_prepare(CAMERA_DEVICE_NIR_IDX);
    _api_fdfr_cam_prepare(CAMERA_DEVICE_RGB_IDX);

#endif

}

void _chk_fdfr_fps(void)
{
    static float ftime = 0;
    static float ffps = 0;
    if(osKernelGetTickCount() - ftime > 3000.0f){
        float sfps = (ffps/3.0f);

        if(sfps != 0){
            dbg_msg_engineering("fdfr-fps:%5.2f",sfps );
        }
        ffps = 0;
        ftime = (float)osKernelGetTickCount();
    }
    else{
        ffps++;
    }
}

static void _api_fdfr_thread_inc_priority(void)
{
#ifdef FDFR_THREAD_PRIORITY_DYNAMIC_ADJUSTMENT
    if ((m_old_thread_priority + 2) != osThreadGetPriority(m_api_fdfr_ctx.tid)) {
        osThreadSetPriority(m_api_fdfr_ctx.tid, (osPriority_t)(m_old_thread_priority + 2));
    }
#endif
}

static void _api_fdfr_thread_rst_priority(void)
{
#ifdef FDFR_THREAD_PRIORITY_DYNAMIC_ADJUSTMENT
    if (m_old_thread_priority != osThreadGetPriority(m_api_fdfr_ctx.tid)) {
        osThreadSetPriority(m_api_fdfr_ctx.tid, m_old_thread_priority);
    }
#endif
}

#if (MEASURE_RECOGNITION == YES)
// static void _kl520_measure_stamp_capture(void)
// {
//     switch (ccnt) {
//         case 1: kl520_measure_stamp(E_MEASURE_FRAME_1_CAP_STR); break;
//         case 2: kl520_measure_stamp(E_MEASURE_FRAME_2_CAP_STR); break;
//         case 3: kl520_measure_stamp(E_MEASURE_FRAME_3_CAP_STR); break;
//         case 4: kl520_measure_stamp(E_MEASURE_FRAME_4_CAP_STR); break;
//         case 5: kl520_measure_stamp(E_MEASURE_FRAME_5_CAP_STR); break;
//         case 6: kl520_measure_stamp(E_MEASURE_FRAME_6_CAP_STR); break;
//         case 7: kl520_measure_stamp(E_MEASURE_FRAME_7_CAP_STR); break;
//         case 8: kl520_measure_stamp(E_MEASURE_FRAME_8_CAP_STR); break;
//     }
// }

static void _kl520_measure_stamp_face_start(void)
{
    switch (ccnt) {
        case 1: kl520_measure_stamp(E_MEASURE_FRAME_1_FACE_STR); break;
        case 2: kl520_measure_stamp(E_MEASURE_FRAME_2_FACE_STR); break;
        case 3: kl520_measure_stamp(E_MEASURE_FRAME_3_FACE_STR); break;
        case 4: kl520_measure_stamp(E_MEASURE_FRAME_4_FACE_STR); break;
        case 5: kl520_measure_stamp(E_MEASURE_FRAME_5_FACE_STR); break;
    }
}

static void _kl520_measure_stamp_face_end(void)
{
    switch (ccnt) {
        case 1: kl520_measure_stamp(E_MEASURE_FRAME_1_FACE_END); break;
        case 2: kl520_measure_stamp(E_MEASURE_FRAME_2_FACE_END); break;
        case 3: kl520_measure_stamp(E_MEASURE_FRAME_3_FACE_END); break;
        case 4: kl520_measure_stamp(E_MEASURE_FRAME_4_FACE_END); break;
        case 5: kl520_measure_stamp(E_MEASURE_FRAME_5_FACE_END); break;
    }
    ++ccnt;
}
#endif

//void kl520_api_aec_n1_element(void *argument)
//{
//    int buf_idx;
//    int prev_buf_idx = - 1;
//    //u32 buf_addr;
//    u16 delay_cnt = 0;
//    
//    for(;;)
//    {
//        kdp_fb_mgr_next_aec(CAMERA_DEVICE_NIR_IDX, &buf_idx);
//        
//        //dbg_msg_console("[%s], buf_idx=%d, buf_addr=%#x", __func__, buf_idx, buf_addr);
//        
//        if(nir_sensor_delay_fdfr() == true || prev_buf_idx == buf_idx){
//            osDelay(1);
//            delay_cnt++;
//            continue;
//        }
//        #if (AEC_MULTI_THREAD == YES)
//            kdp_nir_aec_thread(buf_addr);
//        #endif
//        
//        kdp_fb_mgr_free_aec_buf(CAMERA_DEVICE_NIR_IDX);
//        //dbg_msg_console("[%s], buf_idx=%d, buf_addr=%#x, delay_cnt=%d", __func__, buf_idx, buf_addr, delay_cnt);
//        
//        delay_cnt = 0;
//        prev_buf_idx = buf_idx;
//    }
//}

//void kl520_api_aec_r1_element(void *argument)
//{
//    int buf_idx;
//    int prev_buf_idx = - 1;
//    //u32 buf_addr;
//    u16 delay_cnt = 0;
//    
//    for(;;)
//    {
//        kdp_fb_mgr_next_aec(CAMERA_DEVICE_RGB_IDX, &buf_idx);
//        
//        //dbg_msg_console("[%s], buf_idx=%d, buf_addr=%#x", __func__, buf_idx, buf_addr);
//        
//        if(rgb_sensor_delay_fdfr() == true || prev_buf_idx == buf_idx){
//            osDelay(1);
//            delay_cnt++;
//            continue;
//        }
//        
//        #if (AEC_MULTI_THREAD == YES)
//            kdp_rgb_aec_thread(buf_addr);
//        #endif
//        
//        kdp_fb_mgr_free_aec_buf(CAMERA_DEVICE_RGB_IDX);
//        
//        //dbg_msg_console("[%s], buf_idx=%d, buf_addr=%#x, delay_cnt=%d", __func__, buf_idx, buf_addr, delay_cnt);
//        
//        delay_cnt = 0;
//        prev_buf_idx = buf_idx;
//    }
//}

int kl520_api_fdfr_element(void)
{
    u16 _fdfr_element_cnt = 0;
    int ret = KL520_FACE_FAIL;
    s32 e2e_result;

    while(1){

        if (kl520_is_fdfr_abort() != 0) return KL520_FACE_FAIL;

    #if CFG_ONE_SHOT_MODE == YES
        if(kdp_e2e_get_dual_cam_state() == DUAL_IDENTICAL_CAM_SLEEP) {
            kdp_e2e_set_dual_cam_state(DUAL_IDENTICAL_CAM_RUN);
            camera_sensor_rst();
            osDelay(1);
            continue;
        }
    #endif  
       
        if(nir_sensor_delay_fdfr() == true || rgb_sensor_delay_fdfr() == true){
    #if (CFG_CAMERA_SINGLE_1054 == NO)
            if( kl520_api_cam_state_get(1) != KDP_DEVICE_CAMERA_RUNNING ) break;
    #endif
            osDelay(1);
            continue;	
        }

    #if (CALC_FDFR_MS_ONCE == YES)
        u32 fdfr_srt, fdfr_end;
        fdfr_srt = osKernelGetTickCount();
    #endif
        _api_fdfr_thread_inc_priority();

    // #if (MEASURE_RECOGNITION == YES)
    //     _kl520_measure_stamp_capture();
    // #endif

        if (FALSE == _api_fdfr_thread_data_capture()) {
            _api_fdfr_thread_data_prepare();
            return ret;
        }

    #if CFG_ONE_SHOT_MODE == YES 
        if(kdp_e2e_get_dual_cam_state() == DUAL_IDENTICAL_CAM_RUN) {
            kdp_e2e_set_dual_cam_state(DUAL_IDENTICAL_CAM_SLEEP);
        }
    #endif

    #if (MEASURE_RECOGNITION == YES)
        _kl520_measure_stamp_face_start();
    #endif

        ret = _kl520_api_fdfr_only(&e2e_result);

    #if (MEASURE_RECOGNITION == YES)
        _kl520_measure_stamp_face_end();
    #endif

        #if (CFG_FMAP_EXTRA_ENABLE == YES)
        // jim : call push
        kl520_api_extra_fmap_adv_shot_save( ret );
        #endif

        if(ret != KL520_FACE_OK) { //if ret is OK, free the buf after handling
            _api_fdfr_thread_data_prepare();
        }

    #if (CALC_FDFR_MS_ONCE == YES)
        fdfr_end = osKernelGetTickCount();
        dbg_msg_api("FDFR Time:%d",fdfr_end-fdfr_srt);
    #endif

    #if (CALC_FDFR_FPS == YES)
        _chk_fdfr_fps();
    #endif

        if(_fdfr_element_cnt >= FDFR_BREAK_NUM || e2e_result != E2E_ENVIR) {
            break;
        }
        else{
            _fdfr_element_cnt++;
            if(kl520_api_sim_is_running()) break; //do not loop in case of sim
        }
    }

    return ret;
}

s8 face_pose_val[5];
u32 face_reg_sts;
u8  face_hp_check_idx;

static int is_face_moved_thresh(uint16_t org_x, uint16_t org_y, uint16_t cur_x, uint16_t cur_y)
{
    int x_dis = cur_x - org_x;
    if(x_dis < 0) x_dis = 0 - x_dis;

    int y_dis = cur_y - org_y;
    if(y_dis < 0) y_dis = 0 - y_dis;

    if(x_dis < FACE_MOVE_X_THRESH && y_dis < FACE_MOVE_Y_THRESH) return 0;

    return 1;
}

static void _kl520_fdfr_drawing_timer_cb(void *argument)
{
    kl520_fdfr_drawing_timer_flag = KL520_FACE_FAIL;
}

static void _kl520_fdfr_drawing_timer_create(int face_status)
{
#ifndef FDFR_KEEP_MS_WHEN_FACE_CLOSED
    osDelay(KL520_FDFR_KEEP_MS);
#else
#ifdef USE_FDFR_DRAWING_TIMER
    if(FACE_MODE_RECOGNITION_TEST == m_face_mode)
    {
        if (0 == kl520_fdfr_drawing_timer)
            kl520_fdfr_drawing_timer = osTimerNew(_kl520_fdfr_drawing_timer_cb, osTimerOnce, NULL, NULL);
        if (kl520_fdfr_drawing_timer)
        {
            if(osTimerIsRunning(kl520_fdfr_drawing_timer))
            {
                osTimerStop(kl520_fdfr_drawing_timer);
            }
            osTimerStart(kl520_fdfr_drawing_timer, KL520_FDFR_KEEP_MS);
        }
    }
    if(KL520_FACE_FAIL != face_status)
    {
        kl520_fdfr_drawing_timer_flag = face_status;
    }
#endif
#endif
}

static void _kl520_fdfr_drawing_timer_delete(void)
{
#ifdef USE_FDFR_DRAWING_TIMER
    if (kl520_fdfr_drawing_timer)
    {
        osTimerDelete(kl520_fdfr_drawing_timer);
        kl520_fdfr_drawing_timer = 0;
    }
    kl520_fdfr_drawing_timer_flag = KL520_FACE_FAIL;
#endif
}

void _api_fdfr_rst_timeout(void)
{
    kl520_api_timer_init(PWMTIMER3, PWMTMR_1000MSEC_PERIOD);
    m_fdfr_tick = kdp_current_t3_tick();
}

void _api_fdfr_close_timeout(void)
{
    kl520_api_timer_close(PWMTIMER3);
}

bool _api_fdfr_chk_timeout(int timeout)
{
    bool ret = FALSE;

    static int _nShowimerPre = -1;

    int _nShowimerCur = -1;

    u32 curr_tick = kdp_current_t3_tick();

    _nShowimerCur = curr_tick - m_fdfr_tick;

    if ( _nShowimerCur >= timeout)
    {
        ret = TRUE;
    }

    if(_nShowimerPre != _nShowimerCur){
        dbg_msg_api("Timer Tick=%d/%d",_nShowimerCur,timeout);
    }

    _nShowimerPre = _nShowimerCur;

    return ret;
}

void _api_fdfr_measure_rec_str(void)
{
    #if (MEASURE_RECOGNITION == YES)
    {
        static BOOL api_thrd_fdfr_rec_flag = FALSE;
        if (!api_thrd_fdfr_rec_flag) {
        kl520_measure_stamp(E_MEASURE_THR_FDFR_REC_STR);
        api_thrd_fdfr_rec_flag = TRUE;
        }
    }
    #endif
}


//u32 _api_fdfr_ckeck_position(void)
//{
//    u32 _fdfr_state = FDFR_STATUS_OK;
//    s32 x, y, w, h;
//    if( kdp_e2e_util_get_person_position(&x, &y, &w, &h) == 1)
//    {
//        struct video_input_params params;
//        kdp_video_renderer_get_params(&params);
//        int dp_w = (int)params.dp_out_w;
//        int dp_h = (int)params.dp_out_h;
//        int img_w = (int)params.dp_area_w;
//        int img_h = (int)params.dp_area_h;

//        x = x * dp_w / img_w;
//        y = y * dp_h / img_h;

//        dbg_msg_user("[FACE_POS] (x, y)= (%d, %d)", x, y);

//        if( y < FACE_POSTITON_UP_THRESH )
//            _fdfr_state = FDFR_STATUS_TOOUP;
//        else if(y > FACE_POSTITON_DOWN_THRESH)
//            _fdfr_state = FDFR_STATUS_TOODOWN;
//        else if(x < FACE_POSTITON_LEFT_THRESH)
//            _fdfr_state = FDFR_STATUS_TOOLEFT;
//        else if(x > FACE_POSTITON_RIGHT_THRESH)
//            _fdfr_state = FDFR_STATUS_TOORIGHT;
//    }

//    return _fdfr_state;
//}

u32 _api_fdfr_ckeck_move(void)
{
    static u16 prev_x = 0;
    static u16 prev_y = 0;

    u32 _fdfr_state = FDFR_STATUS_OK;
    s32 x, y, w, h;

    if( kdp_e2e_util_get_person_position(&x, &y, &w, &h) == 1){

        if( is_face_moved_thresh(prev_x, prev_y, (u16)x, (u16)y) == 1 ) {
            prev_x = x;
            prev_y = y;
            face_non_move_cnt = 1;
        } else {
            face_non_move_cnt++;
        }

        if( (kdp_e2e_prop_get2(flow_mode) > FLOW_MODE_SIM_VIRTUAL ) )
        {
            face_non_move_cnt = FACE_NOT_MOVE_CNT;
        }

        dbg_msg_user("face_non_move_cnt=%d", face_non_move_cnt);

        if(face_non_move_cnt < FACE_NOT_MOVE_CNT ) {
            _fdfr_state = FDFR_STATUS_WAIT_DONT_MOVE;
        }
    }

    return _fdfr_state;
}


void _api_fdfr_set_event(u32 flags_api_fdfr_state, u32 kl520_app_flag_state, bool CLR)
{
    if(osThreadFlagsGet() == flags_api_fdfr_state){
        set_event(kl520_api_get_event(), kl520_app_flag_state);
    }

    if(CLR == TRUE){
        osThreadFlagsClear(flags_api_fdfr_state);
    }
}

#if 0
u8 _fdfr_add_util_check(void)
{
    if (FACE_ADD_MODE_5_FACES == m_face_add_mode && dp_draw_info.e2e_dist_type != FDFR_STATUS_NORMAL)
    {
        return  dp_draw_info.e2e_dist_type;
    }

#if (FACE_REG_CHECK_POSITION == YES)
    if(dp_draw_info.e2e_pos_type != FDFR_STATUS_NORMAL)
    {
        return  dp_draw_info.e2e_pos_type;
    }
#endif
    
    return FDFR_STATUS_NORMAL;
}

u8 _fdfr_recognition_util_check(void)
{
    if (dp_draw_info.e2e_dist_type != FDFR_STATUS_NORMAL)
    {
        return  dp_draw_info.e2e_dist_type;
    }

#if (FACE_REG_CHECK_POSITION == YES)
    if(dp_draw_info.e2e_pos_type != FDFR_STATUS_NORMAL)
    {
        return  dp_draw_info.e2e_pos_type;
    }
#endif

    return FDFR_STATUS_NORMAL;
}
#endif

u8 _fdfr_recognition_note_check(void)
{
    static uint32_t nofaceTime = 0;
  
    if( ABS( (s32)(osKernelGetTickCount() - nofaceTime) ) < 100){ return FDFR_STATUS_NORMAL;}
    else{ nofaceTime = osKernelGetTickCount();} 
    
    if((dp_draw_info.e2e_pos.x == 0 && dp_draw_info.e2e_pos.y == 0) ||
        ((dp_draw_info.e2e_ret & E2E_ERROR_FD_MODEL) == E2E_ERROR_FD_MODEL))
    {
        return  FDFR_STATUS_NOFACE;
    }
    if( dp_draw_info.e2e_mask_type != FDFR_STATUS_NORMAL)
    {
        return  dp_draw_info.e2e_mask_type;
    }
#if (FACE_REG_CHECK_POSITION == YES)
    if(dp_draw_info.e2e_pos_type != FDFR_STATUS_NORMAL )
    {   
        return  dp_draw_info.e2e_pos_type;
    }
#endif
#ifdef FACE_REG_CHECK_EYE
    if(dp_draw_info.e2e_eye_type != FDFR_STATUS_NORMAL)
    {
        return  dp_draw_info.e2e_eye_type;
    }
#endif

    u32 dist_type = _api_fdfr_check_dist();
    if (dist_type != FDFR_STATUS_NORMAL ) {
        return dist_type;
    }
    
    return FDFR_STATUS_NORMAL;
}

//api_fdfr_thrd variables
static u8 fmap_index;

static void api_fdfr_handle_face_open_event(struct api_fdfr_context *ctx)
{
    face_non_move_cnt = 0;
    face_attack_cnt = 0;
    fmap_index = 0;
    
    face_reg_sts = 0;
    ctx->state = FDFR_STATUS_OPENED;

    kl520_fdfr_drawing_timer_flag = KL520_FACE_FAIL;
    m_curr_user_id = 0xFF;
    osDelay(KL520_FDFR_SKIP_MS);
    
    _api_fdfr_rst_timeout();

#if (CFG_E2E_REC_NOTE == YES && KL520_REC_EYEMODE == YES && CFG_E2E_STRUCT_LIGHT == YES ) 
    kl520_face_recognition_eye_mode(FDFR_STATUS_IDLE, E2E_OK);
#endif			
    osThreadFlagsClear(FLAGS_API_FDFR_OPEN_EVT);
    
    _fdr_opened = 1;
    if(kl520_is_fdfr_abort() != 0) { //abort is ongoing
        osEventFlagsId_t evt = get_user_com_event_id();
        if(evt != NULL) osEventFlagsSet(evt, 2 /* USER_COM_EVENT_FDR_OPENED */);
        return;
    }

#if (KL520_QUICK_BOOT == YES)
    if (FACE_MODE_ADD == m_face_mode || m_face_mode == FACE_MODE_ENG_CAL)
        set_thread_event(ctx->tid, FLAGS_API_FDFR_ADD_EVT);
    else if (FACE_MODE_RECOGNITION == m_face_mode)
        set_thread_event(ctx->tid, FLAGS_API_FDFR_RECOGNITION_EVT);
    else if (FACE_MODE_RECOGNITION_TEST == m_face_mode)
        set_thread_event(ctx->tid, FLAGS_API_FDFR_RECOGNITION_TEST_EVT);
    else if (FACE_MODE_LIVENESS == m_face_mode)
        set_thread_event(ctx->tid, FLAGS_API_FDFR_LIVENESS_EVT);
    else if (FACE_MODE_SNAP_IMG == m_face_mode)
        set_thread_event(ctx->tid, FLAGS_API_FDFR_SNAPIMG_EVT);
//    else if (FACE_MODE_NONE == m_face_mode)
//    {
//        set_thread_event(ctx->tid, FLAGS_API_FDFR_CLOSE_EVT);
//        set_event(kl520_api_get_event(), KL520_APP_THREAD_ERR);
//    }

#if CFG_COMPARE_1VS1 == YES
    else if (FACE_MODE_COMPARE_1VS1 == m_face_mode)
        set_thread_event(ctx->tid, FLAGS_API_FDFR_COMPARE_1VS1_EVT);
#endif
    else
    {
        set_thread_event(ctx->tid, FLAGS_API_FDFR_CLOSE_EVT);
    }
    
#else
    set_event(evt_face_op_ack_id, FLAGS_FDFR_ACK);
#endif
    return;
}

static void api_fdfr_face_add_normal_face(struct api_fdfr_context *ctx, u8 reg_idx, u8 user_idx)
{
    dbg_msg_algo("FACE_ADD_TYPE_NORMAL");

#ifdef COMPARE_BEFORE_ADD_DISABLE
    u16 user_id = 0;
    u32 ret = 0; 
    
    if (kdp_e2e_prop_get2(face_mode) != FACE_MODE_ENG_CAL) {
        float thres_arr[3] = {KL520_DEFAULT_DB_THR_ENVIR_DIFF_0, KL520_DEFAULT_DB_THR_ENVIR_DIFF_1, KL520_DEFAULT_DB_THR_ENVIR_DIFF_2};
        if(get_enroll_overwrite_flag() == 0) {
            ret = kdp_e2e_db_compare(&user_id, thres_arr);
        } else {
            ret = KAPP_DB_NO_MATCH;
        }
        if(ret == KAPP_DB_FAIL_ABORT) return;
    } else {
        ret = E2E_INVALID_CAL;
    }
    
    if ((E2E_OK == ret) || (ret == E2E_STATUS_FR_MODEL_FLIP_0)) { //flip0 means less than threshold
        dbg_msg_api("matched user_id = %d", user_id);
        _kl520_fdfr_drawing_timer_create(KL520_FACE_EXIST);
        ctx->state = FDFR_STATUS_EXIST;
			 //zcy add for return user id
				m_curr_user_id = user_id;
			
        _api_fdfr_set_event(FLAGS_API_FDFR_ADD_EVT, KL520_APP_FLAG_FDFR_ERR, TRUE);
        return; //exit
    }
    else
#endif
    {
        u8 user_id;
#if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE
        if( m_face_db_add_mode == FACE_ADD_MODE_NO_DB )
        {
            user_id = _kl520_app_calc_db_uid(MAX_USER);
        }
        else
#endif
        {
            user_id = _kl520_app_calc_db_uid(user_idx);
            if(is_enroll_customize_uid() == 1) {
                //set valid, id, fm0, fm1
                kdp_app_db_set_last_register_id_preprocess(user_idx, FALSE );
            }
        }

        if (KL520_FACE_OK == _kl520_api_fdfr_add(user_id, reg_idx + 1)) { // fmap start from 1
            m_curr_user_id = user_id;
            if(FACE_ADD_MODE_1_FACE == m_face_add_mode)
                _kl520_fdfr_drawing_timer_create(KL520_FACE_OK);
            ctx->state = FDFR_STATUS_OK;
            fmap_index++;
            _api_fdfr_set_event(FLAGS_API_FDFR_ADD_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);

            face_reg_sts |= 0x01 << FACE_ADD_TYPE_NORMAL;

#if ( KL520_FACE_ADD_BMP == YES )
            face_succ_index = FACE_ADD_TYPE_NORMAL;
#endif

#if (KL520_API_FACE_NORMAL_RST_TIMEOUT == YES)
            _api_fdfr_rst_timeout();
#endif
            return;
        }
    }
    return;
}

static void api_fdfr_face_add_other_face(struct api_fdfr_context *ctx, u8 reg_idx, u8 user_idx)
{
    //other pose,
    dbg_msg_api("other pose, cal distance with normal face");
    u8 user_id;
    
    u8 five_face_all_done = (0x01 << FACE_ADD_TYPE_NORMAL) | (0x01 << FACE_ADD_TYPE_LEFT) | (0x01 << FACE_ADD_TYPE_UP) \
                            | (0x01 << FACE_ADD_TYPE_RIGHT) | (0x01 << FACE_ADD_TYPE_DOWN);

#if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE
    if( m_face_db_add_mode == FACE_ADD_MODE_NO_DB )
    {
        user_id = _kl520_app_calc_db_uid(MAX_USER);
    }
    else
#endif
    {
        user_id = _kl520_app_calc_db_uid(user_idx);
    }

    if ( (fmap_index != 0) &&
        (E2E_OK == kdp_e2e_db_compare_self( user_id, reg_idx, kl520_api_face_get_reserve_db_num()) ) )
    {
        dbg_msg_algo ("face add, self compare ok");
        // 0: Don't write db; 1: Write db; 2: Write db and release mutex
//        if (((FACE_ADD_MODE_5_FACES == m_face_add_mode) && ( fmap_index == FACE_ADD_TYPE_DOWN ))
//            || (FACE_ADD_MODE_1_FACE == m_face_add_mode))
//        { }
        {
            if (KL520_FACE_OK == _kl520_api_fdfr_add(user_id, fmap_index + 1))
            { // fmap start from 1
                m_curr_user_id = user_id;

                face_reg_sts |= 0x01 << reg_idx;

    #if ( KL520_FACE_ADD_BMP == YES )
                face_succ_index = reg_idx;
    #endif
                
                //dbg_msg_algo("adding face:%d, %d, %x.", ret_idx, reg_idx, face_reg_sts);
                if( (face_reg_sts & five_face_all_done) == five_face_all_done )
                {
                    //face_reg_sts = 0;
                    //kl520_api_dp_five_face_disable();
                    _kl520_fdfr_drawing_timer_create(KL520_FACE_OK);
                    dbg_msg_api("Five Face Done");
                }

                ctx->state = FDFR_STATUS_OK;
                fmap_index++;
                _api_fdfr_set_event(FLAGS_API_FDFR_ADD_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);
                //dbg_msg_algo("yg - registered other face reg idx:%d, fmap:%d.", reg_idx, fmap_index);
                return;
            }
            else
            {
                ctx->state = FDFR_STATUS_ERROR;
                _api_fdfr_set_event(FLAGS_API_FDFR_ADD_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);
                return;
            }
        }
    } else {
        dbg_msg_algo ("face add, self compare failed");
        if ( (kdp_e2e_prop_get2(flow_mode) > FLOW_MODE_SIM_VIRTUAL) )
        {
            ctx->state = FDFR_STATUS_ERROR;
            _api_fdfr_set_event(FLAGS_API_FDFR_ADD_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);
            return;
        }
    }
}

static void api_fdfr_handle_face_add_error(struct api_fdfr_context *ctx, int fdr_result)
{
    if ( (kdp_e2e_prop_get2(flow_mode) > FLOW_MODE_SIM_VIRTUAL) )
    {
        ctx->state = FDFR_STATUS_ERROR;
        _api_fdfr_set_event(FLAGS_API_FDFR_ADD_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);
        return;
    }

    if(KL520_FACE_MASK == fdr_result)
    {
        ctx->state = FDFR_STATUS_MASK;
        _api_fdfr_set_event(FLAGS_API_FDFR_ADD_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);
        //osDelay(KL520_FDFR_KEEP_MS);
        return;
    }
    else if(KL520_FACE_CALLIB_FAIL == fdr_result)
    {
        ctx->state = FDFR_STATUS_CALLIB_FAIL;
        _api_fdfr_set_event(FLAGS_API_FDFR_ADD_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);
     
        osDelay(KL520_FDFR_KEEP_MS);
        return;
    }
    else if(KL520_FACE_LOW_QUALITY  == fdr_result)
    {
        ctx->state = FDFR_STATUS_LOW_QUALITY;               // Remind the user to come closer
        _api_fdfr_set_event(FLAGS_API_FDFR_ADD_EVT, KL520_APP_FLAG_FDFR_ERR, TRUE);
        return;
    }
    else if(KL520_FACE_WAIT_DONT_MOVE == fdr_result) 
    {
        //dbg_msg_algo ("dont move....");
        ctx->state = FDFR_STATUS_WAIT_DONT_MOVE;
        _api_fdfr_set_event(FLAGS_API_FDFR_ADD_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);
    }
    else if(KL520_FACE_BADPOSE == fdr_result) 
    {
        //dbg_msg_algo ("bad pose....:%d, %d", ctx->state, bad_pose_cnt);
        //if (FACE_ADD_TYPE_NORMAL == m_api_fdfr_ctx.reg_idx)
        //    face_non_move_cnt = 0;

        if(ctx->state == FDFR_STATUS_BAD_POSE) {
            bad_pose_cnt++;
        } else {
            bad_pose_cnt = 0;
            ctx->state = FDFR_STATUS_BAD_POSE;
        }
#if (CFG_LED_CTRL_ENHANCE == 1)
#if (CFG_CAMERA_ROTATE != 1)
        kdp_e2e_nir_led_flag_off();
        for (int i = 0; i < 40; i++) {
            if (kl520_is_fdfr_abort() != 0) break;
            osDelay(5);
        }
#else
        // if(bad_pose_cnt % FACE_BAD_POSE_LED_CNT == 1) {
        //     kdp_e2e_nir_led_flag_off();
        //     for (int i = 0; i < 40; i++) {
        //         if (kl520_is_fdfr_abort() != 0) break;
        //         osDelay(5);
        //     }
        // }
#endif
#endif

#ifdef SKIP_SOME_BAD_POSE_EVENT
        if(bad_pose_cnt >= FACE_BAD_POSE_CNT)
#endif
        {
            bad_pose_cnt = 0;
            _api_fdfr_set_event(FLAGS_API_FDFR_ADD_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);
            return; //exit here
        }
    }
    else if(KL520_FACE_TOO_FAR == fdr_result || KL520_FACE_TOO_NEAR == fdr_result) 
    {
        //dbg_msg_algo("face dist too far or too near");
        if(KL520_FACE_TOO_FAR == fdr_result) ctx->state = FDFR_STATUS_TOO_FAR;
        else ctx->state = FDFR_STATUS_TOO_NEAR;
        _api_fdfr_set_event(FLAGS_API_FDFR_ADD_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);
        return;
    }
    else if(fdr_result >= KL520_FACE_TOOUP && fdr_result <= KL520_FACE_TOORIGHT)
    {
        ctx->state = (enum kl520_fdfr_status_code_e)(FDFR_STATUS_TOOUP + (fdr_result - KL520_FACE_TOOUP));
        _api_fdfr_set_event(FLAGS_API_FDFR_ADD_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);
        return;
    }
    else if(KL520_FACE_NOFACE == fdr_result)
    {
        ctx->state = FDFR_STATUS_NOFACE;
        _api_fdfr_set_event(FLAGS_API_FDFR_ADD_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);
        return;
    }
    else if(KL520_FACE_ATTACK == fdr_result) 
    {
        if(face_attack_cnt >= FACE_ATTACK_CNT)
        {
            dbg_msg_algo ("attack in face add");
            face_attack_cnt = 0;
            ctx->state = FDFR_STATUS_ATTACK;
            _api_fdfr_set_event(FLAGS_API_FDFR_ADD_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);
            return; //exit here
        } else {
            face_attack_cnt++;
        }
    }
}

static void api_fdfr_handle_face_add_event(struct api_fdfr_context *ctx)
{
    int fdr_result;
    u8 reg_idx = m_api_fdfr_ctx.reg_idx;
    dbg_msg_algo("[%s]reg_idx: %d", __func__, reg_idx);
    
    kdp_e2e_face_variables *face_var = kdp_e2e_get_face_variables();
    face_var->_is_fm_import = 0;

#if ( KL520_FACE_ADD_BMP == YES )
    if(face_add_bitmap & 0x80) { //first time of calling face add
        _api_fdfr_rst_timeout();
        face_add_bitmap &= ~(u8)0x80;
    }
#endif
    
    if (_api_fdfr_chk_timeout(m_face_add_timeout) == TRUE)
    {
        if (kdp_e2e_prop_get2(face_mode) != FACE_MODE_ENG_CAL) {
            dbg_msg_console("[FACE_ADD] timeout");
        } else {
            dbg_msg_console("Engineering calibration timeout");
        }
        
        kdp_e2e_nir_led_flag_off();

#if (CFG_GUI_ENABLE == YES)
        kl520_pose_timer_delete();
#endif
        ctx->state = FDFR_STATUS_TIMEOUT;

        _api_fdfr_set_event(FLAGS_API_FDFR_ADD_EVT, KL520_APP_FLAG_FDFR_TIMEOUT, TRUE);
        return;
    }


#if (HOST_DEV_COM_PROTO_TYPE == 1)
    if ( g_bImpFmDataReady )
    {
        //Bypass
        fdr_result = KL520_FACE_OK;
        g_bImpFmDataReady = FALSE;
        kl520_api_ap_com_import_fm_r1_inject();
    }
#elif ( HOST_DEV_COM_PROTO_TYPE == 2 )
    if ( g_tImpExpMassDataPkgInfo.nReadyType == DATA_READY_TYPE_IMP_FM )

    {
        //Bypass
        fdr_result = KL520_FACE_OK;
        face_var->_is_fm_import = 1;

        KDP_imp_fm_inejct_data();
        kdp_e2e_prop_update_db_comp_params();//zcy note imp fm
    }
#else
    if(0) { }
#endif
    else
    {
        fdr_result = kl520_api_fdfr_element();
    }

    if (KL520_FACE_OK == fdr_result)
    {   
        u8 user_idx = 0xff;
        if(is_enroll_customize_uid() == 0) {
            user_idx = kl520_api_face_query_first_avail_user_idx();
        } else {
            user_idx = kdp_app_db_get_db_idx(get_enroll_customize_uid());
        }
        if(reg_idx == FACE_ADD_TYPE_NORMAL)
        {
//            if(_oms_overwrite_db_uid != 0xff) {
//                //set valid, id, fm0, fm1
//                kdp_app_db_set_last_register_id_preprocess(user_idx, FALSE );
//            }
            //for normal,
            api_fdfr_face_add_normal_face(ctx, reg_idx, user_idx);
        } else {
            //for other
            api_fdfr_face_add_other_face(ctx, face_hp_check_idx, user_idx);
        }
        _api_fdfr_thread_data_prepare();
    }
    else
    {
        api_fdfr_handle_face_add_error(ctx, fdr_result);
    }
    return;
}


void api_fdfr_face_recognition_set_mandatory_event(void)
{
    g_bRecognitionMandatoryFlag = TRUE;
}


extern float g_nFaceL2;
void fdfr_update_fr_thr(u8 user_id)
{
    kdp_e2e_face_variables *vars_cur = kdp_e2e_get_face_variables();

//    kdp_e2e_db_extra_data tmp;
//    kdp_e2e_db_extra_data *vars_db = &tmp;
//    kdp_e2e_db_extra_read(kdp_app_db_find_exist_id(user_id), vars_db, sizeof(*vars_db));
    
    u16 valid_fm0 = 0, valid_fm1 = 0, type = 0;
    s32 r1 = kdp_e2e_db_get_user_info(user_id, &valid_fm0, &valid_fm1, &type);
    if(r1 != E2E_OK) return;
    
    u8 fmap = 0;

#if (CFG_FLASH_DB_NIR_ONLY == NO)    
    if( valid_fm0 == 1 && valid_fm1 == 1)   fmap = 1;
    else if( valid_fm0 == 5 && valid_fm1 == 5)  fmap = 5;
    else if( valid_fm0 == 1 )   fmap = 1;
#else
    if(valid_fm0 == 1 || valid_fm0 == 5) fmap = valid_fm0;
#endif
    
    if( vars_cur->pre_add == AI_TYPE_UPDATE_FMAP)
    {
        vars_cur->pre_add = CFG_AI_TYPE;
                
        u16 find_id_idx = kdp_app_db_find_exist_id(user_id);
        kdp_app_db_set_last_register_id_preprocess(find_id_idx, FALSE );
        kl520_api_face_set_add_mode(FACE_ADD_MODE_1_FACE);

        if (KL520_FACE_OK == _kl520_api_fdfr_add(user_id, fmap))
        {
            dbg_msg_algo("[update rgb user db]user_id = 0x%x, fmap=%d",user_id, fmap);
        }
    }
#if (USE_RECOG_FR_UPDATE == YES)
    else
    {
        if(fmap == 5)
        {
            u32 tick = osKernelGetTickCount();
            if(tick % 10 != 3) return; //update once every ten times

            //dbg_msg_algo ("update db, nir mode:%d, fq:%.3f, min dist:%.3f.",\
                vars_cur->nir_mode, fq->face_score, g_nFaceL2);
            if(vars_cur->nir_mode != IR_FEW) return; //only indoor mode

            if(g_nFaceL2 > 0.6f) return; //if distance is too big

#if (USE_RGB_FR_IN_1054 == YES)
            if(vars_cur->rgb_nir_fr_flag == 1) return; //if using rgb for fr.
            if(vars_cur->face_pose_cur_val < 1.0f) return; //if pose not very good
#endif

            //check face quality
#if (CFG_REUSE_PREPROC == 1)
            struct face_quality_result_s* fq = kdp_e2e_get_face_quality();
            struct facedet_result_s *n1_fd = kdp_e2e_get_n1_fd();
            struct landmark_result_s *n1_lm = kdp_e2e_get_n1_lm();
            struct kdp_img_cfg *img_cfg = kdp_e2e_get_img_cfg(MIPI_CAM_NIR);
            kl520_measure_stamp(E_MEASURE_NIR_QUALITY_STR);
            int ret = kdp_model_face_quality(fq, img_cfg, n1_fd, n1_lm);
            kl520_measure_stamp(E_MEASURE_NIR_QUALITY_END);
            if(ret != KDP_MODEL_OK) return;
#else
            struct face_quality_result_s* fq = kdp_e2e_get_face_quality();
#endif
            if(fq->face_score < 0.5f) return; //if quality is not good

            u16 find_id_idx = kdp_app_db_find_exist_id(user_id);
            if(find_id_idx >= MAX_USER) return; //if not a valid user.

            //dbg_msg_algo("update db for :%d.", find_id_idx);
            if( KL520_FACE_OK == _kl520_api_fr_fmap_update_db(find_id_idx, fmap) ) { //update 5th
                dbg_msg_algo("db update ok user_id = 0x%x", user_id);
            }
        }
    }

#endif
}

void fdfr_update_fr_thread(void *arg){

    fdfr_update_fr_thr(m_curr_user_id);
    //dbg_msg_algo("db update ok, thread exit,  user_id = 0x%x",m_curr_user_id);
    tid_fdfr_update_fr = NULL;
    osThreadExit();
}

void fdfr_update_fr_entry(void)
{
    osThreadAttr_t attr = {
        .stack_size = 1280
    };
    
    if((tid_fdfr_update_fr == NULL) && (FALSE == g_bPowerDown))
        tid_fdfr_update_fr = osThreadNew(fdfr_update_fr_thread, NULL, &attr);

}

static void _api_fdfr_handle_face_recog_error(struct api_fdfr_context *ctx, int fdr_result)
{
    if (KL520_FACE_INVALID == fdr_result)
    {
        ctx->state = FDFR_STATUS_INVALID;
#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
        kl520_api_snapshot_adv_shot_after_fdfr_element();
#endif
        _api_fdfr_set_event(FLAGS_API_FDFR_RECOGNITION_EVT, KL520_APP_FLAG_FDFR_ERR, TRUE);
        kdp_e2e_nir_led_flag_off();
        // dbg_msg_console("KL520_FACE_INVALID");
    }
    else if (KL520_FACE_DETECTED == fdr_result) {
        ctx->state = FDFR_STATUS_DETECTED;
        m_face_appear = TRUE;
        dbg_msg_algo("KL520_FACE_DETECTED");
    }
    else if (KL520_FACE_MASK == fdr_result)
    {
        ctx->state = FDFR_STATUS_MASK;
        m_face_appear = TRUE;

        _api_fdfr_set_event(FLAGS_API_FDFR_RECOGNITION_EVT, KL520_APP_FLAG_FDFR_ERR, FALSE);

        //osDelay(KL520_FDFR_KEEP_MS);
    }
    else if(KL520_FACE_LOW_QUALITY  == fdr_result)
    {
        ctx->state = FDFR_STATUS_LOW_QUALITY;
        _api_fdfr_set_event(FLAGS_API_FDFR_RECOGNITION_EVT, KL520_APP_FLAG_FDFR_ERR, FALSE);
    }

    if ((_api_fdfr_chk_timeout(m_face_recognition_timeout) == TRUE) || (kdp_e2e_prop_get2(flow_mode) > FLOW_MODE_SIM_VIRTUAL) )
    {
        if(m_face_appear)
        {
            dbg_msg_console("[FACE_RECOGNITION] timeout");
            ctx->state = FDFR_STATUS_TIMEOUT;
            //osDelay(KL520_FDFR_KEEP_MS);
            _api_fdfr_set_event(FLAGS_API_FDFR_RECOGNITION_EVT, KL520_APP_FLAG_FDFR_TIMEOUT, TRUE);

        }
        else
        {
            dbg_msg_console("[FACE_RECOGNITION] timeout(no face)");
            ctx->state = FDFR_STATUS_NOFACE_AND_TIMEOUT;
            _api_fdfr_set_event(FLAGS_API_FDFR_RECOGNITION_EVT, KL520_APP_FLAG_FDFR_ERR, TRUE);

        }
        kdp_e2e_nir_led_flag_off();

#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
        kl520_api_snapshot_adv_shot_after_fdfr_element();
#endif
        return;
    }
    else if ( fdr_result == KL520_FACE_DETECTED )
    {
#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
        kl520_api_snapshot_adv_shot_after_fdfr_element();
#endif
    }
    return;
}

static void _api_fdfr_handle_fr_db_match(struct api_fdfr_context *ctx)
{
    u32 e2e_ret;
    u16 user_id;
    int fdr_result = KL520_FACE_DB_FAIL;

#if CFG_FMAP_AP_CTRL_TYPE == EX_FM_USB_AP_CTRL_ALL && CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_EXTRA_ENABLE == YES
    if( KL520_api_ap_com_get_extra_fmap_status() == USB_CMD_FM_RECOGNIZE_e ){
        e2e_ret = kdp_api_ap_com_wait_host_compare_result(&user_id);
    }
    else
#endif
    {
        kl520_measure_stamp(E_MEASURE_FACE_DB_COMP_STR);

#if KL520_REC_EYEMODE == YES  
        if( get_eye_mode_status() == 1 )
        {// already in eye mode
            e2e_ret = E2E_OK;
        }
        else
#endif
        {
            float thres_arr[3] = {KL520_DEFAULT_DB_THR_ENVIR_DIFF_0, KL520_DEFAULT_DB_THR_ENVIR_DIFF_1, KL520_DEFAULT_DB_THR_ENVIR_DIFF_2};
            e2e_ret = kdp_e2e_db_compare(&user_id, thres_arr);
#if ((CFG_AI_TYPE == AI_TYPE_N1R1) && (USE_FR_FACE_FLIP == YES) )
            if (e2e_ret == E2E_STATUS_FR_MODEL_FLIP_0 || e2e_ret == E2E_STATUS_FR_MODEL_FLIP_1) {
                dbg_msg_algo ("retrying fr for face flip...");
                u32 fr_ret = kdp_e2e_face_n1r1_postproc(1);
                if (E2E_OK == fr_ret) {
                    e2e_ret = kdp_e2e_db_compare_one_user(user_id);
                } else {
                    if(e2e_ret == E2E_STATUS_FR_MODEL_FLIP_0) e2e_ret = E2E_OK;
                }
            }
#endif
        }
        kl520_measure_stamp(E_MEASURE_FACE_DB_COMP_END);
    }
    if (E2E_OK == e2e_ret) {
        kdp_e2e_face_variables *vars_cur = kdp_e2e_get_face_variables();
        fdr_result = KL520_FACE_DB_OK;
#if KL520_REC_EYEMODE == YES                     
        if( get_eye_mode_status() == 0 ) 
#endif                        
        { // no eye mode, update id
            m_curr_user_id = (u8)user_id;
        }

        fdfr_update_fr_entry();
        
        dbg_msg_console("[PASS] KL520_FACE_DB_OK (UseID = %#x, Admin = %d, UseName = %s, fdr_result = %#x)", m_curr_user_id, vars_cur->admin, vars_cur->user_name, fdr_result);

#if KL520_REC_EYEMODE == YES
        u8 face_mode = kl520_face_recognition_eye_mode(dp_draw_info.e2e_eye_type, dp_draw_info.e2e_ret);
        if( get_eye_mode_status() == 1 )
        {
            dbg_msg_console("[Eye Mode] kl520_face_recognition_note" );                            
            ctx->state = (enum kl520_fdfr_status_code_e)face_mode;
            
        }
        else 
        {
            if( ctx->state == FDFR_STATUS_EYE_CLOSE_STATUS_OPEN_EYE ||
                ctx->state == FDFR_STATUS_EYE_CLOSED ||
                ctx->state == FDFR_STATUS_EYE_CLOSE_UNKNOW_STATUS
            ) {
                // keep state;
            }
            else
            {
                ctx->state = FDFR_STATUS_OK;
            }

#ifndef DB_DRAWING_CUSTOMER_COLOR
            _kl520_fdfr_drawing_timer_create(KL520_FACE_DB_OK);
#endif   
#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
            kl520_api_snapshot_adv_shot_after_fdfr_element();
#endif
            kl520_measure_stamp(E_MEASURE_EVENT_SEND_OK);
            _api_fdfr_set_event(FLAGS_API_FDFR_RECOGNITION_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);
            return;

        }
#else
        ctx->state = FDFR_STATUS_OK;
#ifndef DB_DRAWING_CUSTOMER_COLOR
        _kl520_fdfr_drawing_timer_create(KL520_FACE_DB_OK);
#endif
#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
        kl520_api_snapshot_adv_shot_after_fdfr_element();
#endif
        kl520_measure_stamp(E_MEASURE_EVENT_SEND_OK);
        _api_fdfr_set_event(FLAGS_API_FDFR_RECOGNITION_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);

#ifdef DEV_TEST_VERSION
        _face_succ_count++;
#endif
        return;
#endif
    } else {
        if(KAPP_DB_POSE_ERR == e2e_ret) {
            return;
        } else {
            ++m_db_comp_fail_cnt;
            dbg_msg_console("[FAIL] KL520_FACE_DB_FAIL (counter = %u, max = %u)", m_db_comp_fail_cnt, KL520_DB_COMP_FAIL_ALLOWED_MAX);
            if ((m_db_comp_fail_cnt >= KL520_DB_COMP_FAIL_ALLOWED_MAX) || (kdp_e2e_prop_get2(flow_mode) > FLOW_MODE_SIM_VIRTUAL)) {
                m_curr_user_id = 0xFF;
    #if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
                kl520_api_snapshot_adv_shot_after_fdfr_element();
    #endif
    #ifndef DB_DRAWING_CUSTOMER_COLOR
                if ((kdp_e2e_prop_get2(flow_mode) < FLOW_MODE_SIM_VIRTUAL))
                {
                    _kl520_fdfr_drawing_timer_create(KL520_FACE_DB_FAIL);
                }
    #endif
                ctx->state = FDFR_STATUS_COMP_FAIL;
                _api_fdfr_set_event(FLAGS_API_FDFR_RECOGNITION_EVT, KL520_APP_FLAG_FDFR_ERR, TRUE);
                return;
            }
        }
    }
    m_face_appear = TRUE;

    return;
}

static void api_fdfr_handle_face_recognition_event(struct api_fdfr_context *ctx)
{
    dbg_msg_algo ("starting to recog....");
    _api_fdfr_measure_rec_str();
    kdp_e2e_settings* settings = kdp_e2e_settings_get_inst();
#if 0 //zcy add for test  recognise open led 2022-08-16

    if( settings->calibration_count == 0 && !g_bRecognitionMandatoryFlag && 
        !kl520_api_sim_is_running() ) {
        int db_num = kdp_e2e_db_get_total_num();
        if(db_num == 0) {
#ifdef CUSTOMER_FDFR_STATUS_ASSIGN_CHG
            ctx->state = FDFR_STATUS_COMP_FAIL;
#else
            ctx->state = FDFR_STATUS_EMPTY;
#endif
            _api_fdfr_set_event(FLAGS_API_FDFR_RECOGNITION_EVT, KL520_APP_FLAG_FDFR_ERR, TRUE);
            return;
        }
    }
#endif
    int fdr_result = kl520_api_fdfr_element();

#if (CFG_E2E_REC_NOTE == YES)
    if (KL520_FACE_OK != fdr_result )
    {
    
        u8 face_type = _fdfr_recognition_note_check();
        if(face_type != FDFR_STATUS_NORMAL &&
        face_type != FDFR_STATUS_EYE_CLOSED &&
        face_type != FDFR_STATUS_EYE_CLOSE_UNKNOW_STATUS && 
        face_type != FDFR_STATUS_EYE_CLOSE_STATUS_OPEN_EYE )
        {
#if KL520_REC_EYEMODE == YES  
            if(get_eye_mode_status() == 0)

#endif                    
            {
                ctx->state = (enum kl520_fdfr_status_code_e)face_type;
                kl520_face_recognition_note();
            }
        }
    }
#endif

#if KL520_REC_EYEMODE == YES  
    if (KL520_FACE_OK == fdr_result || get_eye_mode_status() == 1 )
#else
    if (KL520_FACE_OK == fdr_result )
#endif
    {
        _api_fdfr_handle_fr_db_match(ctx);

        if (KL520_FACE_OK == fdr_result ) {
            //prepare for cam capture.
            _api_fdfr_thread_data_prepare();
        }
    } else {
        //handle error status
        _api_fdfr_handle_face_recog_error(ctx, fdr_result);
    }
    return;
}

static void api_fdfr_handle_face_liveness_event(struct api_fdfr_context *ctx)
{
    int fdr_result = kl520_api_fdfr_element();
    if (KL520_FACE_OK == fdr_result)
    {
        _kl520_fdfr_drawing_timer_create(KL520_FACE_LIVENESS_OK);
        ctx->state = FDFR_STATUS_OK;
        _api_fdfr_set_event(FLAGS_API_FDFR_LIVENESS_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);
        _api_fdfr_thread_data_prepare();
        return;
    }
    else if (KL520_FACE_INVALID == fdr_result)
    {
        ctx->state = FDFR_STATUS_INVALID;
        _api_fdfr_set_event(FLAGS_API_FDFR_LIVENESS_EVT, KL520_APP_FLAG_FDFR_ERR, TRUE);
        dbg_msg_api("KL520_FACE_INVALID");
    }
    else if (KL520_FACE_MASK == fdr_result)
    {
        ctx->state = FDFR_STATUS_MASK;
        _api_fdfr_set_event(FLAGS_API_FDFR_LIVENESS_EVT, KL520_APP_FLAG_FDFR_ERR, FALSE);
//                dbg_msg_api("KL520_FACE_MASK");
    }
    else if(KL520_FACE_LOW_QUALITY  == fdr_result)
    {
        ctx->state = FDFR_STATUS_LOW_QUALITY;
        _api_fdfr_set_event(FLAGS_API_FDFR_LIVENESS_EVT, KL520_APP_FLAG_FDFR_ERR, FALSE);
    }

    if (_api_fdfr_chk_timeout(m_face_liveness_timeout) == TRUE)
    {
        dbg_msg_console("[FACE_LIVENESS] timeout");
        {
            ctx->state = FDFR_STATUS_TIMEOUT;
            _api_fdfr_set_event(FLAGS_API_FDFR_LIVENESS_EVT, KL520_APP_FLAG_FDFR_TIMEOUT, TRUE);
        }
        return;
    }
    return;
}

static void api_fdfr_handle_snap_image_event(struct api_fdfr_context *ctx)
{
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
#if defined KID_SNAP_IMAGE || defined KID_KN_SNAP_IMAGE
    int fdr_result = kl520_api_fdfr_element();
    if (KL520_FACE_OK == fdr_result)
    {
        ctx->state = FDFR_STATUS_OK;
        
        //save to ddr
        struct kdp_img_cfg *img_cfg = kdp_e2e_get_img_cfg(MIPI_CAM_RGB);
        kl520_api_save_to_snap_ddr(MIPI_CAM_RGB, img_cfg->image_mem_addr, img_cfg->image_mem_len);
        
        img_cfg = kdp_e2e_get_img_cfg(MIPI_CAM_NIR);
        kl520_api_save_to_snap_ddr(MIPI_CAM_NIR, img_cfg->image_mem_addr, img_cfg->image_mem_len);
        
        _api_fdfr_thread_data_prepare();
        _api_fdfr_set_event(FLAGS_API_FDFR_SNAPIMG_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);
    } else {
        dbg_msg_console("[FACE_SNAP_IMG] error");
        ctx->state = FDFR_STATUS_ERROR;
        _api_fdfr_set_event(FLAGS_API_FDFR_SNAPIMG_EVT, KL520_APP_FLAG_FDFR_ERR, TRUE);
    }
#endif
#endif
    return;
}

static void api_fdfr_handle_face_recognition_test_event(struct api_fdfr_context *ctx)
{
    int fdr_result = kl520_api_fdfr_element();
    //u16 dist = kl520_api_dp_layout_fd_hint(&h_boundingbox);
    //dbg_msg_api("distance = %u", dist);
    if (KL520_FACE_OK == fdr_result)
    {

#if (CFG_E2E_STRUCT_LIGHT == YES || CFG_E2E_NIR_TWO_STAGE_LIGHT == YES)
        //if( get_usb_catch_image_log() == 2 )
            kl520_api_snapshot_adv_shot_after_fdfr_element(); // liveness OK
#endif

        _kl520_fdfr_drawing_timer_create(KL520_FACE_LIVENESS_OK);
        ctx->state = FDFR_STATUS_OK;
        _api_fdfr_set_event(FLAGS_API_FDFR_RECOGNITION_TEST_EVT, KL520_APP_FLAG_FDFR_OK, FALSE);
        _api_fdfr_thread_data_prepare();
    }
    else
    {
        // _kl520_fdfr_drawing_timer_create(KL520_FACE_FAIL);
        ctx->state = FDFR_STATUS_ERROR;
    }

#if CFG_USB_EXPORT_STREAM_IMG == YES
#if CFG_USB_EXPORT_LIVENESS_RET == YES
    if (KL520_FACE_OK == fdr_result)
    {
        kl520_api_usb_set_export_liveness_ret(1);
    }
    else
    {
        kl520_api_usb_set_export_liveness_ret(0);
    }
#endif
#endif
    return;
}

static void api_fdfr_handle_face_close_event(struct api_fdfr_context *ctx)
{
    //dbg_msg_algo("handling face close event...");
    _api_fdfr_close_timeout();
    osEventFlagsClear(kl520_api_get_event(), KL520_APP_FLAG_FDFR);
    ctx->state = FDFR_STATUS_IDLE;
    osThreadFlagsClear(FLAGS_API_FDFR_CLOSE_EVT);
    osEventFlagsSet(_fdr_event_id, FDFR_COM_EVENT_CLOSED);
    return;
}

#if CFG_COMPARE_1VS1 == YES
static void api_fdfr_handle_face_compare_event(struct api_fdfr_context *ctx)
{
    static u32 _addr_first_r1 = NULL;
    static u32 _addr_first_n1 = NULL;
    static enum kl520_fdfr_status_code_e user_comp_status = FDFR_STATUS_INVALID;

    u32 e2e_ret;

    int fdr_result = kl520_api_fdfr_element();

    if( _addr_first_r1 == NULL )
    {
        _addr_first_r1 = kdp_ddr_reserve( sizeof( struct fr_result_s ) );
    }
    if( _addr_first_n1 == NULL && kdp_e2e_prop_get2(flow_mode) == FLOW_MODE_SIM_COMP_R1N1 )
    {
        _addr_first_n1 = kdp_ddr_reserve( sizeof( struct fr_result_s ) );
    }

    dbg_msg_api("[%s] fdr_addr_n1_1=0x%x", __func__, _addr_first_n1 );

    if (KL520_FACE_OK == fdr_result)
    {
        if( user_comp_status != FDFR_STATUS_NEXT )
        {
            user_comp_status = FDFR_STATUS_NEXT;
            ctx->state = FDFR_STATUS_NEXT;
            memcpy( (void*)_addr_first_r1, kdp_e2e_get_r1_fr(), sizeof( struct fr_result_s ) );

            if( _addr_first_n1 != NULL && kdp_e2e_prop_get2(flow_mode) == FLOW_MODE_SIM_COMP_R1N1 )
                memcpy( (void*)_addr_first_n1, kdp_e2e_get_n1_fr(), sizeof( struct fr_result_s ) );

            m_curr_user_id = 0xFF;
            _api_fdfr_set_event(FLAGS_API_FDFR_COMPARE_1VS1_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);

        }
        else
        {
            u8 SimilarityR1 = 0;
            u8 SimilarityN1 = 0;

            //do compare
            e2e_ret = kdp_e2e_db_2user_compare( &SimilarityR1, _addr_first_r1, kdp_e2e_get_r1_fr(), \
                                                &SimilarityN1, (kdp_e2e_prop_get2(flow_mode) == FLOW_MODE_SIM_COMP_R1N1)? _addr_first_n1:NULL, kdp_e2e_get_n1_fr() );

            m_curr_user_id = SimilarityR1;
            //dbg_msg_console(" m_curr_user_id = %d, %d ",m_curr_user_id, SimilarityR1);

            if( e2e_ret == E2E_OK )
            {
                user_comp_status = FDFR_STATUS_OK;
                ctx->state = FDFR_STATUS_OK;
                _api_fdfr_set_event(FLAGS_API_FDFR_COMPARE_1VS1_EVT, KL520_APP_FLAG_FDFR_OK, TRUE);
            }
            else
            {
                user_comp_status = FDFR_STATUS_INVALID;
                ctx->state = FDFR_STATUS_INVALID;
                _api_fdfr_set_event(FLAGS_API_FDFR_COMPARE_1VS1_EVT, KL520_APP_FLAG_FDFR_ERR, TRUE);
            }
        }
        _api_fdfr_thread_data_prepare();
    }
    else
    {
        user_comp_status = FDFR_STATUS_INVALID;
        ctx->state = FDFR_STATUS_INVALID;
        _api_fdfr_set_event(FLAGS_API_FDFR_COMPARE_1VS1_EVT, KL520_APP_FLAG_FDFR_ERR, TRUE);

    }
}
#endif

static void api_fdfr_thread(void *argument)
{
    struct api_fdfr_context *ctx = (struct api_fdfr_context*)argument;
    BOOL quit = FALSE;
    fmap_index = 0; //start from normal face
    _fdr_opened = 0;
    kdp_e2e_set_dual_cam_state(DUAL_IDENTICAL_CAM_CLOSED);
#ifdef DEV_TEST_VERSION
    _face_recog_count = 0;
    _face_succ_count = 0;
#endif

    face_hp_check_idx = 0xff;
    if (NULL == mutex_face_close) mutex_face_close = osMutexNew(NULL);

    kl520_measure_stamp(E_MEASURE_THR_FDFR_INIT_RDY);
    //kdp_e2e_db_init_thres();

    //dbg_msg_algo ("fdfr thread started....");
    while (!quit) {
        unsigned int flags = osThreadFlagsGet();
        if (flags & FLAGS_API_FDFR_CLOSE_EVT) {
            osThreadFlagsClear( FLAGS_API_FDFR_OPEN_EVT |
                                FLAGS_API_FDFR_ADD_EVT |
                                FLAGS_API_FDFR_RECOGNITION_EVT |
                                FLAGS_API_FDFR_LIVENESS_EVT |
                                FLAGS_API_FDFR_SNAPIMG_EVT |
                                FLAGS_API_FDFR_RECOGNITION_TEST_EVT |
                                FLAGS_API_FDFR_ABORT_EVT);
            flags = FLAGS_API_FDFR_CLOSE_EVT;
        }
        switch (flags) {
        case FLAGS_API_FDFR_OPEN_EVT:
            api_fdfr_handle_face_open_event(ctx);
            break;

        case FLAGS_API_FDFR_ADD_EVT:
            api_fdfr_handle_face_add_event(ctx);
            break;

        case FLAGS_API_FDFR_RECOGNITION_EVT:
            api_fdfr_handle_face_recognition_event(ctx);
            break;

        case FLAGS_API_FDFR_LIVENESS_EVT:
            api_fdfr_handle_face_liveness_event(ctx);
            break;

        case FLAGS_API_FDFR_RECOGNITION_TEST_EVT:
            api_fdfr_handle_face_recognition_test_event(ctx);
            break;
        
        case FLAGS_API_FDFR_SNAPIMG_EVT:
            api_fdfr_handle_snap_image_event(ctx);
            break;

        case FLAGS_API_FDFR_CLOSE_EVT:
            api_fdfr_handle_face_close_event(ctx);
            _fdr_opened = 0;

            break;

#if CFG_COMPARE_1VS1 == YES
        case FLAGS_API_FDFR_COMPARE_1VS1_EVT:
            api_fdfr_handle_face_compare_event(ctx);
            break;
#endif // CFG_COMPARE_1VS1

        default:
            osDelay(10);
            break;
        }

#if (KL520_QUICK_BOOT == YES)
        if (FLAGS_API_FDFR_OPEN_EVT != flags)
#endif
        {
            _api_fdfr_thread_rst_priority();
            osDelay(1);
        }
    }
    
    dbg_msg_algo ("fdfr thread exited....");

    osThreadExit();
}

void kl520_api_face_set_add_mode(kl520_face_add_mode mode)
{
    m_face_add_mode = mode;
}

kl520_face_add_mode kl520_api_face_get_add_mode(void)
{
    return m_face_add_mode;
}

void kl520_api_face_set_db_add_mode(kl520_face_db_add_mode mode)
{
    m_face_db_add_mode = mode;
}

int kl520_api_face_get_result(u8* face_id)
{
    int ret;
    *face_id = m_curr_user_id;
    switch (m_api_fdfr_ctx.state) {
    case FDFR_STATUS_OK:
        ret = KL520_FACE_OK;
        break;
    case FDFR_STATUS_TIMEOUT:
        ret = KL520_FACE_TIMEOUT;
        break;
    case FDFR_STATUS_COMP_FAIL:
        ret = KL520_FACE_DB_FAIL;
        break;
    case FDFR_STATUS_EXIST:
        ret = KL520_FACE_EXIST;
        break;
    case FDFR_STATUS_NOFACE:
        ret = KL520_FACE_NOFACE;
        break;
    case FDFR_STATUS_NOFACE_AND_TIMEOUT:
        ret = KL520_FACE_NOFACE_AND_TIMEOUT;
        break;
    case FDFR_STATUS_BAD_POSE:
        ret = KL520_FACE_BADPOSE;
        break;
    case FDFR_STATUS_FULL:
        ret = KL520_FACE_FULL;
        break;
    case FDFR_STATUS_WAIT_DONT_MOVE:
        ret = KL520_FACE_WAIT_DONT_MOVE;
        break;
    case FDFR_STATUS_TOO_NEAR:
        ret = KL520_FACE_TOO_NEAR;
        break;
    case FDFR_STATUS_TOO_FAR:
        ret = KL520_FACE_TOO_FAR;
        break;
    case FDFR_STATUS_INVALID:
        ret = KL520_FACE_INVALID;
        dbg_msg_api("[%s]state:%d", __func__, m_api_fdfr_ctx.state);
        break;
    case FDFR_STATUS_MASK:
        ret = KL520_FACE_MASK;
        break;
    case FDFR_STATUS_EYE_CLOSE_STATUS_OPEN_EYE:
        ret = KL520_FACE_EYE_CLOSE_STATUS_OPEN_EYE;
        break;
    case FDFR_STATUS_EYE_CLOSED:
        ret = KL520_FACE_EYE_CLOSED;
        break;
    case FDFR_STATUS_EYE_CLOSE_UNKNOW_STATUS:
        ret = KL520_FACE_EYE_CLOSE_UNKNOW_STATUS;
        break;
    case FDFR_STATUS_TOOUP:
        ret = KL520_FACE_TOOUP;
        break;
    case FDFR_STATUS_TOODOWN:
        ret = KL520_FACE_TOODOWN;
        break;
    case FDFR_STATUS_TOOLEFT:
        ret = KL520_FACE_TOOLEFT;
        break;
    case FDFR_STATUS_TOORIGHT:
        ret = KL520_FACE_TOORIGHT;
        break;
    case FDFR_STATUS_LOW_QUALITY:
        ret = KL520_FACE_LOW_QUALITY;
        break;
    case FDFR_STATUS_CALLIB_FAIL:
        ret = KL520_FACE_CALLIB_FAIL;
        break;
    case FDFR_STATUS_ATTACK:
        ret = KL520_FACE_ATTACK;
        break;
    case FDFR_STATUS_EMPTY:
        ret = KL520_FACE_EMPTY;
        break;
#if CFG_COMPARE_1VS1 == YES
    case FDFR_STATUS_NEXT:
        ret = KL520_FACE_SEND_NEXT_IMAGE;
        break;
#endif
    default:
        dbg_msg_api("[%s]state:%d", __func__, m_api_fdfr_ctx.state);
        ret = KL520_FACE_FAIL;
        break;
    }

    return ret;
}

int kl520_api_face_del(u8 del_ctrl, u8 user_id)
{
    int ret = -1;

    ret = _kl520_api_fdfr_del_user(del_ctrl, user_id);

    return ret;
}

int kl520_api_face_query(u8 face_id)
{
    int ret = -1;

    ret = _kl520_api_fdfr_query_user(face_id);

    return ret;
}

int kl520_api_face_query_all(u8* total_id_num, u8* face_id)
{
    int ret = -1;

    ret = _kl520_api_fdfr_query_all(total_id_num, face_id);

    return ret;
}

u8 kl520_api_face_query_first_avail_user_idx(void)
{
    #if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE
    if( m_face_db_add_mode == FACE_ADD_MODE_NO_DB )
    {
        return MAX_USER;
    }
    else
    #endif
    {
        for (u16 i = 0; i < MAX_USER; ++i) {
            if(KL520_FACE_EMPTY == _kl520_api_fdfr_query_user_is_added(i)) {
                return i;
            }
        }
    }
    return 0xff;
}

u8 kl520_api_face_query_user_idx(u8 user_id)
{
//    unsigned int db_size = ARRAY_SIZE(m_sample_db);
//    {
//        for (int i = 0; i < db_size; i++) {
//            if(user_id == m_sample_db[i].db_user_id) {
//                return i;
//            }
//        }
//    }
    return kdp_app_db_get_db_idx(user_id);
}

int kl520_api_face_add_set_timeout(int timeout)
{
    m_face_add_timeout = timeout;
    dbg_msg_api("kl520_api_face_add_set_timeout=%d", m_face_add_timeout);
    return FDFR_STATUS_OK;
}

int kl520_api_face_add_get_timeout(void)
{
    return m_face_add_timeout;
}

int kl520_api_face_recognition_set_timeout(int timeout)
{
    m_face_recognition_timeout = timeout;
    dbg_msg_api("m_face_recognition_timeout=%d", m_face_recognition_timeout);
    return FDFR_STATUS_OK;
}

int kl520_api_face_recognition_get_timeout(void)
{
    return m_face_recognition_timeout;
}

int kl520_api_face_liveness_set_timeout(int timeout)
{
    m_face_liveness_timeout = timeout;
    dbg_msg_api("m_face_liveness_timeout=%d", m_face_liveness_timeout);
    return FDFR_STATUS_OK;
}

int kl520_api_face_liveness_get_timeout(void)
{
    return m_face_liveness_timeout;
}

void kl520_api_face_set_curr_face_id(u8 curr_face_id)
{
    m_curr_face_id = curr_face_id;
}

u8 kl520_api_face_get_curr_face_id(void)
{
    return m_curr_face_id;
}

int kl520_api_add_wait_and_get(void)
{
    int ret = KL520_APP_FLAG_FDFR_ERR;
    u8 face_id = 0;
    u32 events = 0;
    system_info t_sys_info = { 0 };
#if (E2E_N1_ONLY_STRUCTURE_ENABLE == YES)
    while( 1 )
    {
        events = osEventFlagsWait( kl520_api_get_event(), KL520_APP_FLAG_FDFR | KL520_DEVICE_FLAG_ERR, osFlagsWaitAny, 100); // 100ms
        dbg_msg_console("[%s] event: %#X", __func__, events);
        if( events == 0xfffffffe ) {
            kl520_face_recognition_note();
            continue;
            
        }
        else
        {
            break;
        }
    }
#else
    events = wait_event(kl520_api_get_event(), KL520_APP_FLAG_FDFR | KL520_DEVICE_FLAG_ERR | KL520_APP_THREAD_ERR);
#endif
    

    //dbg_msg_console("[%s] events=%x", __func__, events);
    if(events == KL520_DEVICE_FLAG_ERR)
    {
        ret = kl520_api_get_device_info(&t_sys_info);
        dbg_msg_err("[%s], DEVICE ERROR, ret=0x%x", __func__, ret);
        kl520_api_free_device_info(&t_sys_info);
        ret = KL520_FACE_FAIL; //device error terminate
    }
    else
    {
        ret = kl520_api_face_get_result(&face_id);
        if (KL520_FACE_OK == ret)
        {
            m_curr_face_id = face_id;
//        #if CFG_SHOW_IMG_IN_CONSOLE == YES
//            preset_face_id = face_id;
//        #endif
            dbg_msg_console("[%s], KL520_FACE_OK, face_id=0x%x", __func__, face_id);
        }
        else if(KL520_FACE_EXIST == ret)
        {
		   //zcy add for return user id
		   m_curr_face_id = face_id;
		   dbg_msg_console("[%s], KL520_FACE_EXIST, face_id=0x%x", __func__, face_id);
           
        }
        else if(KL520_FACE_FULL == ret)
        {
            dbg_msg_console("[%s], user database is fulled!", __func__);
        }
        else if(KL520_FACE_TOO_FAR == ret)
        {
            dbg_msg_console("[%s], Face too Far!", __func__);
        }
        else if(KL520_FACE_TOO_NEAR == ret)
        {
            dbg_msg_console("[%s], Face too NEAR!", __func__);
        }
        else if(KL520_FACE_WAIT_DONT_MOVE == ret)
        {
            dbg_msg_console("[%s], Face add stage, Wait! Don't move!", __func__);
        }
        else if(KL520_FACE_BADPOSE == ret)
        {
            dbg_msg_console("[%s], bad pose!", __func__);
        }
        else if(KL520_FACE_MASK == ret)
        {
            dbg_msg_console("[%s], Mask on face!", __func__);
        }
        else if(KL520_FACE_EYE_CLOSED == ret)
        {
            dbg_msg_console("[%s], Eye closed!", __func__);
        }
        else if(KL520_FACE_TOOUP == ret)
        {
            dbg_msg_console("[%s], Face too UP!", __func__);
        }
        else if(KL520_FACE_TOODOWN == ret)
        {
            dbg_msg_console("[%s], Face too DOWN!", __func__);
        }
        else if(KL520_FACE_TOOLEFT == ret)
        {
            dbg_msg_console("[%s], Face too LEFT!", __func__);
        }
        else if(KL520_FACE_TOORIGHT == ret)
        {
            dbg_msg_console("[%s], Face too RIGHT!", __func__);
        }
        else if(KL520_FACE_LOW_QUALITY == ret)
        {
            dbg_msg_console("[%s], Please let the face more close to the sensor!", __func__);
        }
        else if(KL520_FACE_CALLIB_FAIL == ret)
        {
            dbg_msg_console("[%s], MP calllibration fail!", __func__);
        }
        else if(KL520_FACE_ATTACK == ret)
        {
            dbg_msg_console("[%s], Face attack!", __func__);
        }
        else if(KL520_FACE_NOFACE == ret)
        {
            dbg_msg_console("[%s], No face!", __func__);
        }
        else if(KL520_FACE_TIMEOUT == ret)
        {
            dbg_msg_console("[%s], No face!", __func__);
        }
        else
        {
            dbg_msg_console("ERROR, ret=0x%x", ret);
        }
    }
    return ret;
}


int kl520_api_face_add_internal(short x, short y, short w, short h, kl520_face_add_type type)
{
    int ret = KL520_FACE_OK;

    for(int i = 0; i < 200; i++) {
        int face_ret = kl520_api_face_add(x, y, w, h, type);
        if(face_ret == -1) {
            ret = KL520_FACE_FAIL;
        } else {
            ret = kl520_api_add_wait_and_get();
        }

        if((ret < KL520_FACE_BADPOSE || ret >= KL520_FACE_ATTACK) && (ret != KL520_FACE_NOFACE))
        {
#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
            kl520_api_snapshot_adv_shot_5face();
#endif
            osDelay(1);
            break;
//            return ret;
        }
        else if(ret == KL520_FACE_CALLIB_FAIL)
        {
            break;
//            return ret;
        }
        //for bad pose, retry
        //dbg_msg_console("get bad pose for registration, retrying:%d", i+1);
        osDelay(1);
    }

#if (CFG_LED_CTRL_ENHANCE == 1)
    kdp_e2e_nir_led_flag_off();
    osDelay(100);
#endif

    if(ret != KL520_FACE_OK && is_enroll_customize_uid()) {
        // if oms overwrite fails, need restore
        //restore the FLASH to DDR DB.
        kdp_app_db_flash_to_ddr(get_enroll_customize_uid());
    }
    
    return ret;
}

#if ( KL520_FACE_ADD_BMP == YES )
int kl520_api_face_add_internal_ex(short x, short y, short w, short h, u8 f_bmp)
{
    int ret = KL520_FACE_OK;

    u8 flag = 1;
    for(int i = 0; i < 200; i++) {
        if(flag) {
            kl520_api_face_add_ex(x, y, w, h, f_bmp | 0x80);
            flag = 0;
        } else {
            kl520_api_face_add_ex(x, y, w, h, f_bmp);
        }
        ret = kl520_api_add_wait_and_get();

        if(ret < KL520_FACE_BADPOSE)
        {
#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
            kl520_api_snapshot_adv_shot_5face();
#endif
            osDelay(1);
            break;
//            return ret;
        }
        else if(ret == KL520_FACE_CALLIB_FAIL)
        {
            break;
//            return ret;
        }
        //for bad pose, retry
        //dbg_msg_console("get bad pose for registration, retrying:%d", i+1);
        osDelay(1);
    }

    return ret;
}
#endif

#if ( KL520_FACE_ADD_BMP == YES )
int kl520_api_face_add_ex(short x, short y, short w, short h, u8 f_bmp)
{
    if(f_bmp == 0) return -1;
    face_add_bitmap = f_bmp;
    face_succ_index = 0xff;

    if(f_bmp & (0x01 << FACE_ADD_TYPE_NORMAL)) {
        return kl520_api_face_add(x, y, w, h, FACE_ADD_TYPE_NORMAL);
    }
    if(f_bmp & (0x01 << FACE_ADD_TYPE_LEFT)) {
        return kl520_api_face_add(x, y, w, h, FACE_ADD_TYPE_LEFT);
    }
    if(f_bmp & (0x01 << FACE_ADD_TYPE_RIGHT)) {
        return kl520_api_face_add(x, y, w, h, FACE_ADD_TYPE_RIGHT);
    }
    if(f_bmp & (0x01 << FACE_ADD_TYPE_UP)) {
        return kl520_api_face_add(x, y, w, h, FACE_ADD_TYPE_UP);
    }
    if(f_bmp & (0x01 << FACE_ADD_TYPE_DOWN)) {
        return kl520_api_face_add(x, y, w, h, FACE_ADD_TYPE_DOWN);
    }
    return -1;
}
#endif

int kl520_api_face_add(short x, short y, short w, short h, kl520_face_add_type type)
{
    s32 ret = 0;

    if ((FACE_MODE_NONE != m_face_mode) && (FACE_MODE_ADD != m_face_mode) &&
        (FACE_MODE_ENG_CAL != m_face_mode)) {
        dbg_msg_api("Err Add");
        return -1;
    }

    if( m_face_db_add_mode == FACE_ADD_MODE_IN_DB )
    {
        u8 total_id_num;
        u8 face_status[MAX_USER];
        u8 db_size = MAX_USER;

        kl520_api_face_query_all(&total_id_num, &face_status[0]);
        dbg_msg_api("kl520_api_face_add total_id_num=%u", total_id_num);
        if((total_id_num >= db_size) && (is_enroll_customize_uid() == 0))
        {
        #ifndef AUTO_DELETE_USER_ENABLE
            m_api_fdfr_ctx.state = FDFR_STATUS_FULL;
            set_event(kl520_api_get_event(), KL520_APP_FLAG_FDFR_ERR);
            return 0;
        #else
            //delete all users
            _kl520_api_fdfr_del_user(1, 0);
        #endif
        }
    }

    kl520_api_fdfr_model_init();
//    if ((kdp_e2e_prop_get2(flow_mode) < FLOW_MODE_SIM_VIRTUAL))
    {
        ret = _kl520_api_face_preexecute(x, y, w, h);
        if(ret != 0)// device error
        {
            set_event(kl520_api_get_event(), KL520_DEVICE_FLAG_ERR);
            return ret;
        }
    }

    if (kdp_e2e_prop_get2(face_mode) == FACE_MODE_ENG_CAL) {
        m_face_mode = FACE_MODE_ENG_CAL;
    } else {
        m_face_mode = FACE_MODE_ADD;
        kdp_e2e_prop_set2(face_mode, m_face_mode);
    }

    if(FACE_ADD_MODE_5_FACES == m_face_add_mode)
        kl520_api_dp_five_face_enable();

    m_api_fdfr_ctx.reg_idx = type;

    if(!_fdr_opened) {
        if(!_kl520_api_fdfr_try_create_thread()) return -1;
    } else {
        set_thread_event(m_api_fdfr_ctx.tid, FLAGS_API_FDFR_ADD_EVT);
    }

#if (KL520_QUICK_BOOT == NO)
    osDelay(1);
#endif
    return 0;
}

int kl520_api_face_recognition(short x, short y, short w, short h)
{
    s32 ret = 0;
    kl520_measure_stamp(E_MEASURE_API_FACE_REC_STR);

    if (FACE_MODE_NONE != m_face_mode) {
        dbg_msg_api("Err Rec");
        return -1;
    }
    
#ifdef DEV_TEST_VERSION
    _face_recog_count++;
#endif

    kl520_api_fdfr_model_init();
//    if ((kdp_e2e_prop_get2(flow_mode) < FLOW_MODE_SIM_VIRTUAL))
    {
        ret = _kl520_api_face_preexecute(x, y, w, h);
        if(ret != 0)// device error
        {
            set_event(kl520_api_get_event(), KL520_DEVICE_FLAG_ERR);
            return ret;
        }
    }

    m_face_appear = FALSE;
    m_face_mode = FACE_MODE_RECOGNITION;
    kdp_e2e_prop_set2(face_mode, m_face_mode);
    
    if (_kl520_api_fdfr_try_create_thread()) {
    #if (KL520_QUICK_BOOT == NO)
        set_thread_event(m_api_fdfr_ctx.tid, FLAGS_API_FDFR_RECOGNITION_EVT);
    #endif
    } else {
        return -1;
    }
#if (KL520_QUICK_BOOT == NO)
    osDelay(1);
#endif
    return 0;
}

int kl520_api_face_liveness(short x, short y, short w, short h)
{
    s32 ret = 0;
    if (FACE_MODE_NONE != m_face_mode) {
        dbg_msg_api("Err Rec");
        return -1;
    }

    m_face_mode = FACE_MODE_LIVENESS;
    kdp_e2e_prop_set2(face_mode, m_face_mode);

    kl520_api_fdfr_model_init();
//    if ((kdp_e2e_prop_get2(flow_mode) < FLOW_MODE_SIM_VIRTUAL))
    {
        ret = _kl520_api_face_preexecute(x, y, w, h);
        if(ret != 0)// device error
        {
            set_event(kl520_api_get_event(), KL520_DEVICE_FLAG_ERR);
            return ret;
        }
    }
    if (_kl520_api_fdfr_try_create_thread()) {
    #if (KL520_QUICK_BOOT == NO)
        set_thread_event(m_api_fdfr_ctx.tid, FLAGS_API_FDFR_LIVENESS_EVT);
    #endif
    } else {
        return -1;
    }
#if (KL520_QUICK_BOOT == NO)
    osDelay(1);
#endif
    return 0;
}

int kl520_api_snap_image(short x, short y, short w, short h)
{
    s32 ret = 0;
    if (FACE_MODE_NONE != m_face_mode) {
        //dbg_msg_console("Err mode for snap image");
        return -1;
    }

    m_face_mode = FACE_MODE_SNAP_IMG;
    kdp_e2e_prop_set2(face_mode, m_face_mode);

    kl520_api_fdfr_model_init();

    ret = _kl520_api_face_preexecute(x, y, w, h);
    if(ret != 0) { // device error
        //dbg_msg_console("start camera failed for snap image");
        return -1;
    }

    if (_kl520_api_fdfr_try_create_thread() == false) {
        //dbg_msg_console("fdfr thread failed for snap image");
        return -1;
    }

    return 0;
}

int kl520_api_face_recognition_test(short x, short y, short w, short h)
{
    s32 ret = 0;

    if (FACE_MODE_NONE != m_face_mode) {
        dbg_msg_api("Err Rec Test");
        return -1;
    }
    m_face_mode = FACE_MODE_RECOGNITION_TEST;
    kdp_e2e_prop_set2(face_mode, m_face_mode);

    kl520_api_fdfr_model_init();
    ret = _kl520_api_face_preexecute(x, y, w, h);
    if(ret != 0)// device error
    {
        set_event(kl520_api_get_event(), KL520_DEVICE_FLAG_ERR);
        return ret;
    }
    if (_kl520_api_fdfr_try_create_thread()) {
    #if (KL520_QUICK_BOOT == NO)
        set_thread_event(m_api_fdfr_ctx.tid, FLAGS_API_FDFR_RECOGNITION_TEST_EVT);
    #endif
    } else {
        return -1;
    }
#if (KL520_QUICK_BOOT == NO)
    osDelay(1);
#endif
    return 0;
}


#if CFG_COMPARE_1VS1 == YES
int kl520_api_face_2user_compare(short x, short y, short w, short h)
{
    s32 ret = 0;

    if (FACE_MODE_NONE != m_face_mode) {
        dbg_msg_api("Err Rec Test");
        return -1;
    }
    m_face_mode = FACE_MODE_COMPARE_1VS1;
    kdp_e2e_prop_set2(face_mode, m_face_mode);

    kl520_api_fdfr_model_init();
    ret = _kl520_api_face_preexecute(x, y, w, h);
    if(ret != 0)// device error
    {
        set_event(kl520_api_get_event(), KL520_DEVICE_FLAG_ERR);
        return ret;
    }
    if (_kl520_api_fdfr_try_create_thread()) {
    #if (KL520_QUICK_BOOT == NO)
        set_thread_event(m_api_fdfr_ctx.tid, FLAGS_API_FDFR_COMPARE_1VS1_EVT);
    #endif
    }
#if (KL520_QUICK_BOOT == NO)
    osDelay(1);
#endif
    return 0;
}
#endif

void kl520_api_reset_hmi_external_interface(void)
{
    osDelay(1);
    kl520_api_hmi_ctrl_state_set(CTRL_COMM);
    if ((kdp_e2e_prop_get2(flow_mode) < FLOW_MODE_SIM_VIRTUAL))
    {
        kl520_api_cam_disp_close_perm_state_chk();
    }

    kl520_api_ui_fsm_dp_layout_disable();
    kl520_api_cam_disp_state_rst();
    osDelay(1);
}

void kl520_api_face_close(void)
{
    if(mutex_face_close) osMutexAcquire(mutex_face_close, osWaitForever);
    
//    u32 old_face_mode = m_face_mode;

    if((FACE_MODE_NONE == m_face_mode)
    || (FACE_MODE_BUSY == m_face_mode))
    {
        kl520_api_reset_hmi_external_interface();
        dbg_msg_api("face_close repeatedly or fdfr busy");
    }
    else
    {
        face_non_move_cnt = -1;
        m_db_comp_fail_cnt = 0;
        m_face_mode = FACE_MODE_BUSY;
        kdp_e2e_prop_set2(face_mode, m_face_mode);
        _api_fdfr_thread_rst_priority();

#ifdef FDFR_KEEP_MS_WHEN_FACE_CLOSED
//        if ((old_face_mode == FACE_MODE_RECOGNITION) ||
//            (old_face_mode == FACE_MODE_ADD) ||
//            (old_face_mode == FACE_MODE_LIVENESS))
//            osDelay(KL520_FDFR_KEEP_MS);
#endif

        kl520_api_fdfr_terminate_thread();
        kdp_e2e_db_abort_reg();

        kl520_api_face_recognition_variable_reset();

        face_reg_sts = 0;
        kl520_api_dp_five_face_disable();

#ifdef FDFR_KEEP_MS_WHEN_FACE_CLOSED
        if ((kdp_e2e_prop_get2(flow_mode) < FLOW_MODE_SIM_VIRTUAL))
            _kl520_fdfr_drawing_timer_delete();
#endif

        osDelay(1);

        kl520_api_reset_hmi_external_interface();
        kdp_e2e_set_dual_cam_state(DUAL_IDENTICAL_CAM_CLOSED);

        if ((kdp_e2e_prop_get2(flow_mode) < FLOW_MODE_SIM_VIRTUAL))
        {
            kdp_fb_mgr_free_write_buf(CAMERA_DEVICE_NIR_IDX);
            kdp_fb_mgr_free_write_buf(CAMERA_DEVICE_RGB_IDX);

            kdp_fb_mgr_free_inf_buf(CAMERA_DEVICE_NIR_IDX);
            kdp_fb_mgr_free_inf_buf(CAMERA_DEVICE_RGB_IDX);
        }

        osDelay(1);

        m_face_mode = FACE_MODE_NONE;
        kdp_e2e_prop_set2(face_mode, m_face_mode);
        m_face_add_mode = FACE_ADD_MODE_DEFAULT;
    }
    
    if(mutex_face_close) osMutexRelease(mutex_face_close);
}

#if (KL520_QUICK_BOOT == YES)
//////////////////////////
/* tasks API            */
//////////////////////////
osThreadId_t tid_tasks_init_cameras = NULL;
osThreadId_t tid_tasks_init_models = NULL;
#if ( CFG_MODELS_LOAD_BY_ORDER == YES )
osThreadId_t tid_tasks_init_models_each = NULL;
#endif

#if (CFG_AI_TYPE != AI_TYPE_N1R1)
osThreadId_t tid_tasks_preexecute_colse_models = NULL;
void _tasks_preexecute_colse_thread(void *arg)
{
    osDelay(1000);
    kl520_api_face_preexecute_colse();
    tid_tasks_preexecute_colse_models = NULL;
    osThreadExit();

}
#endif

void _tasks_init_cameras_thread(void *arg)
{
    kl520_api_face_preexecute_stage1();
    kl520_api_face_preexecute_stage2();
    kl520_api_face_preexecute_stage3();
#if (CFG_AI_TYPE != AI_TYPE_N1R1)
    #if (CFG_SENSOR_TYPE == SENSOR_TYPE_GC1054_GC1054)
    //tid_tasks_preexecute_colse_models = osThreadNew(_tasks_preexecute_colse_thread, NULL, &attr);
    #else
    {
        osThreadAttr_t attr = {
            .stack_size = 512
        };
        tid_tasks_preexecute_colse_models = osThreadNew(_tasks_preexecute_colse_thread, NULL, &attr);
    }
    #endif
#endif
    tid_tasks_init_cameras = NULL;

    osThreadExit();
}

void _tasks_init_ui(void)
{   
	extern void user_ui_init(void);
    user_ui_init();    
}
void _tasks_init_db(void)
{
    kdp_e2e_db_init_flash_load();    
}

static void _kdp_load_list_models(u32* model_list, int size)
{
    //dbg_msg_console(  " model count : %d ", kdp_model_get_model_count() );
    u8 model_lists_size = size;
    u8 idx_array, idx_model;

    //dbg_msg_console(  " model count : %d , %d", kdp_model_get_model_count(), model_lists_size );
    for( idx_array =0; idx_array<model_lists_size; idx_array++) {
        for( idx_model=0; idx_model<kdp_model_get_model_count(); idx_model++ ) {
            struct kdp_model_s *mode_info = (struct kdp_model_s*)kdp_model_get_model_info(idx_model);
            if( model_list[idx_array] == mode_info->model_type ) break;
        }

        if( idx_model < kdp_model_get_model_count() ) {
            //dbg_msg_console(  " mode_id: %d, model_type[%d] ", mode_name_pool[idx_array], idx_model );
            kdp_model_load_model( idx_model );
        }
    }
    return;
}

static void _kdp_load_face_models(void)
{
    u32 mode_name_pool[] = {
        //KNERON_FD_FCOS_ROTATE,     
        KNERON_FD_FCOS_ROTATE_1054,        
        //KNERON_FD_MASK_MBSSD_200_200_3,
        //KNERON_FD_ROTATE,

        KNERON_LM_PLUS_ROTATE,

        //KNERON_FACE_POSE,
        KNERON_FACE_POSE_ROTATE,     

        KNERON_FACE_QAULITY_ONET_56_56_1,
        //KNERON_FUSE_LIVENESS_850,

        KNERON_NIR_COMBO_ROTATE_1054,
        //KNERON_NIR_LV_ROTATE,
 
        KNERON_FUSE_DUAL_1054,
        KNERON_FACESEG_ROTATE,

#if (IMAGE_SIZE == IMAGE_16MB) || (CFG_FR_SMALL_SIZE)
        KNERON_FR_RES50_1054
#elif CFG_FACE_FR50M == 1
        KNERON_TOF_FR50M_112_112_3
#else
        KNERON_FR_RES50_112_112_3
#endif
    };
    u8 model_lists_size = sizeof(mode_name_pool)/sizeof(u32);
    _kdp_load_list_models(mode_name_pool, model_lists_size);
    return;
}

#if CFG_PALM_PRINT_MODE == 1
static void _kdp_load_palmprint_models(void)
{
    u32 mode_name_pool[] = {
        KNERON_FD_FCOS_ROTATE_1054,        

        KNERON_FACESEG_ROTATE
    };
    u8 model_lists_size = sizeof(mode_name_pool)/sizeof(u32);
    _kdp_load_list_models(mode_name_pool, model_lists_size);
    return;
}
#endif

void _tasks_init_flash_load_thread_each(void)
{
    int cnt = 0;

#if ( CFG_MODELS_LOAD_BY_ORDER == YES )
    kdp_model_info_reload();
#else    
    kdp_model_load_model(-1);
#endif
    
    _tasks_init_db();
    
    while( (kdp_e2e_face_init_done() == FALSE) ||
           (kdp_model_get_model_count() == 0) ) {
        osDelay(10);
        if( cnt > 100 ) return;

        cnt++;
    }

#if CFG_PALM_PRINT_MODE == 1
    if(kdp_is_palm_mode()) {
        _kdp_load_palmprint_models();
    }
    else 
#endif
    {
        _kdp_load_face_models();
    }

    // if array: mode_name_pool was not include whole models, load last models
    for( u8 idx_model=0; idx_model < kdp_model_get_model_count(); idx_model++ ) {
        if( kdp_model_load_flag( idx_model ) != 1 ) {
            kdp_model_load_model( idx_model );
            //report it
            dbg_msg_console(  "model :%d is not fast loaded.", idx_model );
        }
    }

    // final load ui and db
    _tasks_init_ui();
}

void _tasks_init_models_thread(void *arg)
{
    kl520_api_fdfr_model_init();       // Load model
    tid_tasks_init_models = NULL;

    osThreadExit();
}

void kl520_api_tasks_init(void)
{
    kl520_measure_stamp(E_MEASURE_THR_TASKS_INIT);

    osThreadAttr_t attr = {
        .stack_size = 1280,
        .priority = osPriorityRealtime,
        //.attr_bits = osThreadJoinable
    };

    tid_tasks_init_cameras = osThreadNew(_tasks_init_cameras_thread, NULL, &attr);
    tid_tasks_init_models = osThreadNew(_tasks_init_models_thread, NULL, &attr);
#if ( CFG_MODELS_LOAD_BY_ORDER == YES )
    osThreadAttr_t attr2 = {
        .stack_size = 1280,
        .priority = osPriorityNormal
    };
    tid_tasks_init_models_each = osThreadNew( (osThreadFunc_t)_tasks_init_flash_load_thread_each, NULL, &attr2); 
#endif
}

void kl520_api_tasks_init_wait_ready(void)
{
//    osThreadJoin(tid_tasks_init_cameras);
//    osThreadJoin(tid_tasks_init_models);
    
    for (int i = 0; i < 100; i++) {
        if(tid_tasks_init_cameras == NULL && tid_tasks_init_models == NULL) break;
        osDelay(50);
    }

    kl520_measure_stamp(E_MEASURE_THR_TASKS_INIT_RDY);
}
#else
void kl520_api_tasks_init(void)
{
    kl520_api_fdfr_model_init();       // Load model
}
void kl520_api_tasks_init_wait_ready(void) {}
#endif

u8 kl520_api_get_start_user_id(void)
{
    return _kl520_app_calc_db_uid(0);
}

s32 kdp_third_party_aec(uint8_t* p_img)
{
    return 0;
}

#if CFG_PALM_PRINT_MODE == 1
void switch_palm_mode(u8 mode)
{
    if(mode != 0 && mode != 1) return;

    kdp_set_palm_mode(mode);
    kl520_api_face_preexecute_stage1();
    kl520_api_face_preexecute_stage2();
    kdp_reload_db();
    if(mode == 1) { //set to palm mode
        //load palm models
        _kdp_load_palmprint_models();
    } else { //set to face mode
        //face models already loaded.
    }
}
#endif
