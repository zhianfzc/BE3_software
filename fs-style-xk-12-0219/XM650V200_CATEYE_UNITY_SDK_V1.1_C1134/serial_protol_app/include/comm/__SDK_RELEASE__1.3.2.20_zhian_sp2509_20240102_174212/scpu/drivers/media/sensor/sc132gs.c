#include "board_kl520.h"
#if (IMGSRC_0_TYPE == SENSOR_TYPE_SC132GS) || \
    (IMGSRC_1_TYPE == SENSOR_TYPE_SC132GS)
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

#if IMGSRC_0_TYPE == SENSOR_TYPE_SC132GS
#define IMG_RES             IMGSRC_0_RES
#define IMG_FORMAT          IMGSRC_0_FORMAT
#define IMG_MIPILANE_NUM    IMGSRC_0_MIPILANE_NUM
#endif
#if IMGSRC_1_TYPE == SENSOR_TYPE_SC132GS
#define IMG_RES             IMGSRC_1_RES
#define IMG_FORMAT          IMGSRC_1_FORMAT
#define IMG_MIPILANE_NUM    IMGSRC_1_MIPILANE_NUM
#endif


static const struct sensor_datafmt_info sc132gs_colour_fmts[] = {
    { V2K_PIX_FMT_RAW8, V2K_COLORSPACE_RAW },
}; 

static const struct sensor_win_size sc132gs_supported_win_sizes[] = {
    //{ .width = TFT43_WIDTH, .height = TFT43_HEIGHT, },
    { .width = VGA_LANDSCAPE_WIDTH, .height = VGA_LANDSCAPE_HEIGHT, },
    { .width = VGA_PORTRAIT_WIDTH, .height = VGA_PORTRAIT_HEIGHT, },
    { .width = SC132_FULL_RES_WIDTH, .height = SC132_FULL_RES_HEIGHT, },
};

static struct sensor_device sc132gs_dev = {
    .addr = 0x30,
    .device_id = 0XFFFF,
};

#define CALC_FPS     (25)
#define CALC_VTS     (0x0A8C)    // {16h'0x320e, 16h'0x320f}
struct sensor_init_seq INITDATA sc132gs_init_regs[] = {  
#if 1
    {0x0103, 0x01},
    {0x0100, 0x00},
    
    {0x3361, 0xc0},	// strobe  off     
    
    //PLL bypass
    {0x36e9, 0x80},
    {0x36f9, 0x80},
    
    {0x0100, 0x00},
    {0x3018, 0x12},
    {0x3019, 0x0e},
    {0x301a, 0xb4},
    {0x3031, 0x08},	// 0X0A:RAW10;0X08:RAW8
    {0x3032, 0x60},
    {0x3038, 0x44},
    {0x3207, 0x17},
    {0x320c, 0x06},
    {0x320d, 0x40},
//    {0x320e, 0x05}, // for 50 frame rate
//    {0x320f, 0x46}, 
    {0x320e, 0x06},// for 40 frame rate
    {0x320f, 0x97},
    //{0x320e, 0x0A},   // for 25 frame rate
    //{0x320f, 0x8C},
    {0x3250, 0xcc},
    {0x3251, 0x02},
    {0x3252, 0x05},
    {0x3253, 0x41},
    {0x3254, 0x05},
    {0x3255, 0x3b},
    {0x3306, 0x78},
    {0x330a, 0x00},
    {0x330b, 0xc8},
    {0x330f, 0x24},
    {0x3314, 0x80},
    {0x3315, 0x40},
    {0x3317, 0xf0},
    {0x331f, 0x12},
    {0x3364, 0x00},
    {0x3385, 0x41},
    {0x3387, 0x41},
    {0x3389, 0x09},
    {0x33ab, 0x00},
    {0x33ac, 0x00},
    {0x33b1, 0x03},
    {0x33b2, 0x12},
    {0x33f8, 0x02},
    {0x33fa, 0x01},
    {0x3409, 0x08},
    {0x34f0, 0xc0},
    {0x34f1, 0x20},
    {0x34f2, 0x03},
    {0x3622, 0xf5},
    {0x3630, 0x5c},
    {0x3631, 0x80},
    {0x3632, 0xc8},
    {0x3633, 0x32},
    {0x3638, 0x2a},
    {0x3639, 0x07},
    {0x363b, 0x48},
    {0x363c, 0x83},
    {0x363d, 0x10},
    {0x36ea, 0x3a},
    {0x36fa, 0x25},
    {0x36fb, 0x05},
    {0x36fd, 0x04},
    {0x3900, 0x11},
    {0x3901, 0x05},
    {0x3902, 0xc5},
    {0x3904, 0x04},
    {0x3908, 0x91},
    {0x391e, 0x00},
    {0x3e00, 0x00},	    // AEC
//    {0x3e01, 0x5B},	    // 23488
//    {0x3e02, 0xC0},	    //
      {0x3e01, 0x68},	    // 26864
      {0x3e02, 0xF0},       
#ifdef ANA_GAIN
    {0x3e03, 0x03},     // ANA AGC
    {0x3e08, 0x00},	    // AGC  default:null
    {0x3e09, 0x80},	    // AGC    default:{0x3e09, 0x20},
#else
    {0x3e08, 0x23},	    // AGC  default:null
    {0x3e09, 0x35},	    // AGC  gain = 3.0  default:{0x3e09, 0x20},  
#endif

