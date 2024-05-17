#include "board_kl520.h"
#if (IMGSRC_0_TYPE == SENSOR_TYPE_SP2509_R) || \
    (IMGSRC_1_TYPE == SENSOR_TYPE_SP2509_R) || \
    (IMGSRC_0_TYPE == SENSOR_TYPE_SP2509_L) || \
    (IMGSRC_1_TYPE == SENSOR_TYPE_SP2509_L)
#include <stdlib.h>
#include "base.h"
#include "framework/init.h"
#include "framework/v2k.h"
#include "framework/v2k_image.h"
#include "framework/framework_errno.h"
#include "framework/framework_driver.h"
#include "media/camera/sensor.h"
#include "kdp520_i2c.h"
#include "kdp_sensor.h"
#include "dbg.h"

 
#if ( IMGSRC_0_TYPE == SENSOR_TYPE_SP2509_R || IMGSRC_0_TYPE == SENSOR_TYPE_SP2509_L )
#define IMG_RES             IMGSRC_0_RES
#define IMG_FORMAT          IMGSRC_0_FORMAT
#define IMG_MIPILANE_NUM    IMGSRC_0_MIPILANE_NUM
#define IMG_MIRROR          CFG_SENSOR_0_FMT_MIRROR
#define IMG_FLIP            CFG_SENSOR_0_FMT_FLIP
#elif ( IMGSRC_1_TYPE == SENSOR_TYPE_SP2509_R || IMGSRC_1_TYPE ==SENSOR_TYPE_SP2509_L )
#define IMG_RES             IMGSRC_1_RES
#define IMG_FORMAT          IMGSRC_1_FORMAT
#define IMG_MIPILANE_NUM    IMGSRC_1_MIPILANE_NUM
#define IMG_MIRROR          CFG_SENSOR_1_FMT_MIRROR
#define IMG_FLIP            CFG_SENSOR_1_FMT_FLIP
#endif

#define RES_800X600 1
 
//#define CFG_sp2509_X_Y_INVERSE  (1)
static const struct sensor_datafmt_info sp2509_colour_fmts[] = {
    { V2K_PIX_FMT_YCBCR, V2K_COLORSPACE_YUV },
    { V2K_PIX_FMT_RGB565, V2K_COLORSPACE_RGB },
}; 

static const struct sensor_win_size sp2509_supported_win_sizes[] = {    
    { .width = 800, .height = 600, },
};

#ifndef CFG_SENSOR_0_I2C_ADDR
#define CFG_SENSOR_0_I2C_ADDR   (0x3d)
#endif
static struct sensor_device sp2509_dev_l = {
    .i2c_id = I2C_ADAP_0,
    .addr = CFG_SENSOR_0_I2C_ADDR,
    .device_id = 0XFFFF,
};

#ifndef CFG_SENSOR_1_I2C_ADDR
#define CFG_SENSOR_1_I2C_ADDR   (0x3d)
#endif
static struct sensor_device sp2509_dev_r = {
    .i2c_id = I2C_ADAP_1,
    .addr = CFG_SENSOR_1_I2C_ADDR,
    .device_id = 0XFFFF,
};

//850nm only

