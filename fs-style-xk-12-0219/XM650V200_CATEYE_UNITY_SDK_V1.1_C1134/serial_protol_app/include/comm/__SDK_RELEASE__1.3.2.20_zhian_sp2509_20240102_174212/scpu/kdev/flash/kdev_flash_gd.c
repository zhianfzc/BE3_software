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
*  kdev_flash_gd.c
*
*  Project:
*  --------
*  KL520
*
*  Description:
*  ------------
*  This SPI Flash driver is specific for GigaDevice SPI Flash Access
*  HW: Faraday FTSPI020
*
*  Author:
*  -------
*  Teresa Chen
**
******************************************************************************/

#ifdef USE_KDRV

#include "kdev_flash_gd.h"
#include "kdrv_spif.h"
#include "kdev_flash.h"
#include "io.h"
//#define FLASH_GB_DBG
#ifdef FLASH_GB_DBG
#include "kmdw_console.h"
#define flash_msg(fmt, ...) err_msg("[%s] " fmt, __func__, ##__VA_ARGS__)
#else
#define flash_msg(fmt, ...)
#endif
/* Flash Information */
static kdev_flash_info_t FlashInfo = {
    0, /* FLASH_SECTOR_INFO  */
    0, /* FLASH_SECTOR_COUNT */
    0, /* FLASH_SECTOR_SIZE  */
    0, /* FLASH_PAGE_SIZE    */
    0, /* FLASH_PROGRAM_UNIT */
    0,  /* FLASH_ERASED_VALUE */
    0
};


spi_flash_t flash_info;

#if FLASH_4BYTES_CMD_EN
bool kdev_flash_is_4byte_address(void)
{
    bool ret = false;
    
    if( flash_info.flash_size > FLASH_3BYTE_ADDR_MAX )
        ret = true;
    return ret;
}
#endif

void kdev_flash_4Bytes_ctrl(uint8_t  enable)
{
#if FLASH_4BYTES_CMD_EN
    if (kdev_flash_is_4byte_address())
    {
        if (enable) {
            kdrv_spif_set_commands(SPI020_B7_CMD0, SPI020_B7_CMD1, SPI020_B7_CMD2, SPI020_B7_CMD3);
        } else {
            kdrv_spif_set_commands(SPI020_E9_CMD0, SPI020_E9_CMD1, SPI020_E9_CMD2, SPI020_E9_CMD3);
        }
        /* wait for command complete */
        kdrv_spif_wait_command_complete();
    }
#endif
}

void kdev_flash_write_control(uint8_t  enable)
{
    /* fill in command 0~3 */
    if (enable) {
        kdrv_spif_set_commands(SPI020_06_CMD0, SPI020_06_CMD1, SPI020_06_CMD2, SPI020_06_CMD3);
    } else {
        kdrv_spif_set_commands(SPI020_04_CMD0, SPI020_04_CMD1, SPI020_04_CMD2, SPI020_04_CMD3);
    }
    /* wait for command complete */
    kdrv_spif_wait_command_complete();
}

void kdev_flash_64kErase(uint32_t  offset)
{
    /* The start offset should be in 64K boundary */
    /* if(offset % SPI020_64K) return 1; */

    kdev_flash_write_control(1);/* send write enabled */

    /* fill in command 0~3 */
    #if FLASH_4BYTES_CMD_EN
    if (kdev_flash_is_4byte_address())
        kdrv_spif_set_commands(offset, SPI020_DC_CMD1, SPI020_DC_CMD2, SPI020_DC_CMD3);
    else
        kdrv_spif_set_commands(offset, SPI020_D8_CMD1, SPI020_D8_CMD2, SPI020_D8_CMD3);
    #else
    kdrv_spif_set_commands(offset, SPI020_D8_CMD1, SPI020_D8_CMD2, SPI020_D8_CMD3);
    #endif
    /* wait for command complete */
    kdrv_spif_check_status_till_ready();
}

