#include "isp_type.h"
#include "isp_i2c.h"
#include "isp_print.h"
#include "isp_sns_ctrl.h"
#include "XAx_cmos.h"


extern ISP_CMOS_SNS_ATTR gstSnsAttr[ISP_NUM_MAX];

// ExtClk: 37.125M
// 1080P 12.5fps,12Bit,I2C,MIPI 4lane,ADC12Bit
// Nomal Mode register setting
static const ISP_U16 gau16SnsInit_imx307[][2] = 
{
    // Enter Standby
    {0x3000, 0x01},     // Standby mode
    {0x3002, 0x01},  // Master mode stop
    // Initial register setting (need rewrite after reset)
    // Chip id:02
    {0x3011, 0x0a},
    {0x309e, 0x4a},
    {0x309f, 0x4a},
    //chip id:03    
    {0x3128, 0x04},
    {0x313b, 0x41},
    // Mode register setting
    {0x3005, 0x01},
    {0x3007, 0x00},
    {0x3009, 0x02},  // 30fps;0x00->120fps
    {0x3012, 0x64},
    {0x3018, 0x65},  // VMAX
    {0x3019, 0x04},
    //{0x301c, 0x30},  // HMAX;1130H->30fps;14A0H->25fps;
    //{0x301d, 0x11},  // HMAX;
    {0x301c, 0xa0},  // HMAX;1130H->30fps;14A0H->25fps;
    {0x301d, 0x14},  // HMAX;

    {0x3020, 0x80},  //
    {0x3021, 0x02},  // 

    {0x3046,    0x01},  // LVDS 4CH;12Bit
    //0x3048    0x02    // XVS/XHS Output low width
    //0x3049    0x03
#if 1 
	{0x305c, 0x18},  //37.125MHz INCK Setting
    {0x305d, 0x03},
    {0x305e, 0x20},
    {0x305f, 0x01},
    {0x315e, 0x1a},  //37.125MHz INCK5 Setting
    {0x3164, 0x1a},
    {0x3480, 0x49},  //37.125MHz INCK7 Setting
#else
	{0x305c, 0x0c},  //74.25MHz INCK Setting
    {0x305d, 0x03},
    {0x305e, 0x10},
    {0x305f, 0x01},
    {0x315e, 0x1B},  //74.25MHz INCK5 Setting
    {0x3164, 0x1B},
    {0x3480, 0x92},  //74.25MHz INCK7 Setting
#endif
    {0x3129, 0x00},  // ADBIT1,12Bit;
    {0x317c, 0x00},  // ADBIT2,12Bit;
    {0x31ec, 0x0e},  // ADBIT3,12Bit;
    //CHIP ID 06 For MIPI I/F
    {0x3405, 0x20},
    {0x3407, 0x03},
    {0x3414, 0x0a},
    {0x3418, 0x49},
    {0x3419, 0x04},  
    {0x3441, 0x0c},
    {0x3442, 0x0c},
    {0x3443, 0x03},    
    
	//{0x3444, 0x20},
    //{0x3445, 0x25},
    {0x3444, 0x40},	//74.25MHz
    {0x3445, 0x4a},
    
    {0x3446, 0x47},
    {0x3447, 0x00},
    {0x3448, 0x1f},
    {0x3449, 0x00},
    {0x344A, 0x17},
    {0x344B, 0x00},
    {0x344C, 0x0F},
    {0x344D, 0x00},
    {0x344E, 0x17},
    {0x344F, 0x00},
    {0x3450, 0x47},
    {0x3451, 0x00},
    {0x3452, 0x0F},
    {0x3453, 0x00},
    {0x3454, 0x0f},
    {0x3455, 0x00},
    {0x3472, 0x9C},
    {0x3473, 0x07},
    // Standby Cancel
    {0x3000, 0x00},  // standby
    //wait(20)       // DELAY20mS
    //{0x3002, 0x00},  // master mode start
    //{0x304B, 0x0a}  // XVSOUTSEL XHSOUTSEL enable output
    //wait(20)

};

