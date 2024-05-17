#include "board_kl520.h"
#if (IMGSRC_0_TYPE == SENSOR_TYPE_OV02B1B_R) || \
    (IMGSRC_1_TYPE == SENSOR_TYPE_OV02B1B_R) || \
    (IMGSRC_0_TYPE == SENSOR_TYPE_OV02B1B_L) || \
    (IMGSRC_1_TYPE == SENSOR_TYPE_OV02B1B_L)
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

 
#if ( IMGSRC_0_TYPE == SENSOR_TYPE_OV02B1B_R || IMGSRC_0_TYPE == SENSOR_TYPE_OV02B1B_L )
#define IMG_RES             IMGSRC_0_RES
#define IMG_FORMAT          IMGSRC_0_FORMAT
#define IMG_MIPILANE_NUM    IMGSRC_0_MIPILANE_NUM
#define IMG_MIRROR          CFG_SENSOR_0_FMT_MIRROR
#define IMG_FLIP            CFG_SENSOR_0_FMT_FLIP
#elif ( IMGSRC_1_TYPE == SENSOR_TYPE_OV02B1B_R || IMGSRC_1_TYPE ==SENSOR_TYPE_OV02B1B_L )
#define IMG_RES             IMGSRC_1_RES
#define IMG_FORMAT          IMGSRC_1_FORMAT
#define IMG_MIPILANE_NUM    IMGSRC_1_MIPILANE_NUM
#define IMG_MIRROR          CFG_SENSOR_1_FMT_MIRROR
#define IMG_FLIP            CFG_SENSOR_1_FMT_FLIP
#endif

#define RES_800X600 1
 
//#define CFG_ov02b1b_X_Y_INVERSE  (1)
static const struct sensor_datafmt_info ov02b1b_colour_fmts[] = {
    { V2K_PIX_FMT_YCBCR, V2K_COLORSPACE_YUV },
    { V2K_PIX_FMT_RGB565, V2K_COLORSPACE_RGB },
}; 

static const struct sensor_win_size ov02b1b_supported_win_sizes[] = {    
    { .width = 800, .height = 600, },
};

#ifndef CFG_SENSOR_0_I2C_ADDR
#define CFG_SENSOR_0_I2C_ADDR   (0x3c)
#endif
static struct sensor_device ov02b1b_dev_l = {
    .i2c_id = I2C_ADAP_0,
    .addr = CFG_SENSOR_0_I2C_ADDR,
    .device_id = 0XFFFF,
};

#ifndef CFG_SENSOR_1_I2C_ADDR
#define CFG_SENSOR_1_I2C_ADDR   (0x3c)
#endif
static struct sensor_device ov02b1b_dev_r = {
    .i2c_id = I2C_ADAP_1,
    .addr = CFG_SENSOR_1_I2C_ADDR,
    .device_id = 0XFFFF,
};

//850nm only

