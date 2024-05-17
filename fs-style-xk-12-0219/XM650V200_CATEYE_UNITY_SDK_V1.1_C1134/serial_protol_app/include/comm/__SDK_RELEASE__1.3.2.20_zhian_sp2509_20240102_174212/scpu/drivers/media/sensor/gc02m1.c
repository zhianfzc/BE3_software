#include "board_kl520.h"
#if (IMGSRC_0_TYPE == SENSOR_TYPE_GC02M1_R) || \
    (IMGSRC_1_TYPE == SENSOR_TYPE_GC02M1_R) || \
    (IMGSRC_0_TYPE == SENSOR_TYPE_GC02M1_L) || \
    (IMGSRC_1_TYPE == SENSOR_TYPE_GC02M1_L)
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

#if ( IMGSRC_0_TYPE == SENSOR_TYPE_GC02M1_R || IMGSRC_0_TYPE == SENSOR_TYPE_GC02M1_L )
#define IMG_RES             IMGSRC_0_RES
#define IMG_FORMAT          IMGSRC_0_FORMAT
#define IMG_MIPILANE_NUM    IMGSRC_0_MIPILANE_NUM
#define IMG_MIRROR          CFG_SENSOR_0_FMT_MIRROR
#define IMG_FLIP            CFG_SENSOR_0_FMT_FLIP
#elif ( IMGSRC_1_TYPE == SENSOR_TYPE_GC02M1_R || IMGSRC_1_TYPE == SENSOR_TYPE_GC02M1_L )
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

/* SENSOR MIRROR FLIP INFO */
#define GC02M1_MIRROR_NORMAL    0
#define GC02M1_MIRROR_H         1
#define GC02M1_MIRROR_V         0
#define GC02M1_MIRROR_HV        0

#if GC02M1_MIRROR_NORMAL
#define GC02M1_MIRROR	        0x80
#elif GC02M1_MIRROR_H
#define GC02M1_MIRROR	        0x81
#elif GC02M1_MIRROR_V
#define GC02M1_MIRROR	        0x82
#elif GC02M1_MIRROR_HV
#define GC02M1_MIRROR	        0x83
#else
#define GC02M1_MIRROR	        0x80
#endif

//#define CFG_GC02M1_X_Y_INVERSE  (1)
static const struct sensor_datafmt_info gc02m1_colour_fmts[] = {
    { V2K_PIX_FMT_YCBCR, V2K_COLORSPACE_YUV },
    { V2K_PIX_FMT_RGB565, V2K_COLORSPACE_RGB },
}; 

static const struct sensor_win_size gc02m1_supported_win_sizes[] = {    
    { .width = HD_WIDTH, .height = HD_HEIGHT, },
    { .width = VGA_LANDSCAPE_WIDTH, .height = VGA_LANDSCAPE_HEIGHT, },    
    { .width = TFT43_WIDTH, .height = TFT43_HEIGHT, },     
    { .width = QVGA_WIDTH, .height = QVGA_HEIGHT, },
    { .width = UGA_WIDTH, .height = UGA_HEIGHT, },
};

static struct sensor_device gc02m1_dev_l = {
    .i2c_id = I2C_ADAP_0,
    .addr = 0x37,
    .device_id = 0XFFFF,
};

static struct sensor_device gc02m1_dev_r = {
    .i2c_id = I2C_ADAP_1,
    .addr = 0x37,
    .device_id = 0XFFFF,
};

//850nm only
#if 1// ( CFG_LW3D_TYPE == CFG_LW3D_940 )



#define LSC_ON YES
#define GAMMA_1 YES
#define GAMMA_2 YES
#define BLK_ON YES
#define AEC_ON YES

