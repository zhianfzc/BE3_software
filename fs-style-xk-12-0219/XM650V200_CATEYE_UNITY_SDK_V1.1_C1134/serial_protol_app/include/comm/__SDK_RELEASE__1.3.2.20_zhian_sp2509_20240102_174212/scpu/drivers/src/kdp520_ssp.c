#include "kdp520_ssp.h"
#include "kl520_com.h"
#include "types.h"
#include "io.h"
#include "delay.h"
#include "kdp520_gpio.h"
#include "kdp_ddr_table.h"
#include "dbg.h"

#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
#if (1==CFG_SSP0_ENABLE) || (1==CFG_SSP1_ENABLE)

//#define SSP_WAIT_REMOVE
#define SPI_Buffer_size     (KDP_DDR_DRV_COM_BUS_RESERVED)

UINT8   *gRx_buff_SP_SLAVE = (UINT8 *)KDP_DDR_DRV_COM_BUS_RX0_START_ADDR ;          //(UINT8 *)KDP_DDR_DRV_SSP1_RX0_START_ADDR;
volatile UINT8  *gTx_buff_SP_SLAVE = (UINT8 *)KDP_DDR_DRV_COM_BUS_TX_START_ADDR;    //(UINT8 *)KDP_DDR_DRV_SSP1_TX_START_ADDR;
UINT8   gRx_buff_temp[300];

UINT32  gTx_buff_index_SP_SLAVE = 0;    //data length
UINT32  gTx_buff_current_index_SP_SLAVE = 0 ;
UINT32  gRx_buff_index_SP_SLAVE = 0;    //data length




#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )

#define SSP_MASTER_BUFFER   (2048)
UINT8   gTx_buff_SP_MASTER[SSP_MASTER_BUFFER];
UINT8   gRx_buff_SP_MASTER[SSP_MASTER_BUFFER];
UINT32  gTx_buff_index_SP_MASTER = 0;    //data length
UINT32  gTx_buff_current_index_SP_MASTER = 0 ;
UINT32  gRx_buff_index_SP_MASTER = 0;    //data length
#endif

#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
struct st_ssp_spi driver_ssp_master_ctx;
#endif
struct st_ssp_spi driver_ssp_ctx;

int sclkdiv =  0; 	//0; //1; /*7:650kHZ*/
int debug = 0;
int sdl_in_bytes = 16;		//1; //12;
int pcl = 0; //25; //8; //*0*/8;

/*SPI2AHB*/
#define HSPI_RD_CMD (0 << 31)
#define HSPI_WR_CMD (1 << 31)

const char *ffmt_str[] = { "SSP", "SPI", "MicroWire", "I2S", "ACL", "SPDIF" };

extern void kdp520_lcm_set_cs(u8 lv);


#if (SSP_SPI_TIME_TEST_EN == YES)

void _ssp_gpio_debug(void)
{
    UINT32	data;
    //pad mux
    data = inw(SCU_EXTREG_PA_BASE + 0x184);
    data &= 0xFFFFFFF8;		//clear low 3bit
    outw(SCU_EXTREG_PA_BASE + 0x184, data | 0x3);

    data = inw(SCU_EXTREG_PA_BASE + 0x188);
    data &= 0xFFFFFFF8;		//clear low 3bit
    outw(SCU_EXTREG_PA_BASE + 0x188, data | 0x3);

    data = inw(SCU_EXTREG_PA_BASE + 0x18C);
    data &= 0xFFFFFFF8;		//clear low 3bit
    outw(SCU_EXTREG_PA_BASE + 0x18C, data | 0x3);

    data = inw(SCU_EXTREG_PA_BASE + 0x190);
    data &= 0xFFFFFFF8;		//clear low 3bit
    outw(SCU_EXTREG_PA_BASE + 0x190, data | 0x3);

    kdp520_gpio_setdir(  24, GPIO_DIR_OUTPUT );
    kdp520_gpio_setdir(  25, GPIO_DIR_OUTPUT );
    kdp520_gpio_setdir(  26, GPIO_DIR_OUTPUT );
    kdp520_gpio_setdir(  27, GPIO_DIR_OUTPUT );

    //GPO set
    kdp520_gpio_setdata( 1<<25);
    kdp520_gpio_setdata( 1<<24);
    kdp520_gpio_setdata( 1<<26);
    kdp520_gpio_setdata( 1<<27);

    //GPO clear
    kdp520_gpio_cleardata( 1<<24);
    kdp520_gpio_cleardata( 1<<25);
    kdp520_gpio_cleardata( 1<<26);
    kdp520_gpio_cleardata( 1<<27);


    kdp520_gpio_setdata( 1<<25);
    kdp520_gpio_setdata( 1<<24);
    kdp520_gpio_setdata( 1<<26);
    kdp520_gpio_setdata( 1<<27);

    kdp520_gpio_cleardata( 1<<24);
    kdp520_gpio_cleardata( 1<<25);
    kdp520_gpio_cleardata( 1<<26);
    kdp520_gpio_cleardata( 1<<27);

//
//	data = inw(SCU_EXTREG_PA_BASE + 0x158);
//	data &= 0xFFFFFFF8;		//clear low 3bit
//	outw(SCU_EXTREG_PA_BASE + 0x158, data | 0x3);
//
//	kdp520_gpio_setdir(  18, GPIO_DIR_OUTPUT );
//	while(1)
//	{
//		kdp520_gpio_setdata( 1<<18);
//		kdp520_gpio_cleardata( 1<<18);
//	}
}
#endif

void ssp_set_sclkdiv(int base)
{
    int cr1;
    int clksrc;
    
    clksrc= inw(SCU_EXTREG_PA_BASE + 0x34);
    clksrc &= ~0x00ff00ff;//clear spi0/1 master clk divider setting.
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN )
#if ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP0 ) == COM_BUS_SSP0 )
    clksrc|= 0x01;//0x02;//0x1F;
#elif ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP1 ) == COM_BUS_SSP1 )
    clksrc|= 0x00010000;
#endif
#endif
    outw( SCU_EXTREG_PA_BASE+0x34 , clksrc);
    
    cr1 = inw(base + SSP_REG_CR1);
    cr1 &= ~ssp_CR1_SCLKDIV_MASK;
    cr1 |= ssp_CR1_SCLKDIV(sclkdiv);

    outw( base + SSP_REG_CR1 , cr1);

}

