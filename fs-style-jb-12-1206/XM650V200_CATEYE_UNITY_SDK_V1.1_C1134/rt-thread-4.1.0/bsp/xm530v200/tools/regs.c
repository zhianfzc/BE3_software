#include <rthw.h>
#include <rtthread.h>
#include <finsh.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"

static int regs(int argc, char *argv[])
{
	unsigned long m_phyaddr;
	unsigned long m_val;
	unsigned int m_len;
	unsigned int i;
			
	/* for command */
	if (1 == argc)
	{
		printf("\n\n\"regs help\" for help\n\n");
		printf("Usage:\t\n");
		printf("regs [reg]\t\t\t\tread\t\n");
		printf("regs [reg] [val]\t\t\t\twrite\t\n");
		printf("regs -d [reg] [len]\t\t\t\treadn\t\n");
	}
	else if (2 == argc)
	{	
		m_phyaddr = strtoul(argv[1],NULL,16);
		m_val = readl(m_phyaddr);
		printf("[reg] = 0x%08lx,[val]= 0x%08lx\n",m_phyaddr,m_val);
	}
	else if (3 == argc)
	{	
		m_phyaddr = strtoul(argv[1],NULL,16);
		m_val = readl(m_phyaddr);
		printf("[reg] = 0x%08lx,[val]= 0x%08lx -> ",m_phyaddr,m_val);
		m_val = strtoul(argv[2],NULL,16);
		writel(m_val, m_phyaddr);
		m_val = readl(m_phyaddr);
		printf("0x%08lx\n",m_val);
	}
	else if (4 == argc)
	{
		if(!strcmp(argv[1], "-d"))
		{
			m_phyaddr = strtoul(argv[2],NULL,16);
			m_len = strtoul(argv[3],NULL,16);
			printf("[reg] = 0x%08lx,[len]= 0x%08x\n",m_phyaddr,m_len);
			for(i = 0; i < m_len; i+= 4)
			{
				if(!(i % 16))
					printf("\n%08lx: ",m_phyaddr + i);
				m_val = readl(m_phyaddr + i);
				printf("%08lx ",m_val);
			}
			printf("\n");
		}
	}
	
	return 0;
}


FINSH_FUNCTION_EXPORT(regs, read or write register);
MSH_CMD_EXPORT(regs, restart the register);

