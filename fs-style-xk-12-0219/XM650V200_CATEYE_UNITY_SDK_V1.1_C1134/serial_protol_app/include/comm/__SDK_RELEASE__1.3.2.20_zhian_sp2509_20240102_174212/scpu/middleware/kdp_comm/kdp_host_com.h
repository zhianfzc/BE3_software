/*
 * Kneron Host Communication driver
 *
 * Copyright (C) 2018 Kneron, Inc. All rights reserved.
 *
 */
#ifndef __KDP_HOST_COM_H__
#define __KDP_HOST_COM_H__

#include "board_kl520.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK )
#if ( CFG_COM_USB_PROT_TYPE == COM_USB_PROT_KDP )
#include "kl520_com.h"
#include <stdint.h>
#include <errno.h>

#define KDP_CMD_MEM_READ 0x01
#define KDP_CMD_MEM_WRITE 0x02
#define KDP_CMD_CRC_ERR 0x07
#define KDP_CMD_ACK_NACK 0x04
#define KDP_CMD_MEM_CLR 0x06
#define KDP_CMD_TEST_ECHO 0x08
#define KDP_CMD_FILE_WRITE 0x09

#if CFG_USB_PROGRAME_FW_ENABLE == YES
#define KDP_CMD_FLASH_MEM_WRITE 0x0A
#endif

#define KDP_CMD_RESET 0x20
#define KDP_CMD_OTA_UPDATE   0x22

#define KDP_CMD_RECEIVE_IMAGE 0x26
#define KDP_CMD_FDFR_THREAD_CLOSE 0x6000

#if CFG_USB_EXPORT_STREAM_IMG == YES
#define KDP_CMD_EXPORT_STREAM_IMG 0x6100
#endif

extern osThreadId_t tid_host_comm;

/* API */
void user_host_com_init(void);
#endif
#endif
#endif
#endif    //__KDP_HOST_COM_H__
