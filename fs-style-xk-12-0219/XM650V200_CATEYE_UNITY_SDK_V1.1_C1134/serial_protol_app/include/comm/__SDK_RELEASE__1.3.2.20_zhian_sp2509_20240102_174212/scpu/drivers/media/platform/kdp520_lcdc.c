#include "board_kl520.h"
#if (DISPLAY_DEVICE == DISPLAY_DEVICE_LCDC)
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
#include "media/display/lcdc.h"
#include "media/display/display.h"
#include "dbg.h"
#include "kdp_ddr_table.h"
#include "kl520_include.h"
#include "kdp_memory.h"

#define GET_RGB565(r, g, b) \
        (((((unsigned char)r)>>3)<<11) | ((((unsigned char)g)>>2)<<5) | (((unsigned char)b)>>3))
#define FRAME_SIZE_RGB(xres,yres,mbpp)  ((xres) * (yres) * (mbpp) / 8)
//#define TEST_DRAW_RECT

#define MAX_FRAME_NUM       (4)
#define LCDC_FB_BUF_NUM     (3)

u8  _glcdc_fb_buf_cnt = 0; 
u8  _glcdc_re_buf_cnt = 0; 
u32 _glcdc_cur_addr = KDP_DDR_TEST_RGB_IMG_ADDR;
u32 _glcdc_isr_addr = KDP_DDR_TEST_RGB_IMG_ADDR; 
u32 _glcdc_fb_buf_addr[LCDC_FB_BUF_NUM] = {0};
u32 _glcdc_re_buf_addr[LCDC_FB_BUF_NUM] = {0};

struct pip_window
{
    int x;
    int y;
    int width;
    int height;
};

struct kdp520_lcdc_context
{
#ifdef PIP_FEATURE
    u32 frame_num;
    u32 pip_num;
    u32 pip_width[MAX_FRAME_NUM];
    u32 pip_height[MAX_FRAME_NUM];
#endif
    u32 frame_width[MAX_FRAME_NUM];
    u32 frame_height[MAX_FRAME_NUM];
    u32 frame_size[MAX_FRAME_NUM];
    u32 frame_buffer_addr[MAX_FRAME_NUM];

    struct fb_info info; /* FB driver info record */
    struct display_pen_info pen_info;
} lcdc_ctx;

#define to_kdp520_lcdc_drvdata(_info) \
    container_of(_info, struct kdp520_lcdc_context, info)
  
extern struct clock_node clock_node_lcdc;
struct display_driver kdp520_lcdc_driver;



