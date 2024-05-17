/*
 * KDP Frame Buffer driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include <string.h>
#include "cmsis_os2.h"
#include "board_kl520.h"
#include "kdp_memory.h"
#include "kdp_fb_mgr.h"
#include "dbg.h"
#include "framework/irqflags.h"
#include "RTX_Config.h"
#include "kdp_ddr_table.h"
#include "kl520_include.h"
#include "kdp_camera.h"
#include "kdp520_tmr.h"
//#define FOLLOW_INFERENCE_FIRST
//#define READ_FROM_ISR
//#define FB_DEBUG
//#define FB_DEBUG2

#ifdef FB_DEBUG
#define fb_msg(fmt, ...) dbg_msg("[%8.8d] " fmt, osKernelGetTickCount(), ##__VA_ARGS__)
#else
#define fb_msg(fmt, ...)
#endif

#if (FB_USE_SEM_LOCK == 1)
#define local_irq_disable() //
#define local_irq_enable() //

static osStatus_t _fb_sem_lock(osSemaphoreId_t sem, uint32_t tm)
{
    return osSemaphoreAcquire(sem, tm);
}

static osStatus_t _fb_sem_release(osSemaphoreId_t sem)
{
    return osSemaphoreRelease(sem);
}

#else
static inline osStatus_t _fb_sem_lock(osSemaphoreId_t sem, uint32_t tm)
{
    return osOK;
}

static inline osStatus_t _fb_sem_release(osSemaphoreId_t sem)
{
    return osOK;
}

#endif

#if FB_CTX_ENABLE == YES
struct kdp_fb_ctx_s {
    uint32_t    buf_addr;
    uint32_t    buf_seq_num;
    struct frame_info info;
};
#endif
static struct kdp_fb_mgr_s {
    int         buf_size;
    int         buf_num;
#if FB_CTX_ENABLE == YES
    struct kdp_fb_ctx_s fb_ctx[MAX_FRAME_BUFFER];
#else
    uint32_t    buf_addr[MAX_FRAME_BUFFER];
    uint32_t    buf_seq_num[MAX_FRAME_BUFFER];
#endif
    struct      frame_info latest_info;

#if (FB_TILE_RECODE == YES)
    int         latest_write_idx;
    int         tile_calc_idx;
    struct      frame_record_info _record_info[MAX_FRAME_BUFFER];
#endif
    
    int         write_idx;      // camera
    int         write_next_idx; // camera
    int         write_done_idx; // camera

    int         inf_idx;        // inference
    int         inf_done_idx;   // inference

    int         read_idx;       // display

    int         aec_idx;       // aec

    uint32_t    write_count;
    uint32_t    read_count;
    uint32_t    inf_count;
    uint32_t    aec_count;
    fb_write_done_notify    write_done_cb;
    osSemaphoreId_t _sem_fb;

} kdp_frame_buffer[IMGSRC_NUM];

static struct kdp_fb_mgr_s *fb_p[IMGSRC_NUM] = {
    &kdp_frame_buffer[0],
#if (IMGSRC_NUM > 1)
    &kdp_frame_buffer[1],
#endif
};

#if CFG_ONE_SHOT_MODE == YES
uint32_t kdp_fb_mgr_buf_addr( int cam_idx, uint8_t idx )
{
    if (cam_idx >= IMGSRC_NUM)
        return 0; 

    dbg_msg_console("[%s] idx : %d", __func__, idx);
    return fb_p[cam_idx]->buf_addr[idx];
}
#endif

static inline void inc_write_next_idx(struct kdp_fb_mgr_s *fbp)
{
    fbp->write_next_idx = (fbp->write_next_idx + 1) % fbp->buf_num;
}

int kdp_fb_mgr_init(int cam_idx, uint32_t buf_size, int buf_num)
{
    if (cam_idx >= IMGSRC_NUM)
        return 0; 
        
    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];
    int i, size;

#if USE_GUARDBAND
    size = buf_size + (2 * buf_size / USE_GUARDBAND);
#else
    size = buf_size;
#endif

    if (buf_num <= 0 || buf_num > MAX_FRAME_BUFFER)
        buf_num = MAX_FRAME_BUFFER;

#if FB_CTX_ENABLE == YES
    for (i = 0; i < buf_num; i++) {
        fbp->fb_ctx[i].buf_addr = kdp_ddr_reserve(size);
#if USE_GUARDBAND
        fbp->fb_ctx[i].buf_addr += (buf_size / USE_GUARDBAND);
#endif
        //fb_msg("  cam %d: frame buf[%d] : 0x%x", cam_idx, i, fbp->buf_addr[i]);
    }
#else
    for (i = 0; i < buf_num; i++) {
        fbp->buf_addr[i] = kdp_ddr_reserve(size);
#if USE_GUARDBAND
        fbp->buf_addr[i] += (buf_size / USE_GUARDBAND);
#endif
        //fb_msg("  cam %d: frame buf[%d] : 0x%x", cam_idx, i, fbp->buf_addr[i]);
    }
#endif
    fbp->write_idx = fbp->read_idx = fbp->inf_idx = -1;
    fbp->write_done_idx = fbp->write_next_idx = fbp->inf_done_idx = -1;
    fbp->buf_size = buf_size;
    fbp->buf_num = buf_num;
#if (FB_USE_SEM_LOCK == 1)
    if(fbp->_sem_fb == NULL) fbp->_sem_fb = osSemaphoreNew(1, 1, NULL);
#endif


    return 0;
}

uint32_t kdp_fb_mgr_next_write(int cam_idx, int *write_idx) //used only during init
{
    if (cam_idx >= IMGSRC_NUM)
        return 0; 
        
    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];

    osStatus_t sem_rt = _fb_sem_lock(fbp->_sem_fb, 5000);
    local_irq_disable();

    inc_write_next_idx(fbp);

    if (fbp->write_idx == -1)
        fbp->write_idx = fbp->write_next_idx;

    fb_msg("[%d]%s: %d\n", cam_idx, __func__, fbp->write_next_idx);

    // skip the buffer currently for inference or being read
    if (fbp->write_next_idx == fbp->inf_idx) {
        inc_write_next_idx(fbp);
        if (fbp->write_next_idx == fbp->read_idx)
            inc_write_next_idx(fbp);
    } else if (fbp->write_next_idx == fbp->read_idx) {
        inc_write_next_idx(fbp);
        if (fbp->write_next_idx == fbp->inf_idx)
            inc_write_next_idx(fbp);
    }

    if (write_idx)
        *write_idx = fbp->write_next_idx;

    local_irq_enable();
    uint32_t tmp = fbp->buf_addr[fbp->write_next_idx];
    if(sem_rt == osOK) _fb_sem_release(fbp->_sem_fb);
#if FB_CTX_ENABLE == YES
    return fbp->fb_ctx[fbp->write_next_idx].buf_addr;
#else
    return tmp;
#endif
}

int kdp_fb_mgr_write_done(int cam_idx, int write_idx, unsigned char *tile_val) //not used any more
{
    if (cam_idx >= IMGSRC_NUM)
        return 0; 
        
    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];

    local_irq_disable();

    fb_msg("[%d]%s: %d\n", cam_idx, __func__, write_idx);

    if (fbp->inf_idx != -1 ) {
        if (fbp->write_next_idx == fbp->read_idx) {
            fb_msg("\n [%d]%s: [%d done] w-next:%d == read_idx (prev w-done:%d, inf:%d, inf-done:%d) !\n",
                cam_idx, __func__, write_idx, fbp->write_next_idx, fbp->write_done_idx,
                fbp->inf_idx, fbp->inf_done_idx);
        } else if (fbp->write_next_idx == fbp->inf_idx) {
            fb_msg("\n [%d]%s: [%d done] w-next:%d == inf_idx %d !!\n",
                cam_idx, __func__, write_idx, fbp->write_next_idx, fbp->inf_idx);
        }
    }


#ifdef FB_DEBUG2
    if (fbp->write_count < 3) {
        uint32_t *addr0, *addr1, *addr2;
#if FB_CTX_ENABLE == YES
        addr0 = (uint32_t *)fbp->fb_ctx[0].buf_addr + 0x100;
        addr1 = (uint32_t *)fbp->fb_ctx[1].buf_addr + 0x100;
        addr2 = (uint32_t *)fbp->fb_ctx[2].buf_addr + 0x100;
#else
        addr0 = (uint32_t *)fbp->buf_addr[0] + 0x100;
        addr1 = (uint32_t *)fbp->buf_addr[1] + 0x100;
        addr2 = (uint32_t *)fbp->buf_addr[2] + 0x100;
#endif

        fb_msg("[%d] write-done: %d: [0]=0x%x, [1]=0x%x, [2]=0x%x",
            cam_idx, write_idx, *addr0, *addr1, *addr2);
        *addr0 = 1;
        *addr1 = 2;
        *addr2 = 4;
    }
#endif

    /* Mark with sequential number */
    fbp->write_count++;
