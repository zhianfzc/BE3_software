#include "board_kl520.h"
#if (CFG_TOUCH_TYPE == TOUCH_TYPE_GT911)
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
#include "delay.h"
#include "dbg.h"

#include "touch.h"
#include "kdp520_i2c.h"
#include "kl520_include.h"

#define UART_DEBUG_GT911                (0)
#define UART_DEBUG_GT911_INFO           (0)

#define MAX_CONTACTS                    (5)
#define CONTACT_SIZE                    (8)
#define MAX_RX_CONTACT_SIZE             (MAX_CONTACTS*CONTACT_SIZE)
#define MAX_RX_STATUS_SIZE              (1)
#define MAX_RX_BUF_SIZE                 (MAX_RX_STATUS_SIZE+MAX_RX_CONTACT_SIZE)

#define GTP_I2C_ADDR                    (0x14)

#define GTP_CMD_REG                     (0x8040)
#define GTP_CFG_REG                     (0x8047)   	//Config Address
#define GTP_CHK_REG                     (0x80FF)    //Check Sum Err
#define GTP_PID_ADDR                    (0x8140)    //Information
#define GTP_VID_ADDR                    (0x814A)
#define GTP_PID_LEN                     (11)

// For "GTP_CMD_REG" Command
#define CTP_CMD_RPT_MODE                (0x00)
#define CTP_CMD_SLEEP_MODE              (0x05)      //Don't scan.
#define CTP_CMD_CHG_MODE_EN             (0x06)
#define CTP_CMD_CHG_MODE_DIS            (0x07)
#define CTP_CMD_GESTURE_MODE            (0x08)

//Touch Data Address, "GTP_TDA_REG"
#define GTP_COORD_REG                   (0x814E)    //Touch states
#define GTP_TP1_REG                     (0x8150)    //Touch 1
#define GTP_TP2_REG                     (0x8158)    //Touch 2
#define GTP_TP3_REG                     (0x8160)    //Touch 3
#define GTP_TP4_REG                     (0x8168)    //Touch 4
#define GTP_TP5_REG                     (0x8170)    //Touch 5

// For "GTP_TDA_REG"
#define RX_TOUCH_BUFFER_MASK            (0x80)
#define RX_LARGE_DETECT_MASK            (0x40)
#define RX_TOUCH_MASK                   (0x0F)

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

#define TOUCH_EVENT_DOWN                (0x00)
#define TOUCH_EVENT_UP                  (0x01)
#define TOUCH_EVENT_ON                  (0x02)
#define TOUCH_EVENT_RESERVED            (0x03)

#define MOUSE_DOWN_MAX_CNT              (1)//(1)
#define MOUSE_UP_MAX_CNT                (1)//(1)

t_touch_data __touch_data;
static u8 __cnt_down = 0;
static u8 __cnt_up = 0;
static kl520_mouse_state __pre_tp_state = MOUSE_NONE;

void _gt911_WDB(u16 nRegAddr, u8 nWD)
{
#if (UART_DEBUG_GT911)
    u8 nBuf;
    kdp_drv_i2c_read(I2C_ADAP_0, GTP_I2C_ADDR, nRegAddr, 2, &nBuf);
    dbg_msg_console("rAddr 0x%x: 0x%x", nRegAddr, nBuf);
#endif

    kdp_drv_i2c_write(I2C_ADAP_0, GTP_I2C_ADDR, nRegAddr, 2, nWD);

#if (UART_DEBUG_GT911)
    kdp_drv_i2c_read(I2C_ADAP_0, GTP_I2C_ADDR, nRegAddr, 2, &nBuf);
    dbg_msg_console("CMD_WD wAddr 0x%x: 0x%x", nRegAddr, nBuf);
#endif
}

u8 _gt911_RDB(u16 nRegAddr)
{
    u8 nBuf;

    kdp_drv_i2c_read(I2C_ADAP_0, GTP_I2C_ADDR, nRegAddr, 2, &nBuf);

#if (UART_DEBUG_GT911)
    dbg_msg_console("CMD_RD rAddr 0x%x: 0x%x", nRegAddr, nBuf);
#endif
    return nBuf;
}

