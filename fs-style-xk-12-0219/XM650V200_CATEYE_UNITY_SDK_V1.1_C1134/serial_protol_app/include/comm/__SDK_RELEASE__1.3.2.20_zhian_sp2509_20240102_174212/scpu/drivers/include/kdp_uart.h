#ifndef __KDP_UART_H__
#define __KDP_UART_H__


#include "types.h"
#include "base.h"
#include "cmsis_os2.h"
#include "Driver_USART.h" //cmsis driver


#define UART_CLOCK                     (30000000UL) //kneron
#define BAUD_1500000                   (UART_CLOCK / 24000000)  //Verification
#define BAUD_921600                    (UART_CLOCK / 14745600)
#define BAUD_460800                    (UART_CLOCK / 7372800)
#define BAUD_230400                    (UART_CLOCK / 3686400)
#define BAUD_115200                    (UART_CLOCK / 1843200)
#define BAUD_57600                     (UART_CLOCK / 921600)
#define BAUD_38400                     (UART_CLOCK / 614400)
#define BAUD_19200                     (UART_CLOCK / 307200)
#define BAUD_14400                     (UART_CLOCK / 230400)
#define BAUD_9600                      (UART_CLOCK / 153600)
#define BAUD_4800                      (UART_CLOCK / 76800)
#define BAUD_2400                      (UART_CLOCK / 38400)
#define BAUD_1200                      (UART_CLOCK / 19200)

typedef enum {
    DRVUART_PORT0=0,
    DRVUART_PORT1=1,
    DRVUART_PORT2=2,
    DRVUART_PORT3=3,
    DRVUART_PORT4=4,
} DRVUART_PORT;

#define SERIAL_THR                     0x00             /*  Transmitter Holding Register(Write).*/
#define SERIAL_RBR                     0x00             /*  Receive Buffer register (Read).*/
#define SERIAL_IER                     0x04             /*  Interrupt Enable register.*/
#define SERIAL_IIR                     0x08             /*  Interrupt Identification register(Read).*/
#define SERIAL_FCR                     0x08             /*  FIFO control register(Write).*/
#define SERIAL_LCR                     0x0C             /*  Line Control register.*/
#define SERIAL_MCR                     0x10             /*  Modem Control Register.*/
#define SERIAL_LSR                     0x14             /*  Line status register(Read) .*/
#define SERIAL_MSR                     0x18             /*  Modem Status register (Read).*/
#define SERIAL_SPR                     0x1C         /*  Scratch pad register */
#define SERIAL_DLL                     0x0          /*  Divisor Register LSB */
#define SERIAL_DLM                     0x4          /*  Divisor Register MSB */
#define SERIAL_PSR                     0x8             /* Prescale Divison Factor */

#define SERIAL_MDR                     0x20
#define SERIAL_ACR                     0x24
#define SERIAL_TXLENL                  0x28
#define SERIAL_TXLENH                  0x2C
#define SERIAL_MRXLENL                 0x30
#define SERIAL_MRXLENH                 0x34
#define SERIAL_PLR                     0x38
#define SERIAL_FMIIR_PIO               0x3C
#define SERIAL_FEATURE                 0x68

/* IER Register */
#define SERIAL_IER_DR                  0x1          /* Data ready Enable */
#define SERIAL_IER_TE                  0x2          /* THR Empty Enable */
#define SERIAL_IER_RLS                 0x4          /* Receive Line Status Enable */
#define SERIAL_IER_MS                  0x8          /* Modem Staus Enable */

/* IIR Register */
#define SERIAL_IIR_NONE                0x1            /* No interrupt pending */
#define SERIAL_IIR_RLS                 0x6            /* Receive Line Status */
#define SERIAL_IIR_DR                  0x4            /* Receive Data Ready */
#define SERIAL_IIR_TIMEOUT             0xc            /* Receive Time Out */
#define SERIAL_IIR_TE                  0x2            /* THR Empty */
#define SERIAL_IIR_MODEM               0x0            /* Modem Status */

/* FCR Register */
#define SERIAL_FCR_FE                  0x1              /* FIFO Enable */
#define SERIAL_FCR_RXFR                0x2              /* Rx FIFO Reset */
#define SERIAL_FCR_TXFR                0x4              /* Tx FIFO Reset */

