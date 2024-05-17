#include "isp_type.h"
#include "isp_comm_isp.h"
#include "isp_comm_sns.h"
#include "isp_sns_ctrl.h"
#include "isp_math.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "isp_awb_comm.h"
#include "isp_print.h"
#include "XAx_cmos.h"




extern ISP_SNS_REGS_INFO_S g_stSnsRegsInfo;
extern ISP_I2C_DATA_S gstIspI2CData[8];
extern ISP_U32 gau32PreI2CData[8];

static const ISP_CMOS_AGC_TABLE_S g_stIspAgcTable_imx307 =
{
	/* bvalid */
	1,
	/* 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800 */
	/* snr_thresh 2DNr		*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/* drc	*/
	{0x32,0x28,0x1c,0x1a,0x16,0x10,0x0c,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08},
	/* defog	*/
	{0x28,0x28,0x20,0x1a,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18},
	/* defog_vpss	*/
	{0x30,0x30,0x30,0x25,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20}, 
	/* DyDpc_thresh 		*/
	{0x90,0xA8,0xD0,0xD4,0xD8,0xE0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xE0},
	/* saturation_strength */
	{0x6E,0x6E,0x6E,0x6E,0x6E,0x6E,0x64,0x5A,0x5A,0x5A,0x30,0x30,0x30,0x30,0x30,0x30},
	/* Blc	*/
	{0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0},
	/* Constract*/ 
	{0x36,0x36,0x36,0x36,0x36,0x35,0x33,0x33,0x33,0x33,0x33,0x28,0x28,0x28,0x28,0x28},
};

static ISP_S32 cmos_get_isp_default(ISP_DEV IspDev, ISP_CMOS_DEFAULT_S *pstDef)
{
	ISP_U8 u8ColorBw = 0;	// 0:Color  1:Bw

	if (ISP_NULL == pstDef)
	{
		ERR("null pointer when get isp default value!\n");
		return -1;
	}
	gstCmosDnPara[IspDev].pastAgcTab[0] = &g_stIspAgcTable_imx307;
	gstCmosDnPara[IspDev].pastAgcYcNr[0] = &gstYCNr;
	gstCmosDnPara[IspDev].pastAgcSharp[0] = &gstSharpenColor;
	gstCmosDnPara[IspDev].pastAgcSharp[1] = &gstSharpenBw;
	gstCmosDnPara[IspDev].pastAgcVpss3DNr[0] = &gstVpss3DNr;
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
	return 0;
	ISP_U32 u32Tmp;
	ISP_U16 u16OfstH,u16OfstV;
	ISPAPP_WIN_ATTR_S stWinAttr;
	
	u32Tmp = sensor_read_register(IspDev, 0x3007);
	u32Tmp &= 0xF0;
	if(u8Mirror)
	{
		u32Tmp |= 0x02;
		u16OfstH = 0;
	}
	else
	{
		u32Tmp &= ~0x02;
		u16OfstH = 0;
	}
		
	if(u8Flip)
	{
		u32Tmp|= 0x01;
		u16OfstV = 1;
	}
	else
	{
		u32Tmp &= ~0x01;
		u16OfstV = 0;
	}
	sensor_write_register(IspDev, 0x3007, u32Tmp);

	stWinAttr.u8Mode = 1;
	stWinAttr.u16ValH = u16OfstH;
	stWinAttr.u16ValV = u16OfstV;
	XM_MPI_ISPAPP_WinSet(IspDev, &stWinAttr);

	return ISP_SUCCESS;
}