struct sensor_init_seq ov02b1b_init_regs[] = {  
#ifdef RES_800X600
{0xfd,0x00},               
{0xfd,0x00},  
{0x24,0x02},   //pll_mc
{0x25,0x06},   //pll_nc,dpll clk 72M
{0x29,0x03},
{0x2a,0xb4},   //mpll_nc, mpll clk 330M
{0x1e,0x17},   //vlow 0.53v
{0x33,0x07},   //ipx 2.84u
{0x35,0x07},   //pcp off
{0x4a,0x0c},   //ncp -1.4v
{0x3a,0x05},   //icomp1 4.25u
{0x3b,0x02},   //icomp2 1.18u
{0x3e,0x00},
{0x46,0x01},
{0xfd,0x01},  
{0x0e,0x06},  
{0x0f,0x1a},   //exp
{0x18,0x00},   //un fixed-fps
{0x22,0x40},   //analog gain
{0x23,0x02},   //adc_range 0.595v
{0x17,0x2c},   //pd reset row address time
{0x19,0x20},   //dac_d0 1024
{0x1b,0x06},   //rst_num1 96
{0x1c,0x04},   //rst_num2 64
{0x20,0x03},
{0x30,0x01},   //p0
{0x33,0x01},   //p3
{0x31,0x0a},   //p1
{0x32,0x09},   //p2
{0x38,0x01},
{0x39,0x01},   //p9
{0x3a,0x01},   //p10
{0x3b,0x01},
{0x4f,0x04},   //p24
{0x4e,0x05},   //p23
{0x50,0x01},   //p25   
{0x35,0x0c},   //p5
{0x45,0x2a},   //sc1,p20_1
{0x46,0x2a},   //p20_2
{0x47,0x2a},   //p20_3
{0x48,0x2a},   //p20_4
{0x4a,0x2c},   //sc2,p22_1
{0x4b,0x2c},   //p22_2
{0x4c,0x2c},   //p22_3
{0x4d,0x2c},   //p22_4
{0x56,0x3a},   //p31, 1st d0
{0x57,0x0a},   //p32, 1st d1
{0x58,0x24},   //col_en1
{0x59,0x20},   //p34 2nd d0
{0x5a,0x0a},   //p34 2nd d1
{0x5b,0xff},   //col_en2
{0x37,0x0a},   //p7, tx
{0x42,0x0e},   //p17, psw 
{0x68,0x90},
{0x69,0xcd},   //blk en, no sig_clamp
{0x7c,0x08},
{0x7d,0x08},
{0x7e,0x08},
{0x7f,0x08},   //vbl1_4
{0x83,0x14},
{0x84,0x14},
{0x86,0x14},
{0x87,0x07},   //vbl2_4
{0x88,0x0f},
{0x94,0x02},   //evsync del frame 
{0x98,0xd1},   //del bad frame
{0xfe,0x02},
{0xfd,0x03},   //RegPage
{0x97,0x6c},
{0x98,0x60},
{0x99,0x60},
{0x9a,0x6c},
{0xae,0x0d},   //bit0=1,high 8bit
{0x88,0x49},   //BLC_ABL
{0x89,0x7c},   //bit6=1 trigger en
{0xb4,0x05},   //mean trigger 5
{0xbd,0x0d},   //blc_rpc_coe
{0x8c,0x40},   //BLC_BLUE_SUBOFFSET_8lsb
{0x8e,0x40},   //BLC_RED_SUBOFFSET_8lsb
{0x90,0x40},   //BLC_GR_SUBOFFSET_8lsb
{0x92,0x40},   // BLC_GB_SUBOFFSET_8lsb
{0x9b,0x49},   //digtal gain
{0xac,0x40},   //blc random noise rpc_th 4x
{0xfd,0x00},
{0x5a,0x15},
{0x74,0x01},   // PD_MIPIturn on mipi phy 

{0xfd,0x00},  //binning 800x600
{0x28,0x03},
{0x4f,0x03},  //mipi size
{0x50,0x20},
{0x51,0x02},
{0x52,0x58},
{0xfd,0x01},
{0x12,(0x00 | (IMG_MIRROR << 1) | (IMG_FLIP))},
{0x03,0x70},  //h-start
{0x05,0x10},  //v-start
{0x07,0x20},  
{0x09,0xb0},
{0x6c,0x09},  //binning22 en
{0xfe,0x02}, 
{0xfb,0x01},

//// raw8 output
{0xfd,0x00},
{0x55,0x2a},
{0x27,0x01},
{0x6e,0x02},

{0xfd,0x03},
{0xc2,0x01},
{0xfd,0x01},  
#else  
{0xfd,0x00},  
{0x24,0x02},   //pll_mc
{0x25,0x06},   //pll_nc,dpll clk 72M
{0x29,0x01},
{0x2a,0xb4},   //mpll_nc, mpll clk 660M
{0x2b,0x00},
{0x1e,0x17},   //vlow 0.53v
{0x33,0x07},   //ipx 2.84u
{0x35,0x07},   
{0x4a,0x0c},   //ncp -1.4v
{0x3a,0x05},   //icomp1 4.25u
{0x3b,0x02},   //icomp2 1.18u
{0x3e,0x00},
{0x46,0x01},
{0x6d,0x03},
{0xfd,0x01},  
{0x0e,0x02},  
{0x0f,0x1a},   //exp
{0x18,0x00},   //un fixed-fps
{0x22,0xff},   //analog gain
{0x23,0x02},   //adc_range 0.595v
{0x17,0x2c},   //pd reset row address ti
{0x19,0x20},   //dac_d0 1024
{0x1b,0x06},   //rst_num1 96
{0x1c,0x04},   //rst_num2 64
{0x20,0x03},
{0x30,0x01},   //p0
{0x33,0x01},   //p3
{0x31,0x0a},   //p1
{0x32,0x09},   //p2
{0x38,0x01},
{0x39,0x01},   //p9
{0x3a,0x01},   //p10
{0x3b,0x01},
{0x4f,0x04},   //p24
{0x4e,0x05},   //p23
{0x50,0x01},   //p25   
{0x35,0x0c},   //p5
{0x45,0x2a},   //sc1,p20_1
{0x46,0x2a},   //p20_2
{0x47,0x2a},   //p20_3
{0x48,0x2a},   //p20_4
{0x4a,0x2c},   //sc2,p22_1
{0x4b,0x2c},   //p22_2
{0x4c,0x2c},   //p22_3
{0x4d,0x2c},   //p22_4
{0x56,0x3a},   //p31, 1st d0
{0x57,0x0a},   //p32, 1st d1
{0x58,0x24},   //col_en1
{0x59,0x20},   //p34 2nd d0
{0x5a,0x0a},   //p34 2nd d1
{0x5b,0xff},   //col_en2
{0x37,0x0a},   //p7, tx
{0x42,0x0e},   //p17, psw 
{0x68,0x90},
{0x69,0xcd},   //blk en, no sig_clamp
{0x6a,0x8f},
{0x7c,0x0a},
{0x7d,0x09},	//0a
{0x7e,0x09},	//0a
{0x7f,0x08},   
{0x83,0x14},
{0x84,0x14},
{0x86,0x14},
{0x87,0x07},   //vbl2_4
{0x88,0x0f},
{0x94,0x02},   //evsync del frame 
{0x98,0xd1},   //del bad frame
{0xfe,0x02},
{0xfd,0x03},   //RegPage
{0x97,0x78},
{0x98,0x78},
{0x99,0x78},
{0x9a,0x78},
{0xa1,0x40},
{0xb1,0x30},
{0xae,0x0d},   //bit0=1,high 8bit
{0x88,0x5b},   //BLC_ABL
{0x89,0x7c},   //bit6=1 trigger en
{0xb4,0x05},   //mean trigger 5
{0x8c,0x40},   //BLC_BLUE_SUBOFFSET_8lsb
{0x8e,0x40},   //BLC_RED_SUBOFFSET_8lsb
{0x90,0x40},   //BLC_GR_SUBOFFSET_8lsb
{0x92,0x40},   // BLC_GB_SUBOFFSET_8lsb
{0x9b,0x46},   //digtal gain
{0xac,0x40},   //blc random noise rpc_th
{0xfd,0x00},
{0x5a,0x15},
{0x74,0x01},   // PD_MIPIturn on mipi ph

{0xfd,0x00},  //crop to 1600x1200
{0x50,0x40},  //mipi hszie low 8bit
{0x52,0xb0},  //mipi vsize low 8bit
{0xfd,0x01},
{0x03,0x70},  //window hstart low 8bit
{0x05,0x10},  //window vstart low 8bit
{0x07,0x20},  //window hsize low 8bit
{0x09,0xb0},  //window vsize low 8bit


{0xfb,0x01},

// raw8 output
{0xfd,0x00},
{0x55,0x2a},
{0x27,0x01},
{0x6e,0x02},


//stream on
{0xfd,0x03},
{0xc2,0x01},  //MIPI_EN
{0xfd,0x01}, 
#endif
  {0x00, 0x00},
};

