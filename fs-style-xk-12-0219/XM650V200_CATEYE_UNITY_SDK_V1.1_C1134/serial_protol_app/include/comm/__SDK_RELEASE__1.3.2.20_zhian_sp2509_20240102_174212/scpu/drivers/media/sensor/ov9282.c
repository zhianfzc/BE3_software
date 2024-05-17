#include "board_kl520.h"
#if (IMGSRC_0_TYPE == SENSOR_TYPE_OV9282_R) || \
    (IMGSRC_1_TYPE == SENSOR_TYPE_OV9282_R) || \
    (IMGSRC_0_TYPE == SENSOR_TYPE_OV9282_L) || \
    (IMGSRC_1_TYPE == SENSOR_TYPE_OV9282_L)
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

#if ( IMGSRC_0_TYPE == SENSOR_TYPE_OV9282_R || IMGSRC_0_TYPE == SENSOR_TYPE_OV9282_L )
#define IMG_RES             IMGSRC_0_RES
#define IMG_FORMAT          IMGSRC_0_FORMAT
#define IMG_MIPILANE_NUM    IMGSRC_0_MIPILANE_NUM
#define IMG_MIRROR          CFG_SENSOR_0_FMT_MIRROR
#define IMG_FLIP            CFG_SENSOR_0_FMT_FLIP
#elif ( IMGSRC_1_TYPE == SENSOR_TYPE_OV9282_R || IMGSRC_1_TYPE == SENSOR_TYPE_OV9282_L )
#define IMG_RES             IMGSRC_1_RES
#define IMG_FORMAT          IMGSRC_1_FORMAT
#define IMG_MIPILANE_NUM    IMGSRC_1_MIPILANE_NUM
#define IMG_MIRROR          CFG_SENSOR_1_FMT_MIRROR
#define IMG_FLIP            CFG_SENSOR_1_FMT_FLIP
#endif


static const struct sensor_datafmt_info ov9282_colour_fmts[] = {
    { V2K_PIX_FMT_RAW8, V2K_COLORSPACE_RAW },
}; 

static const struct sensor_win_size ov9282_supported_win_sizes[] = {    
    { .width = HD_WIDTH, .height = HD_HEIGHT, },
};

static struct sensor_device ov9282_dev_l = {
    .i2c_id = I2C_ADAP_0,
    .addr = 0x60,
    .device_id = 0XFFFF,
};

static struct sensor_device ov9282_dev_r = {
    .i2c_id = I2C_ADAP_0,
    .addr = 0x60,
    .device_id = 0XFFFF,
};