#if FB_CTX_ENABLE == YES
    fbp->fb_ctx[write_idx].buf_seq_num = fbp->write_count;
#else
    fbp->buf_seq_num[write_idx] = fbp->write_count;
#endif
    fbp->write_done_idx = write_idx;
    fbp->write_idx = fbp->write_next_idx;
#if FB_CTX_ENABLE == YES
    memcpy(fbp->fb_ctx[fbp->write_done_idx].info.tile_val, fbp->latest_info.tile_val, 20);
#endif
    if (fbp->write_done_cb)
        fbp->write_done_cb(cam_idx, write_idx);

    local_irq_enable();

    return 0;
}

uint32_t kdp_fb_mgr_buffer_write_next(int cam_idx, int* write_idx, unsigned char *tile_val) //new func
{
    if (cam_idx >= IMGSRC_NUM)
        return 0; 
        
    if(write_idx == NULL) return 0;
    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];

    if(_fb_sem_lock(fbp->_sem_fb, 0) != osOK) return 0; //in case of fb is busy, skip this interrupt
    local_irq_disable();
    
    if(*write_idx != -1) {
        fbp->write_count++;

        fbp->buf_seq_num[*write_idx] = fbp->write_count;

        fbp->write_done_idx = *write_idx;
        fbp->write_idx = fbp->write_next_idx;

        if (fbp->write_done_cb)
            fbp->write_done_cb(cam_idx, *write_idx);
    }

    inc_write_next_idx(fbp);

    if (fbp->write_idx == -1)
        fbp->write_idx = fbp->write_next_idx;

    fb_msg("[%d]%s: %d\n", cam_idx, __func__, fbp->write_next_idx);

    // skip the buffer currently for inference or being read
    if (fbp->write_next_idx == fbp->inf_idx) {
        inc_write_next_idx(fbp);
        if (fbp->write_next_idx == fbp->read_idx)
            inc_write_next_idx(fbp);
    } else if (fbp->write_next_idx == fbp->read_idx) {
        inc_write_next_idx(fbp);
        if (fbp->write_next_idx == fbp->inf_idx)
            inc_write_next_idx(fbp);
    }

    *write_idx = fbp->write_next_idx;
    
    uint32_t tmp = fbp->buf_addr[fbp->write_next_idx];
    local_irq_enable();
    _fb_sem_release(fbp->_sem_fb);
    
    return tmp;
}

