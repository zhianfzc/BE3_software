#ifndef __ISP_COMM_VDA_H__
#define __ISP_COMM_VDA_H__

#include "isp_type.h"
#include "isp_comm_isp.h"
#include "isp_comm_3a.h"



#define VDAMOWIN 18

typedef struct stVDA_OD_CHN_ATTR
{
	ISP_BOOL bEnable;
	ISP_U16 u16AlarmThrUp;	// 报警阈值1([0,255])
	ISP_U16 u16AlarmThrDn;	// 报警阈值2([0,255])
	ISP_U16 au16Rsv[3];
	ISP_U8 u8AlarmRatioThr;	// 报警阈值3(遮挡比例    	[0,128])
	ISP_U8 u8CycleFrame;		// 遮挡间隔帧
}VDA_OD_CHN_ATTR;

typedef struct stVDA_OD_RESULT
{
	ISP_U8 u8AlarmFlag;
}VDA_OD_RESULT;

typedef struct stVDA_MD_CHN_ATTR
{
	ISP_BOOL bEnble;
	ISP_U8 u8GridLines;
	ISP_U8 u8GridRows;
	ISP_U8 u8CycleFrame;
	ISP_U8 u8AlarmRatioThr;	// 报警阈值(移动比例    	[0,100])
	ISP_U8 u8Sensitivity;

	ISP_U32 au32WinUsedMark[18];
}VDA_MD_CHN_ATTR;

typedef struct  stVDA_STAT_DATA
{
	ISP_U32 au32WinStat[AE_ZONE_ROW][AE_ZONE_COLUMN]; //存取值
}VDA_STAT_DATA;

typedef struct	stVDA_MD_RESULT
{
	ISP_U32 au32WinRlstMark[AE_ZONE_ROW];	// 移动标志位，每行占四个byte
	ISP_U8 u8AlarmFlag;
	ISP_U8 u8AlarmRatioRlst;					// 触发报警百分比([0,100])	
}VDA_MD_RESULT;


typedef struct	stVDA_MD_S
{
	VDA_MD_CHN_ATTR stChnAttr;
	VDA_STAT_DATA stStatData;
	VDA_MD_RESULT stResult;
}VDA_MD_S;

typedef struct  stVDA_OD_S
{
	VDA_OD_CHN_ATTR stChnAttr;
	VDA_OD_RESULT stResult;
}VDA_OD_S;

typedef struct	stVDA_CHN_ATTR
{
	ISP_U8 u8ClearFlg;	//bit0:MD  bit1:OD     (1:do clear)
	ISP_U8 au8Rsv[3];
	ISP_BOOL bCreate;
	VDA_MD_S stVdaMD;
	VDA_OD_S stVdaOD;
	ISP_U32 au32Rsv[8];
}VDA_CHN_ATTR;


typedef enum
{
	VDA_SHL_RAT1=96,		// 3/4    *128
	VDA_SHL_RAT2=64,		// 1/2
	VDA_SHL_RAT3=21,		// 1/6
	VDA_SHL_RAT4=16,		// 1/8
	VDA_SHL_RAT5=13,		// 1/10
	VDA_SHL_RAT6=8			// 1/16
}VDA_MOTION_SHELTER_RATIO;

typedef enum
{
	VDA_MOV_RAT1=2,
	VDA_MOV_RAT2=4,
	VDA_MOV_RAT3=6,
	VDA_MOV_RAT4=8,
	VDA_MOV_RAT5=10,
	VDA_MOV_RAT6=12
}VDA_MOTION_MOVE_RATIO;


#endif 