    {0x3e0e, 0xd2},
    {0x3e14, 0xb0},
    {0x3e1e, 0x7c},
    {0x3e26, 0x20},
    {0x4418, 0x38},
    {0x4503, 0x10},
    {0x4837, 0x14},
    {0x5000, 0x0e},
    {0x540c, 0x51},
    {0x550f, 0x38},
    {0x5780, 0x67},
    {0x5784, 0x10},
    {0x5785, 0x06},
    {0x5787, 0x02},
    {0x5788, 0x00},
    {0x5789, 0x00},
    {0x578a, 0x02},
    {0x578b, 0x00},
    {0x578c, 0x00},
    {0x5790, 0x00},
    {0x5791, 0x00},
    {0x5792, 0x00},
    {0x5793, 0x00},
    {0x5794, 0x00},
    {0x5795, 0x00},
    {0x5799, 0x04},
    
#if IMG_RES == RES_480_640

        //Vbin
        {0x3220, 0x87},
        {0x3215, 0x22},
    #if 1
        {0x3213, 0x08},
        {0x320a, 0x02},	// 640
        {0x320b, 0x80},
    #else
        {0x3213, 0x08},	//{0x3213, 0xfc},
        {0x320a, 0x01},	// 272
        {0x320b, 0x10},
    #endif
        {0x334f, 0xbe},
        {0x3231, 0x0a},
        {0x3230, 0x0c},
        
        //Hsum
        {0x5000, 0x40},
        {0x5901, 0x14},
        {0x5900, 0xf6},
    #if 1
        {0x3208, 0x01},	// 480
        {0x3209, 0xe0},
    #else
        {0x3208, 0x01},	// 480
        {0x3209, 0xe0},
    #endif
        //{0x36ec, 0x13},
        {0x36ec, 0x03},
        {0x3211, 0x1e}, 
        
#else   // default 1080x1280

#endif
    
//	{0x0100, 0x01},
    
    //2lane mipi
    {0x3019, 0x0c}, //[3:0] lane disable
    {0x3018, 0x32}, //[6:5] lane num=[6:5]+1
    
    {0x0100, 0x01}, 

