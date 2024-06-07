#include "board_kl520.h"
#if (IMGSRC_0_TYPE == SENSOR_TYPE_HMX2056) || (IMGSRC_1_TYPE == SENSOR_TYPE_HMX2056)
#include <stdlib.h> 
#include "framework/init.h"
#include "framework/v2k.h"
#include "framework/v2k_image.h"
#include "framework/framework_errno.h"
#include "kdp520_i2c.h"
#include "media/camera/sensor.h"
#include "media/camera/sys_camera.h"
#include "media/v2k_subdev.h"
#include "utility.h"
#include "dbg.h"

#define RES_640x480 1
#define RES_480x272 2
#define HMX2056_RES RES_640x480


struct hmx2056_context {
    struct v2k_subdev subdev;
    //const struct sensor_datafmt_info *fmt;    
};

static const struct sensor_datafmt_info hmx2056_colour_fmts[] = {
    { V2K_PIX_FMT_RGB565, V2K_COLORSPACE_RGB },
    { V2K_PIX_FMT_RAW8, V2K_COLORSPACE_RAW },
}; 

static const struct sensor_win_size hmx2056_supported_win_sizes[] = {
    { .width = TFT43_WIDTH, .height = TFT43_HEIGHT, },    
    { .width = VGA_LANDSCAPE_WIDTH, .height = VGA_LANDSCAPE_HEIGHT, },    
};

#if HMX2056_RES == RES_640x480

