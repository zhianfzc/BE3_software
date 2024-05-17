


#include "flash.h"
#if (FLASH_VENDOR_SELECT == GD25S512MD )
#include "GD25S512MD.h"
#include "types.h"
#include "base.h"
#include "delay.h"
#include "kdp520_flash.h"
#include "kdp520_spi.h"
#include "kl520_include.h"
#include "dbg.h"


extern FLASH_PARMATER_T    st_flash_info;
#define _get_min(x,y) ( x < y ? x: y )
#define DMA_LIMIT 16777212 //3bytes to describe read size (16MB - 3 :4 aligned address)

UINT8 protect_bypass = 0;



void nor_flash_die_selection(UINT8 nidx)
{
    kdp520_spi_set_commands( nidx , DIE_SELEC_C2_CMD1, DIE_SELEC_C2_CMD2, DIE_SELEC_C2_CMD3 );
    kdp520_spi_wait_command_complete();
    while ( (norflash_busy_check()&0x01)  != 0 );
}

UINT8 nor_flash_get_active_die(void)
{
    UINT32   r_buffer_index=0;
    UINT32   r_buffer[1];
//    kdp520_spi_switch_org();
    kdp520_spi_set_commands( RDIE_F8_CMD0, RDIE_F8_CMD1, RDIE_F8_CMD2, RDIE_F8_CMD3 );
    r_buffer_index = 0;
    kdp520_spi_read_Rx_FIFO( r_buffer, &r_buffer_index, 1 );
    kdp520_spi_wait_command_complete();
//    kdp520_spi_switch_fail();
    return r_buffer[0];
}


u32 nor_area_judge_offset(u32 add)
{
    if( add>= 0x02000000 ){
        nor_flash_die_selection(1);
    }
    else{
        nor_flash_die_selection(0);
    }
    #if(GD25D512MD_LOG_EN == YES)
    dbg_msg_flash("active die is %d", nor_flash_get_active_die() );
    #endif
    return nor_flash_get_active_die()*0x02000000;
}

UINT8 norflash_busy_check(void)
{
    UINT32   r_buffer_index=0;
    UINT32   r_buffer[5];
    kdp520_spi_switch_fail();
    kdp520_spi_set_commands( STATUS_05_CMD0, STATUS_05_CMD1, STATUS_05_CMD2, STATUS_05_CMD3 );
    r_buffer_index = 0;
    kdp520_spi_read_Rx_FIFO( r_buffer, &r_buffer_index, 1 );
    kdp520_spi_wait_command_complete();
    kdp520_spi_switch_org();
    return r_buffer[0];
}

void norflash_4Bytes_ctrl(UINT8 enable)
{
    if (enable) {
        kdp520_spi_set_commands(ENTER_4B_ADD_B7_CMD0, ENTER_4B_ADD_B7_CMD1, ENTER_4B_ADD_B7_CMD2, ENTER_4B_ADD_B7_CMD3);
    } else {
        kdp520_spi_set_commands(EXIT_4B_ADD_E9_CMD0, EXIT_4B_ADD_E9_CMD1, EXIT_4B_ADD_E9_CMD2, EXIT_4B_ADD_E9_CMD3);
    }
    kdp520_spi_wait_command_complete();
}

void norflash_write_control(UINT8 enable)
{
    /* fill in command 0~3 */
    if (enable) {
        kdp520_spi_set_commands(WRITE_CON_06_CMD0, WRITE_CON_06_CMD1, WRITE_CON_06_CMD2, WRITE_CON_06_CMD3);
    } else {
        kdp520_spi_set_commands(WRITE_CON_04_CMD0, WRITE_CON_04_CMD1, WRITE_CON_04_CMD2, WRITE_CON_04_CMD3);
    }
    /* wait for command complete */
    kdp520_spi_wait_command_complete();
}