void ssp_ssp_reset(int base_addr)
{
    int cr2;

    cr2 = inw(base_addr + SSP_REG_CR2);

    cr2 |= ssp_CR2_SSPRST;

    outw( base_addr + SSP_REG_CR2 , cr2);
}

void kdp_ssp_clear_txhw(int base_addr)
{
    int cr2;

    cr2 = inw(base_addr + SSP_REG_CR2);

    cr2 |= ssp_CR2_TXFCLR;

    outw( base_addr + SSP_REG_CR2, cr2);
}

void kdp_ssp_clear_rxhw(int base_addr)
{
    int cr2;

    cr2 = inw(base_addr + SSP_REG_CR2);

    cr2 |= ssp_CR2_RXFCLR;

    outw( base_addr + SSP_REG_CR2, cr2);
}
/**
 * CR1.SDL has 7 bits means maximum: 2^7 = 127 + 1 bits.
 * 128 bits = 16 bytes
 */
int ssp_set_data_length(int base_addr, int sdl)
{
    int cr1;
    //convert to bits;
    sdl <<= 3;
    sdl -= 1;
    if (sdl & ~ssp_SDL_MAX_BYTES_MASK) {
        return 1;
    }

//sdl=0x40;
    cr1 = inw(base_addr + SSP_REG_CR1);
    cr1 &= ~ssp_CR1_SDL_MASK;

    cr1 |= ssp_CR1_SDL(sdl);

    outw(  base_addr + SSP_REG_CR1, cr1 );

    return 0;
}


int ssp_set_data_length_New(int base_addr, int sdl)
{
    int cr1;





//sdl=0x40;
    cr1 = inw(base_addr + SSP_REG_CR1);
    cr1 &= ~ssp_CR1_SDL_MASK;

    cr1 |= ssp_CR1_SDL(sdl);

    outw( base_addr + SSP_REG_CR1, cr1 );

    return 0;
}

void ssp_spi_enable(int base_addr, int tx, int rx)
{
    int cr2 = 0;

    if( tx == 0 )
    {
        cr2 = inw( base_addr + SSP_REG_CR2 );
        cr2 &= ~(ssp_CR2_SSPEN | ssp_CR2_TXDOE);
        outw( base_addr + SSP_REG_CR2, cr2);
    }

    if( rx == 0 )
    {
        cr2 = inw( base_addr + SSP_REG_CR2 );
        cr2 &= ~(ssp_CR2_SSPEN | ssp_CR2_TXDOE);
        outw( base_addr + SSP_REG_CR2, cr2);
    }

    if (tx || rx)
    {
        cr2 = (ssp_CR2_SSPEN | ssp_CR2_TXDOE);

        if (tx)
            cr2 |= ssp_CR2_TXEN;

        if (rx)
            cr2 |= ssp_CR2_RXEN;

        outw( base_addr + SSP_REG_CR2, cr2);

    }
}

void ssp_enable_fs(int base_addr, int tx, int rx, int fs)
{
    int cr2 = 0;

    if (tx || rx)
        cr2 = (ssp_CR2_SSPEN | ssp_CR2_TXDOE);

    if (tx)
        cr2 |= ssp_CR2_TXEN;

    if (rx)
        cr2 |= ssp_CR2_RXEN;
    
    if (fs)
        cr2 |= ssp_CR2_FS;

    outw( base_addr + SSP_REG_CR2 , cr2);

    if (debug)
    {


    }
}

/**
 * Return the number of entries TX FIFO can be written to
 */
int ssp_txfifo_depth( UINT32 base_addr )
{
    int depth;

    depth = ssp_FEA_TXFIFO_DEPTH(
            inw(base_addr + SSP_REG_FEATURE));
    depth += 1;
    return depth;
}

int ssp_txfifo_not_full(int base_addr)	//0: full, 1: not full
{
    int sts;

    sts = inw(base_addr + SSP_REG_STS);

    return (sts & ssp_STS_TFNF);
}


/**
 * Return the number of entries RX FIFO can be written to
 */
int ssp_rxfifo_depth(UINT32 base_addr)
{
    int depth;

    depth = ssp_FEA_RXFIFO_DEPTH(
            inw(base_addr + SSP_REG_FEATURE));
    depth += 1;
    return depth;
}

int ssp_rxfifo_full( UINT32 base_addr )	//1:full, 0: not full
{
    int sts;

    sts = inw(base_addr + SSP_REG_STS);

    return (sts & ssp_STS_RFF);
}

void ssp_spi_set_interrupt( UINT32 nbase ,UINT8	nval)
{
    UINT32 data = 0;

    if( (nval & 0x08) == 0x08 )
    {
        data = inw(nbase + SSP_REG_INTR_CR );
        data &= ~(0x08);
        data |= (0x08);
        outw(nbase + SSP_REG_INTR_CR, data  );
    }

    if( (nval & 0x04 ) == 0x04 )
    {
        data = inw(nbase + SSP_REG_INTR_CR );
        data &= ~(0x04);
        data |= (0x04);
        outw(nbase + SSP_REG_INTR_CR, data );
    }
}



UINT32 ssp_get_rxfifo_int_thflag( UINT32 base_addr)
{
    UINT32 ent;

    ent = ( inw(base_addr + SSP_REG_INTR_STS) &0x04) >>2;


    return ent;
}


UINT32 ssp_get_txfifo_int_thflag( UINT32 base_addr )
{
    UINT32 ent;

    ent = (inw(base_addr + SSP_REG_INTR_STS)&0x08) >>3;


    return ent;
}

static void ssp_write_word(int base, const void *data, int wsize)
{
    unsigned int    tmp = 0;

    if (data) {
        switch (wsize) {
        case 1:
            tmp = *(const UINT8 *)data;
            break;

        case 2:
            tmp = *(const UINT16 *)data;
            break;

        default:
            tmp = *(const UINT32 *)data;
            break;
        }
    }

    outw( base + SSP_REG_DATA_PORT, tmp);
}

