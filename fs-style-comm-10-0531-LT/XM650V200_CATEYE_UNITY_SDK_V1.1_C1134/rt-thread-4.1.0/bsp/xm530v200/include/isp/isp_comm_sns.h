#ifndef __ISP_COMM_SNS_H__
#define __ISP_COMM_SNS_H__

#include "isp_type.h"
#include "isp_comm_isp.h"
#include "isp_i2c.h"
#include "isp_ae_comm.h"
#include "isp_awb_comm.h"

#define HD1080_25P_LINES	(1125)
#define HD1080_30P_LINES	(1125)


typedef struct stISP_CMOS_AGC_TABLE_S
{
	ISP_BOOL bValid;
	ISP_U8  au8SnrThresh[ISP_AUTO_STENGTH_NUM];     	/* adjust 2Ddenoise strength, different iso with different strength */
	ISP_U8  au8Drc[ISP_AUTO_STENGTH_NUM];
	ISP_U8  au8Defog[ISP_AUTO_STENGTH_NUM]; 
	ISP_U8  au8DefogVpss[ISP_AUTO_STENGTH_NUM]; 
	ISP_U8  au8DyDpc[ISP_AUTO_STENGTH_NUM]; 		/* adjust DynamicDpc strength, different iso with different strength */
	ISP_U8  au8Saturation[ISP_AUTO_STENGTH_NUM];
	ISP_U16 au16Blc[ISP_AUTO_STENGTH_NUM];			/* adjust blackLevel,different iso with different sharp strength */
	ISP_U16  au16Contrast[ISP_AUTO_STENGTH_NUM];		/*adjust  Y_Tran gamma,iso with different sharp strength*/
} ISP_CMOS_AGC_TABLE_S;
typedef struct stISP_CMOS_NOISE_TABLE_S
{
    ISP_BOOL bValid;
    
    ISP_U8   au8NoiseProfileWeightLut[128];
    ISP_U8   au8DemosaicWeightLut[128];
} ISP_CMOS_NOISE_TABLE_S;

typedef struct stISP_CMOS_DEMOSAIC_S
{
    ISP_BOOL bValid;
    
    ISP_U8   u8VhSlope;
    ISP_U8   u8AaSlope;
    ISP_U8   u8VaSlope;
    ISP_U8   u8UuSlope;
    ISP_U8   u8SatSlope;
    ISP_U8   u8AcSlope;
    ISP_U16  u16VhThresh;
    ISP_U16  u16AaThresh;
    ISP_U16  u16VaThresh;
    ISP_U16  u16UuThresh;
    ISP_U16  u16SatThresh;
    ISP_U16  u16AcThresh;
} ISP_CMOS_DEMOSAIC_S;

typedef struct stISP_CMOS_DRC_S
{
    ISP_U8   u8DrcBlack;
    ISP_U8   u8DrcVs;         /* variance space */
    ISP_U8   u8DrcVi;         /* variance intensity */
    ISP_U8   u8DrcSm;         /* slope max */
    ISP_U16  u16DrcWl;        /* white level */
} ISP_CMOS_DRC_S;

#define LUT_FACTOR (8)
#define GAMMA_FE_LUT_SIZE ((1<<LUT_FACTOR)+1)
typedef struct stISP_CMOS_GAMMAFE_S
{
    ISP_BOOL bValid;         /* wdr sensor should set */
    
    ISP_U16  au16Gammafe[GAMMA_FE_LUT_SIZE];
} ISP_CMOS_GAMMAFE_S;

typedef struct stISP_CMOS_DENOISE_S
{
    ISP_U8   u8SinterThresh;     /* sinter threshold */
    
    ISP_U8   u8NoiseProfile;     /* two different noise profile */ 
    ISP_U16  u16Nr0;             /* nr0 for noise profile 2 */
    ISP_U16  u16Nr1;             /* nr1 for noise profile 2 */
} ISP_CMOS_DENOISE_S;

typedef struct stISP_CMOS_COMM_S
{
    ISP_U8   u8Rggb;             /* rggb start sequence */    
    ISP_U8   u8BalanceFe;
} ISP_CMOS_COMM_S;