/* LCR Register */
#define SERIAL_LCR_LEN5                0x0
#define SERIAL_LCR_LEN6                0x1
#define SERIAL_LCR_LEN7                0x2
#define SERIAL_LCR_LEN8                0x3

#define SERIAL_LCR_STOP                0x4
#define SERIAL_LCR_EVEN                0x18          /* Even Parity */
#define SERIAL_LCR_ODD                 0x8          /* Odd Parity */
#define SERIAL_LCR_PE                  0x8            /* Parity Enable */
#define SERIAL_LCR_SETBREAK            0x40             /* Set Break condition */
#define SERIAL_LCR_STICKPARITY         0x20             /* Stick Parity Enable */
#define SERIAL_LCR_DLAB                0x80         /* Divisor Latch Access Bit */

/* LSR Register */
#define SERIAL_LSR_DR                  0x1          /* Data Ready */
#define SERIAL_LSR_OE                  0x2          /* Overrun Error */
#define SERIAL_LSR_PE                  0x4          /* Parity Error */
#define SERIAL_LSR_FE                  0x8          /* Framing Error */
#define SERIAL_LSR_BI                  0x10         /* Break Interrupt */
#define SERIAL_LSR_THRE                0x20         /* THR Empty */
#define SERIAL_LSR_TE                  0x40         /* Transmitte Empty */
#define SERIAL_LSR_DE                  0x80         /* FIFO Data Error */

/* MCR Register */
#define SERIAL_MCR_DTR                 0x1        /* Data Terminal Ready */
#define SERIAL_MCR_RTS                 0x2        /* Request to Send */
#define SERIAL_MCR_OUT1                0x4        /* output    1 */
#define SERIAL_MCR_OUT2                0x8        /* output2 or global interrupt enable */
#define SERIAL_MCR_LPBK                0x10         /* loopback mode */


/* MSR Register */
#define SERIAL_MSR_DELTACTS            0x1        /* Delta CTS */
#define SERIAL_MSR_DELTADSR            0x2        /* Delta DSR */
#define SERIAL_MSR_TERI                0x4        /* Trailing Edge RI */
#define SERIAL_MSR_DELTACD             0x8        /* Delta CD */
#define SERIAL_MSR_CTS                 0x10         /* Clear To Send */
#define SERIAL_MSR_DSR                 0x20         /* Data Set Ready */
#define SERIAL_MSR_RI                  0x40         /* Ring Indicator */
#define SERIAL_MSR_DCD                 0x80         /* Data Carrier Detect */


/* MDR register */
#define SERIAL_MDR_MODE_SEL            0x03
#define SERIAL_MDR_UART                0x0
#define SERIAL_MDR_SIR                 0x1
#define SERIAL_MDR_FIR                 0x2

/* ACR register */
#define SERIAL_ACR_TXENABLE            0x1
#define SERIAL_ACR_RXENABLE            0x2
#define SERIAL_ACR_SET_EOT             0x4

#ifndef PARITY_NONE
#define PARITY_NONE             0
#endif

#ifndef PARITY_ODD
#define PARITY_ODD              1
#endif

#ifndef PARITY_EVEN
#define PARITY_EVEN             2
#endif

#ifndef PARITY_MARK
#define PARITY_MARK             3
#endif

#ifndef PARITY_SPACE
#define PARITY_SPACE            4
#endif

#define BACKSP_KEY 0x08
#define RETURN_KEY 0x0D
#define DELETE_KEY 0x7F
#define BELL       0x07


#define MAX_UART_INST  5  //max uart instance
#define MAX_BUF_LEN   512 //max buffer len for every individual UART port
#define NO_FIFO 0

#define UART_SUCCESS  0
#define UART_FAIL     -1

#define SERIAL_RX_FIFO_COUNT      0x5C 

#define SERIAL_FIFO_DEPTH_REG     0x68

#define SERIAL_FIFO_DEPTH_16B     0x1
#define SERIAL_FIFO_DEPTH_32B     0x2
#define SERIAL_FIFO_DEPTH_64B     0x4
#define SERIAL_FIFO_DEPTH_128B    0x8