static const ISP_U16 gau16ShutCal[35][4] = {
 //Exp Shut Gain GainIdx
 {1623,2,1024,0},
 {1663,2,1059,1},
 {1706,2,1097,2},
 {1773,2,1135,3},
 {1822,2,1175,4},
 {1861,2,1217,5},
 {1925,2,1259,6},
 {1985,2,1304,7},
 {2042,2,1349,8},
 {2089,2,1397,9},
 {2169,2,1446,10},
 {2246,2,1497,11},
 {2302,2,1549,12},
 {2351,2,1604,13},
 {2407,2,1660,14},
 {2483,2,1719,15},
 {2578,2,1779,16},
 {2671,2,1842,17},
 {2717,2,1906,18},
 {2784,2,1973,19},
 {2929,2,2043,20},
 {2945,3,1024,0},
 {3044,3,1059,1},
 {3100,3,1097,2},
 {3130,3,1135,3},
 {3208,3,1175,4},
 {3398,3,1217,5},
 {3501,3,1259,6},
 {3498,3,1304,7},
 {3655,3,1349,8},
 {3796,3,1397,9},
 {3908,3,1446,10},
 {3976,3,1497,11},
 {4056,3,1549,12},
 {4096,4,1024,0},
};
static ISP_U8 su8Idx = 0xFF;
static ISP_VOID cmos_shut_calc_table(ISP_DEV IspDev, ISP_S32 *ps32IntTime,AE_SENSOR_SHUTINFO_S *pstAeSnsShutInfo)
{
	int i,s32Idx = 0;
	if(*ps32IntTime < 4)
	{
		if(pstAeSnsShutInfo->u64Exp <= gau16ShutCal[0][0])
		{
			s32Idx = 0;
		}
		else if(pstAeSnsShutInfo->u64Exp >= gau16ShutCal[34][0])		
		{			 
			s32Idx = 34;
		}
		else
		{
			for(i = 1; i <= 34; i++)		 
			{			
				if(pstAeSnsShutInfo->u64Exp < gau16ShutCal[i][0]) 		   
				{  
					s32Idx = i-1;
					break;			  
				}			 
			}	 
		}
		
		pstAeSnsShutInfo->u32ShutDb = gau16ShutCal[s32Idx][1];
		su8Idx = s32Idx;
	}
	else			// Normal
	{
		su8Idx = 0xFF;
	}
}

static const ISP_U32 gau32GainTbl[141] = {
 1024,	1059,  1097,  1135, 1175, 1217, 1259, 1304, 1349, 1397, 1446, 1497, 1549, 1604,
 1660,	1719,  1779,  1842, 1906, 1973, 2043, 2048, 2119, 2194, 2271, 2351, 2434, 2519, 
 2608,	2699,  2794,  2892, 2994, 3099, 3208, 3321, 3438, 3559, 3684, 3813, 3947, 4086, 
 4229,	4378,  4532,  4691, 4856, 5027, 5203, 5386, 5576, 5772, 5974, 6184, 6402, 6627, 
 6860,	7101,  7350,  7609, 7876, 8153, 8439, 8736, 9043, 9361, 9690, 10030,10383,10748,
 11125, 11516, 11921, 12340,12774,13222,13687,14168,14666,15182,15715,16267,16839,17431,
 18043, 18677, 19334, 20013,20717,21445,22198,22978,23786,24622,25487,26383,27310,28270,
 29263, 30292, 31356, 32458,33599,34780,36002,37267,38577,39932,41336,42788,44292,45849,
 47460, 49128, 50854, 52641,54491,56406,58388,60440,62564,64763,67039,69395,71833,74358,
 76971, 79676, 82476, 85374,88375,91480,94695,98023,101468,105034,108725,112545,116501,
 120595,124833
};

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

	if(su8Idx == 0xFF)
	{
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
	}
	else
	{
		pstAeSnsGainInfo->u32SnsTimes = u32InTimes;
		pstAeSnsGainInfo->u32GainDb = gau16ShutCal[su8Idx][3];
	}
	return;
}

static ISP_VOID cmos_dgain_calc_table(ISP_DEV IspDev, ISP_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
	int i;

	if(ISP_NULL == pstAeSnsGainInfo)
	{
		ERR("null pointer when get ae sensor gain info	value!\n");
		return;
	}
 
	pstAeSnsGainInfo->u32GainDb = 0;
	pstAeSnsGainInfo->u32SnsTimes = 1024;

	if(su8Idx == 0xFF)
	{
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
	}
	else
	{
		pstAeSnsGainInfo->u32SnsTimes = u32InTimes;
		pstAeSnsGainInfo->u32GainDb = 0;
	}
	return;
}