// ExtClk: 37.125M
// 1080P 12.5fps,12Bit,I2C,MIPI 2lane,ADC12Bit
static const ISP_U16 gau16SnsInit_imx307_2lane[][2] = 
{
	//Enter Standby
	{0x3000,0x01}, 	//Standby mode
	{0x3002,0x01},	//Master mode stop
	{0x3005,0x01},
	{0x3007,0x00},
	{0x3009,0x02},	//30fps;0x00->120fps
	{0x300a,0xf0},
	{0x3011,0x0a},

	{0x3018,0x65},	//VMAX
	{0x3019,0x04},
	{0x301c,0xA0},	//HMAX;1130H->30fps;14A0H->25fps;
	{0x301d,0x14}, 	//HMAX;
	{0x3046,0x01},	//MIPI;12Bit

#if 1
	{0x305c, 0x18},  //37.125MHz INCK Setting
	{0x305d, 0x03},
	{0x305e, 0x20},
	{0x305f, 0x01},
	{0x315e, 0x1a},  //37.125MHz INCK5 Setting
	{0x3164, 0x1a},
	{0x3480, 0x49},  //37.125MHz INCK7 Setting
#else
	{0x305c, 0x0c},  //74.25MHz INCK Setting
	{0x305d, 0x03},
	{0x305e, 0x10},
	{0x305f, 0x01},
	{0x315e, 0x1B},  //74.25MHz INCK5 Setting
	{0x3164, 0x1B},
	{0x3480, 0x92},  //74.25MHz INCK7 Setting
#endif

	{0x309e,0x4a},
	{0x309f,0x4a},

	{0x311c,0x0e},
	{0x3128,0x04},
	{0x3129,0x00},	//ADBIT1,12Bit;
	{0x313b,0x41},
	{0x317c,0x00},	//ADBIT2,12Bit;
	{0x31ec,0x0e},	//ADBIT3,12Bit;

	//CHIP ID 06 For MIPI I/F
	{0x3405,0x10},
	{0x3407,0x01},
	{0x3441,0x0c},
	{0x3442,0x0c},
	{0x3443,0x01}, 	
	{0x3444,0x20},
	{0x3445,0x25},
	{0x3446,0x57},
	{0x3447,0x00},
	{0x3448,0x37},
	{0x3449,0x00},
	{0x344A,0x1f},
	{0x344B,0x00},
	{0x344C,0x1F},
	{0x344D,0x00},
	{0x344E,0x1f},
	{0x344F,0x00},
	{0x3450,0x77},
	{0x3451,0x00},
	{0x3452,0x1F},
	{0x3453,0x00},
	{0x3454,0x17},
	{0x3455,0x00},

	//Cropping mode
	{0x3007,0x40},
	{0x303a,0x06},// WINWV_OB,6-12H
	{0x303C,0x04},// WINPV
	{0x303D,0x00},// WINPV
	{0x303E,0x40},// WINWV
	{0x303F,0x04},// WINWV

	{0x3040,0x08},// WINPH
	{0x3041,0x00},// WINPH
	{0x3042,0x88},// WINWH
	{0x3043,0x07},// WINWH
	{0x3472,0x88},   // X_OUT_SIZE
	{0x3473,0x07},   // X_OUT_SIZE
	{0x3414,0x04},// OPB_SIZE_V=WINWV_OB-2
	{0x3418,0x40},	// Y_OUT_SIZE:WINWV; 
	{0x3419,0x04},
	{0x3404,0x00},

	// Standby Cancel
	{0x3000,0x00},	// standby
};


void sensor_init_imx307(ISP_DEV IspDev)
{
	ISP_U16 u16Num,u16i;
	if((gstSnsAttr[IspDev].u8MipiAttr&0x0F) == MIPI_4LANE)
	{
		u16Num = sizeof(gau16SnsInit_imx307)/sizeof(gau16SnsInit_imx307[0]);
		sensor_write_register(IspDev,(ISP_U32)gau16SnsInit_imx307[0][0], (ISP_U32)gau16SnsInit_imx307[0][1]);   // reset all registers
		for(u16i=1; u16i < u16Num; u16i++)
		{
			sensor_write_register(IspDev,(ISP_U32)gau16SnsInit_imx307[u16i][0], (ISP_U32)gau16SnsInit_imx307[u16i][1]);
		}
		
		XM_MPI_ISP_USleep(20000);	// 20ms
		sensor_write_register(IspDev,0x3002,0x00);
		sensor_write_register(IspDev,0x304B,0x0a);
		XM_MPI_ISP_USleep(20000);	// 20ms
		DEBUG("------------- IMX307 2M 12.5fps init ok! (@20210319) 4lane---------------\n");
	}
	else
	{
		u16Num = sizeof(gau16SnsInit_imx307_2lane)/sizeof(gau16SnsInit_imx307_2lane[0]);
		sensor_write_register(IspDev,(ISP_U32)gau16SnsInit_imx307_2lane[0][0], (ISP_U32)gau16SnsInit_imx307_2lane[0][1]);   // reset all registers
		for(u16i=1; u16i < u16Num; u16i++)
		{
			sensor_write_register(IspDev,(ISP_U32)gau16SnsInit_imx307_2lane[u16i][0], (ISP_U32)gau16SnsInit_imx307_2lane[u16i][1]);
		}
		
		XM_MPI_ISP_USleep(20000);	// 20ms
		sensor_write_register(IspDev,0x3002,0x00);
		sensor_write_register(IspDev,0x304B,0x0a);
		XM_MPI_ISP_USleep(20000);	// 20ms
		DEBUG("------------- IMX307 2M 25fps init ok! (@20210319) 2lane---------------TT\n");
	}
};


