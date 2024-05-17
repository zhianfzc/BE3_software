#include "board_kl520.h"
#if (CFG_PANEL_TYPE == PANEL_ST7789_240X320) || (CFG_PANEL_TYPE == PANEL_ST7789_320X240) || (CFG_PANEL_TYPE==PANEL_ST7789_240X320_8080_AND_SPI)
#include <stdio.h>
#include <string.h>
#include "base.h"
#include "io.h"
#include "types.h"
#include "board_ddr_table.h"
#include "framework/framework_driver.h"
#include "framework/v2k_image.h"
#include "media/display/display.h"
#include "kdp520_pwm_timer.h"
#include "kdp520_dma.h"
#include "kdp520_gpio.h"
#include "dbg.h"
#include "delay.h"
#include "kl520_include.h"
#include "kl520_api_ssp.h"
#include "utils.h"
#include "kdp520_lcm.h"


osMutexId_t mutex_st7789 = NULL;

#define TE_ENABLE
#define PORTRAIT_DMA_ENABLE

#define FRAME_RATE_60HZ 0
#define FRAME_RATE_39HZ 1
#define FRAME_RATE_CTRL FRAME_RATE_60HZ

#define RAM_ENDIAN_MSB  0
#define RAM_ENDIAN_LSB  1
#define RAM_ENDIAN_TYPE RAM_ENDIAN_MSB

#define LCM_ID          0x8552

/* 
This command is used to transfer data from MCU to frame memory.
-When this command is accepted, the column register and the page register are reset to the start column/start
page positions.
-The start column/start page positions are different in accordance with MADCTL setting.
-Sending any other command can stop frame write.
*/
#define PANEL_REG_RAMWR             0x2C //Memory write
#define PANEL_REG_WRMEMC            0x3C //Write memory continue

#if CFG_PANEL_TYPE == PANEL_ST7789_320X240
    #define ST7789_LANDSCAPE
#elif (CFG_PANEL_TYPE == PANEL_ST7789_240X320) || (CFG_PANEL_TYPE == PANEL_ST7789_240X320_SPI)
    #define ST7789_PORTRAIT
#endif

#ifdef ST7789_PORTRAIT
    //#define	ST7789_PORTRAIT_PARTIAL
    //#define ZOOM_IN_RATIO				3/2
#endif

#ifdef ST7789_LANDSCAPE
#define ST7789_PANEL_WIDTH          QVGA_LANDSCAPE_WIDTH
#define ST7789_PANEL_HEIGHT         QVGA_LANDSCAPE_HEIGHT
#else
#define ST7789_PANEL_WIDTH          QVGA_PORTRAIT_WIDTH
#define ST7789_PANEL_HEIGHT         QVGA_PORTRAIT_HEIGHT
#endif
#define ST7789_PANEL_TOTAL_PIXEL    ST7789_PANEL_WIDTH * ST7789_PANEL_HEIGHT
#define OPEN_DOWN_SCALING
#define BYTES_PER_PIXEL             (2)

#ifdef PORTRAIT_DMA_ENABLE
#undef TE_ENABLE
#undef FRAME_RATE_CTRL
#undef RAM_ENDIAN_TYPE

#define FRAME_RATE_CTRL FRAME_RATE_39HZ
#define RAM_ENDIAN_TYPE RAM_ENDIAN_LSB
#define TE_ENABLE

#endif

#define ST7789_SLPIN      (0x10)
#define ST7789_SLPOUT     (0x11)
#define ST7789_INVOFF     (0x20)
#define ST7789_INVON      (0x21)
#define ST7789_DISPOFF    (0x28)
#define ST7789_DISPON     (0x29)
#define ST7789_CASET      (0x2A)
#define ST7789_RASET      (0x2B)
#define ST7789_RAMWR      (0x2C)
#define ST7789_TEON       (0x35)
#define ST7789_MADCTL     (0x36)
#define ST7789_COLMOD     (0x3A)
#define ST7789_RAMCTRL    (0xB0)
#define ST7789_PORCTRL    (0xB2)
#define ST7789_GCTRL      (0xB7)
#define ST7789_VCOMS      (0xBB)
#define ST7789_LCMCTRL    (0xC0)  //LCM Control
#define ST7789_VDVVRHEN   (0xC2)  //VDV and VRH Command Enable
#define ST7789_VRHS       (0xC3)  //VRH Set
#define ST7789_VDVSET     (0xC4)  //VDV Set
#define ST7789_FRCTR2     (0xC6)  //Frame Rate Control in Normal Mode
#define ST7789_PWCTRL1    (0xD0)   //Power Control 1
#define ST7789_PVGAMCTRL  (0xE0)
#define ST7789_NVGAMCTRL  (0xE1)