void kdp_fb_mgr_free_write_buf(int cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return;
    
    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];
    int i;

    osStatus_t sem_rt = _fb_sem_lock(fbp->_sem_fb, 5000);
    local_irq_disable();

    fbp->write_idx = -1;
    fbp->write_next_idx = -1;
    fbp->write_done_idx = -1;
    fbp->write_count = 0;

#if FB_CTX_ENABLE == YES
#ifdef FB_DEBUG2
    fbp->write_count = 0;   // for test purposes

    // memset((void *)fbp->fb_ctx[0].buf_addr, 0, fbp->buf_size);
    // memset((void *)fbp->fb_ctx[1].buf_addr, 0, fbp->buf_size);
    // memset((void *)fbp->fb_ctx[2].buf_addr, 0, fbp->buf_size);
#else
    // memset((void *)fbp->fb_ctx[0].buf_addr, 0, fbp->buf_size);
    // memset((void *)fbp->fb_ctx[1].buf_addr, 0, fbp->buf_size);
#endif
    for (i = 0; i < fbp->buf_num; i++) {
        fbp->fb_ctx[i].buf_seq_num = 0;
    }   
#else
#ifdef FB_DEBUG2
    fbp->write_count = 0;   // for test purposes

    // memset((void *)fbp->buf_addr[0], 0, fbp->buf_size);
    // memset((void *)fbp->buf_addr[1], 0, fbp->buf_size);
    // memset((void *)fbp->buf_addr[2], 0, fbp->buf_size);