typedef struct stISP_CMOS_SHADING_S
{
	ISP_BOOL bValid;
	ISP_U8 u8Ng;
	ISP_U8 u8Nz;
	ISP_U8 u8Str;	// 0x80
	ISP_U16 u16RCenterX;
	ISP_U16 u16RCenterY;
	ISP_U16 u16RRadius0;
	ISP_U16 au16RShadingTbl[SHADING_LUT_NUM];
	
	ISP_U16 u16GrCenterX;
	ISP_U16 u16GrCenterY;
	ISP_U16 u16GrRadius0;
	ISP_U16 au16GrShadingTbl[SHADING_LUT_NUM];
	
	ISP_U16 u16GbCenterX;
	ISP_U16 u16GbCenterY;	
	ISP_U16 u16GbRadius0;
	ISP_U16 au16GbShadingTbl[SHADING_LUT_NUM];
	
	ISP_U16 u16BCenterX;
	ISP_U16 u16BCenterY;
	ISP_U16 u16BRadius0;
	ISP_U16 au16BShadingTbl[SHADING_LUT_NUM];
} ISP_CMOS_SHADING_S;


typedef struct stISP_CMOS_BUILDIN_DEC_S
{
	ISP_BOOL bValid;
	ISP_U32 u32IdxNum;	//[4,8]
	ISP_U32 au32X[8];
	ISP_U32 au32Y[8];
	ISP_U32 au32K[8];
}ISP_CMOS_BUILDIN_DEC_S;

typedef struct stISP_CMOS_GAMMA_S
{
    ISP_BOOL bValid;
    ISP_U16  au16Gamma[GAMMA_NODE_NUMBER];
} ISP_CMOS_GAMMA_S;

