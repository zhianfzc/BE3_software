#include "board_kl520.h"
#if (DISPLAY_DEVICE == DISPLAY_DEVICE_UNKNOWN)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmsis_os2.h>
#include "kneron_mozart.h"
#include "base.h"
#include "io.h"
#include "clock.h"
#include "pinmux.h"
#include "scu_extreg.h"
#include "framework/init.h"
#include "framework/ioport.h"
#include "framework/framework_errno.h"
#include "framework/framework_driver.h"
#include "framework/v2k_image.h"
#include "framework/utils.h"
#include "media/display/virtual_dp.h"
#include "media/display/display.h"
#include "dbg.h"
#include "kdp_ddr_table.h"
#include "kl520_include.h"
#include "kdp_memory.h"

#define GET_RGB565(r, g, b) \
        (((((unsigned char)r)>>3)<<11) | ((((unsigned char)g)>>2)<<5) | (((unsigned char)b)>>3))
#define FRAME_SIZE_RGB(xres,yres,mbpp)  ((xres) * (yres) * (mbpp) / 8)
//#define TEST_DRAW_RECT

struct display_driver kdp520_virtual_dp_driver;

u32 _glcdc_cur_addr = 0;

struct pip_window
{
    int x;
    int y;
    int width;
    int height;
};

struct virtual_dp_pen_info
{
    u16 color;
    unsigned int width;
};

struct kdp520_virtual_dp_context {
    struct virtual_dp_pen_info pen_info;
    u32 dp_buffer_addr;
    struct core_device *camera_ctrller;
};

struct kdp520_virtual_dp_context virtual_dp_ctx_s;

static __inline int _kdp520_virtual_dp_draw_pixel(
        u32 frame_buffer, int x, int y, u32 width, u16 color)
{
    *(u16 *)(frame_buffer + ((y * width + x) << 1) + 0) = color; 
        return 0;
}

static void _kdp520_virtual_dp_alloc_framebuffer(struct kdp520_virtual_dp_context *context)
{
#define DP_BUF_SIZE     (DISPLAY_WIDTH*DISPLAY_HEIGHT*2)

    struct kdp520_virtual_dp_context *ctx = context;

    if ( _glcdc_cur_addr == 0 )
    {
        _glcdc_cur_addr = kdp_ddr_reserve(DP_BUF_SIZE);
    }

    ctx->dp_buffer_addr = _glcdc_cur_addr;
}

static int kdp520_virtual_dp_attach_panel(struct core_device** core_d, struct panel_driver *panel)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);

    display_drv->panel = panel;

    return 0;
}

static int kdp520_virtual_dp_set_params(struct core_device** core_d, struct video_input_params *params)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    struct video_input_params *p = &display_drv->vi_params;
    
    memcpy(p, params, sizeof(*params));
    display_drv->fb_size = calc_framesize(p->src_width, p->src_height, p->src_fmt);    
    dbg_msg_display("display_drv->fb_size=%d", display_drv->fb_size);  
    return 0;
}

static int kdp520_virtual_dp_get_params(struct core_device** core_d, struct video_input_params *params)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    struct video_input_params *p = &display_drv->vi_params;
    memcpy(params, p, sizeof(*params));    
    
    return 0;
}
    
static int kdp520_virtual_dp_set_camera(struct core_device** core_d)
{
    struct kdp520_virtual_dp_context *ctx = &virtual_dp_ctx_s;
    _kdp520_virtual_dp_alloc_framebuffer(ctx);

    return 0;    
}

static int kdp520_virtual_dp_init(struct core_device** core_d)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);

    if (display_drv->panel) {
        display_drv->display_id = display_drv->panel->read_did(core_d);
        dbg_msg_display(" display_id=%x", display_drv->display_id);
        display_drv->panel->init(core_d);
    }
    
    return 0;
}

static int kdp520_virtual_dp_start(struct core_device** core_d)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;
    
    dbg_msg_display("%s", __func__);
    
    return ret;
}

static int kdp520_virtual_dp_stop(struct core_device** core_d)
{
    dbg_msg_display("%s", __func__);

    return 0;//ret;
}

static int kdp520_virtual_dp_set_pen(struct core_device** core_d, u16 color, unsigned int width)
{
    struct kdp520_virtual_dp_context *ctx = &virtual_dp_ctx_s;

    ctx->pen_info.color = color;
    ctx->pen_info.width = width;
   
    return 0;//ret;
}

