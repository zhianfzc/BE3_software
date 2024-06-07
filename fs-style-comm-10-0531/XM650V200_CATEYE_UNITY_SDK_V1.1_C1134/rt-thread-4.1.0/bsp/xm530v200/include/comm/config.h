#ifndef __CONFIG_H__
#define __CONFIG_H__

//default
#define UVC_DAY_NIGHT_SWITCH

//#define PLANTFORM_XM530V202_XM530V203
//#define SOFTWARE_TRIGGER_USB_UP
//#define RGB_MIRROR
//#define IA_ROTATE_CLOCKWISE_INVERT
//#define UVC_IR_SIGNLE
//#define UVC_COLOR_IR_SWITCH
//#define UVC_IR_NO
//#define NO_IA_LAST_TIME
//#define DOUBLE_CAMERA_CLOSE_RLED
//#define USB_APP
//#define USB_UVC_BULK_COMPOITE_DEVICE
//#define UVC_864_480_ADD_640_368
//#define UVC_720P_DELETE_640X480_ADD_864X480
//#define CONFIG_MIRROR_FILP_ASYNC
//#define IA_IMAGE_MIRROR_NOT_FLIP
//#define CUSTOM_DOUBLE_CAMERM_STREAM

//#define RGB_HORI_IR_VERT
//#define RGB_IA_GET_FROM_UVC
//#define WLED_AND_RLED
//#define DEVICE_VERT
//#define OSD_ADD_NOISE
//#define SENSOR_IR_FLIP_MIRROR
//#define CROP_OCCLUDE_DARK_CORNOR_720P
//#define CROP_RGB_HORI_IR_VERT_720P_800x480
//#define CROP_OCCLUDE_DARK_CORNOR_720P_1
//#define CROP_OCCLUDE_DARK_CORNOR_720P_3
//#define CROP_OCCLUDE_DARK_CORNOR_IR_720P_1
//#define CAPTURE_FACE_PICTURE
//#define UNLIMIT_INTERACTIVE_ENROLL_ANGEL
//#define UVC_720P_INDEX_COUNT_8

#define APP_DEBUG_LOG 1 /* debug log 1:on 0:off */

#define FEAT_FLASH_ADDR 	0x250000
#define CONFIG_VIRT_ADDR 	0x83300000
#define IA_CLS_VIRT_ADDR 	0x83301000

#define REGS_ENABLE			0xC00
#define REGS_DISABLE		0x400

#ifdef PLANTFORM_XM530V202_XM530V203
//#define PLANTFORM_XM530V202_XM
#define REGS_RLED_ADDR_V202		0x10020034
#define REGS_WLED_ADDR_V202		0x10020280
#define REGS_AO_EN_ADDR_V202	0x1002002C

#define REGS_RLED_ADDR_V203		0x10020030
#define REGS_AO_EN_ADDR_V203	0x10020028
#else
#define REGS_RLED_ADDR		0x1002015C
#define REGS_AO_EN_ADDR	 	0x10020158
#endif

#define FORE_TIME 130000

#define CONFIG_INFO_FLASH_DEVICE_NAME "flash1"

#define OBJECT_ITEM "Item"
#define OBJECT_USB_VERSION "UsbVersion"
#define OBJECT_USB_TRANSFER "UsbTransfer"
#define OBJECT_USB_MPS "UsbMps"
#define OBJECT_RESOLUTION "Resolution"
#define OBJECT_VIDEO_FPS "VideoFps"
#define OBJECT_JPG_ENCODE "JpgEncode"
#define OBJECT_JPG_QFACTOR "JpgQfactor"
#define OBJECT_LENS "Lens"
#define OBJECT_MIRROR "Mirror"
#define OBJECT_FLIP "Flip"
#define OBJECT_UAC "Uac"
#define OBJECT_AUDIO_FERQ "AudioFerq"
#define OBJECT_FRAME_SIZE "FrameSize"
#define OBJECT_IA_FACE_WIDTH "IaFaceWidth"

enum ITEM_INFO_E
{
	ITEM_INFO_DEFAULT = 0,
	ITEM_INFO_GENERAL = 1,
};

enum USB_VERSION_INFO_E
{
	USB_VERSION_INFO_2_0 = 0,
	USB_VERSION_INFO_1_1 = 1,
};

enum USB_TRANSFER_INFO_E
{
	USB_TRANSFER_INFO_ISO = 0,
	USB_TRANSFER_INFO_BULK = 1,
};

