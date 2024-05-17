#include "board_kl520.h"
#if (IMGSRC_0_TYPE == SENSOR_TYPE_GC1054_R) || \
    (IMGSRC_1_TYPE == SENSOR_TYPE_GC1054_R) || \
    (IMGSRC_0_TYPE == SENSOR_TYPE_GC1054_L) || \
    (IMGSRC_1_TYPE == SENSOR_TYPE_GC1054_L)
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

#if ( IMGSRC_0_TYPE == SENSOR_TYPE_GC1054_R || IMGSRC_0_TYPE == SENSOR_TYPE_GC1054_L )
#define IMG_RES             IMGSRC_0_RES
#define IMG_FORMAT          IMGSRC_0_FORMAT
#define IMG_MIPILANE_NUM    IMGSRC_0_MIPILANE_NUM
#define IMG_MIRROR          CFG_SENSOR_0_FMT_MIRROR
#define IMG_FLIP            CFG_SENSOR_0_FMT_FLIP
#elif ( IMGSRC_1_TYPE == SENSOR_TYPE_GC1054_R || IMGSRC_1_TYPE == SENSOR_TYPE_GC1054_L )
#define IMG_RES             IMGSRC_1_RES
#define IMG_FORMAT          IMGSRC_1_FORMAT
#define IMG_MIPILANE_NUM    IMGSRC_1_MIPILANE_NUM
#define IMG_MIRROR          CFG_SENSOR_1_FMT_MIRROR
#define IMG_FLIP            CFG_SENSOR_1_FMT_FLIP
#endif

 
#define CROPPING_OFFSET_X   (0)
#define CROPPING_OFFSET_Y   (0)
 
#define ROI_OFFSET_X        ((CROPPING_OFFSET_X + 4) >> 3)
#define ROI_OFFSET_Y        ((CROPPING_OFFSET_Y + 4) >> 3)

//#define CFG_GC1054_X_Y_INVERSE  (1)
static const struct sensor_datafmt_info gc1054_colour_fmts[] = {
    { V2K_PIX_FMT_YCBCR, V2K_COLORSPACE_YUV },
    { V2K_PIX_FMT_RGB565, V2K_COLORSPACE_RGB },
}; 

static const struct sensor_win_size gc1054_supported_win_sizes[] = {    
    { .width = HD_WIDTH, .height = HD_HEIGHT, },
    { .width = VGA_LANDSCAPE_WIDTH, .height = VGA_LANDSCAPE_HEIGHT, },    
    { .width = TFT43_WIDTH, .height = TFT43_HEIGHT, },     
    { .width = QVGA_WIDTH, .height = QVGA_HEIGHT, },
    { .width = UGA_WIDTH, .height = UGA_HEIGHT, },
};

static struct sensor_device gc1054_dev_l = {
    .i2c_id = I2C_ADAP_0,
    .addr = 0x21,
    .device_id = 0XFFFF,
};

static struct sensor_device gc1054_dev_r = {
    .i2c_id = I2C_ADAP_1,
    .addr = 0x21,
    .device_id = 0XFFFF,
};

//850nm only
#if 1// ( CFG_LW3D_TYPE == CFG_LW3D_940 )