static int kdp520_virtual_dp_draw_rect(struct core_device** core_d, int org_x, int org_y, u32 width, u32 height)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;
    struct kdp520_virtual_dp_context *ctx = &virtual_dp_ctx_s;
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);

    {
        int x_pos, y_pos;
        int x_unit = 1;
        int y_unit = 1;
        int top = org_y;
        int left = org_x;
        int right = org_x + width;
        int bottom = top + height;
        
        u32 frame_width = display_drv->vi_params.dp_out_w;
        u32 frame_height =  display_drv->vi_params.dp_out_h;
        u32 _addr = _glcdc_cur_addr;
        
        u32 border_size = ctx->pen_info.width;
        int left_border = left + border_size;
        int right_border = right - border_size;
        int top_lower = top + border_size;
        int bottom_upper = bottom - border_size;  
        u16 color = ctx->pen_info.color;
        
        
        for (y_pos=(top>=0)?top:0; (y_pos < bottom) && (y_pos >= 0) && (y_pos < frame_height); y_pos += y_unit)
        {
            if ((y_pos >= top && y_pos < top_lower) || (y_pos >= bottom_upper))
            {
                for (x_pos=(left>=0)?left:0; (x_pos < right) && (x_pos < frame_width); x_pos += x_unit)
                {
                    _kdp520_virtual_dp_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
                }
            }
            else
            {
                for (x_pos=(left>=0)?left:0; (x_pos < left_border) && (x_pos < frame_width); x_pos += x_unit)
                {
                    _kdp520_virtual_dp_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
                }
       
                for (x_pos=(right_border>=0)?right_border:0; (x_pos < right) && (x_pos < frame_width); x_pos += x_unit)
                {
                    _kdp520_virtual_dp_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
                }
            }
        }

        ret = 0;
    } 
    
    return ret;
}
static int kdp520_virtual_dp_draw_line(struct core_device** core_d,
        u32 xs, u32 ys, u32 xe, u32 ye)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;
    struct kdp520_virtual_dp_context *ctx = &virtual_dp_ctx_s;
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);

//    if (core_d)
    {
        u32 x_pos, y_pos;
        u32 x_unit = 1;
        u32 y_unit = 1;
        u32 _addr = _glcdc_cur_addr;
        u32 frame_width = display_drv->vi_params.dp_out_w;
        u32 frame_height =  display_drv->vi_params.dp_out_h;
        u16 color = ctx->pen_info.color;
        u16 pen_width = ctx->pen_info.width;
        
        if(xs == xe)
        {
            if(ys > ye)
            {
                ys ^= ye;
                ye ^= ys;
                ys ^= ye;
            }
                        
            for(y_pos = ys; y_pos <= ye; y_pos += y_unit)
            {
                for(x_pos = xs; (x_pos < xs+pen_width) && (x_pos < frame_width); x_pos += x_unit)
                {
                    _kdp520_virtual_dp_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
                }
            }
            ret = 0;
        }
        else if(ys == ye)
        {
            if(xs > xe)
            {
                xs ^= xe;
                xe ^= xs;
                xs ^= xe;
            }

            for(x_pos = xs; x_pos <= xe; x_pos += x_unit)
            {
                for(y_pos = ys; (y_pos < ys+pen_width) && (y_pos < frame_height); y_pos += y_unit)
                {
                    _kdp520_virtual_dp_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
                }
            }
            ret = 0;
        }
        else
        {
            //s16 y_min = MIN(ys,ye);	
            //s16 y_max = MAX(ys,ye);
            s16 x_min = MIN(xs,xe);	
            s16 x_max = MAX(xs,xe);
            s16 y_step = ((s16)ye-(s16)ys)/(x_max-x_min);			
            s16 x = 0,y = 0,pen=0;
            u16* buffer = (u16*)_addr;

            //dbg_msg_display("[xxxxxx]%d-%d-%d-%d,%d-%d-%d-%d,%d",xs,ys,xe,ye,x_min,x_max,y_min,y_max,y_step );
            if (xs < xe)
            {
                for(x = xs,y = ys; x <= xe; x++, y+=y_step) {
                    *(buffer + y * frame_width + x) = color;	
                    for (pen = 1; pen <= pen_width; pen++)
                        *(buffer + (y+pen) * frame_width + x) = color;
                    }
            }
            else
            {
                for(x = xs,	  y = ys; x >= xe; x--, y+=y_step) {
                    *(buffer + y * frame_width + x) = color;			 
                    for (pen = 1; pen <= pen_width; pen++)
                        *(buffer + (y+pen) * frame_width + x) = color;			
                    }
            }
        }
    } 
    
    return ret;
}

