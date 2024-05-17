#include "board_kl520.h"
#if (CFG_TOUCH_TYPE == TOUCH_TYPE_FT5X06)
#include <stdio.h>
#include <string.h>
#include "base.h"
#include "io.h"
#include "types.h"
#include "board_ddr_table.h"
#include "framework/framework_driver.h"
#include "framework/v2k_image.h"
#include "media/display/display.h"
#include "kdp520_pwm_timer.h"
#include "kdp520_dma.h"
#include "kdp520_gpio.h"
#include "dbg.h"

#include "touch.h"
#include "kdp520_i2c.h"
#include "kl520_include.h"


#define CFG_MAX_TOUCH_POINTS	        (5)
#define FT_TOUCH_STEP	                (6)
#define POINT_READ_BUF			        (3 + FT_TOUCH_STEP * CFG_MAX_TOUCH_POINTS)
#define FT5X06_I2C_ADDR 				(0x38)

#define TOUCH_EVENT_DOWN				(0x00)
#define TOUCH_EVENT_UP					(0x01)
#define TOUCH_EVENT_ON					(0x02)
#define TOUCH_EVENT_RESERVED            (0x03)

#define MOUSE_DOWN_MAX_CNT              (1)//(1)
#define MOUSE_UP_MAX_CNT                (1)//(1)
#define TOUCH_ID                        (0x82)

t_touch_data __touch_data;

int _ft5x06_init(struct core_device *core_d)
{
    int ret;

    struct touch_driver *touch_drv = &kdp520_touch_driver;
    struct touch_driver_ops *ops = (struct touch_driver_ops*)touch_drv->ctrller_ops;
 
    if (!ops)
        return -1;

    ret = 0;

    return ret;
}

u16 _ft5x06_read_touch_id(struct core_device *core_d)
{
    u16 id = 0;
    //TODO : read ID
    return id;
}

void _ft5x06_start(struct core_device *core_d)
{
    struct touch_driver *touch_drv = &kdp520_touch_driver;
    struct touch_driver_ops *ops = (struct touch_driver_ops*)touch_drv->ctrller_ops;
    ops->tp_gpio_set_int_start();
}
void _ft5x06_stop(struct core_device *core_d)
{
    struct touch_driver *touch_drv = &kdp520_touch_driver;
    struct touch_driver_ops *ops = (struct touch_driver_ops*)touch_drv->ctrller_ops;
    ops->tp_gpio_set_int_stop();
}

void _ft5x06_get_raw_data(struct core_device *core_d, u8 init_i2c)
{
    u16 devAddr = FT5X06_I2C_ADDR;
    u16 regAddr = 0x02;
    u8 buf[POINT_READ_BUF] = {0};
    //u8 cnt;
		u8 idx = 0;
    u16 i;
		//u16 id;
    u32 input_x=0, input_y=0;


    struct touch_driver *touch_drv = &kdp520_touch_driver;
    struct touch_driver_ops *ops = (struct touch_driver_ops*)touch_drv->ctrller_ops;

    ops->tp_gpio_set_int_stop();

    kdp_drv_i2c_read_bytes(I2C_ADAP_0, devAddr, regAddr, 2, buf, POINT_READ_BUF); //api style


//{
//        input_x = (unsigned int)((buf[idx + 3] & 0x0F) << 8) | (buf[idx + 4]);
//        input_y = (unsigned int)((buf[idx + 5] & 0x0F) << 8) | (buf[idx + 6]);
//        cnt = (buf[idx + 2] ) & 0x0F;
//        dbg_msg_touch("x:%03d, y:%03d, fcnt : %d state: %d", input_x, input_y, fcnt, state);
//}


    //idx = 0;
    //cnt = (buf[idx + 2] ) & 0x0F;
    //__touch_data.count = cnt;
    //dbg_msg_touch("touch cnt : %d.\n", cnt);
    for (i = 0; i < CFG_MAX_TOUCH_POINTS; i++) {
        
        u16 fshift = i*6;
        int type = buf[idx + 3 + fshift] >> 6;

		/* ignore Reserved events */
		if (type == TOUCH_EVENT_RESERVED)
			continue;

		/* M06 sometimes sends bogus coordinates in TOUCH_DOWN */
//		if (type == TOUCH_EVENT_DOWN)
//			continue;
                
//        id = (buf[idx + 5 + fshift] >> 4) & 0x0f; 
        input_y = (unsigned int)((buf[idx + 3 + fshift] & 0x0F) << 8) | (buf[idx + 4 + fshift]);
        input_x = (unsigned int)((buf[idx + 5 + fshift] & 0x0F) << 8) | (buf[idx + 6 + fshift]);  
        

        if ( ( input_x < 120 ) || ( input_x > 360 ) )
        {
            continue;
        }
        else
        {
            input_x -= 120;
        }

        input_y = ((input_y*QVGA_PORTRAIT_HEIGHT)/TFT43_HEIGHT);
		
//        dbg_msg_touch("i = %d, (%03d,%03d), fcnt : %d state: %d",i, input_x, input_y, cnt, id); 
        
        __touch_data.count++;
        if (touch_drv->inverse_x_axis)
            __touch_data.input_x[i] = touch_drv->x_range_max - input_x;
        else
            __touch_data.input_x[i] = input_x;

        if (touch_drv->inverse_y_axis)
            __touch_data.input_y[i] = touch_drv->y_range_max - input_y;
        else
            __touch_data.input_y[i] = input_y;
    }

    ops->tp_gpio_set_int_start();
}

static u16 _ft5x06_get_info(struct core_device * core)
{
    return TOUCH_ID;    
}

static u8 __cnt_down = 0;
static u8 __cnt_up = 0;
static kl520_mouse_state __pre_tp_state = MOUSE_NONE;
void _ft5x06_state_handler(struct core_device *core_d, void* pData)
{
    kl520_mouse_info* data = (kl520_mouse_info*)pData;

    if (0 == __touch_data.count) {
        __cnt_down = 0;
        if (MOUSE_DOWN==__pre_tp_state || MOUSE_MOVE==__pre_tp_state) {
            data->state = MOUSE_UP;
            __cnt_up++;
        }
        else if (MOUSE_UP==__pre_tp_state) {
            if (MOUSE_UP_MAX_CNT <= __cnt_up)
                data->state = MOUSE_NONE;
            else
                __cnt_up++;
        }
        else
            data->state = MOUSE_NONE;
    }
    else {
        __cnt_up = 0;
        if (MOUSE_DOWN_MAX_CNT > __cnt_down) {
            data->state = MOUSE_DOWN;
            __cnt_down++;
        }
        else
            data->state = MOUSE_MOVE;
    }

    __pre_tp_state = data->state;
    //single touch
    data->x = __touch_data.input_x[0];
    data->y = __touch_data.input_y[0];
}

void _ft5x06_set_inactive(void) {
    __touch_data.count = 0;
}
int _ft5x06_get_active(void) {
    if (MOUSE_NONE == __pre_tp_state)
        return 0;
    else
        return 1;
}

struct touch_panel_driver ft5x06_driver = {
    .init             = _ft5x06_init,
    .start            = _ft5x06_start,
    .stop             = _ft5x06_stop,
    .get_raw_data     = _ft5x06_get_raw_data,
    .state_handler    = _ft5x06_state_handler,
    .set_inactive     = _ft5x06_set_inactive,
    .get_active       = _ft5x06_get_active,
    .read_did         = _ft5x06_get_info,
};

#endif
