#include "autoscene_para.h"
//sensor: SC1346


static ISP_S32 fun_autoscene_sc1346(ISP_DEV IspDev,ISP_EXT_AUTOSCENE_FUN_PARA_S *pstPara, AUTOSCENE_PARA_ADDR_S stAddr)
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
ISP_S32 autoscene_init_sc1346(			ISP_DEV IspDev,
									ISP_U32 u32ProductType,
									AUTOSCENE_PARA_ADDR_S stAddr)
{
	ISPAPP_INFO stProductInfo;
	ISPCHECK_RET(XM_MPI_ISPAPP_InfoGet(&stProductInfo), "XM_MPI_ISPAPP_ProductInfoGet");

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
	pstAutoSceneDn->stAePara.u8AeLumNum = 	02;
	pstAutoSceneDn->stAePara.au32ExpThr[0] = 12000;
	pstAutoSceneDn->stAePara.au32ExpThr[1] = 24000;
	pstAutoSceneDn->stAePara.au32ExpThr[2] = 24000;
	pstAutoSceneDn->stAePara.au32ExpThr[3] = 24000;
	pstAutoSceneDn->stAePara.au8AeLumColor[0] = 38;
	pstAutoSceneDn->stAePara.au8AeLumColor[1] = 38;
	pstAutoSceneDn->stAePara.au8AeLumColor[2] = 38;
	pstAutoSceneDn->stAePara.au8AeLumColor[3] = 38;
	pstAutoSceneDn->stAePara.au8AeLumBw[0] = 54;
	pstAutoSceneDn->stAePara.au8AeLumBw[1] = 54;
	pstAutoSceneDn->stAePara.au8AeLumBw[2] = 54;
	pstAutoSceneDn->stAePara.au8AeLumBw[3] = 54;
	
	pstAutoSceneDn->stAePara.au32GainDef[0] = 64*256; // color
	pstAutoSceneDn->stAePara.au32GainMax[0] = 128*256;	// color
	pstAutoSceneDn->stAePara.au32GainDef[1] = pstAutoSceneDn->stAePara.au32GainDef[0];	// Bw
	pstAutoSceneDn->stAePara.au32GainMax[1] = pstAutoSceneDn->stAePara.au32GainMax[0];	// Bw	

	pstAutoSceneDn->stAePara.au8AeStrategyDef[0] = AE_EXP_HIGHLIGHT_PRIOR;
	pstAutoSceneDn->stAePara.au8AeStrategyStrDef[0] = 0x80;
	pstAutoSceneDn->stAePara.au8AeStrategyDef[1] = AE_EXP_HIGHLIGHT_PRIOR;
	pstAutoSceneDn->stAePara.au8AeStrategyStrDef[1] = 0x80;
	// fun
	pstAutoSceneDn->pfun_autoscene_sns = fun_autoscene_sc1346;
	// SoftIR
	pstAutoSceneDn->stAePara.astSoftIr[0].s32ColorDlt = 118;
	pstAutoSceneDn->stAePara.astSoftIr[0].s32ColorDlt2 = 612;
	pstAutoSceneDn->stAePara.astSoftIr[0].s32ColorExp = 16000;
	pstAutoSceneDn->stAePara.astSoftIr[0].s32BwExp = 37000;
	pstAutoSceneDn->stAePara.astSoftIr[1].s32ColorDlt = 118;
	pstAutoSceneDn->stAePara.astSoftIr[1].s32ColorDlt2 = 612;
	pstAutoSceneDn->stAePara.astSoftIr[1].s32ColorExp = 16000;
	pstAutoSceneDn->stAePara.astSoftIr[1].s32BwExp = 37000;
	pstAutoSceneDn->stAePara.astSoftIr[2].s32ColorDlt = 75;
	pstAutoSceneDn->stAePara.astSoftIr[2].s32ColorDlt2 = 422;
	pstAutoSceneDn->stAePara.astSoftIr[2].s32ColorExp = 20000;
	pstAutoSceneDn->stAePara.astSoftIr[2].s32BwExp = 46000;
	pstAutoSceneDn->stAePara.astSoftIr[3].s32ColorDlt = 60;
	pstAutoSceneDn->stAePara.astSoftIr[3].s32ColorDlt2 = 370;
	pstAutoSceneDn->stAePara.astSoftIr[3].s32ColorExp = 20000;
	pstAutoSceneDn->stAePara.astSoftIr[3].s32BwExp = 50000;
	pstAutoSceneDn->stAePara.astSoftIr[4].s32ColorDlt = 48;
	pstAutoSceneDn->stAePara.astSoftIr[4].s32ColorDlt2 = 336;
	pstAutoSceneDn->stAePara.astSoftIr[4].s32ColorExp = 65000;
	pstAutoSceneDn->stAePara.astSoftIr[4].s32BwExp = 146000;
	pstAutoSceneDn->stAePara.astSoftIr[5].s32ColorDlt = 38;
	pstAutoSceneDn->stAePara.astSoftIr[5].s32ColorDlt2 = 305;
	pstAutoSceneDn->stAePara.astSoftIr[5].s32ColorExp = 126000;
	pstAutoSceneDn->stAePara.astSoftIr[5].s32BwExp = 256000;
	pstAutoSceneDn->stAePara.au32EShutterThr[0] = 33750;
	pstAutoSceneDn->stAePara.au32EShutterThr[1] = 11250;
	if((stProductInfo.u8FunExt == FUN_EXT_DOOR_LOCK_FACE)||(stProductInfo.u8FunExt == FUN_EXT_DOOR_LOCK_FACE_UVC))
	{
		ISP_EXPOSURE_ATTR_S  pstExpAttr;
		ISP_S32 s32Ret;
		s32Ret = XM_MPI_ISP_GetExposureAttr(IspDev, &pstExpAttr);
		if(s32Ret != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_GetExposureAttr failed!\n");
			return s32Ret;
		}
		pstExpAttr.stManual.u32ExpLine = 750;
		pstExpAttr.stManual.s32AGain = 0x400;
		pstExpAttr.stManual.s32DGain = 0x4BD;
		pstExpAttr.stManual.s32IspGain = 0x400;
		pstExpAttr.enOpType = OP_TYPE_MANUAL;
		pstExpAttr.stManual.bManualExpLineEnable = 1;
		pstExpAttr.stManual.bManualAGainEnable = 1;
		pstExpAttr.stManual.bManualDGainEnable = 1;
		pstExpAttr.stManual.bManualIspGainEnable = 1;
		pstExpAttr.stManual.bManualExpEnable = 1;
		s32Ret = XM_MPI_ISP_SetExposureAttr(IspDev, &pstExpAttr);
		if(s32Ret != ISP_SUCCESS)
		{
			ERR("XM_MPI_ISP_SetExposureAttr failed!\n");
			return s32Ret;
		}
		pstAutoSceneDn->stAePara.au32GainDef[0] = 6*256; // color
		pstAutoSceneDn->stAePara.au32GainMax[0] = 16*256;	// color
		pstAutoSceneDn->stAePara.au32GainDef[1] = 6*256; // color
		pstAutoSceneDn->stAePara.au32GainMax[1] = 16*256;	// color
	}

	// other
	ISP_CHROMA_ATTR_S stChromaAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetChromaAttr(IspDev,&stChromaAttr), "XM_MPI_ISP_GetChromaAttr");
	stChromaAttr.enHue2Sel = HUE_SEL_GREEN;
	stChromaAttr.s16Hue2Ofst = 0x50;
	stChromaAttr.u8SthG = 0xA0;
	ISPCHECK_RET(XM_MPI_ISP_SetChromaAttr(IspDev,&stChromaAttr), "XM_MPI_ISP_SetChromaAttr");
	
	ISP_WB_ATTR_S stWBAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetWBAttr(IspDev, &stWBAttr), "XM_MPI_ISP_GetWBAttr");
	stWBAttr.stAuto.u8ROfst = 0x82;
	stWBAttr.stAuto.u8BOfst = 0x84;
	ISPCHECK_RET(XM_MPI_ISP_SetWBAttr(IspDev, &stWBAttr), "XM_MPI_ISP_SetWBAttr");
	return 0;
}




