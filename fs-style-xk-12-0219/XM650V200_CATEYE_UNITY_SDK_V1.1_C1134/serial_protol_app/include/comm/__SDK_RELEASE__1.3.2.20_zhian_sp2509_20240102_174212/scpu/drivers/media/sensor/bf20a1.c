#include "board_kl520.h"
#if (IMGSRC_0_TYPE == SENSOR_TYPE_BF20A1_R) || \
    (IMGSRC_1_TYPE == SENSOR_TYPE_BF20A1_R) || \
    (IMGSRC_0_TYPE == SENSOR_TYPE_BF20A1_L) || \
    (IMGSRC_1_TYPE == SENSOR_TYPE_BF20A1_L)
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

 
#if ( IMGSRC_0_TYPE == SENSOR_TYPE_BF20A1_R || IMGSRC_0_TYPE == SENSOR_TYPE_BF20A1_L )
#define IMG_RES             IMGSRC_0_RES
#define IMG_FORMAT          IMGSRC_0_FORMAT
#define IMG_MIPILANE_NUM    IMGSRC_0_MIPILANE_NUM
#define IMG_MIRROR          CFG_SENSOR_0_FMT_MIRROR
#define IMG_FLIP            CFG_SENSOR_0_FMT_FLIP
#elif ( IMGSRC_1_TYPE == SENSOR_TYPE_BF20A1_R || IMGSRC_1_TYPE ==SENSOR_TYPE_BF20A1_L )
#define IMG_RES             IMGSRC_1_RES
#define IMG_FORMAT          IMGSRC_1_FORMAT
#define IMG_MIPILANE_NUM    IMGSRC_1_MIPILANE_NUM
#define IMG_MIRROR          CFG_SENSOR_1_FMT_MIRROR
#define IMG_FLIP            CFG_SENSOR_1_FMT_FLIP
#endif

 
//#define CFG_bf20a1_X_Y_INVERSE  (1)
static const struct sensor_datafmt_info bf20a1_colour_fmts[] = {
    { V2K_PIX_FMT_YCBCR, V2K_COLORSPACE_YUV },
    { V2K_PIX_FMT_RGB565, V2K_COLORSPACE_RGB },
}; 

static const struct sensor_win_size bf20a1_supported_win_sizes[] = {    
    { .width = HD_WIDTH, .height = HD_HEIGHT, },
    { .width = VGA_LANDSCAPE_WIDTH, .height = VGA_LANDSCAPE_HEIGHT, },    
    { .width = TFT43_WIDTH, .height = TFT43_HEIGHT, },     
    { .width = QVGA_WIDTH, .height = QVGA_HEIGHT, },
    { .width = UGA_WIDTH, .height = UGA_HEIGHT, },
};

static struct sensor_device bf20a1_dev_l = {
    .i2c_id = I2C_ADAP_0,
    .addr = 0x3e,
    .device_id = 0XFFFF,
};

static struct sensor_device bf20a1_dev_r = {
    .i2c_id = I2C_ADAP_1,
    .addr = 0x3e,
    .device_id = 0XFFFF,
};

//850nm only

struct sensor_init_seq bf20a1_init_regs[] = {  
//VGA
//???794  ???494
//Raw 8: XCLK=24M, MCLK=12M
//BYD BF20A1?initial code,???:
//MCLK: 24MHz
//FPS: 30
//Resolution: 640*480
//Data bit: raw8

{0x5d,0x00}, //BLC off
{0x50,0x50},
{0x51,0x50},
{0x52,0x50},
{0x53,0x50},
{0xe0,0x06},//MIPI CLK 120M
{0xe2,0xac},
{0xe3,0xcc},
{0xe5,0x3b},
{0xe6,0x04},
{0x7a,0x2b},
{0x7e,0x10},
{0x00,0x10},
{0x07,0xe1},
{0x08,0x02},
{0x09,0x81},
{0x0a,0x02},
{0x0b,0xe1},
{0x0c,0x90},
{0x25,0x4a},
{0x58,0x10},
{0x59,0x10},
{0x5a,0x10},
{0x5b,0x10},
{0x5c,0x98},
{0x5e,0x78},
{0x5f,0x49},
{0x2d,0x02},
{0x4f,0x00},
{0x10,0x10},//bit[0]: 1 black sun enable
{0xe4,0x32},
{0x15,0x11},
{0x6d,0x01},
{0x6e,0x50},
{0x6a,0x28},//?????
{0x6b,0x07},//{0x6b,0x6c}:???????
{0x6c,0x08},
{0xe8,0x10},
{0x5d,0xff}, //BLC ON
{0x7d,0x16},
{0x6b,0x01},
{0x6c,0xc5},
{0x6a,0x20},//?????
  {0x00, 0x00},
};