#define SERIAL_FIFO_TRIG_LVEL_1   0x0
#define SERIAL_FIFO_TRIG_LVEL_4   0x1
#define SERIAL_FIFO_TRIG_LVEL_8   0x2
#define SERIAL_FIFO_TRIG_LVEL_14  0x3

#define DEFAULT_SYNC_TIMEOUT_CHARS_TIME  512     //for UART_SYNC_MODE: correspond to 512 chars transmission time 

typedef int kdp_uart_hdl_t;
typedef void(*uart_isr_t)(void);
typedef void(*kdp_uart_callback_t)(uint32_t event);

/**
 * UART driver status
 */
typedef enum
{
    KDP_UART_STATUS_OK,     /**< UART driver status : OK*/
    KDP_UART_STATUS_ERROR,  /**< UART driver status : ERROR */
    KDP_UART_STATUS_BUSY    /**< UART driver status : BUSY */
} KDP_UART_DRV_STATUS_t;

/**
 * UART initialization structure
*/
typedef struct KDP_UART_CONFIG
{
    uint32_t baudrate;
    uint8_t  data_bits;
    uint8_t  frame_length;
    uint8_t  stop_bits;
    uint8_t  parity_mode;
    uint8_t  fifo_en;
} KDP_UART_CONFIG_t;

/**
\brief KDP UART Device Driver Capabilities. This capability covers ARM_USART_CAPABILITIES
*/
typedef struct _KDP_USART_CAPABILITIES {
    uint32_t asynchronous : 1;      ///< supports UART (Asynchronous) mode 
    uint32_t synchronous_master : 1;///< supports Synchronous Master mode
    uint32_t synchronous_slave : 1; ///< supports Synchronous Slave mode
    uint32_t serial_irda : 1;       ///< supports SIR (Serial IrDA mode)
    uint32_t fast_irda : 1;         ///< supports FIR (Fast IrDA mode)
    uint32_t flow_control_rts : 1;  ///< RTS Flow Control available
    uint32_t flow_control_cts : 1;  ///< CTS Flow Control available
    uint32_t event_tx_complete : 1; ///< Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE
    uint32_t event_rx_timeout : 1;  ///< Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
    uint32_t rts : 1;               ///< RTS Line: 0=not available, 1=available
    uint32_t cts : 1;               ///< CTS Line: 0=not available, 1=available
    uint32_t dtr : 1;               ///< DTR Line: 0=not available, 1=available
    uint32_t dsr : 1;               ///< DSR Line: 0=not available, 1=available
    uint32_t dcd : 1;               ///< DCD Line: 0=not available, 1=available
    uint32_t ri : 1;                ///< RI Line: 0=not available, 1=available
    uint32_t event_cts : 1;         ///< Signal CTS change event: \ref ARM_USART_EVENT_CTS
    uint32_t event_dsr : 1;         ///< Signal DSR change event: \ref ARM_USART_EVENT_DSR
    uint32_t event_dcd : 1;         ///< Signal DCD change event: \ref ARM_USART_EVENT_DCD
    uint32_t event_ri : 1;          ///< Signal RI change event: \ref ARM_USART_EVENT_RI
} KDP_USART_CAPABILITIES;

// UART flags

#define     UART_INITIALIZED          (1 << 0)
#define     UART_POWERED              (1 << 1)
#define     UART_BASIC_CONFIGURED     (1 << 2)
#define     UART_FIFO_RX_CONFIGURED   (1 << 3)
#define     UART_FIFO_TX_CONFIGURED   (1 << 4)
#define     UART_TX_ENABLED           (1 << 5)
#define     UART_RX_ENABLED           (1 << 6)
#define     UART_LOOPBACK_ENABLED     (1 << 7)

#define     UART_MODE_ASYN_RX  (1 << 0)
#define     UART_MODE_ASYN_TX  (1 << 1)
#define     UART_MODE_SYNC_RX  (1 << 2)
#define     UART_MODE_SYNC_TX  (1 << 3)
#define     SIR_MODE_ASYN      (1 << 4)
#define     FIR_MODE_ASYN      (1 << 5)


