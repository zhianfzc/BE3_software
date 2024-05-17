#ifndef __SPI020_H__
#define __SPI020_H__

//#include "KNERONCM4.h"
#include "kneron_mozart.h"

/*******************************************
 * for constant definition
 ********************************************/
#define FLASH_64K               0x10000
#define FLASH_PAGE_SIZE         256
#define FLASH_NORMAL            0x00
#define FLASH_DTR_RW            0x01
#define FLASH_DUAL_READ         0x02
#define FLASH_DMA_READ          0x04
#define FLASH_DMA_WRITE         0x08
#define FLASH_IO_RW             0x10
#define FLASH_BYTE_MODE         0x20
#define FLASH_QUAD_RW           0x40
#define FLASH_FAST_READ         0x80 //Winbond W25P16
#define FLASH_OP_MODE           FLASH_NORMAL//FLASH_QUAD_RW//(FLASH_QUAD_RW|FLASH_IO_RW)

#define FLASH_CTL_SPI010        0x01
#define FLASH_CTL_SPI020        0x02
// #define FLASH_4BYTES_CMD_EN     0x01        

typedef enum
{
    SPI_CLK_MODE0=0,
    SPI_CLK_MODE3=0x10, 
}spi_clk_mode_t;




/*******************************************
 * for operation define definition
 ********************************************/
#define SPI_CLK_DIVIDER_2   0x00    
#define SPI_CLK_DIVIDER_4   0x01
#define SPI_CLK_DIVIDER_6   0x02
#define SPI_CLK_DIVIDER_8   0x03


#define SPI020_CE_0         0x0000
#define SPI020_CE_1         0x0100
#define SPI020_CE_2         0x0200
#define SPI020_CE_3         0x0300
#define SPI020_CE_VALUE     SPI020_CE_0
#define SPI020_INTR_CFG     0x00000000


/*******************************************
 * for register address definition
 ********************************************/
#define SPI020REG_CMD0      (SPI_FTSPI020_PA_BASE+0x00)
#define SPI020REG_CMD1      (SPI_FTSPI020_PA_BASE+0x04)
#define SPI020REG_CMD2      (SPI_FTSPI020_PA_BASE+0x08)
#define SPI020REG_CMD3      (SPI_FTSPI020_PA_BASE+0x0C)
#define SPI020REG_CONTROL   (SPI_FTSPI020_PA_BASE+0x10)
#define SPI020REG_ACTIMER   (SPI_FTSPI020_PA_BASE+0x14)
#define SPI020REG_STATUS    (SPI_FTSPI020_PA_BASE+0x18)
#define SPI020REG_INTERRUPT (SPI_FTSPI020_PA_BASE+0x20)
#define SPI020REG_INTR_ST   (SPI_FTSPI020_PA_BASE+0x24)
#define SPI020REG_READ_ST   (SPI_FTSPI020_PA_BASE+0x28)
#define SPI020REG_VERSION   (SPI_FTSPI020_PA_BASE+0x50)
#define SPI020REG_FEATURE   (SPI_FTSPI020_PA_BASE+0x54)
#define SPI020REG_SCKINDLY  (SPI_FTSPI020_PA_BASE+0x58)
#define SPI020REG_DATAPORT  (SPI_FTSPI020_PA_BASE+0x100)

/*******************************************
 * for register bits definition
 ********************************************/
/* for SPI020REG_CONTROL */
#define SPI020_ABORT        BIT8
#define SPI020_CLK_MODE     BIT4
#define SPI020_CLK_DIVIDER  (BIT0|BIT1)
/* for SPI020REG_STATUS */
#define SPI020_RXFIFO_READY BIT1
#define SPI020_TXFIFO_READY BIT0
/* for SPI020REG_INTERRUPT */
#define SPI020_cmd_cmplt_intr_en BIT1
#define SPI020_DMA_EN       BIT0
/* for SPI020REG_INTR_ST */
#define SPI020_CMD_CMPLT    BIT0
/* for SPI020REG_FEATURE */
#define SPI020_RX_DEPTH     0xFF00
#define SPI020_TX_DEPTH     0x00FF


