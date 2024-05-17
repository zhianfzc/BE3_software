#ifndef __KDP520_ADC_H
#define __KDP520_ADC_H

#include "types.h"

/* ADC Resource Configuration */
typedef struct {
  int				io_base;		// I2C register interface
  int				irq;				// I2C Event IRQ Number
} kdp_adc_resource;

struct kdp_adc_regs {
	/* DATA: offset 0x000~0x01C */
	UINT32		data[8];
	/* reserve */
	UINT32		reserve[24];
	/* THRHOLD: offset 0x080~0x09C */
	UINT32		thrhold[8];
#define HTHR_EN         (1<<31)
#define HTHR(x)         (((x)&0xFFF)<<16)
#define LTHR_EN         (1<<15)
#define LTHR(x)         (((x)&0xFFF)<<0)
	/* reserve */
	UINT32		reserve1[24];
	/* CTRL: offset 0x100 */
	UINT32		ctrl;
#define SCAN_NUM(x)			(x<<16)
#define SCANMODE_CONT   (1<<9)
#define SCANMODE_SGL    (1<<8)
#define SWSTART					(1<<4)
#define ADC_EN          (1<<0)
	/* TRIM: offset 0x104 */
	UINT32		trim;
	/* INTEN: offset 0x108 */
	UINT32		inten;
#define CHDONE_INTEN(x) (1<<((x)+8))
#define TS_OVR_INTREN 	(1<<3)
#define TS_UDR_INTREN   (1<<2)
#define STOP_INTEN      (1<<1)
#define DONE_INTEN      (1<<0)
	/* INTST: offset 0x10C */
	UINT32		intst;
#define CH_INTRSTS(x)			(1<<((x)+8))
#define TS_THDOD_INTRSTS	(1<<3)
#define TS_THDUD_INTRSTS	(1<<2)
#define ADC_STOP_INTRSTS	(1<<1)
#define ADC_DONE_INTSTS		(1<<0)
	/* TPARAM: offset 0x110 */
	UINT32		tparam;
	/* TPARAM1: offset 0x114 */
	UINT32		smpr;
	/* reserve */
	UINT32		reserve2;
	/* PRESCAL: offset 0x11C */
	UINT32		prescal;
	/* SQR: 		offset 0x120 */
	UINT32 		sqr;
	
};

void kdp520_adc_init(void);
INT32 kdp520_adc_uninit(kdp_adc_resource *res);
void kdp520_adc_reset(kdp_adc_resource *res);
void kdp520_adc_enable(kdp_adc_resource *res, int mode);
int kdp520_adc_read(int id);

#endif //__KDP520_ADC_H