// UART Transfer Information (Run-Time)
typedef struct UART_TRANSFER_INFO
{
    volatile uint32_t                rx_num;        // Total number of data to be received
    volatile uint32_t                tx_num;        // Total number of data to be send
    volatile uint8_t                 *rx_buf;       // Pointer to in data buffer
    volatile uint8_t                 *tx_buf;       // Pointer to out data buffer
    volatile uint32_t                rx_cnt;        // Number of data received
    volatile uint32_t                tx_cnt;        // Number of data sent
    volatile uint32_t                write_idx;     // Write index
    volatile uint32_t                read_idx;      // Read index
} UART_TRANSFER_INFO_t;

typedef struct
{
    volatile uint8_t tx_busy;             // Transmitter busy flag
    volatile uint8_t rx_busy;             // Receiver busy flag
    uint8_t tx_underflow;                 // Transmit data underflow detected (cleared on start of next send operation)
    uint8_t rx_overflow;                  // Receive data overflow detected (cleared on start of next receive operation)
    uint8_t rx_break;                     // Break detected on receive (cleared on start of next receive operation)
    uint8_t rx_framing_error;             // Framing error detected on receive (cleared on start of next receive operation)
    uint8_t rx_parity_error;              // Parity error detected on receive (cleared on start of next receive operation)
} UART_STATUS_t;

// UART Information (Run-Time)
typedef struct
{
    ARM_USART_SignalEvent_t cb_event;           // Event callback
    UART_STATUS_t           status;             // Status flags
    UART_TRANSFER_INFO_t    xfer;               // Transfer information
    uint32_t                flags;              // UART driver flags: UART_FLAG_T
    uint32_t                mode;               // UART mode
} UART_INFO_T;


// UART Resources definitions
typedef struct
{
    uint32_t               irq_num;                    // UART TX IRQ Number
    uart_isr_t             isr;                        // ISR route
    uint32_t               fifo_depth;                 //16/32/64/128 depth, set by UART_CTRL_CONFIG
    uint32_t               tx_fifo_threshold;          // FIFO tx trigger threshold
    uint32_t               rx_fifo_threshold;          // FIFO rx trigger threshold
    uint32_t               fifo_len;                   // FIFO tx buffer len
    uint32_t               clock;                      //clock
    uint32_t               hw_base;                    // hardware base address
} UART_RESOURCES_T;

typedef enum {
    UART_UNINIT,
    UART_INIT_DONE,
    UART_WORKING,
    UART_CLOSED
}kdp_uart_drv_state_t;

typedef enum {
    UART0_DEV,
    UART1_DEV,
    UART2_DEV,
    UART3_DEV,
    UART4_DEV,
    TOTAL_UART_DEV
} kdp_uart_dev_id;

typedef struct {
    bool     bEnFifo;
    uint8_t  fifo_trig_level;
} kdp_uart_fifo_cfg_t;

typedef enum {
    UART_CTRL_CONFIG,      //param: KDP_UART_CONFIG_t
    UART_CTRL_FIFO_RX,     //param: kdp_uart_fifo_cfg_t
    UART_CTRL_FIFO_TX,     //param: kdp_uart_fifo_cfg_t
    UART_CTRL_LOOPBACK,
    UART_CTRL_TX_EN,
    UART_CTRL_RX_EN,
    UART_CTRL_RTS_EN,
    UART_CTRL_DTR_EN,
    UART_CTRL_ABORT_TX,
    UART_CTRL_ABORT_RX,
    UART_CTRL_TIMEOUT_RX,
    UART_CTRL_TIMEOUT_TX
}kdp_uart_ctrl_t;

/* driver instance handle */
typedef struct {
    uint32_t                 uart_port;
    kdp_uart_drv_state_t     state;
    KDP_USART_CAPABILITIES   *pCap;
    KDP_UART_CONFIG_t        config;
    UART_INFO_T              info;
    UART_RESOURCES_T         res;
    int32_t                  nTimeOutTx;       //Tx timeout (ms) for UART_SYNC_MODE
    int32_t                  nTimeOutRx;       //Rx timeout (ms) for UART_SYNC_MODE
    uint32_t                 iir;              //store IIR register value (IIR register will be reset once it is 
                                               //read once, need to store for further process)
} kdp_driver_hdl_t;

