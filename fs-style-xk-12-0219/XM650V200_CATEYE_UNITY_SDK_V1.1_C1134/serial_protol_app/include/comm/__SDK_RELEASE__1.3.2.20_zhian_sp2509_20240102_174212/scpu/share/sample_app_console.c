#include "sample_app_console.h"

#include <stdlib.h>

#include "kneron_mozart_ext.h"
#include "io.h"
#include "board_ddr_table.h"
#include "pinmux.h"
#include "framework/event.h"
#include "framework/v2k.h"
#include "framework/v2k_color.h"
#include "framework/v2k_image.h"
#include "framework/framework_driver.h"
#include "media/display/display.h"
#include "media/display/video_renderer.h"
#include "drivers.h"
#include "ddr.h"
#include "rtc.h"
#include "system.h"
#include "power.h"
#include "power_manager.h"
#include "kdp520_ssp.h"
#include "kdp520_spi.h"
#include "kdp520_pwm_timer.h"
#include "kdp_uart.h"
#include "kdp_com.h"
#include "kdp_memxfer.h"
#include "kdp_e2e_db.h"
#include "flash.h"
#include "touch.h"
#include "ota_update.h"
#include "kl520_include.h"
#include "kl520_api.h"
#include "kl520_api_system.h"
#include "kl520_api_fdfr.h"
#include "kl520_api_device_id.h"
#include "kl520_api_ssp.h"
#include "kl520_api_snapshot.h"
#include "kl520_api_extra_fmap.h"
#include "kl520_api_sim.h"
#include "user_io.h"
#include "sample_app_touch.h"
#include "ota.h"
#include "ota_update.h"
#include "kl520_sys.h"
#include "kdp_model.h"
#if ( CFG_GUI_ENABLE == YES )
#include "sample_gui_main.h"
#include "sample_gui_fsm_events.h"
#endif

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK )
#if ( CFG_COM_USB_PROT_TYPE == COM_USB_PROT_KDP )
#include "kdp_host_com.h"
#elif ( CFG_COM_USB_PROT_TYPE == COM_USB_PROT_KDP_USR )
#include "user_host_com.h"
#endif
#endif
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP )
#include "kdp_comm_protoco.h"
#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP_USR )
#include "user_comm_protoco.h"
#endif
#endif
#endif

#define SAMPLE_IO                   (NO)
#define SAMPLE_ADC                  (NO)
#define SAMPLE_FLASH                (NO)
#define SAMPLE_DISPLAY_SNAPSHOT     (NO)
#if (defined(DEV_TEST_VERSION) || ((CFG_COM_BUS_TYPE & COM_BUS_USB_MASK) && (CFG_PRODUCTION_TEST==1)))
#define SAMPLE_ADV_SNAPSHOT         (YES)
#else
#define SAMPLE_ADV_SNAPSHOT         (NO)
#endif
#define SAMPLE_LCD_BACKLIGHT        (NO)
#define SAMPLE_STABILITY_ENABLE     (NO)
#define SPECIAL_SCENARIO_TEST1      (NO)
#define SPECIAL_SCENARIO_TEST2      (NO)
//#define SPECIAL_SCENARIO_TEST_TOUCH_ENABLE

//#define QUICK_CONSOLE_GUIDELINE

//#define SAC_PROFILE_ENABLE
#ifdef SAC_PROFILE_ENABLE
static u32 _prof_start, _prof_end;
static u32 _prof_offset, _prof_total = 0, _prof_frame_cnt = 0;
#define SAC_PROFILE_START() _prof_start = osKernelGetTickCount();
#define SAC_PROFILE_STOP() { \
                _prof_end = osKernelGetTickCount(); \
                _prof_offset = _prof_end - _prof_start; \
                _prof_total += _prof_offset; \
                ++_prof_frame_cnt; \
                /*dbg_msg_console("[%s] offset=%u average=%u", __func__, _prof_offset, _prof_total / _prof_frame_cnt);*/ \
                }
#else
#define SAC_PROFILE_START()
#define SAC_PROFILE_STOP()
#endif
                
                
#if USR_FLASH_LAST_ADDR > (FLASH_SIZE *1024 * 1024)
#error FLASH SIZE OVERFLOW!!!!! ==========
#endif
#if USR_FLASH_LAST_ADDR > (IMAGE_SIZE *1024 * 1024)
#error IMAGE SIZE OVERFLOW!!!!! ==========
#endif

#ifdef DEV_TEST_VERSION
extern int _face_recog_count;
extern int _face_succ_count;
#endif

osThreadId_t tid_abort_thread = 0;
                
typedef void  (*console_cmd_func)(void);
struct console_cmd_info
{
    char *desc;
    console_cmd_func func;
    //void (*func)(void);
};

enum display_state sample_app_display_state;

osThreadId_t tid_doorlock_console = 0;
//#if CFG_SHOW_IMG_IN_CONSOLE == YES
//static uint8_t preset_face_id = 0xFF;
//#endif


osThreadId_t tid_special_scenario_1 = 0;

osTimerId_t timer_zb = 0;

void timer_cb(void *argument)
{
    osEventFlagsSet(kl520_api_get_event(),KL520_APP_FLAG_ACTION);
}

#ifdef SPECIAL_SCENARIO_TEST_TOUCH_ENABLE
osTimerId_t timer_zb1 = 0;

void timer_cb1(void *argument)
{
    osEventFlagsSet(kl520_api_get_event(),KL520_APP_FLAG_ACTION_TOUCH);
}
#endif

void sample_open_video_renderer(void)
{
    kl520_api_cam_disp_ctrl(API_CTRL_DISP_OPEN, NULL, PERMANENT_DISP);
}

void sample_close_video_renderer(void)
{
    kl520_api_cam_disp_ctrl(API_CTRL_DISP_CLOS, NULL, PERMANENT_DISABLE);
}

int app_kl520_init()
{
   //lcd_power_on();
   //tp_power_on();
   if (0 != kl520_api_touch_open())
   {
       return -1;
   }
   kl520_api_touch_start();

   kl520_api_touch_set_x_range_max(CFG_TOUCH_X_RANGE_MAX);
   kl520_api_touch_set_y_range_max(CFG_TOUCH_Y_RANGE_MAX);
   kl520_api_touch_set_x_axis_inverse(CFG_TOUCH_X_AXIS_INVERSE);
   kl520_api_touch_set_y_axis_inverse(CFG_TOUCH_Y_AXIS_INVERSE);

    return 0;
}

#define SPECIAL_SCENARIO_1_TEST_TIME 3

