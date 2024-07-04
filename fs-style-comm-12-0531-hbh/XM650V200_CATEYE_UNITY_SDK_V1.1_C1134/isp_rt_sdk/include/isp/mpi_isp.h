#ifndef __MPI_ISP_H__
#define __MPI_ISP_H__

#include "isp_comm_isp.h"
#include "isp_comm_sns.h"
#include "isp_comm_3a.h"

ISP_S32 XM_MPI_ISP_Reset(ISP_DEV IspDev);
ISP_S32 XM_MPI_ISP_Init(ISP_DEV IspDev);
ISP_S32 XM_MPI_ISP_CommInit(ISP_COMM_ATTR_S *pstCommAttr);
ISP_S32 XM_MPI_ISP_GetCommAttr(ISP_COMM_ATTR_S *pstCommAttr);
ISP_S32 XM_MPI_ISP_MemInit(ISP_U32 u32ChnMask);
ISP_S32 XM_MPI_ISP_Run(ISP_DEV IspDev);
ISP_S32 XM_MPI_ISP_Exit(ISP_DEV IspDev);
ISP_S32 XM_MPI_ISP_SetVsyncIntEn(ISP_DEV IspDev, ISP_U8 u8Mode);

ISP_S32 XM_MPI_ISP_GetChnAttr(ISP_DEV IspDev,ISP_CHN_ATTR_S *pstChnAttr);
ISP_S32 XM_MPI_ISP_SetChnAttr(ISP_DEV IspDev,ISP_CHN_ATTR_S *pstChnAttr);

ISP_S32 XM_MPI_ISP_GetFrame(ISP_DEV IspDev,ISP_FRAME_DUMP_S *pstDumpInfo);
ISP_S32 XM_MPI_ISP_ReleaseFrame(ISP_DEV IspDev,ISP_FRAME_DUMP_S *pstDumpInfo);

ISP_S32 XM_MPI_ISP_SensorRegCallBack(ISP_DEV IspDev,SENSOR_ID SensorId, const ISP_SENSOR_REGISTER_S *pstRegister);
ISP_S32 XM_MPI_ISP_SensorUnRegCallBack(ISP_DEV IspDev);

ISP_S32 XM_MPI_ISP_SetModuleControl(ISP_DEV IspDev,const ISP_MODULE_CTRL_U *punModCtrl);
ISP_S32 XM_MPI_ISP_GetModuleControl(ISP_DEV IspDev,ISP_MODULE_CTRL_U *punModCtrl);

ISP_S32 XM_MPI_ISP_SetGammaAttr(ISP_DEV IspDev, const ISP_GAMMA_ATTR_S *pstGammaAttr);
ISP_S32 XM_MPI_ISP_GetGammaAttr(ISP_DEV IspDev, ISP_GAMMA_ATTR_S *pstGammaAttr);

ISP_S32 XM_MPI_ISP_SetBlackLevelAttr(ISP_DEV IspDev, const ISP_BLACKLVL_ATTR_S *pstBlackLevel);
ISP_S32 XM_MPI_ISP_GetBlackLevelAttr(ISP_DEV IspDev, ISP_BLACKLVL_ATTR_S *pstBlackLevel);

ISP_S32 XM_MPI_ISP_SetRegister(ISP_U32 u32Addr, ISP_U32 u32Value);
ISP_S32 XM_MPI_ISP_GetRegister(ISP_U32 u32Addr, ISP_U32 *pu32Value);

ISP_S32 XM_MPI_ISP_SetPubAttr(ISP_DEV IspDev, const ISP_PUB_ATTR_S *pstPubAttr);
ISP_S32 XM_MPI_ISP_GetPubAttr(ISP_DEV IspDev, ISP_PUB_ATTR_S *pstPubAttr);

ISP_S32 XM_MPI_ISP_SetSaturationAttr(ISP_DEV IspDev,const ISP_SATURATION_ATTR_S *pstSatAttr);
ISP_S32 XM_MPI_ISP_GetSaturationAttr(ISP_DEV IspDev,ISP_SATURATION_ATTR_S *pstSatAttr);

ISP_S32 XM_MPI_ISP_SetStDefectPixelAttr(ISP_DEV IspDev,ISP_STDPC_ATTR_S *pstStDPAttr);
ISP_S32 XM_MPI_ISP_GetStDefectPixelAttr(ISP_DEV IspDev,ISP_STDPC_ATTR_S *pstStDPAttr);

ISP_S32 XM_MPI_ISP_SetDyDefectPixelAttr(ISP_DEV IspDev,ISP_DYDPC_ATTR_S *pstDyDPAttr);
ISP_S32 XM_MPI_ISP_GetDyDefectPixelAttr(ISP_DEV IspDev,ISP_DYDPC_ATTR_S *pstDyDPAttr);

