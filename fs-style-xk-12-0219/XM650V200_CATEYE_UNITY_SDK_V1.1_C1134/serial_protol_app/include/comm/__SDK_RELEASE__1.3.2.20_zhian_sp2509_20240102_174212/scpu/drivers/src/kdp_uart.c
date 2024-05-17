#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "kdp_uart.h"
#include "kneron_mozart.h"
#include "scu_extreg.h"
#include "io.h"
#include "dbg.h"
#include "pinmux.h"

#define IIR_CODE_MASK 0xf
#define SERIAL_IIR_TX_FIFO_FULL  0x10

/***********************************************************************************
              Global variables
************************************************************************************/
extern UINT32 UART_PORT[5];
void UART0_ISR(void);
void UART1_ISR(void);
void UART2_ISR(void);
void UART3_ISR(void);
void UART4_ISR(void);

/***********************************************************************************
              local variables
************************************************************************************/
static kdp_uart_drv_ctx_t gDrvCtx;

static KDP_USART_CAPABILITIES DriverCapabilities =
{
  1,      ///< supports UART (Asynchronous) mode 
  1,      ///< supports Synchronous Master mode
  1,      ///< supports Synchronous Slave mode
  0,      ///< supports SIR (Serial IrDA mode)
  0,      ///< supports FIR (Fast IrDA mode)
  0,      ///< RTS Flow Control available
  0,      ///< CTS Flow Control available
  1,      ///< Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE
  1,      ///< Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
  0,      ///< RTS Line: 0=not available, 1=available
  0,      ///< CTS Line: 0=not available, 1=available
  0,      ///< DTR Line: 0=not available, 1=available
  0,      ///< DSR Line: 0=not available, 1=available
  0,      ///< DCD Line: 0=not available, 1=available
  0,      ///< RI Line: 0=not available, 1=available
  0,      ///< Signal CTS change event: \ref ARM_USART_EVENT_CTS
  0,      ///< Signal DSR change event: \ref ARM_USART_EVENT_DSR
  0,      ///< Signal DCD change event: \ref ARM_USART_EVENT_DCD
  0      ///< Signal RI change event: \ref ARM_USART_EVENT_RI
};

IRQn_Type gUartIRQTbl[5] = {
    UART_FTUART010_0_IRQ,       //UART0
    UART_FTUART010_1_IRQ,       //UART1
    UART_FTUART010_1_1_IRQ,     //UART2
    UART_FTUART010_1_2_IRQ,     //UART3
    UART_FTUART010_1_3_IRQ      //UART4
};

uart_isr_t gUartISRs[5] = {
    UART0_ISR,
    UART1_ISR,
    UART2_ISR,
    UART3_ISR,
    UART4_ISR
};

uint32_t gUartClk[5] = {
    UART_CLOCK,
    UART_CLOCK_2,
    UART_CLOCK_2,
    UART_CLOCK,
    UART_CLOCK
};

BOOL static gDriverInitialized = FALSE;
UINT32 UART_PORT[5]={UART_FTUART010_0_PA_BASE, UART_FTUART010_1_PA_BASE, UART_FTUART010_1_1_PA_BASE, UART_FTUART010_1_2_PA_BASE, UART_FTUART010_1_3_PA_BASE};


UINT32 _get_uart_status(DRVUART_PORT port_no)
{
    UINT32 status;
    status=inw(UART_PORT[port_no]+SERIAL_LSR);
    return status;
}

UINT32 IsDataReady(UINT32 status)
{
    if((status & SERIAL_IER_DR)==SERIAL_IER_DR)
        return TRUE;
    else
        return FALSE;

}

UINT32 _is_thr_empty(UINT32 status)
{
    //if (((status & SERIAL_LSR_THRE)==SERIAL_LSR_THRE) && ((status & SERIAL_LSR_TE)==SERIAL_LSR_TE))
    if ((status & SERIAL_LSR_THRE)==SERIAL_LSR_THRE)
        return TRUE;
    else
        return FALSE;
}

void _check_rx_status(DRVUART_PORT port_no)
{
    UINT32 Status;
    do {
        Status = _get_uart_status(port_no);
        if(IsDataReady(Status)) break;
        else osDelay(20);
    } while (1);    // wait until Rx ready
}

void _check_tx_status(DRVUART_PORT port_no)
{
    UINT32 Status;
    do
    {
        Status = _get_uart_status(port_no);
    }while (!_is_thr_empty(Status));    // wait until Tx ready
}

char kdp_getchar(DRVUART_PORT port_no)
{
    char Ch;

    _check_rx_status(port_no);
    Ch = inw(UART_PORT[port_no]+SERIAL_RBR);
    return (Ch);
}

void kdp_putchar(DRVUART_PORT port_no, char Ch)
{
    if(Ch!='\0')
    {
        _check_tx_status(port_no);
        outw(UART_PORT[port_no]+SERIAL_THR,Ch);
    }

    if (Ch == '\n')
    {
        _check_tx_status(port_no);
        outw(UART_PORT[port_no]+SERIAL_THR,'\r');
    }
}

void kdp_putstr(DRVUART_PORT port_no, char *str)
{
    char *cp;
    for(cp = str; *cp != 0; cp++)
        kdp_putchar(port_no, *cp);
}

