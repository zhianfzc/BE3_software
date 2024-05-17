#include "board_kl520.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmsis_os2.h>
#include <math.h>
#include "board_ddr_table.h"
#include "kneron_mozart.h"
#include "base.h"
#include "io.h"
#include "clock.h"
#include "pinmux.h"
#include "scu_extreg.h"
#include "framework/event.h"
#include "framework/init.h"
#include "framework/ioport.h"
#include "framework/framework_errno.h"
#include "framework/framework_driver.h"
#include "framework/v2k_color.h"
#include "framework/v2k_image.h"
#include "media/display/lcm.h"
#include "media/display/display.h"
#include "kdp520_pwm_timer.h"
#include "kdp520_gpio.h"
#include "kdp520_i2c.h"
#include "touch.h"
#include "kl520_include.h"
#include "kl520_api.h"
#include "dbg.h"
#include "RTX_Config.h"


//#define DEBUG_TOUCH_DRIVER_STABILITY
//thread event
#define FLAGS_TOUCH_START_EVT     0x0020
//event
#define EVT_TOUCH_ACK_FLAG (0x20)
#ifdef CFG_TOUCH_THREAD_TIMEOUT_CNT
    #define TP_THREAD_TIMEOUT_CNT           (CFG_TOUCH_THREAD_TIMEOUT_CNT)
#else
    #define TP_THREAD_TIMEOUT_CNT           (5)
#endif

struct kdp520_touch_context {
    u32 base;
    struct core_device *touch_ctrller;
};

struct kdp520_touch_context touch_ctx_s;
struct touch_driver kdp520_touch_driver;

typedef enum _touch_state {
    TOUCH_STATE_IDLE = 0,
    TOUCH_STATE_INITED,
    TOUCH_STATE_STOPPED,
    TOUCH_STATE_PROBED,
    TOUCH_STATE_STARTED,
} touch_state;

touch_state m_touch_state;
kl520_mouse_info  kdp520_mouse_info;

osEventFlagsId_t evt_touch_ack_id = 0;
static int stop_isr = FALSE;
static osThreadId_t m_tid_touch;


static void _tp_gpio_set_int_stop(void);
static void _tp_gpio_set_int_start(void);
extern void delay_ms(unsigned int msec);

static void touch_panel_thread(void *arg)
{
    int timeout_cnt = 0;
    struct core_device *core_d = (struct core_device*)arg;
    struct touch_driver *touch_drv = &kdp520_touch_driver;
    struct touch_panel_driver *panel = touch_drv->panel;
    
    while (1) {
        osThreadFlagsWait(FLAGS_TOUCH_START_EVT, osFlagsWaitAny, osWaitForever);
        osThreadFlagsClear(FLAGS_TOUCH_START_EVT);
        set_event(evt_touch_ack_id, EVT_TOUCH_ACK_FLAG);

        while(1)
        {
            if (TOUCH_STATE_STARTED == m_touch_state)
            {
                if ((kdp520_get_int_occurred(KDP_GPIO_INT_PIN_FOR_TOUCH)) && (FALSE == stop_isr))
                {
                    kdp520_clear_int_occurred(KDP_GPIO_INT_PIN_FOR_TOUCH);
                    panel->get_raw_data(core_d, 1);
                    panel->state_handler(core_d, &kdp520_mouse_info);
                    timeout_cnt = 0;
                    if (panel->get_active())
                        set_event(kl520_api_get_event(), KL520_APP_FLAG_TOUCH);
                    osDelay(1);
                }
                else
                {
                    ++timeout_cnt;
                    if (timeout_cnt > TP_THREAD_TIMEOUT_CNT) {
                        if (panel->get_active()) {
                            panel->set_inactive();//reset touch event
                            panel->state_handler(core_d, &kdp520_mouse_info);
                            set_event(kl520_api_get_event(), KL520_APP_FLAG_TOUCH);
                        }
                        timeout_cnt = 0;
                        if (kdp520_mouse_info.state == 3) {
                            stop_isr = TRUE;
                            osDelay((OS_ROBIN_TIMEOUT * osThreadGetCount()) >> 1);
                            kdp520_clear_int_occurred(KDP_GPIO_INT_PIN_FOR_TOUCH);
                            stop_isr = FALSE;
                        } else
                            osDelay(1);
                        continue;
                    }
                    else
                    {
                        osDelay(20);
                        continue;
                    }
                }
            }
            else {
                set_event(evt_touch_ack_id, EVT_TOUCH_ACK_FLAG);
                break;
            }
            osDelay(1);
        }

        if (TOUCH_STATE_IDLE == m_touch_state ||TOUCH_STATE_STOPPED == m_touch_state)
            break;
    
        osDelay(1);
    }

}

static int kdp520_touch_probe(struct core_device *core_d)
{
    dbg_msg_touch("[%s]", __func__);

    m_touch_state = TOUCH_STATE_PROBED;

    return 0;
}

static int kdp520_touch_remove(struct core_device *core_d)
{
    m_touch_state = TOUCH_STATE_IDLE;
    return 0;
}

static int kdp520_touch_attach_panel(struct core_device *core_d, struct touch_panel_driver *panel)
{
    struct touch_driver *touch_drv = &kdp520_touch_driver;

    touch_drv->panel = panel;
    
    return 0;
}

static void _tp_gpio_set_int_stop(void) {
    u8 i = KDP_GPIO_INT_PIN_FOR_TOUCH;
    kdp520_gpio_setintmask(i);
    kdp520_gpio_setintdisable(i);
}