void app_event_thread_kneron(void *arg)
{
    uint32_t flags = 0;
    static int8_t a = 1;
    //u32 cnt = 0;

    for(;;)
    {
        flags = osEventFlagsWait(kl520_api_get_event(),KL520_APP_FLAG_ALL,  osFlagsWaitAny, osWaitForever);

        //dbg_msg_console("ffff flags=%x cnt=%u", flags, ++cnt);

        if ((flags & KL520_APP_FLAG_FDFR_OK) || (flags & KL520_APP_FLAG_FDFR_ERR)|| (flags & KL520_APP_FLAG_FDFR_TIMEOUT) ||(flags & KL520_DEVICE_FLAG_ERR))
        {
            kl520_api_face_close();

             if (a)
             {
                kl520_api_dp_fresh_bg(RED, 3);
             }
             else
             {
                 kl520_api_dp_fresh_bg(BLUE, 3);
             }
        }
        if (flags & KL520_APP_FLAG_ACTION)
        {
            if (a)
            {
                kl520_api_dp_set_backlight(100);
                kl520_api_face_recognition_set_timeout(3);
                kl520_api_face_recognition(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
            }
            else
            {
                kl520_api_dp_set_backlight(100);
                kl520_api_face_add_set_timeout(3);
                //kl520_api_face_add(0, 0, LCD_WIDTH, LCD_HEIGHT, FACE_ADD_TYPE_NORMAL);
                kl520_api_face_recognition(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
            }
            a = !a;
        }
#ifdef SPECIAL_SCENARIO_TEST_TOUCH_ENABLE
        else if (flags & KL520_APP_FLAG_ACTION_TOUCH)
        {
            kl520_api_dp_fresh();
        }
#endif
    }
}

#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
static void snapshot_adv(void)
{
    char buf[256];
    u8 idx;
    dbg_msg_console("adv select log mode:");
    dbg_msg_console("(0)init");
    dbg_msg_console("(1)verify liveness log mode");
    dbg_msg_console("(2)verify un-liveness log mode");
    dbg_msg_console("(3)five face log mode");
    dbg_msg_console("(4)always shot log mode");
    dbg_msg_console("(5)liveness fail type mode (ground true: alive)");
    dbg_msg_console("(6)mask return value");
    dbg_msg_console("(7)structure light liveness");
    //Welcome to add conditions
    kdp_gets(DEBUG_CONSOLE, buf);
    dbg_msg_nocrlf("");
    idx = atoi(strtok(buf, " \r\n\t"));
    kl520_api_snapshot_adv_select(idx);
    kl520_api_snapshot_adv_mode();
}

void sample_snapshot_auto_usb(void)
{
    kl520_api_snapshot_adv_init(MIPI_CAM_NIR, KDP_DDR_TEST_NIR_IMG_SIZE);
    kl520_api_snapshot_adv_init(MIPI_CAM_RGB, KDP_DDR_TEST_RGB_IMG_SIZE);
    kl520_api_snapshot_adv_init(MIPI_CAM_INF, KDP_DDR_TEST_INF_IMG_SIZE);
    snapshot_adv();
}

void sample_snapshot_auto_usb_mode(u8 adv_mode)
{
    kl520_api_snapshot_adv_init(MIPI_CAM_NIR, KDP_DDR_TEST_NIR_IMG_SIZE);
    kl520_api_snapshot_adv_init(MIPI_CAM_RGB, KDP_DDR_TEST_RGB_IMG_SIZE);
    kl520_api_snapshot_adv_init(MIPI_CAM_INF, KDP_DDR_TEST_INF_IMG_SIZE);
    kl520_api_snapshot_adv_select(adv_mode);
    kl520_api_snapshot_adv_mode();
}

#endif

#if CFG_SNAPSHOT_ENABLE == 2
void sample_snapshot(void){

    char buf[256];
    uint32_t id;
    uint32_t rc = 1;

    for(;;)
    {
        if (rc)
            goto adv_prompt;

        dbg_msg_console("(06) Exit");
        dbg_msg_console("(07) Catch");
#if (CFG_SNAPSHOT_ADVANCED == 1)
        dbg_msg_console("(08) Save");
#endif
        dbg_msg_console("(09) Fdfr Sim, %d", kl520_api_sim_is_running());
        dbg_msg_console("(10) AdvInit");
        dbg_msg_console("(11) AdvMode, %d", snapshot_adv_mode);
        dbg_msg_console("(12) AdvSelectMode, %d",snapshot_adv_select);
        dbg_msg_console("(13) AdvLoad For Tool");
#if (CFG_SNAPSHOT_ADVANCED == 1)
        dbg_msg_console("(14) AdvShow NIR");
        dbg_msg_console("(15) AdvShow RGB");
        dbg_msg_console("(16) AdvLoad&Show");
#endif
adv_prompt:
        dbg_msg_console("Snapshot>>");

        rc = kdp_gets(DEBUG_CONSOLE, buf);
        dbg_msg_nocrlf("");
            if (!rc)
                continue;

        id = atoi(strtok(buf, " \r\n\t"));
        if(id == 6)
        {
            break;
        }
        else if(id == 7)
        {
            kl520_api_snapshot_fdfr_catch(MIPI_CAM_NIR);   /*NIR*/
            kl520_api_snapshot_fdfr_catch(MIPI_CAM_RGB);   /*RGB*/
            dbg_msg_console("[Snapshot]fdfr_catch...");
        }
#if (CFG_SNAPSHOT_ADVANCED == 1)
        else if(id == 8)
        {
            kl520_api_snapshot_fdfr_save();
            dbg_msg_console("[Snapshot]fdfr_save...");
        }
#endif
        else if(id == 9)
        {
            kl520_api_sim_fdfr_flow_switch();
        }
        else if(id == 10)
        {
            kl520_api_snapshot_adv_init(MIPI_CAM_NIR, KDP_DDR_TEST_NIR_IMG_SIZE);
            dbg_msg_console("[Snapshot]face_add_init nir...");
            kl520_api_snapshot_adv_init(MIPI_CAM_RGB, KDP_DDR_TEST_RGB_IMG_SIZE);
            dbg_msg_console("[Snapshot]face_add_init rgb...");
            kl520_api_snapshot_adv_init(MIPI_CAM_INF, KDP_DDR_TEST_INF_IMG_SIZE);
            dbg_msg_console("[Snapshot]face_add_init inf...");

            g_SnapUsbBufCnt = 0;   //reset shot count
            g_SnapShotCnt = 0;  //reset shot count
            g_SnapReadCnt = 0;  //reset shot count

        }
        else if(id == 11)
        {
            kl520_api_snapshot_adv_mode();

            g_SnapUsbBufCnt = 0;   //reset shot count
            g_SnapShotCnt = 0;  //reset shot count
            g_SnapReadCnt = 0;  //reset shot count
        }
        else if(id == 12)
        {
            snapshot_adv();

            g_SnapUsbBufCnt = 0;   //reset shot count
            g_SnapShotCnt = 0;  //reset shot count
            g_SnapReadCnt = 0;  //reset shot count
        }
        else if(id == 13)
        {
            u8 idx;
            dbg_msg_console("load idx:");
            kdp_gets(DEBUG_CONSOLE, buf);
            dbg_msg_nocrlf("");
            idx = atoi(strtok(buf, " \r\n\t"));
            kl520_api_snapshot_adv_load(MIPI_CAM_NIR, idx);
            kl520_api_snapshot_adv_load(MIPI_CAM_RGB, idx);
            kl520_api_snapshot_adv_load(MIPI_CAM_INF, idx);
            dbg_msg_console("[Snapshot]adv_load idx:%d",idx);

            g_SnapUsbBufCnt = 0;   //reset shot count
            g_SnapShotCnt = 0;  //reset shot count
            g_SnapReadCnt = 0;  //reset shot count
        }
        else if(id == 14)
        {
            u32 buf_addr;
            sample_open_video_renderer();
            buf_addr = kl520_api_snapshot_addr(MIPI_CAM_NIR);
            kdp_display_set_source(buf_addr, CAMERA_DEVICE_NIR_IDX);
            kl520_api_dp_fresh();
            dbg_msg_console("[Snapshot]show 0 addr:0x%x", buf_addr);
        }
        else if(id == 15)
        {
            u32 buf_addr;
            sample_open_video_renderer();
            buf_addr = kl520_api_snapshot_addr(MIPI_CAM_RGB);
            kdp_display_set_source(buf_addr, CAMERA_DEVICE_RGB_IDX);
            kl520_api_dp_fresh();
            dbg_msg_console("[Snapshot]show 1 addr:0x%x", buf_addr);
        }
        else if(id == 16)
        {
            u32 buf_addr;
            u8 i;
            sample_open_video_renderer();

            dbg_msg_console("[Snapshot]AdvLoad auto rgb...");
            for(i=0;i<CFG_SNAPSHOT_NUMS;i++){
                kl520_api_snapshot_adv_load(MIPI_CAM_RGB, i);
                buf_addr = kl520_api_snapshot_addr(MIPI_CAM_RGB);
                kdp_display_set_source(buf_addr, CAMERA_DEVICE_RGB_IDX);
                kl520_api_dp_fresh();
                osDelay(250);
            }
            dbg_msg_console("[Snapshot]AdvLoad auto nir...");
            for(i=0;i<CFG_SNAPSHOT_NUMS;i++){
                kl520_api_snapshot_adv_load(MIPI_CAM_NIR, i);
                buf_addr = kl520_api_snapshot_addr(MIPI_CAM_NIR);
                kdp_display_set_source(buf_addr, CAMERA_DEVICE_NIR_IDX);
                kl520_api_dp_fresh();
                osDelay(250);
            }

        }
        else if(id == 17)
        {
            kl520_customer_info Cusinfo;
            kl520_api_customer_get(&Cusinfo);

            Cusinfo.nCusInfo0 = Cusinfo.nCusInfo0+1;
            Cusinfo.nCusInfo1 = Cusinfo.nCusInfo1+2;
            Cusinfo.nCusInfo2 = Cusinfo.nCusInfo2+2;
            Cusinfo.nCusInfo3 = Cusinfo.nCusInfo3+3;

            kl520_api_customer_write(&Cusinfo);
            kl520_api_customer_info();

        }
        else if(id == 18)
        {

            kl520_customer_info Cusinfo;

            Cusinfo.nCusInfo0 = 1;
            Cusinfo.nCusInfo1 = 2;
            Cusinfo.nCusInfo2 = 3;
            Cusinfo.nCusInfo3 = 4;

            kl520_api_customer_write(&Cusinfo);
            kl520_api_customer_info();
        }


        osDelay(5);
    }
}

#endif

#if (SAMPLE_DISPLAY_SNAPSHOT == YES) && (CFG_SNAPSHOT_ADVANCED == 1)
void sample_snapshot_record_img(void){

    u8 nRecIdx, nNumCnt;
    u64 time = 20191211;
    u8 result = 0;
    char buf[256];

    dbg_msg_console("Recode index(0~%d):", CFG_SNAPSHOT_NUMS-1);
    kdp_gets(DEBUG_CONSOLE, buf);
    dbg_msg_nocrlf("");
    nRecIdx = atoi(strtok(buf, " \r\n\t"));

    dbg_msg_console("Recode number count:");
    kdp_gets(DEBUG_CONSOLE, buf);
    dbg_msg_nocrlf("");
    nNumCnt = atoi(strtok(buf, " \r\n\t"));

    kl520_api_snapshot_record_img(nRecIdx, nNumCnt, time, result);
}

void sample_snapshot_show_img(void){

    u8 nShowIdx, nNumCnt;
    char buf[256];

    kl520_api_snapshot_db();

    dbg_msg_console("Show index(0~%d):", CFG_SNAPSHOT_NUMS-1);
    kdp_gets(DEBUG_CONSOLE, buf);
    dbg_msg_nocrlf("");
    nShowIdx = atoi(strtok(buf, " \r\n\t"));

    dbg_msg_console("Show number count:");
    kdp_gets(DEBUG_CONSOLE, buf);
    dbg_msg_nocrlf("");
    nNumCnt = atoi(strtok(buf, " \r\n\t"));

    kl520_api_snapshot_show_img(nShowIdx, nNumCnt);
}

void sample_snapshot_del_img(void){

    u8 nDelIdx, nNumCnt;
    char buf[256];

    dbg_msg_console("Delete index(0~%d):", CFG_SNAPSHOT_NUMS-1);
    kdp_gets(DEBUG_CONSOLE, buf);
    dbg_msg_nocrlf("");
    nDelIdx = atoi(strtok(buf, " \r\n\t"));

    dbg_msg_console("Delete number count:");
    kdp_gets(DEBUG_CONSOLE, buf);
    dbg_msg_nocrlf("");
    nNumCnt = atoi(strtok(buf, " \r\n\t"));

    kl520_api_snapshot_del_img(nDelIdx, nNumCnt);

    kl520_api_snapshot_db();
}
#endif



void sample_set_e2e_prop()
{
    char buf[256];
    bool quit = false;
    float r1_confident = 0.0;
    float n1_confident = 0.0;
    int lm_diff = 0;
    int para;

    while(!quit)
    {
        dbg_msg_console("(1)Set r1_lm_chk_low_confident(0.000~0.999)");
        dbg_msg_console("(2)Set n1_lm_chk_low_confident(0.000~0.999)");
        dbg_msg_console("(3)Set rgb_nir_lm_diff_threshold");
        dbg_msg_console("(4)exit");

        kdp_gets(DEBUG_CONSOLE, buf);
        dbg_msg_nocrlf("");

        para = atoi(strtok(buf, " \r\n\t"));

        kdp_e2e_prop *prop = kdp_e2e_prop_get_inst();

        switch(para)
        {
            case 1:
                r1_confident = kdp_e2e_prop_get_value(prop, r1_lm_chk_low_confident);
                dbg_msg_console("The current r1_confident = %f, please set the new value:", r1_confident);
                kdp_gets(DEBUG_CONSOLE, buf);
                dbg_msg_nocrlf("");
                r1_confident = atof(strtok(buf, " \r\n\t"));
                kdp_e2e_prop_set_manual_value(prop, r1_lm_chk_low_confident, r1_confident);
                break;
            case 2:
                n1_confident = kdp_e2e_prop_get2_value(n1_lm_chk_low_confident);
                dbg_msg_console("The current n1_confident = %f", n1_confident);
                kdp_gets(DEBUG_CONSOLE, buf);
                dbg_msg_nocrlf("");
                n1_confident = atof(strtok(buf, " \r\n\t"));
                kdp_e2e_prop_set_manual_value(prop, n1_lm_chk_low_confident, n1_confident);
                break;
            case 3:
                lm_diff = kdp_e2e_prop_get2_value(rgb_nir_lm_diff_threshold);
                dbg_msg_console("The current lm_diff = %d", lm_diff);
                kdp_gets(DEBUG_CONSOLE, buf);
                dbg_msg_nocrlf("");
                lm_diff = atoi(strtok(buf, " \r\n\t"));
                kdp_e2e_prop_set_manual_value(prop, rgb_nir_lm_diff_threshold, lm_diff);
                break;
            case 4:
                quit = true;
                break;
            default:
                break;
        }
    }
}

void special_scenario_test_1(void)
{
    osThreadAttr_t attr = {
        .stack_size = 256
    };     
    
    tid_special_scenario_1 = osThreadNew(app_event_thread_kneron, NULL, &attr);

    app_kl520_init();

    timer_zb = osTimerNew(timer_cb, osTimerPeriodic, NULL, NULL);
    osTimerStart(timer_zb, (SPECIAL_SCENARIO_1_TEST_TIME + 3) * 1000);

#ifdef SPECIAL_SCENARIO_TEST_TOUCH_ENABLE
    timer_zb1 = osTimerNew(timer_cb1, osTimerPeriodic, NULL, NULL);
    osTimerStart(timer_zb, 1000);
#endif
}


#ifdef SPECIAL_SCENARIO_TEST2
void special_scenario_test_2(void)
{
    kl520_api_dp_open(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    kl520_api_dp_fresh_bg(RED, 3);

    osDelay(2000);

    kl520_api_face_recognition_test(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);

    osDelay(5000);

    sample_face_close();
}
#endif

void _control_video_renderer_from_console(BOOL startIt)
{
    if ((DISPLAY_STATE_OPENED == sample_app_display_state) && (startIt))
    {

        kdp_video_renderer_start();

        osDelay(10);

        sample_app_display_state = DISPLAY_STATE_RUNNING;
    }
    else if ((DISPLAY_STATE_RUNNING == sample_app_display_state) && (!startIt))
    {
        kdp_video_renderer_stop();

        osDelay(10);

        sample_app_display_state = DISPLAY_STATE_OPENED;
    }
}

static int _cmd_get_answer_int(const char *str)
{
    int ans;
    char buf[256];

    dbg_msg_nocrlf(str);
    kdp_gets(DEBUG_CONSOLE, buf);
    dbg_msg_nocrlf("");

    ans = atoi(strtok(buf, " \r\n\t"));

    return ans;
}


void force_abort_thread(void *arg)
{
    char input = 0;

    while(1)
    {
        input = kdp_getchar(DEBUG_CONSOLE);

        if('q' == input)
        {
            kl520_api_face_close();
            osDelay(10);
            set_event(kl520_api_get_event(), KL520_APP_FLAG_FDFR_OK);
        }
        osDelay(50);
    }
}

void sample_force_abort_enable(void)
{
#if (CFG_CAMERA_ROTATE != 1)
    osThreadAttr_t attr = {
        .stack_size = 512
    };     
    
    tid_abort_thread = osThreadNew(force_abort_thread, NULL, &attr);
    //dbg_msg_console("[%s] thread id of force abort is 0x%x", __func__, tid_abort_thread);
#endif
}
void sample_force_abort_disable(void)
{
#if (CFG_CAMERA_ROTATE != 1)
    osStatus_t status;

    if (tid_abort_thread) {
        status = osThreadTerminate(tid_abort_thread);
        tid_abort_thread = NULL;
    }
    if(status != osOK)
        dbg_msg_console("[%s] thread id of force abort is 0x%x, status = %d", __func__, tid_abort_thread, status);
#endif
}

void sample_camera_open(void)
{
    kl520_api_camera_open(_cmd_get_answer_int("open camera, index >>"));
}

void sample_camera_start(void)
{
    u8 cam_idx = _cmd_get_answer_int("start camera, index >>");

    kdp_video_renderer_set_cam_idx(cam_idx);
    struct video_input_params params = kdp_video_renderer_setting(cam_idx);
    kdp_video_engineering_switch(&params);

    kl520_api_camera_start(cam_idx);
}

void sample_camera_stop(void)
{
    kl520_api_camera_stop(_cmd_get_answer_int("stop camera, index >>"));
}

void sample_camera_close(void)
{
    kl520_api_camera_close(_cmd_get_answer_int("close camera, index >>"));
}

void sample_lcd_adjust_backlight(void)
{
    int _brightness = 0;
    char buf[256];
    dbg_msg_console(" lcm brightness (brightness:0-100)>>");
    kdp_gets(DEBUG_CONSOLE, buf);
    dbg_msg_nocrlf("");

    _brightness = atoi(strtok(buf, " \r\n\t"));
    kl520_api_dp_set_backlight(_brightness);
}

void sample_lcd_adjust_backlight_step_by_step(void)
{
    u16 i;

    for(i=0;i<100;i++)
    {
        kl520_api_dp_set_backlight(i);
        osDelay(7);
    }

    for(i=100;i>1;i--)
    {
        kl520_api_dp_set_backlight(i);
        osDelay(5);
    }
}

void sample_ui_draw_full(void)
{
#if (CFG_PANEL_TYPE > PANEL_NULL)
#if CFG_UI_USR_IMG == NO
    kl520_api_dp_draw_bitmap(0, 0, 240, 320, (void *)USR_DDR_IMG_UI1_ADDR);
    kl520_api_dp_fresh();
#endif
#endif
}
void sample_ui_draw_part(void)
{
#if (CFG_PANEL_TYPE > PANEL_NULL)
#if CFG_UI_USR_IMG == NO
    kl520_api_dp_set_pen_rgb565(GREEN, 2);
    kl520_api_dp_fill_rect(80, DISPLAY_WIDTH, 80, 80);
    kl520_api_dp_draw_bitmap(0, 0, 240, 135, (void *)USR_DDR_IMG_01_KNERON_ADDR);
    kl520_api_dp_fresh();
#endif
#endif
}

void sample_ui_draw_example(void)
{
#if (CFG_PANEL_TYPE > PANEL_NULL)
#define	EXAMPLE_01	(OFF)
#define	EXAMPLE_02	(ON)


    #if (EXAMPLE_01 == ON)

    sample_open_video_renderer();
    kl520_api_dp_set_pen_rgb565(BLUE, 2);
    kl520_api_dp_fill_rect(0, 240, 80, 80);

    kl520_api_dp_set_pen_rgb565(GREEN, 2);
    kl520_api_dp_fill_rect(80, 240, 80, 80);

    kl520_api_dp_set_pen_rgb565(RED, 2);
    kl520_api_dp_fill_rect(160, 240, 80, 80);
    kl520_api_dp_fresh();
    sample_close_video_renderer();
    #endif

    #if (EXAMPLE_02 == ON)

    sample_open_video_renderer();
#if CFG_UI_USR_IMG == NO
    kl520_api_dp_fresh_bg(BLACK, 2);

    sample_lcd_adjust_backlight_step_by_step();
    kl520_api_dp_draw_bitmap(0, 0, 240, 135, (void *)USR_DDR_IMG_01_KNERON_ADDR);
    kl520_api_dp_fresh();

    sample_lcd_adjust_backlight_step_by_step();
    kl520_api_dp_draw_bitmap(0, (320-45), 80, 45, (void *)USR_DDR_IMG_02_ICON_01_ADDR);
    kl520_api_dp_fresh();

    sample_lcd_adjust_backlight_step_by_step();
    kl520_api_dp_draw_bitmap(80, (320-45), 80, 45, (void *)USR_DDR_IMG_03_ICON_02_ADDR);
    kl520_api_dp_fresh();

    sample_lcd_adjust_backlight_step_by_step();
    kl520_api_dp_draw_bitmap(160, (320-45), 80, 45, (void *)USR_DDR_IMG_05_ICON_04_ADDR);
    kl520_api_dp_fresh();
#endif
    kl520_api_dp_set_backlight(100);
    osDelay(1000);
    kl520_api_dp_fresh_bg(BLACK, 2);

    sample_close_video_renderer();

    #endif
#endif
}

void sample_ui_draw_line(void)
{
    kl520_api_dp_set_pen_rgb565(YELLOW, 2);
    kl520_api_dp_draw_line(10, 10, 10, 200);

    kl520_api_dp_fresh();
}

void sample_ui_draw_rect(void)
{
    kl520_api_dp_set_pen_rgb565(RED, 2);
    kl520_api_dp_draw_rect(150, 30, 80, 200);

    kl520_api_dp_fresh();
}

#if (CFG_PANEL_TYPE==PANEL_ST7789_240X320_8080_AND_SPI)
extern void kdp_video_renderer_reassign_display_device(u8 device_type);
extern void kdp_video_renderer_reconnect_display_panel(u8 device_type);
void sample_switch_display_device(void) {
    u8 type = 0;
    if (0 == _cmd_get_answer_int("device type, 0:DISPLAY_DEVICE_LCM, else:DISPLAY_DEVICE_SPI_LCD >>"))
        type = DISPLAY_DEVICE_LCM;
    else
        type = DISPLAY_DEVICE_SPI_LCD;

//close video_renderer
    kl520_api_cam_disp_ctrl(API_CTRL_DISP_CLOS, NULL, PERMANENT_DISABLE);

    //re-connect display driver.
    kdp_video_renderer_reassign_display_device(type);
    kdp_video_renderer_reconnect_display_panel(type);

//open video_renderer
    kl520_api_cam_disp_ctrl(API_CTRL_DISP_OPEN, NULL, PERMANENT_DISP);

}
#endif

void reserved(void)
{
}
void sample_ui_fill_rect(void)
{
    kl520_api_dp_fresh_bg(BLUE, 2);
}

void sample_io_set_rgb_led(void)
{
    BOOL reset = FALSE;
    int level = 0;
    if (1 == _cmd_get_answer_int("set rgb led attribute, 0:Auto, 1:manual >>"))
        level = _cmd_get_answer_int("set rgb led, level (0, 10, 20 ... 100) >>");
    else
        reset = TRUE;

    kl520_api_set_rgb_led_level(reset, level);
}

void sample_io_open_rgb_led(void)
{
    u16 level = _cmd_get_answer_int("set rgb led, level (0, 10, 20 ... 100) >>");
    rgb_led_open((u16)level);
}
void sample_io_close_rgb_led(void) {rgb_led_close();}

void sample_io_set_nir_led(void)
{
    //int level = _cmd_get_answer_int("set nir led, level (0, 10, 20 ... 100) >>");
    //kl520_api_set_nir_led_level(level);
}

void sample_io_open_nir_led(void)
{
    u16 level = _cmd_get_answer_int("set nir led, level (0, 10, 20 ... 100) >>");
    nir_led_open(level);
}
void sample_io_close_nir_led(void) {nir_led_close();}

void sample_GC1054_gain(void)
{
    u8 cam_idx, gain_h, gain_l;
    u16 level;
    cam_idx = _cmd_get_answer_int("GC1054 camera index >>");
    level = _cmd_get_answer_int("GC1054 set gain (0-2084) >>");
    gain_h = (level >> 8) & 0x0F;
    gain_l = level & 0xFF;
    
    kdp_camera_set_gain(cam_idx, gain_h, gain_l);
}

void sample_GC1054_exp_time(void)
{
    u8 cam_idx, exp_time_h, exp_time_l;
    u16 level;

    cam_idx = _cmd_get_answer_int("GC1054 camera index >>");
    level = _cmd_get_answer_int("GC1054 set exp time (0-8191) >>");
    exp_time_h = (level >> 8) & 0x1F;
    exp_time_l = level & 0xFF;

    kdp_camera_set_exp_time(cam_idx, exp_time_h, exp_time_l);
}

void sample_OV02B1B_gain(void)
{
    u8 cam_idx, gain_h, gain_l;
    u16 level;
    cam_idx = _cmd_get_answer_int("OV02B1B camera index(0-1) >>");
    level = _cmd_get_answer_int("OV02B1B set gain (15-250) >>");
    gain_h = 0;//(level >> 8) & 0x0;
    gain_l = level & 0xFF;
    
    kdp_camera_set_gain(cam_idx, gain_h, gain_l);
}

void sample_OV02B1B_exp_time(void)
{
    u8 cam_idx, exp_time_h, exp_time_l;
    u16 level;

    cam_idx = _cmd_get_answer_int("OV02B1B camera index(0-1) >>");
    dbg_msg_nocrlf("OV02B1B set exp time (%d-%d) >>", MIN_DEFAULT_NIR_EXP_TIME, MAX_DEFAULT_NIR_EXP_TIME);
    level = _cmd_get_answer_int("");
    exp_time_h = (level >> 8) & 0xFF;
    exp_time_l = level & 0xFF;

    kdp_camera_set_exp_time(cam_idx, exp_time_h, exp_time_l);
}

void sample_SP2509_gain(void)
{
    u8 cam_idx, gain_h, gain_l;
    u16 level;
    cam_idx = _cmd_get_answer_int("SP2509 camera index(0-1) >>");
    level = _cmd_get_answer_int("SP2509 set gain (15-220) >>");
    gain_h = 0;//(level >> 8) & 0x0;
    gain_l = level & 0xFF;
    
    kdp_camera_set_gain(cam_idx, gain_h, gain_l);
}

void sample_SP2509_exp_time(void)
{
    u8 cam_idx, exp_time_h, exp_time_l;
    u16 level;

    cam_idx = _cmd_get_answer_int("SP2509 camera index(0-1) >>");
    dbg_msg_nocrlf("SP2509 set exp time (%d-%d) >>", MIN_DEFAULT_NIR_EXP_TIME, MAX_DEFAULT_NIR_EXP_TIME);
    level = _cmd_get_answer_int("");
    exp_time_h = (level >> 8) & 0xFF;
    exp_time_l = level & 0xFF;

    kdp_camera_set_exp_time(cam_idx, exp_time_h, exp_time_l);
}

#if (FB_TILE_RECODE == YES)
void sample_record_tile_val(void)
{
    for(int cam=0; cam<IMGSRC_NUM; cam++)
    {
//        struct frame_info info;
//        kdp_fb_mgr_get_latest_frame_info(cam, &info);
//        dbg_msg_console("  tile[ fb_idx][ cam_isr][til_isr][   diff][fdr_str]   [%6d][%6d][%6d][%6d]    [%6d][%6d][%6d][%6d]    [%6d][%6d][%6d][%6d]    [%6d][%6d][%6d][%6d]",
//                info.tile_val[0],
//                info.tile_val[1],
//                info.tile_val[2],
//                info.tile_val[3],

//                info.tile_val[10],
//                info.tile_val[11],
//                info.tile_val[12],
//                info.tile_val[13],

//                info.tile_val[20],
//                info.tile_val[21],
//                info.tile_val[22],
//                info.tile_val[23],

//                info.tile_val[30],
//                info.tile_val[31],
//                info.tile_val[32],
//                info.tile_val[33]);

        dbg_msg_console("  tile[ fb_idx][ cam_isr][til_isr][   diff][fdr_str][calc_tile][tile_mean]   [tile 0][tile 1][tile 2][tile 3]    [tile10][tile11][tile12][tile13]    [tile20][tile21][tile22][tile23]    [tile30][tile31][tile32][tile33]");

                    
        for(int i=0; i<MAX_FRAME_BUFFER; i++)
        {
            struct frame_record_info info;
            
            kdp_fb_mgr_get_frame_record_info(cam, i,  &info);
        
            dbg_msg_console("cam:%d [%7d][%7d][ %7d][%7d][%7d][%9d][%9d]   [%6d][%6d][%6d][%6d]    [%6d][%6d][%6d][%6d]    [%6d][%6d][%6d][%6d]    [%6d][%6d][%6d][%6d]",
                    cam,
                    info.fb_idx,
                    info._tick_isr_cam,
                    info._tick_isr_tile,
                    info._tick_isr_time_diff,
                    info._tick_fdr_str,
                    info._tick_value_mean,
                    info._tile_value_mean,
            
                    info.tile.tile_val[0],
                    info.tile.tile_val[1],
                    info.tile.tile_val[2],
                    info.tile.tile_val[3],

                    info.tile.tile_val[10],
                    info.tile.tile_val[11],
                    info.tile.tile_val[12],
                    info.tile.tile_val[13],

                    info.tile.tile_val[20],
                    info.tile.tile_val[21],
                    info.tile.tile_val[22],
                    info.tile.tile_val[23],
        
                    info.tile.tile_val[30],
                    info.tile.tile_val[31],
                    info.tile.tile_val[32],
                    info.tile.tile_val[33]);
        }
    }
}
#endif
void sample_io_set_nir_exp_time(void)
{
    u32 time_us = (u32)_cmd_get_answer_int("set nir exp time(us) >>");
    u32 nir_cur_exp_time = time_us * 16 / 14.82;
    kdp_camera_set_exp_time(MIPI_CAM_NIR, (nir_cur_exp_time&0x0000FF00)>>8, nir_cur_exp_time&0xFF);
}

extern osThreadId_t tid_rgb_led_gradually;
void sample_rgb_led_breathe(void)
{
#if (CFG_AI_TYPE == AI_TYPE_R1N1 || CFG_AI_TYPE == AI_TYPE_R1)
    kdp_e2e_rgb_led_gradually();
    if(tid_rgb_led_gradually)
    {
        osThreadJoin(tid_rgb_led_gradually);
        tid_rgb_led_gradually = 0;
    }
#endif
}
void sample_io_rgb_camera_power_on(void) {rgb_camera_power_on();}
void sample_io_rgb_camera_power_off(void) {rgb_camera_power_off();}
void sample_io_nir_camera_power_on(void) {nir_camera_power_on();}
void sample_io_nir_camera_power_off(void) {nir_camera_power_off();}

void sample_io_lcd_power_on(void) { lcd_power_on(); }
void sample_io_lcd_power_off(void) {lcd_power_off();}

void sample_touch_panel_open(void)
{
#if (MEASURE_RECOGNITION == YES)
    kl520_measure_stamp(E_MEASURE_TOUCH_INIT);
    app_kl520_init();
    kl520_measure_stamp(E_MEASURE_TOUCH_INIT_DONE);
#else
#if (CFG_TOUCH_ENABLE == YES)
    kl520_api_touch_open();
    kl520_api_touch_start();
#endif
#endif
}
void sample_touch_panel_close(void)
{
#if CFG_TOUCH_ENABLE == YES
    kl520_api_touch_stop();
    //tp_power_off();
#endif
}
void sample_touch_app_enable(void) {
#if CFG_TOUCH_ENABLE == YES
    sample_app_init_touch_panel();
#if CFG_UI_USR_IMG == NO
    sample_open_video_renderer();
#endif
#endif
}
void sample_touch_app_disable(void) {
#if CFG_TOUCH_ENABLE == YES
#if CFG_UI_USR_IMG == NO
    sample_close_video_renderer();
#endif
    sample_app_deinit_touch_panel();
#endif
}

void sample_face_add_set_timeout(void)
{
    kl520_api_face_add_set_timeout(_cmd_get_answer_int("set timeout, index >>"));
}

void sample_face_recognition_set_timeout(void)
{
    kl520_api_face_recognition_set_timeout(_cmd_get_answer_int("set timeout, index >>"));
}

int sample_input_userinfo(void)
{
    dbg_msg_console("Please enter the username(Up to %d bytes): ", MAX_LEN_USERNAME);
    while(1)
    {
        int nLen;
        char buf[64];
        nLen = kdp_gets(DEBUG_CONSOLE, buf);

        if(nLen > MAX_LEN_USERNAME){
            dbg_msg_console("More than %d bytes: %s",MAX_LEN_USERNAME ,buf);
        }
        else{
            kdp_e2e_face_variables* vars = kdp_e2e_get_face_variables();
            memcpy( vars->user_name, &buf, MAX_LEN_USERNAME );
            dbg_msg_console("Username : %s", vars->user_name);
            
            vars->admin = (u8)_cmd_get_answer_int("Is an administrator:");
            
            break;
        }
    }
    return 1;
}

void sample_get_userinfo(void)
{
    kdp_e2e_face_variables* vars = kdp_e2e_get_face_variables();
    dbg_msg_console("id: 0x%x, admin: %d, username: %s",vars->user_id, vars->admin, vars->user_name);
}

#if (LED_OPEN_MEASUREMENT == YES)
    extern u32 tick_led_open;
#endif
void sample_face_add(void)
{
    u16 height;
    u16 face_add_mode;
    u16 input;

    if (FACE_MODE_NONE != m_face_mode) {
        dbg_msg_console("Err Face Mode");
        return;
    }
    sample_input_userinfo();

#ifdef QUICK_CONSOLE_GUIDELINE
    height = DISPLAY_HEIGHT;
    face_add_mode = 5;
#else
    input = (u16)_cmd_get_answer_int("set the height of display(0~DISPLAY_HEIGHT): ");
    height = (input == 0)?DISPLAY_HEIGHT:input;
#if MAX_FID == 1
    input = 1;
#else
    input = (u16)_cmd_get_answer_int("set face_add mode: [1: 1 face mode, 5: 5 faces mode] >>");
#endif
    face_add_mode = (input == 0)?5:input;
    input = (u16)_cmd_get_answer_int("set multiple id mode:[1: multi id, 0: single id] >>");
    set_enroll_overwrite_flag(input);

#if (LED_OPEN_MEASUREMENT == YES)
    u32 tick_start = osKernelGetTickCount();
    tick_led_open = 0;
#endif
    sample_force_abort_enable();

#endif
    kl520_face_add_type add_type = FACE_ADD_TYPE_NORMAL;
    if (1 == face_add_mode) {
//        kl520_api_face_add_set_timeout(5);
        kl520_api_face_set_add_mode(FACE_ADD_MODE_1_FACE);

        if(kl520_api_face_add_internal(0, 0, DISPLAY_WIDTH, height, add_type) != KL520_FACE_OK)
        {
            dbg_msg_algo ("add 1-face failed.");
        }
    }
    else if (5 == face_add_mode) {
        kl520_api_dp_five_face_enable();
//        kl520_api_face_add_set_timeout(10);
        kl520_api_face_set_add_mode(FACE_ADD_MODE_5_FACES);

        for (add_type = FACE_ADD_TYPE_NORMAL; add_type <= FACE_ADD_TYPE_DOWN; add_type++) 
        {
            if(kl520_api_face_add_internal(0, 0, DISPLAY_WIDTH, height, add_type) != KL520_FACE_OK)
            {
                dbg_msg_algo ("add 5-face failed:%d.", add_type);
                break;
            }
        }
    }
    if(tid_abort_thread != 0)
        sample_force_abort_disable();

    //sample_face_close(); or manually input item 38!
#if (LED_OPEN_MEASUREMENT == YES)
    u32 tick_end = osKernelGetTickCount();
    dbg_msg_algo ("face add time:%d, led open time:%d.", tick_end - tick_start, tick_led_open);
#endif
}

void sample_face_recognition(void)
{
    int ret = KL520_APP_FLAG_FDFR_ERR;
    u8 face_id = 0;
    u32 events = 0;
    u16 input = 0;
    system_info t_sys_info = { 0 };

    //kl520_api_face_recognition_set_timeout(10);

#if (MEASURE_RECOGNITION == YES)
    input = 0;
#else
    input = (u16)_cmd_get_answer_int("set the height of display(0~DISPLAY_HEIGHT): ");
#endif

#if (LED_OPEN_MEASUREMENT == YES)
    u32 tick_start = osKernelGetTickCount();
    dbg_msg_algo("face recog start...");
    tick_led_open = 0;
#endif

    sample_force_abort_enable();

    int face_ret = kl520_api_face_recognition(0, 0, DISPLAY_WIDTH, (input == 0)?DISPLAY_HEIGHT:input);

    kl520_measure_stamp(E_MEASURE_API_RUN_STAT_CHK);
    do {
        if(face_ret != -1) {
            events = wait_event(kl520_api_get_event(), KL520_APP_FLAG_FDFR | KL520_DEVICE_FLAG_ERR);
        }
        //dbg_msg_console("sample_face_recognition events=%x", events);
        if(events == KL520_DEVICE_FLAG_ERR)
        {
            ret = kl520_api_get_device_info(&t_sys_info);
            dbg_msg_err("[%s], DEVICE ERROR, ret=0x%x", __func__, ret);
            kl520_api_free_device_info(&t_sys_info);
            break;
        }
        else
        {
            ret = kl520_api_face_get_result(&face_id);
            if (KL520_FACE_OK == ret)
            {
                kdp_e2e_face_variables *vars_cur = kdp_e2e_get_face_variables();
    #ifdef DB_DRAWING_CUSTOMER_COLOR
                kl520_api_face_notify(KL520_FACE_DB_OK);
    #endif
                kl520_measure_stamp(E_MEASURE_FACE_FACE_OK);
                dbg_msg_console("sample_face_recognition, KL520_FACE_OK, face_id=0x%x, Admin=%d  UseName=%s", face_id, vars_cur->admin, vars_cur->user_name);
                break;
            }
            else if (KL520_FACE_DB_FAIL == ret) {
    #ifdef DB_DRAWING_CUSTOMER_COLOR
                kl520_api_face_notify(KL520_FACE_DB_FAIL);
    #endif
                kl520_measure_stamp(E_MEASURE_FACE_DB_FAIL);
                dbg_msg_console("sample_face_recognition, KL520_FACE_DB_FAIL, face_id=0x%x", face_id);
                break;
            }
            else if (ret >= KL520_FACE_TOO_FAR && ret <= KL520_FACE_LOW_QUALITY )
            {
                dbg_msg_console("KL520_FACE NOTE, ret=0x%x", ret);
            }
            else {
                dbg_msg_console("sample_face_recognition, ERROR, ret=0x%x", ret);
                break;
            }
        }
		osDelay(1);

    }while (1);

    if(tid_abort_thread != 0)
        sample_force_abort_disable();

#ifdef DEV_TEST_VERSION
    dbg_msg_algo ("===FACE RECOG STAT: total:%d, suc:%d.", _face_recog_count, _face_succ_count);
#endif
    
    kl520_measure_stamp(E_MEASURE_FACE_REC_END);
    //sample_face_close(); or manually input item 38!
#if (LED_OPEN_MEASUREMENT == YES)
    u32 tick_end = osKernelGetTickCount();
    dbg_msg_console ("face recog time:%d. led open:%d.", tick_end - tick_start, tick_led_open);
#endif
}

void sample_face_liveness(void)
{
    int ret;
    u8 face_id = 0;
    u32 events = 0;
    u16 input = 0;
    system_info t_sys_info = { 0 };

    //kl520_api_face_liveness_set_timeout(60);
    input = (u16)_cmd_get_answer_int("set the height of display(0~DISPLAY_HEIGHT): ");

    int face_ret = kl520_api_face_liveness(0, 0, DISPLAY_WIDTH, (input == 0)?DISPLAY_HEIGHT:input);

    do {
        if(face_ret != -1) {
            events = wait_event(kl520_api_get_event(), KL520_APP_FLAG_FDFR | KL520_DEVICE_FLAG_ERR);
        }
        dbg_msg_console("[%s] events=%x", __func__, events);
        if(events == KL520_DEVICE_FLAG_ERR)
        {
            ret = kl520_api_get_device_info(&t_sys_info);
            dbg_msg_err("[%s], DEVICE ERROR, ret=0x%x", __func__, ret);
            kl520_api_free_device_info(&t_sys_info);
        }
        else
        {
            ret = kl520_api_face_get_result(&face_id);
            dbg_msg_console("[%s] ret=%x", __func__, ret);
            if (KL520_FACE_OK == ret)
            {
                dbg_msg_console("[%s], KL520_FACE_OK", __func__);
            }
            else {
                dbg_msg_console("[%s], ERROR, ret=0x%x", __func__, ret);
            }
        }
        break;
    }while (0);

    //sample_face_close(); //or manually input item 38!
}

void sample_face_recognition_without_timeout_size(void)
{
    int ret;
    u8 face_id = 0;
    u32 events = 0;
    //u32 flags;

    kl520_api_face_recognition(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);

    do {
        events = wait_event(kl520_api_get_event(), KL520_APP_FLAG_FDFR);
        dbg_msg_console("sample_face_recognition events=%x", events);
        ret = kl520_api_face_get_result(&face_id);
        dbg_msg_console("sample_face_recognition ret=%x", ret);
        if (KL520_FACE_OK == ret)
        {
#ifdef DB_DRAWING_CUSTOMER_COLOR
            kl520_api_face_notify(KL520_FACE_DB_OK);
#endif
            dbg_msg_console("sample_face_recognition, KL520_FACE_OK, face_id=0x%x", face_id);
        }
        else if (KL520_FACE_DB_FAIL == ret) {
#ifdef DB_DRAWING_CUSTOMER_COLOR
            kl520_api_face_notify(KL520_FACE_DB_FAIL);
#endif
            dbg_msg_console("sample_face_recognition, KL520_FACE_DB_FAIL, face_id=0x%x", face_id);
        }
        else {
            dbg_msg_console("sample_face_recognition, ERROR, ret=0x%x", ret);
        }
        break;
    }while (0);

    //sample_face_close(); or manually input item 38!
}

void sample_face_recognition_test(void)
{
    u16 input = 0;
    s32 ret = 0;
    system_info t_sys_info = { 0 };

    input = (u16)_cmd_get_answer_int("set the height of display(0~DISPLAY_HEIGHT): ");
    ret = kl520_api_face_recognition_test(0, 0, DISPLAY_WIDTH, (input == 0)?DISPLAY_HEIGHT:input);
    if(ret !=0)
    {
        ret = kl520_api_get_device_info(&t_sys_info);
        dbg_msg_err("sample_face_recognition_test device error = %x", ret);
        kl520_api_free_device_info(&t_sys_info);
    }
}

void sample_face_close(void)
{
    kl520_measure_stamp(E_MEASURE_FACE_CLOSE_STR);
    kl520_api_face_close();
    kl520_measure_stamp(E_MEASURE_FACE_CLOSE_END);

#if CFG_FMAP_EXTRA_ENABLE == YES
    kl520_api_extra_fmap_close();
#endif

    kl520_api_sim_set_rst();
}

void sample_settings_delete(void)
{
    kl520_api_settings_delete();
}

void sample_face_del_all(void)
{
    char buf[256];

    dbg_msg_nocrlf("input password to delete: ");
    kdp_gets(DEBUG_CONSOLE, buf);
    dbg_msg_nocrlf("");

    if(strcmp(buf, "kneron")) return;

    kl520_api_face_del(1, (u8)0);
    
#ifdef CUSTOMIZE_DB_OFFSET
    reset_user_db_offset();
#endif
}

void sample_face_del_user(void)
{
    kl520_api_face_del(2, _cmd_get_answer_int("set the user id to delete: "));
}

#if (SAMPLE_STABILITY_ENABLE == YES)
void sample_face_del_user_2(void)
{
    u16 user_idx = _cmd_get_answer_int("set the user idx to delete: ");

    kapp_db_user_data_t *data = kdp_e2e_get_db_data();
    data->del_all = 0;
    data->user_id_in = user_idx + kl520_api_get_start_user_id();
    //data->user_idx = user_idx;

    int ret = kdp_app_db_delete(data);
    if(ret != KAPP_OK) {
        dbg_msg_console ("del user :%d failed", user_idx);
    }
}
#endif

void sample_fr_threshold(void)
{
    u8 level = kdp_e2e_get_fr_threshold_level();
    char str[256];
    snprintf(str, 256, "set fr threshold level (%d) :", level);
    u8 n_l = _cmd_get_answer_int(str);
    if(n_l > 4) {
        dbg_msg_console("fr threshold level should be 0~4");
        return;
    }
    kdp_e2e_set_fr_threshold_level(n_l);
    
    dbg_msg_console("fr threshold level set to %d.", n_l);
    
    kl520_customer_info Cusinfo;
    kl520_api_customer_get(&Cusinfo);
    Cusinfo.verify_threshold = n_l;
    kl520_api_customer_write(&Cusinfo);
    
    return;
}

void sample_face_query_all(void)
{
    int i = 0;
    kdp_e2e_db_extra_data tmp;
    kdp_e2e_db_extra_data *pvars = &tmp;

    u16 valid_fm0 = 0, valid_fm1 = 0, type = 0;
    bool tittle = true;

    u8 total_id_num;
    u8 face_status[MAX_USER];
    
    kl520_api_face_query_all(&total_id_num, &face_status[0]);

    for (i = 0; i < MAX_USER; i++)
    {
        s32 ret = kdp_e2e_db_get_user_info_by_idx(i, &valid_fm0, &valid_fm1, &type);
        if(ret != E2E_OK) continue;
        
        if(0 < valid_fm0 && TYPE_VALID == type)
        {
            memset(&tmp, 0, sizeof(tmp));
            kdp_e2e_db_extra_read(i, &tmp, sizeof(tmp));
            
            u8 uid = _kl520_app_calc_db_uid(i);

            if(tittle == true){
                dbg_msg_console("[  i][  id][pre_add][Adm][                            Name][mode][fm0][fm1]");
                tittle = false;
            }
            dbg_msg_console("[%3d][%4d][%5d  ][%3d][%32s][%4d][%3d][%3d]",
                            i, uid, pvars->pre_add,
                            pvars->admin, pvars->user_name,
                            pvars->nir_mode, valid_fm0, valid_fm1);
        }

    }
    dbg_msg_console("Num Face DB:%d", total_id_num);
}

void sample_face_query_user(void)
{
    int ret = 0;
    u16 user_id = _cmd_get_answer_int("set the user id to query: ");
    ret = kl520_api_face_query(user_id);
    if (KL520_FACE_EXIST == ret) {
        dbg_msg_console("user id[%u] is existed");
    }
    else if (KL520_FACE_EMPTY == ret) {
        dbg_msg_console("user id[%u] is empty");
    }
}

void sample_pwm_timer_test_init(void)
{
    kl520_api_timer_init(PWMTIMER4, PWMTMR_5000MSEC_PERIOD);
}
void sample_pwm_timer_test_close(void)
{
    kl520_api_timer_close(PWMTIMER4);
}

#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
void sample_ssp_spi(void) {
    char buf[256];
    int id = 0;
    kdp_flash_initialize();
    kdp_flash_get_info();

    while(1)
    {
        dbg_msg_console("open SSP SPI >> ");
        kdp_gets(DEBUG_CONSOLE, buf);
        dbg_msg_console("");
        id = atoi(strtok(buf, " \r\n\t"));

        dbg_msg_console("=========================");
        dbg_msg_console("<1>SPI initial ....");
        dbg_msg_console("<2>Read font chip..");
        dbg_msg_console("<999>Return");

        if( id == 1 )
            kl520_api_ssp_spi_master_init();
        else if( id == 2 )
            font_test_read();
        else if(id == 999)
            return;
    }
}
#else
void sample_ssp_spi(void) {
    dbg_msg_console("warning, SSP_SPI_MASTER_EN is not defined.\n");
}
#endif



extern FLASH_PARMATER_T     st_flash_info;

#if (OTA_FULL_CONSOLE_EN == YES)
void sample_ota_model(void)
{
    char buf[256];
    //int id = 0;
    while(1)
    {
        dbg_msg_console("open flash ota>> ");
        kdp_gets(DEBUG_CONSOLE, buf);
        dbg_msg_console("");
        //id = atoi(strtok(buf, " \r\n\t"));
        dbg_msg_console("<0....>Uart initial ....");
        dbg_msg_console("<1....>Get Flash info..");
        dbg_msg_console("<3....>exit..");
    }
}
#endif


void sample_Flash_Info(void)
{
    char buf[256];
    int id = 0;
    dbg_msg_console("-- Flash processing start --\n\r ");
    kdp_flash_initialize();
    kdp_flash_get_info();

    while(1)
    {

        dbg_msg_console("open flash >> ");
        kdp_gets(DEBUG_CONSOLE, buf);
        dbg_msg_console("");
        id = atoi(strtok(buf, " \r\n\t"));

        dbg_msg_console("=========================");
        dbg_msg_console("<0>Uart initial ....");
        dbg_msg_console("<1>Get Flash info..");
        #if (FLASH_VENDOR_SELECT == GD25S512MD )
        dbg_msg_console("<2>Set Die 0...");
        dbg_msg_console("<3>Set Die 1...");
        dbg_msg_console("<4>Get active Die");
        #endif

        #if (OTA_USER_BACKUP == YES )
        dbg_msg_console("<5>Init user config...");
        dbg_msg_console("<6>switch to wait active all...");

        dbg_msg_console("<61>switch to wait active fw+model");
        dbg_msg_console("<62>switch to wait active UI...");

        dbg_msg_console("<7>switch wait active to active");
        dbg_msg_console("<8>switch to active...");
        dbg_msg_console("<9>get current status...");
        dbg_msg_console("<10>set fake...");
        dbg_msg_console("<11>Get user config status...");
        dbg_msg_console("<12>Get user offset...");
        dbg_msg_console("<13>Get inactive area...");
        #endif

        dbg_msg_console("<555>Init boot config");

        #if (OTA_FULL_CONSOLE_EN == YES)
        dbg_msg_console("<556>Update boot-config to confirmed");
        dbg_msg_console("<557>Switch to SCPU");
        dbg_msg_console("<558>Switch to NCPU");
        dbg_msg_console("<559>Get to SCPU/NCPU status");
        dbg_msg_console("<665>Show model version");
        dbg_msg_console("<666>Show model info...");
        dbg_msg_console("<667>model info test...");
        dbg_msg_console("<668>calc model info...");
        dbg_msg_console("<669>read another model.");
        dbg_msg_console("<777>model info experiment...");
        dbg_msg_console("<778>model info CRC_check...");
        dbg_msg_console("<888>enter ota user debug mode");
        #endif
        dbg_msg_console("<999>return");

        if( id == 1 )
        {
            kdp_flash_initialize();
            kdp_flash_get_info();
            dbg_msg_console(" *Flash Vendor ID: %X\r\n", kdp_flash_get_id());
            dbg_msg_console(" *Flash Size: %d K Bytes\r\n",   (st_flash_info.flash_size_KByte+1) );
            dbg_msg_console(" *Flash sector numbers: %d \r\n", st_flash_info.total_sector_numbers );
            dbg_msg_console(" *Flash one block with sector numbers : %d\r\n", st_flash_info.block_size_Bytes );
            dbg_msg_console(" *Flash one page size : %d Bytes\r\n", st_flash_info.page_size_Bytes );
        }
        else if( id == 0){
//            kdp_uart_print_register(UART4_DEV);
        }
        #if (FLASH_VENDOR_SELECT == GD25S512MD )
        else if( id ==2 ){
            nor_flash_die_selection(0);
        }
        else if( id==3){
            nor_flash_die_selection(1);
        }
        else if( id==4){
            dbg_msg_console("Active die 0x%x \r\n", nor_flash_get_active_die() );
        }
        #endif

        #if(OTA_USER_BACKUP == YES )
        else if(id==5){
            dbg_msg_console("user config init start");
            ota_user_config_init();
        }
        else if(id ==6){
            dbg_msg_console("user config enter inactive");
            ota_user_select_inactive_area(USER_PARTITION_FW_INFO);
            ota_user_select_inactive_area(USER_PARTITION_MODEL);
            #if ( OTA_USER_BACKUP_SEPERATE == NO )
            ota_user_select_inactive_area(USER_PARTITION_MODEL);
            #endif
            ota_user_select_inactive_area(USER_PARTITION_UI_IMG);
        }
        else if(id ==61){
            dbg_msg_console("user config enter inactive");
            ota_user_select_inactive_area(USER_PARTITION_FW_INFO);
            ota_user_select_inactive_area(USER_PARTITION_MODEL);

            #if ( OTA_USER_BACKUP_SEPERATE == NO )
            ota_user_select_inactive_area(USER_PARTITION_MODEL);
            #endif
        }
        else if(id ==62){
            dbg_msg_console("user config enter inactive");
            #if ( OTA_USER_BACKUP_SEPERATE == NO )
            ota_user_select_inactive_area(USER_PARTITION_MODEL);
            #endif
            ota_user_select_inactive_area(USER_PARTITION_UI_IMG);
        }

        else if(id==7){
            dbg_msg_console("user config to wait active");
            ota_user_select_wait_active_area(USER_PARTITION_FW_INFO);
            ota_user_select_wait_active_area(USER_PARTITION_MODEL);
            ota_user_select_wait_active_area(USER_PARTITION_UI_IMG);
        }
        else if(id==8){
            dbg_msg_console("user config switch to active");
            dbg_msg_console("   return :%d", ota_user_area_boot_check() );

        }
        else if(id==9){
            #if ( OTA_USER_BACKUP_SEPERATE == YES )
            dbg_msg_console("get UI area idx: %d", ota_user_get_active_area(USER_PARTITION_FW_INFO) );
            dbg_msg_console("get fw_info and model area idx %d", ota_user_get_active_area(USER_PARTITION_UI_IMG) );
            #else
            dbg_msg_console("get current area idx");
            ota_user_get_active_area();
            #endif
        }
        else if(id == 10){
            dbg_msg_console("dummy change value");
            ota_user_dummy_changes_status();
        }
        else if(id == 11){
            dbg_msg_console("user config status");
            ota_user_debug_show();
        }
        else if(id == 12){
            dbg_msg_console("current offset");
            dbg_msg_console("fw info offset 0x%x", kdp_get_fwinfo_offset());
            dbg_msg_console("model offset 0x%x", kdp_get_model_offset());
            dbg_msg_console("ui offset: 0x%x " , user_get_ui_offset());
        }
        else if(id == 13){

            dbg_msg_console("fw info inactive area %d", ota_user_check_on_going_area(USER_PARTITION_FW_INFO) );
            dbg_msg_console("model inactive area  %d", ota_user_check_on_going_area(USER_PARTITION_MODEL) );
            dbg_msg_console("ui inactive area %d " , ota_user_check_on_going_area(USER_PARTITION_UI_IMG) );
        }
        #endif
        else if( id == 555 ){
            ota_init_partition_boot_cfg();
        }
#if (OTA_FULL_CONSOLE_EN == YES)
        else if( id == 556 ){
            ota_handle_first_time_boot();
        }
        else if( id == 557 )
        {
            ota_update_show_config();
            kl520_api_ota_switch_SCPU();
            ota_update_show_config();
        }
        else if( id == 558 )
        {
            ota_update_show_config();
            kl520_api_ota_switch_NCPU();
            ota_update_show_config();
        }
        else if( id==559 )
        {
            dbg_msg_console("SCPU active area:%d", ota_get_active_scpu_partition() );
            dbg_msg_console("NCPU active area:%d", ota_get_active_ncpu_partition() );
        }
        else if( id==665 )
        {
            dbg_msg_console("Model numbers :%d", kl520_api_model_count());
            dbg_msg_console("Model 0 version :%d", kl520_api_model_version(0));
            dbg_msg_console("Second Last Model index:%d version :%d",kl520_api_model_count()-2, kl520_api_model_version( kl520_api_model_count()-2 ) );
            dbg_msg_console("Last Model index:%d version :%d",kl520_api_model_count()-1, kl520_api_model_version( kl520_api_model_count()-1 ) );
            dbg_msg_console("Model numbers :%d", kdp_model_get_model_count() );
            dbg_msg_console("All Model size :%d", kdp_clc_all_model_size() );
            dbg_msg_console("Model 0 version :%d", kdp_model_version(0) );
            dbg_msg_console("Second Last Model index:%d version :%d", kdp_model_get_model_count()-2, kdp_model_version( kdp_model_get_model_count()-2 ) );
            dbg_msg_console("Last Model index:%d version :%d", kdp_model_get_model_count()-1, kdp_model_version( kdp_model_get_model_count()-1 ) );
            dbg_msg_console("CRC offset in fw info :%d, value: 0x%x", kdp_crc_offset_in_fwinfo(), drv_read_all_model_crc() );
        }
        else if( id==666 )
        {
            dbg_msg_console("Model numbers :%d", kdp_model_get_model_count() );
            kdp_model_show_info();

            for(int kk=0; kk<kdp_model_get_model_count(); kk++)
            {
                dbg_msg_console("Model idx :%d, size: %d", kk, kdp_clc_each_model_size(kk) );
                dbg_msg_console("Model idx :%d, crc: 0x%x", kk, drv_read_each_model_crc(kk) );
            }
//            kdp_model_info_reload_test();
        }
        else if( id==667 )
        {
            kdp_model_info_clear();
            kdp_model_show_info();
        }
        else if( id==668 )
        {
            uint32_t a=0, c=0, d =0;
            kdp_model_info_get( &a, &c, &d );
            dbg_msg_console("second last start: 0x%x, c:0x%x, partial size:0x%x ", a,c,d );
        }
        #if (OTA_USER_BACKUP == YES )
        else if( id==669 )
        {
            //force read another area
            dbg_msg_console("re-load model info");
            kdp_set_model_offset(KDP_FLASH_ALL_MODEL_OFFSET_1);
            kdp_set_fwinfo_offset(KDP_FLASH_FW_INFO_OFFSET_1);
            kdp_model_info_reload();
            dbg_msg_console("all model size: %d ", kdp_clc_all_model_size() );
        }
        #endif
        else if( id==777 )
        {
//            return;
        }
        else if( id==778 )
        {
            u8  *ptr = (u8 *)KDP_DDR_MODEL_START_ADDR;
            u32 target_length = 16000000;
            dbg_msg_console("model address 0x%x ", ptr );
            dbg_msg_console("crc result is 0x%x ", ota_crc32(ptr , target_length) );

            //calculate CRC....
//            return;
        }

        else if(id==888)
        {
            sample_ota_model();
        }
#endif
        else if(id==999 )
        {
            return;
        }

    }
}

void sample_get_system_info(void)
{
    int ret = 0;
    system_info t_sys_info = { 0 };
    ret = kl520_api_get_device_info(&t_sys_info);

    dbg_msg_console("unique_id : 0x%x", t_sys_info.unique_id);
    dbg_msg_console("boot loader version : %x", t_sys_info.spl_version);
    dbg_msg_console("scpu firmware version : %d (%d.%d.%d.%d)", t_sys_info.fw_scpu_version.date,
                                                                t_sys_info.fw_scpu_version.version[0],
                                                                t_sys_info.fw_scpu_version.version[1],
                                                                t_sys_info.fw_scpu_version.version[2],
                                                                t_sys_info.fw_scpu_version.version[3]);
    dbg_msg_console("ncpu firmware version : %d (%d.%d.%d.%d)", t_sys_info.fw_ncpu_version.date,
                                                                t_sys_info.fw_ncpu_version.version[0],
                                                                t_sys_info.fw_ncpu_version.version[1],
                                                                t_sys_info.fw_ncpu_version.version[2],
                                                                t_sys_info.fw_ncpu_version.version[3]);
#if ( UART_PROTOCOL_VERSION >= 0x0200 )
    dbg_msg_console("protocol version : 0x%x", t_sys_info.com_protocol_version);
#endif

    for (int i = 0; i < t_sys_info.model_count; ++i) {
        dbg_msg_console("model[%d] type:0x%x, version:0x%x",
            i, t_sys_info.model_infos[i]->model_type, t_sys_info.model_infos[i]->model_version);
    }
    
    struct fw_misc_data *all_model_version = kl520_api_get_model_version();
    dbg_msg_console("all model version: %d.%d.%d.%d", all_model_version->version[0],
        all_model_version->version[1], all_model_version->version[2], all_model_version->version[3]);

    dbg_msg_console("device name : %s, id : 0x%x", t_sys_info.device_id_0.device_name, t_sys_info.device_id_0.id);
    dbg_msg_console("device name : %s, id : 0x%x", t_sys_info.device_id_1.device_name, t_sys_info.device_id_1.id);
    dbg_msg_console("device name : %s, id : 0x%x", t_sys_info.device_id_2.device_name, t_sys_info.device_id_2.id);
    dbg_msg_console("device name : %s, id : 0x%x", t_sys_info.device_id_3.device_name, t_sys_info.device_id_3.id);
    dbg_msg_console("device name : %s, id : 0x%x", t_sys_info.device_id_4.device_name, t_sys_info.device_id_4.id);
    for (u32 i = 0; i < t_sys_info.extra_device_cnt; ++i) {
        dbg_msg_console("device name : %s, id : 0x%x",
            t_sys_info.extra_device_id_array[i].device_name, t_sys_info.extra_device_id_array[i].id);
    }
    //dbg_msg_console("device name : %s, id : %x", t_sys_info.device_id_5.device_name, t_sys_info.device_id_5.id);
    dbg_msg_console("Device status = 0x%x", ret);

    kl520_api_free_device_info(&t_sys_info);
}

void sample_stability_recognition_test(void)
{
    int count = 0;
    while(1)
    {
        kl520_api_face_recognition_test(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
        osDelay(3000);
        sample_face_close();
        osDelay(2000);
        count++;
        dbg_msg_console("==========================================================================%d", count);
        dbg_msg_console("Thread NUM:%d",osThreadGetCount());

    }
}
osTimerId_t tiemr_id;
void _stability_recognition_tc(void *arg)
{
    static int count = 0;
    //dbg_msg_console("*(int*)arg=%d", *((int*)arg));
    sample_face_recognition_without_timeout_size();
    osDelay(1);
    sample_face_close();
    ++count;
    dbg_msg_console("=========================================================%d", count);
    dbg_msg_console("Thread NUM:%d",osThreadGetCount());
}

void sample_stability_recognition(void)
{
    static const int k_wish_timeout = 3;
    int bytimer = _cmd_get_answer_int("[0:for , 1:timer >>");

    kl520_api_face_recognition_set_timeout(k_wish_timeout);
    if (bytimer) {

        tiemr_id = osTimerNew(_stability_recognition_tc, osTimerPeriodic, (void*)&k_wish_timeout, NULL);
        osTimerStart(tiemr_id, (k_wish_timeout + 2) * 1000);
    }
    else {
        int count = 0;
        for (;;) {
            sample_face_recognition_without_timeout_size();
            osDelay(1);
            sample_face_close();
            osDelay(k_wish_timeout * 1000);

            count++;
            dbg_msg_console("==========================================================================%d", count);
            dbg_msg_console("Thread NUM:%d",osThreadGetCount());
        }
    }
}

void sample_stability_all(void)
{
    int count = 0;
    int all = _cmd_get_answer_int("[0:touch only, 1:all >>");
    //BOOL app_touch_is_inited;
    if (all) {

        sample_touch_panel_open();
        sample_touch_app_enable();

        while(1)
        {
        #if CFG_TOUCH_ENABLE == YES
            //app_touch_is_inited = sample_app_touch_is_inited();
            //dbg_msg_console("app_touch_is_inited=%d", app_touch_is_inited);
            //if (1 == app_touch_is_inited)
            //    sample_touch_app_disable();
        #endif

            kl520_api_face_recognition_test(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);

        #if CFG_TOUCH_ENABLE == YES
            //if (1 == app_touch_is_inited)
            //    sample_touch_app_enable();
        #endif


            osDelay(2000);
            sample_face_close();
            osDelay(1000);
            count++;
            dbg_msg_console("==========================================================================%d", count);
            dbg_msg_console("Thread NUM:%d",osThreadGetCount());
        }
    }
    else {
        for (;;) {

            sample_touch_panel_open();
            osDelay(1000);
            sample_touch_app_enable();
            osDelay(3000);
            sample_touch_app_disable();
            osDelay(1000);
            sample_touch_panel_close();
            osDelay(1000);

            count++;
            dbg_msg_console("==========================================================================%d", count);
            dbg_msg_console("Thread NUM:%d",osThreadGetCount());
        }
    }
}

void sample_stability_add_user(void)
{
    int count = 0;
    u8 face_id = 0;
    u32 events = 0;

    while(1)
    {
        kl520_api_face_add_set_timeout(2);
#if ( KDP_BIT_CTRL_MODE == YES )
        kl520_api_face_add(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, BIT_CTRL_FACE_ADD_NORMAL);
#else
        kl520_api_face_add(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, FACE_ADD_TYPE_NORMAL);
#endif

        events = wait_event(kl520_api_get_event(), KL520_APP_FLAG_FDFR);
        dbg_msg_console("sample_face_add events=%x", events);
        int ret = kl520_api_face_get_result(&face_id);
        dbg_msg_console("sample_face_add ret=%x", ret);

        if (KL520_FACE_OK == ret){
            dbg_msg_console("sample_face_add, KL520_FACE_OK, face_id=0x%x", face_id);}
        else if(KL520_FACE_EXIST == ret){
            dbg_msg_console("sample_face_add, KL520_FACE_EXIST");}
        else{
            dbg_msg_console("sample_face_add, ERROR, ret=0x%x", ret);}

        osDelay(2000);

        sample_face_close();
        count++;
        dbg_msg_console("==========================================================================%d", count);
    }

}

void sample_power_manager_sw_reset(void)
{
    power_mgr_sw_reset();
}

void sample_power_manager_shutdown(void)
{
    power_manager_shutdown();
}

void sample_get_adc(void)
{
    int adc_value = 0;
    int adc_chn = _cmd_get_answer_int("[input ADC channel(0~3) >>");

    kdp520_adc_init();
    adc_value = kdp520_adc_read(adc_chn);

    dbg_msg_console("ADC[%d] = %d", adc_chn, adc_value);
}

void sample_switch_rbg_nir(void)
{
    u8 cam_idx;
    struct video_input_params params;
    
    kdp_video_renderer_next_idx();
    cam_idx = kdp_video_renderer_get_idx();
    
    params = kdp_video_renderer_setting(cam_idx); 
    kdp_video_engineering_switch(&params);

    kl520_api_camera_start(cam_idx);
}

void sample_sim_fdfr(void)
{
#if CFG_USB_SIMTOOL == 1
    kl520_sim_ctx ctx = {0};
    kl520_api_sim_fdfr(&ctx);
#endif
}

void sample_face_integrate(void)
{
    for(;;)
    {
        dbg_msg_console("==================add===============");
        sample_stability_add_user();
        //sample_face_add();
        //sample_face_close();
        dbg_msg_console("==================rec===============");
        sample_stability_recognition();
        //sample_face_close();
        sample_face_query_all();
    }
}

#if CFG_FMAP_EXTRA_ENABLE == YES

void sample_fetch_db_fmap()
{
    char buf[256];
    u8 idx;
    dbg_msg_console("fetch_face_map:");
    dbg_msg_console("(100)all");
    dbg_msg_console("(0-60)userID");
    //Welcome to add conditions
    kdp_gets(DEBUG_CONSOLE, buf);
    dbg_msg_nocrlf("");
    idx = atoi(strtok(buf, " \r\n\t"));

        if( idx == 100 )
        {
            // fetch all user
            int i = 0;
            u8 total_id_num;
            u8 face_status[MAX_USER];

            kl520_api_face_query_all(&total_id_num, &face_status[0]);
            for(i=0; i<total_id_num; i++)
            {
                dbg_msg_console("idx = %2d, id : %#x exist, addr: %#x, size: %#x", i, face_status[i], kdp_app_db_get_user_addr(i), kdp_app_db_get_user_size() );
            }
            dbg_msg_console("Num Face DB:%d", total_id_num);

        }
        else
        {
            // fetch one user by ID.
            int i = 0;
            int size = kdp_app_db_get_user_size()/4;
            u8 total_id_num;
            u8 face_status[MAX_USER];
            u32* pData;
            union IntFloat {
        int32_t i;
        float f;
            };
            union IntFloat val;

            kl520_api_face_query_all(&total_id_num, &face_status[0]);
            dbg_msg_console("idx = %2d, id : %#x exist, addr: %#x, size: %#x", idx, face_status[idx], kdp_app_db_get_user_addr(idx), kdp_app_db_get_user_size() );
            pData = (u32*) kdp_app_db_get_user_addr(idx);
            if(pData == 0) return;

            for( i = 0; i < size; i++ )
            {
                val.i = *(pData++);
                dbg_msg_console( "emb[%4d]: %#f", i, val.f );
            }
        }
}

void sample_face_no_db_add_mode(void)
{
    char buf[256];
    u8 idx;
    dbg_msg_console("DB in Flash or not:");
    dbg_msg_console("(1)WRITE_IDTO_DB");
    dbg_msg_console("(2)WITHOUT_DB");
    //Welcome to add conditions
    kdp_gets(DEBUG_CONSOLE, buf);
    dbg_msg_nocrlf("");
    idx = atoi(strtok(buf, " \r\n\t"));

    if( idx == 2 )
        kl520_api_face_set_db_add_mode( FACE_ADD_MODE_NO_DB );
    else
        kl520_api_face_set_db_add_mode( FACE_ADD_MODE_IN_DB );
    kl520_api_extra_fmap_mode(FALSE);

    sample_face_add();

}

void sample_extract_fmap_autousb(void)
{
    char buf[256];
    u8 idx;
    dbg_msg_console("DB in Flash or not:");
    dbg_msg_console("(0)close fetch feature map");
    dbg_msg_console("(1)only output when fr is ready");
    dbg_msg_console("(2)always output (bypass mode)");
    //Welcome to add conditions
    kdp_gets(DEBUG_CONSOLE, buf);
    dbg_msg_nocrlf("");
    idx = atoi(strtok(buf, " \r\n\t"));

    kl520_api_extra_fmap_adv_init(MIPI_CAM_NIR, KDP_DDR_TEST_RGB_FR_SIZE);
    kl520_api_extra_fmap_adv_init(MIPI_CAM_RGB, KDP_DDR_TEST_NIR_FR_SIZE);

    if( idx == 1 )
    {
        kl520_api_extra_fmap_mode(TRUE);
        KL520_api_set_extra_fmap_adv_mode( EXTRA_FMAP_FR_e );
    }
    else if( idx == 2 )
    {
        kl520_api_extra_fmap_mode(TRUE);
        KL520_api_set_extra_fmap_adv_mode( EXTRA_FMAP_BYPASS_e );
    }
    else
    {
        kl520_api_extra_fmap_mode(FALSE);
        KL520_api_set_extra_fmap_adv_mode( EXTRA_FMAP_CLOSE_e );
    }

    sample_face_recognition_test();
}

#endif //#if CFG_FMAP_EXTRA_ENABLE == YES

extern uint16_t uart_sample_face_add_timeout(uint16_t time_out_ms)
{
    u16 face_add_mode = 5, ret;

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP ) || ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP_USR )
    u16 nFaceId = 0xFFFF;
    u8  nResult = MR_SUCCESS;
    u8  eFaceType = 0;
#else
    u8 face_id = 0xFF;
#endif
#endif
#endif

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
#if CFG_FMAP_AP_CTRL_TYPE >= EX_FM_UART_AP_CTRL_MAIN_DB
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF ) || ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF_USR )
    if ( g_bImpFmDataReady )
    {
        kdp_e2e_face_variables* vars = kdp_e2e_get_face_variables();
        kl520_api_face_close();
        kl520_api_face_set_add_mode(FACE_ADD_MODE_1_FACE);
        kdp_e2e_prop_set2(flow_mode, FLOW_MODE_SIM_PRE_ADD);
#if (CFG_AI_TYPE == AI_TYPE_N1R1)
        vars->rgb_led_flag = TRUE;
        vars->nir_led_flag = TRUE; //for sim, always on.

        if (vars->rgb_led_flag == FALSE && vars->rgb_led_lv_history_flag == FALSE)
            vars->rgb_led_flag = FALSE;
        else
            vars->rgb_led_flag = TRUE;
#endif
        dbg_msg_console("[%s] pre_add type = %d", vars->pre_add);
        vars->pre_add = AI_TYPE_PR1;

#if ( KDP_BIT_CTRL_MODE == YES )
        kl520_api_face_add(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, BIT_CTRL_FACE_ADD_NORMAL);
#else
        kl520_api_face_add(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, FACE_ADD_TYPE_NORMAL);
#endif

        ret = kl520_api_add_wait_and_get();
        kdp_e2e_prop_set2(flow_mode, FLOW_MODE_NORMAL);
        kl520_api_face_close();
        vars->pre_add = 0;

    }
    else
#endif
#endif
#endif
    if (1 == face_add_mode) {
        kl520_api_face_add_set_timeout(time_out_ms/1000);
        kl520_api_face_set_add_mode(FACE_ADD_MODE_1_FACE);
        //dbg_msg_console("[%s] FACE_ADD_TYPE_NORMAL", __func__);

#if ( KDP_BIT_CTRL_MODE == YES )
        ret = kl520_api_face_add_internal(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, BIT_CTRL_FACE_ADD_NORMAL);
#else
        ret = kl520_api_face_add_internal(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, FACE_ADD_TYPE_NORMAL);
#endif

    }
    else if (5 == face_add_mode) {
        kl520_api_face_add_set_timeout(time_out_ms/1000);
        kl520_api_dp_five_face_enable();
        kl520_api_face_set_add_mode(FACE_ADD_MODE_5_FACES);

        for (u8 i=0;i<5;i++) {
#if ( KDP_BIT_CTRL_MODE == YES )
            if ( i == 0 )
            {
                ret = kl520_api_face_add_internal(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, (kl520_bit_ctrl_face_add)kl520_api_face_idx_chg_2_bit_ctrl(i));
            }
            else if ( i < 3 )
            {
                ret = kl520_api_face_add_internal(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, BIT_CTRL_FACE_ADD_2FACE_LR);
            }
            else
            {
                ret = kl520_api_face_add_internal(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, BIT_CTRL_FACE_ADD_2FACE_UD);
            }
#else
            ret = kl520_api_face_add_internal(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, (kl520_face_add_type)i);
#endif

            if (ret != KL520_FACE_OK)
                break;
        }

        if (tid_abort_thread != 0)
            sample_force_abort_disable();
    }

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF ) || ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF_USR )
    if (KL520_FACE_OK == ret)
    {
        if (0xFF != kl520_api_face_get_curr_face_id()) {
            face_id = kl520_api_face_get_curr_face_id();
            kl520_api_face_set_curr_face_id(0xFF);
        }
        ret = ret + ((face_id&0xFF)<<8);
    }
    else if (KL520_FACE_NOFACE_AND_TIMEOUT == ret || KL520_FACE_TIMEOUT == ret) { ret = 0xFF03; }
    else  if (KL520_FACE_EXIST == ret)                                          { ret = 0xFF02; }
    else                                                                        { ret = 0xFF01; }