int bf20a1_r_i2c_init(void)
{
    int ret = -1;

    ret = 0;
    
    return ret;
}
int bf20a1_l_i2c_init(void)
{
    int ret = -1;

    ret = 0;
    
    return ret;
}

static int bf20a1_write_reg(struct sensor_device *sensor_dev, u16 reg, u8 data)
{
    int ret;

    ret = kdp_drv_i2c_write(sensor_dev->i2c_id, sensor_dev->addr, reg, 1, data);

    return ret;
}

static int bf20a1_read_reg(struct sensor_device *sensor_dev, u16 reg, u8 *data)
{
	int ret;

    ret = kdp_drv_i2c_read(sensor_dev->i2c_id, sensor_dev->addr, reg, 1, data);

	return ret;
}


int bf20a1_init(struct sensor_device *sensor_dev, struct sensor_init_seq *seq, u32 length)
{
    struct sensor_device *dev = sensor_dev;
    struct sensor_init_seq *init_fnc_ptr = seq;
  uint8_t i=0;
    //dbg_msg_camera(" bf20a1_init ");
  /*
    for(i=1;i<=254;i++)
      kdp_drv_i2c_write(0, i, 0x00, 1, 0x78);
  */
    for (u32 i = 0; i < length; i++) {
        if (init_fnc_ptr[i].addr == 0 && init_fnc_ptr[i].value == 0) break; //reaches end
        bf20a1_write_reg(dev, init_fnc_ptr[i].addr , (u8)(init_fnc_ptr[i].value & 0xFF));
    }
 
    
   
    u8 data = 0;
   
    bf20a1_read_reg(dev, 0xfc, &data);
//    dbg_msg_console("bf20a1[%d] sensor high id=%x", dev->i2c_id, data);
    dev->device_id = data << 8;
    bf20a1_read_reg(dev, 0xfd, &data);
//    dbg_msg_console("bf20a1[%d] sensor low id=%x", dev->i2c_id, data);
    dev->device_id |= data;
//    dbg_msg_console("bf20a1[%d] init over, sensor ID = 0x%x", dev->i2c_id, dev->device_id);
    
    /*
    if(dev->device_id != 0x02e0)
        return -1;
    */
    return 0;
}

static const struct sensor_win_size *bf20a1_select_win(u32 *width, u32 *height)
{
    int i, default_size = ARRAY_SIZE(bf20a1_supported_win_sizes) - 1;

    for (i = 0; i < ARRAY_SIZE(bf20a1_supported_win_sizes); i++) {
        if (bf20a1_supported_win_sizes[i].width  >= *width &&
            bf20a1_supported_win_sizes[i].height >= *height) {
            *width = bf20a1_supported_win_sizes[i].width;
            *height = bf20a1_supported_win_sizes[i].height;
            return &bf20a1_supported_win_sizes[i];
        }
    }

    *width = bf20a1_supported_win_sizes[default_size].width;
    *height = bf20a1_supported_win_sizes[default_size].height;
    return &bf20a1_supported_win_sizes[default_size];
}