int ov02b1b_r_i2c_init(void)
{
    int ret = -1;

    ret = 0;
    
    return ret;
}
int ov02b1b_l_i2c_init(void)
{
    int ret = -1;

    ret = 0;
    
    return ret;
}

static int ov02b1b_write_reg(struct sensor_device *sensor_dev, u16 reg, u8 data)
{
    int ret;

    ret = kdp_drv_i2c_write(sensor_dev->i2c_id, sensor_dev->addr, reg, 1, data);

    return ret;
}

static int ov02b1b_read_reg(struct sensor_device *sensor_dev, u16 reg, u8 *data)
{
	int ret;

    ret = kdp_drv_i2c_read(sensor_dev->i2c_id, sensor_dev->addr, reg, 1, data);

	return ret;
}


int ov02b1b_init(struct sensor_device *sensor_dev, struct sensor_init_seq *seq, u32 length)
{
    struct sensor_device *dev = sensor_dev;
    struct sensor_init_seq *init_fnc_ptr = seq;
  uint8_t i=0;
    //dbg_msg_camera(" ov02b1b_init ");
   
    ov02b1b_write_reg(dev, 0xfc , 0x01);//soft reset
    osDelay(5);
    for (u32 i = 0; i < length; i++) {
        if (init_fnc_ptr[i].addr == 0 && init_fnc_ptr[i].value == 0) break; //reaches end
        ov02b1b_write_reg(dev, init_fnc_ptr[i].addr , (u8)(init_fnc_ptr[i].value & 0xFF));
    }
 
    
    ov02b1b_write_reg(dev, 0xfd , 0x00);

    u8 data = 0;
   
    ov02b1b_read_reg(dev, 0x02, &data);
//    dbg_msg_console("ov02b1b[%d] sensor high id=%x", dev->i2c_id, data);
    dev->device_id = data << 8;
    ov02b1b_read_reg(dev, 0x03, &data);
//    dbg_msg_console("ov02b1b[%d] sensor low id=%x", dev->i2c_id, data);
    dev->device_id |= data;
//    dbg_msg_console("ov02b1b[%d] init over, sensor ID = 0x%x", dev->i2c_id, dev->device_id);
    
    /*
    if(dev->device_id != 0x02e0)
        return -1;
    */
    return 0;
}

