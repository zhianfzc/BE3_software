#include "kdp520_gpio.h"	
#include <cmsis_os2.h>
#include "kneron_mozart.h"
#include "board_kl520.h"
#include "io.h"
#include "types.h"
#include "base.h"
#include "pinmux.h"
#include "framework/event.h"
#include "touch.h"
#include "delay.h"
#include "dbg.h"
#include <string.h>

#define GPIO_WAIT_INT_TIMEOUT_CNT   (0x7FFFF)

#define GPIO_BASE    GPIO_FTGPIO010_PA_BASE

volatile UINT32	__gpio_int_occurred = 0;


unsigned int kdp520_gpio_readdata(void)
{
    return inw(GPIO_BASE + GPIO_DIN_OFFSET);
}                         


void kdp520_gpio_writedata(unsigned int data)
{
    outw(GPIO_BASE + GPIO_DOUT_OFFSET, data);
}


// ====================================================================
//	input:
//		dir: 	GPIO_DIR_OUTPUT ==> output
//				GPIO_DIR_INPUT ==> input
// ====================================================================
void kdp520_gpio_setdir(unsigned int pin, unsigned int dir)
{
    unsigned int tmp;
    
     tmp = inw(GPIO_BASE + GPIO_PINOUT_OFFSET);

     if(dir==GPIO_DIR_OUTPUT)
     {
         tmp |= (1<<pin); 
     }
     else
     {
        tmp &= ~(1<<pin);
    }

    outw(GPIO_BASE + GPIO_PINOUT_OFFSET, tmp); 
}


void kdp520_gpio_setbypass(unsigned int pin)
{
    unsigned int tmp;

     tmp = inw(GPIO_BASE + GPIO_PIN_BYPASS);
    tmp |= (1<<pin);
      outw(GPIO_BASE + GPIO_PIN_BYPASS, tmp);
}            


void kdp520_gpio_setdata(unsigned int data)
{
    outw(GPIO_BASE + GPIO_DATASET, data);
}   


void kdp520_gpio_cleardata(unsigned int data)
{
    outw(GPIO_BASE + GPIO_DATACLR, data);	
}


void  kdp520_gpio_pullenable(unsigned int pin)
{	
    unsigned int tmp;

     tmp = inw(GPIO_BASE + GPIO_PULLENABLE);
    tmp |= (1<<pin);
      outw(GPIO_BASE + GPIO_PULLENABLE, tmp);
}   

void  kdp520_gpio_pulldisable(unsigned int pin)
{	
    unsigned int tmp;

     tmp = inw(GPIO_BASE + GPIO_PULLENABLE);
    tmp &= ~(1<<pin);
      outw(GPIO_BASE + GPIO_PULLENABLE, tmp);
}  
   
void  kdp520_gpio_pullhigh(unsigned int pin)
{
    unsigned int tmp;

     tmp = inw(GPIO_BASE + GPIO_PULLTYPE);
    tmp |= (1<<pin);
      outw(GPIO_BASE + GPIO_PULLTYPE, tmp);
}   
   
void  kdp520_gpio_pulllow(unsigned int pin)
{	
    unsigned int tmp;

     tmp = inw(GPIO_BASE + GPIO_PULLTYPE);
    tmp &= ~(1<<pin);
      outw(GPIO_BASE + GPIO_PULLTYPE, tmp);
}  
   
void  kdp520_gpio_setintenable(unsigned int pin)
{
    unsigned int tmp;

     tmp = inw(GPIO_BASE + GPIO_INT_ENABLE);
    tmp |= (1<<pin);
      outw(GPIO_BASE + GPIO_INT_ENABLE, tmp);
}                        


void  kdp520_gpio_setintdisable(unsigned int pin)
{
    unsigned int tmp;

     tmp = inw(GPIO_BASE + GPIO_INT_ENABLE);
    tmp &= ~(1<<pin);
      outw(GPIO_BASE + GPIO_INT_ENABLE, tmp);
}  

BOOL  kdp520_gpio_israwint(unsigned int pin)
{
    unsigned int tmp;

     tmp = inw(GPIO_BASE + GPIO_INT_RAWSTATE);

      if((tmp & (1<<pin)) != 0)
          return TRUE;
      else
          return FALSE;
}   

void  kdp520_gpio_setintmask(unsigned int pin)
{
    unsigned int tmp;

     tmp = inw(GPIO_BASE + GPIO_INT_MASK);
    tmp |= 1<<pin;
      outw(GPIO_BASE + GPIO_INT_MASK, tmp);
}                        


