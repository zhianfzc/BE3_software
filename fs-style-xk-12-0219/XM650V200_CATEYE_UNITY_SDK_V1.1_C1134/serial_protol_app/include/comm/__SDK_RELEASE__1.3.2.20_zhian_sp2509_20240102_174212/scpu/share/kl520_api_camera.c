/*
 * @name : kl520_api_camera.c
 * @brief : camera interface
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#include "board_kl520.h"
#include <stdlib.h>
#include "framework/framework.h"
#include "framework/framework_errno.h"
#include "framework/v2k_ioctl.h"
#include "framework/kdp_dev.h"
#include "framework/mutex.h"
#include "framework/event.h"
#include "framework/framework_driver.h"
//driver include file. need a much better refactor
#include "pinmux.h"
#include "system.h"
#include "framework/v2k_image.h"
#include "media/display/video_renderer.h"
#include "media/display/display.h"
#include "media/display/lcdc.h"
#include "media/display/lcm.h"
#include "kdp_uart.h"
#include "kdp520_pwm_timer.h"
#include "kdp_ddr_table.h"
#include "usr_ddr_img_table.h"
#include "kl520_include.h"
#include "kl520_api.h"
#include "kdp520_i2c.h"
#include "kdp520_gpio.h"
#include "kdp_camera.h"
#include "kdp_fb_mgr.h"
#include "kdp_e2e_face.h"
#include "kl520_api_fdfr.h"
#include "kl520_api_device_id.h"
#include "kl520_api_snapshot.h"
#include "kl520_api_sim.h"
#include "kl520_api_camera.h"
#include "kdp_model.h"

#ifdef MODIFY_CAM_DISP_CT_FLOW
#include "sample_app_console.h"

#if (CFG_GUI_ENABLE == 1)
#include "sample_gui_fsm_events.h"
#include "sample_gui_main.h"
#endif

#endif

#if ( CFG_PANEL_TYPE != PANEL_NULL ) || ( ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK ) && ( CFG_USB_EXPORT_STREAM_IMG == YES ) )
#define KDP_CAM_DISPLAY 1
#endif

#define FLAGS_APP_CAMERA_OPEN_EVT               0x0001
#define FLAGS_APP_CAMERA_START_EVT              0x0002
#define FLAGS_APP_CAMERA_STOP_EVT               0x0004
#define FLAGS_APP_CAMERA_CLOSE_EVT              0x0008

#define FLAGS_CAMERA_ACK                        0x00040000
#define FLAGS_CAMERA_BUSY                       0x00080000

struct camera_context {
#if KDP_CAM_DISPLAY == 1
    int handle;
    //unsigned int idx;
    unsigned int width;
    unsigned int height;
    unsigned int pixelformat;
    unsigned int old_addr;
    unsigned int curr_addr;
    struct v2k_buffer api_buf;
#endif
    enum kdp_device_status_code_e state;
};


static struct camera_context m_camera_ctx[IMGSRC_NUM];
static fn_power_hook m_cb_camera0_power_on = NULL;
static fn_power_hook m_cb_camera0_power_off = NULL;
static fn_power_hook m_cb_camera1_power_on = NULL;
static fn_power_hook m_cb_camera1_power_off = NULL;

#ifdef MODIFY_CAM_DISP_CT_FLOW

#define DISP_IDX        (IMGSRC_NUM)
#define GUI_IDX         (DISP_IDX+1)
#define PERMANENT_NUM   (GUI_IDX+1)
static BOOL g_aPermanentState[PERMANENT_NUM] = {FALSE};

#if ( CFG_GUI_ENABLE == YES )
enum CTRL_STATE g_eCtrlFrom = CTRL_COMM;
enum CTRL_STATE g_ePreCtrlFrom = CTRL_COMM;
#endif

#if KDP_CAM_DISPLAY == 1
static osThreadId_t        m_cam_to_dp_tid = NULL;
static osEventFlagsId_t    m_cam_to_dp_evt_ack = NULL;
static enum DEV_STATE g_eSofDispState = DEV_STATE_NULL;
static BOOL g_bShowCamImgToDp = TRUE;
static BOOL g_bBootupGuiHighPriority = FALSE;
static enum CAM_THREAD_STATE g_eCamThreadState = CAM_THREAD_STATE_NULL;
#endif
#endif

int kdp_api_camera_on_0(void)
{
   SCU_EXTREG_CLK_EN1_SET_csirx0_TxEscClk(1);
   SCU_EXTREG_CLK_EN1_SET_csirx0_vc0(1);
   SCU_EXTREG_CLK_EN1_SET_csirx0_csi(1);

   masked_outw( SCU_EXTREG_CSIRX_CTRL0,
               ((SCU_EXTREG_CSIRX_CTRL0_ClkLnEn | SCU_EXTREG_CSIRX_CTRL0_Enable) |
                (SCU_EXTREG_CSIRX_CTRL0_apb_rst_n | SCU_EXTREG_CSIRX_CTRL0_pwr_rst_n |
                 SCU_EXTREG_CSIRX_CTRL0_sys_rst_n)),
               ((SCU_EXTREG_CSIRX_CTRL0_ClkLnEn | SCU_EXTREG_CSIRX_CTRL0_Enable) |
                (SCU_EXTREG_CSIRX_CTRL0_apb_rst_n | SCU_EXTREG_CSIRX_CTRL0_pwr_rst_n |
                 SCU_EXTREG_CSIRX_CTRL0_sys_rst_n)));

   masked_outw( SCU_EXTREG_DPI2AHB_CTRL,
               (SCU_EXTREG_DPI2AHB_CTRL_rst_n | SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n |
                SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n | SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n),
               (SCU_EXTREG_DPI2AHB_CTRL_rst_n | SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n |
                SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n | SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n));

    return 0;
}

int kdp_api_camera_on_1(void)
{
   SCU_EXTREG_CLK_EN1_SET_csirx1_TxEscClk(1);
   SCU_EXTREG_CLK_EN1_SET_csirx1_vc0(1);
   SCU_EXTREG_CLK_EN1_SET_csirx1_csi(1);

   masked_outw( SCU_EXTREG_CSIRX_CTRL1,
               ((SCU_EXTREG_CSIRX_CTRL1_ClkLnEn | SCU_EXTREG_CSIRX_CTRL1_Enable) |
                (SCU_EXTREG_CSIRX_CTRL1_apb_rst_n | SCU_EXTREG_CSIRX_CTRL1_pwr_rst_n |
                 SCU_EXTREG_CSIRX_CTRL1_sys_rst_n)),
               ((SCU_EXTREG_CSIRX_CTRL1_ClkLnEn | SCU_EXTREG_CSIRX_CTRL1_Enable) |
                (SCU_EXTREG_CSIRX_CTRL1_apb_rst_n | SCU_EXTREG_CSIRX_CTRL1_pwr_rst_n |
                 SCU_EXTREG_CSIRX_CTRL1_sys_rst_n)));

   masked_outw( SCU_EXTREG_DPI2AHB_CTRL,
               (SCU_EXTREG_DPI2AHB_CTRL_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n_1 |
                SCU_EXTREG_DPI2AHB_CTRL_sys_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n_1),
               (SCU_EXTREG_DPI2AHB_CTRL_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n_1 |
                SCU_EXTREG_DPI2AHB_CTRL_sys_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n_1));

    return 0;
}

#if ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36515 ) || ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36404 )
void kl520_api_led_register_hook( u32 cam_idx, fn_led_open fn_open, fn_led_close fn_close, fn_led_light_mode fn_mode )
{
    kdp_e2e_ctrl_led_register( cam_idx, fn_open, fn_close, fn_mode );
}
#else
void kl520_api_led_register_hook(u32 cam_idx, fn_led_open fn_open, fn_led_close fn_close)
{
    kdp_e2e_ctrl_led_register(cam_idx, fn_open, fn_close);
}
#endif

void kl520_api_light_sensor_register_hook(fn_strength_get fn_get)
{
    kdp_e2e_ctrl_light_sensor_register(fn_get);
}

void kl520_api_camera_register_hook(u32 cam_idx, fn_power_hook fn_power_on, fn_power_hook fn_power_off)
{
    if (0 == cam_idx) {
        m_cb_camera0_power_on = fn_power_on;
        m_cb_camera0_power_off = fn_power_off;
    #if (KL520_QUICK_BOOT == YES)
        if (m_cb_camera0_power_on)
            m_cb_camera0_power_on();
    #endif
    }
    else if (1 == cam_idx) {
        m_cb_camera1_power_on = fn_power_on;
        m_cb_camera1_power_off = fn_power_off;
    #if (KL520_QUICK_BOOT == YES)
        if (m_cb_camera1_power_on)
            m_cb_camera1_power_on();
    #endif
    }
}


int kl520_api_camera_get_id(unsigned int cam_idx)
{
    int ret = 0;

    ret = kdp_camera_get_device_id(cam_idx);
    if(ret == DEVICE_NOT_INIT)
    {
        kl520_api_cam_disp_ctrl(API_CTRL_CAM_OPEN, cam_idx, PERMANENT_NULL);
        ret = kdp_camera_get_device_id(cam_idx);
        kl520_api_cam_disp_ctrl(API_CTRL_CAM_CLOS, cam_idx, PERMANENT_NULL);
    }

    return ret;
}

#if KDP_CAM_DISPLAY == 1
extern int kl520_fdfr_drawing_timer_flag;
static void kl520_api_draw_status_box(int flag)
{
    struct video_input_params params;

    if(kdp_video_renderer_get_params(&params))
        return;
    u32 org_x = 0;
    u32 org_y = 0;

    u32 end_x = org_x + params.dp_out_w;
    u32 end_y = org_y + params.dp_out_h;

    u16 color;
    u32 pen_width;
    u32 line_len;
    u32 margin;

    if((KL520_FACE_OK == flag) && (FACE_MODE_ADD == m_face_mode))
    {
#if ( KDP_BIT_CTRL_MODE == YES )
        if(kl520_api_dp_five_face_get() == TRUE)// 5 faces add
        {
            if ( ( g_nFaceRegSts & BIT_CTRL_FACE_ADD_FACE_MASK ) != BIT_CTRL_FACE_ADD_FACE_MASK )
            {
                return;
            }
        }
        else // 1 face add
        {
            if ( !( g_nFaceRegSts & BIT_CTRL_FACE_ADD_NORMAL ) )
            {
                return;
            }
        }
#else
        if(kl520_api_dp_five_face_get() == TRUE)// 5 faces add
        {
            if(!((face_reg_sts & (0x01 << FACE_ADD_TYPE_NORMAL)) &&
            (face_reg_sts & (0x01 << FACE_ADD_TYPE_LEFT)) &&
            (face_reg_sts & (0x01 << FACE_ADD_TYPE_UP)) &&
            (face_reg_sts & (0x01 << FACE_ADD_TYPE_RIGHT)) &&
            (face_reg_sts & (0x01 << FACE_ADD_TYPE_DOWN))))
                return;
        }
        else // 1 face add
        {
            if(!(face_reg_sts & (0x01 << FACE_ADD_TYPE_NORMAL)))
                return;
        }
#endif
    }

    switch (flag)
    {
        case KL520_FACE_OK:
            color       =   LCD_DISPLAY_FD_BOX_COLOR;
            pen_width   =   LCD_DISPLAY_FD_BOX_PEN_WIDTH;
            line_len    =   LCD_DISPLAY_FD_BOX_LEN;
            margin      =   LCD_DISPLAY_FD_BOX_MARGIN;
            break;
        case KL520_FACE_DB_OK:
            color       =   LCD_DISPLAY_DB_BOX_OK_COLOR;
            pen_width   =   LCD_DISPLAY_DB_BOX_PEN_WIDTH;
            line_len    =   LCD_DISPLAY_FD_BOX_LEN;
            margin      =   LCD_DISPLAY_FD_BOX_MARGIN;
            break;
        case KL520_FACE_DB_FAIL:
            color       =   LCD_DISPLAY_DB_BOX_FAIL_COLOR;
            pen_width   =   LCD_DISPLAY_DB_BOX_PEN_WIDTH;
            line_len    =   LCD_DISPLAY_DB_BOX_LEN;
            margin      =   LCD_DISPLAY_DB_BOX_MARGIN;
            break;
        case KL520_FACE_EXIST:
            color       =   LCD_DISPLAY_FD_EXIST_COLOR;
            pen_width   =   LCD_DISPLAY_FD_BOX_PEN_WIDTH;
            line_len    =   LCD_DISPLAY_FD_BOX_LEN;
            margin      =   LCD_DISPLAY_FD_BOX_MARGIN;
            break;
        case KL520_FACE_LIVENESS_OK:
            color       =   LCD_DISPLAY_FD_BOX_LV_OK_COLOR;
            pen_width   =   LCD_DISPLAY_FD_BOX_PEN_WIDTH;
            line_len    =   LCD_DISPLAY_FD_BOX_LEN;
            margin      =   LCD_DISPLAY_FD_BOX_MARGIN;
            break;

        default:    // KL520_FACE_FAIL and other
            color       =   LCD_DISPLAY_HINT_BOX_COLOR;
            pen_width   =   LCD_DISPLAY_HINT_BOX_PEN_WIDTH;
            line_len    =   LCD_DISPLAY_HINT_BOX_LEN;
            margin      =   LCD_DISPLAY_HINT_BOX_MARGIN;
            break;
    }

    kl520_api_dp_set_pen_rgb565(color, pen_width);
    /*Left Upper*/
    kdp_display_draw_line(org_x+margin, org_y+margin, org_x+margin+line_len, org_y + margin);
    kdp_display_draw_line(org_x+margin, org_y+margin, org_x+margin , org_y+margin+line_len);
    /*Right Upper*/
    kdp_display_draw_line(end_x-margin-line_len, org_y+margin, end_x-margin, org_y+margin);
    kdp_display_draw_line(end_x-margin, org_y+margin, end_x-margin, org_y+margin+line_len);