void ssp_read_word_new( UINT32 base, volatile UINT8 *buf )
{

    UINT8 data = inw( base + SSP_REG_DATA_PORT );
    *buf = data;
    return;

}

static void ssp_read_word(int base, void *buf, int wsize)
{
    unsigned int    data = inw(base + SSP_REG_DATA_PORT);

    if (buf) {
        switch (wsize) {
        case 1:
            *(UINT8 *) buf = data;
            break;

        case 2:
            *(UINT16 *) buf = data;
            break;

        default:
            *(UINT32 *) buf = data;
            break;
        }
    }
}

/**
 * len unit is bytes
 *
 * Return number of fifo written.
 */
int ssp_fill_in_fifo(int tx_addr, const void *buf,
              int rx_addr, int *len)
{
    int count = 0;
    int rxfifo, fifo, wsize, i;


    rxfifo = ssp_rxfifo_depth(rx_addr);	//get Rx FIOF level


    //clear before start filling in
    kdp_ssp_clear_txhw(tx_addr);

    i = 0;
    while (ssp_txfifo_not_full(tx_addr))
    {

        if (!*len || !buf)
            break;

        if (i == 0) {
            i = sdl_in_bytes;
            fifo = (sdl_in_bytes + 3) / 4;
        }

        //rx fifo doesn't have enough entries to receive
        if ((count + fifo) > rxfifo)
            break;

        if (i > 3)
            wsize = 4;
        else
            wsize = i;

        ssp_write_word(tx_addr, buf, wsize);

        i -= wsize;
        *len -= wsize;
        fifo--;
        count++;
        // Always add 4 bytes for buffer pointer
        (*(INT8U*)buf) += 4;
    }

    return count;
}

/**
 * count is the number of FIFO entries wanted to be read.
 *
 * Return the remaining number of fifo not read yet.
 */
int ssp_take_out_fifo(int rx_addr, void *buf, int count)
{
    int i, wsize;

    i = 0;
    while (count) {

        if (!buf)
            break;

        while (!kdp_ssp_rxfifo_valid_entries(rx_addr)) {
            ;
        }

        if (i == 0)
            i = sdl_in_bytes;

        if (i > 3)
            wsize = 4;
        else
            wsize = i;

        ssp_read_word(rx_addr, buf, wsize);

        i -= wsize;
        count--;
        (*(INT8U*)buf) += 4;

    }

    return count;
}

/**
 * count is the number of FIFO entries wanted to be read.
 *
 * Return the remaining number of fifo not read yet.
 */
int ssp_take_out_fifo_bidirect(int rx1_addr, void *buf1,
                    int rx2_addr, void *buf2,
                    int count)
{
    int i, wsize;

    i = 0;
    while (count) {

        if (!buf1 || !buf2)
            break;

        while (!kdp_ssp_rxfifo_valid_entries(rx1_addr)) {
            ;
        }

        if (i == 0)
            i = sdl_in_bytes;

        if (i > 3)
            wsize = 4;
        else
            wsize = i;

        ssp_read_word(rx1_addr, buf1, wsize);
        ssp_read_word(rx2_addr, buf2, wsize);

        i -= wsize;
        count--;
        (*(INT8U*)buf1) += 4;
        (*(INT8U*)buf2) += 4;

    }

    return count;
}





void ssp_write_byte_new(int base, volatile UINT8 *data)
{
    outw( base + SSP_REG_DATA_PORT , *data );
}




int ssp_set_pcl(int base, int val)
{
    int cr3;

    if (val & ~ssp_CR3_PCL_MASK) {
        return 1;
    }

    cr3 = inw(base + SSP_REG_CR3);
    cr3 &= ~ssp_CR3_PCL_MASK;
    cr3 |= ssp_CR3_PCL(val);

    outw( base + SSP_REG_CR3, cr3);

    return 0;
}




void drv_ssp_spi_tx_fifo_threshold(UINT32 nbase , UINT8 nval)
{
    UINT32	ntemp ;

    ntemp = inw(nbase + SSP_REG_INTR_CR);

    ntemp &= ~ssp_INTCR_TFTHOD_MASK;

    ntemp = ntemp  | ( ssp_INTCR_TFTHOD( nval )	 );
    outw(  nbase + SSP_REG_INTR_CR , ntemp);

}


void drv_ssp_spi_rx_fifo_threshold(UINT32 nbase, UINT8 nval)
{
    UINT32	ntemp ;

    ntemp = inw(nbase + SSP_REG_INTR_CR);

    ntemp &= ~ssp_INTCR_RFTHOD_MASK;

    ntemp = ntemp  | ( ssp_INTCR_RFTHOD( nval )	 );
    outw(  nbase + SSP_REG_INTR_CR , ntemp);

}


/**************************************************
UINT32 nbase_address: SSP base address
UINT16 ndata: config SPI signal type
UINT8 nmast_slave: 0 or 1:SPI as slave, 2 or 3:SPI as master
 **************************************************/

void drv_ssp_spi_init(UINT32 nbase_address, UINT16 ndata , UINT8 nmast_slave )
{
    UINT32	ntemp ;

    ssp_ssp_reset(nbase_address);


#if 0
    ntemp = ntemp | (ssp_CR0_FFMT_SPI | ssp_CR0_MSTR_SPI | ssp_CR0_SPI_FLASH) ;
    outw( SPI_ssp_1_PA_BASE + SSP_REG_CR0 , ntemp);
#else

    if( nmast_slave == 0 ||  nmast_slave == 1 )
    {
        //SPI act as slave
        ntemp = ndata | ( ssp_CR0_FFMT_SPI | ssp_CR0_SLV_SPI );
        outw(  nbase_address + SSP_REG_CR0 , ntemp);
    }
    else
    {
        //SPI act as master
        ntemp = ndata | ( ssp_CR0_FFMT_SPI | ssp_CR0_MSTR_SPI );
        outw(  nbase_address + SSP_REG_CR0 , ntemp);
    }

#endif

    ssp_set_sclkdiv( nbase_address );


    ssp_set_data_length_New( nbase_address, sdl_in_bytes );


    ssp_set_pcl(nbase_address, pcl);

}

 void drv_ssp_spi_tx_dma_En( UINT32 nbase)
 {
     UINT32	ntemp ;

     ntemp = inw(nbase + SSP_REG_INTR_CR);

     ntemp = ntemp  | ( ssp_INTCR_TFDMAEN );
    // outw(  nbase + SSP_REG_INTR_CR , ntemp );

 }