UINT8 norflash_quad_mode_read(void)
{
#if 0
    UINT8   tmpbuf;
    /* fill in command 0~3 */
    //kdp520_spi_set_commands(SPI020_05_CMD0_ORG, SPI020_05_CMD1_ORG, SPI020_05_CMD2_ORG, SPI020_05_CMD3_ORG&0xFFFFFFFE);
    kdp520_spi_set_commands(SPI020_35_CMD0, SPI020_35_CMD1, SPI020_35_CMD2, SPI020_35_CMD3);//bessel:wait interrupt instead of delay
    //main_delay_count(1000);
    kdp520_spi_wait_command_complete();
    tmpbuf = inl((INT8U * )SPI020REG_READ_ST);
    return tmpbuf;
#else
    UINT32  nrx_buff_word_index = 0;
    UINT32  nrx_buff_word[ 10 ];
    UINT8   tmpbuf;
    kdp520_spi_set_commands(STATUS_35_CMD0, STATUS_35_CMD1, STATUS_35_CMD2, STATUS_35_CMD3);
    kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
    kdp520_spi_wait_command_complete();
    tmpbuf = nrx_buff_word[0];

    return tmpbuf;
#endif
}

void nor_flash_quad_mode_en(UINT8 enable)
{
    UINT8 status2 = 0;
    UINT8 flag;
    status2 = norflash_quad_mode_read();

    flag = status2 & 0x02;
    if((enable && flag) || ((!enable) && (!flag)))
    {
        return;
    }
    status2 &= 0x40;
    if (enable)
    {
        status2 |= 0x02;
    }

    norflash_write_control(1);
    kdp520_spi_set_commands( WRITE_STAT_31_CMD0, WRITE_STAT_31_CMD1, WRITE_STAT_31_CMD2, WRITE_STAT_31_CMD3 );
    kdp520_spi_write_data(&status2, 1);
    kdp520_flash_check_status_til_ready();
    norflash_write_control(0);
}



