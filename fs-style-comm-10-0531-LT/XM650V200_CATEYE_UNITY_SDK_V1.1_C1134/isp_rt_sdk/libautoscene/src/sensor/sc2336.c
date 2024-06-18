#include "autoscene_para.h"
//sensor: SC2336


static ISP_S32 autoscene_awb_scan_sc2336(void)
{
	ISP_U32 u32ChnZeroColorTemp;
	ISP_U32 au32ColorTemp[5] = {2000,4000,5500,6600,7500};
	ISP_S32 as32CtRedDv[5] = {20,20,20,20,35};
	ISP_S32 as32CtGreenDv[5] = {0,0,0,0,0};
	ISP_S32 as32CtBlueDv[5] = {16,15,15,15,16};
	ISP_WB_INFO_S stWBInfo;
	ISP_U8 u8Idx,u8Wgt,u8i,u8Num;
	ISP_S32 s32RedDvResult,s32GreenDvResult,s32BlueDvResult,s32Tmp;
	static ISP_S32 s_s32RedGain = -1,s_s32BlueGain = -1;
	ISP_U32 u32Tmp;

	ISPCHECK_RET(XM_MPI_ISP_QueryWBInfo(0, &stWBInfo), "XM_MPI_ISP_QueryWBInfo");
	u32ChnZeroColorTemp = stWBInfo.u16ColorTemp;

	u8Num = 5;

	if((u8Num<2) || (u32ChnZeroColorTemp <= au32ColorTemp[0]))
	{
		u8Idx = 0;
		u8Wgt = 0xFF;
	}
	else if(u32ChnZeroColorTemp >= au32ColorTemp[u8Num-1])
	{
		u8Idx = u8Num-1;
		u8Wgt = 0xFF;
	}
	else
	{
		for(u8i=1; u8i<u8Num-1; u8i ++)
		{
			if(au32ColorTemp[u8i]>=u32ChnZeroColorTemp)
				break;
		}
		u8Idx = u8i - 1;
		s32Tmp = (ISP_S32)au32ColorTemp[u8Idx+1]-au32ColorTemp[u8Idx];
		u32Tmp = (s32Tmp < 1)?0xFF:(au32ColorTemp[u8Idx+1]-u32ChnZeroColorTemp)*0xFF/s32Tmp;
		u8Wgt = CLIP3(u32Tmp,0,0xFF);
	}
	s32RedDvResult = (u8Wgt == 0xFF)?*(as32CtRedDv+u8Idx):
					((ISP_U32)u8Wgt*(*(as32CtRedDv+u8Idx))+(ISP_U32)(0xFF-u8Wgt)*(*(as32CtRedDv+u8Idx+1)))/0xFF;
	s32GreenDvResult = (u8Wgt == 0xFF)?*(as32CtGreenDv+u8Idx):
					((ISP_U32)u8Wgt*(*(as32CtGreenDv+u8Idx))+(ISP_U32)(0xFF-u8Wgt)*(*(as32CtGreenDv+u8Idx+1)))/0xFF;
	s32BlueDvResult = (u8Wgt == 0xFF)?*(as32CtBlueDv+u8Idx):
					((ISP_U32)u8Wgt*(*(as32CtBlueDv+u8Idx))+(ISP_U32)(0xFF-u8Wgt)*(*(as32CtBlueDv+u8Idx+1)))/0xFF;

	ISP_WB_ATTR_S stWBAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetWBAttr(1, &stWBAttr), "XM_MPI_ISP_GetWBAttr");
	stWBAttr.enOpType = OP_TYPE_MANUAL;
	stWBAttr.stManual.u16Rgain = stWBInfo.u16Rgain + s32RedDvResult;
//printf("~~~~~~~~~~~~~~s32BlueDvResult:%d\n", s32BlueDvResult);
//printf("~~~~~~~~~~~~~~stWBInfo.u16Bgain:%d\n", stWBInfo.u16Bgain);
	stWBAttr.stManual.u16Ggain = stWBInfo.u16Ggain - s32GreenDvResult;
	stWBAttr.stManual.u16Bgain = stWBInfo.u16Bgain - s32BlueDvResult;
	if(s_s32RedGain == stWBAttr.stManual.u16Rgain && s_s32BlueGain == stWBAttr.stManual.u16Bgain)
	{
		return ISP_SUCCESS;
	}
	s_s32RedGain = stWBAttr.stManual.u16Rgain;
	s_s32BlueGain = stWBAttr.stManual.u16Bgain;
	
	ISPCHECK_RET(XM_MPI_ISP_SetWBAttr(1, &stWBAttr), "XM_MPI_ISP_SetWBAttr");
	return ISP_SUCCESS;
}

