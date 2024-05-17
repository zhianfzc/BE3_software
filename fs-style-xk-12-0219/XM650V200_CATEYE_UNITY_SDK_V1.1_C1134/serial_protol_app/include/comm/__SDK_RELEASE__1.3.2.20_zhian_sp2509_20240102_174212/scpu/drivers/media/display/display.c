#include "board_kl520.h"
#include <cmsis_os2.h>
#include "kneron_mozart.h"
#include "clock.h"
#include "framework/framework_driver.h"
#include "framework/v2k_image.h"
#include "media/display/lcdc.h"
#include "media/display/display.h"
#include "dbg.h"

extern struct display_driver *m_display_driver;

u32 kdp_display_get_buffer_addr(void)
{
    if (m_display_driver)
    {
        return m_display_driver->get_buffer(&m_display_driver->core_dev);
    }

    return 0;
}

int kdp_display_set_source(u32 src_addr, int src_dev_idx)
{
    if (m_display_driver)
    {
        return m_display_driver->set_source(&m_display_driver->core_dev, src_addr, src_dev_idx);
    }
    else
    {
        return (-1);
    }
}

int kdp_display_set_pen_rgb565(unsigned short color, unsigned int pen_width)
{
    if (m_display_driver)
    {
        m_display_driver->set_pen(&m_display_driver->core_dev, color, pen_width);
    }
    else
    {
        return (-1);
    }

    return 0;
}

int kdp_display_draw_line(u32 xs, u32 ys, u32 xe, u32 ye)
{
    if (m_display_driver)
    {
        m_display_driver->draw_line(&m_display_driver->core_dev, xs, ys, xe, ye);
    }
    else
    {
        return (-1);
    }

    return 0;
}

int kdp_display_draw_bitmap(u32 org_x, u32 org_y, u32 width, u32 height, void *buf)
{
    if (m_display_driver)
    {
        m_display_driver->draw_bitmap(&m_display_driver->core_dev, org_x, org_y, width, height, buf);
    }
    else
    {
        return (-1);
    }
    
    return 0;
}

int kdp_display_draw_rect(int x, int y, int width, int height)
{
    if (m_display_driver)
    {
        m_display_driver->draw_rect(&m_display_driver->core_dev, x, y, width, height);
    }
    else
    {
        return (-1);
    }
    
    return 0;
}

int kdp_display_fill_rect(u32 org_x, u32 org_y, u32 width, u32 height)
{
    if (m_display_driver)
    {
        m_display_driver->fill_rect(&m_display_driver->core_dev, org_x, org_y, width, height);
    }
    else
    {
        return (-1);
    }
    
    return 0;
}

int kdp_display_get_device_id(void)
{
    if (m_display_driver)
    {
        return (int)m_display_driver->display_id;
    }
    else
    {
        return (int)0xFFFF;
    }
}

int kdp_display_fresh(void)
{
    if (m_display_driver)
    {
        return m_display_driver->fresh(&m_display_driver->core_dev);
    }
    else
    {
        return (-1);
    }
}