#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP ) || ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP_USR )
    if (KL520_FACE_OK == ret){
        if (0xFF != kl520_api_face_get_curr_face_id()) {
            nFaceId = (u8)kl520_api_face_get_curr_face_id();
            kl520_api_face_set_curr_face_id(0xFF);
        }
        ret = ret + ((nFaceId&0xFF)<<8);
        nResult = MR_SUCCESS;
        eFaceType = (u8)(eFaceType|KDP_FACE_DIRECTION_MASK);
    }
    else if (KL520_FACE_TIMEOUT == ret) { ret = 0xFF03; nResult = MR_FAILED_TIME_OUT; }
    else if (KL520_FACE_EXIST == ret)   { 
			ret = 0xFF02; 
		  nResult = MR_FAILED_FACE_ENROLLED; 
			nFaceId = (u8)kl520_api_face_get_curr_face_id(); //zcy add for return user id
	}
    else if (KL520_FACE_FULL == ret)    { ret = 0xFF01; nResult = MR_FAILED_MAX_USER; }
    else                                { ret = 0xFF01; nResult = MR_ABORTED; }

		 dbg_msg_console("[%s] nFaceId= %x", __func__, nFaceId);
    send_enroll_reply_msg(nResult, ((nFaceId >> 8) &0xFF),  ((nFaceId >> 0) &0xFF) , eFaceType, KID_ENROLL);
