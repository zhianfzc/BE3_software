#include "board_kl520.h"
#if (CFG_PANEL_TYPE == PANEL_ST7789_240X320_SPI) || (CFG_PANEL_TYPE==PANEL_ST7789_240X320_8080_AND_SPI)
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


osMutexId_t mutex_st7789v2_spi_lcd = NULL;

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


#define ST7789V2_SPI_LCD_PORTRAIT


#ifdef ST7789V2_SPI_LCD_LANDSCAPE
#define ST7789V2_SPI_LCD_PANEL_WIDTH          QVGA_LANDSCAPE_WIDTH
#define ST7789V2_SPI_LCD_PANEL_HEIGHT         QVGA_LANDSCAPE_HEIGHT
#else
#define ST7789V2_SPI_LCD_PANEL_WIDTH          QVGA_PORTRAIT_WIDTH
#define ST7789V2_SPI_LCD_PANEL_HEIGHT         QVGA_PORTRAIT_HEIGHT
#endif
#define ST7789V2_SPI_LCD_PANEL_TOTAL_PIXEL    ST7789V2_SPI_LCD_PANEL_WIDTH * ST7789V2_SPI_LCD_PANEL_HEIGHT
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


//extern void delay_ms(unsigned int count);
//volatile dma_lld LinkAddr0[320];
//static int _st7789v2_spi_240x320_clear(struct core_device** core_d, u32 color);

static bool mDisplayOn = false;


#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
extern void kl520_api_ssp_spi_master_init(void);
static void _st7789v2_spi_240x320_init_set_reg(struct display_driver_ops* ops);
static int _st7789v2_spi_240x320_init(struct core_device** core_d)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    struct display_driver_ops *ops = (struct display_driver_ops*)display_drv->ctrller_ops;

    if(mutex_st7789v2_spi_lcd == NULL)
        mutex_st7789v2_spi_lcd = osMutexNew(NULL);

    if (!ops)
        return -1;

    kl520_api_ssp_lcd_clock_init(1);
    _st7789v2_spi_240x320_init_set_reg(ops);
    kl520_api_ssp_set_display_size(0,0,239,319);
    mDisplayOn = false;

    return 0;
}
static void _st7789v2_spi_240x320_init_set_reg(struct display_driver_ops* ops)
{
    ops->write_cmd_data_single(ST7789_SLPOUT,NULL);
    delay_ms(150);

    //-----------------------------Display setting--------------------------------
    ops->write_cmd_data(ST7789_MADCTL, 1, 0x00);
    ops->write_cmd_data(ST7789_TEON, 1, 0x00);
    ops->write_cmd_data(ST7789_COLMOD, 1, 0x55); //Interface pixel format Pg 224
    ops->write_cmd_data(ST7789_RAMCTRL, 2, 0x00,0x50);

    ops->write_cmd_data(ST7789_CASET, 4, 0x00,0x00,0x00,0xEF);
    ops->write_cmd_data(ST7789_RASET, 4, 0x00,0x00,0x01,0x3f);
#ifdef CFG_ST7789_COLOR_INVERSE
    ops->write_cmd_data_single(ST7789_INVON, NULL);
#endif

#ifdef CFG_ST7789_X_Y_INVERSE
    ops->write_cmd_data(ST7789_MADCTL, 1, 0xD0); //image from flash
#else
    ops->write_cmd_data(ST7789_MADCTL, 1, 0x00); //image from flash
#endif

    //------------------------- Frame rate setting-------------------
    ops->write_cmd_data(ST7789_PORCTRL, 5, 0x0C,0x0C,0x00,0x33,0x33);	// Porch Control
    ops->write_cmd_data(ST7789_GCTRL, 1, 0x35);	//Gate Control

    //----------------------- Power setting-------------------
    ops->write_cmd_data(ST7789_VCOMS, 1, 0x2b);	//VCOM Setting
    ops->write_cmd_data(ST7789_LCMCTRL, 1, 0x2C);  //LCM Control
    ops->write_cmd_data(ST7789_VDVVRHEN, 1, 0x01);  //VDV and VRH Command Enable
    ops->write_cmd_data(ST7789_VRHS, 1, 0x11);  //VRH Set
    ops->write_cmd_data(ST7789_VDVSET, 1, 0x20);  //VDV Set
    ops->write_cmd_data(ST7789_FRCTR2, 1, 0x1F);  //Frame Rate Control in Normal Mode
    ops->write_cmd_data(ST7789_PWCTRL1, 2, 0xa4,0xa1);   //Power Control 1

    // Set_Gamma: //  Is this a goto?
    ops->write_cmd_data(ST7789_PVGAMCTRL, 14, 0xd0, 0x08, 0x0e, 0x0a, 0x0a, 0x06, 0x38, 0x44, 0x50, 0x29, 0x15, 0x16, 0x33, 0x36);
    ops->write_cmd_data(ST7789_NVGAMCTRL, 14, 0xd0, 0x07, 0x0d, 0x09, 0x08, 0x06, 0x33, 0x33, 0x4d, 0x28, 0x16, 0x15, 0x33, 0x35);
    ops->write_cmd_data(ST7789_CASET, 4, 0x00,0x00,0x00,0xEF);
    ops->write_cmd_data(ST7789_RASET, 4, 0x00,0x00,0x01,0x3f);

    ops->write_cmd_data_single(ST7789_RAMWR, NULL); //Display on
    ops->write_cmd_data_single(ST7789_SLPOUT, NULL); //Display on
    osDelay(120);
    ops->write_cmd_data_single(ST7789_DISPON, NULL); //Display on
}
#endif

