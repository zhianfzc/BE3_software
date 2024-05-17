#include "board_kl520.h"
#if (DISPLAY_DEVICE==DISPLAY_DEVICE_LCM) || (DISPLAY_DEVICE==DISPLAY_DEVICE_SPI_LCD) || (DISPLAY_DEVICE==DISPLAY_DEVICE_LCM_AND_SPI_LCD)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cmsis_os2.h>
#include "board_ddr_table.h"
#include "kneron_mozart.h"
#include "base.h"
#include "io.h"
#include "clock.h"
#include "pinmux.h"
#include "scu_extreg.h"
#include "framework/event.h"
#include "framework/init.h"
#include "framework/ioport.h"
#include "framework/framework_errno.h"
#include "framework/framework_driver.h"
#include "framework/mutex.h"
#include "framework/v2k_color.h"
#include "framework/v2k_image.h"
#include "media/display/lcm.h"
#include "media/display/display.h"
#include "media/display/video_renderer.h"
#include "kdp520_gpio.h"
#include "kdp520_pwm_timer.h"
#include "kl520_include.h"
#include "delay.h"
#include "dbg.h"
#include "user_io.h"
#include "kl520_api_ssp.h"
#include "utils.h"
#include "kdp520_lcm.h"


//static u32 _prof_start, _prof_end;
//static u32 _prof_offset, _prof_total = 0, _prof_frame_cnt = 0;
#define LCM_PROFILE_ENABLE
#ifdef LCM_PROFILE_ENABLE 
#define LCM_PROFILE_START() _prof_start = osKernelGetTickCount(); 
#define LCM_PROFILE_STOP() { \
                _prof_end = osKernelGetTickCount(); \
                _prof_offset = _prof_end - _prof_start; \
                _prof_total += _prof_offset; \
                ++_prof_frame_cnt; \
                /*dbg_msg_display("[%s] offset=%u average=%u", __func__, _prof_offset, _prof_total / _prof_frame_cnt);*/ \
                }
#else
#define LCM_PROFILE_START() 
#define LCM_PROFILE_STOP() 
#endif


#define LCM_BASE                SLCD_FTLCDC210_PA_BASE
#define FLAGS_LCM_START_EVT     0x0020
#define FLAGS_LCM_STOP_EVT      0x0040
#define FLAGS_LCM_QUIT_EVT      0x0008

#define LCM_OPS_WAIT_TIMEOUT_CNT    (1000)

enum lcm_signal_cycle {
    LCM_SIGNAL_WIDTH_LC_PCLK_X1  = 0x00,
    LCM_SIGNAL_WIDTH_LC_PCLK_X2  = 0x01,
    LCM_SIGNAL_WIDTH_LC_PCLK_X3  = 0x02,
    LCM_SIGNAL_WIDTH_LC_PCLK_X4  = 0x03,
    LCM_SIGNAL_WIDTH_LC_PCLK_X5  = 0x04,
    LCM_SIGNAL_WIDTH_LC_PCLK_X6  = 0x05,
    LCM_SIGNAL_WIDTH_LC_PCLK_X7  = 0x06,
    LCM_SIGNAL_WIDTH_LC_PCLK_X8  = 0x07,
    LCM_SIGNAL_WIDTH_LC_PCLK_X9  = 0x08,
    LCM_SIGNAL_WIDTH_LC_PCLK_X10 = 0x09,
    LCM_SIGNAL_WIDTH_LC_PCLK_X11 = 0x0A,
    LCM_SIGNAL_WIDTH_LC_PCLK_X12 = 0x0B,
    LCM_SIGNAL_WIDTH_LC_PCLK_X13 = 0x0C,
    LCM_SIGNAL_WIDTH_LC_PCLK_X14 = 0x0D,
    LCM_SIGNAL_WIDTH_LC_PCLK_X15 = 0x0E,
    LCM_SIGNAL_WIDTH_LC_PCLK_X16 = 0x0F,
};

enum lcm_16bitmode_selection {
    LCM_16BITMODE_ONESHOT   = 0x00,
    LCM_16BITMODE_16_AT_MSB = 0x01,
    LCM_16BITMODE_16_AT_LSB = 0x02,
};

enum lcm_data_bus_type {
    LCM_DATA_BUS_8_BITS  = 0x00,
    LCM_DATA_BUS_9_BITS  = 0x01,
    LCM_DATA_BUS_16_BITS = 0x02,
    LCM_DATA_BUS_18_BITS = 0x03,
};