char kdp_get_serial_char(DRVUART_PORT port_no)
{
    char Ch;
    UINT32 status;

       do
    {
         status=inw(UART_PORT[port_no]+SERIAL_LSR);
    }
    while (!((status & SERIAL_LSR_DR)==SERIAL_LSR_DR));    // wait until Rx ready
    Ch = inw(UART_PORT[port_no] + SERIAL_RBR);
    return (Ch);
}
void kdp_serial_init (DRVUART_PORT port_no, UINT32 baudrate, UINT32 parity,UINT32 num,UINT32 len, UINT32 interruptMode)
{
    UINT32 lcr;

    lcr = inw(UART_PORT[port_no] + SERIAL_LCR) & ~SERIAL_LCR_DLAB;
    // kdp_printf("uart fifo depth=%u\n", inw(UART_PORT[port_no] + SERIAL_FEATURE)  & 0xF);
    /* Set DLAB=1 */
    outw(UART_PORT[port_no] + SERIAL_LCR,SERIAL_LCR_DLAB);
    /* Set baud rate */
    outw(UART_PORT[port_no] + SERIAL_DLM, ((baudrate & 0xff00) >> 8)); //ycmo090930
    outw(UART_PORT[port_no] + SERIAL_DLL, (baudrate & 0xff));
    // outw(UART_PORT[port_no] + SERIAL_DLM, 0x0); //ycmo090930 0x0
    // outw(UART_PORT[port_no] + SERIAL_DLL, 0x10);  //legend: 0xD
    //while(1);
    //clear orignal parity setting
    lcr &= 0xc0;

    switch (parity)
    {
        case PARITY_NONE:
            //do nothing
            break;
        case PARITY_ODD:
            lcr|=SERIAL_LCR_ODD;
                break;
        case PARITY_EVEN:
            lcr|=SERIAL_LCR_EVEN;
            break;
        case PARITY_MARK:
            lcr|=(SERIAL_LCR_STICKPARITY|SERIAL_LCR_ODD);
            break;
        case PARITY_SPACE:
            lcr|=(SERIAL_LCR_STICKPARITY|SERIAL_LCR_EVEN);
            break;

        default:
            break;
    }

    if(num==2)
        lcr|=SERIAL_LCR_STOP;

    len-=5;

    lcr|=len;

    outw(UART_PORT[port_no]+SERIAL_LCR,lcr);
    if (1 == interruptMode)
        outw(UART_PORT[port_no] + SERIAL_FCR, SERIAL_FCR_FE);
}
void kdp_set_serial_int(DRVUART_PORT port_no, UINT32 IntMask)
{
    outw(UART_PORT[port_no] + SERIAL_IER, IntMask);
}
int kdp_gets(DRVUART_PORT port_no, char *buf)
{
    char    *cp;
    char    data;
    UINT32  count;
    count = 0;
    cp = buf;

    do
    {
        data = kdp_getchar(port_no);

        switch(data)
        {
            case RETURN_KEY:
                if(count < 256)
                {
                    *cp = '\0';
                    kdp_putchar(port_no, '\n');
                }
                break;
            case BACKSP_KEY:
            case DELETE_KEY:
                if(count)
                {
                    count--;
                    *(--cp) = '\0';
                    kdp_putstr(port_no, "\b \b");
                }
                break;
            default:
                if( data > 0x1F && data < 0x7F && count < 256)
                {
                    *cp = (char)data;
                    cp++;
                    count++;
                    kdp_putchar(port_no, data);
                }
                break;
        }
    } while(data != RETURN_KEY);

  return (count);
}
/***********************************************************************************
              local functions
************************************************************************************/
static int32_t kdp_uart_get_default_timeout(uint32_t baud)
{
    int32_t timeout;
    switch (baud)
    {
    case BAUD_921600:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 9216;
        break;
    }
    case BAUD_460800:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 4608;
        break;
    }
    case BAUD_115200:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 1152;
        break;
    }
    case BAUD_57600:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 576;
        break;
    }
    case BAUD_38400:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 384;
        break;
    }
    case BAUD_19200:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 192;
        break;
    }
    case BAUD_14400:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 144;
        break;
    }
    case BAUD_9600:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 96;
        break;
    }
    case BAUD_4800:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 48;
        break;
    }
    case BAUD_2400:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 240;
        break;
    }
    case BAUD_1200:
    {
        timeout = (DEFAULT_SYNC_TIMEOUT_CHARS_TIME * 100) / 12;
        break;
    }
    default:
        timeout = DEFAULT_SYNC_TIMEOUT_CHARS_TIME;
    }
    return timeout;
}

static kdp_driver_hdl_t * kdp_uart_get_drv_hdl(uint16_t port)
{
    if (port >= MAX_UART_INST)
    {
        //dbg_msg("Error: invalid port number\n");
        return NULL;
    }
    return gDrvCtx.uart_dev[port];
}

/* calculate fifo config: depth and trigger level
   pCfg->bEnFifo and pCfg->fifo_depth was set by client before calling in
*/
static int32_t kdp_calculate_fifo_cfg(kdp_driver_hdl_t *const pDrv, uint32_t val, kdp_uart_fifo_cfg_t *pCfg)
{
    #define TRIGGER_LEVEL_MAX_IS_LEVEL_8 (NO)
    int32_t depth;

    if (pCfg == NULL)
    {
        //dbg_msg("Error: invalid parameter pCfg\n");
        return UART_FAIL;
    }

    if (pCfg->bEnFifo == FALSE)
    {
        pCfg->fifo_trig_level = SERIAL_FIFO_TRIG_LVEL_1;
        return UART_SUCCESS;
    }

    depth = pDrv->res.fifo_depth;

    if (val <= depth * 4)
    {
        pCfg->fifo_trig_level = SERIAL_FIFO_TRIG_LVEL_1;
    }
    else if ((val > depth * 4) && (val <= depth * 8))
    {
        pCfg->fifo_trig_level = SERIAL_FIFO_TRIG_LVEL_4;
    }
    #if TRIGGER_LEVEL_MAX_IS_LEVEL_8 == YES
    else
    {
        pCfg->fifo_trig_level = SERIAL_FIFO_TRIG_LVEL_8;
    }
    #else
    else if ((val > depth * 8) && (val <= depth * 14))
    {
        pCfg->fifo_trig_level = SERIAL_FIFO_TRIG_LVEL_8;
    }
    else if (val > depth * 14)
    {
        pCfg->fifo_trig_level = SERIAL_FIFO_TRIG_LVEL_14;
    }
    #endif
    return UART_SUCCESS;
}

/**
  \fn          void UART_IRQHandler (UART_RESOURCES  *uart)
  \brief       UART Interrupt handler.
  \param[in]   uart  Pointer to UART resources
*/
static void UART_TX_ISR(kdp_driver_hdl_t *const uart)
{
    int16_t  tx;
    UINT32   status;
    uint32_t ww;
    uint16_t port_no;
    bool     bFifo;

    ww = uart->iir;
    bFifo = ((ww & 0xc0) != 0) ? TRUE : FALSE;
    port_no = uart->uart_port;

    if (uart->info.xfer.tx_num > uart->info.xfer.tx_cnt)
    {
        if (bFifo == FALSE)     //NO FIFO
        {
            do
            {
                status = inw(UART_PORT[port_no] + SERIAL_LSR);
            } while (!((status & SERIAL_LSR_THRE) == SERIAL_LSR_THRE));    // wait until Tx ready
            outw(UART_PORT[port_no] + SERIAL_THR, *(uart->info.xfer.tx_buf++));
            uart->info.xfer.tx_cnt++;

        }
        else     //FIFO mode
        {
            do
            {
                status = inw(UART_PORT[port_no] + SERIAL_LSR);
            } while (!((status & SERIAL_LSR_THRE) == SERIAL_LSR_THRE));    // wait until Tx ready

            tx = uart->info.xfer.tx_num - uart->info.xfer.tx_cnt;
            if (tx <= 0) tx = 0;

            if (tx > uart->res.fifo_len) {
                tx = 16;
            }

            while (tx > 0)
            {
                uart->info.xfer.tx_cnt++;
                outw(UART_PORT[port_no] + SERIAL_THR, *(uart->info.xfer.tx_buf++));
                tx--;
            }

        }

    }

    if (uart->info.xfer.tx_num == uart->info.xfer.tx_cnt)
    {
        // need to add: determine if TX fifo is empty
        status = inw(UART_PORT[port_no] + SERIAL_LSR);
        if ((status & SERIAL_LSR_THRE) == SERIAL_LSR_THRE)    //TX empty to make sure FIFO data to tranmit shift
        {
            uint32_t ww = inw(UART_PORT[port_no] + SERIAL_IER);
            ww &= ~SERIAL_IER_TE;                         // disable Tx empty interrupt
            outw(UART_PORT[port_no] + SERIAL_IER, ww);

            // Clear TX busy flag
            uart->info.status.tx_busy = 0;
            if (uart->info.cb_event) uart->info.cb_event(ARM_USART_EVENT_SEND_COMPLETE);
        }
    }
}