static int _st7789v2_spi_240x320_clear(struct core_device** core_d, u32 color)
{
#if (CFG_PANEL_TYPE == PANEL_ST7789_240X320_SPI) || (CFG_PANEL_TYPE==PANEL_ST7789_240X320_8080_AND_SPI)
    return 0;
#else
    int ret = 0;
    int i;

    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    struct display_driver_ops *ops = (struct display_driver_ops*)display_drv->ctrller_ops;


    if (ops) {
        ops->write_cmd(display_drv->base, PANEL_REG_WRMEMC);

        for(i = 0; i < ST7789V2_SPI_LCD_PANEL_TOTAL_PIXEL; ++i)
        {
            ops->write_data(display_drv->base, (u8)((((color & 0xFF00) >>8 ) & 0xFF)));
            ops->write_data(display_drv->base, (u8)(color & 0x00FF));
        }
    }

    return ret;
#endif
}

u16 _st7789v2_spi_240x320_read_display_id(struct core_device** core_d)
{
    u16 id = 0;

    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    struct display_driver_ops *ops = (struct display_driver_ops*)display_drv->ctrller_ops;

    if (ops) {
        u8 buf[4];
        ops->read_data_with_buf(0xDA, 4, buf);//work around for dummy read...
        ops->read_data_with_buf(0xDB, 4, buf);
        id |= buf[0]<<8;
        ops->read_data_with_buf(0xDC, 4, buf);
        id |= buf[0];
    }
    if(id == LCM_ID)
        return id;
    else 
        return 0xFFFF;
}

void _st7789v2_spi_240x320_start(struct core_device** core_d)
{
    struct display_driver *display_drv = container_of(core_d, struct display_driver, core_dev);
    struct display_driver_ops *ops = (struct display_driver_ops*)display_drv->ctrller_ops;

    if (ops)
        ops->write_cmd_data_single(ST7789_DISPON, NULL);
}

void _st7789v2_spi_240x320_stop(struct core_device **core_d)
{
    struct display_driver* display_drv = container_of(core_d, struct display_driver, core_dev);
    struct display_driver_ops *ops = (struct display_driver_ops*)display_drv->ctrller_ops;
    if (ops) {
        ops->write_cmd_data_single(ST7789_DISPOFF, NULL);

        //work around for ssp dma/normal mixed method...
        osDelay(5);
        kl520_api_ssp_spi_master_init();

        mDisplayOn = false;
    }
}

void _st7789v2_spi_240x320_update(struct core_device** core_d, u32 addr)
{
    struct display_driver* display_drv = container_of(core_d, struct display_driver, core_dev);
    struct display_driver_ops *ops = (struct display_driver_ops*)display_drv->ctrller_ops;

    u32 readAddr = addr;
    u16 lcm_w = ST7789V2_SPI_LCD_PANEL_WIDTH;
    u16 lcm_h = ST7789V2_SPI_LCD_PANEL_HEIGHT;

    if (!ops) return;

    osMutexAcquire(mutex_st7789v2_spi_lcd, osWaitForever);

    osThreadId_t tid = osThreadGetId();
    osPriority_t save_pri = osThreadGetPriority(tid);
    osThreadSetPriority(tid, (osPriority_t)(save_pri+1));

    int te_pin = kdp520_lcm_get_te_pin();
    kdp520_gpio_enableint( te_pin, GPIO_EDGE, TRUE);
    kdp520_wait_gpio_int(1<<te_pin);    
    kdp520_gpio_disableint(te_pin);
    ops->write_img_buf_data(lcm_w*lcm_h*2,(void*)readAddr);

    osThreadSetPriority(tid, save_pri);

    osMutexRelease(mutex_st7789v2_spi_lcd);
   
    if(mDisplayOn == false){
        ops->write_cmd_data_single(ST7789_DISPON, NULL); //Display on
        mDisplayOn = true;
    }
}

void _st7789v2_spi_240x320_preproc_nir(struct core_device** core_d, u32 addr, u32 dest_addr)
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

void _st7789v2_spi_240x320_preproc_rgb(struct core_device** core_d, u32 addr, u32 dest_addr)
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

    

    if ((dp_img_h % dp_h) || (dp_img_w % dp_w))
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

struct panel_driver st7789v2_spi_240x320_driver = {
    .init           = _st7789v2_spi_240x320_init,
    .clear          = _st7789v2_spi_240x320_clear,
    .read_did       = _st7789v2_spi_240x320_read_display_id,
    .start          = _st7789v2_spi_240x320_start,
    .stop           = _st7789v2_spi_240x320_stop,
    .preproc_rgb    = _st7789v2_spi_240x320_preproc_rgb,
    .preproc_nir    = _st7789v2_spi_240x320_preproc_nir,
    .update         = _st7789v2_spi_240x320_update,
};
#endif