static void _tp_gpio_set_int_start(void) {
    u8 i = KDP_GPIO_INT_PIN_FOR_TOUCH;
    kdp520_gpio_clearint(1<<i);
    kdp520_gpio_setintenable(i);
    kdp520_gpio_setintunmask(i);
}

static int kdp520_touch_init(struct core_device *core_d)
{
#if BOARD_VERSION==3
    return 0;
#else
    struct touch_driver *touch_drv = &kdp520_touch_driver;
    u8 i = KDP_GPIO_INT_PIN_FOR_TOUCH;

    //GPIO interrupt setting
    //sd_data3
	kdp520_gpio_setmode(i);	//set gpio
	
    kdp520_gpio_setdir(i, GPIO_DIR_INPUT);
    kdp520_gpio_setedgemode(i, SINGLE);
    kdp520_gpio_settrigger(i, GPIO_EDGE);
    kdp520_gpio_setactivemode(i, GPIO_Rising);
    //clear CPIO interrupt
    kdp520_gpio_clearint(1<<i);
    kdp520_gpio_enablebounce(i, APB_CLOCK /12000); //
    //Enable GPIO interrupt
    kdp520_gpio_setintenable(i);
    kdp520_gpio_setintunmask(i);

    // read device ID
    if ((touch_drv) && (touch_drv->panel))
    {
        if (touch_drv->cb_power_on)
            touch_drv->cb_power_on();

        //dbg_msg_touch("touch driver probing...");
        //delay_ms(100);//need to delay a little for i2c pending issue.
        touch_drv->panel->init(core_d);
        touch_drv->device_id = touch_drv->panel->read_did(core_d);
        //dbg_msg_touch("touch_drv->display_id=%x", touch_drv->device_id);
        if(touch_drv->device_id == 0xFFFF)
            return -1;
    }

    m_touch_state = TOUCH_STATE_INITED;

    return 0;
#endif
}

static int kdp520_touch_power(struct core_device *core_d, BOOL on){
    return 0;
}

static int kdp520_touch_start(struct core_device *core_d)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;

    struct touch_driver *touch_drv = &kdp520_touch_driver;

    //dbg_msg_touch("[%s]", __func__);
    //dbg_msg_touch("touch_drv=%x", touch_drv);
    //dbg_msg_touch("touch_drv->panel=%x", touch_drv->panel);
    
    if ((touch_drv) && (touch_drv->panel) && (TOUCH_STATE_STARTED != m_touch_state)){
        touch_drv->panel->start(core_d);
        
        if (0 == evt_touch_ack_id)
            evt_touch_ack_id = create_event();

        //dbg_msg_touch("touch panel : driver thread start");
        if (0 == m_tid_touch) {
            osThreadAttr_t attr = {
                .stack_size = 768
            };
            
            m_tid_touch = osThreadNew(touch_panel_thread, (void*)core_d, &attr);

            //osThreadSetPriority(m_tid_touch, osPriorityNormal1);
            
            set_thread_event(m_tid_touch, FLAGS_TOUCH_START_EVT);
        }

        m_touch_state = TOUCH_STATE_STARTED;
        
        ret = 0;
    }

    return ret;
}

static int kdp520_touch_stop(struct core_device *core_d)
{
    int ret = -KDP_FRAMEWORK_ERRNO_INVALA;

    struct touch_driver *touch_drv = &kdp520_touch_driver;

    //dbg_msg_touch("[%s]", __func__);
    
    if ((touch_drv) && (touch_drv->panel) && (TOUCH_STATE_STOPPED != m_touch_state)) {
        touch_drv->panel->stop(core_d);

        m_touch_state = TOUCH_STATE_STOPPED;
        osDelay(1);
        wait_event(evt_touch_ack_id, EVT_TOUCH_ACK_FLAG);
        //dbg_msg_touch("touch panel : driver thread end");
        
        if (touch_drv->cb_power_off)
            touch_drv->cb_power_off();
        
        m_tid_touch = 0;
        ret = 0;
    }

    return ret;
}

BOOL kdp520_touch_is_started(struct core_device *core_d)
{
    return (TOUCH_STATE_STARTED == m_touch_state);
}

void kdp520_touch_set_hook(struct core_device *core_d, fn_power_hook fn_power_on, fn_power_hook fn_power_off)
{
    struct touch_driver *touch_drv = &kdp520_touch_driver;

    touch_drv->cb_power_on = fn_power_on;
    touch_drv->cb_power_off = fn_power_off;
}

static struct touch_driver_ops kdp520_touch_ops = {
    .tp_gpio_set_int_stop = _tp_gpio_set_int_stop,
    .tp_gpio_set_int_start = _tp_gpio_set_int_start,
};

extern struct core_device kdp520_touch;
struct touch_driver kdp520_touch_driver = {
    .driver       = {
        .name     = "kdp520_touch",
    },
    .probe        = kdp520_touch_probe,
    .remove       = kdp520_touch_remove,
    .core_dev     = &kdp520_touch,
    .ctrller_ops  = (void*)&kdp520_touch_ops,
    
    .power_mgr    = {
        .power    = kdp520_touch_power,
        .suspend  = NULL,
        .resume   = NULL,
    },
    
    .attach_panel = kdp520_touch_attach_panel,
    .init         = kdp520_touch_init,
    .start        = kdp520_touch_start,
    .stop         = kdp520_touch_stop,
    .is_started   = kdp520_touch_is_started,
    .set_hook     = kdp520_touch_set_hook,

    .x_range_max = CFG_TOUCH_X_RANGE_MAX,
    .y_range_max = CFG_TOUCH_Y_RANGE_MAX,
    .inverse_x_axis = 0,
    .inverse_y_axis = 0,
}; 
