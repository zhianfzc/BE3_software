#ifndef __LCDC_H__
#define __LCDC_H__


#include "base.h"
#include "io.h"
#include "kneron_mozart.h"
#include "board_kl520.h"


#define LCDC_REG_FUNC_ENABLE                                (LCD_FTLCDC210_PA_BASE + 0x0000)
#define LCDC_REG_PANEL_PIXEL                                (LCD_FTLCDC210_PA_BASE + 0x0004)
#define LCDC_REG_INTR_ENABLE_MASK                           (LCD_FTLCDC210_PA_BASE + 0x0008)
#define LCDC_REG_INTR_CLEAR                                 (LCD_FTLCDC210_PA_BASE + 0x000C)
#define LCDC_REG_INTR_STATUS                                (LCD_FTLCDC210_PA_BASE + 0x0010)
#define LCDC_REG_FRAME_BUFFER                               (LCD_FTLCDC210_PA_BASE + 0x0014)
#define LCDC_REG_PANEL_IMAGE0_FRAME0                        (LCD_FTLCDC210_PA_BASE + 0x0018)
#define LCDC_REG_PANEL_IMAGE1_FRAME0                        (LCD_FTLCDC210_PA_BASE + 0x0024)
#define LCDC_REG_PANEL_IMAGE2_FRAME0                        (LCD_FTLCDC210_PA_BASE + 0x0030)
#define LCDC_REG_PANEL_IMAGE3_FRAME0                        (LCD_FTLCDC210_PA_BASE + 0x003C)
#define LCDC_REG_PATGEN_PATTERN_BAR_DISTANCE                (LCD_FTLCDC210_PA_BASE + 0x0048)
#define LCDC_REG_FIFO_THRESHOLD                             (LCD_FTLCDC210_PA_BASE + 0x004C)
#define LCDC_REG_BANDWIDTH_CTRL                             (LCD_FTLCDC210_PA_BASE + 0x0050)
#define LCDC_REG_HORIZONTAL_TIMING_CTRL                     (LCD_FTLCDC210_PA_BASE + 0x0100)
#define LCDC_REG_VERTICAL_TIMING_CTRL                       (LCD_FTLCDC210_PA_BASE + 0x0104)
#define LCDC_REG_VERTICAL_BACK_PORCH                        (LCD_FTLCDC210_PA_BASE + 0x0108)
#define LCDC_REG_POLARITY_CTRL                              (LCD_FTLCDC210_PA_BASE + 0x010C)
#define LCDC_REG_SERIAL_PANEL_PIXEL                         (LCD_FTLCDC210_PA_BASE + 0x0200)
#define LCDC_REG_TV                                         (LCD_FTLCDC210_PA_BASE + 0x0204)
#define LCDC_REG_TV_VERTICAL_BLANK_1                        (LCD_FTLCDC210_PA_BASE + 0x0210)
#define LCDC_REG_TV_VERTICAL_BLANK_2                        (LCD_FTLCDC210_PA_BASE + 0x0214)
#define LCDC_REG_TV_HORIZONTAL_BLANK_1                      (LCD_FTLCDC210_PA_BASE + 0x021C)
#define LCDC_REG_TV_HORIZONTAL_BLANK_2                      (LCD_FTLCDC210_PA_BASE + 0x0220)
#define LCDC_REG_PIP_BLENDING                               (LCD_FTLCDC210_PA_BASE + 0x0300)
#define LCDC_REG_PIP_SUBPIC1_POS                            (LCD_FTLCDC210_PA_BASE + 0x0304)
#define LCDC_REG_PIP_SUBPIC1_DIM                            (LCD_FTLCDC210_PA_BASE + 0x0308)
#define LCDC_REG_PIP_SUBPIC2_POS                            (LCD_FTLCDC210_PA_BASE + 0x030C)
#define LCDC_REG_PIP_SUBPIC2_DIM                            (LCD_FTLCDC210_PA_BASE + 0x0310)

#define LCDC_REG_PIPPOP_FMT_1                               (LCD_FTLCDC210_PA_BASE + 0x0318)
#define LCDC_REG_COLOR_MGR_0                                (LCD_FTLCDC210_PA_BASE + 0x0400)
#define LCDC_REG_COLOR_MGR_1                                (LCD_FTLCDC210_PA_BASE + 0x0404)
#define LCDC_REG_COLOR_MGR_2                                (LCD_FTLCDC210_PA_BASE + 0x0408)
#define LCDC_REG_COLOR_MGR_3                                (LCD_FTLCDC210_PA_BASE + 0x040C)
#define LCDC_REG_LT_OF_GAMMA_RED                            (LCD_FTLCDC210_PA_BASE + 0x0600)
#define LCDC_REG_LT_OF_GAMMA_GREEN                          (LCD_FTLCDC210_PA_BASE + 0x0700)
#define LCDC_REG_LT_OF_GAMMA_BLUE                           (LCD_FTLCDC210_PA_BASE + 0x0800)