#if 0
static void UART_RX_ISR(kdp_driver_hdl_t *const uart)
{
    uint32_t ww;
    uint16_t port_no;

    ww = uart->iir;

    port_no = uart->uart_port;

    if (((ww & SERIAL_IIR_DR) == SERIAL_IIR_DR) || ((ww & SERIAL_IIR_TIMEOUT) == SERIAL_IIR_TIMEOUT))
    {

        if ((ww & 0xc0) == 0)   //non FIFO mode
        {
            *(uart->info.xfer.rx_buf++) = (uint8_t)kdp_get_serial_char((DRVUART_PORT)port_no);
            uart->info.xfer.rx_cnt++;

            if (uart->info.xfer.rx_cnt == uart->info.xfer.rx_num)
            {
                ww = inw(UART_PORT[port_no] + SERIAL_IER);
                ww &= ~SERIAL_IER_DR;                         // disable Rx empty interrupt
                outw(UART_PORT[port_no] + SERIAL_IER, ww);

                // Clear RX busy flag and set receive transfer complete event
                uart->info.status.rx_busy = 0;
                if (uart->info.cb_event) uart->info.cb_event(ARM_USART_EVENT_RECEIVE_COMPLETE);
            }
        }
        else   //FIFO mode
        {
            while (uart->info.xfer.rx_cnt < uart->info.xfer.rx_num)
            {
                /* Read the data from FIFO buffer */

                ww = inw(UART_PORT[port_no] + SERIAL_LSR);
                if ((ww & SERIAL_LSR_DR) == SERIAL_LSR_DR)
                {
                    *(uart->info.xfer.rx_buf++) = (uint8_t)inw(UART_PORT[port_no] + SERIAL_RBR);
                    uart->info.xfer.rx_cnt++;

                    if (uart->info.xfer.rx_cnt == uart->info.xfer.rx_num)
                    {
                        ww = inw(UART_PORT[port_no] + SERIAL_IER);
                        ww &= ~SERIAL_IER_DR;                         // disable Rx empty interrupt
                        outw(UART_PORT[port_no] + SERIAL_IER, ww);

                        // Clear RX busy flag and set receive transfer complete event
                        uart->info.status.rx_busy = 0;
                        if (uart->info.cb_event) uart->info.cb_event(ARM_USART_EVENT_RECEIVE_COMPLETE);
                    }
                }
                else
                {
                    ww = uart->iir;
                    if ((ww & SERIAL_IIR_TIMEOUT) == SERIAL_IIR_TIMEOUT)   // means end of frame
                    {
                        ww = inw(UART_PORT[port_no] + SERIAL_IER);
                        ww &= ~SERIAL_IER_DR;                         // disable Rx empty interrupt
                        outw(UART_PORT[port_no] + SERIAL_IER, ww);

                        // Clear RX busy flag and set receive transfer complete event
                        uart->info.status.rx_busy = 0;
                        if (uart->info.cb_event) uart->info.cb_event(ARM_USART_EVENT_RX_TIMEOUT);

                    }
                    break;
                }
            }
        }
    }
}
#endif
static void UART_RX_ISR_new(kdp_driver_hdl_t *const uart)
{
    uint32_t ww;
    uint16_t port_no;
    uint8_t lsr;
    port_no = uart->uart_port;
    while(1)
    {
        ww = (uint8_t)inw(UART_PORT[port_no] + SERIAL_IIR);

        if( (ww & SERIAL_IIR_RLS) == SERIAL_IIR_RLS ) //iir 0x06
        {
            lsr = inw(UART_PORT[port_no] + SERIAL_LSR);
            if( (lsr&0x01) == 0x01 )
            {
                uart->info.xfer.rx_buf[uart->info.xfer.write_idx] = (uint8_t)inw(UART_PORT[port_no] + SERIAL_RBR);
                uart->info.xfer.write_idx++;
                if(uart->info.xfer.write_idx >= uart->info.xfer.rx_num) uart->info.xfer.write_idx = 0;
                uart->info.xfer.rx_cnt++;
            }
        }

        if( (ww & 0x0F /*SERIAL_IIR_DR*/) == SERIAL_IIR_DR ) //iir 0x04
        {

            UINT8 aa = inw(UART_PORT[port_no] + SERIAL_LSR);
            {
                uart->info.xfer.rx_buf[uart->info.xfer.write_idx] = (uint8_t)inw(UART_PORT[port_no] + SERIAL_RBR);
                uart->info.xfer.write_idx++;
                if(uart->info.xfer.write_idx >= uart->info.xfer.rx_num) uart->info.xfer.write_idx = 0;
                uart->info.xfer.rx_cnt++;
            }
            uart->info.status.rx_busy = 1;
            continue;
        }

        if((ww & 0x0F /*SERIAL_IIR_TIMEOUT*/ ) == SERIAL_IIR_TIMEOUT)    //to  0x0C
        {

            UINT8 cc = inw(UART_PORT[port_no] + SERIAL_LSR);
            uart->info.xfer.rx_buf[uart->info.xfer.write_idx] = (uint8_t)inw(UART_PORT[port_no] + SERIAL_RBR);
            uart->info.xfer.write_idx++;
            if(uart->info.xfer.write_idx >= uart->info.xfer.rx_num) uart->info.xfer.write_idx = 0;
            uart->info.xfer.rx_cnt++;

            break;
        }
        else
        {

            //do nothing
            break;
        }
    }




}

/***********************************************************************************
              global functions
************************************************************************************/

void UART_ISR(uint8_t port_no)
{
    kdp_driver_hdl_t *pHdl = kdp_uart_get_drv_hdl(port_no);

    uint32_t ww = inw(UART_PORT[port_no] + SERIAL_IIR);

    pHdl->iir = ww;

    if ((ww & IIR_CODE_MASK) == SERIAL_IIR_RLS)   // errors: overrun/parity/framing/break
    {
        ww = inw(UART_PORT[port_no] + SERIAL_LSR);   //Read LSR to reset interrupt

        if (ww & SERIAL_LSR_OE)
        {
            pHdl->info.status.rx_overflow = 1;
        }

        if (ww & SERIAL_LSR_PE)
        {
            pHdl->info.status.rx_parity_error = 1;
        }

        if (ww & SERIAL_LSR_BI)
        {
            pHdl->info.status.rx_break = 1;
        }

        if (ww & SERIAL_LSR_FE)
        {
            pHdl->info.status.rx_framing_error = 1;
        }

        if (pHdl->info.status.rx_busy)
        {
//            UART_RX_ISR(pHdl);
            UART_RX_ISR_new(pHdl);
        }

        if (pHdl->info.status.tx_busy)
        {
            UART_TX_ISR(pHdl);
        }

    }
    else if (((ww & IIR_CODE_MASK) == SERIAL_IIR_DR)            // Rx data ready in FIFO
        || ((ww & IIR_CODE_MASK) == SERIAL_IIR_TIMEOUT))       // Character Reception Timeout 
    {
        if (pHdl->info.status.rx_busy)
        {
//            UART_RX_ISR(pHdl);
            UART_RX_ISR_new(pHdl);
        }
        else
        {
            ww = inw(UART_PORT[port_no] + SERIAL_RBR);   //Read RBR to reset interrupt
        }

        if (pHdl->info.status.tx_busy)
        {
            UART_TX_ISR(pHdl);
        }

    }
    else if ((ww & IIR_CODE_MASK) == SERIAL_IIR_TE)  // Transmitter Holding Register Empty
    {
        if (pHdl->info.status.tx_busy)
        {
            UART_TX_ISR(pHdl);
        }

        if (pHdl->info.status.rx_busy)
        {
//            UART_RX_ISR(pHdl);
            UART_RX_ISR_new(pHdl);
        }

    }
    else
    {
        if (pHdl->info.status.tx_busy)
        {
            UART_TX_ISR(pHdl);
        }
    }

    NVIC_ClearPendingIRQ((IRQn_Type)pHdl->res.irq_num);
    NVIC_EnableIRQ((IRQn_Type)pHdl->res.irq_num);

}