//    /*Left Bottom*/
    kdp_display_draw_line(org_x+margin, end_y-margin, org_x+margin+line_len, end_y-margin);
    kdp_display_draw_line(org_x+margin, end_y-margin-line_len, org_x+margin, end_y-margin);
//    /*Right Bottom*/
    kdp_display_draw_line(end_x-margin-line_len, end_y-margin, end_x-margin, end_y-margin);
    kdp_display_draw_line(end_x-margin, end_y-margin-line_len, end_x-margin, end_y-margin);
}

//u16 kl520_api_dp_layout_fd_hint(void)
//{
//    return kdp_e2e_util_get_person_to_camera_distance();
//}

static void _chk_fps(void)
{
    static u32 dtime = 0;
    static float dfps = 0;
    if(osKernelGetTickCount() - dtime > 10000){
        float sfps = (dfps/10);

        if(sfps != 0){
            dbg_msg_api("disaply-fps:%5.2f",sfps );
        }
        dfps = 0;
        dtime = osKernelGetTickCount();
    }
    else{
        dfps++;
    }
}

static void _kl520_api_dp_draw_cam(void)
{
    int buf_idx;
    u32 buf_addr;
    int i = kdp_video_renderer_get_idx();

    if(i != -1)
    {
#if CFG_SNAPSHOT_ENABLE == 2
        if(kl520_api_sim_is_running() == TRUE)
        {
            buf_addr = kl520_api_snapshot_ddr_addr(i);
        }
        else
#endif
        {
            buf_addr = kdp_fb_mgr_next_read(i, &buf_idx);
        }

        if ( g_bShowCamImgToDp&(!g_bBootupGuiHighPriority) )
        {
            kdp_display_set_source(buf_addr, i);
#if CFG_SNAPSHOT_ENABLE == 2
            kl520_api_snapshot_shot(kdp_display_get_buffer_addr());
#endif
        }


#if CFG_USB_EXPORT_STREAM_IMG == YES
        {
            s8 export_image_ctrl = kl520_api_export_stream_get_image_crtl();
            eSTREAM_IMAGE_EXPORT_SRC export_rx_crtl = kl520_api_export_stream_get_image_export_crtl();
            if( export_rx_crtl != STRAM_IMAGE_DISPALY_e && export_rx_crtl < STRAM_IMAGE_TOTAL_SIZE_e )
            {
                if( export_image_ctrl == MIPI_CAM_NIR || export_image_ctrl == IMGSRC_NUM )
                    kl520_api_export_stream_image(MIPI_CAM_NIR, kdp_fb_mgr_next_read(MIPI_CAM_NIR, &buf_idx), export_rx_crtl );
                if( export_image_ctrl == MIPI_CAM_RGB || export_image_ctrl == IMGSRC_NUM )
                    kl520_api_export_stream_image(MIPI_CAM_RGB, kdp_fb_mgr_next_read(MIPI_CAM_RGB, &buf_idx), export_rx_crtl );        
                kl520_api_export_stream_image_ready();
            }

        }
#endif //CFG_USB_EXPORT_STREAM_IMG == YES


        kdp_fb_mgr_free_read_buf(i);
     }
}