#define LCDC_REG_PALETTE                                    (LCD_FTLCDC210_PA_BASE + 0x0A00)

#define LCDC_REG_SCALER_HOR_RES_IN                          (LCD_FTLCDC210_PA_BASE + 0x1100)
#define LCDC_REG_SCALER_VER_RES_IN                          (LCD_FTLCDC210_PA_BASE + 0x1104)
#define LCDC_REG_SCALER_HOR_RES_OUT                         (LCD_FTLCDC210_PA_BASE + 0x1108)
#define LCDC_REG_SCALER_VER_RES_OUT                         (LCD_FTLCDC210_PA_BASE + 0x110C)
#define LCDC_REG_SCALER_MISC                                (LCD_FTLCDC210_PA_BASE + 0x1110)
#define LCDC_REG_SCALER_RES                                 (LCD_FTLCDC210_PA_BASE + 0x112C)

/* LCD Function Enable Parameter (Offset 0x0000) */
#define LCDC_REG_FUNC_ENABLE_TVEn                           BIT13
#define LCDC_REG_FUNC_ENABLE_DitherEn                       BIT6
#define LCDC_REG_FUNC_ENABLE_ScalerEn                       BIT5
#define LCDC_REG_FUNC_ENABLE_OSDEn                          BIT4
#define LCDC_REG_FUNC_ENABLE_EnYCbCr                        BIT3
#define LCDC_REG_FUNC_ENABLE_EnYCbCr420                     BIT2
#define LCDC_REG_FUNC_ENABLE_LCDon                          BIT1
#define LCDC_REG_FUNC_ENABLE_LCDen                          BIT0

#define LCDC_REG_FUNC_ENABLE_GET_PiPEn()                    GET_BITS(LCDC_REG_FUNC_ENABLE, 10, 11)
#define LCDC_REG_FUNC_ENABLE_GET_BlendEn()                  GET_BIT(LCDC_REG_FUNC_ENABLE, 8, 9)
#define LCDC_REG_FUNC_ENABLE_GET_EnYCbCr()                  GET_BIT(LCDC_REG_FUNC_ENABLE, 3)
#define LCDC_REG_FUNC_ENABLE_GET_EnYCbCr420()               GET_BIT(LCDC_REG_FUNC_ENABLE, 2)
#define LCDC_REG_FUNC_ENABLE_GET_LCDon()                    GET_BIT(LCDC_REG_FUNC_ENABLE, 1)
#define LCDC_REG_FUNC_ENABLE_GET_LCDen()                    GET_BIT(LCDC_REG_FUNC_ENABLE, 0)

#define LCDC_REG_FUNC_ENABLE_SET_PenGen(val)                SET_MASKED_BIT(LCDC_REG_FUNC_ENABLE, val, 14)
#define LCDC_REG_FUNC_ENABLE_SET_TVEn(val)                  SET_MASKED_BIT(LCDC_REG_FUNC_ENABLE, val, 13)
#define LCDC_REG_FUNC_ENABLE_SET_PiPEn(val)                 SET_MASKED_BITS(LCDC_REG_FUNC_ENABLE, val, 10, 11)
#define LCDC_REG_FUNC_ENABLE_SET_BlendEn(val)               SET_MASKED_BITS(LCDC_REG_FUNC_ENABLE, val, 8, 9)
#define LCDC_REG_FUNC_ENABLE_SET_ScalerEn(val)              SET_MASKED_BIT(LCDC_REG_FUNC_ENABLE, val, 5)
#define LCDC_REG_FUNC_ENABLE_SET_OSDEn(val)                 SET_MASKED_BIT(LCDC_REG_FUNC_ENABLE, val, 4)
#define LCDC_REG_FUNC_ENABLE_SET_EnYCbCr(val)               SET_MASKED_BIT(LCDC_REG_FUNC_ENABLE, val, 3)
#define LCDC_REG_FUNC_ENABLE_SET_EnYCbCr420(val)            SET_MASKED_BIT(LCDC_REG_FUNC_ENABLE, val, 2)
#define LCDC_REG_FUNC_ENABLE_SET_LCDon(val)                 SET_MASKED_BIT(LCDC_REG_FUNC_ENABLE, val, 1)
#define LCDC_REG_FUNC_ENABLE_SET_LCDen(val)                 SET_MASKED_BIT(LCDC_REG_FUNC_ENABLE, val, 0)


