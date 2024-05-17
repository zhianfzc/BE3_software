#pragma once
#include "types.h"
#include "framework/framework_driver.h"


extern void kdp520_lcm_set_cs(u8 lv);
extern void kdp520_lcm_set_rs(u8 lv);
extern void kdp520_lcm_set_rst(u8 lv);
extern int kdp520_lcm_get_te_pin(void);
extern void kdp520_lcm_set_custom_pinmux(lcm_custom_pinmux* p_param);
extern void kdp520_spi_lcd_set_custom_pinmux(lcm_custom_pinmux* p_param);
