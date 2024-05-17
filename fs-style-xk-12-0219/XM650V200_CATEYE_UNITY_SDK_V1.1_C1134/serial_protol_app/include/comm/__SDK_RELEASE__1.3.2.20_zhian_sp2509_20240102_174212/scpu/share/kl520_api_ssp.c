#include "board_kl520.h"

#include <stdarg.h>
#include "io.h"
#include "pinmux.h"
#include "framework/init.h"
#include "framework/v2k_image.h"
#include "framework/framework_driver.h"
#include "framework/event.h"
#include "kdp520_ssp.h"
#include "kdp_memory.h"
#include "kl520_api_ssp.h"
#include "kl520_include.h"
#include "kdp520_gpio.h"
#include "kdp520_dma.h"
#include "kdp520_lcm.h"
#include "kl520_com.h"
//-----------------------------------
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
#if (1==CFG_SSP0_ENABLE) || (1==CFG_SSP1_ENABLE)

fn_power_hook m_cb_ssp_SPI1_power_on = NULL;
fn_power_hook m_cb_ssp_SPI1_power_off = NULL;

//-----------------------------------
void kl520_api_ssp_spi1_register_hook(fn_power_hook fn_power_on, fn_power_hook fn_power_off)
{
    m_cb_ssp_SPI1_power_on = fn_power_on;
    m_cb_ssp_SPI1_power_off = fn_power_off;
}

void ssp_spi1_power_on(void)
{
    UINT32 data = 0;
    data = inw(SCU_EXTREG_PA_BASE + 0x1C);
    outw(SCU_EXTREG_PA_BASE + 0x1C, data | 0x40);
}

void ssp_spi1_power_off(void)
{
    UINT32 data = 0;
    data = inw(SCU_EXTREG_PA_BASE + 0x1C);
    data &= ~(0x40);
    outw(SCU_EXTREG_PA_BASE + 0x1C, data );
}

//---
UINT8 kl520_api_ssp_spi1_init(enum e_spi edata)
{
    if( kdp_ssp_statemachine( &driver_ssp_ctx, edata ) == e_spi_ret_init_done ){
        return 1;
    }
    return 0;
}

UINT8 kl520_api_ssp_spi1_enable(struct st_ssp_spi *stspi)
{
    if( kdp_ssp_statemachine( stspi, e_spi_enable ) == e_spi_ret_enable_done ){
        return 1;
    }
    return 0;
}

UINT8 kl520_api_SSP_SPI1_disable(struct st_ssp_spi *stspi)
{
    if( kdp_ssp_statemachine( stspi, e_spi_disable ) == e_spi_ret_disableDone ){
        return 1;
    }
    return 0;
}

UINT8 kl520_api_ssp_spi1_receive(struct st_ssp_spi *stspi)
{
    if( kdp_ssp_statemachine( stspi, e_spi_rx ) == e_spi_ret_rxbusy ){
        return 0;		//rx on-going
    }
    else{
        return 1;		//rx done
    }
}

UINT8 kl520_api_ssp_spi1_receive_xor(void)
{

    if( kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_rx_check ) == e_spi_ret_rx_xor_OK ){
            return 1;		//rx data correct
    }
    else{
            return 0;		//rx data fail
    }

}

UINT8 kl520_api_ssp_spi1_transfer(void)
{

    if( kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_tx ) == e_spi_ret_txbusy ){
            return 1;		//rx data correct
    }
    else{
            return 0;		//rx data fail
    }

}

UINT8 kl520_api_ssp_spi1_transfer_checks(void)
{

    if( kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_tx_status_check ) == e_spi_ret_txbusy )
    {
            return 0;
    }
    else
    {
            return 1;
    }

}

void kl520_api_ssp_spi1_write_tx_buff( UINT8 *src, UINT16 nlen )
{
    kdp_ssp_write_buff( &driver_ssp_ctx, src, nlen );
}


UINT16 kl520_api_ssp_spi1_get_tx_buff_size( struct st_ssp_spi *stspi )
{
    return kdp_ssp_get_tx_buf_index( stspi );
}

