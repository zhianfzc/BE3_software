#ifndef __MSG_H__
#define __MSG_H__
#include "board_kl520.h"

#include "com.h"
#define FLAG_COMM_START      BIT(0)
#define FLAG_COMM_ISR        BIT(1)
#define FLAG_COMM_TIMER      BIT(2)
#define FLAG_COMM_APP_DONE   BIT(3) // for tid_appmgr
#define FLAG_COMM_USB_DONE   BIT(4)
#define FLAG_COMM_UART_DONE  BIT(5)

#define USB_HOST                (YES)

#if USB_HOST == 1
#define FLAG_COMM_RX_DONE    FLAG_COMM_USB_DONE
#else
#define FLAG_COMM_RX_DONE    FLAG_COMM_UART_DONE
#endif

enum WR_MODE {
    WMODE_DEF = 0, // regular response message, send immediately
    WMODE_ACK,     // ack packet, send immediately
    WMODE_ALT      // FR data, send only after response message is done
};

/* Modified MSG_DATA_BUF_MAX from 0x100 to 0x1400 to improve the speed of testcase verification */
//extern u8 msg_rbuf[MSG_DATA_BUF_MAX + sizeof(MsgHdr)+ sizeof(RspPram) + 4 + 4];
//extern u8 msg_tbuf[MSG_DATA_BUF_MAX + sizeof(MsgHdr)+ sizeof(RspPram) + 4 + 4];
extern u8* msg_rbuf;
extern u8* msg_tbuf;

void kcomm_msg_init(osThreadId_t tid_comm);
extern int32_t kcomm_read(u32 address, u32 size);
int kcomm_write_msg(u8 *buf, int len, int crc_flag);
int kcomm_write(u8 *buf, int len, int mode);
void kcomm_wait(void);
void kcomm_send_no_rsp(void);
void kcomm_send_rsp(u16 cmd, u8 *buf, int len);

u16 gen_crc16(u8 *data, u32 size);

#endif