static const struct sensor_win_size *ov02b1b_select_win(u32 *width, u32 *height)
{
    int i, default_size = ARRAY_SIZE(ov02b1b_supported_win_sizes) - 1;

    for (i = 0; i < ARRAY_SIZE(ov02b1b_supported_win_sizes); i++) {
        if (ov02b1b_supported_win_sizes[i].width  >= *width &&
            ov02b1b_supported_win_sizes[i].height >= *height) {
            *width = ov02b1b_supported_win_sizes[i].width;
            *height = ov02b1b_supported_win_sizes[i].height;
            return &ov02b1b_supported_win_sizes[i];
        }
    }

    *width = ov02b1b_supported_win_sizes[default_size].width;
    *height = ov02b1b_supported_win_sizes[default_size].height;
    return &ov02b1b_supported_win_sizes[default_size];
}

static int ov02b1b_set_params(
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
    //ov02b1b_reset(sensor_dev);

    /* initialize the sensor with default settings */
//    dbg_msg_console("init len=%d0",sizeof(ov02b1b_init_regs)/sizeof(ov02b1b_init_regs[0]));
    return ov02b1b_init(sensor_dev, ov02b1b_init_regs, sizeof(ov02b1b_init_regs)/sizeof(ov02b1b_init_regs[0]));
}
static int ov02b1b_sensor_enum_fmt(unsigned int index, unsigned int *code)
{
    if (index >= ARRAY_SIZE(ov02b1b_colour_fmts))
        return -KDP_FRAMEWORK_ERRNO_INVALA;

    //dbg_msg_camera("   <%s>\n", __func__);
    *code = ov02b1b_colour_fmts[index].fourcc;
    return 0;
}

static int ov02b1b_r_sensor_set_fmt(struct cam_format *fmt)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    ov02b1b_select_win(&fmt->width, &fmt->height);

    return ov02b1b_set_params(&ov02b1b_dev_r, &fmt->width, &fmt->height, fmt->pixelformat);
}

static int ov02b1b_l_sensor_set_fmt(struct cam_format *fmt)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    ov02b1b_select_win(&fmt->width, &fmt->height);

    return ov02b1b_set_params(&ov02b1b_dev_l, &fmt->width, &fmt->height, fmt->pixelformat);
}

