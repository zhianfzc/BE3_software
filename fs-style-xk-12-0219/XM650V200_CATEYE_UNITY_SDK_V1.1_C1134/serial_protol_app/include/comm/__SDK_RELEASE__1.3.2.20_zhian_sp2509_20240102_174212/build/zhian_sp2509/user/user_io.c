#include "user_io.h"
#include <string.h>
#include "base.h"
#include "pinmux.h"
#include "drivers.h"
//#include "kdp520_pwm_timer.h"
#include "kdp520_i2c.h"
#include "kdp520_gpio.h"
#include "kl520_include.h"
#include "delay.h"
#include "dbg.h"


#define IOEXT_SLAVE_ADDR   (0x5B)
#define GPIO_TE            GPIO_16

#if ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36515 )
#define LED_SLAVE_ID        (0x63)
static io_led_mode g_LedLightMode = LED_STANDBY_MODE;
#endif

static u16 _rgb_led_value = 0xffff;

#if CFG_ONE_SHOT_MODE == YES
BOOL skip_all_led = FALSE;
BOOL production_test_flag = FALSE;
#endif

static void _io_extender_set_io(u16 regAddr, u8 pin, u8 val) 
{
//    u8 data = 0;

//    if (IOEXT_REG_SET_IO_P0 == regAddr) {data = __io_ext_status_p0;}
//    else if (IOEXT_REG_SET_IO_P1 == regAddr) {data = __io_ext_status_p1;}
//    else {kdp_drv_i2c_read(__i2c_port, IOEXT_SLAVE_ADDR, regAddr, 1, &data);}

//    if (0 == val)
//        data &= ~pin;
//    else
//        data |= pin;
//    //dbg_msg_user("_io_extender_set_io, regAddr=%x, data=%x", regAddr, data);
//    kdp_drv_i2c_write(__i2c_port, IOEXT_SLAVE_ADDR, regAddr, 1, data);

//    if (IOEXT_REG_SET_IO_P0 == regAddr) {__io_ext_status_p0 = data;}
//    else if (IOEXT_REG_SET_IO_P1 == regAddr) {__io_ext_status_p1 = data;}
}

void _io_extender_init(void)
{
//    u16 regAddr;

//    _io_extender_set_i2c_port(0); //__i2c_port = 0;

//#ifndef CUSTOMER_SETTING_REMOVE_LOG
//    u8 data = 0;
//    //get device id
//    data = user_io_get_extra_device_id();
//    dbg_msg_user("io extender init, ID = %x", data);
//#endif
//    
//    //set io direction
//    regAddr = IOEXT_REG_DIR_P0;
//    _io_extender_set_io(regAddr, IOEXT_P0|IOEXT_P1|IOEXT_P2|IOEXT_P3|IOEXT_P4|IOEXT_P5|IOEXT_P6|IOEXT_P7, 0);
//    regAddr = IOEXT_REG_DIR_P1;
//    _io_extender_set_io(regAddr, IOEXT_P0|IOEXT_P1|IOEXT_P2|IOEXT_P3|IOEXT_P4|IOEXT_P5|IOEXT_P6|IOEXT_P7, 0);

//    //set P0 value
//    regAddr = IOEXT_REG_SET_IO_P0;
//    _io_extender_set_io(regAddr, IOEXT_P0|IOEXT_P1|IOEXT_P2|IOEXT_P3|IOEXT_P4|IOEXT_P5|IOEXT_P6|IOEXT_P7, 0);
//    delay_us(200);

//#if 1
//#else
//    _io_extender_set_io(regAddr, IOEXT_P5|IOEXT_P3|IOEXT_P1, 1);
//    _io_extender_set_io(regAddr, IOEXT_P2, 1);//0x6E;
//#endif
//    regAddr = IOEXT_REG_SET_SYS_CFG;
//    _io_extender_set_io(regAddr, IOEXT_P0|IOEXT_P1|IOEXT_P2|IOEXT_P3|IOEXT_P4|IOEXT_P5|IOEXT_P6|IOEXT_P7, 0);
//    _io_extender_set_io(regAddr, IOEXT_P4, 1);//0x10

//    regAddr = IOEXT_REG_SET_LED_SW;
//    _io_extender_set_io(regAddr, IOEXT_P0|IOEXT_P1|IOEXT_P2|IOEXT_P3|IOEXT_P4|IOEXT_P5|IOEXT_P6|IOEXT_P7, 1);//0xFF

//    //set P1 value
//    //regAddr = IOEXT_REG_SET_IO_P1;
//    //_io_extender_set_io(regAddr, IOEXT_P0, 0);
//    //dbg_msg_user("io extender init, done");
}

