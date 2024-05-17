#include "board_kl520.h"

#include <stdlib.h>
#include "kneron_mozart_ext.h"
#include "board_ddr_table.h"
#include "pinmux.h"
#include "framework/v2k_image.h"
#include "framework/framework_errno.h"
#include "media/display/display.h"
#include "media/display/video_renderer.h"
#include "kdp520_gpio.h"
#include "kdp520_tmr.h"
#include "kdp_system.h"
#include "kdp_memxfer.h"
#include "kdp_memory.h"
#include "kdp_model.h"
#include "kdp_e2e_prop.h"
#include "kdp_e2e_face.h"
#include "ota_update.h"
#include "flash.h"
#include "kl520_api.h"
#include "kl520_include.h"
#include "kl520_api_device_id.h"
#include "kl520_api_fdfr.h"
#include "kl520_api_camera.h"
#include "kl520_api_snapshot.h"
#include "version.h"
#include "user_io.h" // will be removed in next version
#if ( CFG_GUI_ENABLE == YES )
#include "sample_gui_fsm_events.h"
#endif


#define SHOW_REGISTERED_DIRECTION_ARROWS

extern osMutexId_t flash_memx_lock;
extern u16 kdp520_memxfer_get_flash_device_id(void);

#ifdef DB_DRAWING_CUSTOMER_COLOR
extern int kl520_fdfr_drawing_timer_flag;
#endif

BOOL kl520_api_timer_init(pwmtimer timer, u32 tick)
{
    return kdp_timer_init(timer, tick);
}

void kl520_api_timer_tick_reset(pwmtimer timer)
{
    kdp_timer_tick_reset(timer);
}

int kl520_api_timer_close(pwmtimer timer)
{
    return kdp_timer_close(timer);
}

void kl520_api_tmr2_user(u32 *tick)
{
    static BOOL _led = TRUE; 
    if(*tick < kl520_api_dp_get_backlight())
    {
        if(_led != TRUE)
        {   
            //Temp sol
            kdp520_gpio_setdata(1 << GPIO_26);
        }
        _led = TRUE;
    }
    else
    {        
        if(_led != FALSE)
        {
            //Temp sol
            kdp520_gpio_cleardata(1 << GPIO_26);
        }
        _led = FALSE;
    }

    if(*tick >= 100)
    {
        *tick = 0;
    }
}

void kl520_api_tmr3_user(u32 *tick)
{
}

void kl520_api_tmr4_user(u32 *tick)
{
}

void kl520_api_tmr5_user(u32 *tick)
{
}

void kl520_api_tmr6_user(u32 *tick)
{
}
    
static u8 m_dp_inited = DEV_INIT_STATE_UNINIT;
static BOOL m_dp_layout_enable = FALSE;
static BOOL m_dp_five_face_enable = FALSE;

int kl520_api_dp_get_backlight(void)
{
    return user_io_get_backlight();
}

int kl520_api_dp_set_backlight(int light)
{
    return user_io_set_backlight(light);
}

int kl520_api_dp_fresh(void) 
{
    int ret = kdp_display_fresh();
#if CFG_USB_EXPORT_STREAM_IMG == YES
    {
        s8 export_image_ctrl = kl520_api_export_stream_get_image_crtl();
        eSTREAM_IMAGE_EXPORT_SRC export_rx_crtl = kl520_api_export_stream_get_image_export_crtl();
        if( export_rx_crtl == STRAM_IMAGE_DISPALY_e )
        {
            kl520_api_export_stream_image(MIPI_CAM_RGB, kdp_display_get_buffer_addr(), STRAM_IMAGE_DISPALY_e );
            kl520_api_export_stream_image_ready();
        }
    }
    
#endif //CFG_USB_EXPORT_STREAM_IMG == YES

#if ( CFG_PANEL_TYPE != PANEL_NULL )
    user_io_chk_backlight();
#endif

    return ret;
}


void kl520_api_dp_layout_enable(void)
{
    m_dp_layout_enable = TRUE;
}

void kl520_api_dp_layout_disable(void)
{
    m_dp_layout_enable = FALSE;
}

BOOL kl520_api_dp_layout_get(void)
{
    return m_dp_layout_enable;
}

void kl520_api_dp_five_face_enable(void)
{
    m_dp_five_face_enable = TRUE;
}

void kl520_api_dp_five_face_disable(void)
{
    m_dp_five_face_enable = FALSE;
}

BOOL kl520_api_dp_five_face_get(void)
{
    return m_dp_five_face_enable;
}

int kl520_api_dp_set_pen_rgb565(unsigned short color, unsigned int pen_width)
{
    return kdp_display_set_pen_rgb565(color, pen_width);
}

int kl520_api_dp_draw_line(u32 xs, u32 ys, u32 xe, u32 ye)
{
    return kdp_display_draw_line(xs, ys, xe, ye);
}

int kl520_api_dp_draw_bitmap(u32 org_x, u32 org_y, u32 width, u32 height, void *buf)
{
    return kdp_display_draw_bitmap(org_x, org_y, width, height, buf);
}

int kl520_api_dp_draw_rect(int x, int y, int width, int height)
{
    return kdp_display_draw_rect(x, y, width, height);
}