#if 1   //FLASH_4BYTES_CMD_EN
/*******************************************
 * set 4Bytes command (0xB7)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_B7_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_B7_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_B7_CMD2      0x0
/* Set command word 4, instrction code = 0x06, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_B7_CMD3      (0xB7000002|SPI020_CE_VALUE|SPI020_INTR_CFG)



/*******************************************
 * for 4Bytes read data command (0x13)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_13_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 4 */
#define SPI020_13_CMD1      0x01000004
/* Set command word 2, set data count by input parameter */
#define SPI020_13_CMD2      0x0
/* Set command word 4, instrction code = 0x13, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_13_CMD3      (0x13000000|SPI020_CE_VALUE|SPI020_INTR_CFG)


/*******************************************
 * for 4Bytes read data command (0x0C)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_0C_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 4 */
#define SPI020_0C_CMD1      0x01080004  //bessel:change value from 0x01000004 to 0x01080004(Fast Read instruction need to add eight "dummy"clocks after 24-bit address) 
/* Set command word 2, set data count by input parameter */
#define SPI020_0C_CMD2      0x0
/* Set command word 4, instrction code = 0x0C, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_0C_CMD3      (0x0C000000|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for 4Bytes page write command (0x12)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_12_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 4 */
#define SPI020_12_CMD1      0x01000004
/* Set command word 2, set data count by input parameter */
#define SPI020_12_CMD2      0x0
/* Set command word 4, instrction code = 0x12, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_12_CMD3      (0x12000002|SPI020_CE_VALUE|SPI020_INTR_CFG)


/*******************************************
 * for 4Bytes Quad page write command (0x34)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_34_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 4 */
#define SPI020_34_CMD1      0x01000004
/* Set command word 2, set data count by input parameter */
#define SPI020_34_CMD2      0x0
/* Set command word 4, instrction code = 0x34, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_34_CMD3      (0x34000042|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for 4Bytes sector erase command (0x21):Sector Erase(4K)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_21_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 4 */
#define SPI020_21_CMD1      0x01000004
/* Set command word 2, set data count to 0 */
#define SPI020_21_CMD2      0x0
/* Set command word 4, instrction code = 0x21, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_21_CMD3      (0x21000002|SPI020_CE_VALUE|SPI020_INTR_CFG)


/*******************************************
 * for 4Bytes block erase command (0xDC):64KB Block Erase
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_DC_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 4 */
#define SPI020_DC_CMD1      0x01000004
/* Set command word 2, set data count to 0 */
#define SPI020_DC_CMD2      0x0
/* Set command word 4, instrction code = 0xDC, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_DC_CMD3      (0xDC000002|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for 4Bytes quad read data command for windond device (0x6C)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_6C_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 8, dum_1st_cyc = 0, address length = 4 */
#define SPI020_6C_CMD1      0x01080004
/* Set command word 2, set data count by input parameter */
#define SPI020_6C_CMD2      0x0
/* Set command word 4, instrction code = 0x6C, contiune read = 0,
   start_ce = ??, spi mode = 2, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_6C_CMD3      (0x6C000040|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for 4Bytes 2xIO read data command for MXIC device (0xBC)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_BC_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 4, dum_1st_cyc = 0, address length = 4 */
#define SPI020_BC_CMD1      0x01040004
/* Set command word 2, set data count by input parameter */
#define SPI020_BC_CMD2      0x0
/* Set command word 4, instrction code = 0xBC, contiune read = 0,
   start_ce = ??, spi mode = 3, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_BC_CMD3      (0xBC000060|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for 4Bytes 4xIO read data command device (0xEC)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_EC_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 6, dum_1st_cyc = 0, address length = 4 */
#define SPI020_EC_CMD1    0x01060004
/* Set command word 2, set data count by input parameter */
#define SPI020_EC_CMD2      0x0
/* Set command word 4, instrction code = 0xEB, contiune read = 0,
   start_ce = ??, spi mode = 4, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_EC_CMD3      (0xEC000080|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for 4Bytes dual read data command for windond device (0x3C)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_3C_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 8, dum_1st_cyc = 0, address length = 3 */
