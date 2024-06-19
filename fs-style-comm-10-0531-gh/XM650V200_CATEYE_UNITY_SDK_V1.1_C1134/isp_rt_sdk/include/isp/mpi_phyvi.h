#ifndef __MPI_MIPI_H__
#define __MPI_MIPI_H__
#include "isp_common.h"
#include "isp_idefines.h"
#include "mpi_isp.h"





typedef struct stISPVI_ATTR_S
{
	ISP_BOOL bDataRev;			/* Data reverse */
	ISP_U8 u8HVRepair;			/* HV valid repair */
	ISP_RECT_S stDevRect;			/* Dev capture rect */
	WDR_MODE_E enHdrMode;
}ISPVI_ATTR_S;

typedef struct stMIPI_DEV_ATTR_S
{
	MIPI_LANE      enLane;                 /* MIPI lane num */
	MIPI_DEPTH     enDepth;          /* Depth: 8/10/12/ bit */
	WDR_MODE_E wdrMode;
	ISP_U32   u32MipiChn;
	ISP_U32   u32VichChnMask;
	ISP_U32   u32MipiClk;	//Mbps	
	ISP_U32   u32Version;	//0:V0  1:V1	
	ISP_U32   u32SnsAllLine;	//sensor输出总行数
	ISP_U32   u32SnsAllPixs;	//sensor输出总点数
	ISP_U32   u32SnsActiveLine;	//sensor输出有效行数
	ISP_U32   u32SnsActivePixs;	//sensor输出有效点数
	ISP_U32   u32SnsStartH;
	ISP_U32   u32SnsStartV;
	ISP_U32   u32Delay;		//mipi内部delay
	ISP_U32   u32MipiCtrl;	//[23:16]:ctrl_dly(检测 sot时间 delay:ctrl_dly*2);[7:0]:sot_data(B8);[8]:clk_ctrl(0:CKp;1:CKn) [15:9]:保留
}MIPI_DEV_ATTR_S;

typedef struct stMIPI_REPAIR_ATTR_S
{
	ISP_BOOL bEnable;
	ISP_U8 au8Rsv[4];
	ISP_U32 au32Rsv[4];
}MIPI_REPAIR_ATTR_S;

typedef struct stMIPI_CK_INFO
{
	unsigned int chn;				// bit[0~3]: IspDev   bit[4~7]:MipiChn
	unsigned int cnt;
	unsigned int totalsize_v;		// 总行数
	unsigned int rsv[4];
}MIPI_CK_INFO;


typedef struct stCOMBO_DEV_ATTR_S
{
	SENSOR_CONT input_mode;
	ISPVI_PHY_SEL phy_sel;
	MIPI_DEV_ATTR_S mipi_attr;
}COMBO_DEV_ATTR_S;

typedef enum enMIPI_ATTR_CMD
{
	MIPI_SET_DEV_ATTR = 0,
	MIPI_SET_OUTPUT_CLK_EDGE = 1,
	MIPI_RESET_MIPI = 2,
	MIPI_UNRESET_MIPI = 3,
	MIPI_SET_RUN = 4,
//	MIPI_RESET_SENSOR = 5,
//	MIPI_UNRESET_SENSOR = 6,
	MIPI_BUTT
}MIPI_ATTR_CMD;




ISP_S32 XM_MPI_MIPI_GetDevAttr(ISP_DEV IspDev, COMBO_DEV_ATTR_S *pstComboDevAttr);
ISP_S32 XM_MPI_MIPI_SetDevAttr(ISP_DEV IspDev, MIPI_ATTR_CMD enCmd, const COMBO_DEV_ATTR_S *pstComboDevAttr);

ISP_S32 XM_MPI_MIPI_RefreshFV(ISP_DEV IspDev, ISP_U32 u32DelayMs, ISP_U32 u32TotalSizeHV);

ISP_S32 XM_MPI_PHYVI_SetDevAttr(ISP_DEV IspDev, const ISPVI_ATTR_S *pstIspViAttr);
ISP_S32 XM_MPI_PHYVI_GetDevAttr(ISP_DEV IspDev, ISPVI_ATTR_S *pstIspViAttr);

ISP_S32 XM_MPI_MIPI_RepairTask(ISP_DEV IspDev, const ISP_U16 *pau16R,const ISP_U16 *pau16G,const ISP_U16 *pau16B);

ISP_S32 XM_MPI_MIPI_SetRepairAttr(ISP_DEV IspDev, const MIPI_REPAIR_ATTR_S *pstAttr);
ISP_S32 XM_MPI_MIPI_GetRepairAttr(ISP_DEV IspDev, MIPI_REPAIR_ATTR_S *pstAttr);


#endif /*__MPI_ISP_H__ */

	