kdp_status_t norflash_get_info(void)
{
    #define SPI_Rx_SIZE     (5)
    UINT32  nrx_buff_word_index = 0;
    UINT32  nrx_buff_word[ SPI_Rx_SIZE ];
    UINT32  ntemp =0;

    //do
    if( st_flash_info.page_size_Bytes == 256 && st_flash_info.block_size_Bytes != 0 )
    {
        return  KDP_STATUS_OK;
    }
    kdp520_spi_switch_fail();

    norflash_write_control(1);

    nrx_buff_word_index =0;
    kdp520_spi_set_commands(STATUS_15_CMD0, STATUS_15_CMD1, STATUS_15_CMD2, STATUS_15_CMD3);
    kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
    kdp520_spi_wait_command_complete();

    norflash_write_control(1);

    //Driving
    kdp520_spi_set_commands( 0x00 , WRITE_STAT_11_CMD1, WRITE_STAT_11_CMD2, WRITE_STAT_11_CMD3 );
    kdp520_spi_wait_command_complete();
    delay_ms(20);   //delay_us(300);

    nrx_buff_word_index =0;
    kdp520_spi_set_commands(STATUS_15_CMD0, STATUS_15_CMD1, STATUS_15_CMD2, STATUS_15_CMD3);
    kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
    kdp520_spi_wait_command_complete();

    //read quad bit
    nrx_buff_word_index =0;
    kdp520_spi_set_commands(STATUS_35_CMD0, STATUS_35_CMD1, STATUS_35_CMD2, STATUS_35_CMD3);//bessel:wait interrupt instead of delay
    kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
    kdp520_spi_wait_command_complete();

    //read driving
    nrx_buff_word_index =0;
    kdp520_spi_set_commands(STATUS_15_CMD0, STATUS_15_CMD1, STATUS_15_CMD2, STATUS_15_CMD3);
    kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
    kdp520_spi_wait_command_complete();


    //check status
    nrx_buff_word_index =0;
    kdp520_spi_set_commands( 0x00 , PARA_5A_CMD1, 0x04, PARA_5A_CMD3);
    kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x04 );
    kdp520_spi_wait_command_complete();
    st_flash_info.signature = FLASH_SIGNATURE;

    //check
    if( nrx_buff_word[nrx_buff_word_index-1] != FLASH_SIGNATURE )
    {
        return  KDP_STATUS_ERROR;
    }

    //get ptr
    nrx_buff_word_index =0;
    kdp520_spi_set_commands( 0x0C , PARA_5A_CMD1, 0x01, PARA_5A_CMD3);
    kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
    kdp520_spi_wait_command_complete();
    st_flash_info.PTP = nrx_buff_word[nrx_buff_word_index-1] & 0XFF;

    //get ID
    nrx_buff_word_index =0;
    kdp520_spi_set_commands( 0x10 , PARA_5A_CMD1, 0x04, PARA_5A_CMD3);
    kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x04 );
    kdp520_spi_wait_command_complete();
    st_flash_info.ID = nrx_buff_word[nrx_buff_word_index-1] & 0XFFFFFFFF;

    if( st_flash_info.ID== 0x00 || st_flash_info.ID==0xFF  )
    {
        nrx_buff_word_index =0;
        kdp520_spi_set_commands( RDID_9F_CMD0 , RDID_9F_CMD1, RDID_9F_CMD2, RDID_9F_CMD3 );
        kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, RDID_9F_CMD2 );
        kdp520_spi_wait_command_complete();
        st_flash_info.ID = nrx_buff_word[nrx_buff_word_index-1] & 0xFF;
    }

    //get 4K erase support
    nrx_buff_word_index =0;
    kdp520_spi_set_commands( st_flash_info.PTP + 0, PARA_5A_CMD1, 0x01, PARA_5A_CMD3);
    kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
    kdp520_spi_wait_command_complete();
    st_flash_info.erase_4K_support = nrx_buff_word[nrx_buff_word_index-1] & 0x3;

    //get size
    nrx_buff_word_index =0;
    kdp520_spi_set_commands( st_flash_info.PTP+4 , PARA_5A_CMD1, 0x04, PARA_5A_CMD3);
    kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x04 );
    kdp520_spi_wait_command_complete();
    st_flash_info.flash_size_KByte = (nrx_buff_word[nrx_buff_word_index-1]>>10)>>3;
    ntemp = nrx_buff_word[nrx_buff_word_index-1]>>3;

    //get sector size 0x1C
    nrx_buff_word_index =0;
    kdp520_spi_set_commands( st_flash_info.PTP+0x1C , PARA_5A_CMD1, 0x01, PARA_5A_CMD3);
    kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
    kdp520_spi_wait_command_complete();
    st_flash_info.sector_size_Bytes = 1<<(nrx_buff_word[ nrx_buff_word_index-1 ]&0xFF);
    st_flash_info.total_sector_numbers = (ntemp / st_flash_info.sector_size_Bytes)+1;

    //get sector size 0x20
    nrx_buff_word_index =0;
    kdp520_spi_set_commands( st_flash_info.PTP+0x20 , PARA_5A_CMD1, 0x01, PARA_5A_CMD3);
    kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
    kdp520_spi_wait_command_complete();
    st_flash_info.block_size_Bytes = ( 1<<( nrx_buff_word[ nrx_buff_word_index-1 ] & 0xFF ) )/st_flash_info.sector_size_Bytes ;

    //get page size
    nrx_buff_word_index =0;
    kdp520_spi_set_commands( st_flash_info.PTP+0x28 , PARA_5A_CMD1, 0x01, PARA_5A_CMD3);
    kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
    kdp520_spi_wait_command_complete();
    ntemp = nrx_buff_word[nrx_buff_word_index-1]&0xFF;

    #if 0
    //20191219 add
    nrx_buff_word_index =0;
    kdp520_spi_set_commands(STATUS_15_CMD0, STATUS_15_CMD1, STATUS_15_CMD2, STATUS_15_CMD3);
    kdp520_spi_read_Rx_FIFO( nrx_buff_word, &nrx_buff_word_index, 0x01 );
    kdp520_spi_wait_command_complete();
    nrx_buff_word_index =0;
    kdp520_spi_set_commands(STATUS_35_CMD0, STATUS_35_CMD1, STATUS_35_CMD2, STATUS_35_CMD3);
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
        return  KDP_STATUS_ERROR;
    }
    return  KDP_STATUS_OK;

}