void kl520_api_ssp_spi1_clear_tx_buff_size( struct st_ssp_spi *stspi )
{
    kdp_ssp_clear_tx_buf_index( stspi );
}


UINT32 kl520_api_ssp_spi1_get_rx_buff_size( struct st_ssp_spi *stspi )
{
    return kdp_ssp_get_rx_buf_index( stspi );
}

void kl520_api_ssp_spi1_clear_rx_buff_size( struct st_ssp_spi *stspi  )
{
    kdp_ssp_clear_rx_buf_index( stspi );
}

UINT16 kl520_api_ssp_spi1_get_tx_current_buff_size( void )
{
    return kdp_ssp_get_tx_current_buf_index(&driver_ssp_ctx);
}


void kl520_api_ssp_spi1_clear_tx_current_buff_size( void )
{
    kdp_ssp_clear_tx_current_buf_index(&driver_ssp_ctx);
}


UINT8 kl520_api_ssp_spi1_get_tx_done_flag( void )
{
    return kdp_ssp_get_tx_done_flag( &driver_ssp_ctx );
}


void kl520_api_ssp_spi1_clear_tx_done_flag( void )
{
    kdp_ssp_clear_tx_done_flag( &driver_ssp_ctx );
}


UINT8 kl520_api_ssp_spi1_get_rx_hw_length( void )
{

    return kdp_ssp_rxfifo_valid_entries( driver_ssp_ctx.reg_base_address );

}


void kl520_api_ssp_spi1_clear_rx_hw(void)
{
    kdp_ssp_clear_rxhw( driver_ssp_ctx.reg_base_address );
}

void kl520_api_ssp_spi1_clear_tx_hw(void)
{
    kdp_ssp_clear_txhw( driver_ssp_ctx.reg_base_address );
}


UINT8 kl520_api_ssp_spi1_tx_xor(void)
{
    if( kdp_ssp_statemachine( &driver_ssp_ctx, e_spi_tx_xor ) == e_spi_ret_tx_xor_done )
    {
            return 0;
    }
    else
    {
            return 1;
    }
}

void kl520_api_ssp_spi1_slave_req_init(void)
{
    kdp_slave_request_init();
}

void kl520_api_ssp_spi1_slave_active(void)
{
    kdp_slave_request_active();
}

void kl520_api_ssp_spi1_slave_inactive(void)
{
    kdp_slave_request_inactive();
}

UINT8 kl520_api_ssp_spi1_busy_check(void)
{
    //1: is busy
    return kdp_ssp_busy( driver_ssp_ctx.reg_base_address );
}





void drv_spi_rx_test( struct st_ssp_spi *st_sspspi, UINT8 *sw_buff, UINT16 *sw_buff_index )
{
    UINT16 i =0;

    *sw_buff_index = 0;

    for( i=0; i< *st_sspspi->Rx_buffer_index; i++ )
    {
        if( i==0 )
        {
            *(sw_buff+*sw_buff_index) = *( st_sspspi->Rx_buffer + i );
            *sw_buff_index = *sw_buff_index + 1;

        }
        if( i >= 1 && i < ( (*st_sspspi->Rx_buffer_index -1) )  )
        {
            *(sw_buff+*sw_buff_index) = *( st_sspspi->Rx_buffer + 0 );
            *sw_buff_index = *sw_buff_index + 1;
        }
    }

    *st_sspspi->Tx_buffer_index = 0;
    for( i=0; i< *sw_buff_index; i++ )
    {
        *(st_sspspi->Tx_buffer + i) = *( sw_buff + i );
        *st_sspspi->Tx_buffer_index = *st_sspspi->Tx_buffer_index + 1;
    }

    kl520_api_ssp_spi1_tx_xor();

    dbg_msg_api( "xor *st_sspspi->Tx_buffer_index : %d", *st_sspspi->Tx_buffer_index );

    for( i=0; i< *st_sspspi->Tx_buffer_index; i++ )
    {

        dbg_msg_api( "xor buffer data: 0x%x", *(st_sspspi->Tx_buffer + i) );
    }

}