//#define DEBUG_LCDC
#ifdef DEBUG_LCDC
void debug_lcdc(void)
{
    dbg_msg_display("inw(LCD_FTLCDC210_PA_BASE + 0x0000)=%x", inw(LCDC_REG_FUNC_ENABLE));
    dbg_msg_display("inw((LCD_FTLCDC210_PA_BASE + 0x1100)=%x", inw(LCD_FTLCDC210_PA_BASE + 0x1100));
    dbg_msg_display("inw((LCD_FTLCDC210_PA_BASE + 0x1104)=%x", inw(LCD_FTLCDC210_PA_BASE + 0x1104));
    dbg_msg_display("inw((LCD_FTLCDC210_PA_BASE + 0x1108)=%x", inw(LCD_FTLCDC210_PA_BASE + 0x1108));
    dbg_msg_display("inw((LCD_FTLCDC210_PA_BASE + 0x110C)=%x", inw(LCD_FTLCDC210_PA_BASE + 0x110C));
    dbg_msg_display("inw((LCD_FTLCDC210_PA_BASE + 0x1110)=%x", inw(LCD_FTLCDC210_PA_BASE + 0x1110));
    dbg_msg_display("inw((LCD_FTLCDC210_PA_BASE + 0x112C)=%x", inw(LCD_FTLCDC210_PA_BASE + 0x112C));

    
    dbg_msg_display("LCDC_REG_PANEL_PIXEL_GET_Vcomp()=%x", LCDC_REG_PANEL_PIXEL_GET_Vcomp());
    dbg_msg_display("LCDC_REG_PANEL_PIXEL_GET_UpdateSrc()=%x", LCDC_REG_PANEL_PIXEL_GET_UpdateSrc());

    dbg_msg_display("LCDC_REG_TV_GET_V_Blk3()=%x", LCDC_REG_TV_GET_V_Blk3());
    dbg_msg_display("LCDC_REG_TV_GET_V_Blk2()=%x", LCDC_REG_TV_GET_V_Blk2());    
    dbg_msg_display("LCDC_REG_TV_GET_V_Blk1()=%x", LCDC_REG_TV_GET_V_Blk1());
    dbg_msg_display("LCDC_REG_TV_GET_V_Blk0()=%x", LCDC_REG_TV_GET_V_Blk0());

    dbg_msg_display("LCDC_REG_TV_GET_H_Blk2()=%x", LCDC_REG_TV_GET_H_Blk2());
    dbg_msg_display("LCDC_REG_TV_GET_H_Blk1()=%x", LCDC_REG_TV_GET_H_Blk1());
    dbg_msg_display("LCDC_REG_TV_GET_H_Blk0()=%x", LCDC_REG_TV_GET_H_Blk0());

    dbg_msg_display("Horizontal Back Porch=%u Actual=%u", 
    LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_HBP(), LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_HBP() + 1);
    dbg_msg_display("Horizontal Front Porch=%u Actual=%u", 
    LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_HFP(), LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_HFP() + 1);

    dbg_msg_display("Vertical Back Porch=%u Actual=%u", 
    LCDC_REG_VERTICAL_BACK_PORCH_GET_VBP(), LCDC_REG_VERTICAL_BACK_PORCH_GET_VBP()); // SHARP is turned off
    dbg_msg_display("Vertical Front Porch=%u Actual=%u", 
    LCDC_REG_VERTICAL_TIMING_CTRL_GET_VFP(), LCDC_REG_VERTICAL_TIMING_CTRL_GET_VFP());

    dbg_msg_display("Horizontal Sync Pulses Width=%u Actual=%u", 
    LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_HW(), LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_HW() + 1);
    dbg_msg_display("Pixels-per-Line=%u Actual pixels-per-line=%u", 
    LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_PL(), (LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_PL() + 1) << 4);

    dbg_msg_display("Vertical Sync Pulse Width=%u actual=%u",
    LCDC_REG_VERTICAL_TIMING_CTRL_GET_VW(), LCDC_REG_VERTICAL_TIMING_CTRL_GET_VW() + 1);
    dbg_msg_display("Lines-per-Frame=%u actual=%u", 
    LCDC_REG_VERTICAL_TIMING_CTRL_GET_LF(), LCDC_REG_VERTICAL_TIMING_CTRL_GET_LF() + 1);

    //polarity control
    dbg_msg_display("LCD Panel Clock divisor control=%u actual=%u", 
    LCDC_REG_POLARITY_CTRL_GET_DivNo(), LCDC_REG_POLARITY_CTRL_GET_DivNo() + 1);
    if (LCDC_REG_POLARITY_CTRL_GET_IPWR() == 1) {
        dbg_msg_display("LC_PWROFF output is active low");
    }        
    else if (LCDC_REG_POLARITY_CTRL_GET_IPWR() == 0) {
        dbg_msg_display("LC_PWROFF output is active high");
    }        
    if (LCDC_REG_POLARITY_CTRL_GET_IDE() == 1) {
        dbg_msg_display("LC_DE output pin is active low");
    }        
    else if (LCDC_REG_POLARITY_CTRL_GET_IDE() == 0) {
        dbg_msg_display("LC_DE output pin is active high");
    }        
    if (LCDC_REG_POLARITY_CTRL_GET_ICK() == 1) {
        dbg_msg_display("Data is driven on the LCD data lines on the falling edge of LC_PCLK");
    }        
    else if (LCDC_REG_POLARITY_CTRL_GET_ICK() == 0) {
        dbg_msg_display("Data is driven on the LCD data lines on the rising edge of LC_PCLK");
    }        
    if (LCDC_REG_POLARITY_CTRL_GET_IHS() == 1) {
        dbg_msg_display("LC_HS pin is active low and inactive high");
    }        
    else if (LCDC_REG_POLARITY_CTRL_GET_IHS() == 0) {
        dbg_msg_display("LC_HS pin is active low and inactive high");
    }
    if (LCDC_REG_POLARITY_CTRL_GET_IVS() == 1) {
        dbg_msg_display("LC_VS pin is active low and inactive high");
    } else if (LCDC_REG_POLARITY_CTRL_GET_IVS() == 0) {
        dbg_msg_display("LC_VS pin is active low and inactive high");                        
    }

    dbg_msg_display("horizontal resolution input", SCALER_HOR_RES_IN, hor_no_in);
    dbg_msg_display("vertical resolution input", SCALER_VER_RES_IN, ver_no_in);
    dbg_msg_display("horizontal resolution output", SCALER_HOR_RES_OUT, hor_no_out);
    dbg_msg_display("vertical resolution output", SCALER_VER_RES_OUT, ver_no_out);

    dbg_msg_display("scaling ratio selection of 1st stage", SCALER_MISC, fir_sel);
    dbg_msg_display("horizontal interpolation mode", SCALER_MISC, hor_inter_mode);
    dbg_msg_display("vertical interpolation mode", SCALER_MISC, ver_inter_mode);
    dbg_msg_display("identifies the 2nd stage scaler bypass mode", SCALER_MISC, bypass_mode);
    dbg_msg_display("second-stage initial numerator of the scaler coeff in hor", SCALER_RES, scal_hor_num);
    dbg_msg_display("second-stage initial numerator of the scaler coeff in ver", SCALER_RES, scal_ver_num);
}
#endif

void _lcdc_clear(struct kdp520_lcdc_context *context, int frame_no, u8 r, u8 g, u8 b)
{
//	int i;
//	unsigned int color;
//	unsigned int tmp_color;
//	unsigned char y, cb, cr;
//	unsigned char *fb_y;				// y planet
//	unsigned char *fb_u;				// cb planet
//	unsigned char *fb_v;				// cr planet
	
//    color = RGB565(r, g, b);
//    color = (color<<16) | color;
//    
//	for (i=0; i<fbi->frame_size[frame_no]; i+=4)
//	{
//		*(unsigned int *)(fbi->pFrameBuffer[frame_no]+i) = color;
//	}
}

//u32 lcdc_frame_addr = KDP_DDR_TEST_RGB_IMG_ADDR;

//void LCDC_VSTATUS_IRQHandler(void)



extern u32 kl520_api_dp_buf_addr(void);


