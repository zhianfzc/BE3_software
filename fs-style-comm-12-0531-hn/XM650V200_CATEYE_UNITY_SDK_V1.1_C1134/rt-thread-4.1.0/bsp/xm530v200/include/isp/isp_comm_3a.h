#ifndef __ISP_COMM_3A_H__
#define __ISP_COMM_3A_H__

#include "isp_common.h"
#include "isp_comm_isp.h"



#define MAX_REGISTER_ALG_LIB_NUM 2

typedef enum stISP_ALG_MOD_E
{
    ISP_ALG_AE = 0,
    ISP_ALG_AF,
    ISP_ALG_AWB,
    ISP_ALG_ANTIFOG,
    ISP_ALG_BLC,
    ISP_ALG_DP,
    ISP_ALG_DRC,
    ISP_ALG_DEMOSAIC,
    ISP_ALG_GAMMA,
    ISP_ALG_GAMMAFE,
    ISP_ALG_GE,
    ISP_ALG_NEW_ANTIFOG,
    ISP_ALG_NR,
    ISP_ALG_SHARPEN,
    ISP_ALG_SHADING,
    
    ISP_ALG_BUTT,
} ISP_ALG_MOD_E;

typedef enum stISP_CTRL_CMD_E
{
    ISP_WDR_MODE_SET = 8000,
    ISP_PROC_WRITE,
    
    ISP_AE_FPS_BASE_SET,
    ISP_AWB_ISO_SET,  /* set iso, change saturation when iso change */
    ISP_AE_INTTIME_GET,
    ISP_AWB_INTTIME_SET,

    ISP_CHANGE_IMAGE_MODE_SET,
    ISP_CTRL_CMD_BUTT,
} ISP_CTRL_CMD_E;

typedef struct stISP_CTRL_PROC_WRITE_S
{
    ISP_CHAR *pcProcBuff;
    ISP_U32   u32BuffLen;
    ISP_U32   u32WriteLen;   /* The len count should contain '\0'. */
} ISP_CTRL_PROC_WRITE_S;

/********************************  AE  *************************************/
/* the init param of ae alg */
typedef struct stISP_AE_PARAM_S
{
    SENSOR_ID SensorId;
    
    ISP_U32 u32MaxIspDgain;
    ISP_U32 u32MinIspDgain;
    ISP_U32 u32IspDgainShift;
    ISP_U8   u8FrameRate;
} ISP_AE_PARAM_S;

typedef struct stISP_2A_STAT_S
{
	ISP_U16 au16StatBufR[512];	// 32x16
	ISP_U16 au16StatBufG[512];	// 32x16
	ISP_U16 au16StatBufB[512];	// 32x16
}ISP_2A_STAT_S;

/* the statistics of ae alg */
typedef struct stISP_AE_STAT_1_S
{
    ISP_U32 au32MeteringHist[16];
} ISP_AE_STAT_1_S;

typedef struct stISP_AE_STAT_2_S
{
	ISP_U32 au32MeteringWin[AE_ZONE_ROW][AE_ZONE_COLUMN];
} ISP_AE_STAT_2_S;

typedef struct stISP_AE_STAT_3_S
{    
    ISP_U16  au16MeteringWinV2[ISP_V2_ZONE_ROW][ISP_V2_ZONE_COLUMN];
} ISP_AE_STAT_3_S;


typedef struct stISP_AE_STAT_S
{
	ISP_U8 u8FrameRate;
	ISP_U32 u32FrameCnt;
	ISP_U32 au32MeteringWin[AE_ZONE_ROW][AE_ZONE_COLUMN];
} ISP_AE_STAT_S;




typedef struct stISP_AE_INFO_S
{
    ISP_U32  u32FrameCnt;    /* the counting of frame */
    ISP_U8  u8FrameRate;	
    ISP_AE_STAT_1_S stAeStat1;
    ISP_AE_STAT_2_S stAeStat2;
    ISP_AE_STAT_3_S stAeStat3;
} ISP_AE_INFO_S;



/* the final calculate of ae alg */
typedef struct stISP_AE_RESULT_S
{
    ISP_U32  u32IspDgain;
    ISP_U32  u32IspDgainShift;
    ISP_U32  u32Iso;
    ISP_U32  u32LumError;	// *64
/********************************
u32UpdateCfg:
	bit[0~7]  period
	bit[8~15] shut 
	bit[16~23] snsGain
	bit[24~31] ispGain
********************************/
    ISP_U32  u32UpdateCfg;
	ISP_U32 	u32ExpRatio;
	ISP_U32	au32Rsv[4];
} ISP_AE_RESULT_S;

