/*
 * KDP Camera driver for KL520
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include "board_kl520.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmsis_os2.h>
#include "kneron_mozart.h"
#include "base.h"
#include "io.h"
#include "clock.h"
#include "delay.h"
#include "scu_extreg.h"
#include "framework/v2k.h"
#include "kdp_camera.h"
#include "kdp_fb_mgr.h"
#include "kdp520_tmr.h"
#include "kdp_sensor.h"
#include "kl520_sys.h"
#include "dbg.h"
#include "kdp_e2e_face.h"
#include "kdp_e2e_camera.h"
#include "kl520_include.h"

#define CSI2RX_REG_VIDR         0x00
#define CSI2RX_REG_DIDR         0x01
#define CSI2RX_REG_CR           0x04 //Control Register
#define CSI2RX_REG_VSCR         0x05 //DPI V Sync Control Register
#define CSI2RX_REG_ECR          0x06
#define CSI2RX_REG_TCNR         0x08
#define CSI2RX_REG_HRTVR        0x0A //HS RX Timeout Value Register
#define CSI2RX_REG_FIUR         0x0B 
#define CSI2RX_REG_ITR          0x12 //Initialization Timer Register
#define CSI2RX_REG_VSTR0        0x14 //DPI VC0 V Sync Timing Register
#define CSI2RX_REG_HSTR0        0x15 //DPI VC0 H Sync Timing Register
#define CSI2RX_REG_VSTR1        0x16
#define CSI2RX_REG_HSTR1        0x17
#define CSI2RX_REG_VSTR2        0x18
#define CSI2RX_REG_HSTR2        0x19
#define CSI2RX_REG_VSTR3        0x1A
#define CSI2RX_REG_HSTR3        0x1B
#define CSI2RX_REG_MCR          0x1C //DPI Mapping Control Register
#define CSI2RX_REG_VSTER        0x1E
#define CSI2RX_REG_HPNR         0x20 //DPI Horizontal Pixel Number
#define CSI2RX_REG_PECR         0x28 //PPI Enable Control Register
#define CSI2RX_REG_DLMR         0x2A
#define CSI2RX_REG_CSIERR       0x30
#define CSI2RX_REG_INTSTS       0x33
#define CSI2RX_REG_ESR          0x34
#define CSI2RX_REG_DPISR        0x38
#define CSI2RX_REG_INTER        0x3C
#define CSI2RX_REG_FFR          0x3D
#define CSI2RX_REG_DPCMR        0x48
#define CSI2RX_REG_FRR          0x4C
#define CSI2RX_REG_PFTR         0x50
#define CSI2RX_REG_PUDTR        0x58
#define CSI2RX_REG_FRCR         0x80
#define CSI2RX_REG_FNR          0x88
#define CSI2RX_REG_BPGLR        0x90
#define CSI2RX_REG_BPGHR        0x91

#define MAX_VIDEO_MEM           8 << 20 

#define CSIRX_0_BASE            CSIRX_FTCSIRX100_PA_BASE
#define CSIRX_1_BASE            CSIRX_FTCSIRX100_1_PA_BASE

/* DPI V Sync Control Register (VSCR, Address = 0x05) */
#define CSIRX_REG_VSCR_SET_VSTU0(addr, val) SET_MASKED_BIT(addr, val, 0)

/* PPI Enable Control Register (PECR, Address = 0x28) */
#define CSIRX_REG_PECR_GET_PEC(addr)        GET_BITS(addr, 0, 7)
#define CSIRX_REG_PECR_SET_PEC(addr, val)   SET_MASKED_BITS(addr, val, 0, 7)

/* Data Lane Mapping Register 0 */
#define CSIRX_REG_0_DLMR0                   (CSIRX_0_BASE + 0x2A)
#define CSIRX_REG_0_DLMR0_GET_L1()          GET_BITS(CSIRX_REG_0_DLMR0, 4, 6)
#define CSIRX_REG_0_DLMR0_GET_L0()          GET_BITS(CSIRX_REG_0_DLMR0, 0, 2)
#define CSIRX_REG_1_DLMR0                   (CSIRX_1_BASE + 0x2A)
#define CSIRX_REG_1_DLMR0_GET_L1()          GET_BITS(CSIRX_REG_1_DLMR0, 4, 6)
#define CSIRX_REG_1_DLMR0_GET_L0()          GET_BITS(CSIRX_REG_1_DLMR0, 0, 2)


#define CSIRX_REG_0_FEATURE0                (CSIRX_0_BASE + 0x40)
#define CSIRX_REG_1_FEATURE0                (CSIRX_1_BASE + 0x40)
/* DPI FIFO Address Depth */
#define CSIRX_REG_0_FEATURE0_GET_DFAD()     GET_BITS(CSIRX_REG_0_FEATURE0, 5, 7)
#define CSIRX_REG_1_FEATURE0_GET_DFAD()     GET_BITS(CSIRX_REG_1_FEATURE0, 5, 7)
/* Lane Number */
#define CSIRX_REG_0_FEATURE0_GET_LN()       GET_BITS(CSIRX_REG_0_FEATURE0, 2, 4)
#define CSIRX_REG_1_FEATURE0_GET_LN()       GET_BITS(CSIRX_REG_1_FEATURE0, 2, 4)
/* Virtual channel number */
#define CSIRX_REG_0_FEATURE0_GET_VCN()      GET_BITS(CSIRX_REG_0_FEATURE0, 0, 1)
#define CSIRX_REG_1_FEATURE0_GET_VCN()      GET_BITS(CSIRX_REG_1_FEATURE0, 0, 1)

#define CSIRX_REG_0_FEATURE6                (CSIRX_0_BASE + 0x46)
#define CSIRX_REG_1_FEATURE6                (CSIRX_1_BASE + 0x46)
/* ASC Indicator */
#define CSIRX_REG_0_FEATURE6_GET_ASC()      GET_BIT(CSIRX_REG_0_FEATURE6, 7)
#define CSIRX_REG_1_FEATURE6_GET_ASC()      GET_BIT(CSIRX_REG_1_FEATURE6, 7)
/* APB Indicator */
#define CSIRX_REG_0_FEATURE6_GET_APB()      GET_BIT(CSIRX_REG_0_FEATURE6, 6)
#define CSIRX_REG_1_FEATURE6_GET_APB()      GET_BIT(CSIRX_REG_1_FEATURE6, 6)
/* I2C Indicator */
#define CSIRX_REG_0_FEATURE6_GET_I2C()      GET_BIT(CSIRX_REG_0_FEATURE6, 5)
#define CSIRX_REG_1_FEATURE6_GET_I2C()      GET_BIT(CSIRX_REG_1_FEATURE6, 5)
/* DPCM_EN Indicator */
#define CSIRX_REG_0_FEATURE6_GET_DPCM()     GET_BIT(CSIRX_REG_0_FEATURE6, 4)
#define CSIRX_REG_1_FEATURE6_GET_DPCM()     GET_BIT(CSIRX_REG_1_FEATURE6, 4)
/* CHK_DATA Indicator */
#define CSIRX_REG_0_FEATURE6_GET_CD()       GET_BIT(CSIRX_REG_0_FEATURE6, 3)
#define CSIRX_REG_1_FEATURE6_GET_CD()       GET_BIT(CSIRX_REG_1_FEATURE6, 3)
/* DUAL_PIXEL Indicator */
#define CSIRX_REG_0_FEATURE6_GET_DP()       GET_BIT(CSIRX_REG_0_FEATURE6, 2)
#define CSIRX_REG_1_FEATURE6_GET_DP()       GET_BIT(CSIRX_REG_1_FEATURE6, 2)
/* FRC Indicator */
#define CSIRX_REG_0_FEATURE6_GET_FRC()      GET_BIT(CSIRX_REG_0_FEATURE6, 1)
#define CSIRX_REG_1_FEATURE6_GET_FRC()      GET_BIT(CSIRX_REG_1_FEATURE6, 1)
/* DPI_PG Indicator */
#define CSIRX_REG_0_FEATURE6_GET_PG()       GET_BIT(CSIRX_REG_0_FEATURE6, 0)
#define CSIRX_REG_1_FEATURE6_GET_PG()       GET_BIT(CSIRX_REG_1_FEATURE6, 0)

#define CSIRX_REG_0_FEATURE7                (CSIRX_0_BASE + 0x47)
#define CSIRX_REG_1_FEATURE7                (CSIRX_1_BASE + 0x47)
/* OCTA_PIXEL Indicator */
#define CSIRX_REG_0_FEATURE7_GET_OP()       GET_BIT(CSIRX_REG_0_FEATURE7, 4)
#define CSIRX_REG_1_FEATURE7_GET_OP()       GET_BIT(CSIRX_REG_1_FEATURE7, 4)
/* MONO_RGB_EN Indicator */
#define CSIRX_REG_0_FEATURE7_GET_MR()       GET_BIT(CSIRX_REG_0_FEATURE7, 3)
#define CSIRX_REG_1_FEATURE7_GET_MR()       GET_BIT(CSIRX_REG_1_FEATURE7, 3)
/* CRC_CHK Indicator */
#define CSIRX_REG_0_FEATURE7_GET_CRC()      GET_BIT(CSIRX_REG_0_FEATURE7, 2)
#define CSIRX_REG_1_FEATURE7_GET_CRC()      GET_BIT(CSIRX_REG_1_FEATURE7, 2)
/* LANE_SWAP Indicator */
#define CSIRX_REG_0_FEATURE7_GET_SWAP()     GET_BIT(CSIRX_REG_0_FEATURE7, 1)
#define CSIRX_REG_1_FEATURE7_GET_SWAP()     GET_BIT(CSIRX_REG_1_FEATURE7, 1)
/* QUAD_PIXEL Indicator */
#define CSIRX_REG_0_FEATURE7_GET_QP()       GET_BIT(CSIRX_REG_0_FEATURE7, 0)
#define CSIRX_REG_1_FEATURE7_GET_QP()       GET_BIT(CSIRX_REG_1_FEATURE7, 0)


#define CSIRX_REG_0_BPGLR0                  (CSIRX_0_BASE + 0x90)
#define CSIRX_REG_1_BPGLR0                  (CSIRX_1_BASE + 0x90)
#define CSIRX_REG_0_BPGLR0_SET_VLN(val)     SET_MASKED_BITS(CSIRX_REG_0_BPGLR0, val, 0, 7)
#define CSIRX_REG_1_BPGLR0_SET_VLN(val)     SET_MASKED_BITS(CSIRX_REG_1_BPGLR0, val, 0, 7)