void lcdc_vstatus_isr(void)
{
    int status;
//    struct kdp520_lcdc_context *ctx = &lcdc_ctx;

    status = inw(LCDC_REG_INTR_STATUS);
    //dbg_msg_display("lcdc_vstatus_isr status=%x", status);

    if (status & LCDC_REG_INTR_STATUS_IntVstatus) {
        
//        u32 addr = kl520_api_dp_buf_addr();
//        outw(LCDC_REG_PANEL_IMAGE0_FRAME0, addr);
        //outw(LCDC_REG_PANEL_IMAGE0_FRAME0, 0x6302E000);
        //outw(LCDC_REG_PANEL_IMAGE0_FRAME0, KDP_DDR_TEST_RGB_IMG_ADDR);
        //fb_addr();
        outw(LCDC_REG_PANEL_IMAGE0_FRAME0, _glcdc_isr_addr);
        LCDC_REG_INTR_CLEAR_SET_ClrVstatus();
    }

    outw(LCDC_REG_INTR_CLEAR, status);
    NVIC_ClearPendingIRQ((IRQn_Type)LCDC_FTLCDC210_VSTATUS_IRQ);
}

//static void ddr_zeros(volatile u32 start, u32 size) 
//{
//    u32 end = start + size;
//    while (start < end) {
//        outw(start, 0x0);
//          start += 4;
//    }
//}

static __inline int _kdp520_lcdc_draw_pixel(
        u32 frame_buffer, int x, int y, u32 width, u16 color)
{
    *(u16 *)(frame_buffer + ((y * width + x) << 1) + 0) = color; 
        return 0;
}


static void _kdp520_lcdc_set_divno(u32 div)
{
    LCDC_REG_POLARITY_CTRL_SET_DivNo(div);
}

static void _kdp520_lcdc_set_endian(enum lcdc_fb_data_endianness_type type)
{
    switch (type) {
    case lcdc_fb_data_endianness_lblp:
        LCDC_REG_PANEL_PIXEL_SET_Endian(0);
        break;
    case lcdc_fb_data_endianness_bbbp:
        LCDC_REG_PANEL_PIXEL_SET_Endian(1);
        break;
    case lcdc_fb_data_endianness_lbbp:
        LCDC_REG_PANEL_PIXEL_SET_Endian(2);
        break;
    default:;
    }
}

void kdp520_lcdc_set_panel_type(enum lcdc_panel_type type)
{
    switch (type) {
    case lcdc_6bits_per_channel:
        LCDC_REG_PANEL_PIXEL_SET_PanelType(0);
        break;
    case lcdc_8bits_per_channel:
        LCDC_REG_PANEL_PIXEL_SET_PanelType(1);
        break;
    default:;
    }
}

void kdp520_lcdc_set_bgrsw(enum lcdc_output_fmt_sel type)
{
    switch (type) {
    case lcdc_output_fmt_sel_rgb:
        LCDC_REG_PANEL_PIXEL_SET_BGRSW(0);
        break;
    case lcdc_output_fmt_sel_bgr:
        LCDC_REG_PANEL_PIXEL_SET_BGRSW(1);
        break;
    default:;
    }
}

void kdp520_lcdc_set_framerate(int frame_rate, u32 width, u32 height) 
{
#define LCD_PLL 148500000
    u32 div;
    u32 serial_mode = LCDC_REG_SERIAL_PANEL_PIXEL_GET_SerialMode();
    u32 upper_margin = LCDC_REG_VERTICAL_BACK_PORCH_GET_VBP() + 1;
    u32 lower_margin = LCDC_REG_VERTICAL_TIMING_CTRL_GET_VFP();
    u32 vsync_len = LCDC_REG_VERTICAL_TIMING_CTRL_GET_VW() + 1;
    u32 left_margin = LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_HBP() + 1;
    u32 right_margin = LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_HFP() + 1;
    u32 hsync_len = LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_HW() + 1;   
    u32 pixel_clock = (serial_mode ? 3 : 1 ) * frame_rate *
        (height + upper_margin + lower_margin + vsync_len) *
        (width + left_margin + right_margin + hsync_len);
    
    //dbg_msg_display("[lcdc] kdp520_lcdc_set_framerate width=%d", width);    
    //dbg_msg_display("[lcdc] kdp520_lcdc_set_framerate height=%d", height);
    dbg_msg_display("[lcdc] kdp520_lcdc_set_framerate left_margin=%d, right_margin=%d, hsync_len = %d", left_margin, right_margin,hsync_len);
    
    div = LCD_PLL / pixel_clock;
    dbg_msg_display("[lcdc] kdp520_lcdc_set_framerate div=0x%x, fbi->input_clock=%d, pclk = %d", div, LCD_PLL,pixel_clock);
    if (div < 1)
    {
        dbg_msg_display("invalid input clock for LCD");
        for (;;) ;
    }
    if((height == 1080) && (width == 1920))
        div = 0;
    else if ((height == 480) && (width == 640))
        div = 1;
    else
        div = 5;

    _kdp520_lcdc_set_divno(div);
}

//static u32 _lcdc_get_bpp_from_img_pixfmt(enum lcdc_img_pixfmt pixfmt) 
//{
//    u32 bpp = 0;
//    switch (pixfmt) {
//    case lcdc_img_pixfmt_1bpp: bpp = 0; break;
//    case lcdc_img_pixfmt_2bpp: bpp = 1; break;
//    case lcdc_img_pixfmt_4bpp: bpp = 2; break;
//    case lcdc_img_pixfmt_8bpp: bpp = 3; break;
//    case lcdc_img_pixfmt_16bpp: bpp = 4; break;
//    case lcdc_img_pixfmt_24bpp: bpp = 5; break;
//    case lcdc_img_pixfmt_argb888: bpp = 6; break;
//    case lcdc_img_pixfmt_argb1555: bpp = 7; break;
//    default: bpp = 4; break;
//    }
//    
//    return bpp;
//}

