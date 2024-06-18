#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arm_neon.h>
//#include "xm_wdt.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "board.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vpss.h"

#include "mpi_ispapp.h"
#include "mpi_camera.h"

#include "app_main.h"

//#include "arasan.h"
//#include "atbm_wifi_driver_api.h"
#define CHECK_RET(express,name) \
	do{ \
		XM_S32 Ret; \
		Ret = express; \
		if (XM_SUCCESS != Ret) \
		{ \
			printf("\033[0;31m%s failed at %s: LINE: %d with %#x!\033[0;39m\n", \
					name, __FUNCTION__, __LINE__, Ret); \
		} \
	}while(0)
extern int root_romfs_mount(void);
extern int xm_mmz_init(unsigned int start, unsigned int len);
extern int xm_isp_init(void);
extern int xm_rgn_init(void);
extern int xm_sys_init(void);
extern int xm_vb_init(void);
extern int xm_vpss_init(void);
extern int xm_vo_init(int gpio);
extern int xm_ddr_init(void);
void reg_config(void)
{
	writel(0x1,				0x20000000);
	writel(0xC105C103,      0x20000074);
	writel(0xC103C003,      0x20000078);
	writel(0x0000C103,      0x2000007C);
	writel(0x0,      0x20000120);
	writel(0x7,      0x20000120);

	writel(0x0,				0x20000000);
	writel(0x1,         	0x2000011C);
	writel(1, 				0x20050490);
	writel(1, 				0x20050540);
	writel(1, 				0x200505F0);
}
void driver_load(void)
{
	//xm_wdt_init();
	xm_mmz_init(0x80D00000, 0x2600000);
	xm_sys_init();
	xm_vb_init();
	//xm_vo_init(8);
	xm_vpss_init();
	xm_rgn_init();
	xm_isp_init();
	xm_ddr_init();
	
}

int main(void)
{
	reg_config();
	driver_load();
	
	rt_thread_t tid_dfp;
	tid_dfp = rt_thread_create("app_main",
			(void*)app_main, RT_NULL,
			128000,
			24,
			5);

	if (tid_dfp != RT_NULL)
	{
		rt_thread_startup(tid_dfp);
	}
	else
	{
		printf("create thread dfp_lock failed\n");
		return -1;
	}
	sleep(2);
    return 0;
}

