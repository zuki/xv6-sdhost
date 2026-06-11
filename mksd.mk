SD_IMG ?=
KERN_IMG ?=

BOOT_IMG := $(BUILD_DIR)/boot.img
V6_IMG := $(BUILD_DIR)/v6.img
FAT_IMG := $(BUILD_DIR)/fat.img

SECTOR_SIZE := 512

# The total sd card image is 192 MB, 64 MB for boot sector and 128 MB for file system.
# SDカードイメージ: 192MB = ブートセクタ: 64MB + V6FS: 64MB + FATFS: 64MB
# セクタ（512バイト）単位
SECTORS := 2*200*1024
BOOT_OFFSET := 2048
BOOT_SECTORS := 2*64*1024
FAT_OFFSET := $$(($(BOOT_OFFSET)+$(BOOT_SECTORS)))
FAT_SECTORS := 2*64*1024
V6_OFFSET := $$(($(FAT_OFFSET)+$(FAT_SECTORS)))
V6_SECTORS := 2*64*1024

USR_FILES := $(shell find obj/usr/bin -type f)
DYN_FILES := $(shell find obj/dyn/bin -type f)

.DELETE_ON_ERROR: $(BOOT_IMG) $(SD_IMG)

# TODO: Detect img size automatically
$(BOOT_IMG): $(KERN_IMG) $(shell find boot/*)
	dd if=/dev/zero of=$@ seek=$$(($(BOOT_SECTORS) - 1)) bs=$(SECTOR_SIZE) count=1
	# -F specify FAT32
	# -c 1 specify one sector per cluster so that we can create a smaller one
	mformat -F -c 1 -v bootfs -i $@ ::
	# Copy files into boot partition
	$(foreach x, $^, mcopy -i $@ $(x) ::$(notdir $(x));)

$(FAT_IMG): $(shell find fat/*)
	dd if=/dev/zero of=$@ bs=$(SECTOR_SIZE) count=131072
	mformat -F -c 1 -v fatfs -i $@ ::
	$(foreach x, $^, mcopy -i $@ $(x) ::;)
#	$(foreach x, $^, mcopy -i $@ $(x) ::$(notdir $(x));)

mkfs/mkfs: mkfs/mkfs.c
	cc -o mkfs/mkfs -Imkfs/ mkfs/mkfs.c

$(V6_IMG): mkfs/mkfs test.txt $(USR_FILES) $(DYN_FILES)
	echo $^
	./mkfs/mkfs $@ test.txt $(USR_FILES) $(DYN_FILES)

$(SD_IMG): $(BOOT_IMG) $(FAT_IMG) $(V6_IMG)
	dd if=/dev/zero of=$@ seek=$$(($(SECTORS) - 1)) bs=$(SECTOR_SIZE) count=1
	printf "                                                                \
	  $(BOOT_OFFSET), $$(($(BOOT_SECTORS)*$(SECTOR_SIZE)/1024))K, c,\n      \
	  $(FAT_OFFSET), $$(($(FAT_SECTORS)*$(SECTOR_SIZE)/1024))K, c,\n        \
	  $(V6_OFFSET), $$(($(V6_SECTORS)*$(SECTOR_SIZE)/1024))K, L,\n          \
	" | sfdisk $@
	dd if=$(BOOT_IMG) of=$@ seek=$(BOOT_OFFSET) conv=notrunc
	dd if=$(FAT_IMG) of=$@ seek=$(FAT_OFFSET) conv=notrunc
	dd if=$(V6_IMG) of=$@ seek=$(V6_OFFSET) conv=notrunc