void kdev_flash_32kErase(uint32_t  offset)
{
    /* The start offset should be in 64K boundary */
    /* if(offset % SPI020_64K) return 1; */

    kdev_flash_write_control(1);/* send write enabled */

    /* fill in command 0~3 */
    kdrv_spif_set_commands(offset, SPI020_52_CMD1, SPI020_52_CMD2, SPI020_52_CMD3);
    /* wait for command complete */
    kdrv_spif_check_status_till_ready();
}

void kdev_flash_4kErase(uint32_t  offset)
{
    /* The start offset should be in 64K boundary */
    /* if(offset % SPI020_4K) return 1; */

    kdev_flash_write_control(1);/* send write enabled */

    /* fill in command 0~3 */
    #if FLASH_4BYTES_CMD_EN
    if (kdev_flash_is_4byte_address())
        kdrv_spif_set_commands(offset, SPI020_21_CMD1, SPI020_21_CMD2, SPI020_21_CMD3);
    else
        kdrv_spif_set_commands(offset, SPI020_20_CMD1, SPI020_20_CMD2, SPI020_20_CMD3);
    #else
    kdrv_spif_set_commands(offset, SPI020_20_CMD1, SPI020_20_CMD2, SPI020_20_CMD3);
    #endif
    /* wait for command complete */
    kdrv_spif_check_status_till_ready();
}

uint32_t  kdev_flash_probe(spi_flash_t *flash)
{
    uint32_t   chip_id=0;

    uint32_t   probe_90_instruction=0;
    /* fill in command 0~3 */
    //Read Manufacturer and Device Identification by JEDEC ID(0x9F)
    kdrv_spif_set_commands(SPI020_9F_CMD0, SPI020_9F_CMD1, SPI020_9F_CMD2, SPI020_9F_CMD3);
    /* read data */
    kdrv_spif_read_data(/*(uint8_t  *)*/&chip_id, 0x3);
    /* wait for command complete */
    kdrv_spif_wait_command_complete();

    //flash->manufacturer = (chip_id>>24);
    flash->manufacturer = (uint8_t )chip_id;
    if (flash->manufacturer == 0x00 || flash->manufacturer == 0xFF) {
       /* fill in command 0~3 */
       //Read Manufacturer and Device Identification by 0x90
        kdrv_spif_set_commands(SPI020_90_CMD0, SPI020_90_CMD1, SPI020_90_CMD2, SPI020_90_CMD3);
        /* read data */
        kdrv_spif_read_data(/*(uint8_t  *)*/&chip_id, 0x02/*0x4*/);
        /* wait for command complete */
        kdrv_spif_wait_command_complete();
        //flash->manufacturer = (chip_id>>24);
        flash->manufacturer = (uint8_t )chip_id;
        probe_90_instruction=1;
    }
    flash->flash_id = (chip_id>>8);
    return probe_90_instruction;
}

/* ===================================
 * Init SPI controller and flash device.
 * Init flash information and register functions.
 * =================================== */