int kl520_api_dp_fill_rect(u32 org_x, u32 org_y, u32 width, u32 height)
{
    return kdp_display_fill_rect(org_x, org_y, width, height);
}

int kl520_api_dp_get_device_id(void)
{
	int nDeviceId;

	if ( m_dp_inited == DEV_INIT_STATE_UNINIT )
	{
		kl520_api_dp_open( DISPLAY_WIDTH, DISPLAY_HEIGHT );
		nDeviceId = kdp_display_get_device_id();
		kl520_api_dp_close();
	}
	else
	{
		nDeviceId = kdp_display_get_device_id();
	}

    return nDeviceId;
}

int kl520_api_dp_open(u32 width, u32 height)
{
    s32 ret = -1;
    struct video_input_params params;
    u8 cam_idx = kdp_video_renderer_get_idx();
    
    if (!m_dp_inited) {
        lcd_power_on();

        params = kdp_video_renderer_setting(cam_idx);
        params.dp_out_w = width;
        params.dp_out_h = height;

        //kl520_measure_stamp(E_MEASURE_DISPLAY_INIT);
        ret = kdp_video_renderer_open(&params);
        //kl520_measure_stamp(E_MEASURE_DISPLAY_INIT_DONE);
        if(ret == 0)
            m_dp_inited = DEV_INIT_STATE_INITED;
        else
        {
            m_dp_inited = DEV_INIT_STATE_ERROR;
        }
        
    }
    
    return ret;
}

u8 kl520_api_dp_init_get(void)
{
    return m_dp_inited;
}

int kl520_api_dp_fresh_bg(u16 nColor, u8 nWidth)
{    
    int ret = 0;
#if ( CFG_PANEL_TYPE != PANEL_NULL )
    do
    {
        KDP_CHK_BREAK(kl520_api_dp_set_pen_rgb565(nColor, nWidth));
        KDP_CHK_BREAK(kl520_api_dp_fill_rect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT));
        KDP_CHK_BREAK(kl520_api_dp_fresh());
    } while (0);
#endif
    return ret;
}

void kl520_api_dp_close(void)
{
    if (DEV_INIT_STATE_INITED == m_dp_inited) {
#if ( CFG_PANEL_TYPE != PANEL_NULL )
        struct video_input_params params;
        kdp_video_renderer_get_params(&params);

        kl520_api_dp_fresh_bg(BLACK, 2);

        osDelay(1);
        kl520_api_dp_set_backlight(0);//turn off backlight

        kdp_video_renderer_stop();
#endif
        m_dp_inited = DEV_INIT_STATE_UNINIT;
    }
}

#if (CFG_PANEL_TYPE > PANEL_NULL)
void kl520_api_dp_layout(void)
{   
#if CFG_UI_USR_IMG == NO
    kl520_api_dp_set_pen_rgb565(RED, 2);
    kl520_api_dp_draw_bitmap(0, 0, 240, 135, (void *)USR_DDR_IMG_01_KNERON_ADDR);
    kl520_api_dp_draw_bitmap(0, (320-45), 80, 45, (void *)USR_DDR_IMG_02_ICON_01_ADDR);
    kl520_api_dp_draw_bitmap(80, (320-45), 80, 45, (void *)USR_DDR_IMG_03_ICON_02_ADDR);
    kl520_api_dp_draw_bitmap(160, (320-45), 80, 45, (void *)USR_DDR_IMG_05_ICON_04_ADDR);
    kl520_api_dp_draw_rect(150, 30, 80, 200);
#endif
}
#endif


extern s8 face_pose_val[];
extern int kdp_display_draw_line(u32 xs, u32 ys, u32 xe, u32 ye);

#if (CFG_GUI_ENABLE == YES)
static osTimerId_t kl520_pose_timer = 0;
static int8_t kl520_arrow_show_hor = 0;
static int8_t kl520_arrow_show_ver = 0;
static void kl520_timer_cb(void *argument)
{
	kl520_arrow_show_hor = !kl520_arrow_show_hor;
	kl520_arrow_show_ver = !kl520_arrow_show_ver;
}

void kl520_pose_timer_stop(void)
{
	osTimerStop(kl520_pose_timer);
	kl520_arrow_show_hor = 0;
	kl520_arrow_show_ver = 0;
}

void kl520_pose_timer_delete(void)
{
    osTimerDelete(kl520_pose_timer);
    kl520_pose_timer = 0;
}

