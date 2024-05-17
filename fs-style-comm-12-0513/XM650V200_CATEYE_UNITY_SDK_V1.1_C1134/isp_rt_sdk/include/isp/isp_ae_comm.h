#ifndef __ISP_AE_COMM_H__
#define __ISP_AE_COMM_H__

#include "isp_type.h"


#define AE_LIB_NAME 	"isp_ae_lib"
#define AWB_LIB_NAME 	"isp_awb_lib"

/************************** ae ctrl cmd **************************************/
typedef enum stAE_CTRL_CMD_E
{
    AE_DEBUG_ATTR_SET,
    AE_DEBUG_ATTR_GET, 

    AE_CTRL_BUTT,
} AE_CTRL_CMD_E;

typedef struct stAE_DBG_ATTR_S
{
    ISP_BOOL bAeBypass;
    ISP_BOOL bFrameEndUpdateMode;
    ISP_U32  u32MaxAgain;
    ISP_U32  u32MinAgin;
    ISP_U32  u32MaxDgain;
    ISP_U32  u32MinDgain;
    ISP_U32  u32MaxIspDgain;
    ISP_U32  u32MinIspDgain;
    ISP_U32  u32MaxIntTime;
    ISP_U32  u32MinIntTime;
    ISP_U32  u32Compensation;
#if 0
    ISP_U32  BlackLevel_R;
    ISP_U32  BlackLevel_Gr;
    ISP_U32  BlackLevel_Gb;
    ISP_U32  BlackLevel_B;
#endif
    ISP_U32  u32Hist01;
    ISP_U32  u32Hist12;
    ISP_U32  u32Hist34;
    ISP_U32  u32Hist45;
    ISP_BOOL bManualExposureEn;
    ISP_BOOL bManualAgainEn;
    ISP_BOOL bManualDgainEn;
    ISP_BOOL bManualIspDgainEn;
    ISP_U32  u32ManualExposureLines;
    ISP_U32  u32ManualAgain;
    ISP_U32  u32ManualDgain;
    ISP_U32  u32ManualIspDgain;
    ISP_U32  au32AeWeights[255];    
}AE_DBG_ATTR_S;

typedef struct stAE_DBG_STATUS_S
{
    ISP_U32  u32FrmNumBgn;
    ISP_U32  u32FullLines;
    ISP_U32  u32IntTime;
    ISP_U32  u32Again;
    ISP_U32  u32AgainShift;
    ISP_U32  u32Dgain;
    ISP_U32  u32DgainShift;
    ISP_U32  u32IspDgain;
    ISP_U32  u32IspDgainShift;
    ISP_U32  u32Exposure;
    ISP_U32  u32Increment;
    ISP_U32  u32HistBalance;
    ISP_S32  u32HistError;
    ISP_U32  u32ExpoureStep;
    ISP_U32  u32FrmNumEnd;   
}AE_DBG_STATUS_S;

/************************** sensor's interface to ae *********************/

/* eg: 0.35db, enAccuType=AE_ACCURACY_DB, f32Accuracy=0.35 
*  and the multiply of 0.35db is power(10, (0.35/20))
*  eg: 1/16, 2/16, 3/16 multiplies, enAccuType=AE_ACCURACY_LINEAR, f32Accuracy=0.0625
*  eg: 1,2,4,8,16 multiplies, enAccuType=AE_ACCURACY_DB, f32Accuracy=6
*/
typedef enum stAE_ACCURACY_E
{
    AE_ACCURACY_DB = 0,
    AE_ACCURACY_LINEAR,
    AE_ACCURACY_TABLE,
    
    AE_ACCURACY_BUTT,
} AE_ACCURACY_E;

typedef struct stAE_ACCURACY_S
{
    AE_ACCURACY_E enAccuType;
    float   f32Accuracy;
} AE_ACCURACY_S;

