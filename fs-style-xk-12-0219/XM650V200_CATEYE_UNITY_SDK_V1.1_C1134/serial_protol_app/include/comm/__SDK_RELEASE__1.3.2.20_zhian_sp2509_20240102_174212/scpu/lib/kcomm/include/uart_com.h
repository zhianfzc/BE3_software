/*
 * Kneron Host UART Communication Application Header
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#ifndef __UART_COM_H__
#define __UART_COM_H__
#include "board_kl520.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_KCOMM
void kdp_uart_com_init(void);
void kdp_uart_com_write(uint8_t *p, int32_t len);
int kdp_uart_com_read(uint8_t *BufferPtr, uint32_t BufferLen);

int kdp_com_getc(uint8_t *BufferPtr);
void kdp_com_putc(char ch);
#endif
#endif