//-----test
UINT8 kl520_api_ssp_spi_sample(void)
{
    #define	tx_temp_size (10)
    UINT16	ncount = 0;
    UINT16 i = 0;
    UINT8  temp_buffer[tx_temp_size];
    UINT16 temp_buffer_index = 0;

    for( i=0; i< tx_temp_size; i++ )
    {
        temp_buffer[i] = i;
        temp_buffer_index++;
    }

    if( kl520_api_ssp_spi1_init(e_spi_init_slave) != 1 )
    {
        return 0;
    }

    kl520_api_ssp_spi1_clear_rx_buff_size(&driver_ssp_ctx);
    kl520_api_ssp_spi1_clear_tx_buff_size(&driver_ssp_ctx);
    kl520_api_ssp_spi1_clear_tx_current_buff_size();
    kl520_api_ssp_spi1_clear_tx_done_flag();

    kl520_api_ssp_spi1_enable(&driver_ssp_ctx);

    //dbg_msg_api( "SPI API test start...." );

    kl520_api_ssp_spi1_slave_req_init();

    kl520_api_ssp_spi1_slave_inactive();


    while(1)
    {
        kl520_api_SSP_SPI1_disable(&driver_ssp_ctx);
        kl520_api_ssp_spi1_clear_rx_hw();
        kl520_api_ssp_spi1_clear_tx_hw();
        kl520_api_ssp_spi1_clear_rx_buff_size(&driver_ssp_ctx);
        kl520_api_ssp_spi1_clear_tx_buff_size(&driver_ssp_ctx);
        kl520_api_ssp_spi1_clear_tx_current_buff_size();
        kl520_api_ssp_spi1_clear_tx_done_flag();
        kl520_api_ssp_spi1_enable(&driver_ssp_ctx);

        //================ Master Tx data ================
        ncount++;
        while( kl520_api_ssp_spi1_get_rx_buff_size(&driver_ssp_ctx) == 0 )			//check sw buffer exist at leat 1Byte data
        //while( kl520_api_ssp_spi1_get_rx_hw_length() == 0 )
        {
            delay_us( 300 );
        }

        while( ( kl520_api_ssp_spi1_receive(&driver_ssp_ctx) == 0 ) )
        {
            delay_us( 10 );
        }
        dbg_msg_api("count number : %d, Rx data number: %d \r\n", ncount, kl520_api_ssp_spi1_get_rx_buff_size(&driver_ssp_ctx) );


        //while(1);
        //================ Master Rx data ================
        //disable interrupt
        kl520_api_SSP_SPI1_disable(&driver_ssp_ctx);
        kl520_api_ssp_spi1_clear_rx_hw();
        kl520_api_ssp_spi1_clear_tx_hw();
        //kl520_api_ssp_spi1_clear_rx_buff_size(&driver_ssp_ctx);
        kl520_api_ssp_spi1_clear_tx_buff_size(&driver_ssp_ctx);
        kl520_api_ssp_spi1_clear_tx_current_buff_size();
        kl520_api_ssp_spi1_clear_tx_done_flag();

//		kl520_api_ssp_spi1_write_tx_buff( temp_buffer, temp_buffer_index );
        drv_spi_rx_test( &driver_ssp_ctx, temp_buffer, &temp_buffer_index  );


        kdp_ssp_pre_write_to_fifo( &driver_ssp_ctx, 5 );
        kl520_api_ssp_spi1_enable(&driver_ssp_ctx);

        //control GPIO
        kl520_api_ssp_spi1_slave_active();


        while( kl520_api_ssp_spi1_transfer_checks() == 0 );

        dbg_msg_api("Slave Tx done, Tx_send_size: %d \r\n", kl520_api_ssp_spi1_get_tx_current_buff_size() );


    }

    //dbg_msg_api("SPI Rx done \r\n");


}
u8* spi_tx_buf=0;
u8* spi_rx_buf=0;

