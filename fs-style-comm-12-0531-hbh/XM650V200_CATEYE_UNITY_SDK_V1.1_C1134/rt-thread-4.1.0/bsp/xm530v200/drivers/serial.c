/*
 *  serial.c UART driver
 *
 * COPYRIGHT (C) 2013, Shanghai Real-Thread Technology Co., Ltd
 *
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-03-30     Bernard      the first verion
 */

#include <rthw.h>
#include <rtdevice.h>
#include <board.h>

#include "serial.h"
#include "gic.h"
struct hw_uart_device
{
    rt_uint32_t hw_base;
    rt_uint32_t irqno;
};

#define UART_DR(base)   __REG32(base + 0x00)
#define UART_FR(base)   __REG32(base + 0x18)
#define UART_CR(base)   __REG32(base + 0x30)
#define UART_IMSC(base) __REG32(base + 0x38)
#define UART_ICR(base)  __REG32(base + 0x44)

#define UARTFR_RXFE     0x10
#define UARTFR_TXFF     0x20
#define UARTIMSC_RXIM   0x10
#define UARTIMSC_TXIM   0x20
#define UARTICR_RXIC    0x10
#define UARTICR_TXIC    0x20

#define UART_RATE	24000000

static void rt_hw_uart_isr(int irqno, void *param)
{
    struct rt_serial_device *serial = (struct rt_serial_device *)param;
	//应该清中断
	struct hw_uart_device *uart;
    uart = serial->parent.user_data;
	
//	boot_debug("rt_hw_uart_isr 0x%08x\r\n",readl(uart->hw_base+UART011_MIS));

	/*writel(0x00, UART_ICR(uart->hw_base));*/

	if (readl(uart->hw_base+UART011_MIS)&(UART011_RTIM | UART011_RXIM))
    rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_IND);
	if (readl(uart->hw_base+UART011_MIS)&UART011_TXIM)
    rt_hw_serial_isr(serial, RT_SERIAL_EVENT_TX_DONE);
}

#define DIV_ROUND_CLOSEST(x, divisor)(			\
{							\
	typeof(divisor) __divisor = divisor;		\
	(((x) + ((__divisor) / 2)) / (__divisor));	\
}							\
)
void uart_start(struct rt_serial_device *serial)
{
	struct hw_uart_device *uart;
	RT_ASSERT(serial != RT_NULL);
	uart = (struct hw_uart_device *)serial->parent.user_data;

	//////////////////////////////////////////////
	//中断FIFO水线设置寄存器，按缺省值 1/2full
	//writew(,uart->hw_base+ UART011_IFLS);

	/*
	 * Provoke TX FIFO interrupt into asserting.
	 */
	//缺省波特率115200
	/* Set baud rate */
	//rt_uint32_t quot;
	//quot = DIV_ROUND_CLOSEST(UART_RATE * 4, CONFIG_BAUDRATE);
	//rt_kprintf("%s rate %d : quot: %x\n", __FUNCTION__, CONFIG_BAUDRATE, quot);
	//writew(quot & 0x3f,uart->hw_base + UART011_FBRD);
	//writew(quot >> 6, uart->hw_base + UART011_IBRD);
	
	rt_uint32_t cr = UART01x_CR_UARTEN | UART011_CR_TXE | UART011_CR_LBE;
	writew(cr,uart->hw_base+ UART011_CR);
	//8bit，fifo enable
	writew(0xff, uart->hw_base	+ UART011_MIS);
	writew(0, uart->hw_base  + UART01x_DR);
	while (readw(uart->hw_base	+ UART01x_FR) & UART01x_FR_BUSY)
		dmb();

	cr = UART01x_CR_UARTEN | UART011_CR_RXE | UART011_CR_TXE;
	writew(cr, uart->hw_base  + UART011_CR);

	/*
	 * initialise the old status of the modem signals
	 */

	/*
	 * Finally, enable interrupts, only timeouts when using DMA
	 * if initial RX DMA job failed, start in interrupt mode
	 * as well.
	 */
	/* Clear out any spuriously appearing RX interrupts */
	 writew(UART011_RTIS | UART011_RXIS,
		uart->hw_base  + UART011_ICR);
}

static rt_err_t uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
	rt_uint32_t old_cr,quot,lcr_h;
	
    struct hw_uart_device *uart;
    RT_ASSERT(serial != RT_NULL);
    uart = (struct hw_uart_device *)serial->parent.user_data;
	/* first, disable everything */
	old_cr = readw(uart->hw_base + UART011_CR);
	writew(0, uart->hw_base + UART011_CR);

	quot = DIV_ROUND_CLOSEST(UART_RATE * 4, cfg->baud_rate);
