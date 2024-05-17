#include "kl520_com.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF )

#include "io.h"
#include "pinmux.h"
#include "framework/init.h"
#include "framework/v2k_image.h"
#include "framework/framework_driver.h"
#include "framework/event.h"
#include "kl520_api_ssp.h"
#include "kl520_include.h"
#include "kdp_memory.h"
#include "kdp_ddr_table.h"
#include "sample_user_com_and_gui_fdr.h"

#if ( !(CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN) ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
#include "kdp520_ssp.h"
#endif

uint8_t *pgcom_tx_buf = (uint8_t *)KDP_DDR_DRV_COM_BUS_TX_START_ADDR;
uint8_t *pgcom_rx_buf = (uint8_t *)KDP_DDR_DRV_COM_BUS_RX0_START_ADDR;
uint32_t gcom_tx_index = 0; //data legnth
uint32_t gcom_rx_index = 0; //data legnth

struct st_com_type stCom_type;
static kl520_com_user_ops *_user_com_ops = NULL;

__WEAK void kneron_com_parser( struct st_com_type *st_com ){}

void kl520_com_buf_addr_init(void)
{
    stCom_type.tx_buffer = pgcom_tx_buf;
    stCom_type.rx_buffer = pgcom_rx_buf;
    stCom_type.tx_buffer_index = &gcom_tx_index;
    stCom_type.rx_buffer_index = &gcom_rx_index;
}

extern UINT32 Drv_utility_checksum( UINT8 * buf, UINT16 start, UINT16 end );
UINT16 kneron_lwcom_packet_analyze( struct st_com_type *st_com )
{
    UINT32  i = 0;
    UINT8   *ptr = st_com->rx_buffer;
    UINT32  *ptr_indedx = st_com->rx_buffer_index;
    UINT32  nhead_index = 0xFFFFFFFF;
    UINT32  ncheck_sum = 0;

    st_com->no_head_tail_en = 0;

    //-----------------------
    //----- length check ------
    //-----------------------
    if( *ptr_indedx < 16  )    //Head(4B) + Host(2B) + CMD(2B) + DL(2B) + CSUM(4B) + Tail(2B)
    {
        return COM_BUS_LENGTH_CHECK_ERROR;
    }

    //-----------------------
    //-----	head check ------
    //-----------------------
    for( i = 0; i < ( *ptr_indedx-4 ); i++ )
    {
        if(	 (*(ptr + i + 0 ) == COM_BUS_HEAD_RX_1) && (*(ptr +i +  1 ) == COM_BUS_HEAD_RX_2 )
        && (*(ptr +i +  2 ) == COM_BUS_HEAD_RX_3) && (*(ptr +i +  3 ) == COM_BUS_HEAD_RX_4 )	)
        {
            nhead_index = i;
            *(ptr + i + 0 ) = 0;
            *(ptr +i +  1 ) = 0;
            break;
        }
    }

    //-----------------------
    //packet tail  check
    //-----------------------
    if(	(*(ptr + (*ptr_indedx) -1)  !=  COM_BUS_TAIL_1) || ( *(ptr + (*ptr_indedx) -2 )  != COM_BUS_TAIL_2 ) )
    {
        return COM_BUS_TAIL_CHECK_ERROR;
    }

    if( nhead_index == 0xFFFFFFFF  )
    {
        return COM_BUS_HEAD_CHECK_ERROR;
    }

    st_com->host_number = (*( ptr+nhead_index+4 )<<8) + *( ptr+nhead_index+5 );
    st_com->cmd         = (*( ptr+nhead_index+6 )<<8) + *( ptr+nhead_index+7 );
    st_com->data_len    = (*( ptr+nhead_index+8 )<<8) + *( ptr+nhead_index+9 );
    st_com->data_start_index = nhead_index + sizeof(st_com->head) + sizeof( st_com->host_number ) + sizeof( st_com->cmd ) + sizeof(st_com->data_len);
    st_com->checksum    = (*( ptr+(*ptr_indedx)-6 )<<24) | (*(ptr+(*ptr_indedx)-5 )<<16) |
                          (*( ptr+(*ptr_indedx)-4 )<<8 ) | (*(ptr+(*ptr_indedx)-3 ));
						  
    ncheck_sum = Drv_utility_checksum( ptr, nhead_index+6, *ptr_indedx - 6 );

    if( st_com->checksum != ncheck_sum  )
    {
        return COM_BUS_HEAD_CHECK_ERROR;
    }
    return COM_BUS_PACKET_OK;	//exist good data
}

