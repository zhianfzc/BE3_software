#ifndef __BOARD_KL520_H__
#define __BOARD_KL520_H__
#include <stdio.h>
#include <string.h>
#include "string.h"
#include <cmsis_os2.h>
#include "cmsis_os2.h"                    // ARM::CMSIS:RTOS2:Keil RTX5
#include "types.h"
#include "delay.h"
#include "base.h"
#include "dbg.h"

#include "v2k_image.h"
#include "board_cfg.h"

extern u8 rgb_sensor_index;
extern u8 nir_sensor_index;
extern u8 sensor_0_mirror;
extern u8 sensor_0_flip;
extern u8 sensor_1_mirror;
extern u8 sensor_1_flip;

#define YES                         1
#define NO                          0

#define CFG_AI_3D_LIVENESS_IN_NONE  0
#define CFG_AI_3D_LIVENESS_IN_SCPU  1
#define CFG_AI_3D_LIVENESS_IN_NCPU  2

/* Tile average setting */
#if (CFG_SENSOR_1_TYPE == SENSOR_TYPE_SC035HGS && CFG_CAMERA_ROTATE == 1)
#define ALL_TILE_VALUE              (NO)
#else
#define ALL_TILE_VALUE              (YES)
#endif

#define TILE_AVG_32                 (32)
#define TILE_AVG_64                 (64)
#define TILE_AVG_128                (128)
#define TILE_AVG_CAL_SIZE           (TILE_AVG_128)

#if (ALL_TILE_VALUE == NO)  //only for SC035
#define TILE_AVG_BLOCK_X            (4)
#define TILE_AVG_BLOCK_Y            (5)
#else
#define TILE_AVG_BLOCK_X            (10)  //cannot modify
#define TILE_AVG_BLOCK_Y            (6)   //cannot modify
#endif
#define TILE_AVG_BLOCK_NUMBER       (TILE_AVG_BLOCK_X * TILE_AVG_BLOCK_Y)

#define TILE_AVG_VALID_X            ( ( (NIR_IMG_SOURCE_W / TILE_AVG_CAL_SIZE) < TILE_AVG_BLOCK_X ) ? (NIR_IMG_SOURCE_W / TILE_AVG_CAL_SIZE) : TILE_AVG_BLOCK_X )
#define TILE_AVG_VALID_Y            ( ( (NIR_IMG_SOURCE_H / TILE_AVG_CAL_SIZE) < TILE_AVG_BLOCK_Y ) ? (NIR_IMG_SOURCE_H / TILE_AVG_CAL_SIZE) : TILE_AVG_BLOCK_Y )

#if ( CFG_PALM_PRINT_MODE == YES )
#define BRIGHTNESS_STATS_X_BLOCK_NUM  ( 4 )
#define BRIGHTNESS_STATS_Y_BLOCK_NUM  ( 15 )
#define BRIGHTNESS_STATS_BLOCK_MAX    ( BRIGHTNESS_STATS_X_BLOCK_NUM * BRIGHTNESS_STATS_Y_BLOCK_NUM )  //less than "TILE_AVG_BLOCK_NUMBER"
#define BRIGHTNESS_STATS_X_BLOCK_SIZE ( 100 )   //fixed
#define BRIGHTNESS_STATS_Y_BLOCK_SIZE ( 26 )    //fixed
#define BRIGHTNESS_STATS_X_START      ( 100 )
#define BRIGHTNESS_STATS_Y_START      ( 100 )
#define BRIGHTNESS_STATS_STEP         ( 4 )     //fixed
#endif

/* The other definitions which are based on customer board configuration */
#ifdef CFG_SENSOR_TYPE
#define CFG_MIPIRX_ENABLE           YES
#endif