static bool mDisplayOn = false;


int _st7789_240x320_init(struct core_device** core_d)
{
    int ret = -1;

    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    struct display_driver_ops *ops = (struct display_driver_ops*)display_drv->ctrller_ops;    

    if(mutex_st7789 == NULL)
        mutex_st7789 = osMutexNew(NULL);

    u32 base = display_drv->base;
//    u16 hor_no_in = display_drv->vi_params.input_xres;
//    u16 ver_no_in = display_drv->vi_params.input_yres;
//    u16 hor_no_out = ST7789_PANEL_WIDTH;//MZT_PANEL_WIDTH;
//    u16 ver_no_out = ST7789_PANEL_HEIGHT;//MZT_PANEL_HEIGHT;
//    dbg_msg_display("[%s] hor_no_in=%u ver_no_in=%u hor_no_out=%u ver_no_out=%u",
//    __func__, hor_no_in, ver_no_in, hor_no_out, ver_no_out);
 
    if (!ops)
        return -1;

    ops->write_cmd(base, 0x11); //Exit Sleep
    //delay_us(100);
    delay_ms(10);

    //-----------Display and color format setting-------------//
    // Column Address Set
    ops->write_cmd(base, 0x2A); 
    // XS
    ops->write_data(base, 0x00);
    ops->write_data(base, 0x00);
    // XE
#ifdef ST7789_LANDSCAPE
    ops->write_data(base, 0x01);
    ops->write_data(base, 0x3F);
#else
    ops->write_data(base, 0x00);
    ops->write_data(base, 0xEF);
#endif

    // Row Address Set
    ops->write_cmd(base, 0x2B);
#ifdef ST7789_LANDSCAPE
    // YS
    ops->write_data(base, 0x00);
    ops->write_data(base, 0x00);
    // YE
    ops->write_data(base, 0x00);
    ops->write_data(base, 0xEF);
#else
    #ifdef ST7789_PORTRAIT_PARTIAL
      // YS
    ops->write_data(base, 0x00);
    ops->write_data(base, 0x40);
    // YE
    ops->write_data(base, 0x01);
    ops->write_data(base, 0x00);
    #else
      // YS
    ops->write_data(base, 0x00);
    ops->write_data(base, 0x00);
    // YE
    ops->write_data(base, 0x01);
    ops->write_data(base, 0x3F);
    #endif
#endif
    
#ifdef ST7789_PORTRAIT_PARTIAL
    ops->write_cmd(base, 0x13);
    ops->write_data(base, 0x00);
    ops->write_cmd(base, 0x12);
    ops->write_data(base, 0xFF);
    ops->write_cmd(base, 0x30);
    ops->write_data(base, 0x00);
    ops->write_data(base, 0x40);
    ops->write_data(base, 0x01);
    ops->write_data(base, 0x00);
#endif

#ifdef TE_ENABLE
    ops->write_cmd(base, 0x35); // Enable Tearing effect
    ops->write_data(base, 0x00);
#endif

    ops->write_cmd(base, 0x36); // Memory Access Control
#ifdef ST7789_LANDSCAPE
    // xy_exchange / y_inverse
    //(Rotate positive 90 degrees)
    //MY = 1, MX = 0, MV = 1, ML = 0
#ifdef CFG_ST7789_X_Y_INVERSE
    ops->write_data(base, 0x60);
#else
    ops->write_data(base, 0xA0);
#endif
#else
    //MY = 1, MX = 1, MV = 0, ML = 0 
    {
#ifdef CFG_ST7789_X_Y_INVERSE
        ops->write_data(base, 0xD0); //image from flash
#else
        ops->write_data(base, 0x00); //image from flash
#endif
    }
#endif

    // Interface Pixel Format
    ops->write_cmd(base, 0x3a);//DPI=101  16bit RGB
    ops->write_data(base, 0x55);
    //ops->write_data(base, 0x05);
    //-------------ST7789V Porch setting-----------//
    ops->write_cmd(base, 0xb2);
    ops->write_data(base, 0x0C); // Back porch (normal)
    ops->write_data(base, 0x0C); // Front porch (normal)
    ops->write_data(base, 0x00); // 
    ops->write_data(base, 0X33); // Back porch (idle) + Front porch (idle)
    ops->write_data(base, 0X33); // Back porch (partial) + Front porch (partial)
    
    // Gate Control. 13.26, -10.43
    ops->write_cmd(base, 0xb7);
    ops->write_data(base, 0x35);//(0x71);
    
    //Setting limitation: VCOMS+VCOMS offset+VDV=0.1V~1.675V.
    // VCOMS Setting. used for feed through voltage compensation
    // 1.175V
    ops->write_cmd(base, 0xBB);
    ops->write_data(base, 0x2b);//(0x25);
    
    // LCM Control ???
    ops->write_cmd(base, 0xC0); 
    ops->write_data(base, 0x2c); 
    
    // VDV and VRH command enable
    ops->write_cmd(base, 0xC2); 
    ops->write_data(base, 0x01);
    
    // VRH Set
    ops->write_cmd(base, 0xC3); 
    ops->write_data(base, 0x11);//(0X14);//
    
    // VDVS Set
    ops->write_cmd(base, 0xC4); 
    ops->write_data(base, 0x20); // 0
    
    // Frame rate control in normal mode
#if FRAME_RATE_CTRL == FRAME_RATE_60HZ
    ops->write_cmd(base, 0xC6); 
    ops->write_data(base, 0x0F);
#elif FRAME_RATE_CTRL == FRAME_RATE_39HZ
    ops->write_cmd(base, 0xC6); 
    ops->write_data(base, 0x1F);
#endif

    // Power Control 1
    ops->write_cmd(base, 0xd0); 
    ops->write_data(base, 0xa4);
    ops->write_data(base, 0xa1); // AVDD : 6.8V, AVCL : -4.6V, VDDS : 2.3 V

    //---------------ST7789V gamma setting-------------//
    { // Positive Voltage gamma control
        ops->write_cmd(base, 0xE0); 
        ops->write_data(base, 0xd0);
        ops->write_data(base, 0x08);
        ops->write_data(base, 0x0e);
        ops->write_data(base, 0x0a);
        ops->write_data(base, 0x0a);
        ops->write_data(base, 0x06);
        ops->write_data(base, 0x38);
        ops->write_data(base, 0x44);
        ops->write_data(base, 0x50);
        ops->write_data(base, 0x29);
        ops->write_data(base, 0x15);
        ops->write_data(base, 0x16);
        ops->write_data(base, 0x33);
        ops->write_data(base, 0x36);
    }
    { // Negative Boltage gamma control
        ops->write_cmd(base, 0XE1); 
        ops->write_data(base, 0xd0);
        ops->write_data(base, 0x07);
        ops->write_data(base, 0x0d);
        ops->write_data(base, 0x09);
        ops->write_data(base, 0x08);
        ops->write_data(base, 0x06);
        ops->write_data(base, 0x33);
        ops->write_data(base, 0x33);
        ops->write_data(base, 0x4d);
        ops->write_data(base, 0x28); 
        ops->write_data(base, 0x16);
        ops->write_data(base, 0x15);
        ops->write_data(base, 0x33);
        ops->write_data(base, 0x35);   
    }

    ops->write_cmd(base, 0x21); // Display Inversion On ??
    
    // RAM Control
#if RAM_ENDIAN_TYPE == RAM_ENDIAN_MSB
    ops->write_cmd(base, 0xB0);
    ops->write_data(base, 0x00); // Ram access from MCU interface
    ops->write_data(base, 0xd0); // Normal Endian (MSB first), 18 bit bus width
#elif RAM_ENDIAN_TYPE == RAM_ENDIAN_LSB
    ops->write_cmd(base, 0xB0);
    ops->write_data(base, 0x00); // Ram access from MCU interface
    ops->write_data(base, 0xd8); // Normal Endian (MSB first), 18 bit bus width    
#endif
    
    //ops->write_cmd(base, 0x29); //Display on
    mDisplayOn = false;
    ops->write_cmd(base, 0x28); //Display off

    //_st7789_240x320_clear(core_d, YELLOW);//WHITE);

    ret = 0;

    return ret;
}