    //PLL set
    {0x36e9, 0x24},
    {0x36f9, 0x24},
    
#if 0
    // [gain<2]
    {0x33f8, 0x02},
    {0x3314, 0x80},
    {0x33fa, 0x01},
    {0x3317, 0xf0},
#else
    // [gain>=2]
    {0x33f8, 0x02},
    {0x3314, 0x80},
    {0x33fa, 0x02},
    {0x3317, 0x00},
#endif
        
#if 0
    // test mode
    {0x4501, 0xAC}, // Bit[3]:incremental pattern enable
    {0x3902, 0x85},
    {0x391d, 0xa8},
    {0x3e06, 0x03},
#endif

//	{ 0x3e09, 0x39},

// #if 0			// flip & mirror
//     {0x323b, 0x01},	// flip on
//     {0x3221, 0x66},	// bit[6:5]00:flip off,11:flip on; bit[2:1]00:mirror off,11:mirror on;
// #endif
//     //extract flip & mirror functions later
//     {0x323b, 0x00},	// flip on
//     {0x3221, 0x06},	// bit[6:5]00:flip off,11:flip on; bit[2:1]00:mirror off,11:mirror on;
//     //{0x323b, 0x03},	// flip on
//     //{0x3221, 0x30},	// bit[6:5]00:flip off,11:flip on; bit[2:1]00:mirror off,11:mirror on;    

#if 1			// flip & mirror
    {0x323b, 0x01},	// flip on
    {0x3221, 0x60},	// bit[6:5]00:flip off,11:flip on; bit[2:1]00:mirror off,11:mirror on;
#endif
#if 0    
    //extract flip & mirror functions later
    {0x323b, 0x00},	// flip on
    {0x3221, 0x06},	// bit[6:5]00:flip off,11:flip on; bit[2:1]00:mirror off,11:mirror on;
    //{0x323b, 0x03},	// flip on
    //{0x3221, 0x30},	// bit[6:5]00:flip off,11:flip on; bit[2:1]00:mirror off,11:mirror on;    
#endif

#if 0			// mirror
    //{0x323b, 0x03},	// flip on
    {0x3221, 0x06},	// bit[6:5]00:flip off,11:flip on; bit[2:1]00:mirror off,11:mirror on;
#endif

    { 0x00, 0x00},
#else    
    {0x0103, 0x01},
    {0x0100, 0x00},
    
    //PLL bypass
    {0x36e9, 0x80},
    {0x36f9, 0x80},
    
    {0x0100, 0x00},
    {0x3018, 0x12},
    {0x3019, 0x0e},
    {0x301a, 0xb4},
    {0x3031, 0x08},	// 0X0A:RAW10;0X08:RAW8
    {0x3032, 0x60},
    {0x3038, 0x44},
    {0x3207, 0x17},
    {0x320c, 0x06},
    {0x320d, 0x40},
    {0x320e, 0x05},
    {0x320f, 0x46},
    {0x3250, 0xcc},
    {0x3251, 0x02},
    {0x3252, 0x05},
    {0x3253, 0x41},
    {0x3254, 0x05},
    {0x3255, 0x3b},
    {0x3306, 0x78},
    {0x330a, 0x00},
    {0x330b, 0xc8},
    {0x330f, 0x24},
    {0x3314, 0x80},
    {0x3315, 0x40},
    {0x3317, 0xf0},
    {0x331f, 0x12},
    {0x3364, 0x00},
    {0x3385, 0x41},
    {0x3387, 0x41},
    {0x3389, 0x09},
    {0x33ab, 0x00},
    {0x33ac, 0x00},
    {0x33b1, 0x03},
    {0x33b2, 0x12},
    {0x33f8, 0x02},
    {0x33fa, 0x01},
    {0x3409, 0x08},
    {0x34f0, 0xc0},
    {0x34f1, 0x20},
    {0x34f2, 0x03},
    {0x3622, 0xf5},
    {0x3630, 0x5c},
    {0x3631, 0x80},
    {0x3632, 0xc8},
    {0x3633, 0x32},
    {0x3638, 0x2a},
    {0x3639, 0x07},
    {0x363b, 0x48},
    {0x363c, 0x83},
    {0x363d, 0x10},
    {0x36ea, 0x3a},
    {0x36fa, 0x25},
    {0x36fb, 0x05},
    {0x36fd, 0x04},
    {0x3900, 0x11},
    {0x3901, 0x05},
    {0x3902, 0xc5},
    {0x3904, 0x04},
    {0x3908, 0x91},
    {0x391e, 0x00},
    {0x3e00, 0x00},		// AEC
    {0x3e01, 0x53},		// {0x3e01, 0x53},
    {0x3e02, 0xe0},		// {0x3e02, 0xe0},
    {0x3e08, 0x27},		// AGC
    {0x3e09, 0x2d},		// AGC
    {0x3e0e, 0xd2},
    {0x3e14, 0xb0},
    {0x3e1e, 0x7c},
    {0x3e26, 0x20},
    {0x4418, 0x38},
    {0x4503, 0x10},
    {0x4837, 0x14},
    {0x5000, 0x0e},
    {0x540c, 0x51},
    {0x550f, 0x38},
    {0x5780, 0x67},
    {0x5784, 0x10},
    {0x5785, 0x06},
    {0x5787, 0x02},
    {0x5788, 0x00},
    {0x5789, 0x00},
    {0x578a, 0x02},
    {0x578b, 0x00},
    {0x578c, 0x00},
    {0x5790, 0x00},
    {0x5791, 0x00},
    {0x5792, 0x00},
    {0x5793, 0x00},
    {0x5794, 0x00},
    {0x5795, 0x00},
    {0x5799, 0x04},
    
