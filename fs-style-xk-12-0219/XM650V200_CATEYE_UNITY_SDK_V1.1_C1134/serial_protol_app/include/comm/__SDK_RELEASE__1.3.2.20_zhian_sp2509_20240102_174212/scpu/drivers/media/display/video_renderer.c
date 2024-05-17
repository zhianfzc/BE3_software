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
#include "media/display/lcm.h"


struct display_driver *m_display_driver = NULL;


void kdp_video_renderer_get_cur_display_driver(struct display_driver** pp_driver) {
    *pp_driver = m_display_driver;
}

bool kdp_video_renderer_assign_display_device(void)
{
#if DISPLAY_DEVICE == DISPLAY_DEVICE_UNKNOWN
//    m_display_driver  = NULL;
    m_display_driver  = &kdp520_virtual_dp_driver;
#elif DISPLAY_DEVICE == DISPLAY_DEVICE_LCDC
    m_display_driver  = &kdp520_lcdc_driver;
#elif DISPLAY_DEVICE == DISPLAY_DEVICE_LCM
    m_display_driver  = &kdp520_lcm_driver;
#elif DISPLAY_DEVICE == DISPLAY_DEVICE_SPI_LCD
    m_display_driver  = &kdp520_spi_lcd_driver;
#endif

    return (m_display_driver != NULL);
}
void kdp_video_renderer_reassign_display_device(u8 device_type)
{
#if (CFG_PANEL_TYPE==PANEL_ST7789_240X320_8080_AND_SPI)
    switch (device_type) {
        case DISPLAY_DEVICE_LCM:
            if (m_display_driver) m_display_driver->remove(&m_display_driver->core_dev);
            m_display_driver = &kdp520_lcm_driver;
            break;
        case DISPLAY_DEVICE_SPI_LCD:
            if (m_display_driver) m_display_driver->remove(&m_display_driver->core_dev);
            m_display_driver = &kdp520_spi_lcd_driver;
            break;
        default:
            break;
    }
#endif
}

static u8 renderer_cam_idx = 0;  // MIPI_CAM_RGB is no longer a constant, but a variable.

void kdp_video_renderer_connect_display_panel(void)
{
    if (m_display_driver) {
    #if CFG_PANEL_TYPE == PANEL_NULL
        m_display_driver->attach_panel(&m_display_driver->core_dev, &virtual_dp_driver);
    #elif CFG_PANEL_TYPE == PANEL_MZT_480X272
        m_display_driver->attach_panel(&m_display_driver->core_dev, &mzt_480x272_driver);
    #elif (CFG_PANEL_TYPE == PANEL_ST7789_240X320) ||\
        (CFG_PANEL_TYPE == PANEL_ST7789_320X240) ||\
        (CFG_PANEL_TYPE==PANEL_ST7789_240X320_8080_AND_SPI)
        m_display_driver->attach_panel(&m_display_driver->core_dev, &st7789_240x320_driver);
    #elif (CFG_PANEL_TYPE == PANEL_ST7789_240X320_SPI)
        m_display_driver->attach_panel(&m_display_driver->core_dev, &st7789v2_spi_240x320_driver);
    #endif
    }
}
void kdp_video_renderer_reconnect_display_panel(u8 display_type)
{
#if (CFG_PANEL_TYPE==PANEL_ST7789_240X320_8080_AND_SPI)
    if (m_display_driver) {
        if (DISPLAY_DEVICE_LCM == display_type)
            m_display_driver->attach_panel(&m_display_driver->core_dev, &st7789_240x320_driver);
        else if (DISPLAY_DEVICE_SPI_LCD == display_type)
            m_display_driver->attach_panel(&m_display_driver->core_dev, &st7789v2_spi_240x320_driver);
    }
#endif
}

int kdp_video_engineering_switch(struct video_input_params *params)
{
    int ret = -1;

    if (NULL == m_display_driver) {
        if(kdp_video_renderer_assign_display_device())
        {
            kdp_video_renderer_connect_display_panel();
        }
    }

    if (m_display_driver) {
        if ((0 == m_display_driver->set_params(&m_display_driver->core_dev, params))) {
            ret = 0;
        }
    } 
    #if CFG_PANEL_TYPE == PANEL_MZT_480X272 
    if (m_display_driver) {
        if ((0 == m_display_driver->init(&m_display_driver->core_dev))) {
            ret = 0;
        }
    }
    #endif
    
    
    return ret;
}

void kdp_video_renderer_set_cam_idx(u8 idx)
{
    renderer_cam_idx = idx;
}

