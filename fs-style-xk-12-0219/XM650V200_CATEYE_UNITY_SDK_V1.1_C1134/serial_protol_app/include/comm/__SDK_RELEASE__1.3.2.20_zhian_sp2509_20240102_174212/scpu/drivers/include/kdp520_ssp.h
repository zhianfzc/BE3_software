#ifndef __KDP520_SSP_H__
#define __KDP520_SSP_H__


#include "types.h"
#include "kneron_mozart.h"
#include "board_cfg.h"


#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN ) && ( ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) ) == 0 )
#error "Please select correct spi number for SPI master"
#endif



#define	SSP_SPI_TIME_TEST_EN		(NO)
#define SSP_REG_BASE_M		(SPI_FTSSP010_0_PA_BASE)	//SPI_FTSSP010_1_PA_BASE
#define SSP_REG_BASE_S		(SPI_FTSSP010_1_PA_BASE)	//SPI_FTSSP010_0_PA_BASE

#define SSP_REG_CR0	0x0
#define SSP_REG_CR1	0x4
#define SSP_REG_CR2	0x8
#define SSP_REG_STS	0xc
#define SSP_REG_INTR_CR	0x10
#define SSP_REG_INTR_STS	0x14
#define SSP_REG_DATA_PORT	0x18
#define SSP_REG_CR3	0x1C
#define SSP_REG_REVISION	0x60
#define SSP_REG_FEATURE	0x64

/* REG_CR0 field */
#define ssp_CR0_FSDBK	(1 << 17)
#define ssp_CR0_SCLKFDBK	(1 << 16)
#define ssp_CR0_SPIFSPO	(1 << 15) /* Frame/Sync polarity, SPI only */
#define ssp_CR0_FFMT_MASK	(7 << 12)
#define ssp_CR0_FFMT_SSP	(0 << 12)
#define ssp_CR0_FFMT_SPI	(1 << 12)
#define ssp_CR0_FFMT_MWR	(2 << 12)
#define ssp_CR0_FFMT_I2S	(3 << 12)
#define ssp_CR0_FFMT_ACL	(4 << 12)
#define ssp_CR0_FFMT_SPDIF	(5 << 12)
#define ssp_CR0_SPI_FLASH	(1 << 11) 
#define ssp_CR0_VALIDITY	(1 << 10) // SPDIF validity
#define ssp_CR0_FSDIS_MASK	(3 << 8)
#define ssp_CR0_FSDIST(x)	((x & 0x3) << 8) // frame/sync and data distance, I2S only
#define ssp_CR0_LOOPBACK	(1 << 7)
#define ssp_CR0_LSB	(1 << 6) // 0: MSB, 1:LSB tx and rx first
#define ssp_CR0_FSPO	(1 << 5) // Frame/Sync polarity, I2S or MWR only
#define ssp_CR0_FSJSTFY	(1 << 4) // Padding data in front(1) or back(0) of serial data
#define ssp_CR0_MODE_MASK	(3 << 2)
#define ssp_CR0_MSTR_STREO	(3 << 2)
#define ssp_CR0_MSTR_MONO	(2 << 2)
#define ssp_CR0_SLV_STREO	(1 << 2)
#define ssp_CR0_SLV_MONO	(0 << 2)
#define ssp_CR0_MSTR_SPI	(3 << 2)
#define ssp_CR0_SLV_SPI	(1 << 2)
#define ssp_CR0_SCLKPO_0	(0 << 1) // SCLK polarity, SPI only
#define ssp_CR0_SCLKPO_1	(1 << 1) // SCLK polarity, SPI only
#define FTSSP020_CR0_SCLKPH_0	(0 << 0) // SCLK phase, SPI only
#define FTSSP020_CR0_SCLKPH_1	(1 << 0) // SCLK phase, SPI only

/* REG_CR1 field */
#define ssp_CR1_PDL(x)	((x & 0xff) << 24) // Padding data length
#define ssp_CR1_PDL_MASK	(0xff << 24)
#define ssp_CR1_SDL(x)	((x & 0x7f) << 16) // Serial data length
#define ssp_CR1_SDL_MASK	(0x7f << 16)
#define ssp_SDL_MAX_BYTES_MASK	(0x7f)
#define ssp_CR1_SCLKDIV(x)	(x & 0xffff)
#define ssp_CR1_SCLKDIV_MASK	(0xffff)