    //Vbin
    {0x3220, 0x87},
    {0x3215, 0x22},

#if (IMGSRC_0_TYPE == SENSOR_TYPE_SC132GS)  
        //height
    #if IMGSRC_0_RES == RES_480_640
        {0x3213, 0x08},
        {0x320a, 0x02},	
        {0x320b, 0x80},
    #elif IMGSRC_0_RES == RES_480_272
        {0x3213, 0xfc},	
        {0x320a, 0x01},	
        {0x320b, 0x10},	
    #endif
#elif (IMGSRC_1_TYPE == SENSOR_TYPE_SC132GS)
        //height
    #if IMGSRC_1_RES == RES_480_640
        {0x3213, 0x08},
        {0x320a, 0x02},	
        {0x320b, 0x80},
    #elif IMGSRC_1_RES == RES_480_272
        {0x3213, 0xfc},	
        {0x320a, 0x01},	
        {0x320b, 0x10},	
    #endif
#endif        

    {0x334f, 0xbe},
    {0x3231, 0x0a},
    {0x3230, 0x0c},
    
    //Hsum
    {0x5000, 0x40},
    {0x5901, 0x14},
    {0x5900, 0xf6},
    //width
#if (IMGSRC_0_TYPE == SENSOR_TYPE_SC132GS)  
    #if IMGSRC_0_RES == RES_480_640
        {0x3208, 0x01},
        {0x3209, 0xe0},
    #elif IMGSRC_0_RES == RES_480_272
        {0x3208, 0x01},	
        {0x3209, 0xe0},
    #endif
#elif (IMGSRC_1_TYPE == SENSOR_TYPE_SC132GS)
    #if IMGSRC_1_RES == RES_480_640
        {0x3208, 0x01},
        {0x3209, 0xe0},
    #elif IMGSRC_1_RES == RES_480_272
        {0x3208, 0x01},
        {0x3209, 0xe0},
    #endif
#endif

    //{0x36ec, 0x13},
    {0x36ec, 0x03},
    {0x3211, 0x1e},
    
//	{0x0100, 0x01},
    
    //2lane mipi
    {0x3019, 0x0c}, //[3:0] lane disable
    
#if (IMGSRC_0_TYPE == SENSOR_TYPE_SC132GS)  
    #if IMGSRC_0_MIPILANE_NUM == IMAGE_MIPILANE_NUM_2
        {0x3018, 0x32}, //[6:5] lane num=[6:5]+1
    #elif IMGSRC_0_MIPILANE_NUM == IMAGE_MIPILANE_NUM_1
        {0x3018, 0x12}, //[6:5] lane num=[6:5]+1
    #endif
#elif (IMGSRC_1_TYPE == SENSOR_TYPE_SC132GS)
    #if IMGSRC_1_MIPILANE_NUM == IMAGE_MIPILANE_NUM_2
        {0x3018, 0x32}, //[6:5] lane num=[6:5]+1
    #elif IMGSRC_1_MIPILANE_NUM == IMAGE_MIPILANE_NUM_1
        {0x3018, 0x12}, //[6:5] lane num=[6:5]+1
    #endif
#endif

    {0x0100, 0x01}, 

    //PLL set
    {0x36e9, 0x24},
    {0x36f9, 0x24},
    
#if 1
    // [gain<2]
    {0x33f8, 0x02},
    {0x3314, 0x80},
    {0x33fa, 0x01},
    {0x3317, 0xf0},
#else
    // [gain>=2]
    {0x33f8, 0x02},
    {0x3314, 0x80},
    {0x33fa, 0x02},
    {0x3317, 0x00},
#endif
        
#if 0
    // test mode
    {0x4501, 0xAC}, // Bit[3]:incremental pattern enable
    {0x3902, 0x85},
    {0x391d, 0xa8},
    {0x3e06, 0x03},
#endif

    {0x3361, 0x00},	// strobe
    {0x323b, 0x03},	// flip on
    {0x3221, 0x66},	// bit[6:5]00:flip off,11:flip on; bit[2:1]00:mirror off,11:mirror on;
//	{ 0x3e09, 0x39},