void  kdp520_gpio_setintunmask(unsigned int pin)
{
    unsigned int tmp;

     tmp = inw(GPIO_BASE + GPIO_INT_MASK);
    tmp &= ~(1<<pin);
      outw(GPIO_BASE + GPIO_INT_MASK, tmp);
}


unsigned int kdp520_gpio_getintmaskstatus(void)
{
     return inw(GPIO_BASE + GPIO_INT_MASKSTATE);	
}


void kdp520_gpio_clearint(unsigned int data)
{
      outw(GPIO_BASE + GPIO_INT_CLEAR, data);
}


void kdp520_gpio_settrigger(unsigned int pin, unsigned int trigger)
{
    unsigned int tmp=0, tmp1=0;

     tmp = inw(GPIO_BASE + GPIO_INT_TRIGGER);

      if (trigger)
      {
          tmp |= (1<<pin);
          outw(GPIO_BASE + GPIO_INT_TRIGGER, tmp);
      }
      else
      {
          tmp1 |= (1<<pin);
          tmp &= ~tmp1;
          outw(GPIO_BASE + GPIO_INT_TRIGGER, tmp);  	    
      }
}


void kdp520_gpio_setedgemode(unsigned int pin, unsigned int both)
{
    unsigned int tmp,tmp1=0;

     tmp = inw(GPIO_BASE + GPIO_INT_BOTH);

      if (both)
      {
          tmp |= (1<<pin);
          outw(GPIO_BASE + GPIO_INT_BOTH, tmp);
      }
      else
      {
          tmp1 |= (1<<pin);
          tmp &= ~tmp1;
          outw(GPIO_BASE + GPIO_INT_BOTH, tmp);
      }
      
}

void kdp520_gpio_setactivemode(unsigned int pin, unsigned int active)
{
    unsigned int tmp, tmp1=0;

     tmp = inw(GPIO_BASE + GPIO_INT_RISENEG);

      if (active)
      {
          tmp |= (1<<pin);
          outw(GPIO_BASE + GPIO_INT_RISENEG, tmp);
      }
      else
      {
          tmp1 |= (1<<pin);
          tmp &= ~tmp1;
          outw(GPIO_BASE + GPIO_INT_RISENEG, tmp);  	    
      }
}


void kdp520_gpio_enableint(unsigned int pin, unsigned int trigger, unsigned int active)
{
     kdp520_gpio_setintenable(pin);
     kdp520_gpio_setintunmask(pin);
     kdp520_gpio_settrigger(pin, trigger);	
     kdp520_gpio_setactivemode(pin, active);		
}

void kdp520_gpio_disableint(unsigned int pin)
{
     kdp520_gpio_setintdisable(pin);
     kdp520_gpio_setintmask(pin); 	
}

void kdp520_set_int_occurred(unsigned int occurred)
{
    __gpio_int_occurred |= occurred;
}