static int _st7789_240x320_clear(struct core_device** core_d, u32 color)
{
    int ret = 0;
    int i;

    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    struct display_driver_ops *ops = (struct display_driver_ops*)display_drv->ctrller_ops;


    if (ops) {
        ops->write_cmd(display_drv->base, PANEL_REG_WRMEMC);

        for(i = 0; i < ST7789_PANEL_TOTAL_PIXEL; ++i)
        {
            ops->write_data(display_drv->base, (u8)((((color & 0xFF00) >>8 ) & 0xFF)));
            ops->write_data(display_drv->base, (u8)(color & 0x00FF));
        }
    }

    return ret;
}

u16 _st7789_240x320_read_display_id(struct core_device** core_d)
{
    u16 id = 0;

    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    struct display_driver_ops *ops = (struct display_driver_ops*)display_drv->ctrller_ops;

    if (ops) {
        u32 base = display_drv->base;
        ops->write_cmd(base, 0x04);
        id = ops->read_data(base);  //dummy read 
        id = ops->read_data(base); 
        id = ((ops->read_data(base) & 0xBF) << 8);
        id |= ops->read_data(base);
    }
    if(id == LCM_ID)
        return id;
    else 
        return 0xFFFF;
}

void _st7789_240x320_start(struct core_device** core_d)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    struct display_driver_ops *ops = (struct display_driver_ops*)display_drv->ctrller_ops;

    if (ops)
        ops->write_cmd(display_drv->base, ST7789_DISPON);
}