void UART0_ISR(void)
{
    UART_ISR(0);
}

void UART1_ISR(void)
{
    UART_ISR(1);
}

void UART2_ISR(void)
{
    UART_ISR(2);
}

void UART3_ISR(void)
{
    UART_ISR(3);
}

void UART4_ISR(void)
{
    UART_ISR(4);
}


/* Init the UART device driver, it shall be called once in lifecycle
*/
void kdp_uart_init(void)
{
    if (gDriverInitialized == FALSE)
    {
        memset(&gDrvCtx, 0, sizeof(gDrvCtx));
        gDriverInitialized = TRUE;
    }
}


/*
  Open one UART port
Input:
  com_port: UART port id
  mode: that is a combination of Tx mode|Rx mode, make sure set both, such as UART_MODE_ASYN_RX|UART_MODE_SYNC_TX
  cb: callback function

Output:
  return device handle: >=0 means success; -1 means open fail
*/
kdp_uart_hdl_t kdp_uart_open(uint8_t com_port, uint32_t mode, kdp_uart_callback_t cb)
{
    uint32_t ww;

    if (com_port >= TOTAL_UART_DEV) 
    {
        //dbg_msg("Invalid com_port\n");
        return UART_FAIL;
    }
/*
    if (gDrvCtx.active_dev[com_port] == TRUE)
    {
        //dbg_msg("This UART device has been opened\n");
        return UART_FAIL;
    }
*/
    kdp_driver_hdl_t * pDrv = (kdp_driver_hdl_t *)malloc(sizeof(kdp_driver_hdl_t));
    if (pDrv == NULL) {
        err_msg("Error: memory alloc failed\n");
        return UART_FAIL;
    }

//    if (((mode & UART_MODE_ASYN_TX) || (mode & UART_MODE_ASYN_RX)) && (cb == NULL))
//    {
//        err_msg("Error: Async mode needs callback function\n");
//        return UART_FAIL;
//    }

    gDrvCtx.uart_dev[com_port] = pDrv;

    pDrv->uart_port = com_port;
    pDrv->state = UART_UNINIT;
    // now use a common capbilities for all UARTs, may introduce an array if diff UART has diff capbilities
    pDrv->pCap = &DriverCapabilities;      
    pDrv->info.cb_event = cb;
    pDrv->info.mode = mode;
    pDrv->info.status.tx_busy = 0;
    pDrv->info.status.rx_busy = 0;
    pDrv->info.status.tx_underflow = 0;
    pDrv->info.status.rx_overflow = 0;
    pDrv->info.status.rx_break = 0;
    pDrv->info.status.rx_framing_error = 0;
    pDrv->info.status.rx_parity_error = 0;

    pDrv->info.xfer.tx_num = 0;
    pDrv->info.xfer.rx_num = 0;
    pDrv->info.xfer.tx_cnt = 0;
    pDrv->info.xfer.rx_cnt = 0;
    pDrv->info.xfer.write_idx = 0;
    pDrv->info.xfer.read_idx = 0;

    pDrv->res.irq_num = gUartIRQTbl[com_port];
    pDrv->res.isr = gUartISRs[com_port];

    ww = inw(UART_PORT[com_port] + SERIAL_FIFO_DEPTH_REG);
    ww &= 0x0f;
    pDrv->res.fifo_depth = ww;

    pDrv->res.fifo_len = 16 * ww;

    pDrv->res.hw_base = UART_PORT[com_port];
    pDrv->res.clock = gUartClk[com_port];

    pDrv->nTimeOutRx = DEFAULT_SYNC_TIMEOUT_CHARS_TIME;   //suppose default baud = 115200
    pDrv->nTimeOutTx = DEFAULT_SYNC_TIMEOUT_CHARS_TIME;   //suppose default baud = 115200

    gDrvCtx.total_open_uarts++;
    gDrvCtx.active_dev[com_port] = TRUE;

    pDrv->info.flags |= UART_INITIALIZED;

    return (kdp_uart_hdl_t)com_port;
}

/* close the device
Input:
   handle: device handle
return:
   0 - success; -1 - failure
*/
int32_t kdp_uart_close(kdp_uart_hdl_t handle)
{
    uint32_t com_port = handle;

    if (com_port >= TOTAL_UART_DEV) {
        //dbg_msg("Invalid parameter\n");
        return UART_FAIL;
    }

    if (gDrvCtx.active_dev[com_port] == FALSE)
    {
        //dbg_msg("This UART device has been closed\n");
        return UART_FAIL;
    }

    kdp_driver_hdl_t *pDrv = gDrvCtx.uart_dev[com_port];

    if ((pDrv->info.flags & UART_POWERED) == 0)
    {
        // UART is not powered 
        return UART_API_NOT_POWRERED;
    }

    if (pDrv->info.flags == 0) {
        // Driver not initialized
        return UART_SUCCESS;
    }

    // Reset USART status flags
    pDrv->info.flags = 0;

    free(gDrvCtx.uart_dev[com_port]);
    gDrvCtx.uart_dev[com_port] = NULL;
    gDrvCtx.active_dev[com_port] = FALSE;
    gDrvCtx.total_open_uarts--;
    return UART_SUCCESS;
}

/*
 Query capability
Input:
 handle: driver handle

Output:
 capability of the UART port
*/

KDP_USART_CAPABILITIES * kdp_uart_get_capability(kdp_uart_hdl_t handle)
{
    uint32_t com_port = handle;

    if (com_port >= TOTAL_UART_DEV) {
        //dbg_msg("Invalid parameter\n");
        return NULL;
    }

    if (gDrvCtx.active_dev[com_port] == FALSE)
    {
        //dbg_msg("This UART device is not active\n");
        return NULL;
    }

    kdp_driver_hdl_t *pDrv = gDrvCtx.uart_dev[com_port];
    return pDrv->pCap;
}

