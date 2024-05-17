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



#define HD4MP_20P_LINES_SC401AI		(1500)
#define MAX_SHUT_OFST		(6)



static const ISP_CMOS_AGC_TABLE_S g_stIspAgcTable_sc401AI = {
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
	{0x20,0x20,0x40,0x40,0x40,0x40,0x40,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60}, 
	/* DyDpc_thresh 		*/
	{0x00,0x00,0x00,0x00,0x3f,0x7f,0xcf,0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,0xd7,0xd7},   
	/* saturation_strength */
	{0x89,0x89,0x86,0x86,0x86,0x78,0x60,0x50,0x50,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
	/* Blc	*/
	{0x102,0x102,0x100,0x0fc,0x0f8,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0},
	/* Contrast*/ 
	{0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32}
};

static ISP_S32 cmos_get_isp_default(ISP_DEV IspDev, ISP_CMOS_DEFAULT_S *pstDef)
{
	ISP_U8 u8ColorBw = 0;	// 0:Color	1:Bw
	if (ISP_NULL == pstDef)
	{
		ERR("null pointer when get isp default value!\n");
		return -1;
	}
	gstCmosDnPara[IspDev].pastAgcTab[0] = &g_stIspAgcTable_sc401AI;
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
	ISP_U32 u32Val;
	ISP_U16 u16OfstH,u16OfstV;
	ISPAPP_WIN_ATTR_S stWinAttr;
	u16OfstH = 0;
	u32Val = sensor_read_register(IspDev, 0x3221);
	if(u8Mirror)
	{
		u32Val |= 0x06;
	}
	else
	{
		u32Val &= ~0x06;
	}

	if(u8Flip)
	{
		u32Val |= 0x60;
		u16OfstV = 0;
	}
	else
	{
		u32Val &= ~0x60;
		u16OfstV = 0;
	}
	sensor_write_register(IspDev, 0x3221, u32Val);
	stWinAttr.u8Mode = 1;
	stWinAttr.u16ValH = u16OfstH;
	stWinAttr.u16ValV = u16OfstV;
	XM_MPI_ISPAPP_WinSet(IspDev, &stWinAttr);
	return ISP_SUCCESS;
}


#define DCG_GAIN_VAL	(1504)	// *1024
static ISP_VOID cmos_again_calc_table(ISP_DEV IspDev, ISP_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
	int i;
	int dcg=0;
	if(ISP_NULL == pstAeSnsGainInfo)
	{
		ERR("null pointer when get ae sensor gain info	value!\n");
		return;
	}
	pstAeSnsGainInfo->u32GainDb = 0;
	pstAeSnsGainInfo->u32SnsTimes = 1024;
	if(u32InTimes>=DCG_GAIN_VAL)
	{
		dcg = 1;						//DCG
		u32InTimes=u32InTimes*64/DCG_GAIN_VAL;
	}
	else
	{
		u32InTimes = u32InTimes/16;
	}
	u32InTimes = u32InTimes < 64? 64: u32InTimes;

	for(i = 0; i < 4; i++)
	{
		if(u32InTimes < 128)
		{
			break;
		}
		u32InTimes >>= 1;
		pstAeSnsGainInfo->u32GainDb = (pstAeSnsGainInfo->u32GainDb<<1)|1;
	}
	pstAeSnsGainInfo->u32GainDb = (dcg<<31)|(pstAeSnsGainInfo->u32GainDb<<8)|u32InTimes; 
	u32InTimes = u32InTimes<<(i+4);
	if(dcg)
	{
		u32InTimes = (ISP_U64)u32InTimes*DCG_GAIN_VAL/1024;
	}
	pstAeSnsGainInfo->u32SnsTimes = u32InTimes;
	gau32AGainNow[IspDev] = pstAeSnsGainInfo->u32SnsTimes;
	return;
}

static ISP_VOID cmos_dgain_calc_table(ISP_DEV IspDev, ISP_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
	int i;
	if(ISP_NULL == pstAeSnsGainInfo)
	{
		ERR("cmos_dgain_calc_table failed!\n");
		return;
	}
	pstAeSnsGainInfo->u32GainDb = 0;
	if(u32InTimes<128) u32InTimes = 128;

	for(i = 0; i < 5; i++)
	{
		if(u32InTimes < 256)
		{
			break;
		}
		u32InTimes >>= 1;
		pstAeSnsGainInfo->u32GainDb = (pstAeSnsGainInfo->u32GainDb<<1)|1;
	}
	u32InTimes = (u32InTimes>>1)<<1;
	pstAeSnsGainInfo->u32SnsTimes = u32InTimes<<i;
	pstAeSnsGainInfo->u32GainDb = (pstAeSnsGainInfo->u32GainDb<<8)+u32InTimes;
	gau32DGainNow[IspDev] = pstAeSnsGainInfo->u32SnsTimes;
	return;
}

static ISP_S32 cmos_get_ae_default(ISP_DEV IspDev, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	if (ISP_NULL == pstAeSnsDft)
	{
		ERR("null pointer when get ae default value!\n");
		return -1;
	}
	pstAeSnsDft->u32FullLinesStd = HD4MP_20P_LINES_SC401AI;
	pstAeSnsDft->u8AeCompensation = 0x39;	
	pstAeSnsDft->u32LinesPer500ms = pstAeSnsDft->u32FullLinesStd*20/2;
	pstAeSnsDft->u32FlickerFreq = 0;

	pstAeSnsDft->u32MaxIntTime = pstAeSnsDft->u32FullLinesStd-MAX_SHUT_OFST;
	pstAeSnsDft->u32MinIntTime = 2;    

	pstAeSnsDft->u32MaxAgain = 23320;
	pstAeSnsDft->u32MinAgain = 1024;

	pstAeSnsDft->u32MaxDgain = 4096;	
	pstAeSnsDft->u32MinDgain = 128;

	pstAeSnsDft->u32ISPDgainShift = 8;
	pstAeSnsDft->u32MaxISPDgain= 8 << pstAeSnsDft->u32ISPDgainShift;
	pstAeSnsDft->u32MinISPDgain= 1 << pstAeSnsDft->u32ISPDgainShift;
	return 0;
}


static ISP_S32 cmos_get_sensor_max_resolution(ISP_DEV IspDev, ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution)
{
	if (ISP_NULL == pstSensorMaxResolution)
	{
	    ERR("null pointer when get sensor max resolution \n");
	    return -1;
	}

	pstSensorMaxResolution->u32MaxWidth  = 2560;
	pstSensorMaxResolution->u32MaxHeight = 1448;
	return 0;
}

static ISP_VOID cmos_inttime_update(ISP_DEV IspDev, ISP_U32 u32IntTime)
{
	if(gau32ShutNow[IspDev] == u32IntTime)
		return ;
	gau32ShutNow[IspDev] = u32IntTime;
	u32IntTime = u32IntTime*2;
	sensor_write_register(IspDev, 0x3e02, ((u32IntTime&0xf)<<4));
	sensor_write_register(IspDev, 0x3e01, ((u32IntTime&0x0ff0)>>4));
	sensor_write_register(IspDev, 0x3e00, ((u32IntTime&0xf000)>>12));
	return;
}


static ISP_VOID cmos_gains_update(ISP_DEV IspDev, ISP_U32 u32Again, ISP_U32 u32Dgain)
{
	static ISP_U32 sau32AGain[ISP_NUM_MAX] = {0xFFFFFFFE};
	static ISP_U32 sau32DGain[ISP_NUM_MAX] = {0xFFFFFFFE};
	ISP_U32 u32Tmp,tmp[4];
	ISP_U8 u8i;
	if(sau32AGain[0] == 0xFFFFFFFE)
	{
		for(u8i=0;u8i<ISP_NUM_MAX;u8i ++)
		{
			sau32AGain[u8i] = 0xFFFFFFFF;
			sau32DGain[u8i] = 0xFFFFFFFF;
		}
	}
	if((sau32AGain[IspDev] != u32Again) || (sau32DGain[IspDev] != u32Dgain))
	{
		sau32AGain[IspDev] = u32Again;
		sau32DGain[IspDev] = u32Dgain;
		// 2.GainUpdate
		tmp[0] = u32Again&0xFF; 
		u32Tmp = (u32Again>>8) & 0xFF;
		tmp[1] = 0x03 | (u32Tmp<<2);
		if(u32Again&(1<<31))	// DCG
		{
			tmp[1] |= 0x20;
		}

		tmp[2] = u32Dgain&0xFF; 
		tmp[3] = (u32Dgain>>8) & 0xFF;
		sensor_write_register(IspDev, 0x3e08, tmp[1]);
		sensor_write_register(IspDev, 0x3e09, tmp[0]);
		sensor_write_register(IspDev, 0x3e06, tmp[3]);
		sensor_write_register(IspDev, 0x3e07, tmp[2]);
	}
	return;
}

static ISP_VOID cmos_slow_framerate_set(ISP_DEV IspDev, ISP_U16 u16FullLines, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	static ISP_U16 preU16FullLine = 0xffff;
	if(preU16FullLine == u16FullLines)
		return;

	preU16FullLine = u16FullLines;
	pstAeSnsDft->u32MaxIntTime = u16FullLines - MAX_SHUT_OFST;
	if(gau32ShutNow[IspDev]>pstAeSnsDft->u32MaxIntTime)
	{
		cmos_inttime_update(IspDev, pstAeSnsDft->u32MaxIntTime);
	}

	sensor_write_register(IspDev, 0x320e, (u16FullLines>>8)&0xFF);
	sensor_write_register(IspDev, 0x320f, u16FullLines&0xFF);	

	XM_MPI_MIPI_RefreshFV(IspDev,0, u16FullLines);
}

static ISP_VOID cmos_fps_set(ISP_DEV IspDev, ISP_U8 u8Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_U32 u32TotalSizeV,u32TotalSizeH;
	u32TotalSizeV = HD4MP_20P_LINES_SC401AI;
	switch(u8Fps)
	{
		case 30:	u32TotalSizeH = 4200*20/30;
					break;
		case 25:	u32TotalSizeH = 4200*20/25;
					break;
		case 20:
		default:	u8Fps = 20;
					u32TotalSizeH = 4200;
					break;
	}
	if(pstAeSnsDft != NULL)
	{
		pstAeSnsDft->u32FullLinesStd = u32TotalSizeV;
		pstAeSnsDft->u32MaxIntTime = pstAeSnsDft->u32FullLinesStd-MAX_SHUT_OFST;
		pstAeSnsDft->u32LinesPer500ms = pstAeSnsDft->u32FullLinesStd * u8Fps / 2;
	}
	sensor_write_register(IspDev, 0x320c, (u32TotalSizeH>>9)&0xFF);
	sensor_write_register(IspDev, 0x320d, (u32TotalSizeH>>1)&0xFF);	
	sensor_write_register(IspDev, 0x320e, (u32TotalSizeV>>8)&0xFF);
	sensor_write_register(IspDev, 0x320f, u32TotalSizeV&0xFF);	
	XM_MPI_MIPI_RefreshFV(IspDev,0, u32TotalSizeV);
	return;
}



/****************************************************************************
* AWB
****************************************************************************/
static const ISP_COLORMATRIX_AUTO_S g_stAwbCcm =
{
	5000,
	{
		0x000,379,-108,-15,
		0x000,-70, 381,-55,
		0x000,  5,-120,371
	},
	4000,
	{
		0x000,380,-111,-13,
		0x000,-84, 371,-31,
		0x000,  4,-129,381
	},
	2800,
	{
		0x000,327, -26, -45,
		0x000,-95, 383, -32,
		0x000, -1,-216, 473
	}
};

static const ISP_AWB_CALIBRATION_V2_S gstAwbCal = {
#if 0
	{0, 0, 4096, 3029, 4096, 2435, 1510, 4096},
	{4096, 4096, 0, 0, 3237, 3068, 4096, 1246},
	{-644, -3659, -1477, -4096, -3171, -3896, -2268, -2250},
	213,
	0, 1339, 0, 2439,
	{0, 365, 723, 1074, 1167, 1296, 1518, 1585, 2439, 0, 0, 0, 0, 0, 0, 0},
	{2000, 2150, 2800, 4000, 4150, 5000, 6500, 7500, 12000, 0, 0, 0, 0, 0, 0, 0},
 	{1969, 1024, 1758, 0}
#else
	{0, 0, 4096, 2827, 4096, 2492, 1508, 4096},
	{4096, 4096, 0, 0, 3232, 2794, 4096, 1245},
	{-244, -4059, -1077, -4096, -2859, -4096, -1840, -2031},
	213,
	0, 1339, 0, 2439,
	{0, 366, 723, 1074, 1166, 1296, 1518, 1584, 2439, 0, 0, 0, 0, 0, 0, 0},
	{2000, 2150, 2800, 4000, 4150, 5000, 6500, 7500, 12000, 0, 0, 0, 0, 0, 0, 0},
	{1812, 1024, 1957, 0}
#endif
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


/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
 ISP_S32 cmos_init_chn_sc401AI(ISP_DEV IspDev)
{
	return 0;
}
 
ISP_S32 cmos_init_sensor_exp_function_sc401AI(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
	XM_MPI_ISP_Memset((ISP_U8 *)pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));
	pstSensorExpFunc->pfn_cmos_sensor_init = sensor_init;
	pstSensorExpFunc->pfn_cmos_get_isp_default = cmos_get_isp_default;
	pstSensorExpFunc->pfn_cmos_set_pixel_detect = cmos_set_pixel_detect;
	pstSensorExpFunc->pfn_cmos_get_sensor_max_resolution = cmos_get_sensor_max_resolution;
	pstSensorExpFunc->pfn_cmos_set_mirror_flip = cmos_set_mirror_flip;
	return 0;
}

ISP_S32 cmos_init_ae_exp_function_sc401AI(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
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

ISP_S32 cmos_init_awb_exp_function_sc401AI(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	XM_MPI_ISP_Memset((ISP_U8*)pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));
	pstExpFuncs->pfn_cmos_get_awb_default = cmos_get_awb_default;
	return 0;
}