#define MIPI_0_RX_ENABLE            CFG_SENSOR_MIPI0_RX_EN
#define MIPI_1_RX_ENABLE            CFG_SENSOR_MIPI1_RX_EN
#define IMGSRC_0_TYPE               CFG_SENSOR_0_TYPE
#define IMGSRC_1_TYPE               CFG_SENSOR_1_TYPE
#define IMGSRC_0_RES                CFR_SENSOR_0_RES
#define IMGSRC_1_RES                CFR_SENSOR_1_RES
#define IMGSRC_0_FORMAT             CFR_SENSOR_0_FORMAT
#define IMGSRC_1_FORMAT             CFR_SENSOR_1_FORMAT
#define IMGSRC_0_MIPILANE_NUM       CFR_SENSOR_0_MIPILANE_NUM
#define IMGSRC_1_MIPILANE_NUM       CFR_SENSOR_1_MIPILANE_NUM
#define IMGSRC_NUM                  CFR_SENSOR_NUM

#if (CFG_SENSOR_TYPE == SENSOR_TYPE_GC2145_SC132GS)
#define MIPI_CAM_RGB                0
#define MIPI_CAM_NIR                1
#if CFG_SENSOR_0_FULL_RESOLUTION == YES
#define GC2145_FULL_RES             (YES)
#else
#define GC2145_FULL_RES             (NO)
#endif

#define RGB_IMG_SOURCE_W            CFG_SENSOR_0_WIDTH
#define RGB_IMG_SOURCE_H            CFG_SENSOR_0_HEIGHT

#if CFG_SENSOR_1_FULL_RESOLUTION == YES
#define SC132GS_FULL_RES            (YES)
#else
#define SC132GS_FULL_RES            (NO)
#endif

#define NIR_IMG_SOURCE_W            CFG_SENSOR_1_WIDTH
#define NIR_IMG_SOURCE_H            CFG_SENSOR_1_HEIGHT


#elif (CFG_SENSOR_TYPE == SENSOR_TYPE_SC132GS_GC2145)
#define MIPI_CAM_RGB                1
#define MIPI_CAM_NIR                0
#if CFG_SENSOR_0_FULL_RESOLUTION == YES
#define SC132GS_FULL_RES            (YES)
#else
#define SC132GS_FULL_RES            (NO)
#endif
#define RGB_IMG_SOURCE_W            CFG_SENSOR_1_WIDTH
#define RGB_IMG_SOURCE_H            CFG_SENSOR_1_HEIGHT

#if CFG_SENSOR_1_FULL_RESOLUTION == YES
#define GC2145_FULL_RES             (YES)
#else
#define GC2145_FULL_RES             (NO)
#endif
#define NIR_IMG_SOURCE_W            CFG_SENSOR_0_WIDTH
#define NIR_IMG_SOURCE_H            CFG_SENSOR_0_HEIGHT


#elif (CFG_SENSOR_TYPE == SENSOR_TYPE_HMX2056_OV9286)
#define MIPI_CAM_RGB                0
#define MIPI_CAM_NIR                1
#define RGB_IMG_SOURCE_W            640
#define RGB_IMG_SOURCE_H            480
#define NIR_IMG_SOURCE_W            480
#define NIR_IMG_SOURCE_H            640

#elif (CFG_SENSOR_TYPE == SENSOR_TYPE_HMXRICA)
#define MIPI_CAM_NIR                0
#define RGB_IMG_SOURCE_W            640
#define RGB_IMG_SOURCE_H            480
#define NIR_IMG_SOURCE_W            491
#define NIR_IMG_SOURCE_H            864

#elif (CFG_SENSOR_TYPE == SENSOR_TYPE_GC2145_SC035HGS)
#define MIPI_CAM_RGB                0
#define MIPI_CAM_NIR                1
#if CFG_SENSOR_0_FULL_RESOLUTION == YES
#define RGB_IMG_SOURCE_W            UGA_WIDTH
#define RGB_IMG_SOURCE_H            UGA_HEIGHT
#define GC2145_FULL_RES             (YES)
#else
#define RGB_IMG_SOURCE_W            640
#define RGB_IMG_SOURCE_H            480
#define GC2145_FULL_RES             (NO)
#endif
#define NIR_IMG_SOURCE_W            640
#define NIR_IMG_SOURCE_H            480
#define SC035HGS_FULL_RES            (NO)