int ov02b1b_sensor_set_aec_roi(struct sensor_device *sensor_dev, u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
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

int ov02b1b_r_sensor_set_aec_roi(u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
{
    struct sensor_device *sensor_dev = &ov02b1b_dev_r;
    int ret = ov02b1b_sensor_set_aec_roi(sensor_dev, x1, x2, y1, y2, center_x1, center_x2, center_y1, center_y2);
    return ret;
}

int ov02b1b_l_sensor_set_aec_roi(u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
{
    struct sensor_device *sensor_dev = &ov02b1b_dev_l;
    int ret = ov02b1b_sensor_set_aec_roi(sensor_dev, x1, x2, y1, y2, center_x1, center_x2, center_y1, center_y2);
    return ret;
}

int ov02b1b_sensor_get_lux(struct sensor_device *sensor_dev, u16* exposure, u8* pre_gain_h, u8* pre_gain_l, u8* global_gain, u8* y_average)
{
    enum i2c_adap_id i2c_id = sensor_dev->i2c_id;
    u8 dev_addr = sensor_dev->addr;
 

    return 0;
}

int ov02b1b_r_sensor_get_lux(u16* exposure, u8* pre_gain_h, u8* pre_gain_l, u8* global_gain, u8* y_average)
{
    struct sensor_device *sensor_dev = &ov02b1b_dev_r;
    int ret = ov02b1b_sensor_get_lux(sensor_dev, exposure, pre_gain_h, pre_gain_l, global_gain, y_average);
    return ret;
}

int ov02b1b_l_sensor_get_lux(u16* exposure, u8* pre_gain_h, u8* pre_gain_l, u8* global_gain, u8* y_average)
{
    struct sensor_device *sensor_dev = &ov02b1b_dev_l;
    int ret = ov02b1b_sensor_get_lux(sensor_dev, exposure, pre_gain_h, pre_gain_l, global_gain, y_average);
    return ret;
}

static int ov02b1b_l_sensor_get_id(void)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    return ov02b1b_dev_l.device_id;
}

static int ov02b1b_r_sensor_get_id(void)
{
    //dbg_msg_camera("   <%s>\n", __func__);

    return ov02b1b_dev_r.device_id;
}
 
 
                             
int ov02b1b_sensor_set_gain(struct sensor_device *sensor_dev, u8 ana_gn1, u8 ana_gn2)
{
  
    enum i2c_adap_id i2c_id = sensor_dev->i2c_id;
    u8 dev_addr = sensor_dev->addr;
    
//   dbg_msg_console("[Gain][%d][%d]", ana_gn1,ana_gn2);

    //set to page 1
//    u8 reg = 0xfd;
//    //u8 data = 0x01;
    kdp_drv_i2c_write(i2c_id, dev_addr, 0xfd, 1, 0x01); //P1 

    kdp_drv_i2c_write(i2c_id, dev_addr, 0x22, 1, ana_gn2); //AGC gain

 
    //set to page 3
    //data = 0x03;
    //kdp_drv_i2c_write(i2c_id, dev_addr, reg, 1, 0x03);  //P3

    //kdp_drv_i2c_write(i2c_id, dev_addr, 0x9B, 1, ana_gn2);  //digital gain
    
    kdp_drv_i2c_write(i2c_id, dev_addr, 0xfd, 1, 0x01); //P1 
    kdp_drv_i2c_write(i2c_id, dev_addr, 0xFE, 1, 0x02); //trigger the new exp/gain

    return 0;
}

int ov02b1b_r_sensor_set_gain(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &ov02b1b_dev_r;
    int ret = ov02b1b_sensor_set_gain(sensor_dev, ana_gn1, ana_gn2);
    return ret;
}

int ov02b1b_l_sensor_set_gain(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &ov02b1b_dev_l;
    int ret = ov02b1b_sensor_set_gain(sensor_dev, ana_gn1, ana_gn2);
    return ret;
}

int ov02b1b_sensor_set_exp_time(struct sensor_device *sensor_dev, u8 ana_gn1, u8 ana_gn2)
{
    enum i2c_adap_id i2c_id = sensor_dev->i2c_id;
    u8 dev_addr = sensor_dev->addr;

    //set to page 1
//    u8 reg = 0xfd;
//    u8 data = 0x01;
//   dbg_msg_console("[Exp][%d][%d]", ana_gn1,ana_gn2);
    
    kdp_drv_i2c_write(i2c_id, dev_addr, 0xfd, 1, 0x01); //P1

    //set exp time
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x0E, 1, ana_gn1);
    kdp_drv_i2c_write(i2c_id, dev_addr, 0x0F, 1, ana_gn2);

    kdp_drv_i2c_write(i2c_id, dev_addr, 0xfd, 1, 0x01);  //P1 
    kdp_drv_i2c_write(i2c_id, dev_addr, 0xFE, 1, 0x02); //trigger the new exp/gain
    
    return 0;
}

int ov02b1b_r_sensor_set_exp_time(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &ov02b1b_dev_r;
    int ret = ov02b1b_sensor_set_exp_time(sensor_dev, ana_gn1, ana_gn2);

    return ret;
}

int ov02b1b_l_sensor_set_exp_time(u8 ana_gn1, u8 ana_gn2)
{
    struct sensor_device *sensor_dev = &ov02b1b_dev_l;
    int ret = ov02b1b_sensor_set_exp_time(sensor_dev, ana_gn1, ana_gn2);

    return ret;
}

static int ov02b1b_sensor_sleep(struct sensor_device *sensor_dev, BOOL enable)
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

static int ov02b1b_r_sensor_sleep( BOOL enable )
{
    struct sensor_device *sensor_dev = &ov02b1b_dev_r;
    int ret = ov02b1b_sensor_sleep(sensor_dev, enable);
    
    return 0;
}

static int ov02b1b_l_sensor_sleep( BOOL enable )
{
    struct sensor_device *sensor_dev = &ov02b1b_dev_l;
    int ret = ov02b1b_sensor_sleep(sensor_dev, enable);
    
    return 0;
}

  


static struct sensor_ops ov02b1b_r_ops = {
//    .s_power    = ov02b1b_sensor_power,
//    .reset      = ov02b1b_sensor_reset,
//    .s_stream   = ov02b1b_sensor_stream,
    .enum_fmt           = ov02b1b_sensor_enum_fmt,
//    .get_fmt    = ov02b1b_sensor_get_fmt,
    .set_fmt            = ov02b1b_r_sensor_set_fmt,
    .set_gain           = ov02b1b_r_sensor_set_gain,
    .set_exp_time       = ov02b1b_r_sensor_set_exp_time,
    .get_lux            = ov02b1b_r_sensor_get_lux,
    .set_aec_roi        = ov02b1b_r_sensor_set_aec_roi,
    .set_mirror         = NULL,
    .set_flip           = NULL,
    .get_id             = ov02b1b_r_sensor_get_id,
    .set_fps            = NULL,
    .sleep              = ov02b1b_r_sensor_sleep,
};

static struct sensor_ops ov02b1b_l_ops = {
//    .s_power    = ov02b1b_sensor_power,
//    .reset      = ov02b1b_sensor_reset,
//    .s_stream   = ov02b1b_sensor_stream,
    .enum_fmt           = ov02b1b_sensor_enum_fmt,
//    .get_fmt    = ov02b1b_sensor_get_fmt,
    .set_fmt            = ov02b1b_l_sensor_set_fmt,
    .set_gain           = ov02b1b_l_sensor_set_gain,
    .set_exp_time       = ov02b1b_l_sensor_set_exp_time,
    .get_lux            = ov02b1b_l_sensor_get_lux,
    .set_aec_roi        = ov02b1b_l_sensor_set_aec_roi,
    .set_mirror         = NULL,
    .set_flip           = NULL,
    .get_id             = ov02b1b_l_sensor_get_id,
    .set_fps            = NULL,
    .sleep              = ov02b1b_l_sensor_sleep,
};
void ov02b1b_r_sensor_init(int cam_idx)
{
    //dbg_msg_camera("   <%s>\n", __func__);
    kdp_sensor_register(SENSOR_TYPE_OV02B1B_R, &ov02b1b_r_ops);
}

void ov02b1b_l_sensor_init(int cam_idx)
{
    //dbg_msg_camera("   <%s>\n", __func__);
    kdp_sensor_register(SENSOR_TYPE_OV02B1B_L, &ov02b1b_l_ops);
}
#endif