struct sensor_init_seq gc02m1_init_regs[] = {  
	/*system*/
	{0xfc, 0x01},
	{0xf4, 0x41},
	{0xf5, 0xc0},
	{0xf6, 0x44},
	{0xf8, 0x38},
	{0xf9, 0x82},
	{0xfa, 0x00},
	{0xfd, 0x80},
	{0xfc, 0x81},
	{0xfe, 0x03},
	{0x01, 0x0b},
	{0xf7, 0x01},
	{0xfc, 0x80},
	{0xfc, 0x80},
	{0xfc, 0x80},
	{0xfc, 0x8e},
	/*CISCTL*/
	{0xfe, 0x00},
	{0x87, 0x09},
	{0xee, 0x72},
	{0xfe, 0x01},
	{0x8c, 0x90},
	{0xfe, 0x00},
	{0x90, 0x00},
	{0x03, 0x02},
	{0x04, 0x7E},
	{0x41, 0x02},
	{0x42, 0x8E},
	{0x05, 0x04},
	{0x06, 0x48},
	{0x07, 0x00},
	{0x08, 0x18},
	{0x9d, 0x18},
	{0x09, 0x00},
	{0x0a, 0x02},
	{0x0d, 0x04},
	{0x0e, 0xbc},
	{0x17, GC02M1_MIRROR},
	{0x19, 0x04},
	{0x24, 0x00},
	{0x56, 0x20},
	{0x5b, 0x00},
	{0x5e, 0x01},
	/*analog Register width*/
	{0x21, 0x3c},
	{0x44, 0x20},
	{0xcc, 0x01},
	/*analog mode*/
	{0x1a, 0x04},
	{0x1f, 0x11},
	{0x27, 0x30},
	{0x2b, 0x00},
	{0x33, 0x00},
	{0x53, 0x90},
	{0xe6, 0x50},
	/*analog voltage*/
	{0x39, 0x07},
	{0x43, 0x04},
	{0x46, 0x2a},
	{0x7c, 0xa0},
	{0xd0, 0xbe},
	{0xd1, 0x60},
	{0xd2, 0x40},
	{0xd3, 0xf3},
	{0xde, 0x1d},
	/*analog current*/
	{0xcd, 0x05},
	{0xce, 0x6f},
	/*CISCTL RESET*/
	{0xfc, 0x88},
	{0xfe, 0x10},
	{0xfe, 0x00},
	{0xfc, 0x8e},
	{0xfe, 0x00},
	{0xfe, 0x00},
	{0xfe, 0x00},
	{0xfe, 0x00},
	{0xfc, 0x88},
	{0xfe, 0x10},
	{0xfe, 0x00},
	{0xfc, 0x8e},
	{0xfe, 0x04},
	{0xe0, 0x01},
	{0xfe, 0x00},
	/*ISP*/
	{0xfe, 0x01},
	{0x53, 0x44},
	{0x87, 0x53},
	{0x89, 0x03},
	/*Gain*/
	{0xfe, 0x00},
	{0xb0, 0x74},
	{0xb1, 0x01},
	{0xb2, 0xF4},
	{0xb6, 0x0f},
	{0xfe, 0x04},
	{0xd8, 0x00},
	{0xc0, 0x40},
	{0xc0, 0x00},
	{0xc0, 0x00},
	{0xc0, 0x00},
	{0xc0, 0x60},
	{0xc0, 0x00},
	{0xc0, 0xc0},
	{0xc0, 0x2a},
	{0xc0, 0x80},
	{0xc0, 0x00},
	{0xc0, 0x00},
	{0xc0, 0x40},
	{0xc0, 0xa0},
	{0xc0, 0x00},
	{0xc0, 0x90},
	{0xc0, 0x19},
	{0xc0, 0xc0},
	{0xc0, 0x00},
	{0xc0, 0xD0},
	{0xc0, 0x2F},
	{0xc0, 0xe0},
	{0xc0, 0x00},
	{0xc0, 0x90},
	{0xc0, 0x39},
	{0xc0, 0x00},
	{0xc0, 0x01},
	{0xc0, 0x20},
	{0xc0, 0x04},
	{0xc0, 0x20},
	{0xc0, 0x01},
	{0xc0, 0xe0},
	{0xc0, 0x0f},
	{0xc0, 0x40},
	{0xc0, 0x01},
	{0xc0, 0xe0},
	{0xc0, 0x1a},
	{0xc0, 0x60},
	{0xc0, 0x01},
	{0xc0, 0x20},
	{0xc0, 0x25},
	{0xc0, 0x80},
	{0xc0, 0x01},
	{0xc0, 0xa0},
	{0xc0, 0x2c},
	{0xc0, 0xa0},
	{0xc0, 0x01},
	{0xc0, 0xe0},
	{0xc0, 0x32},
	{0xc0, 0xc0},
	{0xc0, 0x01},
	{0xc0, 0x20},
	{0xc0, 0x38},
	{0xc0, 0xe0},
	{0xc0, 0x01},
	{0xc0, 0x60},
	{0xc0, 0x3c},
	{0xc0, 0x00},
	{0xc0, 0x02},
	{0xc0, 0xa0},
	{0xc0, 0x40},
	{0xc0, 0x80},
	{0xc0, 0x02},
	{0xc0, 0x18},
	{0xc0, 0x5c},
	{0xfe, 0x00},
	{0x9f, 0x10},
	/*BLK*/
	{0xfe, 0x00},
	{0x26, 0x20},
	{0xfe, 0x01},
	{0x40, 0x22},
	{0x46, 0x7f},
	{0x49, 0x0f},
	{0x4a, 0xf0},
	{0xfe, 0x04},
	{0x14, 0x80},
	{0x15, 0x80},
	{0x16, 0x80},
	{0x17, 0x80},
	/*antblooming*/
	{0xfe, 0x01},
	{0x41, 0x20},
	{0x4c, 0x00},
	{0x4d, 0x0c},
	{0x44, 0x08},
	{0x48, 0x03},
	/*Window 1280x720*/
	{0xfe, 0x01},
	{0x90, 0x01},
	{0x91, 0x00},
	{0x92, 0xa0},
	{0x93, 0x00},
	{0x94, 0xf0},
	{0x95, 0x02},
	{0x96, 0xd0},
	{0x97, 0x05},
	{0x98, 0x00},
	/*mipi*/
	{0xfe, 0x03},
	{0x01, 0x23},
	{0x03, 0xce},
	{0x04, 0x48},	
  {0x11, 0x2a},
	{0x12, 0x40},
	{0x13, 0x06},
	{0x15, 0x00},
	{0x21, 0x10},
	{0x22, 0x05},
	{0x23, 0x20},
	{0x25, 0x20},
	{0x26, 0x08},
	{0x29, 0x06},
	{0x2a, 0x0a},
	{0x2b, 0x08},
	/*out*/
	{0xfe, 0x01},
	{0x8c, 0x10},
	{0xfe, 0x00},
	{0x3e, 0x94},
    
