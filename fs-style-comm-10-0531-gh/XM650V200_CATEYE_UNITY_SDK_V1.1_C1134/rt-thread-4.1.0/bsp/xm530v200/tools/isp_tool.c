#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <pthread.h>
#include "isp_print.h"
#include "isp_math.h"
#include "mpi_awb.h"
#include "mpi_ae.h"
#include "mpi_isp.h"


static ISP_DEV gIspDev = 0;

#define IspBoolNum	(2)
static char IspBool[IspBoolNum][10] = {
	"Disable",
	"Enable",
};
#define IspTypeNum	(3)
static char IspType[IspTypeNum][10] = {
	"Auto",
	"Manual",
	"Key",
};
#define MenuNum	(24)
static char Menu[MenuNum][16] = {
	"set chn",
	"Exposure",
	"AeWeight(Null)",
	"Hdr(Null)",
	"AWB",
	"LDC(Null)",
	"CAC(Null)",
	"Gamma(Null)",
	"Lsc(Null)",
	"Sharpen",
	"CCM",
	"SaturationEx",
	"2D",
	"Ltm",
	"3D",
	"YcNr",
	"Drc",
	"Defog_Bpu",
	"Defog_Vpss",
	"Dpc",
	"AEStaten",
	"Blk",
	"Chroma",
	"Csc",
};

/************************************************************************
 * 读取输入参数
************************************************************************/
static int get_choose_int(ISP_U16 *sPrint)
{
	char str[256];
	ISP_U16 _buf[100]={0};
	int uswitch[5]={1,10,100,1000,10000};
	char *pstr1=str;
	char *pstr2=NULL;
	char *pstr3=NULL;
	ISP_U32 tmpbuff=0;
	int ucount=0;
	int icount;
	int isymbol;
	scanf("%s",str);
	while((*pstr1!='\0')&&(*pstr1!='\n'))
	{			
		while( (' '==*pstr1) || ('\t'==*pstr1) )  
			pstr1++; 	
		pstr2=pstr1;

		while((' '!=*pstr1) && ('\t'!=*pstr1) && ('\0'!=*pstr1)&&('\n'!=*pstr1))
			pstr1++;
		pstr3=pstr1;
		if(*pstr2=='-')
		{
			pstr2++;
			isymbol = -1;
		}
		else
		{
			isymbol = 1;
		}
		for(icount=0;;icount++)
		{
			pstr3--;
			if((*pstr3>0x39)||(*pstr3<0x30))
			{	
				if((*pstr3==' ')||(*pstr3=='\t'))
					break;
				else
				{			
					printf("Input error,please input '0~65535' type datas!\n");
					return -1;
				}
			}
			else
			{
				tmpbuff+=(*pstr3-0x030)*uswitch[icount];
				if(tmpbuff>65535)
				{
					break;
				}
			}
			if(pstr3==pstr2)
				break;
		}
		tmpbuff = tmpbuff*isymbol;
		_buf[ucount++]=(ISP_U16)tmpbuff;
		tmpbuff=0;
		if((*pstr1=='\n')||(*pstr1=='\0'))
			break;
	}
	memcpy(sPrint,&_buf,ucount*sizeof(ISP_U16));
	return ucount;
}
static ISP_S32 get_choose_char(void)
{
	ISP_U16	buf[100]={0};
	int s32Ret;
	s32Ret = get_choose_int(buf);
	if((s32Ret==-1)||(s32Ret==0))
		return -1;
	return buf[0];
}

/************************************************************************
 * 切换通道
************************************************************************/
static ISP_S32 dev_sel()
{
	ISP_S32 s32Val;
	printf("IspChn:[ %d ]\n",gIspDev);
	printf("IspChnSet: (0/1/2) >> ");
	s32Val = get_choose_char();
	printf("\n");
	if(s32Val>=0 && s32Val<4)
	{
		gIspDev = s32Val;
	}
	return 0;
}