struct sensor_init_seq INITDATA ov9282_init_regs[] = {  
#if 0
{0x0100,0x00},//software standby mode
{0x0103,0x01},//software_reset
{0x0302,0x32},//PLL1: mul, 16MHz*50=800MHz
{0x0303,0x01}, //MIPI_PHY_CLK 400Mbps
//{0x0303,0x03},  //MIPI_PHY_CLK 200Mbps
{0x030d,0x3f},
{0x030e,0x04},
{0x3001,0x00},//drive strength control
{0x3004,0x00},//GPIO2 output enable : input. D9 output enable : input
{0x3005,0x00},//Bit[0] ~ B it[7] : input
{0x3006,0x04},//ILPWM : output. D0/PCLK/HREF/Strobe output/VSYNC : input
{0x3011,0x0a},//mipi_pad : 1, pgm_vcm : high speed common mode voltage
{0x3013,0x18},//pgm_lptx (Driving strength control of low speed tx), r_iref
{0x301c,0xf0},//sclk_bist/sclk_srb/sclk_grp : 1
{0x3022,0x01},//pd_mipi enable when rst_sync
{0x3030,0x10},//r_aslp_repeat
{0x3039,0x32},//mipi_en (0:DVB 1:MIPI) phy_rst (1:Reset PHY when rst_sync)
{0x303a,0x00},
{0x3500,0x00},
						
{0x3501,0x1E},
{0x3502,0xc0},
						
{0x3503,0x0b},//gain_prec16_en
{0x3505,0x8c},//gain conversation option. dac_finegain_highbit : 1;
{0x3507,0x03},//GAIN SHIFT. left shift 3 bit.
{0x3508,0x00},//Debug mode
{0x3509,0x10},//Gain. 1x gain
{0x3610,0x80},
{0x3611,0xa0},
{0x3620,0x6e},
{0x3632,0x56},
{0x3633,0x78},
{0x3662,0x02},//RAW8
{0x3666,0x00},// output selection 0x0:VSYNC. from fsin pin, used for both frame sync and frame rigger function
{0x366f,0x5a},
{0x3670,0x68},
{0x3680,0x84},
{0x3712,0x00},
{0x372d,0x22},
{0x3731,0x80},
{0x3732,0x30},
{0x3778,0x00},
{0x377d,0x22},
{0x3788,0x02},
{0x3789,0xa4},
{0x378a,0x00},
{0x378b,0x4a},
{0x3799,0x20},
{0x379c,0x02},
{0x3800,0x00},// array horizontal start point high
{0x3801,0x00},// array horizontal start point low
{0x3802,0x00},// array vertical start point high
{0x3803,0x28},// array vertical start point low

{0x3804,0x05},// array horizontal end point high byte
{0x3805,0x0f},// array horizontal end point low byte
{0x3806,0x03},// array vertical end point high byte
{0x3807,0x07},// array vertical end point low byte
{0x3808,0x05},  //ISP ?????????	1280
{0x3809,0x00},
{0x380a,0x02},  //ISP ?????????	720
{0x380b,0xd0},
{0x380c,0x05},
{0x380d,0x8e},
{0x380e,0x03},
{0x380f,0xd8},
{0x3810,0x00},
{0x3811,0x08},
{0x3812,0x00},
{0x3813,0x08},
{0x3814,0x11},
{0x3815,0x11},
{0x3820,0x40},
{0x3821,0x00},
{0x382b,0x5a},
{0x382c,0x09},
{0x382d,0x9a},
{0x389d,0x00},
{0x3881,0x42},
{0x3882,0x04},
{0x3883,0xeb},
{0x3885,0x01},
{0x389d,0x03},
{0x38a8,0x02},
{0x38a9,0x80},
{0x38b1,0x00},
{0x38b3,0x01},
{0x38c4,0x00},
{0x38c5,0xc0},
{0x38c6,0x02},
{0x38c7,0x60},
{0x3920,0xff},
{0x4003,0x40},
{0x4008,0x04},// r_up_bl_start_o
{0x4009,0x0b},// r_up_bl_end_o
{0x400c,0x00},// r_dn_bl_start_o
{0x400d,0x07},// r_dn_bl_end_o
{0x4010,0x40},// r_gain_chg_trig_en_o
{0x4043,0x40},// r_bot_blk_in_en_o
{0x4307,0x30},// embed_st=3
{0x4317,0x00},// dvp enable = 0
{0x4501,0x00},
{0x4507,0x00},
{0x4509,0x00},
{0x450a,0x08},
{0x4601,0x04},
{0x470f,0x00},
{0x4f07,0x00},
{0x4800,0x00},
{0x4837,0x28},
{0x5000,0x9f},
{0x5001,0x00},
{0x5e00,0x00},  //  Bit[7]: Test pattern bar enable
{0x5d00,0x07},
{0x5d01,0x00},
{0x4f00,0x04},
{0x4f10,0x00},
{0x4f11,0x98},
{0x4f12,0x0f},
{0x4f13,0xc4},
						
{0x3006,0x0c},
{0x3210,0x04},
{0x3007,0x02},
{0x301c,0xf2},
{0x3020,0x20},
{0x3025,0x02},
{0x382c,0x09},
{0x382d,0x9a},
{0x3920,0xff},
{0x3923,0x00},
{0x3924,0x00},
{0x3925,0x00},
{0x3926,0x00},
{0x3927,0x00},
{0x3928,0x80},
{0x392b,0x00},
{0x392c,0x00},
{0x392d,0x05},
{0x392e,0x8e},
{0x392f,0xcb},
{0x38b3,0x07},
{0x3885,0x07},
{0x382b,0x5a},
{0x3670,0x68},
{0x3208,0x00},
{0x3501,0x02},
{0x3502,0x01},
{0x3508,0x00},
{0x3509,0xa0},
{0x3927,0x00},
{0x3928,0x93},
{0x3929,0x03},
{0x392A,0x3d},
{0x3208,0x10},
{0x3208,0xA0},
						
{0x0100,0x01},
{0,0},
#else
{0x0103, 0x01},
{0x0302, 0x32}, //50
{0x0303, 0x01},
{0x030d, 0x3f},
{0x030e, 0x04},
{0x3001, 0x00},
{0x3004, 0x00},
{0x3005, 0x00},
{0x3006, 0x04},
{0x3011, 0x0a},
{0x3013, 0x18},
{0x301c, 0xf0},
{0x3022, 0x01},
{0x3030, 0x10},
{0x3039, 0x32}, //0x12
{0x303a, 0x00},
{0x3500, 0x00},

{0x3501, 0x1E},
{0x3502, 0xc0},

{0x3503, 0x0b}, //08 // AEC_MANUAL bit1 AGC_MANUAL, bit0 AEC_MANUAL
{0x3505, 0x8c},
{0x3507, 0x03},
{0x3508, 0x00},
{0x3509, 0x10}, //{0x3509,0x10}, {0x3509,0x20},
{0x3610, 0x80},
{0x3611, 0xa0},
{0x3620, 0x6e},
{0x3632, 0x56},
{0x3633, 0x78},
{0x3662, 0x03},//raw8 0x03, raw10 0x1
{0x3666, 0x00},
{0x366f, 0x5a},
{0x3670, 0x68},
{0x3680, 0x84},
{0x3712, 0x00},
{0x372d, 0x22},
{0x3731, 0x80},
{0x3732, 0x30},
{0x3778, 0x00},
{0x377d, 0x22},
{0x3788, 0x02},
{0x3789, 0xa4},
{0x378a, 0x00},
{0x378b, 0x4a},
{0x3799, 0x20},
{0x379c, 0x02},
{0x3800, 0x00},
{0x3801, 0x00},
{0x3802, 0x00},
{0x3803, 0x28},
{0x3804, 0x05},
{0x3805, 0x0f},
{0x3806, 0x03},
{0x3807, 0x07},
{0x3808, 0x05}, //x
{0x3809, 0x00}, //x
{0x380a, 0x02}, //y
{0x380b, 0xd0}, //y
{0x380c, 0x05},
{0x380d, 0x8e},
{0x380e, 0x03},
{0x380f, 0xd8},
{0x3810, 0x00},
{0x3811, 0x08},
{0x3812, 0x00},
{0x3813, 0x08},
{0x3814, 0x11},
{0x3815, 0x11},
{0x3820, 0x40}, //flip 0x44
{0x3821, 0x00}, //mirror 0x04
{0x382b, 0x5a},
{0x382c, 0x09},
{0x382d, 0x9a},
{0x389d, 0x00},
{0x3881, 0x42},
{0x3882, 0x04},
{0x3883, 0xeb},
{0x3885, 0x01},
{0x389d, 0x03},
{0x38a8, 0x02},
{0x38a9, 0x80},
{0x38b1, 0x00},
{0x38b3, 0x01},
{0x38c4, 0x00},
{0x38c5, 0xc0},
{0x38c6, 0x02},
{0x38c7, 0x60},
{0x3920, 0xff},
{0x4003, 0x40},
{0x4008, 0x04},
{0x4009, 0x0b},
{0x400c, 0x00},
{0x400d, 0x07},
{0x4010, 0x40},
{0x4043, 0x40},
{0x4307, 0x30},
{0x4317, 0x00},
{0x4501, 0x00},
{0x4507, 0x00},
{0x4509, 0x00},
{0x450a, 0x08},
{0x4601, 0x04},
{0x470f, 0x00},
{0x4f07, 0x00},
{0x4800, 0x00},
{0x4837, 0x28},
{0x5000, 0x9f},
{0x5001, 0x00},
{0x5e00, 0x00}, //test patten
{0x5d00, 0x07},
{0x5d01, 0x00},
{0x4f00, 0x04},
{0x4f10, 0x00},
{0x4f11, 0x98},
{0x4f12, 0x0f},
{0x4f13, 0xc4},

{0x0100, 0x01},	// MODE_SELECT : stream

{0x3006, 0x0c},
{0x3210, 0x04},
{0x3007, 0x02},
{0x301c, 0xf2},
{0x3020, 0x20},
{0x3025, 0x02},
{0x382c, 0x09},
{0x382d, 0x9a},
{0x3920, 0xff},
{0x3923, 0x00},
{0x3924, 0x00},
{0x3925, 0x00},
{0x3926, 0x00},
{0x3927, 0x00},
{0x3928, 0x80},
{0x392b, 0x00},
{0x392c, 0x00},
{0x392d, 0x05},
{0x392e, 0x8e},
{0x392f, 0xcb},
{0x38b3, 0x07},
{0x3885, 0x07},
{0x382b, 0x5a},
{0x3670, 0x68},
{0x3208, 0x00},
{0x3501, 0x02},//0x09,
{0x3502, 0x01},//36,
{0x3508, 0x00},
{0x3509, 0xa0},
{0x3927, 0x00},
{0x3928, 0x93},
{0x3929, 0x03},
{0x392A, 0x3d},
{0x3208, 0x10},
{0x3208, 0xA0},

{0x0000, 0x00},
#endif

};

