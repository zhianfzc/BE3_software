/*
 * KDP Sensor driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#include "board_kl520.h"
#include <string.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <cmsis_os2.h>
#include "kdp_sensor.h"

#if 0
#ifndef SENSOR_TYPE_MAX
#ifdef SENSOR_TYPE_GC02M1_L
#define SENSOR_TYPE_MAX         (SENSOR_TYPE_GC02M1_L + 1)
#else
#define SENSOR_TYPE_MAX         (SENSOR_TYPE_SC035HGS + 1)
#endif
#endif
#else
#ifdef SENSOR_TYPE_MAX
#undef SENSOR_TYPE_MAX
#endif
#define SENSOR_TYPE_MAX (2)
#endif

struct kdp_sensor_s {
    int                 inuse;
    struct sensor_ops   *ops;
} sensor_s[SENSOR_TYPE_MAX];


int kdp_sensor_s_power(int sensor_idx, int on)
{
    if (sensor_idx >= SENSOR_TYPE_MAX)
        return -1;
    
    if (sensor_s[sensor_idx].ops->s_power == NULL)
        return -1;
        
    return sensor_s[sensor_idx].ops->s_power(on);
}

int kdp_sensor_reset(int sensor_idx)
{
    if (sensor_idx >= SENSOR_TYPE_MAX)
        return -1;
    
    if (sensor_s[sensor_idx].ops->reset == NULL)
        return -1;
        
    return sensor_s[sensor_idx].ops->reset();
}

int kdp_sensor_s_stream(int sensor_idx, int enable)
{
    if (sensor_idx >= SENSOR_TYPE_MAX)
        return -1;
    
    if (sensor_s[sensor_idx].ops->s_stream == NULL)
        return -1;
        
    return sensor_s[sensor_idx].ops->s_stream(enable);
}

int kdp_sensor_enum_fmt(int sensor_idx, unsigned int index, unsigned int *fourcc)
{
    if (sensor_idx >= SENSOR_TYPE_MAX)
        return -1;
    
    if (sensor_s[sensor_idx].ops->enum_fmt == NULL)
        return -1;
        
    return sensor_s[sensor_idx].ops->enum_fmt(index, fourcc);
}

int kdp_sensor_set_fmt(int sensor_idx, struct cam_format *format)
{
    if (sensor_idx >= SENSOR_TYPE_MAX)
        return -1;
    
    if (sensor_s[sensor_idx].ops->set_fmt == NULL)
        return -1;
        
    return sensor_s[sensor_idx].ops->set_fmt(format);
}

int kdp_sensor_set_gain(int sensor_idx, u8 gain1, u8 gain2)
{
    if (sensor_idx >= SENSOR_TYPE_MAX)
        return -1;

    if (sensor_s[sensor_idx].ops->set_gain == NULL)
        return -1;

    return sensor_s[sensor_idx].ops->set_gain(gain1, gain2);
}

int kdp_sensor_set_exp_time(int sensor_idx, u8 gain1, u8 gain2)
{
    if (sensor_idx >= SENSOR_TYPE_MAX)
        return -1;

    if (sensor_s[sensor_idx].ops->set_exp_time == NULL)
        return -1;

    return sensor_s[sensor_idx].ops->set_exp_time(gain1, gain2);
}

int kdp_sensor_get_lux(int sensor_idx, u16* exposure, u8* pre_gain, u8* post_gain, u8* global_gain, u8* y_average)
{
    if (sensor_idx >= SENSOR_TYPE_MAX)
        return -1;

    if (sensor_s[sensor_idx].ops->get_lux == NULL)
        return -1;

    return sensor_s[sensor_idx].ops->get_lux(exposure, pre_gain, post_gain, global_gain, y_average);
}

int kdp_sensor_set_aec_roi(int sensor_idx, u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
{
    if (sensor_idx >= SENSOR_TYPE_MAX)
        return -1;

    if (sensor_s[sensor_idx].ops->set_aec_roi == NULL)
        return -1;

    return sensor_s[sensor_idx].ops->set_aec_roi(x1, x2, y1, y2, center_x1, center_x2, center_y1, center_y2);
}

int kdp_sensor_set_mirror(int sensor_idx, BOOL enable)
{
    if (sensor_idx >= SENSOR_TYPE_MAX)
        return -1;

    if (sensor_s[sensor_idx].ops->set_mirror == NULL)
        return -1;

    return sensor_s[sensor_idx].ops->set_mirror(enable);
}

int kdp_sensor_set_flip(int sensor_idx, BOOL enable)
{
    if (sensor_idx >= SENSOR_TYPE_MAX)
        return -1;

    if (sensor_s[sensor_idx].ops->set_flip == NULL)
        return -1;

    return sensor_s[sensor_idx].ops->set_flip(enable);
}

int kdp_sensor_set_led(int sensor_idx, BOOL enable)
{
    if (sensor_idx >= SENSOR_TYPE_MAX)
        return -1;

    if (sensor_s[sensor_idx].ops->set_led == NULL)
        return -1;

    return sensor_s[sensor_idx].ops->set_led(enable);
}

int kdp_sensor_set_fps(int sensor_idx, u8 fps)
{
    if (sensor_idx >= SENSOR_TYPE_MAX)
        return -1;

    if (sensor_s[sensor_idx].ops->set_fps == NULL)
        return -1;

    return sensor_s[sensor_idx].ops->set_fps(fps);
}

int kdp_sensor_get_fmt(int sensor_idx, struct cam_format *format)
{
    if (sensor_idx >= SENSOR_TYPE_MAX)
        return -1;
    
    if (sensor_s[sensor_idx].ops->get_fmt == NULL)
        return -1;
        
    return sensor_s[sensor_idx].ops->get_fmt(format);
}

int kdp_sensor_get_device_id(int sensor_idx)
{
    if (sensor_idx >= SENSOR_TYPE_MAX)
        return -1;
    
    if (sensor_s[sensor_idx].ops->get_id == NULL)
        return -1;
        
    return sensor_s[sensor_idx].ops->get_id();
}

int kdp_sensor_register(int sensor_id, struct sensor_ops *sensor_ops_p)
{
    if (sensor_ops_p == NULL)
        return -1;

    if (IMGSRC_0_TYPE == sensor_id)
    {
        if (sensor_s[0].inuse)
            return -1;

        sensor_s[0].ops = sensor_ops_p;
        sensor_s[0].inuse = 1;
    }
    else if (IMGSRC_1_TYPE == sensor_id)
    {
        if (sensor_s[1].inuse)
            return -1;

        sensor_s[1].ops = sensor_ops_p;
        sensor_s[1].inuse = 1;
    }

    return 0;
}

int kdp_sensor_unregister(int sensor_id, struct sensor_ops *sensor_ops_p)
{
    if (sensor_ops_p == NULL)
        return -1;

    if (IMGSRC_0_TYPE == sensor_id)
    {
        if (!sensor_s[0].inuse)
            return -1;

        sensor_s[0].ops = NULL;
        sensor_s[0].inuse = 0;
    }
    else if (IMGSRC_1_TYPE == sensor_id)
    {
        if (!sensor_s[1].inuse)
            return -1;

        sensor_s[1].ops = NULL;
        sensor_s[1].inuse = 0;
    }

    return 0;
}

int kdp_sensor_set_aec_en(int sensor_idx, BOOL enable)
{
    if (sensor_idx >= SENSOR_TYPE_MAX)
        return -1;

    if (sensor_s[sensor_idx].ops->set_aec_en == NULL)
        return -1;

    return sensor_s[sensor_idx].ops->set_aec_en(enable);
}

#if CFG_ONE_SHOT_MODE == YES
int kdp_sensor_sleep(int sensor_idx, BOOL enable)
{
    if (sensor_idx >= SENSOR_TYPE_MAX)
        return -1;

    if (sensor_s[sensor_idx].ops->sleep == NULL)
        return -1;

    return sensor_s[sensor_idx].ops->sleep(enable);;
}
#endif