#elif (CFG_SENSOR_TYPE == SENSOR_TYPE_GC1054_GC1054)
#define MIPI_CAM_RGB                rgb_sensor_index
#define MIPI_CAM_NIR                nir_sensor_index

 
#define RGB_IMG_SOURCE_W            1280
#define RGB_IMG_SOURCE_H            720
#define NIR_IMG_SOURCE_W            1280
#define NIR_IMG_SOURCE_H            720

#elif (CFG_SENSOR_TYPE == SENSOR_TYPE_GC02M1_GC1054)
#define MIPI_CAM_RGB                rgb_sensor_index
#define MIPI_CAM_NIR                nir_sensor_index
 
#define RGB_IMG_SOURCE_W            1280
#define RGB_IMG_SOURCE_H            720
#define NIR_IMG_SOURCE_W            1280
#define NIR_IMG_SOURCE_H            720

#elif (CFG_SENSOR_TYPE == SENSOR_TYPE_GC2145_GC1054)
#define MIPI_CAM_RGB                rgb_sensor_index
#define MIPI_CAM_NIR                nir_sensor_index
 
#define RGB_IMG_SOURCE_W            640
#define RGB_IMG_SOURCE_H            480
#define NIR_IMG_SOURCE_W            1280
#define NIR_IMG_SOURCE_H            720

#elif (CFG_SENSOR_TYPE == SENSOR_TYPE_BF20A1_BF20A1)
 
#define MIPI_CAM_RGB                rgb_sensor_index
#define MIPI_CAM_NIR                nir_sensor_index

#define RGB_IMG_SOURCE_W            CFG_SENSOR_0_WIDTH
#define RGB_IMG_SOURCE_H            CFG_SENSOR_0_HEIGHT

#define NIR_IMG_SOURCE_W            CFG_SENSOR_1_WIDTH
#define NIR_IMG_SOURCE_H            CFG_SENSOR_1_HEIGHT

#elif (CFG_SENSOR_TYPE == SENSOR_TYPE_OV02B1B_OV02B1B) || (CFG_SENSOR_TYPE == SENSOR_TYPE_SP2509_SP2509)
#define MIPI_CAM_RGB                rgb_sensor_index
#define MIPI_CAM_NIR                nir_sensor_index
 
#define RGB_IMG_SOURCE_W            CFG_SENSOR_0_WIDTH
#define RGB_IMG_SOURCE_H            CFG_SENSOR_0_HEIGHT
#define NIR_IMG_SOURCE_W            CFG_SENSOR_1_WIDTH
#define NIR_IMG_SOURCE_H            CFG_SENSOR_1_HEIGHT

#elif (CFG_SENSOR_TYPE == SENSOR_TYPE_USER_DEFINE)
#if (IMGSRC_0_FORMAT==IMAGE_FORMAT_RGB565) || (IMGSRC_0_FORMAT==IMAGE_FORMAT_YCBCR)
#define MIPI_CAM_RGB                0
#define RGB_IMG_SOURCE_W            CFG_SENSOR_0_WIDTH
#define RGB_IMG_SOURCE_H            CFG_SENSOR_0_HEIGHT
#elif (IMGSRC_1_FORMAT==IMAGE_FORMAT_RGB565) || (IMGSRC_1_FORMAT==IMAGE_FORMAT_YCBCR)
#define MIPI_CAM_RGB                1
#define RGB_IMG_SOURCE_W            CFG_SENSOR_1_WIDTH
#define RGB_IMG_SOURCE_H            CFG_SENSOR_1_HEIGHT
#endif
#if (IMGSRC_0_FORMAT==IMAGE_FORMAT_RAW8) || (IMGSRC_0_FORMAT==IMAGE_FORMAT_RAW10)
#define MIPI_CAM_NIR                0
#define NIR_IMG_SOURCE_W            CFG_SENSOR_0_WIDTH
#define NIR_IMG_SOURCE_H            CFG_SENSOR_0_HEIGHT
#elif (IMGSRC_1_FORMAT==IMAGE_FORMAT_RAW8) || (IMGSRC_1_FORMAT==IMAGE_FORMAT_RAW10)
#define MIPI_CAM_NIR                1
#define NIR_IMG_SOURCE_W            CFG_SENSOR_1_WIDTH
#define NIR_IMG_SOURCE_H            CFG_SENSOR_1_HEIGHT
#endif
#if ((IMGSRC_0_TYPE==SENSOR_TYPE_GC2145) && (IMGSRC_0_RES==RES_1600_1200)) \
    || ((IMGSRC_1_TYPE==SENSOR_TYPE_GC2145) && (IMGSRC_1_RES==RES_1600_1200))
