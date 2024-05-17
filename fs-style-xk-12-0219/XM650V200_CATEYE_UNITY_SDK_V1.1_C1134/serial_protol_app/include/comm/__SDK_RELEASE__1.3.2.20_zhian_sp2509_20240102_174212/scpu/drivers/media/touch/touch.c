#include "board_kl520.h"
#ifdef CFG_SENSOR_TYPE
#include <cmsis_os2.h>
#include "kneron_mozart.h"
#include "clock.h"
#include "framework/framework_driver.h"
#include "framework/v2k_image.h"
#include "media/display/lcdc.h"
#include "media/display/video_renderer.h"
#include "dbg.h"
#include "board_ddr_table.h" // for internal used only
#include "kdp520_dma.h"
#include "kl520_include.h"

fn_power_hook m_cb_touch_power_on = NULL;
fn_power_hook m_cb_touch_power_off = NULL;

struct touch_driver *m_touch_driver = NULL;
extern struct touch_driver kdp520_touch_driver;
extern kl520_mouse_info kdp520_mouse_info;

int kl520_api_mouse_info_get(kl520_mouse_info * info)
{
    info->x = kdp520_mouse_info.x;
    info->y = kdp520_mouse_info.y;
    info->state = kdp520_mouse_info.state;
    return 0;
}

static void _board_assign_touch_device(void)
{
    m_touch_driver  = &kdp520_touch_driver;
}
static void _board_connect_touch_panel(void)
{
    if (m_touch_driver) {
        
#if (CFG_TOUCH_TYPE == TOUCH_TYPE_CT130)
        extern struct touch_panel_driver ct130_driver;
        m_touch_driver->attach_panel(m_touch_driver->core_dev, &ct130_driver);
#elif (CFG_TOUCH_TYPE == TOUCH_TYPE_FT5X06)
        extern struct touch_panel_driver ft5x06_driver;
        m_touch_driver->attach_panel(m_touch_driver->core_dev, &ft5x06_driver);
#elif (CFG_TOUCH_TYPE == TOUCH_TYPE_GT911)
        extern struct touch_panel_driver gt911_driver;
        m_touch_driver->attach_panel(m_touch_driver->core_dev, &gt911_driver);
#elif (CFG_TOUCH_TYPE == TOUCH_TYPE_CT328)
        extern struct touch_panel_driver ct328_driver;
        m_touch_driver->attach_panel(m_touch_driver->core_dev, &ct328_driver);
#endif        
        
        m_touch_driver->set_hook(m_touch_driver->core_dev, m_cb_touch_power_on, m_cb_touch_power_off);
    }
}

void kl520_api_touch_register_hook(fn_power_hook fn_power_on, fn_power_hook fn_power_off)
{
    m_cb_touch_power_on = fn_power_on;
    m_cb_touch_power_off = fn_power_off;
}

int kl520_api_touch_open(void)
{
    int ret = -1;

    if (NULL == m_touch_driver) {
        _board_assign_touch_device();
        _board_connect_touch_panel();
    }
    if (m_touch_driver) {
        //ret = m_touch_driver->probe(m_touch_driver->core_dev);
        if (0 == m_touch_driver->init(m_touch_driver->core_dev)) {
            //TODO : exception handler
            ret = 0;
        }
        else 
            ret = -1;
    }
    if(ret != 0)
        dbg_msg_err("[%s] error", __func__);
    return ret;
}

int kl520_api_touch_start(void)
{
    //dbg_msg_touch("m_touch_driver=%x", m_touch_driver);
    //dbg_msg_touch("m_touch_driver->start=%x", m_touch_driver->start);
    //dbg_msg_touch("m_touch_driver->core_dev=%x", m_touch_driver->core_dev);
    if (m_touch_driver)
        m_touch_driver->start(m_touch_driver->core_dev);

    return 0;
}

int kl520_api_touch_stop(void)
{
    if (m_touch_driver)
        m_touch_driver->stop(m_touch_driver->core_dev);

    return 0;
}

BOOL kl520_api_touch_is_started(void) 
{
    return ((m_touch_driver) ? (m_touch_driver->is_started(m_touch_driver->core_dev)) : (FALSE));
}

void kl520_api_touch_set_x_range_max(int x_range_max) {
    if (m_touch_driver)
        m_touch_driver->x_range_max = x_range_max;
}
void kl520_api_touch_set_y_range_max(int y_range_max) {
    if (m_touch_driver)
        m_touch_driver->y_range_max = y_range_max;
}
void kl520_api_touch_set_x_axis_inverse(int on_off) {
    if (m_touch_driver)
        m_touch_driver->inverse_x_axis = on_off;
}
void kl520_api_touch_set_y_axis_inverse(int on_off) {
    if (m_touch_driver)
        m_touch_driver->inverse_y_axis = on_off;
}

u16 kl520_api_touch_get_device_id(void)
{
	u16 nDeviceId;

#if ( CFG_TOUCH_ENABLE == YES )
	if ( m_touch_driver == NULL )
	{
		kl520_api_touch_open();
		nDeviceId = m_touch_driver->device_id;
		kl520_api_touch_stop();
	}
	else
	{
		nDeviceId = m_touch_driver->device_id;
	}
#else
	nDeviceId = 0xFFFF;
#endif

	return nDeviceId;
}

#endif