#define CSIRX_REG_0_BPGHR0                  (CSIRX_0_BASE + 0x91)
#define CSIRX_REG_1_BPGHR0                  (CSIRX_1_BASE + 0x91)
#define CSIRX_REG_0_BPGHR0_SET_PT(val)      SET_MASKED_BITS(CSIRX_REG_0_BPGHR0, val, 6, 7)
#define CSIRX_REG_1_BPGHR0_SET_PT(val)      SET_MASKED_BITS(CSIRX_REG_1_BPGHR0, val, 6, 7)
#define CSIRX_REG_0_BPGHR0_SET_PS(val)      SET_MASKED_BIT(CSIRX_REG_0_BPGHR0, val, 5)
#define CSIRX_REG_1_BPGHR0_SET_PS(val)      SET_MASKED_BIT(CSIRX_REG_1_BPGHR0, val, 5)
#define CSIRX_REG_0_BPGHR0_SET_GE(val)      SET_MASKED_BIT(CSIRX_REG_0_BPGHR0, val, 4)
#define CSIRX_REG_1_BPGHR0_SET_GE(val)      SET_MASKED_BIT(CSIRX_REG_1_BPGHR0, val, 4)
#define CSIRX_REG_0_BPGHR0_SET_VLN(val)     SET_MASKED_BITS(CSIRX_REG_0_BPGHR0, val, 0, 3)
#define CSIRX_REG_1_BPGHR0_SET_VLN(val)     SET_MASKED_BITS(CSIRX_REG_1_BPGHR0, val, 0, 3)

#define DPI2AHB_PAGE_NUM            2           // # of pages by one controller

#define D2A_REG_CTRL            0x00        // Control
#define D2A_REG_FNC             0x04        // Frame Number Control
#define D2A_REG_P0ADDR          0x08        // Page 0 Address
#define D2A_REG_P1ADDR          0x0C        // Page 1 Address
#define D2A_REG_ICT             0x10        // Interrupt Control
#define D2A_REG_IS              0x14        // Interrupt Status
#define D2A_REG_ST              0x18        // Status
#define D2A_REG_PT              0x1C        // Packet Type
#define D2A_REG_FIU0            0x20        // FI Use 0
#define D2A_REG_FIU1            0x24        // FI Use 1
#define D2A_REG_TAVR            0x28        // Tile Average Result (n)

/* D2A interrupt control & status register */
#define D2A_INT_TILE_AVG_D       BIT5
#define D2A_INT_AHB_TX_ERR       BIT4
#define D2A_INT_FIFO_UF          BIT3
#define D2A_INT_FIFO_OF          BIT2
#define D2A_INT_FN_OL            BIT1
#define D2A_INT_WRD              BIT0

#define D2A_INT_ALL              0x3F

/* D2A status register */
#define D2A_ST_PG               0x3

/* D2A Packet type register */
#define D2A_PT_YUV422           0x1E
#define D2A_PT_RGB565           0x22
#define D2A_PT_RGB888           0x24
#define D2A_PT_RAW8             0x2A
#define D2A_PT_RAW10            0x2B
#define D2A_PT_RAW12            0x2C
#define D2A_PT_RAW14            0x2D
#define D2A_PT_RAW16            0x2E

//#define TILE_AVG_SIZE_128       0x00000000
//#define TILE_AVG_SIZE_64        BIT16
//#define TILE_AVG_SIZE_32        BIT17
//#define TILE_AVG_SIZE_VAL       TILE_AVG_SIZE_128

#if ( TILE_AVG_CAL_SIZE == TILE_AVG_128 )
#define D2A_TILE_AVG_SIZE       0x00000000
#elif ( TILE_AVG_CAL_SIZE == TILE_AVG_64 )
#define D2A_TILE_AVG_SIZE       BIT16
#elif ( TILE_AVG_CAL_SIZE == TILE_AVG_32 )
#define D2A_TILE_AVG_SIZE       BIT17
#endif

#define NUM_DPI2AHB_ISR_SKIP    (2)
#define DPI2AHB_RESET_ENABLE    (YES)

struct kdp520_cam_context {
    int             id;
    int             irq;
    osThreadId_t    tid;
    int             inited;

    u32 dpi2ahb_base;
    u32 csi_rx_base;
    u32 phy_csr_base;
    int page_done_num;
    u8  page_done_count;

    u32 frame_buf_addr[DPI2AHB_PAGE_NUM];
    int page_num;

    u32 sensor_id;

    struct cam_format fmt;

    u32 width;
    u32 height;
    u32 bpp;

    u32 pixelformat;
    u32 old_addr;
    u32 curr_addr;

    u32 capabilities;

    int mipi_lane_num;
    int tile_avg_en;
};

struct kdp520_cam_context cam_ctx[KDP_CAM_NUM];
static int buf0_idx[2] = {-1,-1};
static int buf1_idx[2] = {-1,-1};

#if (IMGSRC_0_TYPE==SENSOR_TYPE_GC2145) || (IMGSRC_1_TYPE==SENSOR_TYPE_GC2145)
extern void gc2145_sensor_init(unsigned int cam_idx);
#endif
#if (IMGSRC_0_TYPE==SENSOR_TYPE_SC132GS) || (IMGSRC_1_TYPE==SENSOR_TYPE_SC132GS)
extern void sc132gs_sensor_init(unsigned int cam_idx);
#elif (IMGSRC_0_TYPE==SENSOR_TYPE_SC035HGS) || (IMGSRC_1_TYPE==SENSOR_TYPE_SC035HGS)
extern void sc035hgs_sensor_init(unsigned int cam_idx);
#endif
#if (IMGSRC_0_TYPE == SENSOR_TYPE_GC1054_L || IMGSRC_1_TYPE == SENSOR_TYPE_GC1054_L)
extern void gc1054_l_sensor_init(int cam_idx);
#endif

#if (IMGSRC_0_TYPE == SENSOR_TYPE_GC1054_R || IMGSRC_1_TYPE == SENSOR_TYPE_GC1054_R)
extern void gc1054_r_sensor_init(int cam_idx);
#endif

#if (IMGSRC_0_TYPE == SENSOR_TYPE_OV9282_R || IMGSRC_1_TYPE == SENSOR_TYPE_OV9282_R)
extern void ov9282_r_sensor_init(int cam_idx);
#endif

#if (IMGSRC_0_TYPE == SENSOR_TYPE_OV9282_L || IMGSRC_1_TYPE == SENSOR_TYPE_OV9282_L)
extern void ov9282_l_sensor_init(int cam_idx);
#endif

#if (IMGSRC_0_TYPE == SENSOR_TYPE_GC02M1_L || IMGSRC_1_TYPE == SENSOR_TYPE_GC02M1_L)
extern void gc02m1_l_sensor_init(int cam_idx);
#endif

#if (IMGSRC_0_TYPE == SENSOR_TYPE_GC02M1_R || IMGSRC_1_TYPE == SENSOR_TYPE_GC02M1_R)
extern void gc02m1_r_sensor_init(int cam_idx);
#endif

#if (IMGSRC_0_TYPE == SENSOR_TYPE_MIXO3238 || IMGSRC_1_TYPE == SENSOR_TYPE_MIXO3238)
extern void mixo3238_sensor_init(int cam_idx);
#endif
#if (IMGSRC_0_TYPE == SENSOR_TYPE_BF20A1_R || IMGSRC_1_TYPE == SENSOR_TYPE_BF20A1_R)
extern void bf20a1_r_sensor_init(int cam_idx); 
#endif
#if (IMGSRC_0_TYPE == SENSOR_TYPE_BF20A1_L || IMGSRC_1_TYPE == SENSOR_TYPE_BF20A1_L)
extern void bf20a1_l_sensor_init(int cam_idx); 
#endif
#if (IMGSRC_0_TYPE == SENSOR_TYPE_OV02B1B_R || IMGSRC_1_TYPE == SENSOR_TYPE_OV02B1B_R)
extern void ov02b1b_r_sensor_init(int cam_idx); 
#endif
#if (IMGSRC_0_TYPE == SENSOR_TYPE_OV02B1B_L || IMGSRC_1_TYPE == SENSOR_TYPE_OV02B1B_L)
extern void ov02b1b_l_sensor_init(int cam_idx); 
#endif
#if (IMGSRC_0_TYPE == SENSOR_TYPE_SP2509_R || IMGSRC_1_TYPE == SENSOR_TYPE_SP2509_R)
extern void sp2509_r_sensor_init(int cam_idx); 
#endif
#if (IMGSRC_0_TYPE == SENSOR_TYPE_SP2509_L || IMGSRC_1_TYPE == SENSOR_TYPE_SP2509_L)
extern void sp2509_l_sensor_init(int cam_idx); 
#endif

#if CALC_CAMERA_FPS == YES
#include "kdp520_tmr.h"
u32 rcnt = 0; u32 rfstmp = 0;
u32 ncnt = 0; u32 nfstmp = 0;
static void _calculate_camera_fps(int cam_idx)
{
    u32 avg_stmp = 0;
    float fps;
    if (MIPI_CAM_RGB == cam_idx) {
        if (0 == rcnt) {
            rfstmp = GetCurrentT3Tick();
        }
        else if (rcnt % 100 == 0)
        {
            avg_stmp = (GetCurrentT3Tick() - rfstmp) / 99; 
            fps = 1000 / (float)avg_stmp;
            dbg_msg_console("CCF-RGB (%u) avg_stmp=%u fps=%f", rcnt, avg_stmp, fps);
            rfstmp = GetCurrentT3Tick();
        }
        ++rcnt;
    }
    else if (MIPI_CAM_NIR == cam_idx) {
        if (0 == ncnt) {
            nfstmp = GetCurrentT3Tick();
        }
        else if (ncnt % 100 == 0)
        {
            avg_stmp = (GetCurrentT3Tick() - nfstmp) / 99;
            fps = 1000 / (float)avg_stmp;
            dbg_msg_console("CCF-NIR (%u) avg_stmp=%u fps=%f", ncnt, avg_stmp, fps);
            nfstmp = GetCurrentT3Tick();
        }
        ++ncnt;
    }
}
#endif

#if (MEASURE_RECOGNITION == YES)
int rgb_cnt = 0;
int nir_cnt = 0;