void kl520_api_ssp_spi_master_init(void)
{
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
    driver_ssp_master_ctx.Tx_buffer_index = &gTx_buff_index_SP_MASTER; //need to modify 20200630
    driver_ssp_master_ctx.Rx_buffer_index = &gRx_buff_index_SP_MASTER; //need to modify 20200630
    kdp_ssp_statemachine( &driver_ssp_master_ctx, e_spi_init_master );         //need to modify 20200630
    kdp_ssp_clear_rxhw( driver_ssp_master_ctx.reg_base_address );
    kdp_ssp_clear_txhw( driver_ssp_master_ctx.reg_base_address );
    kdp_ssp_clear_rx_buf_index( &driver_ssp_master_ctx );
    kdp_ssp_clear_tx_buf_index( &driver_ssp_master_ctx );
    kdp_ssp_clear_tx_current_buf_index(&driver_ssp_master_ctx);
    kdp_ssp_clear_tx_done_flag(&driver_ssp_master_ctx);
    kdp_ssp_statemachine( &driver_ssp_master_ctx, e_spi_disable );     //need to modify 20200630
    kdp_ssp_statemachine( &driver_ssp_master_ctx, e_spi_enable );      //need to modify 20200630
    if(spi_tx_buf==0)
    {
        spi_tx_buf = (u8*)kdp_ddr_reserve(320*240*2);
        spi_rx_buf = (u8*)kdp_ddr_reserve(4096);
    }
#endif
}


//font related function
void font_read_Chinese( struct st_ssp_spi *st_spi, UINT16 raw_word )
{
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
    UINT16 BaseAdd=0x00;
    UINT8 raw_word_H = (raw_word&0xFF00)>>8;
    UINT8 raw_word_L = (raw_word&0x00FF);
    UINT32  Address = 0;

    if(raw_word_H >=0xA1 && raw_word_H <= 0Xa9 && raw_word_L >=0xA1)
    {
        Address =( (raw_word_H - 0xA1) * 94 + (raw_word_L - 0xA1))*32+ BaseAdd;
    }
    else if(raw_word_H >=0xB0 && raw_word_H <= 0Xf9 && raw_word_L >=0xA1)
    {
        Address = ((raw_word_H - 0xB0) * 94 + (raw_word_L - 0xA1)+ 1038)*32+ BaseAdd;
    }

    *st_spi->Tx_buffer_index = 0;
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0x3;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) =( Address&0xFF0000) >>16;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) =( Address&0x00FF00)>>8;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index+1;
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) =( Address&0x0000FF);
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index+1;

    Drv_ssp_SPI_master_transmit( st_spi ,2*16, 0 );
#endif
}

void print_font_data( struct st_ssp_spi *st_spi  )
{

    UINT32  rsize =0;
    UINT16  i ,j ;
    UINT8   total_bits = 15;
    UINT16   tempd ;

    rsize = kdp_ssp_get_rx_buf_index(st_spi);
//    dbg_msg_console("spi rx size %d \r\n", rsize );

    for( i=0; i<rsize; i+=2 )
    {
        tempd = (*(st_spi->Rx_buffer + i ))<<8;
        tempd += *(st_spi->Rx_buffer + i+ 1  );

        for( j = total_bits; j > 0 ; j-- )
        {
            if (( tempd & (1<<j)   ) == 0 )
            {
                dbg_msg_flash("--");
            }
            else
            {
                dbg_msg_flash("00");
            }
        }
        dbg_msg_console("\r\n");
    }
    dbg_msg_console("\r\n");
    kdp_ssp_clear_rx_buf_index(st_spi);
    kdp_ssp_clear_tx_done_flag(st_spi);
    kdp_ssp_clear_tx_buf_index(st_spi);

}

