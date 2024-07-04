#include <unistd.h>
#include "isp_type.h"
#include "isp_comm_isp.h"
#include "isp_comm_sns.h"
#include "isp_sns_ctrl.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "isp_awb_comm.h"
#include "mpi_camera.h"
#include "isp_print.h"
#include "XAx_cmos.h"
#include "isp_i2c.h"

WDR_MODE_E genWdrMode = WDR_MODE_NONE;
ISP_U32 gau32ShutShortNow[ISP_NUM_MAX];
ISP_U32 gau32ShutNow[ISP_NUM_MAX];
ISP_U32 gau32AGainNow[ISP_NUM_MAX];
ISP_U32 gau32DGainNow[ISP_NUM_MAX];
ISP_U8 gau8Fps[ISP_NUM_MAX];
ISP_U16 gau16FullLines[ISP_NUM_MAX];

ISP_CMOS_SNS_ATTR gstSnsAttr[ISP_NUM_MAX];
ISP_SNS_REGS_INFO_S g_stSnsRegsInfo;
ISP_I2C_DATA_S gstIspI2CData[8];
ISP_U32 gau32PreI2CData[8];
ISP_CMOS_DN_PARA_S gstCmosDnPara[ISP_NUM_MAX];
ISP_U16 gau16HdrExpMax_L[ISP_NUM_MAX],gau16HdrExpMax_S[ISP_NUM_MAX];
WDR_MODE_E genHdrModeNow[ISP_NUM_MAX];


const ISP_U16 gau16GainTbl_SmatSns[64]={
1024,1088,1152,1216,1280,1344,1408,1472,1536,1600,1664,1728,1792,1856,1920,1984,2048,2176,2304,2432,2560,2688,2816,
2944,3072,3200,3328,3456,3584,3712,3840,3968,4096,4352,4608,4864,5120,5376,5632,5888,6144,6400,6656,6912,7168,7424,
7680,7936,8192,8704,9216,9728,10240,10752,11264,11776,12288,12800,13312,13824,14336,14848,15360,15872
};

const ISP_CMOS_SHADING_S gstShading = {
	ISP_FALSE,
	8,	// ng
	4,	// nz
	0x80,
	//ch:1
	0x1cf,0x10a,0x010,	// x,y,r0
	{0x000,0x001,0x004,0x008,0x00e,0x015,0x01e,0x027,0x032,0x040,0x050,0x061,0x074,0x087,0x09d,0x0b4,0x0cc,0x0eb,0x10a,0x128,0x148,0x16d,0x194,0x1c2,0x1ef,0x224,0x257,0x291,0x2c7,0x2f7,0x328,0x359,0x38b,0x3d8,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff},	// lut61
	//ch:2
	0x1d0,0x115,0x010,	// x,y,r0
	{0x000,0x002,0x006,0x00a,0x011,0x019,0x023,0x02f,0x03d,0x04e,0x061,0x076,0x08d,0x0a6,0x0c1,0x0db,0x0f7,0x11a,0x13b,0x15b,0x181,0x1a6,0x1d3,0x202,0x238,0x272,0x2aa,0x2e7,0x320,0x34d,0x38d,0x3af,0x3e4,0x3ff,0x3fd,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff},	// lut61
	//ch:3
	0x1d0,0x114,0x010,	// x,y,r0
	{0x000,0x001,0x006,0x009,0x011,0x019,0x023,0x02e,0x03c,0x04d,0x061,0x076,0x08f,0x0a6,0x0c1,0x0dc,0x0f8,0x11a,0x13c,0x15c,0x181,0x1a6,0x1d2,0x202,0x237,0x271,0x2aa,0x2e5,0x320,0x34e,0x385,0x3a9,0x3da,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff},	// lut61
	//ch:4
	0x1d7,0x11c,0x010,	// x,y,r0
	{0x000,0x002,0x006,0x009,0x00e,0x015,0x01e,0x028,0x034,0x044,0x054,0x068,0x07d,0x093,0x0aa,0x0c1,0x0dc,0x0fa,0x119,0x136,0x159,0x179,0x1a4,0x1ce,0x1fe,0x233,0x269,0x2a0,0x2d3,0x300,0x31c,0x341,0x379,0x36a,0x363,0x328,0x328,0x328,0x328,0x328,0x328,0x328,0x328,0x328,0x328,0x328,0x328,0x328,0x328,0x328,0x328,0x328,0x328,0x328,0x328,0x328,0x328,0x328,0x328,0x328},	// lut61
};

