RASPI := 3
CONFIG_FAT := 1

ARCH := aarch64
#CROSS := aarch64-elf-
CROSS := aarch64-none-elf-
CC := $(CROSS)gcc
LD := $(CROSS)ld
OBJDUMP := $(CROSS)objdump
OBJCOPY := $(CROSS)objcopy
STRIP := $(CROSS)strip

#QEMU := /Users/dspace/raspi_os/qemu/build/qemu-system-aarch64
QEMU := qemu-system-aarch64
