#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include "mpi_awb.h"
#include "mpi_ispapp.h"
#include "isp_print.h"
#include "autoscene_para.h"

#define DNC_DELAY		(10)	// <0xF0

typedef struct stDNC_INNER_S
{ 
	// 1. DNC
	ISP_U8 u8ColorStatus;		// 0:Color 1:Bw
	
	// 2. IRCUT
	ISP_U8 u8IcrInitFlg;	// 0:NotInit	 1: InitOk
	pthread_mutex_t u32IcrMutex;
}DNC_INNER_S;

static DNC_INNER_S gastDncInnerPar[ISP_NUM_MAX];
static pthread_t g_monitor_pid = 0;
static ISP_U8 gu8DncTaskLoop=1;
extern AUTOSCENE_CONFIG gstConfig;

/*******************************************************************
函数功能:		获取红外灯板的信号
输入参数:		chn: 通道号
			gpio: 管脚号
返回参数:		0:	Day
			1: 	Night
			2:	UN
*******************************************************************/
static ISP_S32 dnc_get_infrared(ISP_DEV chn, ISP_U16 gpio)
{
	static ISP_U8 sau8IrStsLst[ISP_NUM_MAX];
	ISP_U32 tmp = 0;
	ISP_U8 u8ThrL,u8ThrH,u8PolaritySwap=0;
	u8ThrL = 0xAF;
	u8ThrH = 0xCF;
	u8PolaritySwap = 0;
	switch(gpio)
	{
		// ADC
		case 0xFF:
		case 0xFFFF:
				XM_MPI_ISP_GetRegister(0x10020CC0,&tmp);
				tmp = (tmp&~0x73)|0x03|(1<<4);
				XM_MPI_ISP_SetRegister(0x10020CC0, tmp);		// ADC Enable
				
				XM_MPI_ISP_GetRegister(0x10020CC4,&tmp);
				tmp = (tmp<u8ThrL)?0:((tmp>u8ThrH)?1:2);
				break;
		default:
				break;
	}
	if(tmp < 2)
	{
		tmp = tmp?1:0;
		sau8IrStsLst[chn] = u8PolaritySwap?!tmp:tmp;
	}
	return sau8IrStsLst[chn];
}


/*******************************************************************
函数功能:		IR-CUT切换
输入参数:		chn: 通道号
			u32Mode: 备用
			u32Data: IRCUT状态
				0:  滤红外
				1:  不滤红外
				
返回参数:	
*******************************************************************/
static ISP_S32 dnc_ircut(ISP_DEV chn, ISP_U32 u32Mode, ISP_U32 u32Data)
{
	ISP_U8 u8Action = 0;
	ISP_U32 u32A,u32B;

	u32A = gstConfig.astChnAttr[chn].au16IrCutIO[0] & 0xFF;
	u32B = gstConfig.astChnAttr[chn].au16IrCutIO[1] & 0xFF;
	if(0xFF != u32A)
	{
		u8Action ++;
	}
	if(0xFF != u32B)
	{
		if(!u8Action)
		{
			u32A = u32B;
		}
		u8Action ++;
	}
	pthread_mutex_lock(&gastDncInnerPar[chn].u32IcrMutex);
	if(2 == u8Action)
	{
		if(u32Data)
		{
			XM_MPI_ISP_SetRegister(0x10020000+u32A*4, 0x400);
			XM_MPI_ISP_SetRegister(0x10020000+u32B*4, 0xc00);
			usleep(200000);
			XM_MPI_ISP_SetRegister(0x10020000+u32B*4, 0x400);
		}
		else
		{
			XM_MPI_ISP_SetRegister(0x10020000+u32A*4, 0xC00);
			XM_MPI_ISP_SetRegister(0x10020000+u32B*4, 0x400);
			usleep(200000);
			XM_MPI_ISP_SetRegister(0x10020000+u32A*4, 0x400);
		}
	}
	else if(1 == u8Action)
	{
		XM_MPI_ISP_SetRegister(0x10020000+u32A*4, u32Data?0x400:0xC00);
	}
	pthread_mutex_unlock(&gastDncInnerPar[chn].u32IcrMutex);
	return ISP_SUCCESS;
}

