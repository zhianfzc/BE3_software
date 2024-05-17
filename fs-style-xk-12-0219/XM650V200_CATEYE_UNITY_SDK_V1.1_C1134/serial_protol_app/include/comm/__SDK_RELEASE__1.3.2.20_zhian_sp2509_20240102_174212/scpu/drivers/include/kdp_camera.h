/*
 * KDP Camera driver header
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef __KDP_CAMERA_H__
#define __KDP_CAMERA_H__

#include "types.h"


enum {
    KDP_CAM_0,
    KDP_CAM_1,
    KDP_CAM_NUM,    // = IMGSRC_NUM
};

enum camera_state {
    CAMERA_STATE_IDLE = 0,
    CAMERA_STATE_INITED,
    CAMERA_STATE_RUNNING,
    CAMERA_STATE_IN_FDR_INFERENCE,
    CAMERA_STATE_IN_FDR_REGISTRATION,
};

enum display_state {
    DISPLAY_STATE_CLOSED = 0,
    DISPLAY_STATE_OPENED,
    DISPLAY_STATE_RUNNING,
};

struct cam_format {
    unsigned int width;
    unsigned int height;
    unsigned int pixelformat;    /* fourcc */
    unsigned int field;          /* enum v2k_field */
    unsigned int bytesperline;   /* for padding, zero if unused */
    unsigned int sizeimage;
    unsigned int colorspace;     /* enum v2k_colorspace */
};

struct cam_capability {
    char driver[16];
    char desc[16];
    unsigned int version;
    unsigned int capabilities;
};

struct cam_ops {
    int (*open)(unsigned int cam_idx);
    void (*close)(unsigned int cam_idx);
    int (*set_format)(unsigned int cam_idx, struct cam_format *format);
    int (*get_format)(unsigned int cam_idx, struct cam_format *format);
    int (*buffer_init)(unsigned int cam_idx);
    int (*start_capture)(unsigned int cam_idx);
    int (*stop_capture)(unsigned int cam_idx);
    int (*buffer_prepare)(unsigned int cam_idx);
    int (*buffer_capture)(unsigned int cam_idx, int *addr, int *size);
    int (*stream_on)(unsigned int cam_idx);
    int (*stream_off)(unsigned int cam_idx);
    int (*query_capability)(unsigned int cam_idx, struct cam_capability *cap);
    int (*set_gain)(unsigned int cam_idx, u8 gain1, u8 gain2);
    int (*set_exp_time)(unsigned int cam_idx, u8 gain1, u8 gain2);
    int (*get_lux)(unsigned int cam_idx, u16* exposure, u8* pre_gain, u8* post_gain, u8* global_gain, u8* y_average);
    int (*set_aec_roi)(unsigned int cam_idx, u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2);
    int (*set_mirror)(unsigned int cam_idx, BOOL enable);
    int (*set_flip)(unsigned int cam_idx, BOOL enable);
    int (*set_led)(unsigned int cam_idx, BOOL enable);
    int (*get_device_id)(unsigned int cam_idx);
    int (*set_fps)(unsigned int cam_idx, u8 fps);
    int (*set_aec_en)(unsigned int cam_idx, BOOL enable);
	#if CFG_ONE_SHOT_MODE == YES
    int (*sleep)(unsigned int cam_idx, BOOL enable);    
	#endif
    int (*get_tile_en)(unsigned int cam_idx);
};

int kdp_camera_init(void);

int kdp_camera_open(unsigned int cam_idx);

void kdp_camera_close(unsigned int cam_idx);

int kdp_camera_query_capability(unsigned int cam_idx, struct cam_capability *cap);
int kdp_camera_get_device_info(unsigned int cam_idx, struct cam_capability *cap);

int kdp_camera_set_frame_format(unsigned int cam_idx, struct cam_format *format);

int kdp_camera_get_frame_format(unsigned int cam_idx, struct cam_format *format);

int kdp_camera_buffer_init(unsigned int cam_idx);

int kdp_camera_start(unsigned int cam_idx);

int kdp_camera_stop(unsigned int cam_idx);

int kdp_camera_buffer_prepare(unsigned int cam_idx);

int kdp_camera_buffer_capture(unsigned int cam_idx, int *addr, int *size);

int kdp_camera_stream_on(unsigned int cam_idx);

int kdp_camera_stream_off(unsigned int cam_idx);

int kdp_camera_set_gain(unsigned int cam_idx, int gain1, int gain2);

int kdp_camera_set_exp_time(unsigned int cam_idx, int gain1, int gain2);

int kdp_camera_get_lux(unsigned int cam_idx, u16* exposure, u8* pre_gain, u8* post_gain, u8* global_gain, u8* y_average);

int kdp_camera_set_aec_roi(unsigned int cam_idx, u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2);

int kdp_camera_set_mirror(unsigned int cam_idx, BOOL enable);

int kdp_camera_set_flip(unsigned int cam_idx, BOOL enable);

int kdp_camera_set_led(unsigned int cam_idx, BOOL enable);

int kdp_camera_set_fps(unsigned int cam_idx, u8 fps);

int kdp_camera_controller_register(unsigned int cam_idx, struct cam_ops *cam_ops_p);

int kdp_camera_controller_unregister(unsigned int cam_idx, struct cam_ops *cam_ops_p);

int kdp_camera_get_device_id(unsigned int cam_idx);

int kdp_camera_get_tile_en(unsigned int cam_idx);

int kdp_camera_set_aec_en(unsigned int cam_idx, BOOL enable);

#endif // __KDP_CAMERA_H__
