#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "kdp520_dma.h"
#include "kneron_mozart.h"
#include "base.h"
#include "io.h"
#include "delay.h"
#include "dbg.h"
#include "flash.h"
#include "kdp520_spi.h"
#include "kl520_include.h"

#define ASSERT(x)   do { \
                        if (!(x)) 	\
                            for (;;)	\
                                ; 		\
                    } while (0)
//#define CN_SIZE			(0x400000-1)
#define CN_SIZE			4080		//==> for test only

#define CPU_TO_AHB_ADDRSPACE 0x23
#define DMAC_FTDMAC020_PA_BASE_SCPU DMAC_FTDMAC020_PA_BASE

#define DMA_WAIT_INT_TIMEOUT_CNT    (0x7FFFF)

int burst_size_selection[] = {1, 4, 8, 16, 32, 64, 128, 256};

volatile kdp520_dma_reg *m_dma_reg = (kdp520_dma_reg *)DMAC_FTDMAC020_PA_BASE_SCPU;
static volatile UINT32	__dma_int_occurred = 0, __dma_tc_int_occurred = 0;
static UINT8 dma_int_initized = 0;
static UINT8 dma_tc_int_initized = 0;

static u32 llp_buf = KDP_DDR_BASE_SYSTEM_RESERVED;

void AHB_DMA_IRQHandler(void)
{
    //kdp_printf("NCPU_AHB_DMA_IRQHandler\n");
    UINT32 status;


    status = m_dma_reg->dma_int;

    if(status)
        __dma_int_occurred |= status;
  else
        return;

    delay_us(10);

    outw(DMAC_FTDMAC020_PA_BASE_SCPU+DMA_INT_TC_CLR, status);
    outw(DMAC_FTDMAC020_PA_BASE_SCPU+DMA_INT_ERRABT_CLR, status);
}


void AHB_DMA_TC_IRQHandler(void)
{
    //kdp_printf("NCPU_AHB_DMA_TC_IRQHandler\n");
    UINT32 status;


    status = m_dma_reg->dma_int_tc;

    if(status)
        __dma_tc_int_occurred |= status;
    else
        return;

    outw(DMAC_FTDMAC020_PA_BASE_SCPU+DMA_INT_TC_CLR, status);
}

int kdp_dma_is_ch_busy(INT32 Channel)
{
    return ((m_dma_reg->dma_ch_busy >> Channel) & 0x1);
}

int kdp_dma_is_ch_enable(INT32 Channel)
{
    return ((m_dma_reg->dma_ch_enable >> Channel) & 0x1);
}

int kdp_dma_get_busy_status(void)
{
    return m_dma_reg->dma_ch_busy;
}

int kdp_dma_get_enable_status(void)
{
    return m_dma_reg->dma_ch_enable;
}

UINT32 kdp_dma_get_int_status(void)
{
    return m_dma_reg->dma_int;
}



UINT32 kdp_dma_get_ch_int_status(INT32 Channel)
{
    volatile UINT32 IntStatus = 0;

    if((m_dma_reg->dma_int >> Channel) & 0x01)
    {
        if((m_dma_reg->dma_int_tc >> Channel) & 0x01)
            IntStatus |= 1;
        if((m_dma_reg->dma_int_err >> Channel) & 0x01)
            IntStatus |= 2;
    }

    return IntStatus;
}


void kdp_dma_init(UINT32 M0_BigEndian, UINT32 M1_BigEndian, UINT32 Sync)
{
    m_dma_reg->dma_csr = (M0_BigEndian ? DMA_CSR_M0ENDIAN : 0) |
    (M1_BigEndian ? DMA_CSR_M1ENDIAN : 0) | DMA_CSR_DMACEN;

    m_dma_reg->dma_sync = Sync;
}


void kdp_dma_enable_ch(INT32 Channel)
{
    UINT32 reg;

    reg = *(UINT32 *)&m_dma_reg->dma_ch[Channel].csr;
    reg |= DMA_CSR_CH_ENABLE;
    *(UINT32 *)&m_dma_reg->dma_ch[Channel].csr = reg;
}

void kdp_dma_disable_ch(INT32 Channel)
{
    UINT32 reg;

    reg = *(UINT32 *)&m_dma_reg->dma_ch[Channel].csr;
    reg &= ~DMA_CSR_CH_ENABLE;
    *(UINT32 *)&m_dma_reg->dma_ch[Channel].csr = reg;
}

void kdp_dma_enable(unsigned int value)
{
    unsigned int reg;

    value &= 0x1;	// check input

    reg = m_dma_reg->dma_csr;

    reg &= ~(0x1);
    reg |= value;

    m_dma_reg->dma_csr = reg;
}

void kdp_dma_clear_ch_int_status(INT32 Channel)
{
    m_dma_reg->dma_int_tc_clr = 1 << Channel;
    m_dma_reg->dma_int_err_clr = 1 << Channel;
}


void kdp_dma_set_ch_cfg(INT32 Channel, dma_ch_csr Csr)
{
    m_dma_reg->dma_ch[Channel].csr = Csr;
}

dma_ch_csr kdp_dma_get_ch_cfg(INT32 Channel)
{
    return m_dma_reg->dma_ch[Channel].csr;
}


void kdp_dma_set_ch_cn_cfg(INT32 Channel, dma_ch_cfg CnCfg)
{
    m_dma_reg->dma_ch[Channel].cfg = CnCfg;
}