void kneron_lwcom_packet_response_w_tx_buffer( struct st_com_type *st_com, UINT8 *in_data, UINT16 in_data_legn )
{
    UINT16	i ;
    UINT32	nchecksum = 0;
    UINT8	*ptr = st_com->tx_buffer;
    *st_com->tx_buffer_index = 0;

#if ( !(CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN) ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
    if( st_com->no_head_tail_en == 0 )
    {
        *st_com->tx_buffer_index = 0;
        ptr += COM_BUS_RESPONSE_OFFESET;
    }
    else
    {
        //no head and tail Bytes
        *st_com->tx_buffer_index = 0;
        ptr += ( COM_BUS_RESPONSE_OFFESET - 6 );
    }
#endif

    st_com->head = COM_BUS_HEAD_TX;
    st_com->data_len = in_data_legn;

    if( st_com->no_head_tail_en == 0 )
    {
        //head
        *(ptr + *st_com->tx_buffer_index ) = (COM_BUS_HEAD_TX>>24)&0xFF;
        *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
        *(ptr + *st_com->tx_buffer_index ) = (COM_BUS_HEAD_TX>>16)&0xFF;
        *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
        *(ptr + *st_com->tx_buffer_index ) = (COM_BUS_HEAD_TX>>8)&0xFF;
        *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
        *(ptr + *st_com->tx_buffer_index ) = (COM_BUS_HEAD_TX>>0)&0xFF;
        *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
    }

    //host number
    *(ptr + *st_com->tx_buffer_index ) = (st_com->host_number>>8)&0xFF;
    *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
    *(ptr + *st_com->tx_buffer_index ) = (st_com->host_number>>0)&0xFF;
    *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;

    //command
    *(ptr + *st_com->tx_buffer_index ) = (st_com->cmd>>8)&0xFF;
    *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
    *(ptr + *st_com->tx_buffer_index ) = (st_com->cmd>>0)&0xFF;
    *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
    nchecksum+= (st_com->cmd>>8)&0xFF;
    nchecksum+= (st_com->cmd>>0)&0xFF;

    //length
    *(ptr + *st_com->tx_buffer_index ) = (st_com->data_len>>8)&0xFF;
    *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
    *(ptr + *st_com->tx_buffer_index ) = (st_com->data_len>>0)&0xFF;
    *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
    nchecksum+= (st_com->data_len>>8)&0xFF;
    nchecksum+= (st_com->data_len>>0)&0xFF;

    for( i = 0; i < st_com->data_len; i++  )
    {
        *(ptr + *st_com->tx_buffer_index ) = *( in_data+i );
        *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
        nchecksum+= *( in_data+i );
    }

    //checksum
    *(ptr + *st_com->tx_buffer_index ) = (nchecksum>>24)&0xFF;
    *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
    *(ptr + *st_com->tx_buffer_index ) = (nchecksum>>16)&0xFF;
    *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
    *(ptr + *st_com->tx_buffer_index ) = (nchecksum>>8)&0xFF;
    *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
    *(ptr + *st_com->tx_buffer_index ) = (nchecksum>>0)&0xFF;
    *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;

    if( st_com->no_head_tail_en == 0 )
    {
        //tail
        *(ptr + *st_com->tx_buffer_index ) = COM_BUS_TAIL_2;
        *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
        *(ptr + *st_com->tx_buffer_index ) = COM_BUS_TAIL_1;
        *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
    }

}