static void _kl520_measure_stamp_camera_isr(int cam_idx)
{
    if(cam_idx == MIPI_CAM_NIR)
    {
        ++nir_cnt;
        switch (nir_cnt) {
        case 1: kl520_measure_stamp(E_MEASURE_NIR_ISR_01); break;
        case 2: kl520_measure_stamp(E_MEASURE_NIR_ISR_02); break;
        case 3: kl520_measure_stamp(E_MEASURE_NIR_ISR_03); break;
        case 4: kl520_measure_stamp(E_MEASURE_NIR_ISR_04); break;
        case 5: kl520_measure_stamp(E_MEASURE_NIR_ISR_05); break;
        case 6: kl520_measure_stamp(E_MEASURE_NIR_ISR_06); break;
        case 7: kl520_measure_stamp(E_MEASURE_NIR_ISR_07); break;
        case 8: kl520_measure_stamp(E_MEASURE_NIR_ISR_08); break;
        case 9: kl520_measure_stamp(E_MEASURE_NIR_ISR_09); break;
        case 10: kl520_measure_stamp(E_MEASURE_NIR_ISR_10); break;
        }
    }
    else if(cam_idx == MIPI_CAM_RGB)
    {
        ++rgb_cnt;
        switch (rgb_cnt) {
        case 1: kl520_measure_stamp(E_MEASURE_RGB_ISR_01); break;
        case 2: kl520_measure_stamp(E_MEASURE_RGB_ISR_02); break;
        case 3: kl520_measure_stamp(E_MEASURE_RGB_ISR_03); break;
        case 4: kl520_measure_stamp(E_MEASURE_RGB_ISR_04); break;
        case 5: kl520_measure_stamp(E_MEASURE_RGB_ISR_05); break;
        case 6: kl520_measure_stamp(E_MEASURE_RGB_ISR_06); break;
        case 7: kl520_measure_stamp(E_MEASURE_RGB_ISR_07); break;
        case 8: kl520_measure_stamp(E_MEASURE_RGB_ISR_08); break;
        case 9: kl520_measure_stamp(E_MEASURE_RGB_ISR_09); break;
        case 10: kl520_measure_stamp(E_MEASURE_RGB_ISR_10); break;
        }
    }

}
#endif

static void camera_isr(struct kdp520_cam_context *ctx)
{   
    u32 sta_is, sta_st;
    u32 buf_addr[2];
    int cam_idx = ctx->id;
    u32 _ntick = GetCurrentT3Tick();
    sta_is = inw(ctx->dpi2ahb_base + D2A_REG_IS);
    sta_st = inw(ctx->dpi2ahb_base + D2A_REG_ST);
    
    // dbg_msg_console("sensor_id[%d] sta_is:%#x", ctx->sensor_id, sta_is);
    if (sta_is & D2A_INT_WRD) {

        //dbg_msg_camera("camera_isr cam_idx=%d sta_st=%x", cam_idx, sta_st);
        if (1)  //(ctx->page_done_num != sta_st) // 1: page 1 done 2: page 0 done
        {
            if(cam_idx == MIPI_CAM_NIR){ nir_sensor_wait_effect();}
            if(cam_idx == MIPI_CAM_RGB){ rgb_sensor_wait_effect();}     

            //dbg_msg_console(" ctx->page_done_count: %d" , ctx->page_done_count );
            if(ctx->page_done_count >= NUM_DPI2AHB_ISR_SKIP)
            {
                kdp_e2e_face_variables *vars = kdp_e2e_get_face_variables();
                
                #if (MEASURE_RECOGNITION == YES)
                _kl520_measure_stamp_camera_isr(cam_idx);
                #endif

                if( vars->rgb_img_ready == CAM_IMAGE_STATE_NULL && cam_idx == MIPI_CAM_RGB) vars->rgb_img_ready = CAM_IMAGE_STATE_ISR;

                if( (vars->step_cnt_wait_led_gain > 0 && cam_idx == MIPI_CAM_NIR ) 
                 || (vars->step_cnt_rgb_wait_effect > 0 && cam_idx == MIPI_CAM_RGB ) ) {
                    goto skip_buf;
                }
     
                if (sta_st == BIT1) {
#if (FB_TILE_RECODE == YES)
                    kdp_fb_mgr_set_frame_record_cam_isr_tick(cam_idx, buf1_idx[cam_idx], _ntick);
#endif
                    //sta_st=2
                    buf_addr[cam_idx] = kdp_fb_mgr_buffer_write_next(cam_idx, &buf1_idx[cam_idx], vars->info.tile_val);
                    if(buf_addr[cam_idx]) {
                        outw(ctx->dpi2ahb_base + D2A_REG_P0ADDR, buf_addr[cam_idx]);
                    }
                    
                #if CALC_CAMERA_FPS == YES
                    _calculate_camera_fps(cam_idx);
                #endif
                }else if (sta_st == BIT0) {
#if (FB_TILE_RECODE == YES)
                    kdp_fb_mgr_set_frame_record_cam_isr_tick(cam_idx, buf0_idx[cam_idx], _ntick);
#endif
                    //sta_st=1
                    buf_addr[cam_idx] = kdp_fb_mgr_buffer_write_next(cam_idx, &buf0_idx[cam_idx], vars->info.tile_val);
                    if(buf_addr[cam_idx]) {
                        outw(ctx->dpi2ahb_base + D2A_REG_P1ADDR, buf_addr[cam_idx]);
                    }

                #if CALC_CAMERA_FPS == YES
                    _calculate_camera_fps(cam_idx);
                #endif
                }else{
                    dbg_msg_console("isr sta_st error %d" , sta_st );
                }
            }
            else{
                ctx->page_done_count++;
            }
skip_buf:
            ctx->page_done_num = sta_st;
        }
    }

    
    if (ctx->tile_avg_en && ctx->page_done_count >= NUM_DPI2AHB_ISR_SKIP) {
        if (sta_is & D2A_INT_TILE_AVG_D) {

            kdp_e2e_face_variables *vars = kdp_e2e_get_face_variables();
            
            if( (vars->step_cnt_wait_led_gain > 0 && cam_idx == MIPI_CAM_NIR ) 
             || (vars->step_cnt_rgb_wait_effect > 0 && cam_idx == MIPI_CAM_RGB ) ) {
                goto skip_tile;
            }

            int k = 0;
            struct frame_info info;

#if (ALL_TILE_VALUE == NO)  //only for SC035
            for (int i = 0; i < 5; i++)
            {
                u32 tmp = inw(ctx->dpi2ahb_base + D2A_REG_TAVR + i * 10);
                info.tile_val[k++] = tmp & 0xff;
                info.tile_val[k++] = (tmp & 0xff00) >> 8;
                info.tile_val[k++] = (tmp & 0xff0000) >> 16;
                info.tile_val[k++] = (tmp & 0xff000000) >> 24;
            }
            //    for (int i = 0; i < 5; ++i) {
            //        dbg_msg_camera("camera_isr line%d %x %x %x %x", i,
            //        vars->info.tile_val[i*4 + 0], vars->info.tile_val[i*4 + 1], vars->info.tile_val[i*4 + 2], vars->info.tile_val[i*4 + 3]);
            //    }
#else
            for (int i = 0; i < 15; i++)  //get tile value 4*15
            {
                u32 tmp = inw(ctx->dpi2ahb_base + D2A_REG_TAVR + i * 4);
                info.tile_val[k++] = tmp & 0xff;
                info.tile_val[k++] = (tmp & 0xff00) >> 8;
                info.tile_val[k++] = (tmp & 0xff0000) >> 16;
                info.tile_val[k++] = (tmp & 0xff000000) >> 24;
            }
//            for (int i = 0; i < 6; i++)
//            {
//                dbg_msg_console("camera_isr line %d:  %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d", i,
//                  vars->info.tile_val[i*10 + 0], vars->info.tile_val[i*10 + 1], vars->info.tile_val[i*10 + 2], vars->info.tile_val[i*10 + 3],
//                  vars->info.tile_val[i*10 + 4], vars->info.tile_val[i*10 + 5], vars->info.tile_val[i*10 + 6], vars->info.tile_val[i*10 + 7],
//                  vars->info.tile_val[i*10 + 8], vars->info.tile_val[i*10 + 9]);
//            }
#endif
#if FB_WAIT_TILE_VAL == YES
            info._inited = 1;
#endif
#if (FB_TILE_RECODE == YES)
            kdp_fb_mgr_set_frame_record_cam_isr_tile(cam_idx, _ntick, &info);
#else
            kdp_fb_mgr_set_frame_info(cam_idx, &info);
#endif
#if 0
            for (int i = 0; i < 15; i++) {
                u32 reg = ctx->dpi2ahb_base + D2A_REG_TAVR + i * 4;
                u32 val = inw(reg);
                dbg_msg_console("reg = 0x%08x, val = 0x%08x", reg, val);
            }
            
            u32 err_code = inw(CSIRX_FTCSIRX100_1_PA_BASE+0x38);
                
            if(err_code != 0 )
                dbg_msg_console("DDE!,err=%x",err_code);
            else
                dbg_msg_console("DDE PASS!,err=%x",err_code);
#endif
        }
skip_tile:
        outw(ctx->dpi2ahb_base + D2A_REG_FNC, 0x2);
    }

    outw(ctx->dpi2ahb_base + D2A_REG_IS, sta_is);
}

void camera_isr_0(void)
{   
    struct kdp520_cam_context *ctx = &cam_ctx[KDP_CAM_0];

    camera_isr(ctx);
}

void camera_isr_1(void)
{
    struct kdp520_cam_context *ctx = &cam_ctx[KDP_CAM_1];

    camera_isr(ctx);
}

#if(DPI2AHB_RESET_ENABLE == YES)
void dpi2ahb_reset(struct kdp520_cam_context *ctx)
{
    u32 prev_read, read;
    ctx->page_done_num = 0;
    prev_read = inw(ctx->dpi2ahb_base + D2A_REG_CTRL);
    read = (prev_read | BIT0);
    outw(ctx->dpi2ahb_base + D2A_REG_CTRL, read);  //reset
    read = inw(ctx->dpi2ahb_base + D2A_REG_CTRL);

    if(prev_read != read)
        dbg_msg_console("Reset D2A_REG_CTRL error =%#x, %#x", prev_read, read);
}
#endif

