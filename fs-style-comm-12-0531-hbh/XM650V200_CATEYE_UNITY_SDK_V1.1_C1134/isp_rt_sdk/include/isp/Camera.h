//
//  "$Id: Camera.h 4 2011-09-14 14:13:30Z xiaoyongli $"
//
//  Copyright (c)2008-2008, ZheJiang JuFeng Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//
#ifndef __CAMERA_H__
#define __CAMERA_H__ 

#ifdef __cplusplus
extern "C" {
#endif
#include "isp_type.h"


#define GET_REJECT_SHAKE_TIME(cfgTime)  ((cfgTime)*20)
#define BYTE		unsigned char
#define WORD	unsigned short




enum dvr_info_cmd_hs
{
	PRODUCT_TYPE 		= 0,
	VIDEO_CHANNEL 		= 1,
	AUDIO_CHANNEL 		= 2,
	ALARM_IN 			= 3,
	ALARM_OUT			= 4,
	FVIDEO_CHIP			= 5,
	DSP_CHIP 			= 6,
	ANALOG_AUDIO_MODE	= 7,
	TALKBACK			= 8,
	BVIDEO_CHIP			= 9,
	STORE_INTERFACE 	= 10,
	MATRIX		 		= 11,
	WIRELESS_INTERFACE	= 12,
	HD_ENCODE			= 13,
	HD_VERSION 			= 14,
	VD_INTERFACE        = 15,
	NET_INTERFACE       = 16,
	HD_INFO_LEN			= 17
};


enum netmode{
	NET_LAN = 0,
	NET_WLAN_8188EU
};

typedef enum _resolution_bit_{
	RSLTION_BIT_720P = 0,
	RSLTION_BIT_960P,
	RSLTION_BIT_1080P,
	RSLTION_BIT_1536P,
	RSLTION_BIT_4MP,
	RSLTION_BIT_5MP,
	RSLTION_BIT_8MP,

	RSLTION_BIT_BUTT
}RSLTION_BIT;




typedef enum {
	BLIGHT_CLOSE = 0x10,
	BLIGHT_OPEN = 0x11,
	CLIGHT_CLOSE = 0x20,
	MUSICLIGHT_OPEN = 0x21,
	MOODLIGHT_OPEN = 0x22,
}enLIGHT_CTRL;

typedef struct stCOORD_S
{
    int s32X;
    int s32Y;
}COORD_S;

typedef enum IRCUT_SWITCH_MODE
{
	IRCUT_SYN_INFRARED,
	IRCUT_SWITCH_AUTO,
	IRCUT_BUTT
}IRCUT_SWITCH_MODE;
typedef enum DNC_MODE
{
	DNC_AUTO,			// 自动切换
	DNC_MULTICOLOR,		// 彩色
	DNC_BLACKWHITE,		// 强制为黑白模式
	DNC_INTE_WHITE_INF, //智能警戒
	DNC_WHITELAMP_AUTO, //智能暖光
	DNC_IRLAMP_AUTO,	//智能红外
	DNC_LP_MODE,		//车牌模式
	DNC_BUTT
}DNC_MODE;

typedef enum IRCUT_MODE
{
	IRCUT_NIGHT,///晚上
	IRCUT_DAY,///白天滤光片
	IRCUT_AUTO,
}IRCUT_MODE;

typedef enum CAMERA_SCENE
{
	SCENE_AUTO,
	SCENE_INDOOR,
	SCENE_OUTDOOR,
	SCENE_BUTT,
}CAMERA_SCENE;
typedef enum IRCUT_SWITCH_DIRECTION
{
	NORMAL_DIRECTION,
//	CONTRARY_DIRECTION
}IRCUT_SWITCH_DIRECTION;

typedef struct stMOVEMENT_DATA_S
{
	 int 	reg_addr; 
	 int 	data; 
	 int   mode;
}MOVEMENT_DATA_S ;

/// 视频颜色格式
typedef struct CAMERA_COLOR{
	unsigned char	Brightness;		///< 亮度，取值0-100。
	unsigned char	Contrast;		///< 对比度，取值0-100。
	unsigned char 	Saturation;		///< 饱和度，取值0-100。
	unsigned char 	Hue;			///< 色调，取值0-100。
	unsigned char 	Gain;			///< 增益，取值0-100。bit7置位表示自动增益，其他位被忽略。
	unsigned char	WhiteBalance;	///< 自动白电平控制，bit7置位表示开启自动控制.0x0,0x1,0x2分别代表低,中,高等级
	unsigned short	Acutance;       	///< 锐度，取值0-15, 第8位表示水平锐度，高8为表示垂直锐度。
}CAMERA_COLOR;	// 和VIDEO_COLOR一致

// 编码参数
typedef struct tagCAPTURE_FORMAT
{
    BYTE    Compression;        /*!< 压缩模式 */
    BYTE    BitRateControl;     /*!< 码流控制 */
    BYTE    ImageSize;          /*!< 图像分辨率 */
    BYTE    ImageQuality;       /*!< 图像画质 */
    BYTE    FramesPerSecond;    /*!< 帧率 */
    BYTE    AVOption;           /*!< 音视频选项 */
    WORD    BitRate;            ///< 参考码流值，Kbps为单位
    BYTE    GOP;                /*< 帧间隔的数值，建议值49～99*/
    BYTE    reserved[3];        /*< 保留字节*/
} CAPTURE_FORMAT;

//返回VI窗口尺寸的结构体
typedef struct st_VI_WIN_S
{
    unsigned int u32Width;
    unsigned int u32Height;
    unsigned int u32TotalWidth;
    unsigned int u32TotalHeight;
}VI_WIN_S;


typedef struct st_CAMERA_INIT_ATTR_S
{
	unsigned char u8ProductFun;	//产品功能(额外)  --- 例:0x10时 为小方块(低功耗、频率减半)
	unsigned char u8ProductLow;	//降频功能(0：标准频率    		1：频率减半（帧率减半）
	unsigned char u8SnsConnect;	//特殊sensor 或者 指定DVP/MIPI
	unsigned char u8MipiLane;
	unsigned char u8MipiBit;
	unsigned char au8Rsv[11];
}CAMERA_INIT_ATTR_S;


typedef struct st_AIRSLT_INFO_S
{
    unsigned char u8AlarmFlg;	//1: 触发报警   	0:未触发
    unsigned int u32TargetNum;	// 目标个数;
    unsigned int u32Info[64];		// 报警信息(坐标等)
}AIRSLT_INFO_S;

typedef struct stISP_GET_AI_RESULT_S
{	
	/***
	u32Cmd: 
	 0: 人形检测
	 	0x01: 坐标(全身)
	 	0x02: 坐标(半身)
	 	0x03: 坐标(人头)
	 0x10: 车牌检测
	 0x20: 移动侦测（灯泡智能警戒使用）
	 0x40: 人脸检测
	***/
	 unsigned int u32Cmd;  /*命令标记*/

	 unsigned int u32AlarmFlag; /*触发报警(人形/车牌)1：报警(检测到目标)  0：不报警(未检测到目标)*/

	 /***
	 u32TargetNum
	 	bit0~bit7: 目标个数    
	 	bit16~bit31: 框个数
	 ***/
	 unsigned int u32TargetNum; /*目标数(人形/车牌) */
	 
	/***
	u32Cmd = 0x10时:
	 u32Reserved[0]:框的起始点水平坐标
	 u32Reserved[1]:框的起始点垂直坐标
	 u32Reserved[2]:框的结束点水平坐标
	 u32Reserved[3]:框的结束点垂直坐标
	***/
	 unsigned int u32Reserved[64];
}ISP_GET_AI_RESULT_S;

typedef struct stISP_GET_AI_FUNC_S
{
	int(*pfn_get_ai_result)(ISP_GET_AI_RESULT_S *pstAiResult);	// 返回值：0:成功      	-1:失败
} ISP_GET_AI_FUNC_S;



/*************************************************************************
函数功能:	获取/设置 日夜模式
输入参数:			0:	Auto
				1:	Color
				2:	BW
				3:	智能警戒
				4: 	智能暖光(可变光)
				5:	智能红外
				6:	车牌模式
note:
*************************************************************************/
int camera_set_dnc_mode_chn(int chn, unsigned int mode);
int camera_get_dnc_mode_chn(int chn, unsigned int *pMode);
int camera_get_dnc_mode(unsigned int *pMode);
int camera_set_dnc_mode(unsigned int mode);


/*************************************************************************
函数功能:	获取支持的曝光等级数
输出参数:		chn: 通道号[0,2]
			vstd: 制式[0,1](0:PAL 1:NTSC)
			speeds: 每个等级的值
返回参数:		支持的等级的个数
*************************************************************************/
int camera_get_exposure_speeds_chn(int chn, int vstd, unsigned int *speeds);
int camera_get_exposure_speeds(int vstd, unsigned int *speeds);


/*************************************************************************
函数功能:	曝光模式设置
输入参数:		chn: 通道号[0,2]
			level:	0		自动曝光
					1~9		手动曝光(固定几档)	
			value1:	曝光下限(min,us)
			value2:	曝光上限(max,us)
输出参数:		pLevel:	0		自动曝光
					1~9		手动曝光(固定几档)	
			pValue1:	曝光下限(min,us)
			pValue2:	曝光上限(max,us)
note:	
	自动曝光(上下限无效)  		level :0
	自动曝光(上下限有效)  		level :255
	手动曝光(固定几档)			level :1~9
*************************************************************************/
int camera_set_exposure_level_chn(int chn, int level, unsigned int u32MinUs, unsigned int u32MaxUs);
int camera_get_exposure_level_chn(int chn, int *pLevel, unsigned int *pValue1, unsigned int *pValue2);
int camera_get_exposure_level(int *pLevel, unsigned int *pValue1, unsigned int *pValue2);
int camera_set_exposure_level(int level, unsigned int value1, unsigned int value2);

/*******************************************************************
函数功能:	设置/获取自动增益
输入参数:		chn: 通道号[0,2]
			s32GainMax:	最大增益( 	[0,100] def: 50)
			s32AutoEn:	自动增益使能	([0,1]		def: 1)
输出参数:		pGainLevel:	最大增益( 	[0,100] def: 50)
			pAutoEn:	自动增益使能	([0,1]		def: 1)
返回参数:	0 	成功
				-1	出错
Note:			Lycai
*******************************************************************/
int camera_set_gain_chn(int chn, int s32GainMax, int s32AutoEn);
int camera_get_gain_chn(int chn, int *pGainLevel, int *pAutoEn);
int camera_get_gain(int *pGainLevel, int *pAutoEn);
int camera_set_gain(int s32GainMax, int s32AutoEn);


/*******************************************************************
函数功能:	设置/获取曝光参考
输入参数:		chn: 通道号[0,2]
			level: 目标等级[0,100] def:50
输出参数:	无
返回参数:	0 	成功
				-1	出错
Note:			Lycai
*******************************************************************/
int camera_set_refrence_level_chn(int chn, int level);
int camera_get_refrence_level_chn(int chn);
int camera_get_refrence_level(void);
int camera_set_refrence_level(int level);


/*******************************************************************
函数功能:	获取平均亮度值
输入参数:		chn: 通道号[0,2]
返回参数:		当前亮度
返回参数:	0 	成功
				-1	出错
Note:			Lycai
	暂未支持
*******************************************************************/
int camera_get_luminance_chn(int chn);
int camera_get_luminance(void);


/*******************************************************************
函数功能:	获取状态
输入参数:		chn: 通道号[0,2]
输出参数:		status:
				>=0表示状态正常,<0表示状态异常
返回参数:		0 	成功
			-1	出错
Note:			Lycai
	暂未支持
*******************************************************************/
int camera_get_status_chn(int chn, int *status);
int camera_get_status(int *status);


/*******************************************************************
函数功能:	设置自动光圈(打开/关闭)
输入参数:		chn: 通道号[0,2]
输出参数:		mode: [0,1]  0:关闭 1:打开
返回参数:		0 	成功
			-1	出错
Note:			Lycai
*******************************************************************/
int camera_set_aperture_chn(int chn, unsigned int mode);
int camera_set_aperture(unsigned int mode);


/*******************************************************************
函数功能:	设置/获取背光补偿
输入参数:		chn: 通道号[0,2]
			u32Enable: 使能开关[0,1]
			u32Level: 强度等级[0,100] def:0
输出参数:		pu32Enable: 使能开关[0,1]
			pu32Level: 强度等级[0,100] def:0
返回参数:	0:	 成功
		 -1: 失败
*******************************************************************/
int camera_set_blc_chn_v2(int chn, unsigned int u32Enable, unsigned int u32Level);
int camera_get_blc_chn_v2(int chn, unsigned int *pu32Enable, unsigned int *pu32Level);
int camera_get_blc(unsigned int *pMode);
int camera_set_blc(unsigned int mode);
int camera_set_blc_v2(unsigned int u32Enable, unsigned int u32Level);
int camera_get_blc_v2(unsigned int *pu32Enable, unsigned int *pu32Level);

int camera_get_face_exposure(unsigned int *pu32Enable);
int camera_set_face_exposure(unsigned int u32Enable);

int camera_get_plate_exposure(unsigned int *pu32Enable);
int camera_set_plate_exposure(unsigned int u32Enable);

/*******************************************************************
函数功能:	设置/获取情景模式
输入参数:		chn: 通道号[0,2]
			scene: 模式[0,2]
输出参数:	pScene: 模式[0,2]
返回参数:	0:	 成功
		 -1: 失败
*******************************************************************/
int camera_set_scene_chn(int chn, CAMERA_SCENE scene);
int camera_get_scene_chn(int chn, CAMERA_SCENE* pScene);

int camera_get_scene(CAMERA_SCENE* pScene);
int camera_set_scene(CAMERA_SCENE scene);


/*******************************************************************
函数功能:	获取电子慢快门状态(当前)
输入参数:	chn: 通道号[0,2]
输出参数:	*pu8EShutterSts:
					 fps' = fps*0x10/gu8EshutterSts
返回参数:	0:	成功
				-1:	失败
*******************************************************************/
int camera_get_es_status_chn(ISP_S32 chn, ISP_U8 *pu8EShutterSts);
int camera_get_es_status(unsigned char *pu8EShutterSts);


/*************************************************************************
函数功能: 	降帧功能（使能）
输入参数:		chn: 通道号[0,2]
			es_shutter: 
					0:		1/1
					2:		1/2 (default)
					4:		1/3
					6:		1/4
输出参数:	pEshutterLevel
					0:		1/1
					2:		1/2 (default)
					4:		1/3
					6:		1/4	
返回参数:	0:	成功
				-1:	出错
*************************************************************************/
int camera_set_es_shutter_chn(int chn, int es_shutter);
int camera_get_es_shutter_chn(int chn, int *pEshutterLevel);
int camera_get_es_shutter(int *pEshutterLevel);
int camera_set_es_shutter(int es_shutter);


/*******************************************************************
函数功能:	设置/获取日光灯防闪
输入参数:		chn: 通道号[0,2]
			en: 使能开关[0,1] def:0
输出参数:		pEn: 使能开关[0,1] def:0
返回参数:	0:	 成功
		 -1: 失败
*******************************************************************/
int camera_set_reject_flicker_chn(int chn, int en);
int camera_get_reject_flicker_chn(int chn, int *pEn);
int camera_get_reject_flicker(int *pEn);
int camera_set_reject_flicker(int en);


/*************************************************************************
函数功能: 	日夜切换阈值
输入参数:		chn: [0,2]
			dnc_thr: [10,50] def:30
输出参数:		pDnc_thr
返回参数:		0:	成功
			-1:	出错
*************************************************************************/
int camera_set_dnc_thr_chn(int chn, int dnc_thr);
int camera_get_dnc_thr_chn(int chn, int *pDnc_thr);
int camera_get_dnc_thr(int *pDnc_thr);
int camera_set_dnc_thr(int dnc_thr);


/*******************************************************************
函数功能:	获取/设置 日夜调节灵敏度
输入参数:		chn: [0,2]
			level: [1,10] def:5
输出参数:	* pLevel:	[1,10] def:5
返回参数:	0 	成功
				-1	出错
Note:			Lycai
*******************************************************************/
int camera_set_ae_sensitivity_chn(int chn, int level);
int camera_get_ae_sensitivity_chn(int chn, int* pLevel);
int camera_get_ae_sensitivity(int*pLevel);
int camera_set_ae_sensitivity(int ae_sensitivity);


/*******************************************************************
函数功能:	获取/设置 AE调节灵敏度
输入参数:		chn: [0,2]
			level: [1,10] def:5
输出参数:		*pLevel: [1,10] def:5
返回参数:		0 	成功
			-1	出错
Note:			Lycai
*******************************************************************/
int camera_set_ae_sensitivity2_chn(int chn, int level);
int camera_get_ae_sensitivity2_chn(int chn, int* pLevel);
int camera_get_ae_sensitivity2(int* pLevel);
int camera_set_ae_sensitivity2(int level);


/*******************************************************************
函数功能:	获取/设置 IRCUT状态
输入参数:		chn:通道号 [0,2]
			mode:[0,1]  0:不滤红外	1:滤红外
输出参数:	无
返回参数:	0 	成功
				-1	出错
Note:			Lycai
*******************************************************************/
int camera_set_ircut_chn(int chn, int mode);
int camera_set_Ircut(int mode);


/*******************************************************************
函数功能:	获取灯板状态
输入参数:		chn:通道号 [0,2]
输出参数:		无
返回参数:		0: 白天
			1: 晚上
			-1	出错
			others: 不支持
Note:			Lycai
*******************************************************************/
int camera_get_infrared_chn(int chn);
int camera_get_Infrared(void);


/*******************************************************************
函数功能:	获取/设置日夜降噪等级
输入参数:		chn: [0,2]
			daynight(彩色/黑白):[0,1]		
				0(Color) 
				1(BW)
			nf_level(降噪等级):[0,5] Def:3
输出参数:		pNrLevel(降噪等级):[0,5] Def:3
返回参数:	0 	成功
				-1	出错
Note:			Lycai
*******************************************************************/
int camera_set_nf_level_chn(int chn, int daynight, int nf_level);
int camera_get_nf_level_chn(int chn, int daynight, int *pNrLevel);
int CameraGetNFLevel(int daynight, int *pNrLevel);
int CameraSetNFLevel(int daynight, int nf_level);


/*******************************************************************
函数功能:	获取/设置IRCUT反序
输入参数:		chn: [0,2]
			s32Wap(反序):[0,1]		
				0(正序) 
				1(反序)
输出参数:		ps32Swap:[0,1]		
				0(正序) 
				1(反序)
返回参数:	0 	成功
				-1	出错
Note:			Lycai
*******************************************************************/
int camera_set_icr_swap_chn(int chn, int s32Wap);
int camera_get_icr_swap_chn(int chn, int *ps32Swap);
int CameraGetSwapICR(int *pSwap);
int CameraSwapICR(int swap);


/*******************************************************************
函数功能:	获取/设置宽动态
输入参数:		chn: [0,2]
			level(强度等级):[0,100] Def:50
			enable(使能开关):[0,1] Def:0
输出参数:		pLevel(强度等级):[0,100] Def:50
			pEnable(使能开关):[0,1] Def:0
返回参数:	0 	成功
				-1	出错
Note:			Lycai
*******************************************************************/
int camera_get_wdr_chn(int chn, int* pLevel, int* pEnable);
int camera_set_wdr_chn(int chn, int level, int enable);
int camera_get_wdr(int* pLevel, int* pEnable);
int camera_set_wdr(int level, int enable);


/*******************************************************************
函数功能:	获取/设置 图像风格
输入参数:		chn: [0,2]
			choice: [0,2] def:1
输出参数:	*pChoice:	[0,2] def:1
返回参数:	0 	成功
				-1	出错
Note:			Lycai
*******************************************************************/
int camera_get_style_chn(int chn, int *pChoice);
int camera_set_style_chn(int chn, int choice);
int Camera_Get_StyleMode(int *pChoice);
int Camera_Set_StyleMode(int choice);		


/*******************************************************************
函数功能:	获取/设置 IRCUT同步方式
输入参数:		chn: [0,2]
			ircut_mode: [0,1]
				0: 红外灯同步
				1: 自动同步
输出参数:		*pIrcutMode: [0,1]
				0: 红外灯同步
				1: 自动同步
返回参数:		0 	成功
			-1	出错
Note:			Lycai
*******************************************************************/
int camera_set_ircut_mode_chn(int chn, int ircut_mode);
int camera_get_ircut_mode_chn(int chn, int *pIrcutMode);
int camera_get_ircut_mode(int *pIrcutMode);
int camera_set_ircut_mode(int ircut_mode);


/*******************************************************************
函数功能:	获取/设置 镜像（左右）
输入参数:		chn: [0,2]
			mirror: [0,1] def:0
输出参数:	*pMirror:	[0,1] def:0
返回参数:	0 	成功
				-1	出错
Note:			Lycai
*******************************************************************/
int camera_set_mirror_chn(int chn, int mirror);
int camera_get_mirror_chn(int chn, int *pMirror);
int camera_get_mirror(int *pMirror);
int camera_set_mirror(int mirror);


/*******************************************************************
函数功能:	获取/设置 翻转（上下）
输入参数:		chn: [0,2]
			flip: [0,1] def:0
输出参数:	*pFlip:	[0,1] def:0
返回参数:	0 	成功
				-1	出错
Note:			Lycai
*******************************************************************/
int camera_set_flip_chn(int chn, int flip);
int camera_get_flip_chn(int chn, int *pFlip);
int camera_get_flip(int *pFlip);
int camera_set_flip(int flip);


/*************************************************************************
函数功能: 	图像颜色(Web) 接口
输入参数:	channel:	 无效
				pColor:
					Brightness: 亮度(0~100)
					Contrast: 对比度(0~100)
					Saturation: 饱和度(0~100)
					Hue:	色度(0~100)
					Acutance: 锐化(0~15)
返回参数:	0:	成功
				-1:	出错
*************************************************************************/
int camera_get_color_chn(int channel, CAMERA_COLOR * pColor);
int camera_set_color_chn(int channel, CAMERA_COLOR * pColor);


/*************************************************************************
函数功能: 	获取/设置 视频制式(PAL/NTSC)
输入参数:	channel:	 无效
				u32Vstd: 0:(UN)
						1:PAL
						2:NTSC
输出参数:	*pu32Vstd	0:(UN)
							1:PAL
							2:NTSC
返回参数:	0:	成功
				-1:	出错
*************************************************************************/
int camera_get_vstd(int channel,unsigned int *pu32Vstd);
int camera_set_vstd(int channel,unsigned int u32Vstd);


/*************************************************************************
函数功能:	获取/设置 去伪彩
输入参数:		chn:[0,2]
			u32Level:[0,100](def:0)
输出参数:	pu32Level:	[0,100](def:0)
返回参数:	0:	成功
				-1:	出错
*************************************************************************/
int camera_set_demoire_chn(int chn,unsigned int u32Level);
int camera_get_demoire_chn(int chn,unsigned int *pu32Level);
int camera_set_antiFalseColor(unsigned int u32Level);
int camera_get_antiFalseColor(unsigned int *pu32Level);


/*************************************************************************
函数功能: 	获取/设置 去锯齿
输入参数:		chn:[0,2]
			u32Level:[0,100](def:0)
输出参数:	pu32Level:[0,100](def:0)
返回参数:	0:	成功
				-1:	出错
*************************************************************************/
int camera_set_sawtooth_chn(int chn, unsigned int u32Level);
int camera_get_sawtooth_chn(int chn, unsigned int *pu32Level);
int camera_set_sawtooth(unsigned int u32Level);
int camera_get_sawtooth(unsigned int *pu32Level);


/*************************************************************************
函数功能: 	获取/设置 强光抑制（红外防过曝）
输入参数:		chn: 通道号[0,2]
			Enable: 使能开关[0,1] def:0
输出参数:		pEnable: 使能开关[0,1] def:0
返回参数:	0:	成功
				-1:	出错
*************************************************************************/
int camera_set_hlc_chn(int chn,int Enable);
int camera_get_hlc_chn(int chn,int *pEnable);
int camera_set_hlc(int Enable);
int camera_get_hlc(int *pEnable);



/*************************************************************************
函数功能:	获取日夜状态
输入参数: chn: 通道号[0,2]
输出参数:	pu8Mode:
					0:	Color
					1:    BW
note:
*************************************************************************/
int camera_get_dn_state_chn(int chn, unsigned char *pu8Mode);
int camera_get_dn_state(unsigned char *pu8Mode);
int camera_get_dn_state_double_chn(int chn, unsigned int *pu8Mode);

/*************************************************************************
函数功能: 	配置/获取 LED模式
输入参数:		chn:通道号[0,2]
			s32Mode:	0xAB(	A:	0: IR   1: WL   --- 仅读取时有效
							 	B:	0:Auto  1:Manual  2:Intelligence)
			s32State(for Manual mode): 	0:Close 1:Open
输出参数:		ps32Mode:	0xAB(	A:	0: IR   1: WL   --- 仅读取时有效
							 	B:	0:Auto  1:Manual  2:Intelligence)
			ps32State(for Manual mode): 	0:Close 1:Open
返回参数:		0:	成功
			-1:	出错
*************************************************************************/
int camera_set_ledMode_chn(int chn, int s32Mode, int s32State);
int camera_get_ledMode_chn(int chn, int *ps32Mode, int *ps32State);
int camera_set_ledMode(int s32Mode, int s32State);
int camera_get_ledMode(int *ps32Mode, int *ps32State);


/*************************************************************************
函数功能: 	配置 LED类型
输入参数:		chn:通道号 [0,2]
			s32Type:[0,2]	0: IR   1: WL  2:Double
输出参数:	ps32Type:[0,2]	0: IR   1: WL  2:Double
返回参数:	0:	成功
				-1:	出错
*************************************************************************/
int camera_set_ledType_chn(int chn, int s32Type);
int camera_get_ledType_chn(int chn, int *ps32Type);
int camera_set_ledType(int s32Type);
int camera_get_ledType(int *ps32Type);


/*************************************************************************
函数功能: 	配置/获取 LED亮度等级
输入参数:		chn:通道号 [0,2]
			s32Mode: 灯类型(保留)  0:白光灯
			s32Level: 亮度等级 (0~100)
输出参数:	ps32Level: 亮度等级 (0~100)
返回参数:	0:	成功
				-1:	出错
*************************************************************************/
int camera_set_ledLumLevel_chn(int chn, int s32Mode, int s32Level);
int camera_get_ledLumLevel_chn(int chn, int s32Mode, int *ps32Level);
int camera_set_ledLumLevel(int s32Mode, int s32Level);
int camera_get_ledLumLevel(int s32Mode, int *ps32Level);


/*************************************************************************
函数功能: 	配置/获取 移动侦测(智能算法)结果
输入参数:		chn:通道号 [0,2]
			u8MovFlg:[0,1]  1:触发(移动侦测)报警
输出参数:		pu8MovFlg:[0,1]  1:触发(移动侦测)报警
返回参数:		0:	成功
			-1:	出错
*************************************************************************/
int camera_set_smartVda_chn(int chn,unsigned char u8MovFlg);
int camera_get_smartVda_chn(int chn, unsigned char *pu8MovFlg);
int camera_set_smartVda(unsigned char u8MovFlg);
int camera_get_smartVda(unsigned char *pu8MovFlg);


/*************************************************************************
函数功能: 	获取稳定状态(灯泡)
输入参数:	chn: 通道号[0,2]
输出参数:	pu8StateFlg
					0: 稳定
					1: 操作LED
返回参数:	0:	成功
				-1:	出错
*************************************************************************/
int camera_get_stabState_chn(int chn,unsigned char *pu8StateFlg);
int camera_get_stabState(unsigned char *pu8StateFlg);


/*************************************************************************
函数功能: 	获取 报警LED持续时间(ms)
输入参数:		chn: 通道号[0,2]
			s32Tms:  持续时间(ms) ----(精度100ms)
输出参数:	无
返回参数:	0:	成功
				-1:	出错
*************************************************************************/
int camera_set_ledHold_chn(int chn,int s32Tms);
int camera_get_ledHold_chn(int chn,int *ps32Tms);
int camera_set_ledHold(int s32Tms);
int camera_get_ledHold(int *ps32Tms);


/*************************************************************************
函数功能: 		配置人形防过曝使能
输入参数:		chn:通道号[0,2]
			en:	1:使能   0:禁能
输出参数:		ps32En
返回参数:		0:	成功
			-1:	出错
*************************************************************************/
int camera_set_aeweight_chn(int chn, int en);
int camera_get_aeweight_chn(int chn, int *ps32En);
int camera_set_aeweight(int en);
int camera_get_aeweight(int *ps32En);


/*************************************************************************
函数功能:		配置软红外阈值
输入参数:		chn:通道号[0,2]
			dnc_thr:[1,5] def:3 (越小，越早切黑白)
输出参数:	无
返回参数:	0:	成功
				-1: 出错
*************************************************************************/
int camera_set_softIr_thr_chn(int chn, int dnc_thr);
int camera_set_softIr_thr(int dnc_thr);
int camera_get_softIr_thr_chn(ISP_S32 chn, ISP_S32 *pdnc_thr);


/*************************************************************************
函数功能:		配置灯板信号反向（和默认对应）
输入参数:		chn:通道号[0,2]
			s32SWapEn:[0,1] def:0
				0: 不反序    	1:反序
输出参数:		ps32SWapEn:[0,1] def:0
				0: 不反序    	1:反序
返回参数:	0:	成功
				-1: 出错
注：
	灯板->主板
*************************************************************************/
int camera_set_infrared_swap_chn(int chn, int s32SWapEn);
int camera_get_infrared_swap_chn(int chn, int *ps32SWapEn);
int camera_set_infrared_swap(int s32SWapEn);
int camera_get_infrared_swap(int *ps32SWapEn);


/*************************************************************************
函数功能:		配置LED灯控制信号反向（和默认对应）
输入参数:		s32SWapEn:
				bit0:  红外灯 0: 不反序    	1:反序
				bit4:  白光灯 0: 不反序    	1:反序
输出参数:	无
返回参数:	0:	成功
				-1: 出错
注：
	主要针对双光灯板的控制电平(默认高电平亮、低电平灭)
*************************************************************************/
int camera_set_ledDrv_swap_chn(int chn, int s32SWapEn);
int camera_get_ledDrv_swap_chn(int chn, int *ps32SWapEn);
int camera_set_ledDrv_swap(int s32SWapEn);
int camera_get_ledDrv_swap(int *ps32SWapEn);


/***************************************************************
函数功能:	去雾功能
输入参数:		chn: 通道号[0,2]
			enable:	打开关闭[0,1] def:0
			level:	强度等级[0,100] def:0
输出参数:	 	penable:打开关闭[0,1] def:0
			plevel:	强度等级[0,100] def:0
返回参数:	0:		成功
				-1:		出错
***************************************************************/
int camera_get_defog_chn(int chn, int *penable, int *plevel);
int camera_set_defog_chn(int chn, int enable, int level);
int Camera_SetClearFog(int enable, int level);
int Camera_GetClearFog(int *penable, int *plevel);


/***************************************************************
函数功能:	获取当前曝光参数
输出参数:		chn:通道号[0,2]
			*pu32line:  当前曝光行数
			*pu32Gain:	当前曝光增益(*1024)
返回参数:		0:		成功
			-1:		出错
***************************************************************/
int camera_get_exposure_data_chn(int chn,int *pu32line,int *pu32Gain);
int camera_get_exposure_data(int *pu32line, int *pu32Gain);
int Camera_Get_ExposureData(int *pT_us, int *pGain);


/***************************************************************
函数功能:	获取sensor型号
输出参数:		chn:通道号[0,2]
返回参数:		sensor型号ID
***************************************************************/
int camera_get_sensor_chn(ISP_S32 chn);
int GetSensorType(void);


/*************************************************************************
函数功能:	配置软硬光敏选择
输入参数:		chn: 通道号
			*ps32Mode: 模式	
输出参数:	无
返回参数:		0:	成功
			-1:	出错
*************************************************************************/
int camera_get_photosensitive_chn(int chn, int *ps32Mode);
int camera_set_photosensitive_chn(int chn, int s32Mode);
int camera_get_Photosensitive(int *photosensitive);
int camera_set_Photosensitive(int photosensitive);

typedef enum{
	FACE_PRIOR = 0,
	UVC_DAY = 1,
	UVC_NIGHT = 2,
}CAMERA_FACE_UVC_AE_MOD;
/*************************************************************************
函数功能:	双摄FUN_EXT_DOOR_LOCK_FACE_UVC定制专用，用于sensor的开关使能
输入参数:		chn: 通道号
			ps32Mode：sensor开关状态
输出参数:	无
返回参数:		0:	成功
			-1:	出错
*************************************************************************/
int camera_set_sensor_state(int chn, int s32Mode);
/*************************************************************************
函数功能:	双摄FUN_EXT_DOOR_LOCK_FACE_UVC定制专用，用于开关算法控制亮度
输入参数:	s32Mode：AE模式
输出参数:	无
返回参数:		0:	成功
			-1:	出错
*************************************************************************/
int camera_set_ae_mod(int s32Mode);
/*************************************************************************
函数功能:	获取/设置 扫描二维码模式
输入参数:	qrcode:   0:	关
				qrcode:   1:    开				
*************************************************************************/
int camera_get_qrcode(ISP_S32 *pQrcode);
int camera_set_qrcode(ISP_S32 qrcode);

int camera_set_af_switch(ISP_S32 ps32Mode);
int camera_get_af_switch(ISP_S32* ps32Mode);
int camera_get_af_stat(ISP_S32 chn,ISP_S32* AfStat);

/*************************************************************************
函数功能:	获取/设置 UVC的输出分辨率
输入参数:	
			s32CaptureSize:	0:关	1:开				
*************************************************************************/
int camera_get_capture_size(ISP_S32 *s32CaptureSize);
int camera_set_capture_size(ISP_S32 s32CaptureSize);

/************************************************************************************************************
函数功能: 启动ISP相关(XM_MPI_ISPAPP_RUN + 解析isp.json)
输入参数:		u32Mode:
				bit0~bit3: VStd(1:PAL  2:NTSC)
			s32Rsv: 保留
输出参数: 无
返回参数: 		0:成功
			-1:失败
注:
	涉及isp.json的解析
************************************************************************************************************/
int camera_init(unsigned int u32Mode, int s32Rsv);

/*************************************************************************
函数功能:	图像相关进行退出操作
输入参数:	无
输出参数:	无
返回参数:		0:	成功
			-1:	出错
*************************************************************************/
int camera_exit(void);

/*************************************************************************
函数功能:	编码/制式/分辨率 模式切换
输出参数:	u8Mode:
					0x00:AHD	0x01:CVI		0x02:TVI		0x10:XVI
				u8VstdMode:	
					1: PAL	2:NTSC
				u8RlstMode:
					0:1M 1:2M 3:3M 4:4M 5:5M
note:
*************************************************************************/
int camera_set_isp_para(unsigned char u8Mode, unsigned char u8VstdMode, unsigned char u8RlstMode);
int camera_get_isp_para(unsigned char *pu8Mode);

int camera_set_language(int s32Language);

/*****************************************************************************
函数功能:	外部调试选择gamma
输入参数:    	u32Gamma: [High Byte->Low Byte] = [u8IdxWgt(Idx2)  u8Idx2  u8Idx1]
				u8Idx1:	0: 		disable gamma  (线性)
						1~15:	LinearGamma
						128~255:	WdrGamma
				u8Idx2:	0: 		disable gamma  (线性)
						0~15:	LinearGamma
						128~255:	WdrGamma
				u8IdxWgt: (0~255)
						u8Idx1 Weight: 255-u8IdxWgt
						u8Idx2 Weight: u8IdxWgt				
返回参数:    0: 成功 -1: 失败
*****************************************************************************/
int camera_set_gamma(unsigned int u32Gamma);
int camera_get_gamma(unsigned int *pu32Gamma);


/*************************************************************************
函数功能: 	2M抓图接口
输入参数:	无
输出参数:	pstSnapVFInfo
返回参数:	0:	成功
				-1:	出错
*************************************************************************/
int camera_snap(void *pstSnapVFInfo);

/*************************************************************************
函数功能: 	配置图像坐标
输入参数:	chn: 通道
		  mode: 保留
		  stCoord: 坐标信息
输出参数:	无
返回参数:	0:	成功
				-1:	出错
*************************************************************************/
int camera_set_coord_chn(int chn, int mode, COORD_S stCoord);

/*************************************************************************
函数功能: 	配置图像坐标
输入参数:	pstViWin 
输出参数:	无
返回参数:	0:	成功
				-1:	出错
*************************************************************************/
int camera_get_vi_resolution(VI_WIN_S *pstViWin);

/*************************************************************************
函数功能:		注册获取智能库结果的接口函数
输入参数:		pstGetAiFunc:
				指定函数
输出参数:	无
返回参数:	0:	成功
				-1: 出错
*************************************************************************/
int camera_register_callback_aiFun(ISP_GET_AI_FUNC_S *pstGetAiFunc);

/*************************************************************************
函数功能:		设置获取算法支持类型
输入参数:		u32AiType: //bit0:人形bit1: 人脸bit2:车牌
				
输出参数:	无
返回参数:	0:	成功
				-1: 出错
*************************************************************************/


int camera_set_aiType(unsigned int u32AiType);
int camera_get_aiType(unsigned int *pu32AiType);


// 设置白平衡参数
int camera_set_wb_mode(unsigned int mode);
//用于调试
int camera_debug(char *cmd);

int camera_aew_get_version(char *str);

//返回WB增益
void  camera_get_wb_params(void *param);

int camera_save_debug_cmd(char *cmd);

int Camera_Get_DebugFile(char *fliename, unsigned int choice, unsigned int depth);

int camera_scan_task(unsigned int u32Tms);


/*************************************************************************
函数功能: 	camera_set_format
输入参数:	chn
				u32Type: 0MainStream	1:SubStream1	2:SubStream2
				pstFormat: encode paramer
输出参数:	none
返回参数:	0:	成功
				-1:	出错
*************************************************************************/
int camera_set_format_chn(int chn, unsigned int u32Type,const CAPTURE_FORMAT *pstFormat);
int camera_set_format(int chn, unsigned int u32Type,const CAPTURE_FORMAT *pstFormat);


/*************************************************************************
输入参数:	u8Mode:		1: Read 	
							2: Write
				pu8Status: 			fps
							0:		1/1
							2:		1/2
输出参数:	无
返回参数:	0:	成功
				-1:	出错
*************************************************************************/
int camera_es_shutter_ex(unsigned char u8Mode, unsigned char *pu8Status);


/*************************************************************************
函数功能: 	产品型号(读/写)
输入参数:	u8Mode:		1: Read 	
							2: Write
				*pu32ProductType:	产品型号
输出参数:	*pu32ProductType:	产品型号
返回参数:	0:	成功
				-1:	出错
*************************************************************************/
int camera_productType(unsigned char u8Mode, unsigned int *pu32ProductType);


/*************************************************************************
函数功能: 	设置/获取 限制AWB色温
输入参数:	u8MinCt: 	最低色温	bit7: En  bit0~bit6: Choice
			u32Start: 	开始限制增益(x1024)
			u32End: 	放开限制增益(x1024)
输出参数:	无
返回参数:	0:	成功
				-1:	出错
*************************************************************************/
int camera_awbLimit_set(unsigned char u8MinCt, unsigned int u32Start, unsigned int u32End);
int camera_awbLimit_get(unsigned char *pu8MinCt, unsigned int *pu32Start, unsigned int *pu32End);


/*************************************************************************
函数功能: 	设置/获取 Burst使能(BW)
输入参数:	u8Enable: 0:Disable 	1:Enable
输出参数:	无
返回参数:	0:	成功
				-1:	出错
*************************************************************************/
int camera_bwBurst_set(unsigned char u8Enable);
int camera_bwBurst_get(unsigned char *pu8Enable);

int camera_set_wbRB(unsigned char u8Data);
int camera_get_wbRB(unsigned char *pu8Data);

int camera_set_wbGM(unsigned char u8Data);
int camera_get_wbGM(unsigned char *pu8Data);


#ifdef __cplusplus
}
#endif

#endif

