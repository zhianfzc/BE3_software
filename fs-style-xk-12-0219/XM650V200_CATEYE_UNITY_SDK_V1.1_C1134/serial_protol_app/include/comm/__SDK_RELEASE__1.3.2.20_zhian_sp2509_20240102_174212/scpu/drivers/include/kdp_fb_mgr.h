/*
 * KDP Frame Buffer driver header
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef __KDP_FB_MGR_H__
#define __KDP_FB_MGR_H__
#include <stdint.h>
#include "board_kl520.h"
#include "board_cfg.h"

#define MAX_FRAME_BUFFER        7
#define USE_GUARDBAND           64   // 1/n on both sides
#define SKIP_FRAME_NUM          0

#define INF_WAIT_MAX_TIME_MS    1000
#define INF_WAIT_OS_DELAY_MS    10
#define INF_WAIT_OS_DELAY_TICKS (OS_TICK_FREQ * INF_WAIT_OS_DELAY_MS / 1000)

#define FB_CTX_ENABLE               (NO)
#define FB_WAIT_TILE_VAL            (YES)
#define FB_TILE_RECODE              (NO)

//#if (CFG_SENSOR_0_FULL_RESOLUTION == YES || CFG_SENSOR_1_FULL_RESOLUTION == YES)
//    #define TILE_VAL_LEN    (60)
//#else
//    #define TILE_VAL_LEN    (60)
//#endif
struct frame_info
{
    uint8_t tile_val[TILE_AVG_BLOCK_NUMBER];
#if (FB_WAIT_TILE_VAL == YES)
    uint8_t _inited;
#endif
};
#if (FB_TILE_RECODE == YES)
struct frame_record_info
{

    struct      frame_info  tile; //frame buffer tile val
    int         fb_idx;
    uint32_t    _tick_isr_cam;                    //tile cam tick
    uint32_t    _tick_isr_tile;                   //tile isr tick
    int         _tick_isr_time_diff;              //time differential
    uint32_t    _tick_fdr_str;                   //fdfr str tick
    uint8_t     _tile_value_mean;
    uint32_t    _tick_value_mean;                //

};
#endif
typedef int (*fb_write_done_notify)(int cam_idx, int write_idx);

int kdp_fb_mgr_init(int cam_idx, uint32_t buf_size, int buf_num);

uint32_t kdp_fb_mgr_next_write(int cam_idx, int *write_idx);

int kdp_fb_mgr_write_done(int cam_idx, int write_idx, unsigned char *tile_val);

uint32_t kdp_fb_mgr_buffer_write_next(int cam_idx, int* write_idx, unsigned char *tile_val);

void kdp_fb_mgr_free_write_buf(int cam_idx);

uint32_t kdp_fb_mgr_get_buf(int cam_idx, int idx);

uint32_t kdp_fb_mgr_get_buf_seq_num(int cam_idx, int idx);

uint32_t kdp_fb_mgr_get_current_buf_seq_num(int cam_idx);

void kdp_fb_mgr_get_latest_frame_info(int cam_idx, struct frame_info *info);

void kdp_fb_mgr_set_frame_info(int cam_idx, struct frame_info *info);

#if (FB_TILE_RECODE == YES)

void kdp_fb_mgr_set_frame_record_rst(int cam_idx);

void kdp_fb_mgr_set_frame_record_cam_isr_tick(int cam_idx, int idx, uint32_t tick);

void kdp_fb_mgr_set_tile_value_mean_tick(int cam_idx, u8 tile_value);

bool kdp_fb_mgr_tile_delay(int cam_idx, int idx);

void kdp_fb_mgr_set_frame_record_cam_isr_tile(int cam_idx, uint32_t tick, struct frame_info *info);

void kdp_fb_mgr_get_frame_record_info(int cam_idx, int idx, struct frame_record_info *info);

#endif

uint32_t kdp_fb_mgr_next_read(int cam_idx, int *read_idx);

void kdp_fb_mgr_free_read_buf(int cam_idx);

uint32_t kdp_fb_mgr_next_aec(int cam_idx, int *aec_idx);    //for aec

void kdp_fb_mgr_free_aec_buf(int cam_idx);                  //for aec

uint32_t kdp_fb_mgr_next_inf(int cam_idx, int *inf_idx);

int kdp_fb_mgr_inf_done(int cam_idx, int inf_idx);

void kdp_fb_mgr_free_inf_buf(int cam_idx);

int kdp_fb_mgr_notifier_register(int cam_idx, fb_write_done_notify callback);

#endif // __KDP_FB_MGR_H__