BOOL rgb_camera_is_powered_on = FALSE;
BOOL nir_camera_is_powered_on = FALSE;

void rgb_camera_rst(void)
{
    PINMUX_SD_DATA2_SET(PINMUX_SD_DATA2_GPIO26);//RGB reset
    kdp520_gpio_setdir(  GPIO_26, GPIO_DIR_OUTPUT );
    kdp520_gpio_setdata( 1<<GPIO_26);
    
    delay_us(1000);
    
    PINMUX_SD_DATA2_SET(PINMUX_SD_DATA2_GPIO26);//RGB reset
    kdp520_gpio_setdir(  GPIO_26, GPIO_DIR_OUTPUT );
    kdp520_gpio_cleardata( 1<<GPIO_26);
    
    delay_us(1000);

    PINMUX_SD_DATA2_SET(PINMUX_SD_DATA2_GPIO26);//RGB reset
    kdp520_gpio_setdir(  GPIO_26, GPIO_DIR_OUTPUT );
    kdp520_gpio_setdata( 1<<GPIO_26);
    
}

void rgb_camera_power_on(void)
{
    //dbg_msg_user("[%s]", __func__);
    if (!rgb_camera_is_powered_on) 
    {
        PINMUX_LC_DATA10_SET(PINMUX_LC_DATA10_GPIO21);
        kdp520_gpio_setdir(  GPIO_21, GPIO_DIR_OUTPUT );
        kdp520_gpio_setdata( 1<<GPIO_21);
		
        PINMUX_LC_DATA11_SET(PINMUX_LC_DATA11_GPIO0);//RGB PWDN
        kdp520_gpio_setdir(  GPIO_0, GPIO_DIR_OUTPUT );
        kdp520_gpio_setdata( 1<<GPIO_0);
        
        delay_us(1000);
        /*
        _io_extender_set_io(IOEXT_REG_SET_IO_P0, IOEXT_P0, 0);//PWDN
        _io_extender_set_io(IOEXT_REG_SET_IO_P0, IOEXT_P1, 1);//RST
        _io_extender_set_io(IOEXT_REG_SET_IO_P0, IOEXT_P2, 1);//PWR_EN2
        _io_extender_set_io(IOEXT_REG_SET_IO_P0, IOEXT_P4, 0);//PWR_EN1
        */
        PINMUX_LC_DATA9_SET(PINMUX_LC_DATA9_GPIO20);//RGB 1.8V
        kdp520_gpio_setdir(  GPIO_20, GPIO_DIR_OUTPUT );
        kdp520_gpio_setdata( 1<<GPIO_20);
        
        delay_us(1000);
        
        PINMUX_LC_DATA10_SET(PINMUX_LC_DATA10_GPIO21);//NIR PWDN
        kdp520_gpio_setdir(  GPIO_21, GPIO_DIR_OUTPUT );
        kdp520_gpio_cleardata( 1<<GPIO_21);

        PINMUX_LC_DATA11_SET(PINMUX_LC_DATA11_GPIO0);//RGB PWDN
        kdp520_gpio_setdir(  GPIO_0, GPIO_DIR_OUTPUT );
        kdp520_gpio_cleardata( 1<<GPIO_0);
        
        rgb_camera_rst();
        delay_us(1000);
        //delay_us(30000);
        rgb_camera_is_powered_on = TRUE;
    }
}
void rgb_camera_power_off(void)
{
    //dbg_msg_user("[%s]", __func__);
    if (rgb_camera_is_powered_on) {
//        PINMUX_SD_DATA2_SET(PINMUX_SD_DATA2_GPIO26);//RGB reset
//        kdp520_gpio_setdir(  GPIO_26, GPIO_DIR_OUTPUT );
//        kdp520_gpio_cleardata( 1<<GPIO_26);
        /*
        _io_extender_set_io(IOEXT_REG_SET_IO_P0, IOEXT_P0, 1);//PWDN
        _io_extender_set_io(IOEXT_REG_SET_IO_P0, IOEXT_P2, 0);//PWR_EN2
        //if (!nir_camera_is_powered_on)
        //    _io_extender_set_io(IOEXT_REG_SET_IO_P0, IOEXT_P4, 1);//PWR_EN1
        */
        rgb_camera_is_powered_on = FALSE;
    }
}
void nir_camera_power_on(void)
{
    //dbg_msg_user("[%s]", __func__);
    if (!nir_camera_is_powered_on)
    {
        rgb_camera_power_on();
        /*
        tp_power_on();
        _io_extender_set_io(IOEXT_REG_SET_IO_P0, IOEXT_P3, 1);//PWDN
        _io_extender_set_io(IOEXT_REG_SET_IO_P0, IOEXT_P4, 0);//PWR_EN1
        _io_extender_set_io(IOEXT_REG_SET_IO_P0, IOEXT_P5, 1);//PWR_EN
        */
//        PINMUX_LC_DATA9_SET(PINMUX_LC_DATA9_GPIO20);//RGB reset
//        kdp520_gpio_setdir(  GPIO_20, GPIO_DIR_OUTPUT );
//        kdp520_gpio_setdata( 1<<GPIO_20);
        
//        PINMUX_LC_DATA11_SET(PINMUX_LC_DATA11_GPIO0);//RGB reset
//        kdp520_gpio_setdir(  GPIO_0, GPIO_DIR_OUTPUT );
//        kdp520_gpio_setdata( 1<<GPIO_0);
//        //kdp520_gpio_cleardata( 1<<GPIO_0);
        
//        PINMUX_LC_DATA10_SET(PINMUX_LC_DATA10_GPIO21);//RGB reset
//        kdp520_gpio_setdir(  GPIO_21, GPIO_DIR_OUTPUT );
//        kdp520_gpio_setdata( 1<<GPIO_21);

        //delay_us(30000);
        nir_camera_is_powered_on = TRUE;
    }
}
void nir_camera_power_off(void)
{
    //dbg_msg_user("[%s]", __func__);
    if (nir_camera_is_powered_on)
    {
        /*
        _io_extender_set_io(IOEXT_REG_SET_IO_P0, IOEXT_P3, 0);//PWDN
        //if (!rgb_camera_is_powered_on)
        //    _io_extender_set_io(IOEXT_REG_SET_IO_P0, IOEXT_P4, 1);//PWR_EN1
        _io_extender_set_io(IOEXT_REG_SET_IO_P0, IOEXT_P5, 0);//PWR_EN
        */
//        PINMUX_LC_DATA11_SET(PINMUX_LC_DATA11_GPIO0);//NIR
//        kdp520_gpio_setdir(  GPIO_0, GPIO_DIR_OUTPUT );
//        kdp520_gpio_cleardata( 1<<GPIO_0);
        nir_camera_is_powered_on = FALSE;
    }
}