#endif
#endif
#endif

    sample_face_close();

    return ret;
}
extern uint16_t uart_sample_face_recognition_timeout(uint16_t time_out_ms)
{
    int ret = KL520_APP_FLAG_FDFR_ERR;
    u8 face_id = 0;
    u32 events = 0;
    u16 input = 0;
    system_info t_sys_info = { 0 };

    kl520_api_face_recognition_set_timeout(time_out_ms/1000);

    kl520_api_face_recognition(0, 0, DISPLAY_WIDTH, (input == 0)?DISPLAY_HEIGHT:input);

    do {
        events = wait_event(kl520_api_get_event(), KL520_APP_FLAG_FDFR | KL520_DEVICE_FLAG_ERR);
        if(events == KL520_DEVICE_FLAG_ERR)
        {
            ret = kl520_api_get_device_info(&t_sys_info);
            dbg_msg_err("[%s], DEVICE ERROR, ret=0x%x", __func__, ret);
            kl520_api_free_device_info(&t_sys_info);
        }
        else
        {
            ret = kl520_api_face_get_result(&face_id);
            if (KL520_FACE_OK == ret)
            {
                kdp_e2e_face_variables *vars_cur = kdp_e2e_get_face_variables();
    #ifdef DB_DRAWING_CUSTOMER_COLOR
                kl520_api_face_notify(KL520_FACE_DB_OK);
    #endif
                dbg_msg_console("sample_face_recognition, KL520_FACE_OK, face_id=0x%x, Admin=%d, UseName=%s", face_id, vars_cur->admin, vars_cur->user_name);
            }
            else if (KL520_FACE_DB_FAIL == ret)
            {
#ifdef DB_DRAWING_CUSTOMER_COLOR
                kl520_api_face_notify(KL520_FACE_DB_FAIL);
#endif
                dbg_msg_console("sample_face_recognition, KL520_FACE_DB_FAIL, face_id=0x%x", face_id);
            }
            else if (ret >= KL520_FACE_TOO_FAR && ret <= KL520_FACE_LOW_QUALITY )
            {
                dbg_msg_console("KL520_FACE NOTE, ret=0x%x", ret);
                continue;
            }
            else
            {
                dbg_msg_console("sample_face_recognition, ERROR, ret=0x%x", ret);
            }
        }
        break;
    }while (1);

    if (KL520_FACE_OK == ret) ret = ret + ((face_id&0xFF)<<8);
    else if (KL520_FACE_DB_FAIL == ret) ret = 0xFF01;
    else if (KL520_FACE_NOFACE_AND_TIMEOUT == ret || KL520_FACE_TIMEOUT == ret) ret = 0xFF03;
    else ret = 0xFF02;

#ifdef CFG_GUI_RECOG_CLOSE_TEST
#if (0==CFG_GUI_RECOG_CLOSE_TEST)
    sample_face_close();
#endif
#else
    sample_face_close();
#endif

    return ret;
}

