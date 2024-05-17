#include "sample_app_spi.h"

#if 0

#include <string.h>
#include "dbg.h"
#include "framework/event.h"
#include "framework/v2k_color.h"
#include "board_ddr_table.h"
#include "kdp520_ssp.h"
#include "kl520_include.h"
#include "kl520_api_ssp.h"
#include "kl520_com.h"
#include "delay.h"
#include "flash.h"
#include "sample_app_console.h"


#define MAX_MSG_LEN  	(5)
#define MAX_SAVE_LEN 	(30)

// typedef struct _msg_ {
//     u8 flag;
// 	u32 len;
//     u8 data[MAX_SAVE_LEN];
// } msg_kdp;


UINT32 rx_temp ;
//static msg_kdp msg[MAX_MSG_LEN];
UINT16	gmsg_counter = 0;
osThreadId_t    kdp_spi_app_thread_tid;
volatile UINT8  nspi_temp_buffer[300];
UINT32 Xor_error_count = 0;
UINT32 Xor_error_index = 0;
UINT32 restore_count = 0;
UINT32 gflag = 0;
//extern struct st_ssp_spi	driver_ssp_ctx;

UINT8 Drv_XOR_clc( volatile UINT8 *buf, UINT8 len )
{
    UINT16	i = 0;
    UINT8 ntemp = 0;

    ntemp = *(buf+0);
    for( i= 1 ; i < len ; i++ )
    {
        ntemp ^= *(buf +i);
    }
    return 	ntemp;
}

void Drv_spi_dummy_rx(void)
{
    *driver_ssp_ctx.Rx_buffer_index = 0;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x88;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x88;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x88;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x5A;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x00;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x02;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x88;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x89;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x59;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x88;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;
    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x88;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;
    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x88;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x50;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x00;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x01;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x88;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0xD9;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;


    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x88;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x70;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x00;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x05;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x10;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x02;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;
    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x02;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;
    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x02;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;
    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x02;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;
    *(driver_ssp_ctx.Rx_buffer + *driver_ssp_ctx.Rx_buffer_index) = 0x65;
    *driver_ssp_ctx.Rx_buffer_index = *driver_ssp_ctx.Rx_buffer_index+1;

}