struct sensor_init_seq INITDATA hmx2056_init_regs[] = {  
{0x0022,0x00},
{0x0020,0x00},
{0x0025,0x00},
//{0x0025,0x80},
{0x0026,0x87}, //24mhz=0x87
//{0x0026,0x83}, //12mhz=0x83
{0x0027,0x40},
{0x0028,0xC0},
{0x002A,0x25},//228Mhz
{0x002B,0x00}, // divider for system clock 228/4=57, divider for mipi clock=228/2=114
{0x002C,0x0A},
{0x0004,0x10},
//RDCFG - resolution param : 1
{0x0006,0x01},//kay from 0 to 3
//VREAD - resolution param : 2
{0x000D,0x01},
//{0x000D,0x11},
//HREAD - resolution param : 3
{0x000E,0x11},

{0x000F,0x00}, //variable frame rate
{0x0011,0x02},
{0x0012,0x1C},
{0x0013,0x01},
{0x0015,0x02},
{0x0016,0x80},
{0x0018,0x00},
{0x001D,0x40},
{0x0040,0x20},
{0x0053,0x0A},
{0x0044,0x06},
{0x0046,0xD8},
{0x004A,0x0A},
{0x004B,0x72},
{0x0075,0x01},
{0x0070,0x5F},
{0x0071,0xFF},
{0x0072,0x55},
{0x0073,0x50},
{0x0077,0x04},
{0x0080,0xC8},
{0x0082,0xA2},
{0x0083,0xF0},
{0x0085,0x11},
{0x0086,0x02},
{0x0087,0x80},
{0x0088,0x6C},
{0x0089,0x2E},
{0x008A,0x6D},
{0x008D,0x20},
{0x0090,0x00},
{0x0091,0x10},
{0x0092,0x11},
{0x0093,0x12},
{0x0094,0x16},
{0x0095,0x08},
{0x0096,0x00},
{0x0097,0x10},
{0x0098,0x11},
{0x0099,0x12},
{0x009A,0x16},
{0x009B,0x34},
{0x00A0,0x00},
{0x00A1,0x04},
//ISPCTRL0 resolution param : 6 , Full mode
{0x011F,0x00},
//{0x011F,0xF7},
{0x0120,0x37},
{0x0121,0x83},
{0x0122,0x7B},//0114
{0x0123,0xC2},
{0x0124,0xDE},
//ISPCTRL5 resolution param : 4 , Scaler - enable down-scaler
{0x0125,0xFF},//0114
//ISPCTRL6 resolution param : 5 , Windowing Vsync width adjust
{0x0126,0x70},
{0x0128,0x1F},
{0x0132,0x10},
{0x0136,0x0A},
{0x0131,0xBD},
{0x0140,0x14},
{0x0141,0x0A},
{0x0142,0x14},
{0x0143,0x0A},
{0x0144,0x06},
{0x0145,0x00},
{0x0146,0x20},
{0x0147,0x0A},
{0x0148,0x10},
{0x0149,0x0C},
{0x014A,0x80},
{0x014B,0x80},
{0x014C,0x2E},
{0x014D,0x2E},
{0x014E,0x05},
{0x014F,0x05},
{0x0150,0x0D},
{0x0155,0x00},
{0x0156,0x10},
{0x0157,0x0A},
{0x0158,0x0A},
{0x0159,0x0A},
{0x015A,0x05},
{0x015B,0x05},
{0x015C,0x05},
{0x015D,0x05},
{0x015E,0x08},
{0x015F,0xFF},
{0x0160,0x50},
{0x0161,0x20},
{0x0162,0x14},
{0x0163,0x0A},
{0x0164,0x10},
{0x0165,0x08},
{0x0166,0x0A},
{0x018C,0x24},
{0x018D,0x04},
{0x018E,0x00},
{0x018F,0x11},
{0x0190,0x80},
{0x0191,0x47},
{0x0192,0x48},
{0x0193,0x64},
{0x0194,0x32},
{0x0195,0xC8},
{0x0196,0x96},
{0x0197,0x64},
{0x0198,0x32},
{0x0199,0x14},
{0x019A,0x20},
{0x019B,0x14},
{0x01BA,0x10},
{0x01BB,0x04},
{0x01D8,0x40},
{0x01DE,0x60},
{0x01E4,0x10},
{0x01E5,0x10},
{0x01F2,0x0C},
{0x01F3,0x14},
{0x01F8,0x04},
{0x01F9,0x0C},
{0x01FE,0x02},
{0x01FF,0x04},
{0x0220,0x00},
{0x0221,0xB0},
{0x0222,0x00},
{0x0223,0x80},
{0x0224,0x8E},
{0x0225,0x00},
{0x0226,0x88},
{0x022A,0x88},
{0x022B,0x00},
{0x022C,0x88},
{0x022D,0x13},
{0x022E,0x0B},
{0x022F,0x13},
{0x0230,0x0B},
{0x0233,0x13},
{0x0234,0x0B},
{0x0235,0x28},
{0x0236,0x03},
{0x0237,0x28},
{0x0238,0x03},
{0x023B,0x28},
{0x023C,0x03},
{0x023D,0x5C},
{0x023E,0x02},
{0x023F,0x5C},
{0x0240,0x02},
{0x0243,0x5C},
{0x0244,0x02},
{0x0251,0x0E},
{0x0252,0x00},
{0x0280,0x0A},
{0x0282,0x14},
{0x0284,0x2A},
{0x0286,0x50},
{0x0288,0x60},
{0x028A,0x6D},
{0x028C,0x79},
{0x028E,0x82},
{0x0290,0x8A},
{0x0292,0x91},
{0x0294,0x9C},
{0x0296,0xA7},
{0x0298,0xBA},
{0x029A,0xCD},
{0x029C,0xE0},
{0x029E,0x2D},
{0x02A0,0x06},
{0x02E0,0x04},
{0x02C0,0xB1},
{0x02C1,0x01},
{0x02C2,0x7D},
{0x02C3,0x07},
{0x02C4,0xD2},
{0x02C5,0x07},
{0x02C6,0xC4},
{0x02C7,0x07},
{0x02C8,0x79},
{0x02C9,0x01},
{0x02CA,0xC4},
{0x02CB,0x07},
{0x02CC,0xF7},
{0x02CD,0x07},
{0x02CE,0x3B},
{0x02CF,0x07},
{0x02D0,0xCF},
{0x02D1,0x01},
{0x0302,0x00},
{0x0303,0x00},
{0x0304,0x00},
{0x02F0,0x5E},
{0x02F1,0x07},
{0x02F2,0xA0},
{0x02F3,0x00},
{0x02F4,0x02},
{0x02F5,0x00},
{0x02F6,0xC4},
{0x02F7,0x07},
{0x02F8,0x11},
{0x02F9,0x00},
{0x02FA,0x2A},
{0x02FB,0x00},
{0x02FC,0xA1},
{0x02FD,0x07},
{0x02FE,0xB8},
{0x02FF,0x07},
{0x0300,0xA7},
{0x0301,0x00},
{0x0305,0x00},
{0x0306,0x00},
{0x0307,0x7A},
{0x032D,0x00},
{0x032E,0x01},
{0x032F,0x00},
{0x0330,0x01},
{0x0331,0x00},
{0x0332,0x01},
{0x0333,0x82},
{0x0334,0x00},
{0x0335,0x84},
{0x0336,0x00},
{0x0337,0x01},
{0x0338,0x00},
{0x0339,0x01},
{0x033A,0x00},
{0x033B,0x01},
{0x0340,0x30},
{0x0341,0x44},
{0x0342,0x4A},
{0x0343,0x42},
{0x0344,0x74},
{0x0345,0x4F},
{0x0346,0x67},
{0x0347,0x5C},
{0x0348,0x59},
{0x0349,0x67},
{0x034A,0x4D},
{0x034B,0x6E},
{0x034C,0x44},
{0x0350,0x80},
{0x0351,0x80},
{0x0352,0x18},
{0x0353,0x18},
{0x0354,0x6E},
{0x0355,0x4A},
{0x0356,0x7A},
{0x0357,0xC6},
{0x0358,0x06},
{0x035A,0x06},
{0x035B,0xA0},
{0x035C,0x73},
{0x035D,0x5A},
{0x035E,0xC6},
{0x035F,0xA0},
{0x0360,0x02},
{0x0361,0x18},
{0x0362,0x80},
{0x0363,0x6C},
{0x0364,0x00},
{0x0365,0xF0},
{0x0366,0x20},
{0x0367,0x0C},
{0x0369,0x00},
{0x036A,0x10},
{0x036B,0x10},
{0x036E,0x20},
{0x036F,0x00},
{0x0370,0x10},
{0x0371,0x18},
{0x0372,0x0C},
{0x0373,0x38},
{0x0374,0x3A},
{0x0375,0x13},
{0x0376,0x22},
{0x0380,0xFF},//0114
{0x0381,0x4A},
{0x0382,0x36},
{0x038A,0x40},
{0x038B,0x08},
{0x038C,0xC1},
{0x038E,0x40},
//{0x038F,0x09},
//{0x0390,0xD0},
{0x038F,0x02},
{0x0390,0x80},
{0x0391,0x05},
{0x0393,0x80},
{0x0395,0x21},
{0x0398,0x02},
{0x0399,0x74},
{0x039A,0x03},
{0x039B,0x11},
{0x039C,0x03},
{0x039D,0xAE},
{0x039E,0x04},
{0x039F,0xE8},
{0x03A0,0x06},
{0x03A1,0x22},
{0x03A2,0x07},
{0x03A3,0x5C},
{0x03A4,0x09},
{0x03A5,0xD0},
{0x03A6,0x0C},
{0x03A7,0x0E},
{0x03A8,0x10},
{0x03A9,0x18},
{0x03AA,0x20},
{0x03AB,0x28},
{0x03AC,0x1E},
{0x03AD,0x1A},
{0x03AE,0x13},
{0x03AF,0x0C},
{0x03B0,0x0B},
{0x03B1,0x09},
{0x03B3,0x10},
{0x03B4,0x00},
{0x03B5,0x10},
{0x03B6,0x00},
{0x03B7,0xEA},
{0x03B8,0x00},
{0x03B9,0x3A},
{0x03BA,0x01},
{0x03BB,0x9F},
{0x03BC,0xCF},
{0x03BD,0xE7},
{0x03BE,0xF3},
{0x03BF,0x01},
{0x03D0,0xF8},
{0x03E0,0x04},
{0x03E1,0x01},
{0x03E2,0x04},
{0x03E4,0x10},
{0x03E5,0x12},
{0x03E6,0x00},
{0x03E8,0x21},
{0x03E9,0x23},
{0x03EA,0x01},
{0x03EC,0x21},
{0x03ED,0x23},
{0x03EE,0x01},
{0x03F0,0x20},
{0x03F1,0x22},
{0x03F2,0x00},
{0x0420,0x84},
{0x0421,0x00},
{0x0422,0x00},
{0x0423,0x83},
{0x0430,0x08},
{0x0431,0x28},
{0x0432,0x10},
{0x0433,0x08},
{0x0435,0x0C},
{0x0450,0xFF},
{0x0451,0xE8},
{0x0452,0xC4},
{0x0453,0x88},
{0x0454,0x00},
{0x0458,0x98},
{0x0459,0x03},
{0x045A,0x00},
{0x045B,0x28},
{0x045C,0x00},
{0x045D,0x68},
{0x0466,0x14},
{0x047A,0x00},
{0x047B,0x00},
{0x0480,0x58},
{0x0481,0x06},
{0x0482,0x0C},
{0x04B0,0x50},
{0x04B6,0x30},
{0x04B9,0x10},
{0x04B3,0x10},
{0x04B1,0x8E},
{0x04B4,0x20},
{0x0540,0x00},
{0x0541,0x9D},
{0x0542,0x00},
{0x0543,0xBC},
{0x0580,0x01},
{0x0581,0x0F},
{0x0582,0x04},
{0x0594,0x00},
{0x0595,0x04},
{0x05A9,0x03},
{0x05AA,0x40},
{0x05AB,0x80},
{0x05AC,0x0A},
{0x05AD,0x10},
{0x05AE,0x0C},
{0x05AF,0x0C},
{0x05B0,0x03},
{0x05B1,0x03},
{0x05B2,0x1C},
{0x05B3,0x02},
{0x05B4,0x00},
{0x05B5,0x0C},
{0x05B8,0x80},
{0x05B9,0x32},
{0x05BA,0x00},
{0x05BB,0x80},
{0x05BC,0x03},
{0x05BD,0x00},
{0x05BF,0x05},
{0x05C0,0x10},
{0x05C3,0x00},
{0x05C4,0x0C},
{0x05C5,0x20},
{0x05C7,0x01},
{0x05C8,0x14},
{0x05C9,0x54},
{0x05CA,0x14},
{0x05CB,0xE0},
{0x05CC,0x20},
{0x05CD,0x00},
{0x05CE,0x08},
{0x05CF,0x60},
{0x05D0,0x10},
{0x05D1,0x05},
{0x05D2,0x03},
{0x05D4,0x00},
{0x05D5,0x05},
{0x05D6,0x05},
{0x05D7,0x05},
{0x05D8,0x08},
{0x05DC,0x0C},
{0x05D9,0x00},
{0x05DB,0x00},
{0x05DD,0x0F},
{0x05DE,0x00},
{0x05DF,0x0A},