extern uint16_t uart_sample_face_mp_timeout(uint16_t time_out_ms)
{
    int ret = 0;

    kl520_api_face_add_set_timeout(time_out_ms/1000);
    ret = kl520_engineering_calibration(0, NULL);

    return ret;
}

static u8 _sample_face_del_all_exist_users(void)
{
    int i = 0;
    u8 total_id_num = 0;
    u8 face_status[MAX_USER] = {0};

    kl520_api_face_query_all(&total_id_num, &face_status[0]);
    if (!total_id_num)
        return 1;

    for(i=0; i<total_id_num; i++) {
        u8 ret = kl520_api_face_del(2, face_status[i]);
        if (ret)
            return ret;
    }
    return 0;
}
__WEAK extern uint16_t uart_sample_face_del_all(void)
{
#if CFG_DEL_CALIBRATION_SETTING_WHEN_DEL_ALL == YES
    kl520_api_settings_delete();
#endif

    u8 ret = _sample_face_del_all_exist_users();
    if (0 != ret) {ret = 1;}

    return ret;
}
extern uint16_t uart_sample_face_del_user(uint8_t face_id)
{
    u8 ret = kl520_api_face_del(2, face_id);
    if (0 != ret) {ret = 1;}

    return ret;
}

void sample_user_comm_change_baud_rate(void)
{
#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
    kl520_com_reconfig_baud_rate(_cmd_get_answer_int("[set baud rate : (0:115200, 1:921600) >>"));
#endif
#endif
}

