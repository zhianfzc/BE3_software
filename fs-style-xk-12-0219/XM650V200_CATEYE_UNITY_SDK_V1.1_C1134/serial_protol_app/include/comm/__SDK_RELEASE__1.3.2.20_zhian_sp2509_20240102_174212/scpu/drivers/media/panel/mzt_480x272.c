#include "board_kl520.h"
#if CFG_PANEL_TYPE == PANEL_MZT_480X272
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base.h"
#include "types.h"
#include "framework/framework_driver.h"
#include "framework/v2k_image.h"
#include "kdp520_dma.h"
#include "media/display/lcdc.h"
#include "media/display/display.h"
#include "kl520_include.h"
#include "utils.h"
#include "dbg.h"

#define OPEN_DOWN_SCALING


int mzt_480x272_init(struct core_device** core_d)
{
    int ret = 0;
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    
    u16 hor_no_in = display_drv->vi_params.panel_in_w;
    u16 ver_no_in = display_drv->vi_params.panel_in_h;
    
    u16 hor_no_out = display_drv->vi_params.panel_out_w;//MZT_PANEL_WIDTH;
    u16 ver_no_out = display_drv->vi_params.panel_out_h;//MZT_PANEL_HEIGHT;
    
#ifdef DOWN_SCALE_WITH_CROP
    //pick 640x360 from 640x480
    ver_no_in = (u16)((float)hor_no_in * (float)ver_no_out / (float)hor_no_out );
#endif

    dbg_msg_display("[%s] hor_no_in=%u ver_no_in=%u hor_no_out=%u ver_no_out=%u",
    __func__, hor_no_in, ver_no_in, hor_no_out, ver_no_out);
    LCDC_REG_FUNC_ENABLE_SET_LCDon(0);
    LCDC_REG_FUNC_ENABLE_SET_LCDen(0);

    //for DOWN_SCALING
    if ((hor_no_in != hor_no_out) || (ver_no_in != ver_no_out))
    {
        LCDC_REG_FUNC_ENABLE_SET_TVEn(0);
        LCDC_REG_FUNC_ENABLE_SET_ScalerEn(1);    
    }

    //LCDC_REG_FUNC_ENABLE_SET_ScalerEn(1);
    LCDC_REG_PANEL_PIXEL_SET_UpdateSrc(0); //image0

    kdp520_lcdc_set_panel_type(lcdc_6bits_per_channel);
    kdp520_lcdc_set_bgrsw(lcdc_output_fmt_sel_rgb); //RGB
    //LCDC_REG_PATGEN_SET_Img0PatGen(0);

    //Pixel Clock = H total * V total * Frame Rate
    //horizontal timing control
    LCDC_REG_HORIZONTAL_TIMING_CTRL_SET_HBP(1);
    LCDC_REG_HORIZONTAL_TIMING_CTRL_SET_HFP(1);
    LCDC_REG_HORIZONTAL_TIMING_CTRL_SET_HW(40);  
    LCDC_REG_HORIZONTAL_TIMING_CTRL_SET_PL(((hor_no_out >> 4) - 1));
    //LCDC_REG_HORIZONTAL_TIMING_CTRL_SET_PL(((MZT_PANEL_WIDTH >> 4) - 1));      
    
    //vertical timing control
    LCDC_REG_VERTICAL_TIMING_CTRL_SET_VFP(2);
    LCDC_REG_VERTICAL_TIMING_CTRL_SET_VW(9);
    LCDC_REG_VERTICAL_TIMING_CTRL_SET_LF((ver_no_out - 1));
    //LCDC_REG_VERTICAL_TIMING_CTRL_SET_LF((MZT_PANEL_HEIGHT - 1));    

    //vertical back porch
    LCDC_REG_VERTICAL_BACK_PORCH_SET_VBP(2);

    LCDC_REG_POLARITY_CTRL_SET_DivNo(5);
    LCDC_REG_POLARITY_CTRL_SET_IDE(1);
    LCDC_REG_POLARITY_CTRL_SET_ICK(1);
    LCDC_REG_POLARITY_CTRL_SET_IHS(1);
    LCDC_REG_POLARITY_CTRL_SET_IVS(1);

    //serial panel pixel
    LCDC_REG_SERIAL_PANEL_PIXEL_SET_AUO052(0);
    LCDC_REG_SERIAL_PANEL_PIXEL_SET_LSR(0);
    LCDC_REG_SERIAL_PANEL_PIXEL_SET_ColorSeq(2);
    LCDC_REG_SERIAL_PANEL_PIXEL_SET_DeltaType(0);
    LCDC_REG_SERIAL_PANEL_PIXEL_SET_SerialMode(0);

    //TV control
    LCDC_REG_TV_SET_ImgFormat(0);
    LCDC_REG_TV_SET_PHASE(0);
    LCDC_REG_TV_SET_OutFormat(0);

    kdp520_lcdc_set_framerate(60, hor_no_out, ver_no_out);

    //LCD Image Color Management
    outw(LCDC_REG_COLOR_MGR_0, 0x2000);
    outw(LCDC_REG_COLOR_MGR_1, 0x2000);
    outw(LCDC_REG_COLOR_MGR_2, 0x0);
    outw(LCDC_REG_COLOR_MGR_3, 0x40000);
    //LCDC_REG_COLOR_MGR_3_SET_Contr_slope(4);

    //FrameBuffer Parameter
    outw(LCDC_REG_FRAME_BUFFER, 0x0);

    //Ignore some transformation
    outw(LCDC_REG_BANDWIDTH_CTRL, 0x0);

    //for DOWN_SCALING
    if ((hor_no_in != hor_no_out) || (ver_no_in != ver_no_out)) {
        LCDC_REG_SCALER_HOR_RES_IN_SET_hor_no_in((hor_no_in - 1)); 
        LCDC_REG_SCALER_VER_RES_IN_SET_ver_no_in((ver_no_in - 1));
        LCDC_REG_SCALER_HOR_RES_OUT_SET_hor_no_out(hor_no_out);
        LCDC_REG_SCALER_VER_RES_OUT_SET_ver_no_out(ver_no_out);
        LCDC_REG_SCALER_MISC_SET_fir_sel(0);
        LCDC_REG_SCALER_MISC_SET_hor_inter_mode(0);
        LCDC_REG_SCALER_MISC_SET_ver_inter_mode(0);
        LCDC_REG_SCALER_MISC_SET_bypass_mode(0);
        //u32 scal_hor_num = mod((hor_no_in + 1) / hor_no_out) * 256 / hor_no_out;
        //u32 scal_ver_num = mod((ver_no_in + 1) / ver_no_out) * 256 / ver_no_out;
        LCDC_REG_SCALER_RES_SET_scal_hor_num(85);
#ifdef DOWN_SCALE_WITH_CROP
        //keep ratio but with cropping
        LCDC_REG_SCALER_RES_SET_scal_ver_num(85);
#else
        //all are display but ratio is changed
        LCDC_REG_SCALER_RES_SET_scal_ver_num(195);
#endif
    }
    else {
        //reset the scalar control register	
        outw(LCDC_REG_SCALER_MISC, 0x0);
    }

    return ret;
}

