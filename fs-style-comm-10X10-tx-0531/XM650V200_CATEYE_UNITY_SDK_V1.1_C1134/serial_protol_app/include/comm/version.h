/*******************************************************************************
Copyright © zhian Tech Co., Ltd. 2022-2023. All rights reserved.
文件名: version.h
作者@编号： zhian
功能描述: version头文件
******************************************************************************/
#ifndef __VERSION_H__
#define __VERSION_H__
/*2024.4.17 异形板_裁剪uvc720P_3-RGB暗角*/
#define BE3Y_T_VER  1 // if 1 BE3Y-T  , 0 BE3-T

#if BE3Y_T_VER
	#define FIRMWARE_VERSION "A0V02L"
	#define FIRMWARE_VERSION_ZA "A0V02L08_4719"
	#define FIRMWARE_VERSION_ZAPRD "ZF50S_KT_V1.0.3_bc240607_mc10_tx"
#else
	#define FIRMWARE_VERSION "V1.1.3_b"
	#define FIRMWARE_VERSION_ZA "BE3112_b240329"
	#define FIRMWARE_VERSION_ZAPRD "ZF50S_BE3_V1.1.2_b240329"
#endif
char g_version[32];
char g_version_za[32];

#endif

