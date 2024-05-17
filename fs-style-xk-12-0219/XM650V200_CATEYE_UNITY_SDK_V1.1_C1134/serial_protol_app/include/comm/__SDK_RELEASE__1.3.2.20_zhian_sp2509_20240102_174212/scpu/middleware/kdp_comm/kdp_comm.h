#ifndef __KDP_COMM_H__
#define __KDP_COMM_H__

//#define BYTE_ALIGN   __attribute__ ((packed))
//#pragma pack(1)
#include "board_kl520.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP )

#include "host_uart_com.h"
#include "drivers.h"
#include "kdp_uart.h"
//#include "user_com_protoco.h"
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
#define	COM_BUS_RESPONSE_OFFESET	(14)		//data size packet
#else
#define	COM_BUS_RESPONSE_OFFESET	(0)
#endif

#if (CFG_COM_UART_MSG == COM_UART_MSG_KDP)
#include "kdp_comm_msg_define.h"
#else
#include "user_comm_msg_define.h"
#endif

//#define v MR_SUCCESS     0     // success   //Lucien
//#define v MR_REJECTED    1     // module rejected this command
//#define v MR_ABORTED     2     // algo aborted
//#define v MR_FAILED4_CAMERA  4 // camera open failed
//#define v MR_FAILED4_UNKNOWNREASON  5 // UNKNOWN_ERROR
//#define MR_FAILED4_INVALIDPARAM  6  // invalid param
//#define MR_FAILED4_NOMEMORY  7      // no enough memory
//#define MR_FAILED4_UNKNOWNUSER  8   // no user enrolled
//#define MR_FAILED4_MAXUSER  9       // exceed maximum user number
//#define MR_FAILED4_FACEENROLLED  10 // this face has been enrolled
//#define MR_FAILED4_LIVENESSCHECK  12// liveness check failed
//#define MR_FAILED4_TIMEOUT  13      // exceed the time limit
//#define x MR_FAILED4_AUTHORIZATION  14// authorization failed
//#define x MR_FAILED4_CAMERAFOV  15    // camera fov test failed
//#define x MR_FAILED4_CAMERAQUA  16    // camera quality test failed
//#define x MR_FAILED4_CAMERASTRU  17   // camera structure test failed
//#define x MR_FAILED4_BOOT_TIMEOUT  18 // boot up timeout
//#define x MR_FAILED4_READ_FILE   19    // read file failed
//#define x MR_FAILED4_WRITE_FILE  20   // write file failed
//#define x MR_FAILED4_NO_ENCRYPT  21   // encrypt must be set

#pragma pack(1)
typedef enum kl520_com_flags_enum {
    KL520_COM_NORMAL = 0,
    KL520_COM_HAS_ADDITIONAL_IO = 1,

} kl520_com_flags;

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
    u8 *parser_buffer;
    u32 parser_cnt;
    UINT32 parser_end;

    UINT16 head;  //syncword
    UINT8 cmd;    //msgid
    UINT16 host_number;//not used
    UINT16 data_len;//size
    UINT16 data_start_index; //data end_index = data_start_index + data_len
    UINT32 checksum; // exp suncWord XOR-alg-result

#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
    UINT8 no_head_tail_en; //response bit or not deshiman no
#endif
};

typedef struct kl520_com_user_ops_struct {
    u16     (*packet_analyze)(struct st_com_type *st_com);
    void    (*parser)(struct st_com_type *st_com);
} kl520_com_user_ops;

extern struct st_com_type stCom_type;

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
extern void kl520_com_reconfig_baud_rate(int rate);
#endif

extern void kl520_com_buf_addr_init(void);
extern void kl520_com_init(kl520_com_flags flags);
extern void kl520_com_initial( kl520_com_flags flags );
extern UINT8 kl520_com_response( struct st_com_type *st_com );
extern UINT8 kl520_com_response_Done_check(void);
UINT16 kneron_lwcom_packet_analyze( struct st_com_type *st_com );
void kl520_com_reg_user_ops(kl520_com_user_ops *ops);
extern void kl520_com_thread(void);
#endif


extern void kl520_com_bus_init(void);

//-----
#pragma pack()
//#endif
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK )
//--------------------PARAER
// error code for cmd parser
#define NO_ERROR        0
#define NO_MATCH        1
#define RSP_NOT_IMPLEMENTED 0xFFFE
#define RSP_UKNOWN_CMD  0xFFFF
#define BAD_CONFIRMATION 2  // CMD_RESET RESPONSE
#define BAD_MODE         1
#define FILE_ERROR       1  // File data transfer error

#endif
#endif
#endif
#endif    //__KDP_COMM_H__