// void Drv_ssp_spi_rx_dma_en(UINT32 nbase)
// {
//     UINT32	ntemp ;

//     ntemp = inw(nbase + SSP_REG_INTR_CR);

//     ntemp = ntemp  | ( ssp_INTCR_RFDMAEN );
//     outw(  nbase + SSP_REG_INTR_CR , ntemp);

// }

static void _kdp520_ssp_IP_init( struct st_ssp_spi *st_spi )
{
    UINT32 data;

    //spi clock gate setting,
    data = inw(SCU_EXTREG_PA_BASE + 0x1C);
    outw(SCU_EXTREG_PA_BASE + 0x1C, data | 0x55);

#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN )
#if ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP0 ) == COM_BUS_SSP0 )
    if( st_spi->IP_type == 3 )
    {
        data = inw(SCU_EXTREG_PA_BASE + 0x20);
        outw(SCU_EXTREG_PA_BASE + 0x20, data | 0x4 );
    }
#elif ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP1 ) == COM_BUS_SSP1 )
    if( st_spi->IP_type == 3 )
    {
        data = inw(SCU_EXTREG_PA_BASE + 0x20);
        outw(SCU_EXTREG_PA_BASE + 0x20, data | 0x8 );
    }
#endif
#endif

    sdl_in_bytes = st_spi->SDL;
    drv_ssp_spi_init( st_spi->reg_base_address , ssp_CR0_SCLKPO_0 | FTSSP020_CR0_SCLKPH_0,  st_spi->IP_type );

    //SPI DMA setting......
    drv_ssp_spi_tx_dma_En( st_spi->reg_base_address );	//SPI0 as slave
    //Drv_ssp_spi_rx_dma_en(st_spi->reg_base_address);

    //clear before start filling in
    kdp_ssp_clear_txhw( st_spi->reg_base_address );
    kdp_ssp_clear_rxhw( st_spi->reg_base_address );

    //add interrupt IRQ enable bit!!
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
    if( (st_spi->interrupt_en &0x08)>0 || (st_spi->interrupt_en &0x04)>0 )
#endif
    {
        ssp_spi_set_interrupt( st_spi->reg_base_address, st_spi->interrupt_en );      // TxFIFO threshold:  1<<3, RxFIFO threshold flag: 1<<2
    }
    drv_ssp_spi_tx_fifo_threshold( st_spi->reg_base_address, SPI_TX_FIFO_TH );
    drv_ssp_spi_rx_fifo_threshold( st_spi->reg_base_address, SPI_RX_FIFO_TH );
}

int kdp_ssp_rxfifo_valid_entries( UINT32 base_addr )
{
    int ent;

    ent = ssp_STS_RFVE(inw(base_addr + SSP_REG_STS));

    return ent;
}

int kdp_ssp_busy(int base_addr)
{
    return (((inw(base_addr + SSP_REG_STS) ) >> 2) & 0x1);
}

//check Tx done flag
void kdp_ssp_set_tx_done_flag( struct st_ssp_spi *stspi )
{
    stspi->Tx_done_flag = 1;

    #ifdef COM_BUS_RESPONSE_REQUEST_PIN
    if (stCom_type.flags == KL520_COM_HAS_ADDITIONAL_IO)
    {
        kdp_slave_request_inactive();
    }
    #endif
}

UINT8 kdp_ssp_get_tx_done_flag( struct st_ssp_spi *stspi )
{
    return stspi->Tx_done_flag ;
}

void drv_ssp_tx_write_fifo( struct st_ssp_spi *stspi )
{

    UINT32	base_address = stspi->reg_base_address;
    volatile UINT8 dummy_data = 0xAA;
    UINT8 i =0;

    for( i=0; i < ( SPI_TX_FIFO_TH * 2 ); i++ )
    {
        #if (SSP_SPI_TIME_TEST_EN == YES)
        kdp520_gpio_setdata( 1<<24 );	//debug use
        #endif

        if(  *stspi->Tx_buffer_current_index < *stspi->Tx_buffer_index  )
        {

            //write data
            ssp_write_byte_new( base_address, ( stspi->Tx_buffer+ *stspi->Tx_buffer_current_index ) );
            *stspi->Tx_buffer_current_index = *stspi->Tx_buffer_current_index+1;

        }
        else
        {
        ssp_write_byte_new( base_address, &dummy_data );
        kdp_ssp_set_tx_done_flag(stspi);
        //			break;
        }

        #if (SSP_SPI_TIME_TEST_EN == YES)
        kdp520_gpio_cleardata( 1<<24 );
        #endif

        if(  ssp_txfifo_not_full(base_address) == 0 )
        {
            break;
        }
    }
}


void drv_ssp_rx_read_fifo_partial(  struct st_ssp_spi *stspi )
{
    UINT32	base_address = stspi->reg_base_address;
    UINT8 nsize = kdp_ssp_rxfifo_valid_entries( base_address );
    UINT8 i=0;

    //Rx
    for( i=0; i<nsize; i++  )
    {
        #if (SSP_SPI_TIME_TEST_EN == YES)
        kdp520_gpio_setdata( 1<<25 );
        #endif

        ssp_read_word_new( base_address , stspi->Rx_buffer+ *stspi->Rx_buffer_index  );
        *stspi->Rx_buffer_index = *stspi->Rx_buffer_index + 1 ;

        #if (SSP_SPI_TIME_TEST_EN == YES)
        kdp520_gpio_cleardata( 1<<25);
        #endif
    }
}




void _ssp_spi_irqhandler( void )
{
#if( OTA_TIMING_DEBUG_EN == YES ) 
    //timing debug
    kdp520_gpio_setdata( 1<<26);
    #endif

    if(  ssp_get_txfifo_int_thflag( driver_ssp_ctx.reg_base_address ) != 0 )
    {
        drv_ssp_tx_write_fifo( &driver_ssp_ctx );
    }
    if( ssp_get_rxfifo_int_thflag( driver_ssp_ctx.reg_base_address ) != 0 )
    {
        drv_ssp_rx_read_fifo_partial( &driver_ssp_ctx );
    }

    #if( OTA_TIMING_DEBUG_EN == YES )
    //timimg debug
    kdp520_gpio_cleardata( 1<<26);
    #endif



}