    {0x00, 0x00},
};
#endif

int gc02m1_r_i2c_init(void)
{
    int ret = -1;

    ret = 0;
    
    return ret;
}
int gc02m1_l_i2c_init(void)
{
    int ret = -1;

    ret = 0;
    
    return ret;
}

static int gc02m1_write_reg(struct sensor_device *sensor_dev, u16 reg, u8 data)
{
    int ret;

    ret = kdp_drv_i2c_write(sensor_dev->i2c_id, sensor_dev->addr, reg, 1, data);

    return ret;
}

static int gc02m1_read_reg(struct sensor_device *sensor_dev, u16 reg, u8 *data)
{
	int ret;

    ret = kdp_drv_i2c_read(sensor_dev->i2c_id, sensor_dev->addr, reg, 1, data);

	return ret;
}


int gc02m1_init(struct sensor_device *sensor_dev, struct sensor_init_seq *seq, u32 length)
{
    struct sensor_device *dev = sensor_dev;
    struct sensor_init_seq *init_fnc_ptr = seq;
    //dbg_msg_camera(" gc02m1_init ");
    for (u32 i = 0; i < length; i++) {
        if (init_fnc_ptr[i].addr == 0 && init_fnc_ptr[i].value == 0) break; //reaches end
        gc02m1_write_reg(dev, init_fnc_ptr[i].addr , (u8)(init_fnc_ptr[i].value & 0xFF));
    }
   
    dbg_msg_camera("gc02m1[%d] init over, sensor ID = 0x%x", dev->i2c_id, dev->device_id);
    u8 data = 0;
    
    gc02m1_read_reg(dev, 0xf0, &data);
    dbg_msg_camera("gc02m1[%d] sensor high id=%x", dev->i2c_id, data);
    dev->device_id = data << 8;
    gc02m1_read_reg(dev, 0xf1, &data);
    dbg_msg_camera("gc02m1[%d] sensor low id=%x", dev->i2c_id, data);
    dev->device_id |= data;
    
    if(dev->device_id != 0x02e0)
        return -1;
    
    return 0;
}

