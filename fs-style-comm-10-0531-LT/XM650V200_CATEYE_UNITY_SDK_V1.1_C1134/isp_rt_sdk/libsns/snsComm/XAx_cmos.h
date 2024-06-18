#ifndef _XAX_CMOS_H_
#define _XAX_CMOS_H_
#include "isp_comm_sns.h"
#include "isp_math.h"
#include "mpi_ispapp.h"
#include "mpi_isp.h"
#include "mpi_phyvi.h"


extern ISP_CMOS_SNS_ATTR gstSnsAttr[ISP_NUM_MAX];

extern const ISP_CMOS_VPSS_3DNR_S gstVpss3DNr;
extern const ISP_CMOS_SHARPEN0_S gstSharpen0;
extern const ISP_CMOS_SHARPEN_S gstSharpenColor;
extern const ISP_CMOS_SHARPEN_S gstSharpenBw;
extern const ISP_CMOS_YCNR_S gstYCNr;
extern const ISP_CMOS_GAMMA_S gastIspGamma[6];
extern const ISP_CMOS_SHADING_S gstShading;
extern const ISP_CMOS_LOCAL_CAC_S gstLocalCAC;
extern const ISP_CMOS_LTM_S gstLtm;
extern const ISP_CMOS_LTM_S gstLtm_disable;
extern const ISP_CMOS_2DNR_S gst2DNr;

extern const ISP_U16 gau16GainTbl_SmatSns[64];
extern ISP_CMOS_DN_PARA_S gstCmosDnPara[ISP_NUM_MAX];
extern ISP_U32 gau32ShutNow[ISP_NUM_MAX];
extern ISP_U32 gau32ShutShortNow[ISP_NUM_MAX];
extern ISP_U32 gau32AGainNow[ISP_NUM_MAX];
extern ISP_U32 gau32DGainNow[ISP_NUM_MAX];
extern ISP_U8 gau8Fps[ISP_NUM_MAX];
extern ISP_U16 gau16FullLines[ISP_NUM_MAX];
extern ISP_U16 gau16HdrExpMax_L[ISP_NUM_MAX],gau16HdrExpMax_S[ISP_NUM_MAX];
extern WDR_MODE_E genHdrModeNow[ISP_NUM_MAX];

