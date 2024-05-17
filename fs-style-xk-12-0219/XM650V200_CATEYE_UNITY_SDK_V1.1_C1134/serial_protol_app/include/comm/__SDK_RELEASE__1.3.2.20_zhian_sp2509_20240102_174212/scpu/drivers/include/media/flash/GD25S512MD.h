

#ifndef __GD25S512MD_H__
#define __GD25S512MD_H__
#include "flash.h"
#if ( FLASH_VENDOR_SELECT == GD25S512MD )



#define GD25D512MD_LOG_EN   (NO)

/*******************************************
 * for die selection command (0xC2)
 ********************************************/
#define DIE_SELEC_C2_CMD0   0x0
#define DIE_SELEC_C2_CMD1   0x01000001
#define DIE_SELEC_C2_CMD2   0x00
#define DIE_SELEC_C2_CMD3   (0xC2000002)

//#define WRITE_STAT_11_CMD0      0x0
//#define WRITE_STAT_11_CMD1      0x01000001
//#define WRITE_STAT_11_CMD2      0x00
//#define WRITE_STAT_11_CMD3      (0x11000002)

/*******************************************
 * for read die command (0xF8)
 ********************************************/
#define RDIE_F8_CMD0      0x0
#define RDIE_F8_CMD1      0x01000000
#define RDIE_F8_CMD2      0x01
#define RDIE_F8_CMD3      (0xF8000008)




/*******************************************
 * for status command (0x05)
 ********************************************/
#define STATUS_05_CMD0      0x0
#define STATUS_05_CMD1      0x01000000
#define STATUS_05_CMD2      0x01
#define STATUS_05_CMD3      (0x05000008)

/*******************************************
 * read status command (0x35)
 ********************************************/
#define STATUS_35_CMD0      (0x0)
#define STATUS_35_CMD1      (0x01000000)
#define STATUS_35_CMD2      (0x01)
#define STATUS_35_CMD3      (0x35000008)

/*******************************************
 * for status command (0x15)
 ********************************************/
#define STATUS_15_CMD0      0x0
#define STATUS_15_CMD1      0x01000000
#define STATUS_15_CMD2      0x1
#define STATUS_15_CMD3      (0x15000008)

/*******************************************
 * write control command (0x06)
 ********************************************/
#define WRITE_CON_06_CMD0      0x0
#define WRITE_CON_06_CMD1      0x01000000
#define WRITE_CON_06_CMD2      0x0
#define WRITE_CON_06_CMD3      (0x06000002)

/*******************************************
 * Disable control command (0x06)
 ********************************************/
#define WRITE_CON_04_CMD0      0x0
#define WRITE_CON_04_CMD1      0x01000000
#define WRITE_CON_04_CMD2      0x0
#define WRITE_CON_04_CMD3      (0x04000002)

/*******************************************
 * for write status register (0x31)
 ********************************************/
#define WRITE_STAT_31_CMD0      0x0
#define WRITE_STAT_31_CMD1      0x01000000
#define WRITE_STAT_31_CMD2      0x1
#define WRITE_STAT_31_CMD3      (0x31000002)

/*******************************************
 * for write status register (0x11)
 ********************************************/
#define WRITE_STAT_11_CMD0      0x0
#define WRITE_STAT_11_CMD1      0x01000001
#define WRITE_STAT_11_CMD2      0x00
#define WRITE_STAT_11_CMD3      (0x11000002)

/*******************************************
 * for read chip id command (0x9F)
 ********************************************/
#define PARA_5A_CMD0            0x0C
#define PARA_5A_CMD1            0x01080003
#define PARA_5A_CMD2            0x8
#define PARA_5A_CMD3            (0x5A000008)

/*******************************************
 * for 4Bytes or 3Bytes 4KByte sector erase command (0x21)
 ********************************************/
#define ERASE_4K_21_CMD0      0x0
#define ERASE_4K_21_CMD1      0x01000004
#define ERASE_4K_21_CMD2      0x0
#define ERASE_4K_21_CMD3      (0x21000002)


/*******************************************
 * for block erase command (0x52):32KB Block Erase
 ********************************************/
#define ERASE_32K_52_CMD0      0x0
#define ERASE_32K_52_CMD1      0x01000003
#define ERASE_32K_52_CMD2      0x0
#define ERASE_32K_52_CMD3      (0x52000002)


/*******************************************
 * for 4Bytes or 3Byte 64KByte erase command (0xDC)
 ********************************************/
#define ERASE_64K_DC_CMD0      0x0
#define ERASE_64K_DC_CMD1      0x01000004
#define ERASE_64K_DC_CMD2      0x0
#define ERASE_64K_DC_CMD3      (0xDC000002)