dma_ch_cfg kdp_dma_get_ch_cn_cfg(INT32 Channel)
{
    return m_dma_reg->dma_ch[Channel].cfg;
}

void kdp_dma_ch_int_mask(INT32 Channel, dma_ch_cfg Mask)
{
    m_dma_reg->dma_ch[Channel].cfg = Mask;
}

void kdp_dma_ch_linklist(INT32 Channel, dma_ch_llp LLP)
{
    m_dma_reg->dma_ch[Channel].llp = LLP;
}

void kdp_dma_ch_data_ctrl(INT32 Channel, UINT32 SrcAddr, UINT32 DstAddr, UINT32 Size)
{
    m_dma_reg->dma_ch[Channel].src_addr = SrcAddr/*CPU_TO_AHB_ADDRSPACE(SrcAddr)*/;
    m_dma_reg->dma_ch[Channel].dst_addr = DstAddr/*CPU_TO_AHB_ADDRSPACE(DstAddr)*/;
    m_dma_reg->dma_ch[Channel].size = Size;
}

void kdp_dma_link_transfer(
UINT32 Channel,   // use which channel for AHB DMA, 0..7
UINT32 LinkAddr,  // Link-List address
UINT32 LLPCount,  // total link-list node
UINT32 SrcAddr,   // source begin address
UINT32 DstAddr,   // dest begin address
UINT32 Size,      // total bytes
UINT32 SrcWidth,  // source width 8/16/32 bits -> 0/1/2
UINT32 DstWidth,  // dest width 8/16/32 bits -> 0/1/2
UINT32 SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
UINT32 SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
UINT32 DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
UINT32 Priority,  // priority for this chaanel 0(low)/1/2/3(high)
UINT32 Mode,      // Normal/Hardwire,   0/1
int req)
{
    dma_ch DMAChannel;
    UINT32 LLPSize, i, offset;
    dma_lld *LLP;
    UINT32 Count = 0;
    UINT32 AHB_SrcAddr = /*CPU_TO_AHB_ADDRSPACE*/(SrcAddr);
    UINT32 AHB_DstAddr = /*CPU_TO_AHB_ADDRSPACE*/(DstAddr);
    int burst_size = burst_size_selection[SrcSize];			// ref: [18:16] in Cn_CSR

    LLP = (dma_lld *)LinkAddr;
    *((unsigned int *)&DMAChannel.csr) = 0;		// clear value of csr;

    Size = Size / (1 << SrcWidth);		// how many unit want to transfer

    if(LLPCount && LinkAddr )
    {

    LLPSize = 240*2;
    if (req != 0)		// memory to memory does not have this restriction
    {
        LLPSize = RoundDown(CN_SIZE, burst_size);			// how many cycle a descriptor can transfer
        ASSERT((Size%burst_size)==0);
    }
    Count = LLPCount;		// how many link-list structure need to fill
   // ASSERT(Count<=LLPCount);

    // At last, 2 part

    if (Count > 0)
    {

            offset = 320 << 2;

           for(i = 0; i < Count ;i++)
           {

								if (SrcCtrl == 0)  // increase
									 LLP[i].src_addr = (UINT32)AHB_SrcAddr + ((i+1) * offset);
								else if(SrcCtrl==1) // decrease
									 LLP[i].src_addr = (UINT32)AHB_SrcAddr - ((i+1) * offset);
								else if(SrcCtrl==2)	// fixed
									 LLP[i].src_addr = (UINT32)AHB_SrcAddr;

								if(DstCtrl == 0)
									 LLP[i].dst_addr = (UINT32)AHB_DstAddr + ((i+1) * offset);
								else if(DstCtrl == 1)	// Decrease
									 LLP[i].dst_addr = (UINT32)AHB_DstAddr - ((i+1) * offset);
								else if(DstCtrl == 2)
									 LLP[i].dst_addr = (UINT32)AHB_DstAddr;

								*((UINT32 *)&(LLP[i].llp)) = 0;
								LLP[i].llp.link_list_addr = /*CPU_TO_AHB_ADDRSPACE*/((UINT32)&LLP[i+1]) >> 2;


								*((UINT32 *)&(LLP[i].llp_ctrl)) = 0;
								LLP[i].llp_ctrl.tc_msk = 1;
								LLP[i].llp_ctrl.src_width = SrcWidth; /* source transfer size */
								LLP[i].llp_ctrl.dst_width = DstWidth; /* destination transfer size */
								LLP[i].llp_ctrl.src_ctrl = SrcCtrl; /* source increment, decrement or fix */
								LLP[i].llp_ctrl.dst_ctrl = DstCtrl; /* destination increment, decrement or fix */
								LLP[i].llp_ctrl.src_sel = 0; /* source AHB master id */
								LLP[i].llp_ctrl.dst_sel = 0; /* destination AHB master id */
								LLP[i].llp_ctrl.ff_th = 2; //FIE7021 fifo threshold value = 4
								LLP[i].size = LLPSize;

							Size -= LLPSize;

        }
           LLP[i-1].llp.link_list_addr = 0;
           LLP[i-1].llp_ctrl.tc_msk = 0;	// Enable tc status
           LLP[i-1].size = LLPSize;
           Size = LLPSize;
    }

#ifdef DBG_DMA
    for(i = 0; i < LLPCount;i++)
    {
        kdp_printf("src=%0.8X, dst=%0.8X, link=%0.8X, ctrl=%.8X\n", LLP[i].src_addr, LLP[i].dst_addr,
            *(UINT32 *)&(LLP[i].llp), *(UINT32 *)&(LLP[i].llp_ctrl));

    }
#endif
}
    /* program channel */
    kdp_dma_clear_ch_int_status(Channel);

    /* program channel CSR */
       DMAChannel.csr.ff_th = 2; //FIE7021 fifo threshold value = 4
       DMAChannel.csr.priority = Priority; /* priority */
       DMAChannel.csr.prot = 0; /* PROT 1-3 bits */
       DMAChannel.csr.src_size = SrcSize; /* source burst size */
    //FIE7021 this bit should be 0 when change other bits
       DMAChannel.csr.abt = 0; /* NOT transaction abort */
       DMAChannel.csr.src_width = SrcWidth; /* source transfer size */
       DMAChannel.csr.dst_width = DstWidth; /* destination transfer size */
       DMAChannel.csr.mode = Mode; /* Normal mode or Hardware handshake mode */
       DMAChannel.csr.src_ctrl = SrcCtrl; /* source increment, decrement or fix */
       DMAChannel.csr.dst_ctrl = DstCtrl; /* destination increment, decrement or fix */
       DMAChannel.csr.src_sel = 0; /* source AHB master id */
       DMAChannel.csr.dst_sel = 0; /* destination AHB master id */

    DMAChannel.csr.reserved1 = 0;
    DMAChannel.csr.reserved0 = 0;

    /* program channel CFG */
    DMAChannel.cfg.int_tc_msk = 0;				// Enable tc status
    DMAChannel.cfg.int_err_msk = 0;
    DMAChannel.cfg.src_rs = req;
    DMAChannel.cfg.dst_rs = req;
    DMAChannel.cfg.src_he = (SrcCtrl == 2);		// SrcCtrl==2 means fix source address ==> peripheral
    DMAChannel.cfg.dst_he = (DstCtrl == 2);
    DMAChannel.cfg.busy = 0;
    DMAChannel.cfg.reserved1 = 0;
    DMAChannel.cfg.llp_cnt = 0;
    DMAChannel.cfg.reserved2 = 0;

   /* program channel llp */
   *((UINT32 *)&(DMAChannel.llp)) = 0;

    if (Count > 0)
    {
        DMAChannel.csr.tc_msk = 1; /* enable terminal count */
        DMAChannel.llp.link_list_addr = /*CPU_TO_AHB_ADDRSPACE*/((UINT32)&LLP[0]) >> 2;
    }
    else
    {
        DMAChannel.csr.tc_msk = 0; /* no LLP */
    }

    kdp_dma_set_ch_cfg(Channel, DMAChannel.csr);
    kdp_dma_ch_int_mask(Channel, DMAChannel.cfg);
    kdp_dma_ch_linklist(Channel, DMAChannel.llp);

       /* porgram address and size */
       kdp_dma_ch_data_ctrl(Channel, SrcAddr, DstAddr, 240*2);
}





