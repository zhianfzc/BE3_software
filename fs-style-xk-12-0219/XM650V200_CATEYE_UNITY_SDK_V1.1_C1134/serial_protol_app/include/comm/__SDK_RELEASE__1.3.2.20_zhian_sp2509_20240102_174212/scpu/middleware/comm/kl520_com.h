#ifndef __KL520_COM_H__
#define __KL520_COM_H__
#include "board_kl520.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF )
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
typedef enum kl520_com_flags_enum {
    KL520_COM_NORMAL = 0,
    KL520_COM_HAS_ADDITIONAL_IO = 1,

} kl520_com_flags;
#endif

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
#include "host_uart_com.h"
#include "drivers.h"
#include "kdp_uart.h"


#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
#define	COM_BUS_RESPONSE_OFFESET	(14)		//data size packet
#else
#define	COM_BUS_RESPONSE_OFFESET	(0)
#endif

#define     COM_BUS_HEAD_RX                         ((UINT32)0x78875AA5)
#define     COM_BUS_HEAD_RX_1                       ((COM_BUS_HEAD_RX&0xFF000000)>>24 )
#define     COM_BUS_HEAD_RX_2                       ((COM_BUS_HEAD_RX&0x00FF0000)>>16 )
#define     COM_BUS_HEAD_RX_3                       ((COM_BUS_HEAD_RX&0x0000FF00)>>8 )
#define     COM_BUS_HEAD_RX_4                       ((COM_BUS_HEAD_RX&0x000000FF)>>0 )

#define     COM_BUS_HEAD_TX                         ((UINT32)(~(COM_BUS_HEAD_RX)) )
#define     COM_BUS_TAIL                            (0x7887)
#define     COM_BUS_TAIL_1                          (COM_BUS_TAIL&0xFF)
#define     COM_BUS_TAIL_2                          ((COM_BUS_TAIL>>8)&0xFF)

#define     COM_BUS_GET_DATA_HEAD_TX                (0xC410C410)

//error code
#define     COM_BUS_HEAD_CHECK_ERROR                (0xEEE0)
#define     COM_BUS_TAIL_CHECK_ERROR                (0xEEE1)
#define     COM_BUS_LENGTH_CHECK_ERROR              (0xEEE2)
#define     COM_BUS_PACKET_OK                       (0x66)


//=================================

typedef enum {
    DEV_UART,
    DEV_SPI,
    DEV_I2C,
    DEV_USB,
    DEV_SDIO,
    DEV_OTG,
    TOTAL_DEV,
    DEV_NULL=0xFF,
} dev_type;

struct st_com_type
{
    dev_type com_type; //0~4: uart0~uart4, 0x11: SPI1, 0xFF: nothing
    UINT8 uart_port;
    kdp_uart_dev_id dev_id;
    kl520_com_flags flags;
    UINT8 *tx_buffer;
    UINT32 *tx_buffer_index;
    UINT8 *rx_buffer;
    UINT32 *rx_buffer_index;

    UINT32 head;
    UINT16 host_number;
    UINT16 cmd;
    UINT16 data_len;
    UINT16 data_start_index; //data end_index = data_start_index + data_len
    UINT32 checksum;
    UINT8 no_head_tail_en; //response bit or not
};


typedef struct kl520_com_user_ops_struct {
    u16     (*packet_analyze)(struct st_com_type *st_com);
    void    (*packet_response_w_tx_buffer)( struct st_com_type *st_com, UINT8 *in_data, UINT16 in_data_legn );
    void    (*parser)(struct st_com_type *st_com);
} kl520_com_user_ops;

// struct st_com_format
// {
// //20200 1 2 1  on going!!

// };

extern struct st_com_type stCom_type;
extern void kl520_com_thread(void);

UINT16 kneron_lwcom_packet_analyze( struct st_com_type *st_com );
void kneron_lwcom_packet_response_w_tx_buffer( struct st_com_type *st_com, UINT8 *in_data, UINT16 in_data_legn );
void kneron_lwcom_packet_response_brief_w_tx_buffer( struct st_com_type *st_com, UINT16 host_number, UINT16 status );
void kneron_lwcom_set_parameter( struct st_com_type *st_com, UINT16 nhost_number, UINT16 cmd );

extern void kl520_com_buf_addr_init(void);
extern void kl520_com_init(kl520_com_flags flags);
extern void kl520_com_bus_init(void);
extern UINT8 kl520_com_response_Done_check(void);
extern UINT8 kl520_com_response( struct st_com_type *st_com );

//-----
extern void kl520_com_initial( kl520_com_flags flags );
extern void kl520_com_reconfig_baud_rate(int rate);
void kl520_com_reg_user_ops(kl520_com_user_ops *ops);

#endif

#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP )
#include "kdp_comm.h"
#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF_USR )
#include "user_com.h"
#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP_USR )
#include "user_comm.h"
#endif
#endif
#endif