/* LCD Panel Pixel Parameter (Offset 0x0004) */
#define LCDC_REG_PANEL_PIXEL_GET_AddrUpdate()               GET_BIT(LCDC_REG_PANEL_PIXEL, 16)
#define LCDC_REG_PANEL_PIXEL_GET_UpdateSrc()                GET_BITS(LCDC_REG_PANEL_PIXEL, 14, 15)
#define LCDC_REG_PANEL_PIXEL_GET_DitherType()               GET_BITS(LCDC_REG_PANEL_PIXEL, 12, 13)
#define LCDC_REG_PANEL_PIXEL_GET_PanelType()                GET_BIT(LCDC_REG_PANEL_PIXEL, 11)
#define LCDC_REG_PANEL_PIXEL_GET_Vcomp()                    GET_BITS(LCDC_REG_PANEL_PIXEL, 9, 10)
#define LCDC_REG_PANEL_PIXEL_GET_RGBTYPE()                  GET_BITS(LCDC_REG_PANEL_PIXEL, 7, 8)
#define LCDC_REG_PANEL_PIXEL_GET_Endian()                   GET_BITS(LCDC_REG_PANEL_PIXEL, 5, 6)
#define LCDC_REG_PANEL_PIXEL_GET_BGRSW()                    GET_BIT(LCDC_REG_PANEL_PIXEL, 4)
#define LCDC_REG_PANEL_PIXEL_GET_PWROFF()                   GET_BIT(LCDC_REG_PANEL_PIXEL, 3)
#define LCDC_REG_PANEL_PIXEL_GET_BppFifo()                  GET_BITS(LCDC_REG_PANEL_PIXEL, 0, 2)

#define LCDC_REG_PANEL_PIXEL_SET_AddrUpdate(val)            SET_MASKED_BIT(LCDC_REG_PANEL_PIXEL, val, 16)
#define LCDC_REG_PANEL_PIXEL_SET_UpdateSrc(val)             SET_MASKED_BITS(LCDC_REG_PANEL_PIXEL, val, 14, 15)
#define LCDC_REG_PANEL_PIXEL_SET_DitherType(val)            SET_MASKED_BITS(LCDC_REG_PANEL_PIXEL, val, 12, 13)
#define LCDC_REG_PANEL_PIXEL_SET_PanelType(val)             SET_MASKED_BIT(LCDC_REG_PANEL_PIXEL, val, 11)
#define LCDC_REG_PANEL_PIXEL_SET_Vcomp(val)                 SET_MASKED_BIT(LCDC_REG_PANEL_PIXEL, val, 9, 10)
#define LCDC_REG_PANEL_PIXEL_SET_RGBTYPE(val)               SET_MASKED_BITS(LCDC_REG_PANEL_PIXEL, val, 7, 8)
#define LCDC_REG_PANEL_PIXEL_SET_Endian(val)                SET_MASKED_BITS(LCDC_REG_PANEL_PIXEL, val, 5, 6)
#define LCDC_REG_PANEL_PIXEL_SET_BGRSW(val)                 SET_MASKED_BIT(LCDC_REG_PANEL_PIXEL, val, 4)
#define LCDC_REG_PANEL_PIXEL_SET_BppFifo(val)               SET_MASKED_BITS(LCDC_REG_PANEL_PIXEL, val, 0, 2)

#define LCDC_REG_PANEL_PIXEL_RGBTYPE_565                    (0x0 << 7)
#define LCDC_REG_PANEL_PIXEL_RGBTYPE_555                    (0x1 << 7)
#define LCDC_REG_PANEL_PIXEL_RGBTYPE_444                    (0x2 << 7)
#define LCDC_REG_PANEL_PIXEL_RGBTYPE_MASK                   (BIT7 | BIT8)

#define LCDC_REG_PANEL_PIXEL_BppFifo_1bpp                   (0x0)
#define LCDC_REG_PANEL_PIXEL_BppFifo_2bpp                   (0x1)
#define LCDC_REG_PANEL_PIXEL_BppFifo_4bpp                   (0x2)
#define LCDC_REG_PANEL_PIXEL_BppFifo_8bpp                   (0x3)
#define LCDC_REG_PANEL_PIXEL_BppFifo_16bpp                  (0x4)
#define LCDC_REG_PANEL_PIXEL_BppFifo_24bpp                  (0x5)
#define LCDC_REG_PANEL_PIXEL_BppFifo_MASK                   (BIT0 | BIT1 | BIT2)


/* LCD Interrupt Status Clear (Offset 0x000C) */
#define LCDC_REG_INTR_CLEAR_SET_ClrBusErr()                 SET_BIT(LCDC_REG_INTR_CLEAR, 3) //Write only
#define LCDC_REG_INTR_CLEAR_SET_ClrVstatus()                SET_BIT(LCDC_REG_INTR_CLEAR, 2) //Write only
#define LCDC_REG_INTR_CLEAR_SET_ClrNxtBase()                SET_BIT(LCDC_REG_INTR_CLEAR, 1) //Write only
#define LCDC_REG_INTR_CLEAR_SET_ClrFIFOUdn()                SET_BIT(LCDC_REG_INTR_CLEAR, 0) //Write only