#if ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36515 )
void led_set_light_mode( io_led_mode nMode )
{
    g_LedLightMode = nMode;
}

u8 aw36515_get_id(void)
{
    u8 nDeviceId;
    kdp_drv_i2c_read( I2C_ADAP_0, LED_SLAVE_ID, 0x00, 1, &nDeviceId );
    
    return nDeviceId;
}

static void aw36515_init( io_led_mode nMode )
{
    static u8 nFirstInit = TRUE;

    if ( nFirstInit == TRUE )
    {
        //check device id
        if ( aw36515_get_id() != 0x30 )
            return;

        //Set LED timing
        kdp_drv_i2c_write( I2C_ADAP_0, LED_SLAVE_ID, 0x08, 1, 0x1F );

        //Set enable register
        led_set_light_mode( nMode );
        kdp_drv_i2c_write( I2C_ADAP_0, LED_SLAVE_ID, 0x01, 1, g_LedLightMode );  //set mode and turn off LED
        nFirstInit = FALSE;
    }
}
#endif

#if 0
static void aw36404_init(u32 pin, u8 *pulse_cnt, u16 duty)
{
    //kdp520_gpio_setmode(pin);
    dbg_msg_console("  [%s] 36404 nDeviceId ", __FUNCTION__);
    duty = (duty > 100)?(100):(duty);
    u8 count = (u8)(( duty * 64 )/100);
    if(count == (*pulse_cnt))
        return;
    
    kdp520_gpio_cleardata(1U << pin);
    kdp520_gpio_pulldisable(pin);
    kdp520_gpio_setdir(pin, GPIO_DIR_OUTPUT);
    
    
    // =============== start of critical section ====================
    // do not interrupt gpio toggling ,LED driver may not be enable    
   if((*pulse_cnt) == 255)
    {
        *pulse_cnt = count;
    }
    else
    {
        if(count==0)
        {
            *pulse_cnt = 0;
        }
        else
        {
            if(count > (*pulse_cnt))
            {
                count = count - (*pulse_cnt); 
            }
            else
            {
                count = 64 -  (*pulse_cnt) + count ;
            }
        }
        *pulse_cnt += count;
        
        if((*pulse_cnt) > 64)
            *pulse_cnt -= 64;
        
    }
    
    
    uint32_t _primask = __get_PRIMASK();
    __disable_irq();
    if((*pulse_cnt) == 0)
    {
        kdp520_gpio_cleardata( 1U<<pin);
    }
    else
    {
        while(--count)
        {
            kdp520_gpio_setdata( 1U << pin);
            delay_us(1);
            kdp520_gpio_cleardata( 1U << pin);
            delay_us(1);
        }
        kdp520_gpio_setdata( 1U << pin );
    }
    
    if (_primask == 0U) 
    {
        __enable_irq();
    }
    
    if((*pulse_cnt) == 0)
        delay_us(2150);
    // =============== end of critical section ====================
    delay_us(350);//keeping EN pin high at least 350us to turn on LED driver    

}
#endif

