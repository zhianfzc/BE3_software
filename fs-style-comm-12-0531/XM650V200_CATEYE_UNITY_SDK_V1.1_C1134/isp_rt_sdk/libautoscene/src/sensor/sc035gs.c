#include "autoscene_para.h"
//sensor: SC035GS


static ISP_S32 fun_autoscene_sc035gs(ISP_DEV IspDev,ISP_EXT_AUTOSCENE_FUN_PARA_S *pstPara, AUTOSCENE_PARA_ADDR_S stAddr)
{
	return ISP_SUCCESS;
	static ISP_DRC_MANUAL_ATTR_S stDRCManual = {0x00};
	static ISP_U8 u8drcnum_actual = 0;
	ISP_U8 u8drcnum,u8speed = 8;
	ISP_DRC_ATTR_S stDRC;

	if(pstPara->u32Exposure > 45000)
		u8drcnum = 0x00;
	else if(pstPara->u32Exposure <35000)
		u8drcnum = 0x30;
	else
		u8drcnum = stDRCManual.u8Strength;

	if(u8drcnum > u8drcnum_actual)
	{
		if((u8drcnum_actual+u8speed)>u8drcnum)
			u8drcnum_actual = u8drcnum;
		else
			u8drcnum_actual += u8speed;
	}
	else if(u8drcnum < u8drcnum_actual)
	{
		if((u8drcnum+u8speed)>u8drcnum_actual)
			u8drcnum_actual = u8drcnum;
		else
			u8drcnum_actual -= u8speed;
	}

	if(u8drcnum_actual != stDRCManual.u8Strength)
	{
		stDRCManual.u8Strength = u8drcnum_actual;
		if(XM_MPI_ISP_GetDRCAttr(0, &stDRC) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_GetDRCAttr failed!\n");
		}
		stDRC.enOpType = OP_TYPE_MANUAL;
		stDRC.stManual = stDRCManual;
		if(XM_MPI_ISP_SetDRCAttr(0, &stDRC) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_SetDRCAttr failed!\n");
		}

		if(XM_MPI_ISP_GetDRCAttr(1, &stDRC) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_GetDRCAttr failed!\n");
		}
		stDRC.enOpType = OP_TYPE_MANUAL;
		stDRC.stManual = stDRCManual;
		if(XM_MPI_ISP_SetDRCAttr(1, &stDRC) != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_SetDRCAttr failed!\n");
		}
	}

	return ISP_SUCCESS;
}

/*************************************************************************
函数功能:		参数初始化
输入参数: 		u32ProductType:  产品型号（用于区分不同产品）
输出参数:		stAddr:   		配置相关
返回参数: 		0:成功    		-1:失败
note:
*************************************************************************/
ISP_S32 autoscene_init_sc035gs(			ISP_DEV IspDev,
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
	pstAutoSceneDn->stAePara.au8AeLumBw[0] = 26;
	pstAutoSceneDn->stAePara.au8AeLumBw[1] = 26;
	pstAutoSceneDn->stAePara.au8AeLumBw[2] = 26;
	pstAutoSceneDn->stAePara.au8AeLumBw[3] = 26;
	
	pstAutoSceneDn->stAePara.au32GainDef[0] = 3*128; // color
	pstAutoSceneDn->stAePara.au32GainMax[0] = 4*256;	// color
	pstAutoSceneDn->stAePara.au32GainDef[1] = pstAutoSceneDn->stAePara.au32GainDef[0];	// Bw
	pstAutoSceneDn->stAePara.au32GainMax[1] = pstAutoSceneDn->stAePara.au32GainMax[0];	// Bw	

	pstAutoSceneDn->stAePara.au8AeStrategyDef[DNS_COLOR] = AE_EXP_HIGHLIGHT_PRIOR;
	pstAutoSceneDn->stAePara.au8AeStrategyStrDef[DNS_COLOR] = 0x20;
	pstAutoSceneDn->stAePara.au8AeStrategyDef[DNS_BW] = AE_EXP_LOWLIGHT_PRIOR;
	pstAutoSceneDn->stAePara.au8AeStrategyStrDef[DNS_BW] = 0x80;
	// fun
	pstAutoSceneDn->pfun_autoscene_sns = fun_autoscene_sc035gs;
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
	return 0;
}