#else
    // memset((void *)fbp->buf_addr[0], 0, fbp->buf_size);
    // memset((void *)fbp->buf_addr[1], 0, fbp->buf_size);
#endif
    for (i = 0; i < fbp->buf_num; i++) {
        fbp->buf_seq_num[i] = 0;
    }   
#endif

    fb_msg("[%d]%s\n", cam_idx, __func__);

    local_irq_enable();
    if(sem_rt == osOK) _fb_sem_release(fbp->_sem_fb);
}

uint32_t kdp_fb_mgr_get_buf_seq_num(int cam_idx, int idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return 0;
    
    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];
#if FB_CTX_ENABLE == YES
    return fbp->fb_ctx[idx].buf_seq_num;
#else
    return fbp->buf_seq_num[idx];
#endif
}

uint32_t kdp_fb_mgr_get_current_buf_seq_num(int cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return 0;
    
    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];

    return fbp->write_count;
}

uint32_t kdp_fb_mgr_get_buf(int cam_idx, int idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return 0; 
        
    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];

    local_irq_disable();

    idx %= fbp->buf_num;
#if FB_CTX_ENABLE == YES
    uint32_t buf_addr = fbp->fb_ctx[idx].buf_addr;
#else
    uint32_t buf_addr = fbp->buf_addr[idx];
#endif
    memset((void *)buf_addr, 0, fbp->buf_size);

    fb_msg("[%d]%s: %d\n", cam_idx, __func__, idx);

    local_irq_enable();

    return buf_addr;
}

void kdp_fb_mgr_get_latest_frame_info(int cam_idx, struct frame_info *info)
{
    if (cam_idx >= IMGSRC_NUM)
        return; 
        
    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];

    local_irq_disable();

    memcpy(info, &fbp->latest_info, sizeof(struct frame_info));

    fb_msg("[%d]%s: %d\n", cam_idx, __func__);

    local_irq_enable();
}

void kdp_fb_mgr_set_frame_info(int cam_idx, struct frame_info *info)
{
    if (cam_idx >= IMGSRC_NUM)
        return; 
        
    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];

    local_irq_disable();

#if FB_CTX_ENABLE == YES
    memcpy(&fbp->fb_ctx[fbp->write_done_idx].info, info, sizeof(struct frame_info));
#else
    memcpy(&fbp->latest_info, info, sizeof(struct frame_info));
#endif

    fb_msg("[%s] %d idx:%d\n", __func__, cam_idx, fbp->write_done_idx);

    local_irq_enable();
}
#if (FB_TILE_RECODE == YES)
void kdp_fb_mgr_get_frame_record_info(int cam_idx, int idx, struct frame_record_info *info)
{
    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];

    local_irq_disable();

    memcpy(info, &fbp->_record_info[idx], sizeof(struct frame_record_info));

    fb_msg("[%d]%s: %d\n", cam_idx, __func__);

    local_irq_enable();
}

void kdp_fb_mgr_set_frame_record_rst(int cam_idx)
{
    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];

    local_irq_disable();

    for(int i=0; i<MAX_FRAME_BUFFER; i++){
        
        memset(&fbp->_record_info[i].tile.tile_val, 0, sizeof(struct frame_info));
            
        fbp->_record_info[i].fb_idx  = -1;
        fbp->_record_info[i]._tick_isr_time_diff  = -1;
        fbp->_record_info[i]._tick_fdr_str = 0;
        fbp->_record_info[i]._tile_value_mean = 0;
        fbp->_record_info[i]._tick_value_mean = 0;
    }
        
    local_irq_enable();
}

void kdp_fb_mgr_set_frame_record_cam_isr_tick(int cam_idx, int idx, uint32_t tick)
{
    if (cam_idx >= IMGSRC_NUM)
        return ; 
        
    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];

    local_irq_disable();

    fbp->_record_info[idx].fb_idx = fbp->latest_write_idx = idx;
    fbp->_record_info[idx]._tick_isr_cam = tick;
    fbp->_record_info[idx]._tick_isr_time_diff = -1;
    fbp->_record_info[idx]._tile_value_mean = 0;
    fbp->_record_info[idx]._tick_value_mean = 0;
    
    
    local_irq_enable();
}

