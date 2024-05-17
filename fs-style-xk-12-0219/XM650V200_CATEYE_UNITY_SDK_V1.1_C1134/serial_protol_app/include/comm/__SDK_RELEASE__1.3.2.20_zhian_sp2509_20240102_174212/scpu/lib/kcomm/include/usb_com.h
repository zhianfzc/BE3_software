#ifndef __USB_H__
#define __USB_H__
#include "board_kl520.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_KCOMM
void usb_idle(void);
int usb_get_mode(void);
void usb_set_notify_flag(osThreadId_t tid, uint32_t flags);
int usb_com_read(uint8_t *BufferPtr, uint32_t BufferLen, uint32_t flags, uint32_t mode);
int usb_com_write(uint8_t *BufferPtr, uint32_t BufferLen, uint32_t flags, uint32_t mode);
void usb_init(void);
#endif
#endif