void dpi2ahb_enable(struct kdp520_cam_context *ctx)
{
    u32 val = 0;
    switch (ctx->sensor_id) {
#if 0
    case SENSOR_TYPE_HMX2056:
        val |= 0x3000;
        break;
    case SENSOR_TYPE_OV9286:
        val |= 0x3008;
        break;
    case SENSOR_TYPE_HMXRICA:
        val |= 0x3008;
        break;
#endif
    case SENSOR_TYPE_GC2145:
#if GC2145_FULL_RES == YES
            val |= 0x6000;
#else
            val |= 0x3000;
#endif
        break;
    case SENSOR_TYPE_SC132GS:
        val |= 0x3000;
        break;
#ifdef SENSOR_TYPE_BF20A1_R    
    case SENSOR_TYPE_BF20A1_R:
        val |= 0x3000;
        break;      
#endif      
#ifdef SENSOR_TYPE_BF20A1_L
    case SENSOR_TYPE_BF20A1_L:
        val |= 0x3000;
        break;
#endif       
    case SENSOR_TYPE_SC035HGS:
        val |= 0x3000;
        break;
#if defined(SENSOR_TYPE_GC1054_R) || defined(SENSOR_TYPE_GC1054_L)
#ifdef SENSOR_TYPE_GC1054_R
    case SENSOR_TYPE_GC1054_R:
#endif
#ifdef SENSOR_TYPE_GC1054_L
    case SENSOR_TYPE_GC1054_L:
#endif
        val |= 0x3000;
        break;
#endif

#if defined(SENSOR_TYPE_OV9282_R) || defined(SENSOR_TYPE_OV9282_L)
#ifdef SENSOR_TYPE_OV9282_R
    case SENSOR_TYPE_OV9282_R:
#endif
#ifdef SENSOR_TYPE_OV9282_L
    case SENSOR_TYPE_OV9282_L:
#endif
        val |= 0x2000;
        break;
#endif
    
#ifdef SENSOR_TYPE_GC02M1_L    
    case SENSOR_TYPE_GC02M1_L:
        val |= 0x1000;  
        break;    
#endif
#ifdef SENSOR_TYPE_GC02M1_R    
    case SENSOR_TYPE_GC02M1_R:
        val |= 0x1000; 
        break;	    
#endif
#ifdef SENSOR_TYPE_MIXO3238
    case SENSOR_TYPE_MIXO3238:
        val |= 0x5000;
        break;
#endif
#ifdef SENSOR_TYPE_OV02B1B_R    
    case SENSOR_TYPE_OV02B1B_R:
#if CFG_OV02B1B_12MHZ_ENABLE == 1
      val |= 0x2000;
#else
      val |= 0x5000;
#endif
      break;
#endif
#ifdef SENSOR_TYPE_SP2509_R    
    case SENSOR_TYPE_SP2509_R:
      val |= 0x3000;
      break;
#endif
#ifdef SENSOR_TYPE_SP2509_L    
    case SENSOR_TYPE_SP2509_L:
      val |= 0x3000;
      break;
#endif
#ifdef SENSOR_TYPE_OV02B1B_L    
    case SENSOR_TYPE_OV02B1B_L:
#if CFG_OV02B1B_12MHZ_ENABLE == 1
      val |= 0x2000;
#else
      val |= 0x5000;
#endif
      break;
#endif
    }

    if (ctx->tile_avg_en) {
        val |= D2A_TILE_AVG_SIZE;
        outw(ctx->dpi2ahb_base + D2A_REG_FNC, 0x2);
        outw(ctx->dpi2ahb_base + D2A_REG_CTRL, val);
        outw(ctx->dpi2ahb_base + D2A_REG_ICT, D2A_INT_WRD | D2A_INT_TILE_AVG_D);
    } else {
        outw(ctx->dpi2ahb_base + D2A_REG_FNC, 0x0);
        outw(ctx->dpi2ahb_base + D2A_REG_CTRL, val);
        outw(ctx->dpi2ahb_base + D2A_REG_ICT, D2A_INT_WRD);
    }
    
    

#ifdef SOURCE_FROM_PATTERN
    if (MIPI_CSI2RX_0 == ctx->id) {
    #if IMGSRC_0_FORMAT == IMAGE_FORMAT_YCBCR
        outw(ctx->dpi2ahb_base + D2A_REG_PT, D2A_PT_YUV422);
    #elif IMGSRC_0_FORMAT == IMAGE_FORMAT_RGB565
        outw(ctx->dpi2ahb_base + D2A_REG_PT, D2A_PT_RGB565);
    #elif IMGSRC_0_FORMAT == IMAGE_FORMAT_RAW10
        outw(ctx->dpi2ahb_base + D2A_REG_PT, D2A_PT_RAW10);
    #elif IMGSRC_0_FORMAT == IMAGE_FORMAT_RAW8
        outw(ctx->dpi2ahb_base + D2A_REG_PT, D2A_PT_RAW8);
    #endif
    }
    else if (MIPI_CSI2RX_1 == ctx->id) {
    #if IMGSRC_1_FORMAT == IMAGE_FORMAT_YCBCR
        outw(ctx->dpi2ahb_base + D2A_REG_PT, D2A_PT_YUV422);
    #elif IMGSRC_1_FORMAT == IMAGE_FORMAT_RGB565
        outw(ctx->dpi2ahb_base + D2A_REG_PT, D2A_PT_RGB565);
    #elif IMGSRC_1_FORMAT == IMAGE_FORMAT_RAW10
        outw(ctx->dpi2ahb_base + D2A_REG_PT, D2A_PT_RAW10);
    #elif IMGSRC_1_FORMAT == IMAGE_FORMAT_RAW8
        outw(ctx->dpi2ahb_base + D2A_REG_PT, D2A_PT_RAW8);
    #endif
    }
#else
    switch (ctx->fmt.pixelformat) {
    case V2K_PIX_FMT_YCBCR:
        dbg_msg_camera("[%s] V2K_PIX_FMT_YCBCR\n", __func__);
        outw(ctx->dpi2ahb_base + D2A_REG_PT, D2A_PT_YUV422);
        break;        
    case V2K_PIX_FMT_RGB565:
        dbg_msg_camera("[%s] V2K_PIX_FMT_RGB565\n", __func__);
        outw(ctx->dpi2ahb_base + D2A_REG_PT, D2A_PT_RGB565);
        break;
    case V2K_PIX_FMT_RAW10:	
        dbg_msg_camera("[%s] V2K_PIX_FMT_RAW10\n", __func__);
        outw(ctx->dpi2ahb_base + D2A_REG_PT, D2A_PT_RAW10);
        break;
    case V2K_PIX_FMT_RAW8:
        dbg_msg_camera("[%s] V2K_PIX_FMT_RAW8\n", __func__);
        outw(ctx->dpi2ahb_base + D2A_REG_PT, D2A_PT_RAW8);
        break;
    default:;
    }
#endif

    outw(ctx->dpi2ahb_base + D2A_REG_FIU0, 0x1);
}

#define ROUND_UP(x, y) ((((x) + (y - 1)) / y) * y)