//kdp_e2e_nir_tile_value_mean
void kdp_fb_mgr_set_tile_value_mean_tick(int cam_idx, u8 tile_value)
{
    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];
    int idx = fbp->tile_calc_idx;

    
    fbp->_record_info[idx]._tile_value_mean = tile_value;
    fbp->_record_info[idx]._tick_value_mean = GetCurrentT3Tick();
}

bool kdp_fb_mgr_tile_delay(int cam_idx, int idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return false;

    #define TILE_BREAK_NUM  (10)
    u16 tile_delay_cnt = 0;
    
    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];

    if(fbp->_record_info[idx].fb_idx == -1)
        return true;
    
    if(kdp_camera_get_tile_en(cam_idx) == 1)    //enable, to be check
    {
        while(1)
        {
            if(fbp->_record_info[idx]._tick_isr_time_diff >= 0)
                break;
            
            if(tile_delay_cnt > TILE_BREAK_NUM){ break; }
            else { tile_delay_cnt++; }
            
            osDelay(1);

        }
    
    if(tile_delay_cnt > TILE_BREAK_NUM )
        dbg_msg_console("%s: delay %d ms",  __func__, tile_delay_cnt);

        memcpy(&fbp->latest_info, &fbp->_record_info[idx].tile, sizeof(struct frame_info));
    }

    fbp->_record_info[idx]._tick_fdr_str = GetCurrentT3Tick();
    fbp->tile_calc_idx = idx; 
    return false;
}

void kdp_fb_mgr_set_frame_record_cam_isr_tile(int cam_idx, uint32_t tick, struct frame_info *info)
{
    if (cam_idx >= IMGSRC_NUM)
        return 0;

    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];
    uint32_t isr_tick = fbp->_record_info[fbp->latest_write_idx]._tick_isr_cam; 
    local_irq_disable();

    fbp->_record_info[fbp->latest_write_idx]._tick_isr_tile = tick;
    
    if(tick >= isr_tick){
        fbp->_record_info[fbp->latest_write_idx]._tick_isr_time_diff = tick - isr_tick;        
        memcpy(&fbp->_record_info[fbp->latest_write_idx].tile.tile_val, info, sizeof(struct frame_info));
    }
    else
        fbp->_record_info[fbp->latest_write_idx]._tick_isr_time_diff = -1;
    
    local_irq_enable();
}
#endif
uint32_t kdp_fb_mgr_next_read(int cam_idx, int *read_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return 0;

    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];
#ifndef READ_FROM_ISR
    int try_max_count = INF_WAIT_MAX_TIME_MS / INF_WAIT_OS_DELAY_MS;

next_read_buf:
#endif
    local_irq_disable();
    osStatus_t sem_rt = _fb_sem_lock(fbp->_sem_fb, 5000);
    if(sem_rt != osOK) return 0;

#ifdef FOLLOW_INFERENCE_FIRST
    if (fbp->inf_done_idx != -1) {
        // keep displaying the inference done buffer
        fbp->read_idx = fbp->inf_done_idx;
        fb_msg("[%d]%s: %d (inf-done)\n", cam_idx, __func__, fbp->read_idx);
    } else if (fbp->write_done_idx != -1) {
#else // Just follow write-done
    if (fbp->write_done_idx != -1) {
#endif

#if FB_CTX_ENABLE == YES
        if (fbp->fb_ctx[fbp->write_done_idx].buf_seq_num <= SKIP_FRAME_NUM)
#else
        if (fbp->buf_seq_num[fbp->write_done_idx] <= SKIP_FRAME_NUM)
#endif
        {
            if(try_max_count-- >= 0)
            {
                _fb_sem_release(fbp->_sem_fb);
                local_irq_enable();
                osDelay(INF_WAIT_OS_DELAY_TICKS);
                goto next_read_buf;
            }
        }

        fbp->read_idx = fbp->write_done_idx;
        fb_msg("[%d]%s: %d (w-done)\n", cam_idx, __func__, fbp->read_idx);
#ifdef FB_DEBUG2
        if (fbp->write_count < 3) {
            fb_msg("[%d]%s: %d (w-done)", cam_idx, __func__, fbp->read_idx);
        }
#endif
    } else {
#ifndef READ_FROM_ISR
        // let's wait for next available one
        if (try_max_count-- >= 0) {
            _fb_sem_release(fbp->_sem_fb);
            local_irq_enable();
            osDelay(INF_WAIT_OS_DELAY_TICKS);
            goto next_read_buf;
        }

        if (fbp->read_idx == -1)
            fbp->read_idx = 0;
        fb_msg("[%d]%s: reuse read_idx:%d after waiting %d ms\n",
                cam_idx, __func__, fbp->read_idx, INF_WAIT_MAX_TIME_MS);
#else
        if (fbp->read_idx == -1)
            fbp->read_idx = 0;
#endif

#ifdef FB_DEBUG2
        if (fbp->write_count < 3) {
            //fb_msg("[%d]%s: reuse read_idx:%d after waiting %d ms",
            //    cam_idx, __func__, fbp->read_idx, INF_WAIT_MAX_TIME_MS);
        }
#endif
    }

    if (read_idx)
        *read_idx = fbp->read_idx;

    fbp->read_count++;

    uint32_t ret = fbp->buf_addr[fbp->read_idx];
    local_irq_enable();
    _fb_sem_release(fbp->_sem_fb);

#if FB_CTX_ENABLE == YES
    return fbp->fb_ctx[fbp->read_idx].buf_addr;
#else
    return ret;
#endif
}