typedef struct stAE_SENSOR_DEFAULT_S
{
    ISP_U8   u8AeCompensation;
	ISP_U8 	u8UpdateMode;	// 0: Old	1:New(sns_regs)
    ISP_U32  u32LinesPer500ms;
    ISP_U32  u32FlickerFreq;

    ISP_U32  u32FullLinesStd;
    ISP_U32  u32MaxIntTime;     /* unit is line */
    ISP_U32  u32MinIntTime;
    ISP_U32  u32MaxIntTimeTarget;
    ISP_U32  u32MinIntTimeTarget;
    
    ISP_U32  u32MaxAgain;
    ISP_U32  u32MinAgain;
    ISP_U32  u32MaxAgainTarget;
    ISP_U32  u32MinAgainTarget;
    AE_ACCURACY_S stAgainAccu;

    ISP_U32  u32MaxDgain;
    ISP_U32  u32MinDgain;
    ISP_U32  u32MaxDgainTarget;
    ISP_U32  u32MinDgainTarget;
    AE_ACCURACY_S stDgainAccu;

	ISP_U32  u32MaxISPDgain;
    ISP_U32  u32MinISPDgain;
    ISP_U32  u32MaxISPDgainTarget;
    ISP_U32  u32MinISPDgainTarget;
    ISP_U32  u32ISPDgainShift;

/****** 新增加部分*****/
	ISP_U32  u32LinePixel;		//  1行像素个数
	ISP_U32  u32TimeFinePoint;	//  < u32TimeFinePoint考虑细调
/********************************
u32UpdateCfg:
	bit[0~7]  period
	bit[8~15] shut 
	bit[16~23] snsGain
	bit[24~31] ispGain
********************************/
	ISP_U32  u32UpdateCfg;
	ISP_U32  u32MaxAllgain;
	ISP_U32  au32InitExp[4];
/********************************
u16AlgMode:
0x00: alg_V1
0x10: alg_V2
********************************/
	ISP_U16 u16AlgMode;
	ISP_BOOL	 bRsv;
} AE_SENSOR_DEFAULT_S;

typedef struct stAE_SENSOR_DEFAULT_V2_S
{
	ISP_U32  u32MinIntTime;
	ISP_U32  u32MaxIntTime;     /* unit is line */
	ISP_U32  u32MinAgain;
	ISP_U32  u32MaxAgain;
	ISP_U32  u32MinDgain;
	ISP_U32  u32MaxDgain;
	ISP_U32  u32MaxISPDgain;
/********************************
u32UpdateCfg:
	bit[0~7]  period
	bit[8~15] shut 
	bit[16~23] snsGain
	bit[24~31] ispGain
********************************/
	ISP_U32	u32UpdateCfg;
	ISP_U32  au32InitExp[4];
/********************************
u16AlgMode:
0x00: fix V1 
	0x01: first V1, after stab bypass AE
0x10: fix V2
	0x11: first V2, after stab bypass AE
0x20: first V2, after stab V1 
********************************/
	ISP_U16 u16AlgMode;
} AE_SENSOR_DEFAULT_V2_S;


typedef struct stAE_SENSOR_GAININFO_S
{
    ISP_U32  u32SnsTimes;
    ISP_U32  u32GainDb;
    
} AE_SENSOR_GAININFO_S;

typedef struct stAE_SENSOR_SHUTINFO_S
{
    ISP_U64  u64Exp;
    ISP_U32  u32Ofst;
    ISP_U32	u32ShutDb;
} AE_SENSOR_SHUTINFO_S;

typedef enum stISP_SNS_TYPE_E
{
    ISP_SNS_I2C_TYPE = 0,
    ISP_SNS_SSP_TYPE,
    
    ISP_SNS_TYPE_BUTT,
} ISP_SNS_TYPE_E;

typedef struct stISP_I2C_DATA_S
{
	ISP_BOOL bUpdate;
	ISP_U8   u8DelayFrmNum;
	ISP_U8   u8IntPos;
	ISP_U32  u32RegAddr;
	ISP_U32  u32Data;
} ISP_I2C_DATA_S;

typedef struct stISP_SSP_DATA_S
{
	ISP_BOOL bUpdate;
	ISP_U8   u8DelayFrmNum;
	ISP_U8   u8IntPos;
	ISP_U32  u32DevAddr;
	ISP_U32  u32DevAddrByteNum;
	ISP_U32  u32RegAddr;
	ISP_U32  u32RegAddrByteNum;
	ISP_U32  u32Data;
	ISP_U32  u32DataByteNum;
} ISP_SSP_DATA_S;