typedef struct stISP_CMOS_VPSS_3DNR_S
{
	ISP_U8  u8Enable;
	ISP_U8 au8FltThr[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8Df_RatC[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8Df1_RatioY[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8Df1_RatioC[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8MovThr_Max[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8Alpha_Ratio[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8Alpha_Min[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8IdxThr_Max[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8IdxThr_Min[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8Idx_Max[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8FidY[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8FidC[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8FidY_Min[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8FidC_Min[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8YSfThr_Min[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8CSfThr_Min[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8YTfThr_Min[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8CTfThr_Min[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8YAntiRsdlSpd[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8CAntiRsdlSpd[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8YAntiRsdlThr[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8CAntiRsdlThr[ISP_AUTO_STENGTH_NUM];	
	ISP_U16 au16YSfThr_K[ISP_AUTO_STENGTH_NUM];
	ISP_U16 au16CSfThr_K[ISP_AUTO_STENGTH_NUM];
	ISP_U16 au16YTfThr_K[ISP_AUTO_STENGTH_NUM];
	ISP_U16 au16CTfThr_K[ISP_AUTO_STENGTH_NUM];
	ISP_U16 au16MovThr1[ISP_AUTO_STENGTH_NUM];
	ISP_U16 au16MovThr2[ISP_AUTO_STENGTH_NUM];
	ISP_U16 au16MovThr3[ISP_AUTO_STENGTH_NUM];
} ISP_CMOS_VPSS_3DNR_S;

typedef struct stISP_CMOS_SHARPEN0_S
{
	ISP_U8 u8Enable;
	ISP_U8 au8Ud[ISP_AUTO_STENGTH_NUM];			// 纹理锐化高频强度
	ISP_U8 au8D[ISP_AUTO_STENGTH_NUM]; 			// 纹理锐化中频强度
	ISP_U8 au8EdgeStr[ISP_AUTO_STENGTH_NUM];		// 边界锐化强度
	ISP_U8 au8EdgeThr[ISP_AUTO_STENGTH_NUM];		// 边界锐化阈值
	ISP_U8 au8EdgeSlope[ISP_AUTO_STENGTH_NUM];	// 边界锐化斜率
	ISP_U8 au8TextureStr[ISP_AUTO_STENGTH_NUM];		// 纹理锐化强度
	ISP_U8 au8TextureThr[ISP_AUTO_STENGTH_NUM];		// 纹理锐化阈值
	ISP_U8 au8TextureSlope[ISP_AUTO_STENGTH_NUM];	// 纹理锐化斜率
	ISP_U8 au8OverShoot[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8UnderShoot[ISP_AUTO_STENGTH_NUM];
} ISP_CMOS_SHARPEN0_S;

typedef struct stISP_CMOS_SHARPEN_S
{
	ISP_U8 u8Enable;
	ISP_U8 au8Ud[ISP_AUTO_STENGTH_NUM];		// 纹理锐化高频强度
	ISP_U8 au8D[ISP_AUTO_STENGTH_NUM]; 		// 纹理锐化中频强度
	ISP_U8 au8ThrUd[ISP_AUTO_STENGTH_NUM]; 	// 纹理锐化阈值(高频)
	ISP_U8 au8ThrD[ISP_AUTO_STENGTH_NUM]; 	// 纹理锐化阈值(中频)
	ISP_U8 au8Ud_H[ISP_AUTO_STENGTH_NUM];		// 水平锐化高频强度
	ISP_U8 au8D_H[ISP_AUTO_STENGTH_NUM]; 		// 水平锐化中频强度
	ISP_U8 au8ThrUd_H[ISP_AUTO_STENGTH_NUM];	// 水平锐化阈值(高频)
	ISP_U8 au8ThrD_H[ISP_AUTO_STENGTH_NUM]; 	// 水平锐化阈值(中频)
	ISP_U8 au8EdgeStr[ISP_AUTO_STENGTH_NUM];		// 边界锐化强度
	ISP_U8 au8EdgeThr[ISP_AUTO_STENGTH_NUM];		// 边界锐化阈值
	ISP_U8 au8OverShoot[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8UnderShoot[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8DetailStr[ISP_AUTO_STENGTH_NUM];	
	ISP_U8 au8Rsv1[ISP_AUTO_STENGTH_NUM];	// 保留
	ISP_U8 au8Rsv2[ISP_AUTO_STENGTH_NUM];	// 保留
} ISP_CMOS_SHARPEN_S;


typedef struct stISP_CMOS_YCNR_S
{
	ISP_U8 u8Enable;
	ISP_U8 au8Thr[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8Str[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8MixThr[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8Wt1[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8Wt2[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8Wt3[ISP_AUTO_STENGTH_NUM];
	ISP_U16 au16K[ISP_AUTO_STENGTH_NUM];
} ISP_CMOS_YCNR_S;

typedef struct stISP_CMOS_BPU_3DNR_S
{
	ISP_U8 u8Enable;
	ISP_U8 au8MdTh[ISP_AUTO_STENGTH_NUM];		// 动静判决阈值
	ISP_U8 au8TfOfst_S[ISP_AUTO_STENGTH_NUM];	// 静止区域时域增强 	   
	ISP_U8 au8TfStrength[ISP_AUTO_STENGTH_NUM]; 	// 时域强度
	ISP_U8 au8SfStrength[ISP_AUTO_STENGTH_NUM]; 	// 空域强度
	ISP_U8 au8Rsv[ISP_AUTO_STENGTH_NUM];			// 保留
} ISP_CMOS_BPU_3DNR_S;

typedef struct stISP_CMOS_LOCAL_CAC_S
{
	ISP_U8 u8Enable;
	ISP_U16 au16Thresh[ISP_AUTO_STENGTH_NUM];
	ISP_U16 au16GThresh[ISP_AUTO_STENGTH_NUM];
	ISP_U16 au16YThresh[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8DePurpleStrR[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8DePurpleStrB[ISP_AUTO_STENGTH_NUM];
} ISP_CMOS_LOCAL_CAC_S;

typedef struct stISP_CMOS_LTM_S
{
	ISP_U8 u8Enable;
	ISP_U8 au8Rix[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8KjMax[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8KjMin[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8StrD[ISP_AUTO_STENGTH_NUM];
    ISP_U8 au8StrB[ISP_AUTO_STENGTH_NUM];
} ISP_CMOS_LTM_S;

typedef struct stISP_CMOS_2DNR_S
{
	ISP_U8 u8Enable;
	ISP_U16 au16Thr[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8KThr[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8KCoef[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8KMin[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8Thr_c[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8KThr_c[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8KCoef_c[ISP_AUTO_STENGTH_NUM];
	ISP_U8 au8KMin_c[ISP_AUTO_STENGTH_NUM];
} ISP_CMOS_2DNR_S;

typedef struct stISP_CMOS_DEFAULT_S
{    
    ISP_CMOS_COMM_S         stComm;
    ISP_CMOS_AGC_TABLE_S    stAgcTbl;
    ISP_CMOS_GAMMA_S        stGamma;
    ISP_CMOS_SHADING_S      stShading;
	ISP_CMOS_BUILDIN_DEC_S  stBuildInDec;
	ISP_CMOS_VPSS_3DNR_S    stVpss3DNr;
	ISP_CMOS_SHARPEN_S		stSharpen;
	ISP_CMOS_YCNR_S 		stYcNr;
	ISP_CMOS_LOCAL_CAC_S	stLocalCAC;
	ISP_CMOS_LTM_S			stLtm;
	ISP_CMOS_2DNR_S			st2DNr;
} ISP_CMOS_DEFAULT_S;

typedef enum stISP_CMOS_MODE_E
{
    ISP_CMOS_MODE_PIXEL_DETECT = 0,
    ISP_CMOS_MODE_WDR,
    ISP_CMOS_MODE_RESOLUTION,

    ISP_CMOS_MODE_BUTT,
} ISP_CMOS_MODE_E;

typedef struct stISP_CMOS_SENSOR_MAX_RESOLUTION_S
{
  ISP_U32  u32MaxWidth;
  ISP_U32  u32MaxHeight;

}ISP_CMOS_SENSOR_MAX_RESOLUTION;


typedef struct stISP_CMOS_SENSOR_IMAGE_MODE_S
{
  ISP_U16 u16Width;
  ISP_U16 u16Height;
  ISP_U16 u16Fps;
  
}ISP_CMOS_SENSOR_IMAGE_MODE;


typedef struct stISP_SENSOR_EXP_FUNC_S
{
	ISP_VOID(*pfn_cmos_sensor_init)(ISP_DEV IspDev);
	ISP_VOID(*pfn_cmos_sensor_global_init)(ISP_DEV IspDev);

	ISP_S32(*pfn_cmos_get_isp_default)(ISP_DEV IspDev, ISP_CMOS_DEFAULT_S *pstDef);
	ISP_S32(*pfn_cmos_get_sensor_max_resolution)(ISP_DEV IspDev, ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution);

	/* the function of sensor set pixel detect */
	ISP_VOID(*pfn_cmos_set_pixel_detect)(ISP_DEV IspDev, ISP_BOOL bEnable);
	ISP_VOID(*pfn_cmos_set_wdr_mode)(ISP_DEV IspDev, ISP_U8 u8Mode);
	ISP_VOID(*pfn_cmos_set_resolution)(ISP_DEV IspDev, ISP_U32 u32ResolutionMode);
	ISP_S32(*pfn_cmos_set_image_mode)(ISP_DEV IspDev, ISP_CMOS_SENSOR_IMAGE_MODE *pstSensorImageMode);
	ISP_S32(*pfn_cmos_set_mirror_flip)(ISP_DEV IspDev, ISP_U8 u8Mirror,ISP_U8 u8Flip);
} ISP_SENSOR_EXP_FUNC_S;

typedef struct stISP_SENSOR_REGISTER_S
{
    ISP_SENSOR_EXP_FUNC_S stSnsExp;
} ISP_SENSOR_REGISTER_S;


typedef struct stISP_CMOS_SNS_ATTR_S
{
	ISP_U8 u8InputMode;	// 0:DVP 1:MIPI
	ISP_U8 u8MipiAttr;	// bit0~bit3: lane   bit4~bit7:depth
	ISP_U8 u8HdrMode;
	ISP_FUN_EXT u8FunExt;		// function(ext)(0:General  1:face capture  0x40:plate capture	0x50:LiveCamera)  0x60:(IPC+AHD)
	ISP_U8 u8Rsv[4];
	RESL_TYPE enRsltType;
	ISP_U32 u32Rsv[13];
} ISP_CMOS_SNS_ATTR_S;

typedef struct stISP_CMOS_SNS_ATTR
{
	ISP_S32 s32SnsChipId;
	I2C_REG stSnsI2C;
	SENSOR_CONT enCmosIF;
	RESL_TYPE enRsltType;
	WDR_MODE_E enHdrMode;
	ISP_FUN_EXT u8FunExt;		// function(ext)(0:General  1:face capture  0x40:plate capture	0x50:LiveCamera)
	ISP_U8 u8MipiAttr;	// bit0~bit3: lane   bit4~bit7:depth
	ISP_U8 au8Rsv[3];
	ISP_VOID(*pfn_cmos_sensor_init)(ISP_DEV IspDev);
	ISP_S32(*pfn_cmos_init_sensor_exp_function)(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc);
	ISP_S32(*pfn_cmos_init_ae_exp_function)(AE_SENSOR_EXP_FUNC_S *pstExpFuncs);
	ISP_S32(*pfn_cmos_init_awb_exp_function)(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs);
} ISP_CMOS_SNS_ATTR;

typedef struct stISP_CMOS_DN_PARA_S
{
	// AGC(CMOS)
	const ISP_CMOS_AGC_TABLE_S *pastAgcTab[2];			// 0:Color  1:Bw
	const ISP_CMOS_VPSS_3DNR_S *pastAgcVpss3DNr[2];		// 0:Color  1:Bw
	const ISP_CMOS_YCNR_S *pastAgcYcNr[2];				// 0:Color  1:Bw
	const ISP_CMOS_SHARPEN_S *pastAgcSharp[2];			// 0:Color  1:Bw
	const ISP_CMOS_LOCAL_CAC_S *pastAgcCAC[2];			// 0:Color  1:Bw
	const ISP_CMOS_LTM_S *pastAgcLtm[2];				// 0:Color  1:Bw
	const ISP_CMOS_2DNR_S *past2DNr[2];					// 0:Color  1:Bw
	// Gamma
	const ISP_CMOS_GAMMA_S *pastGamma[2];
	ISP_U32 au32Rsv[4];
} ISP_CMOS_DN_PARA_S;

#endif /*__ISP_COMM_SNS_H__ */