void _ssp_spi_enable( struct st_ssp_spi *st_spi )
{

    if( st_spi->IP_type == 0 )
    {
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN )
#if ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP0 ) == COM_BUS_SSP0 )
        NVIC_SetVector( (IRQn_Type)SSP_FTSSP010_1_IRQ, (u32)_ssp_spi_irqhandler );
        NVIC_EnableIRQ( SSP_FTSSP010_1_IRQ );
#elif ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP1 ) == COM_BUS_SSP1 )
        NVIC_SetVector( (IRQn_Type)SSP_FTSSP010_1_1_IRQ, (u32)_ssp_spi_irqhandler );
        NVIC_EnableIRQ( SSP_FTSSP010_1_1_IRQ );
#endif
#endif
    }

    //SPI enable
    ssp_spi_enable( st_spi->reg_base_address, 1, 1);
}

void _ssp_spi_disable( struct st_ssp_spi *st_spi )
{
    if( st_spi->IP_type == 0 )
    {
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN )
#if ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP0 ) == COM_BUS_SSP0 )
        NVIC_DisableIRQ(SSP_FTSSP010_1_IRQ);
#elif ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP1 ) == COM_BUS_SSP1 )
        NVIC_DisableIRQ(SSP_FTSSP010_1_1_IRQ);
#endif
#endif
    }


    //SPI enable
    ssp_spi_enable(st_spi->reg_base_address, 0, 0);
}

void drv_rx_polling_receive_all( struct st_ssp_spi *stspi )
{

    UINT32	base_address = stspi->reg_base_address;

#if 1
    UINT8   temp = 100, nfetch;
    while( (nfetch = kdp_ssp_rxfifo_valid_entries( base_address )) != temp )
    {
        temp = nfetch;
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
        delay_us(5);
#else
        delay_us(50);
#endif
    }

    #if 1
    while( (UINT8)kdp_ssp_rxfifo_valid_entries( base_address )  != 0 )
    {
        nfetch--;
        #if (SSP_SPI_TIME_TEST_EN == YES)
        kdp520_gpio_setdata( 1<<27);
        #endif
        ssp_read_word_new( base_address , ( stspi->Rx_buffer+ (*stspi->Rx_buffer_index) )  );
        *stspi->Rx_buffer_index = *stspi->Rx_buffer_index + 1 ;
        #if (SSP_SPI_TIME_TEST_EN == YES)
        kdp520_gpio_cleardata( 1<<27);
        #endif
    }
    #endif

#else
    //org
    UINT8 ntemp =0, i ;
    ntemp  = (UINT8)kdp_ssp_rxfifo_valid_entries( base_address ) ;

    //Rx
    for( i=0 ; i < ntemp ; i++ )
    {
        #if (SSP_SPI_TIME_TEST_EN == YES)
        kdp520_gpio_setdata( 1<<27);
        #endif

        ssp_read_word_new( base_address , ( stspi->Rx_buffer+ (*stspi->Rx_buffer_index) )  );
        *stspi->Rx_buffer_index = *stspi->Rx_buffer_index + 1 ;

        #if (SSP_SPI_TIME_TEST_EN == YES)
        kdp520_gpio_cleardata( 1<<27);
        #endif

    }
#endif
}


/**************************************************
 * ************************************************
**************************************************
**************************************************/

static void kdp_520_ssp_spi_slave_par_init(struct st_ssp_spi *st_spi)
{
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN )
#if ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP0 ) == COM_BUS_SSP0 )
    st_spi->reg_base_address = SSP_REG_BASE_M;
#elif ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP1 ) == COM_BUS_SSP1 )
    st_spi->reg_base_address = SSP_REG_BASE_S;
#endif
#endif
    st_spi->SDL = 7;
    st_spi->target_Txfifo_depth = ssp_txfifo_depth( st_spi->reg_base_address );
    st_spi->target_Rxfifo_depth = ssp_rxfifo_depth( st_spi->reg_base_address );
    st_spi->tx_rx_en = 3;

    st_spi->interrupt_en = 0x04;
    st_spi->IP_type = 0;
    st_spi->spi_sw_type = 1;

    st_spi->Tx_buffer = (UINT8 *)gTx_buff_SP_SLAVE;
    //st_spi->Tx_buffer_index = &gcom_tx_index ; //&gTx_buff_index_SP_SLAVE;
    st_spi->Tx_buffer_current_index = &gTx_buff_current_index_SP_SLAVE;
    st_spi->Rx_buffer = (UINT8 *)gRx_buff_SP_SLAVE;
    //st_spi->Rx_buffer_index = &gcom_rx_index;	//&gRx_buff_index_SP_SLAVE;
    st_spi->buffer_max_size = KDP_DDR_DRV_COM_BUS_RESERVED;
    st_spi->Rx_tempbuffer = gRx_buff_temp;
}

#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN )
static void kdp_520_ssp_spi_master_par_init(struct st_ssp_spi *st_spi)
{
#if ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP0 ) == COM_BUS_SSP0 )
    st_spi->reg_base_address = SSP_REG_BASE_M;
#elif ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP1 ) == COM_BUS_SSP1 )
    st_spi->reg_base_address = SSP_REG_BASE_S;
#endif

    st_spi->SDL = 15;
    st_spi->target_Txfifo_depth = ssp_txfifo_depth( st_spi->reg_base_address );
    st_spi->target_Rxfifo_depth = ssp_rxfifo_depth( st_spi->reg_base_address );
    st_spi->tx_rx_en = 3;

    st_spi->interrupt_en = 0x00;
    st_spi->IP_type = 3;
    st_spi->spi_sw_type = 0;

    st_spi->Tx_buffer = (UINT8 *)gTx_buff_SP_MASTER;
    //st_spi->Tx_buffer_index = &gTx_buff_index_SP_MASTER;
    st_spi->Tx_buffer_current_index = &gTx_buff_current_index_SP_MASTER;
    st_spi->Rx_buffer = (UINT8 *)gRx_buff_SP_MASTER;
    //st_spi->Rx_buffer_index = &gRx_buff_index_SP_MASTER;
    st_spi->buffer_max_size = SSP_MASTER_BUFFER;
    st_spi->Rx_tempbuffer = gRx_buff_temp;
}
#endif