extern void _kl520_api_dp_layout_lm(kl520_dp_draw_info *info, int flag);
static void _kl520_api_dp_fdfr_drawing(void)
{
    kl520_api_draw_status_box(kl520_fdfr_drawing_timer_flag);

#if (KL520_API_SHOW_BOUNDINGBOX == YES)
    _kl520_api_dp_layout_lm(&dp_draw_info, kl520_fdfr_drawing_timer_flag);
#endif
}
#endif

#if KDP_CAM_DISPLAY == 1
static void camera_to_display_thread(void *argument)
{
    unsigned int flags;
    BOOL quit = FALSE;
#if KDP_CAM_DISPLAY == 1
    BOOL init = FALSE;
#endif	
    u8 nDelayTime = 10;

    kl520_measure_stamp(E_MEASURE_THR_CAM_RDY);

    while (!quit) {
        flags = osThreadFlagsGet();
        //dbg_msg_api("flags = %x", flags);

        if (flags & FLAGS_APP_CAMERA_STOP_EVT)
        {
            osThreadFlagsClear( FLAGS_APP_CAMERA_OPEN_EVT |
                                FLAGS_APP_CAMERA_START_EVT);
            flags = FLAGS_APP_CAMERA_STOP_EVT;
        }
        else if (flags & FLAGS_APP_CAMERA_CLOSE_EVT)
        {
            osThreadFlagsClear( FLAGS_APP_CAMERA_OPEN_EVT |
                                FLAGS_APP_CAMERA_START_EVT |
                                FLAGS_APP_CAMERA_STOP_EVT);
            flags = FLAGS_APP_CAMERA_CLOSE_EVT;
        }

        switch (flags & 0x0F)
        {
            case FLAGS_APP_CAMERA_OPEN_EVT:
            {
                osThreadFlagsClear(FLAGS_APP_CAMERA_OPEN_EVT);
                osDelay(KL520_CAMERA_SKIP_MS);
#if KDP_CAM_DISPLAY == 1
                init = FALSE;
#endif
                g_eCamThreadState = CAM_THREAD_STATE_OPEN;
                break;
            }

            case FLAGS_APP_CAMERA_START_EVT:
            {
                if((NULL != m_cam_to_dp_tid) && (DEV_INIT_STATE_INITED == kl520_api_dp_init_get()))
                {
#if KDP_CAM_DISPLAY == 1
                    _kl520_api_dp_draw_cam();
#endif
#if KDP_CAM_DISPLAY == 1
                    if ( g_bShowCamImgToDp&(!g_bBootupGuiHighPriority) )
                    {
                        if(kl520_api_dp_layout_get())
                        {
                            _kl520_api_dp_fdfr_drawing();
                        }

                        if (kl520_api_ui_fsm_dp_layout_get())
                        {
#if (CFG_GUI_ENABLE == YES)
#ifdef DISPLAY_REGISTER_WITH_CUSTOM_IMG
                            kl520_api_dp_layout_pose_with_customized_size_img();
#else
                            kl520_api_dp_layout_pose();
#endif
#endif
                        }

                        {
                            kl520_api_dp_fresh();
                            if(init == FALSE){
                                kl520_api_dp_set_backlight(kl520_api_dp_get_backlight());
                                kl520_measure_stamp(E_MEASURE_BACKLIGHT_ON);
                                init = TRUE;
                            }
                        }
                    }
#endif

#if (CALC_DISPLAY_FPS == YES)
                    _chk_fps();
#endif

                    g_eCamThreadState = CAM_THREAD_STATE_START;
                }
                else
                {
                    if (DEV_INIT_STATE_ERROR == kl520_api_dp_init_get())
                        osThreadFlagsClear(FLAGS_APP_CAMERA_START_EVT);
                }
                break;
            }

            case FLAGS_APP_CAMERA_STOP_EVT:
            {
                osThreadFlagsClear(FLAGS_APP_CAMERA_STOP_EVT);
#if KDP_CAM_DISPLAY == 1
                init = FALSE;
#endif
                set_event(m_cam_to_dp_evt_ack, FLAGS_CAMERA_ACK);
                g_eCamThreadState = CAM_THREAD_STATE_STOP;
                break;
            }

            case FLAGS_APP_CAMERA_CLOSE_EVT:
            {
                osThreadFlagsClear(FLAGS_APP_CAMERA_CLOSE_EVT);
                quit = TRUE;
#if KDP_CAM_DISPLAY == 1
                init = FALSE;
#endif
                nDelayTime = 0;
                set_event(m_cam_to_dp_evt_ack, FLAGS_CAMERA_ACK);
                g_eCamThreadState = CAM_THREAD_STATE_NULL;
                break;
            }

            default:
                break;
        }

        osDelay(nDelayTime);//100 //performance tuning
    }
    //dbg_msg_api("[%s] quit", __func__);
    osThreadExit();
}
#endif