kdp_status_t address_protect_check( UINT32 addr_start, UINT32 addr_end, UINT32 pro_start, UINT32 pro_end)
{
    if ( protect_bypass == 0 )
    {
        if( ( ( addr_start < pro_start ) && ( addr_end >= pro_start ) ) 
              || ( ( addr_start == pro_start ) && ( addr_end > pro_end ) )
              || ( ( addr_start > pro_start ) && ( addr_start < pro_end ) ) 
          )
        {
            return KDP_STATUS_ERROR;
        }
    }
    return KDP_STATUS_OK;
}

kdp_status_t model_protect_check( UINT32 addr_start, UINT32 addr_end )
{
    kdp_status_t result = KDP_STATUS_OK;
    
    /*FW_INFO*/
    result = address_protect_check( addr_start, addr_end, KDP_FLASH_INF_ADDR, ( KDP_FLASH_INF_ADDR + KDP_FLASH_INF_SIZE ) );
    /*ALL_MODELS*/
    if ( result == KDP_STATUS_OK )
    {
        result = address_protect_check( addr_start, addr_end, KDP_FLASH_ALL_MODELS_ADDR, ( KDP_FLASH_ALL_MODELS_ADDR + KDP_FLASH_ALL_MODELS_SIZE ) );
    }
    
    return result;
}

kdp_status_t norflash_4k_erase(UINT32 address)
{
    u32 foffset =0;
    kdp520_spi_switch_fail();

    foffset = nor_area_judge_offset(address);

    norflash_write_control(1);
    while(1){
        if( ( norflash_busy_check() & 0x03) == 0x02)
        {
            break;
        }
    }
    
        /*Protect FW_INFO and ALL_MODELS*/
    if ( model_protect_check( (address - foffset), (address - foffset + 4096) ) == KDP_STATUS_ERROR )
    {
        return KDP_STATUS_ERROR;
    }
    
    //send erase sector index for 4kByte erase
    kdp520_spi_set_commands( address-foffset , ERASE_4K_21_CMD1, ERASE_4K_21_CMD2, ERASE_4K_21_CMD3);
    kdp520_spi_wait_command_complete();

    while(1)
    {
        if( ( norflash_busy_check() & 0x01) == 0x00)
        {
            break;
        }

        delay_us(50);
    }
    nor_flash_die_selection(0);

    kdp520_spi_switch_org();

    return KDP_STATUS_OK;
}


kdp_status_t norflash_32k_erase(UINT32 offset)
{
    u32 foffset =0;
    kdp520_spi_switch_fail();

    foffset = nor_area_judge_offset(offset);
    
    /*Protect FW_INFO and ALL_MODELS*/
    if ( model_protect_check( (offset - foffset), (offset - foffset + 32768) ) == KDP_STATUS_ERROR )
    {
        return KDP_STATUS_ERROR;
    }
    
    norflash_write_control(1);/* send write enabled */
    kdp520_spi_set_commands( offset-foffset, ERASE_32K_52_CMD1, ERASE_32K_52_CMD2, ERASE_32K_52_CMD3);
    kdp520_spi_wait_command_complete();

    while(1)
    {
        if( ( norflash_busy_check() & 0x01) == 0x00)
        {
            break;
        }

        delay_us(50);
    }
    nor_flash_die_selection(0);

    kdp520_spi_switch_org();
    return KDP_STATUS_OK;
}

kdp_status_t norflash_64k_erase(UINT32 offset)
{

    u32 foffset =0;
    kdp520_spi_switch_fail();
    foffset = nor_area_judge_offset(offset);

    norflash_write_control(1);  /* send write enabled */
    while(1){
        if( ( norflash_busy_check() & 0x03 ) == 0x02 )
        {
            break;
        }
    }

    /*Protect FW_INFO and ALL_MODELS*/
    if ( model_protect_check( (offset - foffset), (offset - foffset + 65536) ) == KDP_STATUS_ERROR )
    {
        return KDP_STATUS_ERROR;
    }

    //send erase sector index for 64kByte erase
    kdp520_spi_set_commands( offset-foffset, ERASE_64K_DC_CMD1, ERASE_64K_DC_CMD2, ERASE_64K_DC_CMD3);
    kdp520_spi_wait_command_complete();

    while(1){
        if( ( norflash_busy_check() & 0x01) == 0x00 )
        {
            break;
        }
        delay_us(50);
    }
    nor_flash_die_selection(0);

    kdp520_spi_switch_org();
    return KDP_STATUS_OK;
}


