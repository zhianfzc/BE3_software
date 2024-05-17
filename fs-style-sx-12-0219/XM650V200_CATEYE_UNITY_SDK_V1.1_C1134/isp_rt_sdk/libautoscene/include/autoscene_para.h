#ifndef __AUTOSCENE_PARA_H__
#define __AUTOSCENE_PARA_H__ 

#ifdef __cplusplus
extern "C" {
#endif
#include "isp_math.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "isp_comm_sns.h"
#include "isp_print.h"
#include "mpi_autoscene.h"
#include "isp_sns_ctrl.h"
#include "mpi_ispapp.h"

typedef struct stISP_EXT_AUTOSCENE_FUN_PARA_S
{	
	unsigned int u32DnMode;		// 0:Color  1:BW
	unsigned int u32Exposure;
	unsigned int u32Shut;
	unsigned int u32AllGain;	// *1024
	unsigned int u32AiStrategyStr;		// AI相关策略强度
	unsigned int au32Rsv[7];	// au32Rsv[0]:保存uvc输出图像大小 au32Rsv[1]:刷新图像参数
}ISP_EXT_AUTOSCENE_FUN_PARA_S;


typedef struct stAUTOSCENC_DNC_SOFTIR_THR
{
	int s32ColorExp;
	int s32ColorDlt;
	int s32ColorDlt2;
	int s32BwExp;
}AUTOSCENC_DNC_SOFTIR_THR;


typedef struct autoscene_ae_lum_s{
	// ae strategy
	ISP_U8 au8AeStrategyDef[2];		//([0]:Color  [1]:Bw)
	ISP_U8 au8AeStrategyStrDef[2];	//([0]:Color  [1]:Bw)
	// ae lum
	ISP_U8 u8AeLumNum;	// [0,4] 
	ISP_U8 au8AeLumColor[4];
	ISP_U8 au8AeLumBw[4];
	ISP_U32 au32ExpThr[4];
	
	ISP_U8 u8RatioSlopeEnable;
	ISP_U16 au16RatioSlopeColor[4];
	ISP_U16 au16RatioSlopeBw[4];

	// gain
	ISP_U32 au32GainDef[2];		// *256 ([0]:Color  [1]:Bw)
	ISP_U32 au32GainMax[2];		// *256 ([0]:Color  [1]:Bw)

	// EEShutter
	ISP_U32 au32EShutterThr[2];	// >=[0]:Enable;  <=[1]:Disable;  au32EshutterThr[1]<au32EshutterThr[0]
	
	// softIR
	AUTOSCENC_DNC_SOFTIR_THR astSoftIr[6];	// 默认使用astSoftIr[3]	
	
}AUTOSCENE_AE_S;

typedef struct autoscene_para_addr_s{
	ISP_U32 au32Addr[2];
	ISP_U32 au32Rsv[8];
}AUTOSCENE_PARA_ADDR_S;

typedef struct autoscene_dn_s{
	// DayNight(Color/Bw cmos)
	ISP_CMOS_DN_PARA_S stCmosAgcPara;
	// Ae parameter
	AUTOSCENE_AE_S stAePara;
	// function
	ISP_S32 (*pfun_autoscene_sns)(ISP_DEV isp_dev, ISP_EXT_AUTOSCENE_FUN_PARA_S *pstPara, AUTOSCENE_PARA_ADDR_S stAddr);
}AUTOSCENE_DN_S;


typedef struct autoscene_menu_s{
	ISP_U8 u8AeLum;			// [0,100] def:50
	ISP_U8 u8AeGainMax;		// [0,100] def:50
	ISP_U8 u8Blc;			// [0,100] def:0
	ISP_U8 u8Hlc;			// [0,100] def:0
	ISP_U8 au8Rsv[14];
}AUTOSCENE_MENU_S;


typedef struct autoscene_inner_info_s{
	ISP_U8 u8AiStrategyStr;	// AI相关策略强度
	ISP_U8 u8InfraredStatus;
	ISP_U8 u8LedStab;		// 灯光状态稳定
	ISP_U8 u8DnStatus;	
	ISP_U8 au8Rsv[4];
	ISP_U16 au16Rsv[4];
	ISP_U32 u32ExpLines;
	ISP_U32 u32ExpAllGain;
	ISP_U32 au32Rsv[2];
}AUTOSCENE_INNER_INFO;

typedef enum autoscene_dn_status_s{
	AS_DN_STATUS_COLOR,
	AS_DN_STATUS_BW,
	AS_DN_STATUS_BUTT,
}AUTOSCENE_DN_STATUS_E;

extern AUTOSCENE_CONFIG gstConfig;

ISP_S32 dnc_set_config(AUTOSCENE_CONFIG *pstConfig);
ISP_S32 dnc_init(ISP_DEV IspDev,AUTOSCENE_AE_S *pstAttr);
ISP_S32 dnc_exit(ISP_DEV IspDev);
ISP_S32 dnc_get_dn_status(ISP_DEV IspDev, AUTOSCENE_DN_STATUS_E *penStatus);
ISP_S32 dnc_get_inner_status(ISP_DEV IspDev, AUTOSCENE_INNER_INFO *pstInfo);

ISP_S32 fvideo_get_capture_size(ISP_DEV IspDev, ISP_S32 *ps32CaptureSize);

ISP_S32 autoscene_init_sc223a(		ISP_DEV IspDev, ISP_U32 u32ProductType, AUTOSCENE_PARA_ADDR_S stAddr);
ISP_S32 autoscene_init_imx307(		ISP_DEV IspDev, ISP_U32 u32ProductType, AUTOSCENE_PARA_ADDR_S stAddr);
ISP_S32 autoscene_init_imx335(		ISP_DEV IspDev, ISP_U32 u32ProductType, AUTOSCENE_PARA_ADDR_S stAddr);
ISP_S32 autoscene_init_sc3335(		ISP_DEV IspDev, ISP_U32 u32ProductType, AUTOSCENE_PARA_ADDR_S stAddr);
ISP_S32 autoscene_init_sc3338(		ISP_DEV IspDev, ISP_U32 u32ProductType, AUTOSCENE_PARA_ADDR_S stAddr);
ISP_S32 autoscene_init_sc401ai(		ISP_DEV IspDev, ISP_U32 u32ProductType, AUTOSCENE_PARA_ADDR_S stAddr);
ISP_S32 autoscene_debug_sc401ai(	ISP_DEV IspDev, ISP_S32 s32Data, ISP_CHAR *pstrData, ISP_CMOS_DN_PARA_S *pstCmosAgcDn);
ISP_S32 autoscene_init_sc4336(		ISP_DEV IspDev, ISP_U32 u32ProductType, AUTOSCENE_PARA_ADDR_S stAddr);
ISP_S32 autoscene_init_sc500ai(		ISP_DEV IspDev, ISP_U32 u32ProductType, AUTOSCENE_PARA_ADDR_S stAddr);
ISP_S32 autoscene_init_sc035gs(		ISP_DEV IspDev, ISP_U32 u32ProductType, AUTOSCENE_PARA_ADDR_S stAddr);
ISP_S32 autoscene_init_sc307e(		ISP_DEV IspDev, ISP_U32 u32ProductType, AUTOSCENE_PARA_ADDR_S stAddr);
ISP_S32 autoscene_init_sc2335(		ISP_DEV IspDev, ISP_U32 u32ProductType, AUTOSCENE_PARA_ADDR_S stAddr);
ISP_S32 autoscene_init_sc2336(		ISP_DEV IspDev, ISP_U32 u32ProductType, AUTOSCENE_PARA_ADDR_S stAddr);
ISP_S32 autoscene_init_sc200ai(		ISP_DEV IspDev, ISP_U32 u32ProductType, AUTOSCENE_PARA_ADDR_S stAddr);
ISP_S32 autoscene_init_sc202cs(ISP_DEV IspDev, ISP_U32 u32ProductType, AUTOSCENE_PARA_ADDR_S stAddr);
ISP_S32 autoscene_init_sc1346(ISP_DEV IspDev, ISP_U32 u32ProductType, AUTOSCENE_PARA_ADDR_S stAddr);



#ifdef __cplusplus
}
#endif

#endif