    {0x05E0,0xA0},
    {0x05E1,0x00},
    {0x05E2,0xA0},
    {0x05E3,0x00},
    {0x05E4,0x04},
    {0x05E5,0x00},
    {0x05E6,0x83},
    {0x05E7,0x02},
    {0x05E8,0x06},
    {0x05E9,0x00},
    {0x05EA,0xE5},
    {0x05EB,0x01},

{0x0660,0x04},
{0x0661,0x16},
{0x0662,0x04},
{0x0663,0x28},
{0x0664,0x04},
{0x0665,0x18},
{0x0666,0x04},
{0x0667,0x21},
{0x0668,0x04},
{0x0669,0x0C},
{0x066A,0x04},
{0x066B,0x25},
{0x066C,0x00},
{0x066D,0x12},
{0x066E,0x00},
{0x066F,0x80},
{0x0670,0x00},
{0x0671,0x0A},
{0x0672,0x04},
{0x0673,0x1D},
{0x0674,0x04},
{0x0675,0x1D},
{0x0676,0x00},
{0x0677,0x7E},
{0x0678,0x01},
{0x0679,0x47},
{0x067A,0x00},
{0x067B,0x73},
{0x067C,0x04},
{0x067D,0x14},
{0x067E,0x04},
{0x067F,0x28},
{0x0680,0x00},
{0x0681,0x22},
{0x0682,0x00},
{0x0683,0xA5},
{0x0684,0x00},
{0x0685,0x1E},
{0x0686,0x04},
{0x0687,0x1D},
{0x0688,0x04},
{0x0689,0x19},
{0x068A,0x04},
{0x068B,0x21},
{0x068C,0x04},
{0x068D,0x0A},
{0x068E,0x04},
{0x068F,0x25},
{0x0690,0x04},
{0x0691,0x15},
{0x0698,0x20},
{0x0699,0x20},
{0x069A,0x01},
{0x069C,0x22},
{0x069D,0x10},
{0x069E,0x10},
{0x069F,0x08},
{0x0B20,0xBE},
//{0x007C,0x33},
{0x007C,0x40},
{0x0B02,0x04},
{0x0B07,0x25},

{0x0B0E,0x1D},
{0x0B0F,0x07},
{0x0B22,0x02},
{0x0B39,0x03},
{0x0B11,0x7F},
{0x0B12,0x7F},
{0x0B17,0xE0},
{0x0B30,0x0F},
{0x0B31,0x02},
{0x0B32,0x00},
{0x0B33,0x00},
{0x0B39,0x0F},
{0x0B3B,0x12},
{0x0B3F,0x01},
{0x0024,0x40},
//{0x0028,0x81},//test pattern enable
{0x0028,0xC0},
{0x0000,0x01},
{0x0100,0x01},
{0x0101,0x01},
{0x0005,0x01},

{ 0,0},
};