static void _ssp_spi_slave_init(struct st_ssp_spi *st_spi)
{
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
    UINT32 data;
#endif
#if (SSP_SPI_TIME_TEST_EN == YES)
    _ssp_gpio_debug();
#endif
    kdp_520_ssp_spi_slave_par_init(st_spi);


#if ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP0 ) == COM_BUS_SSP0 )

    data = ( inw(SCU_EXTREG_PA_BASE + 0x120) &0xFFFFFFF8) | (1<< 6) | 1<<3;
    outw(SCU_EXTREG_PA_BASE + 0x120, data | 0x2) ;
    data = ( inw(SCU_EXTREG_PA_BASE + 0x11C) &0xFFFFFFF8) | (1<< 6) | 1<<3;
    outw(SCU_EXTREG_PA_BASE + 0x11C, data | 0x2);
    data = ( inw(SCU_EXTREG_PA_BASE + 0x128) &0xFFFFFFF8) | (1<< 6) | 1<<3;
    outw(SCU_EXTREG_PA_BASE + 0x128, data | 0x2);
    data = ( inw(SCU_EXTREG_PA_BASE + 0x124) &0xFFFFFFF8) | (1<< 6) | 1<<3;
    outw(SCU_EXTREG_PA_BASE + 0x124, data | 0x2);

#elif ( ( CFG_COM_BUS_TYPE&COM_BUS_SSP1 ) == COM_BUS_SSP1 )
    data = (inw(SCU_EXTREG_PA_BASE + 0x15c)&0xFFFFFFF8) | (1<< 6) | 1<<3;
    outw(SCU_EXTREG_PA_BASE + 0x15c, data | 0x2) ;

    data = ( inw(SCU_EXTREG_PA_BASE + 0x160)&0xFFFFFFF8) | (1<< 6) | 1<<3;
    outw(SCU_EXTREG_PA_BASE + 0x160, data | 0x2);

    data = ( inw(SCU_EXTREG_PA_BASE + 0x164)&0xFFFFFFF8) | (1<< 6) | 1<<3;
    outw(SCU_EXTREG_PA_BASE + 0x164, data | 0x2);
    data = (inw(SCU_EXTREG_PA_BASE + 0x168) &0xFFFFFFF8) | (1<< 6) | 1<<3;
    outw(SCU_EXTREG_PA_BASE + 0x168, data | 0x2);
#endif

}


#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
static void _kl520_api_ssp_spi_master_init(struct st_ssp_spi *st_spi)
{
    kdp_520_ssp_spi_master_par_init(st_spi);
    kdp520_lcm_set_cs(1);
}
#endif

//clear Tx sw buf index
void kdp_ssp_clear_tx_buf_index( struct st_ssp_spi *stspi )
{
    *stspi->Tx_buffer_index = 0;
}

//Get Tx sw buf index
UINT32 kdp_ssp_get_tx_buf_index( struct st_ssp_spi *stspi )
{
    return *stspi->Tx_buffer_index;
}

//check Tx current buf index
void kdp_ssp_clear_tx_current_buf_index( struct st_ssp_spi *stspi )
{
    *stspi->Tx_buffer_current_index = 0;
}

UINT16 kdp_ssp_get_tx_current_buf_index( struct st_ssp_spi *stspi )
{
    return *stspi->Tx_buffer_current_index;
}

//clear Tx done flag
void kdp_ssp_clear_tx_done_flag( struct st_ssp_spi *stspi )
{
    stspi->Tx_done_flag = 0;
}

//clear Rx sw buf index
void kdp_ssp_clear_rx_buf_index( struct st_ssp_spi *stspi )
{
    *stspi->Rx_buffer_index = 0;

}


//Get Rx sw buf index
UINT32 kdp_ssp_get_rx_buf_index( struct st_ssp_spi *stspi )
{
    return *stspi->Rx_buffer_index;
}



void kdp_slave_request_active(void)
{
#ifdef COM_BUS_RESPONSE_REQUEST_PIN
    kdp520_gpio_setdata( 1<<COM_BUS_RESPONSE_REQUEST_PIN );
#endif
}

void kdp_slave_request_inactive(void)
{
#ifdef COM_BUS_RESPONSE_REQUEST_PIN
    kdp520_gpio_cleardata( 1<<COM_BUS_RESPONSE_REQUEST_PIN );
#endif
}

void kdp_ssp_write_buff( struct st_ssp_spi *stspi, UINT8 *src, UINT16 nlen )
{
    UINT16 i = 0;
    *stspi->Tx_buffer_current_index = 0;
    *stspi->Tx_buffer_index = 0;

    for( i = 0; i < nlen; i++ )
    {
        *( stspi->Tx_buffer + i ) = *( src + i );
        *stspi->Tx_buffer_index = *stspi->Tx_buffer_index + 1;
    }
}

void kdp_ssp_pre_write_to_fifo( struct st_ssp_spi *stspi, UINT8  target_byte )
{
    volatile UINT8	*tx_buf = stspi->Tx_buffer;
    volatile UINT32	*tx_buf_current_index = stspi->Tx_buffer_current_index;
    volatile UINT32	*tx_buf_index = stspi->Tx_buffer_index;
    UINT16 i = 0;
    UINT8	ntemp = 0xDD;


    //dbg_msg( "xor loop count : %d ", target_byte );


    for( i = 0; i < target_byte; i++ )
    {
        if( *tx_buf_current_index < *tx_buf_index )
        {
            //dbg_msg( "*tx_index : %d, 0x%x", *tx_buf_index , *(tx_buf+i)  );
            ssp_write_byte_new( stspi->reg_base_address , (tx_buf+i) );
            *tx_buf_current_index = *tx_buf_current_index + 1;
        }
        else
        {
            ssp_write_byte_new( stspi->reg_base_address , &ntemp );
        }
    }
}