// OTA CMD
//
//  format: head   host number response    tail
//  size  :  4   2   2   2
//
void kneron_lwcom_packet_response_brief_w_tx_buffer( struct st_com_type *st_com, UINT16 host_number, UINT16 status )
{
    UINT8   *ptr = st_com->tx_buffer;

#if ( !(CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN) ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
    if( st_com->no_head_tail_en == 0 )
    {
        *st_com->tx_buffer_index = 0;
        ptr += COM_BUS_RESPONSE_OFFESET;
    }
    else
    {
        //no head and tail Bytes
        *st_com->tx_buffer_index = 0;
        ptr += ( COM_BUS_RESPONSE_OFFESET - 6 );
    }
#endif

    st_com->head = COM_BUS_HEAD_TX;

    if( st_com->no_head_tail_en == 0 )
    {
        //head
        *(ptr + *st_com->tx_buffer_index ) = (COM_BUS_HEAD_TX>>24)&0xFF;
        *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
        *(ptr + *st_com->tx_buffer_index ) = (COM_BUS_HEAD_TX>>16)&0xFF;
        *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
        *(ptr + *st_com->tx_buffer_index ) = (COM_BUS_HEAD_TX>>8)&0xFF;
        *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
        *(ptr + *st_com->tx_buffer_index ) = (COM_BUS_HEAD_TX>>0)&0xFF;
        *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
    }

    //host number
    *(ptr + *st_com->tx_buffer_index ) = (host_number>>8)&0xFF;
    *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
    *(ptr + *st_com->tx_buffer_index ) = (host_number>>0)&0xFF;
    *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;

    //status
    *(ptr + *st_com->tx_buffer_index ) = (status>>8)&0xFF;
    *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
    *(ptr + *st_com->tx_buffer_index ) = (status>>0)&0xFF;
    *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;

    if( st_com->no_head_tail_en == 0 )
    {
        //tail
        *(ptr + *st_com->tx_buffer_index ) = COM_BUS_TAIL_2;
        *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
        *(ptr + *st_com->tx_buffer_index ) = COM_BUS_TAIL_1;
        *st_com->tx_buffer_index = *st_com->tx_buffer_index+1;
    }

}

void kneron_lwcom_set_parameter( struct st_com_type *st_com, UINT16 nhost_number, UINT16 cmd )
{
    st_com->cmd = cmd;
    st_com->host_number = nhost_number;
}
// OTA CMD



void kl520_com_init(kl520_com_flags flags)
{
    stCom_type.flags = flags;
    stCom_type.com_type = DEV_NULL;
    stCom_type.uart_port = 0xFF;

    kl520_com_buf_addr_init();

#if ( !(CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN) ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
#ifdef COM_BUS_RESPONSE_REQUEST_PIN
    kdp_slave_request_init();
    kdp_slave_request_inactive();
#endif
#endif
    
    
#if ( !(CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN) ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
    stCom_type.com_type = DEV_SPI;
    driver_ssp_ctx.Tx_buffer_index = stCom_type.tx_buffer_index;
    driver_ssp_ctx.Rx_buffer_index = stCom_type.rx_buffer_index;
    kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_init_slave );
    kdp_ssp_clear_rxhw( driver_ssp_ctx.reg_base_address );
    kdp_ssp_clear_txhw( driver_ssp_ctx.reg_base_address );
    kdp_ssp_clear_rx_buf_index( &driver_ssp_ctx );
    kdp_ssp_clear_tx_buf_index( &driver_ssp_ctx );
    kdp_ssp_clear_tx_current_buf_index(&driver_ssp_ctx);
    kdp_ssp_clear_tx_done_flag(&driver_ssp_ctx);
    kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_disable );
    kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_enable );

#elif ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
    stCom_type.com_type = DEV_UART;

#if ( ( CFG_COM_BUS_TYPE&COM_BUS_UART0 ) == COM_BUS_UART0 )
    stCom_type.uart_port = 0;
    stCom_type.dev_id = UART0_DEV;
#elif ( ( CFG_COM_BUS_TYPE&COM_BUS_UART1 ) == COM_BUS_UART1 )
    stCom_type.uart_port = 1;
    stCom_type.dev_id = UART1_DEV;
#elif ( ( CFG_COM_BUS_TYPE&COM_BUS_UART2 ) == COM_BUS_UART2 )
    stCom_type.uart_port = 2;
    stCom_type.dev_id = UART2_DEV;
#elif ( ( CFG_COM_BUS_TYPE&COM_BUS_UART3 ) == COM_BUS_UART3 )
    stCom_type.uart_port = 3;
    stCom_type.dev_id = UART3_DEV;