enum lcm_panel_interface_type {
    LCM_PANEL_INTEFACE_MONO    = 0x00,
    LCM_PANEL_INTEFACE_16_BITS = 0x01,
    LCM_PANEL_INTEFACE_18_BITS = 0x02,
};

enum lcm_mcu_interface_mode {
    LCM_MCU_INTERFACE_8080 = 0x00,
    LCM_MCU_INTERFACE_6800 = 0x01,
};

struct pip_window
{
    int x;
    int y;
    int width;
    int height;
};

struct lcm_pen_info
{
    u16 color;
    // u8 r;
    // u8 g;
    // u8 b;
    unsigned int width;
};

struct kdp520_lcm_context {
    u32 base;
    struct lcm_pen_info pen_info;
    u32 frame_buffer_addr;
    u32 dp_buffer_addr;
    struct core_device *camera_ctrller;
};

struct kdp520_lcm_context lcm_ctx_s;

enum lcm_state {
    LCM_STATE_IDLE = 0,
    LCM_STATE_INITED,
    LCM_STATE_STOPPED,
    LCM_STATE_PROBED,
    LCM_STATE_STARTED,
};


struct display_driver kdp520_lcm_driver;
struct mutex display_reg_lock;

enum lcm_state m_lcm_state;
//static osThreadId_t m_tid_lcm;

extern void delay_ms(unsigned int count);
extern void kdp_video_renderer_get_cur_display_driver(struct display_driver** pp_driver);


void kdp520_lcm_set_cs(u8 lv) {
    struct display_driver* display_drv;
    kdp_video_renderer_get_cur_display_driver(&display_drv);
    u32 pin_cs = display_drv->custom_pinmux->pin_cs;
    if (GPIO_NULL == pin_cs) return;

    if (!lv)
        kdp520_gpio_cleardata( 1<<pin_cs);
    else
        kdp520_gpio_setdata( 1<<pin_cs);
}
void kdp520_lcm_set_rs(u8 lv) {
    struct display_driver* display_drv;
    kdp_video_renderer_get_cur_display_driver(&display_drv);
    u32 pin_rs = display_drv->custom_pinmux->pin_rs;
    if (GPIO_NULL == pin_rs) return;

    if (!lv)
        kdp520_gpio_cleardata( 1<<pin_rs);
    else
        kdp520_gpio_setdata( 1<<pin_rs);
}
void kdp520_lcm_set_rst(u8 lv) {
    struct display_driver* display_drv;
    kdp_video_renderer_get_cur_display_driver(&display_drv);
    u32 pin_rst = display_drv->custom_pinmux->pin_rst;
    if (GPIO_NULL == pin_rst) return;

    if (!lv)
        kdp520_gpio_cleardata( 1<<pin_rst);
    else
        kdp520_gpio_setdata( 1<<pin_rst);
}
int kdp520_lcm_get_te_pin(void) {
    struct display_driver* display_drv;
    kdp_video_renderer_get_cur_display_driver(&display_drv);
    return display_drv->custom_pinmux->pin_te;
}

static void _lcm_hw_reset(void)
{
    kdp520_lcm_set_rs(1);
    kdp520_lcm_set_rst(0);
    delay_ms(5);
    kdp520_lcm_set_rst(1);
    kdp520_lcm_set_rs(0);
}

static void _lcm_wait_ready(u32 base)
{
    //dbg_msg_display("[%s]", __func__);
    unsigned int rdata;
    unsigned int timeout_cnt = 0;
    
    while((rdata & LCM_REG_RDY_READY_FOR_ACCESS) != LCM_REG_RDY_READY_FOR_ACCESS)
    { 
        rdata = inw(LCM_REG_RDY_P(base));
        timeout_cnt++;
        if (LCM_OPS_WAIT_TIMEOUT_CNT <= timeout_cnt)
        {
            dbg_msg_display("_lcm_wait_ready timeout : %d\n", timeout_cnt);
            break;
        }
    }
}

static void _lcm_write_cmd(u32 base, unsigned char data)
{
    mutex_lock(&display_reg_lock);
    kdp520_lcm_set_cs(0);
    _lcm_wait_ready(base);
    outb(LCM_REG_CMD_P(base), data);
    kdp520_lcm_set_cs(1);
    mutex_unlock(&display_reg_lock);
}