void _gt911_RDBS(u16 nRegAddr, u8 *lpdata, u8 data_len)
{
    kdp_drv_i2c_read_bytes(I2C_ADAP_0, GTP_I2C_ADDR, nRegAddr, 2, lpdata, data_len);
}

void _gt911_ModeChange_Init(void)
{
    //GPIO setting
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
}

void _gt911_ModeChange_Normal2Sleep(void)
{
    kdp520_gpio_setdir(KDP_GPIO_INT_PIN_FOR_TOUCH, GPIO_DIR_OUTPUT);
    kdp520_gpio_cleardata(1<<KDP_GPIO_INT_PIN_FOR_TOUCH);
    _gt911_WDB(GTP_CMD_REG, CTP_CMD_SLEEP_MODE);
    osDelay(GTP_RST_T4_MS);
    kdp520_gpio_setdir(KDP_GPIO_INT_PIN_FOR_TOUCH, GPIO_DIR_INPUT);
}

void _gt911_ModeChange_Sleep2Normal(void)
{
    kdp520_gpio_setdir(KDP_GPIO_INT_PIN_FOR_TOUCH, GPIO_DIR_OUTPUT);
    kdp520_gpio_setdata(1<<KDP_GPIO_INT_PIN_FOR_TOUCH);
    osDelay(GTP_RST_T3_MS);
    kdp520_gpio_cleardata(1<<KDP_GPIO_INT_PIN_FOR_TOUCH);
    osDelay(GTP_RST_T4_MS);
    kdp520_gpio_setdir(KDP_GPIO_INT_PIN_FOR_TOUCH, GPIO_DIR_INPUT);
    osDelay(GTP_RST_T5_MS);
}

void _gt911_ModeChange_Normal2Charger(void)
{
    _gt911_WDB(GTP_CMD_REG, CTP_CMD_CHG_MODE_EN);
}

void _gt911_ModeChange_Charger2Normal(void)
{
    _gt911_WDB(GTP_CMD_REG, CTP_CMD_CHG_MODE_DIS);
}

void _gt911_StartUpCMD(void)
{
    u8 i = 0;
    u8 nDelTime;

    nDelTime = (_gt911_RDB(0x8056)&0x0F)+5;

    while(1)
    {
        _gt911_WDB(GTP_CMD_REG, CTP_CMD_RPT_MODE);
        _gt911_WDB(GTP_COORD_REG, 0);

        osDelay(nDelTime);
        if ( (++i) == 3 )
        {
            break;
        }
    }
}

int _gt911_init(struct core_device *core_d)
{
    int ret;
    struct touch_driver *touch_drv = &kdp520_touch_driver;
    struct touch_driver_ops *ops = (struct touch_driver_ops*)touch_drv->ctrller_ops;

    if (!ops)
        return -1;

    _gt911_ModeChange_Init();
    _gt911_StartUpCMD();

    ret = 0;

    return ret;
}

void _gt911_start(struct core_device *core_d)
{
    struct touch_driver *touch_drv = &kdp520_touch_driver;
    struct touch_driver_ops *ops = (struct touch_driver_ops*)touch_drv->ctrller_ops;
    
    ops->tp_gpio_set_int_start();
}
void _gt911_stop(struct core_device *core_d)
{
    struct touch_driver *touch_drv = &kdp520_touch_driver;
    struct touch_driver_ops *ops = (struct touch_driver_ops*)touch_drv->ctrller_ops;

    ops->tp_gpio_set_int_stop();
    _gt911_ModeChange_Normal2Sleep();
}