static int bf20a1_set_params(
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
    //bf20a1_reset(sensor_dev);

    /* initialize the sensor with default settings */
//    dbg_msg_console("init len=%d0",sizeof(bf20a1_init_regs)/sizeof(bf20a1_init_regs[0]));
    return bf20a1_init(sensor_dev, bf20a1_init_regs, sizeof(bf20a1_init_regs)/sizeof(bf20a1_init_regs[0]));
}
static int bf20a1_sensor_enum_fmt(unsigned int index, unsigned int *code)
{
    if (index >= ARRAY_SIZE(bf20a1_colour_fmts))
        return -KDP_FRAMEWORK_ERRNO_INVALA;

    //dbg_msg_camera("   <%s>\n", __func__);
    *code = bf20a1_colour_fmts[index].fourcc;
    return 0;
}

static int bf20a1_r_sensor_set_fmt(struct cam_format *fmt)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    bf20a1_select_win(&fmt->width, &fmt->height);

    return bf20a1_set_params(&bf20a1_dev_r, &fmt->width, &fmt->height, fmt->pixelformat);
}

static int bf20a1_l_sensor_set_fmt(struct cam_format *fmt)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    bf20a1_select_win(&fmt->width, &fmt->height);

    return bf20a1_set_params(&bf20a1_dev_l, &fmt->width, &fmt->height, fmt->pixelformat);
}