enum kdp_device_status_code_e kl520_api_cam_state_get(unsigned int nCamIdx)
{
    if ( nCamIdx >= IMGSRC_NUM )
    {
        dbg_msg_api("camera index invalid");
        return KDP_DEVICE_STATUS_ERROR;
    }

    return m_camera_ctx[nCamIdx].state;
}

#ifdef MODIFY_CAM_DISP_CT_FLOW

#if KDP_CAM_DISPLAY == 1
BOOL g_bDpResChg = FALSE;
u32 g_nDpResX = DISPLAY_WIDTH;
u32 g_nDpResY = DISPLAY_HEIGHT;
#endif

void kl520_api_hmi_ctrl_state_reset(enum CTRL_STATE ePermSt)  //Human Machine Interface
{
#if ( CFG_GUI_ENABLE == YES )
    g_ePreCtrlFrom = ePermSt;
    g_eCtrlFrom = ePermSt;
#endif
}

void kl520_api_hmi_ctrl_state_set(enum CTRL_STATE ePermSt)  //Human Machine Interface
{
#if ( CFG_GUI_ENABLE == YES )
    g_ePreCtrlFrom = g_eCtrlFrom;
    g_eCtrlFrom = ePermSt;
#endif
}

static int _api_camera_open(u8 nIdx)
{
    int ret = 0;
    unsigned int width;
    unsigned int height;
    unsigned int pixelformat;
    struct cam_capability cap;
    struct cam_format fmt;

    char fmtstr[8];

    if ( nIdx == 0 )
    {
        if ( m_cb_camera0_power_on )
        {
            m_cb_camera0_power_on();
        }

        kdp_api_camera_on_0();

#if IMGSRC_0_RES == RES_640_480
        width = VGA_LANDSCAPE_WIDTH;
        height = VGA_LANDSCAPE_HEIGHT;
#elif IMGSRC_0_RES == RES_480_640
        width = VGA_PORTRAIT_WIDTH;
        height = VGA_PORTRAIT_HEIGHT;
#elif IMGSRC_0_RES == RES_480_272
        width = TFT43_WIDTH;
        height = TFT43_HEIGHT;
#elif IMGSRC_0_RES == RES_864_491
        width = HMX_RICA_WIDTH;
        height = HMX_RICA_HEIGHT;
#elif IMGSRC_0_RES == RES_1600_1200
        width = UGA_WIDTH;
        height = UGA_HEIGHT;
#elif IMGSRC_0_RES == RES_1080_1280
        width = SC132_FULL_RES_WIDTH;
        height = SC132_FULL_RES_HEIGHT;
#else
        width = CFG_SENSOR_0_WIDTH;
        height = CFG_SENSOR_0_HEIGHT;
#endif

#if IMGSRC_0_FORMAT == IMAGE_FORMAT_RGB565
        pixelformat = V2K_PIX_FMT_RGB565;
#elif IMGSRC_0_FORMAT == IMAGE_FORMAT_RAW10
        pixelformat = V2K_PIX_FMT_RAW10;
#elif IMGSRC_0_FORMAT == IMAGE_FORMAT_RAW8
        pixelformat = V2K_PIX_FMT_RAW8;
#endif
    }
    else if ( nIdx == 1 )
    {
        if (m_cb_camera1_power_on)
        {
            m_cb_camera1_power_on();
        }

        kdp_api_camera_on_1();

#if IMGSRC_1_RES == RES_640_480
        width = VGA_LANDSCAPE_WIDTH;
        height = VGA_LANDSCAPE_HEIGHT;
#elif IMGSRC_1_RES == RES_480_640
        width = VGA_PORTRAIT_WIDTH;
        height = VGA_PORTRAIT_HEIGHT;
#elif IMGSRC_1_RES == RES_480_272
        width = TFT43_WIDTH;
        height = TFT43_HEIGHT;
#elif IMGSRC_1_RES == RES_864_491
        width = HMX_RICA_WIDTH;
        height = HMX_RICA_HEIGHT;
#elif IMGSRC_1_RES == RES_1600_1200
        width = UGA_WIDTH;
        height = UGA_HEIGHT;
#elif IMGSRC_1_RES == RES_1080_1280
        width = SC132_FULL_RES_WIDTH;
        height = SC132_FULL_RES_HEIGHT;
#else
        width = CFG_SENSOR_1_WIDTH;
        height = CFG_SENSOR_1_HEIGHT;
#endif

#if IMGSRC_1_FORMAT == IMAGE_FORMAT_RGB565
        pixelformat = V2K_PIX_FMT_RGB565;
#elif IMGSRC_1_FORMAT == IMAGE_FORMAT_RAW10
        pixelformat = V2K_PIX_FMT_RAW10;
#elif IMGSRC_1_FORMAT == IMAGE_FORMAT_RAW8
        pixelformat = V2K_PIX_FMT_RAW8;
#endif
    }

    if ( kdp_camera_open(nIdx) != 0 )
        ret = (ret|BIT1);//return -1;

    memset(&cap, 0, sizeof(cap));
    if ( kdp_camera_get_device_info(nIdx, &cap) != 0 )
        ret = (ret|BIT2);//return -1;

    dbg_msg_api("===== Capability Informations =====");
    dbg_msg_api("   driver: %s", cap.driver);
    dbg_msg_api("   desc: %s", cap.desc);
    dbg_msg_api("   version: %08X", cap.version);

    memset(&fmt, 0, sizeof(fmt));
    fmt.width = width;
    fmt.height = height;
    fmt.pixelformat = pixelformat;

    if ( kdp_camera_set_frame_format(nIdx, (struct cam_format *)&fmt) )
        ret = (ret|BIT3);//return -1;


#if (CFG_CAMERA_DUAL_1054 != 1)
    if ( nIdx == CAMERA_DEVICE_NIR_IDX )
#endif
    {
        kdp_e2e_face_variables *vars = kdp_e2e_get_face_variables();

        u8 gain_h, gain_l;
        u16 level = (u16)(vars->nir_gain);
        vars->nir_gain = (float)level;
        gain_h = (level >> 8) & 0x0F;
        gain_l = level & 0xFF;
        
        kdp_camera_set_gain(nIdx, gain_h, gain_l);

        kdp_camera_set_exp_time(nIdx, (vars->nir_cur_exp_time&0x0000FF00)>>8, vars->nir_cur_exp_time&0xFF);
        
        kdp_camera_set_fps(nIdx, vars->nir_fps);
    }
#if (CFG_SENSOR_0_TYPE == SENSOR_TYPE_GC02M1_R) || \
    (CFG_SENSOR_0_TYPE == SENSOR_TYPE_GC02M1_L) || \
    (CFG_SENSOR_1_TYPE == SENSOR_TYPE_GC02M1_R) || \
    (CFG_SENSOR_1_TYPE == SENSOR_TYPE_GC02M1_L)
    if ( nIdx == CAMERA_DEVICE_RGB_IDX )
    {
        kdp_e2e_face_variables *vars = kdp_e2e_get_face_variables();

        u8 gain_h, gain_l;
        u16 level = 600;
        gain_h = (level >> 8) & 0x0F;
        gain_l = level & 0xFF;

        u8 exp_time_h, exp_time_l;
        u16 exp_time = 623;
        exp_time_h = (exp_time >> 8) & 0x0F;
        exp_time_l = exp_time & 0xFF;
        
        kdp_camera_set_exp_time(nIdx, exp_time_h, exp_time_l);
        kdp_camera_set_gain(nIdx, gain_h, gain_l);

    }
#else
#if ( CFG_PALM_PRINT_MODE == YES )
    if ( kdp_is_palm_mode() && nIdx == CAMERA_DEVICE_RGB_IDX )
    {
        kdp_e2e_face_variables *vars = kdp_e2e_get_face_variables();
        u8 gain_h, gain_l;
        u16 level = PALM_DEFAULT_GAIN;
        
        vars->nir_cur_exp_time = PALM_DEFAULT_EXP_TIME;
        vars->nir_gain = (float)level;
        gain_h = (level >> 8) & 0x0F;
        gain_l = level & 0xFF;
        
        kdp_e2e_bg_init_done( FALSE );
        kdp_camera_set_gain(nIdx, gain_h, gain_l);
        kdp_camera_set_exp_time(nIdx, (vars->nir_cur_exp_time&0x0000FF00)>>8, vars->nir_cur_exp_time&0xFF);
        kdp_camera_set_fps(nIdx, vars->nir_fps);
        kdp_e2e_bg_init_done( FALSE );
    }
#endif
#endif

    if ( kdp_camera_get_frame_format(nIdx, &fmt) != 0 )
        ret = (ret|BIT4);//return -1;

    dbg_msg_api("===== Stream Format Informations =====");
    dbg_msg_api("   width: %d", fmt.width);
    dbg_msg_api("   height: %d", fmt.height);

    memset(fmtstr, 0, 8);
    memcpy(fmtstr, &fmt.pixelformat, 4);
    dbg_msg_api("   pixelformat: %s", fmtstr);
    dbg_msg_api("   field: %d", fmt.field);
    dbg_msg_api("   bytesperline: %d", fmt.bytesperline);
    dbg_msg_api("   sizeimage: %d", fmt.sizeimage);
    dbg_msg_api("   colorspace: %d", fmt.colorspace);

    if ( kdp_camera_buffer_init(nIdx) != 0 )
        ret = (ret|BIT5);//return -1;

#if KDP_CAM_DISPLAY == 1
    if ( m_cam_to_dp_evt_ack == 0 )
    {
        m_cam_to_dp_evt_ack = create_event();
    }

    if ( m_cam_to_dp_tid == 0 )
    {
        osThreadAttr_t attr = {
            .stack_size = 512,
            .attr_bits = osThreadJoinable
        };

#if (CFG_PANEL_TYPE == PANEL_NULL)
#if (CFG_CAMERA_ROTATE == 1)
        attr.priority = (osPriority_t)(osPriorityNormal - 1);
#endif
#endif

        m_cam_to_dp_tid = osThreadNew(camera_to_display_thread, (void*)&m_camera_ctx[nIdx], &attr);
    }
#endif

    if ( ret == 0 )
    {
        m_camera_ctx[nIdx].state = KDP_DEVICE_CAMERA_IDLE;
    }
    else
    {
        dbg_msg_err("[%s] error, cam_idx = %d, ret = %d", __func__, nIdx, ret);
    }

    return ret;
}

