#ifndef __TOUCH_H__
#define __TOUCH_H__

#include "types.h"

#define TOUCH_CLICK_MAX_NUM             (5)
#define MAX_TOUCH_NUM                   (TOUCH_CLICK_MAX_NUM)

#define KDP_GPIO_INT_PIN_FOR_TOUCH      (TOUCH_INT_PIN)
#if (CFG_TOUCH_TYPE == TOUCH_TYPE_GT911)
#define KDP_GPIO_RST_PIN_FOR_TOUCH      (TOUCH_RST_PIN)
#elif (CFG_TOUCH_TYPE == TOUCH_TYPE_CT328)
#define KDP_GPIO_RST_PIN_FOR_TOUCH      (TOUCH_RST_PIN)
#endif

typedef struct __t_touch_data
{
    u32 input_x[TOUCH_CLICK_MAX_NUM];
    u32 input_y[TOUCH_CLICK_MAX_NUM];
    u8 count;
} t_touch_data;

struct touch_driver_ops {
    void (*tp_gpio_set_int_stop)(void);
    void (*tp_gpio_set_int_start)(void);
};

extern struct touch_driver kdp520_touch_driver;


#endif