void font_test_read(void)
{
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
    font_read_Chinese( &driver_ssp_master_ctx, 0xA8A1 );    //a bar
    print_font_data( &driver_ssp_master_ctx );

    font_read_Chinese( &driver_ssp_master_ctx, 0xCED2 );    //��
    print_font_data( &driver_ssp_master_ctx );
    font_read_Chinese( &driver_ssp_master_ctx, 0xC4E3 );    //�A
    print_font_data( &driver_ssp_master_ctx );

    font_read_Chinese( &driver_ssp_master_ctx, 0xCBFD );    //�o
    print_font_data( &driver_ssp_master_ctx );

    font_read_Chinese( &driver_ssp_master_ctx, 0xCBFD );    //rec
    print_font_data( &driver_ssp_master_ctx );


    font_read_Chinese( &driver_ssp_master_ctx, 0xA6B0 );    //rec
    print_font_data( &driver_ssp_master_ctx );

    font_read_Chinese( &driver_ssp_master_ctx, 0xA2A4 );    //rec
    print_font_data( &driver_ssp_master_ctx );
    font_read_Chinese( &driver_ssp_master_ctx, 0xA2A6 );    //rec
    print_font_data( &driver_ssp_master_ctx );
#endif

}

#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
void write_reg(uint8_t cmd)
{
    struct st_ssp_spi *st_spi=&driver_ssp_master_ctx;
   
    *st_spi->Tx_buffer_index = 0;
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = cmd;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;
     
    kdp520_lcm_set_cs(0);
    kdp520_lcm_set_rs(0);
    Drv_ssp_SPI_master_transmit( st_spi ,0, 1 );
    kdp520_lcm_set_rs(1);

    *st_spi->Tx_buffer_index  = 0;
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0xFF;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0xFF;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0xFF;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0xFF;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;
    *st_spi->Tx_buffer_index = 4;
    Drv_ssp_SPI_master_transmit( st_spi ,0, 0 );

    kdp520_lcm_set_cs(1);
}

void query_wifi(u8* buf,u16 len)
{
    u16 i=0;
    struct st_ssp_spi *st_spi=&driver_ssp_master_ctx;
    *st_spi->Tx_buffer_index = 0;
// write length    
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0x01;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;    
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0x00;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;    
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = len;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;    
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0x00;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;    
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0x00;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0x00;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;    
    
    Drv_ssp_SPI_master_transmit( st_spi ,0, 0 );
    delay_ms(10);
// write AT command    
    *st_spi->Tx_buffer_index = 0;
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0x02;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;        
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0x00;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;            
    for(i=0;i<len;i++)
    {
        *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = *buf;
        *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;
        buf++;
    }
    
    Drv_ssp_SPI_master_transmit( st_spi ,0, 0 );
    delay_ms(10);
// write done       
    *st_spi->Tx_buffer_index = 0; 
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0x01;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;    
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0x00;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;    
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0x00;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;    
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0x00;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;    
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0x00;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0x00;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;    
    
    Drv_ssp_SPI_master_transmit( st_spi ,0, 0 );
    delay_ms(10);
//read length    

    *st_spi->Tx_buffer_index = 0; 
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0x04;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;    
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = 0x00;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;    
    Drv_ssp_SPI_master_transmit( st_spi ,4, 0 );
    delay_ms(10);
}

void fLib_SetSSP_DMA(UINT32 trans,UINT32 rec)
{
    UINT32 data;
    struct st_ssp_spi *st_spi=&driver_ssp_master_ctx;
    uint32_t base = st_spi->reg_base_address;

    data = inw(base + SSP_REG_INTR_CR); 

    if (trans)
    data |= ssp_INTCR_TFDMAEN;
    else
    data &= ~ssp_INTCR_TFDMAEN;

    if (rec)
    data |= ssp_INTCR_RFDMAEN;
    else
    data &= ~ssp_INTCR_RFDMAEN; 

    outw(base + SSP_REG_INTR_CR, data);
}