void kdp_dma_ch_datactrl(s32 Channel, u32 SrcAddr, u32 DstAddr, u32 Size)
{
    m_dma_reg->dma_ch[Channel].src_addr = SrcAddr/*CPU_TO_AHB_ADDRSPACE(SrcAddr)*/;
    m_dma_reg->dma_ch[Channel].dst_addr = DstAddr/*CPU_TO_AHB_ADDRSPACE(DstAddr)*/;
    m_dma_reg->dma_ch[Channel].size = Size;
}


void kdp_flash_to_ddr_dma_copy(UINT32 *src_addr, UINT32 *dst_addr, UINT32 size)
{
    uint32_t dat;
    dma_ch DMAChannel;
    UINT8 ch = AHBDMA_Channel0;

    memset(&DMAChannel, 0x0, sizeof(dma_ch));
    dat=inw(SPI020REG_INTERRUPT);
    outw(SPI020REG_INTERRUPT, (dat | SPI020_DMA_EN));/* enable DMA function */
    kdp_dma_init(0,0,0);//fLib_InitDMA(FALSE, FALSE, 0x0);

    kdp_dma_reset_ch(ch);// fLib_DMA_ResetChannel(0);
    kdp_dma_enable_dma_int();// fLib_DMA_EnableDMAInt(); //Enable DMA Interrupt

    DMAChannel.csr.dst_ctrl = AHBDMA_DstInc;
    DMAChannel.csr.src_ctrl = AHBDMA_SrcFix;
    DMAChannel.csr.mode = AHBDMA_HwHandShakeMode;
    DMAChannel.csr.dst_width = AHBDMA_DstWidth_DWord;
    DMAChannel.csr.src_width = AHBDMA_SrcWidth_DWord;
    DMAChannel.csr.priority = 3;
    DMAChannel.csr.src_size = 2;

    DMAChannel.cfg.dst_rs = 0;
    DMAChannel.cfg.dst_he = 0;
    DMAChannel.cfg.src_rs = 11; //SPI_DMA_REQ
    DMAChannel.cfg.src_he = 1;
    DMAChannel.cfg.int_abt_msk = 0;
    DMAChannel.cfg.int_err_msk = 0;
    DMAChannel.cfg.int_tc_msk = 0;

    kdp_dma_set_ch_cfg(ch, DMAChannel.csr); //    fLib_SetDMAChannelCfg(0, csr);
    kdp_dma_set_ch_cn_cfg(ch, DMAChannel.cfg);// fLib_SetDMAChannelCnCfg(0, cfg);
    /* porgram address and size */
    kdp_dma_ch_data_ctrl(ch, SPI020REG_DATAPORT, (UINT32)dst_addr, size>>2);
    kdp_dma_enable_ch(ch);// fLib_EnableDMAChannel(0);


    kdp_dma_wait_dma_int(ch);// fLib_DMA_WaitDMAInt(0);

    kdp_dma_disable_ch(ch);// fLib_DisableDMAChannel(0);
    kdp_flash_dma_read_stop();// spi020_dma_read_stop();




//    kdp_dma_disable_ch(ch);  //    fLib_DMA_DisableDMAInt(); //Disable DMA Interrupt
//    //fLib_printf("====== finish dma_flash_to_ddr_test ========\n");


}

