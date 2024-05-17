#include "isp_type.h"
#include "isp_comm_isp.h"
#include "isp_comm_sns.h"
#include "isp_sns_ctrl.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "isp_awb_comm.h"
#include "XAx_cmos.h"
#include "isp_print.h"



#define HD5MP_15P_LINES_imx335		(2475)

#define HD5MP_25P_LINES_imx335		(1500)
extern void SysDelay_ms(ISP_S32 ms);



static const ISP_CMOS_AGC_TABLE_S g_stIspAgcTable_IMX335 = {
	/* bvalid */
	1,
	/* 100, 200, 400, 800, 1600, 3200, 6400, 12800; 100, 200, 400, 800, 1600, 3200, 6400, 12800 */
	/* snr_thresh 2DNr		*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/* drc	*/
	{0x32,0x28,0x1c,0x1a,0x16,0x10,0x0c,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08},
	/* defog	*/
	{0x28,0x28,0x20,0x1a,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18},
	/* defog_vpss	*/
	{0x30,0x30,0x30,0x25,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20}, 
	/* DyDpc_thresh 		*/
	{0xB8,0xC0,0xD0,0xD4,0xD8,0xDC,0xE0,0xE0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0},   
	/* saturation_strength */
	{0xA8,0xA6,0xA4,0x9c,0x90,0x88,0x74,0x3C,0x3C,0x30,0x30,0x30,0x30,0x30,0x30,0x30},
	/* Blc	*/
	{0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xB9,0xB9,0xB9,0xB9,0xB9,0xB9,0xB9},
	/* Contrast*/ 
	{0x32,0x32,0x32,0x32,0x35,0x35,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x2E,0x2E,0x2E}
};

