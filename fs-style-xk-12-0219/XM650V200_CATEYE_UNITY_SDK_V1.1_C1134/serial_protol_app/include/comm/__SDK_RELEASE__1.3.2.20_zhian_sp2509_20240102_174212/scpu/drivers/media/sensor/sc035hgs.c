#include "board_kl520.h"
#if (IMGSRC_0_TYPE == SENSOR_TYPE_SC035HGS) || \
    (IMGSRC_1_TYPE == SENSOR_TYPE_SC035HGS)
#include <stdlib.h>
#include "base.h"
#include "framework/init.h"
#include "framework/v2k.h"
#include "framework/v2k_image.h"
#include "framework/framework_errno.h"
#include "media/camera/sensor.h"
#include "kdp520_i2c.h"
#include "kdp_sensor.h"
#include "dbg.h"
#include "kl520_include.h"

#if IMGSRC_0_TYPE == SENSOR_TYPE_SC035HGS
#define IMG_RES             IMGSRC_0_RES
#define IMG_FORMAT          IMGSRC_0_FORMAT
#define IMG_MIPILANE_NUM    IMGSRC_0_MIPILANE_NUM
#endif
#if IMGSRC_1_TYPE == SENSOR_TYPE_SC035HGS
#define IMG_RES             IMGSRC_1_RES
#define IMG_FORMAT          IMGSRC_1_FORMAT
#define IMG_MIPILANE_NUM    IMGSRC_1_MIPILANE_NUM
#endif

#define SLAVE_ID			0x30   //7bit

static const struct sensor_datafmt_info sc035hgs_colour_fmts[] = {
    { V2K_PIX_FMT_RAW8, V2K_COLORSPACE_RAW },
};

static const struct sensor_win_size sc035hgs_supported_win_sizes[] = {
    //{ .width = TFT43_WIDTH, .height = TFT43_HEIGHT, },
    { .width = VGA_LANDSCAPE_WIDTH, .height = VGA_LANDSCAPE_HEIGHT, },
    { .width = VGA_PORTRAIT_WIDTH, .height = VGA_PORTRAIT_HEIGHT, },
    //{ .width = SC132_FULL_RES_WIDTH, .height = SC132_FULL_RES_HEIGHT, },
};

static struct sensor_device sc035hgs_dev = {
    .addr = 0x30,
    .device_id = 0XFFFF,
};

