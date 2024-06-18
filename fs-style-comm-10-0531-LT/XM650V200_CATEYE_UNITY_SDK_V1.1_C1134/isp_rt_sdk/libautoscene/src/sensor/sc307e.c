#include "autoscene_para.h"
//sensor: SC307E(SC2315E)


static ISP_S32 fun_autoscene_sc307e(ISP_DEV IspDev,ISP_EXT_AUTOSCENE_FUN_PARA_S *pstPara, AUTOSCENE_PARA_ADDR_S stAddr)
{
	return ISP_SUCCESS;
}

/*************************************************************************
函数功能:		参数初始化
输入参数: 		u32ProductType:  产品型号（用于区分不同产品）
输出参数:		stAddr:   		配置相关
返回参数: 		0:成功    		-1:失败
note:
*************************************************************************/
ISP_S32 autoscene_init_sc307e(			ISP_DEV IspDev,
									ISP_U32 u32ProductType,
									AUTOSCENE_PARA_ADDR_S stAddr)
{
	AUTOSCENE_DN_S *pstAutoSceneDn = (AUTOSCENE_DN_S *)stAddr.au32Addr[0];
	if(pstAutoSceneDn==NULL)
	{
		return -1;
	}
	// agc para
	if(ISP_SUCCESS != sensor_get_agc_para(IspDev, &pstAutoSceneDn->stCmosAgcPara))
	{
		ERR("sensor_get_agc_para failed!\n");
	}

	// ae lum
	pstAutoSceneDn->stAePara.u8AeLumNum = 1;
	pstAutoSceneDn->stAePara.au32ExpThr[0] = 12000;
	pstAutoSceneDn->stAePara.au32ExpThr[1] = 24000;
	pstAutoSceneDn->stAePara.au32ExpThr[2] = 24000;
	pstAutoSceneDn->stAePara.au32ExpThr[3] = 24000;
	pstAutoSceneDn->stAePara.au8AeLumColor[0] = 48;
	pstAutoSceneDn->stAePara.au8AeLumColor[1] = 45;
	pstAutoSceneDn->stAePara.au8AeLumColor[2] = 45;
	pstAutoSceneDn->stAePara.au8AeLumColor[3] = 45;
	pstAutoSceneDn->stAePara.au8AeLumBw[0] = 48;
	pstAutoSceneDn->stAePara.au8AeLumBw[1] = 45;
	pstAutoSceneDn->stAePara.au8AeLumBw[2] = 45;
	pstAutoSceneDn->stAePara.au8AeLumBw[3] = 45;	
	
	pstAutoSceneDn->stAePara.au32GainDef[0] = 32*256; // color
	pstAutoSceneDn->stAePara.au32GainMax[0] = 64*256;	// color
	pstAutoSceneDn->stAePara.au32GainDef[1] = pstAutoSceneDn->stAePara.au32GainDef[0];	// Bw
	pstAutoSceneDn->stAePara.au32GainMax[1] = pstAutoSceneDn->stAePara.au32GainMax[0];	// Bw	

	pstAutoSceneDn->stAePara.au8AeStrategyDef[0] = AE_EXP_HIGHLIGHT_PRIOR;
	pstAutoSceneDn->stAePara.au8AeStrategyStrDef[0] = 0;
	// fun
	pstAutoSceneDn->pfun_autoscene_sns = fun_autoscene_sc307e;
#if 0
	// ae/awb sync
	if(1 == IspDev)
	{
		ISP_EXPOSURE_ATTR_S stExpAttr;
		ISPCHECK_RET(XM_MPI_ISP_GetExposureAttr(IspDev, &stExpAttr),"XM_MPI_ISP_GetExposureAttr");
		stExpAttr.stAuto.u8BindSyncChn = 0;
		ISPCHECK_RET(XM_MPI_ISP_SetExposureAttr(IspDev, &stExpAttr), "XM_MPI_ISP_SetExposureAttr");

		ISP_WB_ATTR_S stWBAttr;
		ISPCHECK_RET(XM_MPI_ISP_GetWBAttr(IspDev, &stWBAttr),"XM_MPI_ISP_GetWBAttr");
		stWBAttr.stAuto.u8BindSyncChn = 0;
		ISPCHECK_RET(XM_MPI_ISP_SetWBAttr(IspDev, &stWBAttr),"XM_MPI_ISP_SetWBAttr");
	}
#endif
	return 0;
}