/*******************************************
 * for chip erase command (0xC7):Erase all
 ********************************************/
#define CHIP_ERASE_C7_CMD0      0x0
#define CHIP_ERASE_C7_CMD1      0x01000000
#define CHIP_ERASE_C7_CMD2      0x0
#define CHIP_ERASE_C7_CMD3      (0xC7000002)

/*******************************************
 * set 4Bytes command (0xB7)
 ********************************************/
#define ENTER_4B_ADD_B7_CMD0      0x0
#define ENTER_4B_ADD_B7_CMD1      0x01000000
#define ENTER_4B_ADD_B7_CMD2      0x0
#define ENTER_4B_ADD_B7_CMD3      (0xB7000002)

/*******************************************
 * Exit 4Bytes command (0xE9)
 ********************************************/
#define EXIT_4B_ADD_E9_CMD0      0x0
#define EXIT_4B_ADD_E9_CMD1      0x01000000
#define EXIT_4B_ADD_E9_CMD2      0x0
#define EXIT_4B_ADD_E9_CMD3      (0xE9000002)



/*******************************************
 * for 4Bytes 4xIO read data command device (0xEC)
 ********************************************/
#define QUAD_READ_EC_CMD0       0x0
#define QUAD_READ_EC_CMD1       0x01060004                    //6 dummy cycle, because in quad mode 1Byte use 2 clock so that 3 dummy need 6 dummy
#define QUAD_READ_EC_CMD2       0x0
#define QUAD_READ_EC_CMD3       (0xEC000080)



/*******************************************
 * for Quad page write command (0x34)
 ********************************************/
#define QPAGE_WRITE_34_CMD0      0x0
#define QPAGE_WRITE_34_CMD1      0x01000004
#define QPAGE_WRITE_34_CMD2      0x0
#define QPAGE_WRITE_34_CMD3      (0x34000042)


/*******************************************
 * for 4Bytes page write command (0x12)
 ********************************************/
#define PAGE_WRITE_12_CMD0      0x0
#define PAGE_WRITE_12_CMD1      0x01000004
#define PAGE_WRITE_12_CMD2      0x0
#define PAGE_WRITE_12_CMD3      (0x12000002|SPI020_CE_VALUE|SPI020_INTR_CFG)


/*******************************************
 * for read chip id command (0x9F)
 ********************************************/
#define RDID_9F_CMD0      0x0
#define RDID_9F_CMD1      0x01000000
#define RDID_9F_CMD2      0x3
#define RDID_9F_CMD3      (0x9F000000|SPI020_CE_VALUE|SPI020_INTR_CFG)


//enable reset
#define RESET_66_CMD0      0x0
#define RESET_66_CMD1      0x01000000
#define RESET_66_CMD2      0x00
#define RESET_66_CMD3      (0x66000002)

// reset device
#define RESET_99_CMD0      0x0
#define RESET_99_CMD1      0x01000000
#define RESET_99_CMD2      0x00
#define RESET_99_CMD3      (0x99000002)



//---flash information----
#define     FLASH_SIGNATURE             (0x50444653)
#define     FLASH_PAGE_SIZE_256_CODE    (0x8)
#define     FLASH_PAGE_SIZE_X_CODE      (0xFF)





extern void nor_flash_die_selection(UINT8 nidx);
extern UINT8 nor_flash_get_active_die(void);
extern UINT8 norflash_busy_check(void);
extern void nor_flash_quad_mode_en(UINT8 enable);
extern UINT8 norflash_get_info(void);
extern kdp_status_t norflash_4k_erase(UINT32 address);
extern kdp_status_t norflash_32k_erase(UINT32 offset);
extern kdp_status_t norflash_64k_erase(UINT32 offset);
extern kdp_status_t norflash_chip_erase(void);
extern kdp_status_t norflash_read(uint32_t addr, void *data, uint32_t target_Bytes);
extern kdp_status_t norflash_program( UINT32 addr, UINT8 *data, UINT32 send_bytes);
extern kdp_status_t norflash_erase_multi_sector(UINT32 nstart_add,  UINT32 nend_add);
extern struct kdp_dev_flash flash_vendor;

extern void kdp520_flash_check_status_til_ready(void);
extern void kdp520_flash_dma_read_stop(void);
extern void kdp520_flash_dma_write_stop(void);
extern void kdp520_flash_quad_enable(UINT8 enable);



#endif
#endif  //end of #if (FLASH_VENDOR_SELECT == GD25S512MD )