typedef struct stISP_SNS_REGS_INFO_S
{
    ISP_SNS_TYPE_E enSnsType;
    ISP_U32  u32RegNum;
    ISP_U8   u8Cfg2ValidDelayMax;

    union
    {
        ISP_I2C_DATA_S *pstI2CData;
        ISP_SSP_DATA_S *pstSspData;
    };
} ISP_SNS_REGS_INFO_S;

typedef struct stAE_SENSOR_EXP_FUNC_S
{
    ISP_S32(*pfn_cmos_get_ae_default)(ISP_DEV IspDev, AE_SENSOR_DEFAULT_S *pstAeSnsDft);

    /* the function of sensor set fps */
    ISP_VOID(*pfn_cmos_fps_set)(ISP_DEV IspDev, ISP_U8 u8Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft);
    ISP_S32(*pfn_cmos_fps_get)(ISP_DEV IspDev, ISP_U8 *pu8Fps);
    ISP_VOID(*pfn_cmos_slow_framerate_set)(ISP_DEV IspDev, ISP_U16 u16SlowFrameRate, AE_SENSOR_DEFAULT_S *pstAeSnsDft);

    /* while isp notify ae to update sensor regs, ae call these funcs. */
    ISP_VOID(*pfn_cmos_inttime_update)(ISP_DEV IspDev, ISP_U32 u32IntTime);
    ISP_VOID(*pfn_cmos_inttime_short_update)(ISP_DEV IspDev, ISP_U32 u32IntTime);
    ISP_VOID(*pfn_cmos_gains_update)(ISP_DEV IspDev, ISP_U32 u32Again, ISP_U32 u32Dgain);
    ISP_VOID(*pfn_cmos_shut_calc_table)(ISP_DEV IspDev, ISP_S32 *ps32IntTime,AE_SENSOR_SHUTINFO_S *pstAeSnsShutInfo);
    ISP_VOID(*pfn_cmos_gains_update2)(ISP_DEV IspDev, ISP_S32 s32DGain,const AE_SENSOR_DEFAULT_S *pstAeSnsDft);

    ISP_VOID (*pfn_cmos_again_calc_table)(ISP_DEV IspDev, ISP_U32 u32InTimes, AE_SENSOR_GAININFO_S *pstAeSnsGainInfo);
    ISP_VOID (*pfn_cmos_dgain_calc_table)(ISP_DEV IspDev, ISP_U32 u32InTimes, AE_SENSOR_GAININFO_S *pstAeSnsGainInfo);

    ISP_S32(*pfn_cmos_get_sns_reg_info)(ISP_DEV IspDev, ISP_SNS_REGS_INFO_S *pstSnsRegsInfo);
    ISP_S32(*pfn_cmos_write_register)(ISP_DEV IspDev, ISP_U32 addr, ISP_U32 data);
    ISP_S32(*pfn_cmos_read_register)(ISP_DEV IspDev, ISP_U32 addr);	
} AE_SENSOR_EXP_FUNC_S;

typedef struct stAE_SENSOR_REGISTER_S
{
    AE_SENSOR_EXP_FUNC_S stSnsExp;
} AE_SENSOR_REGISTER_S;


typedef struct stISP_AE_VARLED_REGISTER_S
{
	ISP_BOOL	bEnable;
	ISP_OP_TYPE_E   enOpType;
	ISP_U32 u32ManualLvl;
	ISP_U32 u32LvlNow;		//RO
	ISP_U32 u32AutoSpeed;	//0~255(default:128)
	ISP_U32 u32MaxLvl;
	ISP_U32 u32MinLvl;
	ISP_U32 u32ExpThreshold_1;	// <:   weaken
	ISP_U32 u32ExpThreshold_2;	// >:	enhance
	ISP_S32(*pfn_varLED_init)(ISP_DEV IspDev);
	ISP_VOID(*pfn_varLED_update)(ISP_DEV IspDev, ISP_U32 u32Lvl);
	ISP_VOID(*pfn_varLED_calc_table)(ISP_DEV IspDev, ISP_U32 u32InTimes, AE_SENSOR_GAININFO_S *pstAeSnsGainInfo);
}ISP_AE_VARLED_REGISTER_S;

#endif