void _st7789_240x320_stop(struct core_device **core_d)
{
    struct display_driver* display_drv = container_of(core_d, struct display_driver, core_dev);
    struct display_driver_ops *ops = (struct display_driver_ops*)display_drv->ctrller_ops;
    if (ops) {
        ops->write_cmd(display_drv->base, ST7789_DISPOFF);
        mDisplayOn = false;
    }
}

void _st7789_240x320_update(struct core_device** core_d, u32 addr)
{
    struct display_driver* display_drv = container_of(core_d, struct display_driver, core_dev);
    struct display_driver_ops *ops = (struct display_driver_ops*)display_drv->ctrller_ops;
    u32 base = display_drv->base;
    u32 readAddr = addr;
    u16 lcm_w = ST7789_PANEL_WIDTH;
    
#ifdef ST7789_PORTRAIT_PARTIAL
    u16 lcm_h = 192;//ST7789_PANEL_HEIGHT;
#else
    u16 lcm_h = ST7789_PANEL_HEIGHT;
#endif

    if (!ops) return;

    osMutexAcquire(mutex_st7789, osWaitForever);
    
#if 0//ndef ST7789_PORTRAIT_PARTIAL
    //home point : (0, 0)
    ops->write_cmd(base, 0x2A);
    ops->write_data(base, 0x00);
    ops->write_data(base, 0x00);
    ops->write_data(base, 0x00);
    ops->write_data(base, 0xEF);
    ops->write_cmd(base, 0x2B);
    ops->write_data(base, 0x00);
    ops->write_data(base, 0x00);
    ops->write_data(base, 0x01);
    ops->write_data(base, 0x3F);
#endif

    osThreadId_t tid = osThreadGetId();
    osPriority_t save_pri = osThreadGetPriority(tid);

    ops->write_cmd(base, PANEL_REG_RAMWR);
    
    kdp520_lcm_set_cs(0);

    int te_pin = kdp520_lcm_get_te_pin();
    kdp520_gpio_setedgemode( 1 << te_pin, 0);
    
    while (kdp_dma_is_ch_busy(AHBDMA_Channel6));
    kdp_dma_reset_ch(AHBDMA_Channel6);
    kdp_dma_clear_interrupt(AHBDMA_Channel6);
    kdp_dma_enable_dma_int();
    kdp_dma_init(0,0,0);

    osThreadSetPriority(tid, (osPriority_t)(save_pri+1));
    
    kdp520_gpio_enableint( te_pin, GPIO_EDGE, FALSE);
    kdp520_wait_gpio_int(1<<te_pin);    
    kdp520_gpio_disableint(te_pin);

            
    kdp_dma_normal_mode(AHBDMA_Channel6,(UINT32)(readAddr),(UINT32)0xC340020c, lcm_w*lcm_h*2 ,2,0,4,0,2,3,0,0); //init DMA and wait TE signal
    kdp_dma_enable_ch(AHBDMA_Channel6);
    kdp_dma_wait_dma_int(AHBDMA_Channel6);
    kdp_dma_disable_ch(AHBDMA_Channel6); 
    kdp520_lcm_set_cs(1);

    osThreadSetPriority(tid, save_pri);

    osMutexRelease(mutex_st7789);
   
    if(mDisplayOn == false){
        ops->write_cmd(base, ST7789_DISPON); //Display on
        mDisplayOn = true;
    }
}