void sample_switch_e2e_board_params(void)
{
    s32 serial_no, sub_id;

    serial_no   = (s32)_cmd_get_answer_int("set serial no. of board params (0~1): ");
    sub_id      = (s32)_cmd_get_answer_int("set sub id of board params (0~1): ");
    kdp_e2e_face_switch_board_params(serial_no, sub_id);
}

void sample_factory(void)
{
    kl520_measure_stamp(E_MEASURE_FACTORY_STR);
    kl520_api_factory();
    kl520_measure_stamp(E_MEASURE_FACTORY_END);
}

void sample_poweroff(void)
{
    kl520_api_poweroff();
}

void sample_log_disable(void)
{
    kdp_ipc_init(FALSE);
    kl520_api_log_set_user_level(CPU_ID_SCPU, LOG_NONE);  //scpu
    kl520_api_log_set_user_level(CPU_ID_NCPU, LOG_NONE);  //ncpu
}

void sample_log_set_user_level(void)
{
    u8 cpu_id = _cmd_get_answer_int("[set cpu id : (0:scpu, 1:ncpu) >>");
    u32 level = _cmd_get_answer_int("[set user level : (0~9) >>");
    kl520_api_log_set_user_level(cpu_id, level);
}

void sample_set_dbid_offset()
{
    u8 _id_off = _cmd_get_answer_int("[set db offset : (0 ~ 80) >>");
    dbg_msg_console ("setting db offset to %d.", _id_off);
    update_user_db_offset(_id_off);
}