ISP_S32 XM_MPI_ISP_GetSharpenAttr(ISP_DEV IspDev, ISP_SHARPEN_ATTR_S *pstSharpenAttr);
ISP_S32 XM_MPI_ISP_SetSharpenAttr(ISP_DEV IspDev, const ISP_SHARPEN_ATTR_S *pstSharpenAttr);

ISP_S32 XM_MPI_ISP_SetNRAttr(ISP_DEV IspDev,const ISP_2DNR_ATTR_S *pstNRAttr);
ISP_S32 XM_MPI_ISP_GetNRAttr(ISP_DEV IspDev,ISP_2DNR_ATTR_S *pstNRAttr);

ISP_S32 XM_MPI_ISP_SetYcNRAttr(ISP_DEV IspDev, const ISP_YCNR_ATTR_S *pstNRAttr);
ISP_S32 XM_MPI_ISP_GetYcNRAttr(ISP_DEV IspDev, ISP_YCNR_ATTR_S *pstNRAttr);

ISP_S32 XM_MPI_ISP_SetBpu3DNrAttr(ISP_DEV IspDev,const BPU_3DNR_ATTR_S *pstNRAttr);
ISP_S32 XM_MPI_ISP_GetBpu3DNrAttr(ISP_DEV IspDev,BPU_3DNR_ATTR_S *pstNRAttr);

ISP_S32 XM_MPI_ISP_GetNrInfo(ISP_DEV IspDev, ISP_NR_INFO_S *pstNRInfo);

ISP_S32 XM_MPI_ISP_SetCSCAttr(ISP_DEV IspDev,const ISP_CSC_ATTR_S *pstCSCAttr);
ISP_S32 XM_MPI_ISP_GetCSCAttr(ISP_DEV IspDev,ISP_CSC_ATTR_S *pstCSCAttr);

ISP_S32 XM_MPI_ISP_SetAntiFalseColorAttr(ISP_DEV IspDev,const ISP_ANTI_FALSECOLOR_S *pstAntiFC);
ISP_S32 XM_MPI_ISP_GetAntiFalseColorAttr(ISP_DEV IspDev,ISP_ANTI_FALSECOLOR_S *pstAntiFC);

ISP_S32 XM_MPI_ISP_SetChromaAttr(ISP_DEV IspDev,ISP_CHROMA_ATTR_S *pstChromaAttr);
ISP_S32 XM_MPI_ISP_GetChromaAttr(ISP_DEV IspDev,ISP_CHROMA_ATTR_S *pstChromaAttr);

ISP_S32 XM_MPI_ISP_AWB_REFRESH(ISP_DEV IspDev);

ISP_S32 XM_MPI_ISP_StabStats(ISP_U32 *pu32AeErr);

ISP_S32 XM_MPI_ISP_SetHdrAttr(ISP_DEV IspDev, const ISP_HDR_ATTR_S *pstHdrDecAttr);
ISP_S32 XM_MPI_ISP_GetHdrAttr(ISP_DEV IspDev, ISP_HDR_ATTR_S *pstHdrDecAttr);

ISP_S32 XM_MPI_ISP_SetWDRMode(ISP_DEV IspDev, const ISP_WDR_MODE_S *pstWDRMode);
ISP_S32 XM_MPI_ISP_GetWDRMode(ISP_DEV IspDev, ISP_WDR_MODE_S *pstWDRMode);

ISP_S32 XM_MPI_ISP_SetDRCAttr(ISP_DEV IspDev, const ISP_DRC_ATTR_S *pstDRC);
ISP_S32 XM_MPI_ISP_GetDRCAttr(ISP_DEV IspDev, ISP_DRC_ATTR_S *pstDRC);

ISP_S32 XM_MPI_ISP_SetDeFogAttr(ISP_DEV IspDev, const ISP_DEFOG_ATTR_S *pstDefogAttr);
ISP_S32 XM_MPI_ISP_GetDeFogAttr(ISP_DEV IspDev, ISP_DEFOG_ATTR_S *pstDefogAttr);

ISP_S32 XM_MPI_ISP_SetVpssDeFogAttr(ISP_DEV IspDev, const ISP_DEFOG_ATTR_S *pstDefogAttr);
ISP_S32 XM_MPI_ISP_GetVpssDeFogAttr(ISP_DEV IspDev, ISP_DEFOG_ATTR_S *pstDefogAttr);

ISP_S32 XM_MPI_ISP_SetLtmAttr(ISP_DEV IspDev, const ISP_LTM_ATTR_S *pstLtmAttr);
ISP_S32 XM_MPI_ISP_GetLtmAttr(ISP_DEV IspDev, ISP_LTM_ATTR_S *pstLtmAttr);