#elif HMX2056_RES == RES_480x272

struct sensor_init_seq INITDATA hmx2056_init_regs[] = {  
{0x0022,0x00},
{0x0020,0x00},
{0x0025,0x00},
//{0x0025,0x80},
{0x0026,0x87}, //24mhz=0x87
//{0x0026,0x83}, //12mhz=0x83
{0x0027,0x40},
{0x0028,0xC0},
{0x002A,0x25},//228Mhz
{0x002B,0x00}, // 0x06: mipi clk /4
{0x002C,0x0A},
{0x0004,0x10},
{0x0006,0x01},//kay from 0 to 3
{0x000D,0x11},
{0x000E,0x11},
{0x000F,0x00},
{0x0011,0x02},
{0x0012,0x1C},
{0x0013,0x01},
{0x0015,0x02},
{0x0016,0x80},
{0x0018,0x00},
{0x001D,0x40},
{0x0040,0x20},
{0x0053,0x0A},
{0x0044,0x06},
{0x0046,0xD8},
{0x004A,0x0A},
{0x004B,0x72},
{0x0075,0x01},
{0x0070,0x5F},
{0x0071,0xFF},
{0x0072,0x55},
{0x0073,0x50},
{0x0077,0x04},
{0x0080,0xC8},
{0x0082,0xA2},
{0x0083,0xF0},
{0x0085,0x11},
{0x0086,0x02},
{0x0087,0x80},
{0x0088,0x6C},
{0x0089,0x2E},
{0x008A,0x6D},
{0x008D,0x20},
{0x0090,0x00},
{0x0091,0x10},
{0x0092,0x11},
{0x0093,0x12},
{0x0094,0x16},
{0x0095,0x08},
{0x0096,0x00},
{0x0097,0x10},
{0x0098,0x11},
{0x0099,0x12},
{0x009A,0x16},
{0x009B,0x34},
{0x00A0,0x00},
{0x00A1,0x04},
{0x011F,0xF7},
{0x0120,0x37},
{0x0121,0x83},
{0x0122,0x7B},//0114
{0x0123,0xC2},
{0x0124,0xDE},
{0x0125,0xFF},//0114
{0x0126,0x70},
{0x0128,0x1F},
{0x0132,0x10},
{0x0136,0x0A},
{0x0131,0xBD},
{0x0140,0x14},
{0x0141,0x0A},
{0x0142,0x14},
{0x0143,0x0A},
{0x0144,0x06},
{0x0145,0x00},
{0x0146,0x20},
{0x0147,0x0A},
{0x0148,0x10},
{0x0149,0x0C},
{0x014A,0x80},
{0x014B,0x80},
{0x014C,0x2E},
{0x014D,0x2E},
{0x014E,0x05},
{0x014F,0x05},
{0x0150,0x0D},
{0x0155,0x00},
{0x0156,0x10},
{0x0157,0x0A},
{0x0158,0x0A},
{0x0159,0x0A},
{0x015A,0x05},
{0x015B,0x05},
{0x015C,0x05},
{0x015D,0x05},
{0x015E,0x08},
{0x015F,0xFF},
{0x0160,0x50},
{0x0161,0x20},
{0x0162,0x14},
{0x0163,0x0A},
{0x0164,0x10},
{0x0165,0x08},
{0x0166,0x0A},
{0x018C,0x24},
{0x018D,0x04},
{0x018E,0x00},
{0x018F,0x11},
{0x0190,0x80},
{0x0191,0x47},
{0x0192,0x48},
{0x0193,0x64},
{0x0194,0x32},
{0x0195,0xC8},
{0x0196,0x96},
{0x0197,0x64},
{0x0198,0x32},
{0x0199,0x14},
{0x019A,0x20},
{0x019B,0x14},
{0x01BA,0x10},
{0x01BB,0x04},
{0x01D8,0x40},
{0x01DE,0x60},
{0x01E4,0x10},
{0x01E5,0x10},
{0x01F2,0x0C},
{0x01F3,0x14},
{0x01F8,0x04},
{0x01F9,0x0C},
{0x01FE,0x02},
{0x01FF,0x04},
{0x0220,0x00},
{0x0221,0xB0},
{0x0222,0x00},
{0x0223,0x80},
{0x0224,0x8E},
{0x0225,0x00},
{0x0226,0x88},
{0x022A,0x88},
{0x022B,0x00},
{0x022C,0x88},
{0x022D,0x13},
{0x022E,0x0B},
{0x022F,0x13},
{0x0230,0x0B},
{0x0233,0x13},
{0x0234,0x0B},
{0x0235,0x28},
{0x0236,0x03},
{0x0237,0x28},
{0x0238,0x03},
{0x023B,0x28},
{0x023C,0x03},
{0x023D,0x5C},
{0x023E,0x02},
{0x023F,0x5C},
{0x0240,0x02},
{0x0243,0x5C},
{0x0244,0x02},
{0x0251,0x0E},
{0x0252,0x00},
{0x0280,0x0A},
{0x0282,0x14},
{0x0284,0x2A},
{0x0286,0x50},
{0x0288,0x60},
{0x028A,0x6D},
{0x028C,0x79},
{0x028E,0x82},
{0x0290,0x8A},
{0x0292,0x91},
{0x0294,0x9C},
{0x0296,0xA7},
{0x0298,0xBA},
{0x029A,0xCD},
{0x029C,0xE0},
{0x029E,0x2D},
{0x02A0,0x06},
{0x02E0,0x04},
{0x02C0,0xB1},
{0x02C1,0x01},
{0x02C2,0x7D},
{0x02C3,0x07},
{0x02C4,0xD2},
{0x02C5,0x07},
{0x02C6,0xC4},
{0x02C7,0x07},
{0x02C8,0x79},
{0x02C9,0x01},
{0x02CA,0xC4},
{0x02CB,0x07},
{0x02CC,0xF7},
{0x02CD,0x07},
{0x02CE,0x3B},
{0x02CF,0x07},
{0x02D0,0xCF},
{0x02D1,0x01},
{0x0302,0x00},
{0x0303,0x00},
{0x0304,0x00},
{0x02F0,0x5E},
{0x02F1,0x07},
{0x02F2,0xA0},
{0x02F3,0x00},
{0x02F4,0x02},
{0x02F5,0x00},
{0x02F6,0xC4},
{0x02F7,0x07},
{0x02F8,0x11},
{0x02F9,0x00},
{0x02FA,0x2A},
{0x02FB,0x00},
{0x02FC,0xA1},
{0x02FD,0x07},
{0x02FE,0xB8},
{0x02FF,0x07},
{0x0300,0xA7},
{0x0301,0x00},
{0x0305,0x00},
{0x0306,0x00},
{0x0307,0x7A},
{0x032D,0x00},
{0x032E,0x01},
{0x032F,0x00},
{0x0330,0x01},
{0x0331,0x00},
{0x0332,0x01},
{0x0333,0x82},
{0x0334,0x00},
{0x0335,0x84},
{0x0336,0x00},
{0x0337,0x01},
{0x0338,0x00},
{0x0339,0x01},
{0x033A,0x00},
{0x033B,0x01},
{0x0340,0x30},
{0x0341,0x44},
{0x0342,0x4A},
{0x0343,0x42},
{0x0344,0x74},
{0x0345,0x4F},
{0x0346,0x67},
{0x0347,0x5C},
{0x0348,0x59},
{0x0349,0x67},
{0x034A,0x4D},
{0x034B,0x6E},
{0x034C,0x44},
{0x0350,0x80},
{0x0351,0x80},
{0x0352,0x18},
{0x0353,0x18},
{0x0354,0x6E},
{0x0355,0x4A},
{0x0356,0x7A},
{0x0357,0xC6},
{0x0358,0x06},
{0x035A,0x06},
{0x035B,0xA0},
{0x035C,0x73},
{0x035D,0x5A},
{0x035E,0xC6},
{0x035F,0xA0},
{0x0360,0x02},
{0x0361,0x18},
{0x0362,0x80},
{0x0363,0x6C},
{0x0364,0x00},
{0x0365,0xF0},
{0x0366,0x20},
{0x0367,0x0C},
{0x0369,0x00},
{0x036A,0x10},
{0x036B,0x10},
{0x036E,0x20},
{0x036F,0x00},
{0x0370,0x10},
{0x0371,0x18},
{0x0372,0x0C},
{0x0373,0x38},
{0x0374,0x3A},
{0x0375,0x13},
{0x0376,0x22},
{0x0380,0xFF},//0114
{0x0381,0x4A},
{0x0382,0x36},
{0x038A,0x40},
{0x038B,0x08},
{0x038C,0xC1},
{0x038E,0x40},
//{0x038F,0x09},
//{0x0390,0xD0},
{0x038F,0x02},
{0x0390,0x80},
{0x0391,0x05},
{0x0393,0x80},
{0x0395,0x21},
{0x0398,0x02},
{0x0399,0x74},
{0x039A,0x03},
{0x039B,0x11},
{0x039C,0x03},
{0x039D,0xAE},
{0x039E,0x04},
{0x039F,0xE8},
{0x03A0,0x06},
{0x03A1,0x22},
{0x03A2,0x07},
{0x03A3,0x5C},
{0x03A4,0x09},
{0x03A5,0xD0},
{0x03A6,0x0C},
{0x03A7,0x0E},
{0x03A8,0x10},
{0x03A9,0x18},
{0x03AA,0x20},
{0x03AB,0x28},
{0x03AC,0x1E},
{0x03AD,0x1A},
{0x03AE,0x13},
{0x03AF,0x0C},
{0x03B0,0x0B},
{0x03B1,0x09},
{0x03B3,0x10},
{0x03B4,0x00},
{0x03B5,0x10},
{0x03B6,0x00},
{0x03B7,0xEA},
{0x03B8,0x00},
{0x03B9,0x3A},
{0x03BA,0x01},
{0x03BB,0x9F},
{0x03BC,0xCF},
{0x03BD,0xE7},
{0x03BE,0xF3},
{0x03BF,0x01},
{0x03D0,0xF8},
{0x03E0,0x04},
{0x03E1,0x01},
{0x03E2,0x04},
{0x03E4,0x10},
{0x03E5,0x12},
{0x03E6,0x00},
{0x03E8,0x21},
{0x03E9,0x23},
{0x03EA,0x01},
{0x03EC,0x21},
{0x03ED,0x23},
{0x03EE,0x01},
{0x03F0,0x20},
{0x03F1,0x22},
{0x03F2,0x00},
{0x0420,0x84},
{0x0421,0x00},
{0x0422,0x00},
{0x0423,0x83},
{0x0430,0x08},
{0x0431,0x28},
{0x0432,0x10},
{0x0433,0x08},
{0x0435,0x0C},
{0x0450,0xFF},
{0x0451,0xE8},
{0x0452,0xC4},
{0x0453,0x88},
{0x0454,0x00},
{0x0458,0x98},
{0x0459,0x03},
{0x045A,0x00},
{0x045B,0x28},
{0x045C,0x00},
{0x045D,0x68},
{0x0466,0x14},
{0x047A,0x00},
{0x047B,0x00},
{0x0480,0x58},
{0x0481,0x06},
{0x0482,0x0C},
{0x04B0,0x50},
{0x04B6,0x30},
{0x04B9,0x10},
{0x04B3,0x10},
{0x04B1,0x8E},
{0x04B4,0x20},
{0x0540,0x00},
{0x0541,0x9D},
{0x0542,0x00},
{0x0543,0xBC},
{0x0580,0x01},
{0x0581,0x0F},
{0x0582,0x04},
{0x0594,0x00},
{0x0595,0x04},
{0x05A9,0x03},
{0x05AA,0x40},
{0x05AB,0x80},
{0x05AC,0x0A},
{0x05AD,0x10},
{0x05AE,0x0C},
{0x05AF,0x0C},
{0x05B0,0x03},
{0x05B1,0x03},
{0x05B2,0x1C},
{0x05B3,0x02},
{0x05B4,0x00},
{0x05B5,0x0C},
{0x05B8,0x80},
{0x05B9,0x32},
{0x05BA,0x00},
{0x05BB,0x80},
{0x05BC,0x03},
{0x05BD,0x00},
{0x05BF,0x05},
{0x05C0,0x10},
{0x05C3,0x00},
{0x05C4,0x0C},
{0x05C5,0x20},
{0x05C7,0x01},
{0x05C8,0x14},
{0x05C9,0x54},
{0x05CA,0x14},
{0x05CB,0xE0},
{0x05CC,0x20},
{0x05CD,0x00},
{0x05CE,0x08},
{0x05CF,0x60},
{0x05D0,0x10},
{0x05D1,0x05},
{0x05D2,0x03},
{0x05D4,0x00},
{0x05D5,0x05},
{0x05D6,0x05},
{0x05D7,0x05},
{0x05D8,0x08},
{0x05DC,0x0C},
{0x05D9,0x00},
{0x05DB,0x00},
{0x05DD,0x0F},
{0x05DE,0x00},
{0x05DF,0x0A},
#if 0
{0x05E0,0xAA},
{0x05E1,0x00},
{0x05E2,0xE0},
{0x05E3,0x00},
#else
//real setting ?
{0x05E0,0xD6},
{0x05E1,0x00},
{0x05E2,0x19},
{0x05E3,0x01},
#endif

{0x05E4,0x04},
{0x05E5,0x00},
{0x05E6,0x83},
{0x05E7,0x02},
{0x05E8,0x06},
{0x05E9,0x00},
{0x05EA,0xE5},
{0x05EB,0x01},
{0x0660,0x04},
{0x0661,0x16},
{0x0662,0x04},
{0x0663,0x28},
{0x0664,0x04},
{0x0665,0x18},
{0x0666,0x04},
{0x0667,0x21},
{0x0668,0x04},
{0x0669,0x0C},
{0x066A,0x04},
{0x066B,0x25},
{0x066C,0x00},
{0x066D,0x12},
{0x066E,0x00},
{0x066F,0x80},
{0x0670,0x00},
{0x0671,0x0A},
{0x0672,0x04},
{0x0673,0x1D},
{0x0674,0x04},
{0x0675,0x1D},
{0x0676,0x00},
{0x0677,0x7E},
{0x0678,0x01},
{0x0679,0x47},
{0x067A,0x00},
{0x067B,0x73},
{0x067C,0x04},
{0x067D,0x14},
{0x067E,0x04},
{0x067F,0x28},
{0x0680,0x00},
{0x0681,0x22},
{0x0682,0x00},
{0x0683,0xA5},
{0x0684,0x00},
{0x0685,0x1E},
{0x0686,0x04},
{0x0687,0x1D},
{0x0688,0x04},
{0x0689,0x19},
{0x068A,0x04},
{0x068B,0x21},
{0x068C,0x04},
{0x068D,0x0A},
{0x068E,0x04},
{0x068F,0x25},
{0x0690,0x04},
{0x0691,0x15},
{0x0698,0x20},
{0x0699,0x20},
{0x069A,0x01},
{0x069C,0x22},
{0x069D,0x10},
{0x069E,0x10},
{0x069F,0x08},
{0x0B20,0xBE},
//{0x007C,0x33},
{0x007C,0x40},
{0x0B02,0x04},
{0x0B07,0x25},
/*
{0x0B08,0x56},// V=342
{0x0B09,0x01},

{0x0B0A,0x5E},// H=606
{0x0B0B,0x02},

{0x0B3E,0x01},
*/


 

{0x0B0E,0x1D},
{0x0B0F,0x07},
{0x0B22,0x02},
{0x0B39,0x03},
{0x0B11,0x7F},
{0x0B12,0x7F},
{0x0B17,0xE0},
{0x0B30,0x0F},
{0x0B31,0x02},
{0x0B32,0x00},
{0x0B33,0x00},
{0x0B39,0x0F},
{0x0B3B,0x12},
{0x0B3F,0x01},
{0x0024,0x40},
//{0x0028,0x81},//test pattern enable
{0x0028,0xC0},
{0x0000,0x01},
{0x0100,0x01},
{0x0101,0x01},
{0x0005,0x01},
 
{ 0,0},
};