static int _api_camera_start(u8 nIdx)
{
    int ret = kdp_camera_start(nIdx);

    if ( ret == 0 )
    {
#if KDP_CAM_DISPLAY == 1
        set_thread_event(m_cam_to_dp_tid, FLAGS_APP_CAMERA_START_EVT);
#endif
        m_camera_ctx[nIdx].state = KDP_DEVICE_CAMERA_RUNNING;
    }

    return ret;
}

static int _api_camera_stop(u8 nIdx)
{
    int ret = -1;

#if KDP_CAM_DISPLAY == 1
    BOOL bStopEvt = TRUE;
    if ( nIdx != kdp_video_renderer_get_idx() )
    {
        bStopEvt = FALSE;
    }

    if ( ( bStopEvt == TRUE ) && ( check_thread_alive(m_cam_to_dp_tid) == TRUE ) )
    {
        set_thread_event(m_cam_to_dp_tid, FLAGS_APP_CAMERA_STOP_EVT);
        osDelay(1);
        wait_event(m_cam_to_dp_evt_ack, FLAGS_CAMERA_ACK);
        clear_event(m_cam_to_dp_evt_ack, FLAGS_CAMERA_ACK);
    }
#endif

    ret = kdp_camera_stop(nIdx);

    if ( ret == 0 )
    {
        m_camera_ctx[nIdx].state = KDP_DEVICE_CAMERA_IDLE;
    }

    return ret;
}