//	rt_kprintf("%s rate %d : quot: %x\n", __FUNCTION__, cfg->baud_rate, quot);

	/* Set baud rate */
	writew(quot & 0x3f,uart->hw_base + UART011_FBRD);
	writew(quot >> 6, uart->hw_base + UART011_IBRD);

	/*
	 * ----------v----------v----------v----------v-----
	 * NOTE: lcrh_tx and lcrh_rx MUST BE WRITTEN AFTER
	 * UART011_FBRD & UART011_IBRD.
	 * ----------^----------^----------^----------^-----
	 */
 	switch (cfg->data_bits) {
	case 5:
		lcr_h = UART01x_LCRH_WLEN_5;
		break;
	case 6:
		lcr_h = UART01x_LCRH_WLEN_6;
		break;
	case 7:
		lcr_h = UART01x_LCRH_WLEN_7;
		break;
	default: // CS8
		lcr_h = UART01x_LCRH_WLEN_8;
		break;
	}
	if (cfg->stop_bits == STOP_BITS_3)//2bit停止位
		lcr_h |= UART01x_LCRH_STP2;
	if (cfg->parity != PARITY_NONE) {
		lcr_h |= UART01x_LCRH_PEN;
		if (cfg->parity  ==  PARITY_EVEN)//偶校验
			lcr_h |= UART01x_LCRH_EPS;
	}
	//fifo使能
	lcr_h |= UART01x_LCRH_FEN;
	writew(lcr_h, uart->hw_base + UART011_LCRH);
	writew(0xff, uart->hw_base + UART011_MIS);
	writew(old_cr, uart->hw_base + UART011_CR);


	writew(UART011_OEIS | UART011_BEIS | UART011_PEIS | UART011_FEIS |
       UART011_RTIS | UART011_RXIS, uart->hw_base + UART011_ICR);

	writew(0, uart->hw_base + UART011_IMSC);

	/* enable rx irq */
	uart_start(serial);
	rt_hw_interrupt_install(uart->irqno, rt_hw_uart_isr, serial, "uart");
	rt_hw_interrupt_umask(uart->irqno);
	return 0;
}


//设置接收超时中断，接收中断
void set_rxtx_int(struct rt_serial_device *serial,int cmd) 
{
    struct hw_uart_device *uart;
    RT_ASSERT(serial != RT_NULL);
    uart = (struct hw_uart_device *)serial->parent.user_data;

	int tmpmscreg;
	tmpmscreg = readw(uart->hw_base  + UART011_IMSC);
	if (cmd & RT_DEVICE_FLAG_INT_RX)
		tmpmscreg|=(UART011_RTIM | UART011_RXIM);
	if (cmd & RT_DEVICE_FLAG_INT_TX)
		tmpmscreg|=UART011_TXIM;
	
	writew(tmpmscreg, uart->hw_base + UART011_IMSC);
}
//关闭中断
void clr_rxtx_int(struct rt_serial_device *serial,int cmd) 
{
    struct hw_uart_device *uart;
    RT_ASSERT(serial != RT_NULL);
    uart = (struct hw_uart_device *)serial->parent.user_data;

	int tmpmscreg;
	tmpmscreg = readw(uart->hw_base  + UART011_IMSC);
	if (cmd & RT_DEVICE_FLAG_INT_TX)
		tmpmscreg &= (~UART011_TXIM);
	if (cmd & RT_DEVICE_FLAG_INT_RX)
		tmpmscreg &= (~(UART011_RTIM | UART011_RXIM));
	writew(tmpmscreg, uart->hw_base + UART011_IMSC);
}

static rt_err_t uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    RT_ASSERT(serial != RT_NULL);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_SET_INT:
		set_rxtx_int(serial,(int)arg);
		break;
	case RT_DEVICE_CTRL_CLR_INT:
		clr_rxtx_int(serial,(int)arg);
        break;
    }

    return RT_EOK;
}

unsigned int uart_pri_ctrl = 1;

static int uart_putc(struct rt_serial_device *serial, char c)
{
	if (uart_pri_ctrl)
	{
		struct hw_uart_device *uart;

		RT_ASSERT(serial != RT_NULL);
		uart = (struct hw_uart_device *)serial->parent.user_data;

		while (UART_FR(uart->hw_base) & UARTFR_TXFF);
		UART_DR(uart->hw_base) = c;
	}

    return 1;
}

static int uart_getc(struct rt_serial_device *serial)
{
    int ch;
    struct hw_uart_device *uart;
    RT_ASSERT(serial != RT_NULL);
    uart = (struct hw_uart_device *)serial->parent.user_data;
	struct rt_device *dev = (struct rt_device *)serial;

    ch = -1;
    if (!(UART_FR(uart->hw_base) & UARTFR_RXFE) && (dev->open_flag&RT_DEVICE_FLAG_INT_RX))
    {
        ch = UART_DR(uart->hw_base) & 0xff;
		if (ch == '+')
			uart_pri_ctrl = 1;
		//if (ch == '-')
			//uart_pri_ctrl = 0;
	}

    return ch;
}

static const struct rt_uart_ops _uart_ops =
{
    uart_configure,
    uart_control,
    uart_putc,
    uart_getc,
};


static struct rt_serial_device _serial0;
static struct rt_serial_device _serial1;
static struct rt_serial_device _serial2;

static struct hw_uart_device _uart0_device =
{
    XM_UART0_BASE,
    IRQ_XM_UART0,
};

static struct hw_uart_device _uart1_device =
{
    XM_UART1_BASE,
    IRQ_XM_UART1,
};


int rt_hw_uart_init(void)
{
    struct serial_configure config;

    config.baud_rate = BAUD_RATE_115200;
    config.bit_order = BIT_ORDER_LSB;
    config.data_bits = DATA_BITS_8;
    config.parity    = PARITY_NONE;
    config.stop_bits = STOP_BITS_1;
    config.invert    = NRZ_NORMAL;
	config.bufsz     = RT_SERIAL_RB_BUFSZ;

    _serial0.ops    = &_uart_ops;
    _serial0.config = config;

    _serial1.ops    = &_uart_ops;
    _serial1.config = config;

    _serial2.ops    = &_uart_ops;
    _serial2.config = config;

    /* register uart device */
    rt_hw_serial_register(&_serial0, "uart0",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                          &_uart0_device);
    rt_hw_serial_register(&_serial1, "uart1",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                          &_uart1_device);
	
    return 0;
}
INIT_BOARD_EXPORT(rt_hw_uart_init);