    { 0x00, 0x00},
#endif    
};

int sc132gs_i2c_init(void)
{
    int ret = -1;

    ret = 0;
    
    return ret;
}

static int sc132gs_write_reg(struct sensor_device *sensor_dev, u16 reg, u8 data)
{
    int ret;

    ret = kdp_drv_i2c_write(I2C_ADAP_0, sensor_dev->addr, reg, 2, data);

    return ret;
}

static int sc132gs_read_reg(struct sensor_device *sensor_dev, u16 reg, u8 *data)
{
    int ret;

    ret = kdp_drv_i2c_read(I2C_ADAP_0, sensor_dev->addr, reg, 2, data);

    return ret;
}

int sc132gs_init(struct sensor_device *sensor_dev, struct sensor_init_seq *seq, u32 length)
{
    struct sensor_device *dev = sensor_dev;
    struct sensor_init_seq *init_fnc_ptr;
    //dbg_msg_camera(" sc132gs_init ");
    for (u32 i = 0; i < length; i++) {
        if (init_fnc_ptr[i].addr == 0 && init_fnc_ptr[i].value == 0) break; //reaches end
        sc132gs_write_reg(dev, init_fnc_ptr[i].addr , (u8)(init_fnc_ptr[i].value & 0xFF));
    }

    //dbg_msg_camera("sc132gs_init init over, sensor ID = 0x%x", dev->device_id);
    u8 data = 0;
    sc132gs_read_reg(dev, 0x3107, &data);
    //dbg_msg_camera("sc132gs_init sensor high id=%x", data);
    dev->device_id = data << 8;
    sc132gs_read_reg(dev, 0x3108, &data);
    //dbg_msg_camera("sc132gs_init sensor low =%x", data);
    dev->device_id |= data;
    
    if(dev->device_id != 0x132)
        return -1;
    
    return 0;
}


static const struct sensor_win_size *sc132gs_select_win(u32 *width, u32 *height)
{
    int i, default_size = ARRAY_SIZE(sc132gs_supported_win_sizes) - 1;

    for (i = 0; i < ARRAY_SIZE(sc132gs_supported_win_sizes); i++) {
        if (sc132gs_supported_win_sizes[i].width  >= *width &&
            sc132gs_supported_win_sizes[i].height >= *height) {
            *width = sc132gs_supported_win_sizes[i].width;
            *height = sc132gs_supported_win_sizes[i].height;
            return &sc132gs_supported_win_sizes[i];
        }
    }

    *width = sc132gs_supported_win_sizes[default_size].width;
    *height = sc132gs_supported_win_sizes[default_size].height;
    return &sc132gs_supported_win_sizes[default_size];
}

static int sc132gs_set_params(
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
    //sc132gs_reset(sensor_dev);
    /* initialize the sensor with default settings */
    return sc132gs_init(sensor_dev, sc132gs_init_regs, sizeof(sc132gs_init_regs)/sizeof(sc132gs_init_regs[0]));

}

static int sc132gs_sensor_enum_fmt(unsigned int index, unsigned int *code)
{
    if (index >= ARRAY_SIZE(sc132gs_colour_fmts))
        return -KDP_FRAMEWORK_ERRNO_INVALA;

    //critical_msg("   <%s>\n", __func__);
    *code = sc132gs_colour_fmts[index].fourcc;
    return 0;
}

static int sc132gs_sensor_set_fmt(struct cam_format *fmt)
{
    //critical_msg("   <%s>\n", __func__);

    sc132gs_select_win(&fmt->width, &fmt->height);

    return sc132gs_set_params(&sc132gs_dev, &fmt->width, &fmt->height, fmt->pixelformat);
}

static int sc132gs_sensor_set_gain(u8 ana_gn1, u8 ana_gn2)
{
    u8 dev_addr = 0x30;
    u16 regAddr = 0x3e08;
    u8 data = ana_gn1;

    kdp_drv_i2c_write(I2C_ADAP_0, dev_addr, regAddr, 2, data);

    regAddr = 0x3e09;
    data = ana_gn2;

    kdp_drv_i2c_write(I2C_ADAP_0, dev_addr, regAddr, 2, data);

    return 0;
}

