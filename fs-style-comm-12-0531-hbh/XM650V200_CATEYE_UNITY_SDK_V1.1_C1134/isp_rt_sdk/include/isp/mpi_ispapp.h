#ifndef __MPI_ISPAPP_H__
#define __MPI_ISPAPP_H__
#if 1
typedef enum enISP_FUN_EXT
{
	FUN_EXT_ONE_SENSOR				= 0x00,//常规设备
	FUN_EXT_DOUBLE_SENSOR			= 0x11,//常规双摄
	FUN_EXT_DOOR_LOCK_FACE_USED		= 0x12,//旧的门锁人脸识别设备（双sc2336）
	FUN_EXT_DOOR_LOCK_FACE			= 0x13,//新的门锁人脸识别模式
	FUN_EXT_DOOR_LOCK_FACE_UVC		= 0x14,//新的门锁人脸识别+UVC出流
	FUN_EXT_DOOR_LOCK_UVC			= 0x15,//新的门锁UVC出流模式
	FUN_EXT_LICENCE_PLATE			= 0x40,//车牌识别模式
} ISP_FUN_EXT;

typedef enum enCAPTURE_SIZE_T {
	UVC_SIZE_640_480 = 0,	// 640*480
	UVC_SIZE_720P,			// 1280*720
	UVC_SIZE_800_480,		// 800*480
	UVC_SIZE_854_480,		// 854*480
	UVC_SIZE_864_480,      	// 864*480
	UVC_SIZE_640_368,      	// 640*368
	UVC_SIZE_480_320,      	// 480*320
	UVC_SIZE_1600_1200,		// 1600*1200
	UVC_SIZE_1280_480,      // 1280*480 double camera splice 2*640*480
	UVC_SIZE_480_800,		// 480*800
	UVC_SIZE_480_864,      	// 480*864
	UVC_SIZE_320_240,      	// 320*240
	UVC_SIZE_1024_600,		// 1024x600
	UVC_SIZE_EXT_V1_NR = 13
}CAPTURE_SIZE_T;
#endif
typedef struct stISPAPP_WIN_ATTR_S
{
/*********************************************
u8Mode:
	0: Real Data(实际值)
	1: 基于标准值进行偏移
	2: Refresh
	
u8Mirror:  镜像打开
u8Flip: 翻转打开

u16ValH: 水平方向值(bit15:为符号)    [0, 0xFFFE]
	0~0x7FFF			:
	0x8000 ~ 0xFFFE	:  <0

u16ValV: 垂直方向值(bit15:为符号)	  [0, 0xFFFE]
	0~0x7FFF			:
	0x8000 ~ 0xFFFE	:  <0

note:  
	u16ValH/u16ValV = 0xFFFF 时标准不写入
*********************************************/
	unsigned char u8Mode;
	//unsigned char u8Mirror;
	//unsigned char u8Flip;
	unsigned short u16ValH;
	unsigned short u16ValV;
	unsigned int au32Rsv[4];
} ISPAPP_WIN_ATTR_S;

