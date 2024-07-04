#Makefile of cat_eye_lock, rt_os and pack

TOP_DIR = $(shell pwd)

APP_DIR = $(TOP_DIR)/serial_protol_app
RTOS_DIR = $(TOP_DIR)/rt-thread-4.1.0/bsp/xm530v200
PACK_DIR = $(TOP_DIR)/pack
FLASH_DIR = $(TOP_DIR)/flash_read

all:
	make -C $(APP_DIR)
	make -C $(APP_DIR) install
	scons -C $(RTOS_DIR)
	cp $(RTOS_DIR)/rtthread.bin $(PACK_DIR)/rt_thread/
	make -C $(PACK_DIR)
	cp -rf $(PACK_DIR)/target $(TOP_DIR)

clean:
	make -C $(APP_DIR) clean
	scons -C $(RTOS_DIR) -c
	make -C $(PACK_DIR) clean
	rm -rf $(TOP_DIR)/target
	rm $(PACK_DIR)/rt_thread/rtthread.bin






