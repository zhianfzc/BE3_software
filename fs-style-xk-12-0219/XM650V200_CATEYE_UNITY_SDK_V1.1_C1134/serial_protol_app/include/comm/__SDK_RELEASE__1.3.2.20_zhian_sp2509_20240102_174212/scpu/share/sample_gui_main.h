#pragma once
#include "board_kl520.h"
#include "user_ui.h"

#include "kl520_api_camera.h"

typedef enum _gui_page_idx
{
    PAGE_IDX_0 = 0,
    PAGE_IDX_1,
    PAGE_IDX_2,
    PAGE_IDX_3,
    PAGE_IDX_4,
    PAGE_IDX_5,
    PAGE_IDX_6,
} gui_page_idx;

extern osThreadId_t tid_gui;

extern void sample_gui_init(void);
extern void gui_app_stop(void);
extern void gui_app_proceed(void);
extern u8 gui_app_get_status(void);
