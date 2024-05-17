#include "dbg.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "kdp_uart.h"
#include "scu_extreg.h"

#define DBG_BUFFER_SIZE 512

static kdp_uart_hdl_t handle0;
static char uart_buffer[DBG_BUFFER_SIZE];
const char uart_error_msg[30] = "ERROR: UART Buffer Overrun.\r\n\0";

int scpu_debug_level;

void kdp_uart_app_uart0_log(void)
{
    int32_t ret;

    handle0 = kdp_uart_open(DEBUG_CONSOLE, UART_MODE_ASYN_RX | UART_MODE_SYNC_TX, 0);
    if (handle0 == (kdp_uart_hdl_t)UART_FAIL)
    {
        //dbg_msg("Open failed\n");
        return;
    }

    ret = kdp_uart_power_control(handle0, ARM_POWER_FULL);

    if (ret != UART_API_RETURN_SUCCESS)
    {
        //dbg_msg("Power on failed\n");
        return;
    }

    KDP_UART_CONFIG_t cfg;
    cfg.baudrate = BAUD_115200; // BAUD_921600;
    cfg.data_bits = 8;
    cfg.frame_length = 0;
    cfg.stop_bits = 1;
    cfg.parity_mode = PARITY_NONE;
    cfg.fifo_en = FALSE;

    ret = kdp_uart_control(handle0, UART_CTRL_CONFIG, (void *)&cfg);

    if (ret != UART_API_RETURN_SUCCESS)
    {
        //dbg_msg("UART config failed\n");
        return;
    }

    //UART0_Rx = FALSE;
    //UART0_Tx = FALSE;

}

#ifdef TARGET_NCPU
void fLib_printf(const char *f, ...)    /* variable arguments */
{
    va_list arg_ptr;

    va_start(arg_ptr, f);
    vsprintf(&uart_buffer[0], f, arg_ptr);
    va_end(arg_ptr);

    //UART0_Tx = FALSE;
    if (strlen(uart_buffer) > DBG_BUFFER_SIZE) {
        kdp_uart_write(handle0, (u8*) uart_error_msg, strlen(uart_error_msg));
        uart_buffer[DBG_BUFFER_SIZE-1] = 0;  // truncate the original output
    }
    kdp_uart_write(handle0, (u8*) uart_buffer, strlen(uart_buffer));      
}
#else
void kdp_printf(const char *f, ...)    /* variable arguments */
{
    va_list arg_ptr;

    va_start(arg_ptr, f);
    vsnprintf(&uart_buffer[0], DBG_BUFFER_SIZE - 1, f, arg_ptr);
    va_end(arg_ptr);

    //UART0_Tx = FALSE;
//    if (strlen(uart_buffer) > DBG_BUFFER_SIZE) {
//        kdp_uart_write(handle0, (u8*) uart_error_msg, strlen(uart_error_msg));
//        uart_buffer[DBG_BUFFER_SIZE-1] = 0;  // truncate the original output
//    }
    kdp_uart_write(handle0, (u8*) uart_buffer, strlen(uart_buffer));     
}
void kdp_level_printf(int level, const char *fmt, ...) /* variable arguments */
{
    uint32_t lvl = log_get_level_scpu();
    lvl >>= 16;

    if ((level == LOG_PROFILE && level == lvl) || (level > 0 && level <= lvl))
    {
        va_list arg_ptr;

        va_start(arg_ptr, fmt);
        vsprintf(&uart_buffer[0], fmt, arg_ptr);
        va_end(arg_ptr);

        //UART0_Tx = false;
        if (strlen(uart_buffer) > DBG_BUFFER_SIZE)
        {
            kdp_uart_write(handle0, (uint8_t *)uart_error_msg, strlen(uart_error_msg));
            uart_buffer[DBG_BUFFER_SIZE-1] = 0; // truncate the original output
        }
        kdp_uart_write(handle0, (uint8_t *)uart_buffer, strlen(uart_buffer));
    }
}

void kdp_user_level_printf(int level, const char *fmt, ...) /* variable arguments */
{
    uint32_t lvl = log_get_user_level_scpu();
    lvl >>= 12;

    if (level > 0 && level <= lvl)
    {
        va_list arg_ptr;

        va_start(arg_ptr, fmt);
        vsprintf(&uart_buffer[0], fmt, arg_ptr);
        va_end(arg_ptr);

        //UART0_Tx = false;
        if (strlen(uart_buffer) > DBG_BUFFER_SIZE)
        {
            kdp_uart_write(handle0, (uint8_t *)uart_error_msg, strlen(uart_error_msg));
            uart_buffer[DBG_BUFFER_SIZE-1] = 0; // truncate the original output
        }

        kdp_uart_write(handle0, (uint8_t *)uart_buffer, strlen(uart_buffer));
    }
}

#endif
void kdp_printf_nocrlf(const char *f, ...)    /* variable arguments */
{
    va_list arg_ptr;
    char buffer[256];

    va_start(arg_ptr, f);
    vsprintf(&buffer[0], f, arg_ptr);
    va_end(arg_ptr);

    //UART0_Tx = FALSE;
    kdp_uart_write(handle0, (uint8_t*)buffer, strlen(buffer));
}

extern struct scpu_to_ncpu_s *s_out_comm;

u32 log_get_level_scpu(void)
{
    return (s_out_comm->debug_flags & 0x000F0000);
}

u32 log_get_level_ncpu(void)
{
    return (s_out_comm->debug_flags & 0x0000000F);
}

u32 log_get_user_level_scpu(void)
{
    return (s_out_comm->debug_flags & 0x0000F000);
}

u32 log_get_user_level_ncpu(void)
{
    return (s_out_comm->debug_flags & 0x000000F0);
}

void log_set_level_scpu(u32 level)
{
    s_out_comm->debug_flags = (s_out_comm->debug_flags & ~0x000F0000) | (((level) << 16) & 0x000F0000);
    scpu_debug_level = (s_out_comm->debug_flags & 0x000F0000) >> 16;
}

void log_set_level_ncpu(u32 level)
{
    s_out_comm->debug_flags = (s_out_comm->debug_flags & ~0x0000000F) | ((level) & 0x0000000F);
}

void log_set_user_level_scpu(u32 level)
{
    s_out_comm->debug_flags = (s_out_comm->debug_flags & ~0x0000F000) | (((level) << 12) & 0x0000F000);
}

void log_set_user_level_ncpu(u32 level)
{
    s_out_comm->debug_flags = (s_out_comm->debug_flags & ~0x000000F0) | ((level) & 0x000000F0);
}