static ISP_S32 cmos_get_ae_default(ISP_DEV IspDev, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	if (ISP_NULL == pstAeSnsDft)
	{
		ERR("null pointer when get ae default value!\n");
		return -1;
	}
	pstAeSnsDft->u8UpdateMode = 1;
	pstAeSnsDft->u32FullLinesStd = HD1080_25P_LINES;
	pstAeSnsDft->u8AeCompensation = 0x39;	
	pstAeSnsDft->u32LinesPer500ms = pstAeSnsDft->u32FullLinesStd*25/2;
	pstAeSnsDft->u32FlickerFreq = 0;

	pstAeSnsDft->u32MaxIntTime = pstAeSnsDft->u32FullLinesStd-4;
	pstAeSnsDft->u32MinIntTime = 2;    

	pstAeSnsDft->u32MaxAgain = 62564;
	pstAeSnsDft->u32MinAgain = 1024;

	pstAeSnsDft->u32MaxDgain= 1024;
	pstAeSnsDft->u32MinDgain = 1024;

	pstAeSnsDft->u32ISPDgainShift = 8;
	pstAeSnsDft->u32MaxISPDgain= 8 << pstAeSnsDft->u32ISPDgainShift;
	pstAeSnsDft->u32MinISPDgain= 1 << pstAeSnsDft->u32ISPDgainShift;

	pstAeSnsDft->u32UpdateCfg = 0x01010002;
	return 0;
}

static ISP_S32 cmos_get_sensor_max_resolution(ISP_DEV IspDev, ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution)
{
	if (ISP_NULL == pstSensorMaxResolution)
	{
	    ERR("null pointer when get sensor max resolution \n");
	    return -1;
	}
	pstSensorMaxResolution->u32MaxWidth  = 1920;
	pstSensorMaxResolution->u32MaxHeight = 1080;
	return 0;
}


static ISP_VOID cmos_inttime_update(ISP_DEV IspDev, ISP_U32 u32IntTime)
{
	static ISP_U32 su32Time = 0xFFFF;
	gau32ShutNow[IspDev] = u32IntTime;
	u32IntTime = gau16FullLines[IspDev] - u32IntTime;
	if(su32Time == u32IntTime)		return ;
	su32Time = u32IntTime;
#if 0
	sensor_write_register(0x3020,u32IntTime & 0xFF);
	sensor_write_register(0x3021,(u32IntTime & 0xFF00) >> 8);
	sensor_write_register(0x3022,(u32IntTime & 0x30000) >> 16);
#else
	gstIspI2CData[0].u32Data = u32IntTime & 0xFF;
	gstIspI2CData[1].u32Data = (u32IntTime & 0xFF00) >> 8;
#endif
	return;
}

static ISP_VOID cmos_gains_update(ISP_DEV IspDev, ISP_U32 u32Again, ISP_U32 u32Dgain)
{
	ISP_U32 u32HCG = 0x02;	 
	ISP_U32 u32Tmp;		  
	if(u32Again >= 21)	 //2048 
	{		 
		u32HCG = u32HCG | 0x10;  // bit[4] HCG	.Reg0x209[7:0]		  
		u32Again = u32Again - 21;	
	}	
	u32Tmp = u32Again + u32Dgain;		 
#if 0
	sensor_write_register(0x3014,u32Tmp & 0xFF);	
	sensor_write_register(0x3009,u32HCG & 0xFF);
#else
	gstIspI2CData[2].u32Data = u32Tmp & 0xFF;
	gstIspI2CData[3].u32Data = u32HCG & 0xFF;
#endif
	return;
}