static void _kl520_api_dp_draw_bitmap_reg_arrow(u8 arrow, u8 ok)
{
    u8 bmp_s1;
    u8 bmp_s2;
    u8 bmp_gap = 3;
    u32 img;
    u32 x, y, w, h;

    struct video_input_params params;
    
    if(kdp_video_renderer_get_params(&params))
        return; 

    u32 xe = (u32)(params.dp_out_w);
    u32 ye = (u32)(params.dp_out_h);

    if (ok)
    {
        bmp_s1 = 45;
        bmp_s2 = 159;
    }
    else
    {
        bmp_s1 = 33;
        bmp_s2 = 17;
    }
    
#if (CFG_CAMERA_ROTATE == 1)
    switch (arrow)
    {
        case FACE_ADD_TYPE_LEFT:
            arrow = FACE_ADD_TYPE_UP;
            break;
        case FACE_ADD_TYPE_RIGHT:
            arrow = FACE_ADD_TYPE_DOWN;
            break;
        case FACE_ADD_TYPE_UP:
            arrow = FACE_ADD_TYPE_RIGHT;
            break;
        case FACE_ADD_TYPE_DOWN:
            arrow = FACE_ADD_TYPE_LEFT;
            break;
        default:
            break;
    }
#endif

    switch (arrow)
    {
        case FACE_ADD_TYPE_LEFT:
            img = (ok)?USR_DDR_IMG_ICON_ARROW_LEFT_OK_ADDR:USR_DDR_IMG_ICON_ARROW_LEFT_ADDR;
            w = bmp_s1;
            h = bmp_s2;
            x = bmp_gap;
            y = (ye - h) >> 1;
            break;
        case FACE_ADD_TYPE_RIGHT:
            img = (ok)?USR_DDR_IMG_ICON_ARROW_RIGHT_OK_ADDR:USR_DDR_IMG_ICON_ARROW_RIGHT_ADDR;
            w = bmp_s1;
            h = bmp_s2;
            x = xe - bmp_gap - w;
            y = (ye - h) >> 1;
            break;
        case FACE_ADD_TYPE_UP:
            img = (ok)?USR_DDR_IMG_ICON_ARROW_UP_OK_ADDR:USR_DDR_IMG_ICON_ARROW_UP_ADDR;
            w = bmp_s2;
            h = bmp_s1;
            x = (xe - w) >> 1;
            y = bmp_gap;
            break;
        case FACE_ADD_TYPE_DOWN:
            img = (ok)?USR_DDR_IMG_ICON_ARROW_DOWN_OK_ADDR:USR_DDR_IMG_ICON_ARROW_DOWN_ADDR;
            w = bmp_s2;
            h = bmp_s1;
            x = (xe - w) >> 1;
            y = ye - bmp_gap - h;
            break;
        default:
            break;
    }

    kl520_api_dp_draw_bitmap(x, y, w, h, (void*)img);
}

void kl520_api_dp_layout_pose(void)
{   
    if((FACE_MODE_ADD != m_face_mode)
    || (kl520_api_dp_five_face_get() == FALSE))
        return;
    
    struct video_input_params params;
    
    if(kdp_video_renderer_get_params(&params))
        return; 
    
    u32 org_x = 0;
    u32 org_y = 0;

    int xs = (int)(params.dp_out_w/2);
    int ys = (int)(params.dp_out_h/2); 
    
    u32 xe =  xs;
    u32 ye =  ys;
    
    kl520_api_dp_set_pen_rgb565(LCD_DISPLAY_HINT_BOX_COLOR, LCD_DISPLAY_HINT_BOX_PEN_WIDTH);

    if (0 == kl520_pose_timer)
    {
        kl520_pose_timer = osTimerNew(kl520_timer_cb, osTimerPeriodic, NULL, NULL);
        kl520_arrow_show_hor = 0;
        kl520_arrow_show_ver = 0;
    }

    if(face_reg_sts & (0x01 << FACE_ADD_TYPE_NORMAL))
    {
        kl520_api_dp_set_pen_rgb565(LCD_DISPLAY_FD_OK_COLOR, LCD_DISPLAY_FD_OK_PEN_WIDTH);

        if (0 == osTimerIsRunning(kl520_pose_timer))
        {
            osTimerStart(kl520_pose_timer, 500);
            kl520_arrow_show_hor = 1;
            kl520_arrow_show_ver = 0;
        }

        //arrow
#ifdef SHOW_REGISTERED_DIRECTION_ARROWS
        if(face_reg_sts & (0x01 << FACE_ADD_TYPE_LEFT))
        {
            _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_LEFT, 1);
        }
        else
        {
            if (kl520_arrow_show_hor)
            {
                _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_LEFT, 0);
            }
        }

        if(face_reg_sts & (0x01 << FACE_ADD_TYPE_RIGHT))
        {
            _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_RIGHT, 1);
        }
        else
        {
            if (kl520_arrow_show_hor)
            {
                _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_RIGHT, 0);
            }
        }
#else
        if (kl520_arrow_show_hor)
        {
            _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_LEFT, 0);
            _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_RIGHT, 0);
        }
#endif
    }

    //left ok and right ok
#if (HEAD_POSE_LR_FIRST == YES)
    if((face_reg_sts & (0x01 << FACE_ADD_TYPE_LEFT)) && (face_reg_sts & (0x01 << FACE_ADD_TYPE_RIGHT)))
#endif
    {
        kl520_api_dp_set_pen_rgb565(LCD_DISPLAY_FD_OK_COLOR, LCD_DISPLAY_FD_OK_PEN_WIDTH);

        //LEFT RIGHT arrow
#ifdef SHOW_REGISTERED_DIRECTION_ARROWS
        if(face_reg_sts & (0x01 << FACE_ADD_TYPE_UP))
        {
            _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_UP, 1);
        }
        else
        {
            if (kl520_arrow_show_ver)
            {
                _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_UP, 0);
            }
        }

        if(face_reg_sts & (0x01 << FACE_ADD_TYPE_DOWN))
        {
            _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_DOWN, 1);
        }
        else
        {
            if (kl520_arrow_show_ver)
            {
                _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_DOWN, 0);
            }
        }
