#ifndef __UART_H__
#define __UART_H__
#include <rtthread.h>

#define SAMPLE_UART_NAME       "uart1"

rt_device_t *serial_get();

int uart_init(char *uart_name, unsigned int baud_rate);
int uart_reset(char *uart_name, unsigned int baud_rate);
//char uart_sample_get_char(unsigned int timeout);
char uart_sample_get_char(char *ch, unsigned int timeout);


int ota_write_falsh(unsigned char *p_ota_data, unsigned int ota_data_size, unsigned char ota_type);

#endif
