/*******************************************************************************
Copyright © zhian Tech Co., Ltd. 2022-2023. All rights reserved.
文件名: version.h
作者@编号： zhian
功能描述: version头文件
******************************************************************************/
#ifndef __VERSION_H__
#define __VERSION_H__

#define BE3Y_T_VER  0 // if 1 BE3Y-T  , 0 BE3-T

#if BE3Y_T_VER
	#define FIRMWARE_VERSION "V1.0.0_b"
	#define FIRMWARE_VERSION_ZA "BE3Y100_b231211"
	#define FIRMWARE_VERSION_ZAPRD "ZF50S_BE3Y_V1.0.0_b231220"
#else
	#define FIRMWARE_VERSION "ZAV02L"
	#define FIRMWARE_VERSION_ZA "ZAV02L08_4612"
	#define FIRMWARE_VERSION_ZAPRD "ZF50S_BE3_bZAV02L08_4612"
#endif
char g_version[32];
char g_version_za[32];

#endif

