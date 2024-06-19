TOP_DIR = .
KERNELDIR =../$(TOP_DIR)/rt-thread-4.1.0
include $(KERNELDIR)/bsp/xm530v200/rules.mk

SRC_DIR = $(TOP_DIR)/src

CFLAGS += -I$(TOP_DIR)/include/comm
CFLAGS += -Os

AFLAGS  += -rv

SRC  := $(wildcard $(SRC_DIR)/*.c) 
OBJ  := $(SRC:%.c=%.o)

TARGET := libserial_protol.a

all: $(TARGET)

$(TARGET):$(OBJ)
	$(RM) $@;
	$(AR) $(AFLAGS) $@ $^

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -f $(TARGET)
	@rm -f $(OBJ)	
	@rm -f $(SRC_DIR)/*.o
	find ./ -type f \( -name '*.elf*' -o -name '*.gdb' \) | xargs rm -f;	

install:
	cp $(TARGET) $(KERNELDIR)/bsp/xm530v200/libs
	cp $(TOP_DIR)/include/comm/app_main.h $(KERNELDIR)/bsp/xm530v200/include/comm
	cp $(TOP_DIR)/include/comm/config.h $(KERNELDIR)/bsp/xm530v200/include/comm