#else
#endif

static int hmx2056_write_reg(struct sensor_device *sensor_dev, u16 reg, u8 data)
{
    int ret;

    ret = kdp_drv_i2c_write(I2C_ADAP_0, sensor_dev->addr, reg, 2, data);

    return ret;
}

void hmx2056_init(struct sensor_device *sensor_dev, struct sensor_init_seq *seq)
{
    struct sensor_device *dev = sensor_dev;
    struct sensor_init_seq *init_fnc_ptr;

    for (init_fnc_ptr = seq; ; ++init_fnc_ptr) {
        //dbg_msg_camera("dev->adapter=%p dev->addr=%x init_fnc_ptr->addr=%x init_fnc_ptr->value=%x",
        //dev->adapter, dev->addr, init_fnc_ptr->addr, init_fnc_ptr->value);
        if(init_fnc_ptr->addr == 0 && init_fnc_ptr->value == 0) break; //reaches end
        hmx2056_write_reg(dev, init_fnc_ptr->addr , (u8)(init_fnc_ptr->value & 0xFF));
    }	
}

static struct hmx2056_context *to_hmx2056(const struct sensor_device *sensor_dev)
{
    return container_of(framework_drv_get_drvdata(&sensor_dev->pin_ctx), struct hmx2056_context, subdev);
}