void _st7789_240x320_preproc_nir(struct core_device** core_d, u32 addr, u32 dest_addr)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    struct display_driver_ops *ops = (struct display_driver_ops*)display_drv->ctrller_ops;

    u32 readAddr;
    u32 writeAddr;
    u16 data_buf;

    u32 bytes_per_pixel = 1;    // raw 8

    u16 src_w = display_drv->vi_params.src_width;

    u16 dp_img_w = display_drv->vi_params.dp_area_w;
    u16 dp_img_h = display_drv->vi_params.dp_area_h;

    u16 dp_h = display_drv->vi_params.dp_out_h;
    u16 dp_w = display_drv->vi_params.dp_out_w;
    
    if (!ops) return;

    u32 orgin_x = DISPLAY_NIR_X_OFFSET;
    for(u32 index =0; index<dp_h; index++)
    {
        readAddr = (((index * dp_img_h / dp_h + DISPLAY_NIR_Y_OFFSET) * src_w + orgin_x) * bytes_per_pixel) + addr;
        writeAddr = index * dp_w * 2 + dest_addr;
        
        for (u32 col = 0; col < dp_w; col++)
        {
            data_buf = (u16)inb(readAddr + ((col * dp_img_w) / dp_w * bytes_per_pixel));
            data_buf = (u16)((data_buf>>3)<<11) | ((data_buf>>2)<<5) | ((data_buf>>3)<<0);
            outhw(writeAddr,data_buf);

            writeAddr += 2;
        }
    }
}

void _st7789_240x320_preproc_rgb(struct core_device** core_d, u32 addr, u32 dest_addr)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    struct display_driver_ops *ops = (struct display_driver_ops*)display_drv->ctrller_ops;

    if (!ops) return;

    u32 readAddr;
    u32 writeAddr;
    u32 bytes_per_pixel = 2;    // rgb565

    u16 src_w = display_drv->vi_params.src_width;
    
    u16 dp_img_w = display_drv->vi_params.dp_area_w;
    u16 dp_img_h = display_drv->vi_params.dp_area_h;
    
    u16 dp_w = display_drv->vi_params.dp_out_w;
    u16 dp_h = display_drv->vi_params.dp_out_h;

    

    if ((dp_img_h != dp_h) || (dp_img_w != dp_w))
    {
        u32 orgin_x = DISPLAY_RGB_X_OFFSET;
        for(u32 index =0; index<dp_h; index++)
        {
            readAddr = (((index * dp_img_h / dp_h + DISPLAY_RGB_Y_OFFSET) * src_w + orgin_x) * bytes_per_pixel) + addr;
            writeAddr = index * dp_w * bytes_per_pixel + dest_addr;
            
            for (u32 col = 0; col < dp_w; col++)
            {
                *((u16 *)writeAddr) = *((u16 *)(readAddr + ((col * dp_img_w) / dp_w * bytes_per_pixel)));
                writeAddr += bytes_per_pixel;
            }
        }
    }
    else
    {
        readAddr = addr;
        writeAddr = dest_addr;

        while (kdp_dma_is_ch_busy(AHBDMA_Channel4));
        kdp_dma_reset_ch(AHBDMA_Channel4);
        kdp_dma_clear_interrupt(AHBDMA_Channel4);
        kdp_dma_enable_dma_int();
        kdp_dma_init(0,0,0);

        readAddr += DISPLAY_RGB_X_OFFSET * bytes_per_pixel + DISPLAY_RGB_Y_OFFSET * src_w *bytes_per_pixel;
        
        for(u32 index =0; index<dp_h; index++)
        {
            while (kdp_dma_is_ch_busy(AHBDMA_Channel4));
            kdp_dma_normal_mode(AHBDMA_Channel4,(UINT32)(readAddr),(UINT32)(writeAddr), dp_w*bytes_per_pixel ,1,1,2,0,0,3,0,0); //init DMA and wait TE signal
            kdp_dma_enable_ch(AHBDMA_Channel4);
            kdp_dma_wait_dma_int(AHBDMA_Channel4);
            kdp_dma_disable_ch(AHBDMA_Channel4);       
            readAddr += src_w*bytes_per_pixel;
            writeAddr += dp_w*2;
        }
    }
}

struct panel_driver st7789_240x320_driver = {
    .init           = _st7789_240x320_init,
    .clear          = _st7789_240x320_clear,
    .read_did       = _st7789_240x320_read_display_id,
    .start          = _st7789_240x320_start,
    .stop           = _st7789_240x320_stop,
    .preproc_rgb    = _st7789_240x320_preproc_rgb,
    .preproc_nir    = _st7789_240x320_preproc_nir,
    .update         = _st7789_240x320_update,
};


#endif