#define LSC_ON YES
#define GAMMA_1 YES
#define GAMMA_2 YES
#define BLK_ON YES
#define AEC_ON YES
struct sensor_init_seq INITDATA gc1054_init_regs[] = {  
#if (CFG_1054_12M_ENABLE == YES)
//MIPI_PRE_DEF7=2d,200d,2d,3d,8d,2d,3d,16d,7d
//Actual_window_size=1280*720,MIPI 1lane
//Mclk=24Mhz?MIPI_clcok=312Mhz,Frame_rate=30fps
//HD=1726,VD=750,row_time=44.25us
/////////////////////////////////////////////////////
//////////////////////   SYS   //////////////////////
/////////////////////////////////////////////////////
{0xf2,0x00},
{0xf6,0x00},
{0xfc,0x04},
{0xf7,0x01},
{0xf8,0x19},
{0xf9,0x06},
{0xfa,0x80},
{0xfc,0x0e},
//////////////////////////////////////////
/////   ANALOG & CISCTL   ////////////////
//////////////////////////////////////////
{0xfe,0x00},
{0x03,0x02}, 
{0x04,0xa6}, 
{0x05,0x02}, //HB
{0x06,0x07},
{0x07,0x00}, //VB
{0x08,0x0a}, 
{0x09,0x00},
{0x0a,0x04}, //row start
{0x0b,0x00},
{0x0c,0x00}, //col start
{0x0d,0x02}, 
{0x0e,0xd4}, //height 724
{0x0f,0x05}, 
{0x10,0x08}, //width 1288
{0x17,(0xc0 | (IMG_MIRROR) | (IMG_FLIP << 1))},
{0x18,0x02},
{0x19,0x08},
{0x1a,0x18},
{0x1d,0x12},
{0x1e,0x50},
{0x1f,0x80},
{0x21,0x30},
{0x23,0xf8},
{0x25,0x10},
{0x28,0x20},
{0x34,0x0a}, //data low
{0x3c,0x10},
{0x3d,0x0e},
{0xcc,0x8e},
{0xcd,0x9a},
{0xcf,0x70},
{0xd0,0xa9},
{0xd1,0xc5},
{0xd2,0xed}, //data high
{0xd8,0x3c}, //dacin offset
{0xd9,0x7a},
{0xda,0x12},
{0xdb,0x50},
{0xde,0x0c},
{0xe3,0x60},
{0xe4,0x78},
{0xfe,0x01},
{0xe3,0x01},
{0xe6,0x10}, //ramps offset
//////////////////////////////////////////
///////////   ISP   //////////////////////
//////////////////////////////////////////
{0xfe,0x01},
{0x80,0x50},
{0x88,0x73},
{0x89,0x03},
{0x90,0x01}, 
{0x92,0x02}, //crop win 2<=y<=4
{0x94,0x03}, //crop win 2<=x<=5
{0x95,0x02}, //crop win height
{0x96,0xd0},
{0x97,0x05}, //crop win width
{0x98,0x00},
//////////////////////////////////////////
///////////   BLK   //////////////////////
//////////////////////////////////////////
{0xfe,0x01},
{0x40,0x22},
{0x43,0x03},
{0x4e,0x3c},
{0x4f,0x00},
{0x60,0x00},
{0x61,0x80},
//////////////////////////////////////////
///////////   GAIN   /////////////////////
//////////////////////////////////////////
{0xfe,0x01},
{0xb0,0x48},
{0xb1,0x01}, 
{0xb2,0x00}, 
{0xb6,0x00}, 
{0xfe,0x02},
{0x01,0x00},
{0x02,0x01},
{0x03,0x02},
{0x04,0x03},
{0x05,0x04},
{0x06,0x05},
{0x07,0x06},
{0x08,0x0e},
{0x09,0x16},
{0x0a,0x1e},
{0x0b,0x36},
{0x0c,0x3e},
{0x0d,0x56},
{0xfe,0x02},
{0xb0,0x00}, //col_gain[11:8]
{0xb1,0x00},
{0xb2,0x00},
{0xb3,0x11},
{0xb4,0x22},
{0xb5,0x54},
{0xb6,0xb8},
{0xb7,0x60},
{0xb9,0x00}, //col_gain[12]
{0xba,0xc0},
{0xc0,0x20}, //col_gain[7:0]
{0xc1,0x2d},
{0xc2,0x40},
{0xc3,0x5b},
{0xc4,0x80},
{0xc5,0xb5},
{0xc6,0x00},
{0xc7,0x6a},
{0xc8,0x00},
{0xc9,0xd4},
{0xca,0x00},
{0xcb,0xa8},
{0xcc,0x00},
{0xcd,0x50},
{0xce,0x00},
{0xcf,0xa1},
//////////////////////////////////////////
/////////   DARKSUN   ////////////////////
//////////////////////////////////////////
{0xfe,0x02},
{0x54,0xf7},
{0x55,0xf0},
{0x56,0x00},
{0x57,0x00},
{0x58,0x00},
{0x5a,0x04},
//////////////////////////////////////////
////////////   DD   //////////////////////
//////////////////////////////////////////
{0xfe,0x04},
{0x81,0x8a},
//////////////////////////////////////////
///////////	 MIPI	/////////////////////
//////////////////////////////////////////
{0xfe,0x03},
{0x01,0x03},
{0x02,0x11},
{0x03,0x90},
{0x10,0x94},
{0x11,0x2a},
{0x12,0x00}, //lwc 1280*5/4
{0x13,0x05},
{0x15,0x06},
{0x21,0x02},
{0x22,0x02},
{0x23,0x08},
{0x24,0x02},
{0x25,0x10},
{0x26,0x04},
{0x29,0x02},
{0x2a,0x02},
{0x2b,0x04},
{0xfe,0x00},
#else
//Actual_window_size=1280*720,MIPI 1lane
//Mclk=24Mhz?MIPI_clcok=312Mhz,Frame_rate=30fps
//HD=1726,VD=785,row_time=42.2564us
/////////////////////////////////////////////////////
//////////////////////   SYS   //////////////////////
/////////////////////////////////////////////////////
{0xf2,0x00},
{0xf6,0x00},
{0xfc,0x04},
{0xf7,0x01},
{0xf8,0x0c},
{0xf9,0x06},
{0xfa,0x80},
{0xfc,0x0e},
//////////////////////////////////////////
/////   ANALOG & CISCTL   ////////////////
//////////////////////////////////////////
{0xfe,0x00},
{0x03,0x02}, 
{0x04,0xa6}, 
{0x05,0x01}, //HB
{0x06,0xe0},
{0x07,0x00}, //VB
{0x08,0x2d}, 
{0x09,0x00},
{0x0a,0x04}, //row start
{0x0b,0x00},
{0x0c,0x00}, //col start
{0x0d,0x02}, 
{0x0e,0xd4}, //height 724
{0x0f,0x05}, 
{0x10,0x08}, //width 1288
{0x17,(0xc0 | (IMG_MIRROR) | (IMG_FLIP << 1))},
{0x18,0x02},
{0x19,0x08},
{0x1a,0x18},
{0x1d,0x12},
{0x1e,0x50},
{0x1f,0x80},
{0x21,0x30},
{0x23,0xf8},
{0x25,0x10},
{0x28,0x20},
{0x34,0x08}, //data low
{0x3c,0x10},
{0x3d,0x0e},
{0xcc,0x8e},
{0xcd,0x9a},
{0xcf,0x70},
{0xd0,0xa9},
{0xd1,0xc5},
{0xd2,0xed}, //data high
{0xd8,0x3c}, //dacin offset
{0xd9,0x7a},
{0xda,0x12},
{0xdb,0x50},
{0xde,0x0c},
{0xe3,0x60},
{0xe4,0x78},
{0xfe,0x01},
{0xe3,0x01},
{0xe6,0x10}, //ramps offset
//////////////////////////////////////////
///////////   ISP   //////////////////////
//////////////////////////////////////////
{0xfe,0x01},
{0x80,0x50},
{0x88,0x73},
{0x89,0x03},
{0x90,0x01}, 
{0x92,0x02}, //crop win 2<=y<=4
{0x94,0x03}, //crop win 2<=x<=5
{0x95,0x02}, //crop win height
{0x96,0xd0},
{0x97,0x05}, //crop win width
{0x98,0x00},
//////////////////////////////////////////
///////////   BLK   //////////////////////
//////////////////////////////////////////
{0xfe,0x01},
{0x40,0x22},
{0x43,0x03},
{0x4e,0x3c},
{0x4f,0x00},
{0x60,0x00},
{0x61,0x80},
//////////////////////////////////////////
///////////   GAIN   /////////////////////
//////////////////////////////////////////
{0xfe,0x01},
{0xb0,0x50}, //global gain
{0xb1,0x01}, //pre_gain high byte
{0xb2,0xf4}, //pre_gain low byte
{0xb6,0x00},
{0xfe,0x02},
{0x01,0x00},
{0x02,0x01},
{0x03,0x02},
{0x04,0x03},
{0x05,0x04},
{0x06,0x05},
{0x07,0x06},
{0x08,0x0e},
{0x09,0x16},
{0x0a,0x1e},
{0x0b,0x36},
{0x0c,0x3e},
{0x0d,0x56},
{0xfe,0x02},
{0xb0,0x00}, //col_gain[11:8]
{0xb1,0x00},
{0xb2,0x00},
{0xb3,0x11},
{0xb4,0x22},
{0xb5,0x54},
{0xb6,0xb8},
{0xb7,0x60},
{0xb9,0x00}, //col_gain[12]
{0xba,0xc0},
{0xc0,0x20}, //col_gain[7:0]
{0xc1,0x2d},
{0xc2,0x40},
{0xc3,0x5b},
{0xc4,0x80},
{0xc5,0xb5},
{0xc6,0x00},
{0xc7,0x6a},
{0xc8,0x00},
{0xc9,0xd4},
{0xca,0x00},
{0xcb,0xa8},
{0xcc,0x00},
{0xcd,0x50},
{0xce,0x00},
{0xcf,0xa1},
//////////////////////////////////////////
/////////   DARKSUN   ////////////////////
//////////////////////////////////////////
{0xfe,0x02},
{0x54,0xf7},
{0x55,0xf0},
{0x56,0x00},
{0x57,0x00},
{0x58,0x00},
{0x5a,0x04},
//////////////////////////////////////////
////////////   DD   //////////////////////
//////////////////////////////////////////
{0xfe,0x04},
{0x81,0x8a},
//////////////////////////////////////////
///////////	 MIPI	/////////////////////
//////////////////////////////////////////
{0xfe,0x03},
{0x01,0x03},
{0x02,0x11},
{0x03,0x90},
{0x10,0x94},
{0x11,0x2a},
{0x12,0x00}, //lwc 1280*5/4
{0x13,0x05},
{0x15,0x06},
{0x21,0x02},
{0x22,0x02},
{0x23,0x08},
{0x24,0x02},
{0x25,0x10},
{0x26,0x04},
{0x29,0x02},
{0x2a,0x02},
{0x2b,0x04},
{0xfe,0x00},
#endif

{0x00,0x00},

};
#endif