#define CALC_FPS     (40)
#define CALC_VTS     (0x044C)    // {16h'0x320e, 16h'0x320f}
struct sensor_init_seq INITDATA sc035hgs_init_regs[] = {
{ 0x0103,0x01 },
{ 0x0100,0x00 },
{ 0x36e9,0x80 },
{ 0x36f9,0x80 },
{ 0x3000,0x00 },
{ 0x3001,0x00 },
{ 0x300f,0x0f },
{ 0x3018,0x33 },
{ 0x3019,0xfc },
{ 0x301c,0x78 },
{ 0x301f,0x9b },
{ 0x3031,0x08 },
{ 0x3037,0x00 },
{ 0x303f,0x01 },
//HTS
{ 0x320c,0x05 },
{ 0x320d,0x54 },
//VTS
//{ 0x320e,0x0b },  //CALC_VTS  //15fps
//{ 0x320f,0x74 },  //CALC_VTS  //15fps
{ 0x320e,0x08 },  //CALC_VTS  //20fps
{ 0x320f,0x98 },  //CALC_VTS  //20fps
//{ 0x320e,0x06 },  //CALC_VTS  //25fps
//{ 0x320f,0xE0 },  //CALC_VTS  //25fps
//{ 0x320e,0x05 },  //CALC_VTS  //30fps
//{ 0x320f,0xba },  //CALC_VTS  //30fps
//{ 0x320e,0x04 },  //CALC_VTS  //40fps
//{ 0x320f,0x4c },  //CALC_VTS  //40fps

{ 0x3217,0x00 },
{ 0x3218,0x00 },
{ 0x3220,0x10 },
{ 0x3223,0x48 },
{ 0x3226,0x74 },
{ 0x3227,0x07 },
{ 0x323b,0x00 },
{ 0x3250,0xf0 },
{ 0x3251,0x02 },
//{ 0x3252,0x02 },
//{ 0x3253,0x08 },
{ 0x3252,0x04 },
{ 0x3253,0x44 },
{ 0x3254,0x02 },
{ 0x3255,0x07 },
{ 0x3304,0x48 },
{ 0x3305,0x00 },
{ 0x3306,0x98 },
{ 0x3309,0x50 },
{ 0x330a,0x01 },
{ 0x330b,0x18 },
{ 0x330c,0x18 },
{ 0x330f,0x40 },
{ 0x3310,0x10 },
{ 0x3314,0x1e },
{ 0x3315,0x30 },
{ 0x3316,0x68 },
{ 0x3317,0x1b },
{ 0x3329,0x5c },
{ 0x332d,0x5c },
{ 0x332f,0x60 },
{ 0x3335,0x64 },
{ 0x3344,0x64 },
{ 0x335b,0x80 },
{ 0x335f,0x80 },
{ 0x3366,0x06 },
{ 0x3385,0x31 },
{ 0x3387,0x39 },
{ 0x3389,0x01 },
{ 0x33b1,0x03 },
{ 0x33b2,0x06 },
{ 0x33bd,0xe0 },
{ 0x33bf,0x10 },
{ 0x3621,0xa4 },
{ 0x3622,0x05 },
{ 0x3624,0x47 },
{ 0x3630,0x4a },
{ 0x3631,0x58 },
{ 0x3633,0x52 },
{ 0x3635,0x03 },
{ 0x3636,0x25 },
{ 0x3637,0x8a },
{ 0x3638,0x0f },
{ 0x3639,0x08 },
{ 0x363a,0x00 },
{ 0x363b,0x48 },
{ 0x363c,0x86 },
{ 0x363e,0xf8 },
{ 0x3640,0x00 },
{ 0x3641,0x01 },
{ 0x36ea,0x38 },
{ 0x36eb,0x0d },
{ 0x36ec,0x1d },
{ 0x36ed,0x20 },
{ 0x36fa,0x36 },
{ 0x36fb,0x10 },
{ 0x36fc,0x02 },
{ 0x36fd,0x00 },
{ 0x3908,0x91 },
{ 0x391b,0x81 },
{ 0x3d08,0x01 },
// AEC
//{ 0x3e01,0xb6 },// 46000, for 15fps
//{ 0x3e02,0xc0 },
{ 0x3e01,0x86 },// 34500, for 20fps
{ 0x3e02,0xc4 },
//{ 0x3e01,0x5b },// 23320, for 30fps
//{ 0x3e02,0x20 },
//{ 0x3e01,0x6b },// 27600, for 25fps
//{ 0x3e02,0xd0 },
//{ 0x3e01,0x2A },// 10800, for 40 fps
//{ 0x3e02,0x30 },
//{ 0x3e01,0x3a },// 1000, for 40 fps
//{ 0x3e02,0x98 },
{ 0x3e03,0x2b },
{ 0x3e06,0x0c },
{ 0x3f04,0x05 },
{ 0x3f05,0x34 },
{ 0x4500,0x59 },
{ 0x4501,0xc4 },
{ 0x4603,0x00 },
{ 0x4800,0x64 },
{ 0x4809,0x01 },
{ 0x4810,0x00 },
{ 0x4811,0x01 },
{ 0x4837,0x42 },
{ 0x5011,0x00 },
{ 0x5988,0x02 },
{ 0x598e,0x05 },
{ 0x598f,0x17 },
{ 0x36e9,0x20 },
{ 0x36f9,0x20 },
{ 0x0100,0x01 },

{ 0x4418,0x0a },   //Delay 10ms
{ 0x363d,0x10 },
{ 0x4419,0x80 },

//gain
{ 0x3e08, 0x03 }, //gain as 4.0
{ 0x3e09, 0x10 },
{ 0x3314, 0x70 },
{ 0x3317, 0x14 },//0826
{ 0x3631, 0x48 },
{ 0x3630, 0x4c },

{0x0000,0x00},
};

int sc035hgs_i2c_init(void)
{
    int ret = -1;

    ret = 0;

    return ret;
}

static int sc035hgs_write_reg(struct sensor_device *sensor_dev, u16 reg, u8 data)
{
    int ret;

    ret = kdp_drv_i2c_write(I2C_ADAP_0, sensor_dev->addr, reg, 2, data);

    return ret;
}

static int sc035hgs_read_reg(struct sensor_device *sensor_dev, u16 reg, u8 *data)
{
    int ret;

    ret = kdp_drv_i2c_read(I2C_ADAP_0, sensor_dev->addr, reg, 2, data);

    return ret;
}