int mzt_480x272_clear(struct core_device** core_d, u32 color)
{
    int ret = 0;

    return ret;
}

u16 mzt_480x272_read_display_id(struct core_device** core_d)
{
    return 0;
}

void mzt_480x272_preproc_rgb(struct core_device** core_d, u32 src_addr, u32 dest_addr)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);

    u16 src_width = display_drv->vi_params.src_width;

    u16 dp_img_w = display_drv->vi_params.dp_area_w;
    u16 dp_img_h = display_drv->vi_params.dp_area_h;
    
    u16 dp_h = display_drv->vi_params.dp_out_h;
    u16 dp_w = display_drv->vi_params.dp_out_w;

    u32 readAddr = src_addr;
    u32 writeAddr = dest_addr;    
    
    u32 bytes_per_pixel = 2;    // rgb565

//    if ((dp_img_h % dp_h) || (dp_img_w % dp_w))
    {
        for(u32 index =0; index<dp_h; index++)
        {
            readAddr = (((index * dp_img_h / dp_h + DISPLAY_RGB_Y_OFFSET) * src_width + DISPLAY_RGB_X_OFFSET) * bytes_per_pixel) + src_addr;
            writeAddr = index * dp_w * bytes_per_pixel + dest_addr;
            for (u32 col = 0; col < dp_w; col++)
            {
                *((u16 *)writeAddr) = *((u16 *)(readAddr + ((col * dp_img_w) / dp_w * bytes_per_pixel)));
                writeAddr += bytes_per_pixel;
            }
        }
    }