static void _lcm_write_data(u32 base, unsigned char data)
{
    mutex_lock(&display_reg_lock);
    kdp520_lcm_set_cs(0);
    _lcm_wait_ready(base);    
    outb(LCM_REG_DATA_P(base), data);
    kdp520_lcm_set_cs(1);
    mutex_unlock(&display_reg_lock);
}

unsigned int _lcm_read_data(u32 base)
{
    mutex_lock(&display_reg_lock);
    kdp520_lcm_set_cs(0);
    _lcm_wait_ready(base);
    outb(LCM_REG_RS_P(base), LCM_REG_RS_DMYRD_RS);
    _lcm_wait_ready(base);
    kdp520_lcm_set_cs(1);
    mutex_unlock(&display_reg_lock);

    return inw(LCM_REG_DATA_P(base));
}

static void _lcm_set_16bitmode_selection(enum lcm_16bitmode_selection sel)
{
    LCM_REG_OP_MODE_SET_16bit_mode(sel);
}

static void _lcm_set_data_bus_type(enum lcm_data_bus_type type) 
{
    LCM_REG_OP_MODE_SET_Bus_IF(type);
}

static void _lcm_set_panel_interface_type(enum lcm_panel_interface_type type) 
{
    LCM_REG_OP_MODE_SET_Panel_IF(type);
}

static void _lcm_set_mcu_interface_mode(enum lcm_mcu_interface_mode mode) 
{
    LCM_REG_OP_MODE_SET_C68(mode);
}

static void _lcm_set_pwh_r(enum lcm_signal_cycle cycle)
{
    //Tpwh width for the read cycles
    LCM_REG_TIMING_SET_Tpwh_r((u32)cycle);
}

static void _lcm_set_as(enum lcm_signal_cycle cycle)
{
    LCM_REG_TIMING_SET_Tas((u32)cycle); 
}

static void _lcm_set_ah(enum lcm_signal_cycle cycle)
{
    LCM_REG_TIMING_SET_Tah((u32)cycle); 
}

static void _lcm_set_pwl(enum lcm_signal_cycle cycle)
{
    LCM_REG_TIMING_SET_Tpwl((u32)cycle); 
}

static void _lcm_set_pwh_w(enum lcm_signal_cycle cycle)
{
    //Tpwh width for the write cycles
    LCM_REG_TIMING_SET_Tpwh_w((u32)cycle); 
}

static void _lcm_alloc_framebuffer(
        struct kdp520_lcm_context *context)
{
    struct kdp520_lcm_context *ctx = context;
	ctx->dp_buffer_addr    = KDP_DDR_DRV_LCM_START_ADDR;
}

static __inline int _kdp520_lcm_draw_pixel(
        u32 frame_buffer, int x, int y, u32 width, u16 color)
{
    *(u16 *)(frame_buffer + ((y * width + x) << 1) + 0) = color; 
    return 0;
}

static int kdp520_lcm_probe(struct core_device** core_d)
{
    struct kdp520_lcm_context *ctx = &lcm_ctx_s;
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);

    dbg_msg_display("[%s]", __func__);

    display_drv->base = SLCD_FTLCDC210_PA_BASE;
    ctx->base = SLCD_FTLCDC210_PA_BASE;
    m_lcm_state = LCM_STATE_PROBED;

    return 0;
}

static int kdp520_lcm_remove(struct core_device** core_d)
{
    m_lcm_state = LCM_STATE_IDLE;

    //set_thread_event(m_tid_lcm, FLAGS_LCM_QUIT_EVT);

    return 0;
}

/* reserved functon
static int kdp520_lcm_reset(struct core_device** core_d)
{
    return 0;
}
*/

static int kdp520_lcm_attach_panel(struct core_device** core_d, struct panel_driver *panel)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);

    display_drv->panel = panel;

    if (m_lcm_state == LCM_STATE_IDLE) {
        kdp520_lcm_probe(core_d);
        //delay_us(150000);
    }

    return 0;
}

static int kdp520_lcm_set_params(struct core_device** core_d, struct video_input_params *params)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    struct video_input_params *p = &display_drv->vi_params;
    
    memcpy(p, params, sizeof(*params));

    display_drv->fb_size = calc_framesize(p->src_width, p->src_height, p->src_fmt);