#define SPI020_3C_CMD1    0x01080004
/* Set command word 2, set data count by input parameter */
#define SPI020_3C_CMD2      0x0
/* Set command word 4, instrction code = 0x3B, contiune read = 0,
   start_ce = ??, spi mode = 1, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_3C_CMD3      (0x3C000020|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************************
 * for Quad page write command with 4byte address (0x34)
 ********************************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_34_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_34_CMD1      0x01000004
/* Set command word 2, set data count by input parameter */
#define SPI020_34_CMD2      0x0
/* Set command word 4, instrction code = 0x32, contiune read = 0,
   start_ce = ??, spi mode = 2, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_34_CMD3      (0x34000042|SPI020_CE_VALUE|SPI020_INTR_CFG)

#endif // end of, #if FLASH_4BYTES_CMD_EN


/*******************************************
 * Exit 4Bytes command (0xE9)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_E9_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_E9_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_E9_CMD2      0x0
/* Set command word 4, instrction code = 0x06, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_E9_CMD3      (0xE9000002|SPI020_CE_VALUE|SPI020_INTR_CFG)



/*******************************************
 * for read chip id command (0x9F)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_9F_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_9F_CMD1      0x01020000
/* Set command word 2, set data count to 3 */
#define SPI020_9F_CMD2      0x3
/* Set command word 4, instrction code = 0x9F, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_9F_CMD3      (0x9F000000|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for read chip id command (0xAB)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_AB_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_AB_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_AB_CMD2      0x0
/* Set command word 4, instrction code = 0xAB, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_AB_CMD3      (0xAB000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for read chip id command (0xB9)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_B9_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_B9_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_B9_CMD2      0x0
/* Set command word 4, instrction code = 0xAB, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_B9_CMD3      (0xB9000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for status command (0x05)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_05_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_05_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_05_CMD2      0x0
/* Set command word 4, instrction code = 0x05, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 1, write enable = 0, intr_en = ? */
#define SPI020_05_CMD3      (0x05000004|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for status command (0x05)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_05_CMD0_w      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_05_CMD1_w      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_05_CMD2_w      0x1
/* Set command word 4, instrction code = 0x05, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 1, write enable = 0, intr_en = ? */
#define SPI020_05_CMD3_w      (0x05000008|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for status command (0x35)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_35_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_35_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_35_CMD2      0x1
/* Set command word 4, instrction code = 0x05, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 1, write enable = 0, intr_en = ? */
/* shantai, 2014/5/2. 0x35 command can not use read status by hardware. (i.e. bit3 can not be 1) */
#define SPI020_35_CMD3      (0x35000008|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for read status register 3 command (0x15)
 ********************************************/
#define SPI020_15_CMD0      0x0
#define SPI020_15_CMD1      0x01000000
#define SPI020_15_CMD2      0x1
#define SPI020_15_CMD3      (0x15000008|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for write status register 3 command (0x11)
 ********************************************/
#define SPI020_11_CMD0      0x0
#define SPI020_11_CMD1      0x01000000
#define SPI020_11_CMD2      0x1
#define SPI020_11_CMD3      (0x11000002|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for write status register 3 command (0x31)
 ********************************************/
#define SPI020_31_CMD0      0x0
#define SPI020_31_CMD1      0x01000000
#define SPI020_31_CMD2      0x1
#define SPI020_31_CMD3      (0x31000002|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for read data command (0x03)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_03_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_03_CMD1      0x01000003
/* Set command word 2, set data count by input parameter */
#define SPI020_03_CMD2      0x0
/* Set command word 4, instrction code = 0x03, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_03_CMD3      (0x03000000|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for read data command (0x0B)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_0B_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_0B_CMD1      0x01080003  //bessel:change value from 0x01000003 to 0x01080003(Fast Read instruction need to add eight "dummy"clocks after 24-bit address) 
/* Set command word 2, set data count by input parameter */
#define SPI020_0B_CMD2      0x0
/* Set command word 4, instrction code = 0x03, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_0B_CMD3      (0x0B000000|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for Fast DTR read data command (0x0D)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_0D_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 6, dum_1st_cyc = 0, address length = 3 */
#define SPI020_0D_CMD1      0x01060003
/* Set command word 2, set data count by input parameter */
#define SPI020_0D_CMD2      0x0
/* Set command word 4, instrction code = 0x0D, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 1, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_0D_CMD3      (0x0D000010|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for Dual IO DTR read data command (0xBD)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_BD_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 6, dum_1st_cyc = 0, address length = 3 */
#define SPI020_BD_CMD1      0x01080003
/* Set command word 2, set data count by input parameter */
#define SPI020_BD_CMD2      0x0
/* Set command word 4, instrction code = 0xBD, contiune read = 0,
   start_ce = ??, spi mode = 3, DTR mode = 1, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_BD_CMD3      (0xBD000070|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for Quad IO DTR read data command (0xED)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_ED_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 6, dum_1st_cyc = 0, address length = 3 */