struct sensor_init_seq sp2509_init_regs[] = {  
#ifdef RES_800X600
//ini
{0xfd, 0x00},
{0x2f, 0x29},
{0x34, 0x00},
{0x35, 0x21},
{0x30, 0x15},
{0x33, 0x01},
{0xfd, 0x01},
{0x3f, 0x00 | (IMG_MIRROR) | (IMG_FLIP << 1)},
{0x44, 0x00},
{0x2a, 0x4c},
{0x2b, 0x1e},
{0x2c, 0x60},
{0x25, 0x11},
{0x03, 0x01},
{0x04, 0xae},
{0x09, 0x00},
{0x0a, 0x02},
{0x06, 0xa6},
{0x31, 0x00},
{0x24, 0x40},
{0x01, 0x01},
{0xfb, 0x73},
{0xfd, 0x01},
{0x16, 0x04},
{0x1c, 0x09},
{0x21, 0x42},
{0x12, 0x04},
{0x13, 0x10},
{0x11, 0x40},
{0x33, 0x81},
{0xd0, 0x00},
{0xd1, 0x01},
{0xd2, 0x00},
{0x50, 0x10},
{0x51, 0x23},
{0x52, 0x20},
{0x53, 0x10},
{0x54, 0x02},
{0x55, 0x20},
{0x56, 0x02},
{0x58, 0x48},
{0x5d, 0x15},
{0x5e, 0x05},
{0x66, 0x66},
{0x68, 0x68},
{0x6b, 0x00},
{0x6c, 0x00},
{0x6f, 0x40},
{0x70, 0x40},
{0x71, 0x0a},
{0x72, 0xf0},
{0x73, 0x10},
{0x75, 0x80},
{0x76, 0x10},
{0x84, 0x00},
{0x85, 0x10},
{0x86, 0x10},
{0x87, 0x00},
{0x8a, 0x22},
{0x8b, 0x22},
{0x19, 0xf1},
{0x29, 0x01},
{0xfd, 0x01},
{0x9d, 0x16},
{0xa0, 0x29},
{0xa1, 0x04},
{0xad, 0x62},
{0xae, 0x00},
{0xaf, 0x85},
{0xb1, 0x01},
{0x8e, 0x06},
{0x8f, 0x40},
{0x90, 0x04},
{0x91, 0xb0},
{0x45, 0x01},
{0x46, 0x00},
{0x47, 0x6c},
{0x48, 0x03},
{0x49, 0x8b},
{0x4a, 0x00},
{0x4b, 0x07},
{0x4c, 0x04},
{0x4d, 0xb7},
{0xf0, 0x40},
{0xf1, 0x40},
{0xf2, 0x40},
{0xf3, 0x40},
{0xac, 0x01},
{0xfd, 0x01},
//res800x600  
{0xfd, 0x00},
{0x33, 0x00},
{0xfd, 0x01},
{0x03, 0x01},   //exp
{0x04, 0xae},   //exp
{0x06, 0x04},   //vblank
{0x24, 0x64},   //gain
{0x30, 0x01},
{0x31, 0x04},
{0x33, 0x40},
{0x66, 0xa6},
{0x68, 0xa8},
{0x71, 0x10},
{0x73, 0x80},
{0x75, 0xb9},
{0x76, 0x80},
{0x8E, 0x03},
{0x8F, 0x20},
{0x90, 0x02},
{0x91, 0x58},
{0x9d, 0x6a},
{0x45, 0x01},
{0x46, 0x00},
{0x47, 0x6c},
{0x48, 0x03},
{0x49, 0x8b},
{0x4A, 0x00},
{0x4B, 0x01},
{0x4C, 0x01},
{0x4D, 0x2d},
{0xd6, 0x50},
{0xd7, 0x00},
{0xc9, 0x20},
{0xca, 0x00},
{0xd8, 0x00},
{0xd9, 0x04},
{0xda, 0x50},
{0xdb, 0x00},
{0xdc, 0x30},
{0xdd, 0x02},
{0xde, 0x50},
{0xdf, 0x00},
{0xea, 0x30},
{0xeb, 0x02},
{0x92, 0x02},

#else  

#endif
  {0x00, 0x00},
};

int sp2509_r_i2c_init(void)
{
    int ret = -1;

    ret = 0;
    
    return ret;
}
int sp2509_l_i2c_init(void)
{
    int ret = -1;

    ret = 0;
    
    return ret;
}

static int sp2509_write_reg(struct sensor_device *sensor_dev, u16 reg, u8 data)
{
    int ret;

    ret = kdp_drv_i2c_write(sensor_dev->i2c_id, sensor_dev->addr, reg, 1, data);
    if (ret)
        dbg_msg_console("[Err%d]sp2509_i2c addr:0x%02x, reg:0x%04x, data:0x%02x", ret, sensor_dev->addr, reg, data);

    return ret;
}

static int sp2509_read_reg(struct sensor_device *sensor_dev, u16 reg, u8 *data)
{
	int ret;

    ret = kdp_drv_i2c_read(sensor_dev->i2c_id, sensor_dev->addr, reg, 1, data);

	return ret;
}