//    dbg_msg_display("[%s] video_input_params->input_xres=%u", __func__, p->input_xres);
//    dbg_msg_display("[%s] video_input_params->input_yres=%u", __func__, p->input_yres);
//    dbg_msg_display("[%s] video_input_params->input_fmt=%x", __func__, p->input_fmt);  
//    dbg_msg_display("[%s] display_driver->fb_size=%d", __func__, display_drv->fb_size);

    return 0;
}

static int kdp520_lcm_get_params(struct core_device** core_d, struct video_input_params *params)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    struct video_input_params *p = &display_drv->vi_params;
    
    //memcpy(p, params, sizeof(*params));
    memcpy(params, p, sizeof(*params));
    
    return 0;
}

static int kdp520_lcm_set_camera(struct core_device** core_d)
{
    struct kdp520_lcm_context *ctx = &lcm_ctx_s;

    _lcm_alloc_framebuffer(ctx);

    return 0;
}

u32 kdp520_lcm_get_frame_buffer(struct core_device** core_d)
{
    struct kdp520_lcm_context *ctx = &lcm_ctx_s;

    return ctx->frame_buffer_addr;
}

static int kdp520_lcm_init(struct core_device** core_d)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    s32 ret = 0;
    static u8 flag_lcm = 0;
    
    if(0 == flag_lcm)
    {
        SCU_EXTREG_SWRST_MASK1_SET_lcm_reset_n(1);
        SCU_EXTREG_MISC_SET_lcm_cken(1);
        
        _lcm_set_as(LCM_SIGNAL_WIDTH_LC_PCLK_X2);
        _lcm_set_ah(LCM_SIGNAL_WIDTH_LC_PCLK_X2);
        _lcm_set_pwl(LCM_SIGNAL_WIDTH_LC_PCLK_X1);
        _lcm_set_pwh_r(LCM_SIGNAL_WIDTH_LC_PCLK_X4);
        _lcm_set_pwh_w(LCM_SIGNAL_WIDTH_LC_PCLK_X2);

        _lcm_set_16bitmode_selection(LCM_16BITMODE_ONESHOT);
        _lcm_set_data_bus_type(LCM_DATA_BUS_8_BITS);
        
        _lcm_set_panel_interface_type(LCM_PANEL_INTEFACE_MONO);
        _lcm_set_mcu_interface_mode(LCM_MCU_INTERFACE_8080);

        // reset lcm
        u32 pin_rst = display_drv->custom_pinmux->pin_rst;
        if (GPIO_NULL != pin_rst)
            _lcm_hw_reset();
        else {
            LCM_REG_ENABLE_SET_RST_SET(); // power on reset
            delay_us(5000);
            //delay_ms(50);
            LCM_REG_ENABLE_SET_RST_CLR();
        }
        LCM_REG_ENABLE_SET_LCM_En(1);
        LCM_REG_ENABLE_SET_BLCTRL_SET(1);
        
        delay_us(1);

        // read display ID
        if ((display_drv) && (display_drv->panel))
        {
            display_drv->display_id = display_drv->panel->read_did(core_d);
            ret = display_drv->panel->init(core_d);
            
            if(display_drv->display_id == 0xFFFF || (ret!=0))
            {
                flag_lcm = 1;
                dbg_msg_err("[%s] error", __func__);
                return -1;
            }
        }

        //change to probed
        m_lcm_state = LCM_STATE_INITED;
    }
    else
    {
        return -1;
    }
    
    return 0;
}

static int kdp520_lcm_power(struct core_device* core_d, BOOL on)
{
    return 0;
}

static int kdp520_lcm_start(struct core_device** core_d)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);

    //dbg_msg_display("[%s]", __func__);
    //dbg_msg_display("display_drv=%x", display_drv);
    //dbg_msg_display("display_drv->panel=%x", display_drv->panel);
    
    if ((display_drv) && (display_drv->panel))
    {
        display_drv->panel->start(core_d);
        
        m_lcm_state = LCM_STATE_STARTED;
        ret = 0;
    }

    return ret;
}

static int kdp520_lcm_stop(struct core_device** core_d)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;
    struct display_driver* display_drv = container_of(core_d, struct display_driver, core_dev);

    //dbg_msg_display("[%s]", __func__);
    
    if ((display_drv) && (display_drv->panel))
    {
        display_drv->panel->stop(core_d);

        LCM_REG_ENABLE_SET_BLCTRL_SET(0);
        LCM_REG_ENABLE_SET_LCM_En(0);
            
        m_lcm_state = LCM_STATE_STOPPED;

        ret = 0;
    }

    return ret;
}