kdp_status_t norflash_chip_erase(void)
{
    kdp520_spi_switch_fail();
    norflash_write_control(1);
    while(1){
        if( ( norflash_busy_check() & 0x03 ) == 0x02)
            break;
    }
    kdp520_spi_set_commands(CHIP_ERASE_C7_CMD0, CHIP_ERASE_C7_CMD1, CHIP_ERASE_C7_CMD2, CHIP_ERASE_C7_CMD3);
    kdp520_spi_wait_command_complete();

    while(1){
        if( ( norflash_busy_check() & 0x01) == 0x00 )
            break;
        delay_us(50);
    }
    nor_flash_die_selection(0);
    kdp520_spi_switch_org();
    return KDP_STATUS_OK;
}

kdp_status_t norflash_read(uint32_t addr, void *data, uint32_t target_Bytes)
{
    s32 total_lens;
    u32 write_addr;
    u32 read_data;

    if ((target_Bytes & 0x3) > 0) return KDP_STATUS_ERROR;

    foffset = nor_area_judge_offset(addr);
    kdp520_spi_switch_org();

    #if(GD25D512MD_LOG_EN == YES)
    dbg_msg_flash("read address 0x%X", addr);
    dbg_msg_flash("read offset 0x%X", foffset);
    #endif
    total_lens = target_Bytes;
    write_addr = (u32)data;

    while (total_lens > 0)
    {
        if(total_lens > DMA_LIMIT)
        {
            //dbg_msg_flash("[DMA] [DMA_LIMIT = %d] read_data = 0x%x, write_addr = 0x%x, total_lens = %d", DMA_LIMIT, read_data, write_addr, total_lens);

        #if FLASH_4BYTES_CMD_EN
            norflash_4Bytes_ctrl(1);
            #ifdef SPI_QUAD_MODE
                kdp520_spi_set_commands( (uint32_t)(addr-foffset) , QUAD_READ_EC_CMD1, DMA_LIMIT, QUAD_READ_EC_CMD3 );
            #else
                #if SPI_BUS_SPEED == SPI_BUS_SPEED_100MHZ
                    dbg_msg_err("[%s] err, 13H cmd can not run on 100MHz");
                #else
                    kdp520_spi_set_commands((uint32_t)data, SPI020_13_CMD1, DMA_LIMIT, SPI020_13_CMD3);
                #endif
            #endif

        #else
            //add 3Bytes address read command here...
        #endif
            
            kdp_flash_to_ddr_dma_copy((UINT32*)read_data, (UINT32*)write_addr, DMA_LIMIT);
            addr += DMA_LIMIT;
            write_addr += DMA_LIMIT;
            total_lens -= DMA_LIMIT;
        }
        else
        {
            //dbg_msg_flash("[DMA] read_data = 0x%x, write_addr = 0x%x, total_lens = %d", read_data, write_addr, total_lens);

        #if FLASH_4BYTES_CMD_EN
            norflash_4Bytes_ctrl(1);
            #ifdef SPI_QUAD_MODE
                kdp520_spi_set_commands( (uint32_t)(addr-foffset) , QUAD_READ_EC_CMD1, total_lens, QUAD_READ_EC_CMD3 );
            #else
                #if SPI_BUS_SPEED == SPI_BUS_SPEED_100MHZ
                    dbg_msg_err("[%s] err, 13H cmd can not run on 100MHz");
                #else
                    kdp520_spi_set_commands((uint32_t)data, SPI020_13_CMD1, total_lens, SPI020_13_CMD3);
                #endif
            #endif

        #else
            //add 3Bytes address read command here...
        #endif
            
            kdp_flash_to_ddr_dma_copy((UINT32*)read_data, (UINT32*)write_addr, total_lens);
            total_lens -= total_lens;
        }
    }


    #ifndef MIXING_MODE_OPEN_RENDERER
    //kdp520_spi_wait_command_complete();/* wait for command complete */
    #endif

    #if FLASH_4BYTES_CMD_EN
    norflash_4Bytes_ctrl(0);
    #endif

    nor_flash_die_selection(0);
    return KDP_STATUS_OK;
}