static ISP_S32 dnc_to_night(ISP_DEV chn)
{
	ISP_CSC_ATTR_S stCscAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetCSCAttr(chn, &stCscAttr), "XM_MPI_ISP_GetCSCAttr");
	stCscAttr.stGlobal.u8SatuVal = 0;
	ISPCHECK_RET(XM_MPI_ISP_SetCSCAttr(chn, &stCscAttr), "XM_MPI_ISP_SetCSCAttr");

	ISP_SATURATION_ATTR_S stSatAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetSaturationAttr(chn, &stSatAttr), "XM_MPI_ISP_GetSaturationAttr");
	stSatAttr.enOpType = OP_TYPE_MANUAL;
	stSatAttr.stManual.u8Saturation = 0x00;
	ISPCHECK_RET(XM_MPI_ISP_SetSaturationAttr(chn, &stSatAttr), "XM_MPI_ISP_SetSaturationAttr");

	ISP_WB_ATTR_S stWBAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetWBAttr(chn,&stWBAttr), "XM_MPI_ISP_GetWBAttr");
	stWBAttr.enOpType = OP_TYPE_MANUAL;
	stWBAttr.stManual.u16Rgain = 0x100;
	stWBAttr.stManual.u16Ggain = 0x100;
	stWBAttr.stManual.u16Bgain = 0x100;
	ISPCHECK_RET(XM_MPI_ISP_SetWBAttr(chn,&stWBAttr), "XM_MPI_ISP_SetWBAttr");

	dnc_ircut(chn,1,DNS_BW);
	gastDncInnerPar[chn].u8ColorStatus = DNS_BW;
	return 0;
}

static ISP_S32 dnc_to_day(ISP_DEV chn)
{
	dnc_ircut(chn,1,DNS_COLOR);

	ISP_WB_ATTR_S stWBAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetWBAttr(chn,&stWBAttr), "XM_MPI_ISP_GetWBAttr");
	stWBAttr.enOpType = OP_TYPE_AUTO;
	ISPCHECK_RET(XM_MPI_ISP_SetWBAttr(chn,&stWBAttr), "XM_MPI_ISP_SetWBAttr");

	ISP_CSC_ATTR_S stCscAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetCSCAttr(chn, &stCscAttr), "XM_MPI_ISP_GetCSCAttr");
	stCscAttr.stGlobal.u8SatuVal = 50;
	ISPCHECK_RET(XM_MPI_ISP_SetCSCAttr(chn, &stCscAttr), "XM_MPI_ISP_SetCSCAttr");

	ISP_SATURATION_ATTR_S stSatAttr;
	ISPCHECK_RET(XM_MPI_ISP_GetSaturationAttr(chn, &stSatAttr), "XM_MPI_ISP_GetSaturationAttr");
	stSatAttr.enOpType = OP_TYPE_AUTO;
	ISPCHECK_RET(XM_MPI_ISP_SetSaturationAttr(chn, &stSatAttr), "XM_MPI_ISP_SetSaturationAttr");
	gastDncInnerPar[chn].u8ColorStatus = DNS_COLOR;
	return 0;
}

static ISP_S32 dnc_dn_scan(ISP_DEV isp_dev, ISP_U32 u32Ms)
{
	static ISP_U8 sau8toDayNum[ISP_NUM_MAX] = {0xFE};
	static ISP_U8 sau8toNightNum[ISP_NUM_MAX];
	ISP_U8 u8i;
	ISP_S32 s32DnStatus;
	if(sau8toDayNum[0] == 0xFE)
	{
		for(u8i=0;u8i<ISP_NUM_MAX;u8i++)
		{
			sau8toDayNum[u8i] = 0xFF;
			sau8toNightNum[u8i] = 0xFF;
		}
	}
	s32DnStatus = dnc_get_infrared(isp_dev, gstConfig.astChnAttr[isp_dev].u16InfraredIO);
	switch(s32DnStatus)
	{
		// day
		case 0:	
		default:	
				if(	(DNS_COLOR!=gastDncInnerPar[isp_dev].u8ColorStatus) ||
					(sau8toDayNum[isp_dev] == 0xFF))
				{
					if(sau8toDayNum[isp_dev] < DNC_DELAY)
					{
						if(sau8toDayNum[isp_dev] < 0xF0)
						{
							sau8toDayNum[isp_dev] ++;
						}
					}
					else
					{
						dnc_to_day(isp_dev);
						sau8toNightNum[isp_dev] = 0;
						sau8toDayNum[isp_dev] = DNC_DELAY;
					}
				}
				break;
		// night
		case 1:	if(	(DNS_BW!=gastDncInnerPar[isp_dev].u8ColorStatus) ||
					(sau8toNightNum[isp_dev] == 0xFF))
				{
					if(sau8toNightNum[isp_dev] < DNC_DELAY)
					{
						if(sau8toNightNum[isp_dev] < 0xF0)
						{
							sau8toNightNum[isp_dev] ++;
						}
					}
					else
					{
						dnc_to_night(isp_dev);
						sau8toDayNum[isp_dev] = 0;
						sau8toNightNum[isp_dev] = DNC_DELAY;
					}
				}
				break;
	}
	return ISP_SUCCESS;
}