#elif ( ( CFG_COM_BUS_TYPE&COM_BUS_UART4 ) == COM_BUS_UART4 )
    stCom_type.uart_port = 4;
    stCom_type.dev_id = UART4_DEV;
#endif

    kdp_uart_app_com( stCom_type.uart_port , BAUD_115200, (UINT8 *)stCom_type.rx_buffer, KDP_DDR_DRV_COM_BUS_RESERVED);
#endif
}

void kl520_com_reconfig_baud_rate(int rate)
{
    u32 baudrate;
    
    if(rate == 0)     {baudrate = BAUD_115200;}
    else if(rate == 1){baudrate = BAUD_921600;}
    else if(rate == 2){baudrate = BAUD_460800;}
    else{baudrate = BAUD_921600;}

#ifdef COM_BUS_RESPONSE_REQUEST_PIN
    stCom_type.flags = KL520_COM_HAS_ADDITIONAL_IO;
#else
    stCom_type.flags = KL520_COM_NORMAL;
#endif

    kl520_com_buf_addr_init();

    kdp_uart_app_com( stCom_type.uart_port, baudrate, (UINT8 *)stCom_type.rx_buffer, KDP_DDR_DRV_COM_BUS_RESERVED);
}

void kl520_com_initial( kl520_com_flags flags )
{
    kl520_com_init( flags );
}


//waiting for new data coming and processing it
//this is a shared function
BOOL kl520_com_wait_receive(void)
{
    BOOL ret = FALSE;
    //check Rx busy
#if ( !(CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN) ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
    UINT8	timeout_cnt = 0, time_out_th = 10;
    UINT16  ncount = 0xFFFF;
    //spi busy check
    if ( stCom_type.com_type == DEV_SPI )
    {
        while(1)
        {
            ncount = *stCom_type.rx_buffer_index;
            if( ncount == 0 )
            {
                timeout_cnt++;
                delay_ms(1);
                if( timeout_cnt > time_out_th )
                {
                #if( COM_DEBUG_LOG_EN == YES)
                    dbg_msg("[com_bus] spi timeout ");
                #endif
                    break;
                }
            }
            else
            {
                timeout_cnt = 0;
                while( kl520_api_ssp_spi1_receive(&driver_ssp_ctx) == 0 );
            #if( COM_DEBUG_LOG_EN == YES)
                dbg_msg("[com_bus] spi data get size: %d ", *stCom_type.rx_buffer_index );
            #endif
                ret = TRUE;
                break;
            }
            osDelay(20);
        }
    }
#elif ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
    UINT16  ncount = 0xFFFF;
    //Uart Busy check
    //go to Uart interface, check bus is busy or idle
    if (stCom_type.com_type == DEV_UART )
    {
        //protect and receive all data
        while( ncount != *stCom_type.rx_buffer_index ){
            ncount = *stCom_type.rx_buffer_index;
            delay_ms(20);
            *stCom_type.rx_buffer_index = kdp_uart_GetRxCount( stCom_type.dev_id );
        }
        if( *stCom_type.rx_buffer_index > 0 ){
            #if( COM_DEBUG_LOG_EN == YES)
            dbg_msg("[com_bus] uart rx receive done ");
            #endif
            ret = TRUE; //data at least 1 Byte
        }
    }
#elif ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK )
    //USB Busy check
    //go to USB interface, check bus is busy or idle
    if ( stCom_type.com_type == DEV_USB )
    {

    }
#elif #if ( CFG_COM_BUS_TYPE&COM_BUS_I2C_MASK )
    //I2C Busy check
    //go to I2C interface, check bus is busy or idle
    if (stCom_type.com_type == DEV_I2C )
    {

    }
#endif

    return ret; //no data receive
}

static UINT16 kl520_com_analyze(void)
{
    //how many parser type?
    //head type + tail
    //no head type+no tail => command only type
    if ((_user_com_ops) && (_user_com_ops->packet_analyze)) 
        return _user_com_ops->packet_analyze(&stCom_type);
    else
        return kneron_lwcom_packet_analyze( &stCom_type );
}