/*************************************************************************************************************************
ISPAPP_TASK_CFG
参数说明:
	1.u32ProductType:		产品型号;用于内部区分产品/参数，建议默认为0
	2.u32DspType:			DSP型号;用于区分DSP型号，建议默认为0
	3.u32StdType:			视频制式PAL/NTSC
								1: PAL
								2: NTSC
	4.u32RsltType:			isp输出分辨率(参数值参考枚举RESL_TYPE) --- 该配置需要匹配代码(需代码支持)，默认建议配置0
							bit[0,7]:sensor0
							bit[8,15]:senesor1
							参数范围:
								0, --- 按软件内部定
								P1080_ = 1,
								P960_ = 2,	
								P1536_ = 3,    
								P4M_ = 4,    
								P5M_ = 5,    
								P4K_ = 6,  
								P5M_2 = 7,		// 16:9  (2880*1620)
								P1080_B = 8,	// 1080P binning(H/2 V/2)							
	5.au32SoftLed[2]:			软光敏、软控灯相关，建议默认为0 (au32SoftLed[0]:通道0  ; au32SoftLed[1]:通道1)
		bit31:
			0	not use
			1	use
		bit23:	控制白光灯的电平
		bit16~bit22:	控制白光灯的GPIO	(0x00: choice by source,100:表示GPIO00)
		bit15: 	控制红外灯的电平
		bit8~bit14:	控制红外灯的GPIO (0x00: choice by source,100:表示GPIO00)
		bit4:
			0	软光敏
			1	硬光敏
		bit0:
			0  	普通模式（红外同步/自动同步... (硬件控制红外灯)）
			1	软件控制灯	
	6.au32RsltCh:			保留、未支持，建议默认为0
	7.u32Infrared:		硬光敏灯板信号相关，建议默认为0
		bit31:
			0	not use		---接口不配置，软件内部定
			1	use			---该接口配置生效
		bit16~bit23: InfraredGPIO_chn1 (0x00: choice by source,100:表示GPIO00)	   管脚区分（通道1）
		bit8~bit15: InfraredGPIO_chn0 (0x00: choice by source,100:表示GPIO00)	   管脚区分（通道0）
		bit1: InfraredSwap_chn1		采集极性反序（通道1）
		bit0: InfraredSwap_chn0		采集极性反序（通道0）
	8.u32IrCut:			IRCUT相关配置，建议默认为0
		bit31:
			0	not use		---接口不配置，软件内部定
			1	use			---该接口配置生效
		bit16~bit23: IrcutGPIO_chn1 (0x00: choice by source,100:表示GPIO00)（通道1）
		bit8~bit15: IrcutGPIO_chn0 (0x00: choice by source,100:表示GPIO00)（通道0）
		bit1: IRCUT_chn1 Swap（通道1）
		bit0: IRCUT_chn0 Swap（通道0）	
	9.u32CmosPort:		针对双摄Sensor1的接口(需代码支持)，默认建议配置0
		0: 软件内部定
		1: 强制为MIPI
		2: 强制为DVP
	10:u32Fps:			强制指定PAL/NTSC下的帧率(需代码支持)   
		bit0~bit7:	sensor0_PAL		
		bit8~bit15: sensor0_NTSC
		bit16~bit23: sensor1_PAL
		bit24~bit31: sensor1_NTSC
		参数范围: 
			0: 按软件内部定
			12/15/25/30
	11.u32FunExt:		功能标记
		bit0~bit7:
			0: 默认(通用IPC  )
			1: 双摄
			0x40: 抓车牌
			0x50: 直播机
			0x60: IPC+AHD(AHD时钟由驱动部配置)
		bit8~bit15: 	HDR标记
			2:HDR(line-by-line)
	12.u32SnsResetIO:
		bit31:
			0	not use		---接口不配置，软件内部定
			1	use			---该接口配置生效
		bit30:		复位电平
			0:  低电平复位；高电平释放
			1:  高电平复位；低电平释放
		bit0~bit15:  复位管脚
			255: 硬件RC电路复位
			166: GPIO166复位
	12.u32Others:
			bit0~bit3: MCLK GPIO选择(DVP1)	--- 0:默认(GPIOH236);	1:GPIOC091
			bit3~bit7: DVP_Data GPIO选择(DVP1)	---  0:默认(GPIOH224~H235);  1:GPIOC80~GPIOC87
*************************************************************************************************************************/
//详细备注如上
typedef struct stISPAPP_TASK_CFG
{
	unsigned int u32ProductType;
	unsigned int u32DspType;			// 0:Rsv
	unsigned int u32StdType;			// 0:Rsv 1:PAL 2:NTSC
	unsigned int u32RsltType;			// 0:Rssv  1:1080P	2:960P 3:1536P ...  bit[0,7]:sensor0   bit[8,15]:senesor1
	unsigned int u32RsltTypeSns;			// 0:Rssv  1:1080P	2:960P 3:1536P ...  bit[0,7]:sensor0   bit[8,15]:senesor1
	unsigned int au32SoftLed[2];
	unsigned int au32RsltCh[4][4];	// 备用
	unsigned int u32Infrared;
	unsigned int au32IrCut[2];
	unsigned int u32IrCutTestIO;			//用来测试IRC功能是否完好的两个IO之一(其中一个IO固定为ADC)
	unsigned int u32CmosPort;			// 0:Rsv  1:MIPI  2:DVP
	unsigned int u32Fps;				// 0:Rsv 
	unsigned int u32FunExt;			// 0:Rsv
	unsigned int u32SnsResetIO;
	unsigned int u32Flag;			// 相关属性标记	
	unsigned int u32SensorType;		// 指定sensor类型：0：不指定 ；bit0-15:sensor1，bit 16-31sensor2
	unsigned int u32Others;				//其他相关
	unsigned int au32Rsv[4];			// 备用
}ISPAPP_TASK_CFG;

typedef struct stISPAPP_INFO
{
	ISP_FUN_EXT u8FunExt;
	unsigned char u8LimitedUp;
	unsigned char u8ChnMask;
	unsigned char au8Rsv[5];
	unsigned int u32ProductType;
	unsigned int u32DspType;			// 0:Rsv
	unsigned int u32SnsNum;
	unsigned int au32SnsStdType[3];			// 0:Rsv 1:PAL 2:NTSC
	unsigned int au32SnsRsltType[3];			// 0:720P  1:1080P	2:960P 3:3M 4:4M 5:5M
	unsigned int au32SnsID[3];
	unsigned int au32Rsv[6];			// 备用
}ISPAPP_INFO;


int XM_MPI_ISPAPP_Run(ISPAPP_TASK_CFG stTaskCfg);

int XM_MPI_ISPAPP_Exit(unsigned int u32Rsv);

/*************************************************************************
函数功能:	配置VI裁剪参数(H、V)  ---针对镜像翻转等
输入参数:		IspDev:  isp设备号
			pstWinAttr
输出参数: 无
返回参数: 0: 成功    		-1: 失败
note:  
	u16ValH/u16ValV = 0xFFFF 时标准不写入
*************************************************************************/
int XM_MPI_ISPAPP_WinSet(int IspDev, ISPAPP_WIN_ATTR_S *pstWinAttr);


/*************************************************************************
函数功能:	配置/刷新PLL相关配置(针对WDR切换模式)
输入参数:	u32WdrSnsMode: WDR sensor当前的模式
			0:Linear	2:HDR(line-by-line)
输出参数: 无
返回参数: 0: 成功 
*************************************************************************/
int XM_MPI_ISPAPP_PllSet(unsigned int u32WdrSnsMode);


/*************************************************************************
函数功能:	获取libispapp.a中的相关信息
输入参数:	无
输出参数: pstInfo: 相关信息
返回参数: 0: 成功 
*************************************************************************/
int XM_MPI_ISPAPP_InfoGet(ISPAPP_INFO *pstInfo);

#endif