static void _lcd_write_cmd(u8 cmd, u8 len, va_list args )
{
    uint32_t dat;
    int i;

    struct st_ssp_spi *st_spi=&driver_ssp_master_ctx;
    uint32_t base = st_spi->reg_base_address;

    *st_spi->Tx_buffer_index = 0;
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = cmd;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;
    for (i = 0; i < len; i++) {
        spi_tx_buf[i]=va_arg(args, int);
    }
     
    kdp520_lcm_set_rs(0);
    kdp520_lcm_set_cs(0);
    Drv_ssp_SPI_master_transmit( st_spi ,0, 0 );
    
    if(len<=0) {
        kdp520_lcm_set_cs(1);
        return;
    }

    kdp_ssp_clear_txhw(base);
    kdp_ssp_clear_rxhw(base);
   
    dma_ch DMAChannel;
    UINT8 ch = AHBDMA_Channel0;
    memset(&DMAChannel, 0x0, sizeof(dma_ch));
   
    dat = inw((base+0x10));
    outw( ( base + 0x10 ) , (dat | BIT3 | (13 << 12)) );/* enable TXDMA function */
    dat = inw((base+0x8));
    outw( ( base + 0x8 ) , (dat & ~BIT7 ) );/* disable RX function */

    // spi0/1 both need to swap s/ncpu dma req because they use u1 mode.
    dat = inw((SCU_EXTREG_PA_BASE+0xB0));
    outw( ( SCU_EXTREG_PA_BASE + 0xB0 ) , (dat | BIT0 ) );/* swap dma req/ack source */
    
    kdp_dma_init(0,0,1);   
    kdp_dma_reset_ch(ch); 
    kdp_dma_enable_dma_int(); 

    DMAChannel.csr.dst_ctrl = AHBDMA_DstFix;
    DMAChannel.csr.src_ctrl = AHBDMA_SrcInc;
    DMAChannel.csr.mode = AHBDMA_HwHandShakeMode;
    DMAChannel.csr.dst_width = AHBDMA_DstWidth_Byte;
    DMAChannel.csr.src_width = AHBDMA_SrcWidth_Byte;
    DMAChannel.csr.priority = 3;
    DMAChannel.csr.src_size = 3;


#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN )
#if ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP0 ) == COM_BUS_SSP0 )
    DMAChannel.cfg.dst_rs = SSP_u1_TX_DMA_REQ;
#elif ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP1 ) == COM_BUS_SSP1 )
    DMAChannel.cfg.dst_rs = SSP_u1_1_TX_DMA_REQ;
#endif
#endif
    DMAChannel.cfg.dst_he = 1;
    DMAChannel.cfg.src_rs = 0;
    DMAChannel.cfg.src_he = 0;
    DMAChannel.cfg.int_abt_msk = 0; 
    DMAChannel.cfg.int_err_msk = 0;
    DMAChannel.cfg.int_tc_msk = 0;

    kdp_dma_set_ch_cfg(ch, DMAChannel.csr); 
    kdp_dma_set_ch_cn_cfg(ch, DMAChannel.cfg); 

    kdp520_lcm_set_rs(1); 
    fLib_SetSSP_DMA(1,0);
    
    kdp_dma_clear_interrupt(ch); 
    kdp_dma_ch_data_ctrl(ch,(UINT32)(spi_tx_buf) , ( base + SSP_REG_DATA_PORT ), len);   
    kdp_dma_enable_ch(ch);
    kdp_dma_wait_dma_int(ch);
    kdp_dma_disable_ch(ch); 
    kdp_dma_clear_interrupt(ch);
    
    fLib_SetSSP_DMA(0,0);
    kdp520_lcm_set_cs(1);
}

void spi_lcd_write_cmd(u8 cmd, u8 len, ...) {
    va_list args;
    va_start(args, len);
    _lcd_write_cmd(cmd, len, args);
    va_end(args);
}
void spi_lcd_write_cmd_single(u8 cmd, ...) {
    va_list args;
    va_start(args, cmd);
    _lcd_write_cmd(cmd, 0, args);
    va_end(args);
}