static const struct sensor_win_size *gc02m1_select_win(u32 *width, u32 *height)
{
    int i, default_size = ARRAY_SIZE(gc02m1_supported_win_sizes) - 1;

    for (i = 0; i < ARRAY_SIZE(gc02m1_supported_win_sizes); i++) {
        if (gc02m1_supported_win_sizes[i].width  >= *width &&
            gc02m1_supported_win_sizes[i].height >= *height) {
            *width = gc02m1_supported_win_sizes[i].width;
            *height = gc02m1_supported_win_sizes[i].height;
            return &gc02m1_supported_win_sizes[i];
        }
    }

    *width = gc02m1_supported_win_sizes[default_size].width;
    *height = gc02m1_supported_win_sizes[default_size].height;
    return &gc02m1_supported_win_sizes[default_size];
}

static int gc02m1_set_params(
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
    //gc02m1_reset(sensor_dev);

    /* initialize the sensor with default settings */
    return gc02m1_init(sensor_dev, gc02m1_init_regs, sizeof(gc02m1_init_regs)/sizeof(gc02m1_init_regs[0]));
}
static int gc02m1_sensor_enum_fmt(unsigned int index, unsigned int *code)
{
    if (index >= ARRAY_SIZE(gc02m1_colour_fmts))
        return -KDP_FRAMEWORK_ERRNO_INVALA;

    //dbg_msg_camera("   <%s>\n", __func__);
    *code = gc02m1_colour_fmts[index].fourcc;
    return 0;
}

static int gc02m1_r_sensor_set_fmt(struct cam_format *fmt)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    gc02m1_select_win(&fmt->width, &fmt->height);

    return gc02m1_set_params(&gc02m1_dev_r, &fmt->width, &fmt->height, fmt->pixelformat);
}

static int gc02m1_l_sensor_set_fmt(struct cam_format *fmt)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    gc02m1_select_win(&fmt->width, &fmt->height);

    return gc02m1_set_params(&gc02m1_dev_l, &fmt->width, &fmt->height, fmt->pixelformat);
}