int gc1054_r_i2c_init(void)
{
    int ret = -1;

    ret = 0;
    
    return ret;
}
int gc1054_l_i2c_init(void)
{
    int ret = -1;

    ret = 0;
    
    return ret;
}

static int gc1054_write_reg(struct sensor_device *sensor_dev, u16 reg, u8 data)
{
    int ret;

    ret = kdp_drv_i2c_write(sensor_dev->i2c_id, sensor_dev->addr, reg, 1, data);

    return ret;
}

static int gc1054_read_reg(struct sensor_device *sensor_dev, u16 reg, u8 *data)
{
	int ret;

    ret = kdp_drv_i2c_read(sensor_dev->i2c_id, sensor_dev->addr, reg, 1, data);

	return ret;
}

int gc1054_init(struct sensor_device *sensor_dev, struct sensor_init_seq *seq, u32 length)
{
    struct sensor_device *dev = sensor_dev;
    struct sensor_init_seq *init_fnc_ptr = seq;
    //dbg_msg_camera(" gc1054_init ");
    for (u32 i = 0; i < length; i++) {
        if (init_fnc_ptr[i].addr == 0 && init_fnc_ptr[i].value == 0) break; //reaches end

        // Update mirror and flip setting
        if (init_fnc_ptr[i].addr == 0x17)
        {
            if (((CFG_SENSOR_0_TYPE == SENSOR_TYPE_GC1054_R) && (&gc1054_dev_r == dev)) ||
                ((CFG_SENSOR_0_TYPE == SENSOR_TYPE_GC1054_L) && (&gc1054_dev_l == dev)))
                init_fnc_ptr[i].value = 0xc0 | (sensor_0_mirror) | (sensor_0_flip << 1);
            else if (((CFG_SENSOR_1_TYPE == SENSOR_TYPE_GC1054_R) && (&gc1054_dev_r == dev)) ||
                     ((CFG_SENSOR_1_TYPE == SENSOR_TYPE_GC1054_L) && (&gc1054_dev_l == dev)))
                init_fnc_ptr[i].value = 0xc0 | (sensor_1_mirror) | (sensor_1_flip << 1);
        }
        
        gc1054_write_reg(dev, init_fnc_ptr[i].addr , (u8)(init_fnc_ptr[i].value & 0xFF));
    }
    dbg_msg_camera("gc1054[%d] init over, sensor ID = 0x%x", dev->i2c_id, dev->device_id);
    u8 data = 0;
    gc1054_read_reg(dev, 0xf0, &data);
    dbg_msg_camera("gc1054[%d] sensor high id=%x", dev->i2c_id, data);
    dev->device_id = data << 8;
    gc1054_read_reg(dev, 0xf1, &data);
    dbg_msg_camera("gc1054[%d] sensor low id=%x", dev->i2c_id, data);
    dev->device_id |= data;
    
    if(dev->device_id != 0x1054)
        return -1;
    
    return 0;
}

