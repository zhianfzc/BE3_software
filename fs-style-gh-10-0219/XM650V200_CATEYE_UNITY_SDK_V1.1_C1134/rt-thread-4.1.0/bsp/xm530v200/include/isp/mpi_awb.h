#ifndef __MPI_AWB_H__
#define __MPI_AWB_H__

#include "isp_comm_isp.h"
#include "isp_comm_3a.h"
#include "isp_awb_comm.h"

ISP_S32 XM_MPI_AWB_GetInner(ISP_DEV IspDev, ISP_U32 *pu32Val);

/* The interface of awb lib register to isp. */
ISP_S32 XM_MPI_AWB_Register(ALG_LIB_S *pstAwbLib);
ISP_S32 XM_MPI_AWB_UnRegister(ALG_LIB_S *pstAwbLib);

/* The callback function of sensor register to awb lib. */
ISP_S32 XM_MPI_AWB_SensorRegCallBack(ISP_DEV IspDev,ALG_LIB_S *pstAwbLib, SENSOR_ID SensorId,
    AWB_SENSOR_REGISTER_S *pstRegister);
ISP_S32 XM_MPI_AWB_SensorUnRegCallBack(ISP_DEV IspDev,ALG_LIB_S *pstAwbLib, SENSOR_ID SensorId);

ISP_S32 XM_MPI_ISP_AWBLibRegCallBack(ISP_DEV IspDev, ALG_LIB_S *pstAWBLib,
        ISP_AWB_REGISTER_S *pstRegister);


ISP_S32 XM_MPI_ISP_SetWBAttr(ISP_DEV IspDev, const ISP_WB_ATTR_S *pstWBAttr);
ISP_S32 XM_MPI_ISP_GetWBAttr(ISP_DEV IspDev, ISP_WB_ATTR_S *pstWBAttr);

ISP_S32 XM_MPI_ISP_SetCCMAttr(ISP_DEV IspDev, const ISP_COLORMATRIX_ATTR_S *pstCCMAttr);
ISP_S32 XM_MPI_ISP_GetCCMAttr(ISP_DEV IspDev, ISP_COLORMATRIX_ATTR_S *pstCCMAttr);

ISP_S32 XM_MPI_ISP_QueryWBInfo(ISP_DEV IspDev, ISP_WB_INFO_S *pstWBInfo);
ISP_S32 XM_MPI_ISP_QueryWBInfo2(ISP_DEV IspDev, ISP_WB_INFO_S_V2 *pstWBInfo);
ISP_S32 XM_MPI_AWB_GetStatistics(ISP_DEV IspDev, ISP_WB_BAYER_STATISTICS_S *pstStat);

ISP_S32 XM_MPI_ISP_SetWBCalAttr(ISP_DEV IspDev, const ISP_AWB_CALIBRATION_V2_S *pstWBCalAttr);
ISP_S32 XM_MPI_ISP_GetWBCalAttr(ISP_DEV IspDev, ISP_AWB_CALIBRATION_V2_S *pstWBCalAttr);


#endif