/* LCD Interrupt Status (Offset 0x0010) */ 
#define LCDC_REG_INTR_STATUS_IntBusErr                       BIT3 //Read only
#define LCDC_REG_INTR_STATUS_IntVstatus                      BIT2 //Read only
#define LCDC_REG_INTR_STATUS_IntNxtBase                      BIT1 //Read only
#define LCDC_REG_INTR_STATUS_IntFIFOUdn                      BIT0 //Read only

/* Frame Buffer parameter (Offset 0x0014) */

/* LCDC Panel Image0 Frame0 Base Address (Offset 0x0018) */

/* PatGen Pattern Bar Distance Parameter (Offset 0x0048) */
#define LCDC_REG_PATGEN_GET_Img0PatGen()                    GET_BIT(LCDC_REG_PATGEN_PATTERN_BAR_DISTANCE, 0)
#define LCDC_REG_PATGEN_SET_Img0PatGen(val)                 SET_MASKED_BIT(LCDC_REG_PATGEN_PATTERN_BAR_DISTANCE, val, 0)

/* FIFO Threshold Control (Offset 0x004C) */


/* LCD Horizontal Timing Control Parameter (Offset 0x0100) */
#define LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_HBP()           GET_BITS(LCDC_REG_HORIZONTAL_TIMING_CTRL, 24, 31)
#define LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_HFP()           GET_BITS(LCDC_REG_HORIZONTAL_TIMING_CTRL, 16, 23)
#define LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_HW()            GET_BITS(LCDC_REG_HORIZONTAL_TIMING_CTRL, 8, 15)
#define LCDC_REG_HORIZONTAL_TIMING_CTRL_GET_PL()            GET_BITS(LCDC_REG_HORIZONTAL_TIMING_CTRL, 0, 7)

#define LCDC_REG_HORIZONTAL_TIMING_CTRL_SET_HBP(val)        SET_MASKED_BITS(LCDC_REG_HORIZONTAL_TIMING_CTRL, val, 24, 31)
#define LCDC_REG_HORIZONTAL_TIMING_CTRL_SET_HFP(val)        SET_MASKED_BITS(LCDC_REG_HORIZONTAL_TIMING_CTRL, val, 16, 23)
#define LCDC_REG_HORIZONTAL_TIMING_CTRL_SET_HW(val)         SET_MASKED_BITS(LCDC_REG_HORIZONTAL_TIMING_CTRL, val, 8, 15)
#define LCDC_REG_HORIZONTAL_TIMING_CTRL_SET_PL(val)         SET_MASKED_BITS(LCDC_REG_HORIZONTAL_TIMING_CTRL, val, 0, 7)

/* LCD Vertical Timing Control Parameter (Offset 0x0104) */
#define LCDC_REG_VERTICAL_TIMING_CTRL_GET_VFP()             GET_BITS(LCDC_REG_VERTICAL_TIMING_CTRL, 24, 31)
#define LCDC_REG_VERTICAL_TIMING_CTRL_GET_VW()              GET_BITS(LCDC_REG_VERTICAL_TIMING_CTRL, 16, 21)
#define LCDC_REG_VERTICAL_TIMING_CTRL_GET_LF()              GET_BITS(LCDC_REG_VERTICAL_TIMING_CTRL, 0, 11)

#define LCDC_REG_VERTICAL_TIMING_CTRL_SET_VFP(val)          SET_MASKED_BITS(LCDC_REG_VERTICAL_TIMING_CTRL, val, 24, 31)
#define LCDC_REG_VERTICAL_TIMING_CTRL_SET_VW(val)           SET_MASKED_BITS(LCDC_REG_VERTICAL_TIMING_CTRL, val, 16, 21)
#define LCDC_REG_VERTICAL_TIMING_CTRL_SET_LF(val)           SET_MASKED_BITS(LCDC_REG_VERTICAL_TIMING_CTRL, val, 0, 11)

/* LCD Vertical Back Porch Parameter (Offset 0x0108) */
#define LCDC_REG_VERTICAL_BACK_PORCH_GET_VBP()              GET_BITS(LCDC_REG_VERTICAL_BACK_PORCH, 0, 7)

#define LCDC_REG_VERTICAL_BACK_PORCH_SET_VBP(val)           SET_MASKED_BITS(LCDC_REG_VERTICAL_BACK_PORCH, val, 0, 7)