int sc035hgs_init(struct sensor_device *sensor_dev, struct sensor_init_seq *seq, u32 length)
{
    struct sensor_device *dev = sensor_dev;
    struct sensor_init_seq *init_fnc_ptr;
    //dbg_msg_camera(" sc035hgs_init ");
    for (u32 i = 0; i < length; i++) {
        if (init_fnc_ptr[i].addr == 0 && init_fnc_ptr[i].value == 0) break; //reaches end
        sc035hgs_write_reg(dev, init_fnc_ptr[i].addr , (u8)(init_fnc_ptr[i].value & 0xFF));
    }

    //dbg_msg_camera("sc035hgs_init init over, sensor ID = 0x%x", dev->device_id);
    u8 data = 0;
    sc035hgs_read_reg(dev, 0x3107, &data);
    //dbg_msg_camera("sc035hgs_init sensor high id=%x", data);
    dev->device_id = data << 8;
    sc035hgs_read_reg(dev, 0x3108, &data);
    //dbg_msg_camera("sc035hgs_init sensor low =%x", data);
    dev->device_id |= data;
    //dbg_msg_console("sc035hgs_init device id =%x", dev->device_id);
    if(dev->device_id != 0x031)
        return -1;

    return 0;
}


static const struct sensor_win_size *sc035hgs_select_win(u32 *width, u32 *height)
{
    int i, default_size = ARRAY_SIZE(sc035hgs_supported_win_sizes) - 1;

    for (i = 0; i < ARRAY_SIZE(sc035hgs_supported_win_sizes); i++) {
        if (sc035hgs_supported_win_sizes[i].width  >= *width &&
            sc035hgs_supported_win_sizes[i].height >= *height) {
            *width = sc035hgs_supported_win_sizes[i].width;
            *height = sc035hgs_supported_win_sizes[i].height;
            return &sc035hgs_supported_win_sizes[i];
        }
    }

    *width = sc035hgs_supported_win_sizes[default_size].width;
    *height = sc035hgs_supported_win_sizes[default_size].height;
    return &sc035hgs_supported_win_sizes[default_size];
}

static int sc035hgs_set_params(
        struct sensor_device *sensor_dev, u32 *width, u32 *height, u32 fourcc)
{
    switch (fourcc) {
        case V2K_PIX_FMT_RGB565:
            dbg_msg_camera("%s: Selected V2K_PIX_FMT_RGB565", __func__);
            break;

        case V2K_PIX_FMT_RAW8:
            dbg_msg_camera("%s: Selected V2K_PIX_FMT_RAW8", __func__);
            break;
        default:;
    }

    /* reset hardware */
    //sc035hgs_reset(sensor_dev);
    /* initialize the sensor with default settings */
    return sc035hgs_init(sensor_dev, sc035hgs_init_regs, sizeof(sc035hgs_init_regs)/sizeof(sc035hgs_init_regs[0]));

}

static int sc035hgs_sensor_enum_fmt(unsigned int index, unsigned int *code)
{
    if (index >= ARRAY_SIZE(sc035hgs_colour_fmts))
        return -KDP_FRAMEWORK_ERRNO_INVALA;

    //critical_msg("   <%s>\n", __func__);
    *code = sc035hgs_colour_fmts[index].fourcc;
    return 0;
}

static int sc035hgs_sensor_set_fmt(struct cam_format *fmt)
{
    //critical_msg("   <%s>\n", __func__);

    sc035hgs_select_win(&fmt->width, &fmt->height);

    return sc035hgs_set_params(&sc035hgs_dev, &fmt->width, &fmt->height, fmt->pixelformat);
}