void nir_led_init(u16 duty)
{
#if ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36515 )
    aw36515_init( LED_STANDBY_MODE );
    if ( g_LedLightMode != LED_STANDBY_MODE )
    {
        nir_led_open( duty );
    }
#else
//    static u8 nir_pulse_cnt=255;
    
    PINMUX_LC_DATA14_SET(PINMUX_LC_DATA14_GPIO3);
    kdp520_gpio_setdir( GPIO_3, GPIO_DIR_OUTPUT);
    kdp520_gpio_setdata( 1<<GPIO_3);
    
    PINMUX_LC_DATA12_SET(PINMUX_LC_DATA12_GPIO1);
    //aw36404_init(GPIO_2, &nir_pulse_cnt, duty);

    //PINMUX_LC_DATA10_SET(PINMUX_LC_DATA10_GPIO21);
    kdp520_gpio_setdir( GPIO_1, GPIO_DIR_OUTPUT);

    if (duty > 0)
    {
        nir_led_open(duty);
    }
    else
    {
        nir_led_close();
    }
#endif
}

void nir_led_open(u16 level)
{
#if CFG_ONE_SHOT_MODE == YES
    if( skip_all_led == TRUE ) return;
#endif

#if ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36515 )
    u8 nLevel, nLedBriReg;

    if ( g_LedLightMode == LED_STANDBY_MODE )
    {
        led_set_light_mode(LED_TORCH_MODE);
    }

    if ( g_LedLightMode == LED_TORCH_MODE )
    {
        nLedBriReg = 0x05;
    }
    else if ( g_LedLightMode == LED_FLASH_MODE )
    {
        nLedBriReg = 0x03;
    }

    level = (level > 100)? (100):(level);
    nLevel = level * 255 / 100;  //mapping to 0~255

    //dbg_msg_console("[nir_led_open]  mode=0x%x, duty = %d ", g_LedLightMode, nLevel);
    kdp_drv_i2c_write( I2C_ADAP_0, LED_SLAVE_ID, 0x01, 1, (g_LedLightMode|IO_LED1) );  //set LED mode and turn on LED1 
    kdp_drv_i2c_write( I2C_ADAP_0, LED_SLAVE_ID, nLedBriReg, 1, nLevel );
#else
//#if CFG_AI_TYPE == AI_TYPE_R1N1   
//       u16 duty = (level > 100) ? (100) : (level);
//       nir_led_init(duty >> 1);
//#elif (CFG_AI_TYPE == AI_TYPE_N1R1 || CFG_AI_TYPE == AI_TYPE_N1)
//       u16 duty = (level > 90) ? (90) : (level);
//       nir_led_init(duty);
//#endif
    
    //kdp520_gpio_setdir( GPIO_2, GPIO_DIR_OUTPUT);
    kdp520_gpio_setdata( 1<<GPIO_1);
#endif
}

