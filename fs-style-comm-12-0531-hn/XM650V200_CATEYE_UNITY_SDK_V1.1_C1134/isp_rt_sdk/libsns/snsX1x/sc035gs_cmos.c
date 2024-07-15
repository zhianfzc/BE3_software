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



#define SC035GS_30P_LINES_DVP		(0x840)
#define SC035GS_30P_LINES_MIPI		(0x83E)
#define MAX_SHUT_OFST		(6)



static const ISP_CMOS_AGC_TABLE_S g_stIspAgcTable_sc035GS = {
	/* bvalid */
	1,
	/* 100, 200, 400, 800, 1600, 3200, 6400, 12800; 100, 200, 400, 800, 1600, 3200, 6400, 12800 */
	/* snr_thresh 2DNr		*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/* drc	*/
	{0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02},
	/* defog	*/
	{0x28,0x28,0x20,0x1a,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18},
	/* defog_vpss	*/
	{0x30,0x30,0x30,0x25,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20}, 
	/* DyDpc_thresh 		*/
	{0xB8,0xC0,0xD0,0xD4,0xD8,0xDC,0xE0,0xE0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0},   
	/* saturation_strength */
	{0x89,0x89,0x86,0x86,0x86,0x78,0x60,0x50,0x50,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
	/* Blc	*/
	{0x102,0x102,0x100,0x0fc,0x0f8,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0},
	/* Contrast*/ 
	{0x50,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32}
};

static const ISP_CMOS_SHARPEN_S gstSharpen_sc035GS = {
	/* bvalid */
	1,
	/* 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800 */
	/* Ud*/
	{0x04,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/* D*/
	{0x04,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*ThrUd*/
	{0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03},
	/*ThrD*/
	{0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03},	
	/*Ud_H*/
	{0x04,0x0a,0x08,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*D_H*/
	{0x04,0x0a,0x08,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*ThrUd_H*/
	{0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03},
	/*ThrD_H*/
	{0x03,0x03,0x03,0x03,0x03,0x06,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08},
	/*EdgeStr*/
	//{0x58,0x58,0x58,0x58,0x58,0x54,0x50,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40},
	{0x04,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*EdgeThr*/
	{0x04,0x06,0x08,0x08,0x08,0x08,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c},
	/*OverShoot*/
	{0x48,0x48,0x48,0x48,0x48,0x48,0x48,0x48,0x48,0x48,0x48,0x48,0x48,0x48,0x48,0x48},
	/*UnderShoot*/
	{0x68,0x68,0x68,0x68,0x68,0x68,0x68,0x68,0x68,0x68,0x68,0x68,0x68,0x68,0x68,0x68},
	/*DetailStr*/
	{0x30,0x30,0x2c,0x2a,0x28,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
	/*Rsv1*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*Rsv2*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};

const ISP_CMOS_VPSS_3DNR_S gstVpss3DNr_sc035GS = {
	0x00,
	/* 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800 */
	/* Flt_Thr    */
	{0x08,0x08,0x08,0x08,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18},
	/* Df_RatC    */
	{0x08,0x08,0x08,0x08,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04},
	/* Df1_RatioY    */
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/* Df1_RatioC    */
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/* MovThr_Max    */
	{0x00,0x00,0x00,0x01,0x01,0x01,0x02,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03},
	/* Alpha_Ratio    */
	{0x70,0x60,0x50,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40},
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

const ISP_CMOS_LTM_S gstLtm_sc035GS = {
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

static ISP_S32 cmos_get_isp_default(ISP_DEV IspDev, ISP_CMOS_DEFAULT_S *pstDef)
{
	ISP_U8 u8ColorBw = 0;	// 0:Color	1:Bw
	if (ISP_NULL == pstDef)
	{
		ERR("null pointer when get isp default value!\n");
		return -1;
	}
	gstCmosDnPara[IspDev].pastAgcTab[0] = &g_stIspAgcTable_sc035GS;
	gstCmosDnPara[IspDev].pastAgcYcNr[0] = &gstYCNr;
	gstCmosDnPara[IspDev].pastAgcSharp[0] = &gstSharpen_sc035GS;
	gstCmosDnPara[IspDev].pastAgcVpss3DNr[0] = &gstVpss3DNr_sc035GS;
	gstCmosDnPara[IspDev].pastGamma[0] = &gastIspGamma[1];
	gstCmosDnPara[IspDev].pastGamma[1] = &gastIspGamma[0];
	gstCmosDnPara[IspDev].pastAgcLtm[0] = &gstLtm_sc035GS;
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
	if(ISP_NULL == pstAeSnsGainInfo)
	{
		return;
	}
	ISP_U8 u8i;
	if(u32InTimes >= gau16GainTbl_SmatSns[63])
	{
		pstAeSnsGainInfo->u32GainDb = 63;
	}
	else
	{
		pstAeSnsGainInfo->u32GainDb = 0;
		for(u8i = 0x1; u8i < 64; u8i ++)		
		{			
			if(u32InTimes <gau16GainTbl_SmatSns[u8i])			 
			{				 
				pstAeSnsGainInfo->u32GainDb = u8i-1;
				break;			  
			}			 
		}
	}
	pstAeSnsGainInfo->u32SnsTimes = (ISP_U32)gau16GainTbl_SmatSns[pstAeSnsGainInfo->u32GainDb];
	return;
}


static ISP_VOID cmos_dgain_calc_table(ISP_DEV IspDev, ISP_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
	if(ISP_NULL == pstAeSnsGainInfo)
	{
		return;
	}

	ISP_U8 u8i;
	if(u32InTimes >= gau16GainTbl_SmatSns[63])
	{
		pstAeSnsGainInfo->u32GainDb = 63;
	}
	else
	{
		pstAeSnsGainInfo->u32GainDb = 0;
		for(u8i = 0x1; u8i < 64; u8i ++)		
		{			
			if(u32InTimes <gau16GainTbl_SmatSns[u8i])			 
			{				 
				pstAeSnsGainInfo->u32GainDb = u8i-1;
				break;			  
			}			 
		}
	}
	pstAeSnsGainInfo->u32SnsTimes = gau16GainTbl_SmatSns[pstAeSnsGainInfo->u32GainDb];
	return;
}


static ISP_S32 cmos_get_ae_default(ISP_DEV IspDev, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	if (ISP_NULL == pstAeSnsDft)
	{
		ERR("null pointer when get ae default value!\n");
		return -1;
	}
	pstAeSnsDft->u32FullLinesStd = (SENSCONT_DVP == gstSnsAttr[IspDev].enCmosIF)?SC035GS_30P_LINES_DVP:SC035GS_30P_LINES_MIPI;
	pstAeSnsDft->u8AeCompensation = 0x39;	
	pstAeSnsDft->u32LinesPer500ms = pstAeSnsDft->u32FullLinesStd*16*30/2;
	pstAeSnsDft->u32FlickerFreq = 0;

	pstAeSnsDft->u32MaxIntTime = (SC035GS_30P_LINES_MIPI-MAX_SHUT_OFST)*16;
	pstAeSnsDft->u32MinIntTime = 1;    

	pstAeSnsDft->u32MaxAgain = 15872;
	pstAeSnsDft->u32MinAgain = 1024;

	pstAeSnsDft->u32MaxDgain = 7936;	
	pstAeSnsDft->u32MinDgain = 1024;

	pstAeSnsDft->u32ISPDgainShift = 8;
	pstAeSnsDft->u32MaxISPDgain= 8 << pstAeSnsDft->u32ISPDgainShift;
	pstAeSnsDft->u32MinISPDgain= 1 << pstAeSnsDft->u32ISPDgainShift;

	pstAeSnsDft->u16AlgMode = 0x10;
	pstAeSnsDft->au32InitExp[0] = 33664;
	return 0;
}


static ISP_S32 cmos_get_sensor_max_resolution(ISP_DEV IspDev, ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution)
{
	if (ISP_NULL == pstSensorMaxResolution)
	{
	    ERR("null pointer when get sensor max resolution \n");
	    return -1;
	}

	pstSensorMaxResolution->u32MaxWidth  = 640;
	pstSensorMaxResolution->u32MaxHeight = 480;
	return 0;
}

static ISP_VOID cmos_inttime_update(ISP_DEV IspDev, ISP_U32 u32IntTime)
{
	static ISP_U32 sau32IntTime[ISP_NUM_MAX] = {0xFFFFFFFE};
	ISP_U8 u8i;
	if(sau32IntTime[0] == 0xFFFFFFFE)
	{
		for(u8i=0;u8i<ISP_NUM_MAX;u8i ++)
		{
			sau32IntTime[u8i] = 0xFFFFFFFF;
		}
	}
	if(sau32IntTime[IspDev] == u32IntTime)
		return ;
	sau32IntTime[IspDev] = u32IntTime;
	gau32ShutNow[IspDev] = u32IntTime;
	sensor_write_register(IspDev, 0x3e02, u32IntTime&0xFF);
	sensor_write_register(IspDev, 0x3e01, ((u32IntTime&0xff00)>>8));
	sensor_write_register(IspDev, 0x3e00, ((u32IntTime&0xFF0000)>>16));
	return;
}

const static ISP_U16 gau16LogicAddr[4] = {
	0x3314,0x3317,0x3631,0x3630
};
const static ISP_U8 gau8Logic_sc035GS[3][4] = {
//  0x3314,0x3317,0x3631,0x3630
	{0x6b, 0x14, 0x58, 0x4a},
	{0x4f, 0x10, 0x48, 0x4c},
	{0x74, 0x15, 0x48, 0x4c}
};

// u8Mode: 1:Refresh
static void gainLogic_sc035GS(ISP_DEV IspDev, ISP_U32 u32AllGain,ISP_U8 u8Mode)
{
	static ISP_U8 sau8Idx[ISP_NUM_MAX] = {0xFE};//[bit0~bit3]:Vstd   [bit4~bit7]:Agc
	ISP_U8 u8i,u8Idx,u8Idx2;
	u8Idx = 0;
	if(sau8Idx[0] == 0xFE)
	{
		for(u8i=0;u8i<ISP_NUM_MAX;u8i ++)
		{
			sau8Idx[u8i] = 0xFF;
		}
	}
	if(u32AllGain< 32)		// <2
	    u8Idx2 = 0;
	else if(u32AllGain<64)	// <4
	    u8Idx2 = 1;
	else
	    u8Idx2 = 2;
	if((((sau8Idx[IspDev]&0x0F) != u8Idx) || ((sau8Idx[IspDev]>>4)&0x0F) != u8Idx2) || u8Mode)
	{
		sau8Idx[IspDev] = ((u8Idx2&0x0F)<<4)|(u8Idx&0x0F);
		//sensor_write_register(IspDev, 0x3812, 0x00);
		for(u8i=0; u8i<4; u8i ++)
		{
		    sensor_write_register(IspDev, (ISP_U32)gau16LogicAddr[u8i], (ISP_U32)gau8Logic_sc035GS[u8Idx2][u8i]);
		}
		//sensor_write_register(IspDev, 0x3812, 0x30);
	}
}

static ISP_VOID cmos_gains_update(ISP_DEV IspDev, ISP_U32 u32Again, ISP_U32 u32Dgain)
{
	static ISP_U32 sau32AGain[ISP_NUM_MAX] = {0xFFFFFFFE};
	static ISP_U32 sau32DGain[ISP_NUM_MAX] = {0xFFFFFFFE};
	ISP_U32 u32Tmp,u32AllGain,tmp[4];
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
		u32AllGain = (ISP_U64)gau16GainTbl_SmatSns[u32Again]*gau16GainTbl_SmatSns[u32Dgain]>>16;
		// 1. Logic
		gainLogic_sc035GS(IspDev, u32AllGain, 0);
		
		// 2.AGainUpdate
		tmp[0] = (u32Again&0x0F) | 0x10;	
		u32Tmp = 0;
		u32Again = u32Again/16;
		for(;u32Again>0;u32Again--)
		{
			u32Tmp = (u32Tmp<<1)|0x01;
		}
		tmp[1] = (u32Tmp<<2)|0x03;
		sensor_write_register(IspDev, 0x3e08, tmp[1]);
		sensor_write_register(IspDev, 0x3e09, tmp[0]);

		// 3.DGainUpdate
		tmp[0] = ((u32Dgain&0x0F)*8) + 0x80;	
		u32Tmp = 0;
		u32Dgain = u32Dgain/16;
		for(;u32Dgain>0;u32Dgain--)
		{
			u32Tmp = (u32Tmp<<1)|0x01;
		}
		tmp[1] = 0x0c|u32Tmp;
		sensor_write_register(IspDev, 0x3e06, tmp[1]);
		sensor_write_register(IspDev, 0x3e07, tmp[0]);
	}
	return;
}

static ISP_VOID cmos_slow_framerate_set(ISP_DEV IspDev, ISP_U16 u16FullLines, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	return;
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
	ISP_U32 u32TotalSizeV,u32Tmp;
	ISP_U32 u32MaxIntTime;
	u32TotalSizeV = (SENSCONT_DVP == gstSnsAttr[IspDev].enCmosIF)?SC035GS_30P_LINES_DVP:SC035GS_30P_LINES_MIPI;
	u32MaxIntTime = SC035GS_30P_LINES_MIPI;
	switch(u8Fps)
	{
		case 30:	u32TotalSizeV = u32TotalSizeV;
					break;
		case 25:	u32TotalSizeV = u32TotalSizeV*30/u8Fps;
					u32MaxIntTime = u32MaxIntTime*30/u8Fps;
					break;
		case 20:
		default:	u8Fps = 20;
					u32TotalSizeV = u32TotalSizeV*30/u8Fps;
					u32MaxIntTime = u32MaxIntTime*30/u8Fps;
					break;
	}
	if(pstAeSnsDft != NULL)
	{
		pstAeSnsDft->u32FullLinesStd = u32TotalSizeV;
		pstAeSnsDft->u32MaxIntTime = (u32MaxIntTime-MAX_SHUT_OFST)*16;
		pstAeSnsDft->u32LinesPer500ms = pstAeSnsDft->u32FullLinesStd*16*u8Fps/2;
	}
	sensor_write_register(IspDev, 0x320e, (u32TotalSizeV>>8)&0xFF);
	sensor_write_register(IspDev, 0x320f, u32TotalSizeV&0xFF);	
	XM_MPI_MIPI_RefreshFV(IspDev,0, u32TotalSizeV);
	if(SENSCONT_DVP == gstSnsAttr[IspDev].enCmosIF)
	{
		u32Tmp = u32TotalSizeV-8;
		sensor_write_register(IspDev, 0x3252, (u32Tmp>>8)&0xFF);
		sensor_write_register(IspDev, 0x3253, u32Tmp&0xFF); 
		
		u32Tmp = u32TotalSizeV-5;
		sensor_write_register(IspDev, 0x3217, (u32Tmp>>8)&0xFF);
		sensor_write_register(IspDev, 0x3218, u32Tmp&0xFF); 
	}
	else
	{
		u32Tmp = u32TotalSizeV-6;
		sensor_write_register(IspDev, 0x3228, (u32Tmp>>8)&0xFF);
		sensor_write_register(IspDev, 0x3229, u32Tmp&0xFF); 
	}
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
	{0, 0, 4096, 3029, 4096, 2435, 1510, 4096},
	{4096, 4096, 0, 0, 3237, 3068, 4096, 1246},
	{-644, -3659, -1477, -4096, -3171, -3896, -2268, -2250},
	213,
	0, 1339, 0, 2439,
	{0, 365, 723, 1074, 1167, 1296, 1518, 1585, 2439, 0, 0, 0, 0, 0, 0, 0},
	{2000, 2150, 2800, 4000, 4150, 5000, 6500, 7500, 12000, 0, 0, 0, 0, 0, 0, 0},
 	{1969, 1024, 1758, 0}
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
 ISP_S32 cmos_init_chn_sc035GS(ISP_DEV IspDev)
{
	return 0;
}
 
ISP_S32 cmos_init_sensor_exp_function_sc035GS(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
	XM_MPI_ISP_Memset((ISP_U8 *)pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));
	pstSensorExpFunc->pfn_cmos_sensor_init = sensor_init;
	pstSensorExpFunc->pfn_cmos_get_isp_default = cmos_get_isp_default;
	pstSensorExpFunc->pfn_cmos_set_pixel_detect = cmos_set_pixel_detect;
	pstSensorExpFunc->pfn_cmos_get_sensor_max_resolution = cmos_get_sensor_max_resolution;
	pstSensorExpFunc->pfn_cmos_set_mirror_flip = cmos_set_mirror_flip;
	return 0;
}

ISP_S32 cmos_init_ae_exp_function_sc035GS(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
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

ISP_S32 cmos_init_awb_exp_function_sc035GS(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	XM_MPI_ISP_Memset((ISP_U8*)pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));
	pstExpFuncs->pfn_cmos_get_awb_default = cmos_get_awb_default;
	return 0;
}