int sp2509_init(struct sensor_device *sensor_dev, struct sensor_init_seq *seq, u32 length)
{
    struct sensor_device *dev = sensor_dev;
    struct sensor_init_seq *init_fnc_ptr = seq;
  uint8_t i=0;
    //dbg_msg_camera(" sp2509_init ");
   
//    sp2509_write_reg(dev, 0xfc , 0x01);//soft reset
//    osDelay(5);
    for (u32 i = 0; i < length; i++) {
        if (init_fnc_ptr[i].addr == 0 && init_fnc_ptr[i].value == 0) break; //reaches end

        // Update mirror and flip setting
        if (init_fnc_ptr[i].addr == 0x3f)
        {
            if (((CFG_SENSOR_0_TYPE == SENSOR_TYPE_SP2509_R) && (&sp2509_dev_r == dev)) ||
                ((CFG_SENSOR_0_TYPE == SENSOR_TYPE_SP2509_L) && (&sp2509_dev_l == dev)))
                init_fnc_ptr[i].value = 0x00 | (sensor_0_mirror) | (sensor_0_flip << 1);
            else if (((CFG_SENSOR_1_TYPE == SENSOR_TYPE_SP2509_R) && (&sp2509_dev_r == dev)) ||
                     ((CFG_SENSOR_1_TYPE == SENSOR_TYPE_SP2509_L) && (&sp2509_dev_l == dev)))
                init_fnc_ptr[i].value = 0x00 | (sensor_1_mirror) | (sensor_1_flip << 1);
        }
        
        sp2509_write_reg(dev, init_fnc_ptr[i].addr , (u8)(init_fnc_ptr[i].value & 0xFF));
    }
 
    
    sp2509_write_reg(dev, 0xfd , 0x00);

    u8 data = 0;
   
    sp2509_read_reg(dev, 0x02, &data);
//    dbg_msg_console("sp2509[%d] sensor high id=%x", dev->i2c_id, data);
    dev->device_id = data << 8;
    sp2509_read_reg(dev, 0x03, &data);
//    dbg_msg_console("sp2509[%d] sensor low id=%x", dev->i2c_id, data);
    dev->device_id |= data;
    dbg_msg_console("sp2509[%d] init over, sensor ID = 0x%x", dev->i2c_id, dev->device_id);
    
    if(dev->device_id != 0x2509)
        return -1;

    return 0;
}

static const struct sensor_win_size *sp2509_select_win(u32 *width, u32 *height)
{
    int i, default_size = ARRAY_SIZE(sp2509_supported_win_sizes) - 1;

    for (i = 0; i < ARRAY_SIZE(sp2509_supported_win_sizes); i++) {
        if (sp2509_supported_win_sizes[i].width  >= *width &&
            sp2509_supported_win_sizes[i].height >= *height) {
            *width = sp2509_supported_win_sizes[i].width;
            *height = sp2509_supported_win_sizes[i].height;
            return &sp2509_supported_win_sizes[i];
        }
    }

    *width = sp2509_supported_win_sizes[default_size].width;
    *height = sp2509_supported_win_sizes[default_size].height;
    return &sp2509_supported_win_sizes[default_size];
}

static int sp2509_set_params(
        struct sensor_device *sensor_dev, u32 *width, u32 *height, u32 fourcc)
{
    switch (fourcc) {	
        case V2K_PIX_FMT_YCBCR:
            dbg_msg_camera("%s: Selected V2K_PIX_FMT_YCBCR", __func__);
            break;            
        case V2K_PIX_FMT_RGB565:
            dbg_msg_camera("%s: Selected V2K_PIX_FMT_RGB565", __func__);
            break;
        default:;
    }

    /* reset hardware */
    //sp2509_reset(sensor_dev);

    /* initialize the sensor with default settings */
//    dbg_msg_console("init len=%d0",sizeof(sp2509_init_regs)/sizeof(sp2509_init_regs[0]));
    return sp2509_init(sensor_dev, sp2509_init_regs, sizeof(sp2509_init_regs)/sizeof(sp2509_init_regs[0]));
}
static int sp2509_sensor_enum_fmt(unsigned int index, unsigned int *code)
{
    if (index >= ARRAY_SIZE(sp2509_colour_fmts))
        return -KDP_FRAMEWORK_ERRNO_INVALA;

    //dbg_msg_camera("   <%s>\n", __func__);
    *code = sp2509_colour_fmts[index].fourcc;
    return 0;
}

static int sp2509_r_sensor_set_fmt(struct cam_format *fmt)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    sp2509_select_win(&fmt->width, &fmt->height);

    return sp2509_set_params(&sp2509_dev_r, &fmt->width, &fmt->height, fmt->pixelformat);
}