/*
Set control for the device
Input:
    handle: device handle
    prop: control enumeration
    pVal: pointer to control value/structure
return:
    None
*/
int32_t kdp_uart_control(kdp_uart_hdl_t handle, kdp_uart_ctrl_t prop, uint8_t *pVal)
{
    uint32_t com_port = handle;

    if (com_port >= TOTAL_UART_DEV) {
        //dbg_msg("Invalid parameter\n");
        return UART_FAIL;
    }

    if (gDrvCtx.active_dev[com_port] == FALSE)
    {
        //dbg_msg("This UART device has been closed\n");
        return UART_FAIL;
    }

    kdp_driver_hdl_t *pDrv = gDrvCtx.uart_dev[com_port];

    if ((pDrv->info.flags & UART_POWERED) == 0)
    {
        // UART is not powered 
        return UART_API_NOT_POWRERED;
    }

    if ((pDrv->info.flags & UART_INITIALIZED) == 0) {
        // Return error, if USART is not initialized
        return UART_API_ERROR;
    }

    DRVUART_PORT port_no = (DRVUART_PORT)pDrv->uart_port;
    switch (prop)
    {
    case UART_CTRL_CONFIG:
    {
        KDP_UART_CONFIG_t cfg = *(KDP_UART_CONFIG_t *)pVal;
        UINT32 baudrate = cfg.baudrate;
        UINT32 parity = cfg.parity_mode;
        UINT32 num = cfg.stop_bits;
        UINT32 len = cfg.data_bits;

        kdp_serial_init(port_no, baudrate, parity, num, len, 0);

        pDrv->config.baudrate = baudrate;
        pDrv->config.data_bits = cfg.data_bits;
        pDrv->config.stop_bits = num;
        pDrv->config.parity_mode = parity;
        pDrv->config.fifo_en = cfg.fifo_en;

        pDrv->res.tx_fifo_threshold = 0;
        pDrv->res.rx_fifo_threshold = 0;

        pDrv->nTimeOutTx = kdp_uart_get_default_timeout(baudrate);
        pDrv->nTimeOutRx = kdp_uart_get_default_timeout(baudrate);

        pDrv->info.flags |= UART_BASIC_CONFIGURED;

        pDrv->state = UART_INIT_DONE;
        break;
    }

    case UART_CTRL_TX_EN:
    {
        uint32_t ww = inw(UART_PORT[port_no] + SERIAL_IER);
        if (*pVal == 0)
        {
            ww &= ~SERIAL_IER_TE;                    // disable TX
            outw(UART_PORT[port_no] + SERIAL_IER, ww);
            pDrv->info.flags &= ~UART_TX_ENABLED;
        }
        else
        {
            ww |= SERIAL_IER_TE;                    // enable TX
            outw(UART_PORT[port_no] + SERIAL_IER, ww);
            pDrv->info.flags |= UART_TX_ENABLED;
        }

        break;
    }

    case UART_CTRL_RX_EN:
    {
        uint32_t ww = inw(UART_PORT[port_no] + SERIAL_IER);
        if (*pVal == 0)
        {
            ww &= ~SERIAL_IER_DR;                    // disable Rx
            outw(UART_PORT[port_no] + SERIAL_IER, ww);
            pDrv->info.flags &= ~UART_RX_ENABLED;
        }
        else
        {
            ww |= SERIAL_IER_DR;                    // enable Rx
            outw(UART_PORT[port_no] + SERIAL_IER, ww);
            pDrv->info.flags |= UART_RX_ENABLED;
        }

        break;
    }

    case UART_CTRL_ABORT_TX:
    {
        /* disable Tx interrupt */
        uint32_t ww = inw(UART_PORT[port_no] + SERIAL_IER);
        ww &= ~SERIAL_IER_TE;
        outw(UART_PORT[port_no] + SERIAL_IER, ww);

        /* reset Tx FIFO */
        ww = inw(UART_PORT[port_no] + SERIAL_FCR);
        ww |= (SERIAL_FCR_TXFR | SERIAL_FCR_FE);
        outw(UART_PORT[port_no] + SERIAL_FCR, ww);

        pDrv->info.status.tx_busy = 0;

        break;
    }

    case UART_CTRL_ABORT_RX:
    {
        /* disable Rx interrupt */
        uint32_t ww = inw(UART_PORT[port_no] + SERIAL_IER);
        ww &= ~SERIAL_IER_DR;
        outw(UART_PORT[port_no] + SERIAL_IER, ww);

        /* reset Rx FIFO */
        ww = inw(UART_PORT[port_no] + SERIAL_FCR);
        ww |= (SERIAL_FCR_RXFR | SERIAL_FCR_FE);
        outw(UART_PORT[port_no] + SERIAL_FCR, ww);

        pDrv->info.status.rx_busy = 0;

        break;
    }

    case UART_CTRL_FIFO_RX:
    {
        uint32_t ww;
        kdp_uart_fifo_cfg_t *pCfg = (kdp_uart_fifo_cfg_t *)pVal;
        uint8_t trig_lvl = 0x3 & pCfg->fifo_trig_level;

        pDrv->res.rx_fifo_threshold = trig_lvl;

        if (pCfg->bEnFifo) {
            ww = inw(UART_PORT[port_no] + SERIAL_FCR);
            ww &= ~(0x3 << 6);
            ww |= (trig_lvl << 6) | SERIAL_FCR_RXFR | SERIAL_FCR_FE;  // val + reset + enable
            outw(UART_PORT[port_no] + SERIAL_FCR, ww);
        }
        else {
            /*
              becasue RX/TX FIFO can only be enable/disabled simutanously,
              cannot be set individually, so set trigger val to 0 for FIFO disable
            */
            ww = inw(UART_PORT[port_no] + SERIAL_FCR);
            ww &= ~(0x3 << 6);
            ww |= (0 << 6) | SERIAL_FCR_RXFR | SERIAL_FCR_FE;  // val + reset + enable
            outw(UART_PORT[port_no] + SERIAL_FCR, ww);
        }

        pDrv->info.flags |= UART_FIFO_RX_CONFIGURED;

        break;
    }

    case UART_CTRL_FIFO_TX:
    {
        uint32_t ww;
        kdp_uart_fifo_cfg_t *pCfg = (kdp_uart_fifo_cfg_t *)pVal;
        uint8_t trig_lvl = 0x3 & pCfg->fifo_trig_level;

        pDrv->res.tx_fifo_threshold = trig_lvl;

        if (pCfg->bEnFifo) {
            ww = inw(UART_PORT[port_no] + SERIAL_FCR);
            ww &= ~(0x3 << 4);
            ww |= (trig_lvl << 4) | SERIAL_FCR_TXFR | SERIAL_FCR_FE;  // val + reset + enable
            outw(UART_PORT[port_no] + SERIAL_FCR, ww);
        }
        else
        {
            /*
              becasue RX/TX FIFO can only be enable/disabled simutanously,
              cannot be set individually, so set trigger val to 0 for FIFO disable
            */
            ww = inw(UART_PORT[port_no] + SERIAL_FCR);
            ww &= ~(0x3 << 4);
            ww |= (0 << 4) | SERIAL_FCR_TXFR | SERIAL_FCR_FE;  // val + reset + enable
            outw(UART_PORT[port_no] + SERIAL_FCR, ww);
        }

        pDrv->info.flags |= UART_FIFO_TX_CONFIGURED;

        break;
    }

    case UART_CTRL_LOOPBACK:
    {
        uint32_t ww = inw(UART_PORT[port_no] + SERIAL_MCR);
        if (*pVal == 0)
        {
            ww &= ~SERIAL_MCR_LPBK;                    // disable loopback
            outw(UART_PORT[port_no] + SERIAL_MCR, ww);
        }
        else
        {
            ww |= SERIAL_MCR_LPBK;                    // enable loopback
            outw(UART_PORT[port_no] + SERIAL_MCR, ww);
        }

        pDrv->info.flags |= UART_LOOPBACK_ENABLED;

        break;
    }

    case UART_CTRL_RTS_EN:
    {
        uint32_t ww = inw(UART_PORT[port_no] + SERIAL_MCR);
        if (*pVal == 0)
        {
            ww &= ~SERIAL_MCR_RTS;                    // disable RTS
            outw(UART_PORT[port_no] + SERIAL_MCR, ww);
        }
        else
        {
            ww |= SERIAL_MCR_RTS;                    // enable RTS
            outw(UART_PORT[port_no] + SERIAL_MCR, ww);
        }

        break;
    }

    case UART_CTRL_DTR_EN:
    {
        uint32_t ww = inw(UART_PORT[port_no] + SERIAL_MCR);
        if (*pVal == 0)
        {
            ww &= ~SERIAL_MCR_DTR;                    // disable DTR
            outw(UART_PORT[port_no] + SERIAL_MCR, ww);
        }
        else
        {
            ww |= SERIAL_MCR_DTR;                    // enable DTR
            outw(UART_PORT[port_no] + SERIAL_MCR, ww);
        }

        break;
    }

    case UART_CTRL_TIMEOUT_RX:
    {
        pDrv->nTimeOutRx = (int32_t)*pVal;

        break;
    }

    case UART_CTRL_TIMEOUT_TX:
    {
        pDrv->nTimeOutTx = (int32_t)*pVal;

        break;
    }

    default:
			break;
    }
    return UART_API_RETURN_SUCCESS;
}