static int kdp520_lcm_set_source(struct core_device** core_d, u32 src_addr, int src_dev_idx)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    if (NULL == display_drv) {
        dbg_msg_display("Not load display_driver yet.");
        return -1;
    }
    struct panel_driver *panel = display_drv->panel;

    if(src_dev_idx == MIPI_CAM_RGB) {
        panel->preproc_rgb(core_d, src_addr, KDP_DDR_DRV_LCM_START_ADDR);
    }
    else if(src_dev_idx == MIPI_CAM_NIR) {
        panel->preproc_nir(core_d, src_addr, KDP_DDR_DRV_LCM_START_ADDR);//pressing to display addr
    }

    return 0;
}

//static int kdp520_lcm_set_pen(struct core_device** core_d, u8 r, u8 g, u8 b, unsigned int width)
static int kdp520_lcm_set_pen(struct core_device** core_d, u16 color, unsigned int width)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;

    struct kdp520_lcm_context *ctx = &lcm_ctx_s;

    //dbg_msg_display("%s", __func__);

//    if (core_d)
    {
        // ctx->pen_info.r = r;
        // ctx->pen_info.g = g;
        // ctx->pen_info.b = b;
        ctx->pen_info.color = color;
        ctx->pen_info.width = width;
        ret = 0;
    } 

    return ret;
}

static int kdp520_lcm_draw_line(struct core_device** core_d, 
        u32 xs, u32 ys, u32 xe, u32 ye)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;

    struct kdp520_lcm_context *ctx = &lcm_ctx_s;
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);

//    if (core_d)
    {
        u32 x_pos, y_pos;
        u32 x_unit = 1;
        u32 y_unit = 1;
        u32 _addr = ctx->dp_buffer_addr;
        u32 frame_width = display_drv->vi_params.dp_out_w;
        u32 frame_height = display_drv->vi_params.dp_out_h;
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
                    _kdp520_lcm_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
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
                    _kdp520_lcm_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
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

static int kdp520_lcm_draw_rect(struct core_device** core_d, int org_x, int org_y, u32 width, u32 height)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;

    struct kdp520_lcm_context *ctx = &lcm_ctx_s;
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);

//    if (core_d)
    {
        int x_pos, y_pos;
        int x_unit = 1;
        int y_unit = 1;
        int top = org_y;
        int left = org_x;
        int right = org_x + width;
        int bottom = top + height;
        u32 frame_width = display_drv->vi_params.dp_out_w;
        u32 frame_height = display_drv->vi_params.dp_out_h;
        u32 _addr = ctx->dp_buffer_addr;
        u32 border_size = ctx->pen_info.width;
        int left_border = left + border_size;
        int right_border = right - border_size;
        int top_lower = top + border_size;
        int bottom_upper = bottom - border_size;  
        u16 color = ctx->pen_info.color;

        //dbg_msg_display("ctx->frame_buffer[frame_no]=%d %d %x", 
        //ctx->frame_width[frame_no], ctx->frame_height[frame_no], ctx->frame_buffer[frame_no]);
        
        for (y_pos=(top>=0)?top:0; (y_pos < bottom) && (y_pos >= 0) && (y_pos < frame_height); y_pos += y_unit)
        {
            if ((y_pos >= top && y_pos < top_lower) || (y_pos >= bottom_upper))
            {
                for (x_pos=(left>=0)?left:0; (x_pos < right) && (x_pos < frame_width); x_pos += x_unit)
                {
                    //dbg_msg_display("x_pos=%d y_pos=%d", x_pos, y_pos);					 
                    _kdp520_lcm_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
                }
            }
            else
            {
                for (x_pos=(left>=0)?left:0; (x_pos < left_border) && (x_pos < frame_width); x_pos += x_unit)
                {
                    //dbg_msg_display("x_pos=%d y_pos=%d", x_pos, y_pos);					 
                    _kdp520_lcm_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
                }
       
                for (x_pos=(right_border>=0)?right_border:0; (x_pos < right) && (x_pos < frame_width); x_pos += x_unit)
                {
                    //dbg_msg_display("x_pos=%d y_pos=%d", x_pos, y_pos);					 
                    _kdp520_lcm_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
                }
            }
        }

        ret = 0;
    } 
    
    return ret;
}