static const struct sensor_win_size *hmx2056_select_win(u32 *width, u32 *height)
{
    int i, default_size = ARRAY_SIZE(hmx2056_supported_win_sizes) - 1;

    for (i = 0; i < ARRAY_SIZE(hmx2056_supported_win_sizes); i++) {
        if (hmx2056_supported_win_sizes[i].width  >= *width &&
            hmx2056_supported_win_sizes[i].height >= *height) {
            *width = hmx2056_supported_win_sizes[i].width;
            *height = hmx2056_supported_win_sizes[i].height;
            return &hmx2056_supported_win_sizes[i];
        }
    }

    *width = hmx2056_supported_win_sizes[default_size].width;
    *height = hmx2056_supported_win_sizes[default_size].height;
    return &hmx2056_supported_win_sizes[default_size];
}

static
int hmx2056_set_params(struct sensor_device *sensor_dev, u32 *width, u32 *height, u32 fourcc)
{
    struct hmx2056_context *ctx = to_hmx2056(sensor_dev);

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
    //hmx2056_reset(sensor_dev);

    /* initialize the sensor with default settings */
    hmx2056_init(sensor_dev, hmx2056_init_regs);

    return 0;
}

static int hmx2056_s_power(struct v2k_subdev *sd, int on)
{
    struct sensor_device *dev = v2k_get_subdevdata(sd);
    struct hmx2056_context *ctx = to_hmx2056(dev);
    int ret;
    
    dbg_msg_camera("%s", __func__);

    ret = 0;
    
    return ret;
}