void kdev_flash_read_flash_id(void)
{
    uint32_t  probe_90_instruction; 
    uint32_t  sizeId;
    char *flash_manu;

    probe_90_instruction=kdev_flash_probe(&flash_info);
   switch (flash_info.manufacturer) {
    case FLASH_WB_DEV:
        flash_manu = "WINBOND";
        break;
    case FLASH_MXIC_DEV:
        flash_manu = "MXIC";
        break;
    case FLASH_Micron_DEV:
        flash_manu = "Micron";
        break;
    case FLASH_GD_DEV:
        flash_manu = "GigaDevice";
        break;
    case FLASH_ZBIT_DEV:
        flash_manu = "Zbit";
        break;
    default:
        flash_manu = "Unknown";
        break;
    }
    if(probe_90_instruction)
    {
        sizeId = flash_info.flash_id & 0x00FF;
        if(sizeId >= FLASH_SIZE_1MB_ID)
            flash_info.flash_size = 0x400 * (1<<(sizeId-FLASH_SIZE_1MB_ID+1));
    }
    else
    {
        sizeId = (flash_info.flash_id & 0xFF00)>>8;
        if(sizeId >= FLASH_SIZE_512MB_ID) {
            flash_info.flash_size = 0x400 * (1<<(sizeId-FLASH_SIZE_1MB_ID-FLASH_SIZE_SHIFT));
            flash_msg("flash_size 0x%2X >= 512MB = %d kbytes\n",sizeId, flash_info.flash_size);
        }
        else if(sizeId >= FLASH_SIZE_1MB_ID) {
            flash_info.flash_size = 0x400 * (1<<(sizeId-FLASH_SIZE_1MB_ID));
            flash_msg("flash_size 0x%2X = %d kbytes\n",sizeId, flash_info.flash_size);
        }
    }

    flash_msg("Manufacturer ID = 0x%02X (%s)\n", flash_info.manufacturer,flash_manu);
    flash_msg("Device ID       = 0x");
    if(probe_90_instruction)
        flash_msg("%02X\n", flash_info.flash_id);
    else
        flash_msg("%04X\n", flash_info.flash_id);

    flash_msg("Flash Size      = ");
    if((flash_info.flash_size%1024)==0x00) {
        flash_msg("%dkByte(%dMByte)\n", flash_info.flash_size,flash_info.flash_size>>10);
    } else {
        flash_msg("%dkByte\n", flash_info.flash_size);
    }
    FlashInfo.flash_size = flash_info.flash_size;
}