static void csi2rx_init(struct kdp520_cam_context *ctx)
{ 
    int val, width;

    width = ROUND_UP(ctx->fmt.width, 4);
    outw(ctx->csi_rx_base + CSI2RX_REG_HPNR , width);
      
    val = inw(ctx->csi_rx_base + CSI2RX_REG_MCR);
    val = (val&(~0xff)) | 0x00;//0x22;
    outw(ctx->csi_rx_base + CSI2RX_REG_MCR , val);

#ifdef SOURCE_FROM_PATTERN    
    CSIRX_REG_PECR_SET_PEC(ctx->csi_rx_base + CSI2RX_REG_PECR, 1);
    
    //outw(ctx->csi_rx_base + CSI2RX_REG_BPGLR, 0x01); 
    val = (inw(ctx->csi_rx_base + CSI2RX_REG_BPGLR) & 0xFF);
    dbg_msg_camera(" CSI2RX_REG_BPGLR val=%x\n", val);
    
    val = (inw(ctx->csi_rx_base + CSI2RX_REG_BPGHR) & 0x0F);
        //val = 0;

    if (MIPI_CSI2RX_0 == ctx->id) {
    #if IMGSRC_0_FORMAT == IMAGE_FORMAT_RGB565
        val |= 0x40;         //RAW10
    #elif IMGSRC_0_FORMAT == IMAGE_FORMAT_RAW10
        val |= 0x40;         //RAW10
    #elif IMGSRC_0_FORMAT == IMAGE_FORMAT_RAW8
        val |= 0x00;         //RAW8
    #endif
    }
    else if (MIPI_CSI2RX_1 == ctx->id) {
    #if IMGSRC_1_FORMAT == IMAGE_FORMAT_RGB565
        val |= 0x40;         //RAW10
    #elif IMGSRC_1_FORMAT == IMAGE_FORMAT_RAW10
        val |= 0x40;         //RAW10
    #elif IMGSRC_1_FORMAT == IMAGE_FORMAT_RAW8
        val |= 0x00;         //RAW8
    #endif        
    }
    //val |= 0x20; //swap          
    outw(ctx->csi_rx_base + CSI2RX_REG_BPGHR, val); 
    val = inw(ctx->csi_rx_base + CSI2RX_REG_BPGHR);
#endif    

    switch (ctx->sensor_id) {
#if 0
    case SENSOR_TYPE_OV9286:
    case SENSOR_TYPE_HMXRICA:
#endif        

#if defined(SENSOR_TYPE_OV9282_R) || defined(SENSOR_TYPE_OV9282_L)
#ifdef SENSOR_TYPE_OV9282_R
    case SENSOR_TYPE_OV9282_R:
#endif
#ifdef SENSOR_TYPE_OV9282_L
    case SENSOR_TYPE_OV9282_L:
#endif
       //val = inw(ctx->csi_rx_base + CSI2RX_REG_PFTR);
        val = (inw(ctx->csi_rx_base + CSI2RX_REG_VSTR0)) & 0xff00;
        val |= 0x02;
        outw(ctx->csi_rx_base + CSI2RX_REG_VSTR0 , val);
    
        val = (inw(ctx->csi_rx_base + CSI2RX_REG_VSTER));
        val |= 0x09;
        outw(ctx->csi_rx_base + CSI2RX_REG_VSTER , val);
    
        val = (inw(ctx->csi_rx_base + CSI2RX_REG_VSCR));
        val |= 0x01;
        outw(ctx->csi_rx_base + CSI2RX_REG_VSCR , val);
    
        val = 245;//100; // * 4 = 400 pixels
        outw(ctx->csi_rx_base + CSI2RX_REG_PFTR , val);
        break;
#endif

    case SENSOR_TYPE_SC132GS:
       //val = inw(ctx->csi_rx_base + CSI2RX_REG_PFTR);
        val = (inw(ctx->csi_rx_base + CSI2RX_REG_VSTR0)) & 0xff00;
        val |= 0x02;
        outw(ctx->csi_rx_base + CSI2RX_REG_VSTR0 , val);
    
        val = (inw(ctx->csi_rx_base + CSI2RX_REG_VSTER));
        val |= 0x09;
        outw(ctx->csi_rx_base + CSI2RX_REG_VSTER , val);
    
        val = (inw(ctx->csi_rx_base + CSI2RX_REG_VSCR));
        val |= 0x01;
        outw(ctx->csi_rx_base + CSI2RX_REG_VSCR , val);
    
        val = 0xff;//100; // * 4 = 400 pixels
        outw(ctx->csi_rx_base + CSI2RX_REG_PFTR , val);
        break;
#ifdef SENSOR_TYPE_GC02M1_L
    case SENSOR_TYPE_GC02M1_L:
#endif
#ifdef SENSOR_TYPE_GC02M1_R
    case SENSOR_TYPE_GC02M1_R:
#endif
#ifdef SENSOR_TYPE_GC1054_R
    case SENSOR_TYPE_GC1054_R:
#endif
#ifdef SENSOR_TYPE_GC1054_L
    case SENSOR_TYPE_GC1054_L:
#endif
#ifdef SENSOR_TYPE_BF20A1_R
    case SENSOR_TYPE_BF20A1_R:
#endif
#ifdef SENSOR_TYPE_BF20A1_L
    case SENSOR_TYPE_BF20A1_L:      
      
      outb(ctx->phy_csr_base + 0x11, 0x2);   
        
      //val = inw(ctx->csi_rx_base + CSI2RX_REG_PFTR);
      val = (inw(ctx->csi_rx_base + CSI2RX_REG_VSTR0)) & 0xff00;
      val |= 0x02;
      outw(ctx->csi_rx_base + CSI2RX_REG_VSTR0 , val);

      val = (inw(ctx->csi_rx_base + CSI2RX_REG_VSTER));
      val |= 0x02;
      outw(ctx->csi_rx_base + CSI2RX_REG_VSTER , val);
      val = 0xff;//100; // * 4 = 400 pixels
      outw(ctx->csi_rx_base + CSI2RX_REG_PFTR , val);
      val = (inw(ctx->csi_rx_base + CSI2RX_REG_VSCR));
      val |= 0x01;
      outw(ctx->csi_rx_base + CSI2RX_REG_VSCR , val);
      break;  
          
#endif

#ifdef SENSOR_TYPE_OV02B1B_L
    case SENSOR_TYPE_OV02B1B_L:
#endif
#ifdef SENSOR_TYPE_OV02B1B_R
    case SENSOR_TYPE_OV02B1B_R:
#endif
#ifdef SENSOR_TYPE_SP2509_R
    case SENSOR_TYPE_SP2509_R:
#endif
#ifdef SENSOR_TYPE_SP2509_L
    case SENSOR_TYPE_SP2509_L:
#endif
        if (ctx->id == 0)
            outb(MIPIRX_PHY_CSR_PA_BASE + 0x11, 0x7);
        else if (ctx->id == 1)
            outb(MIPIRX_PHY_CSR_1_PA_BASE + 0x11, 0x7);

        // val = inw(ctx->csi_rx_base + CSI2RX_REG_PFTR);
        val = (inw(ctx->csi_rx_base + CSI2RX_REG_VSTR0)) & 0xff00;
        val |= 0x02;
        outw(ctx->csi_rx_base + CSI2RX_REG_VSTR0, val);

        val = (inw(ctx->csi_rx_base + CSI2RX_REG_VSTER));
        val |= 0x02;
        outw(ctx->csi_rx_base + CSI2RX_REG_VSTER, val);
        val = 0xff; // 100; // * 4 = 400 pixels
        outw(ctx->csi_rx_base + CSI2RX_REG_PFTR, val);
        val = (inw(ctx->csi_rx_base + CSI2RX_REG_VSCR));
        val |= 0x01;
        outw(ctx->csi_rx_base + CSI2RX_REG_VSCR, val);
        break;

#ifdef SENSOR_TYPE_MIXO3238    
    case SENSOR_TYPE_MIXO3238:    
#endif         
    case SENSOR_TYPE_SC035HGS:
	   //val = inw(ctx->csi_rx_base + CSI2RX_REG_PFTR);
		val = (inw(ctx->csi_rx_base + CSI2RX_REG_VSTR0)) & 0xff00;
		val |= 0x02;
		outw(ctx->csi_rx_base + CSI2RX_REG_VSTR0 , val);

		val = (inw(ctx->csi_rx_base + CSI2RX_REG_VSTER));
		val |= 0x09;
		outw(ctx->csi_rx_base + CSI2RX_REG_VSTER , val);

		val = (inw(ctx->csi_rx_base + CSI2RX_REG_VSCR));
		val |= 0x01;
		outw(ctx->csi_rx_base + CSI2RX_REG_VSCR , val);

		val = 0xff;//100; // * 4 = 400 pixels
		outw(ctx->csi_rx_base + CSI2RX_REG_PFTR , val);
		break;
 
   
    case SENSOR_TYPE_GC2145:
        val = (inw(ctx->csi_rx_base + CSI2RX_REG_VSTR0)) & 0xff00;
        val |= 0x05;
        outw(ctx->csi_rx_base + CSI2RX_REG_VSTR0 , val);
    
        val = (inw(ctx->csi_rx_base + CSI2RX_REG_VSTER));
        val |= 0x08;
        outw(ctx->csi_rx_base + CSI2RX_REG_VSTER , val);
    
        val = (inw(ctx->csi_rx_base + CSI2RX_REG_VSCR));
        val |= 0x01;
        outw(ctx->csi_rx_base + CSI2RX_REG_VSCR , val);
    
        val = (inw(ctx->csi_rx_base + CSI2RX_REG_PFTR));
        val = 0x30;
        outw(ctx->csi_rx_base + CSI2RX_REG_PFTR , val);
        break;
    default:;
    }
    val = inw(ctx->csi_rx_base + CSI2RX_REG_CR);
    val = (val&(~0xff)) | 0x0d; 
    outw(ctx->csi_rx_base + CSI2RX_REG_CR , val);
    ctx->page_done_count = 0;
    delay_us(1000);
}

static void csi2rx_power(struct kdp520_cam_context *ctx, int on)
{
    uint32_t mask, val = 0;

    if (ctx->id == 0) {
        mask = (SCU_EXTREG_CSIRX_CTRL0_ClkLnEn | SCU_EXTREG_CSIRX_CTRL0_Enable);
        if (on)
            val = mask;

        masked_outw(SCU_EXTREG_CSIRX_CTRL0, val, mask);
    } else if (ctx->id == 1) {
        mask = (SCU_EXTREG_CSIRX_CTRL1_ClkLnEn | SCU_EXTREG_CSIRX_CTRL1_Enable);
        if (on)
            val = mask;

        masked_outw(SCU_EXTREG_CSIRX_CTRL1, val, mask);
    }
}

static void csi2rx_reset(struct kdp520_cam_context *ctx)
{
    if (ctx->id == 0) {
        masked_outw( SCU_EXTREG_CSIRX_CTRL0,
                    ((SCU_EXTREG_CSIRX_CTRL0_ClkLnEn | SCU_EXTREG_CSIRX_CTRL0_Enable) |
                     (SCU_EXTREG_CSIRX_CTRL0_apb_rst_n | SCU_EXTREG_CSIRX_CTRL0_pwr_rst_n |
                      SCU_EXTREG_CSIRX_CTRL0_sys_rst_n)),
                    ((SCU_EXTREG_CSIRX_CTRL0_ClkLnEn | SCU_EXTREG_CSIRX_CTRL0_Enable) |
                     (SCU_EXTREG_CSIRX_CTRL0_apb_rst_n | SCU_EXTREG_CSIRX_CTRL0_pwr_rst_n |
                      SCU_EXTREG_CSIRX_CTRL0_sys_rst_n)));

        masked_outw( SCU_EXTREG_CSIRX_CTRL1,
                    ((SCU_EXTREG_CSIRX_CTRL1_ClkLnEn | SCU_EXTREG_CSIRX_CTRL1_Enable) |
                     (SCU_EXTREG_CSIRX_CTRL1_apb_rst_n | SCU_EXTREG_CSIRX_CTRL1_pwr_rst_n |
                      SCU_EXTREG_CSIRX_CTRL1_sys_rst_n)),
                    ((SCU_EXTREG_CSIRX_CTRL1_ClkLnEn | SCU_EXTREG_CSIRX_CTRL1_Enable) |
                     (SCU_EXTREG_CSIRX_CTRL1_apb_rst_n | SCU_EXTREG_CSIRX_CTRL1_pwr_rst_n |
                      SCU_EXTREG_CSIRX_CTRL1_sys_rst_n)));

        masked_outw( SCU_EXTREG_DPI2AHB_CTRL,
                    (SCU_EXTREG_DPI2AHB_CTRL_rst_n | SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n |
                     SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n | SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n),
                    (SCU_EXTREG_DPI2AHB_CTRL_rst_n | SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n |
                     SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n | SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n));

        masked_outw( SCU_EXTREG_DPI2AHB_CTRL,
                    (SCU_EXTREG_DPI2AHB_CTRL_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n_1 |
                     SCU_EXTREG_DPI2AHB_CTRL_sys_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n_1),
                    (SCU_EXTREG_DPI2AHB_CTRL_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n_1 |
                     SCU_EXTREG_DPI2AHB_CTRL_sys_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n_1));

        outb(MIPIRX_PHY_CSR_PA_BASE + 0x11, 0x7);
    
    } else if (ctx->id == 1) {
        masked_outw( SCU_EXTREG_CSIRX_CTRL0,
                    ((SCU_EXTREG_CSIRX_CTRL0_ClkLnEn | SCU_EXTREG_CSIRX_CTRL0_Enable) |
                     (SCU_EXTREG_CSIRX_CTRL0_apb_rst_n | SCU_EXTREG_CSIRX_CTRL0_pwr_rst_n |
                      SCU_EXTREG_CSIRX_CTRL0_sys_rst_n)),
                    ((SCU_EXTREG_CSIRX_CTRL0_ClkLnEn | SCU_EXTREG_CSIRX_CTRL0_Enable) |
                     (SCU_EXTREG_CSIRX_CTRL0_apb_rst_n | SCU_EXTREG_CSIRX_CTRL0_pwr_rst_n |
                      SCU_EXTREG_CSIRX_CTRL0_sys_rst_n)));

        masked_outw( SCU_EXTREG_CSIRX_CTRL1,
                    ((SCU_EXTREG_CSIRX_CTRL1_ClkLnEn | SCU_EXTREG_CSIRX_CTRL1_Enable) |
                     (SCU_EXTREG_CSIRX_CTRL1_apb_rst_n | SCU_EXTREG_CSIRX_CTRL1_pwr_rst_n |
                      SCU_EXTREG_CSIRX_CTRL1_sys_rst_n)),
                    ((SCU_EXTREG_CSIRX_CTRL1_ClkLnEn | SCU_EXTREG_CSIRX_CTRL1_Enable) |
                     (SCU_EXTREG_CSIRX_CTRL1_apb_rst_n | SCU_EXTREG_CSIRX_CTRL1_pwr_rst_n |
                      SCU_EXTREG_CSIRX_CTRL1_sys_rst_n)));

        masked_outw( SCU_EXTREG_DPI2AHB_CTRL,
                    (SCU_EXTREG_DPI2AHB_CTRL_rst_n | SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n |
                     SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n | SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n),
                    (SCU_EXTREG_DPI2AHB_CTRL_rst_n | SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n |
                     SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n | SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n));

        masked_outw( SCU_EXTREG_DPI2AHB_CTRL,
                    (SCU_EXTREG_DPI2AHB_CTRL_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n_1 |
                     SCU_EXTREG_DPI2AHB_CTRL_sys_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n_1),
                    (SCU_EXTREG_DPI2AHB_CTRL_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n_1 |
                     SCU_EXTREG_DPI2AHB_CTRL_sys_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n_1));


        masked_outw( SCU_EXTREG_CSIRX_CTRL0,
                    (SCU_EXTREG_CSIRX_CTRL0_apb_rst_n | SCU_EXTREG_CSIRX_CTRL0_pwr_rst_n |
                     SCU_EXTREG_CSIRX_CTRL0_sys_rst_n),
                    (SCU_EXTREG_CSIRX_CTRL0_apb_rst_n | SCU_EXTREG_CSIRX_CTRL0_pwr_rst_n |
                     SCU_EXTREG_CSIRX_CTRL0_sys_rst_n));

        masked_outw( SCU_EXTREG_CSIRX_CTRL1,
                    (SCU_EXTREG_CSIRX_CTRL1_apb_rst_n | SCU_EXTREG_CSIRX_CTRL1_pwr_rst_n |
                     SCU_EXTREG_CSIRX_CTRL1_sys_rst_n ),
                    (SCU_EXTREG_CSIRX_CTRL1_apb_rst_n | SCU_EXTREG_CSIRX_CTRL1_pwr_rst_n |
                     SCU_EXTREG_CSIRX_CTRL1_sys_rst_n ));

        //dpi2ahb_1 reset
        masked_outw( SCU_EXTREG_DPI2AHB_CTRL,
                    (SCU_EXTREG_DPI2AHB_CTRL_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n_1 |
                     SCU_EXTREG_DPI2AHB_CTRL_sys_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n_1),

                    (SCU_EXTREG_DPI2AHB_CTRL_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n_1 |
                     SCU_EXTREG_DPI2AHB_CTRL_sys_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n_1));


        outb(MIPIRX_PHY_CSR_1_PA_BASE + 0x11, 0x7);

    }
    ctx->page_done_count = 0;
}