static int kdp520_virtual_dp_fill_rect(struct core_device** core_d,
        u32 org_x, u32 org_y, u32 width, u32 height)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;
    struct kdp520_virtual_dp_context *ctx = &virtual_dp_ctx_s;
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);

//    if (core_d)
    {
        u32 x_pos, y_pos;
        u16 color = ctx->pen_info.color;

        u32 frame_width = display_drv->vi_params.dp_out_w;
        u32 frame_height =  display_drv->vi_params.dp_out_h;
        u32 _addr = _glcdc_cur_addr;

        if((org_x >= frame_width)
        || (org_y >= frame_height))
//        || ((org_x + width) > frame_width)
//        || ((org_y + height) > frame_height))
        {
            return ret;
        }
        
        if (org_x + width > frame_width)
            width = frame_width - org_x;
        if (org_y + height > frame_height)
            height = frame_height - org_y;

        for(y_pos=org_y; y_pos<(org_y+height); y_pos++)
        {
            for(x_pos=org_x; x_pos<(org_x+width); x_pos++)
            {
                _kdp520_virtual_dp_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
            }
        }
    } 
    
    return ret;
}

static int kdp520_virtual_dp_draw_bitmap(struct core_device** core_d,
        u32 org_x, u32 org_y, u32 width, u32 height, void* pBuf)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    
    {
        u16 y_pos, x_pos, tmp;

        u32 frame_width = display_drv->vi_params.dp_out_w;
        u32 frame_height =  display_drv->vi_params.dp_out_h;
        u32 _addr = _glcdc_cur_addr;
        
        u8 *p = pBuf;
        u16 * lcd_buffer = (u16 *)_addr;

        if((org_x >= frame_width)
        || (org_y >= frame_height)
        || ((org_x + width) > frame_width)
        || ((org_y + height) > frame_height))
        {
            return ret;
        }

        for(y_pos=org_y; y_pos<(org_y+height); y_pos++)
        {
            for (x_pos = org_x; x_pos < (org_x+width); x_pos++)
            {
                tmp = (*p++);
                tmp |= (*p++)<< 8;
                if (tmp == 0)
                    continue;

                *(lcd_buffer + (y_pos * frame_width + x_pos)) = tmp;
            }
        }
    } 
    
    return ret;
}

int kdp520_virtual_dp_fresh(struct core_device** core_d)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;   
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    if (NULL == display_drv)
    {
        dbg_msg_display("Not load display_driver yet.");
        return -1;
    }
    
    return ret;   
}

int kdp520_virtual_dp_set_source(struct core_device** core_d, u32 src_addr, int src_dev_idx)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    if (NULL == display_drv) {
        dbg_msg_display("Not load display_driver yet.");
        return -1;
    }

    struct panel_driver *panel = display_drv->panel;

    u32 src_cam_idx = display_drv->vi_params.src_cam_idx;

    if(src_cam_idx == MIPI_CAM_RGB) {
        panel->preproc_rgb(core_d, src_addr, _glcdc_cur_addr);
    }
    else if (src_cam_idx == MIPI_CAM_NIR) {
        panel->preproc_nir(core_d, src_addr, _glcdc_cur_addr);//pressing to display addr
    }


    return 0;
}

u32 kdp520_virtual_dp_get_buffer_addr(struct core_device**core_d)
{
    int ret = 0;
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    if (NULL == display_drv)
    {
        dbg_msg_display("Not load display_driver yet.");
        return ret;
    }

    return _glcdc_cur_addr;
}

struct display_driver kdp520_virtual_dp_driver = {
    .set_params   = kdp520_virtual_dp_set_params,
    .get_params   = kdp520_virtual_dp_get_params,
    .set_camera   = kdp520_virtual_dp_set_camera,
    .get_buffer   = kdp520_virtual_dp_get_buffer_addr,
    .init         = kdp520_virtual_dp_init,
    .attach_panel = kdp520_virtual_dp_attach_panel,
    .start        = kdp520_virtual_dp_start,
    .stop         = kdp520_virtual_dp_stop,
    .set_source   = kdp520_virtual_dp_set_source,
    .set_pen      = kdp520_virtual_dp_set_pen,
    .draw_rect    = kdp520_virtual_dp_draw_rect,
    .draw_line    = kdp520_virtual_dp_draw_line,
    .fill_rect    = kdp520_virtual_dp_fill_rect,
    .draw_bitmap  = kdp520_virtual_dp_draw_bitmap,
    .fresh        = kdp520_virtual_dp_fresh
};

#endif