/*
Query driver status
Input:
    handle: driver handle

Output:
    driver status:  NULL means failure
*/
UART_STATUS_t *kdp_uart_get_status(kdp_uart_hdl_t handle)
{
    uint32_t com_port = handle;

    if (com_port >= TOTAL_UART_DEV) {
        //dbg_msg("Invalid parameter\n");
        return NULL;
    }

    if (gDrvCtx.active_dev[com_port] == FALSE)
    {
        //dbg_msg("This UART device has been closed\n");
        return NULL;
    }

    kdp_driver_hdl_t *pDrv = gDrvCtx.uart_dev[com_port];
    return &pDrv->info.status;
}

/*
Write data to Mozart device, such as command, parameters, but not suitable for chunk data
Input:
    hdl: device handle
    buf: data buffer
    len: data buffer length
return:
    driver status
*/
kdp_uart_api_sts_t kdp_uart_write(kdp_uart_hdl_t handle, uint8_t *data, uint32_t len)
{
    uint32_t com_port = handle;

    if (com_port >= TOTAL_UART_DEV) {
        //dbg_msg("Invalid parameter\n");
        return UART_API_INVALID_PARAM;
    }

    if (gDrvCtx.active_dev[com_port] == FALSE)
    {
        //dbg_msg("This UART device has been closed\n");
        return UART_API_INVALID_PARAM;
    }

    kdp_driver_hdl_t *pDrv = gDrvCtx.uart_dev[com_port];

    if ((data == NULL) || (len == 0))
    {
        // Invalid parameters
        return UART_API_INVALID_PARAM;
    }

    if ((pDrv->info.flags & UART_POWERED) == 0)
    {
        // UART is not powered 
        return UART_API_NOT_POWRERED;
    }

    if (pDrv->info.status.tx_busy == 1)
    {
        // Send is not completed yet
        return UART_API_TX_BUSY;
    }

    if (pDrv->info.mode & UART_MODE_ASYN_TX)
    {
        /* setup TX FIFO
           re-calculate FIFO trigger value based on buffer length
        */
        if (pDrv->config.fifo_en == TRUE)
        {
            kdp_uart_fifo_cfg_t cfg;
            cfg.bEnFifo = TRUE;
            cfg.fifo_trig_level = 0;      // init with 0
            kdp_calculate_fifo_cfg(pDrv, len, &cfg);
            kdp_uart_control(com_port, UART_CTRL_FIFO_TX, (uint8_t *)&cfg);
        }

        pDrv->info.status.tx_busy = 1;

        // Save transmit buffer info
        pDrv->info.xfer.tx_buf = (uint8_t *)data;
        pDrv->info.xfer.tx_num = len;
        pDrv->info.xfer.tx_cnt = 0;

        kdp_set_serial_int((DRVUART_PORT)pDrv->uart_port, SERIAL_IER_TE);

        NVIC_SetVector((IRQn_Type)pDrv->res.irq_num, (uint32_t)pDrv->res.isr);
        NVIC_EnableIRQ((IRQn_Type)pDrv->res.irq_num);

        UART_TX_ISR(pDrv);


        return UART_API_TX_BUSY;
    }
    else if (pDrv->info.mode & UART_MODE_SYNC_TX)
    {

        pDrv->info.status.tx_busy = 1;

        while (len > 0)
        {

            _check_tx_status((DRVUART_PORT)com_port);
            outw(UART_PORT[com_port] + SERIAL_THR, *data++);

            len--;
        }

        pDrv->info.status.tx_busy = 0;

        return UART_API_RETURN_SUCCESS;
    }
    else
    {
        //dbg_msg("Error: Sync/Async mode was not set\n");
        return UART_API_ERROR;
    }
}

/**
  Read data from UART receiver.
Input
  data:  buffer for receving data
  len:   size  Data buffer size in bytes
  handle:  Driver handle
return
    driver status
*/
kdp_uart_api_sts_t kdp_uart_read(kdp_uart_hdl_t handle, uint8_t *data, uint32_t len)
{
    uint32_t com_port = handle;

    kdp_uart_rx_busy_clear( handle );

    if (com_port >= TOTAL_UART_DEV) {
        //dbg_msg("Invalid parameter\n");
        return UART_API_INVALID_PARAM;
    }

    if (gDrvCtx.active_dev[com_port] == FALSE)
    {
        //dbg_msg("This UART device has been closed\n");
        return UART_API_INVALID_PARAM;
    }

    kdp_driver_hdl_t *pDrv = gDrvCtx.uart_dev[com_port];

    if ((data == NULL) || (len == 0))
    {
        // Invalid parameters
        return UART_API_INVALID_PARAM;
    }

    if ((pDrv->info.flags & UART_POWERED) == 0)
    {
        // UART is not powered
        return UART_API_NOT_POWRERED;
    }

    // Check if receiver is busy
    if (pDrv->info.status.rx_busy == 1)
    {
        return UART_API_RX_BUSY;
    }

    // Clear RX status
    pDrv->info.status.rx_break = 0;
    pDrv->info.status.rx_framing_error = 0;
    pDrv->info.status.rx_overflow = 0;
    pDrv->info.status.rx_parity_error = 0;

    // Save receive buffer info
    pDrv->info.xfer.rx_buf = (uint8_t *)data;
    pDrv->info.xfer.rx_cnt = 0;
    pDrv->info.xfer.write_idx = 0;
    pDrv->info.xfer.read_idx = 0;
    // Save number of data to be received
    pDrv->info.xfer.rx_num = len;


    /* setup RX FIFO trigger level based on buf len*/
    if (pDrv->config.fifo_en == TRUE)
    {
        kdp_uart_fifo_cfg_t cfg;
        cfg.bEnFifo = TRUE;
        cfg.fifo_trig_level = 0;      // init with 0
        kdp_calculate_fifo_cfg(pDrv, len, &cfg);
        kdp_uart_control(com_port, UART_CTRL_FIFO_RX, (uint8_t *)&cfg);
    }

    pDrv->info.status.rx_busy = 1;

    kdp_set_serial_int((DRVUART_PORT)pDrv->uart_port, SERIAL_IER_DR);

    NVIC_SetVector((IRQn_Type)pDrv->res.irq_num, (uint32_t)pDrv->res.isr);
    NVIC_EnableIRQ((IRQn_Type)pDrv->res.irq_num);

    if (pDrv->info.mode & UART_MODE_ASYN_RX)
    {
        return UART_API_RX_BUSY;
    }
    else if (pDrv->info.mode & UART_MODE_SYNC_RX)
    {
        int32_t timeout = pDrv->nTimeOutRx;
        if (timeout <= 0)
        {
            //dbg_msg("Error: Rx timeout value is not set correctly\n");
            timeout = DEFAULT_SYNC_TIMEOUT_CHARS_TIME;
        }

        while ((pDrv->info.status.rx_busy == 1) && (timeout > 0))
        {
            osDelay(1);
            timeout--;
        }
        if (timeout == 0)
            return UART_API_TIMEOUT;
        return UART_API_RETURN_SUCCESS;
    }
    else
    {
        //dbg_msg("Error: Sync/Async mode was not set\n");
        return UART_API_ERROR;
    }
}

