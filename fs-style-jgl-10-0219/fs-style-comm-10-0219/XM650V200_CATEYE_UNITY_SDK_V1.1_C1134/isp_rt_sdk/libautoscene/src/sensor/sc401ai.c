#include "autoscene_para.h"
//sensor: SC401AI

// DRC(when use ext line--- other mode)
static const ISP_U16 gau16DrcLut_sc401ai[][DRC_IDX_NUM] = {
	//linear
	//{0x000,0x002,0x004,0x006,0x008,0x00C,0x010,0x014,0x018,0x020,0x028,0x030,0x038,0x040,0x048,0x050,0x058,0x060,0x068,0x070,0x080,0x090,0x0A0,0x0B0,0x0C0,0x0D0,0x0E0,0x0F0,0x100,0x110,0x120,0x130,0x140,0x150,0x160,0x180,0x1A0,0x1C0,0x1E0,0x200,0x220,0x240,0x260,0x280,0x2A0,0x2C0,0x2E0,0x300,0x320,0x340,0x360,0x380,0x3A0,0x3C0,0x3E0,0x400,0x440,0x480,0x4C0,0x500,0x540,0x580,0x5C0,0x600,0x640,0x680,0x6C0,0x700,0x740,0x780,0x7C0,0x800,0x840,0x880,0x8C0,0x900,0x940,0x980,0x9C0,0xA00,0xA40,0xA80,0xAC0,0xB00,0xB40,0xB80,0xBC0,0xC00,0xC40,0xC80,0xCC0,0xD00,0xD40,0xD80,0xDC0,0xE00,0xE40,0xE80,0xEC0,0xF00,0xF40,0xF80,0xFC0,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF}
	{0x000,0x005,0x00A,0x00F,0x014,0x01E,0x028,0x030,0x037,0x046,0x054,0x062,0x071,0x082,0x096,0x0AE,0x0C9,0x0E5,0x102,0x11F,0x154,0x183,0x1B1,0x1DE,0x20A,0x235,0x25F,0x288,0x2B2,0x2DB,0x304,0x32E,0x358,0x381,0x3AA,0x3FB,0x449,0x492,0x4D6,0x514,0x54C,0x582,0x5B6,0x5E8,0x619,0x647,0x674,0x6A0,0x6CA,0x6F2,0x71A,0x740,0x765,0x78A,0x7AD,0x7D0,0x810,0x84C,0x883,0x8B7,0x8EB,0x91F,0x956,0x992,0x9D3,0xA18,0xA61,0xAAC,0xAF5,0xB3B,0xB7D,0xBB8,0xBEE,0xC23,0xC56,0xC89,0xCBA,0xCE9,0xD17,0xD44,0xD6F,0xD99,0xDC1,0xDE8,0xE0D,0xE31,0xE53,0xE74,0xE94,0xEB5,0xED6,0xEF8,0xF1A,0xF3A,0xF5A,0xF78,0xF94,0xFAE,0xFC5,0xFD8,0xFE8,0xFF4,0xFFC,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF},
};

static ISP_S32 fun_autoscene_sc401ai(ISP_DEV IspDev,ISP_EXT_AUTOSCENE_FUN_PARA_S *pstPara, AUTOSCENE_PARA_ADDR_S stAddr)
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
ISP_S32 autoscene_init_sc401ai(			ISP_DEV IspDev,
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
	pstAutoSceneDn->stAePara.u8AeLumNum = 2;
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
	
	pstAutoSceneDn->stAePara.au32GainDef[0] = 128*256; // color
	pstAutoSceneDn->stAePara.au32GainMax[0] = 256*256;	// color
	pstAutoSceneDn->stAePara.au32GainDef[1] = pstAutoSceneDn->stAePara.au32GainDef[0];	// Bw
	pstAutoSceneDn->stAePara.au32GainMax[1] = pstAutoSceneDn->stAePara.au32GainMax[0];	// Bw	

	pstAutoSceneDn->stAePara.au8AeStrategyDef[0] = AE_EXP_HIGHLIGHT_PRIOR;
	pstAutoSceneDn->stAePara.au8AeStrategyStrDef[0] = 0x18;

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
	
	// fun
	pstAutoSceneDn->pfun_autoscene_sns = fun_autoscene_sc401ai;

	ISP_CHROMA_ATTR_S stChromaAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetChromaAttr(IspDev,&stChromaAttr), "XM_MPI_ISP_GetChromaAttr");
	stChromaAttr.enHue2Sel = HUE_SEL_GREEN;
	stChromaAttr.s16Hue2Ofst = 0xa0;
	stChromaAttr.u8SthG = 0xc0;
	ISPCHECK_RET(XM_MPI_ISP_SetChromaAttr(IspDev,&stChromaAttr), "XM_MPI_ISP_SetChromaAttr");
	
	ISP_WB_ATTR_S stWBAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetWBAttr(IspDev, &stWBAttr), "XM_MPI_ISP_GetWBAttr");
	stWBAttr.stAuto.u8ROfst = 0x82;
	stWBAttr.stAuto.u8BOfst = 0x84;
	ISPCHECK_RET(XM_MPI_ISP_SetWBAttr(IspDev, &stWBAttr), "XM_MPI_ISP_SetWBAttr");

	ISP_EXPOSURE_ATTR_S stExpAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetExposureAttr(IspDev, &stExpAttr), "XM_MPI_ISP_GetExposureAttr");
	stExpAttr.stAuto.stAGainRange.u32Min = 1440;
	ISPCHECK_RET(XM_MPI_ISP_SetExposureAttr(IspDev, &stExpAttr), "XM_MPI_ISP_SetExposureAttr");
	return 0;
}