#else
        _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_LEFT, 1);
        _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_RIGHT, 1);
        if (kl520_arrow_show_ver)
        {
            _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_UP, 0);
            _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_DOWN, 0);
        }
#endif
    }

    //up ok and down ok
    if((face_reg_sts & (0x01 << FACE_ADD_TYPE_UP)) && (face_reg_sts & (0x01 << FACE_ADD_TYPE_DOWN)))
    {
        kl520_api_dp_set_pen_rgb565(LCD_DISPLAY_FD_OK_COLOR, LCD_DISPLAY_FD_OK_PEN_WIDTH);

#ifndef SHOW_REGISTERED_DIRECTION_ARROWS
        _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_LEFT, 1);
        _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_RIGHT, 1);
        _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_UP, 1);
        _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_DOWN, 1);
#endif

        osTimerStop(kl520_pose_timer);
        kl520_arrow_show_hor = 0;
        kl520_arrow_show_ver = 0;
    }
    
    kdp_display_draw_line( org_x + (xs- KL520_MIDDLE_LINE1), org_y + ys, org_x + (xe + KL520_MIDDLE_LINE1), org_y + ye);
    kdp_display_draw_line( org_x + xs, org_y + (ys - KL520_MIDDLE_LINE1), org_x + xe, org_y + (ye + KL520_MIDDLE_LINE1));  
    
}

#ifdef DISPLAY_REGISTER_WITH_CUSTOM_IMG
static void _kl520_api_dp_layout_for_registration(void)
{
    struct video_input_params params;
    
    if(kdp_video_renderer_get_params(&params))
        return; 
    
    u32 org_x = 0;
    u32 org_y = 0;

    u32 xs = (u32)(params.dp_out_w/2);
    u32 ys = (u32)(params.dp_out_h/2);
    u32 xe =  xs;
    u32 ye =  ys;

    kl520_api_dp_set_pen_rgb565(LCD_DISPLAY_HINT_BOX_COLOR, LCD_DISPLAY_HINT_BOX_PEN_WIDTH);

    if (0 == kl520_pose_timer)
    {
        kl520_pose_timer = osTimerNew(kl520_timer_cb, osTimerPeriodic, NULL, NULL);
        kl520_arrow_show_hor = 0;
        kl520_arrow_show_ver = 0;
    }


    if(face_reg_sts & (0x01 << FACE_ADD_TYPE_NORMAL))
    {
        kl520_api_dp_set_pen_rgb565(LCD_DISPLAY_FD_OK_COLOR, LCD_DISPLAY_FD_OK_PEN_WIDTH);

        if (0 == osTimerIsRunning(kl520_pose_timer))
        {
            osTimerStart(kl520_pose_timer, 500);
            kl520_arrow_show_hor = 1;
            kl520_arrow_show_ver = 1;
        }

        //arrow
        if(face_reg_sts & (0x01 << FACE_ADD_TYPE_LEFT))
        {
            _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_LEFT, 1);
        }
        else
        {
            if (kl520_arrow_show_hor)
            {
                _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_LEFT, 0);
            }
        }

        if(face_reg_sts & (0x01 << FACE_ADD_TYPE_RIGHT))
        {
            _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_RIGHT, 1);
        }
        else
        {
            if (kl520_arrow_show_hor)
            {
                _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_RIGHT, 0);
            }
        }
    }

    //left ok and right ok
#if (HEAD_POSE_LR_FIRST == YES)
    if((face_reg_sts & (0x01 << FACE_ADD_TYPE_LEFT)) && (face_reg_sts & (0x01 << FACE_ADD_TYPE_RIGHT)))
#endif
    {
        kl520_api_dp_set_pen_rgb565(LCD_DISPLAY_FD_OK_COLOR, LCD_DISPLAY_FD_OK_PEN_WIDTH);

        //LEFT RIGHT arrow
        if(face_reg_sts & (0x01 << FACE_ADD_TYPE_UP))
        {
            _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_UP, 1);
        }
        else
        {
            if (kl520_arrow_show_ver)
            {
                _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_UP, 0);
            }
        }

        if(face_reg_sts & (0x01 << FACE_ADD_TYPE_DOWN))
        {
            _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_DOWN, 1);
        }
        else
        {
            if (kl520_arrow_show_ver)
            {
                _kl520_api_dp_draw_bitmap_reg_arrow(FACE_ADD_TYPE_DOWN, 0);
            }
        }
    }

    //up ok and down ok
    if((face_reg_sts & (0x01 << FACE_ADD_TYPE_UP)) && (face_reg_sts & (0x01 << FACE_ADD_TYPE_DOWN)))
    {
        kl520_api_dp_set_pen_rgb565(LCD_DISPLAY_FD_OK_COLOR, LCD_DISPLAY_FD_OK_PEN_WIDTH);
        osTimerStop(kl520_pose_timer);
        kl520_arrow_show_hor = 0;
        kl520_arrow_show_ver = 0;
    }
    
    kdp_display_draw_line( org_x + (xs- KL520_MIDDLE_LINE1), org_y + ys, org_x + (xe + KL520_MIDDLE_LINE1), org_y + ye);
    kdp_display_draw_line( org_x + xs, org_y + (ys - KL520_MIDDLE_LINE1), org_x + xe, org_y + (ye + KL520_MIDDLE_LINE1));  

    kl520_api_dp_draw_bitmap((240-182)/2, 320-30, 182, 30, (void *)USR_DDR_IMG_RECOGNIZE_TEXT_ADDR);
}

