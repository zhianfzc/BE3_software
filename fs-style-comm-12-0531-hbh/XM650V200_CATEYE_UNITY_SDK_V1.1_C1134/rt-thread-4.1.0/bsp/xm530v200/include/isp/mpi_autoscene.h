#ifndef __MPI_AUTOSCENE_H__
#define __MPI_AUTOSCENE_H__ 

#ifdef __cplusplus
extern "C" {
#endif
#include "isp_comm_isp.h"
typedef enum enAUTOSCENE_DNC_STATUS
{
	DNS_COLOR = 0,
	DNS_BW = 1,
	DNS_BUTT,
}AUTOSCENE_DNC_STATUS;

typedef struct stAUTOSCENE_PRODUCT_INFO_S
{
	ISP_U32 u32ProductType;
	ISP_U32 u32SnsType;
	WDR_MODE_E	enWdrMode;
	ISP_U32 au32Rsv[4];
}AUTOSCENE_PRODUCT_INFO_S;

typedef struct stAS_CONFIG_CH
{
/*****************************************
1_0 u16LedIO: (软控灯的GPIO)
	bit0~bit15:	(红外灯GPIO，10进制表示)
	bit16~bit31: (白光灯GPIO，10进制表示)
*****************************************/	
	ISP_U32 u32SoftLedIO;

	ISP_U32 au32Rsv[3];
/*****************************************
1_1	红外灯板 GPIO
*****************************************/
	ISP_U16 u16InfraredIO;
/*****************************************
1_2 IRCUT GPIO _A:[0]   _B:[1]
*****************************************/
	ISP_U16 au16IrCutIO[2];
/*****************************************
1_3	用来测试IRC功能是否完好的两个IO之一(其中一个IO固定为ADC)
*****************************************/
	ISP_U16 u16IrCutTestIO;

	ISP_U16 au16Rsv[4];
/*****************************************
1_3 u8PolaritySwap: 
	bit0:	红外灯板输入极性
	bit1:	红外灯驱动极性
	bit2: 	白光灯驱动极性
	bit3: 	IRCUT输入极性
*****************************************/
	ISP_U8 u8PolaritySwap;

/*****************************************
1_4 u8IRLed（软控灯、软光敏）
	bit0: 	(1: 软件控制灯(WL/IR)        0: 硬件控制灯(常规))
	bit4:	(1: 硬光敏					0: 软光敏) --- bit0为1时有效
*****************************************/
	ISP_U8 u8IRLed;


/*****************************************
1_5 u8LedMode: (LED类型用于接口camera_get_ledMode的返回)
	u8LedMode:	0xAB(	A:	0: IR   仅红外灯
							1: WL   白光灯(CMS 可配置成双光)
							2: Double (仅双光---双光枪机)
							3: WL(only) (仅白光)
						B:	备用
*****************************************/	
	ISP_U8 u8LedMode;

/*****************************************
1_6 u8LedAlarmAlg: (双光 报警算法选择)
	u8LedAlarmAlg:	0: 人形算法
					1: 移动侦测
*****************************************/
	ISP_U8 u8LedAlarmAlg;

/*****************************************
1_7 u8LedAlarmMode: (定义双光灯(智能警戒)，触发报警后的，灯处理模式)
	u8LedAlarmMode:	0x03: 阻吓模式（白灯闪烁）
					0x04: 阻吓模式（白光灯常亮）
					其他: 常规白灯处理(亮白灯+全彩)	
*****************************************/
	ISP_U8 u8LedAlarmMode;
/*****************************************
1_8 u8LowFrequency(低功耗等产品标记)
*****************************************/
	ISP_U8 u8LowFrequency;

	ISP_U8 au8Rsv[8];
}AS_CONFIG_CH;

typedef struct stAUTOSCENE_CONFIG 
{
	ISP_FUN_EXT u8FunExt;
	ISP_U8 u8ChnMask;
	ISP_U8 au8Rsv[2];
/*****************************************
0_1 产品型号
*****************************************/
	ISP_U32 u32ProductType;
/*****************************************
0_2 DSP型号
*****************************************/
	ISP_U32 u32DspType;
/*****************************************
0_3~8 保留
*****************************************/
	ISP_U32 au32Rsv[7];

	AS_CONFIG_CH astChnAttr[ISP_NUM_MAX];
}AUTOSCENE_CONFIG;

ISP_S32 XM_MPI_AUTOSCENE_SetConfig(AUTOSCENE_CONFIG stConfig);

ISP_S32 XM_MPI_AUTOSCENE_Init(ISP_DEV IspDev,AUTOSCENE_PRODUCT_INFO_S stProductInfo);
ISP_S32 XM_MPI_AUTOSCENE_Run(ISP_U8 u8ChnMask, ISP_S32 s32Rsv);
ISP_S32 XM_MPI_AUTOSCENE_Exit(ISP_DEV IspDev);
ISP_S32 XM_MPI_AUTOSCENE_AI_Face_Lum_Double(ISP_S32 s32chn,ISP_U32 u32flag,ISP_U32 u32Level,ISP_U32 u32Max,ISP_U32 u32Min);

ISP_S32 XM_MPI_AUTOSCENE_SetLumTarget(ISP_DEV IspDev, ISP_S32 s32Tgt);
ISP_S32 XM_MPI_AUTOSCENE_GetLumTarget(ISP_DEV IspDev, ISP_S32 *ps32Tgt);

ISP_S32 XM_MPI_AUTOSCENE_SetGainLvl(ISP_DEV IspDev, ISP_S32 s32En, ISP_S32 s32Level);
ISP_S32 XM_MPI_AUTOSCENE_GetGainLvl(ISP_DEV IspDev, ISP_S32 *ps32En, ISP_S32 *ps32Level);

ISP_S32 XM_MPI_AUTOSCENE_GetBlc(ISP_DEV IspDev, ISP_S32 *ps32En, ISP_S32 *ps32Level);
ISP_S32 XM_MPI_AUTOSCENE_SetBlc(ISP_DEV IspDev, ISP_S32 s32En, ISP_S32 s32Level);

ISP_S32 XM_MPI_AUTOSCENE_GetHlc(ISP_DEV IspDev, ISP_S32 *ps32En, ISP_S32 *ps32Level);
ISP_S32 XM_MPI_AUTOSCENE_SetHlc(ISP_DEV IspDev, ISP_S32 s32En, ISP_S32 s32Level);

ISP_S32 XM_MPI_AUTOSCENE_Debug(ISP_DEV IspDev, ISP_S32 s32Data, ISP_CHAR *pstrData);



#ifdef __cplusplus
}
#endif

#endif