static void kl520_com_parser(void)
{
    kl520_com_response_Done_check();//work around for uart rx with no tx.
    // if( stCom_type.no_head_tail_en == 1  )
    // {
    //     _kneron_host_com_parser( &stCom_type );		//for future work
    //     stCom_type.no_head_tail_en = 0;
    // }
    // else
    // {
    if ((_user_com_ops) && (_user_com_ops->parser)) 
        return _user_com_ops->parser(&stCom_type);
    else
        return kneron_com_parser( &stCom_type );
        //Kneron API
        //for add customer parser API
    //}
}


void kl520_com_response_start(void)
{
#if ( !(CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN) ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
    UINT32	nchecksum =0;
    UINT16	noffest = 0;

    if( stCom_type.no_head_tail_en == 0 )
    {
        *(stCom_type.tx_buffer+ noffest++) = ( COM_BUS_GET_DATA_HEAD_TX>>24 &0xFF );
        *(stCom_type.tx_buffer+ noffest++) = ( COM_BUS_GET_DATA_HEAD_TX>>16 &0xFF );
        *(stCom_type.tx_buffer+ noffest++) = ( COM_BUS_GET_DATA_HEAD_TX>>8 &0xFF );
        *(stCom_type.tx_buffer+ noffest++) = ( COM_BUS_GET_DATA_HEAD_TX>>0 &0xFF );
    }

    *(stCom_type.tx_buffer+ noffest++) = ( *stCom_type.tx_buffer_index>>24 &0xFF );
    *(stCom_type.tx_buffer+ noffest++) = ( *stCom_type.tx_buffer_index>>16 &0xFF );
    *(stCom_type.tx_buffer+ noffest++) = ( *stCom_type.tx_buffer_index>>8 &0xFF );
    *(stCom_type.tx_buffer+ noffest++) = ( *stCom_type.tx_buffer_index>>0 &0xFF );

    nchecksum = ( *stCom_type.tx_buffer_index>>24 &0xFF ) +( *stCom_type.tx_buffer_index>>16 &0xFF )
                            +( *stCom_type.tx_buffer_index>>8 &0xFF )+( *stCom_type.tx_buffer_index>>0 &0xFF );

    *(stCom_type.tx_buffer+ noffest++) =  ( nchecksum>>24 &0xFF );
    *(stCom_type.tx_buffer+ noffest++) =  ( nchecksum>>16 &0xFF );
    *(stCom_type.tx_buffer+ noffest++) = ( nchecksum>>8 &0xFF );
    *(stCom_type.tx_buffer+ noffest++) = ( nchecksum>>0 &0xFF );

    if( stCom_type.no_head_tail_en == 0 )
    {
        //tail
        *(stCom_type.tx_buffer+ noffest++) = ( COM_BUS_TAIL_2 );
        *(stCom_type.tx_buffer+ noffest++) = ( COM_BUS_TAIL_1 );
    }

    *stCom_type.tx_buffer_index += noffest;

//	kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_disable );
    kl520_api_ssp_spi1_clear_tx_current_buff_size();
    kdp_ssp_clear_rxhw( driver_ssp_ctx.reg_base_address );
    kdp_ssp_clear_txhw( driver_ssp_ctx.reg_base_address );
//	kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_enable );

  //Need set a GPIO
    kdp_ssp_pre_write_to_fifo( &driver_ssp_ctx, 5 );

#elif ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )

    if( kdp_uart_write( stCom_type.uart_port, stCom_type.tx_buffer, *stCom_type.tx_buffer_index ) == UART_API_ERROR )
    {
        #if( COM_DEBUG_LOG_EN == YES)
        dbg_msg("[com_bus] Response Uart Tx fail ");
        #endif
    }

#endif

}