ISP_S32 XM_MPI_ISP_SetVsyncCallback(ISP_DEV IspDev, ISP_VSYNC_CALBAK_S *pstVsyncCalBak);
ISP_S32 XM_MPI_ISP_GetVsyncCallback(ISP_DEV IspDev, ISP_VSYNC_CALBAK_S *pstVsyncCalBak);

ISP_S32 XM_MPI_ISP_SetFpnAttr(ISP_DEV IspDev,ISP_FPN_ATTR_S *pstFpnAttr);
ISP_S32 XM_MPI_ISP_GetFpnAttr(ISP_DEV IspDev,ISP_FPN_ATTR_S *pstFpnAttr);

ISP_VOID * XM_MPI_ISP_Malloc(ISP_S8 *ps8Name, ISP_U32 u32Bytes);
ISP_VOID XM_MPI_ISP_Free(ISP_S8 *ps8Name, ISP_VOID *pVirAddr);

ISP_VOID *XM_MPI_ISP_Mmap(ISP_U32 u32PhyAddr, ISP_U32 u32Size);
ISP_S32 XM_MPI_ISP_Munmap(ISP_VOID* pVirAddr, ISP_U32 u32Size);

ISP_S32 XM_MPI_ISP_Memncpy(ISP_VOID *pDst, ISP_VOID *pSrc, ISP_U32 u32Num);
ISP_VOID XM_MPI_ISP_Memset(ISP_VOID *pAddr,ISP_U8 u8Ch, ISP_U32 u32Num);
ISP_S32 XM_MPI_ISP_Memcmp(const ISP_VOID *buf1,  const ISP_VOID *buf2, ISP_S32 s32Count);

ISP_S32 XM_MPI_ISP_GetDCIAttr(ISP_DEV IspDev, ISP_DCI_ATTR_S *pstDciAttr);
ISP_S32 XM_MPI_ISP_SetDCIAttr(ISP_DEV IspDev, ISP_DCI_ATTR_S *pstDciAttr);

ISP_S32 XM_MPI_ISP_GetDspChip(ISP_U32 *pu32DspChip);

ISP_S32 XM_MPI_ISP_SetIspGainBase(ISP_U32 u32Base);
ISP_S32 XM_MPI_ISP_GetIspGainBase(ISP_U32 *pu32Base);

ISP_S32 XM_MPI_ISP_SetVpss3DNrAttr(ISP_DEV IspDev, const ISP_3DNR_ATTR_S *pstNr3D);
ISP_S32 XM_MPI_ISP_GetVpss3DNrAttr(ISP_DEV IspDev, ISP_3DNR_ATTR_S *pstNr3D);

ISP_S32 XM_MPI_ISP_Bind(const ISP_CHN_S *pstSrcChn, const ISP_CHN_S *pstDestChn);

ISP_S32 XM_MPI_ISP_SetDebug(ISP_DEV IspDev, const ISP_MODULE_DEBUG_U *pstDebug);
ISP_S32 XM_MPI_ISP_GetDebug(ISP_DEV IspDev,ISP_MODULE_DEBUG_U *pstDebug);

ISP_S32 XM_MPI_ISP_SetLimitUpAttr(ISP_DEV IspDev, const ISP_LIMIT_UP_ATTR_S *pstLimitUpAttr);
ISP_S32 XM_MPI_ISP_GetLimitUpAttr(ISP_DEV IspDev, ISP_LIMIT_UP_ATTR_S *pstLimitUpAttr);

ISP_S32 XM_MPI_ISP_SetLocalCacAttr(ISP_DEV IspDev, const ISP_LOCAL_CAC_ATTR_S *pstLocalCacAttr);
ISP_S32 XM_MPI_ISP_GetLocalCacAttr(ISP_DEV IspDev, ISP_LOCAL_CAC_ATTR_S *pstLocalCacAttr);

ISP_VOID XM_MPI_ISP_USleep(ISP_U32 u32Us);

ISP_S32 XM_MPI_ISP_SetShadingAttr(ISP_DEV IspDev, const ISP_SHADING_ATTR_S *pstShadingAttr);
ISP_S32 XM_MPI_ISP_GetShadingAttr(ISP_DEV IspDev, ISP_SHADING_ATTR_S *pstShadingAttr);

ISP_S32 XM_MPI_ISP_SetShadingCalibration(ISP_DEV IspDev, const ISP_SHADING_CALIBRATION_CFG_S* pstShadCaliCfg);
ISP_S32 XM_MPI_ISP_GetShadingCalibration(ISP_DEV IspDev, ISP_SHADING_CALIBRATION_CFG_S* pstLSCCaliCfg);


#endif /*__MPI_ISP_H__ */