static void _kl520_api_dp_layout_for_recognition(void)
{
    struct video_input_params params;
    
    if(kdp_video_renderer_get_params(&params))
        return; 

#if ( CFG_GUI_ENABLE == YES )
    user_gui_update_renderer();
    //kl520_api_dp_draw_bitmap((240-66), 0, 66, 66, (void *)USR_DDR_IMG_DIGIT_0_ADDR);
#endif
}

void kl520_api_dp_layout_pose_with_customized_size_img(void)
{   
    if((FACE_MODE_ADD == m_face_mode) && (kl520_api_dp_five_face_get() == TRUE))
        _kl520_api_dp_layout_for_registration();
    else if( (FACE_MODE_RECOGNITION == m_face_mode) )
        _kl520_api_dp_layout_for_recognition();
    else
        return;
}
#endif
#endif

void _kl520_api_dp_layout_lm(kl520_dp_draw_info *info, int flag)
{   
    u16 color;
    u32 pen_width;
    
    struct video_input_params params;
    
    if(kdp_video_renderer_get_params(&params))
        return; 
    
    int org_x = (int)0;
    int org_y = (int)0;

    int dp_w = (int)params.dp_out_w;
    int dp_h = (int)params.dp_out_h;

    int img_w = (int)params.dp_area_w;
    int img_h = (int)params.dp_area_h;

    
    if (KL520_FACE_FAIL == flag)
    {
#if (KL520_API_SHOW_FAIL_LM == YES)
        color = LCD_DISPLAY_FDR_FAIL_COLOR;
        pen_width = LCD_DISPLAY_FDR_FAIL_PEN_WIDTH;
#else
        return;
#endif
    }
    else
    {
        color = LCD_DISPLAY_LM_OK_COLOR;
        pen_width = LCD_DISPLAY_LM_OK_PEN_WIDTH;
    }

    kl520_api_dp_set_pen_rgb565(color, pen_width);

    if (params.src_cam_idx == CAMERA_DEVICE_NIR_IDX) { /*NIR*/
        int bbx = (int)(org_x + info->n1_rc.start_x - DISPLAY_NIR_X_OFFSET) * dp_w / img_w;
        int bby = (int)(org_y + info->n1_rc.start_y - DISPLAY_NIR_Y_OFFSET) * dp_h / img_h;
        int bbw = (int)(info->n1_rc.end_x - info->n1_rc.start_x) * dp_w / img_w;
        int bbh = (int)(info->n1_rc.end_y - info->n1_rc.start_y) * dp_h / img_h;
    
        for(u8 i=0;i<LAND_MARK_POINTS;i++)
        {
            int lmx = (int)(org_x + info->n1_pt_array[i].x - DISPLAY_NIR_X_OFFSET) * dp_w / img_w;
            int lmy = (int)(org_y + info->n1_pt_array[i].y - DISPLAY_NIR_Y_OFFSET) * dp_h / img_h;
            
            if(lmx>0 && lmy>0)
                kl520_api_dp_draw_rect(lmx, lmy, 4, 4);
        }
        
        kl520_api_dp_set_pen_rgb565(color, LCD_DISPLAY_LM_BOX_PEN_WIDTH);
            if(bbx>0 && bby>0)
                kl520_api_dp_draw_rect(bbx, bby, bbw, bbh);
    
    }
    else if (params.src_cam_idx == CAMERA_DEVICE_RGB_IDX) {
        int bbx = (int)(org_x + info->r1_rc.start_x - DISPLAY_RGB_X_OFFSET) * dp_w / img_w;
        int bby = (int)(org_y + info->r1_rc.start_y - DISPLAY_RGB_Y_OFFSET) * dp_h / img_h;
        int bbw = (int)(info->r1_rc.end_x - info->r1_rc.start_x) * dp_w / img_w;
        int bbh = (int)(info->r1_rc.end_y - info->r1_rc.start_y) * dp_h / img_h;
    
        for(u8 i=0;i<LAND_MARK_POINTS;i++)
        {
            int lmx = (int)(org_x + info->r1_pt_array[i].x - DISPLAY_RGB_X_OFFSET) * dp_w / img_w;
            int lmy = (int)(org_y + info->r1_pt_array[i].y - DISPLAY_RGB_Y_OFFSET) * dp_h / img_h;
            
            if(lmx>0 && lmy>0)
                kl520_api_dp_draw_rect(lmx, lmy, 4, 4);
        }
                    
        kl520_api_dp_set_pen_rgb565(color, LCD_DISPLAY_LM_BOX_PEN_WIDTH);
            if(bbx>0 && bby>0)
                kl520_api_dp_draw_rect(bbx, bby, bbw, bbh);
    }
}