//static void _lcdc_set_img_fmt(struct lcdc_img_pixfmt_pxp *fmt)
//{
//    u32 val = inw(LCDC_REG_PIPPOP_FMT_1);
//    u32 bpp0 = _lcdc_get_bpp_from_img_pixfmt(fmt->pixfmt_img0);
//    u32 bpp1 = _lcdc_get_bpp_from_img_pixfmt(fmt->pixfmt_img1) << 4;
//    u32 bpp2 = _lcdc_get_bpp_from_img_pixfmt(fmt->pixfmt_img2) << 8;
//    u32 bpp3 = _lcdc_get_bpp_from_img_pixfmt(fmt->pixfmt_img3) << 12;

//    //val |= bpp0 | bpp1 | bpp2 | bpp3;
//    val = bpp0 | bpp1 | bpp2 | bpp3;
//    outw(LCDC_REG_PIPPOP_FMT_1, val);
//}

//static
//void _lcdc_decide_frame_dim(
//        struct kdp520_lcdc_context *context)    
//{
//    struct kdp520_lcdc_context *ctx = context;
//    int i;

//    //dbg_msg_display(" frame_num=%d", ctx->frame_num);
//    
////    for (i = 0; i < ctx->frame_num; ++i)
////    {
////        ctx->frame_width[i] = ctx->input_xres;
////        ctx->frame_height[i] = ctx->input_yres;
////        //dbg_msg_display("ctx->frame_width[i]=%d ctx->frame_height[i]=%d",ctx->frame_width[i], ctx->frame_height[i]);
////        if (ctx->frame_num > 1)
////        {
////            if (i > 0)
////            {
////                //fbi->frame_width[i] = fbi->pip_width[i-1]*(float)fbi->frame_width[i]/(float)fbi->panel_width;
////                ctx->frame_width[i] = ctx->pip_width[i];		
////                //fbi->frame_height[i] = fbi->pip_height[i-1]*(float)fbi->frame_height[i]/(float)fbi->panel_height;
////                ctx->frame_height[i] = ctx->pip_height[i];
////            }
////        }
////    }
//}

//static
//void _lcdc_decide_frame_num(
//        struct kdp520_lcdc_context *context)
//{
//    struct kdp520_lcdc_context *ctx = context;

//    if (image_input_format_rgb565 == ctx->input_fmt || 
//        image_input_format_rgb555 == ctx->input_fmt || 
//        image_input_format_rgb444 == ctx->input_fmt || 
//        image_input_format_rgb24 == ctx->input_fmt || 
//        image_input_format_ycbcr422 == ctx->input_fmt)
//    {
//        if (ctx->pip_num > 0)
//        {
//            ctx->frame_num = ctx->pip_num + 1;
//        }
//        else
//        {
//            ctx->frame_num = 1;
//        }
//    }
//    else
//    {
//        ctx->frame_num = 1;
//    }
//}

int _lcdc_set_framebase(struct kdp520_lcdc_context *context, 
        u16 image, u32 frame_buffer_addr)
{
    dbg_msg_display("[%s] image=%d frame_buffer_addr=%x", __func__, image, frame_buffer_addr);
    switch (image) {
        case 0: outw(LCDC_REG_PANEL_IMAGE0_FRAME0, frame_buffer_addr); break;
        case 1: outw(LCDC_REG_PANEL_IMAGE1_FRAME0, frame_buffer_addr); break;
        case 2: outw(LCDC_REG_PANEL_IMAGE2_FRAME0, frame_buffer_addr); break;
        case 3: outw(LCDC_REG_PANEL_IMAGE3_FRAME0, frame_buffer_addr); break;
        default:;
    }

    return 0;
}

static void _kdp520_lcdc_alloc_framebuffer(struct kdp520_lcdc_context *context)
{
//#ifdef PIP_FEATURE
//    _kdp520_lcdc_decide_frame_num(ctx);
//    _kdp520_lcdc_decide_frame_dim(ctx);
//#endif

    //LCDC_REG_PANEL_PIXEL_SET_Vcomp(0);
}


//static void _lcdc_init_rgb_value()
//{
//    int i;
//    for(i = 0; i < 0x100; i++)
//    {
//        writeb(i, LCDC_REG_LT_OF_GAMMA_RED + i);
//        writeb(i, LCDC_REG_LT_OF_GAMMA_GREEN + i);
//        writeb(i, LCDC_REG_LT_OF_GAMMA_BLUE + i);
//    }
//}

//static 
//void _lcdc_pip_update(struct kdp520_lcdc_context *context)
//{
//    LCDC_REG_PIP_SUBPIC1_POS_SET_PiP_Update(1);
//}

//static
//void _lcdc_set_pip_pos(struct kdp520_lcdc_context *context, int pip_idx, u32  h_pos, u32 v_pos)
//{
////    struct kdp520_lcdc_context *ctx = context;

//    switch (pip_idx) {
//    case 0:
//        LCDC_REG_PIP_SUBPIC1_POS_SET_PiP1Hpos(0);
//        LCDC_REG_PIP_SUBPIC1_POS_SET_PiP1Vpos(0);
//        LCDC_REG_PIP_SUBPIC1_POS_SET_PiP1Hpos(h_pos);
//        LCDC_REG_PIP_SUBPIC1_POS_SET_PiP1Vpos(v_pos);
//        break;

//    case 1:
//        LCDC_REG_PIP_SUBPIC2_POS_SET_PiP1Hpos(0);
//        LCDC_REG_PIP_SUBPIC2_POS_SET_PiP1Vpos(0);
//        LCDC_REG_PIP_SUBPIC2_POS_SET_PiP1Hpos(h_pos);
//        LCDC_REG_PIP_SUBPIC2_POS_SET_PiP1Vpos(v_pos);
//        break;
//    case 2:
//        break;
//    default:;
//    }

