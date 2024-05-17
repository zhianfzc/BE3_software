#ifndef __USER_UI_H__
#define __USER_UI_H__

#pragma once
#include "board_kl520.h"

#define GUI_V1                          (0)
#define GUI_V2                          (1)
#define GUI_VERSION_TYPE                (GUI_V2)

#define DISPLAY_RESULT_HOLD_TIME        (1500)  //ms

void kl520_api_ddr_img_init(void);
void kl520_api_ddr_img_user(void);
void user_ui_init(void);

#endif    //__USER_UI_H__