BOOL kdp520_get_int_occurred(unsigned int pin)
{
    if (__gpio_int_occurred & (1 << pin))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void kdp520_clear_int_occurred(unsigned int pin)
{
    __gpio_int_occurred &= ~(1 << pin);
}

void kdp520_wait_gpio_int(unsigned int pin)
{	
    u32 cnt = 0;
    
    while(!(__gpio_int_occurred & (pin)))
    {
      //__WFE (); 								   // Power-Down until next Event/Interrupt
        delay_us(1);
        
        if (GPIO_WAIT_INT_TIMEOUT_CNT <= cnt)
        {
            dbg_msg("kdp520_wait_gpio_int TIMEOUT:%d Pin:%d\n", cnt, pin);
            break;
        }
        cnt++;
    }	
    
    __gpio_int_occurred &= ~(pin);
}

void kdp520_gpio_enablebounce(unsigned int pin, unsigned int clkdiv)
{
    unsigned int tmp;

     tmp = inw(GPIO_BASE + GPIO_INT_BOUNCEENABLE);
     tmp |= (1<<pin);
    outw(GPIO_BASE + GPIO_INT_BOUNCEENABLE, tmp);
    
     outw(GPIO_BASE + GPIO_INT_PRESCALE, clkdiv); 	
}

void kdp520_gpio_disablebounce(unsigned int pin)
{
    unsigned int tmp;

     tmp = inw(GPIO_BASE + GPIO_INT_BOUNCEENABLE);
     tmp &= ~(1<<pin);
     outw(GPIO_BASE + GPIO_INT_BOUNCEENABLE, tmp);
}

void kdp520_gpio_setmode(unsigned int pin)
{
    switch(pin)
    {
        case 0:
            PINMUX_SPI_WP_N_SET(PINMUX_SPI_WP_N_GPIO0);
            break;
        case 1:
            PINMUX_SPI_HOLD_N_SET(PINMUX_SPI_HOLD_N_GPIO1);
            break;
        case 2:
            PINMUX_SWJ_TRST_SET(PINMUX_SWJ_TRST_GPIO2);
            break;
        case 3:
            PINMUX_SWJ_TDI_SET(PINMUX_SWJ_TDI_GPIO3);
            break;
        case 4:
            PINMUX_SWJ_SWDITMS_SET(PINMUX_SWJ_SWDITMS_GPIO4);
            break;
        case 5:
            PINMUX_SWJ_SWCLKTCK_SET(PINMUX_SWJ_SWCLKTCK_GPIO5);
            break;
        case 6:
            PINMUX_SWJ_TDO_SET(PINMUX_SWJ_TDO_GPIO6);
            break;
        case 7:
            PINMUX_LC_PCLK_SET(PINMUX_LC_PCLK_GPIO7);
            break;
        case 8:
            PINMUX_LC_VS_SET(PINMUX_LC_VS_GPIO8);
            break;
        case 9:
            PINMUX_LC_HS_SET(PINMUX_LC_HS_GPIO9);
            break;
        case 10:
            PINMUX_LC_DE_SET(PINMUX_LC_DE_GPIO10);
            break;
        case 11:
            PINMUX_LC_DATA0_SET(PINMUX_LC_DATA0_GPIO11);
            break;
        case 12:
            PINMUX_LC_DATA1_SET(PINMUX_LC_DATA1_GPIO12);
            break;
        case 13:
            PINMUX_LC_DATA2_SET(PINMUX_LC_DATA2_GPIO13);
            break;
        case 14:
            PINMUX_LC_DATA3_SET(PINMUX_LC_DATA3_GPIO14);
            break;
        case 15:
            PINMUX_LC_DATA4_SET(PINMUX_LC_DATA4_GPIO15);
            break;
        case 16:
            PINMUX_LC_DATA5_SET(PINMUX_LC_DATA5_GPIO16);
            break;
        case 17:
            PINMUX_LC_DATA6_SET(PINMUX_LC_DATA6_GPIO17);
            break;
        case 18:
            PINMUX_LC_DATA7_SET(PINMUX_LC_DATA7_GPIO18);
            break;
        case 19:
            PINMUX_LC_DATA8_SET(PINMUX_LC_DATA8_GPIO19);
            break;
        case 20:
            PINMUX_LC_DATA9_SET(PINMUX_LC_DATA9_GPIO20);
            break;
        case 21:
            PINMUX_LC_DATA10_SET(PINMUX_LC_DATA10_GPIO21);
            break;
        case 22:
            PINMUX_SD_CLK_SET(PINMUX_SD_CLK_GPIO22);
            break;
        case 23:
            PINMUX_SD_CMD_SET(PINMUX_SD_CMD_GPIO23);
            break;
        case 24:
            PINMUX_SD_DATA0_SET(PINMUX_SD_DATA0_GPIO24);
            break;
        case 25:
            PINMUX_SD_DATA1_SET(PINMUX_SD_DATA1_GPIO25);
             break;
        case 26:
            PINMUX_SD_DATA2_SET(PINMUX_SD_DATA2_GPIO26);
            break;
        case 27:
            PINMUX_SD_DATA3_SET(PINMUX_SD_DATA3_GPIO27);
            break;
        case 28:
            PINMUX_UART0_TX_SET(PINMUX_UART0_TX_GPIO28);
            break;
        case 29:
            PINMUX_I2C0_CLK_SET(PINMUX_I2C0_CLK_GPIO29);
            break;
        case 30:
            PINMUX_I2C0_DATA_SET(PINMUX_I2C0_DATA_GPIO30);
            break;
        case 31:
            PINMUX_PWM0_SET(PINMUX_PWM0_GPIO31);
            break;
    }   
}

extern void kdp520_gpio_set_dcsr_param(kdp520_gpio_custom_dcsr_param* dst_param, kdp520_gpio_custom_dcsr_param* src_param) {
    memcpy(dst_param, src_param, sizeof(kdp520_gpio_custom_dcsr_param));
}

void GPIO010_IRQHandler(void)
{
    UINT32 IntrMaskedStat;

    IntrMaskedStat = kdp520_gpio_getintmaskstatus();
    //clear CPIO interrupt
    kdp520_gpio_clearint(IntrMaskedStat);
    //__gpio_int_occurred |= IntrMaskedStat;
    kdp520_set_int_occurred(IntrMaskedStat);
}