#define SPI020_ED_CMD1      0x01080003
/* Set command word 2, set data count by input parameter */
#define SPI020_ED_CMD2      0x0
/* Set command word 4, instrction code = 0x0D, contiune read = 0,
   start_ce = ??, spi mode = 4, DTR mode = 1, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_ED_CMD3      (0xED000090|SPI020_CE_VALUE|SPI020_INTR_CFG)




/*******************************************
 * for GD Clear SR Flags command (0x30)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_30_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_30_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_30_CMD2      0x0
/* Set command word 4, instrction code = 0x30, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_30_CMD3      (0x30000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for write enable command (0x06)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_06_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_06_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_06_CMD2      0x0
/* Set command word 4, instrction code = 0x06, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_06_CMD3      (0x06000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for write disable command (0x04)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_04_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_04_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_04_CMD2      0x0
/* Set command word 4, instrction code = 0x04, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_04_CMD3      (0x04000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for page write command (0x02)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_02_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_02_CMD1      0x01000003
/* Set command word 2, set data count by input parameter */
#define SPI020_02_CMD2      0x0
/* Set command word 4, instrction code = 0x02, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_02_CMD3      (0x02000002|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for Quad page write command (0x32)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_32_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_32_CMD1      0x01000003
/* Set command word 2, set data count by input parameter */
#define SPI020_32_CMD2      0x0
/* Set command word 4, instrction code = 0x32, contiune read = 0,
   start_ce = ??, spi mode = 2, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_32_CMD3      (0x32000042|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for chip erase command (0xC7):Erase all
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_C7_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_C7_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_C7_CMD2      0x0
/* Set command word 4, instrction code = 0xC7, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_C7_CMD3      (0xC7000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for chip erase command (0x60):Erase all
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_60_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_60_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_60_CMD2      0x0
/* Set command word 4, instrction code = 0x60, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_60_CMD3      (0x60000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for block erase command (0xD8):64KB Block Erase
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_D8_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_D8_CMD1      0x01000003
/* Set command word 2, set data count to 0 */
#define SPI020_D8_CMD2      0x0
/* Set command word 4, instrction code = 0xD8, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_D8_CMD3      (0xD8000002|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for block erase command (0x52):32KB Block Erase
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_52_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
 dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_52_CMD1      0x01000003
/* Set command word 2, set data count to 0 */
#define SPI020_52_CMD2      0x0
/* Set command word 4, instrction code = 0xD8, contiune read = 0,
 start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
 status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_52_CMD3      (0x52000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for sector erase command (0x20):Sector Erase(4K)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_20_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_20_CMD1      0x01000003
/* Set command word 2, set data count to 0 */
#define SPI020_20_CMD2      0x0
/* Set command word 4, instrction code = 0x20, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_20_CMD3      (0x20000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for dual read data command for windond device (0x3B)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_3B_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 8, dum_1st_cyc = 0, address length = 3 */
#define SPI020_3B_CMD1    0x01080003
/* Set command word 2, set data count by input parameter */
#define SPI020_3B_CMD2      0x0
/* Set command word 4, instrction code = 0x3B, contiune read = 0,
   start_ce = ??, spi mode = 1, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_3B_CMD3      (0x3B000020|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for quad read data command for windond device (0x6B)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_6B_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 8, dum_1st_cyc = 0, address length = 3 */
#define SPI020_6B_CMD1    0x01080003
/* Set command word 2, set data count by input parameter */
#define SPI020_6B_CMD2      0x0
/* Set command word 4, instrction code = 0x3B, contiune read = 0,
   start_ce = ??, spi mode = 2, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_6B_CMD3      (0x6B000040|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for 2xIO read data command for MXIC device (0xBB)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_BB_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 4, dum_1st_cyc = 0, address length = 3 */
#define SPI020_BB_CMD1    0x01040003
/* Set command word 2, set data count by input parameter */
#define SPI020_BB_CMD2      0x0
/* Set command word 4, instrction code = 0xBB, contiune read = 0,
   start_ce = ??, spi mode = 3, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_BB_CMD3      (0xBB000060|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for 4xIO read data command device (0xEB)
 ********************************************/