static int _api_camera_close(u8 nIdx)
{
    int ret = 0;

#if KDP_CAM_DISPLAY == 1
    BOOL bStopThread = TRUE;
    u8 i;
    for ( i = 0; i < IMGSRC_NUM; i++ )
    {
        if ( ( i != nIdx ) && ( m_camera_ctx[i].state != KDP_DEVICE_CAMERA_NULL ) )
        {
            bStopThread = FALSE;
            break;
        }
    }

    if ( ( bStopThread ) && ( check_thread_alive(m_cam_to_dp_tid) == TRUE ) )
    {
        set_thread_event(m_cam_to_dp_tid, FLAGS_APP_CAMERA_CLOSE_EVT);
        osDelay(1);
        wait_event(m_cam_to_dp_evt_ack, FLAGS_CAMERA_ACK);
        clear_event(m_cam_to_dp_evt_ack, FLAGS_CAMERA_ACK);
        osThreadJoin(m_cam_to_dp_tid);
        m_cam_to_dp_tid = NULL;
    }
#endif

    kdp_camera_close(nIdx);

    if ( nIdx == 0 )
    {
        if ( m_cb_camera0_power_off )
        {
            m_cb_camera0_power_off();
        }
    }
    else if ( nIdx == 1 )
    {
        if ( m_cb_camera1_power_off )
        {
            m_cb_camera1_power_off();
        }
    }

    m_camera_ctx[nIdx].state = KDP_DEVICE_CAMERA_NULL;

    return ret;
}

static void permanent_state_set(u8 nIdx, enum PERM_STATE ePermCtrl, enum PERM_STATE ePermSt)
{
    if ( ePermSt == ePermCtrl )
    {
        g_aPermanentState[nIdx] = TRUE;
    }
    else if ( ePermSt == PERMANENT_DISABLE )
    {
        g_aPermanentState[nIdx] = FALSE;
    }

#if ( CFG_GUI_ENABLE == YES )
    if ( ePermSt ==  PERMANENT_GUI )
    {
        g_bShowCamImgToDp = FALSE;
    }
#endif
}

u16 kl520_api_camera_open(unsigned int cam_idx)
{
    int ret = 0;
    ret = kl520_api_cam_disp_ctrl(API_CTRL_CAM_OPEN, cam_idx, PERMANENT_NULL);
    return ret;//return 0;
}

int kl520_api_camera_start(unsigned int cam_idx)
{
	int ret = 0;
    ret = kl520_api_cam_disp_ctrl(API_CTRL_CAM_STAR, cam_idx, PERMANENT_CAM);
    return ret;
}

int kl520_api_camera_stop(unsigned int cam_idx)
{
	int ret = 0;
    ret = kl520_api_cam_disp_ctrl(API_CTRL_CAM_STOP, cam_idx, PERMANENT_DISABLE);
    return ret;
}

int kl520_api_camera_close(unsigned int cam_idx)
{
    int ret = 0;
    ret = kl520_api_cam_disp_ctrl(API_CTRL_CAM_CLOS, cam_idx, PERMANENT_NULL);
    return ret;
}