#define GC2145_FULL_RES             (YES)
#else
#define GC2145_FULL_RES             (NO)
#endif
#if ((IMGSRC_0_TYPE==SENSOR_TYPE_SC132GS) && (IMGSRC_0_RES==RES_1080_1280)) \
    || ((IMGSRC_1_TYPE==SENSOR_TYPE_SC132GS) && (IMGSRC_1_RES==RES_1080_1280))
#define SC132GS_FULL_RES             (YES)
#else
#define SC132GS_FULL_RES             (NO)
#endif

#else
#define MIPI_0_RX_ENABLE            YES
#define MIPI_1_RX_ENABLE            NO
#define IMGSRC_NUM                  1
#endif

#if CFG_PANEL_TYPE == PANEL_NULL || CFG_PANEL_TYPE == PANEL_MZT_480X272
#define PANEL_WIDTH     (TFT43_WIDTH)
#define PANEL_HEIGHT    (TFT43_HEIGHT)
#elif (CFG_PANEL_TYPE == PANEL_ST7789_240X320 || CFG_PANEL_TYPE == PANEL_ST7789_240X320_SPI || (CFG_PANEL_TYPE==PANEL_ST7789_240X320_8080_AND_SPI) )
#define PANEL_WIDTH     (QVGA_PORTRAIT_WIDTH)
#define PANEL_HEIGHT    (QVGA_PORTRAIT_HEIGHT)
#elif (CFG_PANEL_TYPE == PANEL_ST7789_320X240)
#define PANEL_WIDTH     (QVGA_LANDSCAPE_WIDTH)
#define PANEL_HEIGHT    (QVGA_LANDSCAPE_HEIGHT)
#elif (CFG_PANEL_TYPE == PANEL_MZT)
#define PANEL_WIDTH     (VGA_LANDSCAPE_WIDTH)
#define PANEL_HEIGHT    (VGA_LANDSCAPE_HEIGHT)
#endif

#if CFG_PANEL_TYPE == PANEL_NULL
#   define DISPLAY_DEVICE           DISPLAY_DEVICE_UNKNOWN
#elif CFG_PANEL_TYPE == PANEL_MZT_480X272
#   define DISPLAY_DEVICE           DISPLAY_DEVICE_LCDC
#   if CFG_DISPLAY_DMA_ENABLE == YES
#       define CFG_LCM_DMA_ENABLE YES
#   endif
#elif (CFG_PANEL_TYPE == PANEL_ST7789_240X320) || (CFG_PANEL_TYPE == PANEL_ST7789_320X240) || (CFG_PANEL_TYPE==PANEL_ST7789_240X320_8080_AND_SPI)
#   define DISPLAY_DEVICE           DISPLAY_DEVICE_LCM
#   if CFG_DISPLAY_DMA_ENABLE == YES
#       define CFG_LCM_DMA_ENABLE YES
#   endif
#elif (CFG_PANEL_TYPE == PANEL_ST7789_240X320_SPI)
#   define DISPLAY_DEVICE           DISPLAY_DEVICE_SPI_LCD
#   if CFG_DISPLAY_DMA_ENABLE == YES
#       define CFG_LCM_DMA_ENABLE YES
#   endif
#else
#   define DISPLAY_DEVICE           DISPLAY_DEVICE_UNKNOWN
#endif

#define KDP_BIT_CTRL_MODE                   ( NO )
//#define KDP_REMOTE_CTRL_BY_TYPE				( NO )

#ifdef CFG_CATEYE_COMMON_MIPI
#define CATEYE_COMMON_MIPI                  ( YES )
#else
#define CATEYE_COMMON_MIPI                  ( NO )
#endif
#endif
