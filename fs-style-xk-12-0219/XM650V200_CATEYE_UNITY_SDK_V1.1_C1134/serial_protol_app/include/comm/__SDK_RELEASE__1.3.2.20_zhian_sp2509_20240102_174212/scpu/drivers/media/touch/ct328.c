#include "board_kl520.h"
#if (CFG_TOUCH_TYPE == TOUCH_TYPE_CT328)
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


#define MOUSE_DOWN_MAX_CNT      (1)//(1)
#define MOUSE_UP_MAX_CNT        (1)//(1)
#define TOUCH_ID                0x148//(0x82)

t_touch_data __touch_data;
#define GTP_INIT_T2_MS                  (11)                //T2: > 10ms
#define GTP_INIT_T3_MS                  (55)                //T3: > 50ms
#define GTP_INIT_T7_US                  (150)               //T7: > 100us
#define GTP_INIT_T8_MS                  (6)                 //T8: > 5ms
#define GTP_INIT_T9_MS                  (200)               //T9: < 100ms, Touch panel calibration.

#define GTP_RST_T1_MS                   (1)                 //T1: > 100us
#define GTP_RST_T2_US                   (GTP_INIT_T7_US)    //T2: > 100us
#define GTP_RST_T3_MS                   (GTP_INIT_T8_MS)    //T3: > 5ms
#define GTP_RST_T4_MS                   (GTP_INIT_T3_MS)    //T4: > 50ms
#define GTP_RST_T5_MS                   (GTP_INIT_T9_MS)    //T5: < 100ms, Touch panel calibration.

int _ct328_init(struct core_device *core_d)
{
    int ret;

    struct touch_driver *touch_drv = &kdp520_touch_driver;
    struct touch_driver_ops *ops = (struct touch_driver_ops*)touch_drv->ctrller_ops;
    kdp520_gpio_setmode(KDP_GPIO_INT_PIN_FOR_TOUCH);
    kdp520_gpio_setmode(KDP_GPIO_RST_PIN_FOR_TOUCH);

    kdp520_gpio_setdir(KDP_GPIO_INT_PIN_FOR_TOUCH, GPIO_DIR_OUTPUT);
    kdp520_gpio_setdir(KDP_GPIO_RST_PIN_FOR_TOUCH, GPIO_DIR_OUTPUT);

    kdp520_gpio_cleardata(1<<KDP_GPIO_INT_PIN_FOR_TOUCH);
    kdp520_gpio_cleardata(1<<KDP_GPIO_RST_PIN_FOR_TOUCH);

    osDelay(GTP_INIT_T2_MS);
    kdp520_gpio_setdata(1<<KDP_GPIO_INT_PIN_FOR_TOUCH);

    delay_us(GTP_INIT_T7_US);
    kdp520_gpio_setdata(1<<KDP_GPIO_RST_PIN_FOR_TOUCH);

    osDelay(GTP_INIT_T8_MS);
    kdp520_gpio_cleardata(1<<KDP_GPIO_INT_PIN_FOR_TOUCH);

    osDelay(GTP_INIT_T3_MS);
    kdp520_gpio_setdir(KDP_GPIO_INT_PIN_FOR_TOUCH, GPIO_DIR_INPUT);

    osDelay(GTP_INIT_T9_MS);
    if (!ops)
        return -1;

    ret = 0;

    return ret;
}

u16 _ct328_read_touch_id(struct core_device *core_d)
{
    u16 id = 0;
    //TODO : read ID
    return id;
}

void _ct328_start(struct core_device *core_d)
{
    struct touch_driver *touch_drv = &kdp520_touch_driver;
    struct touch_driver_ops *ops = (struct touch_driver_ops*)touch_drv->ctrller_ops;
    ops->tp_gpio_set_int_start();
}
void _ct328_stop(struct core_device *core_d)
{
    struct touch_driver *touch_drv = &kdp520_touch_driver;
    struct touch_driver_ops *ops = (struct touch_driver_ops*)touch_drv->ctrller_ops;
    ops->tp_gpio_set_int_stop();
}