/* LCD Polarity Control Parameter (Offset 0x010C) */
#define LCDC_REG_POLARITY_CTRL_GET_TV_DivNo()               GET_BITS(LCDC_REG_POLARITY_CTRL, 16, 22)
#define LCDC_REG_POLARITY_CTRL_GET_DivNo()                  GET_BITS(LCDC_REG_POLARITY_CTRL, 8, 14)
#define LCDC_REG_POLARITY_CTRL_GET_IPWR()                   GET_BIT(LCDC_REG_POLARITY_CTRL, 4)
#define LCDC_REG_POLARITY_CTRL_GET_IDE()                    GET_BIT(LCDC_REG_POLARITY_CTRL, 3)
#define LCDC_REG_POLARITY_CTRL_GET_ICK()                    GET_BIT(LCDC_REG_POLARITY_CTRL, 2)
#define LCDC_REG_POLARITY_CTRL_GET_IHS()                    GET_BIT(LCDC_REG_POLARITY_CTRL, 1)
#define LCDC_REG_POLARITY_CTRL_GET_IVS()                    GET_BIT(LCDC_REG_POLARITY_CTRL, 0)

#define LCDC_REG_POLARITY_CTRL_SET_TV_DivNo(val)            SET_MASKED_BITS(LCDC_REG_POLARITY_CTRL, val, 16, 22)
#define LCDC_REG_POLARITY_CTRL_SET_DivNo(val)               SET_MASKED_BITS(LCDC_REG_POLARITY_CTRL, val, 8, 14)
#define LCDC_REG_POLARITY_CTRL_SET_IPWR(val)                SET_MASKED_BIT(LCDC_REG_POLARITY_CTRL, val, 4)
#define LCDC_REG_POLARITY_CTRL_SET_IDE(val)                 SET_MASKED_BIT(LCDC_REG_POLARITY_CTRL, val, 3)
#define LCDC_REG_POLARITY_CTRL_SET_ICK(val)                 SET_MASKED_BIT(LCDC_REG_POLARITY_CTRL, val, 2)
#define LCDC_REG_POLARITY_CTRL_SET_IHS(val)                 SET_MASKED_BIT(LCDC_REG_POLARITY_CTRL, val, 1)
#define LCDC_REG_POLARITY_CTRL_SET_IVS(val)                 SET_MASKED_BIT(LCDC_REG_POLARITY_CTRL, val, 0)

/* LCD Serial Panel Pixel Parameter (Offset 0x0200) */
#define LCDC_REG_SERIAL_PANEL_PIXEL_GET_AUO052()            GET_BIT(LCDC_REG_SERIAL_PANEL_PIXEL, 5)
#define LCDC_REG_SERIAL_PANEL_PIXEL_GET_LSR()               GET_BIT(LCDC_REG_SERIAL_PANEL_PIXEL, 4)
#define LCDC_REG_SERIAL_PANEL_PIXEL_GET_ColorSeq()          GET_BITS(LCDC_REG_SERIAL_PANEL_PIXEL, 2, 3)
#define LCDC_REG_SERIAL_PANEL_PIXEL_GET_DeltaType()         GET_BIT(LCDC_REG_SERIAL_PANEL_PIXEL, 1)
#define LCDC_REG_SERIAL_PANEL_PIXEL_GET_SerialMode()        GET_BIT(LCDC_REG_SERIAL_PANEL_PIXEL, 0)

#define LCDC_REG_SERIAL_PANEL_PIXEL_SET_AUO052(val)         SET_MASKED_BIT(LCDC_REG_SERIAL_PANEL_PIXEL, val, 5)
#define LCDC_REG_SERIAL_PANEL_PIXEL_SET_LSR(val)            SET_MASKED_BIT(LCDC_REG_SERIAL_PANEL_PIXEL, val, 4)
#define LCDC_REG_SERIAL_PANEL_PIXEL_SET_ColorSeq(val)       SET_MASKED_BITS(LCDC_REG_SERIAL_PANEL_PIXEL, val, 2, 3)
#define LCDC_REG_SERIAL_PANEL_PIXEL_SET_DeltaType(val)      SET_MASKED_BIT(LCDC_REG_SERIAL_PANEL_PIXEL, val, 1)
#define LCDC_REG_SERIAL_PANEL_PIXEL_SET_SerialMode(val)     SET_MASKED_BIT(LCDC_REG_SERIAL_PANEL_PIXEL, val, 0)

/* LCD TV Parameter (Offset 0x204) */
#define LCDC_REG_TV_GET_ImgFormat()                         GET_BIT(LCDC_REG_TV, 2)
#define LCDC_REG_TV_GET_PHASE()                             GET_BIT(LCDC_REG_TV, 1)
#define LCDC_REG_TV_GET_OutFormat()                         GET_BIT(LCDC_REG_TV, 0)

#define LCDC_REG_TV_SET_ImgFormat(val)                      SET_MASKED_BIT(LCDC_REG_TV, val, 2)
#define LCDC_REG_TV_SET_PHASE(val)                          SET_MASKED_BIT(LCDC_REG_TV, val, 1)
#define LCDC_REG_TV_SET_OutFormat(val)                      SET_MASKED_BIT(LCDC_REG_TV, val, 0)


