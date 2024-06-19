#ifndef __ISP_AWB_COMM_H__
#define __ISP_AWB_COMM_H__

#include "isp_type.h"
#include "isp_comm_isp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define AWB_LIB_NAME "isp_awb_lib"

typedef struct stISP_AWB_CALIBRATION_V2_S
{
    ISP_S16 A[8];
    ISP_S16 B[8];
    ISP_S16 C[8];
    ISP_U16 key;
    ISP_U16 ini_x;
    ISP_U16 ini_y;
    ISP_U16 dis_min;
    ISP_U16 dis_max;
    ISP_U16 dis[16];	
    ISP_U16 val[16];	
    ISP_U16 init_gain[4];	
}ISP_AWB_CALIBRATION_V2_S;

typedef struct stAWB_SENSOR_DEFAULT_S
{
	ISP_COLORMATRIX_AUTO_S stCcm;
	const ISP_AWB_CALIBRATION_V2_S* pstAwbCal;
	ISP_AWBCCM_RSV_S stRsv;		// ±£¡Ù±∏”√
} AWB_SENSOR_DEFAULT_S;

typedef struct stAWB_SENSOR_EXP_FUNC_S
{
    ISP_S32(*pfn_cmos_get_awb_default)(ISP_DEV IspDev, AWB_SENSOR_DEFAULT_S *pstAwbSnsDft);
} AWB_SENSOR_EXP_FUNC_S;

typedef struct stAWB_SENSOR_REGISTER_S
{
    AWB_SENSOR_EXP_FUNC_S stSnsExp;
} AWB_SENSOR_REGISTER_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