/************************************************************************
 * 1.曝光菜单
************************************************************************/
#define ExposureNum	(5)
static char Exposure[ExposureNum][20] = {
	"Exp",
	"Enable/Disable",
	"Exp Mode",
	"Manual Thr",
	"Auto Thr",
};
#define ExposureManualNum	(10)
static char ExposureManual[ExposureManualNum][20] = {
	"AGain",
	"DGain",
	"ExpLine",
	"IspGain",
	"IspGain",
	"ExpLineEnable",
	"AGainEnable",
	"DGainEnable",
	"IspGainEnable",
	"ExpEnable",
};
#define ExposureAutoNum	(13)
static char ExposureAuto[ExposureAutoNum][20] = {
	"Time Max",
	"Time Min",
	"DGain Max",
	"DGain Min",
	"AGain Max",
	"AGain Min",
	"ISPDGain Max",
	"SysGain Max",
	"Speed",
	"Tolerance",
	"AEStrategyMode",
	"AEStrategyStr",
	"Compensation",
};
static ISP_S32 Exposure_menu_3th(ISP_S32 data)//曝光次级菜单
{
	//ISP_U8 u8i;
	ISP_S32 option,option2;
	ISP_S32 s32Ret;
	ISP_EXPOSURE_ATTR_S stExpAttr;

	s32Ret=XM_MPI_ISP_GetExposureAttr(gIspDev, &stExpAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_GetExposureAttr failed !\n");
		return ISP_FAILURE;
	}
	switch (data) 
	{
		case 1://使能
			printf("3th Menu( %s ):\n",Exposure[0]);
			printf("%s [%s]",Exposure[0],IspBool[stExpAttr.bByPass?0:1]);
			printf("%s (0:%s 1:%s)",Exposure[0],IspBool[1],IspBool[0]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=1)
				stExpAttr.bByPass = option;
			break;
		case 2://模式
			printf("3th Menu( %s ):\n",Exposure[0]);
			printf("%s [%s]",Exposure[0],IspType[stExpAttr.enOpType]);
			printf("%s (0:%s 1:%s 2:%s)",Exposure[0],IspType[0],IspType[1],IspType[2]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=2)
				stExpAttr.enOpType = option;
			break;
		case 3://手动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetExposureAttr(gIspDev, &stExpAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetExposureAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Exposure[0]);
				printf(" 1  %20s %d:\n",	ExposureManual[0],	stExpAttr.stManual.s32AGain				);
				printf(" 2  %20s %d:\n",	ExposureManual[1],	stExpAttr.stManual.s32DGain				);
				printf(" 3  %20s %d:\n",	ExposureManual[2],	stExpAttr.stManual.u32ExpLine			);
				printf(" 4  %20s %d:\n",	ExposureManual[3],	stExpAttr.stManual.s32IspGain			);
				printf(" 5  %20s %d:\n",	ExposureManual[4],	stExpAttr.stManual.u32Exp				);
				printf(" 6  %20s %d:\n",	ExposureManual[5],	stExpAttr.stManual.bManualExpLineEnable	);
				printf(" 7  %20s %d:\n",	ExposureManual[6],	stExpAttr.stManual.bManualAGainEnable	);
				printf(" 8  %20s %d:\n",	ExposureManual[7],	stExpAttr.stManual.bManualDGainEnable	);
				printf(" 9  %20s %d:\n",	ExposureManual[8],	stExpAttr.stManual.bManualIspGainEnable	);
				printf(" 10 %20s %d:\n",	ExposureManual[9],	stExpAttr.stManual.bManualExpEnable		);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=ExposureManualNum)
				{
					do {
						printf("%s>> ",ExposureManual[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 1:		stExpAttr.stManual.s32AGain				=	option2;break;
						case 2:		stExpAttr.stManual.s32DGain				=	option2;break;
						case 3:		stExpAttr.stManual.u32ExpLine			=	option2;break;
						case 4:		stExpAttr.stManual.s32IspGain			=	option2;break;
						case 5:		stExpAttr.stManual.u32Exp				=	option2;break;
						case 6:		stExpAttr.stManual.bManualExpLineEnable	=	option2;break;
						case 7:		stExpAttr.stManual.bManualAGainEnable	=	option2;break;
						case 8:		stExpAttr.stManual.bManualDGainEnable	=	option2;break;
						case 9:		stExpAttr.stManual.bManualIspGainEnable	=	option2;break;
						case 10:	stExpAttr.stManual.bManualExpEnable		=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetExposureAttr(gIspDev, &stExpAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetExposureAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
		case 4://自动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetExposureAttr(gIspDev, &stExpAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetExposureAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Exposure[0]);
				printf(" 1  %20s %d:\n",	ExposureAuto[0],	stExpAttr.stAuto.stExpTimeRange.u32Max	);
				printf(" 2  %20s %d:\n",	ExposureAuto[1],	stExpAttr.stAuto.stExpTimeRange.u32Min	);
				printf(" 3  %20s %d:\n",	ExposureAuto[2],	stExpAttr.stAuto.stAGainRange.u32Max	);
				printf(" 4  %20s %d:\n",	ExposureAuto[3],	stExpAttr.stAuto.stAGainRange.u32Min	);
				printf(" 5  %20s %d:\n",	ExposureAuto[4],	stExpAttr.stAuto.stDGainRange.u32Max	);
				printf(" 6  %20s %d:\n",	ExposureAuto[5],	stExpAttr.stAuto.stDGainRange.u32Min	);
				printf(" 7  %20s %d:\n",	ExposureAuto[6],	stExpAttr.stAuto.stISPDGainRange.u32Max	);
				printf(" 8  %20s %d:\n",	ExposureAuto[7],	stExpAttr.stAuto.stSysGainRange.u32Max	);
				printf(" 9  %20s %d:\n",	ExposureAuto[8],	stExpAttr.stAuto.u8Speed				);
				printf(" 10 %20s %d:\n",	ExposureAuto[9],	stExpAttr.stAuto.u8Tolerance			);
				printf(" 11 %20s %d:\n",	ExposureAuto[10],	stExpAttr.stAuto.enAEStrategyMode		);
				printf(" 12 %20s %d:\n",	ExposureAuto[11],	stExpAttr.stAuto.u16AEStrategyStr		);
				printf(" 13 %20s %d:\n",	ExposureAuto[12],	stExpAttr.stAuto.u8Compensation			);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=ExposureAutoNum)
				{
					do {
						printf("%s>> ",ExposureAuto[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 1:		stExpAttr.stAuto.stExpTimeRange.u32Max	=	option2;break;
						case 2:		stExpAttr.stAuto.stExpTimeRange.u32Min	=	option2;break;
						case 3:		stExpAttr.stAuto.stAGainRange.u32Max	=	option2;break;
						case 4:		stExpAttr.stAuto.stAGainRange.u32Min	=	option2;break;
						case 5:		stExpAttr.stAuto.stDGainRange.u32Max	=	option2;break;
						case 6:		stExpAttr.stAuto.stDGainRange.u32Min	=	option2;break;
						case 7:		stExpAttr.stAuto.stISPDGainRange.u32Max	=	option2;break;
						case 8:		stExpAttr.stAuto.stSysGainRange.u32Max	=	option2;break;
						case 9:		stExpAttr.stAuto.u8Speed				=	option2;break;
						case 10:	stExpAttr.stAuto.u8Tolerance			=	option2;break;
						case 11:	stExpAttr.stAuto.enAEStrategyMode		=	option2;break;
						case 12:	stExpAttr.stAuto.u16AEStrategyStr		=	option2;break;
						case 13:	stExpAttr.stAuto.u8Compensation			=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetExposureAttr(gIspDev, &stExpAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetExposureAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
	}
	s32Ret=XM_MPI_ISP_SetExposureAttr(gIspDev, &stExpAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_SetExposureAttr failed !\n");
		return ISP_FAILURE;
	}
	return ISP_SUCCESS;
}
static ISP_S32 Exposure_menu_2th(void)//曝光主菜单
{
	ISP_U8 u8i=0;
	ISP_S32 option;
	while(1)
	{
		printf("----------------------------------------\n");
		printf("2th Menu( %s ):\n",Exposure[0]);
		printf(" %2d %s\n",0,Menu[0]);
		for(u8i=1;u8i<ExposureNum;u8i++)
		{
			printf(" %2d %s\n",u8i,Exposure[u8i]);
		}
		printf(" 99. Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);

		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				Exposure_menu_3th(option);
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
	}
}

/************************************************************************
 * 4.白平衡菜单
************************************************************************/
#define AwbNum	(5)
static char Awb[AwbNum][20] = {
	"Awb",
	"Enable/Disable",
	"Awb Mode",
	"Manual Thr",
	"Auto Thr",
};
#define AwbManualNum	(3)
static char AwbManual[AwbManualNum][20] = {
	"Rgain",
	"Ggain",
	"Bgain",
};
#define AwbAutoNum	(7)
static char AwbAuto[AwbAutoNum][20] = {
	"SpeedToH",
	"SpeedToL",
	"Ofst(Rb)",
	"Ofst(R)",
	"Ofst(B)",
	"Ofst(GM)",
	"Zone",
};
static ISP_S32 Awb_menu_3th(ISP_S32 data)//白平衡次级菜单
{
	//ISP_U8 u8i;
	ISP_S32 option,option2;
	ISP_S32 s32Ret;
	ISP_WB_ATTR_S stWBAttr;

	s32Ret=XM_MPI_ISP_GetWBAttr(gIspDev, &stWBAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_GetWBAttr failed !\n");
		return ISP_FAILURE;
	}
	switch (data) 
	{
		case 1://使能
			printf("3th Menu( %s ):\n",Awb[0]);
			printf("%s [%s]",Awb[0],IspBool[stWBAttr.bByPass?0:1]);
			printf("%s (0:%s 1:%s)",Awb[0],IspBool[1],IspBool[0]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=1)
				stWBAttr.bByPass = option;
			break;
		case 2://模式
			printf("3th Menu( %s ):\n",Awb[0]);
			printf("%s [%s]",Awb[0],IspType[stWBAttr.enOpType]);
			printf("%s (0:%s 1:%s 2:%s)",Awb[0],IspType[0],IspType[1],IspType[2]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=2)
				stWBAttr.enOpType = option;
			break;
		case 3://手动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetWBAttr(gIspDev, &stWBAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetWBAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Awb[0]);
				printf(" 1  %20s %d:\n",	AwbManual[0],	stWBAttr.stManual.u16Rgain	);
				printf(" 2  %20s %d:\n",	AwbManual[1],	stWBAttr.stManual.u16Ggain	);
				printf(" 3  %20s %d:\n",	AwbManual[2],	stWBAttr.stManual.u16Bgain	);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=AwbManualNum)
				{
					do {
						printf("%s>> ",AwbManual[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 1:		stWBAttr.stManual.u16Rgain		=	option2;break;
						case 2:		stWBAttr.stManual.u16Ggain		=	option2;break;
						case 3:		stWBAttr.stManual.u16Bgain		=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetWBAttr(gIspDev, &stWBAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetWBAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
		case 4://自动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetWBAttr(gIspDev, &stWBAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetWBAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Awb[0]);
				printf(" 1  %20s %d:\n",	AwbAuto[0],	stWBAttr.stAuto.u16SpeedToH	);
				printf(" 2  %20s %d:\n",	AwbAuto[1],	stWBAttr.stAuto.u16SpeedToL	);
				printf(" 3  %20s %d:\n",	AwbAuto[2],	stWBAttr.stAuto.u8RbOfst	);
				printf(" 4  %20s %d:\n",	AwbAuto[3],	stWBAttr.stAuto.u8ROfst		);
				printf(" 5  %20s %d:\n",	AwbAuto[4],	stWBAttr.stAuto.u8BOfst		);
				printf(" 6  %20s %d:\n",	AwbAuto[5],	stWBAttr.stAuto.u8GmOfst	);
				printf(" 7  %20s %d:\n",	AwbAuto[6],	stWBAttr.stAuto.u8ZoneSel	);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=AwbAutoNum)
				{
					do {
						printf("%s>> ",AwbAuto[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 1:		stWBAttr.stAuto.u16SpeedToH		=	option2;break;
						case 2:		stWBAttr.stAuto.u16SpeedToL		=	option2;break;
						case 3:		stWBAttr.stAuto.u8RbOfst		=	option2;break;
						case 4:		stWBAttr.stAuto.u8ROfst			=	option2;break;
						case 5:		stWBAttr.stAuto.u8BOfst			=	option2;break;
						case 6:		stWBAttr.stAuto.u8GmOfst		=	option2;break;
						case 7:		stWBAttr.stAuto.u8ZoneSel		=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetWBAttr(gIspDev, &stWBAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetWBAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
	}
	s32Ret=XM_MPI_ISP_SetWBAttr(gIspDev, &stWBAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_SetWBAttr failed !\n");
		return ISP_FAILURE;
	}
	return ISP_SUCCESS;
}
static ISP_S32 Awb_menu_2th(void)//白平衡主菜单
{
	ISP_U8 u8i=0;
	ISP_S32 option;
	while(1)
	{
		printf("----------------------------------------\n");
		printf("2th Menu( %s ):\n",Awb[0]);
		printf(" %2d %s\n",0,Menu[0]);
		for(u8i=1;u8i<AwbNum;u8i++)
		{
			printf(" %2d %s\n",u8i,Awb[u8i]);
		}
		printf(" 99. Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);

		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				Awb_menu_3th(option);
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
	}
}

#if 0
/************************************************************************
 * 5.LDC菜单
************************************************************************/
#define LdcNum	(3)
static char Ldc[LdcNum][20] = {
	"Ldc",
	"Enable/Disable",
	"Auto Thr",
};
#define LdcAutoNum	(7)
static char LdcAuto[LdcAutoNum][20] = {
	"u32CenterX",
	"u32CenterY",
	"s32Dr",
	"s32D",
	"s32R",
	"u32Width",
	"u32Height",
};
static ISP_S32 Ldc_menu_3th(ISP_S32 data)//LDC次级菜单
{
	//ISP_U8 u8i;
	ISP_S32 option,option2;
	ISP_S32 s32Ret;
	ISP_LDC_ATTR_S stLdcAttr;

	s32Ret=XM_MPI_ISP_GetLDCAttr(gIspDev, &stLdcAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_GetLDCAttr failed !\n");
		return ISP_FAILURE;
	}
	switch (data) 
	{
		case 1://使能
			printf("3th Menu( %s ):\n",Ldc[0]);
			printf("%s [%s]",Ldc[0],IspBool[stLdcAttr.bEnable]);
			printf("%s (0:%s 1:%s)",Ldc[0],IspBool[0],IspBool[1]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=1)
				stLdcAttr.bEnable = option;
			break;
		case 2://自动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetLDCAttr(gIspDev, &stLdcAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetLDCAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Ldc[0]);
				printf(" 1  %20s %d:\n",	LdcAuto[0],		stLdcAttr.u32CenterX	);
				printf(" 2  %20s %d:\n",	LdcAuto[1],		stLdcAttr.u32CenterY	);
				printf(" 3  %20s %d:\n",	LdcAuto[2],		stLdcAttr.s32Dr			);
				printf(" 4  %20s %d:\n",	LdcAuto[3],		stLdcAttr.s32D			);
				printf(" 5  %20s %d:\n",	LdcAuto[4],		stLdcAttr.s32R			);
				printf(" 6  %20s %d:\n",	LdcAuto[5],		stLdcAttr.u32Width		);
				printf(" 7  %20s %d:\n",	LdcAuto[6],		stLdcAttr.u32Height		);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=LdcAutoNum)
				{
					do {
						printf("%s>> ",LdcAuto[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 1:		stLdcAttr.u32CenterX	=	option2;break;
						case 2:		stLdcAttr.u32CenterY	=	option2;break;
						case 3:		stLdcAttr.s32Dr			=	option2;break;
						case 4:		stLdcAttr.s32D			=	option2;break;
						case 5:		stLdcAttr.s32R			=	option2;break;
						case 6:		stLdcAttr.u32Width		=	option2;break;
						case 7:		stLdcAttr.u32Height		=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetLDCAttr(gIspDev, &stLdcAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetLDCAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
	}
	s32Ret=XM_MPI_ISP_SetLDCAttr(gIspDev, &stLdcAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_SetLDCAttr failed !\n");
		return ISP_FAILURE;
	}
	return ISP_SUCCESS;
}
static ISP_S32 Ldc_menu_2th(void)//LDC主菜单
{
	ISP_U8 u8i=0;
	ISP_S32 option;
	while(1)
	{
		printf("----------------------------------------\n");
		printf("2th Menu( %s ):\n",Ldc[0]);
		printf(" %2d %s\n",0,Menu[0]);
		for(u8i=1;u8i<LdcNum;u8i++)
		{
			printf(" %2d %s\n",u8i,Ldc[u8i]);
		}
		printf(" 99. Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);

		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
			case 2:
				Ldc_menu_3th(option);
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
	}
}
#endif

/************************************************************************
 * 8.Lsc菜单
************************************************************************/
#define LscNum	(6)
static char Lsc[LscNum][20] = {
	"Lsc",
	"Enable/Disable",
	"Lsc Mode",
	"GlobalSth",
	"Manual Thr",
	"Auto Thr",
};
#define LscManualNum	(1)
static char LscManual[LscManualNum][20] = {
	"Str",
};
#define LscAutoNum	(1)
static char LscAuto[LscAutoNum][20] = {
	"Str",
};
static ISP_S32 Lsc_menu_3th(ISP_S32 data)//Lsc次级菜单
{
	ISP_U8 u8i,u8j;
	ISP_S32 option,option2,option3;
	ISP_S32 s32Ret;
	ISP_SHADING_ATTR_S stShadingAttr;

	s32Ret=XM_MPI_ISP_GetShadingAttr(gIspDev, &stShadingAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_GetShadingAttr failed !\n");
		return ISP_FAILURE;
	}
	switch (data) 
	{
		case 1://使能
			printf("3th Menu( %s ):\n",Lsc[0]);
			printf("%s [%s]",Lsc[0],IspBool[stShadingAttr.bEnable]);
			printf("%s (0:%s 1:%s)",Lsc[0],IspBool[0],IspBool[1]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=1)
				stShadingAttr.bEnable = option;
			break;
		case 2://模式
			printf("3th Menu( %s ):\n",Lsc[0]);
			printf("%s [%s]",Lsc[0],IspType[stShadingAttr.enOpType]);
			printf("%s (0:%s 1:%s 2:%s)",Lsc[0],IspType[0],IspType[1],IspType[2]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=2)
				stShadingAttr.enOpType = option;
			break;
		case 3://全局强度
			printf("3th Menu( %s ):\n",Lsc[0]);
			printf("%s [%d]",Lsc[data],stShadingAttr.stAuto.u8GlobalStr);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=255)
				stShadingAttr.stAuto.u8GlobalStr = option;
			break;
		case 4://手动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetShadingAttr(gIspDev, &stShadingAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetShadingAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Lsc[0]);
				printf(" 1  %20s %d:\n",	LscManual[0],	stShadingAttr.stManual.u8Str			);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=LscManualNum)
				{
					do {
						printf("%s>> ",LscManual[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 1:		stShadingAttr.stManual.u8Str			=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetShadingAttr(gIspDev, &stShadingAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetShadingAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
		case 5://自动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetShadingAttr(gIspDev, &stShadingAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetShadingAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Lsc[0]);
				for(u8i=0;u8i<LscAutoNum;u8i++)
				{
					printf(" %2d  %20s ",	u8i+1,	LscAuto[u8i]);
					for(u8j=0;u8j<ISP_AUTO_STENGTH_NUM;u8j++)
					{
						switch(u8i)
						{
							case 0:		printf("[%5d] ",stShadingAttr.stAuto.au8Str[u8j]				);break;
						}
					}
					printf("\n");
				}
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;

				if(option>=1 && option<=LscAutoNum)
				{
					do {
						printf("%s>> ",LscAuto[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					if(option2>=0 && option2<ISP_AUTO_STENGTH_NUM)
					{
						do {
							printf("%s[%d]>> ",LscAuto[option-1],option2);
							option3 = get_choose_char();
							printf("\n");
						} while (option3 < 0);
						if(option3>=0 && option3<=255)
						{
							switch(option)
							{
								case 1:		stShadingAttr.stAuto.au8Str[option2]			=	option3;break;
							}
							s32Ret=XM_MPI_ISP_SetShadingAttr(gIspDev, &stShadingAttr);
							if(s32Ret != ISP_SUCCESS)
							{
								ERR("XM_MPI_ISP_SetShadingAttr failed !\n");
								return ISP_FAILURE;
							}
						}
					}
				}
			}
			break;
	}
	s32Ret=XM_MPI_ISP_SetShadingAttr(gIspDev, &stShadingAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_SetShadingAttr failed !\n");
		return ISP_FAILURE;
	}
	return ISP_SUCCESS;
}
static ISP_S32 Lsc_menu_2th(void)//Lsc主菜单
{
	ISP_U8 u8i=0;
	ISP_S32 option;
	while(1)
	{
		printf("----------------------------------------\n");
		printf("2th Menu( %s ):\n",Lsc[0]);
		printf(" %2d %s\n",0,Menu[0]);
		for(u8i=1;u8i<LscNum;u8i++)
		{
			printf(" %2d %s\n",u8i,Lsc[u8i]);
		}
		printf(" 99. Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);

		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
				Lsc_menu_3th(option);
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
	}
}

/************************************************************************
 * 9.锐化菜单
************************************************************************/
#define SharpenNum	(6)
static char Sharpen[SharpenNum][20] = {
	"Sharpen",
	"Enable/Disable",
	"Sharpen Mode",
	"GlobalSth",
	"Manual Thr",
	"Auto Thr",
};
#define SharpenManualNum	(13)
static char SharpenManual[SharpenManualNum][20] = {
	"Ud",
	"D",
	"ThrUd",
	"ThrD",
	"Ud_H",
	"D_H",
	"ThrUd_H",
	"ThrD_H",
	"EdgeStr",
	"EdgeThr",
	"OverShoot",
	"UnderShoot",
	"DetailStr",
};
#define SharpenAutoNum	(13)
static char SharpenAuto[SharpenAutoNum][20] = {
	"Ud",
	"D",
	"ThrUd",
	"ThrD",
	"Ud_H",
	"D_H",
	"ThrUd_H",
	"ThrD_H",
	"EdgeStr",
	"EdgeThr",
	"OverShoot",
	"UnderShoot",
	"DetailStr",
};
static ISP_S32 Sharpen_menu_3th(ISP_S32 data)//锐化次级菜单
{
	ISP_U8 u8i,u8j;
	ISP_S32 option,option2,option3;
	ISP_S32 s32Ret;
	ISP_SHARPEN_ATTR_S stSharpenAttr;

	s32Ret=XM_MPI_ISP_GetSharpenAttr(gIspDev, &stSharpenAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_GetSharpenAttr failed !\n");
		return ISP_FAILURE;
	}
	switch (data) 
	{
		case 1://使能
			printf("3th Menu( %s ):\n",Sharpen[0]);
			printf("%s [%s]",Sharpen[0],IspBool[stSharpenAttr.bEnable]);
			printf("%s (0:%s 1:%s)",Sharpen[0],IspBool[0],IspBool[1]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=1)
				stSharpenAttr.bEnable = option;
			break;
		case 2://模式
			printf("3th Menu( %s ):\n",Sharpen[0]);
			printf("%s [%s]",Sharpen[0],IspType[stSharpenAttr.enOpType]);
			printf("%s (0:%s 1:%s 2:%s)",Sharpen[0],IspType[0],IspType[1],IspType[2]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=2)
				stSharpenAttr.enOpType = option;
			break;
		case 3://全局强度
			printf("3th Menu( %s ):\n",Sharpen[0]);
			printf("%s [%d]",Sharpen[data],stSharpenAttr.u8GlobalSth);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=255)
				stSharpenAttr.u8GlobalSth = option;
			break;
		case 4://手动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetSharpenAttr(gIspDev, &stSharpenAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetSharpenAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Sharpen[0]);
				printf(" 1  %20s %d:\n",	SharpenManual[0],	stSharpenAttr.stManual.u8Ud			);
				printf(" 2  %20s %d:\n",	SharpenManual[1],	stSharpenAttr.stManual.u8D			);
				printf(" 3  %20s %d:\n",	SharpenManual[2],	stSharpenAttr.stManual.u8ThrUd		);
				printf(" 4  %20s %d:\n",	SharpenManual[3],	stSharpenAttr.stManual.u8ThrD		);
				printf(" 5  %20s %d:\n",	SharpenManual[4],	stSharpenAttr.stManual.u8Ud_H		);
				printf(" 6  %20s %d:\n",	SharpenManual[5],	stSharpenAttr.stManual.u8D_H		);
				printf(" 7  %20s %d:\n",	SharpenManual[6],	stSharpenAttr.stManual.u8ThrUd_H	);
				printf(" 8  %20s %d:\n",	SharpenManual[7],	stSharpenAttr.stManual.u8ThrD_H		);
				printf(" 9  %20s %d:\n",	SharpenManual[8],	stSharpenAttr.stManual.u8EdgeStr	);
				printf(" 10 %20s %d:\n",	SharpenManual[9],	stSharpenAttr.stManual.u8EdgeThr	);
				printf(" 11 %20s %d:\n",	SharpenManual[10],	stSharpenAttr.stManual.u8OverShoot	);
				printf(" 12 %20s %d:\n",	SharpenManual[11],	stSharpenAttr.stManual.u8UnderShoot	);
				printf(" 13 %20s %d:\n",	SharpenManual[12],	stSharpenAttr.stManual.u8DetailStr	);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=SharpenManualNum)
				{
					do {
						printf("%s>> ",SharpenManual[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 1:		stSharpenAttr.stManual.u8Ud			=	option2;break;
						case 2:		stSharpenAttr.stManual.u8D			=	option2;break;
						case 3:		stSharpenAttr.stManual.u8ThrUd		=	option2;break;
						case 4:		stSharpenAttr.stManual.u8ThrD		=	option2;break;
						case 5:		stSharpenAttr.stManual.u8Ud_H		=	option2;break;
						case 6:		stSharpenAttr.stManual.u8D_H		=	option2;break;
						case 7:		stSharpenAttr.stManual.u8ThrUd_H	=	option2;break;
						case 8:		stSharpenAttr.stManual.u8ThrD_H		=	option2;break;
						case 9:		stSharpenAttr.stManual.u8EdgeStr	=	option2;break;
						case 10:	stSharpenAttr.stManual.u8EdgeThr	=	option2;break;
						case 11:	stSharpenAttr.stManual.u8OverShoot	=	option2;break;
						case 12:	stSharpenAttr.stManual.u8UnderShoot	=	option2;break;
						case 13:	stSharpenAttr.stManual.u8DetailStr	=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetSharpenAttr(gIspDev, &stSharpenAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetSharpenAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
		case 5://自动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetSharpenAttr(gIspDev, &stSharpenAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetSharpenAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Sharpen[0]);
				for(u8i=0;u8i<SharpenAutoNum;u8i++)
				{
					printf(" %2d  %20s ",	u8i+1,	SharpenAuto[u8i]);
					for(u8j=0;u8j<ISP_AUTO_STENGTH_NUM;u8j++)
					{
						switch(u8i)
						{
							case 0:		printf("[%5d] ",stSharpenAttr.stAuto.au8Ud[u8j]				);break;
							case 1:		printf("[%5d] ",stSharpenAttr.stAuto.au8D[u8j]				);break;
							case 2:		printf("[%5d] ",stSharpenAttr.stAuto.au8ThrUd[u8j]			);break;
							case 3:		printf("[%5d] ",stSharpenAttr.stAuto.au8ThrD[u8j]			);break;
							case 4:		printf("[%5d] ",stSharpenAttr.stAuto.au8Ud_H[u8j]			);break;
							case 5:		printf("[%5d] ",stSharpenAttr.stAuto.au8D_H[u8j]			);break;
							case 6:		printf("[%5d] ",stSharpenAttr.stAuto.au8ThrUd_H[u8j]		);break;
							case 7:		printf("[%5d] ",stSharpenAttr.stAuto.au8ThrD_H[u8j]			);break;
							case 8:		printf("[%5d] ",stSharpenAttr.stAuto.au8EdgeStr[u8j]		);break;
							case 9:		printf("[%5d] ",stSharpenAttr.stAuto.au8EdgeThr[u8j]		);break;
							case 10:	printf("[%5d] ",stSharpenAttr.stAuto.au8OverShoot[u8j]		);break;
							case 11:	printf("[%5d] ",stSharpenAttr.stAuto.au8UnderShoot[u8j]		);break;
							case 12:	printf("[%5d] ",stSharpenAttr.stAuto.au8DetailStr[u8j]		);break;
						}
					}
					printf("\n");
				}
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;

				if(option>=1 && option<=SharpenAutoNum)
				{
					do {
						printf("%s>> ",SharpenAuto[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					if(option2>=0 && option2<ISP_AUTO_STENGTH_NUM)
					{
						do {
							printf("%s[%d]>> ",SharpenAuto[option-1],option2);
							option3 = get_choose_char();
							printf("\n");
						} while (option3 < 0);
						if(option3>=0 && option3<=255)
						{
							switch(option)
							{
								case 1:		stSharpenAttr.stAuto.au8Ud[option2]			=	option3;break;
								case 2:		stSharpenAttr.stAuto.au8D[option2]			=	option3;break;
								case 3:		stSharpenAttr.stAuto.au8ThrUd[option2]		=	option3;break;
								case 4:		stSharpenAttr.stAuto.au8ThrD[option2]		=	option3;break;
								case 5:		stSharpenAttr.stAuto.au8Ud_H[option2]		=	option3;break;
								case 6:		stSharpenAttr.stAuto.au8D_H[option2]		=	option3;break;
								case 7:		stSharpenAttr.stAuto.au8ThrUd_H[option2]	=	option3;break;
								case 8:		stSharpenAttr.stAuto.au8ThrD_H[option2]		=	option3;break;
								case 9:		stSharpenAttr.stAuto.au8EdgeStr[option2]	=	option3;break;
								case 10:	stSharpenAttr.stAuto.au8EdgeThr[option2]	=	option3;break;
								case 11:	stSharpenAttr.stAuto.au8OverShoot[option2]	=	option3;break;
								case 12:	stSharpenAttr.stAuto.au8UnderShoot[option2]	=	option3;break;
								case 13:	stSharpenAttr.stAuto.au8DetailStr[option2]	=	option3;break;
							}
							s32Ret=XM_MPI_ISP_SetSharpenAttr(gIspDev, &stSharpenAttr);
							if(s32Ret != ISP_SUCCESS)
							{
								ERR("XM_MPI_ISP_SetSharpenAttr failed !\n");
								return ISP_FAILURE;
							}
						}
					}
				}
			}
			break;
	}
	s32Ret=XM_MPI_ISP_SetSharpenAttr(gIspDev, &stSharpenAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_SetSharpenAttr failed !\n");
		return ISP_FAILURE;
	}
	return ISP_SUCCESS;
}
static ISP_S32 Sharpen_menu_2th(void)//锐化主菜单
{
	ISP_U8 u8i=0;
	ISP_S32 option;
	while(1)
	{
		printf("----------------------------------------\n");
		printf("2th Menu( %s ):\n",Sharpen[0]);
		printf(" %2d %s\n",0,Menu[0]);
		for(u8i=1;u8i<SharpenNum;u8i++)
		{
			printf(" %2d %s\n",u8i,Sharpen[u8i]);
		}
		printf(" 99. Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);

		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
				Sharpen_menu_3th(option);
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
	}
}

/************************************************************************
 * 10.CCM菜单
************************************************************************/
#define CcmNum	(5)
static char Ccm[CcmNum][20] = {
	"Ccm",
	"Enable/Disable",
	"Ccm Mode",
	"Manual Thr",
	"Auto Thr",
};
static ISP_S32 Ccm_menu_3th(ISP_S32 data)//CCM次级菜单
{
	//ISP_U8 u8i;
	ISP_S32 option,option2;
	ISP_S32 s32Ret;
	ISP_COLORMATRIX_ATTR_S stCCMAttr;

	s32Ret=XM_MPI_ISP_GetCCMAttr(gIspDev, &stCCMAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_GetCCMAttr failed !\n");
		return ISP_FAILURE;
	}
	switch (data) 
	{
		case 1://使能
			printf("3th Menu( %s ):\n",Ccm[0]);
			printf("%s [%s]",Ccm[0],IspBool[stCCMAttr.bByPass?0:1]);
			printf("%s (0:%s 1:%s)",Ccm[0],IspBool[1],IspBool[0]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=1)
				stCCMAttr.bByPass = option;
			break;
		case 2://模式
			printf("3th Menu( %s ):\n",Ccm[0]);
			printf("%s [%s]",Ccm[0],IspType[stCCMAttr.enOpType]);
			printf("%s (0:%s 1:%s 2:%s)",Ccm[0],IspType[0],IspType[1],IspType[2]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=2)
				stCCMAttr.enOpType = option;
			break;
		case 3://手动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetCCMAttr(gIspDev, &stCCMAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetCCMAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Ccm[0]);
				printf("1[%5d] 2[%5d] 3[%5d]\n",stCCMAttr.stManual.as16CCM[1],stCCMAttr.stManual.as16CCM[2],stCCMAttr.stManual.as16CCM[3]);
				printf("4[%5d] 5[%5d] 6[%5d]\n",stCCMAttr.stManual.as16CCM[5],stCCMAttr.stManual.as16CCM[6],stCCMAttr.stManual.as16CCM[7]);
				printf("7[%5d] 8[%5d] 9[%5d]\n",stCCMAttr.stManual.as16CCM[9],stCCMAttr.stManual.as16CCM[10],stCCMAttr.stManual.as16CCM[11]);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=9)
				{
					do {
						printf(">> ");
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 1:		stCCMAttr.stManual.as16CCM[1]	=	option2;break;
						case 2:		stCCMAttr.stManual.as16CCM[2]	=	option2;break;
						case 3:		stCCMAttr.stManual.as16CCM[3]	=	option2;break;
						case 4:		stCCMAttr.stManual.as16CCM[5]	=	option2;break;
						case 5:		stCCMAttr.stManual.as16CCM[6]	=	option2;break;
						case 6:		stCCMAttr.stManual.as16CCM[7]	=	option2;break;
						case 7:		stCCMAttr.stManual.as16CCM[9]	=	option2;break;
						case 8:		stCCMAttr.stManual.as16CCM[10]	=	option2;break;
						case 9:		stCCMAttr.stManual.as16CCM[11]	=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetCCMAttr(gIspDev, &stCCMAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetCCMAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
		case 4://自动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetCCMAttr(gIspDev, &stCCMAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_SetCCMAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Ccm[0]);
				printf(" 0[%5d]\n",stCCMAttr.stAuto.u16HighColorTemp);
				printf(" 1[%5d]  2[%5d]  3[%5d]  [%5d]\n",stCCMAttr.stAuto.as16HighCCM[1],stCCMAttr.stAuto.as16HighCCM[2],stCCMAttr.stAuto.as16HighCCM[3],stCCMAttr.stAuto.as16HighCCM[0]);
				printf(" 4[%5d]  5[%5d]  6[%5d]  [%5d]\n",stCCMAttr.stAuto.as16HighCCM[5],stCCMAttr.stAuto.as16HighCCM[6],stCCMAttr.stAuto.as16HighCCM[7],stCCMAttr.stAuto.as16HighCCM[4]);
				printf(" 7[%5d]  8[%5d]  9[%5d]  [%5d]\n",stCCMAttr.stAuto.as16HighCCM[9],stCCMAttr.stAuto.as16HighCCM[10],stCCMAttr.stAuto.as16HighCCM[11],stCCMAttr.stAuto.as16HighCCM[8]);
				printf("10[%5d]\n",stCCMAttr.stAuto.u16MidColorTemp);
				printf("11[%5d] 12[%5d] 13[%5d]\n",stCCMAttr.stAuto.as16MidCCM[1],stCCMAttr.stAuto.as16MidCCM[2],stCCMAttr.stAuto.as16MidCCM[3]);
				printf("14[%5d] 15[%5d] 16[%5d]\n",stCCMAttr.stAuto.as16MidCCM[5],stCCMAttr.stAuto.as16MidCCM[6],stCCMAttr.stAuto.as16MidCCM[7]);
				printf("17[%5d] 18[%5d] 19[%5d]\n",stCCMAttr.stAuto.as16MidCCM[9],stCCMAttr.stAuto.as16MidCCM[10],stCCMAttr.stAuto.as16MidCCM[11]);
				printf("20[%5d]\n",stCCMAttr.stAuto.u16LowColorTemp);
				printf("21[%5d] 22[%5d] 23[%5d]\n",stCCMAttr.stAuto.as16LowCCM[1],stCCMAttr.stAuto.as16LowCCM[2],stCCMAttr.stAuto.as16LowCCM[3]);
				printf("24[%5d] 25[%5d] 26[%5d]\n",stCCMAttr.stAuto.as16LowCCM[5],stCCMAttr.stAuto.as16LowCCM[6],stCCMAttr.stAuto.as16LowCCM[7]);
				printf("27[%5d] 28[%5d] 29[%5d]\n",stCCMAttr.stAuto.as16LowCCM[9],stCCMAttr.stAuto.as16LowCCM[10],stCCMAttr.stAuto.as16LowCCM[11]);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=0 && option<=29)
				{
					do {
						printf(">> ");
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 0:		stCCMAttr.stAuto.u16HighColorTemp		=	option2;break;
						case 1:		stCCMAttr.stAuto.as16HighCCM[1]			=	option2;break;
						case 2:		stCCMAttr.stAuto.as16HighCCM[2]			=	option2;break;
						case 3:		stCCMAttr.stAuto.as16HighCCM[3]			=	option2;break;
						case 4:		stCCMAttr.stAuto.as16HighCCM[5]			=	option2;break;
						case 5:		stCCMAttr.stAuto.as16HighCCM[6]			=	option2;break;
						case 6:		stCCMAttr.stAuto.as16HighCCM[7]			=	option2;break;
						case 7:		stCCMAttr.stAuto.as16HighCCM[9]			=	option2;break;
						case 8:		stCCMAttr.stAuto.as16HighCCM[10]		=	option2;break;
						case 9:		stCCMAttr.stAuto.as16HighCCM[11]		=	option2;break;
						case 10:	stCCMAttr.stAuto.u16MidColorTemp		=	option2;break;
						case 11:	stCCMAttr.stAuto.as16MidCCM[1]			=	option2;break;
						case 12:	stCCMAttr.stAuto.as16MidCCM[2]			=	option2;break;
						case 13:	stCCMAttr.stAuto.as16MidCCM[3]			=	option2;break;
						case 14:	stCCMAttr.stAuto.as16MidCCM[5]			=	option2;break;
						case 15:	stCCMAttr.stAuto.as16MidCCM[6]			=	option2;break;
						case 16:	stCCMAttr.stAuto.as16MidCCM[7]			=	option2;break;
						case 17:	stCCMAttr.stAuto.as16MidCCM[9]			=	option2;break;
						case 18:	stCCMAttr.stAuto.as16MidCCM[10]			=	option2;break;
						case 19:	stCCMAttr.stAuto.as16MidCCM[11]			=	option2;break;
						case 20:	stCCMAttr.stAuto.u16LowColorTemp		=	option2;break;
						case 21:	stCCMAttr.stAuto.as16LowCCM[1]			=	option2;break;
						case 22:	stCCMAttr.stAuto.as16LowCCM[2]			=	option2;break;
						case 23:	stCCMAttr.stAuto.as16LowCCM[3]			=	option2;break;
						case 24:	stCCMAttr.stAuto.as16LowCCM[5]			=	option2;break;
						case 25:	stCCMAttr.stAuto.as16LowCCM[6]			=	option2;break;
						case 26:	stCCMAttr.stAuto.as16LowCCM[7]			=	option2;break;
						case 27:	stCCMAttr.stAuto.as16LowCCM[9]			=	option2;break;
						case 28:	stCCMAttr.stAuto.as16LowCCM[10]			=	option2;break;
						case 29:	stCCMAttr.stAuto.as16LowCCM[11]			=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetCCMAttr(gIspDev, &stCCMAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetCCMAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
	}
	s32Ret=XM_MPI_ISP_SetCCMAttr(gIspDev, &stCCMAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_SetCCMAttr failed !\n");
		return ISP_FAILURE;
	}
	return ISP_SUCCESS;
}
static ISP_S32 Ccm_menu_2th(void)//CCM主菜单
{
	ISP_U8 u8i=0;
	ISP_S32 option;
	while(1)
	{
		printf("----------------------------------------\n");
		printf("2th Menu( %s ):\n",Ccm[0]);
		printf(" %2d %s\n",0,Menu[0]);
		for(u8i=1;u8i<CcmNum;u8i++)
		{
			printf(" %2d %s\n",u8i,Ccm[u8i]);
		}
		printf(" 99. Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);

		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				Ccm_menu_3th(option);
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
	}
}

/************************************************************************
 * 11.饱和度菜单
************************************************************************/
#define SatNum	(5)
static char Sat[SatNum][20] = {
	"Exp",
	"Enable/Disable",
	"Exp Mode",
	"Manual Thr",
	"Auto Thr",
};
#define SatManualNum	(1)
static char SatManual[SatManualNum][20] = {
	"u8Saturation",
};
#define SatAutoNum	(1)
static char SatAuto[SatAutoNum][20] = {
	"u8Saturation",
};
static ISP_S32 Sat_menu_3th(ISP_S32 data)//饱和度次级菜单
{
	ISP_U8 u8i;
	ISP_S32 option,option2;
	ISP_S32 s32Ret;
	ISP_SATURATION_ATTR_S stSatAttr;

	s32Ret=XM_MPI_ISP_GetSaturationAttr(gIspDev, &stSatAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_GetSaturationAttr failed !\n");
		return ISP_FAILURE;
	}
	switch (data) 
	{
		case 1://使能
			break;
		case 2://模式
			printf("3th Menu( %s ):\n",Sat[0]);
			printf("%s [%s]",Sat[0],IspType[stSatAttr.enOpType]);
			printf("%s (0:%s 1:%s 2:%s)",Sat[0],IspType[0],IspType[1],IspType[2]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=2)
				stSatAttr.enOpType = option;
			break;
		case 3://手动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetSaturationAttr(gIspDev, &stSatAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetSaturationAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Sat[0]);
				printf(" 1  %20s %d:\n",	SatManual[0],	stSatAttr.stManual.u8Saturation			);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=SatManualNum)
				{
					do {
						printf("%s>> ",SatManual[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 1:		stSatAttr.stManual.u8Saturation			=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetSaturationAttr(gIspDev, &stSatAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetSaturationAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
		case 4://自动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetSaturationAttr(gIspDev, &stSatAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetSaturationAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Sat[0]);
				printf("%s",SatAuto[0]);
				for(u8i=0;u8i<ISP_AUTO_STENGTH_NUM;u8i++)
				{
					printf(" %d[%3d]",	u8i+1,		stSatAttr.stAuto.au8Sat[u8i]	);
				}
				printf("\n");
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=ISP_AUTO_STENGTH_NUM)
				{
					do {
						printf("%s>> ",SatAuto[0]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					stSatAttr.stAuto.au8Sat[option-1] = option2;
					s32Ret=XM_MPI_ISP_SetSaturationAttr(gIspDev, &stSatAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetSaturationAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
	}
	s32Ret=XM_MPI_ISP_SetSaturationAttr(gIspDev, &stSatAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_SetSaturationAttr failed !\n");
		return ISP_FAILURE;
	}
	return ISP_SUCCESS;
}
static ISP_S32 Sat_menu_2th(void)//饱和度主菜单
{
	ISP_U8 u8i=0;
	ISP_S32 option;
	while(1)
	{
		printf("----------------------------------------\n");
		printf("2th Menu( %s ):\n",Sat[0]);
		printf(" %2d %s\n",0,Menu[0]);
		for(u8i=1;u8i<SatNum;u8i++)
		{
			printf(" %2d %s\n",u8i,Sat[u8i]);
		}
		printf(" 99. Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);

		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				Sat_menu_3th(option);
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
	}
}

/************************************************************************
 * 12.2D菜单
************************************************************************/
#define Nr2DNum	(6)
static char Nr2D[Nr2DNum][20] = {
	"Nr2D",
	"Enable/Disable",
	"Nr2D Mode",
	"GlobalSth",
	"Manual Thr",
	"Auto Thr",
};
#define Nr2DManualNum	(8)
static char Nr2DManual[Nr2DManualNum][20] = {
	"Thr",
	"KThr",
	"KCoef",
	"KMin",
	"Thr_c",
	"KThr_c",
	"KCoef_c",
	"KMin_c",
};
#define Nr2DAutoNum	(8)
static char Nr2DAuto[Nr2DAutoNum][20] = {
	"Thr",
	"KThr",
	"KCoef",
	"KMin",
	"Thr_c",
	"KThr_c",
	"KCoef_c",
	"KMin_c",
};
static ISP_S32 Nr2D_menu_3th(ISP_S32 data)//2D次级菜单
{
	ISP_U8 u8i,u8j;
	ISP_S32 option,option2,option3;
	ISP_S32 s32Ret;
	ISP_2DNR_ATTR_S stNr2DAttr;

	s32Ret=XM_MPI_ISP_GetNRAttr(gIspDev, &stNr2DAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_GetNRAttr failed !\n");
		return ISP_FAILURE;
	}
	switch (data) 
	{
		case 1://使能
			printf("3th Menu( %s ):\n",Nr2D[0]);
			printf("%s [%s]",Nr2D[0],IspBool[stNr2DAttr.bEnable]);
			printf("%s (0:%s 1:%s)",Nr2D[0],IspBool[0],IspBool[1]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=1)
				stNr2DAttr.bEnable = option;
			break;
		case 2://模式
			printf("3th Menu( %s ):\n",Nr2D[0]);
			printf("%s [%s]",Nr2D[0],IspType[stNr2DAttr.enOpType]);
			printf("%s (0:%s 1:%s 2:%s)",Nr2D[0],IspType[0],IspType[1],IspType[2]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=2)
				stNr2DAttr.enOpType = option;
			break;
		case 3://全局强度
			printf("3th Menu( %s ):\n",Nr2D[0]);
			printf("%s [%d]",Nr2D[data],stNr2DAttr.stAuto.u8GlobalSth);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=255)
				stNr2DAttr.stAuto.u8GlobalSth = option;
			break;
		case 4://手动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetNRAttr(gIspDev, &stNr2DAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetNRAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Nr2D[0]);
				printf(" 1  %20s %d:\n",	Nr2DManual[0],	stNr2DAttr.stManual.u16Thr		);
				printf(" 2  %20s %d:\n",	Nr2DManual[1],	stNr2DAttr.stManual.u8KThr		);
				printf(" 3  %20s %d:\n",	Nr2DManual[2],	stNr2DAttr.stManual.u8KCoef		);
				printf(" 4  %20s %d:\n",	Nr2DManual[3],	stNr2DAttr.stManual.u8KMin		);
				printf(" 5  %20s %d:\n",	Nr2DManual[4],	stNr2DAttr.stManual.u8Thr_c		);
				printf(" 6  %20s %d:\n",	Nr2DManual[5],	stNr2DAttr.stManual.u8KThr_c	);
				printf(" 7  %20s %d:\n",	Nr2DManual[6],	stNr2DAttr.stManual.u8KCoef_c	);
				printf(" 8  %20s %d:\n",	Nr2DManual[7],	stNr2DAttr.stManual.u8KMin_c	);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=Nr2DManualNum)
				{
					do {
						printf("%s>> ",Nr2DManual[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 1:		stNr2DAttr.stManual.u16Thr		=	option2;break;
						case 2:		stNr2DAttr.stManual.u8KThr		=	option2;break;
						case 3:		stNr2DAttr.stManual.u8KCoef		=	option2;break;
						case 4:		stNr2DAttr.stManual.u8KMin		=	option2;break;
						case 5:		stNr2DAttr.stManual.u8Thr_c		=	option2;break;
						case 6:		stNr2DAttr.stManual.u8KThr_c	=	option2;break;
						case 7:		stNr2DAttr.stManual.u8KCoef_c	=	option2;break;
						case 8:		stNr2DAttr.stManual.u8KMin_c	=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetNRAttr(gIspDev, &stNr2DAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetNRAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
		case 5://自动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetNRAttr(gIspDev, &stNr2DAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetNRAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Nr2D[0]);
				for(u8i=0;u8i<Nr2DAutoNum;u8i++)
				{
					printf(" %2d  %20s ",	u8i+1,	Nr2DAuto[u8i]);
					for(u8j=0;u8j<ISP_AUTO_STENGTH_NUM;u8j++)
					{
						switch(u8i)
						{
							case 0:		printf("[%5d] ",stNr2DAttr.stAuto.au16Thr[u8j]		);break;
							case 1:		printf("[%5d] ",stNr2DAttr.stAuto.au8KThr[u8j]		);break;
							case 2:		printf("[%5d] ",stNr2DAttr.stAuto.au8KCoef[u8j]		);break;
							case 3:		printf("[%5d] ",stNr2DAttr.stAuto.au8KMin[u8j]		);break;
							case 4:		printf("[%5d] ",stNr2DAttr.stAuto.au8Thr_c[u8j]		);break;
							case 5:		printf("[%5d] ",stNr2DAttr.stAuto.au8KThr_c[u8j]	);break;
							case 6:		printf("[%5d] ",stNr2DAttr.stAuto.au8KCoef_c[u8j]	);break;
							case 7:		printf("[%5d] ",stNr2DAttr.stAuto.au8KMin_c[u8j]	);break;
						}
					}
					printf("\n");
				}
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;

				if(option>=1 && option<=Nr2DAutoNum)
				{
					do {
						printf("%s>> ",Nr2DAuto[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					if(option2>=0 && option2<ISP_AUTO_STENGTH_NUM)
					{
						do {
							printf("%s[%d]>> ",Nr2DAuto[option-1],option2);
							option3 = get_choose_char();
							printf("\n");
						} while (option3 < 0);
						if(option3>=0 && option3<=255)
						{
							switch(option)
							{
								case 1:		stNr2DAttr.stAuto.au16Thr[option2]		=	option3;break;
								case 2:		stNr2DAttr.stAuto.au8KThr[option2]		=	option3;break;
								case 3:		stNr2DAttr.stAuto.au8KCoef[option2]		=	option3;break;
								case 4:		stNr2DAttr.stAuto.au8KMin[option2]		=	option3;break;
								case 5:		stNr2DAttr.stAuto.au8Thr_c[option2]		=	option3;break;
								case 6:		stNr2DAttr.stAuto.au8KThr_c[option2]	=	option3;break;
								case 7:		stNr2DAttr.stAuto.au8KCoef_c[option2]	=	option3;break;
								case 8:		stNr2DAttr.stAuto.au8KMin_c[option2]	=	option3;break;
							}
							s32Ret=XM_MPI_ISP_SetNRAttr(gIspDev, &stNr2DAttr);
							if(s32Ret != ISP_SUCCESS)
							{
								ERR("XM_MPI_ISP_SetNRAttr failed !\n");
								return ISP_FAILURE;
							}
						}
					}
				}
			}
			break;
	}
	s32Ret=XM_MPI_ISP_SetNRAttr(gIspDev, &stNr2DAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_SetNRAttr failed !\n");
		return ISP_FAILURE;
	}
	return ISP_SUCCESS;
}
static ISP_S32 Nr2D_menu_2th(void)//2D主菜单
{
	ISP_U8 u8i=0;
	ISP_S32 option;
	while(1)
	{
		printf("----------------------------------------\n");
		printf("2th Menu( %s ):\n",Nr2D[0]);
		printf(" %2d %s\n",0,Menu[0]);
		for(u8i=1;u8i<Nr2DNum;u8i++)
		{
			printf(" %2d %s\n",u8i,Nr2D[u8i]);
		}
		printf(" 99. Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);

		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
				Nr2D_menu_3th(option);
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
	}
}

/************************************************************************
 * 13.LTM菜单
************************************************************************/
#define LtmNum	(6)
static char Ltm[LtmNum][20] = {
	"Ltm",
	"Enable/Disable",
	"Ltm Mode",
	"GlobalSth",
	"Manual Thr",
	"Auto Thr",
};
#define LtmManualNum	(5)
static char LtmManual[LtmManualNum][20] = {
	"Rix",
	"KjMax",
	"KjMin",
	"StrD",
	"StrB",
};
#define LtmAutoNum	(5)
static char LtmAuto[LtmAutoNum][20] = {
	"Rix",
	"KjMax",
	"KjMin",
	"StrD",
	"StrB",
};
static ISP_S32 Ltm_menu_3th(ISP_S32 data)//LTM次级菜单
{
	ISP_U8 u8i,u8j;
	ISP_S32 option,option2,option3;
	ISP_S32 s32Ret;
	ISP_LTM_ATTR_S stLtmAttr;

	s32Ret=XM_MPI_ISP_GetLtmAttr(gIspDev, &stLtmAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_GetLtmAttr failed !\n");
		return ISP_FAILURE;
	}
	switch (data) 
	{
		case 1://使能
			printf("3th Menu( %s ):\n",Ltm[0]);
			printf("%s [%s]",Ltm[0],IspBool[stLtmAttr.bEnable]);
			printf("%s (0:%s 1:%s)",Ltm[0],IspBool[0],IspBool[1]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=1)
				stLtmAttr.bEnable = option;
			break;
		case 2://模式
			printf("3th Menu( %s ):\n",Ltm[0]);
			printf("%s [%s]",Ltm[0],IspType[stLtmAttr.enOpType]);
			printf("%s (0:%s 1:%s 2:%s)",Ltm[0],IspType[0],IspType[1],IspType[2]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=2)
				stLtmAttr.enOpType = option;
			break;
		case 3://全局强度
			printf("3th Menu( %s ):\n",Ltm[0]);
			printf("%s [%d]",Ltm[data],stLtmAttr.u8GlobalSth);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=255)
				stLtmAttr.u8GlobalSth = option;
			break;
		case 4://手动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetLtmAttr(gIspDev, &stLtmAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetLtmAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Ltm[0]);
				printf(" 1  %20s %d:\n",	LtmManual[0],	stLtmAttr.stManual.u8Rix		);
				printf(" 2  %20s %d:\n",	LtmManual[1],	stLtmAttr.stManual.u8KjMax		);
				printf(" 3  %20s %d:\n",	LtmManual[2],	stLtmAttr.stManual.u8KjMin		);
				printf(" 4  %20s %d:\n",	LtmManual[3],	stLtmAttr.stManual.u8StrD		);
				printf(" 5  %20s %d:\n",	LtmManual[4],	stLtmAttr.stManual.u8StrB		);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=LtmManualNum)
				{
					do {
						printf("%s>> ",LtmManual[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 1:		stLtmAttr.stManual.u8Rix		=	option2;break;
						case 2:		stLtmAttr.stManual.u8KjMax		=	option2;break;
						case 3:		stLtmAttr.stManual.u8KjMin		=	option2;break;
						case 4:		stLtmAttr.stManual.u8StrD		=	option2;break;
						case 5:		stLtmAttr.stManual.u8StrB		=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetLtmAttr(gIspDev, &stLtmAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetLtmAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
		case 5://自动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetLtmAttr(gIspDev, &stLtmAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetLtmAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Ltm[0]);
				for(u8i=0;u8i<LtmAutoNum;u8i++)
				{
					printf(" %2d  %20s ",	u8i+1,	LtmAuto[u8i]);
					for(u8j=0;u8j<ISP_AUTO_STENGTH_NUM;u8j++)
					{
						switch(u8i)
						{
							case 0:		printf("[%5d] ",stLtmAttr.stAuto.au8Rix[u8j]	);break;
							case 1:		printf("[%5d] ",stLtmAttr.stAuto.au8KjMax[u8j]	);break;
							case 2:		printf("[%5d] ",stLtmAttr.stAuto.au8KjMin[u8j]	);break;
							case 3:		printf("[%5d] ",stLtmAttr.stAuto.au8StrD[u8j]	);break;
							case 4:		printf("[%5d] ",stLtmAttr.stAuto.au8StrB[u8j]	);break;
						}
					}
					printf("\n");
				}
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;

				if(option>=1 && option<=LtmAutoNum)
				{
					do {
						printf("%s>> ",LtmAuto[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					if(option2>=0 && option2<ISP_AUTO_STENGTH_NUM)
					{
						do {
							printf("%s[%d]>> ",LtmAuto[option-1],option2);
							option3 = get_choose_char();
							printf("\n");
						} while (option3 < 0);
						if(option3>=0 && option3<=255)
						{
							switch(option)
							{
								case 1:		stLtmAttr.stAuto.au8Rix[option2]	=	option3;break;
								case 2:		stLtmAttr.stAuto.au8KjMax[option2]	=	option3;break;
								case 3:		stLtmAttr.stAuto.au8KjMin[option2]	=	option3;break;
								case 4:		stLtmAttr.stAuto.au8StrD[option2]	=	option3;break;
								case 5:		stLtmAttr.stAuto.au8StrB[option2]	=	option3;break;
							}
							s32Ret=XM_MPI_ISP_SetLtmAttr(gIspDev, &stLtmAttr);
							if(s32Ret != ISP_SUCCESS)
							{
								ERR("XM_MPI_ISP_SetLtmAttr failed !\n");
								return ISP_FAILURE;
							}
						}
					}
				}
			}
			break;
	}
	s32Ret=XM_MPI_ISP_SetLtmAttr(gIspDev, &stLtmAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_SetLtmAttr failed !\n");
		return ISP_FAILURE;
	}
	return ISP_SUCCESS;
}
static ISP_S32 Ltm_menu_2th(void)//LTM主菜单
{
	ISP_U8 u8i=0;
	ISP_S32 option;
	while(1)
	{
		printf("----------------------------------------\n");
		printf("2th Menu( %s ):\n",Ltm[0]);
		printf(" %2d %s\n",0,Menu[0]);
		for(u8i=1;u8i<LtmNum;u8i++)
		{
			printf(" %2d %s\n",u8i,Ltm[u8i]);
		}
		printf(" 99. Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);

		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
				Ltm_menu_3th(option);
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
	}
}

/************************************************************************
 * 14.3D菜单
************************************************************************/
#define Nr3DNum	(5)
static char Nr3D[Nr3DNum][20] = {
	"Nr3D",
	"Enable/Disable",
	"Nr3D Mode",
	"Manual Thr",
	"Auto Thr",
};
#define Nr3DManualNum	(29)
static char Nr3DManual[Nr3DManualNum][20] = {
	"FltThr",
	"Df_RatC",
	"Df1_RatioY",
	"Df1_RatioC",
	"MovThr_Max",
	"Alpha_Ratio",
	"Alpha_Min",
	"IdxThr_Max",
	"IdxThr_Min",
	"Idx_Max",
	"FidY",
	"FidC",
	"FidY_Min",
	"FidC_Min",
	"YSfThr_Min",
	"CSfThr_Min",
	"YTfThr_Min",
	"CTfThr_Min",
	"YAntiRsdlSpd",
	"CAntiRsdlSpd",
	"YAntiRsdlThr",
	"CAntiRsdlThr",	
	"YSfThr_K",
	"CSfThr_K",
	"YTfThr_K",
	"CTfThr_K",
	"MovThr1",
	"MovThr2",
	"MovThr3",
};
#define Nr3DAutoNum	(29)
static char Nr3DAuto[Nr3DAutoNum][20] = {
	"FltThr",
	"Df_RatC",
	"Df1_RatioY",
	"Df1_RatioC",
	"MovThr_Max",
	"Alpha_Ratio",
	"Alpha_Min",
	"IdxThr_Max",
	"IdxThr_Min",
	"Idx_Max",
	"FidY",
	"FidC",
	"FidY_Min",
	"FidC_Min",
	"YSfThr_Min",
	"CSfThr_Min",
	"YTfThr_Min",
	"CTfThr_Min",
	"YAntiRsdlSpd",
	"CAntiRsdlSpd",
	"YAntiRsdlThr",
	"CAntiRsdlThr",	
	"YSfThr_K",
	"CSfThr_K",
	"YTfThr_K",
	"CTfThr_K",
	"MovThr1",
	"MovThr2",
	"MovThr3",
};
static ISP_S32 Nr3D_menu_3th(ISP_S32 data)//Nr3D次级菜单
{
	ISP_U8 u8i,u8j;
	ISP_S32 option,option2,option3;
	ISP_S32 s32Ret;
	ISP_3DNR_ATTR_S stNr3DAttr;

	s32Ret=XM_MPI_ISP_GetVpss3DNrAttr(gIspDev, &stNr3DAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_GetVpss3DNrAttr failed !\n");
		return ISP_FAILURE;
	}
	switch (data) 
	{
		case 1://使能
			printf("3th Menu( %s ):\n",Nr3D[0]);
			printf(" 1  %20s %d:\n",	Nr3D[1],		stNr3DAttr.u8Enable&0xf);
			printf(" 2  %20s %d:\n",	"Nr3D OSD",		(stNr3DAttr.u8Enable>>4)&0xf);

			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);

			if(option == 99)
				return ISP_SUCCESS;
			if(option>=1 && option<=2)
			{
				do {
					printf(">> ");
					option2 = get_choose_char();
					printf("\n");
				} while (option2 < 0);
				if(option == 1)
				{
					if(option2>=0 && option2<=1)
					{
						stNr3DAttr.u8Enable = stNr3DAttr.u8Enable&0xf0;
						stNr3DAttr.u8Enable = stNr3DAttr.u8Enable|option2;
					}
				}
				else
				{
					if(option2>=0 && option2<=2)
					{
						stNr3DAttr.u8Enable = stNr3DAttr.u8Enable&0x0f;
						stNr3DAttr.u8Enable = stNr3DAttr.u8Enable|(option2<<4);
					}
				}

				s32Ret=XM_MPI_ISP_SetVpss3DNrAttr(gIspDev, &stNr3DAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_SetVpss3DNrAttr failed !\n");
					return ISP_FAILURE;
				}
			}
			break;
		case 2://模式
			printf("3th Menu( %s ):\n",Nr3D[0]);
			printf("%s [%s]",Nr3D[0],IspType[stNr3DAttr.enOpType]);
			printf("%s (0:%s 1:%s 2:%s)",Nr3D[0],IspType[0],IspType[1],IspType[2]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=2)
				stNr3DAttr.enOpType = option;
			break;
		case 3://手动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetVpss3DNrAttr(gIspDev, &stNr3DAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetVpss3DNrAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Nr3D[0]);
				printf(" 1  %20s %d:\n",	Nr3DManual[0],		stNr3DAttr.stManual.u8FltThr		);
				printf(" 2  %20s %d:\n",	Nr3DManual[1],		stNr3DAttr.stManual.u8Df_RatC		);
				printf(" 3  %20s %d:\n",	Nr3DManual[2],		stNr3DAttr.stManual.u8Df1_RatioY	);
				printf(" 4  %20s %d:\n",	Nr3DManual[3],		stNr3DAttr.stManual.u8Df1_RatioC	);
				printf(" 5  %20s %d:\n",	Nr3DManual[4],		stNr3DAttr.stManual.u8MovThr_Max	);
				printf(" 6  %20s %d:\n",	Nr3DManual[5],		stNr3DAttr.stManual.u8Alpha_Ratio	);
				printf(" 7  %20s %d:\n",	Nr3DManual[6],		stNr3DAttr.stManual.u8Alpha_Min		);
				printf(" 8  %20s %d:\n",	Nr3DManual[7],		stNr3DAttr.stManual.u8IdxThr_Max	);
				printf(" 9  %20s %d:\n",	Nr3DManual[8],		stNr3DAttr.stManual.u8IdxThr_Min	);
				printf(" 10 %20s %d:\n",	Nr3DManual[9],		stNr3DAttr.stManual.u8Idx_Max		);
				printf(" 11 %20s %d:\n",	Nr3DManual[10],		stNr3DAttr.stManual.u8FidY			);
				printf(" 12 %20s %d:\n",	Nr3DManual[11],		stNr3DAttr.stManual.u8FidC			);
				printf(" 13 %20s %d:\n",	Nr3DManual[12],		stNr3DAttr.stManual.u8FidY_Min		);
				printf(" 14 %20s %d:\n",	Nr3DManual[13],		stNr3DAttr.stManual.u8FidC_Min		);
				printf(" 15 %20s %d:\n",	Nr3DManual[14],		stNr3DAttr.stManual.u8YSfThr_Min	);
				printf(" 16 %20s %d:\n",	Nr3DManual[15],		stNr3DAttr.stManual.u8CSfThr_Min	);
				printf(" 17 %20s %d:\n",	Nr3DManual[16],		stNr3DAttr.stManual.u8YTfThr_Min	);
				printf(" 18 %20s %d:\n",	Nr3DManual[17],		stNr3DAttr.stManual.u8CTfThr_Min	);
				printf(" 19 %20s %d:\n",	Nr3DManual[18],		stNr3DAttr.stManual.u8YAntiRsdlSpd	);
				printf(" 20 %20s %d:\n",	Nr3DManual[19],		stNr3DAttr.stManual.u8CAntiRsdlSpd	);
				printf(" 21 %20s %d:\n",	Nr3DManual[20],		stNr3DAttr.stManual.u8YAntiRsdlThr	);
				printf(" 22 %20s %d:\n",	Nr3DManual[21],		stNr3DAttr.stManual.u8CAntiRsdlThr	);
				printf(" 23 %20s %d:\n",	Nr3DManual[22],		stNr3DAttr.stManual.u16YSfThr_K		);
				printf(" 24 %20s %d:\n",	Nr3DManual[23],		stNr3DAttr.stManual.u16CSfThr_K		);
				printf(" 25 %20s %d:\n",	Nr3DManual[24],		stNr3DAttr.stManual.u16YTfThr_K		);
				printf(" 26 %20s %d:\n",	Nr3DManual[25],		stNr3DAttr.stManual.u16CTfThr_K		);
				printf(" 27 %20s %d:\n",	Nr3DManual[26],		stNr3DAttr.stManual.u16MovThr1		);
				printf(" 28 %20s %d:\n",	Nr3DManual[27],		stNr3DAttr.stManual.u16MovThr2		);
				printf(" 29 %20s %d:\n",	Nr3DManual[28],		stNr3DAttr.stManual.u16MovThr3		);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=Nr3DManualNum)
				{
					do {
						printf("%s>> ",Nr3DManual[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 1:		stNr3DAttr.stManual.u8FltThr		=	option2;break;
						case 2:		stNr3DAttr.stManual.u8Df_RatC		=	option2;break;
						case 3:		stNr3DAttr.stManual.u8Df1_RatioY	=	option2;break;
						case 4:		stNr3DAttr.stManual.u8Df1_RatioC	=	option2;break;
						case 5:		stNr3DAttr.stManual.u8MovThr_Max	=	option2;break;
						case 6:		stNr3DAttr.stManual.u8Alpha_Ratio	=	option2;break;
						case 7:		stNr3DAttr.stManual.u8Alpha_Min		=	option2;break;
						case 8:		stNr3DAttr.stManual.u8IdxThr_Max	=	option2;break;
						case 9:		stNr3DAttr.stManual.u8IdxThr_Min	=	option2;break;
						case 10:	stNr3DAttr.stManual.u8Idx_Max		=	option2;break;
						case 11:	stNr3DAttr.stManual.u8FidY			=	option2;break;
						case 12:	stNr3DAttr.stManual.u8FidC			=	option2;break;
						case 13:	stNr3DAttr.stManual.u8FidY_Min		=	option2;break;
						case 14:	stNr3DAttr.stManual.u8FidC_Min		=	option2;break;
						case 15:	stNr3DAttr.stManual.u8YSfThr_Min	=	option2;break;
						case 16:	stNr3DAttr.stManual.u8CSfThr_Min	=	option2;break;
						case 17:	stNr3DAttr.stManual.u8YTfThr_Min	=	option2;break;
						case 18:	stNr3DAttr.stManual.u8CTfThr_Min	=	option2;break;
						case 19:	stNr3DAttr.stManual.u8YAntiRsdlSpd	=	option2;break;
						case 20:	stNr3DAttr.stManual.u8CAntiRsdlSpd	=	option2;break;
						case 21:	stNr3DAttr.stManual.u8YAntiRsdlThr	=	option2;break;
						case 22:	stNr3DAttr.stManual.u8CAntiRsdlThr	=	option2;break;
						case 23:	stNr3DAttr.stManual.u16YSfThr_K		=	option2;break;
						case 24:	stNr3DAttr.stManual.u16CSfThr_K		=	option2;break;
						case 25:	stNr3DAttr.stManual.u16YTfThr_K		=	option2;break;
						case 26:	stNr3DAttr.stManual.u16CTfThr_K		=	option2;break;
						case 27:	stNr3DAttr.stManual.u16MovThr1		=	option2;break;
						case 28:	stNr3DAttr.stManual.u16MovThr2		=	option2;break;
						case 29:	stNr3DAttr.stManual.u16MovThr3		=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetVpss3DNrAttr(gIspDev, &stNr3DAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetVpss3DNrAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
		case 4://自动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetVpss3DNrAttr(gIspDev, &stNr3DAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetVpss3DNrAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Nr3D[0]);
				for(u8i=0;u8i<Nr3DAutoNum;u8i++)
				{
					printf(" %2d  %20s ",	u8i+1,	Nr3DAuto[u8i]);
					for(u8j=0;u8j<ISP_AUTO_STENGTH_NUM;u8j++)
					{
						switch(u8i)
						{
							case 0:		printf("[%5d] ",stNr3DAttr.stAuto.au8FltThr[u8j]		);break;
							case 1:		printf("[%5d] ",stNr3DAttr.stAuto.au8Df_RatC[u8j]		);break;
							case 2:		printf("[%5d] ",stNr3DAttr.stAuto.au8Df1_RatioY[u8j]	);break;
							case 3:		printf("[%5d] ",stNr3DAttr.stAuto.au8Df1_RatioC[u8j]	);break;
							case 4:		printf("[%5d] ",stNr3DAttr.stAuto.au8MovThr_Max[u8j]	);break;
							case 5:		printf("[%5d] ",stNr3DAttr.stAuto.au8Alpha_Ratio[u8j]	);break;
							case 6:		printf("[%5d] ",stNr3DAttr.stAuto.au8Alpha_Min[u8j]		);break;
							case 7:		printf("[%5d] ",stNr3DAttr.stAuto.au8IdxThr_Max[u8j]	);break;
							case 8:		printf("[%5d] ",stNr3DAttr.stAuto.au8IdxThr_Min[u8j]	);break;
							case 9:		printf("[%5d] ",stNr3DAttr.stAuto.au8Idx_Max[u8j]		);break;
							case 10:	printf("[%5d] ",stNr3DAttr.stAuto.au8FidY[u8j]			);break;
							case 11:	printf("[%5d] ",stNr3DAttr.stAuto.au8FidC[u8j]			);break;
							case 12:	printf("[%5d] ",stNr3DAttr.stAuto.au8FidY_Min[u8j]		);break;
							case 13:	printf("[%5d] ",stNr3DAttr.stAuto.au8FidC_Min[u8j]		);break;
							case 14:	printf("[%5d] ",stNr3DAttr.stAuto.au8YSfThr_Min[u8j]	);break;
							case 15:	printf("[%5d] ",stNr3DAttr.stAuto.au8CSfThr_Min[u8j]	);break;
							case 16:	printf("[%5d] ",stNr3DAttr.stAuto.au8YTfThr_Min[u8j]	);break;
							case 17:	printf("[%5d] ",stNr3DAttr.stAuto.au8CTfThr_Min[u8j]	);break;
							case 18:	printf("[%5d] ",stNr3DAttr.stAuto.au8YAntiRsdlSpd[u8j]	);break;
							case 19:	printf("[%5d] ",stNr3DAttr.stAuto.au8CAntiRsdlSpd[u8j]	);break;
							case 20:	printf("[%5d] ",stNr3DAttr.stAuto.au8YAntiRsdlThr[u8j]	);break;
							case 21:	printf("[%5d] ",stNr3DAttr.stAuto.au8CAntiRsdlThr[u8j]	);break;
							case 22:	printf("[%5d] ",stNr3DAttr.stAuto.au16YSfThr_K[u8j]		);break;
							case 23:	printf("[%5d] ",stNr3DAttr.stAuto.au16CSfThr_K[u8j]		);break;
							case 24:	printf("[%5d] ",stNr3DAttr.stAuto.au16YTfThr_K[u8j]		);break;
							case 25:	printf("[%5d] ",stNr3DAttr.stAuto.au16CTfThr_K[u8j]		);break;
							case 26:	printf("[%5d] ",stNr3DAttr.stAuto.au16MovThr1[u8j]		);break;
							case 27:	printf("[%5d] ",stNr3DAttr.stAuto.au16MovThr2[u8j]		);break;
							case 28:	printf("[%5d] ",stNr3DAttr.stAuto.au16MovThr3[u8j]		);break;
						}
					}
					printf("\n");
				}
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;

				if(option>=1 && option<=Nr3DAutoNum)
				{
					do {
						printf("%s>> ",Nr3DAuto[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					if(option2>=0 && option2<ISP_AUTO_STENGTH_NUM)
					{
						do {
							printf("%s[%d]>> ",Nr3DAuto[option-1],option2);
							option3 = get_choose_char();
							printf("\n");
						} while (option3 < 0);
						if(option3>=0 && option3<=255)
						{
							switch(option)
							{
								case 1:		stNr3DAttr.stAuto.au8FltThr[option2]		=	option3;break;
								case 2:		stNr3DAttr.stAuto.au8Df_RatC[option2]		=	option3;break;
								case 3:		stNr3DAttr.stAuto.au8Df1_RatioY[option2]	=	option3;break;
								case 4:		stNr3DAttr.stAuto.au8Df1_RatioC[option2]	=	option3;break;
								case 5:		stNr3DAttr.stAuto.au8MovThr_Max[option2]	=	option3;break;
								case 6:		stNr3DAttr.stAuto.au8Alpha_Ratio[option2]	=	option3;break;
								case 7:		stNr3DAttr.stAuto.au8Alpha_Min[option2]		=	option3;break;
								case 8:		stNr3DAttr.stAuto.au8IdxThr_Max[option2]	=	option3;break;
								case 9:		stNr3DAttr.stAuto.au8IdxThr_Min[option2]	=	option3;break;
								case 10:	stNr3DAttr.stAuto.au8Idx_Max[option2]		=	option3;break;
								case 11:	stNr3DAttr.stAuto.au8FidY[option2]			=	option3;break;
								case 12:	stNr3DAttr.stAuto.au8FidC[option2]			=	option3;break;
								case 13:	stNr3DAttr.stAuto.au8FidY_Min[option2]		=	option3;break;
								case 14:	stNr3DAttr.stAuto.au8FidC_Min[option2]		=	option3;break;
								case 15:	stNr3DAttr.stAuto.au8YSfThr_Min[option2]	=	option3;break;
								case 16:	stNr3DAttr.stAuto.au8CSfThr_Min[option2]	=	option3;break;
								case 17:	stNr3DAttr.stAuto.au8YTfThr_Min[option2]	=	option3;break;
								case 18:	stNr3DAttr.stAuto.au8CTfThr_Min[option2]	=	option3;break;
								case 19:	stNr3DAttr.stAuto.au8YAntiRsdlSpd[option2]	=	option3;break;
								case 20:	stNr3DAttr.stAuto.au8CAntiRsdlSpd[option2]	=	option3;break;
								case 21:	stNr3DAttr.stAuto.au8YAntiRsdlThr[option2]	=	option3;break;
								case 22:	stNr3DAttr.stAuto.au8CAntiRsdlThr[option2]	=	option3;break;
								case 23:	stNr3DAttr.stAuto.au16YSfThr_K[option2]		=	option3;break;
								case 24:	stNr3DAttr.stAuto.au16CSfThr_K[option2]		=	option3;break;
								case 25:	stNr3DAttr.stAuto.au16YTfThr_K[option2]		=	option3;break;
								case 26:	stNr3DAttr.stAuto.au16CTfThr_K[option2]		=	option3;break;
								case 27:	stNr3DAttr.stAuto.au16MovThr1[option2]		=	option3;break;
								case 28:	stNr3DAttr.stAuto.au16MovThr2[option2]		=	option3;break;
								case 29:	stNr3DAttr.stAuto.au16MovThr3[option2]		=	option3;break;
							}
							s32Ret=XM_MPI_ISP_SetVpss3DNrAttr(gIspDev, &stNr3DAttr);
							if(s32Ret != ISP_SUCCESS)
							{
								ERR("XM_MPI_ISP_SetVpss3DNrAttr failed !\n");
								return ISP_FAILURE;
							}
						}
					}
				}
			}
			break;
	}
	s32Ret=XM_MPI_ISP_SetVpss3DNrAttr(gIspDev, &stNr3DAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_SetVpss3DNrAttr failed !\n");
		return ISP_FAILURE;
	}
	return ISP_SUCCESS;
}
static ISP_S32 Nr3D_menu_2th(void)//Nr3D主菜单
{
	ISP_U8 u8i=0;
	ISP_S32 option;
	while(1)
	{
		printf("----------------------------------------\n");
		printf("2th Menu( %s ):\n",Nr3D[0]);
		printf(" %2d %s\n",0,Menu[0]);
		for(u8i=1;u8i<Nr3DNum;u8i++)
		{
			printf(" %2d %s\n",u8i,Nr3D[u8i]);
		}
		printf(" 99. Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);

		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				Nr3D_menu_3th(option);
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
	}
}

/************************************************************************
 * 15.YcNr菜单
************************************************************************/
#define YcNrNum	(5)
static char YcNr[YcNrNum][20] = {
	"YcNr",
	"Enable/Disable",
	"YcNr Mode",
	"Manual Thr",
	"Auto Thr",
};
#define YcNrManualNum	(7)
static char YcNrManual[YcNrManualNum][20] = {
	"u8Thr",
	"u8Str",
	"u8MixThr",
	"u8Wt1",
	"u8Wt2",
	"u8Wt3",
	"u16K",
};
#define YcNrAutoNum	(7)
static char YcNrAuto[YcNrAutoNum][20] = {
	"u8Thr",
	"u8Str",
	"u8MixThr",
	"u8Wt1",
	"u8Wt2",
	"u8Wt3",
	"u16K",
};
static ISP_S32 YcNr_menu_3th(ISP_S32 data)//YcNr次级菜单
{
	ISP_U8 u8i,u8j;
	ISP_S32 option,option2,option3;
	ISP_S32 s32Ret;
	ISP_YCNR_ATTR_S stYcNrAttr;

	s32Ret=XM_MPI_ISP_GetYcNRAttr(gIspDev, &stYcNrAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_GetYcNRAttr failed !\n");
		return ISP_FAILURE;
	}
	switch (data) 
	{
		case 1://使能
			printf("3th Menu( %s ):\n",YcNr[0]);
			printf("%s [%s]",YcNr[0],IspBool[stYcNrAttr.bEnable]);
			printf("%s (0:%s 1:%s)",YcNr[0],IspBool[0],IspBool[1]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=1)
				stYcNrAttr.bEnable = option;
			break;
		case 2://模式
			printf("3th Menu( %s ):\n",YcNr[0]);
			printf("%s [%s]",YcNr[0],IspType[stYcNrAttr.enOpType]);
			printf("%s (0:%s 1:%s 2:%s)",YcNr[0],IspType[0],IspType[1],IspType[2]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=2)
				stYcNrAttr.enOpType = option;
			break;
		case 3://手动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetYcNRAttr(gIspDev, &stYcNrAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetYcNRAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",YcNr[0]);
				printf(" 1  %20s %d:\n",	YcNrManual[0],		stYcNrAttr.stManual.u8Thr		);
				printf(" 2  %20s %d:\n",	YcNrManual[1],		stYcNrAttr.stManual.u8Str		);
				printf(" 3  %20s %d:\n",	YcNrManual[2],		stYcNrAttr.stManual.u8MixThr	);
				printf(" 4  %20s %d:\n",	YcNrManual[3],		stYcNrAttr.stManual.u8Wt1		);
				printf(" 5  %20s %d:\n",	YcNrManual[4],		stYcNrAttr.stManual.u8Wt2		);
				printf(" 6  %20s %d:\n",	YcNrManual[5],		stYcNrAttr.stManual.u8Wt3		);
				printf(" 7  %20s %d:\n",	YcNrManual[6],		stYcNrAttr.stManual.u16K		);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=YcNrManualNum)
				{
					do {
						printf("%s>> ",YcNrManual[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 1:		stYcNrAttr.stManual.u8Thr		=	option2;break;
						case 2:		stYcNrAttr.stManual.u8Str		=	option2;break;
						case 3:		stYcNrAttr.stManual.u8MixThr	=	option2;break;
						case 4:		stYcNrAttr.stManual.u8Wt1		=	option2;break;
						case 5:		stYcNrAttr.stManual.u8Wt2		=	option2;break;
						case 6:		stYcNrAttr.stManual.u8Wt3		=	option2;break;
						case 7:		stYcNrAttr.stManual.u16K		=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetYcNRAttr(gIspDev, &stYcNrAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetYcNRAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
		case 4://自动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetYcNRAttr(gIspDev, &stYcNrAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetYcNRAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",YcNr[0]);
				for(u8i=0;u8i<YcNrAutoNum;u8i++)
				{
					printf(" %2d  %20s ",	u8i+1,	YcNrAuto[u8i]);
					for(u8j=0;u8j<ISP_AUTO_STENGTH_NUM;u8j++)
					{
						switch(u8i)
						{
							case 0:		printf("[%5d] ",stYcNrAttr.stAuto.au8Thr[u8j]		);break;
							case 1:		printf("[%5d] ",stYcNrAttr.stAuto.au8Str[u8j]		);break;
							case 2:		printf("[%5d] ",stYcNrAttr.stAuto.au8MixThr[u8j]	);break;
							case 3:		printf("[%5d] ",stYcNrAttr.stAuto.au8Wt1[u8j]		);break;
							case 4:		printf("[%5d] ",stYcNrAttr.stAuto.au8Wt2[u8j]		);break;
							case 5:		printf("[%5d] ",stYcNrAttr.stAuto.au8Wt3[u8j]		);break;
							case 6:		printf("[%5d] ",stYcNrAttr.stAuto.au16K[u8j]		);break;
						}
					}
					printf("\n");
				}
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;

				if(option>=1 && option<=YcNrAutoNum)
				{
					do {
						printf("%s>> ",YcNrAuto[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					if(option2>=0 && option2<ISP_AUTO_STENGTH_NUM)
					{
						do {
							printf("%s[%d]>> ",YcNrAuto[option-1],option2);
							option3 = get_choose_char();
							printf("\n");
						} while (option3 < 0);
						if(option3>=0 && option3<=255)
						{
							switch(option)
							{
								case 1:		stYcNrAttr.stAuto.au8Thr[option2]		=	option3;break;
								case 2:		stYcNrAttr.stAuto.au8Str[option2]		=	option3;break;
								case 3:		stYcNrAttr.stAuto.au8MixThr[option2]	=	option3;break;
								case 4:		stYcNrAttr.stAuto.au8Wt1[option2]		=	option3;break;
								case 5:		stYcNrAttr.stAuto.au8Wt2[option2]		=	option3;break;
								case 6:		stYcNrAttr.stAuto.au8Wt3[option2]		=	option3;break;
								case 7:		stYcNrAttr.stAuto.au16K[option2]		=	option3;break;
							}
							s32Ret=XM_MPI_ISP_SetYcNRAttr(gIspDev, &stYcNrAttr);
							if(s32Ret != ISP_SUCCESS)
							{
								ERR("XM_MPI_ISP_SetYcNRAttr failed !\n");
								return ISP_FAILURE;
							}
						}
					}
				}
			}
			break;
	}
	s32Ret=XM_MPI_ISP_SetYcNRAttr(gIspDev, &stYcNrAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_SetYcNRAttr failed !\n");
		return ISP_FAILURE;
	}
	return ISP_SUCCESS;
}
static ISP_S32 YcNr_menu_2th(void)//YcNr主菜单
{
	ISP_U8 u8i=0;
	ISP_S32 option;
	while(1)
	{
		printf("----------------------------------------\n");
		printf("2th Menu( %s ):\n",YcNr[0]);
		printf(" %2d %s\n",0,Menu[0]);
		for(u8i=1;u8i<YcNrNum;u8i++)
		{
			printf(" %2d %s\n",u8i,YcNr[u8i]);
		}
		printf(" 99. Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);

		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				YcNr_menu_3th(option);
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
	}
}

/************************************************************************
 * 16.Drc菜单
************************************************************************/
#define DrcNum	(5)
static char Drc[DrcNum][20] = {
	"Drc",
	"Enable/Disable",
	"Drc Mode",
	"Manual Thr",
	"Auto Thr",
};
#define DrcManualNum	(1)
static char DrcManual[DrcManualNum][20] = {
	"u8Strength",
};
#define DrcAutoNum	(1)
static char DrcAuto[DrcAutoNum][20] = {
	"au8Sth",
};
static ISP_S32 Drc_menu_3th(ISP_S32 data)//Drc次级菜单
{
	ISP_U8 u8i;
	ISP_S32 option,option2;
	ISP_S32 s32Ret;
	ISP_DRC_ATTR_S stDrcAttr;

	s32Ret=XM_MPI_ISP_GetDRCAttr(gIspDev, &stDrcAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_GetDRCAttr failed !\n");
		return ISP_FAILURE;
	}
	switch (data) 
	{
		case 1://使能
			printf("3th Menu( %s ):\n",Drc[0]);
			printf("%s [%s]",Drc[0],IspBool[stDrcAttr.bEnable]);
			printf("%s (0:%s 1:%s)",Drc[0],IspBool[0],IspBool[1]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=1)
				stDrcAttr.bEnable = option;
			break;
		case 2://模式
			printf("3th Menu( %s ):\n",Drc[0]);
			printf("%s [%s]",Drc[0],IspType[stDrcAttr.enOpType]);
			printf("%s (0:%s 1:%s 2:%s)",Drc[0],IspType[0],IspType[1],IspType[2]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=2)
				stDrcAttr.enOpType = option;
			break;
		case 3://手动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetDRCAttr(gIspDev, &stDrcAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetDRCAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Drc[0]);
				printf(" 1  %20s %d:\n",	DrcManual[0],	stDrcAttr.stManual.u8Strength			);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=DrcManualNum)
				{
					do {
						printf("%s>> ",DrcManual[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 1:		stDrcAttr.stManual.u8Strength			=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetDRCAttr(gIspDev, &stDrcAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetDRCAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
		case 4://自动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetDRCAttr(gIspDev, &stDrcAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetDRCAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Drc[0]);
				printf("%s",DrcAuto[0]);
				for(u8i=0;u8i<ISP_AUTO_STENGTH_NUM;u8i++)
				{
					printf(" %d[%3d]",	u8i,		stDrcAttr.stAuto.au8Sth[u8i]	);
				}
				printf("\n");
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=0 && option<ISP_AUTO_STENGTH_NUM)
				{
					do {
						printf("%s>> ",DrcAuto[option]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					stDrcAttr.stAuto.au8Sth[option] = option2;
					s32Ret=XM_MPI_ISP_SetDRCAttr(gIspDev, &stDrcAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetDRCAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
	}
	s32Ret=XM_MPI_ISP_SetDRCAttr(gIspDev, &stDrcAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_SetDRCAttr failed !\n");
		return ISP_FAILURE;
	}
	return ISP_SUCCESS;
}
static ISP_S32 Drc_menu_2th(void)//Drc主菜单
{
	ISP_U8 u8i=0;
	ISP_S32 option;
	while(1)
	{
		printf("----------------------------------------\n");
		printf("2th Menu( %s ):\n",Drc[0]);
		printf(" %2d %s\n",0,Menu[0]);
		for(u8i=1;u8i<DrcNum;u8i++)
		{
			printf(" %2d %s\n",u8i,Drc[u8i]);
		}
		printf(" 99. Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);

		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				Drc_menu_3th(option);
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
	}
}

/************************************************************************
 * 17.Defog_Bpu菜单
************************************************************************/
#define Defog_BpuNum	(5)
static char Defog_Bpu[Defog_BpuNum][20] = {
	"Defog_Bpu",
	"Enable/Disable",
	"Defog_Bpu Mode",
	"Manual Thr",
	"Auto Thr",
};
#define Defog_BpuManualNum	(1)
static char Defog_BpuManual[Defog_BpuManualNum][20] = {
	"u8Strength",
};
#define Defog_BpuAutoNum	(1)
static char Defog_BpuAuto[Defog_BpuAutoNum][20] = {
	"au8Sth",
};
static ISP_S32 Defog_Bpu_menu_3th(ISP_S32 data)//Defog_Bpu次级菜单
{
	ISP_U8 u8i;
	ISP_S32 option,option2;
	ISP_S32 s32Ret;
	ISP_DEFOG_ATTR_S stDefog_BpuAttr;

	s32Ret=XM_MPI_ISP_GetDeFogAttr(gIspDev, &stDefog_BpuAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_GetDeFogAttr failed !\n");
		return ISP_FAILURE;
	}
	switch (data) 
	{
		case 1://使能
			printf("3th Menu( %s ):\n",Defog_Bpu[0]);
			printf("%s [%s]",Defog_Bpu[0],IspBool[stDefog_BpuAttr.bEnable]);
			printf("%s (0:%s 1:%s)",Defog_Bpu[0],IspBool[0],IspBool[1]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=1)
				stDefog_BpuAttr.bEnable = option;
			break;
		case 2://模式
			printf("3th Menu( %s ):\n",Defog_Bpu[0]);
			printf("%s [%s]",Defog_Bpu[0],IspType[stDefog_BpuAttr.enOpType]);
			printf("%s (0:%s 1:%s 2:%s)",Defog_Bpu[0],IspType[0],IspType[1],IspType[2]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=2)
				stDefog_BpuAttr.enOpType = option;
			break;
		case 3://手动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetDeFogAttr(gIspDev, &stDefog_BpuAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetDeFogAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Defog_Bpu[0]);
				printf(" 1  %20s %d:\n",	Defog_BpuManual[0],	stDefog_BpuAttr.stManual.u8Strength			);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=Defog_BpuManualNum)
				{
					do {
						printf("%s>> ",Defog_BpuManual[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 1:		stDefog_BpuAttr.stManual.u8Strength			=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetDeFogAttr(gIspDev, &stDefog_BpuAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetDeFogAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
		case 4://自动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetDeFogAttr(gIspDev, &stDefog_BpuAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetDeFogAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Defog_Bpu[0]);
				printf("%s",Defog_BpuAuto[0]);
				for(u8i=0;u8i<ISP_AUTO_STENGTH_NUM;u8i++)
				{
					printf(" %d[%3d]",	u8i,		stDefog_BpuAttr.stAuto.au8Sth[u8i]	);
				}
				printf("\n");
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=0 && option<ISP_AUTO_STENGTH_NUM)
				{
					do {
						printf("%s>> ",Defog_BpuAuto[0]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					stDefog_BpuAttr.stAuto.au8Sth[option] = option2;
					s32Ret=XM_MPI_ISP_SetDeFogAttr(gIspDev, &stDefog_BpuAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetDeFogAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
	}
	s32Ret=XM_MPI_ISP_SetDeFogAttr(gIspDev, &stDefog_BpuAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_SetDeFogAttr failed !\n");
		return ISP_FAILURE;
	}
	return ISP_SUCCESS;
}
static ISP_S32 Defog_Bpu_menu_2th(void)//Defog_Bpu主菜单
{
	ISP_U8 u8i=0;
	ISP_S32 option;
	while(1)
	{
		printf("----------------------------------------\n");
		printf("2th Menu( %s ):\n",Defog_Bpu[0]);
		printf(" %2d %s\n",0,Menu[0]);
		for(u8i=1;u8i<Defog_BpuNum;u8i++)
		{
			printf(" %2d %s\n",u8i,Defog_Bpu[u8i]);
		}
		printf(" 99. Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);

		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				Defog_Bpu_menu_3th(option);
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
	}
}

/************************************************************************
 * 18.Defog_Vpss菜单
************************************************************************/
#define Defog_VpssNum	(5)
static char Defog_Vpss[Defog_VpssNum][20] = {
	"Defog_Vpss",
	"Enable/Disable",
	"Defog_Vpss Mode",
	"Manual Thr",
	"Auto Thr",
};
#define Defog_VpssManualNum	(1)
static char Defog_VpssManual[Defog_VpssManualNum][20] = {
	"u8Strength",
};
#define Defog_VpssAutoNum	(1)
static char Defog_VpssAuto[Defog_VpssAutoNum][20] = {
	"au8Sth",
};
static ISP_S32 Defog_Vpss_menu_3th(ISP_S32 data)//Defog_Vpss次级菜单
{
	ISP_U8 u8i;
	ISP_S32 option,option2;
	ISP_S32 s32Ret;
	ISP_DEFOG_ATTR_S stDefog_VpssAttr;

	s32Ret=XM_MPI_ISP_GetVpssDeFogAttr(gIspDev, &stDefog_VpssAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_GetVpssDeFogAttr failed !\n");
		return ISP_FAILURE;
	}
	switch (data) 
	{
		case 1://使能
			printf("3th Menu( %s ):\n",Defog_Vpss[0]);
			printf("%s [%s]",Defog_Vpss[0],IspBool[stDefog_VpssAttr.bEnable]);
			printf("%s (0:%s 1:%s)",Defog_Vpss[0],IspBool[0],IspBool[1]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=1)
				stDefog_VpssAttr.bEnable = option;
			break;
		case 2://模式
			printf("3th Menu( %s ):\n",Defog_Vpss[0]);
			printf("%s [%s]",Defog_Vpss[0],IspType[stDefog_VpssAttr.enOpType]);
			printf("%s (0:%s 1:%s 2:%s)",Defog_Vpss[0],IspType[0],IspType[1],IspType[2]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=2)
				stDefog_VpssAttr.enOpType = option;
			break;
		case 3://手动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetVpssDeFogAttr(gIspDev, &stDefog_VpssAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetVpssDeFogAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Defog_Vpss[0]);
				printf(" 1  %20s %d:\n",	Defog_VpssManual[0],	stDefog_VpssAttr.stManual.u8Strength			);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=Defog_VpssManualNum)
				{
					do {
						printf("%s>> ",Defog_VpssManual[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 1:		stDefog_VpssAttr.stManual.u8Strength			=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetVpssDeFogAttr(gIspDev, &stDefog_VpssAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetVpssDeFogAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
		case 4://自动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetVpssDeFogAttr(gIspDev, &stDefog_VpssAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetVpssDeFogAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Defog_Vpss[0]);
				printf("%s",Defog_VpssAuto[0]);
				for(u8i=0;u8i<ISP_AUTO_STENGTH_NUM;u8i++)
				{
					printf(" %d[%3d]",	u8i,		stDefog_VpssAttr.stAuto.au8Sth[u8i]	);
				}
				printf("\n");
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=0 && option<ISP_AUTO_STENGTH_NUM)
				{
					do {
						printf("%s>> ",Defog_VpssAuto[0]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					stDefog_VpssAttr.stAuto.au8Sth[option] = option2;
					s32Ret=XM_MPI_ISP_SetVpssDeFogAttr(gIspDev, &stDefog_VpssAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetVpssDeFogAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
	}
	s32Ret=XM_MPI_ISP_SetVpssDeFogAttr(gIspDev, &stDefog_VpssAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_SetVpssDeFogAttr failed !\n");
		return ISP_FAILURE;
	}
	return ISP_SUCCESS;
}
static ISP_S32 Defog_Vpss_menu_2th(void)//Defog_Vpss主菜单
{
	ISP_U8 u8i=0;
	ISP_S32 option;
	while(1)
	{
		printf("----------------------------------------\n");
		printf("2th Menu( %s ):\n",Defog_Vpss[0]);
		printf(" %2d %s\n",0,Menu[0]);
		for(u8i=1;u8i<Defog_VpssNum;u8i++)
		{
			printf(" %2d %s\n",u8i,Defog_Vpss[u8i]);
		}
		printf(" 99. Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);

		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				Defog_Vpss_menu_3th(option);
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
	}
}

/************************************************************************
 * 19.Dpc菜单
************************************************************************/
#define DpcNum	(4)
static char Dpc[DpcNum][20] = {
	"Dpc",
	"Dpc Mode",
	"Manual Thr",
	"Auto Thr",
};
#define DpcManualNum	(1)
static char DpcManual[DpcManualNum][20] = {
	"u8Sth",
};
#define DpcAutoNum	(1)
static char DpcAuto[DpcAutoNum][20] = {
	"au8Sth",
};
static ISP_S32 Dpc_menu_3th(ISP_S32 data)//Dpc次级菜单
{
	ISP_U8 u8i;
	ISP_S32 option,option2;
	ISP_S32 s32Ret;
	ISP_DYDPC_ATTR_S stDpcAttr;

	s32Ret=XM_MPI_ISP_GetDyDefectPixelAttr(gIspDev, &stDpcAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_GetDyDefectPixelAttr failed !\n");
		return ISP_FAILURE;
	}
	switch (data) 
	{
		case 1://模式
			printf("3th Menu( %s ):\n",Dpc[0]);
			printf("%s [%s]",Dpc[0],IspType[stDpcAttr.enOpType]);
			printf("%s (0:%s 1:%s 2:%s)",Dpc[0],IspType[0],IspType[1],IspType[2]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=2)
				stDpcAttr.enOpType = option;
			break;
		case 2://手动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetDyDefectPixelAttr(gIspDev, &stDpcAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetDyDefectPixelAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Dpc[0]);
				printf(" 1  %20s %d:\n",	DpcManual[0],	stDpcAttr.stManual.u8Sth			);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=DpcManualNum)
				{
					do {
						printf("%s>> ",DpcManual[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 1:		stDpcAttr.stManual.u8Sth			=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetDyDefectPixelAttr(gIspDev, &stDpcAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetDyDefectPixelAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
		case 3://自动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetDyDefectPixelAttr(gIspDev, &stDpcAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetDyDefectPixelAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Dpc[0]);
				printf("%s",DpcAuto[0]);
				for(u8i=0;u8i<ISP_AUTO_STENGTH_NUM;u8i++)
				{
					printf(" %d[%3d]",	u8i,		stDpcAttr.stAuto.au8Sth[u8i]	);
				}
				printf("\n");
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=0 && option<ISP_AUTO_STENGTH_NUM)
				{
					do {
						printf("%s>> ",DpcAuto[option]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					stDpcAttr.stAuto.au8Sth[option] = option2;
					s32Ret=XM_MPI_ISP_SetDyDefectPixelAttr(gIspDev, &stDpcAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetDyDefectPixelAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
	}
	s32Ret=XM_MPI_ISP_SetDyDefectPixelAttr(gIspDev, &stDpcAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_SetDyDefectPixelAttr failed !\n");
		return ISP_FAILURE;
	}
	return ISP_SUCCESS;
}
static ISP_S32 Dpc_menu_2th(void)//Dpc主菜单
{
	ISP_U8 u8i=0;
	ISP_S32 option;
	while(1)
	{
		printf("----------------------------------------\n");
		printf("2th Menu( %s ):\n",Dpc[0]);
		printf(" %2d %s\n",0,Menu[0]);
		for(u8i=1;u8i<DpcNum;u8i++)
		{
			printf(" %2d %s\n",u8i,Dpc[u8i]);
		}
		printf(" 99. Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);

		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
			case 2:
			case 3:
				Dpc_menu_3th(option);
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
	}
}

/************************************************************************
 * 20.AE统计菜单
************************************************************************/
static ISP_S32 AEStaten_menu_2th(void)//AE统计主菜单
{
	ISP_S32 s32Ret;
	ISP_U32 u32AllGain = 0;
	ISP_INNER_STATE_INFO_S stInnerStateInfo;
	s32Ret = XM_MPI_ISP_QueryInnerStateInfo(gIspDev, &stInnerStateInfo);
   	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_QueryInnerStateInfo failed !\n");
		return ISP_FAILURE;
	}
	if(stInnerStateInfo.u32ExposureTime)
	{
		u32AllGain = stInnerStateInfo.u32Exposure * 10 / stInnerStateInfo.u32ExposureTime;
	}

	printf("Ae Shutter(short): %d\n", stInnerStateInfo.u32ExpTime_short);
	printf("Ae Shutter: %d\n", stInnerStateInfo.u32ExposureTime);
	printf("Ae AGain: %d\n", stInnerStateInfo.u32AnalogGain);
	printf("Ae DGain: %d\n", stInnerStateInfo.u32DigitalGain);
	printf("Ae IspDGain: %d\n", stInnerStateInfo.u32IspDGain);
	printf("Ae Exp: %d\n", stInnerStateInfo.u32Exposure);
	printf("Ae AllGain: %d.%d\n", u32AllGain/10,u32AllGain%10);
	printf("Ae Lum: %d\n", stInnerStateInfo.u8AveLum);
    return ISP_SUCCESS;
}

/************************************************************************
 * 21.Blk菜单
************************************************************************/
#define BlkNum	(4)
static char Blk[BlkNum][20] = {
	"Blk",
	"Blk Mode",
	"Manual Thr",
	"Auto Thr",
};
#define BlkManualNum	(1)
static char BlkManual[BlkManualNum][20] = {
	"u16Blc",
};
#define BlkAutoNum	(1)
static char BlkAuto[BlkAutoNum][20] = {
	"au16Blc",
};
static ISP_S32 Blk_menu_3th(ISP_S32 data)//Blk次级菜单
{
	ISP_U8 u8i;
	ISP_S32 option,option2;
	ISP_S32 s32Ret;
	ISP_BLACKLVL_ATTR_S stBlkAttr;

	s32Ret=XM_MPI_ISP_GetBlackLevelAttr(gIspDev, &stBlkAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_GetBlackLevelAttr failed !\n");
		return ISP_FAILURE;
	}
	switch (data) 
	{
		case 1://模式
			printf("3th Menu( %s ):\n",Blk[0]);
			printf("%s [%s]",Blk[0],IspType[stBlkAttr.enOpType]);
			printf("%s (0:%s 1:%s 2:%s)",Blk[0],IspType[0],IspType[1],IspType[2]);
			do {
				printf(">> ");
				option = get_choose_char();
				printf("\n");
			} while (option < 0);
			if(option>=0 && option<=2)
				stBlkAttr.enOpType = option;
			break;
		case 2://手动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetBlackLevelAttr(gIspDev, &stBlkAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetBlackLevelAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Blk[0]);
				printf(" 1  %20s %d:\n",	BlkManual[0],	stBlkAttr.stManual.u16Blc			);
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=BlkManualNum)
				{
					do {
						printf("%s>> ",BlkManual[option-1]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					switch(option)
					{
						case 1:		stBlkAttr.stManual.u16Blc			=	option2;break;
					}
					s32Ret=XM_MPI_ISP_SetBlackLevelAttr(gIspDev, &stBlkAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetBlackLevelAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
		case 3://自动
			while(1)
			{
				s32Ret=XM_MPI_ISP_GetBlackLevelAttr(gIspDev, &stBlkAttr);
				if(s32Ret != ISP_SUCCESS)
				{
					ERR("XM_MPI_ISP_GetBlackLevelAttr failed !\n");
					return ISP_FAILURE;
				}
				printf("3th Menu( %s ):\n",Blk[0]);
				printf("%s",BlkAuto[0]);
				for(u8i=0;u8i<ISP_AUTO_STENGTH_NUM;u8i++)
				{
					printf(" %d[%3d]",	u8i,		stBlkAttr.stAuto.au16Blc[u8i]	);
				}
				printf("\n");
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;
				if(option>=1 && option<=ISP_AUTO_STENGTH_NUM)
				{
					do {
						printf("%s>> ",BlkAuto[0]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					stBlkAttr.stAuto.au16Blc[option-1] = option2;
					s32Ret=XM_MPI_ISP_SetBlackLevelAttr(gIspDev, &stBlkAttr);
					if(s32Ret != ISP_SUCCESS)
					{
						ERR("XM_MPI_ISP_SetBlackLevelAttr failed !\n");
						return ISP_FAILURE;
					}
				}
			}
			break;
	}
	s32Ret=XM_MPI_ISP_SetBlackLevelAttr(gIspDev, &stBlkAttr);
	if(s32Ret != ISP_SUCCESS)
	{
		ERR("XM_MPI_ISP_SetBlackLevelAttr failed !\n");
		return ISP_FAILURE;
	}
	return ISP_SUCCESS;
}
static ISP_S32 Blk_menu_2th(void)//Blk主菜单
{
	ISP_U8 u8i=0;
	ISP_S32 option;
	while(1)
	{
		printf("----------------------------------------\n");
		printf("2th Menu( %s ):\n",Blk[0]);
		printf(" %2d %s\n",0,Menu[0]);
		for(u8i=1;u8i<BlkNum;u8i++)
		{
			printf(" %2d %s\n",u8i,Blk[u8i]);
		}
		printf(" 99. Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);

		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
			case 2:
			case 3:
				Blk_menu_3th(option);
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
	}
}

/************************************************************************
 * 22.Chroma菜单
************************************************************************/
#define ChromaNum	(14)
static char Chroma[ChromaNum][20] = {
	"Chroma",
	"Chroma Mode",
	"Hue1Sel",
	"Hue2Sel",
	"Hue3Sel",
	"Hue1Ofst",
	"Hue2Ofst",
	"Hue3Ofst",
	"SthPu",
	"SthR",
	"SthYe",
	"SthG",
	"SthCy",
	"SthB",
};
static ISP_S32 Chroma_menu_2th(void)//Chroma主菜单
{
	ISP_U8 u8i=0;
	ISP_S32 option,option2;
	ISP_CHROMA_ATTR_S stChromaAttr;
	while(1)
	{
		ISPCHECK_RET(XM_MPI_ISP_GetChromaAttr(gIspDev,&stChromaAttr), "XM_MPI_ISP_GetChromaAttr");
		printf("----------------------------------------\n");
		printf("2th Menu( %s ):\n","Chroma");
		printf(" %2d %20s\n",u8i,Menu[0]);
		printf(" %2d %20s %d\n",	1,	Chroma[1],	stChromaAttr.bEnable);
		printf(" %2d %20s %d\n",	2,	Chroma[2],	stChromaAttr.enHue1Sel);
		printf(" %2d %20s %d\n",	3,	Chroma[3],	stChromaAttr.enHue2Sel);
		printf(" %2d %20s %d\n",	4,	Chroma[4],	stChromaAttr.enHue3Sel);
		printf(" %2d %20s %d\n",	5,	Chroma[5],	stChromaAttr.s16Hue1Ofst);
		printf(" %2d %20s %d\n",	6,	Chroma[6],	stChromaAttr.s16Hue2Ofst);
		printf(" %2d %20s %d\n",	7,	Chroma[7],	stChromaAttr.s16Hue3Ofst);
		printf(" %2d %20s %d\n",	8,	Chroma[8],	stChromaAttr.u8SthPu);
		printf(" %2d %20s %d\n",	9,	Chroma[9],	stChromaAttr.u8SthR);
		printf(" %2d %20s %d\n",	10,	Chroma[10],	stChromaAttr.u8SthYe);
		printf(" %2d %20s %d\n",	11,	Chroma[11],	stChromaAttr.u8SthG);
		printf(" %2d %20s %d\n",	12,	Chroma[12],	stChromaAttr.u8SthCy);
		printf(" %2d %20s %d\n",	13,	Chroma[13],	stChromaAttr.u8SthB);
		printf(" 99. Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);
		if((option >= 1) && (option <= ChromaNum-1))
		{
			do {
				printf("%20s>> ",Chroma[option]);
				option2 = get_choose_char();
				printf("\n");
			} while (option2 < 0);
		}
		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
				option2 = CLIP3(option2,0,1);
				stChromaAttr.bEnable = option2;
				break;
			case 2:
				option2 = CLIP3(option2,0,5);
				stChromaAttr.enHue1Sel = option2;
				break;
			case 3:
				option2 = CLIP3(option2,0,5);
				stChromaAttr.enHue2Sel = option2;
				break;
			case 4:
				option2 = CLIP3(option2,0,5);
				stChromaAttr.enHue3Sel = option2;
				break;
			case 5:
				option2 = CLIP3(option2,0,65535);
				stChromaAttr.s16Hue1Ofst = option2;
				break;
			case 6:
				option2 = CLIP3(option2,0,65535);
				stChromaAttr.s16Hue2Ofst = option2;
				break;
			case 7:
				option2 = CLIP3(option2,0,65535);
				stChromaAttr.s16Hue3Ofst = option2;
				break;
			case 8:
				option2 = CLIP3(option2,0,255);
				stChromaAttr.u8SthPu = option2;
				break;
			case 9:
				option2 = CLIP3(option2,0,255);
				stChromaAttr.u8SthR = option2;
				break;
			case 10:
				option2 = CLIP3(option2,0,255);
				stChromaAttr.u8SthYe = option2;
				break;
			case 11:
				option2 = CLIP3(option2,0,255);
				stChromaAttr.u8SthG = option2;
				break;
			case 12:
				option2 = CLIP3(option2,0,255);
				stChromaAttr.u8SthCy = option2;
				break;
			case 13:
				option2 = CLIP3(option2,0,255);
				stChromaAttr.u8SthB = option2;
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
		ISPCHECK_RET(XM_MPI_ISP_SetChromaAttr(gIspDev,&stChromaAttr), "XM_MPI_ISP_SetChromaAttr");
	}
}

/************************************************************************
 * 23.Csc菜单
************************************************************************/
#define CscNum	(6)
static char Csc[CscNum][20] = {
	"Csc",
	"Luma",
	"Contrast",
	"Hue",
	"Saturation",
	"Contrast(Auto)"
};
static ISP_S32 Csc_menu_3th(ISP_S32 data)//Lsc次级菜单
{
	ISP_U8 u8i,u8j;
	ISP_S32 option,option2,option3;
	ISP_S32 s32Ret;
	ISP_CSC_ATTR_S stCscAttr;

	switch (data) 
	{
		case 5://Contrast自动
			while(1)
			{
				ISPCHECK_RET(XM_MPI_ISP_GetCSCAttr(gIspDev, &stCscAttr), "XM_MPI_ISP_GetCSCAttr");
				printf("3th Menu( %s ):\n",Lsc[0]);
				for(u8i=0;u8i<1;u8i++)
				{
					printf(" %2d  %20s ",	1,	Csc[5]);
					for(u8j=0;u8j<ISP_AUTO_STENGTH_NUM;u8j++)
					{
						switch(u8i)
						{
							case 0:		printf("[%5d] ",stCscAttr.stAuto.au8ContrVal[u8j]				);break;
						}
					}
					printf("\n");
				}
				printf(" 99. Quit\n");
				do {
					printf(">> ");
					option = get_choose_char();
					printf("\n");
				} while (option < 0);
				if(option == 99)
					return ISP_SUCCESS;

				if(option>=1 && option<=1)
				{
					do {
						printf("%s>> ",Csc[5]);
						option2 = get_choose_char();
						printf("\n");
					} while (option2 < 0);
					if(option2>=0 && option2<ISP_AUTO_STENGTH_NUM)
					{
						do {
							printf("%s[%d]>> ",Csc[5],option2);
							option3 = get_choose_char();
							printf("\n");
						} while (option3 < 0);
						if(option3>=0 && option3<=100)
						{
							switch(option)
							{
								case 1:		stCscAttr.stAuto.au8ContrVal[option2]			=	option3;break;
							}
							ISPCHECK_RET(XM_MPI_ISP_SetCSCAttr(gIspDev, &stCscAttr), "XM_MPI_ISP_SetCSCAttr");
						}
					}
				}
			}
			break;
	}
	return ISP_SUCCESS;
}
static ISP_S32 Csc_menu_2th(void)//Chroma主菜单
{
	ISP_U8 u8i=0;
	ISP_S32 option,option2;
	ISP_CSC_ATTR_S stCscAttr;
	while(1)
	{
		ISPCHECK_RET(XM_MPI_ISP_GetCSCAttr(gIspDev, &stCscAttr), "XM_MPI_ISP_GetCSCAttr");
		printf("----------------------------------------\n");
		printf("2th Menu( %s ):\n","Chroma");
		printf(" %2d %20s\n",u8i,Menu[0]);
		printf(" %2d %20s %d\n",	1,	Csc[1],	stCscAttr.stGlobal.u8LumaVal);
		printf(" %2d %20s %d\n",	2,	Csc[2],	stCscAttr.stGlobal.u8ContrVal);
		printf(" %2d %20s %d\n",	3,	Csc[3],	stCscAttr.stGlobal.u8HueVal);
		printf(" %2d %20s %d\n",	4,	Csc[4],	stCscAttr.stGlobal.u8SatuVal);
		printf(" %2d %20s\n",		5,	Csc[5]);

		printf(" 99. Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);
		if((option >= 1) && (option <= CscNum-2))
		{
			do {
				printf("%20s>> ",Csc[option]);
				option2 = get_choose_char();
				printf("\n");
			} while (option2 < 0);
		}
		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
				option2 = CLIP3(option2,0,100);
				stCscAttr.stGlobal.u8LumaVal = option2;
				break;
			case 2:
				option2 = CLIP3(option2,0,100);
				stCscAttr.stGlobal.u8ContrVal = option2;
				break;
			case 3:
				option2 = CLIP3(option2,0,100);
				stCscAttr.stGlobal.u8HueVal = option2;
				break;
			case 4:
				option2 = CLIP3(option2,0,100);
				stCscAttr.stGlobal.u8SatuVal = option2;
				break;
			case 5:
				Csc_menu_3th(option);
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
		ISPCHECK_RET(XM_MPI_ISP_SetCSCAttr(gIspDev, &stCscAttr), "XM_MPI_ISP_SetCSCAttr");
	}
}

/************************************************************************
 * 调试工具主函数
************************************************************************/

ISP_S32 ispTool(void)
{
	ISP_S32 option;
	ISP_U8 u8i;
	while (1) 
	{
		printf("----------------------------------------\n");
		printf("1th Menu:\n");
		for(u8i=0;u8i<MenuNum;u8i++)
		{
			printf(" %2d %s\n",u8i,Menu[u8i]);
		}
		printf(" 99. Quit\n");
		printf("----------------------------------------\n");

		do {
			printf(">> ");
			option = get_choose_char();
			printf("\n");
		} while (option < 0);

		switch (option) 
		{
			case 0:
				dev_sel();
				break;
			case 1:
				Exposure_menu_2th();
				break;
			case 2:
				break;
			case 3:
				break;
			case 4:
				Awb_menu_2th();
				break;
			case 5:
				//Ldc_menu_2th();
				break;
			case 6:
				break;
			case 7:
				break;
			case 8:
				Lsc_menu_2th();
				break;
			case 9:
				Sharpen_menu_2th();
				break;
			case 10:
				Ccm_menu_2th();
				break;
			case 11:
				Sat_menu_2th();
				break;
			case 12:
				Nr2D_menu_2th();
				break;
			case 13:
				Ltm_menu_2th();
				break;
			case 14:
				Nr3D_menu_2th();
				break;
			case 15:
				YcNr_menu_2th();
				break;
			case 16:
				Drc_menu_2th();
				break;
			case 17:
				Defog_Bpu_menu_2th();
				break;
			case 18:
				Defog_Vpss_menu_2th();
				break;
			case 19:
				Dpc_menu_2th();
				break;
			case 20:
				AEStaten_menu_2th();
				break;
			case 21:
				Blk_menu_2th();
				break;
			case 22:
				Chroma_menu_2th();
				break;
			case 23:
				Csc_menu_2th();
				break;
			case 99:
				printf("\n");
				return ISP_SUCCESS;
				break;
		}
	}
	return ISP_SUCCESS;
}

MSH_CMD_EXPORT(ispTool, ispTool);
