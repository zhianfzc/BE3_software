#include "autoscene_para.h"
//sensor: IMX307

// DRC(when use ext line--- other mode)
static const ISP_U16 gau16DrcLut_imx307[][DRC_IDX_NUM] = {
	//linear
	//{0x000,0x002,0x004,0x006,0x008,0x00C,0x010,0x014,0x018,0x020,0x028,0x030,0x038,0x040,0x048,0x050,0x058,0x060,0x068,0x070,0x080,0x090,0x0A0,0x0B0,0x0C0,0x0D0,0x0E0,0x0F0,0x100,0x110,0x120,0x130,0x140,0x150,0x160,0x180,0x1A0,0x1C0,0x1E0,0x200,0x220,0x240,0x260,0x280,0x2A0,0x2C0,0x2E0,0x300,0x320,0x340,0x360,0x380,0x3A0,0x3C0,0x3E0,0x400,0x440,0x480,0x4C0,0x500,0x540,0x580,0x5C0,0x600,0x640,0x680,0x6C0,0x700,0x740,0x780,0x7C0,0x800,0x840,0x880,0x8C0,0x900,0x940,0x980,0x9C0,0xA00,0xA40,0xA80,0xAC0,0xB00,0xB40,0xB80,0xBC0,0xC00,0xC40,0xC80,0xCC0,0xD00,0xD40,0xD80,0xDC0,0xE00,0xE40,0xE80,0xEC0,0xF00,0xF40,0xF80,0xFC0,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF}
	{0x000,0x005,0x00A,0x00F,0x014,0x01E,0x028,0x030,0x037,0x046,0x054,0x062,0x071,0x082,0x096,0x0AE,0x0C9,0x0E5,0x102,0x11F,0x154,0x183,0x1B1,0x1DE,0x20A,0x235,0x25F,0x288,0x2B2,0x2DB,0x304,0x32E,0x358,0x381,0x3AA,0x3FB,0x449,0x492,0x4D6,0x514,0x54C,0x582,0x5B6,0x5E8,0x619,0x647,0x674,0x6A0,0x6CA,0x6F2,0x71A,0x740,0x765,0x78A,0x7AD,0x7D0,0x810,0x84C,0x883,0x8B7,0x8EB,0x91F,0x956,0x992,0x9D3,0xA18,0xA61,0xAAC,0xAF5,0xB3B,0xB7D,0xBB8,0xBEE,0xC23,0xC56,0xC89,0xCBA,0xCE9,0xD17,0xD44,0xD6F,0xD99,0xDC1,0xDE8,0xE0D,0xE31,0xE53,0xE74,0xE94,0xEB5,0xED6,0xEF8,0xF1A,0xF3A,0xF5A,0xF78,0xF94,0xFAE,0xFC5,0xFD8,0xFE8,0xFF4,0xFFC,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF,0xFFF},
};

static ISP_S32 fun_autoscene_imx307(ISP_DEV IspDev,ISP_EXT_AUTOSCENE_FUN_PARA_S *pstPara, AUTOSCENE_PARA_ADDR_S stAddr)
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
ISP_S32 autoscene_init_imx307(			ISP_DEV IspDev,
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
	// fun
	pstAutoSceneDn->pfun_autoscene_sns = fun_autoscene_imx307;

	ISP_CHROMA_ATTR_S stChromaAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetChromaAttr(IspDev,&stChromaAttr), "XM_MPI_ISP_GetChromaAttr");
	stChromaAttr.enHue2Sel = HUE_SEL_GREEN;
	stChromaAttr.s16Hue2Ofst = 0xa0;
	stChromaAttr.u8SthG = 0xc0;
	ISPCHECK_RET(XM_MPI_ISP_SetChromaAttr(IspDev,&stChromaAttr), "XM_MPI_ISP_SetChromaAttr");
	
	ISP_WB_ATTR_S stWBAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetWBAttr(IspDev, &stWBAttr), "XM_MPI_ISP_GetWBAttr");
	stWBAttr.stAuto.u8ROfst = 0x80;
	stWBAttr.stAuto.u8BOfst = 0x80;
	ISPCHECK_RET(XM_MPI_ISP_SetWBAttr(IspDev, &stWBAttr), "XM_MPI_ISP_SetWBAttr");
	return 0;
}



