-include config.mk

MUSL_INC = /Users/zuki/musl/include

CFLAGS := -Wall -g -O2 -std=gnu11 \
          -fno-pie -fno-pic -fno-stack-protector \
          -fno-zero-initialized-in-bss \
          -static -fno-builtin -nostdlib -nostdinc -ffreestanding -nostartfiles \
          -MMD -MP -Iinc -Iinc/wlan -Iinc/wlan/hostap/wpa_supplicant \
	  -Iinc/wlan/hostap/src
#          -mgeneral-regs-only \
#	kern/wlan/hostap/wpa_supplicant/events.c:2168:5: error:
#	'-mgeneral-regs-only' is incompatible with the use of floating-point types


CFLAGS += -DLOG_DEBUG -DRASPI=$(RASPI) \
           -DCONFIG_DRIVER_XV6 \
           -DCONFIG_CRYPTO_INTERNAL \
           -DCONFIG_BACKEND_FILE \
           -DCONFIG_NO_CONFIG_WRITE \
           -DCONFIG_NO_RANDOM_POOL \
           -DCONFIG_NO_ROBUST_AV \
           -DCONFIG_NO_WMM_AC \
           -DOS_NO_C_LIB_DEFINES \
           -DIEEE8021X_EAPOL \
	   -DCONFIG_FAT

ifeq (${CONFIG_FAT}, 1)
CFLAGS += -DCONFIG_FAT
endif

CFLAGS += -mlittle-endian -mcmodel=small -mno-outline-atomics

ifeq ($(strip $(RASPI)), 3)
CFLAGS += -mcpu=cortex-a53 -mtune=cortex-a53
else ifeq ($(strip $(RASPI)), 4)
CFLAGS += -mcpu=cortex-a72 -mtune=cortex-a72
else
$(error RASPI must be set to 3 or 4)
endif

SRC_DIRS := kern
BUILD_DIR = obj

KERN_ELF := $(BUILD_DIR)/kernel8.elf
KERN_IMG := $(BUILD_DIR)/kernel8.img
SD_IMG := $(BUILD_DIR)/sd.img

FIRMFILE := firmware.bin
FIRMWARE := firmware.o

all:
	rm -f $(SD_IMG)
	$(MAKE) -C boot
	$(MAKE) -C usr
	$(MAKE) -C dyn
	$(MAKE) $(SD_IMG)

# Automatically find sources and headers
SRCS := $(shell find $(SRC_DIRS) -name *.c -or -name *.S)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
-include $(DEPS)

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<
$(BUILD_DIR)/%.S.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(FIRMFILE): fat/firmware/brcmfmac43455-sdio.bin
	cp $< $@
$(FIRMWARE): $(FIRMFILE)
	$(OBJCOPY) -I binary -O elf64-littleaarch64 $< $@

$(KERN_ELF): kern/linker.ld $(OBJS) $(FIRMWARE)
	$(LD) -o $@ -T $< $(OBJS) $(FIRMWARE)
	$(OBJDUMP) -S -d $@ > $(basename $@).asm
	$(OBJDUMP) -x $@ > $(basename $@).hdr

$(KERN_IMG): $(KERN_ELF)
	$(OBJCOPY) -O binary $< $@

-include mksd.mk

QEMU_CMD ?= $(QEMU) -M raspi3b -nographic -serial null -serial mon:stdio -drive file=$(SD_IMG),if=sd,format=raw -netdev user,id=net0,hostfwd=tcp::8080-:80 -device usb-net,netdev=net0 -trace events=events,file=trace.log

#QEMU_CMD ?= $(QEMU) -M raspi3b -nographic -serial null -serial mon:stdio -drive file=$(SD_IMG),if=sd,format=raw

qemu: all
	$(QEMU_CMD) -kernel $(KERN_IMG)
qemu-gdb: all
	$(QEMU_CMD) -kernel $(KERN_IMG) -S -gdb tcp::1234
gdb:
	gdb -n -x .gdbinit

init:
	sudo apt install -y gcc-aarch64-linux-gnu gdb-multiarch
	sudo apt install -y qemu-system-arm qemu-efi-aarch64 qemu-utils
	sudo apt install -y mtools
	sudo apt install -y indent
	git submodule update --init --recursive
	(cd libc && export CROSS_COMPILE=$(CROSS) && ./configure --target=$(ARCH))

LINT_SRC := $(shell find $(SRC_DIRS) usr -name *.c)
LINT_TMP := $(LINT_SRC:%=%~)

lint:
	indent -kr -psl -ss -nut -ncs $(LINT_SRC)
	rm $(LINT_TMP)

clean:
	$(MAKE) -C usr clean
	rm -rf $(BUILD_DIR) firmware.*
	rm -f mkfs/mkfs

.PHONY: init all lint clean qemu qemu-gdb gdb