void _gt911_get_raw_data(struct core_device *core_d, u8 init_i2c)
{
    struct touch_driver *touch_drv = &kdp520_touch_driver;
    struct touch_driver_ops *ops = (struct touch_driver_ops*)touch_drv->ctrller_ops;

    u16 nRegAddr;
    u8  aRxBuf[MAX_RX_BUF_SIZE] = {0};
    u8  i, nIdx = 0, nTchNum, nID;
    u32 nIptX, nIptY;
//    u32 nSzie;

#if (UART_DEBUG_GT911) || (UART_DEBUG_GT911_INFO)
    BOOL bBufferStatus = FALSE;
    BOOL bLargeDetect = FALSE;
#endif

    ops->tp_gpio_set_int_stop();

    nRegAddr = GTP_COORD_REG;
    _gt911_RDBS(nRegAddr, aRxBuf, MAX_RX_BUF_SIZE);

    if ( aRxBuf[nIdx] != 0 )
    {
        _gt911_WDB(nRegAddr, 0);
    }

    nTchNum = ( aRxBuf[nIdx]&RX_TOUCH_MASK );

#if (UART_DEBUG_GT911) || (UART_DEBUG_GT911_INFO)
    bBufferStatus = ( aRxBuf[0]&RX_TOUCH_BUFFER_MASK ) ? 1:0;
    bLargeDetect = ( aRxBuf[0]&RX_LARGE_DETECT_MASK ) ? 1:0;
    dbg_msg_console("---------- Touch Information ----------");
    dbg_msg_console("Touch Buffer Status: %d", bBufferStatus);
    dbg_msg_console("Large Detect Status: %d", bLargeDetect);
    dbg_msg_console("       Touch Number: %d", nTchNum);
#endif

    nIdx++;
    for ( i = 0; i < nTchNum; i++, nIdx += CONTACT_SIZE )
    {
        nID   = aRxBuf[nIdx];
        nIptX = (unsigned int)((aRxBuf[nIdx+2]<<8)|aRxBuf[nIdx+1]);
        nIptY = (unsigned int)((aRxBuf[nIdx+4]<<8)|aRxBuf[nIdx+3]);
//        nSzie = (unsigned int)((aRxBuf[nIdx+6]<<8)|aRxBuf[nIdx+5]);

        if ( ( nIptX < 120 ) || ( nIptX > 360 ) )
        {
            continue;
        }
        else
        {
            nIptX -= 120;
        }

        nIptY = ((nIptY*QVGA_PORTRAIT_HEIGHT)/TFT43_HEIGHT);

        __touch_data.count++;
        if (touch_drv->inverse_x_axis)
            __touch_data.input_x[nID] = touch_drv->x_range_max - nIptX;
        else
            __touch_data.input_x[nID] = nIptX;

        if (touch_drv->inverse_y_axis)
            __touch_data.input_y[nID] = touch_drv->y_range_max - nIptY;
        else
            __touch_data.input_y[nID] = nIptY;

#if (UART_DEBUG_GT911)
        dbg_msg_console("Touch ID: %d", i+1);
        dbg_msg_console("Track ID: %d", nID);
        dbg_msg_console(" Coord X: %d", nIptX);
        dbg_msg_console(" Coord Y: %d", nIptY);
//        dbg_msg_console("    Size: %d", nSzie);
        dbg_msg_console("\n");
#endif
    }

    ops->tp_gpio_set_int_start();
}

void _gt911_state_handler(struct core_device *core_d, void* pData)
{
    kl520_mouse_info* data = (kl520_mouse_info*)pData;

    if (__touch_data.count == 0)
    {
        __cnt_down = 0;
        if (MOUSE_DOWN==__pre_tp_state || MOUSE_MOVE==__pre_tp_state)
        {
            data->state = MOUSE_UP;
            __cnt_up++;
        }
        else if (MOUSE_UP==__pre_tp_state)
        {
            if (MOUSE_UP_MAX_CNT <= __cnt_up)
                data->state = MOUSE_NONE;
            else
                __cnt_up++;
        }
        else
            data->state = MOUSE_NONE;
    }
    else
    {
        __cnt_up = 0;
        if (__cnt_down < MOUSE_DOWN_MAX_CNT)
        {
            data->state = MOUSE_DOWN;
            __cnt_down++;
        }
        else
        {
            data->state = MOUSE_MOVE;
        }
    }

    __pre_tp_state = data->state;
    //single touch
    data->x = __touch_data.input_x[0];
    data->y = __touch_data.input_y[0];
}

void _gt911_set_inactive(void) {
    __touch_data.count = 0;
}

int _gt911_get_active(void) {
    if (MOUSE_NONE == __pre_tp_state)
        return 0;
    else
        return 1;
}