s32 kl520_api_cam_disp_ctrl(u8 nCt, unsigned int nCamIdx, enum PERM_STATE ePermSt)
{
#if KDP_CAM_DISPLAY == 1
    BOOL bPanDisplayCt = (nCt&0x80)>>7; //[7]Control Bit6
    BOOL bPanDisplaySw = (nCt&0x40)>>6; //[6]Display on/off
    enum PERM_STATE eBackupPermSt = ePermSt;    
#endif

    bool cam_open = false;
    bool cam_start = false;
    bool cam_close = false;
    bool cam_stop = false;
    
    switch(nCt) {
    case API_CTRL_CAM_OPEN:
        cam_open = true;
        break;
    case API_CTRL_CAM_CLOS:
        cam_close = true;
        break;
    case API_CTRL_CAM_STAR:
        cam_start = true;
        break;
    case API_CTRL_CAM_STOP:
        cam_stop = true;
        break;
    case API_CTRL_CAM_EN:
        cam_open = true;
        cam_start = true;
        break;
    case API_CTRL_CAM_DIS:
        cam_stop = true;
        cam_close = true;
    default:
        break;
    }

    s32 ret = 0;
    if ( nCamIdx >= IMGSRC_NUM ) {
        dbg_msg_api("camera index invalid");
        return -1;
    }

    if ( cam_open && ( m_camera_ctx[nCamIdx].state <= KDP_DEVICE_CAMERA_NULL ) ) {
        tile_avg_valid_x = TILE_AVG_VALID_X;
        tile_avg_valid_y = TILE_AVG_VALID_Y;
        ret = (s32)_api_camera_open(nCamIdx);
        if(ret != 0) return -1;
    }

    if ( cam_start || cam_stop ) {
//        if ( ePermSt == PERMANENT_CAM_DISP )
//        {
//            ePermSt = PERMANENT_CAM;
//        }
        if ( cam_start ) {
            if ( m_camera_ctx[nCamIdx].state <= KDP_DEVICE_CAMERA_NULL ) {
                dbg_msg_api("camera[%d] is not inited!", nCamIdx);
                return -1;
            } else {
#if KDP_CAM_DISPLAY == 1
                g_bShowCamImgToDp = TRUE;
#endif

#if ( CFG_GUI_ENABLE == YES )
                if ( nCamIdx == kdp_video_renderer_get_idx() )
                {
                    if ( g_eCtrlFrom != CTRL_GUI )
                    {
                        gui_app_stop();
                    }
                }
                else
                {
//                    if ( g_eCtrlFrom == CTRL_CMD )
//                    {
//                        g_bShowCamImgToDp = FALSE;
//                    }
                }
#endif

                if ( ( m_camera_ctx[nCamIdx].state == KDP_DEVICE_CAMERA_IDLE )
                    || ( m_camera_ctx[nCamIdx].state == KDP_DEVICE_CAMERA_IDLE_PERM ) ) {
                    ret = (s32)_api_camera_start(nCamIdx);
                }
#if KDP_CAM_DISPLAY == 1
                else if ( g_eCamThreadState != CAM_THREAD_STATE_START )
                {
                    set_thread_event(m_cam_to_dp_tid, FLAGS_APP_CAMERA_START_EVT);  //For console control
                }
#endif
                permanent_state_set(nCamIdx, PERMANENT_CAM, ePermSt);
            }
        } else {
            if ( m_camera_ctx[nCamIdx].state == KDP_DEVICE_CAMERA_RUNNING )
            {
                permanent_state_set(nCamIdx, PERMANENT_CAM, ePermSt);
                ret |= (s32)_api_camera_stop(nCamIdx);

                if ( g_aPermanentState[nCamIdx] )
                {
                    m_camera_ctx[nCamIdx].state = KDP_DEVICE_CAMERA_IDLE_PERM;
                }
            }
#if ( CFG_GUI_ENABLE == YES )
            if ( m_camera_ctx[nCamIdx].state != KDP_DEVICE_CAMERA_RUNNING )
            {
                if ( ( !g_aPermanentState[nCamIdx] ) &&
                     ( gui_app_get_status() == 0 ) &&
                     ( nCamIdx == kdp_video_renderer_get_idx() ) )
                {
                    osDelay(10);
                    kl520_api_dp_layout_disable();
                    gui_app_proceed();

                    if ( ( g_ePreCtrlFrom == CTRL_COMM ) || ( g_eCtrlFrom == CTRL_CMD ) )
                    {
                        user_gui_update_renderer();
                    }
                }
            }
#endif
        }

        //ePermSt = eBackupPermSt;
    }

    if (cam_close) {
        if ( m_camera_ctx[nCamIdx].state == KDP_DEVICE_CAMERA_IDLE ) {
            ret |= (s32)_api_camera_close(nCamIdx);
        }
    }

#if KDP_CAM_DISPLAY == 1
    if ( bPanDisplayCt )
    {
        kl520_api_dp_layout_disable();

        if ( ( ePermSt == PERMANENT_GUI ) || ( ePermSt == PERMANENT_CAM_DISP ) )
        {
            permanent_state_set(GUI_IDX, PERMANENT_GUI, ePermSt);

            ePermSt = PERMANENT_DISP;
        }

        if ( ( bPanDisplaySw ) && ( g_eSofDispState == DEV_STATE_NULL ) )
        {
            permanent_state_set(DISP_IDX, PERMANENT_DISP, ePermSt);

            if (kl520_api_dp_open(g_nDpResX, g_nDpResY))
            {
                return ret;
            }

#if ( CFG_GUI_ENABLE == YES )
            if ( !(g_aPermanentState[CAMERA_DEVICE_RGB_IDX]|g_aPermanentState[CAMERA_DEVICE_NIR_IDX]) )
            {
                gui_app_proceed();
                user_gui_update_renderer();
            }
            else
#endif
            if ( ( g_eCamThreadState == CAM_THREAD_STATE_START ) || ( kdp_e2e_prop_get2(flow_mode) < FLOW_MODE_SIM_VIRTUAL ) )
            {
                g_bShowCamImgToDp = TRUE;
            }

            g_eSofDispState = DEV_STATE_RUN;
        }
        else if ( ( !bPanDisplaySw ) && ( g_eSofDispState == DEV_STATE_RUN ) )
        {
            g_bShowCamImgToDp = FALSE;

            permanent_state_set(DISP_IDX, PERMANENT_DISP, ePermSt);

#if ( CFG_GUI_ENABLE == YES )
            gui_app_stop();
#endif
            kl520_api_dp_close();

            g_eSofDispState = DEV_STATE_NULL;
        }
    }
#endif

    return ret;
}