void norflash_write_running( UINT8 type, UINT32 offset, UINT32 total_send_byte, UINT8 *buf )
{
    u8  *write_buf = buf;
    u32 foffset =0;

    kdp520_spi_switch_org();
    foffset = nor_area_judge_offset(offset);

    norflash_write_control(1);

    /* fill in command 0~3 */
    if(type & FLASH_QUAD_RW)
    {
        #if( FLASH_QUAD_EN == YES )
        kdp520_spi_set_commands(offset-foffset, QPAGE_WRITE_34_CMD1, total_send_byte, QPAGE_WRITE_34_CMD3);
        #else
        kdp520_spi_set_commands(offset, SPI020_32_CMD1, total_send_byte, SPI020_32_CMD3);
        #endif
    }
    else
    {
        #if (FLASH_4BYTES_CMD_EN)
        kdp520_spi_set_commands(offset-foffset, PAGE_WRITE_12_CMD1, total_send_byte, PAGE_WRITE_12_CMD3);
        #else
        kdp520_spi_set_commands(offset, SPI020_02_CMD1, total_send_byte, SPI020_02_CMD3);
        #endif
    }

    if ( type & FLASH_DMA_WRITE )
    {
        outw(SPI020REG_INTERRUPT, SPI020_cmd_cmplt_intr_en | SPI020_DMA_EN);
        return;
    }

    kdp520_spi_write_Tx_FIFO(write_buf, total_send_byte);
    kdp520_spi_wait_command_complete();

    //check status
    kdp520_spi_switch_fail();
    while(1)
    {
        if( ( norflash_busy_check() & 0x01 ) == 0x00 )
        {
            break;
        }
        delay_us(40);
    }
    nor_flash_die_selection(0);
    kdp520_spi_switch_fail();
    return;
}

UINT8 program_address_check( uint32_t addr )
{
    //check sector start address
    if( ( addr % st_flash_info.sector_size_Bytes ) != 0 )
    {
        return 0;   //fail
    }
    return 1;       //pass
}


kdp_status_t norflash_program( UINT32 addr, UINT8 *data, UINT32 send_bytes)
{
    UINT32  i =0;
    //UINT32    data_acc = send_bytes;
    UINT32  pages_number = send_bytes / st_flash_info.page_size_Bytes;
    UINT16  nbytes_num = send_bytes % st_flash_info.page_size_Bytes;
    UINT16  page_size = st_flash_info.page_size_Bytes;

    #if( FLASH_QUAD_EN == YES )
    UINT8   flash_type = FLASH_QUAD_RW;
    #else
    UINT8   flash_type = FLASH_NORMAL;
    #endif

    #if( OTA_LOG_EN == YES )
    dbg_msg_flash("flash program start");
    #endif

    //----check page start address aligned ----
    if(  program_address_check( addr ) == 0 )
    {
        return KDP_STATUS_ERROR;
    }

    if( (send_bytes %4) != 0 )
    {
        return KDP_STATUS_ERROR;
    }

    /*Protect FW_INFO and ALL_MODELS*/
    if ( model_protect_check( addr, (addr + send_bytes) ) == KDP_STATUS_ERROR )
    {
        return KDP_STATUS_ERROR;
    }

    //parsing
    for( i=0; i<pages_number ; i++  )
    {
        norflash_write_running( flash_type, ((UINT32 )addr + (i*page_size)) , page_size, ( UINT8 *)(data + i*page_size) );
    }
    if( nbytes_num > 0 )
    {
        norflash_write_running( flash_type, ( (UINT32 )addr + ( page_size *pages_number ) )
                                , nbytes_num, (UINT8 *)( data+ page_size * pages_number ) );
    }

    #if( OTA_LOG_EN == YES )
    dbg_msg_flash("flash program Done");
    #endif

    return KDP_STATUS_OK;


}