/* lCD TV Horizontal Blank Parameter (Offset : 0x0210) */
#define LCDC_REG_TV_GET_V_Blk1()                            GET_BITS(LCDC_REG_TV_VERTICAL_BLANK_1, 12, 21)
#define LCDC_REG_TV_GET_V_Blk0()                            GET_BITS(LCDC_REG_TV_VERTICAL_BLANK_1, 0, 9)
/* lCD TV Horizontal Blank Parameter (Offset : 0x0214) */
#define LCDC_REG_TV_GET_V_Blk3()                            GET_BITS(LCDC_REG_TV_VERTICAL_BLANK_2, 12, 21)
#define LCDC_REG_TV_GET_V_Blk2()                            GET_BITS(LCDC_REG_TV_VERTICAL_BLANK_2, 0, 9)

/* lCD TV Horizontal Blank Parameter (Offset : 0x021C) */
#define LCDC_REG_TV_GET_H_Blk1()                            GET_BITS(LCDC_REG_TV_HORIZONTAL_BLANK_1, 12, 21)
#define LCDC_REG_TV_GET_H_Blk0()                            GET_BITS(LCDC_REG_TV_HORIZONTAL_BLANK_1, 0, 9)
/* lCD TV Horizontal Blank Parameter (Offset : 0x0220) */
#define LCDC_REG_TV_GET_H_Blk2()                            GET_BITS(LCDC_REG_TV_HORIZONTAL_BLANK_2, 0, 9)

/* lCD TV Horizontal Blank Parameter (Offset : 0x021C) */
#define LCDC_REG_TV_GET_H_Blk1()                            GET_BITS(LCDC_REG_TV_HORIZONTAL_BLANK_1, 12, 21)
#define LCDC_REG_TV_GET_H_Blk0()                            GET_BITS(LCDC_REG_TV_HORIZONTAL_BLANK_1, 0, 9)
/* lCD TV Horizontal Blank Parameter (Offset : 0x0220) */
#define LCDC_REG_TV_GET_H_Blk2()                            GET_BITS(LCDC_REG_TV_HORIZONTAL_BLANK_2, 0, 9)


/* PiP Blending (Offset 0x0300) */
#define LCDC_REG_PIP_BLENDING_GET_PiPBlend_d()              GET_BITS(LCDC_REG_PIP_BLENDING, 16, 23)
#define LCDC_REG_PIP_BLENDING_GET_PiPBlend_h()              GET_BITS(LCDC_REG_PIP_BLENDING, 8, 15)
#define LCDC_REG_PIP_BLENDING_GET_PiPBlend_l()              GET_BITS(LCDC_REG_PIP_BLENDING, 0, 7)

#define LCDC_REG_PIP_BLENDING_SET_PiPBlend_d(val)           SET_MASKED_BITS(LCDC_REG_PIP_BLENDING, val, 16, 23)
#define LCDC_REG_PIP_BLENDING_SET_PiPBlend_h(val)           SET_MASKED_BITS(LCDC_REG_PIP_BLENDING, val, 8, 15)
#define LCDC_REG_PIP_BLENDING_SET_PiPBlend_l(val)           SET_MASKED_BITS(LCDC_REG_PIP_BLENDING, val, 0, 7)

/* PiP Sub-Picture1 Position (Offset 0x0304) */
#define LCDC_REG_PIP_SUBPIC1_POS_GET_PiP1Hpos()             GET_BITS(LCDC_REG_PIP_SUBPIC1_POS, 16, 26)
#define LCDC_REG_PIP_SUBPIC1_POS_GET_PiP1Vpos()             GET_BITS(LCDC_REG_PIP_SUBPIC1_POS, 0, 10)

#define LCDC_REG_PIP_SUBPIC1_POS_SET_PiP_Update(val)        SET_MASKED_BIT(LCDC_REG_PIP_SUBPIC1_POS, val, 28)
#define LCDC_REG_PIP_SUBPIC1_POS_SET_PiP1Hpos(val)          SET_MASKED_BITS(LCDC_REG_PIP_SUBPIC1_POS, val, 16, 26)
#define LCDC_REG_PIP_SUBPIC1_POS_SET_PiP1Vpos(val)          SET_MASKED_BITS(LCDC_REG_PIP_SUBPIC1_POS, val, 0, 10)

/* PiP Sub-Picture1 Dimension (Offset 0x308) */
#define LCDC_REG_PIP_SUBPIC1_DIM_GET_PiP1Hdim()             GET_BITS(LCDC_REG_PIP_SUBPIC1_DIM, 16, 26)
#define LCDC_REG_PIP_SUBPIC1_DIM_GET_PiP1Vdim()             GET_BITS(LCDC_REG_PIP_SUBPIC1_DIM, 0, 10)
        
