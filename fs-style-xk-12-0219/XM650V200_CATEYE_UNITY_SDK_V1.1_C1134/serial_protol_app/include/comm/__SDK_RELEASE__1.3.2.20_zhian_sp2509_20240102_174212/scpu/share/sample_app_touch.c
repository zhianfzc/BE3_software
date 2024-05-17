#include "sample_app_touch.h"
#include <string.h>
#include "board_ddr_table.h"
#include "framework/event.h"
#include "framework/v2k_color.h"
#include "media/display/video_renderer.h"
#include "dbg.h"
#include "touch.h"
#include "kl520_api.h"

osThreadId_t tid_app_touchpanel = 0;

#if (CFG_PANEL_TYPE > PANEL_NULL)
#if CFG_UI_USR_IMG == NO
static void _api_touch_app_icon_handler(kl520_mouse_info * info)
{
    if (MOUSE_DOWN == info->state)
    {
        if ( (80 > info->x) && ((320-80) < info->y) ) {
            kl520_api_dp_set_pen_rgb565(GREEN, 2);
            kl520_api_dp_fill_rect(0, (320-45), 80, 45);
            kl520_api_dp_fresh();
        }
    }
    else
    {
        kl520_api_dp_draw_bitmap(0, (320-45), 80, 45, (void *)USR_DDR_IMG_02_ICON_01_ADDR);
        kl520_api_dp_fresh();
    }
}
#endif
#endif

#if (KL520_API_SHOW_TOUCH_LOG == YES)
static void _api_touch_app_print_data(kl520_mouse_info * info)
{
    dbg_msg_console("TOUCH STATE : %d, [%d,%d]", info->state, info->x, info->y);
    //dbg_msg("TOUCH X : %d", info->x);
    //dbg_msg("TOUCH Y : %d", info->y);
}
#endif
void _api_touch_app_thread(void *arg)
{
    u32 flags;
    while(1)
    {
        kl520_mouse_info info = {MOUSE_NONE, 0, 0};

        flags = wait_event(kl520_api_get_event(), KL520_APP_FLAG_TOUCH_ALL);
        clear_event(kl520_api_get_event(), KL520_APP_FLAG_TOUCH_ALL);
        if (flags & KL520_APP_FLAG_TOUCH_DEINIT) {
            //dbg_msg("touch panel : app thread, KL520_APP_FLAG_TOUCH_DEINIT");
            break;
        }

        //==============================================
        //customer application start here
        //==============================================
        if (flags & KL520_APP_FLAG_TOUCH) {
            kl520_api_mouse_info_get(&info);

        //example
#if (CFG_PANEL_TYPE > PANEL_NULL)
    #if CFG_UI_USR_IMG == NO
        if (g_touch_sample_app_enable_flag)
            _api_touch_app_icon_handler(&info);
    #endif
#endif
        //for printing data (for customer)
    #if (KL520_API_SHOW_TOUCH_LOG == YES)
        if (MOUSE_NONE != info.state)
            _api_touch_app_print_data(&info);
    #endif
        }

    }
    osThreadExit();
}

BOOL sample_app_touch_is_inited(void)
{
    return (NULL != tid_app_touchpanel);
}

void sample_app_init_touch_panel(void)
{
    kl520_api_touch_set_x_range_max(CFG_TOUCH_X_RANGE_MAX);
    kl520_api_touch_set_y_range_max(CFG_TOUCH_Y_RANGE_MAX);
    kl520_api_touch_set_x_axis_inverse(CFG_TOUCH_X_AXIS_INVERSE);
    kl520_api_touch_set_y_axis_inverse(CFG_TOUCH_Y_AXIS_INVERSE);

    {

        osThreadAttr_t attr = {
            .stack_size = 512,
            .attr_bits = osThreadJoinable
        };

        //dbg_msg("touch panel : app thread start");
        if (NULL == tid_app_touchpanel)
            tid_app_touchpanel = osThreadNew(_api_touch_app_thread, NULL, &attr);
    }
}

void sample_app_deinit_touch_panel(void)
{
    if (tid_app_touchpanel) {
        set_event(kl520_api_get_event(), KL520_APP_FLAG_TOUCH_DEINIT);
        osThreadJoin(tid_app_touchpanel);
        //dbg_msg("touch panel : app thread end");
        tid_app_touchpanel = 0;
    }  
}
