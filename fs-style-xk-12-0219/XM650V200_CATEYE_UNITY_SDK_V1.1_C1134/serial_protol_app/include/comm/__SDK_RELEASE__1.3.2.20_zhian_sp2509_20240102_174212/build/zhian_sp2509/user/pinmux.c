#include "board_kl520.h"
#include "pinmux.h"
#include "scu_extreg.h"
#include "kdp520_gpio.h"
#include "kdp520_lcm.h"


void pinmux_uart2_set(IO_STATUS eIO, BOOL bValue)
{
    if ( eIO == IO_NORMAL )
    {
        PINMUX_LC_DATA6_SET(PINMUX_LC_DATA6_UART2_TX);
        PINMUX_LC_DATA7_SET(PINMUX_LC_DATA7_UART2_RX);
        SCU_EXTREG_LC_DATA6_SET_dcsr(0);
        SCU_EXTREG_LC_DATA7_SET_dcsr(0);
    }
    else
    {
        kdp520_gpio_setmode(17);
        kdp520_gpio_setmode(18);

        if ( eIO == IO_INPUT )
        {
            kdp520_gpio_setdir(17, GPIO_DIR_INPUT);
            kdp520_gpio_setdir(18, GPIO_DIR_INPUT);
        }
        else if ( eIO == IO_OUTPUT )
        {
            kdp520_gpio_setdir(17, GPIO_DIR_OUTPUT);
            kdp520_gpio_setdir(18, GPIO_DIR_OUTPUT);
            if ( bValue )
            {
                kdp520_gpio_setdata(1<<17);
                kdp520_gpio_setdata(1<<18);
            }
            else
            {
                kdp520_gpio_cleardata(1<<17);
                kdp520_gpio_cleardata(1<<18);
            }
        }
    }
}

void pinmux_init(void)
{
//#if ( CFG_SENSOR_TYPE == SENSOR_TYPE_GC1054_GC1054 )
    PINMUX_SD_CLK_SET(PINMUX_SD_CLK_I2C1_CLK);
    PINMUX_SD_CMD_SET(PINMUX_SD_CMD_I2C1_DATA);
//#endif

#if ( CFG_PANEL_TYPE != PANEL_NULL )
    PINMUX_SPI_WP_N_SET(0);
    PINMUX_SPI_HOLD_N_SET(0);
    PINMUX_LC_VS_SET(PINMUX_LC_VS_LCM_DB0);
    PINMUX_LC_HS_SET(PINMUX_LC_HS_LCM_DB1);
    PINMUX_LC_DE_SET(PINMUX_LC_DE_LCM_DB2);
    PINMUX_LC_DATA0_SET(PINMUX_LC_DATA0_LCM_DB3);
    PINMUX_LC_DATA1_SET(PINMUX_LC_DATA1_LCM_DB4);
    PINMUX_LC_DATA2_SET(PINMUX_LC_DATA2_LCM_DB5);
    PINMUX_LC_DATA3_SET(PINMUX_LC_DATA3_LCM_DB6);
    PINMUX_LC_DATA4_SET(PINMUX_LC_DATA4_LCM_DB7);

    PINMUX_LC_DATA5_SET(PINMUX_LC_DATA5_GPIO16);//TE PIN
    PINMUX_LC_DATA15_SET(PINMUX_LC_DATA_GPIO4);
    lcm_custom_pinmux param = {
        .pin_te = GPIO_16,
        .pin_cs = GPIO_4,
        .pin_rs = GPIO_NULL,
        .pin_rst = GPIO_NULL,
    };
    kdp520_lcm_set_custom_pinmux(&param);

    pinmux_uart2_set(IO_NORMAL, 0);
    PINMUX_LC_DATA14_SET(PINMUX_LC_DATA14_GPIO3);
 
//    PINMUX_LC_DATA8_SET(PINMUX_LC_DATA8_LCM_DB11);



//    PINMUX_LC_DATA12_SET(PINMUX_LC_DATA12_LCM_DB15);
//    PINMUX_LC_DATA13_SET(PINMUX_LC_DATA13_LCM_DB16);
//    PINMUX_LC_DATA14_SET(PINMUX_LC_DATA14_LCM_DB17);
    PINMUX_SD_CLK_SET(PINMUX_SD_CLK_LCM_WR);
    PINMUX_SD_CMD_SET(PINMUX_SD_CMD_LCM_RS);
    PINMUX_SD_DATA0_SET(PINMUX_SD_DATA0_LCM_RD);
    PINMUX_SD_DATA1_SET(PINMUX_SD_DATA1_LCM_RESETn);
    // PINMUX_SD_DATA2_SET(PINMUX_SD_DATA2_LCM_BLCTRL);
    PINMUX_SD_DATA3_SET(PINMUX_SD_DATA3_LCM_TP_INT1);
#endif
}