//    LCDC_REG_PIP_SUBPIC1_POS_SET_PiP_Update(1);
//}

//static
//void _lcdc_set_pip_dim(struct kdp520_lcdc_context *context, int pip_idx, u32 h_dim, u32 v_dim)
//{
////    struct kdp520_lcdc_context *ctx = context;

////    ctx->pip_width[pip_idx] = h_dim;
////    ctx->pip_height[pip_idx] = v_dim;

//    switch (pip_idx) {
//    case 0:
//        LCDC_REG_PIP_SUBPIC1_DIM_SET_PiP1Hdim(0);
//        LCDC_REG_PIP_SUBPIC1_DIM_SET_PiP1Vdim(0);
//        LCDC_REG_PIP_SUBPIC1_DIM_SET_PiP1Hdim(h_dim);
//        LCDC_REG_PIP_SUBPIC1_DIM_SET_PiP1Vdim(v_dim);
//        break;

//    case 1:
//        LCDC_REG_PIP_SUBPIC1_DIM_SET_PiP2Hdim(0);
//        LCDC_REG_PIP_SUBPIC1_DIM_SET_PiP2Vdim(0);

//        LCDC_REG_PIP_SUBPIC1_DIM_SET_PiP2Hdim(h_dim);
//        LCDC_REG_PIP_SUBPIC1_DIM_SET_PiP2Vdim(v_dim);    
//        break;
//    case 2:
//        break;
//    default:;
//    }
//} 

//static int _lcdc_set_pip_enable(struct kdp520_lcdc_context *context, u32 pip_num)
//{
////    struct kdp520_lcdc_context *ctx = context;

//    //ctx->pip_num = pip_num;
//    //ctx->frame_num = pip_num + 1;

//    LCDC_REG_FUNC_ENABLE_SET_PiPEn(0);
//    LCDC_REG_FUNC_ENABLE_SET_PiPEn(pip_num);

//    //_lcdc_alloc_framebuffer(ctx);

//    return 0;
//}

//void _lcdc_set_pip_alpha_blending(
//        struct kdp520_lcdc_context *context, 
//        enum alpha_blending_type type, int blend_l, int blend_h)
//{
////    struct kdp520_lcdc_context *ctx = context;

//    switch (type) {
//    case alpha_blending_none:
//        LCDC_REG_FUNC_ENABLE_SET_BlendEn(0);
//        break;
//    case alpha_blending_global:
//        LCDC_REG_PIP_BLENDING_SET_PiPBlend_l(blend_l);
//        LCDC_REG_PIP_BLENDING_SET_PiPBlend_h(blend_h);
//        LCDC_REG_FUNC_ENABLE_SET_BlendEn(1);
//        break;
//    case alpha_blending_pixel:
//        LCDC_REG_FUNC_ENABLE_SET_BlendEn(2);
//        break;
//    }
//}

//static int kdp520_fb_blank(int blank_mode, struct fb_info *fbi)
//{
//    //struct kdp520fb_drvdata *drvdata = to_kdp520_lcdc_drvdata(fbi);

//    /* to Do */

//    return 0; 
//}

//static int kdp520_lcdc_probe(struct core_device** core_d)
//{
//    return 0;
//}

//static int kdp520_lcdc_remove(struct core_device** core_d)
//{
//    //struct kdp520_lcdc_context *ctx = framework_drv_get_drvdata(&core_d->pin_ctx);
//    //kdp520_fb_blank(0, &drvdata->info);

//    LCDC_REG_FUNC_ENABLE_SET_LCDon(0);
//    LCDC_REG_FUNC_ENABLE_SET_LCDen(0);

//    return 0;
//}

//static int kdp520_lcdc_reset(struct core_device** core_d)
//{
//    return 0;
//}

static int kdp520_lcdc_attach_panel(struct core_device** core_d, struct panel_driver *panel)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);

    display_drv->panel = panel;

//    if (m_lcm_state == LCM_STATE_IDLE) {
//        kdp520_lcm_probe(core_d);
//        //delay_us(150000);
//    }

    return 0;
}
static int kdp520_lcdc_set_params(struct core_device** core_d, struct video_input_params *params)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    struct video_input_params *p = &display_drv->vi_params;
    
    memcpy(p, params, sizeof(*params));
    display_drv->fb_size = calc_framesize(p->src_width, p->src_height, p->src_fmt);    
    dbg_msg_display("display_drv->fb_size=%d", display_drv->fb_size);  
    return 0;
}

static int kdp520_lcdc_get_params(struct core_device** core_d, struct video_input_params *params)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    struct video_input_params *p = &display_drv->vi_params;
    memcpy(params, p, sizeof(*params));    
    
    return 0;
}
    
static int kdp520_lcdc_set_camera(struct core_device** core_d)
{
    struct kdp520_lcdc_context *ctx = &lcdc_ctx;
//    dbg_msg_display("[%s] ctx->cam_src_idx=%d", __func__, ctx->cam_src_idx);
    _kdp520_lcdc_alloc_framebuffer(ctx);    
//    
    return 0;    
}

static void _kdp520_lcdc_init_rgb_value()
{
    int i;
    for (i = 0; i < 0x100; i++) {
        writeb(i, LCDC_REG_LT_OF_GAMMA_RED + i);
        writeb(i, LCDC_REG_LT_OF_GAMMA_GREEN + i);
        writeb(i, LCDC_REG_LT_OF_GAMMA_BLUE + i);
    }
}