int ov9282_r_i2c_init(void)
{
    int ret = -1;

    ret = 0;
    
    return ret;
}
int ov9282_l_i2c_init(void)
{
    int ret = -1;

    ret = 0;
    
    return ret;
}

static int ov9282_write_reg(struct sensor_device *sensor_dev, u16 reg, u8 data)
{
    int ret;

    ret = kdp_drv_i2c_write(sensor_dev->i2c_id, sensor_dev->addr, reg, 2, data);
    
    if (ret != 0)
        dbg_msg_console("[%s] ret:%d", __func__, ret);

    return ret;
}

static int ov9282_read_reg(struct sensor_device *sensor_dev, u16 reg, u8 *data)
{
	int ret;

    ret = kdp_drv_i2c_read(sensor_dev->i2c_id, sensor_dev->addr, reg, 2, data);

	return ret;
}

int ov9282_init(struct sensor_device *sensor_dev, struct sensor_init_seq *seq, u32 length)
{
    struct sensor_device *dev = sensor_dev;
    struct sensor_init_seq *init_fnc_ptr = seq;
    //dbg_msg_camera(" ov9282_init ");
    for (u32 i = 0; i < length; i++) {
        if (init_fnc_ptr[i].addr == 0 && init_fnc_ptr[i].value == 0) break; //reaches end

        // Update mirror and flip setting
//        if (init_fnc_ptr[i].addr == 0x17)
//        {
//            if (((CFG_SENSOR_0_TYPE == SENSOR_TYPE_OV9282_R) && (&ov9282_dev_r == dev)) ||
//                ((CFG_SENSOR_0_TYPE == SENSOR_TYPE_OV9282_L) && (&ov9282_dev_l == dev)))
//                init_fnc_ptr[i].value = 0xc0 | (sensor_0_mirror) | (sensor_0_flip << 1);
//            else if (((CFG_SENSOR_1_TYPE == SENSOR_TYPE_OV9282_R) && (&ov9282_dev_r == dev)) ||
//                     ((CFG_SENSOR_1_TYPE == SENSOR_TYPE_OV9282_L) && (&ov9282_dev_l == dev)))
//                init_fnc_ptr[i].value = 0xc0 | (sensor_1_mirror) | (sensor_1_flip << 1);
//        }
        
        ov9282_write_reg(dev, init_fnc_ptr[i].addr , (u8)(init_fnc_ptr[i].value & 0xFF));
    }
    dbg_msg_camera("ov9282[%d] init over, sensor ID = 0x%x", dev->i2c_id, dev->device_id);
    u8 data = 0;
    ov9282_read_reg(dev, 0x300A, &data);
    dbg_msg_console("ov9282[%d] sensor high id=%x", dev->i2c_id, data);
    dev->device_id = data << 8;
    ov9282_read_reg(dev, 0x300B, &data);
    dbg_msg_console("ov9282[%d] sensor low id=%x", dev->i2c_id, data);
    dev->device_id |= data;
    
//    if(dev->device_id != 0x1054)
//        return -1;
    
    return 0;
}

