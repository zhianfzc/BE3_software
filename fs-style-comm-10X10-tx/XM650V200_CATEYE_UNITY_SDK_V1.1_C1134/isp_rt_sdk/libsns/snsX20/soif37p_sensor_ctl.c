#include "isp_type.h"
#include "isp_print.h"
#include "isp_i2c.h"
#include "isp_sns_ctrl.h"
#include "XAx_cmos.h"


//Mclk: 27M
//Pclk: 86.4M
//Active:1928*1088
//Total:2560*1125*30 fps
//mipi 10bit 2lane
static const ISP_U8 gau8SnsInit_F37P_30Fps[][2] = {
	{0x12,0x40},
	{0x48,0x8A},
	{0x48,0x0A},
	{0x0E,0x19},
	{0x0F,0x04},
	{0x10,0x20},
	{0x11,0x80},
	{0x46,0x09},
	{0x47,0x66},
	{0x0D,0xF2},
	{0x57,0x6A},
	{0x58,0x22},
	{0x5F,0x41},
	{0x60,0x24},
	{0xA5,0xC0},
	{0x20,0x00},
	{0x21,0x05},
	{0x22,0x65},
	{0x23,0x04},
	{0x24,0xC4},
	{0x25,0x40},
	{0x26,0x43},
	{0x27,0xC6},
	{0x28,0x11},
	{0x29,0x04},
	{0x2A,0xBB},
	{0x2B,0x14},
	{0x2C,0x00},
	{0x2D,0x00},
	{0x2E,0x16},
	{0x2F,0x04},
	{0x41,0xC9},
	{0x42,0x33},
	{0x47,0x46},
	{0x76,0x6A},
	{0x77,0x09},
	{0x80,0x01},
	{0xAF,0x22},
	{0xAB,0x00},
	{0x1D,0x00},
	{0x1E,0x04},
	{0x6C,0x40},
	{0x9E,0xF8},
	{0x6E,0x2C},
	{0x70,0x6C},
	{0x71,0x6D},
	{0x72,0x6A},
	{0x73,0x56},
	{0x74,0x02},
	{0x78,0x9D},
	{0x89,0x01},
	{0x6B,0x20},
	{0x86,0x40},
	{0x31,0x10},
	{0x32,0x18},
	{0x33,0xE8},
	{0x34,0x5E},
	{0x35,0x5E},
	{0x3A,0xAF},
	{0x3B,0x00},
	{0x3C,0xFF},
	{0x3D,0xFF},
	{0x3E,0xFF},
	{0x3F,0xBB},
	{0x40,0xFF},
	{0x56,0x92},
	{0x59,0xAF},
	{0x5A,0x47},
	{0x61,0x18},
	{0x6F,0x04},
	{0x85,0x5F},
	{0x8A,0x44},
	{0x91,0x13},
	{0x94,0xA0},
	{0x9B,0x83},
	{0x9C,0xE1},
	{0xA4,0x80},
	{0xA6,0x22},
	{0xA9,0x1C},
	{0x5B,0xE7},
	{0x5C,0x28},
	{0x5D,0x67},
	{0x5E,0x11},
	{0x62,0x21},
	{0x63,0x0F},
	{0x64,0xD0},
	{0x65,0x02},
	{0x67,0x49},
	{0x66,0x00},
	{0x68,0x00},
	{0x69,0x72},
	{0x6A,0x12},
	{0x7A,0x00},
	{0x82,0x20},
	{0x8D,0x47},
	{0x8F,0x90},
	{0x45,0x01},
	{0x97,0x62},
	{0x13,0x81},
	{0x96,0x84},
	{0x4A,0x01},
	{0xB1,0x00},
	{0xA1,0x0F},
	{0xBE,0x00},
	{0x7E,0x48},
	{0xB5,0xC0},
	{0x50,0x02},
	{0x49,0x10},
	{0x7F,0x57},
	{0x90,0x00},
	{0x7B,0x4A},
	{0x7C,0x0C},
	{0x8C,0xFF},
	{0x8E,0x00},
	{0x8B,0x01},
	{0x0C,0x00},
	{0xBC,0x11},
	{0x19,0x20},
	{0x1B,0x4F},
	{0x97,0x62},
	{0x01,0x1F},
	{0x02,0x00},
	{0xB2,0x30},
	{0x98,0x27},
	{0xAE,0x05},
	{0x99,0xF8},
	{0x97,0x63},
	{0x12,0x00},
	{0x00,0x10},
};

void sensor_init_f37p(ISP_DEV IspDev)
{
	ISP_U16 u16Num,u16i;
	u16Num = sizeof(gau8SnsInit_F37P_30Fps)/sizeof(gau8SnsInit_F37P_30Fps[0]);
	for(u16i=0; u16i < u16Num; u16i++)
	{
		sensor_write_register(IspDev,(ISP_U32)gau8SnsInit_F37P_30Fps[u16i][0], (ISP_U32)gau8SnsInit_F37P_30Fps[u16i][1]);
	}
	DEBUG("------------- F37P (fast) ----------------\n");
	XM_MPI_ISP_USleep(20000);
}