void kdp_slave_com_app_thread(void *arg)
{
    UINT32	volatile g_rx_index ;
#if ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP1 ) == COM_BUS_SSP1 )
    UINT32  len;
    UINT32  XOR;
    UINT8   noffset_ptr = 0;
    UINT32   noffset_total = 0;
    UINT16  i = 0 , k = 0;
    int kk = 0;
#endif

    memset(&msg, 0, sizeof(msg));
//    if( kl520_api_ssp_spi1_init(e_spi_init_slave) != 1 )
//    {
//        return;
//    }

#if ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP1 ) == COM_BUS_SSP1 )
    kl520_api_ssp_spi1_slave_req_init();

    kl520_api_ssp_spi1_clear_rx_hw();
    kl520_api_ssp_spi1_clear_tx_hw();
    kl520_api_ssp_spi1_clear_rx_buff_size(&driver_ssp_ctx);
    kl520_api_ssp_spi1_clear_tx_buff_size(&driver_ssp_ctx);
    kl520_api_ssp_spi1_clear_tx_current_buff_size();
    kl520_api_ssp_spi1_clear_tx_done_flag();
    kl520_api_ssp_spi1_enable(&driver_ssp_ctx);

    kl520_api_ssp_spi1_slave_inactive();
    delay_ms( 5 );
    kl520_api_ssp_spi1_slave_active();
    delay_ms( 1 );
    kl520_api_ssp_spi1_slave_inactive();

//    Drv_spi_dummy_rx( );
#endif

    driver_ssp_ctx.Rx_tempbuffer_index = 0;
    while(1)
    {
        //================ 520 Rx data  ================
#if ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP1 ) == COM_BUS_SSP1 )
        while( *driver_ssp_ctx.Rx_buffer_index == 0 )
        {
            osDelay(20);
        }
        while( ( kl520_api_ssp_spi1_receive(&driver_ssp_ctx) == 0 ) )
        {
        }

#if 0
        rx_temp = *driver_ssp_ctx.Rx_buffer_index;
        memcpy( nspi_temp_buffer, (UINT8 *)driver_ssp_ctx.Rx_buffer, rx_temp);		//ping-pong
        kl520_api_ssp_spi1_clear_rx_buff_size(&driver_ssp_ctx);
#else
        rx_temp = driver_ssp_ctx.Rx_tempbuffer_index;
        for( k=0; k<rx_temp; k++ )
        {
            nspi_temp_buffer[k] = *(driver_ssp_ctx.Rx_tempbuffer+k);
        }
        //memcpy( nspi_temp_buffer, (UINT8 *)driver_ssp_ctx.Rx_tempbuffer, rx_temp);        //ping-pong
        driver_ssp_ctx.Rx_tempbuffer_index = 0;
#endif

        dbg_msg("rx_temp : %d\n", rx_temp);
        dbg_msg("=====================\n");

//        dbg_msg("=====Receive Done\n");
//        for( k = 0 ; k < rx_temp; k++ )
//        {
//          dbg_msg(" %x ",nspi_temp_buffer[ k + 0 ] );
//        }

        for( k = 0 ; k < rx_temp; k++ )
        {
#if 0

//            dbg_msg("loop: %d\n", k);
            if( nspi_temp_buffer[ k + 0 ] < FDN_CHECK || nspi_temp_buffer[ k + 0 ] > FDN_MAX )
            {
                dbg_msg(" data cont: %d \n", k, nspi_temp_buffer[ k + 0 ] );
                continue;
            }
            noffset_ptr = k ;
            len = ( nspi_temp_buffer[noffset_ptr+1] <<8 | nspi_temp_buffer[noffset_ptr+2] );
            noffset_total = len + 2  + 1 + 1;   //head + data_len + length_width + xor =  next packet ptr
            noffset_total--;                    // this data ptr

            if( noffset_total > rx_temp )
            {
                Xor_error_index++;
                dbg_msg(" Data index error k:%d, data:0x%x \n", noffset_ptr, nspi_temp_buffer[ noffset_total ] );
                //while(1);
                continue;
            }
            XOR = Drv_XOR_clc( nspi_temp_buffer+noffset_ptr , noffset_total  );

            if(  XOR != nspi_temp_buffer[ noffset_total + k - 0 ])
            {
                Xor_error_count++;
//                dbg_msg(" Xor error: %d, XOR: 0x%x, 0x%x \n", k, XOR, nspi_temp_buffer[ noffset_total + k - 0 ] );
                for(int kk=k;kk<=noffset_total; kk++)
                {
                    dbg_msg("rx_temp:%d \n",rx_temp );
                    dbg_msg("pte_head %d , ptr_total:%d\n",  noffset_ptr, noffset_total );
                    dbg_msg("error data: %d, data: 0x%x, XOR: 0x%x, \n", kk, nspi_temp_buffer[ noffset_ptr + kk ] , XOR );
                }
                for(int jj= 0;jj< rx_temp; jj++)
                {
                    dbg_msg("raw data idx: %d, data: 0x%x \n", jj, nspi_temp_buffer[ jj ]);
                }
            }
            else
            {
//                dbg_msg(" Xor head OK :%d \n", k);
                gmsg_counter++;
                for( int j = 0 ; j < MAX_MSG_LEN ; j++ )
                {
                    if(!msg[j].flag)
                    {
                        memcpy(msg[j].data, (UINT8 *)&nspi_temp_buffer[noffset_ptr] ,len + 4);
                        msg[j].flag = 1;
                        msg[j].len = len+4;
                        break;
                    }
                }
            }
            k += noffset_total;
#else

            if( nspi_temp_buffer[ k + 0 ] < FDN_CHECK || nspi_temp_buffer[ k + 0 ] > FDN_MAX )
            {
            dbg_msg(" data cont: %d \n", k, nspi_temp_buffer[ k + 0 ] );
            continue;
            }
            noffset_ptr = k ;
            len = ( nspi_temp_buffer[noffset_ptr+1] <<8 | nspi_temp_buffer[noffset_ptr+2] );
            noffset_total = len + 2  + 1 + 1;   //head + data_len + length_width + xor =  next packet ptr
            noffset_total--;                    // this data ptr

            if(  (noffset_ptr + noffset_total) >= rx_temp )
            {
                restore_count++;
                //restore to backup buffer
                driver_ssp_ctx.Rx_tempbuffer_index = 0;
                for( kk = k ; kk <rx_temp ; kk++ )
                {
                    dbg_msg("== Backup index:%d, data: 0x%x ==\n", kk, nspi_temp_buffer[ kk ] );
                    *(driver_ssp_ctx.Rx_tempbuffer+driver_ssp_ctx.Rx_tempbuffer_index  ) = * ( nspi_temp_buffer+kk );
                    driver_ssp_ctx.Rx_tempbuffer_index++;
                }
                Xor_error_index++;
                break;
            }
            XOR = Drv_XOR_clc( nspi_temp_buffer+noffset_ptr , noffset_total  );

            if(  XOR != nspi_temp_buffer[ noffset_total + k - 0 ])
            {

                Xor_error_count++;
                dbg_msg(" Xor error index : %d, XOR: 0x%x, 0x%x \n", k, XOR );
//                for( kk=k;kk<=noffset_total; kk++)
//                {
//                    dbg_msg("rx_temp:%d \n",rx_temp );
//                    dbg_msg("pte_head %d , ptr_total:%d\n",  noffset_ptr, noffset_total );
//                    dbg_msg("error data: %d, data: 0x%x, XOR: 0x%x, \n", kk, nspi_temp_buffer[ noffset_ptr + kk ] , XOR );
//                }
//                for( i= 0;i< rx_temp; i++)
//                {
//                    dbg_msg("raw data idx: %d, data: 0x%x \n", i, nspi_temp_buffer[ i ]);
//                }

            }
            else
            {
                //                dbg_msg(" Xor head OK :%d \n", k);
                gmsg_counter++;
                for( int j = 0 ; j < MAX_MSG_LEN ; j++ )
                {
                    if(!msg[j].flag)
                    {
                        memcpy(msg[j].data, (UINT8 *)&nspi_temp_buffer[noffset_ptr] ,len + 4);
                        msg[j].flag = 1;
                        msg[j].len = len+4;
                        break;
                    }
                }
            }
            k += noffset_total;
            dbg_msg("final index  %d ",k );
#endif
        }

        //data check
        for( int j = 0 ; j < MAX_MSG_LEN ; j++ )
        {
            if( msg[j].flag == 1  )
            {
                dbg_msg("flag index 0x%x \n", j );
                for( i = 0 ; i < msg[j].len; i++ ){
                dbg_msg("index:%d, data: 0x%x \n", i, msg[j].data[i] );
                }
            }
        }
#endif
    }
}

void slave_com_init( void )
{
    osThreadAttr_t attr = {
        .stack_size = 256,
        .priority = osPriorityNormal
    };    
    
#if KL520_COM_RESPONSE_BY_ADDITIONAL_IO == YES
    kl520_com_init(KL520_COM_HAS_ADDITIONAL_IO);
#else
    kl520_com_init(KL520_COM_NORMAL);
#endif

    kdp_spi_app_thread_tid = osThreadNew( kdp_slave_com_app_thread, NULL, &attr );
}
#endif