static const struct sensor_win_size *ov9282_select_win(u32 *width, u32 *height)
{
    int i, default_size = ARRAY_SIZE(ov9282_supported_win_sizes) - 1;

    for (i = 0; i < ARRAY_SIZE(ov9282_supported_win_sizes); i++) {
        if (ov9282_supported_win_sizes[i].width  >= *width &&
            ov9282_supported_win_sizes[i].height >= *height) {
            *width = ov9282_supported_win_sizes[i].width;
            *height = ov9282_supported_win_sizes[i].height;
            return &ov9282_supported_win_sizes[i];
        }
    }

    *width = ov9282_supported_win_sizes[default_size].width;
    *height = ov9282_supported_win_sizes[default_size].height;
    return &ov9282_supported_win_sizes[default_size];
}

static int ov9282_set_params(
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
    //ov9282_reset(sensor_dev);

    /* initialize the sensor with default settings */
    return ov9282_init(sensor_dev, ov9282_init_regs, sizeof(ov9282_init_regs)/sizeof(ov9282_init_regs[0]));
}
static int ov9282_sensor_enum_fmt(unsigned int index, unsigned int *code)
{
    if (index >= ARRAY_SIZE(ov9282_colour_fmts))
        return -KDP_FRAMEWORK_ERRNO_INVALA;

    //dbg_msg_camera("   <%s>\n", __func__);
    *code = ov9282_colour_fmts[index].fourcc;
    return 0;
}