static int csi2rx_clock_start(struct kdp520_cam_context *ctx)
{
    dbg_msg_camera("%s: cam %d", __func__, ctx->id);

    if (ctx->id == KDP_CAM_0) {
        clk_enable(CLK_MIPI_CSIRX0_CSI_CLK);
        clk_enable(CLK_MIPI_CSIRX0_VC0_CLK);
        clk_enable(CLK_MIPI_CSIRX0_TXESC_CLK);
    }
    else if (ctx->id == KDP_CAM_1) {
        clk_enable(CLK_MIPI_CSIRX1_CSI_CLK);
        clk_enable(CLK_MIPI_CSIRX1_VC0_CLK);
        clk_enable(CLK_MIPI_CSIRX1_TXESC_CLK);
    }
    delay_us(3500);
    return 0;
}

static void csi2rx_clock_stop(struct kdp520_cam_context *ctx)
{
    dbg_msg_camera("%s: cam %d", __func__, ctx->id);

    if (ctx->id == KDP_CAM_0) {
        masked_outw(SCU_EXTREG_CSIRX_CTRL0,
                    0,
                   (SCU_EXTREG_CSIRX_CTRL0_ClkLnEn | SCU_EXTREG_CSIRX_CTRL0_Enable));

        clk_disable(CLK_MIPI_CSIRX0_CSI_CLK);
        clk_disable(CLK_MIPI_CSIRX0_VC0_CLK);
        clk_disable(CLK_MIPI_CSIRX0_TXESC_CLK);
    }
    else if (ctx->id == KDP_CAM_1) {
        masked_outw(SCU_EXTREG_CSIRX_CTRL1,
                    0,
                   (SCU_EXTREG_CSIRX_CTRL1_ClkLnEn | SCU_EXTREG_CSIRX_CTRL1_Enable));

        clk_disable(CLK_MIPI_CSIRX1_CSI_CLK);
        clk_disable(CLK_MIPI_CSIRX1_VC0_CLK);
        clk_disable(CLK_MIPI_CSIRX1_TXESC_CLK);
    }
    delay_us(3500);
}

/* API */

static int kdp520_cam_open(unsigned int cam_idx)
{
    struct kdp520_cam_context *ctx = &cam_ctx[cam_idx];

    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);

#if(DPI2AHB_RESET_ENABLE == YES)
    dpi2ahb_reset(ctx);
#endif
    csi2rx_power(ctx, 1);
    csi2rx_clock_start(ctx);
    csi2rx_reset(ctx);

    ctx->page_done_num = 0;

#if FB_WAIT_TILE_VAL == YES    
    struct frame_info info;
    memset(&info, 0, sizeof(struct frame_info));
    kdp_fb_mgr_set_frame_info(ctx->id, &info);
#endif
#if FB_TILE_RECODE == YES
    kdp_fb_mgr_set_frame_record_rst(ctx->id);
#endif
    return 0;
}

static void kdp520_cam_close(unsigned int cam_idx)
{
    struct kdp520_cam_context *ctx = &cam_ctx[cam_idx];

    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);
    csi2rx_clock_stop(ctx);
    csi2rx_power(ctx, 0);
}

static int kdp520_cam_query_capability(unsigned int cam_idx, struct cam_capability *cap)
{
    struct kdp520_cam_context *ctx = &cam_ctx[cam_idx];

    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);

    ctx->capabilities = V2K_CAP_VIDEO_CAPTURE | V2K_CAP_STREAMING | V2K_CAP_DEVICE_CAPS;

    strcpy(cap->driver, "kl520_camera");
    strcpy(cap->desc, "kl520_camera");
    cap->version = 0x00010001;
    cap->capabilities = ctx->capabilities;
    return 0;
}

static int kdp520_cam_set_format(unsigned int cam_idx, struct cam_format *format)
{
    struct kdp520_cam_context *ctx = &cam_ctx[cam_idx];
    int bpp;

    ctx->fmt = *format;

    if (format->pixelformat == V2K_PIX_FMT_RGB565 || format->pixelformat == V2K_PIX_FMT_RAW10)
        bpp = 2;
    else if (format->pixelformat == V2K_PIX_FMT_RAW8)
        bpp = 1;

    ctx->fmt.sizeimage = format->width * format->height * bpp;

    // critical_msg("[%s] cam %d: w=%d h=%d p=0x%x f=%d b=%d s=%d c=%d\n", __func__, cam_idx,
    //         ctx->fmt.width, ctx->fmt.height, ctx->fmt.pixelformat, ctx->fmt.field,
    //         ctx->fmt.bytesperline, ctx->fmt.sizeimage, ctx->fmt.colorspace);

    dpi2ahb_enable(ctx);
    csi2rx_init(ctx);

    return kdp_sensor_set_fmt(cam_idx, &ctx->fmt);
}

static int kdp520_cam_get_format(unsigned int cam_idx, struct cam_format *format)
{
    struct kdp520_cam_context *ctx = &cam_ctx[cam_idx];

    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);

    *format = ctx->fmt;
    return 0;
}

static int kdp520_cam_buffer_init(unsigned int cam_idx)
{
    struct kdp520_cam_context *ctx = &cam_ctx[cam_idx];

    //critical_msg("[%s] cam %d: size=%d\n", __func__, cam_idx, ctx->fmt.sizeimage);

    if (ctx->inited == 0) {
        kdp_fb_mgr_init(cam_idx, ctx->fmt.sizeimage, MAX_FRAME_BUFFER);
        ctx->inited = 1;
    }

#if(DPI2AHB_RESET_ENABLE == YES)
    buf0_idx[cam_idx] = -1;
    buf1_idx[cam_idx] = -1;
#endif

    outw(ctx->dpi2ahb_base + D2A_REG_P0ADDR, kdp_fb_mgr_next_write(cam_idx, &buf1_idx[cam_idx]));
    outw(ctx->dpi2ahb_base + D2A_REG_P1ADDR, kdp_fb_mgr_next_write(cam_idx, &buf0_idx[cam_idx]));   //mipi data put on P1ADDR at first ISR
    //outw(ctx->dpi2ahb_base + D2A_REG_FNC, 0);

    return 0;
}

static int kdp520_cam_start_capture(unsigned int cam_idx)
{
    struct kdp520_cam_context *ctx = &cam_ctx[cam_idx];
    int i;
    uint32_t val;

    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);

    val = 0;
    for (i = 0; i < ctx->mipi_lane_num; i++)
        val |= BIT0 << i;

    CSIRX_REG_PECR_SET_PEC(ctx->csi_rx_base + CSI2RX_REG_PECR, val);

    NVIC_EnableIRQ((IRQn_Type)ctx->irq);

    return 0;
}

static int kdp520_cam_stop_capture(unsigned int cam_idx)
{
    struct kdp520_cam_context *ctx = &cam_ctx[cam_idx];

    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);

    NVIC_DisableIRQ((IRQn_Type)ctx->irq);

    return 0;
}

#if 0
static int kdp520_cam_buffer_prepare(unsigned int cam_idx)
{
    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);
    return 0;
}

static int kdp520_cam_buffer_capture(unsigned int cam_idx, int *addr, int *size)
{
    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);
    return 0;
}

static int kdp520_cam_stream_on(unsigned int cam_idx)
{
    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);
//    kdp_csi_rx_stream_on(cam_idx);
    return 0;
}

static int kdp520_cam_stream_off(unsigned int cam_idx)
{
    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);
//    kdp_csi_rx_stream_off(cam_idx);
    return 0;
}
#endif

static int kdp520_cam_set_gain(unsigned int cam_idx, u8 gain1, u8 gain2)
{
    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);

    kdp_sensor_set_gain(cam_idx, gain1, gain2);

    return 0;
}

