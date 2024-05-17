//
// Created by Liming Shao on 4/30/2018.
//

#ifndef __KDP_COMM_UTILS_H__
#define __KDP_COMM_UTILS_H__

#include "board_kl520.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP )
#include <stdint.h>

void printHex(const uint8_t *ptr, int len, char *tag);

void printState(uint8_t state[4][4], char *tag);
#endif
#endif
#endif
#endif    //__KDP_COMM_UTILS_H__