static int sc035hgs_sensor_set_gain(u8 ana_gn1, u8 ana_gn2)
{
	static u8 nPreGain1 = 0;
	static u8 nPreGain2 = 0;
    u8 data = 0;

	if ( ana_gn1 != nPreGain1 || ana_gn2 != nPreGain2 )
	{
		kdp_drv_i2c_read(I2C_ADAP_0, SLAVE_ID, 0x3e08, 2, &data);
		data &= ~0x1C;  //bit[4:2]

		//group hold: setting and wait
		kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3235, 2, 0x00);  //start row in frame
		kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3236, 2, 0x00);  //start row in frame
		kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3802, 2, 0x00);  //frame delay
		kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3812, 2, 0x00);  //group hold start

		data |= ana_gn1 << 2;
		kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3e08, 2, data);
		kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3e09, 2, ana_gn2);

		if ( ana_gn1 == 0 )  //gain < 2
		{
			kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3314, 2, 0x70);
			kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3317, 2, 0x14);
			kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3631, 2, 0x58);
			kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3630, 2, 0x4A);
		}
		else if ( ana_gn1 == 1 )  //2 <= gain < 4
		{
			kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3314, 2, 0x70);
			kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3317, 2, 0x14);
			kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3631, 2, 0x48);
			kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3630, 2, 0x4c);
		}
		else  //gain > 4
		{
			kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3314, 2, 0x78);
			kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3317, 2, 0x15);
			kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3631, 2, 0x48);
			kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3630, 2, 0x4c);
		}

		//group hold:write
		kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3812, 2, 0x30);  //group hold end

		nPreGain1 = ana_gn1;
		nPreGain2 = ana_gn2;
	}
	
    return 0;
}

static int sc035hgs_sensor_set_exp_time(u8 ana_gn1, u8 ana_gn2)
{
    kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3e01, 2, ana_gn1);
    kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3e02, 2, ana_gn2);

    return 0;
}

static int sc035hgs_sensor_set_mirror(BOOL enable)
{
    u8 data = 0;
	
    kdp_drv_i2c_read(I2C_ADAP_0, SLAVE_ID, 0x3221, 2, &data);
    data &= ~0x06;

    if ( enable )
	{
        kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3221, 2, data | 0x06);
    }
    else
	{
        kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3221, 2, data);
    }

    return 0;
}

static int sc035hgs_sensor_set_flip(BOOL enable)
{
    u8 data = 0;
    kdp_drv_i2c_read(I2C_ADAP_0, SLAVE_ID, 0x3221, 2, &data);
    data &= ~0x60;

    if (enable)
    {
        kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3221, 2, data | 0x60);
    }
    else
	{
        kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3221, 2, data);
    }

    return 0;
}

static int sc035hgs_sensor_get_id(void)
{
    //critical_msg("   <%s>\n", __func__);

    return sc035hgs_dev.device_id;
}

static int sc035hgs_sensor_set_strobe(BOOL enable)
{
    u8 data;

    if (enable)
    {
    	data = 0x0;//0x0  -> strobe en
					   //0x80;	on
	}
	else
	{
		data = 0xc0;	//off
	}

    kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x3361, 2, data);
	
	return 0;
}

static int sc035hgs_sensor_set_fps(u8 fps)
{
    u8 data;
	
    u32 dst_vts = (u32)(CALC_VTS * CALC_FPS / fps);
    data = (dst_vts >> 8) & 0x000000FF;
    kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x320e, 2, data);
    // dbg_msg_console("[%s] reg addr:0x320e, data:%#x", __func__, data);

    data = dst_vts & 0x000000FF;
    kdp_drv_i2c_write(I2C_ADAP_0, SLAVE_ID, 0x320f, 2, data);
    // dbg_msg_console("[%s] reg addr:0x320f, data:%#x", __func__, data);

    return 0;
}



static struct sensor_ops sc035hgs_ops = {
//    .s_power    = sc035hgs_sensor_power,
//    .reset      = sc035hgs_sensor_reset,
//    .s_stream   = sc035hgs_sensor_stream,
    .enum_fmt           = sc035hgs_sensor_enum_fmt,
//    .get_fmt    = sc035hgs_sensor_get_fmt,
    .set_fmt            = sc035hgs_sensor_set_fmt,
    .set_gain           = sc035hgs_sensor_set_gain,
    .set_exp_time       = sc035hgs_sensor_set_exp_time,
    .get_lux            = NULL,
    .set_aec_roi        = NULL,
    .set_mirror         = sc035hgs_sensor_set_mirror,
    .set_flip           = sc035hgs_sensor_set_flip,
	.set_led			= sc035hgs_sensor_set_strobe,
    .get_id             = sc035hgs_sensor_get_id,
    .set_fps            = sc035hgs_sensor_set_fps,
};

void sc035hgs_sensor_init(int cam_idx)
{
    //critical_msg("   <%s>\n", __func__);
    kdp_sensor_register(SENSOR_TYPE_SC035HGS, &sc035hgs_ops);
}

#endif