static int sp2509_l_sensor_set_fmt(struct cam_format *fmt)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    sp2509_select_win(&fmt->width, &fmt->height);

    return sp2509_set_params(&sp2509_dev_l, &fmt->width, &fmt->height, fmt->pixelformat);
}

int sp2509_sensor_set_aec_roi(struct sensor_device *sensor_dev, u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
{
//    enum i2c_adap_id i2c_id = sensor_dev->i2c_id;
//    u8 dev_addr = sensor_dev->addr;

//    //set to page 1
//    u8 reg = 0xfe;
//    u8 data = 0x01;
//    kdp_drv_i2c_write(i2c_id, dev_addr, reg, 1, data);
//unavailable
/*  
  
    //set roi
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x01, 1, x1 + ROI_OFFSET_X);
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x02, 1, x2 + ROI_OFFSET_X);
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x03, 1, y1 + ROI_OFFSET_Y);
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x04, 1, y2 + ROI_OFFSET_Y);

    //set center roi
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x05, 1, center_x1 + ROI_OFFSET_X);
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x06, 1, center_x2 + ROI_OFFSET_X);
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x07, 1, center_y1 + ROI_OFFSET_Y);
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x08, 1, center_y2 + ROI_OFFSET_Y);
*/
    return 0;
}

int sp2509_r_sensor_set_aec_roi(u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
{
    struct sensor_device *sensor_dev = &sp2509_dev_r;
    int ret = sp2509_sensor_set_aec_roi(sensor_dev, x1, x2, y1, y2, center_x1, center_x2, center_y1, center_y2);
    return ret;
}

int sp2509_l_sensor_set_aec_roi(u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
{
    struct sensor_device *sensor_dev = &sp2509_dev_l;
    int ret = sp2509_sensor_set_aec_roi(sensor_dev, x1, x2, y1, y2, center_x1, center_x2, center_y1, center_y2);
    return ret;
}

int sp2509_sensor_get_lux(struct sensor_device *sensor_dev, u16* exposure, u8* pre_gain_h, u8* pre_gain_l, u8* global_gain, u8* y_average)
{
    enum i2c_adap_id i2c_id = sensor_dev->i2c_id;
    u8 dev_addr = sensor_dev->addr;
 

    return 0;
}

int sp2509_r_sensor_get_lux(u16* exposure, u8* pre_gain_h, u8* pre_gain_l, u8* global_gain, u8* y_average)
{
    struct sensor_device *sensor_dev = &sp2509_dev_r;
    int ret = sp2509_sensor_get_lux(sensor_dev, exposure, pre_gain_h, pre_gain_l, global_gain, y_average);
    return ret;
}

int sp2509_l_sensor_get_lux(u16* exposure, u8* pre_gain_h, u8* pre_gain_l, u8* global_gain, u8* y_average)
{
    struct sensor_device *sensor_dev = &sp2509_dev_l;
    int ret = sp2509_sensor_get_lux(sensor_dev, exposure, pre_gain_h, pre_gain_l, global_gain, y_average);
    return ret;
}

static int sp2509_l_sensor_get_id(void)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    return sp2509_dev_l.device_id;
}

static int sp2509_r_sensor_get_id(void)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    return sp2509_dev_r.device_id;
}
 
int sp2509_sensor_set_gain(struct sensor_device *sensor_dev, u8 ana_gn1, u8 ana_gn2)
{
    enum i2c_adap_id i2c_id = sensor_dev->i2c_id;
    u8 dev_addr = sensor_dev->addr;
    
    kdp_drv_i2c_write(i2c_id, dev_addr, 0xfd, 1, 0x01);
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x24, 1, ana_gn2);
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x01, 1, 0x01);
    // dbg_msg_console("SP2509MIPI_SetGain = %d", ana_gn2);

    return 0;
}

int sp2509_r_sensor_set_gain(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &sp2509_dev_r;
    int ret = sp2509_sensor_set_gain(sensor_dev, ana_gn1, ana_gn2);
    return ret;
}

int sp2509_l_sensor_set_gain(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &sp2509_dev_l;
    int ret = sp2509_sensor_set_gain(sensor_dev, ana_gn1, ana_gn2);
    return ret;
}

