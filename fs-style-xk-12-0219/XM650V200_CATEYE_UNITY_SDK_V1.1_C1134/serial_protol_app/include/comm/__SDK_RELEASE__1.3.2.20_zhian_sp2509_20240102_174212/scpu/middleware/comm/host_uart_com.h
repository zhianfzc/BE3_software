#ifndef __UART_COM_H__
#define __UART_COM_H__
#include "board_kl520.h"
#include "host_msg.h"
#include <stdarg.h>

extern void kdp_uart_app_com(u8 port, u32 baudrate, u8* buffer, u32 size);

#ifdef USB_HOST
#else
extern void kdp_uart_com_init(void);
extern void kdp_uart_com_write(uint8_t *p, int32_t len);
extern int kdp_uart_com_read(uint8_t *BufferPtr, uint32_t BufferLen);
#endif
#endif