kdp_status_t norflash_erase_multi_sector(UINT32 nstart_add,  UINT32 nend_add)
{

    UINT32 nstart_index = nstart_add /st_flash_info.sector_size_Bytes;
    UINT32 nend_index = nend_add/st_flash_info.sector_size_Bytes;
    if( ( nend_add%st_flash_info.sector_size_Bytes) == 0  && nend_index > nstart_index ){
        nend_index --;
    }

    UINT16  i =0;
    //UINT16 nBlock_numbers = st_flash_info.sector_size_Bytes / st_flash_info.block_size_Bytes;
    UINT16 nsectors_in_block = 0;

    //dbg_msg_flash(" Block erase start=======" );
    //dbg_msg_flash(" start sector index %d, end %d", nstart_index, nend_index );

    nsectors_in_block = st_flash_info.block_size_Bytes;

    if( nstart_index > nend_index )
    {
        return KDP_STATUS_ERROR;
    }

    /*Protect FW_INFO and ALL_MODELS*/
    if ( model_protect_check( nstart_add, nend_add ) == KDP_STATUS_ERROR )
    {
        return KDP_STATUS_ERROR;
    }

    for( i = nstart_index; i<= nend_index; ){
        if( (i%nsectors_in_block) == 0){
            if( (i+nsectors_in_block-1) <= nend_index ){
                norflash_64k_erase( i*st_flash_info.sector_size_Bytes );
                i+= nsectors_in_block;
            }
            else{
                //4K erase
                norflash_4k_erase( i*st_flash_info.sector_size_Bytes );
                i++;
            }
        }
        else{
            //4K erase
            norflash_4k_erase( i*st_flash_info.sector_size_Bytes );
            i++;
        }
    }
    return KDP_STATUS_OK;
}



UINT8 norflash_id(void)
{
    return st_flash_info.ID;
}


kdp_status_t norflash_get_status(void)
{
    UINT32 flash_status = norflash_busy_check() & 0x01;
    return ( flash_status == 1 ? KDP_STATUS_ERROR : KDP_STATUS_OK );
}

void norflash_set_protect_bypass(UINT8 bypass)
{
    protect_bypass = bypass;
}

void kdp520_flash_reset(void)
{
    kdp520_spi_set_commands(RESET_66_CMD0, RESET_66_CMD1, RESET_66_CMD2, RESET_66_CMD3);
    kdp520_spi_set_commands(RESET_99_CMD0, RESET_99_CMD1, RESET_99_CMD2, RESET_99_CMD3);
}



FLASH_DEV flash_vendor = {
  .initial = kdp520_spi_initialize,
  .program = norflash_program,
  .read = norflash_read,
  .erasemultisector = norflash_erase_multi_sector,
  .erase4KB = norflash_4k_erase,
  .erase64KB = norflash_64k_erase,
  .eraseallchip = norflash_chip_erase,
  .get_info = norflash_get_info,
  .get_id = norflash_id,
  .GetStatus = norflash_get_status,
  .set_bypass = norflash_set_protect_bypass,
};



void _spi020_check_status_til_ready_2(void)
{
    /* fill in command 0~3 */
    kdp520_spi_set_commands(SPI020_05_CMD0_ORG, SPI020_05_CMD1_ORG, SPI020_05_CMD2_ORG, SPI020_05_CMD3_ORG);
    /* wait for command complete */
    kdp520_spi_wait_command_complete();
}

void kdp520_flash_check_status_til_ready(void)
{
    /* savecodesize, move into here */
    kdp520_spi_wait_command_complete();
    /* read status */
    _spi020_check_status_til_ready_2();
}

void kdp520_flash_dma_read_stop(void)
{
    kdp520_spi_wait_command_complete();/* wait for command complete */
//  kdp520_flash_check_status_til_ready();

    outw((INT8U * )SPI020REG_INTERRUPT, SPI020_cmd_cmplt_intr_en);/* disable DMA function */
}

void kdp520_flash_dma_write_stop(void)
{
    kdp520_spi_wait_command_complete();/* savecodesize, move into kdp520_flash_check_status_til_ready */
    outw((INT8U * )SPI020REG_INTERRUPT, SPI020_cmd_cmplt_intr_en);/* disable DMA function */
    //kdp520_flash_check_status_til_ready();
    _spi020_check_status_til_ready_2();
}



#endif  //end of #if (FLASH_VENDOR_SELECT == GD25Q256D ) || (FLASH_VENDOR_SELECT == GD25S512MD )
