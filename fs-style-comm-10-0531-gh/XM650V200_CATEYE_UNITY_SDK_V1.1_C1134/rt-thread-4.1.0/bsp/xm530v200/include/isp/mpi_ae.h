#ifndef __MPI_AE_H__
#define __MPI_AE_H__

#include "isp_comm_isp.h"
#include "isp_comm_3a.h"
#include "isp_ae_comm.h"



/* The interface of ae lib register to isp. */
ISP_S32 XM_MPI_AE_GetInner(ISP_DEV IspDev, ISP_U32 *pu32Val);

ISP_S32 XM_MPI_AE_Register(ISP_DEV IspDev, ALG_LIB_S *pstAeLib);
ISP_S32 XM_MPI_AE_UnRegister(ISP_DEV IspDev, ALG_LIB_S *pstAeLib);

/* The callback function of sensor register to ae lib. */
ISP_S32 XM_MPI_ISP_AELibRegCallBack(ISP_DEV IspDev, ALG_LIB_S *pstAeLib, ISP_AE_REGISTER_S *pstRegister);
ISP_S32 XM_MPI_ISP_AELibUnRegCallBack(ISP_DEV IspDev, ALG_LIB_S *pstAeLib);

ISP_S32 XM_MPI_AE_SensorRegCallBack(ISP_DEV IspDev, ALG_LIB_S *pstAeLib, SENSOR_ID SensorId,
    													AE_SENSOR_REGISTER_S *pstRegister);
ISP_S32 XM_MPI_AE_SensorUnRegCallBack(ISP_DEV IspDev, ALG_LIB_S *pstAeLib, SENSOR_ID SensorId);

ISP_S32 XM_MPI_ISP_SetExposureAttr(ISP_DEV IspDev, const ISP_EXPOSURE_ATTR_S *pstExpAttr);
ISP_S32 XM_MPI_ISP_GetExposureAttr(ISP_DEV IspDev, ISP_EXPOSURE_ATTR_S *pstExpAttr);

ISP_S32 XM_MPI_ISP_SetWDRExposureAttr(ISP_DEV IspDev, const ISP_WDR_EXPOSURE_ATTR_S *pstWDRExpAttr);
ISP_S32 XM_MPI_ISP_GetWDRExposureAttr(ISP_DEV IspDev, ISP_WDR_EXPOSURE_ATTR_S *pstWDRExpAttr);

ISP_S32 XM_MPI_ISP_SetAERouteAttr(ISP_DEV IspDev, const ISP_AE_ROUTE_S *pstAERouteAttr);
ISP_S32 XM_MPI_ISP_GetAERouteAttr(ISP_DEV IspDev, ISP_AE_ROUTE_S *pstAERouteAttr);

ISP_S32 XM_MPI_ISP_QueryInnerStateInfo(ISP_DEV IspDev, ISP_INNER_STATE_INFO_S *pstInnerStateInfo);

ISP_S32 XM_MPI_ISP_SetAICalibrate(ISP_DEV IspDev, const ISP_AI_CALIBRATE_S *pstAICalibrate);
ISP_S32 XM_MPI_ISP_GetAICalibrate(ISP_DEV IspDev, ISP_AI_CALIBRATE_S *pstAICalibrate);

ISP_S32 XM_MPI_ISP_SetIrisAttr(ISP_DEV IspDev, const ISP_IRIS_ATTR_S *pstIrisAttr);
ISP_S32 XM_MPI_ISP_GetIrisAttr(ISP_DEV IspDev, ISP_IRIS_ATTR_S *pstIrisAttr);


ISP_S32 XM_MPI_ISP_SetExpStaInfo(ISP_DEV IspDev, ISP_EXP_STA_INFO_S *pstExpStatistic);
ISP_S32 XM_MPI_ISP_GetExpStaInfo(ISP_DEV IspDev, ISP_EXP_STA_INFO_S *pstExpStatistic);

ISP_S32 XM_MPI_ISP_SetSlowFrameRate(ISP_DEV IspDev, ISP_U8 u8Value);
ISP_S32 XM_MPI_ISP_GetSlowFrameRate(ISP_DEV IspDev, ISP_U8 *pu8Value);


ISP_S32 XM_MPI_ISP_GetStatisticsAE(ISP_DEV IspDev, ISP_AE_STAT_S *pstAeStat);
ISP_S32 XM_MPI_ISP_GetStatisticsAEV2(ISP_DEV IspDev, ISP_AE_STAT_3_S *pstAeStatV2);

ISP_S32 XM_MPI_AE_SetFrameRate(ISP_DEV IspDev, ISP_U8 u8Fps);

//可调节光补光LED
ISP_S32 XM_MPI_AE_SetVarLEDAttr(ISP_DEV IspDev,const ISP_AE_VARLED_REGISTER_S *pstVarLED);
ISP_S32 XM_MPI_AE_GetVarLEDAttr(ISP_DEV IspDev, ISP_AE_VARLED_REGISTER_S *pstVarLED);

/************************************************************************************************************
函数功能: 配置FW的放大系数（降帧无法使用FH修改时---通过FW进行降帧）
输入参数:	 	IspDev: 指定对应isp ID
			u32AmpCoef: 放大系数(*16)
输出参数: 无
返回参数: 无
注:
	该放大倍数会同步到曝光统计中

************************************************************************************************************/
ISP_S32 XM_MPI_AE_SetAmpSlowFrameByFW(ISP_DEV IspDev, ISP_U32 u32AmpCoef);
ISP_S32 XM_MPI_AE_GetAmpSlowFrameByFW(ISP_DEV IspDev, ISP_U32 *pu32AmpCoef);


#endif