int bf20a1_sensor_set_aec_roi(struct sensor_device *sensor_dev, u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
{
    enum i2c_adap_id i2c_id = sensor_dev->i2c_id;
    u8 dev_addr = sensor_dev->addr;

    //set to page 1
    u8 reg = 0xfe;
    u8 data = 0x01;
    kdp_drv_i2c_write(i2c_id, dev_addr, reg, 1, data);
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

int bf20a1_r_sensor_set_aec_roi(u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
{
    struct sensor_device *sensor_dev = &bf20a1_dev_r;
    int ret = bf20a1_sensor_set_aec_roi(sensor_dev, x1, x2, y1, y2, center_x1, center_x2, center_y1, center_y2);
    return ret;
}

int bf20a1_l_sensor_set_aec_roi(u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
{
    struct sensor_device *sensor_dev = &bf20a1_dev_l;
    int ret = bf20a1_sensor_set_aec_roi(sensor_dev, x1, x2, y1, y2, center_x1, center_x2, center_y1, center_y2);
    return ret;
}

int bf20a1_sensor_get_lux(struct sensor_device *sensor_dev, u16* exposure, u8* pre_gain_h, u8* pre_gain_l, u8* global_gain, u8* y_average)
{
    enum i2c_adap_id i2c_id = sensor_dev->i2c_id;
    u8 dev_addr = sensor_dev->addr;
 

    return 0;
}

int bf20a1_r_sensor_get_lux(u16* exposure, u8* pre_gain_h, u8* pre_gain_l, u8* global_gain, u8* y_average)
{
    struct sensor_device *sensor_dev = &bf20a1_dev_r;
    int ret = bf20a1_sensor_get_lux(sensor_dev, exposure, pre_gain_h, pre_gain_l, global_gain, y_average);
    return ret;
}

int bf20a1_l_sensor_get_lux(u16* exposure, u8* pre_gain_h, u8* pre_gain_l, u8* global_gain, u8* y_average)
{
    struct sensor_device *sensor_dev = &bf20a1_dev_l;
    int ret = bf20a1_sensor_get_lux(sensor_dev, exposure, pre_gain_h, pre_gain_l, global_gain, y_average);
    return ret;
}

static int bf20a1_l_sensor_get_id(void)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    return bf20a1_dev_l.device_id;
}

static int bf20a1_r_sensor_get_id(void)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    return bf20a1_dev_r.device_id;
}
 
 
                             
int bf20a1_sensor_set_gain(struct sensor_device *sensor_dev, u8 ana_gn1, u8 ana_gn2)
{
  
    enum i2c_adap_id i2c_id = sensor_dev->i2c_id;
    u8 dev_addr = sensor_dev->addr;

 
 
    return 0;
}

int bf20a1_r_sensor_set_gain(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &bf20a1_dev_r;
    int ret = bf20a1_sensor_set_gain(sensor_dev, ana_gn1, ana_gn2);
    return ret;
}

int bf20a1_l_sensor_set_gain(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &bf20a1_dev_l;
    int ret = bf20a1_sensor_set_gain(sensor_dev, ana_gn1, ana_gn2);
    return ret;
}

int bf20a1_sensor_set_exp_time(struct sensor_device *sensor_dev, u8 ana_gn1, u8 ana_gn2)
{
    enum i2c_adap_id i2c_id = sensor_dev->i2c_id;
    u8 dev_addr = sensor_dev->addr;
 
    return 0;
}

int bf20a1_r_sensor_set_exp_time(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &bf20a1_dev_r;
    int ret = bf20a1_sensor_set_exp_time(sensor_dev, ana_gn1, ana_gn2);

    return ret;
}

int bf20a1_l_sensor_set_exp_time(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &bf20a1_dev_l;
    int ret = bf20a1_sensor_set_exp_time(sensor_dev, ana_gn1, ana_gn2);

    return ret;
}

static int bf20a1_sensor_sleep(struct sensor_device *sensor_dev, BOOL enable)
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

static int bf20a1_r_sensor_sleep( BOOL enable )
{
    struct sensor_device *sensor_dev = &bf20a1_dev_r;
    int ret = bf20a1_sensor_sleep(sensor_dev, enable);
    
    return 0;
}

static int bf20a1_l_sensor_sleep( BOOL enable )
{
    struct sensor_device *sensor_dev = &bf20a1_dev_l;
    int ret = bf20a1_sensor_sleep(sensor_dev, enable);
    
    return 0;
}

  


static struct sensor_ops bf20a1_r_ops = {
//    .s_power    = bf20a1_sensor_power,
//    .reset      = bf20a1_sensor_reset,
//    .s_stream   = bf20a1_sensor_stream,
    .enum_fmt           = bf20a1_sensor_enum_fmt,
//    .get_fmt    = bf20a1_sensor_get_fmt,
    .set_fmt            = bf20a1_r_sensor_set_fmt,
    .set_gain           = bf20a1_r_sensor_set_gain,
    .set_exp_time       = bf20a1_r_sensor_set_exp_time,
    .get_lux            = bf20a1_r_sensor_get_lux,
    .set_aec_roi        = bf20a1_r_sensor_set_aec_roi,
    .set_mirror         = NULL,
    .set_flip           = NULL,
    .get_id             = bf20a1_r_sensor_get_id,
    .set_fps            = NULL,
    .sleep              = bf20a1_r_sensor_sleep,
};

static struct sensor_ops bf20a1_l_ops = {
//    .s_power    = bf20a1_sensor_power,
//    .reset      = bf20a1_sensor_reset,
//    .s_stream   = bf20a1_sensor_stream,
    .enum_fmt           = bf20a1_sensor_enum_fmt,
//    .get_fmt    = bf20a1_sensor_get_fmt,
    .set_fmt            = bf20a1_l_sensor_set_fmt,
    .set_gain           = bf20a1_l_sensor_set_gain,
    .set_exp_time       = bf20a1_l_sensor_set_exp_time,
    .get_lux            = bf20a1_l_sensor_get_lux,
    .set_aec_roi        = bf20a1_l_sensor_set_aec_roi,
    .set_mirror         = NULL,
    .set_flip           = NULL,
    .get_id             = bf20a1_l_sensor_get_id,
    .set_fps            = NULL,
    .sleep              = bf20a1_l_sensor_sleep,
};
void bf20a1_r_sensor_init(int cam_idx)
{
    //dbg_msg_camera("   <%s>\n", __func__);
    kdp_sensor_register(SENSOR_TYPE_BF20A1_R, &bf20a1_r_ops);
}

void bf20a1_l_sensor_init(int cam_idx)
{
    //dbg_msg_camera("   <%s>\n", __func__);
    kdp_sensor_register(SENSOR_TYPE_BF20A1_L, &bf20a1_l_ops);
}
#endif
