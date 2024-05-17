#include "autoscene_para.h"
//sensor: SC202CS

static ISP_SHADING_CALIBRATION_CFG_S g_Lsc_sc202cs = 
{
	8,	// ng
	4,	// nz
	//ch:1
	0x1b4,0x134,0x010,	// x,y,r0
	{0x000,0x001,0x005,0x00a,0x012,0x01a,0x024,0x031,0x03f,0x050,0x062,0x077,0x090,0x0aa,0x0cb,0x0ee,0x117,0x144,0x177,0x1aa,0x1e3,0x229,0x276,0x2c8,0x326,0x390,0x3e6,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff},	// lut61
	//ch:2
	0x1b8,0x135,0x010,	// x,y,r0
	{0x000,0x002,0x004,0x009,0x00e,0x014,0x01c,0x025,0x030,0x03b,0x049,0x059,0x06b,0x07f,0x095,0x0b1,0x0ce,0x0f1,0x116,0x13c,0x16b,0x19f,0x1d7,0x219,0x25e,0x2ac,0x2fa,0x348,0x38e,0x3dd,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff},	// lut61
	//ch:3
	0x1b7,0x129,0x010,	// x,y,r0
	{0x000,0x003,0x005,0x00a,0x00f,0x016,0x01e,0x027,0x032,0x03e,0x04b,0x05c,0x06e,0x081,0x099,0x0b5,0x0d3,0x0f5,0x11a,0x140,0x171,0x1a5,0x1df,0x222,0x268,0x2ad,0x2fc,0x34b,0x3a3,0x3fb,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff},	// lut61
	//ch:4
	0x1c2,0x127,0x010,	// x,y,r0
	{0x000,0x002,0x003,0x006,0x00d,0x013,0x01a,0x021,0x02b,0x036,0x042,0x051,0x061,0x075,0x08a,0x0a2,0x0bd,0x0de,0x0ff,0x11e,0x14a,0x174,0x1a5,0x1df,0x220,0x264,0x2ab,0x2f6,0x33e,0x383,0x3da,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff,0x3ff},	// lut61
};