/* GD Flash */
void kdev_flash_read_status(void)
{
	uint16_t nrx_buff_word_index = 0;
    uint32_t RDSR1=0; //05h
    uint32_t RDSR2=0; //35h
    uint32_t RDCR=0; //15h

	kdev_flash_write_control(1);
    flash_msg("kdev_flash_write_control done\n");
	kdrv_spif_set_commands( SPI020_05_CMD0_w , SPI020_05_CMD1_w, SPI020_05_CMD2_w, SPI020_05_CMD3_w );
    nrx_buff_word_index = 0;
    kdrv_spif_read_Rx_FIFO( &RDSR1, &nrx_buff_word_index, 0x01 );
    flash_msg("SPI020_05_CMD buf[0]=0x%2X\n", RDSR1 );
	kdrv_spif_wait_command_complete();

	kdrv_spif_set_commands(SPI020_15_CMD0, SPI020_15_CMD1, SPI020_15_CMD2, SPI020_15_CMD3 );
    nrx_buff_word_index = 0;
    kdrv_spif_read_Rx_FIFO( &RDCR, &nrx_buff_word_index, 0x01 );
    flash_msg("SPI020_15_CMD1 buf[0]=0x%2X\n", RDCR );
	kdrv_spif_wait_command_complete();

    flash_msg("Manufacturer ID = 0x%02X \n", flash_info.manufacturer);
    
    //enable volatile bit
    kdrv_spif_set_commands(SPI020_50_CMD0, SPI020_50_CMD1, SPI020_50_CMD2, SPI020_50_CMD3);
    kdrv_spif_wait_command_complete();
    flash_msg("SPI020_50_CMD0 done\n");

    RDSR1 &= ~0x7C; /* disable Block protect bit6,5,4,3,2*/
    if( (RDCR  & 0x0C) ) /* check Program/Erase Error bits */
    {
        kdev_flash_write_control(1);
        flash_msg("need to clean program/erase error 0x%2X \n", RDCR&~0x0C );
        kdrv_spif_set_commands(SPI020_30_CMD0, SPI020_30_CMD1, SPI020_30_CMD2, SPI020_30_CMD3 );
        flash_msg("SPI020_30_CMD0 done\n");
        kdrv_spif_wait_command_complete();
    }
    RDCR  &= ~0x60; /* driver output strength 00 100% */

    flash_msg("RDSR1(%4X) RDCR(%4X) \n", RDSR1, RDCR);
    kdrv_spif_set_commands(SPI020_01_CMD0, SPI020_01_CMD1, 1, SPI020_01_CMD3 );
    kdrv_spif_write_data((uint8_t*)&RDSR1, 1);
    flash_msg("spi020_check_status_til_ready\n");
    kdrv_spif_wait_command_complete();//kdrv_spif_check_status_til_ready();

    kdrv_spif_set_commands(SPI020_11_CMD0, SPI020_11_CMD1, SPI020_11_CMD2, SPI020_11_CMD3 );
    flash_msg("need to set driver strength 0x%2X \n", RDCR );
    kdrv_spif_write_data((uint8_t*)&RDCR, 1);
    kdrv_spif_check_status_till_ready();
    
    /* check Quad mode */
    if(FLASH_OP_MODE & FLASH_QUAD_RW)
    {
        flash_msg("Set QE enabled \n");
        kdrv_spif_set_commands(SPI020_35_CMD0, SPI020_35_CMD1, SPI020_35_CMD2, SPI020_35_CMD3 );
        nrx_buff_word_index = 0;
        kdrv_spif_read_Rx_FIFO( &RDSR2, &nrx_buff_word_index, 0x01 );
        flash_msg("SPI020_35_CMD1 buf[0]=0x%2X\n", RDSR2 );
        kdrv_spif_wait_command_complete();
        if(!(RDSR2 & BIT1))
        {
            flash_msg("RDSR2 & BIT1=0x%2X\n", RDSR2 & BIT1 );
            RDSR2 |= BIT1;
            kdrv_spif_set_commands(SPI020_31_CMD0, SPI020_31_CMD1, SPI020_31_CMD2, SPI020_31_CMD3 );
            kdrv_spif_write_data((uint8_t*)(&RDSR2), 1);
            kdrv_spif_wait_command_complete();//spi020_check_status_til_ready();
            flash_msg("FLASH_WB/GD/ZBIT Set QE OK!! \n");

            /*flash_msg("read FLASH_WB/GD/ZBIT status-2!! \n");
            kdrv_spif_set_commands(SPI020_35_CMD0, SPI020_35_CMD1, SPI020_35_CMD2, SPI020_35_CMD3 );
            nrx_buff_word_index = 0;
            kdrv_spif_read_Rx_FIFO( &RDSR2, &nrx_buff_word_index, 0x01 );
            flash_msg("SPI020_35_CMD1 buf[0]=0x%2X\n", RDSR2 );
            kdrv_spif_wait_command_complete();*/
        }
    }
}

kdev_status_t kdev_flash_initialize(void)//ARM_Flash_SignalEvent_t cb_event)
{
    kdrv_spif_initialize();
    kdev_flash_read_flash_id();
    kdev_flash_read_status();
    #if FLASH_4BYTES_CMD_EN
    kdev_flash_4Bytes_ctrl(1);
    #else
    kdev_flash_4Bytes_ctrl(0);
    #endif
    return KDEV_STATUS_OK;       
}

kdev_status_t kdev_flash_uninitialize(void)
{
    return KDEV_STATUS_OK;
}

kdev_status_t kdev_flash_power_control(ARM_POWER_STATE state)
{
    switch (state) {
    case ARM_POWER_OFF:
        break;

    case ARM_POWER_LOW:
        break;

    case ARM_POWER_FULL:
        break;

    default:
        return KDEV_STATUS_ERROR;
    }
    return KDEV_STATUS_OK;
}

