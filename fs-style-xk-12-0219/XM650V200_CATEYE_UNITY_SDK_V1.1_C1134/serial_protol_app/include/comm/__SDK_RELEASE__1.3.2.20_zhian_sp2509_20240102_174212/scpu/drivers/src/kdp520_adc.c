#include "drivers.h"
#ifndef USE_KDRV
#include <stdio.h>
#include <string.h>
//#include "Driver_Common.h"
#include "io.h"
#include "kneron_mozart.h"
//#include "Driver_USART.h"
#include "kdp_uart.h"
#include "kdp520_adc.h"
#include "dbg.h"

#define NUM_TDC         4
#define SCANMODE        SCANMODE_CONT

kdp_adc_resource ADC_Resources = {
    ADC_FTTSC010_0_PA_BASE,
    ADC_FTADCC010_IRQ, 
};

void _kdp520_adc_config(void)
{ 
	struct kdp_adc_regs *regs = (struct kdp_adc_regs *)ADC_Resources.io_base;

	UINT32 val;
	
//set ADC clock pre-scaler control register (offset 0x11C)		
	//outw(regs->prescal, val);	

//set ADC timing parameter register (offset 0x110)		
	//outw(regs->tparam, val);	

//set ADC sampling timing parameter control register (offset 0x114)		
	//outw(regs->smpr, val);		
	
//set ADC scan sequence register (offset 0x120)		
	//outw(regs->sqr, val);			

//set ADC interrupt enable register (offset 0x108)		
	//outw(regs->inten, val);
	
//set ADC control register (offset 0x100)	
		val = 0;
		val = ADC_EN | SWSTART | SCANMODE_CONT | SCAN_NUM(4);

		outw(&regs->ctrl, val);
		//_print_adc_register(&ADC_Resources);
}

//void _api_adc_print_register(kdp_adc_resource *res)
//{
//	UINT32 val;
//	struct kdp_adc_regs *regs = (struct kdp_adc_regs *)res->io_base;
//	
//	val = inw(&regs->prescal);	//0x11C
//	dbg_msg("0x%x = 0x%x", &regs->prescal, val);
//	val = inw(&regs->tparam);	//0x110
//	dbg_msg("0x%x = 0x%x", &regs->tparam, val);
//	val = inw(&regs->smpr);		//0x114
//	dbg_msg("0x%x = 0x%x", &regs->smpr, val);
//	val = inw(&regs->sqr);			//0x120
//	dbg_msg("0x%x = 0x%x", &regs->sqr, val);
//	val = inw(&regs->inten);		//0x108
//	dbg_msg("0x%x = 0x%x", &regs->inten, val);
//	val = inw(&regs->ctrl);		//0x100
//	dbg_msg("0x%x = 0x%x", &regs->ctrl, val);
//}

void kdp520_adc_init()
{
    UINT32 i;
    UINT32 val = 0;

    struct kdp_adc_regs *regs = (struct kdp_adc_regs *)ADC_Resources.io_base;
    
    //Enable ADCCLK in SCU base 0xC2380000
    //Set the 22th bit of the "Clock Enable Register1"(offset = 0x018) to 1
    val = inw(SCU_EXTREG_PA_BASE + 0x018);
    outw(SCU_EXTREG_PA_BASE + 0x018, (val | (1 << 22 )));
    
    osDelay(1);
    _kdp520_adc_config();
    
    // Wait for scanning done.
    for(i = 0; i < NUM_TDC; i++) {
        while(!(readl(&regs->intst) & CH_INTRSTS(i))) {}	
    }

	while(!(readl(&regs->intst) & ADC_DONE_INTSTS));
}

int32_t kdp520_adc_uninit(kdp_adc_resource *res)
{
	return 0;
}

void kdp520_adc_reset(kdp_adc_resource *res)
{
	int val;
	struct kdp_adc_regs *regs = (struct kdp_adc_regs *)res->io_base;
	
	//disable 
	val = readl(&regs->ctrl);
	val &= ~ADC_EN; //clear enable bit 
	writel(val, &regs->ctrl);		
	
	//enable
	val |= ADC_EN; //clear enable bit 
	writel(val, &regs->ctrl);		
	
}

void kdp520_adc_enable(kdp_adc_resource *res, int mode)
{
	int val;
	struct kdp_adc_regs *regs = (struct kdp_adc_regs *)res->io_base;
	
	val = readl(&regs->ctrl);
	val &= ~ADC_EN; //clear enable bit 
	val |= mode;
	writel(val, &regs->ctrl);		
}

int kdp520_adc_read(int id)
{
    UINT32 val = 0;
    struct kdp_adc_regs *regs = (struct kdp_adc_regs *)ADC_Resources.io_base;
       
    val = readl(&regs->data[id]);    
    
    return val;
}

#endif