/* REG_CR2 field */
#define ssp_CR2_FSOS(x)	((x & 0x3) << 10) // frame/sync output select, SPI only
#define ssp_CR2_FSOS_MASK	(3 << 10)
#define ssp_CR2_FS 	(1 << 9) // 0: low, 1: high frame/sync output
#define ssp_CR2_TXEN	(1 << 8)
#define ssp_CR2_RXEN	(1 << 7)
#define ssp_CR2_SSPRST	(1 << 6)
#define ssp_CR2_ACRST	(1 << 5)
#define ssp_CR2_ACWRST	(1 << 4)
#define ssp_CR2_TXFCLR	(1 << 3) // W1C, Clear TX FIFO
#define ssp_CR2_RXFCLR	(1 << 2) // W1C, Clear RX FIFO
#define ssp_CR2_TXDOE	(1 << 1) // TX Data Output Enable, SSP slave only
#define ssp_CR2_SSPEN	(1 << 0)

/* REG_STS 0xC field */
#define ssp_STS_TFVE(x)	((x >> 12) & 0x3f) // TX FIFO valid entries
#define ssp_STS_RFVE(x)	((x >> 4) & 0x3f) // RX FIFO valid entries
#define ssp_STS_BUSY	(1 << 2)
#define ssp_STS_TFNF	(1 << 1) // TX FIFO not full
#define ssp_STS_RFF	(1 << 0) // RX FIFO full

/* REG_INTR_CR 0x10 field */
#define ssp_INTCR_RFTHOD_UNIT	(1 << 17)
#define ssp_INTCR_TFTHOD(x)	((x & 0x1f) << 12)
#define ssp_INTCR_TFTHOD_MASK	(0x1f << 12)
#define ssp_INTCR_RFTHOD(x)	((x & 0x1f) << 7)
#define ssp_INTCR_RFTHOD_MASK	(0x1f << 7)
#define ssp_INTCR_AC97FCEN	(1 << 6)
#define ssp_INTCR_TFDMAEN	(1 << 5)
#define ssp_INTCR_RFDMAEN	(1 << 4)
#define ssp_INTCR_TFTHIEN	(1 << 3)
#define ssp_INTCR_RFTHIEN	(1 << 2)
#define ssp_INTCR_TFURIEN	(1 << 1)
#define ssp_INTCR_RFORIEN	(1 << 0)

/* REG_INTR_STS 0x14 field */
#define ssp_INTSTS_AC97CI	(1 << 4)
#define ssp_INTSTS_TFTHI	(1 << 3) // TX FIFO threshold
#define ssp_INTSTS_RFTHI	(1 << 2) // RX FIFO threshold
#define ssp_INTSTS_TFUI	(1 << 1) // TX FIFO underrun
#define ssp_INTSTS_RFORI	(1 << 0) // RX FIFO overrun

/* REG_CR3 0x1C field */
#define ssp_CR3_DPDL(x)	((x & 0xff) << 16) // Padding Data length
#define ssp_CR3_DPDL_MASK	(0xff << 16)
#define ssp_CR3_DPDLEN	(1 << 12)
#define ssp_CR3_PCL(x)	(x & 0x3ff) // Padding Cycle length
#define ssp_CR3_PCL_MASK	0x3ff


// REG_FEATURE 0x64 field
#define ssp_FEA_TXFIFO_DEPTH(x)	((x >> 16) & 0xff)
#define ssp_FEA_RXFIFO_DEPTH(x)	((x >> 8) & 0xff)

//===========================================================================
#define	SPI_TX_FIFO_TH		(4)
#define	SPI_RX_FIFO_TH		(4)
#define SPI_MAX_FIFO        (16)

#define		SPI_PACKET_HEAD								(0x78875A01)
#define		SPI_PACKET_READ_LARGE_HEAD		(0x78875A02)
#define		SPI_PACKET_WRITE_LARGE_HEAD		(0x78875A03)

// SPI only
typedef enum {
    SPI_CS_LOW = 0,
    SPI_CS_HI = ssp_CR0_SPIFSPO,
} SPI_CHIP_SELECT;


typedef enum {
    // CLKPO = 0, CLKPHA = 0
    SPI_MODE_0 = (ssp_CR0_SCLKPO_0 | FTSSP020_CR0_SCLKPH_0),
    // CLKPO = 0, CLKPHA = 1
    SPI_MODE_1 = (ssp_CR0_SCLKPO_0 | FTSSP020_CR0_SCLKPH_1),
    // CLKPO = 1, CLKPHA = 0
    SPI_MODE_2 = (ssp_CR0_SCLKPO_1 | FTSSP020_CR0_SCLKPH_0),
    // CLKPO = 1, CLKPHA = 1
    SPI_MODE_3 = (ssp_CR0_SCLKPO_1 | FTSSP020_CR0_SCLKPH_1),
    SPI_MODE_MAX
} SPI_MODE_TYPE;