uint32_t kdp_fb_mgr_next_aec(int cam_idx, int *aec_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return 0;

    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];
#ifndef READ_FROM_ISR
    int try_max_count = INF_WAIT_MAX_TIME_MS / INF_WAIT_OS_DELAY_MS;

next_aec_buf:
#endif
    local_irq_disable();

#ifdef FOLLOW_INFERENCE_FIRST
    if (fbp->inf_done_idx != -1) {
        // keep displaying the inference done buffer
        fbp->aec_idx = fbp->inf_done_idx;
        fb_msg("[%d]%s: %d (inf-done)\n", cam_idx, __func__, fbp->aec_idx);
    } else if (fbp->write_done_idx != -1) {
#else // Just follow write-done
    if (fbp->write_done_idx != -1) {
#endif

#if FB_CTX_ENABLE == YES
        if (fbp->fb_ctx[fbp->write_done_idx].buf_seq_num <= SKIP_FRAME_NUM)
#else
        if (fbp->buf_seq_num[fbp->write_done_idx] <= SKIP_FRAME_NUM)
#endif
        {
            if(try_max_count-- >= 0)
            {
                local_irq_enable();
                osDelay(INF_WAIT_OS_DELAY_TICKS);
                goto next_aec_buf;
            }
        }

        fbp->aec_idx = fbp->write_done_idx;
        fb_msg("[%d]%s: %d (w-done)\n", cam_idx, __func__, fbp->aec_idx);
#ifdef FB_DEBUG2
        if (fbp->write_count < 3) {
            fb_msg("[%d]%s: %d (w-done)", cam_idx, __func__, fbp->aec_idx);
        }
#endif
    } else {
#ifndef READ_FROM_ISR
        // let's wait for next available one
        if (try_max_count-- >= 0) {
            local_irq_enable();
            osDelay(INF_WAIT_OS_DELAY_TICKS);
            goto next_aec_buf;
        }

        if (fbp->aec_idx == -1)
            fbp->aec_idx = 0;
        fb_msg("[%d]%s: reuse read_idx:%d after waiting %d ms\n",
                cam_idx, __func__, fbp->aec_idx, INF_WAIT_MAX_TIME_MS);
#else
        if (fbp->aec_idx == -1)
            fbp->aec_idx = 0;
#endif

#ifdef FB_DEBUG2
        if (fbp->write_count < 3) {
            //fb_msg("[%d]%s: reuse read_idx:%d after waiting %d ms",
            //    cam_idx, __func__, fbp->read_idx, INF_WAIT_MAX_TIME_MS);
        }
#endif
    }

    if (aec_idx)
        *aec_idx = fbp->aec_idx;

    fbp->aec_count++;

    local_irq_enable();

#if FB_CTX_ENABLE == YES
    return fbp->fb_ctx[fbp->aec_idx].buf_addr;
#else
    return fbp->buf_addr[fbp->aec_idx];
#endif
}

