/* -------------------------------------------------------------------------- 
 * @file msg.c 
 *      pc command message threads (Uart/USB)
 *---------------------------------------------------------------------------*/
#include "host_msg.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#include <stdarg.h>
#include "io.h"
#include "string.h"
//#include "kmdw_console.h"
#include "host_usb_com.h"
#include "host_uart_com.h"

#define DEF_LOG_CATEG  "host_msg"

#define CRC16_CONSTANT 0x8005

//u8 msg_rbuf[MSG_DATA_BUF_MAX + sizeof(MsgHdr)+ sizeof(RspPram) + 4 + 4];  // allow u16 crc value plus
//u8 msg_tbuf[MSG_DATA_BUF_MAX + sizeof(MsgHdr)+ sizeof(RspPram) + 4 + 4];  // workaround needed for usb bug
u8* msg_rbuf = NULL;
u8* msg_tbuf = NULL;

/* ############################
 * ##    static functions    ##
 * ############################ */

#if ( USB_HOST == NO )
static void save_command(void)
{
    memcpy(msg_tbuf, msg_rbuf, msg_rbuf[2]+4);  // copying the received command (must be < 255 bytes) to tx buf
}

static void restore_command(void)
{
    memcpy(msg_rbuf, msg_tbuf, msg_tbuf[2]+4);  // copying the received command to tx buf
}

#endif

/* ############################
 * ##    public functions    ##
 * ############################ */

u16 gen_crc16(u8 *data, u32 size)
{
    u16 out = 0;
    int bits_read = 0, bit_flag, i;

    /* Sanity check: */
    if (data == NULL)
        return 0;

    while (size > 0) {
        bit_flag = out >> 15;

        /* Get next bit: */
        out <<= 1;
        out |= (*data >> bits_read) & 1; // item a) work from the least significant bits

                                         /* Increment bit counter: */
        bits_read++;
        if (bits_read > 7)
        {
            bits_read = 0;
            data++;
            size--;
        }

        /* Cycle check: */
        if (bit_flag)
            out ^= CRC16_CONSTANT;

    }

    // push out the last 16 bits
    for (i = 0; i < 16; ++i) {
        bit_flag = out >> 15;
        out <<= 1;
        if (bit_flag)
            out ^= CRC16_CONSTANT;
    }

    // reverse the bits
    u16 crc = 0;
    i = 0x8000;
    int j = 0x0001;
    for (; i != 0; i >>= 1, j <<= 1) {
        if (i & out) crc |= j;
    }

    return crc;
}

/**
 * @brief kcomm_write(), write raw msg to host
 * @param buf buffer data to write
 * @param len data length
 */
int kcomm_write(u8 *buf, int len, int mode)
{
#if ( USB_HOST == YES )
    usb_com_write(buf, len, FLAG_COMM_RX_DONE, mode);  // use msg mode
#else
    kdp_uart_com_write(buf, len);
#endif

    return 0;
}

void kcomm_send_no_rsp(void)
{
#if ( USB_HOST == YES )
    usb_idle();
#endif
}

void kcomm_msg_init(osThreadId_t tid_comm)
{
#if ( USB_HOST == YES )
    usb_set_notify_flag(tid_comm, FLAG_COMM_RX_DONE);
    usb_init();
#else
    kdp_uart_com_init();
#endif
}

/**
 * @brief kcomm_write_msg(), write msg to PC by uart
 *        also format uart packet here
 * @param buf buffer data to write
 * @param len data length
 */
int kcomm_write_msg(u8 *buf, int len, int crc_flag)
{
    MsgHdr *msghdr;
    msghdr = (MsgHdr*) buf;
    msghdr->preamble = MSG_HDR_RSP;

    if (crc_flag) {
        u32 *crc = (u32*)&buf[len];
        *crc = gen_crc16(buf, len);
        msghdr->ctrl = len | PKT_CRC_FLAG; // packet + crc(4B) - pkt header (2B)
        len += 4; // adjust len to include crc
    } else {
        msghdr->ctrl = len - 4;
    }

    kcomm_write(buf, len, WMODE_DEF);

    return 0;
}

/**
 * @brief kcomm_send_rsp(), pack message and send out, use crc for UART only
 * @param cmd message command
 * @param buf data buffer
 * @param len length of data buffer, if len < 0, don't flip the cmd
 */
void kcomm_send_rsp(u16 cmd, u8 *buf, int len)
{
    if (len > 0) { // copy stuff beyond len and address
        memcpy((msg_tbuf + (sizeof(MsgHdr)+sizeof(RspPram))), buf, len);
    }

    MsgHdr *hdr = (MsgHdr*) msg_tbuf;
    CmdPram *args = (CmdPram*) (msg_rbuf + sizeof(MsgHdr));
    RspPram *rsp = (RspPram*) (msg_tbuf + sizeof(MsgHdr));

    if (len >= 0) {
        hdr->cmd = cmd | 0x8000;
        hdr->msg_len = len + sizeof(RspPram);  // minimum msg payload include RspPram
        rsp->param1 = args->param1;
        rsp->param2 = args->param2;
        len += sizeof(MsgHdr)+sizeof(RspPram);
    } else {
        hdr->cmd = cmd;
        hdr->msg_len = 0; // no message payload
        len = sizeof(MsgHdr); // just send msg header
    }

    /* params won't be sent if len < 0 */
    dlog("resp: param1 [%u], param2 [%u], data len [%d]", args->param1, args->param2, len);

#if ( USB_HOST == YES )
    kcomm_write_msg(msg_tbuf, len, 0);  // no crc
#else
    kcomm_write_msg(msg_tbuf, len, 1);  // use crc
#endif
}

void kcomm_wait(void)
{
#if ( USB_HOST == YES )
    while (usb_get_mode() != 1)
        ;  // wait till message is done
#endif
}

// The ack_packet needs to be static, place it on stack doesn't seem to work
// TODO: encode source id and add crc for uart data mode
u8 ack_packet[20] = {0x35, 0x8A, 12, 0, 4, 0x80, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int32_t kcomm_read(u32 address, u32 size)
{
#if ( USB_HOST == YES )
    if(size == 0 || size >= 1024*1024) return -1; //if wrong length

    usb_com_write (ack_packet, 16, FLAG_COMM_RX_DONE, WMODE_ACK);
    return usb_com_read((u8*)address, size, FLAG_COMM_RX_DONE, 1);
#else
    // NOTE: For uart, we also use msg_rx buffer for data receiving, thus destroying the command data that might
    //       be needed later.  So we must preserve it (save it in the msg_tx buffer) for restoration at the end.
    u32 block = size >> 12;
    u32 residual = size & 0x0FFF;
    u32 offset = 0;

    save_command();

    while (block) {
        kdp_uart_com_write(ack_packet, 16);
        kdp_uart_com_read((u8*) address + offset, 0x1000); // FLAG_COMM_UART_DONE);
        offset += 0x1000;
        block--;
    }

    if (residual) {
        kdp_uart_com_write(ack_packet, 16);
        kdp_uart_com_read((u8*) address + offset, residual); //, FLAG_COMM_UART_DONE);
    }

    restore_command();

    return size;
#endif
}
#endif
