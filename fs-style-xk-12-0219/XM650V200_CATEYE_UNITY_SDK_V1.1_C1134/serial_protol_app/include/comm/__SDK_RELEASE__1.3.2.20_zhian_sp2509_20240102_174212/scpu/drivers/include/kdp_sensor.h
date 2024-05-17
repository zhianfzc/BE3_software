/*
 * KDP Sensor driver header
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef __KDP_SENSOR_H__
#define __KDP_SENSOR_H__

#include <cmsis_os2.h>
#include "kdp_camera.h"

struct sensor_ops {
    int (*s_power)(int on);
    int (*reset)(void);
    int (*s_stream)(int enable);
    int (*enum_fmt)(unsigned int index, unsigned int *fourcc);
    int (*get_fmt)(struct cam_format *format);
    int (*set_fmt)(struct cam_format *format);
    int (*set_gain)(u8 gain1, u8 gain2);
    int (*set_exp_time)(u8 gain1, u8 gain2);
    int (*get_lux)(u16* exposure, u8* pre_gain, u8* post_gain, u8* global_gain, u8* y_average);
    int (*set_aec_roi)(u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2);
    int (*set_mirror)(BOOL enable);
    int (*set_flip)(BOOL enable);
	int (*set_led)(BOOL enable);
    int (*get_id)();
    int (*set_fps)(u8 fps);
    int (*set_aec_en)(BOOL enable);
    int (*sleep)(BOOL enable);
};

int kdp_sensor_s_power(int sensor_idx, int on);

int kdp_sensor_reset(int sensor_idx);

int kdp_sensor_s_stream(int sensor_idx, int enable);

int kdp_sensor_enum_fmt(int sensor_idx, unsigned int index, unsigned int *fourcc);

int kdp_sensor_set_fmt(int sensor_idx, struct cam_format *format);

int kdp_sensor_get_fmt(int sensor_idx, struct cam_format *format);

int kdp_sensor_set_gain(int sensor_idx, u8 gain1, u8 gain2);

int kdp_sensor_set_exp_time(int sensor_idx, u8 gain1, u8 gain2);

int kdp_sensor_get_lux(int sensor_idx, u16* exposure, u8* pre_gain, u8* post_gain, u8* global_gain, u8* y_average);

int kdp_sensor_set_aec_roi(int sensor_idx, u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2);

int kdp_sensor_set_mirror(int sensor_idx, BOOL enable);

int kdp_sensor_set_flip(int sensor_idx, BOOL enable);

int kdp_sensor_set_led(int sensor_idx, BOOL enable);

int kdp_sensor_set_fps(int sensor_idx, u8 fps);

int kdp_sensor_get_device_id(int sensor_idx);

int kdp_sensor_register(int sensor_idx, struct sensor_ops *sensor_ops_p);

int kdp_sensor_unregister(int sensor_idx, struct sensor_ops *sensor_ops_p);

int kdp_sensor_set_aec_en(int sensor_idx, BOOL enable);

#if CFG_ONE_SHOT_MODE == YES
int kdp_sensor_sleep(int sensor_idx, BOOL enable);
#endif

#endif // __KDP_SENSOR_H__