void kdev_flash_read(uint8_t  type, uint32_t  offset, uint32_t  len, void *buf)
{
    uint32_t  *read_buf;//uint8_t                *read_buf;

    #if FLASH_4BYTES_CMD_EN
    kdev_flash_4Bytes_ctrl(1);
    #endif

    if (type & FLASH_DMA_READ) {
        outw(SPI020REG_INTERRUPT, SPI020_cmd_cmplt_intr_en | SPI020_DMA_EN);/* enable DMA function */
    }

    /* fill in command 0~3 */
    if (type & FLASH_DTR_RW) {
        if (type & FLASH_DUAL_READ)
                kdrv_spif_set_commands(offset, SPI020_BD_CMD1, len, SPI020_BD_CMD3);
            else if(type & FLASH_QUAD_RW)
                kdrv_spif_set_commands(offset, SPI020_ED_CMD1, len, SPI020_ED_CMD3);
            else
                kdrv_spif_set_commands(offset, SPI020_0D_CMD1, len, SPI020_0D_CMD3);
    } else if (type & FLASH_DUAL_READ) {
        if(type & FLASH_IO_RW) {
            //fLib_printf("Daul (0xBB) read\n");
            #if FLASH_4BYTES_CMD_EN
            if (kdev_flash_is_4byte_address())
                kdrv_spif_set_commands(offset, SPI020_BC_CMD1, len, SPI020_BC_CMD3);
            else
                kdrv_spif_set_commands(offset, SPI020_BB_CMD1, len, SPI020_BB_CMD3);
            #else
            kdrv_spif_set_commands(offset, SPI020_BB_CMD1, len, SPI020_BB_CMD3);
            #endif
        } else {
            //fLib_printf("Daul (0x3B) read\n");
            #if FLASH_4BYTES_CMD_EN
            if (kdev_flash_is_4byte_address())
                kdrv_spif_set_commands(offset, SPI020_3C_CMD1, len, SPI020_3C_CMD3);
            else
                kdrv_spif_set_commands(offset, SPI020_3B_CMD1, len, SPI020_3B_CMD3);
            #else
            kdrv_spif_set_commands(offset, SPI020_3B_CMD1, len, SPI020_3B_CMD3);
            #endif
        }
    } else if(type & FLASH_QUAD_RW) {
        if(type & FLASH_IO_RW) {
            //fLib_printf("Quad (0xEB) read\n");
            #if FLASH_4BYTES_CMD_EN
            if (kdev_flash_is_4byte_address())
                kdrv_spif_set_commands(offset, SPI020_EC_CMD1, len, SPI020_EC_CMD3);
            else
                kdrv_spif_set_commands(offset, SPI020_EB_CMD1, len, SPI020_EB_CMD3);
            #else
            kdrv_spif_set_commands(offset, SPI020_EB_CMD1, len, SPI020_EB_CMD3);
            #endif
        } else {
            //fLib_printf("Quad (0x6B) read\n");
            #if FLASH_4BYTES_CMD_EN
            if (kdev_flash_is_4byte_address())
                kdrv_spif_set_commands(offset, SPI020_6C_CMD1, len, SPI020_6C_CMD3);
            else
                kdrv_spif_set_commands(offset, SPI020_6B_CMD1, len, SPI020_6B_CMD3);
            #else
                kdrv_spif_set_commands(offset, SPI020_6B_CMD1, len, SPI020_6B_CMD3);
            #endif
        }
    } else if(type & FLASH_FAST_READ) {
        //fLib_printf("Fast (0x0B) read\n");
        #if FLASH_4BYTES_CMD_EN
        if (kdev_flash_is_4byte_address())
            kdrv_spif_set_commands(offset, SPI020_0C_CMD1, len, SPI020_0C_CMD3);
        else
            kdrv_spif_set_commands(offset, SPI020_0B_CMD1, len, SPI020_0B_CMD3);
        #else
        kdrv_spif_set_commands(offset, SPI020_0B_CMD1, len, SPI020_0B_CMD3);
        #endif
    } else {/* normal read */ 
        //fLib_printf("Normal (0x03) read\n");
        #if FLASH_4BYTES_CMD_EN
        if (kdev_flash_is_4byte_address())
            kdrv_spif_set_commands(offset, SPI020_13_CMD1, len, SPI020_13_CMD3);
        else
            kdrv_spif_set_commands(offset, SPI020_03_CMD1, len, SPI020_03_CMD3);
        #else
        kdrv_spif_set_commands(offset, SPI020_03_CMD1, len, SPI020_03_CMD3);
        #endif
    }

    if (type & FLASH_DMA_READ) {
        return;
    }

    read_buf = (uint32_t  *)buf;
    kdrv_spif_read_data(read_buf, len);/* read data */
    kdrv_spif_wait_command_complete();/* wait for command complete */

    #if FLASH_4BYTES_CMD_EN
    kdev_flash_4Bytes_ctrl(0);
    #endif
}