/*
Power control
Input:
    handle: driver handle
    pwr_set: power status expected to be set

Output:
    success or fail
*/
int32_t kdp_uart_power_control(kdp_uart_hdl_t handle, ARM_POWER_STATE state)
{
    uint32_t data;
    uint32_t com_port = handle;

    if (com_port >= TOTAL_UART_DEV) {
        //dbg_msg("Invalid parameter\n");
        return UART_API_INVALID_PARAM;
    }

    if (gDrvCtx.active_dev[com_port] == FALSE)
    {
        //dbg_msg("This UART device is not active\n");
        return UART_API_INVALID_PARAM;
    }

    kdp_driver_hdl_t *pDrv = gDrvCtx.uart_dev[com_port];

    if (pDrv->info.status.rx_busy == 1) {
        // Receive busy
        return UART_API_RX_BUSY;
    }

    if (pDrv->info.status.tx_busy == 1) {
        // transmit busy
        return UART_API_TX_BUSY;
    }

    if (state == ARM_POWER_FULL)
    {
        if ((pDrv->info.flags & UART_INITIALIZED) == 0U)
        {
            return UART_API_ERROR;
        }

        if ((pDrv->info.flags & UART_POWERED) != 0U)
        {
            return UART_API_RETURN_SUCCESS;
        }

        /* power on UART */
        switch (pDrv->uart_port)
        {
        case UART0_DEV:


            break;

        case UART1_DEV:

            data = inw(0xc238001c);
            outw(0xc238001c, data | 0x1000);
            PINMUX_LC_DATA2_SET(4);
            PINMUX_LC_DATA3_SET(4);
            SCU_EXTREG_CLK_EN2_SET_uart1_0_fref(1);

            break;

        case UART2_DEV:
    
            data = inw(SCU_EXTREG_PA_BASE + 0x154);
            data &= 0xFFFFFFF8;		//clear low 3bit
            data &= 0xFFFFFFE7;		//clear bit 3 and bit4
            
            outw(SCU_EXTREG_PA_BASE + 0x154, data | 0x1 | 1<<4 );

            data = inw(SCU_EXTREG_PA_BASE + 0x158);
            data &= 0xFFFFFFF8;		//clear low 3bit
            data &= 0xFFFFFFE7;		//clear bit 3 and bit4
            
            outw(SCU_EXTREG_PA_BASE + 0x158, data | 0x1 | 1<<4 );

            SCU_EXTREG_CLK_EN2_SET_uart1_1_fref(1);

            break;

        case UART3_DEV:

            data = inw(0xc238001c);
            outw(0xc238001c, data | 0x4000);
            PINMUX_SD_DATA0_SET(6);
            PINMUX_SD_DATA1_SET(6);
            SCU_EXTREG_CLK_EN2_SET_uart1_2_fref(1);

            break;

        case UART4_DEV:

            data = inw(0xc238001c);
            outw(0xc238001c, data | 0x8000);
            PINMUX_SD_DATA2_SET(6);
            PINMUX_SD_DATA3_SET(6);

            SCU_EXTREG_CLK_EN2_SET_uart1_3_fref(1);

            break;

        default:
						break;
        }

        /* set power on flag */
        pDrv->info.flags |= UART_POWERED;

        kdp_uart_fifo_cfg_t cfg;

        /* config FIFO trigger value with default value or client set via control API*/
        cfg.fifo_trig_level = pDrv->res.rx_fifo_threshold;
        cfg.bEnFifo = pDrv->config.fifo_en;
        kdp_uart_control(com_port, UART_CTRL_FIFO_RX, (void *)&cfg);

        cfg.fifo_trig_level = pDrv->res.tx_fifo_threshold;
        cfg.bEnFifo = (cfg.fifo_trig_level == 0) ? FALSE : TRUE;
        kdp_uart_control(com_port, UART_CTRL_FIFO_TX, (void *)&cfg);

        NVIC_ClearPendingIRQ((IRQn_Type)pDrv->res.irq_num);
    }
    else if (state == ARM_POWER_OFF)
    {

        if ((pDrv->info.flags & UART_POWERED) == 0U)
        {
            return UART_API_RETURN_SUCCESS;
        }

        /* power off UART */
        switch (pDrv->uart_port)
        {
        case UART0_DEV:


            break;

        case UART1_DEV:

            data = inw(0xc238001c);
            outw(0xc238001c, data | 0x0000);
            data = inw(0xc2380144);
            outw(0xc2380144, data | 0x0);
            data = inw(0xc2380148);
            outw(0xc2380148, data | 0x0);
            SCU_EXTREG_CLK_EN2_SET_uart1_0_fref(0);

            break;

        case UART2_DEV:

            data = inw(0xc238001c);
            outw(0xc238001c, data | 0x0000);
            data = inw(0xc238017c);
            outw(0xc238017c, data | 0x0);
            data = inw(0xc2380180);
            outw(0xc2380180, data | 0x0);

            SCU_EXTREG_CLK_EN2_SET_uart1_1_fref(0);

            break;

        case UART3_DEV:

            data = inw(0xc238001c);
            outw(0xc238001c, data | 0x0000);
            data = inw(0xc2380184);
            outw(0xc2380184, data | 0x0);
            data = inw(0xc2380188);
            outw(0xc2380188, data | 0x0);
            SCU_EXTREG_CLK_EN2_SET_uart1_2_fref(0);

            break;

        case UART4_DEV:

            data = inw(0xc238001c);
            outw(0xc238001c, data | 0x0000);
            data = inw(0xc238018c);
            outw(0xc238018c, data | 0x0);
            data = inw(0xc2380190);
            outw(0xc2380190, data | 0x0);

            SCU_EXTREG_CLK_EN2_SET_uart1_3_fref(0);

            break;

        default:
						break;
        }

        NVIC_ClearPendingIRQ((IRQn_Type)pDrv->res.irq_num);

        /* disable Tx/Rx interrupts */
        uint32_t ww = inw(pDrv->uart_port + SERIAL_IER);
        ww &= ~(SERIAL_IER_DR | SERIAL_IER_TE);
        outw(UART_PORT[com_port] + SERIAL_IER, ww);

        /* set power on flag */
        pDrv->info.flags &= ~UART_POWERED;
    }
    else
    {
        return UART_API_ERROR;
    }

    return UART_API_RETURN_SUCCESS;

}


