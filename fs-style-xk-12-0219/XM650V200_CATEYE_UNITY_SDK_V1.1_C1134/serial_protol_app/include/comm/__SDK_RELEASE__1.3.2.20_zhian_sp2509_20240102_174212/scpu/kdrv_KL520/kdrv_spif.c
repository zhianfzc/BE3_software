/* Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
*
* The information contained herein is property of Kneron, Inc.
* Terms and conditions of usage are described in detail in Kneron
* STANDARD SOFTWARE LICENSE AGREEMENT.
*
* Licensees are granted free, non-transferable use of the information.
* NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
* from the file.
*/

/******************************************************************************
*  Filename:
*  ---------
*  kdrv_spif.c
*
*  Project:
*  --------
*  KL520
*
*  Description:
*  ------------
*  This SPI Flash driver is for Generic SPI Flash Access
*  HW: Faraday FTSPI020
*
*  Author:
*  -------
*  Teresa Chen
**
******************************************************************************/

/******************************************************************************
Head Block of The File
******************************************************************************/

#ifdef USE_KDRV
#include "kdrv_SPI020.h"
#include "kdrv_spif.h"
#include "io.h"

//#define SPIF_DBG
#ifdef SPIF_DBG
#include "kmdw_console.h"
#define spif_msg(fmt, ...) info_msg("[KDRV_SPIF] " fmt, ##__VA_ARGS__)
#else
#define spif_msg(fmt, ...)
#endif

#define min_t(x,y) ( x < y ? x: y )
#define MEMXFER_OPS_NONE 	0x00
#define MEMXFER_OPS_CPU 	0x01
#define MEMXFER_OPS_DMA 	0x02

#define MEMXFER_INITED      0x10
#define MEMXFER_OPS_MASK MEMXFER_OPS_CPU | MEMXFER_OPS_DMA

void kdrv_spif_initialize(void)
{
    uint32_t   reg;
#if 1 //default is 12mA
    int mA;
    uint32_t addr;
    /*16mA*/
    //mA = 0; //4mA
    //mA = 1; //8mA
    //mA = 2; //12mA
    mA = 3; //16mA
    spif_msg("Set SPI driving = %d mA\n", (mA+1)*4);
    for(addr=SCU_EXTREG_PA_BASE  + 0x100; addr<=SCU_EXTREG_PA_BASE  + 0x114; addr+=4)
    {
        spif_msg("addr = 0x%2X\n",addr);
        reg = inw(addr);                //SPI IO control
        spif_msg("reg = 0x%2X\n",reg);
        reg &= ~0x000000C0;             //clear bit6, bit7
        spif_msg("reg = 0x%2X\n",reg);
        reg |= (mA<<6);        //select driving strength
        reg |= (1<<8);        //select slew rate slow
        spif_msg("reg = 0x%2X\n",reg);
        outw(addr, reg);
    }
#endif
#if 1
    outw(SPI020REG_CONTROL, SPI020_ABORT);
    /* Wait reset completion */
    do {
        if((inw(SPI020REG_CONTROL)&SPI020_ABORT)==0x00)
            break;
    } while(1);
#endif
    /* Set control register */
    reg = inw(SPI020REG_CONTROL); // 0x80
    reg &= ~(SPI020_CLK_MODE | SPI020_CLK_DIVIDER);
    reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_4; // SCPU:200MHz, Flash: 50MHz
    outw(SPI020REG_CONTROL, reg); 
}