static int kdp520_cam_set_exp_time(unsigned int cam_idx, u8 gain1, u8 gain2)
{
    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);

    kdp_sensor_set_exp_time(cam_idx, gain1, gain2);

    return 0;
}

static int kdp520_cam_get_lux(unsigned int cam_idx, u16* exposure, u8* pre_gain, u8* post_gain, u8* global_gain, u8* y_average)
{
    //dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);

    kdp_sensor_get_lux(cam_idx, exposure, pre_gain, post_gain, global_gain, y_average);

    return 0;
}

static int kdp520_cam_set_aec_roi(unsigned int cam_idx, u8 x1, u8 x2, u8 y1, u8 y2, u8 center_x1, u8 center_x2, u8 center_y1, u8 center_y2)
{
    //dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);

    kdp_sensor_set_aec_roi(cam_idx, x1, x2, y1, y2, center_x1, center_x2, center_y1, center_y2);

    return 0;
}

int kdp520_cam_set_mirror(unsigned int cam_idx, BOOL enable)
{
    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);

    kdp_sensor_set_mirror(cam_idx, enable);

    return 0;
}

int kdp520_cam_set_flip(unsigned int cam_idx, BOOL enable)
{
    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);

    kdp_sensor_set_flip(cam_idx, enable);

    return 0;
}

int kdp520_cam_set_led(unsigned int cam_idx, BOOL enable)
{
    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);

    kdp_sensor_set_led(cam_idx, enable);

    return 0;
}

static int kdp520_cam_set_fps(unsigned int cam_idx, u8 fps)
{
    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);

    kdp_sensor_set_fps(cam_idx, fps);

    return 0;
}

static int kdp520_get_device_id(unsigned int cam_idx)
{
    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);

    return kdp_sensor_get_device_id(cam_idx);
}

static int kdp520_get_tile_en(unsigned int cam_idx)
{
    struct kdp520_cam_context *ctx = &cam_ctx[cam_idx];

    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);

    return ctx->tile_avg_en;
}

#if CFG_ONE_SHOT_MODE == YES
static int kdp520_cam_sleep(unsigned int cam_idx, BOOL enable)
{
    dbg_msg_camera("[%s] cam: %d, en: %d\n", __func__, cam_idx, enable);

    kdp_sensor_sleep(cam_idx, enable);

    return 0;
}
#endif

static int kdp520_set_aec_en(unsigned int cam_idx, BOOL enable)
{
    dbg_msg_camera("[%s] cam: %d\n", __func__, cam_idx);

    return kdp_sensor_set_aec_en(cam_idx, enable);
}


static struct cam_ops kdp520_camera_ops = {
    .open               = kdp520_cam_open,
    .close              = kdp520_cam_close,
    .query_capability   = kdp520_cam_query_capability,
    .set_format         = kdp520_cam_set_format,
    .get_format         = kdp520_cam_get_format,
    .buffer_init        = kdp520_cam_buffer_init,
    .start_capture      = kdp520_cam_start_capture,
    .stop_capture       = kdp520_cam_stop_capture,
//    .buffer_prepare     = kdp520_cam_buffer_prepare,
//    .buffer_capture     = kdp520_cam_buffer_capture,
//    .stream_on          = kdp520_cam_stream_on,
//    .stream_off         = kdp520_cam_stream_off,
    .set_gain           = kdp520_cam_set_gain,
    .set_exp_time       = kdp520_cam_set_exp_time,
    .get_lux            = kdp520_cam_get_lux,
    .set_aec_roi        = kdp520_cam_set_aec_roi,
    .set_mirror         = kdp520_cam_set_mirror,
    .set_flip           = kdp520_cam_set_flip,
    .set_led           	= kdp520_cam_set_led,
    .get_device_id      = kdp520_get_device_id,
    .set_fps            = kdp520_cam_set_fps,
    .set_aec_en         = kdp520_set_aec_en,
#if CFG_ONE_SHOT_MODE == YES	
    .sleep              = kdp520_cam_sleep,     
#endif	
    .get_tile_en        = kdp520_get_tile_en,
};

void sys_camera_global_init(void)
{
    //critical_msg("   <%s>\n", __func__);

    SCU_EXTREG_CLK_EN1_SET_csirx0_TxEscClk(1);
    SCU_EXTREG_CLK_EN1_SET_csirx0_vc0(1);
    SCU_EXTREG_CLK_EN1_SET_csirx0_csi(1);

    SCU_EXTREG_CLK_EN1_SET_csirx1_TxEscClk(1);
    SCU_EXTREG_CLK_EN1_SET_csirx1_vc0(1);
    SCU_EXTREG_CLK_EN1_SET_csirx1_csi(1);

    masked_outw( SCU_EXTREG_CSIRX_CTRL0,
               ((SCU_EXTREG_CSIRX_CTRL0_ClkLnEn | SCU_EXTREG_CSIRX_CTRL0_Enable) |
                (SCU_EXTREG_CSIRX_CTRL0_apb_rst_n | SCU_EXTREG_CSIRX_CTRL0_pwr_rst_n |
                 SCU_EXTREG_CSIRX_CTRL0_sys_rst_n)),
               ((SCU_EXTREG_CSIRX_CTRL0_ClkLnEn | SCU_EXTREG_CSIRX_CTRL0_Enable) |
                (SCU_EXTREG_CSIRX_CTRL0_apb_rst_n | SCU_EXTREG_CSIRX_CTRL0_pwr_rst_n |
                 SCU_EXTREG_CSIRX_CTRL0_sys_rst_n)));

    masked_outw( SCU_EXTREG_CSIRX_CTRL1,
               ((SCU_EXTREG_CSIRX_CTRL1_ClkLnEn | SCU_EXTREG_CSIRX_CTRL1_Enable) |
                (SCU_EXTREG_CSIRX_CTRL1_apb_rst_n | SCU_EXTREG_CSIRX_CTRL1_pwr_rst_n |
                 SCU_EXTREG_CSIRX_CTRL1_sys_rst_n)),
               ((SCU_EXTREG_CSIRX_CTRL1_ClkLnEn | SCU_EXTREG_CSIRX_CTRL1_Enable) |
                (SCU_EXTREG_CSIRX_CTRL1_apb_rst_n | SCU_EXTREG_CSIRX_CTRL1_pwr_rst_n |
                 SCU_EXTREG_CSIRX_CTRL1_sys_rst_n)));

    masked_outw( SCU_EXTREG_DPI2AHB_CTRL,
               (SCU_EXTREG_DPI2AHB_CTRL_rst_n | SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n |
                SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n | SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n),
               (SCU_EXTREG_DPI2AHB_CTRL_rst_n | SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n |
                SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n | SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n));

    masked_outw( SCU_EXTREG_DPI2AHB_CTRL,
               (SCU_EXTREG_DPI2AHB_CTRL_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n_1 |
                SCU_EXTREG_DPI2AHB_CTRL_sys_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n_1),
               (SCU_EXTREG_DPI2AHB_CTRL_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_apb_rst_n_1 |
                SCU_EXTREG_DPI2AHB_CTRL_sys_rst_n_1 | SCU_EXTREG_DPI2AHB_CTRL_pwr_rst_n_1));
}

