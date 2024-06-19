#ifndef __ISP_PRINT_H__
#define  __ISP_PRINT_H__

#if (defined SOC_SYSTEM) ||(defined SOC_ALIOS)
#include <stdio.h>
#endif

#define NONE          "\033[m"   
#define RED           "\033[0;32;31m"   
#define LIGHT_RED     "\033[1;31m"   
#define GREEN         "\033[0;32;32m"   
#define LIGHT_GREEN   "\033[1;32m"   
#define BLUE          "\033[0;32;34m"   
#define LIGHT_BLUE    "\033[1;34m"   
#define DARY_GRAY     "\033[1;30m"   
#define CYAN          "\033[0;36m"   
#define LIGHT_CYAN    "\033[1;36m"   
#define PURPLE        "\033[0;35m"   
#define LIGHT_PURPLE "\033[1;35m"   
#define BROWN         "\033[0;33m"   
#define YELLOW        "\033[1;33m"   
#define LIGHT_GRAY    "\033[0;37m"   
#define WHITE         "\033[1;37m"   



#define ERR(fmt, args ... )    	printf(RED "ERR: " fmt NONE, ## args)
#define DEBUG(fmt, args ... ) 	printf(LIGHT_PURPLE "DEBUG: " fmt NONE,## args)
#define DBG(fmt, args ... ) 	printf(LIGHT_PURPLE fmt NONE,## args)
//#define WARN(fmt, args ... ) 	printf(BLUE "WARN: " fmt NONE,## args)


#define ISPCHECK_RET(express, name) \
		do { \
			int Ret; \
			Ret = express; \
			if (ISP_SUCCESS != Ret) { \
				printf(LIGHT_RED "[%s:%d]%s failed with %#x!\n" NONE, \
				__FILE__, __LINE__, name, Ret); \
				return Ret; \
			} \
		} while(0)


#endif // __ISP_PRINT_H__



