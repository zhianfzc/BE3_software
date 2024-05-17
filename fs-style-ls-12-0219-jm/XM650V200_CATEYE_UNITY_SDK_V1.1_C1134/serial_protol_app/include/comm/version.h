/*******************************************************************************
Copyright © zhian Tech Co., Ltd. 2022-2023. All rights reserved.
文件名: version.h
作者@编号： zhian
功能描述: version头文件
******************************************************************************/
#ifndef __VERSION_H__
#define __VERSION_H__

#define BE3Y_T_VER  1 // if 1 BE3Y-T  , 0 BE3-T

#if BE3Y_T_VER
	#define FIRMWARE_VERSION "V1.0.4_b"
	#define FIRMWARE_VERSION_ZA "LT104_b240424"
	#define FIRMWARE_VERSION_ZAPRD "ZF50S_BE3_LT_V1.0.4_b240424"
#else
	#define FIRMWARE_VERSION "V1.1.3b"
	#define FIRMWARE_VERSION_ZA "L113_b240327"
	#define FIRMWARE_VERSION_ZAPRD "ZF50S_BE3_V1.1.3_b240327"
#endif
char g_version[32];
char g_version_za[32];

#endif

