TOP_DIR = ../../
KERNELDIR = $(TOP_DIR)/rt-thread-4.1.0
include rules.mk
include ./configs/support_config
CSTM_ID=GENERAL
SENSOR_ID=X1x,X20,X30,X40,X50
TOPDIR = ..
INC = $(TOPDIR)/include
SRCA = .
DstDir := $(TOP_DIR)/rt-thread-4.1.0/bsp/xm530v200/isp_libs

SNSCOM = ./snsComm
SNSX1x = ./snsX1x
SNSX20 = ./snsX20
SNSX30 = ./snsX30
SNSX40 = ./snsX40
SNSX50 = ./snsX50
SNSX50 = ./snsX80

OUTPUT = ./_libs

LIB_OBJS :=

ifeq ($(findstring X1x,$(SENSOR_ID)),X1x)
	CFLAGS += -DSENS_X1x
	include snsX1x/Makefile_Sensor_X1x
endif
ifeq ($(findstring X20,$(SENSOR_ID)),X20)
	CFLAGS += -DSENS_X20
	include snsX20/Makefile_Sensor_X20
endif
ifeq ($(findstring X30,$(SENSOR_ID)),X30)
	CFLAGS += -DSENS_X30
	include snsX30/Makefile_Sensor_X30
endif
ifeq ($(findstring X40,$(SENSOR_ID)),X40)
	CFLAGS += -DSENS_X40
	include snsX40/Makefile_Sensor_X40
endif
ifeq ($(findstring X50,$(SENSOR_ID)),X50)
	CFLAGS += -DSENS_X50
	include snsX50/Makefile_Sensor_X50
endif
ifeq ($(findstring X80,$(SENSOR_ID)),X80)
	CFLAGS += -DSENS_X80
	include snsX80/Makefile_Sensor_X80
endif

CFLAGS  +=  $(CFLAGS_SENSOR)
CFLAGS	+= -I$(INC)/isp
CFLAGS	+= -I$(SNSX1x)/
CFLAGS	+= -I$(SNSX20)/
CFLAGS	+= -I$(SNSX30)/
CFLAGS	+= -I$(SNSX40)/
CFLAGS	+= -I$(SNSX50)/
CFLAGS	+= -I$(SNSX80)/
CFLAGS	+= -I$(SNSCOM)/

CFLAGS += -DSOC_SYSTEM


##############################################
#ISP
##############################################
CFLAGS += -DCSTM_$(CSTM_ID)


CFLAGS  += -Os

AFLAGS  += -rv

SRCS = $(wildcard *.c $(SNSCOM)/*.c)
LIB_OBJS += $(SRCS:%.c=%.o)	



LIBNAME = $(OUTPUT)/libsns_XAx.a


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