enum e_spi
{
    //case
    e_spi_init_slave = 0,
    e_spi_init_master,
    e_spi_enable,
    e_spi_rx ,
    e_spi_rx_check,
    e_spi_tx ,
    e_spi_tx_status_check ,
    e_spi_tx_large ,        //for future command mode use
    e_spi_rx_large ,        //receive command action
    e_spi_master_tx_rx,
    e_spi_disable ,
    e_spi_idle ,
    e_spi_tx_xor,


    //return status
    e_spi_ret_init_done,
    e_spi_ret_enable_done,
    e_spi_ret_rxbusy,
    e_spi_ret_rxdone,
    e_spi_ret_rx_xor_OK,
    e_spi_ret_rx_xor_error,

    e_spi_ret_txbusy,
    e_spi_ret_txdone,
    e_spi_ret_disableDone,
    e_spi_ret_tx_xor_done,
    e_spi_ret_idle,
};

struct st_ssp_spi
{
    UINT32  reg_base_address;               //base address
    UINT8   IP_type;                        //3: master 0:slave
    UINT8   SDL;                            //data length
    UINT8   target_Txfifo_depth;            //max is 16Byte
    UINT8   target_Rxfifo_depth;            //max is 16Byte
    UINT8   tx_rx_en;                       //0: all disable, 1:Tx enable, 2:Rx enable, 3:Tx and Rx are all enable
    UINT8   spi_sw_type;                    //0:polling(no interrupt IRQ), 1:interrupt+polling, 2:DMA only
    UINT8   interrupt_en;                   //0x00:no interurpt, 0x08: Tx int ebable, 0x04:Rx interupt enable,
                                            //0x0C: Tx and Rx interrupt are all enable


    //buffer related
    volatile UINT8  *Tx_buffer;
    volatile UINT32 *Tx_buffer_index;
    volatile UINT32 *Tx_buffer_current_index;
    volatile UINT8  Tx_done_flag;           //0: no done, 1: done flag


    volatile UINT8  *Rx_buffer;
    volatile UINT32 *Rx_buffer_index;
    volatile UINT32 buffer_max_size;
    volatile UINT32 pre_size;
    
    enum e_spi  eflow;

    volatile UINT8      *Rx_tempbuffer;
    volatile UINT32     Rx_tempbuffer_index;
};

#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
extern UINT8   gTx_buff_SP_MASTER[];
extern UINT8   gRx_buff_SP_MASTER[];
extern UINT32  gTx_buff_index_SP_MASTER;
extern UINT32  gTx_buff_current_index_SP_MASTER;
extern UINT32  gRx_buff_index_SP_MASTER;
#endif

#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
extern struct st_ssp_spi driver_ssp_master_ctx;
#endif
extern struct st_ssp_spi driver_ssp_ctx;


int kdp_ssp_rxfifo_valid_entries( UINT32 base_addr );
int kdp_ssp_busy(int base_addr);	
void kdp_ssp_clear_txhw(int base_addr);
void kdp_ssp_clear_rxhw(int base_addr);

/* public api */
void kdp_ssp_clear_tx_buf_index( struct st_ssp_spi *stspi );
void kdp_ssp_clear_tx_current_buf_index( struct st_ssp_spi *stspi );
void kdp_ssp_clear_tx_done_flag( struct st_ssp_spi *stspi );
enum e_spi kdp_ssp_statemachine( struct st_ssp_spi *st_spi, enum e_spi espi_flow);

UINT32 kdp_ssp_get_tx_buf_index( struct st_ssp_spi *stspi );
void kdp_ssp_write_buff( struct st_ssp_spi *stspi, UINT8 *src, UINT16 nlen );
UINT16 kdp_ssp_get_tx_current_buf_index( struct st_ssp_spi *stspi );
UINT8 kdp_ssp_get_tx_done_flag( struct st_ssp_spi *stspi );
void kdp_ssp_clear_rx_buf_index( struct st_ssp_spi *stspi );
UINT32 kdp_ssp_get_rx_buf_index( struct st_ssp_spi *stspi );
void kdp_ssp_pre_write_to_fifo( struct st_ssp_spi *stspi, UINT8 target_byte );
void kdp_slave_request_init(void);
void kdp_slave_request_active(void);
void kdp_slave_request_inactive(void);
void Drv_ssp_SPI_master_transmit( struct st_ssp_spi *st_spi , UINT32 rx_target_size, UINT8 rx_all  );
void drv_rx_polling_receive_all( struct st_ssp_spi *stspi );

#endif 