void kdp_video_renderer_next_idx(void)
{
    renderer_cam_idx = kdp_video_renderer_get_idx();
    renderer_cam_idx++;
    renderer_cam_idx = renderer_cam_idx%IMGSRC_NUM;
}

u8 kdp_video_renderer_get_idx(void)
{
    return renderer_cam_idx;
}

struct video_input_params kdp_video_renderer_setting(u8 idx)
{
    struct video_input_params params;

    if (CFR_CAM_RGB == idx)
    {
        params.src_fmt = V2K_PIX_FMT_RGB565;
        params.src_type = V2K_TYPE_DYNAMIC;
        params.src_cam_idx = MIPI_CAM_RGB;
        params.src_width = RGB_IMG_SOURCE_W;
        params.src_height = RGB_IMG_SOURCE_H;

        params.dp_area_w = DISPLAY_RGB_WIDTH;
        params.dp_area_h = DISPLAY_RGB_HEIGHT;
        params.dp_area_x = 0;
        params.dp_area_y = 0;
        params.dp_out_w = DISPLAY_RGB_OUT_WIDTH;
        params.dp_out_h = DISPLAY_RGB_OUT_HEIGHT;

        params.panel_in_w = PANEL_IN_WIDTH;
        params.panel_in_h = PANEL_IN_HEIGHT;
        params.panel_out_w = PANEL_WIDTH;
        params.panel_out_h = PANEL_HEIGHT;
    }
    else if (CFR_CAM_NIR == idx)
    {
#if CFG_PANEL_TYPE == PANEL_MZT_480X272
        params.src_fmt = V2K_PIX_FMT_RGB565; // If source format use RAW8, display and UI need modifying.
#else
        params.src_fmt = V2K_PIX_FMT_RAW8;
#endif
        params.src_type = V2K_TYPE_DYNAMIC;
        params.src_cam_idx = MIPI_CAM_NIR;
        params.src_width = NIR_IMG_SOURCE_W;
        params.src_height = NIR_IMG_SOURCE_H;

        params.dp_area_w = DISPLAY_NIR_WIDTH;
        params.dp_area_h = DISPLAY_NIR_HEIGHT;
        params.dp_area_x = 0;
        params.dp_area_y = 0;
        params.dp_out_w = DISPLAY_NIR_OUT_WIDTH;
        params.dp_out_h = DISPLAY_NIR_OUT_HEIGHT;

        params.panel_in_w = PANEL_IN_WIDTH;
        params.panel_in_h = PANEL_IN_HEIGHT;
        params.panel_out_w = PANEL_WIDTH;
        params.panel_out_h = PANEL_HEIGHT;
    }

    return params;
}

int kdp_video_renderer_open(struct video_input_params *params)
{
    int ret = -1;

    if (NULL == m_display_driver) {
        kdp_video_renderer_assign_display_device();
        kdp_video_renderer_connect_display_panel();
    }
    
    if (m_display_driver) {
        if ((0 == m_display_driver->set_params(&m_display_driver->core_dev, params)) &&
            (0 == m_display_driver->init(&m_display_driver->core_dev)) &&
            (0 == m_display_driver->set_camera(&m_display_driver->core_dev))) {
            ret = 0;
        }
        else
        {
            m_display_driver = NULL;
        }
        
    }

    return ret;
}

int kdp_video_renderer_set_dp_res(struct video_input_params *params)
{
    if (m_display_driver)
    {
        m_display_driver->set_params(&m_display_driver->core_dev, params);
    }

    return 0;
}

//u16 kdp_video_renderer_get_yres(void)
//{
//    struct video_input_params params;
//   
//    if (m_display_driver){
//        m_display_driver->get_params(m_display_driver->core_dev, &params);
//    }
//    return (u16)(params.input_yres); 
//}

int kdp_video_renderer_set_camera(void)
{
    return m_display_driver->set_camera(&m_display_driver->core_dev);
}

int kdp_video_renderer_start(void)
{
//    dbg_msg_display("m_display_driver=%x", m_display_driver);
//    dbg_msg_display("m_display_driver->start=%x", m_display_driver->start);
//    dbg_msg_display("m_display_driver->core_dev=%x", m_display_driver->core_dev);
    if (m_display_driver)
        m_display_driver->start(&m_display_driver->core_dev);

    return 0;
}

int kdp_video_renderer_stop(void)
{
    if (m_display_driver)
        m_display_driver->stop(&m_display_driver->core_dev);

    return 0;
}

int kdp_video_renderer_get_params(struct video_input_params *p_params)
{
    if (m_display_driver){
        m_display_driver->get_params(&m_display_driver->core_dev, p_params);
        return 0;
    }
    else
        return -1;
}
#endif