s32 kl520_api_cam_disp_close_perm_state_chk(void)
{
    s32 ret = 0;

    if ( g_aPermanentState[CAMERA_DEVICE_RGB_IDX] )
    {
        ret |= kl520_api_cam_disp_ctrl(API_CTRL_CAM_STOP, CAMERA_DEVICE_RGB_IDX, PERMANENT_NULL);
    }
    else
    {
        ret |= kl520_api_cam_disp_ctrl(API_CTRL_CAM_DIS, CAMERA_DEVICE_RGB_IDX, PERMANENT_NULL);
    }

    if ( g_aPermanentState[CAMERA_DEVICE_NIR_IDX] )
    {
        ret |= kl520_api_cam_disp_ctrl(API_CTRL_CAM_STOP, CAMERA_DEVICE_NIR_IDX, PERMANENT_NULL);
    }
    else
    {
        ret |= kl520_api_cam_disp_ctrl(API_CTRL_CAM_DIS, CAMERA_DEVICE_NIR_IDX, PERMANENT_NULL);
    }

#if KDP_CAM_DISPLAY == 1
    if ( g_aPermanentState[DISP_IDX] )
    {
        ret |= kl520_api_cam_disp_ctrl(API_CTRL_DISP_OPEN, NULL, PERMANENT_NULL);
    }
    else
    {
        ret |= kl520_api_cam_disp_ctrl(API_CTRL_DISP_CLOS, NULL, PERMANENT_NULL);

    }
#endif

    return ret;
}

s32 kl520_api_cam_disp_state_rst(void)
{
    s32 ret = 0;

#if ( CFG_GUI_ENABLE == YES )
    if ( ( !(g_aPermanentState[CAMERA_DEVICE_RGB_IDX]|g_aPermanentState[CAMERA_DEVICE_NIR_IDX]) ) && ( g_bDpResChg ) )
    {
        kl520_api_disp_resolution_set(DISPLAY_WIDTH, DISPLAY_HEIGHT);
        g_bDpResChg = FALSE;

        user_gui_update_renderer();
    }
#endif

    if ( g_aPermanentState[CAMERA_DEVICE_RGB_IDX] )
    {
        ret |= kl520_api_cam_disp_ctrl(API_CTRL_CAM_EN, CAMERA_DEVICE_RGB_IDX, PERMANENT_NULL);
    }

    if ( g_aPermanentState[CAMERA_DEVICE_NIR_IDX] )
    {
        ret |= kl520_api_cam_disp_ctrl(API_CTRL_CAM_EN, CAMERA_DEVICE_NIR_IDX, PERMANENT_NULL);
    }

#if ( CFG_GUI_ENABLE == YES )
    if ( g_aPermanentState[GUI_IDX] )
    {
        permanent_state_set(DISP_IDX, PERMANENT_DISP, PERMANENT_DISP);

    }
#endif

#if KDP_CAM_DISPLAY == 1
    if ( g_aPermanentState[DISP_IDX] )
    {
        ret |= kl520_api_cam_disp_ctrl(API_CTRL_DISP_OPEN, NULL, PERMANENT_NULL);
    }
    else
    {
        ret |= kl520_api_cam_disp_ctrl(API_CTRL_DISP_CLOS, NULL, PERMANENT_NULL);
    }
#endif

    return ret;
}

s32 kl520_api_disp_open_chk(void)
{
    s32 ret = 0;
#if KDP_CAM_DISPLAY == 1

    if ( g_eSofDispState == DEV_STATE_RUN )
    {

    }
    else
    {
        ret = kl520_api_cam_disp_ctrl(API_CTRL_DISP_OPEN, NULL, PERMANENT_NULL);
    }
#endif
    return ret;
}

//s32 kl520_api_disp_close_chk(void)
//{
//    s32 ret = 0;

//    if ( g_eSofDispState == DEV_STATE_NULL )
//    {

//    }
//    else
//    {
//        ret = kl520_api_cam_disp_ctrl(API_CTRL_DISP_CLOS, NULL, PERMANENT_NULL);
//    }
//    return ret;
//}

void kl520_api_disp_resolution_set(u32 nW, u32 nH)
{
#if KDP_CAM_DISPLAY == 1
    if ( ( nW != g_nDpResX ) || ( nH != g_nDpResY ) )
    {
        struct video_input_params params = kdp_video_renderer_setting(kdp_video_renderer_get_idx());

        params.dp_out_w = nW;
        params.dp_out_h = nH;
        kdp_video_renderer_set_dp_res(&params);

        g_bDpResChg = TRUE;
    }


    g_nDpResX = nW;
    g_nDpResY = nH;
#endif
}

#if CFG_USB_EXPORT_STREAM_IMG == YES
BOOL kl520_api_cam_to_dp_thread_alive(void)
{
    return check_thread_alive( m_cam_to_dp_tid );
}
#endif

u16 kl520_api_set_exposure_only( u8 nCamIdx, u32 nExpTime )
{
    u16 ret = 0;
    
    if (nExpTime < MIN_DEFAULT_NIR_EXP_TIME)
        nExpTime = MIN_DEFAULT_NIR_EXP_TIME;
    else if (nExpTime > MAX_DEFAULT_NIR_EXP_TIME)
        nExpTime = MAX_DEFAULT_NIR_EXP_TIME;
    
    ret = kdp_camera_set_exp_time( nCamIdx, (nExpTime&0x0000FF00)>>8, nExpTime&0xFF );
    if (nCamIdx == MIPI_CAM_RGB)
        rgb_sensor_rst((u8)RGB_LED_WAIT_FRAME);
    else
        nir_sensor_rst((u8)NIR_LED_WAIT_FRAME);
    
    return ret;
}

#endif