static u32 _kdp520_lcdc_get_bpp_from_img_pixfmt(enum lcdc_img_pixfmt pixfmt)
{
    u32 bpp = 0;
    switch (pixfmt) {
        case lcdc_img_pixfmt_1bpp:
            bpp = 0;
            break;
        case lcdc_img_pixfmt_2bpp:
            bpp = 1;
            break;
        case lcdc_img_pixfmt_4bpp:
            bpp = 2;
            break;
        case lcdc_img_pixfmt_8bpp:
            bpp = 3;
            break;
        case lcdc_img_pixfmt_16bpp:
            bpp = 4;
            break;
        case lcdc_img_pixfmt_24bpp:
            bpp = 5;
            break;
        case lcdc_img_pixfmt_argb888:
            bpp = 6;
            break;
        case lcdc_img_pixfmt_argb1555:
            bpp = 7;
            break;
        default:
            bpp = 4;
            break;
    }
    return bpp;
}

static void _kdp520_lcdc_set_img_fmt(struct lcdc_img_pixfmt_pxp *fmt)
{
    u32 val = inw(LCDC_REG_PIPPOP_FMT_1);
    u32 bpp0 = _kdp520_lcdc_get_bpp_from_img_pixfmt(fmt->pixfmt_img0);
    u32 bpp1 = _kdp520_lcdc_get_bpp_from_img_pixfmt(fmt->pixfmt_img1) << 4;
    u32 bpp2 = _kdp520_lcdc_get_bpp_from_img_pixfmt(fmt->pixfmt_img2) << 8;
    u32 bpp3 = _kdp520_lcdc_get_bpp_from_img_pixfmt(fmt->pixfmt_img3) << 12;

    //val |= bpp0 | bpp1 | bpp2 | bpp3;
    val = bpp0 | bpp1 | bpp2 | bpp3;
    LCDC_REG_PIPPOP_FMT_1_SET_BppFifo0(val);
}

static void _kdp520_lcdc_init_palette()
{
    int i;
    u16 color;

    for (i = 0; i < 256; i++) {
        color = ((i >> 3) << 11) | ((i >> 2) << 5) | ((i >> 3) << 0);
        writew(color, LCDC_REG_PALETTE + i * 2);
    }
}

static void _kdp520_lcdc_set_img_src_fmt(struct display_driver *display_drv)
{
    struct lcdc_img_pixfmt_pxp img_pixfmt;

    u32 pix_param = inw(LCDC_REG_PANEL_PIXEL);
    if (V2K_PIX_FMT_RGB565 == display_drv->vi_params.src_fmt) {
        img_pixfmt.pixfmt_img0 = lcdc_img_pixfmt_16bpp;
        img_pixfmt.pixfmt_img1 = lcdc_img_pixfmt_16bpp;
        img_pixfmt.pixfmt_img2 = lcdc_img_pixfmt_16bpp;
        img_pixfmt.pixfmt_img3 = lcdc_img_pixfmt_16bpp;
        _kdp520_lcdc_init_rgb_value();
        _kdp520_lcdc_set_img_fmt(&img_pixfmt);

        LCDC_REG_FUNC_ENABLE_SET_EnYCbCr(0);
        pix_param = (pix_param & ~(LCDC_REG_PANEL_PIXEL_RGBTYPE_MASK |
                                   LCDC_REG_PANEL_PIXEL_BppFifo_MASK) ) |
                    LCDC_REG_PANEL_PIXEL_BppFifo_16bpp |
                    LCDC_REG_PANEL_PIXEL_RGBTYPE_565;

        outw(LCDC_REG_PANEL_PIXEL, pix_param);
    } else if (V2K_PIX_FMT_RAW8 == display_drv->vi_params.src_fmt) {
        img_pixfmt.pixfmt_img0 = lcdc_img_pixfmt_8bpp;
        img_pixfmt.pixfmt_img1 = lcdc_img_pixfmt_8bpp;
        img_pixfmt.pixfmt_img2 = lcdc_img_pixfmt_8bpp;
        img_pixfmt.pixfmt_img3 = lcdc_img_pixfmt_8bpp;
        _kdp520_lcdc_init_rgb_value();
        _kdp520_lcdc_init_palette();
        _kdp520_lcdc_set_img_fmt(&img_pixfmt);

        LCDC_REG_FUNC_ENABLE_SET_EnYCbCr(0);
        pix_param = (pix_param & ~(LCDC_REG_PANEL_PIXEL_RGBTYPE_MASK |
                                   LCDC_REG_PANEL_PIXEL_BppFifo_MASK) ) |
                    LCDC_REG_PANEL_PIXEL_BppFifo_8bpp;
        outw(LCDC_REG_PANEL_PIXEL, pix_param);
    }
}

static int kdp520_lcdc_init(struct core_device** core_d)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
#if 1 // test new flow
    SCU_EXTREG_SWRST_SET_LCDC_resetn(0); //software reset

    SCU_EXTREG_CLK_EN1_SET_LC_SCALER(1);
    SCU_EXTREG_CLK_EN1_SET_LC_CLK(1);

    SCU_EXTREG_SWRST_SET_LCDC_resetn(1); //reset release
    clock_mgr_change_pll5_clock(1, 0x54, 5); //***** MUST SET Clock
#else
    SCU_EXTREG_SWRST_SET_LCDC_resetn(0); //software reset

    SCU_EXTREG_CLK_EN1_SET_LC_SCALER(1);
    SCU_EXTREG_CLK_EN1_SET_LC_CLK(1);

    clock_mgr_change_pll5_clock(1, 0x54, 5);

    SCU_EXTREG_SWRST_SET_LCDC_resetn(1); //reset release