const ISP_CMOS_SHARPEN_S gstSharpen_sc202cs_480_320 = {
/* bvalid */
	1,
	/* 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800 */
	/* Ud*/
	{0x40,0x40,0x38,0x30,0x28,0x20,0x14,0x14,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/* D*/
	{0x20,0x20,0x20,0x20,0x1C,0x18,0x14,0x14,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*ThrUd*/
	{0x06,0x08,0x08,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A},
	/*ThrD*/
	{0x04,0x06,0x08,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A},	
	/*Ud_H*/
	{0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08},
	/*D_H*/
	{0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08},
	/*ThrUd_H*/
	{0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03},
	/*ThrD_H*/
	{0x03,0x03,0x03,0x03,0x03,0x06,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08},
	/*EdgeStr*/
	{0x80,0x80,0x70,0x60,0x40,0x20,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c},
	/*EdgeThr*/
	{0x00,0x02,0x04,0x08,0x0A,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
	/*OverShoot*/
	{0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0xFF,0xFF,0xFF,0xFF,0xFF},
	/*UnderShoot*/
	{0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0xFF,0xFF,0xFF,0xFF,0xFF},
	/*DetailStr*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*Rsv1*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*Rsv2*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};

const ISP_CMOS_SHARPEN_S gstSharpen_sc202cs = {
/* bvalid */
	1,
	/* 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800 */
	/* Ud*/
	{0x40,0x40,0x38,0x30,0x28,0x20,0x14,0x14,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/* D*/
	{0x20,0x20,0x20,0x20,0x1C,0x18,0x14,0x14,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*ThrUd*/
	{0x06,0x08,0x08,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A},
	/*ThrD*/
	{0x04,0x06,0x08,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A},	
	/*Ud_H*/
	{0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08},
	/*D_H*/
	{0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08},
	/*ThrUd_H*/
	{0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03},
	/*ThrD_H*/
	{0x03,0x03,0x03,0x03,0x03,0x06,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08},
	/*EdgeStr*/
	{0x80,0x80,0x70,0x60,0x40,0x20,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c},
	/*EdgeThr*/
	{0x00,0x02,0x04,0x08,0x0A,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},
	/*OverShoot*/
	{0xC8,0xC8,0xC4,0xC0,0xB0,0x90,0x6d,0x4d,0x4d,0x69,0x69,0xFF,0xFF,0xFF,0xFF,0xFF},
	/*UnderShoot*/
	{0xC8,0xC8,0xC4,0xC0,0xB0,0x90,0x6d,0x4d,0x4d,0x69,0x69,0xFF,0xFF,0xFF,0xFF,0xFF},
	/*DetailStr*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*Rsv1*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	/*Rsv2*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};

static ISP_S32 fun_autoscene_sc202cs(ISP_DEV IspDev,ISP_EXT_AUTOSCENE_FUN_PARA_S *pstPara, AUTOSCENE_PARA_ADDR_S stAddr)
{
	static ISP_U32 s32CaptureSize = 0xff;
	AUTOSCENE_DN_S *pstAutoSceneDn = (AUTOSCENE_DN_S *)stAddr.au32Addr[0];
	if(pstAutoSceneDn==NULL)
	{
		return -1;
	}
	if(s32CaptureSize != pstPara->au32Rsv[0])
	{
		s32CaptureSize = pstPara->au32Rsv[0];
		pstPara->au32Rsv[1] |= 1<<IspDev;
	}

	switch(pstPara->au32Rsv[0])
	{
		case UVC_SIZE_480_320:		// 480*320
			pstAutoSceneDn->stCmosAgcPara.pastAgcSharp[0] = &gstSharpen_sc202cs_480_320;
			break;
		case UVC_SIZE_640_480:		// 640*480
		case UVC_SIZE_720P:			// 1280*720
		case UVC_SIZE_800_480:		// 800*480
		case UVC_SIZE_854_480:		// 854*480
		case UVC_SIZE_864_480:		// 864*480
		case UVC_SIZE_640_368:		// 640*368
		case UVC_SIZE_1600_1200:	// 1600*1200
		case UVC_SIZE_1280_480:		// 1280*480 double camera splice 2*640*480
		case UVC_SIZE_480_800:		// 480*800
		case UVC_SIZE_480_864:		// 480*864
		case UVC_SIZE_320_240:		// 320*240
		case UVC_SIZE_1024_600:		// 1024x600
		case UVC_SIZE_EXT_V1_NR:
		default:
			pstAutoSceneDn->stCmosAgcPara.pastAgcSharp[0] = &gstSharpen_sc202cs;
			break;
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
ISP_S32 autoscene_init_sc202cs(			ISP_DEV IspDev,
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
	if(stProductInfo.u8FunExt == FUN_EXT_DOOR_LOCK_FACE_USED)
	{
		pstAutoSceneDn->stAePara.u8AeLumNum = 2;
		pstAutoSceneDn->stAePara.au32ExpThr[0] = 12000;
		pstAutoSceneDn->stAePara.au32ExpThr[1] = 24000;
		pstAutoSceneDn->stAePara.au32ExpThr[2] = 24000;
		pstAutoSceneDn->stAePara.au32ExpThr[3] = 24000;
		pstAutoSceneDn->stAePara.au8AeLumColor[0] = 35;
		pstAutoSceneDn->stAePara.au8AeLumColor[1] = 35;
		pstAutoSceneDn->stAePara.au8AeLumColor[2] = 35;
		pstAutoSceneDn->stAePara.au8AeLumColor[3] = 35;
		pstAutoSceneDn->stAePara.au8AeLumBw[0] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumBw[1] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumBw[2] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumBw[3] = 0x30;
		pstAutoSceneDn->stAePara.au32GainDef[0] = 1*256;	// color
		pstAutoSceneDn->stAePara.au32GainMax[0] = 1*256;	// color
		pstAutoSceneDn->stAePara.au32GainDef[1] = pstAutoSceneDn->stAePara.au32GainDef[0];	// Bw
		pstAutoSceneDn->stAePara.au32GainMax[1] = pstAutoSceneDn->stAePara.au32GainMax[0];	// Bw
		pstAutoSceneDn->stAePara.au8AeStrategyDef[0] = AE_EXP_NORMAL;
		pstAutoSceneDn->stAePara.au8AeStrategyStrDef[0] = 0x20;
		pstAutoSceneDn->stAePara.au8AeStrategyDef[1] = AE_EXP_NORMAL;
		pstAutoSceneDn->stAePara.au8AeStrategyStrDef[1] = 0x20;
	}
	else if(stProductInfo.u8FunExt == FUN_EXT_DOOR_LOCK_FACE)
	{
		pstAutoSceneDn->stAePara.u8AeLumNum = 2;
		pstAutoSceneDn->stAePara.au32ExpThr[0] = 12000;
		pstAutoSceneDn->stAePara.au32ExpThr[1] = 24000;
		pstAutoSceneDn->stAePara.au32ExpThr[2] = 24000;
		pstAutoSceneDn->stAePara.au32ExpThr[3] = 24000;
		pstAutoSceneDn->stAePara.au8AeLumColor[0] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumColor[1] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumColor[2] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumColor[3] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumBw[0] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumBw[1] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumBw[2] = 0x30;
		pstAutoSceneDn->stAePara.au8AeLumBw[3] = 0x30;
		pstAutoSceneDn->stAePara.au32GainDef[0] = 8*256;	// color
		pstAutoSceneDn->stAePara.au32GainMax[0] = 8*256;	// color
		pstAutoSceneDn->stAePara.au32GainDef[1] = pstAutoSceneDn->stAePara.au32GainDef[0];	// Bw
		pstAutoSceneDn->stAePara.au32GainMax[1] = pstAutoSceneDn->stAePara.au32GainMax[0];	// Bw
		pstAutoSceneDn->stAePara.au8AeStrategyDef[0] = AE_EXP_NORMAL;
		pstAutoSceneDn->stAePara.au8AeStrategyStrDef[0] = 0x20;
		pstAutoSceneDn->stAePara.au8AeStrategyDef[1] = AE_EXP_NORMAL;
		pstAutoSceneDn->stAePara.au8AeStrategyStrDef[1] = 0x20;
	}
	else
	{
		pstAutoSceneDn->stAePara.u8AeLumNum = 2;
		pstAutoSceneDn->stAePara.au32ExpThr[0] = 12000;
		pstAutoSceneDn->stAePara.au32ExpThr[1] = 24000;
		pstAutoSceneDn->stAePara.au32ExpThr[2] = 24000;
		pstAutoSceneDn->stAePara.au32ExpThr[3] = 24000;
		pstAutoSceneDn->stAePara.u8AeLumNum = 4;
		pstAutoSceneDn->stAePara.au32ExpThr[0] = 800;
		pstAutoSceneDn->stAePara.au32ExpThr[1] = 9000;
		pstAutoSceneDn->stAePara.au32ExpThr[2] = 24000;
		pstAutoSceneDn->stAePara.au32ExpThr[3] = 96000;
		pstAutoSceneDn->stAePara.au8AeLumColor[0] = 54;
		pstAutoSceneDn->stAePara.au8AeLumColor[1] = 54;
		pstAutoSceneDn->stAePara.au8AeLumColor[2] = 54;
		pstAutoSceneDn->stAePara.au8AeLumColor[3] = 42;
		pstAutoSceneDn->stAePara.au8AeLumBw[0] = 42;
		pstAutoSceneDn->stAePara.au8AeLumBw[1] = 36;
		pstAutoSceneDn->stAePara.au8AeLumBw[2] = 34;
		pstAutoSceneDn->stAePara.au8AeLumBw[3] = 30;
		pstAutoSceneDn->stAePara.au32GainDef[0] = 64*256;	// color
		pstAutoSceneDn->stAePara.au32GainMax[0] = 64*256;	// color
		pstAutoSceneDn->stAePara.au32GainDef[1] = pstAutoSceneDn->stAePara.au32GainDef[0];	// Bw
		pstAutoSceneDn->stAePara.au32GainMax[1] = pstAutoSceneDn->stAePara.au32GainMax[0];	// Bw
		pstAutoSceneDn->stAePara.au8AeStrategyDef[0] = AE_EXP_NORMAL;
		pstAutoSceneDn->stAePara.au8AeStrategyStrDef[0] = 0x10;
		pstAutoSceneDn->stAePara.au8AeStrategyDef[1] = AE_EXP_NORMAL;
		pstAutoSceneDn->stAePara.au8AeStrategyStrDef[1] = 0x10;
	}
	// fun
	pstAutoSceneDn->pfun_autoscene_sns = fun_autoscene_sc202cs;
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
	ISP_WB_ATTR_S stWBAttr;
	ISP_EXPOSURE_ATTR_S  pstExpAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetChromaAttr(IspDev,&stChromaAttr), "XM_MPI_ISP_GetChromaAttr");
	ISPCHECK_RET(XM_MPI_ISP_GetWBAttr(IspDev, &stWBAttr), "XM_MPI_ISP_GetWBAttr");
	ISPCHECK_RET(XM_MPI_ISP_GetExposureAttr(IspDev, &pstExpAttr), "XM_MPI_ISP_GetExposureAttr");
	switch(stProductInfo.u8FunExt)
	{
		case FUN_EXT_ONE_SENSOR:
			stChromaAttr.enHue2Sel = HUE_SEL_GREEN;
			stChromaAttr.s16Hue2Ofst = 0x0;
			stChromaAttr.u8SthG = 0x80;
			break;
		case FUN_EXT_DOOR_LOCK_FACE:
			stChromaAttr.enHue2Sel = HUE_SEL_GREEN;
			stChromaAttr.s16Hue2Ofst = 0x00;
			stChromaAttr.u8SthG = 0xA0;
			stWBAttr.enOpType = OP_TYPE_MANUAL;
			stWBAttr.stManual.u16Rgain = 496;
			stWBAttr.stManual.u16Ggain = 256;
			stWBAttr.stManual.u16Bgain = 414;
			pstExpAttr.stManual.u32ExpLine = 1500;
			pstExpAttr.stManual.s32AGain = 0x2000;
			pstExpAttr.stManual.s32DGain = 0x400;
			pstExpAttr.stManual.s32IspGain = 0x400;
			pstExpAttr.enOpType = OP_TYPE_MANUAL;
			pstExpAttr.stManual.bManualExpLineEnable = 1;
			pstExpAttr.stManual.bManualAGainEnable = 1;
			pstExpAttr.stManual.bManualDGainEnable = 1;
			pstExpAttr.stManual.bManualIspGainEnable = 1;
			pstExpAttr.stManual.bManualExpEnable = 1;
			break;
		case FUN_EXT_DOOR_LOCK_FACE_UVC:
			stChromaAttr.enHue2Sel = HUE_SEL_GREEN;
			stChromaAttr.s16Hue2Ofst = 0x00;
			stChromaAttr.u8SthR = 0x80;
			stChromaAttr.u8SthYe = 0x60;
			stChromaAttr.u8SthG = 0x50;
			stChromaAttr.u8SthCy = 0x60;
			stChromaAttr.u8SthB = 0x40;
			stWBAttr.stAuto.u8ROfst = 126;
			stWBAttr.stAuto.u8BOfst = 0x80;
			stWBAttr.stAuto.u8GmOfst = 122;
			stWBAttr.stAuto.u8BindSyncChn = 0x40;
			pstExpAttr.stManual.u32ExpLine = 1500;
			pstExpAttr.stManual.s32AGain = 0x2000;
			pstExpAttr.stManual.s32DGain = 0x400;
			pstExpAttr.stManual.s32IspGain = 0x400;
			pstExpAttr.enOpType = OP_TYPE_MANUAL;
			pstExpAttr.stManual.bManualExpLineEnable = 1;
			pstExpAttr.stManual.bManualAGainEnable = 1;
			pstExpAttr.stManual.bManualDGainEnable = 1;
			pstExpAttr.stManual.bManualIspGainEnable = 1;
			pstExpAttr.stManual.bManualExpEnable = 1;
			ISP_SHADING_CALIBRATION_CFG_S *pstLSCCaliCfg;
			ISP_SHADING_ATTR_S pstShadingAttr;
			pstLSCCaliCfg = &g_Lsc_sc202cs;
			ISPCHECK_RET(XM_MPI_ISP_SetShadingCalibration(IspDev, pstLSCCaliCfg), "XM_MPI_ISP_SetShadingCalibration");
			ISPCHECK_RET(XM_MPI_ISP_GetShadingAttr(IspDev, &pstShadingAttr), "XM_MPI_ISP_GetExposureAttr");
			pstShadingAttr.bEnable = ISP_TRUE;
			pstShadingAttr.enOpType = OP_TYPE_AUTO;
			pstShadingAttr.stAuto.u8GlobalStr = 0x80;
			for(ISP_U8 u8i = 0;u8i<ISP_AUTO_STENGTH_NUM;u8i++)
			{
				pstShadingAttr.stAuto.au8Str[u8i] = 0x10;
			}
			ISPCHECK_RET(XM_MPI_ISP_SetShadingAttr(IspDev, &pstShadingAttr), "XM_MPI_ISP_GetExposureAttr");
			break;
		default:
			break;
	}
	ISPCHECK_RET(XM_MPI_ISP_SetChromaAttr(IspDev,&stChromaAttr), "XM_MPI_ISP_SetChromaAttr");
	ISPCHECK_RET(XM_MPI_ISP_SetWBAttr(IspDev, &stWBAttr), "XM_MPI_ISP_SetWBAttr");
	ISPCHECK_RET(XM_MPI_ISP_SetExposureAttr(IspDev, &pstExpAttr), "XM_MPI_ISP_SetExposureAttr");
	return 0;
}