typedef struct {
    int8_t             total_open_uarts;
    bool               active_dev[TOTAL_UART_DEV];
    kdp_driver_hdl_t   *uart_dev[MAX_UART_INST];

} kdp_uart_drv_ctx_t;

/******** kdp_uart_api_sts_t *********/
typedef uint32_t kdp_uart_api_sts_t;

#define UART_API_RETURN_SUCCESS     0
#define UART_API_NOT_POWRERED       (1<<0)
#define UART_API_TX_BUSY            (1<<1)
#define UART_API_RX_BUSY            (1<<2)
#define UART_API_INVALID_PARAM      (1<<3)
#define UART_API_ERROR              (1<<4)
#define UART_API_TIMEOUT            (1<<5)

/*************************************/

/* Init the UART device driver, it shall be called once in lifecycle
*/
void kdp_uart_init(void);

/*
  Open one UART port
Input:
  com_port: UART port id
  cb: callback function

Output:
  return device handle: >=0 means success; -1 means open fail
*/
kdp_uart_hdl_t kdp_uart_open(uint8_t com_port, uint32_t mode, kdp_uart_callback_t cb);

/*
 Query capability
Input:
 handle: driver handle

Output:
 capability of the UART port
*/

KDP_USART_CAPABILITIES* kdp_uart_get_capability(kdp_uart_hdl_t hanle);

/*
Set control for the device
Input:
    handle: device handle
    prop: control enumeration
    val: pointer to control value/structure
return:
    error code
*/
int32_t kdp_uart_control(kdp_uart_hdl_t handle, kdp_uart_ctrl_t prop, uint8_t * val);

/*
Query driver status
Input:
    handle: driver handle

Output:
    driver status
*/
UART_STATUS_t* kdp_uart_get_status(kdp_uart_hdl_t hanle);

/*
Write data to Mozart device, such as command, parameters, but not suitable for chunk data
Input:
    hdl: device handle
    buf: data buffer
    len: data buffer length
return:
    driver status
*/
kdp_uart_api_sts_t kdp_uart_write(kdp_uart_hdl_t hdl, uint8_t *buf, uint32_t len);

/*
Read data from Mozart device
Input:
    handle: device handle
    buf: data buffer
    len: data buffer length
return:
    driver status
*/
kdp_uart_api_sts_t kdp_uart_read(kdp_uart_hdl_t handle, uint8_t *buf, uint32_t len);

/*
Power control
Input:
    handle: driver handle
    pwr_set: power status expected to be set

Output:
    success or fail
*/
int32_t kdp_uart_power_control(kdp_uart_hdl_t handle, ARM_POWER_STATE pwr_st);


/* close the device
Input:
    handle: device handle
return:
    0 - success; -1 - failure
*/
int32_t kdp_uart_close(kdp_uart_hdl_t handle);

/* get char number in Rx buffer 
Input:
    handle: device handle
Return:
    Received bytes 
*/
uint32_t kdp_uart_GetRxCount(kdp_uart_hdl_t handle);

/* get char number in Tx buffer 
Input:
    handle: device handle
Return:
    Sent bytes
*/
uint32_t kdp_uart_GetTxCount(kdp_uart_hdl_t handle);

uint32_t kdp_uart_GetRxBufSize(kdp_uart_hdl_t handle);
uint32_t kdp_uart_GetWriteIndex(kdp_uart_hdl_t handle);
uint32_t kdp_uart_GetReadIndex(kdp_uart_hdl_t handle);
uint32_t kdp_uart_SetWriteIndex(kdp_uart_hdl_t handle, uint32_t index);
uint32_t kdp_uart_SetReadIndex(kdp_uart_hdl_t handle, uint32_t index);

char kdp_getchar(DRVUART_PORT port_no);
int kdp_gets(DRVUART_PORT port_no, char *buf);
void kdp_uart_rx_busy_clear(kdp_uart_hdl_t handle);
extern void kdp_uart_print_register(uint8_t port_no);

#endif  //__KDP_UART_H__
