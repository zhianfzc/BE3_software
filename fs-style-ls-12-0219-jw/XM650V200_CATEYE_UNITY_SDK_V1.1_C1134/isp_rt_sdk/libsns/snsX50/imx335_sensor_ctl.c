#include "isp_type.h"
#include "isp_print.h"
#include "isp_i2c.h"
#include "isp_sns_ctrl.h"
#include "XAx_cmos.h"

extern void SysDelay_ms(ISP_S32 ms);


static const ISP_U16 gau16SnsInit_IMX335[][2] = {
    //ExtClk: 24M   2616*1964 4000*2475
    //MIPI Data Rate��891Mbps/Lane��
    {0x3000,        0x01},  //STANDBY
    {0x3004,        0x04},  //stream off

    {0x3002,        0x00},	//XMSTA
    {0x3004,        0x00},  //RESTART
    {0x300C,        0x3b},  //BCWAIT_TIME
    {0x300D,        0x2a},  //CPWAIT_TIME
    {0x3018,        0x00},  //WINMODE
    {0x302C,        0x30},  //HTRIMMING_START
    {0x302D,        0x00},  //HTRIMMING_START
    {0x302E,        0x38},  //HNUM
    {0x302F,        0x0A},  //HNUM
    {0x3030,        0xAC},  //VMAX,4950d;
    {0x3031,        0x26},  //VMAX
    {0x3032,        0x00},  //VMAX
    {0x3034,        0xF4},  //HMAX,1000d;
    {0x3035,        0x01},  //HMAX
    {0x304C,        0x14},  //OPB_SIZE_V
    {0x304E,        0x00},  //HREVERSE
    {0x304F,        0x00},  //VREVERSE
    {0x3050,        0x01},  //ADBIT
    {0x3056,        0xAC},  //Y_OUT_SIZE
    {0x3057,        0x07},  //Y_OUT_SIZE
    {0x3058,        0x09},  //SHR0
    {0x3059,        0x00},  //SHR0
    {0x305A,        0x00},  //SHR0
    {0x3072,        0x28},  //AREA2_WIDTH_1
    {0x3073,        0x00},  //AREA2_WIDTH_1
    {0x3074,        0xB0},  //AREA3_ST_ADR_1
    {0x3075,        0x00},  //AREA3_ST_ADR_1
    {0x3076,        0x58},  //AREA3_WIDTH_1
    {0x3077,        0x0F},  //AREA3_WIDTH_1
    {0x3078,        0x01},  //Reverse
    {0x3079,        0x02},  //Reverse
    {0x307A,        0xFF},  //Reverse
    {0x307B,        0x02},  //Reverse
    {0x307C,        0x00},  //Reverse
    {0x307D,        0x00},  //Reverse
    {0x307E,        0x00},  //Reverse
    {0x307F,        0x00},  //Reverse
    {0x3080,        0x01},  //Reverse
    {0x3081,        0x02},  //0xFE:Reverse  0x02:Normal
    {0x3082,        0xFF},  //Reverse
    {0x3083,        0x02},  //0xFE:Reverse  0x02:Normal
    {0x3084,        0x00},  //Reverse
    {0x3085,        0x00},  //Reverse
    {0x3086,        0x00},  //Reverse
    {0x3087,        0x00},  //Reverse
    {0x30A4,        0x33},  //Reverse
    {0x30A8,        0x10},  //Reverse
    {0x30A9,        0x04},  //Reverse
    {0x30AC,        0x00},  //Reverse
    {0x30AD,        0x00},  //Reverse
    {0x30B0,        0x10},  //Reverse
    {0x30B1,        0x08},  //Reverse
    {0x30B4,        0x00},  //Reverse
    {0x30B5,        0x00},  //Reverse
    {0x30B6,        0x00},  //0xFA:Reverse  0x00:Normal
    {0x30B7,        0x00},  //0x01:Reverse  0x00:Normal
    {0x3112,        0x08},  //Reverse
    {0x3113,        0x00},  //Reverse
    {0x3116,        0x08},  //0x02:Reverse  0x08:Normal
    {0x3117,        0x00},  //Reverse
    {0x314C,        0x29},  //INCKSEL1
    {0x314D,        0x01},  //INCKSEL1
    {0x315A,        0x06},  //INCKSEL2
    {0x3168,        0xa0},  //INCKSEL3
    {0x316A,        0x7e},  //INCKSEL4
    {0x3199,        0x00},  //HADD/VADD
    {0x319D,        0x01},  //MDBIT 0:10Bit  1:12Bit
    {0x319E,        0x02},  //SYS_MODE  0x01:1188M  0x02:891M
    {0x31A0,        0x2A},  //XH&VSOUTSEL
    {0x3300,        0x00},  //TCYCLE
    {0x31A1,        0x00},  //XH&VSDRV
    {0x3288,        0x21},  //
    {0x328A,        0x02},  //
    {0x3302,        0x32},  //BLKLEVEL
    {0x3414,        0x05},  //
    {0x3416,        0x18},  //
    {0x341C,        0x47},  //ADBIT1 10Bit:0x01FF  12Bit:0x0047
    {0x341D,        0x00},  //ADBIT1 10Bit:0x01FF  12Bit:0x0047{0x3648,  0,  8) =        0x01  //
    {0x364A,        0x04},  //
    {0x364C,        0x04},  //
    {0x3678,        0x01},  //
    {0x367C,        0x31},  //
    {0x367E,        0x31},  //
    {0x3706,        0x10},  //
    {0x3708,        0x03},  //
    {0x3714,        0x02},  //
    {0x3715,        0x02},  //
    {0x3716,        0x01},  //
    {0x3717,        0x03},  //
    {0x371C,        0x3D},  //
    {0x371D,        0x3F},  //
    {0x372C,        0x00},  //
    {0x372D,        0x00},  //
    {0x372E,        0x46},  //
    {0x372F,        0x00},  //
    {0x3730,        0x89},  //
    {0x3731,        0x00},  //
    {0x3732,        0x08},  //
    {0x3733,        0x01},  //
    {0x3734,        0xFE},  //
    {0x3735,        0x05},  //
    {0x3740,        0x02},  //
    {0x375D,        0x00},  //
    {0x375E,        0x00},  //
    {0x375F,        0x11},  //
    {0x3760,        0x01},  //
    {0x3768,        0x1B},  //
    {0x3769,        0x1B},  //
    {0x376A,        0x1B},  //
    {0x376B,        0x1B},  //
    {0x376C,        0x1A},  //
    {0x376D,        0x17},  //
    {0x376E,        0x0F},  //
    {0x3776,        0x00},  //
    {0x3777,        0x00},  //
    {0x3778,        0x46},  //
    {0x3779,        0x00},  //
    {0x377A,        0x89},  //
    {0x377B,        0x00},  //
    {0x377C,        0x08},  //
    {0x377D,        0x01},  //
    {0x377E,        0x23},  //
    {0x377F,        0x02},  //
    {0x3780,        0xD9},  //
    {0x3781,        0x03},  //
    {0x3782,        0xF5},  //
    {0x3783,        0x06},  //
    {0x3784,        0xA5},  //
    {0x3788,        0x0F},  //
    {0x378A,        0xD9},  //
    {0x378B,        0x03},  //
    {0x378C,        0xEB},  //
    {0x378D,        0x05},  //
    {0x378E,        0x87},  //
    {0x378F,        0x06},  //
    {0x3790,        0xF5},  //
    {0x3792,        0x43},  //
    {0x3794,        0x7A},  //
    {0x3796,        0xA1},  //
    {0x37B0,        0x36},  //0x37: XMASTER=High(Slave)  0x36:XMASTER=Low(Master)
    {0x3A00,        0x01},  //for CUE 0x00:HS 0x01: LP-11
    {0x3A01,        0x03},  //LANEMODE 0x03:4Lane  0x01:2Lane
    {0x3A18,        0x8F},  //TCLKPOST
    {0x3A19,        0x00},  //TCLKPOST
    {0x3A1A,        0x4F},  //TCLKPREPARE
    {0x3A1B,        0x00},  //TCLKPREPARE
    {0x3A1C,        0x47},  //TCLKTRAIL
    {0x3A1D,        0x00},  //TCLKTRAIL
    {0x3A1E,        0x37},  //TCLKZERO
    {0x3A1F,        0x01},  //TCLKZERO
    {0x3A20,        0x4F},  //THSPREPARE
    {0x3A21,        0x00},  //THSPREPARE
    {0x3A22,        0x87},  //THSZERO
    {0x3A23,        0x00},  //THSZERO
    {0x3A24,        0x4F},  //THSTRAIL
    {0x3A25,        0x00},  //THSTRAIL
    {0x3A26,        0x7F},  //THSEXIT
    {0x3A27,        0x00},  //THSEXIT
    {0x3A28,        0x3F},  //TLPX
    {0x3A29,        0x00},  //TLPX
    
    
    // release standby
    //{0x3000,        0x00},
    //wait_ms( 100 )
    
    // XMSTA
    //{0x3002,        0x00},
    //wait_ms( 100 )
    // V_inverted setting
    //{0x3078,        0x01},  //Reverse
    //{0x3079,        0x02},  //Reverse
    //{0x307A,        0xFF},  //Reverse
    //{0x307B,        0x02},  //Reverse
    //{0x307C,        0x00},  //Reverse
    //{0x307D,        0x00},  //Reverse
    //{0x307E,        0x00},  //Reverse
    //{0x307F,        0x00},  //Reverse
    //{0x3080,        0x01},  //Reverse
    //{0x3081,        0xFE},  //0xFE:Reverse  0x02:Normal
    //{0x3082,        0xFF},  //Reverse
    //{0x3083,        0xFE},  //0xFE:Reverse  0x02:Normal
    //{0x3084,        0x00},  //Reverse
    //{0x3085,        0x00},  //Reverse
    //{0x3086,        0x00},  //Reverse
    //{0x3087,        0x00},  //Reverse
    //{0x30A4,        0x33},  //Reverse
    //{0x30A8,        0x10},  //Reverse
    //{0x30A9,        0x04},  //Reverse
    //{0x30AC,        0x00},  //Reverse
    //{0x30AD,        0x00},  //Reverse
    //{0x30B0,        0x10},  //Reverse
    //{0x30B1,        0x08},  //Reverse
    //{0x30B4,        0x00},  //Reverse
    //{0x30B5,        0x00},  //Reverse
    //{0x30B6,        0xFA},  //0xFA:Reverse  0x00:Normal
    //{0x30B7,        0x01},  //0x01:Reverse  0x00:Normal
    //{0x3112,        0x08},  //Reverse
    //{0x3113,        0x00},  //Reverse
    //{0x3116,        0x02},  //0x02:Reverse  0x08:Normal
    //{0x3117,        0x00},  //Reverse
    
    // End of File
};

void sensor_init_imx335(ISP_DEV IspDev)
{
	ISP_U16 u16Num,u16i;
	u16Num = sizeof(gau16SnsInit_IMX335)/sizeof(gau16SnsInit_IMX335[0]);

	sensor_write_register(IspDev,(ISP_U32)gau16SnsInit_IMX335[0][0], (ISP_U32)gau16SnsInit_IMX335[0][1]);
	sensor_write_register(IspDev,(ISP_U32)gau16SnsInit_IMX335[1][0], (ISP_U32)gau16SnsInit_IMX335[1][1]);
	SysDelay_ms(50);
	for(u16i=2; u16i < u16Num; u16i++)
	{
		sensor_write_register(IspDev,(ISP_U32)gau16SnsInit_IMX335[u16i][0], (ISP_U32)gau16SnsInit_IMX335[u16i][1]);
	}
	SysDelay_ms(10);
	 // release standby
	sensor_write_register(IspDev,0x3000,0x00);
	SysDelay_ms(50);
	sensor_write_register(IspDev,0x3002,0x00); //// XMSTA
	SysDelay_ms(50);

	
	DEBUG("------------- IMX335 5M 15fps  init ok! (@201801228_158x4)----------------20200602\n");
}