int kl520_api_get_scpu_version(struct fw_misc_data *g_fw_misc) 
{
    u32 temp;
    
    temp = readl(SdRAM_MEM_BASE + SdRAM_MEM_SIZE - DRAM_MISCDATA_SIZE);
    g_fw_misc->version[0] = (u8)temp;
    g_fw_misc->version[1] = (u8)(temp>>8);
    g_fw_misc->version[2] = (u8)(temp>>16);
    g_fw_misc->version[3] = (u8)(temp>>24);
    g_fw_misc->date = readl(SdRAM_MEM_BASE + SdRAM_MEM_SIZE - DRAM_MISCDATA_SIZE + 4);
    dbg_msg_api("ZF-BP3-X-2509_v%d.%d.%d.%d_%d.bin"   , g_fw_misc->version[0]
                                                            , g_fw_misc->version[1]
                                                            , g_fw_misc->version[2] 
                                                            , g_fw_misc->version[3]
                                                            , g_fw_misc->date);
    return 0;
}

int kl520_api_get_ncpu_version(struct fw_misc_data *g_fw_misc) 
{
    u32 temp;
    
    temp = readl(SdRAM_MEM_BASE + SdRAM_MEM_SIZE - DRAM_MISCDATA_SIZE + 8);
    g_fw_misc->version[0] = (u8)temp;
    g_fw_misc->version[1] = (u8)(temp>>8);
    g_fw_misc->version[2] = (u8)(temp>>16);
    g_fw_misc->version[3] = (u8)(temp>>24);
    g_fw_misc->date = readl(SdRAM_MEM_BASE + SdRAM_MEM_SIZE - DRAM_MISCDATA_SIZE + 12);
    dbg_msg_api("KDP520-NCPU_v%d.%d.%d.%d_%d.bin"   , g_fw_misc->version[0]
                                                            , g_fw_misc->version[1]
                                                            , g_fw_misc->version[2] 
                                                            , g_fw_misc->version[3]
                                                            , g_fw_misc->date);
    return 0;
}

struct fw_misc_data *kl520_api_get_model_version(void) 
{
    return &g_model_version;
}

u32 kl520_api_get_unique_id(void) {
    return kdp_sys_get_unique_id();
}
u16 kl520_api_memxfer_get_device_id(void) {
    return kdp520_memxfer_get_flash_device_id();
}
// static u16 kl520_api_io_ext_get_device_id(u32 device_idx) {
//     return user_io_get_extra_device_id(0);
// }

int kl520_api_get_version_info(system_info* t_system_info)
{
    int ret = 0;
    
    //TODO : get fw version and kl520 id.
    t_system_info->unique_id = kl520_api_get_unique_id();
    t_system_info->spl_version = kdp_sys_get_spl_version();
    kl520_api_get_scpu_version(&t_system_info->fw_scpu_version);
    kl520_api_get_ncpu_version(&t_system_info->fw_ncpu_version);
    
    {
        kdp_model_load_model(-1);
        t_system_info->model_count = kdp_model_get_model_count();
        if (!t_system_info->model_infos)
            t_system_info->model_infos = (struct kdp_model_s**)kdp_ddr_reserve(sizeof(struct kdp_model_s*) * t_system_info->model_count);
        for (int i = 0; i < t_system_info->model_count; ++i) {
            t_system_info->model_infos[i] = (struct kdp_model_s*)kdp_model_get_model_info(i);
        }
    }
    
    return ret;
}

int kl520_api_get_device_info(system_info* t_system_info)
{
    int ret = 0;
    
    //TODO : get fw version and kl520 id.
    t_system_info->unique_id = kl520_api_get_unique_id();
    t_system_info->spl_version = kdp_sys_get_spl_version();
    kl520_api_get_scpu_version(&t_system_info->fw_scpu_version);
    kl520_api_get_ncpu_version(&t_system_info->fw_ncpu_version);
    
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) && ( UART_PROTOCOL_VERSION >= 0x0200 )
	extern u16 kdp_comm_get_protocol_version(void);
    t_system_info->com_protocol_version = kdp_comm_get_protocol_version();