const ISP_CMOS_GAMMA_S gastIspGamma[6] = 
{
	{//0
		1,
		{	
		0,22,44,62,81,96,113,129,146,161,174,188,200,212,223,234,243,254,264,273,282,291,299,309,317,325,335,343,351,359,368,376,
		383,399,414,428,443,457,470,483,496,508,520,532,544,555,565,576,586,606,625,643,660,676,691,706,721,734,747,760,772,784,795,
		806,817,827,837,846,856,865,873,881,889,897,904,911,918,925,932,937,944,950,956,961,967,972,977,982,987,992,996,1002,1006,1011,
		1015,1020,1023
		}
	},
	{//1
		1,
		{
		0,33,110,149,173,192,207,220,232,243,255,266,278,288,299,310,320,330,339,347,356,363,371,379,387,394,402,409,417,424,431,
		438,445,459,472,485,498,511,523,535,546,557,568,579,590,600,610,620,629,648,665,682,698,713,728,742,755,768,780,792,803,814,
		824,834,844,853,862,870,879,887,894,902,909,915,922,928,934,940,946,951,956,962,966,971,976,980,984,989,993,997,1001,1005,
		1008,1012,1016,1020,1023
		}
	},
	{//2
		1,
		{
		0,18,36,54,72,90,107,125,142,158,175,190,206,221,235,249,262,275,287,299,311,323,334,346,357,
		368,380,391,402,413,424,434,445,464,481,497,511,523,535,546,556,567,577,588,598,608,618,627,637,
		655,673,690,706,722,737,751,765,778,791,803,814,825,836,846,855,864,873,881,888,895,902,909,915,
		921,926,931,936,941,946,950,955,959,963,967,971,975,979,983,987,991,995,1000,1004,1009,1014,1019,
		1023
		}
	},
	{//3
		1,
		{
		0,30,83,118,149,172,192,206,218,230,242,252,262,271,281,290,299,307,315,324,331,338,347,356,363,371,378,387,394,401,409,415,
		421,433,444,454,465,476,488,498,509,519,528,538,546,555,565,574,582,597,612,626,640,653,666,678,690,702,713,724,734,745,755,
		765,775,784,794,803,812,821,830,839,847,855,864,872,880,888,895,903,911,918,925,933,940,947,954,961,968,975,982,989,996,1003,
		1010,1016,1023
		}
	},
	{//4
		1,
		{
		0,19,39,59,78,97,116,135,154,172,190,207,224,240,255,271,285,299,313,326,339,352,364,375,386,397,408,418,428,438,447,457,465,
		482,498,513,527,541,553,565,576,586,596,606,615,623,632,640,648,663,677,691,705,717,730,742,755,766,778,790,801,812,823,834,
		844,853,862,871,879,887,894,902,909,915,922,928,934,939,945,950,955,960,965,970,974,979,983,988,992,996,1000,1004,1008,
		1012,1016,1019,1023
		}
	},
	{//	5	sc4336的宇视效果添加
		1,
		{
		0,39,78,115,150,182,211,234,254,270,284,295,304,312,320,328,335,343,351,359,366,374,382,389,397,
		404,412,419,427,434,441,448,455,469,482,495,508,520,532,544,555,566,577,587,597,608,617,627,636,
		654,672,689,705,720,735,750,764,777,789,802,813,824,835,845,855,864,872,880,888,895,902,908,914,
		920,926,931,936,941,945,950,955,959,963,968,972,976,981,985,989,994,998,1002,1006,1011,1015,1019,1023
		}
	}
};