#define LCDC_REG_PIP_SUBPIC1_DIM_SET_PiP1Hdim(val)          SET_MASKED_BITS(LCDC_REG_PIP_SUBPIC1_DIM, val, 16, 26)
#define LCDC_REG_PIP_SUBPIC1_DIM_SET_PiP1Vdim(val)          SET_MASKED_BITS(LCDC_REG_PIP_SUBPIC1_DIM, val, 0, 10)

/* PiP Sub-Picture2 Position (Offset 0x030C) */
#define LCDC_REG_PIP_SUBPIC2_POS_GET_PiP1Hpos()             GET_BITS(LCDC_REG_PIP_SUBPIC2_POS, 16, 26)
#define LCDC_REG_PIP_SUBPIC2_POS_GET_PiP1Vpos()             GET_BITS(LCDC_REG_PIP_SUBPIC2_POS, 0, 10)

#define LCDC_REG_PIP_SUBPIC2_POS_SET_PiP1Hpos(val)          SET_MASKED_BITS(LCDC_REG_PIP_SUBPIC2_POS, val, 16, 26)
#define LCDC_REG_PIP_SUBPIC2_POS_SET_PiP1Vpos(val)          SET_MASKED_BITS(LCDC_REG_PIP_SUBPIC2_POS, val, 0, 10)

/* PiP Sub-Picture2 Dimension (Offset 0x0310) */
#define LCDC_REG_PIP_SUBPIC1_DIM_GET_PiP2Hdim()             GET_BITS(LCDC_REG_PIP_SUBPIC2_DIM, 16, 26)
#define LCDC_REG_PIP_SUBPIC1_DIM_GET_PiP2Vdim()             GET_BITS(LCDC_REG_PIP_SUBPIC2_DIM, 0, 10)

#define LCDC_REG_PIP_SUBPIC1_DIM_SET_PiP2Hdim(val)          SET_MASKED_BITS(LCDC_REG_PIP_SUBPIC2_DIM, val, 16, 26)
#define LCDC_REG_PIP_SUBPIC1_DIM_SET_PiP2Vdim(val)          SET_MASKED_BITS(LCDC_REG_PIP_SUBPIC2_DIM, val, 0, 10)


/* PiP/PoP Image Format 1 (Offset 0x0318) */
#define LCDC_REG_PIPPOP_FMT_1_GET_BppFifo0()                GET_BITS(LCDC_REG_PIPPOP_FMT_1, 0, 2)
#define LCDC_REG_PIPPOP_FMT_1_SET_BppFifo0(val)             SET_MASKED_BITS(LCDC_REG_PIPPOP_FMT_1, val, 0, 2)

/* LCD Color Management Parameter 0 */
/* LCD Color Management Parameter 1 */
/* LCD Color Management Parameter 2 */
/* LCD Color Management Parameter 3 */
#define LCDC_REG_COLOR_MGR_3_GET_Contr_slope()              GET_BITS(LCDC_REG_COLOR_MGR_3, 16, 20)
#define LCDC_REG_COLOR_MGR_3_SET_Contr_slope(val)           SET_MASKED_BITS(LCDC_REG_COLOR_MGR_3, val, 16, 20)


/* Horizontal Resolution Register of Scaler Input (Offset 0x1100) */
#define LCDC_REG_SCALER_HOR_RES_IN_GET_hor_no_in()          GET_BITS(LCDC_REG_SCALER_HOR_RES_IN, 0, 11)
#define LCDC_REG_SCALER_HOR_RES_IN_SET_hor_no_in(val)       SET_MASKED_BITS(LCDC_REG_SCALER_HOR_RES_IN, val, 0, 11)

/* Vertical Resolution Register of Scaler Input (Offset 0x1104) */
#define LCDC_REG_SCALER_VER_RES_IN_GET_ver_no_in()          GET_BITS(LCDC_REG_SCALER_VER_RES_IN, 0, 11)
#define LCDC_REG_SCALER_VER_RES_IN_SET_ver_no_in(val)       SET_MASKED_BITS(LCDC_REG_SCALER_VER_RES_IN, val, 0, 11)

/* Horizontal Resolution Register of Scaler Output (Offset 0x1108) */
#define LCDC_REG_SCALER_HOR_RES_OUT_GET_hor_no_out()        GET_BITS(LCDC_REG_SCALER_HOR_RES_OUT, 0, 13)
#define LCDC_REG_SCALER_HOR_RES_OUT_SET_hor_no_out(val)     SET_MASKED_BITS(LCDC_REG_SCALER_HOR_RES_OUT, val, 0, 13)