#endif


    {
        kdp_model_load_model(-1);
        t_system_info->model_count = kdp_model_get_model_count();
        if (!t_system_info->model_infos)
            t_system_info->model_infos = (struct kdp_model_s**)kdp_ddr_reserve(sizeof(struct kdp_model_s*) * t_system_info->model_count);
        for (int i = 0; i < t_system_info->model_count; ++i) {
            t_system_info->model_infos[i] = (struct kdp_model_s*)kdp_model_get_model_info(i);
        }
    }

    //NIR camera
    strncpy(t_system_info->device_id_0.device_name, "Camera 0", sizeof(t_system_info->device_id_0.device_name));
    t_system_info->device_id_0.id = kl520_api_camera_get_id(0);
    //RGB camera
    strncpy(t_system_info->device_id_1.device_name, "Camera 1", sizeof(t_system_info->device_id_1.device_name));
    t_system_info->device_id_1.id = kl520_api_camera_get_id(1);
    //Flash
    strncpy(t_system_info->device_id_2.device_name, "nor flash", sizeof(t_system_info->device_id_2.device_name));
    t_system_info->device_id_2.id = kl520_api_memxfer_get_device_id();
    //Display
    strncpy(t_system_info->device_id_3.device_name, "Display", sizeof(t_system_info->device_id_3.device_name));
    t_system_info->device_id_3.id = kl520_api_dp_get_device_id();
    //Touch
    strncpy(t_system_info->device_id_4.device_name, "Touch", sizeof(t_system_info->device_id_4.device_name));
    t_system_info->device_id_4.id = kl520_api_touch_get_device_id();

    //t_system_info->device_id_5.device_name = "aw9523b";
    //t_system_info->device_id_5.id = kl520_api_io_ext_get_device_id(0);
    t_system_info->extra_device_cnt = user_io_get_extra_device_cnt();
    if (t_system_info->extra_device_cnt > 0) {
        t_system_info->extra_device_id_array = 
            (device_id*)malloc(t_system_info->extra_device_cnt * sizeof(device_id));
        if (t_system_info->extra_device_id_array) {
            for (u32 i = 0; i < t_system_info->extra_device_cnt; ++i) {
                device_id *device = &t_system_info->extra_device_id_array[i];
                device->id = user_io_get_extra_device_id(i);
                user_io_get_extra_device_name(i, device->device_name, sizeof(device->device_name));
                if (device->id == 0) {
                    ret |= KL520_DEVICE_ERR_IOEXTENDER_ID;
                }
            }
        }
    }

    if(t_system_info->device_id_0.id != NIR_SENSOR_ID)
        ret |= KL520_DEVICE_ERR_NIR_ID;

    if(t_system_info->device_id_1.id != RGB_SENSOR_ID)
        ret |= KL520_DEVICE_ERR_RGB_ID;

    if(t_system_info->device_id_2.id != FLASH_ID)
        ret |= KL520_DEVICE_ERR_FLASH_ID;

    if(t_system_info->device_id_3.id != LCM_ID)
        ret |= KL520_DEVICE_ERR_LCM_ID;

    if(t_system_info->device_id_4.id != TOUCH_ID)
        ret |= KL520_DEVICE_ERR_TOUCH_ID;

    return ret;
}

void kl520_api_free_device_info(system_info* t_system_info)
{
    if (t_system_info->extra_device_id_array) {
        free(t_system_info->extra_device_id_array);
        t_system_info->extra_device_id_array = NULL;
    }
}

static osEventFlagsId_t api_event_id = 0;
osEventFlagsId_t kl520_api_get_event()
{
    if (0 == api_event_id)
    {
        api_event_id = create_event();
    }

    return api_event_id;
}

#ifdef DB_DRAWING_CUSTOMER_COLOR
void kl520_api_face_notify(int face_status)
{
    if((KL520_FACE_DB_OK == face_status)
    || (KL520_FACE_DB_FAIL == face_status))
    {
        kl520_fdfr_drawing_timer_flag = face_status;
    }
}
#endif

#if (MEASURE_RECOGNITION == YES)
u32 nTimeStamp[E_MEASURE_NUM];
#endif

void kl520_measure_init(void){
    
    kdp_tmr_init(TMR3, PWMTMR_1MSEC_PERIOD);
}

#if (MEASURE_RECOGNITION == YES)
void kl520_measure_stamp(measure_stamp_e eStamp)
{
    if(nTimeStamp[eStamp] == 0) 
        nTimeStamp[eStamp] = GetCurrentT3Tick();
}
#endif

void kl520_measure_info(void){
#if (MEASURE_RECOGNITION == YES)    
    u16 i;
    
    dbg_msg_console(" ID |MEASURE_TIMESTAMP           | TIMESTAMP(ms)");
    
    for(i=0;i<E_MEASURE_NUM;i++)
    {   
        dbg_msg_console(" %02d |%-33s| %06d", i, MEASURE_STAMP_STRING[i], nTimeStamp[i]);
    }

    i = 68;
    dbg_msg_console(" ID |MEASURE_TIMESTAMP           | TIMESTAMP(ms)");
    dbg_msg_console(" %02d |%-33s| %06d", i, MEASURE_STAMP_STRING[i], nTimeStamp[i]);
#endif
}

void kl520_api_log_set_user_level(u32 cpu_id, u8 level)
{
    if (kdp_com_is_inited()) {
        if (cpu_id == CPU_ID_SCPU)
            log_set_user_level_scpu(level);
        else
            log_set_user_level_ncpu(level);
    }
}

void kl520_api_set_rgb_led_level(BOOL reset, s32 level)
{
    kdp_e2e_prop *prop = kdp_e2e_prop_get_inst();
    if (!reset) {
        kdp_e2e_prop_set_manual_value(prop, r1_led_value, level);
    }
    else
    {
        kdp_e2e_prop_reset_r1_led_value();
    }
}

//////////////////////////
/* Engineering mode API */
//////////////////////////
//u8 default_cam_idx = IMGSRC_NUM; //default unused

//void kl520_engineering_switch_rbg_nir(void)
//{        
//    if(default_cam_idx == IMGSRC_NUM)
//    {
//        default_cam_idx = CAMERA_DEVICE_NIR_IDX;
//    }
//    else
//    {
//        default_cam_idx++;
//        default_cam_idx = default_cam_idx%IMGSRC_NUM;
//    }
//}

//int kl520_engineering_get_camera_num(void)
//{
//    int ret = -1;
//    
//    struct video_input_params params;