static int kdp520_lcm_fill_rect(struct core_device** core_d, 
        u32 org_x, u32 org_y, u32 width, u32 height)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;

    struct kdp520_lcm_context *ctx = &lcm_ctx_s;
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);

//    if (core_d)
    {
        u32 x_pos, y_pos;
        u32 _addr = ctx->dp_buffer_addr;
        u32 frame_width = display_drv->vi_params.dp_out_w;
        u32 frame_height = display_drv->vi_params.dp_out_h;
        u16 color = ctx->pen_info.color;

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
                _kdp520_lcm_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
            }
        }
    } 
    
    return ret;
}

static int kdp520_lcm_draw_bitmap(struct core_device** core_d, 
        u32 org_x, u32 org_y, u32 width, u32 height, void* pBuf)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;

    struct kdp520_lcm_context *ctx = &lcm_ctx_s;
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);

    #if 0
    u32* buf = pBuf;
    #endif
    
    //if (core_d)
    {
        u16 y_pos, x_pos, tmp;
        u32 _addr = ctx->dp_buffer_addr;
        u32 frame_width = display_drv->vi_params.dp_out_w;
        u32 frame_height = display_drv->vi_params.dp_out_h;
    #if 0
        u8 bpp = 2;
    #endif
        u8 *p = pBuf;
        u16 * lcd_buffer = (u16 *)_addr;

        if((org_x >= frame_width)
        || (org_y >= frame_height)
        || ((org_x + width) > frame_width)
        || ((org_y + height) > frame_height))
        {
            return ret;
        }

        #if 0
        for(y_pos=org_y; y_pos<(org_y+height); y_pos++)
        {
            memcpy((void *)(_addr+((y_pos*frame_width+org_x)<<1)), (void *)buf, width*bpp);
            buf += width*bpp;
        }
        #endif
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

static int kdp520_lcm_fresh(struct core_device **core_d)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    struct kdp520_lcm_context *ctx = &lcm_ctx_s;
    struct panel_driver *panel = display_drv->panel;
    
    if (NULL == display_drv)
    {
        dbg_msg_display("Not load display_driver yet.");
        return -1;
    }

    panel->update(core_d, ctx->dp_buffer_addr);

    return 0;
}

static u32 kdp520_lcm_get_buffer_addr(struct core_device** core_d)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    if (NULL == display_drv)
    {
        dbg_msg_display("Not load display_driver yet.");
        return 0;
    }

    return KDP_DDR_DRV_LCM_START_ADDR;
}


static struct display_driver_ops kdp520_lcm_ops = {
    .write_cmd  = _lcm_write_cmd,
    .write_data = _lcm_write_data,
    .read_data  = _lcm_read_data,
};
static struct core_device kdp520_lcm;
static lcm_custom_pinmux kdp520_lcm_custom_pinmux;
void kdp520_lcm_set_custom_pinmux(lcm_custom_pinmux* p_param) {
    memcpy(&kdp520_lcm_custom_pinmux, p_param, sizeof(lcm_custom_pinmux));
    if (GPIO_NULL != kdp520_lcm_custom_pinmux.pin_cs)
        kdp520_gpio_setdir(kdp520_lcm_custom_pinmux.pin_cs, GPIO_DIR_OUTPUT);
    if (GPIO_NULL != kdp520_lcm_custom_pinmux.pin_te)
        kdp520_gpio_setdir(kdp520_lcm_custom_pinmux.pin_te, GPIO_DIR_INPUT);
    if (GPIO_NULL != kdp520_lcm_custom_pinmux.pin_rs)
        kdp520_gpio_setdir(kdp520_lcm_custom_pinmux.pin_rs, GPIO_DIR_OUTPUT);
    if (GPIO_NULL != kdp520_lcm_custom_pinmux.pin_rst)
        kdp520_gpio_setdir(kdp520_lcm_custom_pinmux.pin_rst, GPIO_DIR_OUTPUT);
};

struct display_driver kdp520_lcm_driver = {
    .driver       = {
        .name     = "kdp520_lcm",
    },
    .probe        = kdp520_lcm_probe,
    .remove       = kdp520_lcm_remove,
    .core_dev     = &kdp520_lcm,
    .custom_pinmux = &kdp520_lcm_custom_pinmux,
    