void kl520_api_ssp_set_display_size(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    spi_lcd_write_cmd(0x2A,(x0>>8),(x0),(x1>>8),(x1));
    spi_lcd_write_cmd(0x2B,(y0>>8),(y0),(y1>>8),(y1)); //Row address set
}


void spi_lcd_write_img_buf_data(u32 len, void* buf)
{
    uint32_t dat;
    struct st_ssp_spi *st_spi=&driver_ssp_master_ctx;
    uint32_t base = st_spi->reg_base_address;
 
    //1byte unit
    dat = inw(base + SSP_REG_CR1);
    dat &= ~ssp_CR1_SDL_MASK;
    dat |= ssp_CR1_SDL(7);
    outw( base + SSP_REG_CR1, dat );

    spi_lcd_write_cmd_single(0x2c,NULL); //write data
    kdp_ssp_clear_txhw(base);
    kdp_ssp_clear_rxhw(base);

    //2byte unit for speed up
    dat = inw(base + SSP_REG_CR1);
    dat &= ~ssp_CR1_SDL_MASK;
    dat |= ssp_CR1_SDL(15);
    outw( base + SSP_REG_CR1, dat );

    dma_ch DMAChannel;
    UINT8 ch = AHBDMA_Channel0;
    memset(&DMAChannel, 0x0, sizeof(dma_ch));

    dat = inw((base+0x10));
    outw( ( base + 0x10 ) , (dat | BIT3 | (13 << 12)) );/* enable TXDMA function */
    dat = inw((base+0x8));
    outw( ( base + 0x8 ) , (dat & ~BIT7 ) );/* disable RX function */

    // spi0/1 both need to swap s/ncpu dma req because they use u1 mode.
    dat = inw((SCU_EXTREG_PA_BASE+0xB0));
    outw( ( SCU_EXTREG_PA_BASE + 0xB0 ) , (dat | BIT0 ) );/* swap dma req/ack source */

    kdp_dma_init(0,0,1);
    kdp_dma_reset_ch(ch);
    kdp_dma_enable_dma_int();

    DMAChannel.csr.dst_ctrl = AHBDMA_DstFix;
    DMAChannel.csr.src_ctrl = AHBDMA_SrcInc;
    DMAChannel.csr.mode = AHBDMA_HwHandShakeMode;
    DMAChannel.csr.dst_width = AHBDMA_DstWidth_Word;
    DMAChannel.csr.src_width = AHBDMA_SrcWidth_Word;
    DMAChannel.csr.priority = 3;
    DMAChannel.csr.src_size = 1;//set smaller if speed is lower.

#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN )
#if ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP0 ) == COM_BUS_SSP0 )
    DMAChannel.cfg.dst_rs = SSP_u1_TX_DMA_REQ;
#elif ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP1 ) == COM_BUS_SSP1 )
    DMAChannel.cfg.dst_rs = SSP_u1_1_TX_DMA_REQ;
#endif
#endif
    DMAChannel.cfg.dst_he = 1;
    DMAChannel.cfg.src_rs = 0;
    DMAChannel.cfg.src_he = 0;
    DMAChannel.cfg.int_abt_msk = 0;
    DMAChannel.cfg.int_err_msk = 0;
    DMAChannel.cfg.int_tc_msk = 0;

    kdp_dma_set_ch_cfg(ch, DMAChannel.csr);
    kdp_dma_set_ch_cn_cfg(ch, DMAChannel.cfg);

    kdp520_lcm_set_cs(0);
    kdp520_lcm_set_rs(1);
    fLib_SetSSP_DMA(1,0);

    kdp_dma_clear_interrupt(ch);
    kdp_dma_ch_data_ctrl(ch,(UINT32)(buf) , ( base + SSP_REG_DATA_PORT ), len>>1);// >>2 for 2byte mode.
    kdp_dma_enable_ch(ch);
    kdp_dma_wait_dma_int(ch);
    kdp_dma_disable_ch(ch);
    kdp_dma_clear_interrupt(ch);

    fLib_SetSSP_DMA(0,0);

    kdp520_lcm_set_rs(1);
    kdp520_lcm_set_cs(1);

    spi_lcd_write_cmd_single(0x29, NULL); //Display on

    // spi0/1 both need to swap s/ncpu dma req because they use u1 mode.
    dat = inw((SCU_EXTREG_PA_BASE+0xB0));
    outw( ( SCU_EXTREG_PA_BASE + 0xB0 ) , (dat & (~BIT0) ) );/* swap dma req/ack source */

}

