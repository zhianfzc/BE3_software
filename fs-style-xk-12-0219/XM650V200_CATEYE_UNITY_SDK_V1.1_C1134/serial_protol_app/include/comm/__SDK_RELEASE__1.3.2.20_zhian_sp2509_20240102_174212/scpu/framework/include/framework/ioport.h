#ifndef __IOPORT_H__
#define __IOPORT_H__


#include "types.h"


#define IORESOURCE_TYPE_BITS    0x00003600	
#define IORESOURCE_MEM          0x00000200
#define IORESOURCE_IRQ          0x00000400
#define IORESOURCE_BUS          0x00001000


struct ioport_setting {
    u32 start;
    u32 flags;
};


#endif	