const ISP_CMOS_VPSS_3DNR_S gstVpss3DNr_imx335 = {
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
	{0x80,0x80,0x80,0x74,0x70,0x6c,0x60,0x46,0x46,0x46,0x46,0x46,0x46,0x46,0x46,0x46},
	/* Alpha_Min    */
	{0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07},
	/* IdxThr_Max    */
	{0x22,0x22,0x22,0x22,0x1c,0x1c,0x1c,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d},
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

static ISP_S32 cmos_get_isp_default(ISP_DEV IspDev, ISP_CMOS_DEFAULT_S *pstDef)
{
	ISP_U8 u8ColorBw = 0;	// 0:Color	1:Bw
	if (ISP_NULL == pstDef)
	{
		ERR("null pointer when get isp default value!\n");
		return -1;
	}
	gstCmosDnPara[IspDev].pastAgcTab[0] = &g_stIspAgcTable_IMX335;
	gstCmosDnPara[IspDev].pastAgcYcNr[0] = &gstYCNr;
	gstCmosDnPara[IspDev].pastAgcSharp[0] = &gstSharpenColor;
	gstCmosDnPara[IspDev].pastAgcSharp[1] = &gstSharpenBw;
	gstCmosDnPara[IspDev].pastAgcVpss3DNr[0] = &gstVpss3DNr_imx335;
	gstCmosDnPara[IspDev].pastGamma[0] = &gastIspGamma[1];
	gstCmosDnPara[IspDev].pastGamma[1] = &gastIspGamma[0];
	gstCmosDnPara[IspDev].pastAgcLtm[0] = &gstLtm;
	gstCmosDnPara[IspDev].past2DNr[0] = &gst2DNr;
	if(gstCmosDnPara[IspDev].pastAgcTab[1] == NULL)
	{
		gstCmosDnPara[IspDev].pastAgcTab[1] = gstCmosDnPara[IspDev].pastAgcTab[0];
	}	
	if(gstCmosDnPara[IspDev].pastAgcYcNr[1] == NULL)
	{
		gstCmosDnPara[IspDev].pastAgcYcNr[1] = gstCmosDnPara[IspDev].pastAgcYcNr[0];
	}	
	if(gstCmosDnPara[IspDev].pastAgcSharp[1] == NULL)
	{
		gstCmosDnPara[IspDev].pastAgcSharp[1] = gstCmosDnPara[IspDev].pastAgcSharp[0];
	}
	if(gstCmosDnPara[IspDev].pastAgcVpss3DNr[1] == NULL)
	{
		gstCmosDnPara[IspDev].pastAgcVpss3DNr[1] = gstCmosDnPara[IspDev].pastAgcVpss3DNr[0];
	}
	if(gstCmosDnPara[IspDev].pastGamma[1] == NULL)
	{
		gstCmosDnPara[IspDev].pastGamma[1] = gstCmosDnPara[IspDev].pastGamma[0];
	}
	if(gstCmosDnPara[IspDev].pastAgcLtm[1] == NULL)
	{
		gstCmosDnPara[IspDev].pastAgcLtm[1] = gstCmosDnPara[IspDev].pastAgcLtm[0];
	}
	if(gstCmosDnPara[IspDev].past2DNr[1] == NULL)
	{
		gstCmosDnPara[IspDev].past2DNr[1] = gstCmosDnPara[IspDev].past2DNr[0];
	}
	pstDef->stGamma.bValid = ISP_TRUE;
	XM_MPI_ISP_Memncpy((ISP_U8*)&pstDef->stGamma, (ISP_U8*)(gstCmosDnPara[IspDev].pastGamma[u8ColorBw]), sizeof(ISP_CMOS_GAMMA_S));
	XM_MPI_ISP_Memncpy((ISP_U8*)&pstDef->stAgcTbl, (ISP_U8*)(gstCmosDnPara[IspDev].pastAgcTab[u8ColorBw]), sizeof(ISP_CMOS_AGC_TABLE_S));
	XM_MPI_ISP_Memncpy((ISP_U8*)&pstDef->stVpss3DNr, (ISP_U8*)(gstCmosDnPara[IspDev].pastAgcVpss3DNr[u8ColorBw]), sizeof(ISP_CMOS_VPSS_3DNR_S));
	XM_MPI_ISP_Memncpy((ISP_U8*)&pstDef->stSharpen, (ISP_U8*)(gstCmosDnPara[IspDev].pastAgcSharp[u8ColorBw]), sizeof(ISP_CMOS_SHARPEN_S));
	XM_MPI_ISP_Memncpy((ISP_U8*)&pstDef->stYcNr, (ISP_U8*)(gstCmosDnPara[IspDev].pastAgcYcNr[u8ColorBw]), sizeof(ISP_CMOS_YCNR_S));
	XM_MPI_ISP_Memncpy((ISP_U8*)&pstDef->stLtm, (ISP_U8*)(gstCmosDnPara[IspDev].pastAgcLtm[u8ColorBw]), sizeof(ISP_CMOS_LTM_S));
	XM_MPI_ISP_Memncpy((ISP_U8*)&pstDef->st2DNr, (ISP_U8*)(gstCmosDnPara[IspDev].past2DNr[u8ColorBw]), sizeof(ISP_CMOS_2DNR_S));
	XM_MPI_ISP_Memncpy((ISP_U8*)&pstDef->stShading, (ISP_U8*)&gstShading, sizeof(ISP_CMOS_SHADING_S));
	XM_MPI_ISP_Memncpy((ISP_U8*)&pstDef->stLocalCAC, (ISP_U8*)&gstLocalCAC, sizeof(ISP_CMOS_LOCAL_CAC_S));
	return 0;
}

static ISP_VOID cmos_set_pixel_detect(ISP_DEV IspDev, ISP_BOOL bEnable)
{
    return;
}

static ISP_S32 cmos_set_mirror_flip(ISP_DEV IspDev, ISP_U8 u8Mirror,ISP_U8 u8Flip)
{

    if(u8Mirror)
    {
        sensor_write_register(IspDev,0x304E, 0x01);
    }
    else
    {
        sensor_write_register(IspDev,0x304E, 0x00);
    }
    if(u8Flip)
    {
        sensor_write_register(IspDev,0x304F, 0x01);
        sensor_write_register(IspDev,0x3081, 0xFE);
        sensor_write_register(IspDev,0x3083, 0xFE);
        sensor_write_register(IspDev,0x30B6, 0xFA);
        sensor_write_register(IspDev,0x30B7, 0x01);
        sensor_write_register(IspDev,0x3116, 0x02);
    }
    else
    {
        sensor_write_register(IspDev,0x304F, 0x00);
        sensor_write_register(IspDev,0x3081, 0x02);
        sensor_write_register(IspDev,0x3083, 0x02);
        sensor_write_register(IspDev,0x30B6, 0x00);
        sensor_write_register(IspDev,0x30B7, 0x00);
        sensor_write_register(IspDev,0x3116, 0x08);
    }
	//XM_MPI_ISPAPP_WinSet(IspDev, &stWinAttr);
    return ISP_SUCCESS;
 }

static const ISP_U32 gau32GainTbl[141] =
{1024,	1059,  1097,  1135, 1175, 1217, 1259, 1304, 1349, 1397, 1446, 1497, 1549, 1604,
 1660,	1719,  1779,  1842, 1906, 1973, 2043, 2048, 2119, 2194, 2271, 2351, 2434, 2519, 
 2608,	2699,  2794,  2892, 2994, 3099, 3208, 3321, 3438, 3559, 3684, 3813, 3947, 4086, 
 4229,	4378,  4532,  4691, 4856, 5027, 5203, 5386, 5576, 5772, 5974, 6184, 6402, 6627, 
 6860,	7101,  7350,  7609, 7876, 8153, 8439, 8736, 9043, 9361, 9690, 10030,10383,10748,
 11125, 11516, 11921, 12340,12774,13222,13687,14168,14666,15182,15715,16267,16839,17431,
 18043, 18677, 19334, 20013,20717,21445,22198,22978,23786,24622,25487,26383,27310,28270,
 29263, 30292, 31356, 32458,33599,34780,36002,37267,38577,39932,41336,42788,44292,45849,
 47460, 49128, 50854, 52641,54491,56406,58388,60440,62564,64763,67039,69395,71833,74358,
 76971, 79676, 82476, 85374,88375,91480,94695,98023,101468,105034,108725,112545,116501,
 120595,124833};

static ISP_VOID cmos_again_calc_table(ISP_DEV IspDev, ISP_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
	int i;
	if(ISP_NULL == pstAeSnsGainInfo)
	{
		ERR("null pointer when get ae sensor gain info	value!\n");
		return;
	}
	pstAeSnsGainInfo->u32GainDb = 0;
	pstAeSnsGainInfo->u32SnsTimes = 1024;

	if (u32InTimes >= gau32GainTbl[140])	   
	{			 
		pstAeSnsGainInfo->u32GainDb = 140;
	}
	else
	{
		for(i = 1; i <= 140; i++)		 
		{			
			if(u32InTimes < gau32GainTbl[i])			
			{  
				pstAeSnsGainInfo->u32GainDb = i-1;
				break;			  
			}			 
		}	 
	}
	pstAeSnsGainInfo->u32SnsTimes = gau32GainTbl[pstAeSnsGainInfo->u32GainDb];
	return;
}

static ISP_VOID cmos_dgain_calc_table(ISP_DEV IspDev, ISP_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
	if(ISP_NULL == pstAeSnsGainInfo)
	{
		ERR("cmos_dgain_calc_table failed!\n");
		return;
	}
	pstAeSnsGainInfo->u32SnsTimes = u32InTimes;
	pstAeSnsGainInfo->u32GainDb = 0;
	return;
}

static ISP_S32 cmos_get_ae_default(ISP_DEV IspDev, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	if (ISP_NULL == pstAeSnsDft)
	{
		ERR("null pointer when get ae default value!\n");
		return -1;
	}
	pstAeSnsDft->u32FullLinesStd = HD5MP_15P_LINES_imx335;
	pstAeSnsDft->u8AeCompensation = 0x39;	
	pstAeSnsDft->u32LinesPer500ms = pstAeSnsDft->u32FullLinesStd*25/2;
	pstAeSnsDft->u32FlickerFreq = 0;

	pstAeSnsDft->u32MaxIntTime = pstAeSnsDft->u32FullLinesStd-9;
	pstAeSnsDft->u32MinIntTime = 1;    

	pstAeSnsDft->u32MaxAgain = 15782;
	pstAeSnsDft->u32MinAgain = 1024;

	pstAeSnsDft->u32MaxDgain = 1024;  
	pstAeSnsDft->u32MinDgain = 1024;

	pstAeSnsDft->u32ISPDgainShift = 8;
	pstAeSnsDft->u32MaxISPDgain= 8 << pstAeSnsDft->u32ISPDgainShift;
	pstAeSnsDft->u32MinISPDgain= 1 << pstAeSnsDft->u32ISPDgainShift;

	pstAeSnsDft->bRsv = ISP_FALSE;

	return 0;
}

static ISP_S32 cmos_get_sensor_max_resolution(ISP_DEV IspDev, ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution)
{
	if (ISP_NULL == pstSensorMaxResolution)
	{
	    ERR("null pointer when get sensor max resolution \n");
	    return -1;
	}

	pstSensorMaxResolution->u32MaxWidth  = 2592;
	pstSensorMaxResolution->u32MaxHeight = 1944;
	return 0;
}
static ISP_VOID cmos_inttime_update(ISP_DEV IspDev, ISP_U32 u32IntTime)
{
	static ISP_U32 su32IntTime = 0xFFFFFFFF;
	ISP_U32 u32Tmp;	
	if(su32IntTime == u32IntTime)
		return ;
	su32IntTime = u32IntTime;
	gau32ShutNow[IspDev] = u32IntTime;
	u32IntTime = gau16FullLines[IspDev] - u32IntTime;
	su32IntTime = u32IntTime;
	u32Tmp = u32IntTime*4;
	sensor_write_register(IspDev,0x3058, u32Tmp&0xFF);
	sensor_write_register(IspDev,0x3059, (u32Tmp>>8)&0xFF);
	sensor_write_register(IspDev,0x305a, (u32Tmp>>16)&0x0F);

	return;
}

static ISP_VOID cmos_gains_update(ISP_DEV IspDev, ISP_U32 u32Again, ISP_U32 u32Dgain)
{
	 static ISP_U32 Preu32Again = 0xffff;
    ISP_U32 u32Tmp;	
    
    if(Preu32Again == u32Again)
        return;
    Preu32Again = u32Again;
    u32Tmp = u32Again + u32Dgain;   

	sensor_write_register(IspDev,0x30E8, u32Tmp & 0xFF);
	sensor_write_register(IspDev,0x30E9, (u32Tmp>>8)&0x07);
	return;
}


static ISP_VOID cmos_slow_framerate_set(ISP_DEV IspDev, ISP_U16 u16FullLines, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
#if 0
    static XM_U16 preU16FullLine = 0xffff;
    XM_U32 u32Temp;
    if(preU16FullLine == u16FullLines)
        return;

    preU16FullLine = u16FullLines;
    u16FullLines = (u16FullLines>=4096)?4000:u16FullLines;
    gu16FullLines = u16FullLines;
    pstAeSnsDft->u32MaxIntTime = u16FullLines - 9;
    SysDelay_ms(30);
    if(gu32ShutNow>pstAeSnsDft->u32MaxIntTime)
    {
        cmos_inttime_update(pstAeSnsDft->u32MaxIntTime);
    }
    u32Temp = u16FullLines*4;
    sensor_write_register(0x3030, u32Temp&0xFF);
	sensor_write_register(0x3031, (u32Temp>>8)&0xFF);
	sensor_write_register(0x3032, (u32Temp>>16)&0x0F);
       
	XM_MPI_MIPI_RefreshFV(0, (XM_U32)(u16FullLines*2));
#else
	static ISP_U32 preU32FullPixel = 0xffff;
	ISP_U32 u32Temp,u32Pixel,u32AmpCoef;
	if(!gau16FullLines[IspDev])	return ;
	u32AmpCoef = 16*u16FullLines/gau16FullLines[IspDev];
	u32Pixel = 4000*u32AmpCoef/16;
	if(preU32FullPixel == u32Pixel)
		return;
	preU32FullPixel = u32Pixel;
	SysDelay_ms(30);
	u32Temp = u32Pixel>>3;
	sensor_write_register(IspDev,0x3034, u32Temp&0xFF);
	sensor_write_register(IspDev,0x3035, (u32Temp>>8)&0xFF);
	XM_MPI_MIPI_RefreshFV(IspDev,0, u32Pixel|(1<<31));
	XM_MPI_AE_SetAmpSlowFrameByFW(IspDev,u32AmpCoef);
#endif
}

static ISP_VOID cmos_fps_set(ISP_DEV IspDev, ISP_U8 u8Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_U32 u32TotalSizeV,u32TotalSizeH;
    ISP_U32 u32Temp;
	u32TotalSizeV = HD5MP_15P_LINES_imx335;
	switch(u8Fps)
	{				
		case 20:	u8Fps = 20;
					u32TotalSizeV = HD5MP_15P_LINES_imx335*15/u8Fps;
					u32TotalSizeH = 4000;
					break;
		case 25:	u8Fps = 25;
					u32TotalSizeV = HD5MP_25P_LINES_imx335;
					u32TotalSizeH = 3960;
					break;
		case 15:
		default:	u8Fps = 15;
					u32TotalSizeV = HD5MP_15P_LINES_imx335;
					u32TotalSizeH = 4000;
					break;

	}
	DEBUG("cmos_fps_set %dfps\n", u8Fps);
	// Change the frame rate via changing the vertical blanking
	if(pstAeSnsDft != NULL)
	{
		pstAeSnsDft->u32FullLinesStd = u32TotalSizeV;
		pstAeSnsDft->u32MaxIntTime = u32TotalSizeV-9;
		pstAeSnsDft->u32LinesPer500ms = pstAeSnsDft->u32FullLinesStd * u8Fps / 2;
        gau16FullLines[IspDev] = u32TotalSizeV;
	}
	u32Temp = u32TotalSizeV<<2;
	sensor_write_register(IspDev,0x3030, u32Temp&0xFF);
	sensor_write_register(IspDev,0x3031, (u32Temp>>8)&0xFF);
	sensor_write_register(IspDev,0x3032, (u32Temp>>16)&0x0F);
	u32Temp = u32TotalSizeH>>3;
	sensor_write_register(IspDev,0x3034, u32Temp&0xFF);
	sensor_write_register(IspDev,0x3035, (u32Temp>>8)&0xFF);
	XM_MPI_MIPI_RefreshFV(IspDev,0, u32TotalSizeV*2);
	return;
}

static const ISP_COLORMATRIX_AUTO_S g_stAwbCcm =
{
    5000,//0.25
    {
        0x000,329, -78, 5,
        0x000,-43, 356, -57,
        0x000,39, -77, 294
    },
    4000,//0.23
    {
        0x000,333, -90,13,
        0x000,-58, 350,-36,
        0x000, 43,-89,302
    },
    2800,//0.2
    {
        0x000, 269, -2,-11,
        0x000,-73, 363,-34,
        0x000, 31,-142,367
    }
};

static const ISP_AWB_CALIBRATION_V2_S gstAwbCal = {
	{0, 0, 4096, 3396, 4096, 2899, 1545, 4096},
	{4096, 4096, 0, 0, 2811, 3034, 4096, 1013},
	{-585, -3603, -1457, -4096, -3052, -4096, -2164, -2237},
	213,
	0, 1254, 0, 2367,
	{0, 416, 647, 983, 1085, 1199, 1462, 1491, 2367, 0, 0, 0, 0, 0, 0, 0},
	{2000, 2150, 2800, 4000, 4150, 5000, 6500, 7500, 12000, 0, 0, 0, 0, 0, 0, 0},
 	{1824, 1024, 2017, 0}
};
static ISP_S32 cmos_get_awb_default(ISP_DEV IspDev, AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
	if (ISP_NULL == pstAwbSnsDft)
	{
		ERR("null pointer when get awb default value!\n");
		return ISP_FAILURE;
	}
	XM_MPI_ISP_Memset((ISP_U8*)pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));
	pstAwbSnsDft->pstAwbCal = &gstAwbCal;
	// CCM
	XM_MPI_ISP_Memncpy((ISP_U8*)&(pstAwbSnsDft->stCcm), (ISP_U8*)&g_stAwbCcm, sizeof(ISP_COLORMATRIX_AUTO_S));
	return ISP_SUCCESS;
}


 ISP_S32 cmos_init_chn_imx335(ISP_DEV IspDev)
{
	return 0;
}
 