#endif
    //clear all interrupt status
    outw(LCDC_REG_INTR_CLEAR, 0x0f);

    //enable buserr / vstatus / next frame / fifo underrun interrupt
    outw(LCDC_REG_INTR_ENABLE_MASK, 0x0f);

    _kdp520_lcdc_set_img_src_fmt(display_drv);

    // set FIFO thread
    outw(LCDC_REG_FIFO_THRESHOLD, 0x04040404);

    _kdp520_lcdc_set_endian(lcdc_fb_data_endianness_lblp);

    NVIC_SetVector((IRQn_Type)LCDC_FTLCDC210_VSTATUS_IRQ, (uint32_t)lcdc_vstatus_isr);
    NVIC_EnableIRQ(LCDC_FTLCDC210_VSTATUS_IRQ);

#ifdef TEST_LCDC_PATTERN
    LCDC_REG_FUNC_ENABLE_SET_PenGen(1);
#endif

    if (display_drv->panel) {
        display_drv->display_id = display_drv->panel->read_did(core_d);
        dbg_msg_display(" display_id=%x", display_drv->display_id);
        display_drv->panel->init(core_d);
    }
    
    return 0;
}

//static int kdp520_lcdc_power(struct core_device** core_d, BOOL on)
//{
//    return 0;
//}

//static int kdp520_lcdc_pip_test(struct core_device** core_d)
//{
//    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;
//    
//    return ret;
//}    

static int kdp520_lcdc_start(struct core_device** core_d)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;
    
    dbg_msg_display("%s", __func__);

    LCDC_REG_FUNC_ENABLE_SET_LCDen(1);
    LCDC_REG_FUNC_ENABLE_SET_LCDon(1);    
    
    return ret;
}

static int kdp520_lcdc_stop(struct core_device** core_d)
{
    //int ret = -KDP_FRAMEWORK_ERRNO_INVALA;
 
    //struct kdp520_lcdc_context *ctx = &lcdc_ctx;
    
    dbg_msg_display("%s", __func__);

    LCDC_REG_FUNC_ENABLE_SET_LCDon(0);
    //LCDC_REG_FUNC_ENABLE_SET_LCDen(0);
    
    return 0;//ret;
}
//static int kdp520_lcdc_set_pen(struct core_device** core_d, u8 r, u8 g, u8 b, unsigned int width)
static int kdp520_lcdc_set_pen(struct core_device** core_d, u16 color, unsigned int width)
{
    //int ret = -KDP_FRAMEWORK_ERRNO_INVALA;

    struct kdp520_lcdc_context *ctx = &lcdc_ctx;

    ctx->pen_info.color = color;
    ctx->pen_info.width = width;
   
    
    return 0;//ret;
}

static int kdp520_lcdc_draw_rect(struct core_device** core_d, int org_x, int org_y, u32 width, u32 height)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;
    struct kdp520_lcdc_context *ctx = &lcdc_ctx;
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
        
//        dbg_msg_display("org_x = %d, org_y =%d",org_x, org_y);
//        dbg_msg_display("width = %d, height =%d",width, height);
//        dbg_msg_display("frame_width = %d, frame_height =%d",frame_width, frame_height);
        //ctx->frame_width[frame_no], ctx->frame_height[frame_no], ctx->frame_buffer[frame_no]);
        
        for (y_pos=(top>=0)?top:0; (y_pos < bottom) && (y_pos >= 0) && (y_pos < frame_height); y_pos += y_unit)
        {
            if ((y_pos >= top && y_pos < top_lower) || (y_pos >= bottom_upper))
            {
                for (x_pos=(left>=0)?left:0; (x_pos < right) && (x_pos < frame_width); x_pos += x_unit)
                {
                    //dbg_msg_display("x_pos=%d y_pos=%d", x_pos, y_pos);					 
                    _kdp520_lcdc_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
                }
            }
            else
            {
                for (x_pos=(left>=0)?left:0; (x_pos < left_border) && (x_pos < frame_width); x_pos += x_unit)
                {
                    //dbg_msg_display("x_pos=%d y_pos=%d", x_pos, y_pos);					 
                    _kdp520_lcdc_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
                }
       
                for (x_pos=(right_border>=0)?right_border:0; (x_pos < right) && (x_pos < frame_width); x_pos += x_unit)
                {
                    //dbg_msg_display("x_pos=%d y_pos=%d", x_pos, y_pos);					 
                    _kdp520_lcdc_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
                }
            }
        }

        ret = 0;
    } 
    
    return ret;
}
static int kdp520_lcdc_draw_line(struct core_device** core_d, 
        u32 xs, u32 ys, u32 xe, u32 ye)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;

    struct kdp520_lcdc_context *ctx = &lcdc_ctx;
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);

//    if (core_d)
    {
        u32 x_pos, y_pos;
        u32 x_unit = 1;
        u32 y_unit = 1;
        
        u32 frame_width = display_drv->vi_params.dp_out_w;
        u32 frame_height =  display_drv->vi_params.dp_out_h;
        u32 _addr = _glcdc_cur_addr;
        
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
                    _kdp520_lcdc_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
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
                    _kdp520_lcdc_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
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

static int kdp520_lcdc_fill_rect(struct core_device** core_d, 
        u32 org_x, u32 org_y, u32 width, u32 height)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;

    struct kdp520_lcdc_context *ctx = &lcdc_ctx;
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
                _kdp520_lcdc_draw_pixel(_addr, x_pos, y_pos, frame_width, color);
            }
        }
    } 
    
    return ret;
}