void kdev_flash_dma_read_stop(void)
{
    kdrv_spif_wait_command_complete();/* wait for command complete */
    outw((int8_t  * )SPI020REG_INTERRUPT, SPI020_cmd_cmplt_intr_en);/* disable DMA function */
}

void kdev_flash_dma_write_stop(void)
{
    kdrv_spif_wait_command_complete();/* savecodesize, move into spi020_check_status_til_ready */
    outw((int8_t  * )SPI020REG_INTERRUPT, SPI020_cmd_cmplt_intr_en);/* disable DMA function */
    kdrv_spif_check_status_till_ready_2();
}

uint8_t  kdev_flash_r_state_OpCode_35(void)
{
	uint16_t nrx_buff_word_index = 0;
    uint32_t RDSR2=0; //35h
    /* fill in command 0~3 */
    kdrv_spif_set_commands(SPI020_35_CMD0, SPI020_35_CMD1, SPI020_35_CMD2, SPI020_35_CMD3 );
    nrx_buff_word_index = 0;
    kdrv_spif_read_Rx_FIFO( &RDSR2, &nrx_buff_word_index, 0x01 );
    //fLib_printf("SPI020_35_CMD1 buf[0]=0x%2X\n", RDSR2 );
    kdrv_spif_wait_command_complete();
    return (uint8_t)RDSR2;
}

void kdev_flash_write(uint8_t  type, uint32_t  offset, uint32_t  len, void *buf, uint32_t  buf_offset)
{
    uint8_t  *write_buf;

    /* This function does not take care about 4 bytes alignment */
    /* if ((uint32_t )(para->buf) % 4) return 1; */
    #if FLASH_4BYTES_CMD_EN
    kdev_flash_4Bytes_ctrl(1);
    #endif

    //fLib_printf("write: offset:%x\n", offset);
    kdev_flash_write_control(1);/* send write enabled */   

    /* fill in command 0~3 */
    if(type & FLASH_QUAD_RW) {
        //fLib_printf("Quad (0x32) write\n");
        #if FLASH_4BYTES_CMD_EN
        if (kdev_flash_is_4byte_address())
            kdrv_spif_set_commands(offset, SPI020_34_CMD1, len, SPI020_34_CMD3);
        else
            kdrv_spif_set_commands(offset, SPI020_32_CMD1, len, SPI020_32_CMD3);
        #else
        kdrv_spif_set_commands(offset, SPI020_32_CMD1, len, SPI020_32_CMD3);
        #endif
    } else {
        //fLib_printf("Normal (0x02) write\n");
        #if FLASH_4BYTES_CMD_EN
        if (kdev_flash_is_4byte_address())
            kdrv_spif_set_commands(offset, SPI020_12_CMD1, len, SPI020_12_CMD3);
        else
            kdrv_spif_set_commands(offset, SPI020_02_CMD1, len, SPI020_02_CMD3);
        #else
        kdrv_spif_set_commands(offset, SPI020_02_CMD1, len, SPI020_02_CMD3);
        #endif
    }

    if (type & FLASH_DMA_WRITE) {
        outw(SPI020REG_INTERRUPT, SPI020_cmd_cmplt_intr_en | SPI020_DMA_EN);/* enable DMA function */
        return;
    }

    write_buf = (uint8_t  *)buf+buf_offset;
        //fLib_printf("write_buf:%x, len=%x\n",write_buf, len);
    kdrv_spif_write_data(write_buf, len);
    kdrv_spif_check_status_till_ready();

    #if FLASH_4BYTES_CMD_EN
    kdev_flash_4Bytes_ctrl(0);
    #endif
    return;
}