void nir_led_close(void) 
{
#if ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36515 )
    u8 nData;

    led_set_light_mode( LED_TORCH_MODE );
    kdp_drv_i2c_write( I2C_ADAP_0, LED_SLAVE_ID, 0x01, 1, g_LedLightMode );  //Set to torch mode and turn off LED
#else
    kdp520_gpio_cleardata( 1<<GPIO_1);
    
//    if (nir_led_is_opened) {
//        nir_led_init(0);
//        nir_led_is_opened = FALSE;
//    }
#endif
}

//volatile u8 rgb_pulse_cnt=255;
void rgb_led_init(u16 duty)
{
#if ( CFG_LED_DRIVER_TYPE != LED_DRIVER_AW36515 )
//    static u8 rgb_pulse_cnt=255;
//    PINMUX_PWM0_SET(PINMUX_PWM0_GPIO31);
//    aw36404_init(GPIO_31, &rgb_pulse_cnt, duty);
//    PINMUX_LC_DATA14_SET(PINMUX_LC_DATA14_GPIO3);
//    aw36404_init(GPIO_3, &rgb_pulse_cnt, duty);
#endif
}

void rgb_led_open(u16 level)
{
#if CFG_ONE_SHOT_MODE == YES
    if( skip_all_led == TRUE ) return;
#endif
#if ( CFG_LED_DRIVER_TYPE != LED_DRIVER_AW36515 )
    u16 duty = (level > 100) ? (100) : (level);
    rgb_led_init(duty);
    _rgb_led_value = duty;
#endif
}

void rgb_led_close(void) 
{
#if ( CFG_LED_DRIVER_TYPE != LED_DRIVER_AW36515 )
    if (0 != _rgb_led_value)
    {
        rgb_led_init(0);
        _rgb_led_value = 0;
    }
#endif
}

BOOL lcd_power_is_opened = FALSE;
void lcd_power_on(void) 
{
    if (!lcd_power_is_opened)
    {
        //dbg_msg_user("[%s]", __func__);
        _io_extender_set_io(IOEXT_REG_SET_IO_P0, IOEXT_P6, 1);
        _io_extender_set_io(IOEXT_REG_SET_IO_P0, IOEXT_P7, 0);
        lcd_power_is_opened = TRUE;
    }
}
void lcd_power_off(void)
{
    //dbg_msg_user("[%s]", __func__);
    _io_extender_set_io(IOEXT_REG_SET_IO_P0, IOEXT_P6, 0);
    _io_extender_set_io(IOEXT_REG_SET_IO_P0, IOEXT_P7, 1);
    lcd_power_is_opened = FALSE;
    //_io_extender_set_io(IOEXT_REG_SET_LED_SW, 0xFF, 1);
}

BOOL tp_is_powered_on = FALSE;
void tp_power_on(void)
{
    if (!tp_is_powered_on)
    {
        lcd_power_on();
        //dbg_msg_user("[%s]", __func__);
        _io_extender_set_io(IOEXT_REG_SET_IO_P1, IOEXT_P0, 1);//RST_N
        _io_extender_set_io(IOEXT_REG_SET_IO_P1, IOEXT_P1, 0);//EN_N
        delay_us(300);
        tp_is_powered_on = TRUE;
    }
}
void tp_power_off(void)
{
    if (tp_is_powered_on)
    {
        //dbg_msg_user("[%s]", __func__);
        _io_extender_set_io(IOEXT_REG_SET_IO_P1, IOEXT_P1, 1);
        tp_is_powered_on = FALSE;
    }
}

