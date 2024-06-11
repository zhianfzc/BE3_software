#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include "isp_print.h"
#include "autoscene_para.h"
#include "mpi_camera.h"

AUTOSCENE_CONFIG gstConfig;
static AUTOSCENE_DN_S gastAutoSceneDn[ISP_NUM_MAX];
static pthread_t gTaskPid = 0;
static ISP_U8 gu8Loop = 0;
static AUTOSCENE_MENU_S gastAutoSceneMenu[ISP_NUM_MAX];
static ISP_U8 gu8ChnMask = 0;	// bit0:Chn0  bit1:Chn1  bit2:Chn2 (置1:对应Chn使用)
static ISP_S32 (*g_pfun_ae_scane)(ISP_DEV IspDev, ISP_EXT_AUTOSCENE_FUN_PARA_S *pstPara) = NULL;
static ISP_S32 (*g_pfun_debug)(ISP_DEV IspDev, ISP_S32 s32Data, ISP_CHAR *pstrData, ISP_CMOS_DN_PARA_S *pstCmosAgcDn) = NULL;

// s32DnMode: 0:Color  1:BW
static int autoscene_dn(ISP_DEV IspDev, int s32DnMode, ISP_EXT_AUTOSCENE_FUN_PARA_S *pstPara)
{
	const ISP_CMOS_AGC_TABLE_S *pstAgcTab = NULL;
	const ISP_CMOS_VPSS_3DNR_S *pstAgcVpss3DNr = NULL;
	const ISP_CMOS_YCNR_S *pstAgcYcNr = NULL;
	const ISP_CMOS_SHARPEN_S *pstAgcSharp = NULL;
	const ISP_CMOS_GAMMA_S *pstGamma = NULL;
	const ISP_CMOS_LTM_S *pstLtm = NULL;
	const ISP_CMOS_2DNR_S *pstNr2D = NULL;
	ISP_2DNR_ATTR_S stNRAttr;
	ISP_DYDPC_ATTR_S stDyDPAttr;
	ISP_SATURATION_ATTR_S stSatAttr;
	ISP_CSC_ATTR_S stCSCAttr;
	ISP_DRC_ATTR_S stDRC;
	ISP_DEFOG_ATTR_S stDefogAttr;
	ISP_DEFOG_ATTR_S stVpssDefogAttr;
	ISP_BLACKLVL_ATTR_S stBlackLevel;
	ISP_GAMMA_ATTR_S stGammaAttr;
	ISP_3DNR_ATTR_S stVpss3DNrAttr;
	ISP_YCNR_ATTR_S stYcNrAttr;
	ISP_SHARPEN_ATTR_S stSharpenAttr;
	ISP_LTM_ATTR_S stLtmAttr;
	ISP_U8 u8i;
	if(s32DnMode)
	{
		s32DnMode = 1;
		//DEBUG("autoscene chn[%d]: Bw!\n", IspDev);
	}
	else
	{
		//DEBUG("autoscene chn[%d]: color!\n", IspDev);
	}
	pstAgcTab = gastAutoSceneDn[IspDev].stCmosAgcPara.pastAgcTab[s32DnMode];
	pstAgcVpss3DNr = gastAutoSceneDn[IspDev].stCmosAgcPara.pastAgcVpss3DNr[s32DnMode];
	pstAgcYcNr = gastAutoSceneDn[IspDev].stCmosAgcPara.pastAgcYcNr[s32DnMode]; 
	pstAgcSharp = gastAutoSceneDn[IspDev].stCmosAgcPara.pastAgcSharp[s32DnMode]; 
	pstGamma = gastAutoSceneDn[IspDev].stCmosAgcPara.pastGamma[s32DnMode];
	pstLtm = gastAutoSceneDn[IspDev].stCmosAgcPara.pastAgcLtm[s32DnMode];
	pstNr2D = gastAutoSceneDn[IspDev].stCmosAgcPara.past2DNr[s32DnMode];
	if(NULL != pstAgcTab)
	{
		if(XM_MPI_ISP_GetDyDefectPixelAttr(IspDev, &stDyDPAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_GetDyDefectPixelAttr failed!\n");
		}
		if(XM_MPI_ISP_GetSaturationAttr(IspDev, &stSatAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_GetSaturationAttr failed!\n");
		}
		if(XM_MPI_ISP_GetCSCAttr(IspDev, &stCSCAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_GetCSCAttr failed!\n");
		}
		if(XM_MPI_ISP_GetDRCAttr(IspDev, &stDRC) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_GetDRCAttr failed!\n");
		}
		if(XM_MPI_ISP_GetDeFogAttr(IspDev, &stDefogAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_GetDeFogAttr failed!\n");
		}
		if(XM_MPI_ISP_GetVpssDeFogAttr(IspDev, &stVpssDefogAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_GetVpssDeFogAttr failed!\n");
		}
		if(XM_MPI_ISP_GetBlackLevelAttr(IspDev, &stBlackLevel) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_GetBlackLevelAttr failed!\n");
		}
		for(u8i=0;u8i<ISP_AUTO_STENGTH_NUM;u8i++)
		{
			stDRC.stAuto.au8Sth[u8i] = pstAgcTab->au8Drc[u8i];
			stDefogAttr.stAuto.au8Sth[u8i] = pstAgcTab->au8Defog[u8i];
			stDyDPAttr.stAuto.au8Sth[u8i] = pstAgcTab->au8DyDpc[u8i];
			stSatAttr.stAuto.au8Sat[u8i] = pstAgcTab->au8Saturation[u8i];
			stBlackLevel.stAuto.au16Blc[u8i] = pstAgcTab->au16Blc[u8i];
			stCSCAttr.stAuto.au8ContrVal[u8i] = (ISP_U8)(pstAgcTab->au16Contrast[u8i]&0xFF);
			stVpssDefogAttr.stAuto.au8Sth[u8i] = pstAgcTab->au8DefogVpss[u8i];
		}
		if(XM_MPI_ISP_SetDyDefectPixelAttr(IspDev, &stDyDPAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_SetDyDefectPixelAttr failed!\n");
		}
		if(XM_MPI_ISP_SetSaturationAttr(IspDev, &stSatAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_SetSaturationAttr failed!\n");
		}
		if(XM_MPI_ISP_SetCSCAttr(IspDev, &stCSCAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_SetCSCAttr failed!\n");
		}
		if(XM_MPI_ISP_SetDRCAttr(IspDev, &stDRC) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_SetDRCAttr failed!\n");
		}
		if(XM_MPI_ISP_SetDeFogAttr(IspDev, &stDefogAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_SetDeFogAttr failed!\n");
		}
		if(XM_MPI_ISP_SetVpssDeFogAttr(IspDev, &stVpssDefogAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_SetVpssDeFogAttr failed!\n");
		}
		if(XM_MPI_ISP_SetBlackLevelAttr(IspDev, &stBlackLevel) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_SetBlackLevelAttr failed!\n");
		}
	}
	if(NULL != pstNr2D)
	{
		if(XM_MPI_ISP_GetNRAttr(IspDev, &stNRAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_GetNRAttr failed!\n");
		}
		stNRAttr.bEnable = pstNr2D->u8Enable?ISP_TRUE:ISP_FALSE;
		for(u8i=0;u8i<ISP_AUTO_STENGTH_NUM;u8i++)
		{
			stNRAttr.stAuto.au16Thr[u8i] = pstNr2D->au16Thr[u8i];
			stNRAttr.stAuto.au8KThr[u8i] = pstNr2D->au8KThr[u8i];
			stNRAttr.stAuto.au8KCoef[u8i] = pstNr2D->au8KCoef[u8i];
			stNRAttr.stAuto.au8KMin[u8i] = pstNr2D->au8KMin[u8i];
			stNRAttr.stAuto.au8Thr_c[u8i] = pstNr2D->au8Thr_c[u8i];
			stNRAttr.stAuto.au8KThr_c[u8i] = pstNr2D->au8KThr_c[u8i];
			stNRAttr.stAuto.au8KCoef_c[u8i] = pstNr2D->au8KCoef_c[u8i];
			stNRAttr.stAuto.au8KMin_c[u8i] = pstNr2D->au8KMin_c[u8i];
		}
		if(XM_MPI_ISP_SetNRAttr(IspDev, &stNRAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_SetNRAttr failed!\n");
		}
	}
	if(NULL != pstAgcVpss3DNr)
	{
		if(XM_MPI_ISP_GetVpss3DNrAttr(IspDev, &stVpss3DNrAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_GetVpss3DNrAttr failed!\n");
		}
		stVpss3DNrAttr.u8Enable = pstAgcVpss3DNr->u8Enable?(stVpss3DNrAttr.u8Enable|(1<<0)):(stVpss3DNrAttr.u8Enable&~(1<<0));
		for(u8i=0;u8i<ISP_AUTO_STENGTH_NUM;u8i++)
		{
			stVpss3DNrAttr.stAuto.au8FltThr[u8i]=pstAgcVpss3DNr->au8FltThr[u8i];
			stVpss3DNrAttr.stAuto.au8Df_RatC[u8i]=pstAgcVpss3DNr->au8Df_RatC[u8i];
			stVpss3DNrAttr.stAuto.au8Df1_RatioY[u8i]=pstAgcVpss3DNr->au8Df1_RatioY[u8i];
			stVpss3DNrAttr.stAuto.au8Df1_RatioC[u8i]=pstAgcVpss3DNr->au8Df1_RatioC[u8i];
			stVpss3DNrAttr.stAuto.au8MovThr_Max[u8i]=pstAgcVpss3DNr->au8MovThr_Max[u8i];
			stVpss3DNrAttr.stAuto.au8Alpha_Ratio[u8i]=pstAgcVpss3DNr->au8Alpha_Ratio[u8i];
			stVpss3DNrAttr.stAuto.au8Alpha_Min[u8i]=pstAgcVpss3DNr->au8Alpha_Min[u8i];
			stVpss3DNrAttr.stAuto.au8IdxThr_Max[u8i]=pstAgcVpss3DNr->au8IdxThr_Max[u8i];
			stVpss3DNrAttr.stAuto.au8IdxThr_Min[u8i]=pstAgcVpss3DNr->au8IdxThr_Min[u8i];
			stVpss3DNrAttr.stAuto.au8Idx_Max[u8i]=pstAgcVpss3DNr->au8Idx_Max[u8i];
			stVpss3DNrAttr.stAuto.au8FidY[u8i]=pstAgcVpss3DNr->au8FidY[u8i];
			stVpss3DNrAttr.stAuto.au8FidC[u8i]=pstAgcVpss3DNr->au8FidC[u8i];
			stVpss3DNrAttr.stAuto.au8FidY_Min[u8i]=pstAgcVpss3DNr->au8FidY_Min[u8i];
			stVpss3DNrAttr.stAuto.au8FidC_Min[u8i]=pstAgcVpss3DNr->au8FidC_Min[u8i];
			stVpss3DNrAttr.stAuto.au8YSfThr_Min[u8i]=pstAgcVpss3DNr->au8YSfThr_Min[u8i];
			stVpss3DNrAttr.stAuto.au8CSfThr_Min[u8i]=pstAgcVpss3DNr->au8CSfThr_Min[u8i];
			stVpss3DNrAttr.stAuto.au8YTfThr_Min[u8i]=pstAgcVpss3DNr->au8YTfThr_Min[u8i];
			stVpss3DNrAttr.stAuto.au8CTfThr_Min[u8i]=pstAgcVpss3DNr->au8CTfThr_Min[u8i];
			stVpss3DNrAttr.stAuto.au16YSfThr_K[u8i]=pstAgcVpss3DNr->au16YSfThr_K[u8i];
			stVpss3DNrAttr.stAuto.au16CSfThr_K[u8i]=pstAgcVpss3DNr->au16CSfThr_K[u8i];
			stVpss3DNrAttr.stAuto.au16YTfThr_K[u8i]=pstAgcVpss3DNr->au16YTfThr_K[u8i];
			stVpss3DNrAttr.stAuto.au16CTfThr_K[u8i]=pstAgcVpss3DNr->au16CTfThr_K[u8i];
			stVpss3DNrAttr.stAuto.au16MovThr1[u8i]=pstAgcVpss3DNr->au16MovThr1[u8i];
			stVpss3DNrAttr.stAuto.au16MovThr2[u8i]=pstAgcVpss3DNr->au16MovThr2[u8i];
			stVpss3DNrAttr.stAuto.au16MovThr3[u8i]=pstAgcVpss3DNr->au16MovThr3[u8i];
			stVpss3DNrAttr.stAuto.au8YAntiRsdlSpd[u8i]=pstAgcVpss3DNr->au8YAntiRsdlSpd[u8i];
			stVpss3DNrAttr.stAuto.au8CAntiRsdlSpd[u8i]=pstAgcVpss3DNr->au8CAntiRsdlSpd[u8i];
			stVpss3DNrAttr.stAuto.au8YAntiRsdlThr[u8i]=pstAgcVpss3DNr->au8YAntiRsdlThr[u8i];
			stVpss3DNrAttr.stAuto.au8CAntiRsdlThr[u8i]=pstAgcVpss3DNr->au8CAntiRsdlThr[u8i];
		}
		if(XM_MPI_ISP_SetVpss3DNrAttr(IspDev, &stVpss3DNrAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_SetVpss3DNrAttr failed!\n");
		}
	}
	if(NULL != pstAgcYcNr)
	{
		if(XM_MPI_ISP_GetYcNRAttr(IspDev, &stYcNrAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_GetYcNRAttr failed!\n");
		}
		stYcNrAttr.bEnable = (pstAgcYcNr->u8Enable&(1<<0))?ISP_TRUE:ISP_FALSE;
		for(u8i=0;u8i<ISP_AUTO_STENGTH_NUM;u8i++)
		{
			stYcNrAttr.stAuto.au8Thr[u8i] = pstAgcYcNr->au8Thr[u8i];
			stYcNrAttr.stAuto.au8Str[u8i] = pstAgcYcNr->au8Str[u8i];
			stYcNrAttr.stAuto.au8MixThr[u8i] = pstAgcYcNr->au8MixThr[u8i];
			stYcNrAttr.stAuto.au8Wt1[u8i] = pstAgcYcNr->au8Wt1[u8i];
			stYcNrAttr.stAuto.au8Wt2[u8i] = pstAgcYcNr->au8Wt2[u8i];
			stYcNrAttr.stAuto.au8Wt3[u8i] = pstAgcYcNr->au8Wt3[u8i];
			stYcNrAttr.stAuto.au16K[u8i] = pstAgcYcNr->au16K[u8i];
		}
		if(XM_MPI_ISP_SetYcNRAttr(IspDev, &stYcNrAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_SetYcNRAttr failed!\n");
		}
	}
	if(NULL != pstAgcSharp)
	{
		if(XM_MPI_ISP_GetSharpenAttr(IspDev, &stSharpenAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_GetSharpenAttr failed!\n");
		}
		stSharpenAttr.bEnable = pstAgcSharp->u8Enable?ISP_TRUE:ISP_FALSE;
		for(u8i=0;u8i<ISP_AUTO_STENGTH_NUM;u8i++)
		{
			stSharpenAttr.stAuto.au8Ud[u8i] = pstAgcSharp->au8Ud[u8i];
			stSharpenAttr.stAuto.au8D[u8i] = pstAgcSharp->au8D[u8i];
			stSharpenAttr.stAuto.au8ThrUd[u8i] = pstAgcSharp->au8ThrUd[u8i];
			stSharpenAttr.stAuto.au8ThrD[u8i] = pstAgcSharp->au8ThrD[u8i];
			stSharpenAttr.stAuto.au8Ud_H[u8i] = pstAgcSharp->au8Ud_H[u8i];
			stSharpenAttr.stAuto.au8D_H[u8i] = pstAgcSharp->au8D_H[u8i];
			stSharpenAttr.stAuto.au8ThrUd_H[u8i] = pstAgcSharp->au8ThrUd_H[u8i];
			stSharpenAttr.stAuto.au8ThrD_H[u8i] = pstAgcSharp->au8ThrD_H[u8i];
			stSharpenAttr.stAuto.au8EdgeStr[u8i] = pstAgcSharp->au8EdgeStr[u8i];
			stSharpenAttr.stAuto.au8EdgeThr[u8i] = pstAgcSharp->au8EdgeThr[u8i];
			stSharpenAttr.stAuto.au8OverShoot[u8i] = pstAgcSharp->au8OverShoot[u8i];
			stSharpenAttr.stAuto.au8UnderShoot[u8i] = pstAgcSharp->au8UnderShoot[u8i];
			stSharpenAttr.stAuto.au8DetailStr[u8i] = pstAgcSharp->au8DetailStr[u8i];
			stSharpenAttr.stAuto.au8Rsv1[u8i] = pstAgcSharp->au8Rsv1[u8i];
			stSharpenAttr.stAuto.au8Rsv2[u8i] = pstAgcSharp->au8Rsv2[u8i];
		}
		if(XM_MPI_ISP_SetSharpenAttr(IspDev, &stSharpenAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_SetSharpenAttr failed!\n");
		}
	}
	if(NULL != pstGamma)
	{
		if(XM_MPI_ISP_GetGammaAttr(IspDev, &stGammaAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_GetGammaAttr failed!\n");
		}
		for(u8i=0;u8i<GAMMA_NODE_NUMBER;u8i++)
		{
			stGammaAttr.u16Table[u8i] = pstGamma->au16Gamma[u8i];
		}
		if(XM_MPI_ISP_SetGammaAttr(IspDev, &stGammaAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_SetGammaAttr failed!\n");
		}
	}
	if(NULL != pstLtm)
	{
		if(XM_MPI_ISP_GetLtmAttr(IspDev, &stLtmAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_GetLtmAttr failed!\n");
		}
		stLtmAttr.bEnable = pstLtm->u8Enable?ISP_TRUE:ISP_FALSE;
		for(u8i=0;u8i<ISP_AUTO_STENGTH_NUM;u8i++)
		{
			stLtmAttr.stAuto.au8Rix[u8i] = pstLtm->au8Rix[u8i];
			stLtmAttr.stAuto.au8KjMax[u8i] = pstLtm->au8KjMax[u8i];
			stLtmAttr.stAuto.au8KjMin[u8i] = pstLtm->au8KjMin[u8i];
			stLtmAttr.stAuto.au8StrD[u8i] = pstLtm->au8StrD[u8i];
			stLtmAttr.stAuto.au8StrB[u8i] = pstLtm->au8StrB[u8i];
		}
		if(XM_MPI_ISP_SetLtmAttr(IspDev, &stLtmAttr) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_SetLtmAttr failed!\n");
		}
	}
	return 0;
}

static ISP_S32 autoscene_get_info(ISP_DEV IspDev, ISP_EXT_AUTOSCENE_FUN_PARA_S *pstPara)
{
	AUTOSCENE_DN_STATUS_E eDnStatus=AS_DN_STATUS_COLOR;
	ISP_INNER_STATE_INFO_S stInnerStateInfo;
	// 1. day night
	ISPCHECK_RET(dnc_get_dn_status(IspDev, &eDnStatus), "dnc_get_dn_status");
	pstPara->u32DnMode = (ISP_U32)eDnStatus;

	// 2. ae info
	if(ISP_SUCCESS != XM_MPI_ISP_QueryInnerStateInfo(IspDev, &stInnerStateInfo))
	{
		ERR("XM_MPI_ISP_QueryInnerStateInfo failed!\n");
		return ISP_FAILURE;
	}
	pstPara->u32Exposure = 	stInnerStateInfo.u32Exposure;
	pstPara->u32Shut = stInnerStateInfo.u32ExposureTime;
	pstPara->u32AllGain = stInnerStateInfo.u32AllGain;

	// 3. others
	AUTOSCENE_INNER_INFO stInfo;
	ISPCHECK_RET(dnc_get_inner_status(IspDev, &stInfo), "dnc_get_inner_status");
	pstPara->u32AiStrategyStr = (ISP_U32)(stInfo.u8AiStrategyStr);

	ISP_S32 s32CaptureSize;
	fvideo_get_capture_size(IspDev,&s32CaptureSize);
	pstPara->au32Rsv[0] = (ISP_U32)(s32CaptureSize);
	return ISP_SUCCESS;
}

static ISP_S32 autoscene_ae_scane(ISP_DEV IspDev, ISP_EXT_AUTOSCENE_FUN_PARA_S *pstPara)
{
	static ISP_U8 sau8LumTgtLst[ISP_NUM_MAX];
	static ISP_U16 sau16AEStrategy[ISP_NUM_MAX] = {0xFFFE};
	static ISP_U32 sau32SysGainMax[ISP_NUM_MAX];
	ISP_U32 u32WebLevel;
	ISP_U8 u8LumTarget;
	ISP_S32 s32DefLum,s32Tmp;
	ISP_U8 u8Idx,u8Wgt,u8i,u8Num;
	ISP_U32 u32Tmp,u32GainDef,u32GainMax,u32SysGainMax;
	ISP_U8 *pu8Lum;
	ISP_U16 *pu16RatioSlope;
	ISP_U8 u8DnStatus,u8AEStrategy,u8AEStrategyStr;
	AUTOSCENE_MENU_S *pstMenu;
	AUTOSCENE_DN_S *pstDnPara;
	if(NULL == pstPara)
	{
		return ISP_FAILURE;
	}
	if(sau16AEStrategy[0] == 0xFFFE)	// first
	{
		for(u8i=0; u8i<ISP_NUM_MAX; u8i ++)
		{
			sau32SysGainMax[u8i] = 0;
			sau16AEStrategy[u8i] = 0xFFFF;
			sau8LumTgtLst[u8i] = 0xFF;
		}
	}
	u8DnStatus = pstPara->u32DnMode?DNS_BW:DNS_COLOR;
	pstMenu = &gastAutoSceneMenu[IspDev];
	pstDnPara = &gastAutoSceneDn[IspDev];
	// ae lum
	u8Num = pstDnPara->stAePara.u8AeLumNum;
	u32WebLevel = (ISP_U32)(pstMenu->u8AeLum);
	if(u8DnStatus)	// BW
	{
		pu8Lum = pstDnPara->stAePara.au8AeLumBw;
		pu16RatioSlope = pstDnPara->stAePara.au16RatioSlopeBw;
		u8AEStrategy = pstDnPara->stAePara.au8AeStrategyDef[DNS_BW];
		u8AEStrategyStr = pstDnPara->stAePara.au8AeStrategyStrDef[DNS_BW];
	}
	else
	{
		pu8Lum = pstDnPara->stAePara.au8AeLumColor;
		pu16RatioSlope = pstDnPara->stAePara.au16RatioSlopeColor;
		u8AEStrategy = pstDnPara->stAePara.au8AeStrategyDef[DNS_COLOR];
		u8AEStrategyStr = pstDnPara->stAePara.au8AeStrategyStrDef[DNS_COLOR];
	}
	u8LumTarget = 50;
	if(u8Num > 0)
	{
		if((u8Num<2) || (pstPara->u32Exposure <= pstDnPara->stAePara.au32ExpThr[0]))
		{
			u8Idx = 0;
			u8Wgt = 0xFF;
		}
		else if(pstPara->u32Exposure >= pstDnPara->stAePara.au32ExpThr[u8Num-1])
		{
			u8Idx = u8Num-1;
			u8Wgt = 0xFF;
		}
		else
		{
			for(u8i=1; u8i<u8Num-1; u8i ++)
			{
				if(pstDnPara->stAePara.au32ExpThr[u8i]>=pstPara->u32Exposure)
					break;
			}
			u8Idx = u8i - 1;
			s32Tmp = (ISP_S32)pstDnPara->stAePara.au32ExpThr[u8Idx+1]-pstDnPara->stAePara.au32ExpThr[u8Idx];
			u32Tmp = (s32Tmp<1)?0xFF:(pstDnPara->stAePara.au32ExpThr[u8Idx+1]-pstPara->u32Exposure)*0xFF/s32Tmp;
			u8Wgt = CLIP3(u32Tmp,0,0xFF);
		}
		s32DefLum = (u8Wgt == 0xFF)?*(pu8Lum+u8Idx):
					((ISP_U32)u8Wgt*(*(pu8Lum+u8Idx))+(ISP_U32)(0xFF-u8Wgt)*(*(pu8Lum+u8Idx+1)))/0xFF;
		u8LumTarget = (u32WebLevel < 50)?s32DefLum*u32WebLevel/50:s32DefLum+s32DefLum*(u32WebLevel -50)/50;

		if(pstDnPara->stAePara.u8RatioSlopeEnable == 1)
		{
			u8AEStrategyStr = (u8Wgt == 0xFF)?*(pu16RatioSlope+u8Idx):
				((ISP_U32)u8Wgt*(*(pu16RatioSlope+u8Idx))+(ISP_U32)(0xFF-u8Wgt)*(*(pu16RatioSlope+u8Idx+1)))/0xFF;
		}
	}
	// gain
	u32WebLevel = (ISP_U32)(pstMenu->u8AeGainMax);
	u32GainDef = pstDnPara->stAePara.au32GainDef[u8DnStatus];
	u32GainMax = pstDnPara->stAePara.au32GainMax[u8DnStatus];
	u32SysGainMax = (u32WebLevel<=50)?(512*(50-u32WebLevel)+u32GainDef*u32WebLevel)/50:(u32GainDef*(100-u32WebLevel)+u32GainMax*(u32WebLevel-50))/50;

	// ae strategy
	if((pstMenu->u8Blc) && (DNS_COLOR == u8DnStatus))	// BLC
	{
		u8AEStrategy = AE_EXP_LOWLIGHT_PRIOR;
		u8AEStrategyStr = 100;
	}
	u32Tmp = (pstPara->u32AiStrategyStr&0xFF);
	u8AEStrategyStr = MIN2(u8AEStrategyStr,0xFF);
	u32Tmp = (u32Tmp<50)?u32Tmp*u8AEStrategyStr/50:(ISP_U32)u8AEStrategyStr+(200-u8AEStrategyStr)*(u32Tmp-50)/50; 
	u8AEStrategyStr = MIN2(u32Tmp, 0xFF);
	u32Tmp = ((ISP_U32)u8AEStrategy<<8) | u8AEStrategyStr;
	if((sau8LumTgtLst[IspDev] == u8LumTarget) && (sau32SysGainMax[IspDev] == u32SysGainMax)&&(sau16AEStrategy[IspDev] == u32Tmp))
	{
		return ISP_SUCCESS;
	}

	ISP_EXPOSURE_ATTR_S  pstExpAttr;
	ISP_S32 s32Ret;
	s32Ret = XM_MPI_ISP_GetExposureAttr(IspDev, &pstExpAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_GetExposureAttr failed!\n");
		return s32Ret;
	}
	pstExpAttr.stAuto.u8Compensation = u8LumTarget;
	if(sau16AEStrategy[IspDev] != u32Tmp)
	{
		pstExpAttr.stAuto.u16AEStrategyStr = u8AEStrategyStr;
		pstExpAttr.stAuto.enAEStrategyMode = u8AEStrategy;
	}

	pstExpAttr.stAuto.stSysGainRange.u32Max = u32SysGainMax<<2;
	s32Ret = XM_MPI_ISP_SetExposureAttr(IspDev, &pstExpAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_SetExposureAttr failed!\n");
		return s32Ret;
	}
	sau16AEStrategy[IspDev] = u32Tmp&0xFFFF;
	sau8LumTgtLst[IspDev] = u8LumTarget;
	sau32SysGainMax[IspDev] = u32SysGainMax;
	return ISP_SUCCESS;
}


static int autoscene_task(ISP_VOID)
{
	ISP_EXT_AUTOSCENE_FUN_PARA_S stPara;
	ISP_U32 au32DnStatusLst[ISP_NUM_MAX];
	ISP_DEV ispDev;
	ISP_MODULE_DEBUG_U stDebug;
	AUTOSCENE_PARA_ADDR_S stAddr;
	XM_MPI_ISP_Memset(&stAddr,0,sizeof(AUTOSCENE_PARA_ADDR_S));
	
	//prctl(PR_SET_NAME,(unsigned long)"autoscene_task");
	for(ispDev=0;ispDev<ISP_NUM_MAX;ispDev ++)
	{
		au32DnStatusLst[ispDev] = 0xFF;
	}
	gu8Loop = 1;
	while(gu8Loop)
	{
		for(ispDev=0;ispDev<ISP_NUM_MAX;ispDev++)
		{
			stAddr.au32Addr[0] = (ISP_U32)&gastAutoSceneDn[ispDev];
			ISPCHECK_RET(XM_MPI_ISP_GetDebug(ispDev,&stDebug), "XM_MPI_ISP_GetDebug");
			if(stDebug.bitBypassAScene)
			{
				continue;
			}
			if(gu8ChnMask&(1<<ispDev))
			{
				// 1. get info
				autoscene_get_info(ispDev, &stPara);
				// 2. ae scane
				if(g_pfun_ae_scane)
				{
					(*g_pfun_ae_scane)(ispDev, &stPara);
				}
				// 3. dn para switch
				if((au32DnStatusLst[ispDev] != stPara.u32DnMode)||((stPara.au32Rsv[1]&(1<<ispDev)) == 1))
				{
					au32DnStatusLst[ispDev] = stPara.u32DnMode;
					stPara.au32Rsv[1] &= ~(1<<ispDev);
					autoscene_dn(ispDev, stPara.u32DnMode, &stPara);
				}
				// 4. others
				if(gastAutoSceneDn[ispDev].pfun_autoscene_sns)
				{
					(*gastAutoSceneDn[ispDev].pfun_autoscene_sns)(ispDev, &stPara, stAddr);
				}
			}
		}
		XM_MPI_ISP_USleep(200000);
	}
	return 0;
}

static ISP_S32 common_init()
{
	static volatile ISP_U8 su8First = 1;
	ISP_DEV ispDev;
	if(!su8First)
	{
		return ISP_SUCCESS;
	}
	su8First = 0;
	XM_MPI_ISP_Memset((ISP_U8*)gastAutoSceneDn,0,sizeof(gastAutoSceneDn));
	g_pfun_ae_scane = autoscene_ae_scane;
	for(ispDev=0;ispDev<ISP_NUM_MAX;ispDev ++)
	{
		gastAutoSceneMenu[ispDev].u8AeLum = 50;
		gastAutoSceneMenu[ispDev].u8AeGainMax = 50;
		gastAutoSceneMenu[ispDev].u8Blc = 0;
		gastAutoSceneMenu[ispDev].u8Hlc = 0;
		
		gastAutoSceneDn[ispDev].stAePara.u8AeLumNum = 1;
		gastAutoSceneDn[ispDev].stAePara.au8AeLumColor[0] = 55;
		gastAutoSceneDn[ispDev].stAePara.au8AeLumBw[0] = 55;
		
		gastAutoSceneDn[ispDev].stAePara.au32GainDef[0] = 62*256; // color
		gastAutoSceneDn[ispDev].stAePara.au32GainMax[0] = 128*256;	// color
		gastAutoSceneDn[ispDev].stAePara.au32GainDef[1] = gastAutoSceneDn[ispDev].stAePara.au32GainDef[0];	// Bw
		gastAutoSceneDn[ispDev].stAePara.au32GainMax[1] = gastAutoSceneDn[ispDev].stAePara.au32GainMax[0];	// Bw

		gastAutoSceneDn[ispDev].stAePara.au8AeStrategyDef[0] = AE_EXP_LOWLIGHT_PRIOR;
		gastAutoSceneDn[ispDev].stAePara.au8AeStrategyStrDef[0] = 0x00;
		gastAutoSceneDn[ispDev].stAePara.au8AeStrategyDef[1] = gastAutoSceneDn[ispDev].stAePara.au8AeStrategyDef[0];
		gastAutoSceneDn[ispDev].stAePara.au8AeStrategyStrDef[1] = gastAutoSceneDn[ispDev].stAePara.au8AeStrategyStrDef[0];
	}
	return ISP_SUCCESS;
}

ISP_S32 XM_MPI_AUTOSCENE_SetConfig(AUTOSCENE_CONFIG stConfig)
{
	dnc_set_config(&stConfig);
	return ISP_SUCCESS;
}


/*************************************************************************
函数功能:		libautoscene初始化(注册)
输入参数: 		IspDev: ISP设备号
			stProductInfo:  产品相关信息（用于区分不同产品）
输出参数:		无
返回参数: 		0:成功    		-1:失败
note:
*************************************************************************/
ISP_S32 XM_MPI_AUTOSCENE_Init(ISP_DEV IspDev,AUTOSCENE_PRODUCT_INFO_S stProductInfo)
{
	AUTOSCENE_PARA_ADDR_S stAddr;
	XM_MPI_ISP_Memset(&stAddr,0,sizeof(AUTOSCENE_PARA_ADDR_S));
	stAddr.au32Addr[0] = (ISP_U32)&gastAutoSceneDn[IspDev];
	common_init();
	switch(stProductInfo.u32SnsType)
	{
		case SENSOR_CHIP_SC401AI:
			autoscene_init_sc401ai(IspDev,stProductInfo.u32ProductType,stAddr);
			g_pfun_debug = autoscene_debug_sc401ai;
			break;
		case SENSOR_CHIP_SC4336:
			autoscene_init_sc4336(IspDev,stProductInfo.u32ProductType,stAddr);
			break;
		case SENSOR_CHIP_IMX307:
			autoscene_init_imx307(IspDev,stProductInfo.u32ProductType,stAddr);
			break;
		case SENSOR_CHIP_IMX335:
			autoscene_init_imx335(IspDev,stProductInfo.u32ProductType,stAddr);
			break;
		case SENSOR_CHIP_SC3335:
			autoscene_init_sc3335(IspDev,stProductInfo.u32ProductType,stAddr);
			break;
		case SENSOR_CHIP_SC3338:
			autoscene_init_sc3338(IspDev,stProductInfo.u32ProductType,stAddr);
			break;
		case SENSOR_CHIP_SC223A:
			autoscene_init_sc223a(IspDev,stProductInfo.u32ProductType,stAddr);
			break;
		case SENSOR_CHIP_SC1346:
			autoscene_init_sc1346(IspDev,stProductInfo.u32ProductType,stAddr);
			break;
		case SENSOR_CHIP_SC500AI:
			autoscene_init_sc500ai(IspDev,stProductInfo.u32ProductType,stAddr);
			break;
		case SENSOR_CHIP_SC035GS:
			autoscene_init_sc035gs(IspDev,stProductInfo.u32ProductType,stAddr);
			break;
		case SENSOR_CHIP_SC307E:
			autoscene_init_sc307e(IspDev,stProductInfo.u32ProductType,stAddr);
			break;
		case SENSOR_CHIP_SC2335:
			autoscene_init_sc2335(IspDev,stProductInfo.u32ProductType,stAddr);
			break;
		case SENSOR_CHIP_SC2336:
			autoscene_init_sc2336(IspDev,stProductInfo.u32ProductType,stAddr);
			break;
		case SENSOR_CHIP_SC200AI:
			autoscene_init_sc200ai(IspDev,stProductInfo.u32ProductType,stAddr);
			break;
		case SENSOR_CHIP_SC202CS:
			autoscene_init_sc202cs(IspDev,stProductInfo.u32ProductType,stAddr);
			break;
		default:
			break;
	}
	ISPCHECK_RET(dnc_init(IspDev, &gastAutoSceneDn[IspDev].stAePara), "dnc_init");
	return ISP_SUCCESS;
}

ISP_S32 XM_MPI_AUTOSCENE_AI_Face_Lum_Double(ISP_U32 u32flag,ISP_U32 u32Level,ISP_U32 u32Max,ISP_U32 u32Min)
{
	static ISP_S8 s8Num = 6;
	static ISP_S8 s8flag = 0xff;
	ISP_EXPOSURE_ATTR_S  pstExpAttr;
	ISP_S32 s32Ret;
	if(u32Level>256)
		return ISP_SUCCESS;

	if(u32Level < u32Min)
	{
		s8Num++;
	}
	else if(u32Level > u32Max)
	{
		s8Num--;
	}
	s8Num = (s8Num>3)?3:s8Num;
	s8Num = (s8Num<0)?0:s8Num;
	if(s8flag != s8Num)
	{
		s8flag = s8Num;
		switch(s8Num)
		{
			case 0:
				gastAutoSceneDn[0].stAePara.au8AeStrategyDef[1] = AE_EXP_HIGHLIGHT_PRIOR;
				gastAutoSceneDn[0].stAePara.au8AeStrategyStrDef[1] = 0x80;
				gastAutoSceneDn[0].stAePara.au8AeLumBw[0] = 0x20;
				gastAutoSceneDn[0].stAePara.au8AeLumBw[1] = 0x20;
				gastAutoSceneDn[1].stAePara.au8AeStrategyDef[1] = AE_EXP_HIGHLIGHT_PRIOR;
				gastAutoSceneDn[1].stAePara.au8AeStrategyStrDef[1] = 0x80;
				gastAutoSceneDn[1].stAePara.au8AeLumBw[0] = 0x20;
				gastAutoSceneDn[1].stAePara.au8AeLumBw[1] = 0x20;
				break;
			case 1:
				gastAutoSceneDn[0].stAePara.au8AeStrategyDef[1] = AE_EXP_NORMAL;
				gastAutoSceneDn[0].stAePara.au8AeStrategyStrDef[1] = 0x20;
				gastAutoSceneDn[0].stAePara.au8AeLumBw[0] = 0x30;
				gastAutoSceneDn[0].stAePara.au8AeLumBw[1] = 0x30;
				gastAutoSceneDn[1].stAePara.au8AeStrategyDef[1] = AE_EXP_NORMAL;
				gastAutoSceneDn[1].stAePara.au8AeStrategyStrDef[1] = 0x20;
				gastAutoSceneDn[1].stAePara.au8AeLumBw[0] = 0x30;
				gastAutoSceneDn[1].stAePara.au8AeLumBw[1] = 0x30;
				break;
			case 2:
				gastAutoSceneDn[0].stAePara.au8AeStrategyDef[1] = AE_EXP_LOWLIGHT_PRIOR;
				gastAutoSceneDn[0].stAePara.au8AeStrategyStrDef[1] = 0x60;
				gastAutoSceneDn[0].stAePara.au8AeLumBw[0] = 0x38;
				gastAutoSceneDn[0].stAePara.au8AeLumBw[1] = 0x38;
				gastAutoSceneDn[1].stAePara.au8AeStrategyDef[1] = AE_EXP_LOWLIGHT_PRIOR;
				gastAutoSceneDn[1].stAePara.au8AeStrategyStrDef[1] = 0x60;
				gastAutoSceneDn[1].stAePara.au8AeLumBw[0] = 0x38;
				gastAutoSceneDn[1].stAePara.au8AeLumBw[1] = 0x38;
				break;
			case 3:
				gastAutoSceneDn[0].stAePara.au8AeStrategyDef[1] = AE_EXP_LOWLIGHT_PRIOR;
				gastAutoSceneDn[0].stAePara.au8AeStrategyStrDef[1] = 0xa0;
				gastAutoSceneDn[0].stAePara.au8AeLumBw[0] = 0x40;
				gastAutoSceneDn[0].stAePara.au8AeLumBw[1] = 0x40;
				gastAutoSceneDn[1].stAePara.au8AeStrategyDef[1] = AE_EXP_LOWLIGHT_PRIOR;
				gastAutoSceneDn[1].stAePara.au8AeStrategyStrDef[1] = 0xa0;
				gastAutoSceneDn[1].stAePara.au8AeLumBw[0] = 0x40;
				gastAutoSceneDn[1].stAePara.au8AeLumBw[1] = 0x40;
				break;
			default:
				printf("XM_MPI_AUTOSCENE_AI_Face_Lum_Double Err\n");
				break;
		}
		ISP_EXPOSURE_ATTR_S  pstExpAttr;
		ISP_S32 s32Ret;
		s32Ret = XM_MPI_ISP_GetExposureAttr(0, &pstExpAttr);
		if(s32Ret != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_GetExposureAttr failed!\n");
			return s32Ret;
		}
		//pstExpAttr.stAuto.u8Compensation = gastAutoSceneDn[0].stAePara.au8AeLumBw[0];
		//pstExpAttr.stAuto.u16AEStrategyStr = gastAutoSceneDn[0].stAePara.au8AeStrategyStrDef[1];
		//pstExpAttr.stAuto.enAEStrategyMode = gastAutoSceneDn[0].stAePara.au8AeStrategyDef[1];
		s32Ret = XM_MPI_ISP_SetExposureAttr(0, &pstExpAttr);
		if(s32Ret != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_SetExposureAttr failed!\n");
			return s32Ret;
		}
	}
	return ISP_SUCCESS;
}

ISP_S32 XM_MPI_AUTOSCENE_Run(ISP_U8 u8ChnMask, ISP_S32 s32Rsv)
{
	int s32Ret;
	if(gTaskPid)
	{
		return ISP_FAILURE;
	}
	gu8ChnMask = u8ChnMask;
	s32Ret = pthread_create(&gTaskPid, NULL, (void *)autoscene_task, NULL);
	if(s32Ret != 0)
	{
		ERR("pthread_create autoscene_task failed!\n");
	}
	return s32Ret;
}

ISP_S32 XM_MPI_AUTOSCENE_Exit(ISP_DEV IspDev)
{
	gu8Loop = 0;
	dnc_exit(IspDev);
	if(gTaskPid)
	{
		pthread_join (gTaskPid, NULL);
	}
	DEBUG("autoscene exit!\n");
	return ISP_SUCCESS;
}

ISP_S32 XM_MPI_AUTOSCENE_GetLumTarget(ISP_DEV IspDev, ISP_S32 *ps32Tgt)
{
	common_init();
	if(ps32Tgt == NULL)
	{
		return ISP_FAILURE;
	}
	*ps32Tgt = gastAutoSceneMenu[IspDev].u8AeLum;
	return  ISP_SUCCESS;
}

ISP_S32 XM_MPI_AUTOSCENE_SetLumTarget(ISP_DEV IspDev, ISP_S32 s32Tgt)
{
	common_init();
	gastAutoSceneMenu[IspDev].u8AeLum = CLIP3(s32Tgt,0,100);
	return ISP_SUCCESS;
}

ISP_S32 XM_MPI_AUTOSCENE_GetGainLvl(ISP_DEV IspDev, ISP_S32 *ps32En, ISP_S32 *ps32Level)
{
	common_init();
	if(ps32En == NULL || ps32Level == NULL)
	{
		return ISP_FAILURE;
	}
	*ps32En = gastAutoSceneMenu[IspDev].u8AeGainMax?1:0;
	*ps32Level = (ISP_S32)(gastAutoSceneMenu[IspDev].u8AeGainMax);
	return  ISP_SUCCESS;
}

ISP_S32 XM_MPI_AUTOSCENE_SetGainLvl(ISP_DEV IspDev, ISP_S32 s32En, ISP_S32 s32Level)
{
	common_init();
	s32Level = !s32En?0:s32Level;
	gastAutoSceneMenu[IspDev].u8AeGainMax = (ISP_U8)CLIP3(s32Level,0,100);
	return ISP_SUCCESS;
}

ISP_S32 XM_MPI_AUTOSCENE_GetBlc(ISP_DEV IspDev, ISP_S32 *ps32En, ISP_S32 *ps32Level)
{
	common_init();
	if(ps32En == NULL || ps32Level == NULL)
	{
		return ISP_FAILURE;
	}
	*ps32En = gastAutoSceneMenu[IspDev].u8Blc?1:0;
	*ps32Level = (ISP_S32)(gastAutoSceneMenu[IspDev].u8Blc);
	return  ISP_SUCCESS;
}

ISP_S32 XM_MPI_AUTOSCENE_SetBlc(ISP_DEV IspDev, ISP_S32 s32En, ISP_S32 s32Level)
{
	common_init();
	s32Level = !s32En?0:s32Level;
	gastAutoSceneMenu[IspDev].u8Blc = (ISP_U8)CLIP3(s32Level,0,100);
	return ISP_SUCCESS;
}

ISP_S32 XM_MPI_AUTOSCENE_GetHlc(ISP_DEV IspDev, ISP_S32 *ps32En, ISP_S32 *ps32Level)
{
	common_init();
	if(ps32En == NULL || ps32Level == NULL)
	{
		return ISP_FAILURE;
	}
	*ps32En = gastAutoSceneMenu[IspDev].u8Hlc?1:0;
	*ps32Level = (ISP_S32)(gastAutoSceneMenu[IspDev].u8Hlc);
	return  ISP_SUCCESS;
}

ISP_S32 XM_MPI_AUTOSCENE_SetHlc(ISP_DEV IspDev, ISP_S32 s32En, ISP_S32 s32Level)
{
	common_init();
	s32Level = !s32En?0:s32Level;
	gastAutoSceneMenu[IspDev].u8Hlc = (ISP_U8)CLIP3(s32Level,0,100);
	return ISP_SUCCESS;
}

ISP_S32 XM_MPI_AUTOSCENE_Debug(ISP_DEV IspDev, ISP_S32 s32Data, ISP_CHAR *pstrData)
{
	if(g_pfun_debug)
	{
		return (*g_pfun_debug)(IspDev, s32Data, pstrData, &(gastAutoSceneDn[IspDev].stCmosAgcPara));
	}
	return ISP_SUCCESS;
}