extern void norflash_write_running( UINT8 type, UINT32 offset, UINT32 total_send_byte, UINT8 *buf );
void kdp_ddr_to_flash_dma_copy(UINT32 src_addr, UINT32 dst_addr, UINT32 size){
    
    //UINT32 i;

    UINT32 write_len=size;
    UINT8 ch = AHBDMA_Channel1;
    
    UINT32 access_byte;
    UINT32 offset=0;


    dma_ch DMAChannel;

    memset(&DMAChannel, 0x0, sizeof(dma_ch));
    //memset(&csr, 0x0, sizeof(fLib_DMA_CH_CSR_t));
    //memset(&cfg, 0x0, sizeof(fLib_DMA_CH_CFG_t));

    //fLib_printf("before spi020_flash_64kErase\n");
    //spi020_flash_64kErase((UINT32)dst_addr+offset);
    //fLib_printf("after spi020_flash_64kErase\n");
	//fLib_InitDMA(FALSE, FALSE, 0x0);				
    //fLib_DMA_ClearAllInterrupt();
	//fLib_DMA_EnableDMAInt(); //Enable DMA Interrupt		
	//fLib_DMA_ResetChannel(0);	

    kdp_dma_init(0,0,0);//fLib_InitDMA(FALSE, FALSE, 0x0);

    kdp_dma_reset_ch(ch);// fLib_DMA_ResetChannel(0);
    kdp_dma_enable_dma_int();// fLib_DMA_EnableDMAInt(); //Enable DMA Interrupt

    DMAChannel.csr.dst_ctrl = AHBDMA_DstFix;
    DMAChannel.csr.src_ctrl = AHBDMA_SrcInc;
    DMAChannel.csr.mode = AHBDMA_HwHandShakeMode;
    DMAChannel.csr.dst_width = AHBDMA_DstWidth_DWord;
    DMAChannel.csr.src_width = AHBDMA_SrcWidth_DWord;

    DMAChannel.cfg.dst_rs = SPI_DMA_REQ;
    DMAChannel.cfg.dst_he = 1;
    DMAChannel.cfg.src_rs = 0;
    DMAChannel.cfg.src_he = 0;
    DMAChannel.cfg.int_abt_msk = 0;	
    DMAChannel.cfg.int_err_msk = 0;
    DMAChannel.cfg.int_tc_msk = 0;

    kdp_dma_set_ch_cfg(ch, DMAChannel.csr); //    fLib_SetDMAChannelCfg(0, csr);
    kdp_dma_set_ch_cn_cfg(ch, DMAChannel.cfg);// fLib_SetDMAChannelCnCfg(0, cfg);

    while(write_len>0)
    {
       
        access_byte = MIN(write_len, kdp520_spi_txfifo_depth());     //access_byte = min_t(write_len, spi020_txfifo_depth());
        
        norflash_write_running(FLASH_DMA_WRITE, (UINT32)dst_addr+offset, access_byte, NULL);    //spi020_flash_write(FLASH_DMA_WRITE, (UINT32)dst_addr+offset, access_byte, NULL);

        kdp_dma_ch_data_ctrl(ch, ((UINT32)src_addr)+offset, (UINT32)SPI020REG_DATAPORT, size>>2);           //fLib_DMA_CHDataCtrl(0, ((UINT32)src_addr)+offset, SPI020REG_DATAPORT, access_byte>>2);		
        
        kdp_dma_enable_ch(ch);// fLib_EnableDMAChannel(0);
        
        //kdp_dma_wait_dma_int(ch);       //fLib_DMA_WaitDMAInt(0);		
        kdp_flash_dma_write_stop();     //spi020_dma_write_stop();
        kdp_dma_disable_ch(ch);         //fLib_DisableDMAChannel(0);  

        //dbg_msg_console("access_byte=%d", access_byte);
        write_len-=access_byte;
        offset+=access_byte;
    }

    kdp_dma_disable_ch(ch);     //fLib_DMA_DisableDMAInt(); //Disable DMA Interrupt
}


typedef struct
{
    u32 src_addr;
    u32 dst_addr;
    u32 llp;
    u32 control;
    u32 total_size;
} _link_list_descriptor_t;

void kdp_dma_ch_llp_data_ctrl( INT32 Channel, UINT32 SrcAddr, UINT32 DstAddr, UINT32 Size, UINT32 llp_buf )
{
    m_dma_reg->dma_ch[Channel].src_addr = SrcAddr/*CPU_TO_AHB_ADDRSPACE(SrcAddr)*/;
    m_dma_reg->dma_ch[Channel].dst_addr = DstAddr/*CPU_TO_AHB_ADDRSPACE(DstAddr)*/;
    m_dma_reg->dma_ch[Channel].size = Size;
    m_dma_reg->dma_ch[Channel].llp.link_list_addr = (llp_buf >> 2);
    m_dma_reg->dma_ch[Channel].csr.tc_msk = 1;
}