static int kdp520_lcdc_draw_bitmap(struct core_device** core_d, 
        u32 org_x, u32 org_y, u32 width, u32 height, void* pBuf)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;

    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);

    #if 0
    u32* buf = pBuf;
    #endif
    
//    if (core_d)
    {
        u16 y_pos, x_pos, tmp;

        u32 frame_width = display_drv->vi_params.dp_out_w;
        u32 frame_height =  display_drv->vi_params.dp_out_h;
        u32 _addr = _glcdc_cur_addr;
        
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

int kdp520_lcdc_fresh(struct core_device** core_d)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;   
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    if (NULL == display_drv)
    {
        dbg_msg_display("Not load display_driver yet.");
        return -1;
    }

// #if ( CFG_GUI_ENABLE == YES )

    struct panel_driver *panel = display_drv->panel;
//
    u32 src_cam_idx = display_drv->vi_params.src_cam_idx;

    u32 scr_img_w = display_drv->vi_params.panel_in_w;
    u32 scr_img_h = display_drv->vi_params.panel_in_h;

//    u32 scr_w = display_drv->vi_params.panel_out_w;
//    u32 scr_h = display_drv->vi_params.panel_out_h;

    u32 dp_addr = _glcdc_re_buf_addr[_glcdc_re_buf_cnt];

    if(dp_addr == 0){
        _glcdc_re_buf_addr[_glcdc_re_buf_cnt] = dp_addr = kdp_ddr_reserve(scr_img_w*scr_img_h*2);
        memset((void *)dp_addr, 0, scr_img_w*scr_img_h*2);
    }
    else
    {
        memset((void *)dp_addr, 0, scr_img_w*scr_img_h*2);
    }

    //if(FACE_MODE_NONE != kl520_api_fdfr_facemode_get())    //RGB

    if(src_cam_idx == MIPI_CAM_RGB) {
        panel->posproc_rgb(core_d, _glcdc_cur_addr, dp_addr);
    }
    else if (src_cam_idx == MIPI_CAM_NIR) {
        panel->posproc_nir(core_d, _glcdc_cur_addr, dp_addr);//pressing to display addr
    }

//    _glcdc_cur_addr = dp_addr;
    _glcdc_isr_addr = dp_addr;
//
    _glcdc_re_buf_cnt++;
    _glcdc_re_buf_cnt = _glcdc_re_buf_cnt%LCDC_FB_BUF_NUM;

//#else
//    _glcdc_isr_addr = _glcdc_cur_addr;
//#endif


    LCDC_REG_FUNC_ENABLE_SET_LCDen(1);
    LCDC_REG_FUNC_ENABLE_SET_LCDon(1);
    
    return ret;   
}

int kdp520_lcdc_set_source(struct core_device** core_d, u32 src_addr, int src_dev_idx)
{
#define DP_BUF_SIZE     (DISPLAY_WIDTH*DISPLAY_HEIGHT*2)

    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    if (NULL == display_drv) {
        dbg_msg_display("Not load display_driver yet.");
        return -1;
    }
    struct panel_driver *panel = display_drv->panel;

//    u32 src_w = display_drv->vi_params.src_width;
//    u32 src_h = display_drv->vi_params.src_height;
//    u32 dp_w = display_drv->vi_params.dp_out_w;
//    u32 dp_h = display_drv->vi_params.dp_out_h;
    u32 src_cam_idx = display_drv->vi_params.src_cam_idx;     
    u32 _lcdc_addr = _glcdc_fb_buf_addr[_glcdc_fb_buf_cnt];

    //Init
    if(_lcdc_addr == 0){
        _lcdc_addr = kdp_ddr_reserve(DP_BUF_SIZE);
        _glcdc_fb_buf_addr[_glcdc_fb_buf_cnt] = _lcdc_addr;
        memset((void *)_lcdc_addr, 0, DP_BUF_SIZE);
    }

    if(src_cam_idx == MIPI_CAM_RGB) {
        panel->preproc_rgb(core_d, src_addr, _lcdc_addr);
    }
    else if (src_cam_idx == MIPI_CAM_NIR) {
        panel->preproc_nir(core_d, src_addr, _lcdc_addr);//pressing to display addr
    }

    //_glcdc_isr_addr = _glcdc_cur_addr = _lcdc_addr;
    _glcdc_cur_addr = _lcdc_addr;
    _glcdc_fb_buf_cnt++;
    _glcdc_fb_buf_cnt = _glcdc_fb_buf_cnt%LCDC_FB_BUF_NUM;

    return 0;
}


u32 kdp520_lcdc_get_buffer_addr(struct core_device**core_d)
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

extern struct core_device kdp520_lcdc;

struct display_driver kdp520_lcdc_driver = {
    

    .set_params   = kdp520_lcdc_set_params,
    .get_params   = kdp520_lcdc_get_params,
    .set_camera   = kdp520_lcdc_set_camera,
    .get_buffer   = kdp520_lcdc_get_buffer_addr,
//    .buffer_init   = kdp520_lcdc_buffer_init,
    .init         = kdp520_lcdc_init,
    .attach_panel = kdp520_lcdc_attach_panel,    
    .start        = kdp520_lcdc_start,
    .stop         = kdp520_lcdc_stop,
    .set_source   = kdp520_lcdc_set_source,
    .set_pen      = kdp520_lcdc_set_pen,
    .draw_rect    = kdp520_lcdc_draw_rect,
    .draw_line    = kdp520_lcdc_draw_line,
    .fill_rect    = kdp520_lcdc_fill_rect,
    .draw_bitmap  = kdp520_lcdc_draw_bitmap,
    .fresh        = kdp520_lcdc_fresh
};

#endif

