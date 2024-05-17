#ifndef __APP_MAIN_H__
#define __APP_MAIN_H__

#include "config.h"

/* judge usb upgrade and usb upgrade*/
extern int usb_up_judge();

/* detect face and usb power */
extern void intf_adc_detect();

/* get face power sign */
extern unsigned char *face_power_sign();

/* get usb power sign */
extern unsigned char *usb_power_sign();

/*  app system init */
extern int app_system_init(CONFIG_INFO_ST *pst_config_info);

/* work timer start */
extern void work_timer_start();

/* recognize start */
extern int recognize_start(void *pstCfgParam);

/* usb start */
extern int usb_start(void *pstCfgParam);

int app_main(void);

#endif