void kdp_dma_mem_link_list_init( u32 llp_buf, dma_ch_csr ch_crs, u32 count )
{

    u32 control = (ch_crs.ff_th << 29) |
                  (0x1 << 28) |
                  (ch_crs.src_width << 25) |
                  (ch_crs.dst_width << 22) |
                  (ch_crs.src_ctrl << 20) |
                  (ch_crs.dst_ctrl << 18);

    u32 src_width_shift = ch_crs.src_width;

    _link_list_descriptor_t *llp_array = (_link_list_descriptor_t *)llp_buf;

    for (u32 i = 0; i < count; i++)
    {
        //dbg_msg_console( "tx_buf[%d]= %#x, rx_buf[%d]= %#x, size[%d]= %d", i, llp_array[i].src_addr, i, llp_array[i].dst_addr, i , llp_array[i].total_size );
        llp_array[i].llp = (u32)(&(llp_array[i + 1]));
        llp_array[i].control = control;
        llp_array[i].total_size = llp_array[i].total_size >> src_width_shift;// (u32) (size[i] >> src_width_shift);
    }
    llp_array[count - 1].llp = 0;
    llp_array[count - 1].control &= ~(0x1 << 28);

}

u8 kdp_dma_memcpy( u32 count )
{
    u8 ret = 0;
    dma_ch DMAChannel;
    UINT8 ch = AHBDMA_Channel1;

    kdp_dma_init(0,0,0);//fLib_InitDMA(FALSE, FALSE, 0x0);

    while (kdp_dma_is_ch_busy(ch));// { osDelay(1); } ;

    kdp_dma_reset_ch(ch);// fLib_DMA_ResetChannel(0);
    kdp_dma_clear_interrupt(ch);
    kdp_dma_enable_dma_int();// fLib_DMA_EnableDMAInt(); //Enable DMA Interrupt

    // tx channel setting
    memset(&DMAChannel, 0x0, sizeof(dma_ch));
    DMAChannel.csr.dst_ctrl = AHBDMA_DstInc;
    DMAChannel.csr.src_ctrl = AHBDMA_SrcInc;
    DMAChannel.csr.mode = AHBDMA_NormalMode;
    DMAChannel.csr.dst_width = AHBDMA_DstWidth_Byte;
    DMAChannel.csr.src_width = AHBDMA_DstWidth_Byte;
    DMAChannel.csr.priority = 3;
    DMAChannel.csr.src_size = 0;

    DMAChannel.cfg.dst_rs = 0; //SSP_u1_TX_DMA_REQ
    DMAChannel.cfg.dst_he = 0;
    DMAChannel.cfg.src_rs = 0;
    DMAChannel.cfg.src_he = 0;
    DMAChannel.cfg.int_abt_msk = 0;
    DMAChannel.cfg.int_err_msk = 0;
    DMAChannel.cfg.int_tc_msk = 0;

    //dbg_msg_console(" src addr = %#X, dst addr = %#X" , *tx_buf, *rx_buf, *size );

    //_link_list_descriptor_t llp_buf[count];
    kdp_dma_mem_link_list_init( llp_buf, DMAChannel.csr, count );

    kdp_dma_set_ch_cfg(ch, DMAChannel.csr); //    fLib_SetDMAChannelCfg(0, csr);
    kdp_dma_set_ch_cn_cfg(ch, DMAChannel.cfg);// fLib_SetDMAChannelCnCfg(0, cfg);
    /* porgram address and size */
    kdp_dma_ch_llp_data_ctrl(ch, *((u32*)(llp_buf)) , *((u32*)(llp_buf+4)), *((u32*)(llp_buf+16)), llp_buf );


    kdp_dma_enable_ch(ch);// fLib_EnableDMAChannel(0);
    ret |= kdp_dma_wait_dma_int(ch);// fLib_DMA_WaitDMAInt(0);

    kdp_dma_disable_ch(ch);// fLib_DisableDMAChannel(0);

   // kdp_dma_disable_dma_int();// fLib_DMA_EnableDMAInt(); //Enable DMA Interrupt

    return ret;

}

u8 kdp_api_2D_memcpy( dma_2d_memcpy_t info )
{
    u16 i;
    u8 ret;

    {
        u16 src_raws_lenth = info.src.data_type*info.src.w;
        u16 dst_raws_lenth = info.dst.data_type*info.dst.w;

        u16 src_w_offset = info.src.start_x*info.src.data_type + info.src.start_y*src_raws_lenth;
        u16 dst_w_offset = info.dst.start_x*info.src.data_type + info.dst.start_y*src_raws_lenth;

        u32 data_raws_lens = info.data_w*info.dst.data_type; // if dst.data_type != src.data_type, first choose dst.data_type

        _link_list_descriptor_t *llp_array = (_link_list_descriptor_t *)llp_buf;

        for( i=0; i<info.data_h; i++ )
        {
            llp_array[i].src_addr = ( info.src.addr + src_w_offset + (src_raws_lenth*i) );
            llp_array[i].dst_addr = ( info.dst.addr + dst_w_offset + (dst_raws_lenth*i) );
            llp_array[i].total_size = ( data_raws_lens );
        }
    }

    ret = kdp_dma_memcpy( info.data_h );

    return ret;

}