    .power_mgr    = {
        .power    = kdp520_lcm_power,
        .suspend  = NULL,
        .resume   = NULL,
    },

    .ctrller_ops  = (void*)&kdp520_lcm_ops,
    .attach_panel = kdp520_lcm_attach_panel,
    .set_params   = kdp520_lcm_set_params,
    .get_params   = kdp520_lcm_get_params,
    .set_camera   = kdp520_lcm_set_camera,
    .get_buffer   = kdp520_lcm_get_buffer_addr,
    .init         = kdp520_lcm_init,
    .start        = kdp520_lcm_start,
    .stop         = kdp520_lcm_stop,
    .set_source   = kdp520_lcm_set_source,
    .set_pen      = kdp520_lcm_set_pen,
    .draw_rect    = kdp520_lcm_draw_rect,
    .draw_line    = kdp520_lcm_draw_line,
    .fill_rect    = kdp520_lcm_fill_rect,
    .draw_bitmap  = kdp520_lcm_draw_bitmap,
    .fresh        = kdp520_lcm_fresh,
};

#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
static struct display_driver_ops kdp520_spi_lcd_ops = {
    .write_cmd_data = spi_lcd_write_cmd,
    .write_cmd_data_single = spi_lcd_write_cmd_single,
    .write_img_buf_data = spi_lcd_write_img_buf_data,
    .read_data_with_buf = kl520_api_ssp_master_read,
};
struct core_device kdp520_spi_lcd;
static lcm_custom_pinmux kdp520_spi_lcd_custom_pinmux;
void kdp520_spi_lcd_set_custom_pinmux(lcm_custom_pinmux* p_param) {
    memcpy(&kdp520_spi_lcd_custom_pinmux, p_param, sizeof(lcm_custom_pinmux));
    memcpy(&kdp520_spi_lcd_custom_pinmux, p_param, sizeof(lcm_custom_pinmux));
    if (GPIO_NULL != kdp520_spi_lcd_custom_pinmux.pin_cs)
        kdp520_gpio_setdir(kdp520_spi_lcd_custom_pinmux.pin_cs, GPIO_DIR_OUTPUT);
    if (GPIO_NULL != kdp520_spi_lcd_custom_pinmux.pin_te)
        kdp520_gpio_setdir(kdp520_spi_lcd_custom_pinmux.pin_te, GPIO_DIR_INPUT);
    if (GPIO_NULL != kdp520_spi_lcd_custom_pinmux.pin_rs)
        kdp520_gpio_setdir(kdp520_spi_lcd_custom_pinmux.pin_rs, GPIO_DIR_OUTPUT);
    if (GPIO_NULL != kdp520_spi_lcd_custom_pinmux.pin_rst)
        kdp520_gpio_setdir(kdp520_spi_lcd_custom_pinmux.pin_rst, GPIO_DIR_OUTPUT);
};

struct display_driver kdp520_spi_lcd_driver = {
    .driver       = {
        .name     = "kdp520_spi_lcd",
    },
    .probe        = kdp520_lcm_probe,
    .remove       = kdp520_lcm_remove,
    .core_dev     = &kdp520_spi_lcd,
    .custom_pinmux = &kdp520_spi_lcd_custom_pinmux,

    .power_mgr    = {
        .power    = kdp520_lcm_power,
        .suspend  = NULL,
        .resume   = NULL,
    },
    
    .ctrller_ops  = (void*)&kdp520_spi_lcd_ops,
    .attach_panel = kdp520_lcm_attach_panel,
    .set_params   = kdp520_lcm_set_params,
    .get_params   = kdp520_lcm_get_params,
    .set_camera   = kdp520_lcm_set_camera,
    .get_buffer   = kdp520_lcm_get_buffer_addr,
    .init         = kdp520_lcm_init,
    .start        = kdp520_lcm_start,
    .stop         = kdp520_lcm_stop,
    .set_source   = kdp520_lcm_set_source,
    .set_pen      = kdp520_lcm_set_pen,
    .draw_rect    = kdp520_lcm_draw_rect,
    .draw_line    = kdp520_lcm_draw_line,
    .fill_rect    = kdp520_lcm_fill_rect,
    .draw_bitmap  = kdp520_lcm_draw_bitmap,
    .fresh        = kdp520_lcm_fresh,
};
#endif

#endif