static const struct sensor_win_size *gc1054_select_win(u32 *width, u32 *height)
{
    int i, default_size = ARRAY_SIZE(gc1054_supported_win_sizes) - 1;

    for (i = 0; i < ARRAY_SIZE(gc1054_supported_win_sizes); i++) {
        if (gc1054_supported_win_sizes[i].width  >= *width &&
            gc1054_supported_win_sizes[i].height >= *height) {
            *width = gc1054_supported_win_sizes[i].width;
            *height = gc1054_supported_win_sizes[i].height;
            return &gc1054_supported_win_sizes[i];
        }
    }

    *width = gc1054_supported_win_sizes[default_size].width;
    *height = gc1054_supported_win_sizes[default_size].height;
    return &gc1054_supported_win_sizes[default_size];
}

static int gc1054_set_params(
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
    //gc1054_reset(sensor_dev);

    /* initialize the sensor with default settings */
    return gc1054_init(sensor_dev, gc1054_init_regs, sizeof(gc1054_init_regs)/sizeof(gc1054_init_regs[0]));
}
static int gc1054_sensor_enum_fmt(unsigned int index, unsigned int *code)
{
    if (index >= ARRAY_SIZE(gc1054_colour_fmts))
        return -KDP_FRAMEWORK_ERRNO_INVALA;

    //dbg_msg_camera("   <%s>\n", __func__);
    *code = gc1054_colour_fmts[index].fourcc;
    return 0;
}