static int hmx2056_reset(struct v2k_subdev *sd)
{
    //struct sensor_device *dev = v2k_get_subdevdata(sd);
    //struct hmx2056_context *ctx = to_hmx2056(dev);
    //int ret;

    //ret = sys_camera_power_on(&dev->pin_ctx, ssdd);
    //if (ret < 0)
    //    return ret;

    //return ret;
    
    return 0;
}

static int hmx2056_s_stream(struct v2k_subdev *sd, int enable)
{
    //struct sensor_device *dev = v2k_get_subdevdata(sd);

    return 0;
}

static int hmx2056_enum_fmt(
        struct v2k_subdev *sd, unsigned int index, unsigned int *code)
{
    if (index >= ARRAY_SIZE(hmx2056_colour_fmts))
        return -KDP_FRAMEWORK_ERRNO_INVALA;

    *code = hmx2056_colour_fmts[index].fourcc;
    return 0;
}

static int hmx2056_get_fmt(struct v2k_subdev *sd, struct v2k_format *format)
{
    //struct sensor_device *dev = v2k_get_subdevdata(sd);

    return 0;
}

static int hmx2056_set_fmt(struct v2k_subdev *sd, struct v2k_format *format)
{
    struct v2k_format *fmt = format;
    struct sensor_device *dev = v2k_get_subdevdata(sd);

    dbg_msg_camera("[%s]", __func__);

    hmx2056_select_win(&fmt->width, &fmt->height);

    return hmx2056_set_params(dev, &fmt->width, &fmt->height, fmt->pixelformat);
}