void sample_mp_calibration(void)
{
    /*
    BOOL algo_offset_calibration = _cmd_get_answer_int("[offset calibration : (0:Off, 1:On) >>");
    BOOL algo_distance_calibration = _cmd_get_answer_int("[distance calibration : (0:Off, 1:On) >>");
    BOOL camera_calibration = _cmd_get_answer_int("[camera calibration : (0:Off, 1:On) >>"); //color, 3a, ...
    BOOL display_calibration = _cmd_get_answer_int("[display calibration : (0:Off, 1:On) >>"); //x, y, width, height
    BOOL touch_calibration = _cmd_get_answer_int("[touch calibration : (0:Off, 1:On) >>"); //x, y, width, height
    if (algo_offset_calibration)
        kl520_engineering_calibration(0, NULL);
    if (algo_distance_calibration)
        kl520_engineering_calibration(1, NULL);
    if (camera_calibration)
        kl520_engineering_calibration(2, NULL);
    if (display_calibration)
        kl520_engineering_calibration(3, NULL);
    if (touch_calibration)
        kl520_engineering_calibration(4, NULL);
*/
    kl520_engineering_calibration(10, NULL);
}

#if CFG_PALM_PRINT_MODE == 1
void sample_switch_mode(void)
{
    u8 mode = _cmd_get_answer_int("set recognition mode: (0:face, 1:palm) >>");
    if(kdp_is_palm_mode() == mode) return;

    switch_palm_mode(mode);
    return;
}
#endif

void sample_app_console_quit(void)
{
    dbg_msg_console("bye bye !!");
}

void switch_scpu_part(void)
{
    ota_update_show_config();
    kl520_api_ota_switch_SCPU();
    ota_update_show_config();
}

void sample_user_rotate_180_enable(void)
{
    u8 enable = _cmd_get_answer_int("[set user rotate 180 enable: (0:disable, 1:enable) >>");

    kl520_customer_info Cusinfo;
    kl520_api_customer_get(&Cusinfo);

    if (enable)
        enable = ROTATE_180_ENABLE;
    else
        enable = ROTATE_180_DISABLE;

    if (Cusinfo.user_rotate_180 != enable)
    {
        dbg_msg_console("Save rotate setting to flash");
        Cusinfo.user_rotate_180 = enable;
        kl520_api_customer_write(&Cusinfo);
        power_mgr_sw_reset();
    }
}

#if CFG_CONSOLE_MODE == 2
void sample_get_image(void)
{
    u32 frame_addr = 0;
    int buf_idx;
    
    frame_addr = kdp_fb_mgr_next_inf(0, &buf_idx);
    
    if (0 == frame_addr){
        dbg_msg_console("kdp_fb_mgr_next_inf return null");
        return;
    }
    
    // Add user code here
    
    kdp_fb_mgr_inf_done(0, buf_idx);
}

struct console_cmd_info console_cmd_array[] = {
    {"sample_camera_open",                      sample_camera_open },
    {"sample_camera_start",                     sample_camera_start },
    {"sample_camera_stop",                      sample_camera_stop },
    {"sample_camera_close",                     sample_camera_close },
    
    {"sample_io_open_nir_led",                  sample_io_open_nir_led},
    {"sample_io_close_nir_led",                 sample_io_close_nir_led},
    
#if (CFG_SENSOR_TYPE == SENSOR_TYPE_GC1054_GC1054) || (CFG_SENSOR_TYPE == SENSOR_TYPE_GC02M1_GC1054) || (CFG_SENSOR_TYPE == SENSOR_TYPE_GC2145_GC1054)
    {"sample_GC1054_gain",                      sample_GC1054_gain},
    {"sample_GC1054_exp_time",                  sample_GC1054_exp_time},
#elif (CFG_SENSOR_TYPE == SENSOR_TYPE_OV02B1B_OV02B1B)
    {"sample_OV02B1B_gain",                     sample_OV02B1B_gain},
    {"sample_OV02B1B_exp_time",                 sample_OV02B1B_exp_time},
#else
    {"reserved",                                reserved},
    {"reserved",                                reserved},
#endif

    {"Quit",                                    reserved },
};

#elif CFG_CONSOLE_MODE == 1

struct console_cmd_info console_cmd_array[] = {

    {"sample_camera_open",                      sample_camera_open },
    {"sample_camera_start",                     sample_camera_start },
    {"sample_camera_stop",                      sample_camera_stop },
    {"sample_camera_close",                     sample_camera_close },