#if 1 // just for debug
static const ISP_CMOS_SHARPEN_S gstSharpen_debug = {
	/* bvalid */
	1,
	/* 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800 */
	/* Ud*/
	{0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a},
	/* D*/
	{0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a},
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
	{0x58,0x58,0x58,0x58,0x58,0x54,0x50,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40},
	/*EdgeThr*/
	{0x04,0x04,0x04,0x04,0x04,0x04,0x06,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c},
	/*OverShoot*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*UnderShoot*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*DetailStr*/
	{0x50,0x50,0x40,0x2a,0x28,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
	/*Rsv1*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*Rsv2*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};
static const ISP_COLORMATRIX_AUTO_S g_stAwbCcm_debug = {
	5000,
	{
		0x000,379,-108,-15,
		0x000,-70, 381,-55,
		0x000,-25, -85,366
	},
	4000,
	{
		0x000,380,-111,-13,
		0x000,-84, 371,-31,
		0x000,	4,-129,381
	},
	2800,
	{
		0x000,327, -26, -45,
		0x000,-95, 383, -32,
		0x000, -1,-216, 473
	}
};
static const ISP_COLORMATRIX_AUTO_S g_stAwbCcm_normal = {
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
ISP_S32 autoscene_debug_sc401ai(ISP_DEV IspDev, ISP_S32 s32Data, ISP_CHAR *pstrData, ISP_CMOS_DN_PARA_S *pstCmosAgcDn)
{
	static ISP_U8 su8DataLst[ISP_NUM_MAX];
	ISP_U8 u8i;
	const ISP_CMOS_SHARPEN_S *pstAgcSharp;
	const ISP_COLORMATRIX_AUTO_S *pstCCM;
	ISP_CMOS_DN_PARA_S stCmosDnPara;
	ISP_CHROMA_ATTR_S stChromaAttr;
	ISP_DRC_ATTR_S stDRC;
	CHECK_ISPCHN(IspDev);
	if(NULL == pstCmosAgcDn)
	{
		return ISP_FAILURE;
	}
	if(su8DataLst[IspDev] == (s32Data&0xff))
	{
		return ISP_SUCCESS;
	}
	
	if(s32Data)	// Debug
	{
		DEBUG("Debug Mode!\n");
		ISPCHECK_RET(XM_MPI_ISP_GetChromaAttr(IspDev,&stChromaAttr), "XM_MPI_ISP_GetChromaAttr");
		stChromaAttr.enHue2Sel = HUE_SEL_GREEN;
		stChromaAttr.s16Hue2Ofst = 0x00;
		stChromaAttr.u8SthG = 0x80;
		stChromaAttr.u8SthR = 0x64;
		ISPCHECK_RET(XM_MPI_ISP_SetChromaAttr(IspDev,&stChromaAttr), "XM_MPI_ISP_SetChromaAttr");

		ISPCHECK_RET(XM_MPI_ISP_GetDRCAttr(IspDev, &stDRC), "XM_MPI_ISP_GetDRCAttr");
		stDRC.bEnable = ISP_FALSE;
		ISPCHECK_RET(XM_MPI_ISP_SetDRCAttr(IspDev, &stDRC), "XM_MPI_ISP_SetDRCAttr");

		pstAgcSharp = &gstSharpen_debug;
		pstCCM = &g_stAwbCcm_debug;	
	}
	else		// Normal
	{
		DEBUG("Normal Mode!\n");
		ISPCHECK_RET(XM_MPI_ISP_GetChromaAttr(IspDev,&stChromaAttr), "XM_MPI_ISP_GetChromaAttr");
		stChromaAttr.enHue2Sel = HUE_SEL_GREEN;
		stChromaAttr.s16Hue2Ofst = 0xa0;
		stChromaAttr.u8SthG = 0xc0;
		stChromaAttr.u8SthR = 0x80;
		ISPCHECK_RET(XM_MPI_ISP_SetChromaAttr(IspDev,&stChromaAttr), "XM_MPI_ISP_SetChromaAttr");

		ISPCHECK_RET(XM_MPI_ISP_GetDRCAttr(IspDev, &stDRC), "XM_MPI_ISP_GetDRCAttr");
		stDRC.bEnable = ISP_TRUE;
		ISPCHECK_RET(XM_MPI_ISP_SetDRCAttr(IspDev, &stDRC), "XM_MPI_ISP_SetDRCAttr");
		
		ISPCHECK_RET(sensor_get_agc_para(IspDev, &stCmosDnPara), "sensor_get_agc_para");
		pstAgcSharp = stCmosDnPara.pastAgcSharp[0];						// just color
		pstCCM = &g_stAwbCcm_normal;
	}
	pstCmosAgcDn->pastAgcSharp[0] = pstAgcSharp;// just color
	
	ISP_SHARPEN_ATTR_S stSharpenAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetSharpenAttr(IspDev, &stSharpenAttr), "XM_MPI_ISP_GetSharpenAttr");
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
	ISPCHECK_RET(XM_MPI_ISP_SetSharpenAttr(IspDev, &stSharpenAttr), "XM_MPI_ISP_SetSharpenAttr");	

	ISP_COLORMATRIX_ATTR_S stCCMAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetCCMAttr(IspDev, &stCCMAttr), "XM_MPI_ISP_GetCCMAttr");
	XM_MPI_ISP_Memncpy(&(stCCMAttr.stAuto), (ISP_VOID*)pstCCM, sizeof(ISP_COLORMATRIX_AUTO_S));
	ISPCHECK_RET(XM_MPI_ISP_SetCCMAttr(IspDev, &stCCMAttr), "XM_MPI_ISP_SetCCMAttr");	
	su8DataLst[IspDev] = s32Data&0xff;
	return ISP_SUCCESS;
}
#endif
