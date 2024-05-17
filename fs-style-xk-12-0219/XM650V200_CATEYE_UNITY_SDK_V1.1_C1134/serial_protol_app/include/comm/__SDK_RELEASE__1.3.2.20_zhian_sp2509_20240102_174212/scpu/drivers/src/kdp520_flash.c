#include "types.h"
#include "framework/utils.h"
#include "kdp520_flash.h"




#ifdef CFG_WINWOND_FLASH
UINT8 kdp520_flash_get_info(void)
{
	#define	SPI_Rx_SIZE		(5)
	UINT32 	nrx_buff_word_index = 0;
	UINT32	nrx_buff_word[ SPI_Rx_SIZE ];
	UINT32	ntemp =0;

	//do
	if( st_flash_info.page_size_Bytes == 256
			&& st_flash_info.block_size_Bytes != 0 )
	{
		return 1;
	}
	kdp520_spi_switch_fail();

	kdp520_flash_write_control(1);

	kdp520_spi_set_commands( 0x00 , SPI020_11_CMD1, SPI020_11_CMD2, SPI020_11_CMD3 );
	kdp520_spi_wait_command_complete();
	delay_ms(10);

#if 0
	//reset chip
	kdp520_spi_set_commands( SPI020_66_CMD0 , SPI020_66_CMD1, SPI020_66_CMD2, SPI020_66_CMD3 );
	kdp520_spi_wait_command_complete();
	kdp520_spi_set_commands( SPI020_99_CMD0 , SPI020_99_CMD1, SPI020_99_CMD2, SPI020_99_CMD3 );
	kdp520_spi_wait_command_complete();

	delay_ms(30);		//org 800
#endif

#if 1		//
	nrx_buff_word_index =0;
	kdp520_spi_set_commands(SPI020_15_CMD0, SPI020_15_CMD1, SPI020_15_CMD2, SPI020_15_CMD3);
	spi020_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
	kdp520_spi_wait_command_complete();

	kdp520_flash_write_control(1);
	//disable quad mode bit
	kdp520_spi_set_commands( 0x00 , SPI020_11_CMD1, SPI020_11_CMD2, SPI020_11_CMD3 );
	kdp520_spi_wait_command_complete();
	delay_ms(30);   //delay_us(300);

	//20191219 add
	nrx_buff_word_index =0;
	kdp520_spi_set_commands(SPI020_15_CMD0, SPI020_15_CMD1, SPI020_15_CMD2, SPI020_15_CMD3);
	spi020_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
	kdp520_spi_wait_command_complete();
    nrx_buff_word_index =0;
	kdp520_spi_set_commands(SPI020_35_CMD0, SPI020_35_CMD1, SPI020_35_CMD2, SPI020_35_CMD3);//bessel:wait interrupt instead of delay
	spi020_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
	kdp520_spi_wait_command_complete();
	delay_ms(10);

#endif

	nrx_buff_word_index =0;
	kdp520_spi_set_commands(SPI020_15_CMD0, SPI020_15_CMD1, SPI020_15_CMD2, SPI020_15_CMD3);
	kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
	kdp520_spi_wait_command_complete();
	delay_ms(10);

	//check status
	nrx_buff_word_index =0;
	kdp520_spi_set_commands( 0x00 , SPI020_5A_CMD1, 0x04, SPI020_5A_CMD3);
	kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x04 );
	kdp520_spi_wait_command_complete();
	delay_ms(10);
	st_flash_info.signature = FLASH_SIGNATURE;

	//check
	if( nrx_buff_word[nrx_buff_word_index-1] != FLASH_SIGNATURE )
	{
		return 0;
	}

	//get ptr
	nrx_buff_word_index =0;
	kdp520_spi_set_commands( 0x0C , SPI020_5A_CMD1, 0x01, SPI020_5A_CMD3);
	kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
	kdp520_spi_wait_command_complete();
	delay_ms(10);
	st_flash_info.PTP = nrx_buff_word[nrx_buff_word_index-1] & 0XFF;

	//get ID
	nrx_buff_word_index =0;
	kdp520_spi_set_commands( 0x10 , SPI020_5A_CMD1, 0x04, SPI020_5A_CMD3);
	kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x04 );
	kdp520_spi_wait_command_complete();
	delay_ms(10);
	st_flash_info.ID = nrx_buff_word[nrx_buff_word_index-1] & 0XFFFFFFFF;

	if( st_flash_info.ID== 0x00 || st_flash_info.ID==0xFF  )
	{
		nrx_buff_word_index =0;
		kdp520_spi_set_commands( SPI020_9F_CMD0 , SPI020_9F_CMD1, SPI020_9F_CMD2, SPI020_9F_CMD3 );
		kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, SPI020_9F_CMD2 );
		kdp520_spi_wait_command_complete();
		delay_ms(10);
		st_flash_info.ID = nrx_buff_word[nrx_buff_word_index-1] & 0xFF;
	}

	//get 4K erase support
	nrx_buff_word_index =0;
	kdp520_spi_set_commands( st_flash_info.PTP + 0, SPI020_5A_CMD1, 0x01, SPI020_5A_CMD3);
	kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
	kdp520_spi_wait_command_complete();
	delay_ms(10);
	st_flash_info.erase_4K_support = nrx_buff_word[nrx_buff_word_index-1] & 0x3;

	//get size
	nrx_buff_word_index =0;
	kdp520_spi_set_commands( st_flash_info.PTP+4 , SPI020_5A_CMD1, 0x04, SPI020_5A_CMD3);
	kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x04 );
	kdp520_spi_wait_command_complete();
	delay_ms(10);
	st_flash_info.flash_size_KByte = (nrx_buff_word[nrx_buff_word_index-1]>>10)>>3;
	ntemp = nrx_buff_word[nrx_buff_word_index-1]>>3;

	//get sector size 0x1C
	nrx_buff_word_index =0;
	kdp520_spi_set_commands( st_flash_info.PTP+0x1C , SPI020_5A_CMD1, 0x01, SPI020_5A_CMD3);
	kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
	kdp520_spi_wait_command_complete();
	delay_ms(10);
	st_flash_info.sector_size_Bytes = 1<<(nrx_buff_word[ nrx_buff_word_index-1 ]&0xFF);
	st_flash_info.total_sector_numbers = (ntemp / st_flash_info.sector_size_Bytes)+1;

	//get sector size 0x20
	nrx_buff_word_index =0;
	kdp520_spi_set_commands( st_flash_info.PTP+0x20 , SPI020_5A_CMD1, 0x01, SPI020_5A_CMD3);
	kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
	kdp520_spi_wait_command_complete();
	delay_ms(10);
	st_flash_info.block_size_Bytes = ( 1<<( nrx_buff_word[ nrx_buff_word_index-1 ] & 0xFF ) )/st_flash_info.sector_size_Bytes ;

	//get page size
	nrx_buff_word_index =0;
	kdp520_spi_set_commands( st_flash_info.PTP+0x28 , SPI020_5A_CMD1, 0x01, SPI020_5A_CMD3);
	kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
	kdp520_spi_wait_command_complete();
	delay_ms(10);
	ntemp = nrx_buff_word[nrx_buff_word_index-1]&0xFF;

#if 0
	//20191219 add
	nrx_buff_word_index =0;
	kdp520_spi_set_commands(SPI020_15_CMD0, SPI020_15_CMD1, SPI020_15_CMD2, SPI020_15_CMD3);
	kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
	kdp520_spi_wait_command_complete();
nrx_buff_word_index =0;
	kdp520_spi_set_commands(SPI020_35_CMD0, SPI020_35_CMD1, SPI020_35_CMD2, SPI020_35_CMD3);//bessel:wait interrupt instead of delay
	kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
	kdp520_spi_wait_command_complete();


#endif


	kdp520_spi_switch_org();

	if( (ntemp>>4) == FLASH_PAGE_SIZE_256_CODE )
	{
		st_flash_info.page_size_Bytes = 256;
	}
	else
	{
		st_flash_info.page_size_Bytes = 0;
		return 0;
	}


	return 1;
}
#else


#endif