typedef struct stISP_AE_EXP_FUNC_S
{
    ISP_S32 (*pfn_ae_init)(ISP_DEV IspDev, ISP_S32 s32Handle, const ISP_AE_PARAM_S *pstAeParam);
    ISP_S32 (*pfn_ae_run)(ISP_DEV IspDev, ISP_S32 s32Handle,
        const ISP_AE_INFO_S *pstAeInfo,
        ISP_AE_RESULT_S *pstAeResult,
        ISP_S32 s32Rsv
        );
    ISP_S32 (*pfn_ae_ctrl)(ISP_DEV IspDev, ISP_S32 s32Handle, ISP_U32 u32Cmd, ISP_VOID *pValue);
    ISP_S32 (*pfn_ae_exit)(ISP_DEV IspDev, ISP_S32 s32Handle);
} ISP_AE_EXP_FUNC_S;

typedef struct stISP_AE_REGISTER_S
{
    ISP_AE_EXP_FUNC_S stAeExpFunc;
} ISP_AE_REGISTER_S;


/********************************  AWB  *************************************/
/* the statistics of awb alg */
typedef struct stISP_AWB_STAT_ZONE_S
{
	ISP_U32  u32Data1;	// Wp
	ISP_U32  u32Data2;	// Cb
	ISP_U32  u32Data3;	// Cr
} ISP_AWB_STAT_ZONE_S;



typedef struct stISP_AWB_STAT_1_S
{
	ISP_AWB_STAT_ZONE_S stStat1[4][4];	// 4:Chn  // 4:Zone
} ISP_AWB_STAT_1_S;

typedef struct stISP_AWB_STAT_2_S
{
    ;
} ISP_AWB_STAT_2_S;

typedef struct stISP_AWB_INFO_S
{
	ISP_U32 u32FrameCnt;
	ISP_U32 u32Rsv;
	ISP_AWB_STAT_1_S *pstAwbStat1;
	ISP_AWB_STAT_2_S *pstAwbStat2;
} ISP_AWB_INFO_S;

/* the statistics's attr of awb alg */
typedef struct stISP_AWB_FT_L_ATTR_S
{
	ISP_BOOL bChange;

	ISP_U32  u32Lh;
	ISP_U32  u32Lv;
	ISP_U32  u32L45;
	ISP_U32  u32L135;
} ISP_AWB_FT_L_ATTR_S;

typedef struct stISP_AWB_FT_K_ATTR_S
{
	ISP_BOOL bChange;

	ISP_U32  u32KCbCr1;
	ISP_U32  u32KCbCr2;
	ISP_U32  u32KCbCr3;
	ISP_U32  u32KCbCr4;
} ISP_AWB_FT_K_ATTR_S;

typedef struct stISP_AWB_WDW_ATTR_S
{
	ISP_BOOL bChange;
	ISP_U8 u8Mode;
	ISP_U32  u32VBegin;
	ISP_U32  u32VEnd;
	ISP_U32  u32HBegin;
	ISP_U32  u32HEnd;
} ISP_AWB_WDW_ATTR_S;

typedef struct stISP_AWB_BG_ATTR_S
{
	ISP_BOOL bChange;
	ISP_U32  u32R_Ch[4];
	ISP_U32  u32B_Ch[4];
} ISP_AWB_BG_ATTR_S;

typedef struct stISP_AWB_RESULT_BK_S
{
	ISP_AWB_FT_L_ATTR_S stFtL;
	ISP_AWB_FT_K_ATTR_S stFtK;
	ISP_AWB_WDW_ATTR_S stWin;
	ISP_AWB_BG_ATTR_S stBg;
} ISP_AWB_RESULT_BK_S;

typedef struct stISP_AWB_RESULT_GAIN_S
{
	ISP_BOOL bChange;
	ISP_U32  au32WhiteBalanceGain[3];
} ISP_AWB_RESULT_GAIN_S;

typedef struct stISP_AWB_RESULT_CC_S
{
	ISP_BOOL bChange;
	ISP_U16  au16ColorMatrix[12];
} ISP_AWB_RESULT_CC_S;

/* the final calculate of awb alg */
typedef struct stISP_AWB_RESULT_S
{
	ISP_AWB_RESULT_GAIN_S *pstRlt_Gain;
	ISP_AWB_RESULT_CC_S *pstRlt_Cc;
	ISP_AWB_RESULT_BK_S *pstRlt_Bk;
	ISP_U16 *pstRlt_RGB;
} ISP_AWB_RESULT_S;

