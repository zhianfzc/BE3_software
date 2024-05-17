#ifndef __VIDEO_RENDERER_H__
#define __VIDEO_RENDERER_H__

#include "framework/v2k_image.h"

extern struct display_driver kdp520_virtual_dp_driver;
extern struct panel_driver virtual_dp_driver;
extern struct display_driver kdp520_lcdc_driver;
extern struct panel_driver mzt_480x272_driver;
extern struct display_driver kdp520_lcm_driver;
extern struct panel_driver st7789_240x320_driver;
extern struct display_driver kdp520_spi_lcd_driver;
extern struct panel_driver st7789v2_spi_240x320_driver;

void kdp_video_renderer_set_cam_idx(u8 idx);
void kdp_video_renderer_next_idx(void);
u8   kdp_video_renderer_get_idx(void);

struct video_input_params kdp_video_renderer_setting(u8 idx);
/**
 * @brief open a video renderer.
 * @param cam_idx,  choose which camera the image source comes from.
 * @param width,    video width
 * @param height,   video height
 * @return 0,       if kdp_video_renderer_open was successfully, or
 *         others,  if an error occurred
 */
//int kdp_video_renderer_open(unsigned int cam_idx, int width, int height);
int kdp_video_renderer_open(struct video_input_params *params);

int kdp_video_renderer_set_dp_res(struct video_input_params *params);


int kdp_video_engineering_switch(struct video_input_params *params);

/**
 * @brief choose which camera the image source comes from.
 * @param cam_idx,  camera device index
 * @return 0,       if kdp_video_renderer_set_camera was successfully, or
 *         others,  if an error occurred
 */
int kdp_video_renderer_set_camera(void);
/**
 * @brief start video renderer.
 * @return 0,       if kdp_video_renderer_start was successfully, or
 *         others,  if an error occurred
 */
int kdp_video_renderer_start(void);
/**
 * @brief stop video renderer.
 * @return 0,       if kdp_video_renderer_stop was successfully, or
 *         others,  if an error occurred
 */
int kdp_video_renderer_stop(void);

/**
 * @brief get the video parameter.
 * @p_params    output the parameter pointer
 * @return      0,       if kdp_video_renderer_get_params was successfully, or
 *              others,  if an error occurred
 */
int kdp_video_renderer_get_params(struct video_input_params *p_params);


extern bool kdp_video_renderer_assign_display_device(void);
extern void kdp_video_renderer_connect_display_panel(void);

extern struct display_driver *m_display_driver;

#endif
