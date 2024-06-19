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



#define HD_20P_LINES_SC3338		(1350)
#define MAX_SHUT_OFST		(8)



static const ISP_CMOS_AGC_TABLE_S g_stIspAgcTable_sc3338 = {
	/* bvalid */
	1,
	/* 100, 200, 400, 800, 1600, 3200, 6400, 12800; 100, 200, 400, 800, 1600, 3200, 6400, 12800 */
	/* snr_thresh 2DNr		*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/* drc	*/
	{0x32,0x28,0x1c,0x1a,0x16,0x10,0x0c,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08},
	/* defog	*/
	{0x16,0x16,0x18,0x18,0x1a,0x1a,0x20,0x20,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28},
	/* defog_vpss	*/
	{0x30,0x30,0x30,0x25,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20}, 
	/* DyDpc_thresh 		*/
	{0xB8,0xC0,0xD0,0xD4,0xD8,0xDC,0xE0,0xE0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0},   
	/* saturation_strength */
	{0xC0,0xBE,0xB8,0xB0,0xAD,0xA8,0xA4,0xA4,0x80,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
	/* Blc	*/
	{0x104,0x104,0x102,0x100,0xFE,0xF8,0xF4,0xE8,0xE8,0xE8,0xE8,0xE8,0xE8,0xE8,0xE8,0xE8},
	/* Contrast*/ 
	{0x33,0x33,0x33,0x32,0x32,0x32,0x30,0x30,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32},
};

const ISP_CMOS_SHARPEN_S gstSharpenColor_sc3338 = {
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
	{0x48,0x5a,0x50,0x40,0x40,0x40,0x40,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38},
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
	
const ISP_CMOS_VPSS_3DNR_S gstVpss3DNr_sc3338 = {
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
	{0x78,0x78,0x6e,0x6c,0x6a,0x60,0x50,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45},
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

static ISP_S32 cmos_get_isp_default(ISP_DEV IspDev, ISP_CMOS_DEFAULT_S *pstDef)
{
	ISP_U8 u8ColorBw = 0;	// 0:Color	1:Bw
	if (ISP_NULL == pstDef)
	{
		ERR("null pointer when get isp default value!\n");
		return -1;
	}
	gstCmosDnPara[IspDev].pastAgcTab[0] = &g_stIspAgcTable_sc3338;
	gstCmosDnPara[IspDev].pastAgcYcNr[0] = &gstYCNr;
	gstCmosDnPara[IspDev].pastAgcSharp[0] = &gstSharpenColor_sc3338;
	gstCmosDnPara[IspDev].pastAgcSharp[1] = &gstSharpenBw;
	gstCmosDnPara[IspDev].pastAgcVpss3DNr[0] = &gstVpss3DNr_sc3338;
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

static ISP_VOID cmos_again_calc_table(ISP_DEV IspDev, ISP_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
	int i;
	ISP_U32 au32Again[7] = {32, 49, 98, 195, 390, 779, 1557};
	if(ISP_NULL == pstAeSnsGainInfo)
	{
		return;
	}
	u32InTimes = CLIP3(u32InTimes,32,1557);

	for(i = 1; i < 7; i++)
	{
		if(u32InTimes < au32Again[i])
		{
			break;
		}
	}
	pstAeSnsGainInfo->u32GainDb = i-1; 
	pstAeSnsGainInfo->u32SnsTimes = au32Again[i-1];
	gau32AGainNow[IspDev] = pstAeSnsGainInfo->u32SnsTimes;
	return;
}


static ISP_VOID cmos_dgain_calc_table(ISP_DEV IspDev, ISP_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
	int i;
	ISP_U8 u8Flg2x = 0;
	if(ISP_NULL == pstAeSnsGainInfo)
	{
		return;
	}
	u32InTimes = CLIP3(u32InTimes,32,128);
	for(i = 0; i < 6; i++)
	{
		if(u32InTimes < 64)
		{
			break;
		}
		u32InTimes >>= 1;
		u8Flg2x = (u8Flg2x<<1)|0x01;
	}
	u32InTimes = (u32InTimes)*4;
	pstAeSnsGainInfo->u32GainDb = ((ISP_U32)u8Flg2x<<16)|u32InTimes; 
	pstAeSnsGainInfo->u32SnsTimes = (u32InTimes>>2)<<i;
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
	pstAeSnsDft->u32FullLinesStd = HD_20P_LINES_SC3338;
	pstAeSnsDft->u8AeCompensation = 0x39;
	pstAeSnsDft->u32LinesPer500ms = pstAeSnsDft->u32FullLinesStd*25/2;
	pstAeSnsDft->u32FlickerFreq = 0;


	pstAeSnsDft->u32MaxIntTime = pstAeSnsDft->u32FullLinesStd-8;
	pstAeSnsDft->u32MinIntTime = 1;  

	pstAeSnsDft->u32MaxAgain = 1557; //48.64
	pstAeSnsDft->u32MinAgain = 32;


	pstAeSnsDft->u32MaxDgain = 128;  
	pstAeSnsDft->u32MinDgain = 32;


	pstAeSnsDft->u32ISPDgainShift = 8;
	pstAeSnsDft->u32MaxISPDgain= 8 << pstAeSnsDft->u32ISPDgainShift;
	pstAeSnsDft->u32MinISPDgain= 1 << pstAeSnsDft->u32ISPDgainShift;

	gau32AGainNow[IspDev] = pstAeSnsDft->u32MinAgain;
	gau32DGainNow[IspDev] = pstAeSnsDft->u32MinDgain;
	return 0;
}


static ISP_S32 cmos_get_sensor_max_resolution(ISP_DEV IspDev, ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution)
{
	if (ISP_NULL == pstSensorMaxResolution)
	{
	    ERR("null pointer when get sensor max resolution \n");
	    return -1;
	}

	pstSensorMaxResolution->u32MaxWidth  = 2304;
	pstSensorMaxResolution->u32MaxHeight = 1296;
	return 0;
}

static ISP_VOID cmos_inttime_update(ISP_DEV IspDev, ISP_U32 u32IntTime)
{
	if(gau32ShutNow[IspDev] == u32IntTime)	return;
	gau32ShutNow[IspDev] = u32IntTime;
	sensor_write_register(IspDev, 0x3e02, ((u32IntTime&0xf)<<4));
	sensor_write_register(IspDev, 0x3e01, ((u32IntTime&0x0ff0)>>4));
	sensor_write_register(IspDev, 0x3e00, ((u32IntTime&0xf000)>>12));
	return;
}

const static ISP_U8 gau8Logic_sc3338[4][3] = {
	//  0x363c, 0x363c, 0x330e
		{0x05, 0x05, 0x20},
		{0x07, 0x07, 0x1c},
		{0x07, 0x07, 0x1c},
		{0x07, 0x07, 0x12}
};
const static ISP_U16 gau16LogicAddr[3] = {
	0x363c, 0x363c, 0x330e
};
static void gainLogic_sc3338(ISP_DEV IspDev, ISP_U32 u32AllGain)
{
	int u8Reg0x3e09;
	u8Reg0x3e09 = sensor_read_register(IspDev,0x3e09);
	if(u8Reg0x3e09 >= 0x4b)
	{
		sensor_write_register(IspDev,0x57aa,0xcd);
		sensor_write_register(IspDev,0x57ad,0x00);
	}
	else if(u8Reg0x3e09 <= 0x49)
	{
		sensor_write_register(IspDev,0x57aa,0x2a);
		sensor_write_register(IspDev,0x57ad,0x0d);
	}
	else
	{
		sensor_write_register(IspDev,0x57aa,0x2a);
		sensor_write_register(IspDev,0x57ad,0x0d);
	}
}
static ISP_VOID cmos_gains_update(ISP_DEV IspDev, ISP_U32 u32Again, ISP_U32 u32Dgain)
{
	static ISP_U32 sau32AGain[ISP_NUM_MAX] = {0xFFFFFFF};
	static ISP_U32 sau32DGain[ISP_NUM_MAX];
	ISP_U32 u32AllGain,tmp[4];
	ISP_U32 au32AgainReg[7] = {0x00, 0x40, 0x48, 0x49, 0x4B, 0x4f, 0x5f};
	u32AllGain = (ISP_U64)gau32AGainNow[IspDev]*gau32DGainNow[IspDev]/128;
	gainLogic_sc3338(IspDev, u32AllGain);
	if((sau32AGain[IspDev] != u32Again) || (sau32DGain[IspDev] != u32Dgain))
	{
		sau32AGain[IspDev] = u32Again;
		sau32DGain[IspDev] = u32Dgain;
		// 2.GainUpdate
		// a.Again
		tmp[0] = au32AgainReg[u32Again&0xFF];
		// b.DGain
		tmp[1] = (u32Dgain>>16)&0xFF;
		tmp[2] = u32Dgain&0xFF;
		sensor_write_register(IspDev,0x3e09, tmp[0]);
		sensor_write_register(IspDev,0x3e06, tmp[1]);
		sensor_write_register(IspDev,0x3e07, tmp[2]);
	}
	return;
}

static ISP_VOID cmos_slow_framerate_set(ISP_DEV IspDev,ISP_U16 u16FullLines, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	static ISP_U16 preU16FullLine = 0xffff;
	if(preU16FullLine == u16FullLines)
		return;
	preU16FullLine = u16FullLines;
	pstAeSnsDft->u32MaxIntTime = u16FullLines - MAX_SHUT_OFST;
	if(gau32ShutNow[IspDev]>pstAeSnsDft->u32MaxIntTime)
	{
		cmos_inttime_update(IspDev,pstAeSnsDft->u32MaxIntTime);
	}
	sensor_write_register(IspDev, 0x320e, (u16FullLines>>8)&0xFF);
	sensor_write_register(IspDev, 0x320f, u16FullLines&0xFF);	
	XM_MPI_MIPI_RefreshFV(IspDev,0, u16FullLines);
}


static ISP_VOID cmos_fps_set(ISP_DEV IspDev,ISP_U8 u8Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	static ISP_U8 su8Mode = 0xFF;
	ISP_U32 u32Pixs=3750;
	ISP_U32 u32TotalSizeV;
	ISP_U32 u32ExpNow;
	ISP_U8 u8Mode;
	u32ExpNow = sensor_read_register(IspDev,0x3e00);
	u32ExpNow = (u32ExpNow<<8) | sensor_read_register(IspDev,0x3e01);
	u32Pixs = sensor_read_register(IspDev,0x3e02);
	u32ExpNow = (u32ExpNow<<4)|((u32Pixs>>4)&0x0F);
	switch(u8Fps)
	{
#if 0
		case 30:
			u32Pixs = 2400;
			u32TotalSizeV = HD1296_25_LINES;
			break;
		case 25:
			u8Fps = 25;
			u32Pixs = 2880;
			u32TotalSizeV = HD1296_25_LINES;
			break;
#endif
		case 15:
			u32Pixs = 3750*20/15;
			u32TotalSizeV = HD_20P_LINES_SC3338;
			break;

		case 12:
			u32Pixs = 3750*20/12;
			u32TotalSizeV = HD_20P_LINES_SC3338;
			break;
		default:
		case 20:
			u32Pixs = 3750;
			u32TotalSizeV = HD_20P_LINES_SC3338;
			break;
	}		
	u8Mode = u8Fps;
	if(su8Mode == u8Mode)
	{
		return;
	}
	su8Mode = u8Mode;
	
	if(pstAeSnsDft != NULL) 
	{
		pstAeSnsDft->u32FullLinesStd = u32TotalSizeV;
		pstAeSnsDft->u32MaxIntTime = HD_20P_LINES_SC3338-8;
		pstAeSnsDft->u32LinesPer500ms = HD_20P_LINES_SC3338 * u8Fps / 2;
	}
	if(u32ExpNow > (u32TotalSizeV-8))
	{
		u32ExpNow = u32TotalSizeV-8;
		cmos_inttime_update(IspDev,u32ExpNow);
	}
	sensor_write_register(IspDev,0x320c, (u32Pixs>>9)&0xFF);
	sensor_write_register(IspDev,0x320d, (u32Pixs>>1)&0xFF);
	sensor_write_register(IspDev,0x320e, (u32TotalSizeV>>8)&0xFF);
	sensor_write_register(IspDev,0x320f, u32TotalSizeV&0xFF);
	gau8Fps[IspDev] = u8Fps;
	if(u8Fps==20)
	{
		XM_MPI_MIPI_RefreshFV(IspDev,0, (1<<31)|u32Pixs);
	}	
	return;
}


/****************************************************************************
* AWB
****************************************************************************/
const static ISP_COLORMATRIX_AUTO_S g_stAwbCcm =
{
	5000,
	{
		0x0000, 263,  -60,  53,
		0x0000, 3,  207,	46,
		0x0000, 36,  -87,	307
	},	
	4000,
	{
		0x0000, 239, -45 , 62,
		0x0000, 16, 160, 80,
		0x0000, 37, -106, 325
		
	},
	2800,
	{
		0x0000, 227,  -40, 69,
		0x0000, -11, 164,	103,
		0x0000, -6, -198,	448
	}
};
const static ISP_AWB_CALIBRATION_V2_S gstAwbCal = {

	{0, 0, 4096, 2623, 3247, 2220, 3138, 4096},
	{4096, 4096, 0, 0, 4096, 3156, 4096, 1776},
	{-396, -3619, -1580, -4096, -3347, -4096, -3279, -2988},
	213,
	0, 1493, 0, 2536,
	{0, 487, 855, 1098, 1222, 1359, 1574, 1652, 2536, 0, 0, 0, 0, 0, 0, 0},
	{2000, 2150, 2800, 4000, 4150, 5000, 6500, 7500, 12000, 0, 0, 0, 0, 0, 0, 0},
	{1599, 1024, 2006, 0}
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
 ISP_S32 cmos_init_chn_sc3338(ISP_DEV IspDev)
{
	return 0;
}
 
ISP_S32 cmos_init_sensor_exp_function_sc3338(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
	XM_MPI_ISP_Memset((ISP_U8 *)pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));
	pstSensorExpFunc->pfn_cmos_sensor_init = sensor_init;
	pstSensorExpFunc->pfn_cmos_get_isp_default = cmos_get_isp_default;
	pstSensorExpFunc->pfn_cmos_set_pixel_detect = cmos_set_pixel_detect;
	pstSensorExpFunc->pfn_cmos_get_sensor_max_resolution = cmos_get_sensor_max_resolution;
	pstSensorExpFunc->pfn_cmos_set_mirror_flip = cmos_set_mirror_flip;
	return 0;
}

ISP_S32 cmos_init_ae_exp_function_sc3338(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
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

ISP_S32 cmos_init_awb_exp_function_sc3338(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	XM_MPI_ISP_Memset((ISP_U8*)pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));
	pstExpFuncs->pfn_cmos_get_awb_default = cmos_get_awb_default;
	return 0;
}