static struct v2k_subdev_ops hmx2056_subdev_ops = {
    .s_power	= hmx2056_s_power,
    .reset	    = hmx2056_reset,
    .s_stream	= hmx2056_s_stream,
    .enum_fmt 	= hmx2056_enum_fmt, 
    .get_fmt	= hmx2056_get_fmt,
    .set_fmt	= hmx2056_set_fmt,	
};


static int hmx2056_probe(struct sensor_device *sensor_dev)
{
    int ret;    
    struct hmx2056_context *ctx;

    ctx = calloc(1, sizeof(struct hmx2056_context));
    if (!ctx)
        return -KDP_FRAMEWORK_ERRNO_NOMEM;

    v2k_subdev_sensor_init(&ctx->subdev, sensor_dev, &hmx2056_subdev_ops);

    //ctx->fmt = &hmx2056_colour_fmts[0];
    ret = 0;

    return ret;
}

static int hmx2056_remove(struct sensor_device *sensor_dev)
{
    //free_bus

    return 0;
}

extern struct core_device hmx2056_link_device;
static struct sensor_driver hmx2056_i2c_driver = {
    .driver = {
        .name = "sensor-hmx2056",
    },
    .probe		= hmx2056_probe,
    .remove		= hmx2056_remove,
    .core_dev   = &hmx2056_link_device,    
};
KDP_SENSOR_DRIVER_SETUP(hmx2056_i2c_driver);

#endif