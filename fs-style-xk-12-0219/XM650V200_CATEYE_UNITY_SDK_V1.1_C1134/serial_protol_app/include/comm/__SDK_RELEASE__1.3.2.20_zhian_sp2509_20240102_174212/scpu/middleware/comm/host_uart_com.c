/*
 * Kneron Host UART Communication driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#include "host_uart_com.h"

#include "kdp_uart.h"//"kdrv_uart.h"
#include "io.h"
#include "kneron_mozart.h"  
#include "com.h"
#include "flash.h"
#include "sample_app_spi.h"

#ifdef USB_HOST
#else
#include "Driver_Common.h"
//#include "kmdw_console.h"

#define FLAG_UART4_RX_DONE BIT(9)
#define FLAG_UART4_RX_TIMEOUT BIT(10)
#define FLAG_UART4_TX_DONE BIT(11)

static kdrv_uart_handle_t handle;
static osThreadId_t tid_uart_com;

bool UART4_Rx = false;
bool UART4_Tx = false;

#ifndef BOARD_CUST1

static uint32_t flags_to_notify, data_size;
static osThreadId_t tid_to_notify;
static uint32_t kdp_uart_data;
static u8 *data_start;
#endif
#endif

static  kdp_uart_hdl_t  handle2;		//
//static  kdp_uart_hdl_t  handle3;		//
static  kdp_uart_hdl_t  handle4;		//


#define FLAG_UART4_TEST_RX_DONE         BIT(9)
#define FLAG_UART4_TEST_RX_TIMEOUT      BIT(10)
#define FLAG_UART4_TEST_TX_DONE         BIT(11)


bool UART0_Rx = false;
bool UART0_Tx = false;

bool UART1_Rx = false;
bool UART1_Tx = false;

bool UART2_Rx = false;
bool UART2_Tx = false;

bool UART3_Rx = false;
bool UART3_Tx = false;

bool UART4_Rx = false;
bool UART4_Tx = false;


void UART0_callback(uint32_t event)
{
    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
        UART0_Rx = true;
    }

    if (event & ARM_USART_EVENT_SEND_COMPLETE)
    {
        UART0_Tx = true;
    }
}


void UART1_callback(uint32_t event)
{
    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
        UART1_Rx = true;
    }

    if (event & ARM_USART_EVENT_SEND_COMPLETE)
    {
        UART1_Tx = true;
    }
}

void UART2_callback(uint32_t event)
{
    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
        UART2_Rx = true;
    }

    if (event & ARM_USART_EVENT_SEND_COMPLETE)
    {
        UART2_Tx = true;
    }
}

void UART3_callback(uint32_t event)
{
    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
        UART3_Rx = true;
    }

    if (event & ARM_USART_EVENT_SEND_COMPLETE)
    {
        UART3_Tx = true;
    }
}

void UART4_callback(uint32_t event)
{
    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
        UART4_Rx = true;
    }

    if (event & ARM_USART_EVENT_SEND_COMPLETE)
    {
        UART4_Tx = true;
    }

    if (event & ARM_USART_EVENT_RX_TIMEOUT)
    {
        UART4_Rx = true;
    }
}

void kdp_uart_app_com(u8 port, u32 baudrate, u8* buffer, u32 size)
{
    int32_t ret;
    UINT32	data;
    kdp_uart_hdl_t handle;
    
    if (4 == port) {
        kdp_uart_close(UART4_DEV);
        //pad mux
        data = inw( SCU_EXTREG_PA_BASE + 0x18C );
        data &= 0xFFFFFFF8;		//clear low 3bit
        data &= 0xFFFFFFE7;		//clear bit 3 and bit4
        
        outw( SCU_EXTREG_PA_BASE + 0x18C, data | 0x6 | 1<<4 );
        data = inw( SCU_EXTREG_PA_BASE + 0x190 );
        data &= 0xFFFFFFF8;		//clear low 3bit
        data &= 0xFFFFFFE7;		//clear bit 3 and bit4
        
        outw( SCU_EXTREG_PA_BASE + 0x190, data | 0x6 | 1<<4 );
        
        handle = handle4;
        handle = kdp_uart_open(UART4_DEV, UART_MODE_ASYN_RX | UART_MODE_SYNC_TX, UART4_callback);
    }
    else if (2 == port) {
        kdp_uart_close(UART2_DEV);
        //pad mux		//20200118 Jeff add
        kdp_uart_close(UART2_DEV);
        data = inw(SCU_EXTREG_PA_BASE + 0x154);
        data &= 0xFFFFFFF8;		//clear low 3bit
        data &= 0xFFFFFFE7;		//clear bit 3 and bit4
        
        outw(SCU_EXTREG_PA_BASE + 0x154, data | 0x1 | 1<<4 );

        data = inw(SCU_EXTREG_PA_BASE + 0x158);
        data &= 0xFFFFFFF8;		//clear low 3bit
        data &= 0xFFFFFFE7;		//clear bit 3 and bit4
        
        outw(SCU_EXTREG_PA_BASE + 0x158, data | 0x1 | 1<<4 );

        handle = handle2;
        handle = kdp_uart_open(UART2_DEV, UART_MODE_ASYN_RX | UART_MODE_SYNC_TX, UART2_callback);        
    }
    else
    {
      return;
    }

    
    if (handle == UART_FAIL)
        return;

    ret = kdp_uart_power_control(handle, ARM_POWER_FULL);
    if (ret != UART_API_RETURN_SUCCESS)
        return;

    KDP_UART_CONFIG_t cfg;
    cfg.baudrate = baudrate;
    cfg.data_bits = 8;
    cfg.frame_length = 0;
    cfg.stop_bits = 1;
    cfg.parity_mode = PARITY_NONE;
    cfg.fifo_en = TRUE;

    ret = kdp_uart_control(handle, UART_CTRL_CONFIG, (void *)&cfg);
    if (ret != UART_API_RETURN_SUCCESS)
        return;

    kdp_uart_read( handle, buffer, size );                  //for read case
    
    if (4 == port) {
        UART4_Rx = FALSE;
        UART4_Tx = FALSE;
    }
    else if (2 == port) {
        UART2_Rx = FALSE;
        UART2_Tx = FALSE;        
    }

}

#ifdef USB_HOST
#else
/* ############################
 * ##    static functions    ##
 * ############################ */
