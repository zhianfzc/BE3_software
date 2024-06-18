#ifndef __MPI_VDA_H__
#define __MPI_VDA_H__

#include "isp_comm_vda.h"

ISP_S32 XM_MPI_VDA_CreatChn(ISP_DEV IspDev);
ISP_S32 XM_MPI_VDA_DestroyChn(ISP_DEV IspDev);

ISP_S32 XM_MPI_VDA_SetAttrMD(ISP_DEV IspDev, VDA_MD_CHN_ATTR *pstMdAttr);
ISP_S32 XM_MPI_VDA_GetAttrMD(ISP_DEV IspDev, VDA_MD_CHN_ATTR *pstMdAttr);
ISP_S32 XM_MPI_VDA_GetResultMD(ISP_DEV IspDev, VDA_MD_RESULT* pstRslt);


ISP_S32 XM_MPI_VDA_SetAttrOD(ISP_DEV IspDev, VDA_OD_CHN_ATTR* pstAttr);
ISP_S32 XM_MPI_VDA_GetAttrOD(ISP_DEV IspDev, VDA_OD_CHN_ATTR* pstAttr);
ISP_S32 XM_MPI_VDA_GetResultOD(ISP_DEV IspDev, VDA_OD_RESULT *pstRslt);



#endif /* End of #ifndef __MPI_VDA_H__ */