void kdp_dma_linkmode(
UINT32 Channel,   // use which channel for AHB DMA, 0..7
UINT32 LinkAddr,  // Link-List address
UINT32 LLPCount,  // total link-list node
UINT32 SrcAddr,   // source begin address
UINT32 DstAddr,   // dest begin address
UINT32 Size,      // total bytes
UINT32 SrcWidth,  // source width 8/16/32 bits -> 0/1/2
UINT32 DstWidth,  // dest width 8/16/32 bits -> 0/1/2
UINT32 SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
UINT32 SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
UINT32 DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
UINT32 Priority,  // priority for this chaanel 0(low)/1/2/3(high)
UINT32 Mode,      // Normal/Hardwire,   0/1
int    req
)
{
    dma_ch DMAChannel;
    UINT32 LLPSize, i, offset;
    dma_lld *LLP;
    UINT32 Count = 0;
    UINT32 AHB_SrcAddr = /*CPU_TO_AHB_ADDRSPACE*/(SrcAddr);
    UINT32 AHB_DstAddr = /*CPU_TO_AHB_ADDRSPACE*/(DstAddr);
    int burst_size = burst_size_selection[SrcSize];			// ref: [18:16] in Cn_CSR

    LLP = (dma_lld *)LinkAddr;
    *((unsigned int *)&DMAChannel.csr) = 0;		// clear value of csr;

#ifdef DBG_DMA
    kdp_printf("Ch%d, Src=%08X, Dst=%08X, Size=%08X SrcWidth=%db, DstWidth=%db, SrcSize=%dB\n"
           "SrcCtrl=%s, DstCtrl=%s, Priority=%d, Mode=%s, LLPCnt = %d\n",
           Channel, SrcAddr, DstAddr, Size, 1 << (SrcWidth + 3), 1 << (DstWidth + 3),
           ((SrcSize == 0) ? 1 : 1 << (SrcSize+1)),
           ((SrcCtrl == 0) ? "Inc" : ((SrcCtrl == 1) ? "Dec" : "Fix")),
           ((DstCtrl == 0) ? "Inc" : ((DstCtrl == 1) ? "Dec" : "Fix")),
           Priority, ((Mode == 0) ? "Normal" : "HW"), LLPCount);
#endif

    Size = Size / (1 << SrcWidth);		// how many unit want to transfer

    if(LLPCount && LinkAddr)
    {
    //LLPSize = CN_SIZE;
    if (req != 0)		// memory to memory does not have this restriction
    {
        LLPSize = RoundDown(CN_SIZE, burst_size);			// how many cycle a descriptor can transfer
        ASSERT((Size%burst_size)==0);
    }
    Count = divRoundDown(Size, LLPSize);		// how many link-list structure need to fill
    ASSERT(Count<=LLPCount);

    // At last, 2 part
    if (Count > 0)
    {
            offset = LLPSize << SrcWidth;
           for(i = 0; i < Count ;i++)
           {
              if (SrcCtrl == 0)  // increase
                 LLP[i].src_addr = (UINT32)AHB_SrcAddr + ((i+1) * offset);
              else if(SrcCtrl==1) // decrease
                 LLP[i].src_addr = (UINT32)AHB_SrcAddr - ((i+1) * offset);
              else if(SrcCtrl==2)	// fixed
                 LLP[i].src_addr = (UINT32)AHB_SrcAddr;

              if(DstCtrl == 0)
                 LLP[i].dst_addr = (UINT32)AHB_DstAddr + ((i+1) * offset);
              else if(DstCtrl == 1)	// Decrease
                 LLP[i].dst_addr = (UINT32)AHB_DstAddr - ((i+1) * offset);
              else if(DstCtrl == 2)
                 LLP[i].dst_addr = (UINT32)AHB_DstAddr;

              *((UINT32 *)&(LLP[i].llp)) = 0;
              LLP[i].llp.link_list_addr = /*CPU_TO_AHB_ADDRSPACE*/((UINT32)&LLP[i+1]) >> 2;

              *((UINT32 *)&(LLP[i].llp_ctrl)) = 0;
              LLP[i].llp_ctrl.tc_msk = 1;
              LLP[i].llp_ctrl.src_width = SrcWidth; /* source transfer size */
              LLP[i].llp_ctrl.dst_width = DstWidth; /* destination transfer size */
              LLP[i].llp_ctrl.src_ctrl = SrcCtrl; /* source increment, decrement or fix */
              LLP[i].llp_ctrl.dst_ctrl = DstCtrl; /* destination increment, decrement or fix */
              LLP[i].llp_ctrl.src_sel = 0; /* source AHB master id */
              LLP[i].llp_ctrl.dst_sel = 0; /* destination AHB master id */
              LLP[i].llp_ctrl.ff_th = 2; //FIE7021 fifo threshold value = 4
            LLP[i].size = LLPSize;
            Size -= LLPSize;
        }
           LLP[i-1].llp.link_list_addr = 0;
           LLP[i-1].llp_ctrl.tc_msk = 0;	// Enable tc status
           LLP[i-1].size = Size;
           Size = LLPSize;
    }

#ifdef DBG_DMA
    for(i = 0; i < LLPCount;i++)
    {
        kdp_printf("src=%0.8X, dst=%0.8X, link=%0.8X, ctrl=%.8X\n", LLP[i].src_addr, LLP[i].dst_addr,
            *(UINT32 *)&(LLP[i].llp), *(UINT32 *)&(LLP[i].llp_ctrl));

    }
#endif
}
    /* program channel */
    kdp_dma_clear_ch_int_status(Channel);

    /* program channel CSR */
       DMAChannel.csr.ff_th = 2; //FIE7021 fifo threshold value = 4
       DMAChannel.csr.priority = Priority; /* priority */
       DMAChannel.csr.prot = 0; /* PROT 1-3 bits */
       DMAChannel.csr.src_size = SrcSize; /* source burst size */
    //FIE7021 this bit should be 0 when change other bits
       DMAChannel.csr.abt = 0; /* NOT transaction abort */
       DMAChannel.csr.src_width = SrcWidth; /* source transfer size */
       DMAChannel.csr.dst_width = DstWidth; /* destination transfer size */
       DMAChannel.csr.mode = Mode; /* Normal mode or Hardware handshake mode */
       DMAChannel.csr.src_ctrl = SrcCtrl; /* source increment, decrement or fix */
       DMAChannel.csr.dst_ctrl = DstCtrl; /* destination increment, decrement or fix */
       DMAChannel.csr.src_sel = 0; /* source AHB master id */
       DMAChannel.csr.dst_sel = 0; /* destination AHB master id */

    DMAChannel.csr.reserved1 = 0;
    DMAChannel.csr.reserved0 = 0;

    /* program channel CFG */
    DMAChannel.cfg.int_tc_msk = 0;				// Enable tc status
    DMAChannel.cfg.int_err_msk = 0;
    DMAChannel.cfg.src_rs = req;
    DMAChannel.cfg.dst_rs = req;
    DMAChannel.cfg.src_he = (SrcCtrl == 2);		// SrcCtrl==2 means fix source address ==> peripheral
    DMAChannel.cfg.dst_he = (DstCtrl == 2);
    DMAChannel.cfg.busy = 0;
    DMAChannel.cfg.reserved1 = 0;
    DMAChannel.cfg.llp_cnt = 0;
    DMAChannel.cfg.reserved2 = 0;

   /* program channel llp */
   *((UINT32 *)&(DMAChannel.llp)) = 0;

    if (Count > 0)
    {
        DMAChannel.csr.tc_msk = 1; /* enable terminal count */
        DMAChannel.llp.link_list_addr = /*CPU_TO_AHB_ADDRSPACE*/((UINT32)&LLP[0]) >> 2;
    }
    else
    {
        DMAChannel.csr.tc_msk = 0; /* no LLP */
    }

    kdp_dma_set_ch_cfg(Channel, DMAChannel.csr);
    kdp_dma_ch_int_mask(Channel, DMAChannel.cfg);
    kdp_dma_ch_linklist(Channel, DMAChannel.llp);

       /* porgram address and size */
       kdp_dma_ch_data_ctrl(Channel, SrcAddr, DstAddr, Size);
}


