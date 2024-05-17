#ifndef __VERSION_H__
#define __VERSION_H__

#define DRAM_MISCDATA_SIZE 	0x00000010

#if ( (CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP) || (CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP_USR) )
#define UART_MAJOR_VERSION                       (2)
#define UART_MINOR_VERSION                       (0)
#else
#define UART_MAJOR_VERSION                       (0)
#define UART_MINOR_VERSION                       (0)
#endif
#define UART_PROTOCOL_VERSION                    ( UART_MAJOR_VERSION<<8 | UART_MINOR_VERSION )

struct fw_misc_data {
    unsigned char version[4];
    unsigned int date;
};

extern struct fw_misc_data g_model_version;

#endif