typedef struct stISP_AWB_EXP_FUNC_S
{
    ISP_S32 (*pfn_awb_init)(ISP_DEV IspDev, ISP_S32 s32Handle, ISP_AWB_RESULT_BK_S* const pstAwbParam,ISP_U32 Width,ISP_U32 Height);
    ISP_S32 (*pfn_awb_run)(ISP_DEV IspDev, ISP_S32 s32Handle,
        const ISP_AWB_INFO_S *pstAwbInfo,
        ISP_AWB_RESULT_S *pstAwbResult,
        ISP_S32 s32Rsv
        );
    ISP_S32 (*pfn_awb_ctrl)(ISP_DEV IspDev, ISP_S32 s32Handle, ISP_U32 u32Cmd, ISP_VOID *pValue);
    ISP_S32 (*pfn_awb_exit)(ISP_DEV IspDev, ISP_S32 s32Handle);
} ISP_AWB_EXP_FUNC_S;


typedef struct stISP_AWB_CALBACK_FUNC_S
{
    ISP_S32 (*pfn_awb_phystat_get)(ISP_DEV IspDev, ISP_2A_STAT_S *pstStat, ISP_U32 u32BlockSize);
} ISP_AWB_CALBACK_FUNC_S;

typedef struct stISP_AWB_REGISTER_S
{    
    ISP_AWB_EXP_FUNC_S stAwbExpFunc;
    ISP_AWB_CALBACK_FUNC_S stAwbLibCalBack;
} ISP_AWB_REGISTER_S;

/********************************  AF  *************************************/

/* the init param of af alg */
typedef struct stISP_AF_PARAM_S
{
    SENSOR_ID SensorId;
    ISP_S32 s32Rsv;
} ISP_AF_PARAM_S;

/* the statistics of af alg */
typedef struct stISP_AF_STAT_S
{
    ISP_U16 u16FocusMetrics;     /* The integrated and normalized measure of contrast*/
    ISP_U16 u16ThresholdRead;    /* The ISP recommend value of AF threshold*/
    ISP_U16 u16ThresholdWrite;   /* The user defined value of AF threshold (or 0 to use threshold from previous frame)*/
    ISP_U16 u16FocusIntensity;   /* The average brightness*/
    ISP_U8  u8MetricsShift;      /* Metrics scaling factor:the bigger value for this register means all zones metrics go higher,0x03 is the default, Range: [0x0, 0xF] */
    ISP_U8  u8NpOffset;          /* The AF noise profile offset, Range: [0x0, 0xFF] */
    ISP_U16 au16ZoneMetrics[AE_ZONE_ROW][AE_ZONE_COLUMN]; /* The zoned measure of contrast*/ 
} ISP_AF_STAT_S;

typedef struct stISP_AF_INFO_S
{
    ISP_U32  u32FrameCnt;
    
    ISP_AF_STAT_S   *pstStatistics;
} ISP_AF_INFO_S;

typedef struct stISP_AF_STAT_ATTR_S
{
    ISP_BOOL bChange;

    ISP_U16 u16ThresholdWrite;
    ISP_U8  u8MetricsShift;
    ISP_U8  u8NpOffset;
} ISP_AF_STAT_ATTR_S;

/* the final calculate of af alg */
typedef struct stISP_AF_RESULT_S
{
    ISP_AF_STAT_ATTR_S stStatAttr;
    ISP_S32 s32Rsv;
} ISP_AF_RESULT_S;

typedef struct stISP_AF_EXP_FUNC_S
{
    ISP_S32 (*pfn_af_init)(ISP_S32 s32Handle, const ISP_AF_PARAM_S *pstAfParam);
    ISP_S32 (*pfn_af_run)(ISP_S32 s32Handle,
        const ISP_AF_INFO_S *pstAfInfo,
        ISP_AF_RESULT_S *pstAfResult,
        ISP_S32 s32Rsv
        );
    ISP_S32 (*pfn_af_ctrl)(ISP_S32 s32Handle, ISP_U32 u32Cmd, ISP_VOID *pValue);
    ISP_S32 (*pfn_af_exit)(ISP_S32 s32Handle);
} ISP_AF_EXP_FUNC_S;

typedef struct stISP_AF_REGISTER_S
{    
    ISP_AF_EXP_FUNC_S stAfExpFunc;
} ISP_AF_REGISTER_S;

typedef struct stALG_LIB_S
{
    ISP_S32  s32Id;
    ISP_CHAR acLibName[20];
} ALG_LIB_S;

typedef struct stISP_BIND_ATTR_S
{
    SENSOR_ID   SensorId;
    ALG_LIB_S   stAeLib;
    ALG_LIB_S   stAfLib;
    ALG_LIB_S   stAwbLib;
} ISP_BIND_ATTR_S;


#endif /*__ISP_COMM_SNS_H__ */

