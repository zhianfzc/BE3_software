#ifndef __KL520_API_SSP_SPI_H__
#define __KL520_API_SSP_SPI_H__


#include "types.h"
#include "kdp520_ssp.h"
#include "kdp520_gpio.h"


extern UINT8 kl520_api_ssp_spi_sample(void);
extern void kl520_api_ssp_spi1_slave_req_init(void);
extern void kl520_api_ssp_spi1_slave_inactive(void);
extern void kl520_api_ssp_spi1_slave_active(void);
extern void kl520_api_ssp_spi1_clear_rx_hw(void);
extern void kl520_api_ssp_spi1_clear_tx_hw(void);
extern void kl520_api_ssp_spi1_clear_rx_buff_size(struct st_ssp_spi *stspi );
extern void kl520_api_ssp_spi1_clear_tx_buff_size(struct st_ssp_spi *stspi );
extern void kl520_api_ssp_spi1_clear_tx_current_buff_size( void );
extern void kl520_api_ssp_spi1_clear_tx_done_flag( void );
extern UINT8 kl520_api_ssp_spi1_init(enum e_spi edata);
extern UINT8 kl520_api_ssp_spi1_receive(struct st_ssp_spi *stspi);
extern void kl520_api_ssp_lcd_clock_init(u8 tx);
extern void kl520_api_ssp_set_display_size(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
extern void spi_lcd_write_cmd(u8 cmd, u8 len, ...);
extern void spi_lcd_write_cmd_single(u8 cmd, ...);
extern void spi_lcd_write_img_buf_data(u32 len, void* buf);
extern void kl520_api_ssp_master_read(u8 reg, u8 len, u8* buf);

extern void font_read_Chinese( struct st_ssp_spi *st_spi, UINT16 raw_word );
extern void kl520_api_ssp_spi_master_init(void);
extern void font_test_read(void);
extern void kdp520_api_ssp_set_DO_dcsr_param(kdp520_gpio_custom_dcsr_param* src_param);

#endif