void kdrv_spif_memxfer_initialize(uint8_t flash_mode, uint8_t mem_mode)
{
    uint32_t reg;
  	uint32_t ntemp;

  	//set as mode0 for SPI020_IO and driving 12mA
  	ntemp = ( 0<<0  | ( 3<<7 )  );
  	outw( SCU_EXTREG_PA_BASE  + 0x100 ,ntemp );
  	outw( SCU_EXTREG_PA_BASE  + 0x104 ,ntemp  );
  	outw( SCU_EXTREG_PA_BASE  + 0x108 ,ntemp  );
  	outw( SCU_EXTREG_PA_BASE  + 0x10C ,ntemp  );
  	outw( SCU_EXTREG_PA_BASE  + 0x110 ,ntemp  );
  	outw( SCU_EXTREG_PA_BASE  + 0x114 ,ntemp  );
    if (!(flash_mode & MEMXFER_INITED)) {
        outw(SPI020REG_CONTROL, SPI020_ABORT);
        do
        {
            if((inw(SPI020REG_CONTROL)&SPI020_ABORT)==0x00)
             break;
        }while(1);

        /* Set control register */
        reg = inw(SPI020REG_CONTROL); // 0x80
        reg &= ~(SPI020_CLK_MODE | SPI020_CLK_DIVIDER);
    #if SPI_BUS_SPEED == SPI_BUS_SPEED_100MHZ
        reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_2;
    #elif SPI_BUS_SPEED == SPI_BUS_SPEED_50MHZ
        reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_4;
    #elif SPI_BUS_SPEED == SPI_BUS_SPEED_25MHZ
        reg |= SPI_CLK_MODE0 | SPI_CLK_DIVIDER_8;
    #endif
        outw(SPI020REG_CONTROL, reg);
    }
}

void kdrv_spif_set_commands(uint32_t cmd0, uint32_t cmd1, uint32_t cmd2, uint32_t cmd3)
{
    outw((int8_t * )SPI020REG_CMD0, cmd0);
    outw((int8_t * )SPI020REG_CMD1, cmd1);
    outw((int8_t * )SPI020REG_CMD2, cmd2);
    outw((int8_t * )SPI020REG_CMD3, cmd3);
}

/* Wait until command complete */
void kdrv_spif_wait_command_complete(void)
{
    uint32_t  reg;

    do {
        reg = inw((int8_t * )SPI020REG_INTR_ST);
    } while ((reg & SPI020_CMD_CMPLT)==0x0);
//  outw(SPI020REG_INTR_ST, (reg | SPI020_CMD_CMPLT));/* clear command complete status */
    outw((int8_t * )SPI020REG_INTR_ST, SPI020_CMD_CMPLT);/* clear command complete status */
}

uint8_t kdrv_spif_rx_FIFO_empty_check(void)
{
    if ( (inw((int8_t * )SPI020REG_STATUS) & SPI020_RXFIFO_READY) != 0)
    {
        return 1;	//empty
    }
    return 0;//at leat 1 data
}


/* Wait until the rx fifo ready */
void kdrv_spif_wait_rx_full(void)
{
    while(!(inw((int8_t * )SPI020REG_STATUS) & SPI020_RXFIFO_READY));
}

/* Wait until the tx fifo ready */
void kdrv_spif_wait_tx_empty(void)
{
    while(!(inw((int8_t * )SPI020REG_STATUS) & SPI020_TXFIFO_READY));
}

/* Get the rx fifo depth, unit in byte */
uint32_t kdrv_spif_rxfifo_depth(void)
{
    return ((inw((int8_t * )SPI020REG_FEATURE) & SPI020_RX_DEPTH) >> (8-2));
}

/* Get the tx fifo depth, unit in byte */
uint32_t kdrv_spif_txfifo_depth(void)
{
//      unsigned int read;
        
    return ((inw((int8_t * )SPI020REG_FEATURE) & SPI020_TX_DEPTH) << 2);
}

void kdrv_spif_read_Rx_FIFO( uint32_t *buf_word, uint16_t *buf_word_index, uint32_t target_byte )
{
    while( 1 )
    {
      while( kdrv_spif_rx_FIFO_empty_check() == 1 )
      {
        *( buf_word + *buf_word_index )= inw( ( uint8_t * )SPI020REG_DATAPORT );
        *buf_word_index = (*buf_word_index) + 1;
      }
      if(  (*buf_word_index*4) >= target_byte )
      {
      	return;
      }
    }
}