int gc02m1_sensor_set_aec_roi(struct sensor_device *sensor_dev, u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
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

int gc02m1_r_sensor_set_aec_roi(u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
{
    struct sensor_device *sensor_dev = &gc02m1_dev_r;
    int ret = gc02m1_sensor_set_aec_roi(sensor_dev, x1, x2, y1, y2, center_x1, center_x2, center_y1, center_y2);
    return ret;
}

int gc02m1_l_sensor_set_aec_roi(u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
{
    struct sensor_device *sensor_dev = &gc02m1_dev_l;
    int ret = gc02m1_sensor_set_aec_roi(sensor_dev, x1, x2, y1, y2, center_x1, center_x2, center_y1, center_y2);
    return ret;
}

int gc02m1_sensor_get_lux(struct sensor_device *sensor_dev, u16* exposure, u8* pre_gain_h, u8* pre_gain_l, u8* global_gain, u8* y_average)
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

int gc02m1_r_sensor_get_lux(u16* exposure, u8* pre_gain_h, u8* pre_gain_l, u8* global_gain, u8* y_average)
{
    struct sensor_device *sensor_dev = &gc02m1_dev_r;
    int ret = gc02m1_sensor_get_lux(sensor_dev, exposure, pre_gain_h, pre_gain_l, global_gain, y_average);
    return ret;
}

int gc02m1_l_sensor_get_lux(u16* exposure, u8* pre_gain_h, u8* pre_gain_l, u8* global_gain, u8* y_average)
{
    struct sensor_device *sensor_dev = &gc02m1_dev_l;
    int ret = gc02m1_sensor_get_lux(sensor_dev, exposure, pre_gain_h, pre_gain_l, global_gain, y_average);
    return ret;
}

static int gc02m1_l_sensor_get_id(void)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    return gc02m1_dev_l.device_id;
}

static int gc02m1_r_sensor_get_id(void)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    return gc02m1_dev_r.device_id;
}
 
 
                             
int gc02m1_sensor_set_gain(struct sensor_device *sensor_dev, u8 ana_gn1, u8 ana_gn2)
{
  
    enum i2c_adap_id i2c_id = sensor_dev->i2c_id;
    u8 dev_addr = sensor_dev->addr;

    UINT32 gainLevelTable[17] = {
                     64,
                     96,
                    127,
                    157,
                    197,
                    226,
                    259,					
                    287,
                    318,
                    356,			
                    391,
                    419,
                    450,
                    480,
                    513,
                    646,
                    0xffffffff,
                   };
                  
    int total;	
    total = sizeof(gainLevelTable) / sizeof(UINT32);
                   
    u32 gain = 0;
    gain = ana_gn1<<8;
    gain = gain | ana_gn2;  
    
    //set to page 1
    u8 reg = 0xfe;
    u8 data = 0x00;
    kdp_drv_i2c_write(i2c_id, dev_addr, reg, 1, data);
    int Analog_Index;
    UINT32 tol_dig_gain = 0;	
    for(Analog_Index = 0; Analog_Index < total; Analog_Index++)
    {
        if((gainLevelTable[Analog_Index] <= gain)&&(gain < gainLevelTable[Analog_Index+1]))
            break;
    }	
    tol_dig_gain = gain*1024/gainLevelTable[Analog_Index];	 
    kdp_drv_i2c_write(i2c_id, dev_addr, 0xb6, 1, (u8)Analog_Index);     
    kdp_drv_i2c_write(i2c_id, dev_addr, 0xb1, 1, (u8)(tol_dig_gain >> 8));
    kdp_drv_i2c_write(i2c_id, dev_addr, 0xb2, 1, (u8)(tol_dig_gain & 0xff));
    
 
    return 0;
}

int gc02m1_r_sensor_set_gain(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &gc02m1_dev_r;
    int ret = gc02m1_sensor_set_gain(sensor_dev, ana_gn1, ana_gn2);
    return ret;
}

int gc02m1_l_sensor_set_gain(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &gc02m1_dev_l;
    int ret = gc02m1_sensor_set_gain(sensor_dev, ana_gn1, ana_gn2);
    return ret;
}

int gc02m1_sensor_set_exp_time(struct sensor_device *sensor_dev, u8 ana_gn1, u8 ana_gn2)
{
    enum i2c_adap_id i2c_id = sensor_dev->i2c_id;
    u8 dev_addr = sensor_dev->addr;

    #define FRAME_LENGTH    (639)

    u16 intt = (ana_gn1 << 8) | ana_gn2;
    u16 vts;

    if ((intt + 16) < FRAME_LENGTH)
        vts = FRAME_LENGTH;
    else
        vts = intt + 16;

    //set to page 1
    u8 reg = 0xfe;
    u8 data = 0x00;
    kdp_drv_i2c_write(i2c_id, dev_addr, reg, 1, data);

    //set exp time
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x03, 1, ana_gn1);
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x04, 1, ana_gn2);
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x41, 1, (u8)((vts >> 8) & 0xff));
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x42, 1, (u8)(vts & 0xff));

    return 0;
}

