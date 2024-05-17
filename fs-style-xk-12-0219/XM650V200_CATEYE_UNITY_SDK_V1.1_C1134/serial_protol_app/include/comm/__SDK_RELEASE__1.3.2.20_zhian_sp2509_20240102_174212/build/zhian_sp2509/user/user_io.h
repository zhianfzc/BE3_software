#ifndef __USER_IO_H__
#define __USER_IO_H__

#include "types.h"

#define IOEXT_P0     (1<<0)
#define IOEXT_P1     (1<<1)
#define IOEXT_P2     (1<<2)
#define IOEXT_P3     (1<<3)
#define IOEXT_P4     (1<<4)
#define IOEXT_P5     (1<<5)
#define IOEXT_P6     (1<<6)
#define IOEXT_P7     (1<<7)

#if ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36515 )
#define IO_LED1      ( 1 << 0 )
#define IO_LED2      ( 1 << 1 )
#endif

typedef enum __io_ext_reg
{
    IOEXT_REG_ID = 0x10,
    IOEXT_REG_DIR_P0 = 0x04,
    IOEXT_REG_DIR_P1 = 0x05,
    IOEXT_REG_SET_IO_P0 = 0x02,
    IOEXT_REG_SET_IO_P1 = 0x03,
    IOEXT_REG_SET_SYS_CFG = 0x11,
    IOEXT_REG_SET_LED_SW = 0x12,
} io_ext_reg;

#if ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36515 )
typedef enum __io_led_mode
{
    LED_STANDBY_MODE = 0x80,
    LED_TORCH_MODE = 0x88,
    LED_FLASH_MODE = 0x8C,
} io_led_mode;

extern void led_set_light_mode( io_led_mode nMode );
#endif

//extern void rgb_led_init(u16 duty);
//extern void nir_led_init(u16 duty);
extern void backlight_init(void);
extern void rgb_led_open(u16 level);
extern void rgb_led_close(void);
extern void nir_led_open(u16 level);
extern void nir_led_close(void);
extern void rgb_camera_power_on(void);
extern void rgb_camera_power_off(void);
extern void nir_camera_power_on(void);
extern void nir_camera_power_off(void);
extern void lcd_power_on(void);
extern void lcd_power_off(void);
extern void tp_power_on(void);
extern void tp_power_off(void);

void user_io_init(void);
extern int user_io_get_backlight(void);
extern int user_io_set_backlight(int light);
extern void user_io_chk_backlight(void);
extern int user_io_light_sensor_get(void);

u32 user_io_get_extra_device_cnt(void);
u32 user_io_get_extra_device_id(u32 device_idx);
void user_io_get_extra_device_name(u32 device_idx, char *out_name, u32 max_lenth);

void user_io_poweroff(void);
void user_console_cmd01(void);
void user_console_cmd02(void);
void user_console_cmd03(void);
void user_console_cmd04(void);
void user_console_cmd05(void);

#endif    //__USER_IO_H__