kdev_status_t kdev_flash_readdata(uint32_t addr, void *data, uint32_t cnt)
{
    uint8_t Option;

    Option = FLASH_OP_MODE;
    kdev_flash_read(Option, addr , cnt , data);
    return KDEV_STATUS_OK;
}

kdev_status_t kdev_flash_programdata(uint32_t addr, const void *data, uint32_t cnt)
{
    uint8_t Option;
    uint16_t wloop = 0;
    uint16_t i = 0;
    uint16_t final = 0;

    if (cnt % FLASH_PAGE_SIZE == 0)
        wloop = (cnt / FLASH_PAGE_SIZE);
    else
        wloop = (cnt / FLASH_PAGE_SIZE) + 1;

    Option = FLASH_OP_MODE;
    for(i=0; i<wloop; i++)
    {
        if(i == (wloop - 1))
        {
            final = cnt-(i*FLASH_PAGE_SIZE); //should <= 256
            kdev_flash_write(Option, (addr+(i*FLASH_PAGE_SIZE)), final, (void *)data, (i*FLASH_PAGE_SIZE));
        }
        else
        {
            kdev_flash_write(Option, (addr+(i*FLASH_PAGE_SIZE)), FLASH_PAGE_SIZE, (void *)data, (i*FLASH_PAGE_SIZE));
        }
    }
    return KDEV_STATUS_OK;
}

kdev_status_t kdev_flash_erase_sector(uint32_t addr)
{
    kdev_flash_4kErase(addr); //for program partial
    //kdev_flash_64kErase(addr); //for program all
    return KDEV_STATUS_OK;
}

kdev_status_t kdev_flash_erase_multi_sector(uint16_t start_addr, uint16_t end_addr)
{
    uint16_t i=0;
    flash_msg("_flash_erase_multi_sectors start_addr = %d! end_addr = %d!", start_addr, end_addr);
    for(i=start_addr; i<end_addr; i++)
    {
        flash_msg("_flash_erase_multi_sectors addr = %d!", i*SPI020_SECTOR_SIZE);
        kdev_flash_4kErase(i*SPI020_SECTOR_SIZE);
        flash_msg("_flash_erase_multi_sectors addr = %d done!", i*SPI020_SECTOR_SIZE);
    }
    return KDEV_STATUS_OK;
}

kdev_status_t kdev_flash_erase_chip(void)
{
    kdev_flash_write_control(1);/* send write enabled */

    /* fill in command 0~3 */
    kdrv_spif_set_commands(SPI020_C7_CMD0, SPI020_C7_CMD1, SPI020_C7_CMD2, SPI020_C7_CMD3);
    /* wait for command complete */
    kdrv_spif_check_status_till_ready();
    return KDEV_STATUS_OK;
}

kdev_flash_status_t kdev_flash_get_status(void)
{
    kdev_flash_status_t status;
    uint32_t flash_status;
    
    kdrv_spif_set_commands(SPI020_05_CMD0, SPI020_05_CMD1, SPI020_05_CMD2, SPI020_05_CMD3);
    kdrv_spif_wait_command_complete();
    /* read data */
    flash_status = inw((int8_t  * )SPI020REG_READ_ST);
    *(uint32_t*)&status = flash_status;
    return status;
}

kdev_flash_info_t * kdev_flash_get_info(void)
{
    return &FlashInfo;
}

#endif