BOOL flag_display_backlight = FALSE;
int _glight = 50;
int user_io_get_backlight(void)
{
	return _glight;
}

int user_io_set_backlight(int light)
{   
#if ( CFG_PANEL_TYPE != PANEL_NULL )
    u32 duty;
    if(flag_display_backlight)
    {
        duty = light;
    }
    else
    {
        duty = 0;
    }
    
    static u8 lcd_bl_pulse_cnt=255;
    PINMUX_LC_DATA12_SET(PINMUX_LC_DATA12_GPIO1);
    aw36404_init(GPIO_1, &lcd_bl_pulse_cnt, duty);

    if(light == 0)
    {
        flag_display_backlight = FALSE;
        return 0;
    }
#endif
    _glight = light;
    
	return _glight;
}

void user_io_chk_backlight(void)
{
    if (!flag_display_backlight)
    {
        kdp520_gpio_setedgemode(1 << GPIO_TE, 0);
        kdp520_gpio_enableint( GPIO_TE, GPIO_EDGE, GPIO_Rising);
        kdp520_wait_gpio_int(1<<GPIO_TE);
        osDelay(10);
        kdp520_wait_gpio_int(1<<GPIO_TE);
        kdp520_gpio_disableint(GPIO_TE);

        flag_display_backlight = TRUE;

        if(user_io_get_backlight())
        {
            user_io_set_backlight(user_io_get_backlight());
        }
    }
}

void backlight_init(void)
{

#if (DISPLAY_DEVICE == DISPLAY_DEVICE_LCM)
    user_io_set_backlight(user_io_get_backlight());
#endif
}

/* allowed quantized value : 0 ~ 1023 */
int user_io_light_sensor_get(void)
{
    kdp520_adc_init();
    return kdp520_adc_read(0);
}

void user_io_init(void)
{
    /*
    _io_extender_init();
    */
    rgb_led_close();
    nir_led_init(0);

    backlight_init();

    //rgb_camera_power_off();
    //nir_camera_power_off();
    /*
    lcd_power_off();
    tp_power_off();
    */
#if ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36515 )
    kl520_api_led_register_hook( CAMERA_DEVICE_NIR_IDX, nir_led_open, nir_led_close, led_set_light_mode );
    kl520_api_led_register_hook( CAMERA_DEVICE_RGB_IDX, rgb_led_open, rgb_led_close, led_set_light_mode );
#else
    kl520_api_led_register_hook(CAMERA_DEVICE_NIR_IDX, nir_led_open, nir_led_close);
    kl520_api_led_register_hook(CAMERA_DEVICE_RGB_IDX, rgb_led_open, rgb_led_close);
#endif
    kl520_api_light_sensor_register_hook(user_io_light_sensor_get);

    kl520_api_camera_register_hook(CAMERA_DEVICE_NIR_IDX, nir_camera_power_on, nir_camera_power_off);
    kl520_api_camera_register_hook(CAMERA_DEVICE_RGB_IDX, rgb_camera_power_on, rgb_camera_power_off);
    /*
    kl520_api_touch_register_hook(tp_power_on, tp_power_off);
    */
}

u32 user_io_get_extra_device_cnt(void)
{
    return 0;
}

u32 user_io_get_extra_device_id(u32 device_idx)
{
    u8 data = 0;
//    if (device_idx < user_io_get_extra_device_cnt()) {
//        switch (device_idx) {
//        case 0: kdp_drv_i2c_read(__i2c_port, IOEXT_SLAVE_ADDR, IOEXT_REG_ID, 1, &data); if (IO_EXTENDER_ID != data) data = 0; break;
//        default:;
//        }
//    }
    
    return (u32)data;
}

void user_io_get_extra_device_name(u32 device_idx, char *out_name, u32 max_lenth)
{
//    if (device_idx < user_io_get_extra_device_cnt()) {
//        switch (device_idx) {
//        case 0: strncpy(out_name, "aw9523b", max_lenth); break;
//        default:;
//        }
//    }
}

void user_io_poweroff(void)
{
}

void user_console_cmd01(void)
{
}

void user_console_cmd02(void)
{
}

void user_console_cmd03(void)
{
}

void user_console_cmd04(void)
{
}

void user_console_cmd05(void)
{
}
