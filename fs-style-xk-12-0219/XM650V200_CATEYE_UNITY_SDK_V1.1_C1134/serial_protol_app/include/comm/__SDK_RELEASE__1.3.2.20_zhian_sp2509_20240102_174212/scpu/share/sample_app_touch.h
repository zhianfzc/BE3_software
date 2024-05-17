#ifndef __SAMPLE_APP_TOUCH_H__
#define __SAMPLE_APP_TOUCH_H__

#include "types.h"
#include "cmsis_os2.h"
#include "board_cfg.h"
#include "kl520_include.h"


BOOL sample_app_touch_is_inited(void);
void sample_app_init_touch_panel(void);
void sample_app_deinit_touch_panel(void);

#endif