void kdp_fb_mgr_free_read_buf(int cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return ;

    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];

    osStatus_t sem_rt = _fb_sem_lock(fbp->_sem_fb, 5000);
    local_irq_disable();

    fbp->read_idx = -1;

    fb_msg("[%d]%s\n", cam_idx, __func__);

    local_irq_enable();
    if(sem_rt == osOK) _fb_sem_release(fbp->_sem_fb);
}


void kdp_fb_mgr_free_aec_buf(int cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return ;

    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];

    local_irq_disable();

    fbp->aec_idx = -1;

    fb_msg("[%d]%s\n", cam_idx, __func__);

    local_irq_enable();
}

uint32_t kdp_fb_mgr_next_inf(int cam_idx, int *inf_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return 0;

    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];

#if (KL520_QUICK_BOOT == YES)
    int try_max_count = 200;
#else
    int try_max_count = INF_WAIT_MAX_TIME_MS / INF_WAIT_OS_DELAY_MS;
#endif

next_inf_buf:
    local_irq_disable();
    // inference uses the latest write_done_idx
    //fb_msg("fbp->write_done_idx=%d fbp->inf_done_idx=%d", fbp->write_done_idx, fbp->inf_done_idx);
    
    osStatus_t sem_rt = _fb_sem_lock(fbp->_sem_fb, 5000);
    if(sem_rt != osOK) return 0; // in case no sem, return 0

    if (fbp->write_done_idx == fbp->inf_done_idx) {
        // latest write_done_idx is already used. let's wait for new one
        if (try_max_count-- >= 0) {
            _fb_sem_release(fbp->_sem_fb);
            local_irq_enable();
        #if (KL520_QUICK_BOOT == YES)
            osDelay(5);//osDelay(1);
        #else
            osDelay(INF_WAIT_OS_DELAY_TICKS);
        #endif
            goto next_inf_buf;
        }
        //fb_msg("[%d]%s: reuse w-done:%d after waiting %d ms\n",
        //        cam_idx, __func__, fbp->write_done_idx, INF_WAIT_MAX_TIME_MS);
    } else {
        //fb_msg("[%d]%s: %d\n", cam_idx, __func__, fbp->write_done_idx);
    }

    // inference uses the latest write_done_idx
    fbp->inf_idx = fbp->write_done_idx;
    if (inf_idx)
        *inf_idx = fbp->inf_idx;
    
    int tmp = fbp->inf_idx;
    uint32_t ret = fbp->buf_addr[tmp];
    local_irq_enable();
    _fb_sem_release(fbp->_sem_fb);
    
    if (tmp >= 0) {
    #if FB_CTX_ENABLE == YES
        return fbp->fb_ctx[fbp->inf_idx].buf_addr;
    #else
        return ret;
    #endif
    }
    else {
        dbg_msg_algo ("cap frm failed:%d.%d.%d.", cam_idx, fbp->inf_idx, fbp->write_done_idx);
        return 0x0;
    }
}

int kdp_fb_mgr_inf_done(int cam_idx, int inf_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return 0; 
    
    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];

    osStatus_t sem_rt = _fb_sem_lock(fbp->_sem_fb, 5000);
    local_irq_disable();

    fbp->inf_done_idx = inf_idx;
    fbp->inf_count++;

    fb_msg("[%d]%s: %d\n", cam_idx, __func__, inf_idx);

    local_irq_enable();
    if(sem_rt == osOK) _fb_sem_release(fbp->_sem_fb);

    return 0;
}

void kdp_fb_mgr_free_inf_buf(int cam_idx)
{
    if (cam_idx >= IMGSRC_NUM)
        return; 
    
    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];
    
    osStatus_t sem_rt = _fb_sem_lock(fbp->_sem_fb, 5000);

    local_irq_disable();

    fbp->inf_idx = -1;
    fbp->inf_done_idx = -1;

    fb_msg("[%d]%s\n", cam_idx, __func__);

    local_irq_enable();
    if(sem_rt == osOK) _fb_sem_release(fbp->_sem_fb);
}

int kdp_fb_mgr_notifier_register(int cam_idx, fb_write_done_notify callback)
{
    struct kdp_fb_mgr_s *fbp = fb_p[cam_idx];

    fbp->write_done_cb = callback;
    return 0;
}