void kdrv_spif_read_data(/*uint8_t*/uint32_t *buf, uint32_t length)
{
    uint32_t  access_byte;//, tmp_read;

    while(length > 0)
    {
        kdrv_spif_wait_rx_full();
        access_byte = min_t(length, kdrv_spif_rxfifo_depth());
        length -= access_byte;
        while(access_byte > 0)
        {
            *buf= inw((int8_t * )SPI020REG_DATAPORT);
            buf ++;
            if(access_byte>=4)
                access_byte -= 4;
            else
                access_byte=0;
            #if 0
            switch(access_byte)
            {
            case 1:
                tmp_read = inw((int8_t * )SPI020REG_DATAPORT);
                *buf = tmp_read&0xFF;
                access_byte = 0;//break while loop 
                break;
            case 2:
                tmp_read = inw((int8_t * )SPI020REG_DATAPORT);
                *buf = tmp_read&0xFF;
                buf++;
                *buf = (tmp_read&0xFF00)>>8;
                access_byte = 0;// break while loop 
                break;
            case 3:// read chip id will use this case 
                tmp_read = inw((int8_t * )SPI020REG_DATAPORT);
                *buf = tmp_read&0x00FF;
                buf++;
                *buf = (tmp_read&0xFF00)>>8;
                buf++;
                *buf = (tmp_read&0xFF0000)>>16;
                access_byte = 0;// break while loop 
                break;
            default:// access_byte>=4 
                *(uint32_t *)buf= inw((int8_t * )SPI020REG_DATAPORT);
                buf +=4;
                access_byte -= 4;
                break;
            }
            #endif
        }
    }
}

void kdrv_spif_check_status_till_ready_2(void)
{
#if 1
    uint32_t gSPI_RX_buff[4];
    uint16_t gSPI_RX_buff_index;
	uint32_t countdown = 0;
    
	while(1)
	{
        countdown =1000;
		kdrv_spif_set_commands( SPI020_05_CMD0_w, SPI020_05_CMD1_w, SPI020_05_CMD2_w, SPI020_05_CMD3_w);
		gSPI_RX_buff_index = 0;
		kdrv_spif_read_Rx_FIFO( gSPI_RX_buff, &gSPI_RX_buff_index, 1 );
		kdrv_spif_wait_command_complete();

		if( (gSPI_RX_buff[0] & 0x01) == 0x00)
		{
			break;
		}
		while(countdown--);
	}
#else
//  main_delay_count(0x5100);

    /* fill in command 0~3 */
    kdrv_spif_set_commands(SPI020_05_CMD0, SPI020_05_CMD1, SPI020_05_CMD2, SPI020_05_CMD3);
//  main_delay_count(0x80);
    /* wait for command complete */
    kdrv_spif_wait_command_complete();
#endif
}

void kdrv_spif_check_status_till_ready(void)
{
    /* savecodesize, move into here */
    kdrv_spif_wait_command_complete();

    /* read status */
    kdrv_spif_check_status_till_ready_2();

}

void kdrv_spif_check_quad_status_till_ready(void)
{
    /* savecodesize, move into here */
    kdrv_spif_wait_command_complete();

    /* read status */
    /* fill in command 0~3 */
    kdrv_spif_set_commands(SPI020_05_CMD0, SPI020_05_CMD1, SPI020_05_CMD2, SPI020_05_CMD3);
//  main_delay_count(0x80);
    /* wait for command complete */
    kdrv_spif_wait_command_complete();

}

void kdrv_spif_write_data(uint8_t *buf, uint32_t length)
{
    int32_t  access_byte;

    /* This function assume length is multiple of 4 */
    while(length > 0) {
        kdrv_spif_wait_tx_empty();
        access_byte = min_t(length, kdrv_spif_txfifo_depth());
        length -= access_byte;
        while(access_byte > 0) {
            outw((int8_t * )SPI020REG_DATAPORT, *((uint32_t *)buf));
            buf += 4;
            access_byte -= 4;
        }
    }
}
#endif