int gc02m1_r_sensor_set_exp_time(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &gc02m1_dev_r;
    int ret = gc02m1_sensor_set_exp_time(sensor_dev, ana_gn1, ana_gn2);

    return ret;
}

int gc02m1_l_sensor_set_exp_time(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &gc02m1_dev_l;
    int ret = gc02m1_sensor_set_exp_time(sensor_dev, ana_gn1, ana_gn2);

    return ret;
}

static int gc02m1_sensor_sleep(struct sensor_device *sensor_dev, BOOL enable)
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

static int gc02m1_r_sensor_sleep( BOOL enable )
{
    struct sensor_device *sensor_dev = &gc02m1_dev_r;
    int ret = gc02m1_sensor_sleep(sensor_dev, enable);
    
    return 0;
}

static int gc02m1_l_sensor_sleep( BOOL enable )
{
    struct sensor_device *sensor_dev = &gc02m1_dev_l;
    int ret = gc02m1_sensor_sleep(sensor_dev, enable);
    
    return 0;
}


static struct sensor_ops gc02m1_r_ops = {
//    .s_power    = gc02m1_sensor_power,
//    .reset      = gc02m1_sensor_reset,
//    .s_stream   = gc02m1_sensor_stream,
    .enum_fmt           = gc02m1_sensor_enum_fmt,
//    .get_fmt    = gc02m1_sensor_get_fmt,
    .set_fmt            = gc02m1_r_sensor_set_fmt,
    .set_gain           = gc02m1_r_sensor_set_gain,
    .set_exp_time       = gc02m1_r_sensor_set_exp_time,
    .get_lux            = gc02m1_r_sensor_get_lux,
    .set_aec_roi        = gc02m1_r_sensor_set_aec_roi,
    .set_mirror         = NULL,
    .set_flip           = NULL,
    .get_id             = gc02m1_r_sensor_get_id,
    .set_fps            = NULL,
    .sleep              = gc02m1_r_sensor_sleep,
};
static struct sensor_ops gc02m1_l_ops = {
//    .s_power    = gc02m1_sensor_power,
//    .reset      = gc02m1_sensor_reset,
//    .s_stream   = gc02m1_sensor_stream,
    .enum_fmt           = gc02m1_sensor_enum_fmt,
//    .get_fmt    = gc02m1_sensor_get_fmt,
    .set_fmt            = gc02m1_l_sensor_set_fmt,
    .set_gain           = gc02m1_l_sensor_set_gain,
    .set_exp_time       = gc02m1_l_sensor_set_exp_time,
    .get_lux            = gc02m1_l_sensor_get_lux,
    .set_aec_roi        = gc02m1_l_sensor_set_aec_roi,
    .set_mirror         = NULL,
    .set_flip           = NULL,
    .get_id             = gc02m1_l_sensor_get_id,
    .set_fps            = NULL,
    .sleep              = gc02m1_l_sensor_sleep,
};
void gc02m1_r_sensor_init(int cam_idx)
{
    //dbg_msg_camera("   <%s>\n", __func__);
    kdp_sensor_register(SENSOR_TYPE_GC02M1_R, &gc02m1_r_ops);
}
void gc02m1_l_sensor_init(int cam_idx)
{
    //dbg_msg_camera("   <%s>\n", __func__);
    kdp_sensor_register(SENSOR_TYPE_GC02M1_L, &gc02m1_l_ops);
}
#endif