static int gc1054_r_sensor_set_fmt(struct cam_format *fmt)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    gc1054_select_win(&fmt->width, &fmt->height);

    return gc1054_set_params(&gc1054_dev_r, &fmt->width, &fmt->height, fmt->pixelformat);
}

static int gc1054_l_sensor_set_fmt(struct cam_format *fmt)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    gc1054_select_win(&fmt->width, &fmt->height);

    return gc1054_set_params(&gc1054_dev_l, &fmt->width, &fmt->height, fmt->pixelformat);
}

int gc1054_sensor_set_aec_roi(struct sensor_device *sensor_dev, u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
{
    enum i2c_adap_id i2c_id = sensor_dev->i2c_id;
    u8 dev_addr = sensor_dev->addr;

    //set to page 1
    u8 reg = 0xfe;
    u8 data = 0x01;
    kdp_drv_i2c_write(i2c_id, dev_addr, reg, 1, data);

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

    return 0;
}

int gc1054_r_sensor_set_aec_roi(u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
{
    struct sensor_device *sensor_dev = &gc1054_dev_r;
    int ret = gc1054_sensor_set_aec_roi(sensor_dev, x1, x2, y1, y2, center_x1, center_x2, center_y1, center_y2);
    return ret;
}

int gc1054_l_sensor_set_aec_roi(u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
{
    struct sensor_device *sensor_dev = &gc1054_dev_l;
    int ret = gc1054_sensor_set_aec_roi(sensor_dev, x1, x2, y1, y2, center_x1, center_x2, center_y1, center_y2);
    return ret;
}

int gc1054_sensor_get_lux(struct sensor_device *sensor_dev, u16* exposure, u8* pre_gain_h, u8* pre_gain_l, u8* global_gain, u8* y_average)
{
    enum i2c_adap_id i2c_id = sensor_dev->i2c_id;
    u8 dev_addr = sensor_dev->addr;

    //set to page 0
    u8 reg = 0xfe;
    u8 data = 0x00;
    kdp_drv_i2c_write(i2c_id, dev_addr, reg, 1, data);

    //get exposure
    reg = 0x03;
    *exposure = 0;
    data = 0;
    int ret = kdp_drv_i2c_read(i2c_id, dev_addr, reg, 1, &data);
    *exposure |= (data << 8);

    reg = 0x04;
    data = 0;
    ret = kdp_drv_i2c_read(i2c_id, dev_addr, reg, 1, &data);
    *exposure |= data;

    // set to page 1
    reg = 0xfe;
    data = 0x01;
    kdp_drv_i2c_write(i2c_id, dev_addr, reg, 1, data);

    //get pre gain
    reg = 0xb1;
    data = 0;
    ret = kdp_drv_i2c_read(i2c_id, dev_addr, reg, 1, &data);
    *pre_gain_h = (data & 0x0F);

    reg = 0xb2;
    data = 0;
    ret = kdp_drv_i2c_read(i2c_id, dev_addr, reg, 1, &data);
    *pre_gain_l = data;

    // get global gain
    reg = 0xb0;
    data = 0;
    ret = kdp_drv_i2c_read(i2c_id, dev_addr, reg, 1, &data);
    *global_gain = data;

//    // set y_average
//    reg = 0x14;
//    data = 0;
//    ret = kdp_drv_i2c_read(i2c_id, dev_addr, reg, 1, &data);
//    *y_average = data;

    return 0;
}

int gc1054_r_sensor_get_lux(u16* exposure, u8* pre_gain_h, u8* pre_gain_l, u8* global_gain, u8* y_average)
{
    struct sensor_device *sensor_dev = &gc1054_dev_r;
    int ret = gc1054_sensor_get_lux(sensor_dev, exposure, pre_gain_h, pre_gain_l, global_gain, y_average);
    return ret;
}

int gc1054_l_sensor_get_lux(u16* exposure, u8* pre_gain_h, u8* pre_gain_l, u8* global_gain, u8* y_average)
{
    struct sensor_device *sensor_dev = &gc1054_dev_l;
    int ret = gc1054_sensor_get_lux(sensor_dev, exposure, pre_gain_h, pre_gain_l, global_gain, y_average);
    return ret;
}

static int gc1054_l_sensor_get_id(void)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    return gc1054_dev_l.device_id;
}

static int gc1054_r_sensor_get_id(void)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    return gc1054_dev_r.device_id;
}
UINT32 gainLevelTable[12] = {
								64, 
								91, 
								127, 
								182, 
								258, 
								369, 
								516, 
								738, 
								1032, 
								1491, 
								2084, 
								0xffffffff,
							 };
static 	int total;	
                             
int gc1054_sensor_set_gain(struct sensor_device *sensor_dev, u8 ana_gn1, u8 ana_gn2)
{
    enum i2c_adap_id i2c_id = sensor_dev->i2c_id;
    u8 dev_addr = sensor_dev->addr;

    //set to page 1
    u8 reg = 0xfe;
    u8 data = 0x01;
    kdp_drv_i2c_write(i2c_id, dev_addr, reg, 1, data);

    u16 gain = 0;
    gain = ana_gn1<<8;
    gain = gain | ana_gn2;
    
    total = sizeof(gainLevelTable) / sizeof(UINT32);
    int Analog_Index;
    u16 tol_dig_gain = 0;	
    for(Analog_Index = 0; Analog_Index < total; Analog_Index++)
    {
        if((gainLevelTable[Analog_Index] <= gain)&&(gain < gainLevelTable[Analog_Index+1]))
            break;
    }
    
    tol_dig_gain = gain*64/gainLevelTable[Analog_Index];	
    //set gain
    kdp_drv_i2c_write(i2c_id, dev_addr, 0xb6, 1, (u8)Analog_Index);
    kdp_drv_i2c_write(i2c_id, dev_addr, 0xb1, 1, (tol_dig_gain>>6));
    kdp_drv_i2c_write(i2c_id, dev_addr, 0xb2, 1, ((tol_dig_gain&0x3f)<<2));
    
    return 0;
}

int gc1054_r_sensor_set_gain(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &gc1054_dev_r;
    int ret = gc1054_sensor_set_gain(sensor_dev, ana_gn1, ana_gn2);
    return ret;
}

int gc1054_l_sensor_set_gain(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &gc1054_dev_l;
    int ret = gc1054_sensor_set_gain(sensor_dev, ana_gn1, ana_gn2);
    return ret;
}

int gc1054_sensor_set_exp_time(struct sensor_device *sensor_dev, u8 ana_gn1, u8 ana_gn2)
{
    enum i2c_adap_id i2c_id = sensor_dev->i2c_id;
    u8 dev_addr = sensor_dev->addr;

    //set to page 1
    u8 reg = 0xfe;
    u8 data = 0x00;
    kdp_drv_i2c_write(i2c_id, dev_addr, reg, 1, data);

    //set exp time
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x03, 1, ana_gn1);
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x04, 1, ana_gn2);

    return 0;
}

