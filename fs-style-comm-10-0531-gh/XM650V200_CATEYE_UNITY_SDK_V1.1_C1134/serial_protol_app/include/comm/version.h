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
	#define FIRMWARE_VERSION "V1.0.3_b"
	#define FIRMWARE_VERSION_ZA "Y103_b240418"
	#define FIRMWARE_VERSION_ZAPRD "ZF50S_BE3Y_V1.0.3_b240418"
#else
	#define FIRMWARE_VERSION "A4V01L"
	#define FIRMWARE_VERSION_ZA "A4V01L08_4618"
	#define FIRMWARE_VERSION_ZAPRD "ZF50S_BE3_cA4V01L08_4618"
#endif
char g_version[32];
char g_version_za[32];

#endif