/* the function of sensor set fps */
static ISP_VOID cmos_fps_set(ISP_DEV IspDev, ISP_U8 u8Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_U32 u32TotalSizeV,u32LineLength;
	u32TotalSizeV = HD1080_25P_LINES;
	u32LineLength = 2640;
	switch(u8Fps)
	{
		case 20:
			u32LineLength = u32LineLength*25/u8Fps;
			DEBUG("cmos_fps_set IMX307 2M %dfps\n", u8Fps);
			break;
		case 30:
			u32LineLength = u32LineLength*25/u8Fps;;
			DEBUG("cmos_fps_set IMX307 2M %dfps\n", u8Fps);
			break;
		case 25:
		default:
			DEBUG("cmos_fps_set IMX307 2M 25fps\n");
			break;
	}
	u32LineLength = u32LineLength<<1;
	sensor_write_register(IspDev, 0x301c, u32LineLength&0xFF);
	sensor_write_register(IspDev, 0x301d, (u32LineLength>>8)&0xFF);
	XM_MPI_MIPI_RefreshFV(IspDev,0, u32TotalSizeV);
	if(pstAeSnsDft != NULL) 
	{
		pstAeSnsDft->u32FullLinesStd = u32TotalSizeV;
		pstAeSnsDft->u32MaxIntTime = pstAeSnsDft->u32FullLinesStd - 4;
		pstAeSnsDft->u32LinesPer500ms = pstAeSnsDft->u32FullLinesStd * u8Fps / 2;
		gau16FullLines[IspDev] = u32TotalSizeV;
	}
	return;
}

static ISP_VOID cmos_slow_framerate_set(ISP_DEV IspDev, ISP_U16 u16FullLines, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	static ISP_U16 preU16FullLine = 0xffff;
	if(preU16FullLine == u16FullLines)
		return;

	preU16FullLine = u16FullLines;
	pstAeSnsDft->u32MaxIntTime = u16FullLines - 4;
	if(gau32ShutNow[IspDev]>pstAeSnsDft->u32MaxIntTime)
	{
		cmos_inttime_update(IspDev, pstAeSnsDft->u32MaxIntTime);
	}

	sensor_write_register(IspDev, 0x3018, u16FullLines&0xff);
	sensor_write_register(IspDev, 0x3019, (u16FullLines&0xff00)>>8);
	sensor_write_register(IspDev, 0x301A, (u16FullLines&0x30000)>>16);
	XM_MPI_MIPI_RefreshFV(IspDev,0, u16FullLines);
    return;
}

