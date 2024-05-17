#include "kdp_comm.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP )
#include "ota.h"
#include "io.h"
#include "pinmux.h"
#include "framework/init.h"
#include "framework/v2k_image.h"
#include "framework/framework_driver.h"
#include "framework/event.h"
#include "kl520_api_ssp.h"
#include "kdp_memory.h"
#include "kdp_ddr_table.h"
#include "kl520_include.h"
#include "kl520_api.h"
#include "kl520_com.h"
#include "kdp_comm_app.h"
#include "kdp_comm_protoco.h"
#include "kdp_comm_and_gui_fdr.h"
#include "kl520_api_camera.h"
//AES-code
#include "kdp_comm_aes.h"
#include "kdp_comm_utils.h"
#include "kl520_api_fdfr.h"

#if ( !(CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN) ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
#include "kdp520_ssp.h"
#endif

//-----------------------------------

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
uint8_t *pgcom_tx_buf = (uint8_t *)KDP_DDR_DRV_COM_BUS_TX_START_ADDR;
uint8_t *pgcom_rx_buf = (uint8_t *)KDP_DDR_DRV_COM_BUS_RX0_START_ADDR;
uint8_t *pgcom_rx1_buf = (uint8_t *)KDP_DDR_DRV_COM_BUS_RX1_START_ADDR;
uint32_t gcom_tx_index = 0; //data legnth
uint32_t gcom_rx_index = 0; //data legnth
#endif

struct st_com_type stCom_type = {(dev_type)0};

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
static kl520_com_user_ops *_user_com_ops = NULL;
#endif

__WEAK void kneron_com_parser( struct st_com_type *st_com ){}

#define MAX_BUF_SIZE  256  //1024
#define MAX_BUF_SIZE_OTA_MAX  5120  //1024
#define MSG_MAX_SIZE  4096
uint8_t* msg_dec = NULL;//(uint8_t*)kdp_ddr_reserve(MSG_MAX_SIZE);

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
//===============================================
// Initial
//===============================================
void kl520_com_buf_addr_init(void)
{
    stCom_type.tx_buffer = pgcom_tx_buf;
    stCom_type.rx_buffer = pgcom_rx_buf;
    stCom_type.parser_buffer = pgcom_rx1_buf;
    stCom_type.parser_cnt = 0;
    stCom_type.parser_end = 0;
    stCom_type.tx_buffer_index = &gcom_tx_index;
    stCom_type.rx_buffer_index = &gcom_rx_index;
}

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
void kl520_com_reconfig_baud_rate(int rate)
{
    u32 baudrate;

    if(rate == 0)     {baudrate = BAUD_115200;}
    else if(rate == 1){baudrate = BAUD_921600;}
    else if(rate == 2){baudrate = BAUD_460800;}
    else if(rate == 3){baudrate = BAUD_230400;}
    else if(rate == 4){baudrate = BAUD_1500000;}
    else{baudrate = BAUD_921600;}

#ifdef COM_BUS_RESPONSE_REQUEST_PIN
    stCom_type.flags = KL520_COM_HAS_ADDITIONAL_IO;
#else
    stCom_type.flags = KL520_COM_NORMAL;
#endif

    kl520_com_buf_addr_init();

    kdp_uart_app_com( stCom_type.uart_port, baudrate, (UINT8 *)stCom_type.rx_buffer, KDP_DDR_DRV_COM_BUS_RESERVED);
}
#endif