int kdp_camera_kl520_init(void)
{
    //critical_msg("[%s] init\n", __func__);

#if (IMGSRC_0_TYPE >= 0)
    cam_ctx[KDP_CAM_0].irq = D2A_FTDPI2AHB_IRQ;
    NVIC_SetVector((IRQn_Type)cam_ctx[KDP_CAM_0].irq, (u32)camera_isr_0);
    cam_ctx[KDP_CAM_0].dpi2ahb_base = DPI2AHB_CSR_PA_BASE;
    cam_ctx[KDP_CAM_0].csi_rx_base = CSIRX_FTCSIRX100_PA_BASE;
    cam_ctx[KDP_CAM_0].phy_csr_base = MIPIRX_PHY_CSR_PA_BASE;
    cam_ctx[KDP_CAM_0].id = KDP_CAM_0;
    cam_ctx[KDP_CAM_0].mipi_lane_num = IMGSRC_0_MIPILANE_NUM;
    kdp_camera_controller_register(KDP_CAM_0, &kdp520_camera_ops);
#endif

#if (IMGSRC_1_TYPE >= 0)
    cam_ctx[KDP_CAM_1].irq = D2A_FTDPI2AHB_1_IRQ;
    NVIC_SetVector((IRQn_Type)cam_ctx[KDP_CAM_1].irq, (u32)camera_isr_1);
    cam_ctx[KDP_CAM_1].dpi2ahb_base = DPI2AHB_CSR_1_PA_BASE;
    cam_ctx[KDP_CAM_1].csi_rx_base = CSIRX_FTCSIRX100_1_PA_BASE;
    cam_ctx[KDP_CAM_1].phy_csr_base = MIPIRX_PHY_CSR_1_PA_BASE;
    cam_ctx[KDP_CAM_1].id = KDP_CAM_1;
    cam_ctx[KDP_CAM_1].mipi_lane_num = IMGSRC_1_MIPILANE_NUM;
    kdp_camera_controller_register(KDP_CAM_1, &kdp520_camera_ops);
#endif

    sys_camera_global_init();
#if IMGSRC_0_TYPE == SENSOR_TYPE_MIXO3238    
    cam_ctx[KDP_CAM_0].sensor_id = SENSOR_TYPE_MIXO3238;
    cam_ctx[KDP_CAM_0].tile_avg_en = 1;
    mixo3238_sensor_init(KDP_CAM_0);
#endif 

#if IMGSRC_0_TYPE ==SENSOR_TYPE_BF20A1_R    
    cam_ctx[KDP_CAM_0].sensor_id = SENSOR_TYPE_BF20A1_R;
    cam_ctx[KDP_CAM_0].tile_avg_en = 1;
    bf20a1_r_sensor_init(KDP_CAM_0);
#endif 
#if IMGSRC_1_TYPE ==SENSOR_TYPE_BF20A1_L    
    cam_ctx[KDP_CAM_1].sensor_id = SENSOR_TYPE_BF20A1_L;
    cam_ctx[KDP_CAM_1].tile_avg_en = 1;
    bf20a1_l_sensor_init(KDP_CAM_1);
#endif 

#if IMGSRC_1_TYPE ==SENSOR_TYPE_BF20A1_R    
    cam_ctx[KDP_CAM_1].sensor_id = SENSOR_TYPE_BF20A1_R;
    cam_ctx[KDP_CAM_1].tile_avg_en = 1;
    bf20a1_r_sensor_init(KDP_CAM_1);
#endif 
#if IMGSRC_0_TYPE ==SENSOR_TYPE_BF20A1_L    
    cam_ctx[KDP_CAM_0].sensor_id = SENSOR_TYPE_BF20A1_L;
    cam_ctx[KDP_CAM_0].tile_avg_en = 1;
    bf20a1_l_sensor_init(KDP_CAM_0);
#endif 

#if IMGSRC_1_TYPE ==SENSOR_TYPE_OV02B1B_R    
    cam_ctx[KDP_CAM_1].sensor_id = SENSOR_TYPE_OV02B1B_R;
    cam_ctx[KDP_CAM_1].tile_avg_en = (KDP_CAM_1 == MIPI_CAM_NIR)?1:0;
    ov02b1b_r_sensor_init(KDP_CAM_1);
#endif 
#if IMGSRC_0_TYPE ==SENSOR_TYPE_OV02B1B_L    
    cam_ctx[KDP_CAM_0].sensor_id = SENSOR_TYPE_OV02B1B_L;
    cam_ctx[KDP_CAM_0].tile_avg_en = (KDP_CAM_0 == MIPI_CAM_NIR)?1:0;
    ov02b1b_l_sensor_init(KDP_CAM_0);
#endif 

#if IMGSRC_1_TYPE ==SENSOR_TYPE_OV02B1B_L    
    cam_ctx[KDP_CAM_1].sensor_id = SENSOR_TYPE_OV02B1B_L;
    cam_ctx[KDP_CAM_1].tile_avg_en = (KDP_CAM_1 == MIPI_CAM_NIR)?1:0;
    ov02b1b_l_sensor_init(KDP_CAM_1);
#endif 
#if IMGSRC_0_TYPE ==SENSOR_TYPE_OV02B1B_R    
    cam_ctx[KDP_CAM_0].sensor_id = SENSOR_TYPE_OV02B1B_R;
    cam_ctx[KDP_CAM_0].tile_avg_en = (KDP_CAM_0 == MIPI_CAM_NIR)?1:0;
    ov02b1b_r_sensor_init(KDP_CAM_0);
#endif 



#if IMGSRC_1_TYPE ==SENSOR_TYPE_SP2509_R    
    cam_ctx[KDP_CAM_1].sensor_id = SENSOR_TYPE_SP2509_R;
    cam_ctx[KDP_CAM_1].tile_avg_en = (KDP_CAM_1 == MIPI_CAM_NIR)?1:0;
    sp2509_r_sensor_init(KDP_CAM_1);
#endif 
#if IMGSRC_0_TYPE ==SENSOR_TYPE_SP2509_L    
    cam_ctx[KDP_CAM_0].sensor_id = SENSOR_TYPE_SP2509_L;
    cam_ctx[KDP_CAM_0].tile_avg_en = (KDP_CAM_0 == MIPI_CAM_NIR)?1:0;
    sp2509_l_sensor_init(KDP_CAM_0);
#endif 

#if IMGSRC_1_TYPE ==SENSOR_TYPE_SP2509_L    
    cam_ctx[KDP_CAM_1].sensor_id = SENSOR_TYPE_SP2509_L;
    cam_ctx[KDP_CAM_1].tile_avg_en = (KDP_CAM_1 == MIPI_CAM_NIR)?1:0;
    sp2509_l_sensor_init(KDP_CAM_1);
#endif 
#if IMGSRC_0_TYPE ==SENSOR_TYPE_SP2509_R    
    cam_ctx[KDP_CAM_0].sensor_id = SENSOR_TYPE_SP2509_R;
    cam_ctx[KDP_CAM_0].tile_avg_en = (KDP_CAM_0 == MIPI_CAM_NIR)?1:0;
    sp2509_r_sensor_init(KDP_CAM_0);
#endif 

#if IMGSRC_0_TYPE == SENSOR_TYPE_GC2145
    cam_ctx[KDP_CAM_0].sensor_id = SENSOR_TYPE_GC2145;
    cam_ctx[KDP_CAM_0].tile_avg_en = 0;
    gc2145_sensor_init(KDP_CAM_0);
#elif IMGSRC_1_TYPE == SENSOR_TYPE_GC2145
    cam_ctx[KDP_CAM_1].sensor_id = SENSOR_TYPE_GC2145;
    cam_ctx[KDP_CAM_1].tile_avg_en = 0;
    gc2145_sensor_init(KDP_CAM_1);
#endif

#if IMGSRC_0_TYPE == SENSOR_TYPE_SC132GS
    cam_ctx[KDP_CAM_0].sensor_id = SENSOR_TYPE_SC132GS;
    cam_ctx[KDP_CAM_0].tile_avg_en = 1;
    sc132gs_sensor_init(KDP_CAM_0);
#elif IMGSRC_1_TYPE == SENSOR_TYPE_SC132GS
    cam_ctx[KDP_CAM_1].sensor_id = SENSOR_TYPE_SC132GS;
    cam_ctx[KDP_CAM_1].tile_avg_en = 1;
    sc132gs_sensor_init(KDP_CAM_1);
#endif

#if IMGSRC_0_TYPE == SENSOR_TYPE_SC035HGS
    cam_ctx[KDP_CAM_0].sensor_id = SENSOR_TYPE_SC035HGS;
    cam_ctx[KDP_CAM_0].tile_avg_en = 1;
    sc035hgs_sensor_init(KDP_CAM_0);
#elif IMGSRC_1_TYPE == SENSOR_TYPE_SC035HGS
    cam_ctx[KDP_CAM_1].sensor_id = SENSOR_TYPE_SC035HGS;
    cam_ctx[KDP_CAM_1].tile_avg_en = 1;
    sc035hgs_sensor_init(KDP_CAM_1);
#endif

#if IMGSRC_0_TYPE == SENSOR_TYPE_GC1054_R
    cam_ctx[KDP_CAM_0].sensor_id = SENSOR_TYPE_GC1054_R;
    cam_ctx[KDP_CAM_0].tile_avg_en = (KDP_CAM_0 == MIPI_CAM_NIR)?1:0;
    gc1054_r_sensor_init(KDP_CAM_0);
#elif IMGSRC_0_TYPE == SENSOR_TYPE_GC1054_L
    cam_ctx[KDP_CAM_0].sensor_id = SENSOR_TYPE_GC1054_L;
    cam_ctx[KDP_CAM_0].tile_avg_en = (KDP_CAM_0 == MIPI_CAM_NIR)?1:0;
    gc1054_l_sensor_init(KDP_CAM_0);
#endif

#if IMGSRC_1_TYPE == SENSOR_TYPE_GC1054_R
    cam_ctx[KDP_CAM_1].sensor_id = SENSOR_TYPE_GC1054_R;
    cam_ctx[KDP_CAM_1].tile_avg_en = (KDP_CAM_1 == MIPI_CAM_NIR)?1:0;
    gc1054_r_sensor_init(KDP_CAM_1);
#elif IMGSRC_1_TYPE == SENSOR_TYPE_GC1054_L
    cam_ctx[KDP_CAM_1].sensor_id = SENSOR_TYPE_GC1054_L;
    cam_ctx[KDP_CAM_1].tile_avg_en = (KDP_CAM_1 == MIPI_CAM_NIR)?1:0;
    gc1054_l_sensor_init(KDP_CAM_1);		
   // gc1054_sensor_init(KDP_CAM_1);
#endif

#if IMGSRC_0_TYPE == SENSOR_TYPE_OV9282_R
    cam_ctx[KDP_CAM_0].sensor_id = SENSOR_TYPE_OV9282_R;
    cam_ctx[KDP_CAM_0].tile_avg_en = (KDP_CAM_0 == MIPI_CAM_NIR)?1:0;
    ov9282_r_sensor_init(KDP_CAM_0);
#elif IMGSRC_0_TYPE == SENSOR_TYPE_OV9282_L
    cam_ctx[KDP_CAM_0].sensor_id = SENSOR_TYPE_OV9282_L;
    cam_ctx[KDP_CAM_0].tile_avg_en = (KDP_CAM_0 == MIPI_CAM_NIR)?1:0;
    ov9282_l_sensor_init(KDP_CAM_0);
#endif

#if IMGSRC_1_TYPE == SENSOR_TYPE_OV9282_R
    cam_ctx[KDP_CAM_1].sensor_id = SENSOR_TYPE_OV9282_R;
    cam_ctx[KDP_CAM_1].tile_avg_en = (KDP_CAM_1 == MIPI_CAM_NIR)?1:0;
    ov9282_r_sensor_init(KDP_CAM_1);
#elif IMGSRC_1_TYPE == SENSOR_TYPE_OV9282_L
    cam_ctx[KDP_CAM_1].sensor_id = SENSOR_TYPE_OV9282_L;
    cam_ctx[KDP_CAM_1].tile_avg_en = (KDP_CAM_1 == MIPI_CAM_NIR)?1:0;
    ov9282_l_sensor_init(KDP_CAM_1);		
#endif

#if IMGSRC_1_TYPE == SENSOR_TYPE_GC02M1_R
    cam_ctx[KDP_CAM_1].sensor_id = SENSOR_TYPE_GC02M1_R;
    cam_ctx[KDP_CAM_1].tile_avg_en = (KDP_CAM_1 == MIPI_CAM_NIR)?1:0;
    gc02m1_r_sensor_init(KDP_CAM_1);
#elif IMGSRC_1_TYPE == SENSOR_TYPE_GC02M1_L
    cam_ctx[KDP_CAM_1].sensor_id = SENSOR_TYPE_GC02M1_L;
    cam_ctx[KDP_CAM_1].tile_avg_en = (KDP_CAM_1 == MIPI_CAM_NIR)?1:0;
    gc02m1_l_sensor_init(KDP_CAM_1);		
   // gc1054_sensor_init(KDP_CAM_1);
#endif

#if IMGSRC_0_TYPE == SENSOR_TYPE_GC02M1_R
    cam_ctx[KDP_CAM_0].sensor_id = SENSOR_TYPE_GC02M1_R;
    cam_ctx[KDP_CAM_0].tile_avg_en = (KDP_CAM_0 == MIPI_CAM_NIR)?1:0;
    gc02m1_r_sensor_init(KDP_CAM_0);
#elif IMGSRC_0_TYPE == SENSOR_TYPE_GC02M1_L
    cam_ctx[KDP_CAM_0].sensor_id = SENSOR_TYPE_GC02M1_L;
    cam_ctx[KDP_CAM_0].tile_avg_en = (KDP_CAM_0 == MIPI_CAM_NIR)?1:0;
    gc02m1_l_sensor_init(KDP_CAM_0);		
   // gc1054_sensor_init(KDP_CAM_1);
#endif


    return 0;
}
