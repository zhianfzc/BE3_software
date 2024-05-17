/*******************************************************************************
Copyright ? XmSilicon Tech Co., Ltd. 2022-2023. All rights reserved.
文件名: main.c
作者@编号: 
功能描述: xm650v200双摄门锁demo
******************************************************************************/
#include <rtthread.h>
#include <stdio.h>
#include <unistd.h>

#include "app_main.h"
#include "st_lock_msg.h"
#include "version.h"

int app_main(void)
{
	MY_STATUS_PRT(FIRMWARE_VERSION);
#ifdef SUM_TIME
	 clk_get_pts(&g_start_pts);
#endif

#ifdef SOFTWARE_TRIGGER_USB_UP
	/* 获取配置 */
	mem_apply_config_info(CONFIG_VIRT_ADDR);
	usb_up_judge();
#else
	usb_up_judge();

	/* 获取配置 */
	mem_apply_config_info(CONFIG_VIRT_ADDR);
#endif

	CONFIG_INFO_ST *pst_config_info = config_info_get();
	
	#if 1
	//printf("CONFIG_INFO_AUDIO_FREQ %d\n",pst_config_info->CONFIG_INFO_AUDIO_FREQ);
	//printf("CONFIG_INFO_FLIP %d\n",pst_config_info->CONFIG_INFO_FLIP);
	//printf("CONFIG_INFO_RESOLUTION %d\n",pst_config_info->CONFIG_INFO_RESOLUTION);
	//printf("CONFIG_INFO_USB_TRANSFER %d\n",pst_config_info->CONFIG_INFO_USB_TRANSFER);
	//printf("CONFIG_INFO_JPG_ENCODE %d\n",pst_config_info->CONFIG_INFO_JPG_ENCODE);
	
	char g_tmp[32] ={0,};
	
	char *sfrq[4];
	if(pst_config_info->CONFIG_INFO_AUDIO_FREQ == 8000){
		//sfrq ="8k";
		sprintf(sfrq,"%s","8k");
	}else{
		//sfrq ="16k";
		sprintf(sfrq,"%s","16k");
	}
	
	char *sf[1];
	if(pst_config_info->CONFIG_INFO_USB_TRANSFER ==0){
		//sf = "I";
		sprintf(sf,"%s","I");
	}else{
		//sf = "B";
		sprintf(sf,"%s","B");
	}
	
	int usbtag = 0;
	
	if(pst_config_info->CONFIG_INFO_USB_MPS == 512){
		usbtag = 0;
	}else{
		usbtag = 1;
	}
	
	//zcy add get version
	sprintf(g_tmp,"%s_%s%d_%dF%dJ%d_%d%s%d",FIRMWARE_VERSION,sfrq,pst_config_info->CONFIG_INFO_RESOLUTION,usbtag,pst_config_info->CONFIG_INFO_FRAME_SIZE,pst_config_info->CONFIG_INFO_JPG_QFACTOR,pst_config_info->CONFIG_INFO_JPG_ENCODE,sf,pst_config_info->CONFIG_INFO_FLIP);
	memcpy(g_version,g_tmp,32);
	
	memset(g_tmp,0,32);
	sprintf(g_tmp,"%s_%s%d_%dF%dJ%d_%d%s%d",FIRMWARE_VERSION_ZA,sfrq,pst_config_info->CONFIG_INFO_RESOLUTION,usbtag,pst_config_info->CONFIG_INFO_FRAME_SIZE,pst_config_info->CONFIG_INFO_JPG_QFACTOR,pst_config_info->CONFIG_INFO_JPG_ENCODE,sf,pst_config_info->CONFIG_INFO_FLIP);
	memcpy(g_version_za,g_tmp,32);
	
	printf("ver %s\n",g_version);
	//printf("ver za %s\n",g_version_za);
	#endif

	rt_thread_t tid_recognize, tid_uart, tid_timer, tid_uvc_adc, tid_usb;

	/* 检测猫眼供电 */
	tid_uvc_adc = rt_thread_create("tid_uvc_adc", (void*)intf_adc_detect, NULL, 2048, 26, 10);
	if (tid_uvc_adc != RT_NULL)
		rt_thread_startup(tid_uvc_adc);
	else
		MY_ERR_PRT("creat uvc_adc_detect", tid_uvc_adc);

	/* 系统初始化 */
	int ret = app_system_init(pst_config_info);
	if (ret != 0)
	{
		MY_ERR_PRT("system_init", ret);
		return -1;
	}

	unsigned char *p_face_power_sign = face_power_sign();
	unsigned char *p_usb_power_sign = usb_power_sign();

	while (1)
	{
		if (*p_face_power_sign == SIGN_ENABLE)
		{
			//CHECK_RET(XM_MPI_CAM_SetAEMod(0, FACE_PRIOR), "XM_MPI_CAM_SetAEMod");

			/* 计时器 */
			tid_timer = rt_thread_create("tid_timer", (void*)work_timer_start, NULL, 2048, 24, 10);
			if (tid_timer != RT_NULL)
				rt_thread_startup(tid_timer);
			else
				MY_ERR_PRT("creat work_timer", tid_timer);

			/* 串口通讯 	*/
			tid_uart = rt_thread_create("tid_uart", (void*)uart_connect, NULL, 64000, 25, 10);
			if (tid_uart != RT_NULL)
				rt_thread_startup(tid_uart);
			else
				MY_ERR_PRT("creat uart_connect", tid_uart);

			/* 开启识别 */
			tid_recognize = rt_thread_create("tid_recognize", (void*)recognize_start, (void*)pst_config_info, 1536000, 24, 20);
			if (tid_recognize != RT_NULL)
				rt_thread_startup(tid_recognize);
			else
				MY_ERR_PRT("creat recognize_start", tid_recognize);

			*p_face_power_sign = SIGN_ENABLE_END;
		}

		if (*p_usb_power_sign == SIGN_ENABLE)
		{
			//set_rled_luminance(87, 20);

			/* usb */
			tid_usb = rt_thread_create("tid_uvc_adc", (void*)usb_start, (void*)pst_config_info, 5120, 24, 10);
			if (tid_usb != RT_NULL)
				rt_thread_startup(tid_usb);
			else
				MY_ERR_PRT("creat uvc_creat", tid_usb);

			*p_usb_power_sign = SIGN_ENABLE_END;
		}

		if (*p_face_power_sign == SIGN_ENABLE_END && *p_usb_power_sign == SIGN_ENABLE_END)
			break;

		usleep(10*1000);
	}

	while (1)
		usleep(10*1000);

	return 0;
}