static ISP_S32 dnc_task(ISP_VOID)
{
	ISP_DEV chn;
	ISPAPP_INFO stProductInfo;
	prctl(PR_SET_NAME,(unsigned long)"autoscene_dnc");
	ISPCHECK_RET(XM_MPI_ISPAPP_InfoGet(&stProductInfo), "XM_MPI_ISPAPP_ProductInfoGet");
	gu8DncTaskLoop = 1;
	while(gu8DncTaskLoop)
	{
		for(chn=0;chn<ISP_NUM_MAX;chn ++)
		{
			if(stProductInfo.u8ChnMask&(1<<chn))
			{
				dnc_dn_scan(chn, 100);
			}
		}
		usleep(100000);
	}
	return ISP_SUCCESS;
}

ISP_S32 dnc_set_config(AUTOSCENE_CONFIG *pstConfig)
{
	if(NULL == pstConfig)
	{
		return ISP_FAILURE;
	}
	memcpy(&gstConfig, pstConfig, sizeof(AUTOSCENE_CONFIG));

	if(!gstConfig.astChnAttr[0].au16IrCutIO[0])
	{
		gstConfig.astChnAttr[0].au16IrCutIO[0] = 164;
		gstConfig.astChnAttr[0].au16IrCutIO[1] = 165;
	}
	if(!gstConfig.astChnAttr[0].u16InfraredIO)
	{
		gstConfig.astChnAttr[0].u16InfraredIO = 0xFF;
	}
	return ISP_SUCCESS;
}

ISP_S32 dnc_init(ISP_DEV IspDev,AUTOSCENE_AE_S *pstAttr)
{
	ISP_S32 s32Ret;
	ISP_U8 u8i;
	if(!g_monitor_pid)
	{
		for(u8i = 0; u8i < ISP_NUM_MAX; u8i ++)
		{
			pthread_mutex_init(&gastDncInnerPar[u8i].u32IcrMutex, NULL);
		}
		s32Ret = pthread_create(&g_monitor_pid, NULL, (void *)dnc_task, NULL);
		if(s32Ret != 0)
		{
			ERR("pthread_create dnc_task failed!\n");
			return ISP_FAILURE;
		}
	}
	return ISP_SUCCESS;
}

ISP_S32 dnc_exit(ISP_DEV IspDev)
{
	gu8DncTaskLoop = 0;
	if(g_monitor_pid)
	{
		pthread_join (g_monitor_pid, NULL);
		g_monitor_pid = 0;
	}
	return ISP_SUCCESS;
}


ISP_S32 dnc_get_dn_status(ISP_DEV IspDev, AUTOSCENE_DN_STATUS_E *penStatus)
{
	if(penStatus == NULL)
	{
		return ISP_FAILURE;
	}

	*penStatus = gastDncInnerPar[IspDev].u8ColorStatus?AS_DN_STATUS_BW:AS_DN_STATUS_COLOR;
	return ISP_SUCCESS;
}

ISP_S32 dnc_get_inner_status(ISP_DEV IspDev, AUTOSCENE_INNER_INFO *pstInfo)
{
	if(pstInfo == NULL)
	{
		return ISP_FAILURE;
	}
	pstInfo->u8AiStrategyStr = 50;
	return ISP_SUCCESS;
}