/* default:RESOLUTION_INFO_480_320 */
enum RESOLUTION_INFO_E
{
	RESOLUTION_INFO_480_320 = 0, 	/* 1.480x320 2.640x480 3.800x480 4.720P 5.1280x480 */
	RESOLUTION_INFO_480_320_1 = 1, 	/* 1.480x320 2.480x800 3.480x864 4.800x480 5.1280x480 */
	RESOLUTION_INFO_480_320_2 = 2,	/* 1.480x320 2.480x800 3.480x864 4.800x480 5.864x480 */
	RESOLUTION_INFO_640_480 = 3, 	/* 1.640x480 2.480x320 3.800x480 4.720P 5.1280x480 */
	RESOLUTION_INFO_800_480 = 4, 	/* 1.800x480 2.720P 3.640x480 4.480x320 5.1280x480 */
	RESOLUTION_INFO_864_480 = 5,	/* 1.864x480 2.800x480 3.640x480 4.480x320 5.1280x480 */
	RESOLUTION_INFO_864_480_2 = 6,	/* 1.864x480 2.800x480 3.320x240 4.480x320 5.1280x480 */
	RESOLUTION_INFO_720P = 7,		/* 1.720P 2.800x480 3.640x480 4.480x320 5.1280x480 */
	RESOLUTION_INFO_720P_1 = 8,		/* 1.720P 2.864x480 3.800x480 4.640x480 5.480x320  */
	RESOLUTION_INFO_720P_2 = 9,		/* 1.720P 2.864x480 3.800x480 4.640x480 5.1280x480 */
	RESOLUTION_INFO_720P_3 = 10,	/* 1.720P 2.864x480 3.800x480 4.320x240 5.480x320 */
	RESOLUTION_INFO_480_864 = 11,	/* 1.480x864 2.480x800 3.480x320 4.320x240 5.1280x480 */
	RESOLUTION_INFO_854_480 = 12,	/* 1.480x320 2.320x240 3.800x480 4.854x480 5.720P */
	RESOLUTION_INFO_1024_600 = 13,	/* 1.1024x600 2.640x480 3.800x480 4.720P 5.1280x480 */
	RESOLUTION_INFO_864_480_3 = 14,	/* 1.864x480 2.800x480 3.320x240 4.480x320 5.854*480 */
};

enum JPG_ENCODE_INFO_E
{
	JPG_ENCODE_YUV420 = 0,
	JPG_ENCODE_YUV422 = 1,
};

enum LENS_INFO_E
{
	LENS_INFO_RGB940_IR = 0,
	LENS_INFO_RGB850_IR = 1,
};

enum MIRROR_INFO_E
{
	MIRROR_INFO_OFF = 0,
	MIRROR_INFO_ON = 1,
};

enum FLIP_INFO_E
{
	FLIP_INFO_OFF = 0,
	FLIP_INFO_ON = 1,
};

enum UAC_INFO_E
{
	UAC_INFO_OFF = 0,
	UAC_INFO_ON = 1,
};

enum AUDIO_FERQ_INFO_E
{
	AUDIO_FERQ_INFO_8K = 8000,
	AUDIO_FERQ_INFO_16K = 16000,
};

typedef struct config_info_st
{
	unsigned int CONFIG_INFO_ITEM;
	unsigned int CONFIG_INFO_USB_VERSION;
	unsigned int CONFIG_INFO_USB_TRANSFER;
	unsigned int CONFIG_INFO_USB_MPS;
	unsigned int CONFIG_INFO_RESOLUTION;
	unsigned int CONFIG_INFO_VIDEO_FPS;
	unsigned int CONFIG_INFO_JPG_ENCODE;
	unsigned int CONFIG_INFO_JPG_QFACTOR;
	unsigned int CONFIG_INFO_LENS;
	unsigned int CONFIG_INFO_FLIP;
	unsigned int CONFIG_INFO_MIRROR;
	unsigned int CONFIG_INFO_UAC;
	unsigned int CONFIG_INFO_AUDIO_FREQ;
	unsigned int CONFIG_INFO_FRAME_SIZE;
	unsigned int CONFIG_INFO_IA_FACE_WITDH;
	unsigned char double_camera_switch;
	unsigned char usb_up_sign;
}CONFIG_INFO_ST;

//int apply_config_info();

/* get config infomation form memory */
int mem_apply_config_info(unsigned char *p_config_mem_addr);

/* get config infomation struct */
CONFIG_INFO_ST *config_info_get();

int double_camera_config_flash();
int double_camera_config_reset_flash();

#ifdef SOFTWARE_TRIGGER_USB_UP
int usb_up_config_flash();
#endif

#endif