const ISP_CMOS_SHARPEN_S gstSharpenColor = {
	/* bvalid */
	1,
	/* 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800 */
	/* Ud*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/* D*/
	{0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80},
	/*ThrUd*/
	{0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03},
	/*ThrD*/
	{0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03},	
	/*Ud_H*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*D_H*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*ThrUd_H*/
	{0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03},
	/*ThrD_H*/
	{0x03,0x03,0x03,0x03,0x03,0x06,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08},
	/*EdgeStr*/
	{0x48,0x46,0x44,0x40,0x40,0x40,0x40,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38},
	/*EdgeThr*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*OverShoot*/
	{0xAF,0xAF,0x9F,0xAF,0xDF,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
	/*UnderShoot*/
	{0xFF,0xFF,0xDF,0xDF,0xDF,0xDF,0xDF,0xDF,0xDF,0xDF,0xDF,0xDF,0xDF,0xDF,0xDF,0xDF},
	/*DetailStr*/
	{0x38,0x38,0x38,0x38,0x10,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*Rsv1*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*Rsv2*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};

const ISP_CMOS_SHARPEN_S gstSharpenBw = {
	/* bvalid */
	1,
	/* 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800 */
	/* Ud*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/* D*/
	{0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80},
	/*ThrUd*/
	{0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03},
	/*ThrD*/
	{0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03},	
	/*Ud_H*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*D_H*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*ThrUd_H*/
	{0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03},
	/*ThrD_H*/
	{0x03,0x03,0x03,0x03,0x03,0x06,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08},
	/*EdgeStr*/
	{0x68,0x68,0x58,0x48,0x40,0x40,0x40,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38},
	/*EdgeThr*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*OverShoot*/
	{0x9F,0x9F,0x9F,0xA7,0xA7,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF,0xBF},
	/*UnderShoot*/
	{0xFF,0xFF,0xFF,0xFF,0xEF,0xDF,0xDF,0xDF,0xDF,0xDF,0xDF,0xDF,0xDF,0xDF,0xDF,0xDF},
	/*DetailStr*/
	{0x30,0x30,0x2c,0x2a,0x28,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
	/*Rsv1*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*Rsv2*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};
	
const ISP_CMOS_VPSS_3DNR_S gstVpss3DNr = {
	0x01,
	/* 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800 */
	/* Flt_Thr    */
	{0x04,0x04,0x04,0x06,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18},
	/* Df_RatC    */
	{0x08,0x08,0x08,0x08,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04},
	/* Df1_RatioY    */
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/* Df1_RatioC    */
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/* MovThr_Max    */
	{0x00,0x00,0x00,0x01,0x01,0x01,0x02,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03},
	/* Alpha_Ratio    */
	{0x80,0x80,0x80,0x74,0x70,0x6c,0x60,0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50},
	/* Alpha_Min    */
	{0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07},
	/* IdxThr_Max    */
	{0x22,0x22,0x22,0x22,0x1c,0x1c,0x1c,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22},
	/* IdxThr_Min    */
	{0x20,0x20,0x20,0x20,0x1a,0x1a,0x1a,0x1a,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20},
	/* Idx_Max    */
	{0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20},
	/* FidY    */
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/* FidC    */
	{0x00,0x00,0x00,0x01,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02},
	/* FidY_Min    */
	{0x08,0x08,0x08,0x08,0x08,0x06,0x06,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04},
	/* FidC_Min    */
	{0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02},
	/* YSfThr_Min	 */
	{0x18,0x18,0x18,0x18,0x1c,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20},
	/* CSfThr_Min	 */
	{0x04,0x04,0x04,0x04,0x04,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06},
	/* YTfThr_Min    */
	{0x08,0x08,0x0c,0x0c,0x10,0x10,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14},
	/* CTfThr_Min    */
	{0x04,0x04,0x04,0x04,0x04,0x06,0x06,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08},
	/* YAntiRsdlSpd*/
	{0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04},
	/* CAntiRsdlSpd*/
	{0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06},
	/* YAntiRsdlThr*/
	{0x0f,0x0f,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20},
	/* CAntiRsdlThr*/
	{0x0a,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20},
	/* YSfThr_K    */
	{0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400},
	/* CSfThr_K    */
	{0x0200,0x0200,0x0400,0x0700,0x0c00,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000},
	/* YTfThr_K    */
	{0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400},
	/* CTfThr_K    */
	{0x0800,0x0800,0x0800,0x0800,0x0800,0x0800,0x0800,0x0800,0x0800,0x0800,0x0800,0x0800,0x0800,0x0800,0x0800,0x0800},
	/* MovThr1	  */
	{0xc000,0xc000,0x1000,0x1000,0x1000,0x1000,0x1000,0x0800,0x0800,0x0800,0x0800,0x0800,0x0800,0x0800,0x0800,0x0800},
	/* MovThr2	  */
	{0x8000,0x8000,0x8000,0x8000,0x8000,0x4000,0x4000,0x4000,0x4000,0x4000,0x4000,0x4000,0x4000,0x4000,0x4000,0x4000},
	/* MovThr3	  */
	{0x4000,0x4000,0x4000,0x4000,0x4000,0x3000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000},
};

const ISP_CMOS_YCNR_S gstYCNr = {
	0x01,	
	//Thresh
	{0x04,0x06,0x06,0x0a,0x0a,0x0a,0x0f,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
	//Str
	{0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20},
	//MixStr
	{0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06},
	//Wt1
	{0x00,0x00,0x01,0x02,0x04,0x04,0x05,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06},
	//Wt2
	{0x0f,0x0f,0x0d,0x0c,0x0a,0x09,0x08,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07},
	//Wt3
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	//Rsv
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};

const ISP_CMOS_LOCAL_CAC_S gstLocalCAC = {
	0x01,
	//thr
	{0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400,0x0400},
	//thr_g
	{0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180},
	//thr_y
	{0x0200,0x0200,0x0200,0x0200,0x0200,0x0200,0x0200,0x0200,0x0200,0x0200,0x0200,0x0200,0x0200,0x0200,0x0200,0x0200},
	//str_r
	{0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20},
	//str_b
	{0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60},
};

const ISP_CMOS_LTM_S gstLtm = {
	0x01,
	// rix
	{0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c},
	// kj_max
	{0x41,0x41,0x41,0x21,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05},
	// kj_min
	{0x60,0x60,0x50,0x40,0xa0,0xb0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0},
	// str_d
	{0x3C,0x46,0x50,0x5A,0x64,0x82,0x96,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA},
	// str_b
	{0xAA,0xB4,0xBE,0xC8,0xD2,0xDC,0xE6,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0}
};

const ISP_CMOS_LTM_S gstLtm_disable = {
	0x00,
	// rix
	{0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c},
	// kj_max
	{0x41,0x41,0x41,0x21,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05},
	// kj_min
	{0x60,0x60,0x50,0x40,0xa0,0xb0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0},
	// str_d
	{0x3C,0x46,0x50,0x5A,0x64,0x82,0x96,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA},
	// str_b
	{0xAA,0xB4,0xBE,0xC8,0xD2,0xDC,0xE6,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0}
};

const ISP_CMOS_2DNR_S gst2DNr = {
	0x01,
	{0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08},
	{0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
	{0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef},
	{0x20,0x20,0x20,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08},
	{0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
	{0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef},
	{0x20,0x20,0x20,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
};
	
int sensor_register_callback(ISP_DEV IspDev)
{
	ISP_S32 s32Ret;
	ALG_LIB_S stLib;
	ISP_SENSOR_REGISTER_S stIspRegister;
	AE_SENSOR_REGISTER_S  stAeRegister;
	AWB_SENSOR_REGISTER_S stAwbRegister;
	CHECK_ISPCHN(IspDev);
	XM_MPI_ISP_Memset(&stIspRegister, 0, sizeof(ISP_SENSOR_REGISTER_S));
	XM_MPI_ISP_Memset(&stAeRegister, 0, sizeof(AE_SENSOR_REGISTER_S));
	XM_MPI_ISP_Memset(&stAwbRegister, 0, sizeof(AWB_SENSOR_REGISTER_S));
	sensor_init(IspDev);	
	if(gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function)
	{
		gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function(&(stIspRegister.stSnsExp));
	}
	if(gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function)
	{
		gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function(&(stAeRegister.stSnsExp));
	}
	if(gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function)
	{
		gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function(&(stAwbRegister.stSnsExp));
	}

	stLib.s32Id = (ISP_S32)(gstSnsAttr[IspDev].s32SnsChipId);
	s32Ret = XM_MPI_ISP_SensorRegCallBack(IspDev,stLib.s32Id, 
			(const ISP_SENSOR_REGISTER_S * const)&stIspRegister);
	if (s32Ret)
	{
		ERR("SensorRegBk fail\r\n");
		return s32Ret;
	}
	XM_MPI_ISP_Memncpy((ISP_U8*)stLib.acLibName, (ISP_U8*)AE_LIB_NAME, sizeof(AE_LIB_NAME));
	s32Ret = XM_MPI_AE_SensorRegCallBack(IspDev, &stLib, (ISP_S32)(gstSnsAttr[IspDev].s32SnsChipId), &stAeRegister);
	if (s32Ret)
	{
		ERR("SensorRegBk ae fail\r\n");
		return s32Ret;
	}
	XM_MPI_ISP_Memncpy((ISP_U8*)stLib.acLibName, (ISP_U8*)AWB_LIB_NAME, sizeof(AWB_LIB_NAME));
	s32Ret = XM_MPI_AWB_SensorRegCallBack(IspDev, &stLib, (ISP_S32)(gstSnsAttr[IspDev].s32SnsChipId), &stAwbRegister);
	if (s32Ret)
	{
		ERR("SensorRegBk awb fail\r\n");
		return s32Ret;
	}
	return 0;
}

int sensor_unregister_callback(ISP_DEV IspDev)
{
    ISP_S32 s32Ret;
    ALG_LIB_S stLib;
	CHECK_ISPCHN(IspDev);
    s32Ret = XM_MPI_ISP_SensorUnRegCallBack(IspDev);
    if (s32Ret)
    {
        ERR("SensorUnregBk fail\r\n");
        return s32Ret;
    }
    
    stLib.s32Id = (ISP_S32)(gstSnsAttr[IspDev].s32SnsChipId);
    s32Ret = XM_MPI_AE_SensorUnRegCallBack(IspDev, &stLib, (ISP_S32)(gstSnsAttr[IspDev].s32SnsChipId));
    if (s32Ret)
    {
        ERR("SensorUnregBk ae fail\r\n");
        return s32Ret;
    }
    return 0;
}

int sensor_mode_set(ISP_DEV IspDev,ISP_U8 u8Mode)
{
	return 0;
}

int  sensor_mode_get(ISP_DEV IspDev)
{
	return 0;
}

int  sensor_get_chip(ISP_DEV IspDev,ISP_U8 u8I2cCh,I2C_DATA_S *pstI2CData,ISP_U8 u8FunExt)
{
	static ISP_U8 su8First=1;
	ISP_S32 s32Id,s32Tmp;
	if(su8First)
	{
		su8First = 0;
		XM_MPI_ISP_Memset((ISP_U8 *)&(gstCmosDnPara[0]), 0, sizeof(gstCmosDnPara));
	}

	gstSnsAttr[IspDev].s32SnsChipId = SENSOR_CHIP_UNKNOW;
	gstSnsAttr[IspDev].stSnsI2C.u8Channel = u8I2cCh;
	gstSnsAttr[IspDev].stSnsI2C.s32DevFd = -1;
	// SmartSns
#if 1	// 新板子
	if(	FUN_EXT_DOOR_LOCK_FACE_USED == u8FunExt ||
		FUN_EXT_DOOR_LOCK_FACE == u8FunExt ||
		FUN_EXT_DOOR_LOCK_FACE_UVC == u8FunExt)
	{
		if(IspDev==0)
		{
			#if 0
			gstSnsAttr[0].stSnsI2C.u8DevAddr = 0x30;
			gstSnsAttr[0].stSnsI2C.u32RegAddrByte = 2;
			gstSnsAttr[0].stSnsI2C.u32RegDataByte = 1;
			sensor_write_register(0,0x0103,0x01);
			gstSnsAttr[1].stSnsI2C.u8DevAddr = 0x32;
			gstSnsAttr[1].stSnsI2C.u32RegAddrByte = 2;
			gstSnsAttr[1].stSnsI2C.u32RegDataByte = 1;
			sensor_write_register(1,0x0103,0x01);
			XM_MPI_ISP_USleep(100);
			gstSnsAttr[0].pfn_cmos_sensor_init = sensor_init_sc2336;
			gstSnsAttr[0].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc2336;
			gstSnsAttr[0].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc2336;
			gstSnsAttr[0].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc2336;
			gstSnsAttr[1].pfn_cmos_sensor_init = sensor_init_sc2336;
			gstSnsAttr[1].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc2336;
			gstSnsAttr[1].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc2336;
			gstSnsAttr[1].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc2336;
			#else
			gstSnsAttr[0].stSnsI2C.u8DevAddr = 0x10;
			gstSnsAttr[0].stSnsI2C.u32RegAddrByte = 2;
			gstSnsAttr[0].stSnsI2C.u32RegDataByte = 1;
			gstSnsAttr[1].stSnsI2C.u8DevAddr = 0x30;
			gstSnsAttr[1].stSnsI2C.u32RegAddrByte = 2;
			gstSnsAttr[1].stSnsI2C.u32RegDataByte = 1;
			gstSnsAttr[0].s32SnsChipId = SENSOR_CHIP_SC202CS;
			gstSnsAttr[0].pfn_cmos_sensor_init = sensor_init_sc202cs;
			gstSnsAttr[0].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc202cs;
			gstSnsAttr[0].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc202cs;
			gstSnsAttr[0].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc202cs;
			cmos_init_chn_sc202cs(0);
			gstSnsAttr[1].s32SnsChipId = SENSOR_CHIP_SC1346;
			gstSnsAttr[1].pfn_cmos_sensor_init = sensor_init_sc1346;
			gstSnsAttr[1].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc1346;
			gstSnsAttr[1].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc1346;
			gstSnsAttr[1].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc1346;
			cmos_init_chn_sc1346(1);
			#endif
		}
		else
		{
			gstSnsAttr[0].stSnsI2C.u8DevAddr = 0x10;
			gstSnsAttr[0].stSnsI2C.u32RegAddrByte = 2;
			gstSnsAttr[0].stSnsI2C.u32RegDataByte = 1;
			gstSnsAttr[1].stSnsI2C.u8DevAddr = 0x30;
			gstSnsAttr[1].stSnsI2C.u32RegAddrByte = 2;
			gstSnsAttr[1].stSnsI2C.u32RegDataByte = 1;
			gstSnsAttr[0].s32SnsChipId = SENSOR_CHIP_SC202CS;
			gstSnsAttr[0].pfn_cmos_sensor_init = sensor_init_sc202cs;
			gstSnsAttr[0].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc202cs;
			gstSnsAttr[0].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc202cs;
			gstSnsAttr[0].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc202cs;
			cmos_init_chn_sc202cs(0);
			gstSnsAttr[1].s32SnsChipId = SENSOR_CHIP_SC1346;
			gstSnsAttr[1].pfn_cmos_sensor_init = sensor_init_sc1346;
			gstSnsAttr[1].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc1346;
			gstSnsAttr[1].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc1346;
			gstSnsAttr[1].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc1346;
			cmos_init_chn_sc1346(1);
		}
		
		#if 0
		// SmartSns
		gstSnsAttr[IspDev].stSnsI2C.u8DevAddr = IspDev?0x30:0x10;
		gstSnsAttr[IspDev].stSnsI2C.u32RegAddrByte = 2;
		gstSnsAttr[IspDev].stSnsI2C.u32RegDataByte = 1;
		sensor_write_register(IspDev,0x0103,0x01);
		sensor_write_register(IspDev,0x0103,0x01);// reset all registers
		XM_MPI_ISP_USleep(50000);
		s32Tmp = sensor_read_register(IspDev, 0x3107);
		s32Id = (s32Tmp<<8) | sensor_read_register(IspDev, 0x3108);
		switch(s32Id)
		{
			case 0xda4d:	DEBUG("SC1346! Chn:%d\n", IspDev);
						#ifdef SENS_X1X_SC1346
							gstSnsAttr[IspDev].s32SnsChipId = SENSOR_CHIP_SC1346;
							gstSnsAttr[IspDev].pfn_cmos_sensor_init = sensor_init_sc1346;
							gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc1346;
							gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc1346;
							gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc1346;
							cmos_init_chn_sc1346((ISP_DEV)IspDev);
							goto RET_TAB;
						#endif
							break;
			case 0xeb52:	DEBUG("SC202CS! Chn:%d\n", IspDev);
						#ifdef SENS_X20_SC202CS
							gstSnsAttr[IspDev].s32SnsChipId = SENSOR_CHIP_SC202CS;
							gstSnsAttr[IspDev].pfn_cmos_sensor_init = sensor_init_sc202cs;
							gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc202cs;
							gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc202cs;
							gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc202cs;
							cmos_init_chn_sc202cs(IspDev);
							goto RET_TAB;
						#endif
						break;
		}
		#endif
		goto RET_TAB;
	}
#endif

	gstSnsAttr[IspDev].stSnsI2C.u8DevAddr = 0x10;
	gstSnsAttr[IspDev].stSnsI2C.u32RegAddrByte = 2;
	gstSnsAttr[IspDev].stSnsI2C.u32RegDataByte = 1;
	sensor_write_register(IspDev,0x0103,0x01);
	sensor_write_register(IspDev,0x0103,0x01);// reset all registers
	XM_MPI_ISP_USleep(50000);
	s32Tmp = sensor_read_register(IspDev, 0x3107);
	s32Id = (s32Tmp<<8) | sensor_read_register(IspDev, 0x3108);
	DEBUG("s32Id = %d\n",s32Id);
	switch(s32Id)
	{
		case 0xeb52:	DEBUG("SC202CS! Chn:%d\n", IspDev);
					#ifdef SENS_X20_SC202CS
						gstSnsAttr[IspDev].s32SnsChipId = SENSOR_CHIP_SC202CS;
						gstSnsAttr[IspDev].pfn_cmos_sensor_init = sensor_init_sc202cs;
						gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc202cs;
						gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc202cs;
						gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc202cs;
						cmos_init_chn_sc202cs(IspDev);
						goto RET_TAB;
					#endif
						break;
	}
	gstSnsAttr[IspDev].stSnsI2C.u8DevAddr = 0x30;
	if((IspDev > 0) && (0 == u8I2cCh))
	{
		gstSnsAttr[IspDev].stSnsI2C.u8DevAddr = IspDev?0x32:0x30;
	}
	gstSnsAttr[IspDev].stSnsI2C.u32RegAddrByte = 2;
	gstSnsAttr[IspDev].stSnsI2C.u32RegDataByte = 1;
	sensor_write_register(IspDev,0x0103,0x01);
	sensor_write_register(IspDev,0x0103,0x01);// reset all registers
	XM_MPI_ISP_USleep(2);
	s32Tmp = sensor_read_register(IspDev, 0x3107);
	s32Id = (s32Tmp<<8) | sensor_read_register(IspDev, 0x3108);
	s32Tmp = sensor_read_register(IspDev, 0x3109);
	switch(s32Id)
	{
		case 0xce1f:	DEBUG("SC500AI! Chn:%d\n", IspDev);
					#ifdef SENS_X50_SC500AI
						gstSnsAttr[IspDev].s32SnsChipId = SENSOR_CHIP_SC500AI;
						gstSnsAttr[IspDev].pfn_cmos_sensor_init = sensor_init_sc500AI;
						gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc500AI;
						gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc500AI;
						gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc500AI;
						cmos_init_chn_sc500AI((ISP_DEV)IspDev);
						goto RET_TAB;
					#endif
						break;
		case 0xcd2e:	DEBUG("SC401AI! Chn:%d\n", IspDev);
					#ifdef SENS_X40_SC401AI
						gstSnsAttr[IspDev].s32SnsChipId = SENSOR_CHIP_SC401AI;
						gstSnsAttr[IspDev].pfn_cmos_sensor_init = sensor_init_sc401AI;
						gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc401AI;
						gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc401AI;
						gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc401AI;
						cmos_init_chn_sc401AI((ISP_DEV)IspDev);
						goto RET_TAB;
					#endif
						break;
		case 0xdc42:	DEBUG("SC4336! Chn:%d\n", IspDev);
					#ifdef SENS_X40_SC4336
						gstSnsAttr[IspDev].s32SnsChipId = SENSOR_CHIP_SC4336;
						gstSnsAttr[IspDev].pfn_cmos_sensor_init = sensor_init_sc4336;
						gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc4336;
						gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc4336;
						gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc4336;
						cmos_init_chn_sc4336((ISP_DEV)IspDev);
						goto RET_TAB;
					#endif
						break;
		case 0xcc1a:	DEBUG("SC3335! Chn:%d\n", IspDev);
					#ifdef SENS_X30_SC3335
						gstSnsAttr[IspDev].s32SnsChipId = SENSOR_CHIP_SC3335;
						gstSnsAttr[IspDev].pfn_cmos_sensor_init = sensor_init_sc3335;
						gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc3335;
						gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc3335;
						gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc3335;
						cmos_init_chn_sc3335((ISP_DEV)IspDev);
						goto RET_TAB;
					#endif
						break;
		case 0xcc41:	DEBUG("SC3338! Chn:%d\n", IspDev);
					#ifdef SENS_X30_SC3338
						gstSnsAttr[IspDev].s32SnsChipId = SENSOR_CHIP_SC3338;
						gstSnsAttr[IspDev].pfn_cmos_sensor_init = sensor_init_sc3338;
						gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc3338;
						gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc3338;
						gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc3338;
						cmos_init_chn_sc3338((ISP_DEV)IspDev);
						goto RET_TAB;
					#endif
						break;
		case 0xcb14:	DEBUG("SC2335! Chn:%d\n", IspDev);
					#ifdef SENS_X20_SC2335
						gstSnsAttr[IspDev].s32SnsChipId = SENSOR_CHIP_SC2335;
						gstSnsAttr[IspDev].pfn_cmos_sensor_init = sensor_init_sc2335;
						gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc2335;
						gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc2335;
						gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc2335;
						cmos_init_chn_sc2335((ISP_DEV)IspDev);
						goto RET_TAB;
					#endif
						break;
		case 0xcb3a:	DEBUG("SC2336! Chn:%d\n", IspDev);
					#ifdef SENS_X20_SC2336
						gstSnsAttr[IspDev].s32SnsChipId = SENSOR_CHIP_SC2336;
						gstSnsAttr[IspDev].pfn_cmos_sensor_init = sensor_init_sc2336;
						gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc2336;
						gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc2336;
						gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc2336;
						cmos_init_chn_sc2336((ISP_DEV)IspDev);
						goto RET_TAB;
					#endif
						break;

		case 0xcb3e:	DEBUG("SC223A! Chn:%d\n", IspDev);
					#ifdef SENS_X20_SC223A
						gstSnsAttr[IspDev].s32SnsChipId = SENSOR_CHIP_SC223A;
						gstSnsAttr[IspDev].pfn_cmos_sensor_init = sensor_init_sc223A;
						gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc223A;
						gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc223A;
						gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc223A;
						cmos_init_chn_sc223A((ISP_DEV)IspDev);
						goto RET_TAB;
					#endif
						break;
		case 0xcb1c:	DEBUG("SC200AI! Chn:%d\n", IspDev);
					#ifdef SENS_X20_SC200AI
						gstSnsAttr[IspDev].s32SnsChipId = SENSOR_CHIP_SC200AI;
						gstSnsAttr[IspDev].pfn_cmos_sensor_init = sensor_init_sc200AI;
						gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc200AI;
						gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc200AI;
						gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc200AI;
						cmos_init_chn_sc200AI((ISP_DEV)IspDev);
						goto RET_TAB;
					#endif
						break;
		case 0x2238:	DEBUG("SC307E! Chn:%d\n", IspDev);
					#ifdef SENS_X20_SC307E
						gstSnsAttr[IspDev].s32SnsChipId = SENSOR_CHIP_SC307E;
						gstSnsAttr[IspDev].pfn_cmos_sensor_init = sensor_init_sc307E;
						gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc307E;
						gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc307E;
						gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc307E;
						cmos_init_chn_sc307E((ISP_DEV)IspDev);
						goto RET_TAB;
					#endif
						break;
		case 0x0031:	DEBUG("SC035GS! Chn:%d\n", IspDev);
					#ifdef SENS_X1X_SC035GS
						gstSnsAttr[IspDev].s32SnsChipId = SENSOR_CHIP_SC035GS;
						gstSnsAttr[IspDev].pfn_cmos_sensor_init = sensor_init_sc035GS;
						gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_sc035GS;
						gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_sc035GS;
						gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_sc035GS;
						cmos_init_chn_sc035GS((ISP_DEV)IspDev);
						goto RET_TAB;
					#endif
						break;

	}
	// Sony(IMX307/IMX327)
	gstSnsAttr[IspDev].stSnsI2C.u8DevAddr = 0x1A;
	gstSnsAttr[IspDev].stSnsI2C.u32RegAddrByte = 2;
	gstSnsAttr[IspDev].stSnsI2C.u32RegDataByte = 1;
	sensor_write_register(IspDev,0x3003, 0x01);
	XM_MPI_ISP_USleep(50000);
	s32Id = sensor_read_register(IspDev, 0x3008);
	s32Id = ((s32Id<<8)|(sensor_read_register(IspDev, 0x301E)));
	if(s32Id == 0xA0B2)
	{
		s32Id = sensor_read_register(IspDev, 0x31dc);
		s32Tmp = (s32Id&0x06);
		switch(s32Tmp)
		{
			// IMX307
			case 0x04:	DEBUG("IMX307! Chn:%d\n", IspDev);
						#ifdef SENS_X20_IMX307
						gstSnsAttr[IspDev].s32SnsChipId = SENSOR_CHIP_IMX307;
						gstSnsAttr[IspDev].pfn_cmos_sensor_init = sensor_init_imx307;
						gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_imx307;
						gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_imx307;
						gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_imx307;
						cmos_init_chn_imx307((ISP_DEV)IspDev);
						goto RET_TAB;
						#endif
						break;
		}
	}
	//IMX335
	gstSnsAttr[IspDev].stSnsI2C.u8DevAddr = 0x1A;
	gstSnsAttr[IspDev].stSnsI2C.u32RegAddrByte = 2;
	gstSnsAttr[IspDev].stSnsI2C.u32RegDataByte = 1;
	s32Id = sensor_read_register(IspDev, 0x3302);
	switch(s32Id)
	{
		case 0x32:	DEBUG("IMX335! Chn:%d\n", IspDev);
					#ifdef SENS_X50_IMX335
					gstSnsAttr[IspDev].s32SnsChipId = SENSOR_CHIP_IMX335;
					gstSnsAttr[IspDev].pfn_cmos_sensor_init = sensor_init_imx335;
					gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function = cmos_init_sensor_exp_function_imx335;
					gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function = cmos_init_ae_exp_function_imx335;
					gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function = cmos_init_awb_exp_function_imx335;
					cmos_init_chn_imx335((ISP_DEV)IspDev);
					goto RET_TAB;
					#endif
					break;
	}

	// Unknow
	gstSnsAttr[IspDev].s32SnsChipId = SENSOR_CHIP_UNKNOW;
	gstSnsAttr[IspDev].pfn_cmos_sensor_init = NULL;
	gstSnsAttr[IspDev].pfn_cmos_init_sensor_exp_function = NULL;
	gstSnsAttr[IspDev].pfn_cmos_init_ae_exp_function = NULL;
	gstSnsAttr[IspDev].pfn_cmos_init_awb_exp_function = NULL;
	ERR("Sensor:unknow! Chn:%d\n", IspDev);	
RET_TAB:	
	if(pstI2CData != NULL)
	{
		pstI2CData->dev_addr = gstSnsAttr[IspDev].stSnsI2C.u8DevAddr;
		pstI2CData->addr_byte_num = gstSnsAttr[IspDev].stSnsI2C.u32RegAddrByte;
		pstI2CData->data_byte_num = gstSnsAttr[IspDev].stSnsI2C.u32RegDataByte;
	}
	return gstSnsAttr[IspDev].s32SnsChipId;
}

void  sensor_set_chip(ISP_DEV IspDev,ISP_S32 *s32SnsChip)
{
	return;
}

int  sensor_get_temp(ISP_S16 *ps16Temp)
{
	if(ps16Temp == NULL)
	{
		return ISP_FAILURE;
	}
	*ps16Temp = 40*128;
	return ISP_SUCCESS;
}

ISP_S32 cmos_set_sns_attr(ISP_DEV IspDev, ISP_CMOS_SNS_ATTR_S *pstSnsAttr)
{
	if((pstSnsAttr == NULL) || (IspDev>=ISP_NUM_MAX))
	{
		ERR("cmos_set_sns_attr failed!\n");
		return ISP_FAILURE;
	}
	gstSnsAttr[IspDev].enCmosIF = pstSnsAttr->u8InputMode;
	gstSnsAttr[IspDev].enHdrMode = pstSnsAttr->u8HdrMode;
	gstSnsAttr[IspDev].u8FunExt = pstSnsAttr->u8FunExt;
	gstSnsAttr[IspDev].enRsltType = pstSnsAttr->enRsltType;
	gstSnsAttr[IspDev].u8MipiAttr = pstSnsAttr->u8MipiAttr;
	return ISP_SUCCESS;
}

ISP_S32 cmos_get_sns_attr(ISP_DEV IspDev, ISP_CMOS_SNS_ATTR_S *pstSnsAttr)
{
	if((pstSnsAttr == NULL) || (IspDev>=ISP_NUM_MAX))
	{
		ERR("cmos_get_sns_attr failed!\n");
		return ISP_FAILURE;
	}
	pstSnsAttr->u8InputMode = gstSnsAttr[IspDev].enCmosIF;
	return ISP_SUCCESS;
}

ISP_VOID cmos_set_wdr_mode(WDR_MODE_E enWdrMode)
{
	genWdrMode = enWdrMode;
	return;
}

/*************************************************************************************
输入参数: 	u8Mode(0:Stop  1:Restart)
*************************************************************************************/
void sensor_restart(ISP_U8 u8Mode)
{
	return;
}

/*************************************************************************************
输入参数: 	u8Mode(0:Stop  1:Restart)
*************************************************************************************/
void sensor_restart_double(ISP_DEV IspDev, ISP_U8 u8Mode)
{
	if(u8Mode == 0)
	{
		switch(gstSnsAttr[IspDev].s32SnsChipId)
		{
			case SENSOR_CHIP_SC202CS:
				sensor_write_register(IspDev,0x100, 0x00);
				break;
			case SENSOR_CHIP_SC1346:
				sensor_write_register(IspDev,0x100, 0x00);
				break;
		}
	}
	else
	{
		switch(gstSnsAttr[IspDev].s32SnsChipId)
		{
			case SENSOR_CHIP_SC202CS:
				sensor_write_register(IspDev,0x100, 0x01);
				break;
			case SENSOR_CHIP_SC1346:
				sensor_write_register(IspDev,0x100, 0x01);
				break;
		}
	}
	return;
}

/*************************************************************************************
函数功能: 获取AGC相关的参数(彩色/黑白需要区分)
输入参数: IspDev: 指定ISP设备号
输出参数: pstAgcPara(各参数地址)
返回参数: 		-1:失败
			0: 成功
*************************************************************************************/
ISP_S32 sensor_get_agc_para(ISP_DEV IspDev, ISP_CMOS_DN_PARA_S *pstDnPara)
{
	if(pstDnPara==NULL)
	{
		return ISP_FAILURE;
	}
	XM_MPI_ISP_Memncpy((ISP_U8 *)pstDnPara, (ISP_U8 *)&(gstCmosDnPara[IspDev]), sizeof(ISP_CMOS_DN_PARA_S));
	return ISP_SUCCESS;
}


