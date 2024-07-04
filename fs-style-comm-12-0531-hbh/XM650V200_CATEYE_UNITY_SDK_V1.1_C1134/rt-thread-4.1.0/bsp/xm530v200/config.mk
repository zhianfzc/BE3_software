BSP_ROOT ?= /home/xiaojianfei/gitlab/rt-thread/rt-thread-4.1.0/bsp/xm530v200
RTT_ROOT ?= /home/xiaojianfei/gitlab/rt-thread/rt-thread-4.1.0

CROSS_COMPILE ?=/opt/gcc-arm-none-eabi-6_2-2016q4/bin/arm-none-eabi-

#CFLAGS := -march=armv7-a -mtune=cortex-a5 -mfpu=neon-vfpv4 -ftree-vectorize -ffast-math -mfloat-abi=softfp -Wall -g -gdwarf-2 -O0
#AFLAGS := -c -march=armv7-a -mtune=cortex-a5 -mfpu=neon-vfpv4 -ftree-vectorize -ffast-math -mfloat-abi=softfp -x assembler-with-cpp -D__ASSEMBLY__ -gdwarf-2
#LFLAGS := -march=armv7-a -mtune=cortex-a5 -mfpu=neon-vfpv4 -ftree-vectorize -ffast-math -mfloat-abi=softfp -Wl,--gc-sections,-Map=rtthread.map,-cref,-u,system_vectors -T link.lds
CFLAGS := -march=armv7-a -mtune=cortex-a5 -mfpu=neon-vfpv4 -ftree-vectorize -mfloat-abi=softfp -Wall -g -gdwarf-2 -O0
AFLAGS := -c -march=armv7-a -mtune=cortex-a5 -mfpu=neon-vfpv4 -ftree-vectorize -mfloat-abi=softfp -x assembler-with-cpp -D__ASSEMBLY__ -gdwarf-2
LFLAGS := -march=armv7-a -mtune=cortex-a5 -mfpu=neon-vfpv4 -ftree-vectorize -mfloat-abi=softfp -Wl,--gc-sections,-Map=rtthread.map,-cref,-u,system_vectors -T link.lds
EXTERN_LIB := -lmmz_ko -lmpi -lvb_ko -lvi_ko -lsys_ko -lrgn_ko -lispapp_XM530V200 -lphyvi_XM530V200 -lsns_XAx_XM530V200 -lisp_XM530V200 -lae_XM530V200 -lawb_XM530V200 -lisp_ko -lm -lc -L/home/xiaojianfei/gitlab/rt-thread/rt-thread-4.1.0/bsp/xm530v200/libs

CPPPATHS :=-I$(BSP_ROOT) \
	-I$(BSP_ROOT)/applications \
	-I$(BSP_ROOT)/drivers \
	-I$(BSP_ROOT)/drivers/xm_sfc \
	-I$(BSP_ROOT)/include \
	-I$(BSP_ROOT)/include/isp \
	-I$(BSP_ROOT)/include/mpp \
	-I$(BSP_ROOT)/tools \
	-I$(RTT_ROOT)/components/dfs/filesystems/devfs \
	-I$(RTT_ROOT)/components/dfs/filesystems/elmfat \
	-I$(RTT_ROOT)/components/dfs/filesystems/nfs \
	-I$(RTT_ROOT)/components/dfs/filesystems/ramfs \
	-I$(RTT_ROOT)/components/dfs/filesystems/romfs \
	-I$(RTT_ROOT)/components/dfs/include \
	-I$(RTT_ROOT)/components/drivers/include \
	-I$(RTT_ROOT)/components/drivers/spi \
	-I$(RTT_ROOT)/components/drivers/spi/sfud/inc \
	-I$(RTT_ROOT)/components/finsh \
	-I$(RTT_ROOT)/components/libc/compilers/common \
	-I$(RTT_ROOT)/components/libc/compilers/newlib \
	-I$(RTT_ROOT)/components/libc/cplusplus \
	-I$(RTT_ROOT)/components/libc/posix/delay \
	-I$(RTT_ROOT)/components/libc/posix/io/aio \
	-I$(RTT_ROOT)/components/libc/posix/io/mman \
	-I$(RTT_ROOT)/components/libc/posix/io/poll \
	-I$(RTT_ROOT)/components/libc/posix/io/stdio \
	-I$(RTT_ROOT)/components/libc/posix/io/termios \
	-I$(RTT_ROOT)/components/libc/posix/ipc \
	-I$(RTT_ROOT)/components/libc/posix/pthreads \
	-I$(RTT_ROOT)/components/libc/posix/signal \
	-I$(RTT_ROOT)/components/net/lwip/lwip-2.1.2/src/include \
	-I$(RTT_ROOT)/components/net/lwip/lwip-2.1.2/src/include/netif \
	-I$(RTT_ROOT)/components/net/lwip/port \
	-I$(RTT_ROOT)/components/net/netdev/include \
	-I$(RTT_ROOT)/components/net/sal/impl \
	-I$(RTT_ROOT)/components/net/sal/include \
	-I$(RTT_ROOT)/components/net/sal/include/dfs_net \
	-I$(RTT_ROOT)/components/net/sal/include/socket \
	-I$(RTT_ROOT)/components/net/sal/include/socket/sys_socket \
	-I$(RTT_ROOT)/include \
	-I$(RTT_ROOT)/libcpu/arm/common \
	-I$(RTT_ROOT)/libcpu/arm/cortex-a 

DEFINES := -DHAVE_CCONFIG_H -DRT_USING_NEWLIB -D_POSIX_C_SOURCE=1 -D__RTTHREAD__
