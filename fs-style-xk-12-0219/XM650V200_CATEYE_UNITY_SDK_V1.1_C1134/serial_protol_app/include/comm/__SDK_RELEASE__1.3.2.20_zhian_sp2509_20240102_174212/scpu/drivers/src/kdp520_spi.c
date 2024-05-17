
#include "kdp520_spi.h"
#include "board_cfg.h"
#if (CFG_SPI_ENABLE == YES) || ( CFG_SSP0_ENABLE == YES ) || (CFG_SSP1_ENABLE == YES)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cmsis_os2.h>
#include "types.h"
#include "io.h"
#include "delay.h"
#include "framework/utils.h"
#include "kdp520_gpio.h"
#include "kdp_ddr_table.h"
#include "kl520_com.h"
#include "dbg.h"
#include "flash.h"

#if (FLASH_VENDOR_SELECT==GD25Q256D)
#include "GD25Q256D.h"
#elif (FLASH_VENDOR_SELECT==GD25S512MD)
#include "GD25S512MD.h"
#elif (FLASH_VENDOR_SELECT==W25Q256JV)
#include "W25Q256JV.h"
#elif (FLASH_VENDOR_SELECT==W25M512JV)
#include "W25M512JV.h"
#endif






/* Get the rx fifo depth, unit in byte */
UINT32 kdp520_spi_rxfifo_depth(void)
{
    return ((inl((INT8U * )SPI020REG_FEATURE) & SPI020_RX_DEPTH) >> (8-2));
}

/* Get the tx fifo depth, unit in byte */
UINT32 kdp520_spi_txfifo_depth(void)
{
    return ((inl((INT8U * )SPI020REG_FEATURE) & SPI020_TX_DEPTH) << 2);
}

/* Wait until the rx fifo ready */
void kdp520_spi_wait_rx_full(void)
{
    while(!(inl((INT8U * )SPI020REG_STATUS) & SPI020_RXFIFO_READY));
}

void kdp520_spi_write_data(UINT8 *buf, UINT32 length)
{
    INT32  access_byte;

    /* This function assume length is multiple of 4 */
    while(length > 0) {
        kdp520_spi_wait_tx_empty();
        access_byte = GET_MIN(length, kdp520_spi_txfifo_depth());
        length -= access_byte;
        while(access_byte > 0) {
            outw((INT8U * )SPI020REG_DATAPORT, *((UINT32 *)buf));
            buf += 4;
            access_byte -= 4;
        }
    }
}

/* Wait until the tx fifo ready */
void kdp520_spi_wait_tx_empty(void)
{
    while(!(inl((INT8U * )SPI020REG_STATUS) & SPI020_TXFIFO_READY));
}

void kdp520_spi_set_commands(UINT32 cmd0, UINT32 cmd1, UINT32 cmd2, UINT32 cmd3)
{
    outw((INT8U * )SPI020REG_CMD0, cmd0);
    outw((INT8U * )SPI020REG_CMD1, cmd1);
    outw((INT8U * )SPI020REG_CMD2, cmd2);
    outw((INT8U * )SPI020REG_CMD3, cmd3);
}
void kdp520_spi_write_Tx_FIFO( UINT8 *buf, UINT32 length )
{
    INT32  access_byte = 0;
    /* This function assume length is multiple of 4 */
    while( access_byte < length )
    {
        kdp520_spi_wait_tx_empty();
        outw((INT8U * )SPI020REG_DATAPORT, *((UINT32 *)buf));
        buf+= 4;
        access_byte +=4;
    }
}


void kdp520_spi_read_Rx_FIFO( UINT32 *buf_word, UINT32 *buf_word_index, UINT32 target_byte )
{
    UINT32 temp = target_byte;
    while( 1 )
    {
        while ( (inl((INT8U * )SPI020REG_STATUS) & SPI020_RXFIFO_READY) == 0 );
        *( buf_word + *buf_word_index )= inl( ( UINT8 * )SPI020REG_DATAPORT );
        *buf_word_index = (*buf_word_index) + 1;
        if( ( (*buf_word_index)*4 ) >= temp )
        {
            return;
        }
    }
}


kdp_status_t kdp520_spi_initialize(void)
{
    UINT32  reg;
    UINT32  ntemp;
    ntemp = ( 1<<8 |( 2<<6 )  );

    outw( SCU_EXTREG_PA_BASE  + 0x100 ,ntemp );
    outw( SCU_EXTREG_PA_BASE  + 0x104 ,ntemp  );
    outw( SCU_EXTREG_PA_BASE  + 0x108 ,ntemp  );
    outw( SCU_EXTREG_PA_BASE  + 0x10C ,ntemp  );

    #if( FLASH_QUAD_EN == YES )
    outw( SCU_EXTREG_PA_BASE  + 0x110 ,ntemp  );
    outw( SCU_EXTREG_PA_BASE  + 0x114 ,ntemp  );
    #endif

    delay_us(800);

    //Reset SPI IP
    outw(SPI020REG_CONTROL, SPI020_ABORT);
    /* Wait reset completion */
    do {
        if((inl(SPI020REG_CONTROL)&SPI020_ABORT)==0x00)
        break;
    } while(1);

    /* Set control register */
    reg = inl(SPI020REG_CONTROL);
    reg &= ~(SPI020_CLK_MODE | SPI020_CLK_DIVIDER);
    
    #if SPI_BUS_SPEED == SPI_BUS_SPEED_100MHZ
        reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_2;
    #elif SPI_BUS_SPEED == SPI_BUS_SPEED_50MHZ
        reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_4;
    #elif SPI_BUS_SPEED == SPI_BUS_SPEED_25MHZ
        reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_8;
    #endif
    outw( SPI020REG_CONTROL, reg );
    
    kdp520_spi_pre_log();

#if (FLASH_WORKING_EN == 0)
    #if( FLASH_QUAD_EN == YES )
    kdp520_flash_quad_enable(1);
    #endif
#else
    #if( FLASH_QUAD_EN == YES )
    nor_flash_quad_mode_en(1);
    #endif
#endif
    return KDP_STATUS_OK;
}

/* Wait until command complete */
void kdp520_spi_wait_command_complete(void)
{
    UINT32  reg;

    do {
        reg = inl((INT8U * )SPI020REG_INTR_ST);
    } while ((reg & SPI020_CMD_CMPLT)==0x0);
    outw((INT8U * )SPI020REG_INTR_ST, SPI020_CMD_CMPLT);/* clear command complete status */
}




UINT32  gflash_clock_log;
void kdp520_spi_pre_log( void )
{
    gflash_clock_log = inw( SPI020REG_CONTROL );
}

void kdp520_spi_switch_fail( void )
{
    UINT32  reg ;
    //  //Reset SPI IP
    outw(SPI020REG_CONTROL, SPI020_ABORT);
    /* Wait reset completion */
    do {
        if((inl(SPI020REG_CONTROL)&SPI020_ABORT)==0x00)
            break;
    } while(1);
    /* Set control register */
    reg = inl(SPI020REG_CONTROL);
    reg &= ~(SPI020_CLK_MODE | SPI020_CLK_DIVIDER);
    reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_4;
    outw( SPI020REG_CONTROL, reg );
}

void kdp520_spi_switch_org( void )
{
    //  Reset SPI IP
    outw(SPI020REG_CONTROL, SPI020_ABORT);
    /* Wait reset completion */
    do {
        if( (inl(SPI020REG_CONTROL)&SPI020_ABORT) == 0x00 )
            break;
    } while(1);
    outw( SPI020REG_CONTROL, gflash_clock_log );
}


#endif