//    ret = kdp_video_renderer_get_params(&params);
//    
//    if(ret == -1)
//        return -1;
//    
//    return params.src_cam_idx;
//    
////    
////    if(params.src_cam_idx != default_cam_idx && default_cam_idx != IMGSRC_NUM)       //switch setting
////    {
////        if (params.src_cam_idx == CAMERA_DEVICE_RGB_IDX)
////        {
////            params.src_cam_idx = CAMERA_DEVICE_NIR_IDX; 
////            params.src_width = NIR_IMG_SOURCE_W;
////            params.src_height = NIR_IMG_SOURCE_H;

////            params.dp_out_w = DISPLAY_NIR_OUT_WIDTH;                    //display
////            params.dp_out_h = DISPLAY_NIR_OUT_HEIGHT;                   //display                    
////            
////            params.dp_area_x = 0;             //display origin x
////            params.dp_area_y = 0;             //display origin y
////            
////            params.dp_area_w = DISPLAY_NIR_WIDTH;
////            params.dp_area_h = DISPLAY_NIR_HEIGHT;
////        }
////        else if (params.src_cam_idx == CAMERA_DEVICE_NIR_IDX){

////            params.src_cam_idx = CAMERA_DEVICE_RGB_IDX;
////            params.src_width = RGB_IMG_SOURCE_W;
////            params.src_height = RGB_IMG_SOURCE_H;

////            params.dp_out_w = DISPLAY_RGB_OUT_WIDTH;                    //display
////            params.dp_out_h = DISPLAY_RGB_OUT_HEIGHT;                   //display                  
////            
////            params.dp_area_x = 0;               //display origin x
////            params.dp_area_y = 0;               //display origin y

////            params.dp_area_w = DISPLAY_RGB_WIDTH;
////            params.dp_area_h = DISPLAY_RGB_HEIGHT;

////        }

////        kdp_video_engineering_switch(&params);  
////    }

//}

int kl520_engineering_calibration(u8 type, u32 *args)
{
    int ret = -1;
    u16 height = DISPLAY_HEIGHT;

    int old_tm = kl520_api_face_add_get_timeout();
    kl520_api_face_add_set_timeout(type); //set timeout val;

    //set to 1 face mode
    kl520_api_face_set_add_mode(FACE_ADD_MODE_1_FACE);
    kdp_e2e_prop_set2(face_mode, FACE_MODE_ENG_CAL); //set to calibration mode

    if(kl520_api_face_add_internal(0, 0, DISPLAY_WIDTH, height, FACE_ADD_TYPE_NORMAL) != KL520_FACE_OK)
    {
        dbg_msg_algo("camera engineering calibration failed.\n");
        ret = -1;
    } else {
        ret = 0;
    }

    kl520_api_face_add_set_timeout(old_tm); //set it back
    kl520_api_face_close();

    return ret;
}

void kl520_api_ota_switch_SCPU(void)
{
    ota_update_force_switch_active_partition(1);
}

void kl520_api_ota_switch_NCPU(void)
{
    ota_update_force_switch_active_partition(2);
}

int kl520_api_model_count(void)
{
    return kdp_model_get_model_count();
}

int kl520_api_model_version(uint8_t idx)
{
    return kdp_model_version( idx );
}

int kl520_api_customer_get(kl520_customer_info *cusinfo){

    //int ret = -1;
    static u32 CusInfoAddr = 0;
    u32 nLength = sizeof(kl520_customer_info);
    if(nLength > KDP_FLASH_INF_SIZE){
        dbg_msg_api("error Length");
    }

    if(CusInfoAddr == 0){
        CusInfoAddr= kdp_ddr_reserve(nLength);
    }

    kdp_memxfer_flash_to_ddr(CusInfoAddr, KDP_FLASH_INF_ADDR, nLength);

    memcpy(cusinfo, (void *)CusInfoAddr, nLength);

    return 0;
}

int kl520_api_customer_write(kl520_customer_info *cusinfo){

    kdp_memxfer_ddr_to_flash((u32)KDP_FLASH_INF_ADDR, (u32)cusinfo, sizeof(kl520_customer_info));

    return 0;
}

BOOL kl520_api_customer_chk_key_exist(u8* ptr, u8 nLen)
{
    u8 i, nCnt = 0;

    for ( i = 0; i < nLen; i++ )
    {
        if ( *(ptr+i) == 0xFF )
        {
            nCnt++;
        }
    }

    if ( nCnt == nLen)
    {
        return FALSE;   //No Key
    }
    else
    {
        return TRUE;   //Key inside
    }
}

int kl520_api_customer_clr(void)
{

    memset((void *)KDP_DDR_MEM_START, 0xFF, sizeof(kl520_customer_info));
    kdp_memxfer_ddr_to_flash((u32)KDP_FLASH_INF_ADDR, (u32)KDP_DDR_MEM_START, sizeof(kl520_customer_info));

    return 0;
}

u32 kl520_api_customer_info(void){

    kl520_customer_info tCusinf;
    kl520_api_customer_get(&tCusinf);

    dbg_msg_api("tCusinf.nCusInfo0=%d",tCusinf.nCusInfo0);
    dbg_msg_api("tCusinf.nCusInfo1=%d",tCusinf.nCusInfo1);
    dbg_msg_api("tCusinf.nCusInfo2=%u",tCusinf.nCusInfo2);
    dbg_msg_api("tCusinf.nCusInfo3=%llu",tCusinf.nCusInfo3);

    return 0;
}