void kl520_com_init(kl520_com_flags flags)
{
    stCom_type.flags = flags;
    stCom_type.com_type = DEV_NULL;
    stCom_type.uart_port = 0xFF;

    stCom_type.tx_buffer = pgcom_tx_buf;
    stCom_type.rx_buffer = pgcom_rx_buf;
    stCom_type.parser_buffer = pgcom_rx1_buf;
    stCom_type.parser_cnt = 0;
    stCom_type.parser_end = 0;
    stCom_type.tx_buffer_index = &gcom_tx_index;
    stCom_type.rx_buffer_index = &gcom_rx_index;

#if ( ENCRYPTION_MODE&AES_ENCRYPTION ) || ( ENCRYPTION_MODE&XOR_ENCRYPTION )
    if ( msg_dec == NULL )
    {
        msg_dec = (uint8_t*)kdp_ddr_reserve(MSG_MAX_SIZE);
    }
#endif

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

void kl520_com_initial( kl520_com_flags flags )
{
    kl520_com_init( flags );
}

//===============================================
//normal command and analyze
//===============================================
void kl520_com_response_start(void)
{
      //dbg_msg_console("kl520_com_response_start func coming in");
#if ( !(CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN) ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
    UINT32  nchecksum =0;
    UINT16  noffest = 0;
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

//  kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_disable );
    kl520_api_ssp_spi1_clear_tx_current_buff_size();
    kdp_ssp_clear_rxhw( driver_ssp_ctx.reg_base_address );
    kdp_ssp_clear_txhw( driver_ssp_ctx.reg_base_address );
//  kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_enable );

  //Need set a GPIO
    kdp_ssp_pre_write_to_fifo( &driver_ssp_ctx, 5 );

#elif ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
    //dbg_msg_console("lmm-edit [com_bus] Response Uart Tx starting ");
    if( kdp_uart_write( stCom_type.uart_port, stCom_type.tx_buffer, *stCom_type.tx_buffer_index ) == UART_API_ERROR )
    {
        dbg_msg_console("[com_bus] Response Uart Tx fail ");
        #if( COM_DEBUG_LOG_EN == YES)
        dbg_msg_console("[com_bus] Response Uart Tx fail ");
        #endif
    }
    dbg_msg_console("[com_bus] Response Uart Tx end ");

#endif
}

UINT8 kl520_com_response( struct st_com_type *st_com )
{
    if( *st_com->tx_buffer_index > 0 )
    {
        //dbg_msg_console("kl520_com_response in");
        //do response or not !!!
//      CurTime = osKernelGetTickCount();
//      InterTime = CurTime-LastTime;
//      if( InterTime < MSG_INTERNAL_TIME )
//      {
//          dbg_msg_console("$$$$$$$$$$ internal time < 100,time:%d $$$$$$$$$",InterTime);
//          //return 0;
//          osDelay( (MSG_INTERNAL_TIME-InterTime) );
//      }
        kl520_com_response_start();
//      LastTime = osKernelGetTickCount();
//        return 0;//( kl520_com_response_Done_check() );
        return 0;//( kl520_com_response_Done_check() );
    }

    return 0;
}

UINT16 kl520_com_rx_size(void)
{
    return *stCom_type.rx_buffer_index;
}

UINT8 kl520_com_response_Done_check(void)
{
    volatile UINT16 current_count = 0, pre_count = 0;
#if ( !(CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN) ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
    UINT8 response_TO = 0 , response_TO_target = 100;

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

    // int remain_len = *stCom_type.rx_buffer_index - stCom_type.rx_count;
    // if(remain_len > 0)
    // {
    //     memcpy(stCom_type.rx_buffer, stCom_type.rx_buffer + stCom_type.rx_count, remain_len);
    //     *stCom_type.rx_buffer_index = remain_len;
    // }
    // else
    // {
    //     *stCom_type.rx_buffer_index = 0;
    // }
    // stCom_type.rx_count = 0;

    // kdp_uart_read( stCom_type.uart_port, stCom_type.rx_buffer + (*stCom_type.rx_buffer_index), KDP_DDR_DRV_COM_BUS_RESERVED );
#if( COM_DEBUG_LOG_EN == YES)
    dbg_msg("[com_bus] Response Uart Tx Done");
#endif
#endif

    //clear all sw buffers
    *stCom_type.tx_buffer_index = 0;
    // *stCom_type.rx_buffer_index = 0;
    return 1;

}

//waiting for new data coming and processing it
//this is a shared function

#define UART_PACKET_SIZE_CHECK_EN (YES)
#if UART_PACKET_SIZE_CHECK_EN == YES
#define PACKET_RECEIVE_TIMEOUT_CNT_NUM 3
s16 kneron_lwcom_packet_size( struct st_com_type *st_com, u16 header_index)
{    
    uint32_t  i = 0;
    uint8_t   *ptr = st_com->parser_buffer;
    uint32_t  ptr_indedx = st_com->parser_cnt;//last id
    uint32_t  nhead_index = 0xFFFFFFFF;
    BOOL    bHeadChkFlag = FALSE;
    BOOL    bAllZeroByte = TRUE;
    s16     rx_count_size;

    if ( ptr_indedx < MSG_HEAD_SIZE ) return -3;

    u16 header_cnt = 0;
    for( i = 0; i < ( ptr_indedx-2 ); i++ )
    {
        if( *(ptr+i) != 0x00 )
        {
            bAllZeroByte = FALSE;
        }
        if(	 (*(ptr + i + 0 ) == COM_BUS_HEAD_RX_1) && (*(ptr +i +  1 ) == COM_BUS_HEAD_RX_2 )  )
        {
            header_cnt++;
            if(header_cnt > header_index)
            {
                bHeadChkFlag = TRUE;
                nhead_index = i;
                break;
            }
            else
            {
                ptr[i] = 0;
            }
        }
    }    

    if ( bAllZeroByte ) return -2; 
    if ( !bHeadChkFlag ) return -1;
    if( nhead_index + (MSG_HEAD_BIG_SIZE+MSG_LEN_SIZE) > ptr_indedx ) return 0;

    if (g_nEncryptionMode == NO_ENCRYPTION)
    {
        rx_count_size = StreamsToBigEndU16(ptr + nhead_index + MSG_HEAD_BIG_SIZE);
        rx_count_size += (MSG_HEAD_BIG_SIZE + MSG_LEN_SIZE + MSG_CHK_SIZE);
    }
    else
    {
        if ((*(ptr + nhead_index + 2) == KID_INIT_ENCRYPTION) ||
            (*(ptr + nhead_index + 2) == KID_SET_RELEASE_ENC_KEY) ||
            (*(ptr + nhead_index + 2) == KID_SET_DEBUG_ENC_KEY) ||
            get_ota_data_status() > NO) //wait add ota cmd.

        {
            rx_count_size = StreamsToBigEndU16(ptr + nhead_index + MSG_HEAD_BIG_SIZE);
            rx_count_size += (MSG_HEAD_BIG_SIZE + MSG_LEN_SIZE + MSG_CHK_SIZE);
        }
        else
        {
            rx_count_size = StreamsToBigEndU16(ptr + nhead_index + MSG_AES_HEAD_SIZE);
            rx_count_size += MSG_AES_HEAD_TAIL_SIZE;
        }
    }
    return rx_count_size+nhead_index;
}
#endif

void kdp_comm_receive_data(void)
{
    u32 write_idx = kdp_uart_GetWriteIndex(stCom_type.dev_id);
    u32 read_idx = kdp_uart_GetReadIndex(stCom_type.dev_id);
    
    if (write_idx != read_idx)
    {
        // dbg_msg_console("write_idx:%d, read_idx:%d", write_idx, read_idx);

        u32 buf_size = kdp_uart_GetRxBufSize(stCom_type.dev_id);
        u32 len;
        if(write_idx > read_idx)
        {
            len = write_idx-read_idx;
            memcpy(&stCom_type.parser_buffer[stCom_type.parser_cnt], &stCom_type.rx_buffer[read_idx], len);
            stCom_type.parser_cnt += len;
        }
        else
        {
            len = buf_size-read_idx;
            memcpy(&stCom_type.parser_buffer[stCom_type.parser_cnt], &stCom_type.rx_buffer[read_idx], len);
            stCom_type.parser_cnt += len;
            if(write_idx > 0)
            {
                memcpy(&stCom_type.parser_buffer[stCom_type.parser_cnt], &stCom_type.rx_buffer[0], write_idx);
                stCom_type.parser_cnt += write_idx;
            }
        }
        kdp_uart_SetReadIndex(stCom_type.dev_id, write_idx);
#ifdef DEV_PKT_LOG_DETAIL
        dbg_msg_nocrlf("kdp_comm_receive_data: ");
        for(u32 i=0; i< stCom_type.parser_cnt; i++) {
            dbg_msg_nocrlf("%02x ", stCom_type.parser_buffer[i]);
        }
        dbg_msg_nocrlf("\r\n");
#endif
    }
}

//waiting for new data coming and processing it
//this is a shared function
BOOL kl520_com_wait_receive(void)
{
    BOOL ret = FALSE;
    //check Rx busy
#if ( !(CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN) ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
    UINT8   timeout_cnt = 0, time_out_th = 10;
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
#if 0 //UART_PACKET_SIZE_CHECK_EN == YES
    UINT16  expection_data_lens = 0;
    UINT8   wait_expection_part_count = 0;
#endif   

    //Uart Busy check
    //go to Uart interface, check bus is busy or idle
    if (stCom_type.com_type == DEV_UART)
    {
#if 1
        static s16 expected_length = 0;
        static s16 last_expected_length = 0;
        static u32 last_parset_cnt = 0;
        static u32 last_tick = 0;
        if (last_parset_cnt != stCom_type.parser_cnt)
        {
            last_parset_cnt = 0;
            last_expected_length = 0;
        }
        kdp_comm_receive_data();

        u32 tick = osKernelGetTickCount();
        if (stCom_type.parser_cnt > 0)
        {
            if ((last_parset_cnt != stCom_type.parser_cnt) || (last_expected_length != expected_length))
            {
                if (stCom_type.parser_cnt >= MSG_ALL_SIZE)
                {
                    if (0 == expected_length)
                    {
                        expected_length = kneron_lwcom_packet_size(&stCom_type, 0);
                    }

                    if (expected_length > 0)
                    {
                        if (stCom_type.parser_cnt >= expected_length)
                        {
                            expected_length = 0;
                            ret = TRUE;
                        }
                    }
                    else
                    {
                        expected_length = 0;
                    }
                }
                last_tick = tick;
                last_expected_length = expected_length;
            }
            else
            {
                if((tick - last_tick) > 20)
                {
                    expected_length = kneron_lwcom_packet_size(&stCom_type, 1);
                    if (expected_length <= 0)
                    {
                        stCom_type.parser_cnt = 0;
                        expected_length = 0;
                        last_expected_length = 0;
                    }
                }
            }
        }
        last_parset_cnt = stCom_type.parser_cnt;
        
#else
        //protect and receive all data
        u32 last_rx_index = *stCom_type.rx_buffer_index;
        while( ncount != *stCom_type.rx_buffer_index ){
            ncount = *stCom_type.rx_buffer_index;
            osDelay(20);
            *stCom_type.rx_buffer_index = last_rx_index + kdp_uart_GetRxCount( stCom_type.dev_id );
#if UART_PACKET_SIZE_CHECK_EN == YES
            if(  *stCom_type.rx_buffer_index >= (MSG_HEAD_BIG_SIZE+MSG_LEN_SIZE) && expection_data_lens == 0 )
            {
                s16 check_size = kneron_lwcom_packet_size( &stCom_type, 0 );

                if( check_size > 0 )
                {
                    expection_data_lens = check_size;
                }
                dbg_msg_com("expec : %d, ncount: %d, %d", expection_data_lens, *stCom_type.rx_buffer_index, ncount ); 
            } 
            if( ncount == *stCom_type.rx_buffer_index ) wait_expection_part_count++; 
            else wait_expection_part_count = 0;
#endif

            if( get_mass_data_status() == NO )
            {
                if(*stCom_type.rx_buffer_index > MAX_BUF_SIZE)
                    *stCom_type.rx_buffer_index = 0;
            }
            else
            {
                if(*stCom_type.rx_buffer_index > MAX_BUF_SIZE_OTA_MAX)
                    *stCom_type.rx_buffer_index = 0;
            }
        }
        if( *stCom_type.rx_buffer_index > 0 ){
            #if( COM_DEBUG_LOG_EN == YES)
            dbg_msg("[com_bus] uart rx receive done ");
            #endif
            ret = TRUE; //data at least 1 Byte
        }
#endif
    }

#elif ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK )
    //USB Busy check
    //go to USB interface, check bus is busy or idle
    if ( stCom_type.com_type == DEV_USB )
    {

    }
#elif ( CFG_COM_BUS_TYPE&COM_BUS_I2C_MASK )
    //I2C Busy check
    //go to I2C interface, check bus is busy or idle
    if (stCom_type.com_type == DEV_I2C )
    {

    }
#endif

    return ret; //no data receive
}

void print_hex_long_data(u8* ptr, u16 nIdxS, u16 nIndE, u16 nMaxPrt)
{
    int i;

    if ( nIndE-nIdxS < nMaxPrt )
    {
        //Bypass
    }
    else
    {
        nIndE = nIdxS+nMaxPrt;
    }

    for (i = nIdxS; i < nIndE; i++)
    {
        kdp_printf("0x%02X ", *( ptr+i ));
    }
    kdp_printf("\n");
}

UINT16 kneron_lwcom_packet_analyze( struct st_com_type *st_com )  //uart analyze  --lmm-edit
{
    uint32_t  i = 0;
    uint8_t   *ptr = st_com->parser_buffer;
    uint32_t  ptr_indedx = st_com->parser_cnt;//last id
    uint32_t  nhead_index = 0xFFFFFFFF;
    uint32_t  ncheck_sum = 0;
    BOOL    bHeadChkFlag = FALSE;
    
#ifdef DEV_PKT_LOG_DETAIL
    dbg_msg_nocrlf("%d -- Rx: ", osKernelGetTickCount());
    for(u8 i=0; i< ptr_indedx; i++) {
        dbg_msg_nocrlf("%02x ", ptr[i]);
    }
    dbg_msg_nocrlf("\r\n");
#endif

    if( ( ptr_indedx < 4 ) || ( ptr == NULL ) ) //may be edit
    {
        st_com->parser_end = ptr_indedx;
        return COM_BUS_PACKAGE_SIZE_ERROR;
    }

    for( i = 0; i < ( ptr_indedx-2 ); i++ )
    {
        if(  (*(ptr + i + 0 ) == COM_BUS_HEAD_RX_1) && (*(ptr +i +  1 ) == COM_BUS_HEAD_RX_2 )  )
        {
            bHeadChkFlag = TRUE;

            nhead_index = i;
            *(ptr + i + 0 ) = 0;
            *(ptr + i +  1 ) = 0;
            break;
        }
    }

    if ( !bHeadChkFlag )
    {
        st_com->parser_end = ptr_indedx;
        return COM_BUS_HEAD_CHECK_ERROR;
    }

    if ( g_nEncryptionMode == NO_ENCRYPTION )
    {
        st_com->cmd         = (*( ptr+nhead_index+MSG_HEAD_SIZE ));//1 byte cmd
        st_com->data_len    = StreamsToBigEndU16(ptr+nhead_index+MSG_HEAD_BIG_SIZE);//2 byte size
        st_com->data_start_index = nhead_index+MSG_HEAD_BIG_SIZE+MSG_LEN_SIZE;
        st_com->checksum    = (*( ptr+nhead_index+5+ st_com->data_len ));

        //Check data length
        if ( ptr_indedx-MSG_HEAD_BIG_SIZE-MSG_LEN_SIZE-MSG_CHK_SIZE < st_com->data_len )
        {
            st_com->parser_end = ptr_indedx;
            return COM_BUS_DATA_SIZE_ERROR;
        }

        ncheck_sum = checksum_cal(ptr+nhead_index+MSG_HEAD_SIZE, 0, st_com->data_len+MSG_CMD_BIG_SIZE);
        dbg_msg_com("cmd:0x%02x,size:0x%04x,data_idx:0x%02x,checksum:0x%02x,self-checksum:0x%02x",st_com->cmd,st_com->data_len,st_com->data_start_index,st_com->checksum,ncheck_sum);

        if( st_com->checksum != ncheck_sum  )
        {
            g_nCheckSum_Error++;
            if(g_nCheckSum_Error >= 5)
            {
                g_bKID_SetKey = FALSE;
            }
            dbg_msg_console("checksum error");
            st_com->parser_end = ptr_indedx;
            return COM_BUS_CHECK_SUM_ERROR;
        }
        else
        {
            g_nCheckSum_Error = 0;
        }

        if ((KID_INIT_ENCRYPTION == st_com->cmd)
            || (KID_SET_RELEASE_ENC_KEY == st_com->cmd)
            || (KID_SET_DEBUG_ENC_KEY == st_com->cmd))
        {
            g_bKID_SetKey = TRUE;
        }

        st_com->parser_end = nhead_index + MSG_ALL_SIZE + st_com->data_len;

    }
#if ( ENCRYPTION_MODE != NO_ENCRYPTION )
    else
    {

        if ( ( *(ptr+nhead_index+2) == KID_INIT_ENCRYPTION ) ||
            ( *(ptr+nhead_index+2) == KID_SET_RELEASE_ENC_KEY ) ||
            ( *(ptr+nhead_index+2) == KID_SET_DEBUG_ENC_KEY ) ||
            get_ota_data_status() > NO )//wait add ota cmd.
        {
            st_com->cmd         = (*( ptr+nhead_index+MSG_HEAD_SIZE ));//1 byte cmd
            st_com->data_len    = StreamsToBigEndU16(ptr+nhead_index+MSG_HEAD_BIG_SIZE);//2 byte size
            st_com->data_start_index = nhead_index + MSG_HEAD_BIG_SIZE+MSG_LEN_SIZE;
            st_com->checksum    = (*( ptr+nhead_index+5+ st_com->data_len ));

            //Check data length
            if ( ptr_indedx-MSG_HEAD_BIG_SIZE-MSG_LEN_SIZE-MSG_CHK_SIZE < st_com->data_len )
            {
                st_com->parser_end = ptr_indedx;
                return COM_BUS_DATA_SIZE_ERROR;
            }

            ncheck_sum = checksum_cal(ptr+nhead_index+MSG_HEAD_SIZE, 0, st_com->data_len+MSG_CMD_BIG_SIZE);
            dbg_msg_com("cmd:0x%02x,size:0x%04x,data:0x%02x,checksum:0x%02x,self-checksum:0x%02x",st_com->cmd,st_com->data_len,st_com->data_start_index,st_com->checksum,ncheck_sum);

            if( st_com->checksum != ncheck_sum  )
            {
                g_nCheckSum_Error++;
                if(g_nCheckSum_Error >= 5)
                {
                    g_bKID_SetKey = FALSE;
                }
                dbg_msg_console("checksum error");
                st_com->parser_end = ptr_indedx;
                return COM_BUS_CHECK_SUM_ERROR;
            }
            g_nCheckSum_Error = 0;
            g_bKID_SetKey = TRUE;
            
            st_com->parser_end = nhead_index + MSG_ALL_SIZE + st_com->data_len;
        }
#ifdef KID_SOFT_RESET
        else if ( *(ptr+nhead_index+2) == KID_SOFT_RESET )
        {
            st_com->cmd = KID_SOFT_RESET;   //1 byte cmd
            st_com->data_len    = StreamsToBigEndU16(ptr+nhead_index+3);//2 byte size
            st_com->data_start_index = nhead_index + sizeof(st_com->head) + sizeof( st_com->cmd ) + sizeof(st_com->data_len);
            st_com->checksum    = (*( ptr+nhead_index+5+ st_com->data_len ));
            ncheck_sum = checksum_cal(ptr+nhead_index+2, 0, st_com->data_len + 3);
            if( st_com->checksum != ncheck_sum  )
            {
                g_nCheckSum_Error++;
                if(g_nCheckSum_Error >= 5)
                {
                    g_bKID_SetKey = FALSE;
                }
                dbg_msg_console("checksum error");
                st_com->parser_end = ptr_indedx;
                return COM_BUS_HEAD_CHECK_ERROR;
            }
            st_com->parser_end = nhead_index + MSG_ALL_SIZE + st_com->data_len;
        }
#endif
        else
        {
            //dbg_msg_console("dec analisize coming in ");
            Msg_AesEncryptData data;
            data.body_size = StreamsToBigEndU16(ptr+nhead_index+MSG_AES_HEAD_SIZE);
            
            if( data.body_size < 3 )
            {
                dbg_msg_com("Data size error");
                st_com->parser_end = ptr_indedx;
                return COM_BUS_DATA_SIZE_ERROR;
            }
            
            //Check data length
            if ( ptr_indedx-MSG_AES_HEAD_TAIL_SIZE < data.body_size )
            {
                st_com->parser_end = ptr_indedx;
                return COM_BUS_DATA_SIZE_ERROR;
            }

            data.checknum =  *(ptr+nhead_index+data.body_size+MSG_AES_HEAD_BIG_SIZE);

            st_com->checksum   = data.checknum;
    //        dbg_msg_console("body data:");

            ncheck_sum = checksum_cal(ptr+nhead_index+MSG_AES_HEAD_BIG_SIZE, 0, data.body_size);

            //dbg_msg_console("data.body_size:0x%04x, data.checknum:0x%02x, self-checksum:0x%02x",data.body_size, data.checknum, ncheck_sum);
            if( st_com->checksum != ncheck_sum  )
            {
                g_nCheckSum_Error++;
                if(g_nCheckSum_Error >= 5)
                {
                    g_bKID_SetKey = FALSE;
                }
                dbg_msg_console("checksum error");
                st_com->parser_end = ptr_indedx;
                return COM_BUS_CHECK_SUM_ERROR;
            }
            g_nCheckSum_Error = 0;

    //        kdp_printf("[-----Cipher text-----]\n");
    //        print_hex_long_data(ptr, 0, MSG_HEAD_SIZE+MSG_LEN_SIZE+data.body_size+MSG_CHK_SIZE, 48);

            memset(msg_dec, 0, data.body_size);

    //        uint32_t StartTime = osKernelGetTickCount();

            // if ( g_nEncryptionMode != NO_ENCRYPTION )
            {
                PkgDecrypt(ptr+nhead_index+MSG_AES_HEAD_BIG_SIZE, data.body_size, debug_key, KEY_SIZE, msg_dec);
            }
    //        uint32_t StopTime = osKernelGetTickCount() - StartTime;
    //        dbg_msg_console("aesDecrypt %d bytes cost time:%dms", data.body_size, StopTime);

            st_com->cmd         = *msg_dec;//1 byte cmd
            st_com->data_len    = StreamsToBigEndU16(msg_dec+MSG_CMD_SIZE);//2 byte size
            st_com->data_start_index = MSG_CMD_BIG_SIZE;

            memset(ptr, 0, data.body_size+MSG_AES_HEAD_TAIL_SIZE);
            memcpy(ptr, msg_dec, MSG_CMD_BIG_SIZE+st_com->data_len);

            st_com->parser_end = nhead_index+data.body_size + MSG_AES_HEAD_TAIL_SIZE;

#ifdef DEV_PKT_LOG_DETAIL
        kdp_printf("[-----Plain Text-----]\n");
        print_hex_long_data(ptr, 0, MSG_CMD_SIZE+MSG_LEN_SIZE+st_com->data_len, 48);
#endif

            dbg_msg_com("st_com->cmd:0x%02x,st_com->data_len:0x%04x",st_com->cmd,st_com->data_len);
        }
    }
#endif
    return COM_BUS_PACKET_OK;   //exist good data
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

static void kl520_com_parser(void)  //lmm-edt parser func.
{
    kl520_com_response_Done_check();//work around for uart rx with no tx.
    // if( stCom_type.no_head_tail_en == 1  )
    // {
    //     _kneron_host_com_parser( &stCom_type );      //for future work
    //     stCom_type.no_head_tail_en = 0;
    // }
    // else
    // {
    if ((_user_com_ops) && (_user_com_ops->parser))
        return _user_com_ops->parser(&stCom_type);
    else
        return kneron_com_parser( &stCom_type );//lmm
        //Kneron API
        //for add customer parser API
    //}
}

void kl520_com_reg_user_ops(kl520_com_user_ops *ops)
{
    _user_com_ops = ops;
}

s32 kdp_comm_check_header(s32 start_idx)
{
    if ((start_idx + 1) < stCom_type.parser_cnt)
    {
        u8 *parser_data = stCom_type.parser_buffer;

        for(s32 i = start_idx; i < ( stCom_type.parser_cnt-1 ); i++ )
        {
            if(  (parser_data[i] == COM_BUS_HEAD_RX_1) && (parser_data[i+1] == COM_BUS_HEAD_RX_2 )  )
            {
                return i;
            }
        }
    }

    return -1;
}

void kdp_comm_post_process(u16 result)
{
    s32 head_idx = kdp_comm_check_header((COM_BUS_PACKET_OK == result)?stCom_type.parser_end:0);
    if (head_idx >= 0)
    {
        stCom_type.parser_end = (u32)head_idx;
    }

    // dbg_msg_console("parser_cnt:%d, parser_end:%d", stCom_type.parser_cnt, stCom_type.parser_end);
    if(stCom_type.parser_cnt > stCom_type.parser_end)
    {
        memcpy(stCom_type.parser_buffer, &stCom_type.parser_buffer[stCom_type.parser_end], stCom_type.parser_cnt - stCom_type.parser_end);
        stCom_type.parser_cnt = stCom_type.parser_cnt - stCom_type.parser_end;
    }
    else
    {
        stCom_type.parser_cnt = 0;
    }
    stCom_type.parser_end = 0;
}

// extern void get_uart1_rx_cnt(void);
extern void sample_switch_rbg_nir(void);
u32 last_hb_tick = 0;
extern osMutexId_t mutex_rsp_msg;
void kl520_com_thread(void)
{
    UINT16	npacket_result = 0;
    #if( COM_DEBUG_LOG_EN == YES)
    //dbg_msg_console("[com_bus] communication start~lmm-edit");//lmm-edit
    #endif

    kl520_measure_stamp(E_MEASURE_SYS_READY);
    send_system_ready_note_msg();//zcy first send ready
	  dbg_msg_console_zhian("send_system_ready_note_msg");

    init_user_com_thread();
    kl520_api_fdfr_init_thrd();

    if(mutex_rsp_msg == NULL) mutex_rsp_msg = osMutexNew(NULL);
    
    kl520_api_tasks_init_wait_ready();

    u32 disable_count = 0;

    while(1)
    {
        osDelay(8);//30
        //step1: wait and receive data
        if(kl520_com_wait_receive())
        {
            g_bAutoPowerOff = FALSE;
            g_nAutoPowerOffCnt = 0;
            disable_count = 0;

            //data analyze!   => user_com.c and user_com.h, need to check host_com or customer_com
            if( (npacket_result = kl520_com_analyze() ) == COM_BUS_PACKET_OK )
            {
                //dbg_msg_console("[com_bus] communication analyze OK ");
                //parser and trigger customer API
                osThreadSetPriority(com_bus_tid,osPriorityAboveNormal5);
                //parser and trigger customer API
                kl520_com_parser();//de shi man uart com-lmm-edit
                osThreadSetPriority(com_bus_tid,osPriorityNormal);
            }
            else
            {
                osThreadSetPriority(com_bus_tid,osPriorityAboveNormal5);
#if ( ENCRYPTION_MODE != NO_ENCRYPTION )
                if ( npacket_result == COM_BUS_ENCRYPTION_ERROR )
                {
                    send_data_error_reply_msg(MR_FAILED_NO_ENCRYPT);
					dbg_msg_algo("npacket_result: %#x", npacket_result);
                    // send_communication_abnormal_reply_msg(MR_FAILED_NO_ENCRYPT, npacket_result);
                }
                else
#endif
                {
                    if (npacket_result != COM_BUS_PACKAGE_SIZE_ERROR)
                    {
                        send_data_error_reply_msg(MR_REJECTED);
                    }
					dbg_msg_algo("npacket_result: %#x", npacket_result);
                    // send_communication_abnormal_reply_msg(MR_REJECTED, npacket_result);
                }
                osThreadSetPriority(com_bus_tid,osPriorityNormal);
                #if( COM_DEBUG_LOG_EN == YES)
               // dbg_msg("[com_bus] communication analyze fail ");
							  dbg_msg_console("[com_bus] communication analyze fail ");
                #endif
                //nee tp modify this!!!
                //kneron_lwcom_packet_response_w_tx_buffer( &stCom_type, (UINT8 *)&npacket_result, sizeof(npacket_result) );
                //kl520_com_response( &stCom_type );
                kl520_com_response_Done_check();
            }
            kdp_comm_post_process(npacket_result);
        #if( COM_DEBUG_LOG_EN == YES)
        //dbg_msg("===========");
        #endif
        }
        else
        {
            g_bAutoPowerOff = TRUE;
            disable_count++;

#if ( ENABLE_AUTO_POWER_OFF_MODE == YES )
            g_nAutoPowerOffCnt = osKernelGetTickCount();
            if ( ( ( g_bAutoPowerOff == TRUE ) 
#if (ENCRYPTION_MODE > 0)
                && (g_bKID_SetKey == FALSE) 
#endif
                && ( g_nAutoPowerOffCnt > ( AUTO_POWER_OFF_TIME_CNT ) ) 
                && ( user_com_thread_event_get() != USER_COM_THREAD_EVENT_NON_OVERWRITABLE ) )
                || (disable_count > 5000) ) // 2.5min
            {
                dbg_msg_console("[%d]AUTO POWER OFF", osKernelGetTickCount());
                user_com_event_power_off();
            }

//            if(user_app_GetOtaStatus() < YES && user_app_GetSetUSBStatus(2) == 0 && !flag_task_busy && flag_auto_powerOff == 1 && time_cnt > (CNT_USER_AUTO_POWEROFF+power_off_delay_time)) //5000ms/8=625  real time is 7.87S
//            {
//            }
#endif
        }

#if (CFG_KL520_VERSION == KL520A)
        u32 cur_tick = osKernelGetTickCount();
        int diff = cur_tick - last_hb_tick;
        if((cur_tick < last_hb_tick) || diff > 2500) { //if loop again or 2.5s passed.
            //last_hb_tick = cur_tick;
            //send hb.
//            dbg_msg_console("YG --- sending HB msg to MCU. %d.", diff);
            send_heartbeat_msg();
        }
#endif
    }

}
#endif
//===============================================
extern void Drv_OTA_Thread( void );
void kl520_com_bus_init(void)
{
    Drv_OTA_Thread();
}

//#endif
#endif
#endif