void kdp_dma_normal_mode(
UINT32 Channel,   // use which channel for AHB DMA, 0..7
UINT32 SrcAddr,   // source begin address
UINT32 DstAddr,   // dest begin address
UINT32 Size,      // total bytes
UINT32 SrcWidth,  // source width 8/16/32 bits -> 0/1/2
UINT32 DstWidth,  // dest width 8/16/32 bits -> 0/1/2
UINT32 SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
UINT32 SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
UINT32 DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
UINT32 Priority,  // priority for this chaanel 0(low)/1/2/3(high)
UINT32 Mode,      // Normal/Hardwire,   0/1
int    req
)
{
    kdp_dma_linkmode(
            Channel,   // use which channel for AHB DMA, 0..7
            NULL,
            0,  // total link-list node
            SrcAddr,   // source begin address
            DstAddr,   // dest begin address
            Size,      // total bytes
            SrcWidth,  // source width 8/16/32 bits -> 0/1/2
            DstWidth,  // dest width 8/16/32 bits -> 0/1/2
            SrcSize,   // source burst size, How many "SrcWidth" will be transmmited at one times ?
            SrcCtrl,   // source address change : Inc/dec/fixed --> 0/1/2
            DstCtrl,   // dest address change : Inc/dec/fixed --> 0/1/2
            Priority,  // priority for this chaanel 0(low)/1/2/3(high)
            Mode,      // Normal/Hardwire,   0/1
            req);
}


void kdp_dma_set_interrupt(UINT32 channel, UINT32 tcintr, UINT32 errintr, UINT32 abtintr)
{
    dma_ch_cfg cfg;
//	int i;

    cfg =  kdp_dma_get_ch_cn_cfg(channel); //ycmo091007 add

    if(tcintr)
        cfg.int_tc_msk = 0;	// Enable terminal count interrupt
    else
        cfg.int_tc_msk = 1;	// Disable terminal count interrupt

    if(errintr)
        cfg.int_err_msk = 0;	// Enable error interrupt
    else
        cfg.int_err_msk = 1;	// Disable error interrupt

    if(abtintr)
        cfg.int_abt_msk = 0;	// Enable abort interrupt
    else
        cfg.int_abt_msk = 1;	// Disable abort interrupt

    kdp_dma_ch_int_mask(channel, cfg);
}