void _ct328_get_raw_data(struct core_device *core_d, u8 init_i2c)
{
    u8 buf[30] = {0};
    u8 i2c_buf[8] = {0};
    u8 cnt, i2c_len, idx, len_1=0, len_2=0;
    u16 i;

    u8 finger_id;
    u32 input_x=0, input_y=0;
    u16 regAddr;
    int dev_addr = 0x1A;

    struct touch_driver *touch_drv = &kdp520_touch_driver;
    struct touch_driver_ops *ops = (struct touch_driver_ops*)touch_drv->ctrller_ops;

    //NVIC_DisableIRQ(GPIO_FTGPIO010_IRQ);
    ops->tp_gpio_set_int_stop();

    regAddr = 0xD000;

    kdp_drv_i2c_read_bytes(I2C_ADAP_0, dev_addr, regAddr, 2, buf, 7); //api style

    if (0xAB!=buf[6] || 0x80==buf[5])
    {
        goto OUT_PROCESS;
    }

    cnt = buf[5] & 0x7F;
    if (MAX_TOUCH_NUM<cnt || 0==cnt)
        goto OUT_PROCESS;

    if (1 == cnt)
    {
        goto FINGER_PROCESS;
    }
    else
    {
        i2c_len = (cnt-1)*5 + 1;
        len_1   = i2c_len;

        for(idx=0; idx<i2c_len; idx+=6)
        {
            regAddr = 0xD007+idx;
            if(len_1>=6) {
                len_2  = 6;
                len_1 -= 6;
            }
            else {
                len_2 = len_1;
                len_1 = 0;
            }

            kdp_drv_i2c_read_bytes(I2C_ADAP_0, dev_addr, regAddr, 2, i2c_buf, len_2); //api style

            for(i=0; i<len_2; i++)
                buf[5+idx+i] = i2c_buf[i];
        }
        i2c_len += 5;

        if (buf[i2c_len - 1] != 0xAB)
            goto OUT_PROCESS;
    }

FINGER_PROCESS:
    regAddr = 0xD000;
    i2c_buf[2] = 0xAB;


    kdp_drv_i2c_write(I2C_ADAP_0, dev_addr, regAddr, 2, i2c_buf[2]); //api style


    idx = 0;
    __touch_data.count = cnt;
    //dbg_msg_touch("touch cnt : %d.\n", cnt);
    for (i = 0; i < cnt; i++) {
        input_x = (unsigned int)((buf[idx + 1] << 4) | ((buf[idx + 3] >> 4) & 0x0F));
        input_y = (unsigned int)((buf[idx + 2] << 4) | (buf[idx + 3] & 0x0F));
        finger_id = (buf[idx] >> 4) & 0x0F;
        dbg_msg_touch("\nx : %d, y : %d, id : %d.", input_x, input_y, finger_id);
        idx += 5;

#if (CFG_CAMERA_ROTATE == YES)
//        if ((PANEL_WIDTH > touch_drv->x_range_max) && (PANEL_HEIGHT < touch_drv->y_range_max))
        {
            if (touch_drv->inverse_x_axis)
                __touch_data.input_x[i] = touch_drv->y_range_max - input_y;
            else
                __touch_data.input_x[i] = input_y;

            if (touch_drv->inverse_y_axis)
                __touch_data.input_y[i] = touch_drv->x_range_max - input_x;
            else
                __touch_data.input_y[i] = input_x;
        }
#else
        {
            if (touch_drv->inverse_x_axis)
                __touch_data.input_x[i] = touch_drv->x_range_max - input_x;
            else
                __touch_data.input_x[i] = input_x;

            if (touch_drv->inverse_y_axis)
                __touch_data.input_y[i] = touch_drv->y_range_max - input_y;
            else
                __touch_data.input_y[i] = input_y;
        }
#endif
    }
    goto END;

OUT_PROCESS:
    regAddr = 0xD000;
    buf[2] = 0xAB;
    

    kdp_drv_i2c_write(I2C_ADAP_0, dev_addr, regAddr, 2, buf[2]); //api style

END:
    //NVIC_EnableIRQ(GPIO_FTGPIO010_IRQ);
    ops->tp_gpio_set_int_start();
}

static u16 _ct328_get_info(struct core_device * core)
{
    u8 dev_addr = 0x1A;
    u16 regAddr = 0;
    u8 buf1[20] = {0};
    unsigned int chip_type;
    //unsigned int firmware_version, project_version, chip_type, checksum;

    //firmware_version=0;
    //project_version=0;
    chip_type=0;
    //checksum=0;

    regAddr = 0xD1;
		buf1[2]=0x01;
    kdp_drv_i2c_write(I2C_ADAP_0, dev_addr, regAddr, 1, buf1[2]);

    osDelay(10);

    regAddr = 0xD204;
    kdp_drv_i2c_read_bytes(I2C_ADAP_0, dev_addr, regAddr, 2, buf1, 4);

    chip_type = buf1[3];
    chip_type <<= 8;
    chip_type |= buf1[2];
    
    osDelay(1);
#if 0
    project_version |= buf1[1];
    project_version <<= 8;
    project_version |= buf1[0];

    regAddr = 0xD208;
    kdp_drv_i2c_read_bytes(I2C_ADAP_0, dev_addr, regAddr, 2, buf1, 4);

    firmware_version = buf1[3];
    firmware_version <<= 8;
    firmware_version |= buf1[2];
    firmware_version <<= 8;
    firmware_version |= buf1[1];
    firmware_version <<= 8;
    firmware_version |= buf1[0];

    regAddr = 0xD20C;
    kdp_drv_i2c_read_bytes(I2C_ADAP_0, dev_addr, regAddr, 2, buf1, 4);

    checksum = buf1[3];
    checksum <<= 8;
    checksum |= buf1[2];
    checksum <<= 8;
    checksum |= buf1[1];
    checksum <<= 8;
    checksum |= buf1[0];
#endif
    //need to write reg 0xD109 back to touch_ic for making touch-ic be in normal status.
    regAddr = 0xD1;
		buf1[2]=0x09;
    kdp_drv_i2c_write(I2C_ADAP_0, dev_addr, regAddr, 1, buf1[2]);

		//raw data mode
//    regAddr = 0xD1;
//		buf1[2]=0x0A;
//    kdp_drv_i2c_write(I2C_ADAP_0, dev_addr, regAddr, 1, buf1[2]);
		
    if(chip_type == TOUCH_ID)
        return chip_type;
    else 
        return 0xFFFF;
    
}

static u8 __cnt_down = 0;
static u8 __cnt_up = 0;
static kl520_mouse_state __pre_tp_state = MOUSE_NONE;
void _ct328_state_handler(struct core_device *core_d, void* pData)
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

void _ct328_set_inactive(void) {
    __touch_data.count = 0;
}
int _ct328_get_active(void) {
    if (MOUSE_NONE == __pre_tp_state)
        return 0;
    else
        return 1;
}

struct touch_panel_driver ct328_driver = {
    .init             = _ct328_init,
    .start            = _ct328_start,
    .stop             = _ct328_stop,
    .get_raw_data     = _ct328_get_raw_data,
    .state_handler    = _ct328_state_handler,
    .set_inactive     = _ct328_set_inactive,
    .get_active       = _ct328_get_active,
    .read_did         = _ct328_get_info,
};
#endif