static int ov9282_r_sensor_set_fmt(struct cam_format *fmt)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    ov9282_select_win(&fmt->width, &fmt->height);

    return ov9282_set_params(&ov9282_dev_r, &fmt->width, &fmt->height, fmt->pixelformat);
}

static int ov9282_l_sensor_set_fmt(struct cam_format *fmt)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    ov9282_select_win(&fmt->width, &fmt->height);

    return ov9282_set_params(&ov9282_dev_l, &fmt->width, &fmt->height, fmt->pixelformat);
}

static int ov9282_l_sensor_get_id(void)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    return ov9282_dev_l.device_id;
}

static int ov9282_r_sensor_get_id(void)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    return ov9282_dev_r.device_id;
}


static struct sensor_ops ov9282_r_ops = {
//    .s_power    = ov9282_sensor_power,
//    .reset      = ov9282_sensor_reset,
//    .s_stream   = ov9282_sensor_stream,
    .enum_fmt           = ov9282_sensor_enum_fmt,
//    .get_fmt    = ov9282_sensor_get_fmt,
    .set_fmt            = ov9282_r_sensor_set_fmt,
    .set_gain           = NULL,
    .set_exp_time       = NULL,
    .get_lux            = NULL,
    .set_aec_roi        = NULL,
    .set_mirror         = NULL,
    .set_flip           = NULL,
    .get_id             = ov9282_r_sensor_get_id,
    .set_fps            = NULL,
    .sleep              = NULL,
};
static struct sensor_ops ov9282_l_ops = {
//    .s_power    = ov9282_sensor_power,
//    .reset      = ov9282_sensor_reset,
//    .s_stream   = ov9282_sensor_stream,
    .enum_fmt           = ov9282_sensor_enum_fmt,
//    .get_fmt    = ov9282_sensor_get_fmt,
    .set_fmt            = ov9282_l_sensor_set_fmt,
    .set_gain           = NULL,
    .set_exp_time       = NULL,
    .get_lux            = NULL,
    .set_aec_roi        = NULL,
    .set_mirror         = NULL,
    .set_flip           = NULL,
    .get_id             = ov9282_l_sensor_get_id,
    .set_fps            = NULL,
    .sleep              = NULL,
};
void ov9282_r_sensor_init(int cam_idx)
{
    //dbg_msg_camera("   <%s>\n", __func__);
    kdp_sensor_register(SENSOR_TYPE_OV9282_R, &ov9282_r_ops);
}
void ov9282_l_sensor_init(int cam_idx)
{
    //dbg_msg_camera("   <%s>\n", __func__);
    kdp_sensor_register(SENSOR_TYPE_OV9282_L, &ov9282_l_ops);
}
#endif
