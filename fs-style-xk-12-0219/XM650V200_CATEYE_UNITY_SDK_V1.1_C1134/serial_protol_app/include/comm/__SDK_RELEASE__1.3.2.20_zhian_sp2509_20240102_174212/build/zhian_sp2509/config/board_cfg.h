#ifndef __BOARD_CFG_H__
#define __BOARD_CFG_H__


// PRE-definition for board configuration
#define SENSOR_TYPE_NULL                                                                -1
#define SENSOR_TYPE_HMX2056                                                              0
#define SENSOR_TYPE_OV9286                                                               1
#define SENSOR_TYPE_HMXRICA                                                              2
#define SENSOR_TYPE_GC2145                                                               3
#define SENSOR_TYPE_SC132GS                                                              4
#define SENSOR_TYPE_SC035HGS                                                             5
#define SENSOR_TYPE_GC1054_R                                                             6
#define SENSOR_TYPE_GC1054_L                                                             7
#define SENSOR_TYPE_SP2509_R                                                             8
#define SENSOR_TYPE_SP2509_L                                                             9
#define SENSOR_TYPE_MIXO3238                                                            10
#define SENSOR_TYPE_BF20A1_R                                                            11
#define SENSOR_TYPE_BF20A1_L                                                            12
#define SENSOR_TYPE_OV02B1B_R                                                           13
#define SENSOR_TYPE_OV02B1B_L                                                           14
#define SENSOR_TYPE_MAX                                                                 15
#define SENSOR_TYPE_HMX2056_OV9286                                                      16
#define SENSOR_TYPE_GC2145_SC132GS                                                      17
#define SENSOR_TYPE_SC132GS_GC2145                                                      18
#define SENSOR_TYPE_GC2145_SC035HGS                                                      19
#define SENSOR_TYPE_GC1054_GC1054                                                       20
#define SENSOR_TYPE_SP2509_SP2509                                                       21
#define SENSOR_TYPE_BF20A1_BF20A1                                                       22
#define SENSOR_TYPE_OV02B1B_OV02B1B                                                      23
#define SENSOR_TYPE_USER_DEFINE                                                       0xff
#define LED_DRIVER_AW36404                                                               0
#define LED_DRIVER_AW36515                                                               1
#define LED_DRIVER_GPIO                                                                  2
#define RES_640_480                                                                      0
#define RES_480_640                                                                      1
#define RES_480_272                                                                      2
#define RES_272_480                                                                      3
#define RES_864_491                                                                      4
#define RES_1600_1200                                                                    5
#define RES_1080_1280                                                                    6
#define RES_1280_720                                                                     7
#define RES_1920_1080                                                                    8
#define RES_800_600                                                                      9
#define RES_USER_DEFINE                                                                 10
#define IMAGE_FORMAT_RGB565                                                              0
#define IMAGE_FORMAT_RAW10                                                               1
#define IMAGE_FORMAT_RAW8                                                                2
#define IMAGE_FORMAT_YCBCR                                                               3
#define IMAGE_MIPILANE_NUM_1                                                             1
#define IMAGE_MIPILANE_NUM_2                                                             2
#define DISPLAY_DEVICE_UNKNOWN                                                           0
#define DISPLAY_DEVICE_LCDC                                                              1
#define DISPLAY_DEVICE_LCM                                                               2
#define DISPLAY_DEVICE_SPI_LCD                                                           3
#define DISPLAY_DEVICE_LCM_AND_SPI_LCD                                                       4
#define PANEL_NULL                                                                       0
#define PANEL_MZT_480X272                                                                1
#define PANEL_ST7789_240X320                                                             2
#define PANEL_ST7789_320X240                                                             3
#define PANEL_MZT                                                                        4
#define PANEL_ST7789_240X320_SPI                                                         5
#define PANEL_ST7789_240X320_8080_AND_SPI                                                       6
#define CFG_SENSOR_TYPE                                          SENSOR_TYPE_SP2509_SP2509
#define CFG_SENSOR_MIPI0_RX_EN                                                           1
#define CFG_SENSOR_MIPI1_RX_EN                                                           1
#define CFG_SENSOR_0_TYPE                                             SENSOR_TYPE_SP2509_R
#define CFR_SENSOR_0_FORMAT                                              IMAGE_FORMAT_RAW8
#define CFR_SENSOR_0_MIPILANE_NUM                                     IMAGE_MIPILANE_NUM_1
#define CFR_SENSOR_0_RES                                                       RES_800_600
#define CFG_SENSOR_0_WIDTH                                                             800
#define CFG_SENSOR_0_HEIGHT                                                            600
#define CFG_SENSOR_0_FMT_MIRROR                                                          0
#define CFG_SENSOR_0_FMT_FLIP                                                            1
#define CFG_SENSOR_0_I2C_ADDR                                                         0x3d
#define CFG_SENSOR_1_TYPE                                             SENSOR_TYPE_SP2509_L
#define CFR_SENSOR_1_FORMAT                                              IMAGE_FORMAT_RAW8
#define CFR_SENSOR_1_MIPILANE_NUM                                     IMAGE_MIPILANE_NUM_1
#define CFR_SENSOR_1_RES                                                       RES_800_600
#define CFG_SENSOR_1_WIDTH                                                             800
#define CFG_SENSOR_1_HEIGHT                                                            600
#define CFG_SENSOR_1_FMT_MIRROR                                                          0
#define CFG_SENSOR_1_FMT_FLIP                                                            1
#define CFG_SENSOR_1_I2C_ADDR                                                         0x3d
#define CFR_SENSOR_NUM                                                                   2
#define CFR_CAM_RGB                                                                      0
#define CFR_CAM_NIR                                                                      1
#define CFG_SENSOR_0_FULL_RESOLUTION                                                       1
#define CFG_SENSOR_1_FULL_RESOLUTION                                                       1
#define CFG_PANEL_TYPE                                                          PANEL_NULL
#define CFG_TOUCH_X_RANGE_MAX                                                          240
#define CFG_TOUCH_Y_RANGE_MAX                                                          320
#define CFG_TOUCH_X_AXIS_INVERSE                                                         0
#define CFG_TOUCH_Y_AXIS_INVERSE                                                         1
#define CFG_DISPLAY_DMA_ENABLE                                                           1
#define CFG_PREFER_DISPLAY                                                               1
#define CFG_I2C_0_ENABLE                                                                 1
#define CFG_I2C_1_ENABLE                                                                 1
#define CFG_I2C_2_ENABLE                                                                 0
#define CFG_I2C_3_ENABLE                                                                 0
#define CFG_UART0_ENABLE                                                                 1
#define CFG_UART1_ENABLE                                                                 0
#define CFG_UART1_TX_DMA_ENABLE                                                          0
#define CFG_UART1_RX_DMA_ENABLE                                                          0
#define CFG_UART2_ENABLE                                                                 1
#define CFG_UART2_TX_DMA_ENABLE                                                          0
#define CFG_UART2_RX_DMA_ENABLE                                                          0
#define CFG_UART3_ENABLE                                                                 0
#define CFG_UART3_TX_DMA_ENABLE                                                          0
#define CFG_UART3_RX_DMA_ENABLE                                                          0
#define CFG_UART4_ENABLE                                                                 1
#define CFG_UART4_TX_DMA_ENABLE                                                          0
#define CFG_UART4_RX_DMA_ENABLE                                                          0
#define CFG_ADC0_ENABLE                                                                  0
#define CFG_ADC0_DMA_ENABLE                                                              0
#define CFG_ADC1_ENABLE                                                                  0
#define CFG_ADC1_DMA_ENABLE                                                              0
#define CFG_ADC2_ENABLE                                                                  0
#define CFG_ADC2_DMA_ENABLE                                                              0
#define CFG_ADC3_ENABLE                                                                  0
#define CFG_ADC3_DMA_ENABLE                                                              0
#define CFG_PWM1_DMA_ENABLE                                                              0
#define CFG_PWM2_DMA_ENABLE                                                              0
#define CFG_PWM3_DMA_ENABLE                                                              0
#define CFG_PWM4_DMA_ENABLE                                                              0
#define CFG_PWM5_DMA_ENABLE                                                              0
#define CFG_PWM6_DMA_ENABLE                                                              0
#define CFG_SSP0_ENABLE                                                                  0
#define CFG_SSP0_TX_DMA_ENABLE                                                           0
#define CFG_SSP0_RX_DMA_ENABLE                                                           0
#define CFG_SSP1_ENABLE                                                                  1
#define CFG_SSP1_TX_DMA_ENABLE                                                           0
#define CFG_SSP1_RX_DMA_ENABLE                                                           0
#define CFG_SPI_ENABLE                                                                   1
#define CFG_SPI_DMA_ENABLE                                                               0
#define CFG_SD_ENABLE                                                                    1
#define CFG_SD_DMA_ENABLE                                                                0
#define CFG_USBD_ENABLE                                                                  1
#define CFG_USBH_ENABLE                                                                  0
#define CFG_USB_OTG_ENABLE                                                               0
#define CFG_UI_USR_IMG                                                                   1
#define CFG_OTA_EN                                                                       0
#define CFG_LED_DRIVER_TYPE                                                LED_DRIVER_GPIO
// =======================================================================================
#define AI_TYPE_R1                                                                       0
#define AI_TYPE_R1N1                                                                     1
#define AI_TYPE_N1                                                                       2
#define AI_TYPE_N1R1                                                                     3
#define AI_TYPE_PR1                                                                      4
#define AI_TYPE_UPDATE_FMAP                                                              5
#define CFG_AI_TYPE                                                           AI_TYPE_N1R1
#define CFG_AI_3D_ENABLE                                                                 1
#define CFG_AI_3D_LIVENESS_IN                                                            2
#define CFG_AI_USE_FIXED_IMG                                                             0
#define USE_N1_FACE_POSE                                                                 1
#define USE_FUSE_LV__MODEL                                                               1
#define CFG_RGB_CV_LIVENESS                                                              0
#define USE_LIVENESS_CV                                                                  0
#define USE_FACE_QUALITY                                                                 1
#define KL520_FACE_ADD_BMP                                                               1
#define CFG_LW3D_NORMAL                                                                  0
#define CFG_LW3D_850                                                                     1
#define CFG_LW3D_940                                                                     2
#define CFG_LW3D_TYPE                                                         CFG_LW3D_850
#define CFG_FLASH_DB_NIR_ONLY                                                            1
#define CFG_REUSE_PREPROC                                                                1
#define CFG_CAMERA_ROTATE                                                                1
#define CFG_CAMERA_DUAL_1054                                                             1
#define CFG_NIR_MODE2_SPLIT                                                              1
#define CFG_FCOS_FD_ROTATE                                                               1
#define CFG_ZHIAN                                                                        1
#define CFG_LM_ONET_PLUS                                                                 1
#define CFG_REC_ERR_NOT_RET                                                              1
#define CFG_LED_CTRL_ENHANCE                                                             0
#define CFG_MODELS_LOAD_BY_ORDER                                                         1
#define CFG_MAX_USER                                                                   100
#define CFG_ONE_SHOT_MODE                                                                1
#define CFG_ONE_SHOT_ENHANCE                                                             1
#define CFG_ONESHOT_PARALLEL                                                             0
#define CFG_USB_EXPORT_LIVENESS_RET                                                       1
#define CUSTOMIZE_DB_OFFSET                                                 0x01 //zcymod 
#define CFG_DEL_CALIBRATION_SETTING_WHEN_DEL_ALL                                                       1
// CFG_E2E_SETTING========================================================================
#define CFG_E2E_STRUCT_LIGHT                                                             0
#define CFG_E2E_NIR_TWO_STAGE_LIGHT                                                       0
#define CFG_E2E_CHECK_POSITION                                                           1
#define CFG_E2E_REC_NOTE                                                                 1
#define CFG_E2E_RGB_LED_STRENGTH                                                       100
#define CFG_E2E_NIR_LED_STRENGTH                                                        60
#define CFG_E2E_RGB_LED_DEFAULT_DIM_ENV_STRENGTH                                                       2
#define CFG_E2E_RGB_LED_WEAK_ENHANCE_COUNT                                                     200
#define CFG_E2E_RGB_LED_WEAK_ENHANCE_STEP                                                       2
#define CFG_E2E_RGB_LED_WEAK_ENHANCE_MAX                                                      20
#define CFG_E2E_RGB_LED_WEAK_ENHANCE_MIN                                                       4
#define CFG_E2E_RGB_LED_DEFAULT_DARK_ENV_STRENGTH                                                      15
#define CFG_E2E_RGB_LED_STRONG_ENHANCE_COUNT                                                    1000
#define CFG_E2E_RGB_LED_STRONG_ENHANCE_STRONG_STEP                                                       3
#define CFG_E2E_RGB_LED_STRONG_ENHANCE_STRONG_MAX                                                      80
#define CFG_E2E_RGB_LED_STRONG_ENHANCE_STRONG_MIN                                                      20
// =======================================================================================
//  1.Different rgb led on different hardware 2.bctc liveness version 
//  Recognition and liveness in dark room is affected by these variable.
// =======================================================================================
#define CFG_IGNORE_RGB_LED                                                               1
#define CFG_LV_HARD                                                                      0
// =======================================================================================
#define CFG_TOUCH_ENABLE                                                                 0
#define TOUCH_INT_PIN                                                                   27
#define TOUCH_TYPE_CT130                                                                 0
#define TOUCH_TYPE_FT5X06                                                                1
#define CFG_TOUCH_TYPE                                                    TOUCH_TYPE_CT130
#define CFG_SNAPSHOT_ENABLE                                                              0
#define CFG_USB_SIMTOOL                                                                  0
#define CFG_SNAPSHOT_INFO                                                                1
#define CFG_SNAPSHOT_NUMS                                                               10
#define CFG_SNAPSHOT_ADVANCED                                                            0
#define CFG_OTA_FLASH_BUF_ENABLE                                                         1
#define CFG_OTA_IMAGE_BUF_ENABLE                                                         1
#define CFG_KDP_SETTINGS_ENABLE                                                          1
#define CFG_KDP_SETTINGS_SIZE                                                        20480
#define CFG_USR_SETTINGS_ENABLE                                                          1
#define CFG_USR_SETTINGS_SIZE                                                         4096
#define CFG_GUI_ENABLE                                                                   0
#define CFG_FMAP_EXTRA_ENABLE                                                            0
#define DISPLAY_REGISTER_WITH_CUSTOM_IMG                                                       1
// =======================================================================================
// EXPORT_INFORMATION like stream, db, fm, etc...
// =======================================================================================
#define EX_FM_DISABLE                                                                    0
#define EX_FM_USB_AP_CTRL_ALL                                                            1
#define EX_FM_USB_AP_CTRL_MAIN_DB                                                        2
#define EX_FM_UART_AP_CTRL_MAIN_DB                                                       3
#define CFG_FMAP_AP_CTRL_TYPE                                   EX_FM_UART_AP_CTRL_MAIN_DB
#define CFG_FMAP_EX_FIG_ENABLE                                                           0
#define CFG_USB_PROGRAME_FW_ENABLE                                                       0
#define CFG_USB_EXPORT_STREAM_IMG                                                        0
#define CFG_USB_CLOUD_DB_UPDATE                                                          0
// =======================================================================================
// Display Setting
// =======================================================================================
#define DISPLAY_ENABLE                                                                   0
#define DISPLAY_DOWNSCALE                                                                0
#define DISPLAY_WIDTH                                                                  320
#define DISPLAY_HEIGHT                                                                 240
#define DISPLAY_RGB_WIDTH                                                             1280
#define DISPLAY_RGB_HEIGHT                                                             720
#define DISPLAY_RGB_X_OFFSET                                                             0
#define DISPLAY_RGB_Y_OFFSET                                                             0
#define DISPLAY_RGB_OUT_WIDTH                                                          320
#define DISPLAY_RGB_OUT_HEIGHT                                                         240
#define DISPLAY_NIR_WIDTH                                                             1280
#define DISPLAY_NIR_HEIGHT                                                             720
#define DISPLAY_NIR_X_OFFSET                                                             0
#define DISPLAY_NIR_Y_OFFSET                                                             0
#define DISPLAY_NIR_OUT_WIDTH                                                          320
#define DISPLAY_NIR_OUT_HEIGHT                                                         240
#define PANEL_IN_WIDTH                                                                 320
#define PANEL_IN_HEIGHT                                                                240
#define PANEL_RGB_X_OFFSET                                                               0
#define PANEL_RGB_Y_OFFSET                                                               0
#define PANEL_NIR_X_OFFSET                                                               0
#define PANEL_NIR_Y_OFFSET                                                               0
// CFG_ST7789_X_Y_INVERSE=1
// =======================================================================================
// In this section there are some macros used to selection communication interface which you want
// If you want to use snapshot or OTA, you need select the correct bus type
// If you select COM_BUS_TYPE_UART4, and then you need filled it to CFG_COM_BUS_TYPE
// For example: "#define CFG_COM_BUS_TYPE  COM_BUS_TYPE_UART4"
// =======================================================================================
#define COM_BUS_UART_MASK                                                       0x0000007F
#define COM_BUS_USB_MASK                                                        0x00000080
#define COM_BUS_SPI_MASK                                                        0x00000F00
#define COM_BUS_I2C_MASK                                                        0x0000F000
#define COM_BUS_SDIO_MASK                                                       0x000F0000
#define COM_BUS_OTG_MASK                                                        0x00100000
#define COM_BUS_UART0                                                                 0x01
#define COM_BUS_UART1                                                                 0x02
#define COM_BUS_UART2                                                                 0x04
#define COM_BUS_UART3                                                                 0x08
#define COM_BUS_UART4                                                                 0x10
#define COM_BUS_USB                                                                   0x80
#define COM_BUS_SSP0                                                                 0x100
#define COM_BUS_SSP1                                                                 0x200
#define COM_BUS_SPI_MS_EN                                                            0x800
#define COM_BUS_I2C0                                                                0x1000
#define COM_BUS_I2C1                                                                0x2000
#define COM_BUS_I2C2                                                                0x4000
#define COM_BUS_I2C3                                                                0x8000
#define COM_BUS_SDIO                                                               0x10000
#define COM_BUS_OTG                                                               0x100000
#define CFG_COM_BUS_TYPE                                                   (COM_BUS_UART2)
// =======================================================================================
#define COM_PROTOCOL_TYPE_LWCOM                                                          0
#define COM_PROTOCOL_TYPE_KCOMM                                                          1
#define CFG_COM_PROTOCOL_TYPE                                      COM_PROTOCOL_TYPE_LWCOM
#define COM_USB_PROT_DEF                                                                 0
#define COM_USB_PROT_KDP                                                                 1
#define COM_USB_PROT_DEF_USR                                                             2
#define COM_USB_PROT_KDP_USR                                                             3
#define CFG_COM_USB_PROT_TYPE                                             COM_USB_PROT_DEF
#define COM_UART_PROT_DEF                                                                0
#define COM_UART_PROT_KDP                                                                1
#define COM_UART_PROT_DEF_USR                                                            2
#define COM_UART_PROT_KDP_USR                                                            3
#define CFG_COM_URT_PROT_TYPE                                            COM_UART_PROT_KDP
#define NO_ENCRYPTION                                                                    0
#define AES_ENCRYPTION                                                                0x01
#define XOR_ENCRYPTION                                                                0x02
#define ENCRYPTION_MODE                                    (AES_ENCRYPTION|XOR_ENCRYPTION)
#define COM_UART_MSG_KDP                                                                 0
#define COM_UART_MSG_USER                                                                1
#define CFG_COM_UART_MSG                                                 COM_UART_MSG_USER
// =======================================================================================
#define CFG_USB_MANUFACTURER                         {'K',0,'n',0,'e',0,'r',0,'o',0,'n',0}
#define CFG_USB_PRODUCT                              {'K',0,'n',0,'e',0,'r',0,'o',0,'n',0}
#define CFG_USB_SERIAL                   {'5',0,'5',0,'6',0,'6',0,'7',0,'7',0,'8',0,'8',0}
#define BOARD_VERSION                                                                    0
#define FAT_FS                                                                           0
#define PETIT_FS                                                                         1
#define CFG_OTG_FSTYPE                                                            PETIT_FS
#define KL520A                                                                           0
#define KL520B                                                                           1
#define CFG_KL520_VERSION                                                           KL520B
// =======================================================================================
//  CFG_BOARD_PARAMS_0_ID = X. Means the Xth parameter group of CFG_BOARD_PARAMS_0.
//  The board uses the same camera sensor, but the module components are different, such as lens.
//  developer please refer to /doc/developer_only/board_cfg_desc.txt for more details.
// =======================================================================================
#define CFG_BOARD_PARAMS_0_ID                                                            2
#define CFG_BOARD_PARAMS_0_0      {65.0f,130.0f,0.75f,1.33f,1.33f,-120.0f,40.0f,1.385f,0.8f,0.8f,1.00f,1.0f}
#define CFG_BOARD_PARAMS_0_1      {65.0f,130.0f,0.7825f,0.98f,0.942f,-160.0f,6.0f,1.0f,1.14f,1.00f,1.385f,150.0f}
#define CFG_BOARD_PARAMS_0_2      {130.0f,65.0f,-0.35f,1.0f,1.0f,-160.0f,6.0f,1.0f,1.14f,1.00f,1.385f,150.0f}
#define CFG_BOARD_PARAMS_1_ID                                                            0
#define CFG_BOARD_PARAMS_1_0                                                 {2.77f,94.0f}
// =======================================================================================
//  Memory selection for different flash vendor
// =======================================================================================
#define IMAGE_16MB                                                                      16
#define IMAGE_32MB                                                                      32
#define IMAGE_64MB                                                                      64
#define IMAGE_SIZE                                                              IMAGE_16MB
#define FLASH_16MB                                                                      16
#define FLASH_32MB                                                                      32
#define FLASH_64MB                                                                      64
#define FLASH_SIZE                                                              FLASH_32MB
#define GD25Q256D                                                                        0
#define GD25S512MD                                                                       1
#define W25Q256JV                                                                        2
#define FLASH_VENDOR_SELECT                                                      W25Q256JV
#define CFG_OTA_FLASH_SLAVE_ENABLE                                                       0
#define CFG_USB_EXPORT_LIVENESS_RET                                                       1
// =======================================================================================
//  mipi pll setting
//  for more different setting please refer to /doc/developer_only/csirx_pll_setting_table.xslx for more details.
// =======================================================================================
#define CFG_PLL_MS                                                                       2
#define CFG_PLL_NS                                                                     200
#define CFG_PLL_PS                                                                       2
#define CFG_CSIRX0_TXESCCLK                                                              3
#define CFG_CSIRX0_CSI                                                                  16
#define CFG_CSIRX0_VC0                                                                   7
#define CFG_CSIRX1_TXESCCLK_PLL3                                                         3
#define CFG_CSIRX1_CSI                                                                  16
#define CFG_CSIRX1_VC0                                                                   7
#define MIPI_PRE_DEF1                                             {2,268,2,5,27,13,5,15,4}
#define MIPI_PRE_DEF2                                              {1,210,3,5,15,3,2,31,7}
#define MIPI_PRE_DEF3                                               {2,242,2,4,11,5,4,7,1}
#define MIPI_PRE_DEF4                                               {2,242,2,4,7,1,4,11,5}
#define MIPI_PRE_DEF5                                               {2,200,2,3,9,4,3,13,3}
#define MIPI_PRE_DEF6                                              {2,200,2,3,16,7,3,16,7}
#define MIPI_PRE_DEF7                                              {2,200,2,3,12,2,3,16,7}
#define MIPI_PRE_DEF8                                              {2,200,2,3,16,7,3,12,2}
#define MIPI_PRE_DEF9                                              {2,300,2,2,3,5,5,27,10}
#define MIPI_PRE_DEF10                                           {2,200,2,3,24,24,3,24,24}
// ====1600x1200====
#define MIPI_PRE_DEF11                                             {2,268,2,4,10,4,4,10,4}
// ================
// =====800x600=====
#define MIPI_PRE_DEF12                                             {2,268,2,4,28,9,4,28,9}
// ================
#define MIPI_PRE_DEF13                                             {2,400,2,3,26,7,3,26,7}
#define MIPI_CUSTOM               {CFG_PLL_MS,CFG_PLL_NS,CFG_PLL_PS,CFG_CSIRX0_TXESCCLK,CFG_CSIRX0_CSI,CFG_CSIRX0_VC0,CFG_CSIRX1_TXESCCLK_PLL3,CFG_CSIRX1_CSI,CFG_CSIRX1_VC0}
#define CFG_MIPI_PLL_SETTING                                                MIPI_PRE_DEF13
// =======================================================================================
// =======================================================================================

#endif
