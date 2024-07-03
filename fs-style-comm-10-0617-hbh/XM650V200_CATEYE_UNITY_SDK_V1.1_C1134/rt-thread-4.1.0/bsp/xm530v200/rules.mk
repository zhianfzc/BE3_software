CROSS	= /opt/gcc-arm-none-eabi-6_2-2016q4/bin/arm-none-eabi-
CC	= @echo " GCC	$@"; $(CROSS)gcc
CPP	= @echo " G++	$@"; $(CROSS)g++
LD	= @echo " LD	$@"; $(CROSS)ld
AR	= @echo " AR	$@"; $(CROSS)ar
RM	= @echo " RM	$@"; rm -f

#CFLAGS += -march=armv7-a -mtune=cortex-a5 -mfpu=neon-vfpv4 -ftree-vectorize -ffast-math -mfloat-abi=softfp -fsigned-char -Wall -g -gdwarf-2 -O2 -DHAVE_CCONFIG_H -D__RTTHREAD__ -DRT_USING_NEWLIB -D_POSIX_C_SOURCE=1 

CFLAGS += -march=armv7-a -mtune=cortex-a5 -ftree-vectorize -mfloat-abi=softfp -fsigned-char -Wall -g -gdwarf-2 -O2 -DHAVE_CCONFIG_H -D__RTTHREAD__ -DRT_USING_NEWLIB -D_POSIX_C_SOURCE=1 

CFLAGS += -I$(KERNELDIR)/include 
CFLAGS += -I$(KERNELDIR)/libcpu/arm/common 
CFLAGS += -I$(KERNELDIR)/libcpu/arm/cortex-a 
CFLAGS += -I$(KERNELDIR)/components/finsh 
CFLAGS += -I$(KERNELDIR)/components/dfs/include 
CFLAGS += -I$(KERNELDIR)/components/dfs/filesystems/elmfat 
CFLAGS += -I$(KERNELDIR)/components/dfs/filesystems/devfs 
CFLAGS += -I$(KERNELDIR)/components/dfs/filesystems/ramfs 
CFLAGS += -I$(KERNELDIR)/components/dfs/filesystems/romfs 
CFLAGS += -I$(KERNELDIR)/components/net/lwip/lwip-2.0.3/src/include 
CFLAGS += -I$(KERNELDIR)/components/net/lwip/lwip-2.0.3/src/include/ipv4 
CFLAGS += -I$(KERNELDIR)/components/net/lwip/lwip-2.0.3/src/include/netif 
CFLAGS += -I$(KERNELDIR)/components/net/lwip/port 
CFLAGS += -I$(KERNELDIR)/components/net/netdev/include 
CFLAGS += -I$(KERNELDIR)/components/net/sal/include 
CFLAGS += -I$(KERNELDIR)/components/net/sal/include/socket 
CFLAGS += -I$(KERNELDIR)/components/net/sal/impl 
CFLAGS += -I$(KERNELDIR)/components/net/sal/include/dfs_net 
CFLAGS += -I$(KERNELDIR)/components/net/sal/include/socket/sys_socket 
CFLAGS += -I$(KERNELDIR)/components/drivers/spi 
CFLAGS += -I$(KERNELDIR)/components/drivers/include 
CFLAGS += -I$(KERNELDIR)/components/drivers/spi/sfud/inc 
CFLAGS += -I$(KERNELDIR)/components/libc/compilers/common 
CFLAGS += -I$(KERNELDIR)/components/libc/compilers/newlib 
CFLAGS += -I$(KERNELDIR)/components/libc/posix/signal 
CFLAGS += -I$(KERNELDIR)/components/libc/posix/pthreads 
CFLAGS += -I$(KERNELDIR)/components/libc/posix/io/stdio 
CFLAGS += -I$(KERNELDIR)/components/libc/posix/io/poll 
CFLAGS += -I$(KERNELDIR)/components/libc/posix/io/mman/
CFLAGS += -I$(KERNELDIR)/components/libc/posix/delay 
CFLAGS += -I$(KERNELDIR)/components/libc/posix/ipc 
CFLAGS += -I$(KERNELDIR)/components/libc/posix/ipc/system-v/
CFLAGS += -I$(KERNELDIR)/components/libc/cplusplus 
CFLAGS += -I$(KERNELDIR)/components/legacy
CFLAGS += -I$(KERNELDIR)/bsp/xm530v200/
CFLAGS += -I$(KERNELDIR)/bsp/xm530v200/drivers/
CFLAGS += -I$(KERNELDIR)/bsp/xm530v200/include/
CFLAGS += -I$(KERNELDIR)/bsp/xm530v200/drivers/usb/