/* Vertical Resolution Register of Scaler Output (Offset 0x110C) */
#define LCDC_REG_SCALER_VER_RES_OUT_GET_ver_no_out()        GET_BITS(LCDC_REG_SCALER_VER_RES_OUT, 0, 13)
#define LCDC_REG_SCALER_VER_RES_OUT_SET_ver_no_out(val)     SET_MASKED_BITS(LCDC_REG_SCALER_VER_RES_OUT, val, 0, 13)

/* Scaler Control (Offset 0x1110) */
#define LCDC_REG_SCALER_MISC_GET_fir_sel()                  GET_BITS(LCDC_REG_SCALER_MISC, 6, 8)
#define LCDC_REG_SCALER_MISC_GET_hor_inter_mode()           GET_BITS(LCDC_REG_SCALER_MISC, 3, 4)
#define LCDC_REG_SCALER_MISC_GET_ver_inter_mode()           GET_BITS(LCDC_REG_SCALER_MISC, 1, 2)
#define LCDC_REG_SCALER_MISC_GET_bypass_mode()              GET_BIT(LCDC_REG_SCALER_MISC, 0)

#define LCDC_REG_SCALER_MISC_SET_fir_sel(val)               SET_MASKED_BITS(LCDC_REG_SCALER_MISC, val, 6, 8)
#define LCDC_REG_SCALER_MISC_SET_hor_inter_mode(val)        SET_MASKED_BITS(LCDC_REG_SCALER_MISC, val, 3, 4)
#define LCDC_REG_SCALER_MISC_SET_ver_inter_mode(val)        SET_MASKED_BITS(LCDC_REG_SCALER_MISC, val, 1, 2)
#define LCDC_REG_SCALER_MISC_SET_bypass_mode(val)           SET_MASKED_BIT(LCDC_REG_SCALER_MISC, val, 0)


/* Scaler Control (Offset 0x1110) */
#define LCDC_REG_SCALER_RES_GET_scal_hor_num()              GET_BITS(LCDC_REG_SCALER_RES, 8, 15)
#define LCDC_REG_SCALER_RES_GET_scal_ver_num()              GET_BITS(LCDC_REG_SCALER_RES, 0, 7)

#define LCDC_REG_SCALER_RES_SET_scal_hor_num(val)           SET_MASKED_BITS(LCDC_REG_SCALER_RES, val, 8, 15)
#define LCDC_REG_SCALER_RES_SET_scal_ver_num(val)           SET_MASKED_BITS(LCDC_REG_SCALER_RES, val, 0, 7)

enum lcdc_panel_type {
    lcdc_6bits_per_channel = 0,
    lcdc_8bits_per_channel,
};
enum lcdc_fb_data_endianness_type {
    lcdc_fb_data_endianness_lblp = 0, // little-endian byte little-endian pixel
    lcdc_fb_data_endianness_bbbp ,    // big-endian byte big-endian pixel
    lcdc_fb_data_endianness_lbbp ,    // little-endian byte big-endian pixel
};
enum lcdc_output_fmt_sel {
    lcdc_output_fmt_sel_rgb = 0, 
    lcdc_output_fmt_sel_bgr ,    
};
enum lcdc_img_pixfmt {
    lcdc_img_pixfmt_1bpp = 0,
    lcdc_img_pixfmt_2bpp ,
    lcdc_img_pixfmt_4bpp ,
    lcdc_img_pixfmt_8bpp ,
    lcdc_img_pixfmt_16bpp ,
    lcdc_img_pixfmt_24bpp ,
    lcdc_img_pixfmt_argb888 ,
    lcdc_img_pixfmt_argb1555,
};

struct lcdc_img_pixfmt_pxp {
    enum lcdc_img_pixfmt pixfmt_img0;
    enum lcdc_img_pixfmt pixfmt_img1;
    enum lcdc_img_pixfmt pixfmt_img2;
    enum lcdc_img_pixfmt pixfmt_img3;
};

enum alpha_blending_type {
    alpha_blending_none = 0,
    alpha_blending_global,
    alpha_blending_pixel
};


struct fb_info {
    void *par;    
};

extern int kdp520_lcdc_draw_cam(u32 addr, unsigned int cam_idx);

#if defined(CFG_SENSOR_TYPE) && (DISPLAY_DEVICE == DISPLAY_DEVICE_LCDC)
void kdp520_lcdc_set_panel_type(enum lcdc_panel_type type);
void kdp520_lcdc_set_bgrsw(enum lcdc_output_fmt_sel type);
void kdp520_lcdc_set_framerate(int frame_rate, u32 width, u32 height);
#else
static void kdp520_lcdc_set_panel_type(enum lcdc_panel_type type) {}
static void kdp520_lcdc_set_bgrsw(enum lcdc_output_fmt_sel type) {}
static void kdp520_lcdc_set_framerate(int frame_rate, u32 width, u32 height) {}
#endif

#endif