int sp2509_sensor_set_exp_time(struct sensor_device *sensor_dev, u8 ana_gn1, u8 ana_gn2)
{
    enum i2c_adap_id i2c_id = sensor_dev->i2c_id;
    u8 dev_addr = sensor_dev->addr;
    
//        write_cmos_sensor(0xfd, 0x01); 
//        write_cmos_sensor(0x03, (shutter >> 8) & 0xFF);
//        write_cmos_sensor(0x04, shutter  & 0xFF); 
//        write_cmos_sensor(0x01, 0x01); 

    //set to page 1
    kdp_drv_i2c_write(i2c_id, dev_addr, 0xfd, 1, 0x01); //P1

    //set exp time
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x03, 1, ana_gn1);
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x04, 1, ana_gn2);

    kdp_drv_i2c_write(i2c_id, dev_addr, 0x01, 1, 0x01);  //P1 
    
    return 0;
}

int sp2509_r_sensor_set_exp_time(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &sp2509_dev_r;
    int ret = sp2509_sensor_set_exp_time(sensor_dev, ana_gn1, ana_gn2);

    return ret;
}

int sp2509_l_sensor_set_exp_time(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &sp2509_dev_l;
    int ret = sp2509_sensor_set_exp_time(sensor_dev, ana_gn1, ana_gn2);

    return ret;
}

static int sp2509_sensor_sleep(struct sensor_device *sensor_dev, BOOL enable)
{
    #define HW_EN 0

    enum i2c_adap_id i2c_id = sensor_dev->i2c_id;
    u8 dev_addr = sensor_dev->addr;
   
    if( enable == 1 )
    {
        #if HW_EN == YES
    	// pin high
        kdp520_gpio_setdata( 1<<0);
        #else

     

        #endif
    }
    else
    {
        #if HW_EN == YES
    	// pin low
        kdp520_gpio_cleardata( 1<<0);
        #else
     

        #endif
    }
    
    return 0;
}

static int sp2509_r_sensor_sleep( BOOL enable )
{
    struct sensor_device *sensor_dev = &sp2509_dev_r;
    int ret = sp2509_sensor_sleep(sensor_dev, enable);
    
    return 0;
}

static int sp2509_l_sensor_sleep( BOOL enable )
{
    struct sensor_device *sensor_dev = &sp2509_dev_l;
    int ret = sp2509_sensor_sleep(sensor_dev, enable);
    
    return 0;
}

  


static struct sensor_ops sp2509_r_ops = {
//    .s_power    = sp2509_sensor_power,
//    .reset      = sp2509_sensor_reset,
//    .s_stream   = sp2509_sensor_stream,
    .enum_fmt           = sp2509_sensor_enum_fmt,
//    .get_fmt    = sp2509_sensor_get_fmt,
    .set_fmt            = sp2509_r_sensor_set_fmt,
    .set_gain           = sp2509_r_sensor_set_gain,
    .set_exp_time       = sp2509_r_sensor_set_exp_time,
    .get_lux            = NULL,
    .set_aec_roi        = NULL,
    .set_mirror         = NULL,
    .set_flip           = NULL,
    .get_id             = sp2509_r_sensor_get_id,
    .set_fps            = NULL,
    .sleep              = NULL,
};

static struct sensor_ops sp2509_l_ops = {
//    .s_power    = sp2509_sensor_power,
//    .reset      = sp2509_sensor_reset,
//    .s_stream   = sp2509_sensor_stream,
    .enum_fmt           = sp2509_sensor_enum_fmt,
//    .get_fmt    = sp2509_sensor_get_fmt,
    .set_fmt            = sp2509_l_sensor_set_fmt,
    .set_gain           = sp2509_l_sensor_set_gain,
    .set_exp_time       = sp2509_l_sensor_set_exp_time,
    .get_lux            = NULL,
    .set_aec_roi        = NULL,
    .set_mirror         = NULL,
    .set_flip           = NULL,
    .get_id             = sp2509_l_sensor_get_id,
    .set_fps            = NULL,
    .sleep              = NULL,
};
void sp2509_r_sensor_init(int cam_idx)
{
    //dbg_msg_camera("   <%s>\n", __func__);
    kdp_sensor_register(SENSOR_TYPE_SP2509_R, &sp2509_r_ops);
}

void sp2509_l_sensor_init(int cam_idx)
{
    //dbg_msg_camera("   <%s>\n", __func__);
    kdp_sensor_register(SENSOR_TYPE_SP2509_L, &sp2509_l_ops);
}
#endif