static ISP_S32 fun_autoscene_sc2336(ISP_DEV IspDev,ISP_EXT_AUTOSCENE_FUN_PARA_S *pstPara, AUTOSCENE_PARA_ADDR_S stAddr)
{
	ISPAPP_INFO stProductInfo;
	ISPCHECK_RET(XM_MPI_ISPAPP_InfoGet(&stProductInfo), "XM_MPI_ISPAPP_ProductInfoGet");
	
	if(stProductInfo.u8FunExt == FUN_EXT_DOUBLE_SENSOR && 0 == IspDev)
	{
		autoscene_awb_scan_sc2336();
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
ISP_S32 autoscene_init_sc2336(			ISP_DEV IspDev,
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
	pstAutoSceneDn->stAePara.u8AeLumNum = 2;
	pstAutoSceneDn->stAePara.au32ExpThr[0] = 12000;
	pstAutoSceneDn->stAePara.au32ExpThr[1] = 24000;
	pstAutoSceneDn->stAePara.au32ExpThr[2] = 24000;
	pstAutoSceneDn->stAePara.au32ExpThr[3] = 24000;
	if(stProductInfo.u8FunExt == FUN_EXT_DOOR_LOCK_FACE_USED)
	{
		pstAutoSceneDn->stAePara.au8AeLumColor[0] = 35;
		pstAutoSceneDn->stAePara.au8AeLumColor[1] = 35;
		pstAutoSceneDn->stAePara.au8AeLumColor[2] = 35;
		pstAutoSceneDn->stAePara.au8AeLumColor[3] = 35;	
		pstAutoSceneDn->stAePara.au8AeLumBw[0] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumBw[1] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumBw[2] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumBw[3] = 0x30;
	}
	if(stProductInfo.u8FunExt == FUN_EXT_DOOR_LOCK_FACE)
	{
		pstAutoSceneDn->stAePara.au8AeLumColor[0] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumColor[1] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumColor[2] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumColor[3] = 0x30;	
		pstAutoSceneDn->stAePara.au8AeLumBw[0] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumBw[1] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumBw[2] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumBw[3] = 0x30;
	}
	else if(stProductInfo.u8FunExt == FUN_EXT_DOUBLE_SENSOR)
	{
		pstAutoSceneDn->stAePara.au8AeLumColor[0] = 45;
		pstAutoSceneDn->stAePara.au8AeLumColor[1] = 45;
		pstAutoSceneDn->stAePara.au8AeLumColor[2] = 40;
		pstAutoSceneDn->stAePara.au8AeLumColor[3] = 35;	
		pstAutoSceneDn->stAePara.au8AeLumBw[0] = 48;
		pstAutoSceneDn->stAePara.au8AeLumBw[1] = 40;
		pstAutoSceneDn->stAePara.au8AeLumBw[2] = 35;
		pstAutoSceneDn->stAePara.au8AeLumBw[3] = 30;
	}
	else
	{
		pstAutoSceneDn->stAePara.au8AeLumColor[0] = 52;//52
		pstAutoSceneDn->stAePara.au8AeLumColor[1] = 56;//51
		pstAutoSceneDn->stAePara.au8AeLumColor[2] = 56;//51
		pstAutoSceneDn->stAePara.au8AeLumColor[3] = 48;//
		pstAutoSceneDn->stAePara.au8AeLumBw[0] = 54;
		pstAutoSceneDn->stAePara.au8AeLumBw[1] = 56;
		pstAutoSceneDn->stAePara.au8AeLumBw[2] = 50;
		pstAutoSceneDn->stAePara.au8AeLumBw[3] = 45;
	}
	if(stProductInfo.u8FunExt == FUN_EXT_DOUBLE_SENSOR)
	{
		pstAutoSceneDn->stAePara.au32GainDef[0] = 128*256; // color
		pstAutoSceneDn->stAePara.au32GainMax[0] = 256*256;	// color
	}
	else if(stProductInfo.u8FunExt == FUN_EXT_DOOR_LOCK_FACE_USED)
	{
		pstAutoSceneDn->stAePara.au32GainDef[0] = 1*256; // color
		pstAutoSceneDn->stAePara.au32GainMax[0] = 1*256;	// color
	}
	else if(stProductInfo.u8FunExt == FUN_EXT_DOOR_LOCK_FACE)
	{
		pstAutoSceneDn->stAePara.au32GainDef[0] = 2*256; // color
		pstAutoSceneDn->stAePara.au32GainMax[0] = 2*256;	// color
		pstAutoSceneDn->stAePara.au32GainDef[1] = 1*256; // color
		pstAutoSceneDn->stAePara.au32GainMax[1] = 1*256;	// color
	}
	else
	{
		pstAutoSceneDn->stAePara.au32GainDef[0] = 64*256; // color
		pstAutoSceneDn->stAePara.au32GainMax[0] = 64*256;	// color
	}
	pstAutoSceneDn->stAePara.au32GainDef[1] = pstAutoSceneDn->stAePara.au32GainDef[0];	// Bw
	pstAutoSceneDn->stAePara.au32GainMax[1] = pstAutoSceneDn->stAePara.au32GainMax[0];	// Bw	


	if(stProductInfo.u8FunExt == FUN_EXT_DOUBLE_SENSOR)
	{
		pstAutoSceneDn->stAePara.au8AeStrategyDef[0] = AE_EXP_HIGHLIGHT_PRIOR;
		pstAutoSceneDn->stAePara.au8AeStrategyStrDef[0] = 0x18;
	}
	else if(stProductInfo.u8FunExt == FUN_EXT_DOOR_LOCK_FACE_USED)
	{
		pstAutoSceneDn->stAePara.au8AeStrategyDef[0] = AE_EXP_NORMAL;
		pstAutoSceneDn->stAePara.au8AeStrategyStrDef[0] = 0x20;
	}
	else if(stProductInfo.u8FunExt == FUN_EXT_DOOR_LOCK_FACE)
	{
		pstAutoSceneDn->stAePara.au8AeStrategyDef[0] = AE_EXP_NORMAL;
		pstAutoSceneDn->stAePara.au8AeStrategyStrDef[0] = 0x20;
		pstAutoSceneDn->stAePara.au8AeStrategyDef[1] = AE_EXP_NORMAL;
		pstAutoSceneDn->stAePara.au8AeStrategyStrDef[1] = 0x20;
	}
	else
	{
		pstAutoSceneDn->stAePara.au8AeStrategyDef[0] = AE_EXP_NORMAL;
		pstAutoSceneDn->stAePara.au8AeStrategyStrDef[0] = 0x20;
		pstAutoSceneDn->stAePara.au8AeStrategyDef[1] = AE_EXP_NORMAL;
		pstAutoSceneDn->stAePara.au8AeStrategyStrDef[1] = 0x20;
	}
	// fun
	pstAutoSceneDn->pfun_autoscene_sns = fun_autoscene_sc2336;
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
	pstAutoSceneDn->stAePara.astSoftIr[2].s32ColorExp = 26000;
	pstAutoSceneDn->stAePara.astSoftIr[2].s32BwExp = 56000;
	pstAutoSceneDn->stAePara.astSoftIr[3].s32ColorDlt = 60;
	pstAutoSceneDn->stAePara.astSoftIr[3].s32ColorDlt2 = 370;
	pstAutoSceneDn->stAePara.astSoftIr[3].s32ColorExp = 32000;
	pstAutoSceneDn->stAePara.astSoftIr[3].s32BwExp = 73000;
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

	// other
	ISP_CHROMA_ATTR_S stChromaAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetChromaAttr(IspDev,&stChromaAttr), "XM_MPI_ISP_GetChromaAttr");
	stChromaAttr.enHue2Sel = HUE_SEL_GREEN;
	stChromaAttr.s16Hue2Ofst = 0;
	stChromaAttr.u8SthG = 0x80;
	if(stProductInfo.u8FunExt != FUN_EXT_DOOR_LOCK_FACE_USED && stProductInfo.u8FunExt != FUN_EXT_DOUBLE_SENSOR)
    {
	   stChromaAttr.u8SthG = 0x90;//80
    }
	else if(stProductInfo.u8FunExt == FUN_EXT_DOOR_LOCK_FACE)
	{
		stChromaAttr.u8SthR = 0x20;
	}
	ISPCHECK_RET(XM_MPI_ISP_SetChromaAttr(IspDev,&stChromaAttr), "XM_MPI_ISP_SetChromaAttr");
	
	ISP_WB_ATTR_S stWBAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetWBAttr(IspDev, &stWBAttr), "XM_MPI_ISP_GetWBAttr");
	stWBAttr.stAuto.u8GmOfst = 0x80;
	stWBAttr.stAuto.u8ROfst = 0x82;
	stWBAttr.stAuto.u8BOfst = 0x84;
	if(stProductInfo.u8FunExt != FUN_EXT_DOOR_LOCK_FACE_USED && stProductInfo.u8FunExt != FUN_EXT_DOUBLE_SENSOR)
    {
		stWBAttr.stAuto.u8GmOfst = 0x7f;//0x80
		stWBAttr.stAuto.u8ROfst = 0x80;//82
		stWBAttr.stAuto.u8BOfst = 0x80;//84
    }
	if(stProductInfo.u8FunExt == FUN_EXT_DOUBLE_SENSOR && 1 == IspDev)
	{
		//stWBAttr.stAuto.u8BindSyncChn = 0;
	}
	else if(stProductInfo.u8FunExt == FUN_EXT_DOOR_LOCK_FACE_USED && 1 == IspDev)
	{
		stWBAttr.stAuto.u8BindSyncChn = 0;
	}
	else if(stProductInfo.u8FunExt == FUN_EXT_DOOR_LOCK_FACE && 0 == IspDev)
	{
		stWBAttr.enOpType = OP_TYPE_MANUAL;
		stWBAttr.stManual.u16Rgain = 496;
		stWBAttr.stManual.u16Ggain = 256;
		stWBAttr.stManual.u16Bgain = 414;
	}
	ISPCHECK_RET(XM_MPI_ISP_SetWBAttr(IspDev, &stWBAttr), "XM_MPI_ISP_SetWBAttr");

	ISP_EXPOSURE_ATTR_S  pstExpAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetExposureAttr(IspDev, &pstExpAttr), "XM_MPI_ISP_GetExposureAttr");

	if(stProductInfo.u8FunExt == FUN_EXT_DOUBLE_SENSOR && IspDev == 1)
	{
		pstExpAttr.stAuto.u8BindSyncChn = 0;
		pstExpAttr.stAuto.u32BindSyncRatio = 1500;
	}
	else if(stProductInfo.u8FunExt == FUN_EXT_DOOR_LOCK_FACE_USED)
	{
		if(1 == IspDev)
		{
			pstExpAttr.stAuto.u8BindSyncChn = 0;
			//pstExpAttr.stAuto.u32BindSyncRatio = 1280;
			pstExpAttr.stAuto.stExpTimeRange.u32Max = 0x400;
			pstExpAttr.stAuto.stExpTimeRange.u32Min = 0x4;
		}
		else
		{
			pstExpAttr.stAuto.stExpTimeRange.u32Max = 0x400;
			pstExpAttr.stAuto.stExpTimeRange.u32Min = 0x4;
		}
		pstExpAttr.stAuto.stAGainRange.u32Max = 0x400;
		pstExpAttr.stAuto.stDGainRange.u32Max = 0x400;
		pstExpAttr.stAuto.stISPDGainRange.u32Max = 0x400;
	}
	else if(stProductInfo.u8FunExt == FUN_EXT_DOOR_LOCK_FACE)
	{
		//pstExpAttr.stAuto.stExpTimeRange.u32Max = 0x400;
		pstExpAttr.stAuto.stExpTimeRange.u32Min = 0x4;
		//pstExpAttr.stAuto.stAGainRange.u32Max = 0x400;
		//pstExpAttr.stAuto.stDGainRange.u32Max = 0x400;
		//pstExpAttr.stAuto.stISPDGainRange.u32Max = 0x400;
	}
	ISPCHECK_RET(XM_MPI_ISP_SetExposureAttr(IspDev, &pstExpAttr), "XM_MPI_ISP_SetExposureAttr");


	//XM_MPI_ISP_SetRegister(0x32050000, 0);
	//XM_MPI_ISP_SetRegister(0x32050004, 0x30);
	//XM_MPI_ISP_SetRegister(0x32050008, 0xfff);
	//XM_MPI_ISP_SetRegister(0x34003004, 0x4);

	//autoscene_set_sharp_table_sc2336(IspDev);

	return 0;
}