static int sc132gs_sensor_set_exp_time(u8 ana_gn1, u8 ana_gn2)
{
    u8 dev_addr = 0x30;
    u16 regAddr = 0x3e01;
    u8 data = ana_gn1;

    kdp_drv_i2c_write(I2C_ADAP_0, dev_addr, regAddr, 2, data);

    regAddr = 0x3e02;
    data = ana_gn2;

    kdp_drv_i2c_write(I2C_ADAP_0, dev_addr, regAddr, 2, data);

    return 0;
}

static int sc132gs_sensor_set_mirror(BOOL enable)
{
    u8 data = 0;
    kdp_drv_i2c_read(I2C_ADAP_0, 0x30, 0x3221, 2, &data);
    data &= ~0x06;

    if (enable) {
        kdp_drv_i2c_write(I2C_ADAP_0, 0x30, 0x3221, 2, data | 0x06);
    }
    else {
        kdp_drv_i2c_write(I2C_ADAP_0, 0x30, 0x3221, 2, data);
    }

    return 0;
}

static int sc132gs_sensor_set_flip(BOOL enable)
{
    u8 data = 0;
    kdp_drv_i2c_read(I2C_ADAP_0, 0x30, 0x3221, 2, &data);
    data &= ~0x60;

    if (enable) {
        kdp_drv_i2c_write(I2C_ADAP_0, 0x30, 0x323b, 2, 0x01);
        kdp_drv_i2c_write(I2C_ADAP_0, 0x30, 0x3221, 2, data | 0x60);
    }
    else {
        kdp_drv_i2c_write(I2C_ADAP_0, 0x30, 0x323b, 2, 0x00);
        kdp_drv_i2c_write(I2C_ADAP_0, 0x30, 0x3221, 2, data);
    }

    return 0;
}

static int sc132gs_sensor_get_id(void)
{
    //critical_msg("   <%s>\n", __func__);

    return sc132gs_dev.device_id;
}

static int sc132gs_sensor_set_strobe(BOOL enable)
{
    u8 dev_addr = 0x30;
    u16 regAddr = 0x3361;
    
		u8 data;
	
    if (enable) {
				data = 0x0;//0x0  -> strobe en
                           //0x80;	on
		}
		else{
				data = 0xc0;	//off
		}

    kdp_drv_i2c_write(I2C_ADAP_0, dev_addr, regAddr, 2, data);
	return 0;
}

static int sc132gs_sensor_set_fps(u8 fps)
{
    u16 regAddr;
    u8 data;
    u8 dev_addr = 0x30;

    u32 dst_vts = (u32)(CALC_VTS * CALC_FPS / fps);

    regAddr = 0x320e;
    data = (dst_vts >> 8) & 0x000000FF;
    kdp_drv_i2c_write(I2C_ADAP_0, dev_addr, regAddr, 2, data);
    // dbg_msg_camera("[%s] reg addr:%#x, data:%#x", __func__, regAddr, data);

    regAddr = 0x320f;
    data = dst_vts & 0x000000FF;
    kdp_drv_i2c_write(I2C_ADAP_0, dev_addr, regAddr, 2, data);
    // dbg_msg_console("[%s] reg addr:%#x, data:%#x", __func__, regAddr, data);

    return 0;
}



static struct sensor_ops sc132gs_ops = {
//    .s_power    = sc132gs_sensor_power,
//    .reset      = sc132gs_sensor_reset,
//    .s_stream   = sc132gs_sensor_stream,
    .enum_fmt           = sc132gs_sensor_enum_fmt,
//    .get_fmt    = sc132gs_sensor_get_fmt,
    .set_fmt            = sc132gs_sensor_set_fmt,
    .set_gain           = sc132gs_sensor_set_gain,
    .set_exp_time       = sc132gs_sensor_set_exp_time,
    .get_lux            = NULL,
    .set_aec_roi        = NULL,
    .set_mirror         = sc132gs_sensor_set_mirror,
    .set_flip           = sc132gs_sensor_set_flip,
	.set_led			= sc132gs_sensor_set_strobe,
    .get_id             = sc132gs_sensor_get_id,
    .set_fps            = sc132gs_sensor_set_fps,
};

void sc132gs_sensor_init(int cam_idx)
{
    //critical_msg("   <%s>\n", __func__);
    kdp_sensor_register(SENSOR_TYPE_SC132GS, &sc132gs_ops);
}

#endif