static ISP_S32 cmos_get_sns_regs_info(ISP_DEV IspDev, ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{
	ISP_S32 i;
	if(g_stSnsRegsInfo.u32RegNum != 4)	// First time
	{
		g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
		g_stSnsRegsInfo.u8Cfg2ValidDelayMax = 2;		
		g_stSnsRegsInfo.u32RegNum = 4;
		g_stSnsRegsInfo.pstI2CData = gstIspI2CData;
		for (i = 0; i < g_stSnsRegsInfo.u32RegNum; i++)
		{	
			gstIspI2CData[i].bUpdate = ISP_TRUE;
		}
		
		gstIspI2CData[0].u8DelayFrmNum = 0;		// Shut_L
		gstIspI2CData[0].u32RegAddr = 0x3020;
		gstIspI2CData[1].u8DelayFrmNum = 0;
		gstIspI2CData[1].u32RegAddr = 0x3021;	// Shut_H

		gstIspI2CData[2].u8DelayFrmNum = 0;	// Gain
		gstIspI2CData[2].u32RegAddr = 0x3014;
		
		gstIspI2CData[3].u8DelayFrmNum = 1;
		gstIspI2CData[3].u32RegAddr = 0x3009;	// HCG
	}
	else
	{
		for (i = 0; i < g_stSnsRegsInfo.u32RegNum; i++)        
		{
			if(gstIspI2CData[i].u32Data == gau32PreI2CData[i])            
			{                
				gstIspI2CData[i].bUpdate = ISP_FALSE;
			}            
			else            
			{                
				gstIspI2CData[i].bUpdate = ISP_TRUE;
				gau32PreI2CData[i] = gstIspI2CData[i].u32Data;
			}
		}
	}

	if (ISP_NULL == pstSnsRegsInfo)
	{
		ERR("null pointer when get sns reg info!\n");
		return -1;
	}
	XM_MPI_ISP_Memncpy((ISP_U8*)pstSnsRegsInfo, (ISP_U8*)&g_stSnsRegsInfo, sizeof(ISP_SNS_REGS_INFO_S));
	return 0;
}



/****************************************************************************
* AWB
****************************************************************************/
static const ISP_COLORMATRIX_AUTO_S g_stAwbCcm =
{
	5082,//0.16
	{
		0x0000, 540, -266, -18,
		0x0000, -104, 456, -96,
		0x0000, 20, -178, 414,
	},
	4000,//0.16
	{
		0x0000, 540, -266, -18,
		0x0000, -104, 456, -96,
		0x0000, 20, -178, 414,
	},
	2800,//0.13
	{
		0x0000, 439, -170, -13,
		0x0000, -120, 427, -51,
		0x0000, 56, -352, 552,
	}
};

const static ISP_AWB_CALIBRATION_V2_S gstAwbCal = {
	{0, 0, 4096, 2968, 4096, 2601, 1701, 4096},
	{4096, 4096, 0, 0, 2782, 2658, 4096, 917},
	{-80, -4090, -1032, -4096, -2515, -4096, -1802, -1729},
	213,
	0, 1251, 0, 2388,
	{0, 348, 655, 1006, 1098, 1237, 1423, 1475, 2388, 0, 0, 0, 0, 0, 0, 0},
	{2000, 2150, 2800, 4000, 4150, 5000, 6500, 7500, 12000, 0, 0, 0, 0, 0, 0, 0},
 	{1870, 1024, 1540, 0}// 1920->1540
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


static ISP_S32 sensor_write_register_imx307(ISP_DEV IspDev, ISP_U32 addr, ISP_U32 data)
{
	return sensor_write_register(IspDev, addr, data);
}

static ISP_S32 sensor_read_register_imx307(ISP_DEV IspDev, ISP_U32 addr)
{
	return sensor_read_register(IspDev, addr);
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
ISP_S32 cmos_init_chn_imx307(ISP_DEV IspDev)
{
	return 0;
}


ISP_S32 cmos_init_sensor_exp_function_imx307(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
	XM_MPI_ISP_Memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));
	pstSensorExpFunc->pfn_cmos_sensor_init = sensor_init;
	pstSensorExpFunc->pfn_cmos_get_isp_default = cmos_get_isp_default;
	pstSensorExpFunc->pfn_cmos_set_pixel_detect = cmos_set_pixel_detect;
	pstSensorExpFunc->pfn_cmos_get_sensor_max_resolution = cmos_get_sensor_max_resolution;
	pstSensorExpFunc->pfn_cmos_set_mirror_flip = cmos_set_mirror_flip;
	return 0;
}

ISP_S32 cmos_init_ae_exp_function_imx307(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	XM_MPI_ISP_Memset((ISP_U8 *)pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));
	pstExpFuncs->pfn_cmos_get_ae_default    = cmos_get_ae_default;
	pstExpFuncs->pfn_cmos_fps_set           = cmos_fps_set;
	pstExpFuncs->pfn_cmos_fps_get = NULL;
	pstExpFuncs->pfn_cmos_slow_framerate_set= cmos_slow_framerate_set;    
	pstExpFuncs->pfn_cmos_inttime_update    = cmos_inttime_update;
	pstExpFuncs->pfn_cmos_gains_update      = cmos_gains_update;
	pstExpFuncs->pfn_cmos_again_calc_table  = cmos_again_calc_table;
	pstExpFuncs->pfn_cmos_dgain_calc_table  = cmos_dgain_calc_table;
	pstExpFuncs->pfn_cmos_shut_calc_table = cmos_shut_calc_table;
	pstExpFuncs->pfn_cmos_get_sns_reg_info = cmos_get_sns_regs_info;
	pstExpFuncs->pfn_cmos_write_register = sensor_write_register_imx307;
	pstExpFuncs->pfn_cmos_read_register = sensor_read_register_imx307;
	return 0;
}

ISP_S32 cmos_init_awb_exp_function_imx307(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	XM_MPI_ISP_Memset((ISP_U8*)pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));
	pstExpFuncs->pfn_cmos_get_awb_default = cmos_get_awb_default;
	return 0;
}