UINT8 Drv_ssp_SPI_XOR_check(struct st_ssp_spi *stspi)
{
    UINT16	i = 0;
    UINT16 rx_size = *stspi->Rx_buffer_index;
    volatile UINT8	*rx_ptr = stspi->Rx_buffer;
    UINT8 ntemp = 0;

    ntemp = *rx_ptr;
    for( i=1 ; i < (rx_size-1) ; i++ )
    {
        ntemp ^= *(rx_ptr +i);
    }

    if( ntemp != *(rx_ptr+rx_size-1) )
    {
        return NO;		//xor check fail
    }
    return YES;		 //xor check pass
}

void Drv_ssp_SPI_XOR_clc( struct st_ssp_spi *stspi )
{
    UINT16	i = 0;
    volatile UINT32 *tx_size = stspi->Tx_buffer_index;
    volatile UINT8	*tx_ptr = stspi->Tx_buffer;
    UINT8 ntemp = 0;

    ntemp = *tx_ptr;
    for( i=1 ; i < *tx_size ; i++ )
    {
        ntemp ^= *(tx_ptr +i);
    }

    *( tx_ptr + *tx_size ) = ntemp;
    *tx_size = *tx_size + 1 ;
}



/************************************************************************
    SPI master read and write data continuously in the same transaction time
    support write command and read data in the same time.
    rx_all: 0: receive data after transmit done
            1: receive all data during transmit and receive
    ps:
        step 1. assign write command data size
        step 2. assign read data size
        step 3. SPI transmit write command and then read data continuously
************************************************************************/
 void Drv_ssp_SPI_master_transmit( struct st_ssp_spi *st_spi , UINT32 rx_target_size, UINT8 rx_all )
{

    UINT32 i;
    UINT32  tx_size = *st_spi->Tx_buffer_index;
    //UINT8   dummy = 0xCC;
//    UINT32  data;

    dbg_msg_flash("[SPI]Tx size: %d", tx_size);
    dbg_msg_flash("[SPI]Rx size: %d", rx_target_size);
   // kdp520_lcm_set_cs(0);

    //send tx data
    for ( i = 0; i < tx_size; i++ )
    {
        ssp_write_byte_new( st_spi->reg_base_address , (st_spi->Tx_buffer+i) );
        if( ( i / SPI_TX_FIFO_TH ) != 1 )
        {
            continue;
        }
        while( ( inw(st_spi->reg_base_address + SSP_REG_STS)& (0x3F<<12) ) != 0 );
       // drv_rx_polling_receive_all( st_spi );
    }
    while( ( inw(st_spi->reg_base_address + SSP_REG_STS)& (0x3F<<12) ) != 0 );
    //drv_rx_polling_receive_all( st_spi );

    kdp_ssp_set_tx_done_flag(st_spi);
    if( rx_all != 1)
    {
        kdp_ssp_clear_rx_buf_index( st_spi );
    }

    for ( i = 0; i < rx_target_size; i++ )
    {
        int tmp = 0;//0xFF;
        ssp_write_byte_new( st_spi->reg_base_address , (UINT8 *)&tmp );
        if( ( i % (SPI_MAX_FIFO/2) ) != 0 )
        {
            continue;
        }
//        while( ( inw(st_spi->reg_base_address + SSP_REG_STS)& (0x3F<<12) ) != 0 );
//        drv_rx_polling_receive_all(st_spi);
        while( ( inw(st_spi->reg_base_address + SSP_REG_STS)& (0x3F<<12) ) > ( SPI_TX_FIFO_TH<<12 ) );
      //  drv_ssp_rx_read_fifo_partial(st_spi);
    }

    while( ( inw(st_spi->reg_base_address + SSP_REG_STS)& (0x3F<<12) ) != 0 );
    while( ( inw(st_spi->reg_base_address + SSP_REG_STS) & 0x4 ) != 0 );
   // kdp520_lcm_set_cs(1);
   // drv_rx_polling_receive_all(st_spi);
}



enum e_spi kdp_ssp_statemachine( struct st_ssp_spi *st_spi,  enum e_spi espi_flow )
{
    //pad mux
    //UINT32 data, i =0;

    st_spi->eflow = espi_flow;