//    else
//    {
//        while (kdp_dma_is_ch_busy(AHBDMA_Channel4));
//        kdp_dma_reset_ch(AHBDMA_Channel4);
//        kdp_dma_clear_interrupt(AHBDMA_Channel4);
//        kdp_dma_enable_dma_int();
//        kdp_dma_init(0,0,0);
//        
//        while (kdp_dma_is_ch_busy(AHBDMA_Channel4));
//        kdp_dma_normal_mode(AHBDMA_Channel4,(UINT32)(readAddr),(UINT32)(writeAddr), dp_w*dp_h*bytes_per_pixel ,1,1,2,0,0,3,0,0); //init DMA and wait TE signal
//        kdp_dma_enable_ch(AHBDMA_Channel4);
//        kdp_dma_wait_dma_int(AHBDMA_Channel4);
//        kdp_dma_disable_ch(AHBDMA_Channel4);
//    }
}

void mzt_480x272_posproc_rgb(struct core_device** core_d, u32 src_addr, u32 dest_addr)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);    
    
    u32 index = 0;    

    u32 dp_w = display_drv->vi_params.dp_out_w;                    //display
    u32 dp_h = display_drv->vi_params.dp_out_h;                  //display

    u32 scr_img_w = display_drv->vi_params.panel_in_w;

    u32 screen_in_w = display_drv->vi_params.panel_in_w;
    //u32 scr_img_h = display_drv->vi_params.panel_in_h;

    u32 readAddr = src_addr;
    u32 writeAddr = dest_addr;    
    
    u32 bytes_per_pixel = 2;    // rgb565
    
    //writeAddr += (screen_in_w-dp_w)/2*bytes_per_pixel;  //align center

    writeAddr += (PANEL_RGB_Y_OFFSET*scr_img_w)*bytes_per_pixel;  //align center		
    writeAddr += (PANEL_RGB_X_OFFSET)*bytes_per_pixel;  //align center			

    while (kdp_dma_is_ch_busy(AHBDMA_Channel4));
    kdp_dma_reset_ch(AHBDMA_Channel4);
    kdp_dma_clear_interrupt(AHBDMA_Channel4);
    kdp_dma_enable_dma_int();
    kdp_dma_init(0,0,0);   

    for(index =0; index<dp_h; index++)
    {  
        while (kdp_dma_is_ch_busy(AHBDMA_Channel4));
        kdp_dma_normal_mode(AHBDMA_Channel4,(UINT32)(readAddr),(UINT32)(writeAddr), dp_w*bytes_per_pixel ,1,1,2,0,0,3,0,0); //init DMA and wait TE signal
        kdp_dma_enable_ch(AHBDMA_Channel4);
        kdp_dma_wait_dma_int(AHBDMA_Channel4);
        kdp_dma_disable_ch(AHBDMA_Channel4);      
       
        readAddr += dp_w*bytes_per_pixel;
        writeAddr += screen_in_w*bytes_per_pixel;  
    }
}

void mzt_480x272_preproc_nir(struct core_device** core_d, u32 src_addr, u32 dest_addr)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    
    u16 src_width = display_drv->vi_params.src_width;
    u16 dp_img_w = display_drv->vi_params.dp_area_w;
    u16 dp_img_h = display_drv->vi_params.dp_area_h;
    u16 dp_h = display_drv->vi_params.dp_out_h;
    u16 dp_w = display_drv->vi_params.dp_out_w;
    
    u16 data_buf;
    u32 readAddr;
    u32 writeAddr;    
    
    u32 bytes_per_pixel = 1;    // raw8
    u16 nByteDisplay;
	