/* get char number in Rx buffer 
Input:
    handle: device handle
Return:
    Received bytes 
*/
uint32_t kdp_uart_GetRxCount(kdp_uart_hdl_t handle)
{
    uint32_t data;
    uint32_t com_port = handle;

    if (com_port >= TOTAL_UART_DEV) {
        //dbg_msg("Invalid parameter\n");
        return UART_API_INVALID_PARAM;
    }

    if (gDrvCtx.active_dev[com_port] == FALSE)
    {
        //dbg_msg("This UART device is not active\n");
        return UART_API_INVALID_PARAM;
    }

    kdp_driver_hdl_t *pDrv = gDrvCtx.uart_dev[com_port];
    data = pDrv->info.xfer.rx_cnt;
    return data;
}

/* get char number in Tx buffer 
Input:
    handle: device handle
Return:
    Sent bytes
*/
uint32_t kdp_uart_GetTxCount(kdp_uart_hdl_t handle)
{
    uint32_t data;
    uint32_t com_port = handle;

    if (com_port >= TOTAL_UART_DEV) {
        //dbg_msg("Invalid parameter\n");
        return UART_API_INVALID_PARAM;
    }

    if (gDrvCtx.active_dev[com_port] == FALSE)
    {
        //dbg_msg("This UART device is not active\n");
        return UART_API_INVALID_PARAM;
    }

    kdp_driver_hdl_t *pDrv = gDrvCtx.uart_dev[com_port];
    data = pDrv->info.xfer.tx_cnt;
    return data;

}

uint32_t kdp_uart_GetRxBufSize(kdp_uart_hdl_t handle)
{
    uint32_t com_port = handle;

    if (com_port >= TOTAL_UART_DEV) {
        //dbg_msg("Invalid parameter\n");
        return UART_API_INVALID_PARAM;
    }

    if (gDrvCtx.active_dev[com_port] == FALSE)
    {
        //dbg_msg("This UART device is not active\n");
        return UART_API_INVALID_PARAM;
    }

    kdp_driver_hdl_t *pDrv = gDrvCtx.uart_dev[com_port];
    return pDrv->info.xfer.rx_num;
}

uint32_t kdp_uart_GetWriteIndex(kdp_uart_hdl_t handle)
{
    uint32_t com_port = handle;

    if (com_port >= TOTAL_UART_DEV) {
        //dbg_msg("Invalid parameter\n");
        return UART_API_INVALID_PARAM;
    }

    if (gDrvCtx.active_dev[com_port] == FALSE)
    {
        //dbg_msg("This UART device is not active\n");
        return UART_API_INVALID_PARAM;
    }

    kdp_driver_hdl_t *pDrv = gDrvCtx.uart_dev[com_port];
    return pDrv->info.xfer.write_idx;
}

uint32_t kdp_uart_GetReadIndex(kdp_uart_hdl_t handle)
{
    uint32_t com_port = handle;

    if (com_port >= TOTAL_UART_DEV) {
        //dbg_msg("Invalid parameter\n");
        return UART_API_INVALID_PARAM;
    }

    if (gDrvCtx.active_dev[com_port] == FALSE)
    {
        //dbg_msg("This UART device is not active\n");
        return UART_API_INVALID_PARAM;
    }

    kdp_driver_hdl_t *pDrv = gDrvCtx.uart_dev[com_port];
    return pDrv->info.xfer.read_idx;
}

uint32_t kdp_uart_SetWriteIndex(kdp_uart_hdl_t handle, uint32_t index)
{
    uint32_t com_port = handle;

    if (com_port >= TOTAL_UART_DEV) {
        //dbg_msg("Invalid parameter\n");
        return UART_API_INVALID_PARAM;
    }

    if (gDrvCtx.active_dev[com_port] == FALSE)
    {
        //dbg_msg("This UART device is not active\n");
        return UART_API_INVALID_PARAM;
    }

    kdp_driver_hdl_t *pDrv = gDrvCtx.uart_dev[com_port];
    pDrv->info.xfer.write_idx = index;
    return UART_API_RETURN_SUCCESS;
}

uint32_t kdp_uart_SetReadIndex(kdp_uart_hdl_t handle, uint32_t index)
{
    uint32_t com_port = handle;

    if (com_port >= TOTAL_UART_DEV) {
        //dbg_msg("Invalid parameter\n");
        return UART_API_INVALID_PARAM;
    }

    if (gDrvCtx.active_dev[com_port] == FALSE)
    {
        //dbg_msg("This UART device is not active\n");
        return UART_API_INVALID_PARAM;
    }

    kdp_driver_hdl_t *pDrv = gDrvCtx.uart_dev[com_port];
    pDrv->info.xfer.read_idx = index;
    return UART_API_RETURN_SUCCESS;
}

void kdp_uart_print_register(uint8_t port_no)
{
    kdp_driver_hdl_t *pHdl = kdp_uart_get_drv_hdl(port_no);

    uint32_t ww = inw(UART_PORT[port_no] + SERIAL_RBR);
    kdp_printf("SERIAL_RBR(0x00) = 0x%x\n", ww);

    ww = inw(UART_PORT[port_no] + SERIAL_IER);
    kdp_printf("SERIAL_IER(0x04) = 0x%x\n", ww);

    ww = inw(UART_PORT[port_no] + SERIAL_IIR);
    kdp_printf("SERIAL_IIR(0x08) = 0x%x\n", ww);

    ww = inw(UART_PORT[port_no] + SERIAL_LCR);
    kdp_printf("SERIAL_LCR(0x0C) = 0x%x\n", ww);

    ww = inw(UART_PORT[port_no] + SERIAL_MCR);
    kdp_printf("SERIAL_MCR(0x10) = 0x%x\n", ww);

    ww = inw(UART_PORT[port_no] + SERIAL_LSR);
    kdp_printf("SERIAL_LSR(0x14) = 0x%x\n", ww);

    ww = inw(UART_PORT[port_no] + SERIAL_MSR);
    kdp_printf("SERIAL_MSR(0x18) = 0x%x\n", ww);

    ww = inw(UART_PORT[port_no] + 0x20);
    kdp_printf("UART2(0x20) = 0x%x\n", ww);

    ww = inw(UART_PORT[port_no] + 0x30);
    kdp_printf("UART2(0x30) = 0x%x\n", ww);

    ww = inw(UART_PORT[port_no] + 0x34);
    kdp_printf("UART2(0x34) = 0x%x\n", ww);

    ww = inw(UART_PORT[port_no] + 0x44);
    kdp_printf("UART2(0x44) = 0x%x\n", ww);

    ww = inw(UART_PORT[port_no] + 0x48);
    kdp_printf("UART2(0x48) = 0x%x\n", ww);

    ww = inw(UART_PORT[port_no] + 0x4C);
    kdp_printf("UART2(0x4C) = 0x%x\n", ww);

    ww = inw(UART_PORT[port_no] + 0x5C);
    kdp_printf("UART2(0x5C) = 0x%x\n", ww);

    ww = inw(UART_PORT[port_no] + 0x68);
    kdp_printf("UART2(0x68) = 0x%x\n", ww);
}
void kdp_uart_rx_busy_clear(kdp_uart_hdl_t handle)
{
    kdp_driver_hdl_t *pDrv = gDrvCtx.uart_dev[handle];
//    kdp_printf("Rx busy 0x%x\n", pDrv->info.status.rx_busy );
    pDrv->info.status.rx_busy = 0;

}