void kdp_dma_reset_ch(UINT8 channel)
{
    UINT32 base = DMAC_FTDMAC020_PA_BASE_SCPU+DMA_CHANNEL0_BASE+channel*DMA_CHANNEL_OFFSET;

    outw(base+DMA_CHANNEL_CSR_OFFSET,0);
    outw(base+DMA_CHANNEL_CFG_OFFSET,7);
    outw(base+DMA_CHANNEL_SRCADDR_OFFSET,0);
    outw(base+DMA_CHANNEL_DSTADDR_OFFSET,0);
    outw(base+DMA_CHANNEL_LLP_OFFSET,0);
    outw(base+DMA_CHANNEL_SIZE_OFFSET,0);
}

void kdp_dma_clear_interrupt(UINT8 channel)
{
    outw(DMAC_FTDMAC020_PA_BASE_SCPU+DMA_INT_TC_CLR, (1 << channel));
    outw(DMAC_FTDMAC020_PA_BASE_SCPU+DMA_INT_ERRABT_CLR, (0x00010001 << channel));
}

void kdp_dma_clear_all_interrupt()
{
    // Clear all interrupt source
    outw(DMAC_FTDMAC020_PA_BASE_SCPU+DMA_INT_TC_CLR,0xFF);
    outw(DMAC_FTDMAC020_PA_BASE_SCPU+DMA_INT_ERRABT_CLR,0xFF00FF);
}

void kdp_dma_wait_int_status(UINT32 Channel)
{
    UINT32 choffset;
    volatile UINT32 status;

    choffset = 1 << Channel;

    while((inw(DMAC_FTDMAC020_PA_BASE_SCPU+DMA_TC)&choffset)==0)
        ;


    kdp_dma_disable_ch(Channel);
}

int kdp_dma_get_ch_num()
{
    //kdp_printf("channel_num reg: %d\n",readl(DMAC_FTDMAC020_PA_BASE_SCPU+DMA_FEATURE));
    return (readl(DMAC_FTDMAC020_PA_BASE_SCPU+DMA_FEATURE)>>12)&0xf;
}

void kdp_dma_enable_dma_int(void)
{
    if(0 == dma_int_initized)
    {
        __dma_int_occurred = 0;
        NVIC_SetVector((IRQn_Type)DMA_FTDMAC020_0_IRQ, (uint32_t)AHB_DMA_IRQHandler);
        NVIC_EnableIRQ(DMA_FTDMAC020_0_IRQ);
        dma_int_initized = 1;
    }
}

void kdp_dma_disable_dma_int(void)
{
    if(dma_int_initized)
    {
        NVIC_DisableIRQ(DMA_FTDMAC020_0_IRQ);
        dma_int_initized = 0;
    }
}

void kdp_dma_enable_dma_tc_int(void)
{
    if(0 == dma_tc_int_initized)
    {
        __dma_tc_int_occurred = 0;
        NVIC_SetVector((IRQn_Type)DMA_FTDMAC020_0_TC_IRQ, (uint32_t)AHB_DMA_TC_IRQHandler);
        NVIC_EnableIRQ(DMA_FTDMAC020_0_TC_IRQ);
        dma_tc_int_initized = 1;
    }
}

void kdp_dma_disable_dma_tc_int(void)
{
    if(dma_tc_int_initized)
    {
        NVIC_DisableIRQ(DMA_FTDMAC020_0_TC_IRQ);
        dma_tc_int_initized = 0;
    }
}

u8 kdp_dma_wait_dma_int(UINT32 channel)
{
    u32 cnt = 0;
    u8 ret = 1;
    while(!(__dma_int_occurred & (1 << channel)))
    {
       //kdp_printf("kdp_dma_wait_dma_int: __dma_int_occurred %d\n", __dma_int_occurred);
        //__WFE (); 								   // Power-Down until next Event/Interrupt
        delay_us(1);
        if (DMA_WAIT_INT_TIMEOUT_CNT < cnt)
        {
            err_msg("kdp_dma_wait_dma_int time :%d ch :%d", cnt, channel);
            ret = 0;
            break;
        }
        cnt++;
    }
    __dma_int_occurred &= ~(1 << channel);

    return ret;
}

void kdp_dma_wait_dma_tc_int(UINT32 channel)
{
    while(!(__dma_tc_int_occurred & (1 << channel)))
    {
      __WFE (); 								   // Power-Down until next Event/Interrupt
    }

    __dma_tc_int_occurred &= ~(1 << channel);
}

#if (CFG_E2E_NIR_TWO_STAGE_LIGHT == YES)
osMutexId_t mutex_img_reserv_dm = NULL;
void kl520_api_img_reserv_dma(u32 writeAddr, u32 readAddr, u32 len)
{
    if(mutex_img_reserv_dm == NULL)
        mutex_img_reserv_dm = osMutexNew(NULL);

    osMutexAcquire(mutex_img_reserv_dm, osWaitForever);

    while (kdp_dma_is_ch_busy(AHBDMA_Channel2));

    kdp_dma_reset_ch(AHBDMA_Channel2);
    kdp_dma_clear_interrupt(AHBDMA_Channel2);
    kdp_dma_enable_dma_int();
    kdp_dma_init(0,0,0);

    while (kdp_dma_is_ch_busy(AHBDMA_Channel2));
    kdp_dma_normal_mode(AHBDMA_Channel2,(UINT32)(readAddr),(UINT32)(writeAddr), len ,1,1,2,0,0,2,0,0); //init DMA and wait TE signal
    kdp_dma_enable_ch(AHBDMA_Channel2);
    kdp_dma_wait_dma_int(AHBDMA_Channel2);
    kdp_dma_disable_ch(AHBDMA_Channel2);
    osMutexRelease(mutex_img_reserv_dm);
}
#endif