//	if ( display_drv->vi_params.src_fmt == V2K_PIX_FMT_RAW8 )
//	{
//		nByteDisplay = 1;
//	}
//	else
	{
		nByteDisplay = 2;
	}
	
    //if ((dp_img_h % dp_h) || (dp_img_w % dp_w))
    {
        for(u32 index =0; index<dp_h; index++)
        {
            readAddr = ((((index * dp_img_h) / dp_h + DISPLAY_NIR_Y_OFFSET) * src_width + DISPLAY_NIR_X_OFFSET) * bytes_per_pixel) + src_addr;
            writeAddr = (index * dp_w * nByteDisplay) + dest_addr;
            
            for (u32 col = 0; col < dp_w; col++)
            {
                data_buf = inb(readAddr + ((col * dp_img_w) / dp_w * bytes_per_pixel));
//				if ( display_drv->vi_params.src_fmt == V2K_PIX_FMT_RAW8 )
//				{
//					outb(writeAddr,data_buf);
//				}
//				else
				{
					data_buf = (u16)((data_buf>>3)<<11) | ((data_buf>>2)<<5) | ((data_buf>>3)<<0);
                	outhw(writeAddr,data_buf);
				}

                writeAddr += nByteDisplay;
            }
        }
    }
}

void mzt_480x272_posproc_nir(struct core_device** core_d, u32 src_addr, u32 dest_addr)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);    
    
    u32 index = 0;    

    u32 scr_img_w = display_drv->vi_params.panel_in_w;
    u32 dp_w = display_drv->vi_params.dp_out_w;                    //display
    u32 dp_h = display_drv->vi_params.dp_out_h;                  //display

    u32 readAddr = src_addr;
    u32 writeAddr = dest_addr;

	u32 bytes_per_pixel;
	
//	if ( display_drv->vi_params.src_fmt == V2K_PIX_FMT_RAW8 )
//	{
//		bytes_per_pixel = 1;    // raw8
//	}
//	else
	{
		bytes_per_pixel = 2;    // rgb565
	}
   
    writeAddr += (PANEL_NIR_Y_OFFSET*scr_img_w)*bytes_per_pixel;  //align center		
    writeAddr += (PANEL_NIR_X_OFFSET)*bytes_per_pixel;  //align center		

    while (kdp_dma_is_ch_busy(AHBDMA_Channel4));
    kdp_dma_reset_ch(AHBDMA_Channel4);
    kdp_dma_clear_interrupt(AHBDMA_Channel4);
    kdp_dma_enable_dma_int();
    kdp_dma_init(0,0,0);   

    for(index =0; index<dp_h; index++)
    {
        while (kdp_dma_is_ch_busy(AHBDMA_Channel4));
//		if ( display_drv->vi_params.src_fmt == V2K_PIX_FMT_RAW8 )
//		{
//			kdp_dma_normal_mode(AHBDMA_Channel4,(UINT32)(readAddr),(UINT32)(writeAddr), dp_w*bytes_per_pixel ,0,0,2,0,0,3,0,0); //init DMA and wait TE signal
//		}
//		else
		{
			kdp_dma_normal_mode(AHBDMA_Channel4,(UINT32)(readAddr),(UINT32)(writeAddr), dp_w*bytes_per_pixel ,1,1,2,0,0,3,0,0); //init DMA and wait TE signal
		}    
		kdp_dma_enable_ch(AHBDMA_Channel4);
        kdp_dma_wait_dma_int(AHBDMA_Channel4);
        kdp_dma_disable_ch(AHBDMA_Channel4);      
      
        readAddr += dp_w*bytes_per_pixel;
        writeAddr += scr_img_w*bytes_per_pixel;  
    }
}

struct panel_driver mzt_480x272_driver = {
    .init           = mzt_480x272_init,
    .clear          = mzt_480x272_clear,
    .read_did       = mzt_480x272_read_display_id,
    .preproc_rgb    = mzt_480x272_preproc_rgb,
    .posproc_rgb    = mzt_480x272_posproc_rgb,
    .preproc_nir    = mzt_480x272_preproc_nir,
    .posproc_nir    = mzt_480x272_posproc_nir,
};

#endif