int gc1054_r_sensor_set_exp_time(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &gc1054_dev_r;
    int ret = gc1054_sensor_set_exp_time(sensor_dev, ana_gn1, ana_gn2);

    return ret;
}

int gc1054_l_sensor_set_exp_time(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &gc1054_dev_l;
    int ret = gc1054_sensor_set_exp_time(sensor_dev, ana_gn1, ana_gn2);

    return ret;
}

static int gc1054_sensor_sleep(struct sensor_device *sensor_dev, BOOL enable)
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

        kdp_drv_i2c_write(i2c_id, dev_addr, 0xfe, 1, 0x03 );
        kdp_drv_i2c_write(i2c_id, dev_addr, 0x10, 1, 0x00 );
        kdp_drv_i2c_write(i2c_id, dev_addr, 0xfe, 1, 0x00 );
        kdp_drv_i2c_write(i2c_id, dev_addr, 0xf7, 1, 0x00 );
        kdp_drv_i2c_write(i2c_id, dev_addr, 0xfc, 1, 0x01 );
        kdp_drv_i2c_write(i2c_id, dev_addr, 0xf9, 1, 0x01 );

        #endif
    }
    else
    {
        #if HW_EN == YES
    	// pin low
        kdp520_gpio_cleardata( 1<<0);
        #else
        kdp_drv_i2c_write(i2c_id, dev_addr, 0xf9, 1, 0x06 );
        kdp_drv_i2c_write(i2c_id, dev_addr, 0xf7, 1, 0x01 );
        kdp_drv_i2c_write(i2c_id, dev_addr, 0xfc, 1, 0x0e ); // 0x0e
        kdp_drv_i2c_write(i2c_id, dev_addr, 0xfe, 1, 0x03 );
        kdp_drv_i2c_write(i2c_id, dev_addr, 0x10, 1, 0x94 ); // 0x94
        kdp_drv_i2c_write(i2c_id, dev_addr, 0xfe, 1, 0x00 );

        #endif
    }
    
    return 0;
}