#ifndef BOARD_CUST1
static void uart_comm_thread(void *argument)
{
    int32_t flags;

    uint32_t msg_buf_max_len = MSG_DATA_BUF_MAX + sizeof(MsgHdr) + sizeof(RspPram) + 4;

    UART4_Rx = FALSE;
    memset(msg_rbuf, 0, msg_buf_max_len);
    kdp_uart_data = 0;
    kdrv_uart_read(handle, msg_rbuf, msg_buf_max_len);

    for (;;) {
        flags = osThreadFlagsWait(FLAG_UART4_RX_DONE | FLAG_UART4_RX_TIMEOUT, osFlagsWaitAny, osWaitForever);
        osThreadFlagsClear(flags);

        if (flags & (FLAG_UART4_RX_DONE | FLAG_UART4_RX_TIMEOUT))
        {
            uint32_t count = kdrv_uart_get_rx_count(handle);
            if (kdp_uart_data) {
                memcpy(data_start, msg_rbuf, count);
                if (count < data_size) {  // only partial data read
                    // TO DO: copy partial data and restart the data read
                    err_msg("[UART] data block read failed\r\n");
                    data_size = count;
                }
                kdp_uart_data = 0;  // exit data mode
                osThreadFlagsSet(tid_to_notify, flags_to_notify);  // wake up calling thread
            }
            else {
                osThreadFlagsSet(tid_to_notify, flags_to_notify);  // wake up calling thread
            }

            UART4_Rx = FALSE;
            kdrv_status_t sts4 = kdrv_uart_read(handle, msg_rbuf, msg_buf_max_len);
            if (sts4 != KDRV_STATUS_UART_TX_RX_BUSY) {
                dbg_msg("failed on kdrv_uart_read, error code = %d\n", sts4);
            }
        }
    }
}

#endif

static void UART4_callback(uint32_t event)
{
    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
        UART4_Rx = true;
        osThreadFlagsSet(tid_uart_com, FLAG_UART4_RX_DONE);
    }

    if (event & ARM_USART_EVENT_SEND_COMPLETE)
    {
        UART4_Tx = true;
        osThreadFlagsSet(tid_uart_com, FLAG_UART4_TX_DONE);
    }

    if (event & ARM_USART_EVENT_RX_TIMEOUT)
    {
        UART4_Rx = true;
        osThreadFlagsSet(tid_uart_com, FLAG_UART4_RX_TIMEOUT);
    }

}

/* ############################
 * ##    public functions    ##
 * ############################ */

/* kai2do: used by drivers/media/platform/kdp_command_adapter.c? */
void kdp_uart_app_com4(void)
{
    // UART4 pinmux
    kdrv_pinmux_config(KDRV_PIN_SD_DAT_2, PIN_MODE_6, PIN_PULL_NONE, PIN_DRIVING_NONE);
    kdrv_pinmux_config(KDRV_PIN_SD_DAT_3, PIN_MODE_6, PIN_PULL_NONE, PIN_DRIVING_NONE);

    kdrv_status_t sts = kdrv_uart_open(&handle, UART4_DEV, UART_MODE_ASYN_RX | UART_MODE_SYNC_TX, UART4_callback);
    if (sts != KDRV_STATUS_OK) {
        dbg_msg("Open failed\n");
        return;
    }

    kdrv_uart_config_t cfg;
    cfg.baudrate = BAUD_921600;
    cfg.data_bits = 8;
    cfg.frame_length = 0;
    cfg.stop_bits = 1;
    cfg.parity_mode = PARITY_NONE;
    cfg.fifo_en = TRUE;

    sts = kdrv_uart_configure(handle, UART_CTRL_CONFIG, (void *)&cfg);
    if (sts != KDRV_STATUS_OK) {
        dbg_msg("UART config failed\n");
        return;
    }

    UART4_Rx = FALSE;
    UART4_Tx = FALSE;
}

void kdp_uart_com_init(void)
{
#ifndef BOARD_CUST1
    osThreadAttr_t attr = {
        .stack_size = 512
    };

    kdp_uart_app_com4();
    tid_uart_com = osThreadNew(uart_comm_thread, NULL, &attr);
#endif
}

int kdp_uart_com_read(uint8_t *BufferPtr, uint32_t BufferLen)
{
#ifndef BOARD_CUST1
    kdp_uart_data = 1;
    data_start = BufferPtr;
    osThreadFlagsWait(flags_to_notify, osFlagsWaitAll, osWaitForever);  // put calling thread to sleep
    return data_size;
#else
    return 0;
#endif
}

void kdp_uart_com_write(uint8_t *p, int32_t len)
{
    kdrv_uart_write(handle, p, len);
}

int kdp_com_getc(uint8_t *BufferPtr)
{
    int32_t flags;
    kdrv_uart_read(handle, BufferPtr, 1);
    flags = osThreadFlagsWait(FLAG_UART4_RX_DONE | FLAG_UART4_RX_TIMEOUT, osFlagsWaitAny, osWaitForever);
    osThreadFlagsClear(flags);

    return 0;

}

void kdp_com_putc(char ch)
{
    kdrv_uart_write(handle, (uint8_t *)&ch, 1);
}
#endif