ISP_S32 cmos_init_sensor_exp_function_imx335(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
	XM_MPI_ISP_Memset((ISP_U8 *)pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));
	pstSensorExpFunc->pfn_cmos_sensor_init = sensor_init;
	pstSensorExpFunc->pfn_cmos_get_isp_default = cmos_get_isp_default;
	pstSensorExpFunc->pfn_cmos_set_pixel_detect = cmos_set_pixel_detect;
	pstSensorExpFunc->pfn_cmos_get_sensor_max_resolution = cmos_get_sensor_max_resolution;
	pstSensorExpFunc->pfn_cmos_set_mirror_flip = cmos_set_mirror_flip;
	return 0;
}

ISP_S32 cmos_init_ae_exp_function_imx335(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	XM_MPI_ISP_Memset((ISP_U8 *)pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));
	pstExpFuncs->pfn_cmos_fps_get = NULL;
	pstExpFuncs->pfn_cmos_get_ae_default    = cmos_get_ae_default;
	pstExpFuncs->pfn_cmos_fps_set           = cmos_fps_set;
	pstExpFuncs->pfn_cmos_slow_framerate_set= cmos_slow_framerate_set;    
	pstExpFuncs->pfn_cmos_inttime_update    = cmos_inttime_update;
	pstExpFuncs->pfn_cmos_gains_update      = cmos_gains_update;
	pstExpFuncs->pfn_cmos_again_calc_table  = cmos_again_calc_table;
	pstExpFuncs->pfn_cmos_dgain_calc_table  = cmos_dgain_calc_table;
	return 0;
}

ISP_S32 cmos_init_awb_exp_function_imx335(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	XM_MPI_ISP_Memset((ISP_U8*)pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));
	pstExpFuncs->pfn_cmos_get_awb_default = cmos_get_awb_default;
	return 0;
}