static int gc1054_r_sensor_sleep( BOOL enable )
{
    struct sensor_device *sensor_dev = &gc1054_dev_r;
    int ret = gc1054_sensor_sleep(sensor_dev, enable);
    
    return 0;
}

static int gc1054_l_sensor_sleep( BOOL enable )
{
    struct sensor_device *sensor_dev = &gc1054_dev_l;
    int ret = gc1054_sensor_sleep(sensor_dev, enable);
    
    return 0;
}


static struct sensor_ops gc1054_r_ops = {
//    .s_power    = gc1054_sensor_power,
//    .reset      = gc1054_sensor_reset,
//    .s_stream   = gc1054_sensor_stream,
    .enum_fmt           = gc1054_sensor_enum_fmt,
//    .get_fmt    = gc1054_sensor_get_fmt,
    .set_fmt            = gc1054_r_sensor_set_fmt,
    .set_gain           = gc1054_r_sensor_set_gain,
    .set_exp_time       = gc1054_r_sensor_set_exp_time,
    .get_lux            = gc1054_r_sensor_get_lux,
    .set_aec_roi        = gc1054_r_sensor_set_aec_roi,
    .set_mirror         = NULL,
    .set_flip           = NULL,
    .get_id             = gc1054_r_sensor_get_id,
    .set_fps            = NULL,
    .sleep              = gc1054_r_sensor_sleep,
};
static struct sensor_ops gc1054_l_ops = {
//    .s_power    = gc1054_sensor_power,
//    .reset      = gc1054_sensor_reset,
//    .s_stream   = gc1054_sensor_stream,
    .enum_fmt           = gc1054_sensor_enum_fmt,
//    .get_fmt    = gc1054_sensor_get_fmt,
    .set_fmt            = gc1054_l_sensor_set_fmt,
    .set_gain           = gc1054_l_sensor_set_gain,
    .set_exp_time       = gc1054_l_sensor_set_exp_time,
    .get_lux            = gc1054_l_sensor_get_lux,
    .set_aec_roi        = gc1054_l_sensor_set_aec_roi,
    .set_mirror         = NULL,
    .set_flip           = NULL,
    .get_id             = gc1054_l_sensor_get_id,
    .set_fps            = NULL,
    .sleep              = gc1054_l_sensor_sleep,
};
void gc1054_r_sensor_init(int cam_idx)
{
    //dbg_msg_camera("   <%s>\n", __func__);
    kdp_sensor_register(SENSOR_TYPE_GC1054_R, &gc1054_r_ops);
}
void gc1054_l_sensor_init(int cam_idx)
{
    //dbg_msg_camera("   <%s>\n", __func__);
    kdp_sensor_register(SENSOR_TYPE_GC1054_L, &gc1054_l_ops);
}
#endif