UINT8 kl520_com_response_Done_check(void)
{
    volatile UINT16 current_count = 0, pre_count = 0;    
#if ( !(CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN) ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
    UINT8	response_TO = 0 , response_TO_target = 100;

#ifdef COM_BUS_RESPONSE_REQUEST_PIN
    if (stCom_type.flags == KL520_COM_HAS_ADDITIONAL_IO)
    {
        kdp_slave_request_active();
    }
#endif

#if( COM_DEBUG_LOG_EN == YES)
    dbg_msg("[com_bus] Response SPI Tx wait ing !!!");
#endif
    kdp_ssp_clear_tx_done_flag( &driver_ssp_ctx ) ;
    while( kdp_ssp_get_tx_done_flag( &driver_ssp_ctx ) == 0 )
    {
        osDelay(50);
        response_TO ++;
        if(response_TO > response_TO_target)
        {
#if( COM_DEBUG_LOG_EN == YES)
            dbg_msg("[com_bus] Response SPI Tx wait Timeout !!!");
#endif
            *stCom_type.tx_buffer_index = 0;
            *stCom_type.rx_buffer_index = 0;
            kl520_api_ssp_spi1_clear_tx_current_buff_size();
            kdp_ssp_clear_rxhw( driver_ssp_ctx.reg_base_address );
            kdp_ssp_clear_txhw( driver_ssp_ctx.reg_base_address );
            return 0;
        }
    }

#if( COM_DEBUG_LOG_EN == YES)
    dbg_msg("[com_bus] Response SPI Tx wait done!!!");
#endif
    while( kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_rx ) == e_spi_ret_rxbusy )
    {
        delay_us(30);
    }

#if( COM_DEBUG_LOG_EN == YES)
    dbg_msg("[com_bus] Response SPI Tx busy done!!!");
#endif

#ifdef COM_BUS_RESPONSE_REQUEST_PIN
    if (stCom_type.flags == KL520_COM_HAS_ADDITIONAL_IO)
    {
        kdp_slave_request_inactive();
    }
#endif

#if( COM_DEBUG_LOG_EN == YES)
    dbg_msg("[com_bus] Response SPI Tx Done");
#endif
    kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_disable );
    kdp_ssp_clear_rxhw( driver_ssp_ctx.reg_base_address );
    kdp_ssp_clear_txhw( driver_ssp_ctx.reg_base_address );
    kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_enable );

#elif ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )

  kdp_uart_read( stCom_type.uart_port, stCom_type.rx_buffer, KDP_DDR_DRV_COM_BUS_RESERVED );
#if( COM_DEBUG_LOG_EN == YES)
    dbg_msg("[com_bus] Response Uart Tx Done");
#endif
#endif

    //clear all sw buffers
    *stCom_type.tx_buffer_index = 0;
    *stCom_type.rx_buffer_index = 0;
    return 1;

}

UINT16 kl520_com_rx_size(void)
{
    return *stCom_type.rx_buffer_index;
}

UINT8 kl520_com_response( struct st_com_type *st_com )
{
    if( *st_com->tx_buffer_index > 0 )
    {
        //do response or not !!!
        kl520_com_response_start();
        return ( kl520_com_response_Done_check() );
    }

    return 0;
}



//===============================================
//normal command and analyze
//===============================================
void kl520_com_thread(void)
{
    UINT16	npacket_result = 0;
    #if( COM_DEBUG_LOG_EN == YES)
    dbg_msg("[com_bus] communication start ");
    #endif

#ifdef CFG_USER_CUSTOM_COMM_WHEN_BOOTING
    user_custom_comm_power_on();
#endif

    while(1)
    {
        osDelay(30);
        //step1: wait and receive data
        if( (kl520_com_wait_receive()) && ( kl520_com_rx_size() > 0 ) )
        {
            //data analyze!   => user_com.c and user_com.h, need to check host_com or customer_com
            if( (npacket_result = kl520_com_analyze() ) == COM_BUS_PACKET_OK )
            {
                //parser and trigger customer API
                kl520_com_parser();
            }
            else
            {
                #if( COM_DEBUG_LOG_EN == YES)
                dbg_msg("[com_bus] communication analyze fail ");
                #endif

                //nee tp modify this!!!
                kneron_lwcom_packet_response_w_tx_buffer( &stCom_type, (UINT8 *)&npacket_result, sizeof(npacket_result) );
                kl520_com_response( &stCom_type );
            }
        #if( COM_DEBUG_LOG_EN == YES)
        dbg_msg("===========");
        #endif
        }
    }

}
//===============================================
extern void Drv_OTA_Thread( void );
void kl520_com_bus_init(void)
{
    Drv_OTA_Thread();
}

void kl520_com_reg_user_ops(kl520_com_user_ops *ops)
{
    _user_com_ops = ops;
}
#endif
#endif
//#endif
#endif
