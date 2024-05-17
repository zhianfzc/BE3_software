/*
 * KDP Camera driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include "board_kl520.h"
#include <string.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <cmsis_os2.h>
#include "kdp_camera.h"
#include "kl520_api.h"
#include "kl520_api_snapshot.h"

struct kdp_camera_s {
    int             inuse;
    struct cam_ops  *ops;
} camera_s[IMGSRC_NUM];

#ifdef KL520
extern int kdp_camera_kl520_init(void);
#endif

int kdp_camera_init(void)
{
    kl520_customer_info Cusinfo;
    kl520_api_customer_get(&Cusinfo);

    if (Cusinfo.user_rotate_180 == ROTATE_180_ENABLE)
    {
#if (CFG_SENSOR_0_TYPE == CFG_SENSOR_1_TYPE)
        rgb_sensor_index = (CFR_CAM_RGB==0 ? 1 : 0);
        nir_sensor_index = (CFR_CAM_NIR==0 ? 1 : 0);
#endif
#if (CFG_SENSOR_0_TYPE > SENSOR_TYPE_NULL)
        sensor_0_mirror = (CFG_SENSOR_0_FMT_MIRROR==0 ? 1 : 0);
        sensor_0_flip = (CFG_SENSOR_0_FMT_FLIP==0 ? 1 : 0);
#endif
#if (CFG_SENSOR_1_TYPE > SENSOR_TYPE_NULL)
        sensor_1_mirror = (CFG_SENSOR_1_FMT_MIRROR==0 ? 1 : 0);
        sensor_1_flip = (CFG_SENSOR_1_FMT_FLIP==0 ? 1 : 0);
#endif
    }

#ifdef KL520
    kdp_camera_kl520_init();
#endif
    return 0;
}

int kdp_camera_open(unsigned int cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;
    
    if ((camera_s[cam_idx].ops->open == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;

    return camera_s[cam_idx].ops->open(cam_idx);
}

void kdp_camera_close(unsigned int cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return;
    
    if ((camera_s[cam_idx].ops->close == NULL) || camera_s[cam_idx].ops == NULL)
        return;
        
    camera_s[cam_idx].ops->close(cam_idx);
}

int kdp_camera_query_capability(unsigned int cam_idx, struct cam_capability *cap)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;
    
    if ((camera_s[cam_idx].ops->query_capability == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;
        
    return camera_s[cam_idx].ops->query_capability(cam_idx, cap);
}

int kdp_camera_get_device_info(unsigned int cam_idx, struct cam_capability *cap)
{
    return kdp_camera_query_capability(cam_idx, cap);
}

int kdp_camera_set_frame_format(unsigned int cam_idx, struct cam_format *format)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;
    
    if ((camera_s[cam_idx].ops->set_format == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;
        
    return camera_s[cam_idx].ops->set_format(cam_idx, format);
}

int kdp_camera_get_frame_format(unsigned int cam_idx, struct cam_format *format)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;
    
    if ((camera_s[cam_idx].ops->get_format == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;
        
    return camera_s[cam_idx].ops->get_format(cam_idx, format);
}

int kdp_camera_buffer_init(unsigned int cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;
    
    if ((camera_s[cam_idx].ops->buffer_init == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;
        
    return camera_s[cam_idx].ops->buffer_init(cam_idx);
}

int kdp_camera_start(unsigned int cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;
    
    if ((camera_s[cam_idx].ops->start_capture == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;

    return camera_s[cam_idx].ops->start_capture(cam_idx);
}

int kdp_camera_stop(unsigned int cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;
    
    if ((camera_s[cam_idx].ops->stop_capture == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;
        
    return camera_s[cam_idx].ops->stop_capture(cam_idx);
}

int kdp_camera_buffer_prepare(unsigned int cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;
    
    if ((camera_s[cam_idx].ops->buffer_prepare == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;
        
    return camera_s[cam_idx].ops->buffer_prepare(cam_idx);
}

int kdp_camera_buffer_capture(unsigned int cam_idx, int *addr, int *size)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;
    
    if ((camera_s[cam_idx].ops->buffer_capture == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;
        
    return camera_s[cam_idx].ops->buffer_capture(cam_idx, addr, size);
}

int kdp_camera_stream_on(unsigned int cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;
    
    if ((camera_s[cam_idx].ops->stream_on == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;
        
    return camera_s[cam_idx].ops->stream_on(cam_idx);
}

int kdp_camera_stream_off(unsigned int cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;
    
    if ((camera_s[cam_idx].ops->stream_off == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;
        
    return camera_s[cam_idx].ops->stream_off(cam_idx);
}

int kdp_camera_set_gain(unsigned int cam_idx, int gain1, int gain2)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;

    if ((camera_s[cam_idx].ops->set_gain == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;

    return camera_s[cam_idx].ops->set_gain(cam_idx, gain1, gain2);
}

int kdp_camera_set_exp_time(unsigned int cam_idx, int gain1, int gain2)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;

    if ((camera_s[cam_idx].ops->set_exp_time == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;

    return camera_s[cam_idx].ops->set_exp_time(cam_idx, gain1, gain2);
}

int kdp_camera_get_lux(unsigned int cam_idx, u16* exposure, u8* pre_gain, u8* post_gain, u8* global_gain, u8* y_average)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;

    if ((camera_s[cam_idx].ops->get_lux == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;

    return camera_s[cam_idx].ops->get_lux(cam_idx, exposure, pre_gain, post_gain, global_gain, y_average);
}

int kdp_camera_set_aec_roi(unsigned int cam_idx, u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;

    if ((camera_s[cam_idx].ops->set_aec_roi == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;

    return camera_s[cam_idx].ops->set_aec_roi(cam_idx, x1, x2, y1, y2, center_x1, center_x2, center_y1, center_y2);
}

int kdp_camera_set_mirror(unsigned int cam_idx, BOOL enable)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;

    if ((camera_s[cam_idx].ops->set_mirror == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;

    return camera_s[cam_idx].ops->set_mirror(cam_idx, enable);
}

int kdp_camera_set_led(unsigned int cam_idx, BOOL enable)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;

    if ((camera_s[cam_idx].ops->set_led == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;

    return camera_s[cam_idx].ops->set_led(cam_idx, enable);
}

int kdp_camera_set_fps(unsigned int cam_idx, u8 fps)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;

    if ((camera_s[cam_idx].ops->set_fps == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;

    return camera_s[cam_idx].ops->set_fps(cam_idx, fps);
}


int kdp_camera_set_flip(unsigned int cam_idx, BOOL enable)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;

    if ((camera_s[cam_idx].ops->set_flip == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;

    return camera_s[cam_idx].ops->set_flip(cam_idx, enable);
}

int kdp_camera_get_device_id(unsigned int cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;
    
    if ((camera_s[cam_idx].ops->get_device_id == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;
        
    return camera_s[cam_idx].ops->get_device_id(cam_idx);
}

int kdp_camera_get_tile_en(unsigned int cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;
    
    if ((camera_s[cam_idx].ops->get_tile_en == NULL) || camera_s[cam_idx].ops == NULL)
        return -1;

    return camera_s[cam_idx].ops->get_tile_en(cam_idx);
}
int kdp_camera_controller_register(unsigned int cam_idx, struct cam_ops *cam_ops_p)
{
    if (cam_idx >= IMGSRC_NUM || cam_ops_p == NULL)
        return -1;

    if (camera_s[cam_idx].inuse)
        return -1;

    camera_s[cam_idx].ops = cam_ops_p;
    camera_s[cam_idx].inuse = 1;

    return 0;
}

int kdp_camera_controller_unregister(unsigned int cam_idx, struct cam_ops *cam_ops_p)
{
    if (cam_idx >= IMGSRC_NUM || cam_ops_p == NULL)
        return -1;

    if (!camera_s[cam_idx].inuse)
        return -1;

    camera_s[cam_idx].ops = NULL;
    camera_s[cam_idx].inuse = 0;

    return 0;
}
#if CFG_ONE_SHOT_MODE == YES
int kdp_camera_set_sleep(unsigned int cam_idx, BOOL enable)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;

    if (camera_s[cam_idx].ops->sleep == NULL)
        return -1;

    return camera_s[cam_idx].ops->sleep(cam_idx, enable);
}
#endif

int kdp_camera_set_aec_en(unsigned int cam_idx, BOOL enable)
{
    if (cam_idx >= IMGSRC_NUM)
        return -1;
		
    if (camera_s[cam_idx].ops->set_aec_en == NULL)
        return -1;
				
    return camera_s[cam_idx].ops->set_aec_en(cam_idx, enable);
}

