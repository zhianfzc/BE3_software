
TOPDIR = ..
TOP_DIR = ../../
KERNELDIR = $(TOP_DIR)/rt-thread-4.1.0
include rules.mk

CSTM_ID=GENERAL
USE_LIBFVIDEO:=YES#

INC = $(TOPDIR)/include
INC2 = ./include
SRC = ./src
DstDir = $(TOP_DIR)/rt-thread-4.1.0/bsp/xm530v200/isp_libs

CFLAGS	+= -I$(INC)/isp
CFLAGS	+= -I$(INC2)

CFLAGS  += -Os
AFLAGS  += -rv

CFLAGS += -DSOC_SYSTEM

SRCS = $(wildcard $(SRC)/sensor/*.c)
LIB_OBJS = $(SRCS:%.c=%.o)
LIB_OBJS += $(SRC)/autoscene.o
ifeq ($(USE_LIBFVIDEO),YES)
	CFLAGS += -DUSE_LIBFVIDEO
	LIB_OBJS += $(SRC)/dnc_fvideo.o
	LIB_OBJS += $(SRC)/fvideo.o
	EXT += _fv
else
	LIB_OBJS += $(SRC)/dnc.o
endif

LIBNAME = _libs/libautoscene$(EXT).a


TARGET	= $(LIBNAME)

all: $(TARGET)
	cp $(LIBNAME) $(DstDir)/

$(LIBNAME): $(LIB_OBJS)
	$(RM) $@;
	$(AR) $(AFLAGS) $@ $^

$(SRC)/%.o: $(SRC)/%.c
	$(CC)  $(CFLAGS) -c -o $@ $<

 
clean:
	rm -f $(TARGET) $(LIB_OBJS);
	find ./ -type f \( -name '*.elf*' -o -name '*.gdb' \) | xargs rm -f