    {"sample_open_video_renderer",              sample_open_video_renderer},
    {"sample_close_video_renderer",             sample_close_video_renderer},
#if SAMPLE_LCD_BACKLIGHT == YES    
    {"sample_lcd_adjust_backlight",             sample_lcd_adjust_backlight},
    {"sample_lcd_adjust_backlight_step_by_step",sample_lcd_adjust_backlight_step_by_step},
#else
    {"reserved",                                reserved},
    {"reserved",                                reserved},
#endif
#if CFG_TOUCH_ENABLE == YES
    {"sample_touch_panel_open",                 sample_touch_panel_open},
    {"sample_touch_panel_close",                sample_touch_panel_close},
    {"sample_touch_app_enable",                 sample_touch_app_enable},
    {"sample_touch_app_disable",                sample_touch_app_disable},
#else
    {"reserved",                                reserved},
    {"reserved",                                reserved},
    {"reserved",                                reserved},
    {"reserved",                                reserved},
#endif

#if CFG_PALM_PRINT_MODE == 1
    {"switch_mode",                             sample_switch_mode},
#else
    {"reserved",                                reserved},
#endif
    
#if (SAMPLE_IO == YES)
    {"sample_io_set_nir_exp_time",              sample_io_set_nir_exp_time},
    {"sample_io_set_rgb_led",                   sample_io_set_rgb_led},//15
    {"sample_io_open_rgb_led",                  sample_io_open_rgb_led},
    {"sample_io_close_rgb_led",                 sample_io_close_rgb_led},
    {"sample_io_set_nir_led",                   sample_io_set_nir_led},
    {"sample_io_open_nir_led",                  sample_io_open_nir_led},
    {"sample_io_close_nir_led",                 sample_io_close_nir_led},
    {"sample_io_rgb_camera_power_on",           sample_io_rgb_camera_power_on},
    {"sample_io_rgb_camera_power_off",          sample_io_rgb_camera_power_off},
    {"sample_io_nir_camera_power_on",           sample_io_nir_camera_power_on},
    {"sample_io_nir_camera_power_off",          sample_io_nir_camera_power_off},
    {"sample_io_lcd_power_on",                  sample_io_lcd_power_on},
    {"sample_io_lcd_power_off",                 sample_io_lcd_power_off},
    {"sample_io_breathe_rgb_led",               sample_rgb_led_breathe},
#else    
    {"reserved",                                reserved},//15
    {"reserved",                                reserved},//15
    {"reserved",                                reserved},
    {"reserved",                                reserved},
    {"sample_io_open_nir_led",                  sample_io_open_nir_led},
    {"sample_io_close_nir_led",                 sample_io_close_nir_led},
#if (CFG_SENSOR_TYPE == SENSOR_TYPE_GC1054_GC1054) || (CFG_SENSOR_TYPE == SENSOR_TYPE_GC02M1_GC1054) || (CFG_SENSOR_TYPE == SENSOR_TYPE_GC2145_GC1054)
    {"sample_GC1054_gain",                      sample_GC1054_gain},
    {"sample_GC1054_exp_time",                  sample_GC1054_exp_time},
#elif (CFG_SENSOR_TYPE == SENSOR_TYPE_OV02B1B_OV02B1B)
    {"sample_OV02B1B_gain",                     sample_OV02B1B_gain},
    {"sample_OV02B1B_exp_time",                 sample_OV02B1B_exp_time},
#elif (CFG_SENSOR_TYPE == SENSOR_TYPE_SP2509_SP2509)
    {"sample_SP2509_gain",                     sample_SP2509_gain},
    {"sample_SP2509_exp_time",                 sample_SP2509_exp_time},
#else
    {"reserved",                                reserved},
    {"reserved",                                reserved},
#endif	
    {"reserved",                                reserved},
    {"reserved",                                reserved},
    {"reserved",                                reserved},
    {"reserved",                                reserved},
    {"reserved",                                reserved},
    {"reserved",                                reserved},
#endif

    {"reserved",                                reserved},
    
#if SAMPLE_STABILITY_ENABLE == YES
    {"sample_stability_add_user",               sample_stability_add_user},
    {"sample_stability_recognition_test",       sample_stability_recognition_test}, //30
    {"sample_stability_recognition",            sample_stability_recognition},
    {"sample_stability_all",                    sample_stability_all},
#else
    {"reserved",                                reserved},
    {"reserved",                                reserved},
    {"sample_stability_recognition",                                sample_stability_recognition},
    {"reserved",                                reserved},
#endif
    

    {"sample_face_add_set_timeout(s)",          sample_face_add_set_timeout},
    {"sample_face_recognition_set_timeout(s)",  sample_face_recognition_set_timeout},
    {"sample_face_add",                         sample_face_add},
    {"sample_face_recognition",                 sample_face_recognition},
    {"sample_face_recognition_test",            sample_face_recognition_test},
    {"sample_face_close",                       sample_face_close},

    {"sample_face_del_all",                     sample_face_del_all},
#if (SAMPLE_STABILITY_ENABLE == YES)
    {"sample_face_del_user",                    sample_face_del_user},
    {"sample_face_del_user_2",                  sample_face_del_user_2},
    {"sample_face_query_user",                  sample_face_query_user},
#else
    {"sample_face_del_user",                    sample_face_del_user},
    {"sample_fr_threshold",                     sample_fr_threshold},
    {"reserved",                                reserved},
#endif
    {"sample_face_query_allusers",              sample_face_query_all},
    
#if (SAMPLE_STABILITY_ENABLE == YES)
    {"sample_face_liveness",                    sample_face_liveness},
    {"sample_face_integrate",                   sample_face_integrate},
#else
    {"reserved",                                reserved},
    {"reserved",                                reserved},
#endif
    
    {"reserved",                                reserved},//sample_pwm_timer_test_init
    {"reserved",                                reserved},//sample_pwm_timer_test_close

#if SAMPLE_FLASH == YES
    {"sample_Flash_Info",                       sample_Flash_Info},
    {"sample_ssp_spi",                          sample_ssp_spi},
#else
    {"reserved",                                reserved},
    {"reserved",                                reserved},
#endif
    

    {"sample_get_system_info",                  sample_get_system_info},
    {"reserved",                                reserved},

#if SAMPLE_ADC == YES
    {"sample_get_adc",                          sample_get_adc},
#else
    {"reserved",                                reserved},
#endif
    
    {"sample_switch_rbg_nir",                   sample_switch_rbg_nir},//53
    {"reserved",                                reserved},//sample_sim_fdfr//54

#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
    {"sample_snapshot_auto_usb",                sample_snapshot_auto_usb},//55
#if CFG_SNAPSHOT_ENABLE == 2
    {"sample_snapshot",                         sample_snapshot}, //56
#else
    {"reserved",                                reserved},
#endif
#else
    {"reserved",                                reserved},
    {"reserved",                                reserved},
#endif

#if (MEASURE_RECOGNITION == YES)
    {"kl520_measure_info",                      kl520_measure_info},
#else
    {"reserved",                                reserved},
#endif

    {"sample_user_comm_change_baud_rate",       sample_user_comm_change_baud_rate},
    
#if (FB_TILE_RECODE == YES)
    {"sample_record_tile_val",                  sample_record_tile_val},
#else
    {"reserved",                                reserved},
#endif
    //face related extensions : 60
    {"sample_set_e2e_prop",                     reserved},
    //#if CFG_FMAP_EXTRA_ENABLE == YES
    #if 0
    {"sample_fetch_db_fmap",                    sample_fetch_db_fmap}, // jim
    {"sample_face_no_db_add_mode",              sample_face_no_db_add_mode}, //jim
    {"sample_extract_fmap_autousb",             sample_extract_fmap_autousb}, //jim
    #else
    {"reserved",                                reserved},
    {"reserved",                                reserved},
    {"reserved",                                reserved},
    #endif
    {"sample_switch_e2e_board_params",          sample_switch_e2e_board_params},

    //system related extensions : 65
    {"sample_power_manager_sw_reset",           sample_power_manager_sw_reset},
    {"sample_power_manager_shutdown",           sample_power_manager_shutdown},
    {"switch_scpu_part",                        switch_scpu_part},
    {"sample_factory",                          sample_factory},
    {"sample_poweroff",                         sample_poweroff},

    //ui related extensions : 70
    {"sample_ui_fill_rect ",                    sample_ui_fill_rect},
    {"reserved ",                               reserved},
#if (CFG_PANEL_TYPE==PANEL_ST7789_240X320_8080_AND_SPI)
    {"sample_switch_display_device",            sample_switch_display_device},
#else
    {"reserved",                                reserved},//sample_ui_draw_rect
#endif
    {"reserved",                                reserved},//sample_ui_draw_part
    {"reserved",                                reserved},//sample_ui_draw_full
    {"reserved",                                reserved},//sample_ui_draw_example
    
    {"user_console_cmd01",                      user_console_cmd01},
    {"user_console_cmd02",                      user_console_cmd02},
    {"user_console_cmd03",                      user_console_cmd03},
    {"user_console_cmd04",                      user_console_cmd04},

    //special scenario test : 80
#if SPECIAL_SCENARIO_TEST1 == YES
    {"special_scenario_test_1",                 special_scenario_test_1},
#else
    {"reserved",                                reserved},
#endif

#if SPECIAL_SCENARIO_TEST2 == YES
    {"special_scenario_test_2",                 special_scenario_test_2},
#else
    {"reserved",                                reserved},
#endif
#if (SAMPLE_DISPLAY_SNAPSHOT == YES) && (CFG_SNAPSHOT_ADVANCED == 1)
    {"sample_snapshot_record_img",              sample_snapshot_record_img},
    {"sample_snapshot_show_img",                sample_snapshot_show_img},
    {"sample_snapshot_del_img",                 sample_snapshot_del_img},
#else
    {"reserved",                                reserved},
    {"reserved",                                reserved},
    {"reserved",                                reserved},
#endif
    //debugging related extension : 85
    {"sample_log_disable",                      sample_log_disable},
    {"sample_log_set_user_level",               sample_log_set_user_level},
    {"sample_user_rotate_180_enable",           sample_user_rotate_180_enable},
    {"reserved",                                reserved},
#if (DB_OFFSET_CMD == YES)
    {"sample_db_offset",                        sample_set_dbid_offset},
#else
    {"reserved",                                reserved},
#endif

    //mass product related extensions : 90
    {"sample_mp_calibration",                   sample_mp_calibration},

    {"Quit",                                    reserved },
};
#endif

#if CFG_CONSOLE_MODE

#if (MEASURE_RECOGNITION == YES)
#if (MEASURE_WITH_TOUCH == YES)
    int pre_cmd_array[]={9, 36, 57, 0};
#else
#if (FB_TILE_RECODE == YES)
    int pre_cmd_array[]={36, 38, 57, 59, 0};
#else
    int pre_cmd_array[]={36, 38, 59, 0};
#endif
    //int pre_cmd_array[]={44, 57, 0};
#endif
#else
    int pre_cmd_array[]={0};
#endif

void doorlock_console_thread(void *arg)
{
    BOOL show_item = TRUE;
    u8 pre_cmd_idx = 0;
    unsigned int i = 0;
    unsigned int cmd_size = ARRAY_SIZE(console_cmd_array);
    char buf[256];
    console_cmd_func cmd_func;
    uint32_t rc = 1;

#if (KL520_QUICK_BOOT == YES)
    //kl520_api_tasks_init_wait_ready();
#endif

#if ( !(CFG_COM_BUS_TYPE&COM_BUS_UART_MASK) )
    kl520_api_fdfr_init_thrd();
#endif    
    
    kl520_measure_stamp(E_MEASURE_THR_CONSOLE_RDY);
    
#if ((CFG_COM_BUS_TYPE&COM_BUS_UART_MASK) || (CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK)) && (SAMPLE_ADV_SNAPSHOT == YES)
    sample_snapshot_auto_usb_mode(4);
#else
    while(1)
    {
        int id = 0;

        if (show_item) {
            if (rc)
                goto cmd_prompt;

            dbg_msg_console(" === kl520 sample app console Test Kit (%u) === ", cmd_size);

            for (i = 0; i < cmd_size; ++i)
            {
                if(strncmp("reserved", console_cmd_array[i].desc, strlen("reserved")) == 0) 
                    continue;

                sprintf(buf, "(%2d) %s : ", i + 1, console_cmd_array[i].desc);
                dbg_msg_console("%s", buf);
            }
            dbg_msg_console("Thread NUM:%d",osThreadGetCount());
cmd_prompt:
            if(pre_cmd_array[pre_cmd_idx] == 0){
                dbg_msg_console(" command >>");
                rc = kdp_gets(DEBUG_CONSOLE, buf);
                dbg_msg_nocrlf("");
                if (!rc)
                    continue;
                id = atoi(strtok(buf, " \r\n\t"));
            }
            else{
                id = pre_cmd_array[pre_cmd_idx];
                pre_cmd_idx++;
            }

            if (id > 0 && id < cmd_size)
            {
#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
                kl520_api_sim_set_usercmd(id);
#endif

                cmd_func = console_cmd_array[id - 1].func;
                if (!cmd_func) continue;
                if ((cmd_func == sample_stability_add_user) ||
                    (cmd_func == sample_stability_recognition) ||
                    (cmd_func == sample_stability_all)) {
                    show_item = FALSE;
                }
                cmd_func();
            }
            else if (cmd_size == id) {
                cmd_func();
                break;
            }
            else
                continue;
        }
        else
            osDelay(10000);
    }
#endif
}

void sample_doorlock_entry(void)
{
    osThreadAttr_t attr = {
        .stack_size = 2048
    };

    tid_doorlock_console = osThreadNew(doorlock_console_thread, NULL, &attr);
    

    
    
}

#endif

