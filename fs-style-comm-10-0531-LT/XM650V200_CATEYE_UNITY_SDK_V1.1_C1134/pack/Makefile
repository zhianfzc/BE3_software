#Makefile of dfp_lock_pack_rt
PLATFORM=ZF50S
DEVICE=CAT_EYE
TIME=$(shell date +%y%m%d%H%M)
BURN_FILE=$(PLATFORM)_$(DEVICE).LOCK$(TIME).bin

TOP_DIR=$(shell pwd)
TAR_DIR=$(TOP_DIR)/target

TAR_FILE=$(TAR_DIR)/$(BURN_FILE)

default: pack

splice:
	@dd if=$(TOP_DIR)/uboot/u-boot.bin of=$(TAR_FILE) bs=4K count=64 conv=sync
	@dd if=$(TOP_DIR)/ConfigFile/ProductDefinition of=$(TAR_FILE) bs=4K count=1 seek=64 conv=sync
	@dd if=$(TOP_DIR)/rt_thread/rtthread.bin of=$(TAR_FILE) bs=4K count=512 seek=80 conv=sync
	@dd if=$(TOP_DIR)/feature/FeatureValue.txt of=$(TAR_FILE) bs=4K count=101 seek=592 conv=sync
	@dd if=$(TOP_DIR)/ia/default/dpu_models of=$(TAR_FILE) bs=4K count=2975 seek=693 conv=sync
	#@dd if=$(TOP_DIR)/ia/general/dpu_models of=$(TAR_FILE) bs=4K count=2929 seek=693 conv=sync

	#补齐0 不计算以下空间大小	
	@dd if=$(TOP_DIR)/feature/FeatureValue.txt of=$(TAR_FILE) bs=4K count=1 seek=3668 conv=sync

prebuild:
	mkdir -p $(TAR_DIR)

.PHONY: clean pack
	
pack: prebuild splice

clean:
	rm -rf $(TAR_DIR)