void kl520_api_ssp_lcd_clock_init(u8 tx)
{
    struct st_ssp_spi *st_spi=&driver_ssp_master_ctx;
    uint32_t base = st_spi->reg_base_address;

    int dat;
    
    dat = inw(base + SSP_REG_CR1);
    dat &= ~ssp_CR1_SDL_MASK;
    dat |= ssp_CR1_SDL(7);
    outw( base + SSP_REG_CR1, dat );

    dat = inw(base + SSP_REG_CR1);
    dat &= ~ssp_CR1_SCLKDIV_MASK;
    
    if (tx) dat |= ssp_CR1_SCLKDIV(2);
    else dat |= ssp_CR1_SCLKDIV(100);
    outw( base + SSP_REG_CR1 , dat);

    dat = inw(SCU_EXTREG_PA_BASE + 0x20);

#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN )
#if ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP0 ) == COM_BUS_SSP0 )
    dat |= 0x04;
#elif ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP1 ) == COM_BUS_SSP1 )
    dat |= 0x08;
#endif
#endif
    outw( SCU_EXTREG_PA_BASE + 0x20 , dat); 
}

static kdp520_gpio_custom_dcsr_param _spi_DO_custom_dcsr_param;
extern void kdp520_api_ssp_set_DO_dcsr_param(kdp520_gpio_custom_dcsr_param* src_param) {
    kdp520_gpio_set_dcsr_param(&_spi_DO_custom_dcsr_param, src_param);
}

static int _tmp_DO_level = 0;
static void kdp520_api_ssp_set_DO_driving_strength(int enable) {
    //set driving strength to min(4mA) for DI/DO 3-line mode.
    if (!_spi_DO_custom_dcsr_param.get_dcsr_func || !_spi_DO_custom_dcsr_param.set_dcsr_func) return;
    if (enable) {
        _tmp_DO_level = _spi_DO_custom_dcsr_param.get_dcsr_func();
        _spi_DO_custom_dcsr_param.set_dcsr_func( _spi_DO_custom_dcsr_param.level );
    }
    else
        _spi_DO_custom_dcsr_param.set_dcsr_func(_tmp_DO_level);
}

void kl520_api_ssp_master_read(u8 reg, u8 len, u8* buf)
{
    //set clk down for rx
    kl520_api_ssp_lcd_clock_init(0);
    kdp520_api_ssp_set_DO_driving_strength(1);

    struct st_ssp_spi *st_spi=&driver_ssp_master_ctx;
    *st_spi->Tx_buffer_index = 0;
    *(st_spi->Tx_buffer + *st_spi->Tx_buffer_index) = reg;
    *st_spi->Tx_buffer_index = *st_spi->Tx_buffer_index + 1;

    for(int i=0; i<len+1; i++ )
        *(st_spi->Rx_buffer + i ) = 0;

    kdp520_lcm_set_cs(0);

    Drv_ssp_SPI_master_transmit( st_spi ,len, 0 );
    delay_ms(10);
    drv_rx_polling_receive_all(st_spi);

    for(int i=1; i<len+1; i++ )
        *(buf++) = *(st_spi->Rx_buffer + i );

    kdp_ssp_clear_rx_buf_index(st_spi);
    kdp_ssp_clear_tx_done_flag(st_spi);
    kdp_ssp_clear_tx_buf_index(st_spi);

    kdp520_lcm_set_cs(1);
    //set clk back for tx
    kl520_api_ssp_lcd_clock_init(1);
    kdp520_api_ssp_set_DO_driving_strength(0);
}

#endif
#endif
#endif
