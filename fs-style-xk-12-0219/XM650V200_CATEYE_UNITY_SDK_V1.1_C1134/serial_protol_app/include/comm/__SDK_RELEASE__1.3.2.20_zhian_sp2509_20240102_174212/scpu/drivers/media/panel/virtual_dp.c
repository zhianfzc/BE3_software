#include "board_kl520.h"
#if CFG_PANEL_TYPE == PANEL_NULL
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

int virtual_dp_init(struct core_device** core_d)
{
    int ret = 0;
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);

    dbg_msg_display("[%s] hor_no_in=%u ver_no_in=%u hor_no_out=%u ver_no_out=%u", \
                    __func__, display_drv->vi_params.panel_in_w, display_drv->vi_params.panel_in_h, display_drv->vi_params.panel_out_w, display_drv->vi_params.panel_out_h);

    return ret;
}

int virtual_dp_clear(struct core_device** core_d, u32 color)
{
    int ret = 0;

    return ret;
}

u16 virtual_dp_read_display_id(struct core_device** core_d)
{
    return 0x5566;
}

void virtual_dp_preproc_rgb(struct core_device** core_d, u32 src_addr, u32 dest_addr)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);

    u32 readAddr = src_addr;
    u32 writeAddr = dest_addr;
    u32 bytes_per_pixel = 2;

    u16 src_width = display_drv->vi_params.src_width;

    u16 dp_img_w = display_drv->vi_params.dp_area_w;
    u16 dp_img_h = display_drv->vi_params.dp_area_h;
    
    u16 dp_h = display_drv->vi_params.dp_out_h;
    u16 dp_w = display_drv->vi_params.dp_out_w;

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
}

void virtual_dp_preproc_nir(struct core_device** core_d, u32 src_addr, u32 dest_addr)
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
    
    {
        for(u32 index =0; index<dp_h; index++)
        {
            readAddr = ((((index * dp_img_h) / dp_h + DISPLAY_NIR_Y_OFFSET) * src_width + DISPLAY_NIR_X_OFFSET) * bytes_per_pixel) + src_addr;
            writeAddr = (index * dp_w * 2) + dest_addr;
            
            for (u32 col = 0; col < dp_w; col++)
            {
                data_buf = (u16)inb(readAddr + ((col * dp_img_w) / dp_w * bytes_per_pixel));
                data_buf = (u16)((data_buf>>3)<<11) | ((data_buf>>2)<<5) | ((data_buf>>3)<<0);
                outhw(writeAddr,data_buf);

                writeAddr += 2;
            }
        }
    }
}

struct panel_driver virtual_dp_driver = {
    .init           = virtual_dp_init,
    .clear          = virtual_dp_clear,
    .read_did       = virtual_dp_read_display_id,
    .preproc_rgb    = virtual_dp_preproc_rgb,
    .preproc_nir    = virtual_dp_preproc_nir,
};

#endif