/* Set command word 0, set SPI flash address by input parameter */
#define SPI020_EB_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 6, dum_1st_cyc = 0, address length = 3 */
#define SPI020_EB_CMD1    0x01060003
/* Set command word 2, set data count by input parameter */
#define SPI020_EB_CMD2      0x0
/* Set command word 4, instrction code = 0xEB, contiune read = 0,
   start_ce = ??, spi mode = 4, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_EB_CMD3      (0xEB000080|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for EWSR (enable-write-status-register) command (0x50)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_50_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_50_CMD1      0x01000000
/* Set command word 2, set data count to 0 */
#define SPI020_50_CMD2      0x0
/* Set command word 4, instrction code = 0x50, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_50_CMD3      (0x50000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for WRSR (write-status-register) command (0x01)
 ********************************************/
/* Set command word 0, set SPI flash address to 0 */
#define SPI020_01_CMD0      0x0
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 0 */
#define SPI020_01_CMD1      0x01000000
/* Set command word 2, set data count to 1 */
#define SPI020_01_CMD2      0x2
/* Set command word 4, instrction code = 0x01, contiune read = 0,
   start_ce = ??, spi mode = 0, DTR mode = 0, status = 0,
   status_en = 0, write enable = 1, intr_en = ? */
#define SPI020_01_CMD3      (0x01000002|SPI020_CE_VALUE|SPI020_INTR_CFG)
/*******************************************
 * for REMS (read electronic manufacturer & device ID) command (0x90)
 ********************************************/
/* Set command word 0x000001, output the manufacturer ID first, the second byte is device ID */
#define SPI020_90_CMD0      0x0 //0x01 -> 0x00
/* Set command word 1, continue read = 0, instruction leng = 1,
   dum_2nd_cyc = 0, dum_1st_cyc = 0, address length = 3 */
#define SPI020_90_CMD1      0x01000003
/* Set command word 2, set data count by input parameter */
#define SPI020_90_CMD2      0x02 //0x4 //0x04 -> 0x02
/* Set command word 4, instrction code = 0x3B, contiune read = 0,
   start_ce = ??, spi mode = 1, DTR mode = 0, status = 0,
   status_en = 0, write enable = 0, intr_en = ? */
#define SPI020_90_CMD3      (0x90000000|SPI020_CE_VALUE|SPI020_INTR_CFG)


/*******************************************
 * 0x66 reset enable
 ********************************************/

//enable reset
#define SPI020_66_CMD0      0x0 //0x01 -> 0x00
#define SPI020_66_CMD1      0x01000000
#define SPI020_66_CMD2      0x00
#define SPI020_66_CMD3      (0x66000002|SPI020_CE_VALUE|SPI020_INTR_CFG)


#define SPI020_66_CMD0_ORG      0x0 //0x01 -> 0x00
#define SPI020_66_CMD1_ORG      0x01000000
#define SPI020_66_CMD2_ORG      0x00
#define SPI020_66_CMD3_ORG      (0x66000000|SPI020_CE_VALUE|SPI020_INTR_CFG)

// reset device
#define SPI020_99_CMD0      0x0
#define SPI020_99_CMD1      0x01000000
#define SPI020_99_CMD2      0x00
#define SPI020_99_CMD3      (0x99000002|SPI020_CE_VALUE|SPI020_INTR_CFG)

/*******************************************
 * for structure definition
 ********************************************/
/* For system information. Be careful, this must be 4 alignment */
typedef struct {
    uint8_t   reserved;       /* original -> FLASH_CTL_SPI010/FLASH_CTL_SPI020 - for spi010/spi020 */
    uint8_t   manufacturer;   /* Manufacturer id */
    uint16_t  flash_id;       /* device id */
    uint32_t  flash_size;     /* flash size in byte */
    uint8_t   support_dual;   /* flash support dual read mode or not */
    uint8_t   sys_version;    /* system version, get from SYS_VERSION_ADDR */
    uint8_t   dev_mode;       /* current usb link type, 0/1/2/3 for unknow/SS/HS/FS */
    uint8_t   vender_specific; /* specific purpose for vendor */
} spi_flash_t;

//--------------------------------------------------
// Function Prototypes
//--------------------------------------------------

#endif/* __SPI020_H__ */