extern ISP_S32 cmos_init_sensor_exp_function_sc035GS(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc);
extern ISP_S32 cmos_init_ae_exp_function_sc035GS(AE_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern ISP_S32 cmos_init_awb_exp_function_sc035GS(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern void sensor_init_sc035GS(ISP_DEV IspDev);
extern ISP_S32 cmos_init_chn_sc035GS(ISP_DEV IspDev);

extern ISP_S32 cmos_init_sensor_exp_function_sc1346(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc);
extern ISP_S32 cmos_init_ae_exp_function_sc1346(AE_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern ISP_S32 cmos_init_awb_exp_function_sc1346(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern void sensor_init_sc1346(ISP_DEV IspDev);
extern ISP_S32 cmos_init_chn_sc1346(ISP_DEV IspDev);

extern ISP_S32 cmos_init_sensor_exp_function_imx307(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc);
extern ISP_S32 cmos_init_ae_exp_function_imx307(AE_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern ISP_S32 cmos_init_awb_exp_function_imx307(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern void sensor_init_imx307(ISP_DEV IspDev);
extern ISP_S32 cmos_init_chn_imx307(ISP_DEV IspDev);

extern ISP_S32 cmos_init_sensor_exp_function_sc2335(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc);
extern ISP_S32 cmos_init_ae_exp_function_sc2335(AE_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern ISP_S32 cmos_init_awb_exp_function_sc2335(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern void sensor_init_sc2335(ISP_DEV IspDev);
extern ISP_S32 cmos_init_chn_sc2335(ISP_DEV IspDev);

extern ISP_S32 cmos_init_sensor_exp_function_sc2336(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc);
extern ISP_S32 cmos_init_ae_exp_function_sc2336(AE_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern ISP_S32 cmos_init_awb_exp_function_sc2336(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern void sensor_init_sc2336(ISP_DEV IspDev);
extern ISP_S32 cmos_init_chn_sc2336(ISP_DEV IspDev);

extern ISP_S32 cmos_init_sensor_exp_function_sc223A(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc);
extern ISP_S32 cmos_init_ae_exp_function_sc223A(AE_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern ISP_S32 cmos_init_awb_exp_function_sc223A(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern void sensor_init_sc223A(ISP_DEV IspDev);
extern ISP_S32 cmos_init_chn_sc223A(ISP_DEV IspDev);

extern ISP_S32 cmos_init_sensor_exp_function_sc200AI(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc);
extern ISP_S32 cmos_init_ae_exp_function_sc200AI(AE_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern ISP_S32 cmos_init_awb_exp_function_sc200AI(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern void sensor_init_sc200AI(ISP_DEV IspDev);
extern ISP_S32 cmos_init_chn_sc200AI(ISP_DEV IspDev);

extern ISP_S32 cmos_init_sensor_exp_function_sc307E(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc);
extern ISP_S32 cmos_init_ae_exp_function_sc307E(AE_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern ISP_S32 cmos_init_awb_exp_function_sc307E(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern void sensor_init_sc307E(ISP_DEV IspDev);
extern ISP_S32 cmos_init_chn_sc307E(ISP_DEV IspDev);

extern ISP_S32 cmos_init_sensor_exp_function_sc202cs(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc);
extern ISP_S32 cmos_init_ae_exp_function_sc202cs(AE_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern ISP_S32 cmos_init_awb_exp_function_sc202cs(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern void sensor_init_sc202cs(ISP_DEV IspDev);
extern ISP_S32 cmos_init_chn_sc202cs(ISP_DEV IspDev);

extern ISP_S32 cmos_init_sensor_exp_function_imx335(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc);
extern ISP_S32 cmos_init_ae_exp_function_imx335(AE_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern ISP_S32 cmos_init_awb_exp_function_imx335(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern void sensor_init_imx335(ISP_DEV IspDev);
extern ISP_S32 cmos_init_chn_imx335(ISP_DEV IspDev);

extern ISP_S32 cmos_init_sensor_exp_function_sc3335(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc);
extern ISP_S32 cmos_init_ae_exp_function_sc3335(AE_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern ISP_S32 cmos_init_awb_exp_function_sc3335(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern void sensor_init_sc3335(ISP_DEV IspDev);
extern ISP_S32 cmos_init_chn_sc3335(ISP_DEV IspDev);

extern ISP_S32 cmos_init_sensor_exp_function_sc3338(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc);
extern ISP_S32 cmos_init_ae_exp_function_sc3338(AE_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern ISP_S32 cmos_init_awb_exp_function_sc3338(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern void sensor_init_sc3338(ISP_DEV IspDev);
extern ISP_S32 cmos_init_chn_sc3338(ISP_DEV IspDev);

extern ISP_S32 cmos_init_sensor_exp_function_sc401AI(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc);
extern ISP_S32 cmos_init_ae_exp_function_sc401AI(AE_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern ISP_S32 cmos_init_awb_exp_function_sc401AI(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern void sensor_init_sc401AI(ISP_DEV IspDev);
extern ISP_S32 cmos_init_chn_sc401AI(ISP_DEV IspDev);

extern ISP_S32 cmos_init_sensor_exp_function_sc4336(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc);
extern ISP_S32 cmos_init_ae_exp_function_sc4336(AE_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern ISP_S32 cmos_init_awb_exp_function_sc4336(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern void sensor_init_sc4336(ISP_DEV IspDev);
extern ISP_S32 cmos_init_chn_sc4336(ISP_DEV IspDev);

extern ISP_S32 cmos_init_sensor_exp_function_sc500AI(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc);
extern ISP_S32 cmos_init_ae_exp_function_sc500AI(AE_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern ISP_S32 cmos_init_awb_exp_function_sc500AI(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs);
extern void sensor_init_sc500AI(ISP_DEV IspDev);
extern ISP_S32 cmos_init_chn_sc500AI(ISP_DEV IspDev);

#endif