static u16 _gt911_get_info(struct core_device * core)
{
#define GT911_INFO  (0)
    struct touch_driver *touch_drv = &kdp520_touch_driver;
    struct touch_driver_ops *ops = (struct touch_driver_ops*)touch_drv->ctrller_ops;

    u16 nRegAddr;
    u8  aRxBuf[GTP_PID_LEN] = {0};

    u32 nPID;
#if (GT911_INFO) || (UART_DEBUG_GT911) || (UART_DEBUG_GT911_INFO)
    u16 nFV, nCoordRslX, nCoordRslY;
    u8  nVID;
    u8  nTemp;
#endif

    ops->tp_gpio_set_int_stop();

    nRegAddr = GTP_CFG_REG;
    _gt911_RDBS(nRegAddr, aRxBuf, 8);

#if (UART_DEBUG_GT911) || (UART_DEBUG_GT911_INFO)
    dbg_msg_console("---------- Config Information ----------");
    nTemp = ( aRxBuf[6]&0x08 ) ? 1:0;
    dbg_msg_console("          CFG Version: 0x%x", aRxBuf[0]);
    dbg_msg_console("   Coord Resolution X: %d", ((aRxBuf[2]<<8)|aRxBuf[1]));
    dbg_msg_console("   Coord Resolution Y: %d", ((aRxBuf[4]<<8)|aRxBuf[3]));
    dbg_msg_console("       Coord X2Y Mode: %d", nTemp);
    dbg_msg_console(" Touch Support Number: %d", (aRxBuf[5]&0x0F));
    dbg_msg_console("     INT Trigger Type: %d", (aRxBuf[7]&0x03));
#endif

    nRegAddr += 0x08;
    _gt911_RDBS(nRegAddr, aRxBuf, 8);

#if (UART_DEBUG_GT911) || (UART_DEBUG_GT911_INFO)
    dbg_msg_console("        Palm Node Thr: %d", (aRxBuf[2]));
    dbg_msg_console("            Touch Thr: %d", (aRxBuf[4]));
    dbg_msg_console("      Touch Leave Thr: %d", (aRxBuf[5]));
    dbg_msg_console("         Touch Bounce: %d", (aRxBuf[0]&0x0F));
    dbg_msg_console("Enter Green Mode Time: %d/s", (aRxBuf[6]&0x0F));
    dbg_msg_console("          Report Rate: %d/ms", (aRxBuf[7]&0x0F)+5);
#endif

    _gt911_RDBS(GTP_PID_ADDR, aRxBuf, GTP_PID_LEN);
    nPID        = ((aRxBuf[3]<<24)|(aRxBuf[2]<<16)|(aRxBuf[1]<<8)|aRxBuf[0]);
#if (GT911_INFO) || (UART_DEBUG_GT911) || (UART_DEBUG_GT911_INFO)
    nFV         = ((aRxBuf[5]<<8)|aRxBuf[4]);
    nCoordRslX  = ((aRxBuf[7]<<8)|aRxBuf[6]);
    nCoordRslY  = ((aRxBuf[9]<<8)|aRxBuf[8]);
    nVID        = aRxBuf[10];
#endif

#if ( UART_DEBUG_GT911 ) || ( UART_DEBUG_GT911_INFO )
    dbg_msg_console("---------- Coord Information ----------");
    dbg_msg_console("           Product ID: 0x%x.0x%x.0x%x.0x%x", aRxBuf[3], aRxBuf[2], aRxBuf[1], aRxBuf[0]);
    dbg_msg_console("     Firmware Version: 0x%x.0x%x", aRxBuf[5], aRxBuf[4]);
    dbg_msg_console("   Coord Resolution X: %d", nCoordRslX);
    dbg_msg_console("   Coord Resolution Y: %d", nCoordRslY);
    dbg_msg_console("            Vendor ID: %d", nVID);
    dbg_msg_console("\n");
#endif

    ops->tp_gpio_set_int_start();

    return nPID;
}

struct touch_panel_driver gt911_driver = {
    .init             = _gt911_init,
    .start            = _gt911_start,
    .stop             = _gt911_stop,
    .get_raw_data     = _gt911_get_raw_data,
    .state_handler    = _gt911_state_handler,
    .set_inactive     = _gt911_set_inactive,
    .get_active       = _gt911_get_active,
    .read_did         = _gt911_get_info,
};

#endif