    switch( st_spi->eflow )
    {
        case e_spi_init_slave:
        {
            _ssp_spi_slave_init( st_spi );
            _kdp520_ssp_IP_init(st_spi);
            st_spi->eflow = e_spi_idle;
            return e_spi_ret_init_done;
        }
#if ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MS_EN ) && ( ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )&( ~COM_BUS_SPI_MS_EN ) )
        case e_spi_init_master:
        {
            _kl520_api_ssp_spi_master_init( st_spi );
            _kdp520_ssp_IP_init(st_spi);
            st_spi->eflow = e_spi_idle;
            return e_spi_ret_init_done;
        }
#endif
        case e_spi_enable:
        {
            st_spi->pre_size = 0;
            _ssp_spi_enable(st_spi);
            st_spi->eflow = e_spi_idle;
            return e_spi_ret_enable_done;
        }
//			break;
        case e_spi_rx:
        {
            st_spi->eflow = e_spi_idle;

            //if no type2 case this condition will not be implemented
            if( *st_spi->Rx_buffer_index > 4 ){
            //call a function to judge first 3 Bytes
            #if 0
            //check head type
            st_spi->eflow = e_spi_rx_large;
            //	or
            st_spi->eflow = e_spi_tx_large;
            break;
            #endif
            }

            if( /*st_spi->pre_size == 0 ||*/ st_spi->pre_size != *st_spi->Rx_buffer_index )
            {
                st_spi->pre_size = *st_spi->Rx_buffer_index;
                #if(SSP_SPI_TIME_TEST_EN ==YES)
                kdp520_gpio_setdata( 1<<26);
                #endif
                //delay_us(20);
                delay_us(10);
                #if(SSP_SPI_TIME_TEST_EN ==YES)
                kdp520_gpio_cleardata( 1<<26);
                #endif

                return e_spi_ret_rxbusy;
            }
            else
            {
                #if(SSP_SPI_TIME_TEST_EN ==YES)
                kdp520_gpio_setdata( 1<<24);
                #endif

                st_spi->pre_size = 0;
                //no data to be updated
                //read the remaining data in the FIFO
                drv_rx_polling_receive_all( st_spi );
                #if(SSP_SPI_TIME_TEST_EN ==YES)
                kdp520_gpio_cleardata( 1<<24);
                #endif
//                for( i = 0; i< *(st_spi->Rx_buffer_index) ; i++ ){
//                    *( st_spi->Rx_tempbuffer + st_spi->Rx_tempbuffer_index + i )= *( st_spi->Rx_buffer + i );
//                }
//                st_spi->Rx_tempbuffer_index = st_spi->Rx_tempbuffer_index + *(st_spi->Rx_buffer_index);
                return e_spi_ret_rxdone;
            }
            //return e_spi_ret_rxbusy;
        }
//			break;
        case e_spi_rx_check:
        {
            st_spi->eflow = e_spi_idle;

            //check packet reliability
            if( Drv_ssp_SPI_XOR_check( st_spi ) == 1 )
            {
                return e_spi_ret_rx_xor_OK;			//data is correct
            }
            else
            {
                return e_spi_ret_rx_xor_error ;			//data is in-correct
            }
        }
        //break;
        case e_spi_tx:
        {
            _ssp_spi_disable( st_spi );
            kdp_ssp_clear_rxhw( st_spi->reg_base_address );
            kdp_ssp_clear_txhw( st_spi->reg_base_address );
            kdp_ssp_clear_rx_buf_index( st_spi );
            kdp_ssp_clear_tx_buf_index( st_spi );
            kdp_ssp_clear_tx_current_buf_index( st_spi );
            kdp_ssp_clear_tx_done_flag( st_spi );

            //			kdp_ssp_write_buff( st_spi, temp_buffer, temp_buffer_index );		//for future use
            kdp_ssp_pre_write_to_fifo( st_spi, 5 );
#ifdef COM_BUS_RESPONSE_REQUEST_PIN
            if (stCom_type.flags == KL520_COM_HAS_ADDITIONAL_IO)
            {
                kdp_slave_request_active();                     //send GPIO request
            }
#endif
            _ssp_spi_enable(st_spi);

            st_spi->eflow = e_spi_idle;
            return	e_spi_ret_txbusy;
            }
//			break;
        case e_spi_tx_status_check:
        {
            st_spi->eflow = e_spi_idle;

            if(  kdp_ssp_get_tx_done_flag( st_spi ) == 1 )
            {
#ifdef COM_BUS_RESPONSE_REQUEST_PIN
                if (stCom_type.flags == KL520_COM_HAS_ADDITIONAL_IO)
                {
                    kdp_slave_request_inactive();
                }
#endif
                return e_spi_ret_txdone;
            }
            else
            {
                return e_spi_ret_txbusy;
            }
        }
//			break;
#if 0
        case e_spi_tx_large:	//command and then response same as type 2!

            //do it until done
            //transmit case 1
            //transmit case 2

            st_spi->eflow = e_spi_idle;
            break;
        case e_spi_rx_large:		//maybe no use
            {
                //do it until done
                //receive case 1
                //receive case 2

                st_spi->eflow = e_spi_idle;
            }
            break;
#endif

        case e_spi_master_tx_rx:


            break;

        case e_spi_disable:
            {
                _ssp_spi_disable( st_spi );
                st_spi->eflow = e_spi_idle;
                return e_spi_ret_disableDone;
            }
//			break;
        case e_spi_tx_xor:
            Drv_ssp_SPI_XOR_clc( st_spi );

            return e_spi_ret_tx_xor_done;
        case e_spi_idle:

            break;
        default :
            break;
    }

    return e_spi_ret_idle;
}

/**
 * Enable Slave before Master
 *
 * cs_low equals to 0 means  frame/sync(chip select) active low.
 * lsb equals to 0 means MSB tx first. 
 */
char *mode_string[SPI_MODE_MAX] = { "CLKPO = 0, CLKPHA = 0",
                    "CLKPO = 0, CLKPHA = 1",
                    "CLKPO = 1, CLKPHA = 0",
                    "CLKPO = 1, CLKPHA = 1",
                 };




void kdp_slave_request_init(void)
{
#ifdef COM_BUS_RESPONSE_REQUEST_PIN
    UINT32 data;
    //init GPIO as Output low

    //LC_Data[14]
#if ( COM_BUS_RESPONSE_REQUEST_PIN == 3 )
    data = inw(SCU_EXTREG_PA_BASE + 0x174);
    data &= 0xFFFFFFF8;     //clear low 3bit
    outw(SCU_EXTREG_PA_BASE + 0x174, data | 0x3);
#else
    kdp520_gpio_setmode(COM_BUS_RESPONSE_REQUEST_PIN);
#endif

    kdp520_gpio_setdir( COM_BUS_RESPONSE_REQUEST_PIN , GPIO_DIR_OUTPUT );
    kdp520_gpio_cleardata( 1<<COM_BUS_RESPONSE_REQUEST_PIN );
#endif
}

void ssp_write_tx_buffer( struct st_ssp_spi *sspsw, UINT8 *src_ptr, UINT32 size )
{
    int i = 0;
    *sspsw->Tx_buffer_index = 0;

    for(i = 0; i < size; i++ )
    {
        *(sspsw->Tx_buffer + *sspsw->Tx_buffer_index) = *(src_ptr+i);
        *sspsw->Tx_buffer_index = *sspsw->Tx_buffer_index+1;
    }
}
#define SSP_TEST 0
#if (1==SSP_TEST)
void ssp_wr_byte(u32 base,u8 data)
{
    outw( base + SSP_REG_DATA_PORT , data );
}
int ssp_TRX(u32 base,u8* TX_ptr,u8* RX_ptr,u32 len)
{
    kdp_ssp_clear_txhw(base);
    kdp_ssp_clear_rxhw(base);
    
    while(len)
    {
        while(!(inw(base+SSP_REG_STS) & 0x02));
        outw( base + SSP_REG_DATA_PORT , *TX_ptr );
        while(ssp_STS_RFVE(inw(base + SSP_REG_STS)))
        {
            *RX_ptr=inw(base + SSP_REG_DATA_PORT );
            RX_ptr++;
        }
        len--;
        TX_ptr++;
        
    }
    return 0
}
#endif

#endif
#endif
