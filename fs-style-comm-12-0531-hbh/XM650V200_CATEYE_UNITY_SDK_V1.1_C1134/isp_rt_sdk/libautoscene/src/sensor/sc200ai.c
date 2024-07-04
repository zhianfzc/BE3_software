#include "autoscene_para.h"
//sensor: SC200AI

static ISP_S32 fun_autoscene_sc200ai(ISP_DEV IspDev,ISP_EXT_AUTOSCENE_FUN_PARA_S *pstPara, AUTOSCENE_PARA_ADDR_S stAddr)
{
	return ISP_SUCCESS;
}

static ISP_U16 gau16TabEdge[32] = {
	0x0AB, 0x0C9, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 
	0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 
	0x100, 0x100, 0x90, 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 
};
	
static ISP_U16 gau16TabText[32] = {
	0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 	0x100, 
	0x100, 	0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 		0x100, 
	0x100, 0x100, 	0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100,
};
	
static ISP_S32 autoscene_set_sharp_table_sc200ai(ISP_DEV IspDev)
{
	ISP_U8 u8i;
	ISP_SHARPEN_ATTR_S stSharpenAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetSharpenAttr(IspDev, &stSharpenAttr), "XM_MPI_ISP_GetSharpenAttr");

	for(u8i=0;u8i<32;u8i++)
	{
		stSharpenAttr.stCom.au16TabEdge[u8i] = gau16TabEdge[u8i];
		stSharpenAttr.stCom.au16TabText[u8i] = gau16TabText[u8i];
	}
	ISPCHECK_RET(XM_MPI_ISP_SetSharpenAttr(IspDev, &stSharpenAttr), "XM_MPI_ISP_SetSharpenAttr");	
#if 1
	//亮度锐化
	XM_MPI_ISP_SetRegister(0x34003260,0x130);
	XM_MPI_ISP_SetRegister(0x34003264,0x130);
	XM_MPI_ISP_SetRegister(0x34003268,0x130);
	XM_MPI_ISP_SetRegister(0x3400326c,0x130);
	XM_MPI_ISP_SetRegister(0x34003270,0x130);
	XM_MPI_ISP_SetRegister(0x34003274,0x130);
	XM_MPI_ISP_SetRegister(0x34003278,0x130);
	XM_MPI_ISP_SetRegister(0x3400327c,0x130);
	XM_MPI_ISP_SetRegister(0x34003280,0x130);
	XM_MPI_ISP_SetRegister(0x34003284,0x130);
	XM_MPI_ISP_SetRegister(0x34003288,0x130);
	XM_MPI_ISP_SetRegister(0x3400328c,0x130);
	XM_MPI_ISP_SetRegister(0x34003290,0x130);
	XM_MPI_ISP_SetRegister(0x34003294,0x130);
	XM_MPI_ISP_SetRegister(0x34003298,0x130);
	XM_MPI_ISP_SetRegister(0x3400329c,0x130);
	XM_MPI_ISP_SetRegister(0x340032a0,0x130);
	XM_MPI_ISP_SetRegister(0x340032a4,0x130);
	XM_MPI_ISP_SetRegister(0x340032a8,0x130);
	XM_MPI_ISP_SetRegister(0x340032ac,0x130);
	XM_MPI_ISP_SetRegister(0x340032b0,0x130);
	XM_MPI_ISP_SetRegister(0x340032b4,0x120);
	XM_MPI_ISP_SetRegister(0x340032b8,0x120);
	XM_MPI_ISP_SetRegister(0x340032bc,0x110);
	XM_MPI_ISP_SetRegister(0x340032c0,0x110);
	XM_MPI_ISP_SetRegister(0x340032c4,0x100);
	XM_MPI_ISP_SetRegister(0x340032c4,0x100);
	XM_MPI_ISP_SetRegister(0x340032c4,0x100);
	XM_MPI_ISP_SetRegister(0x340032c4,0x100);
	XM_MPI_ISP_SetRegister(0x340032c4,0x100);
	XM_MPI_ISP_SetRegister(0x340032c4,0x0f0);
	XM_MPI_ISP_SetRegister(0x340032c4,0x0e0);
#endif
	return ISP_SUCCESS;
}



/*************************************************************************
函数功能:		参数初始化
输入参数: 		u32ProductType:  产品型号（用于区分不同产品）
输出参数:		stAddr:   		配置相关
返回参数: 		0:成功    		-1:失败
note:
*************************************************************************/
ISP_S32 autoscene_init_sc200ai(			ISP_DEV IspDev,
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
	pstAutoSceneDn->stAePara.au8AeLumColor[0] = 48;
	pstAutoSceneDn->stAePara.au8AeLumColor[1] = 40;
	pstAutoSceneDn->stAePara.au8AeLumColor[2] = 35;
	pstAutoSceneDn->stAePara.au8AeLumColor[3] = 35;
	pstAutoSceneDn->stAePara.au8AeLumBw[0] = 45;
	pstAutoSceneDn->stAePara.au8AeLumBw[1] = 40;
	pstAutoSceneDn->stAePara.au8AeLumBw[2] = 35;
	pstAutoSceneDn->stAePara.au8AeLumBw[3] = 35;
	
	pstAutoSceneDn->stAePara.au32GainDef[0] = 256*256; // color
	pstAutoSceneDn->stAePara.au32GainMax[0] = 256*256;	// color
	pstAutoSceneDn->stAePara.au32GainDef[1] = pstAutoSceneDn->stAePara.au32GainDef[0];	// Bw
	pstAutoSceneDn->stAePara.au32GainMax[1] = pstAutoSceneDn->stAePara.au32GainMax[0];	// Bw	

	pstAutoSceneDn->stAePara.au8AeStrategyDef[0] = AE_EXP_HIGHLIGHT_PRIOR;
	pstAutoSceneDn->stAePara.au8AeStrategyStrDef[0] = 0x18;
	pstAutoSceneDn->stAePara.au8AeStrategyDef[1] = AE_EXP_HIGHLIGHT_PRIOR;
	pstAutoSceneDn->stAePara.au8AeStrategyStrDef[1] = 0x18;	
	// fun
	pstAutoSceneDn->pfun_autoscene_sns = fun_autoscene_sc200ai;
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
	ISPCHECK_RET(XM_MPI_ISP_SetChromaAttr(IspDev,&stChromaAttr), "XM_MPI_ISP_SetChromaAttr");
	
	ISP_WB_ATTR_S stWBAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetWBAttr(IspDev, &stWBAttr), "XM_MPI_ISP_GetWBAttr");
	stWBAttr.stAuto.u8ROfst = 0x82;
	stWBAttr.stAuto.u8BOfst = 0x84;
	ISPCHECK_RET(XM_MPI_ISP_SetWBAttr(IspDev, &stWBAttr), "XM_MPI_ISP_SetWBAttr");

	XM_MPI_ISP_SetRegister(0x32050000, 0);
	XM_MPI_ISP_SetRegister(0x32050004, 0x30);
	XM_MPI_ISP_SetRegister(0x32050008, 0xfff);
	//XM_MPI_ISP_SetRegister(0x34003004, 0x4);

	autoscene_set_sharp_table_sc200ai(IspDev);
	return 0;
}



