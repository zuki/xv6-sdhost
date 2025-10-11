# Pageシステム（BudyyとSlab）を導入

- xv6-milkv-duoで実装（Akira Midorikawa氏の[Cyanurus](https://github.com/redcap97/cyanurus)のコードを流用）
  したPageシステムを導入した
- kalloc()とkrfrre()はbuddy_alloc()とbuddy_free()へのスタブとした
- kmalloc()とkmfree()も実装した
- slabにアライメント指定フラグを追加した

```bash
[0]main: cpu 0 init finished
[3]main: cpu 3 init finished
[1]main: cpu 1 init finished
[0]sdhost_probe: firmware sets clock divider
[2]main: cpu 2 init finished
[0]sdhost_set_ios: ios clock 400000, pwr 0, bus_width 0, timing 0, vdd 0, drv_t0
[0]sdhost_finish_command: error detected: CMD 0x4205, HSTS 0x40, EDM 0x10800
[0]sdhost_finish_command: command 5 timeout
[0]emmc_card_reset: OCR: 0xff80, 1.8v support: 0, SDHC support: 1
[0]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 0, timing 0, vdd 0, drv0
[0]emmc_card_reset: card CID: 0x2544d53, 0x41313647, 0x31299f69, 0x3a0138c3
[0]emmc_card_reset: RCA: 0x1234
[3]emmc_card_reset: SCR: version 3.0x, bus_widths 0x5
[3]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 1, timing 0, vdd 0, drv0
[3]emmc_card_reset: found valid version 3.0x SD card
[1]dev_init: LBA of 1st block 0x20800, 0x1f800 blocks totally
[1]iinit: sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 38
init: starting sh
sh: argv[0] = 'sh'
sh: testenv = 'FROM_INIT'
$ ls
.              4000 1 512
..             4000 1 512
cat            8000 2 38568
init           8000 3 22400
echo           8000 4 39480
mkfs           8000 5 45128
sh             8000 6 54056
utest          8000 7 17744
ls             8000 8 41304
console        0 9 0
$ echo abc
abc
```

## 変更、追加したファイル

```bash
$ git status
On branch mac
Changes to be committed:
  (use "git restore --staged <file>..." to unstage)
	new file:   inc/buddy.h
	modified:   inc/mm.h
	new file:   inc/slab.h
	new file:   kern/buddy.c
	modified:   kern/mm.c
	new file:   kern/slab.c
```

# 実際にslabを使用したらエラーが発生

```bash
[3]main: cpu 3 init finished
[2]main: cpu 2 init finished
[0]main: cpu 0 init finished
[1]main: cpu 1 init finished
[3]mbox_set_sdhost_clock: unexpected tag resp 0x80000000, normal for qemu
[3]sdhost_set_ios: ios clock 400000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[3]sdhost_finish_command: error detected: CMD 0x4205, HSTS 0x40, EDM 0x10800
[3]sdhost_finish_command: command 5 timeout
[3]emmc_card_reset: OCR: 0xffff, 1.8v support: 0, SDHC support: 0
[3]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[3]emmc_card_reset: card CID: 0xaa585951, 0x454d5521, 0x1deadbe, 0xef006219
[3]emmc_card_reset: RCA: 0x4567
[3]emmc_card_reset: SCR: version 2.00, bus_widths 0x5
[3]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 1, timing 0, vdd 0, drv_type 0
[3]emmc_card_reset: found valid version 2.00 SD card
[2]sd_init: partition[0]: TYPE: 12, LBA = 0x800, #SECS = 0x20000
[2]sd_init: partition[1]: TYPE: 131, LBA = 0x20800, #SECS = 0x1f800
[2]sd_init: sd_init ok

[2]bget: b->data: 0xa30080				// 物理アドレス?
[2]sd_rw: bno: 0x1, flags: 0
[2]sd_start: dev: 1, blockno: 0x1, bno: 0x20801, seek: 0x4100200
CurrentEL: 0x1
DAIF: Debug(1) SError(1) IRQ(1) FIQ(1)
SPSel: 0x1
SPSR_EL1: 0x800003c5
SP: 0xffff000000a4cda0
SP_EL0: 0x0
ELR_EL1: 0xffff00000008c914, EC: 0x25, ISS: 0x46.
FAR_EL1: 0xa30080
irq of type 4 unimplemented.
kern/console.c:286: kernel panic at cpu 0.
QEMU: Terminated
```

## slab_cache_alloc()でobjectのアドレスにPAGE_STARTを足して返すように変更

```bash
qemu-system-aarch64 -M raspi3b -nographic -serial null -serial mon:stdio -drive file=obj/sd.img,if=sd,format=raw -kernel obj/kernel8.img
[1]main: cpu 1 init finished
[3]main: cpu 3 init finished
[0]main: cpu 0 init finished
[2]main: cpu 2 init finished
[1]mbox_set_sdhost_clock: unexpected tag resp 0x80000000, normal for qemu
[1]sdhost_set_ios: ios clock 400000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[1]sdhost_finish_command: error detected: CMD 0x4205, HSTS 0x40, EDM 0x10800
[1]sdhost_finish_command: command 5 timeout
[1]emmc_card_reset: OCR: 0xffff, 1.8v support: 0, SDHC support: 0
[1]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[1]emmc_card_reset: card CID: 0xaa585951, 0x454d5521, 0x1deadbe, 0xef006219
[1]emmc_card_reset: RCA: 0x4567
[2]emmc_card_reset: SCR: version 2.00, bus_widths 0x5
[2]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 1, timing 0, vdd 0, drv_type 0
[2]emmc_card_reset: found valid version 2.00 SD card
[3]sd_init: partition[0]: TYPE: 12, LBA = 0x800, #SECS = 0x20000
[3]sd_init: partition[1]: TYPE: 131, LBA = 0x20800, #SECS = 0x1f800
[3]sd_init: sd_init ok

[2]iinit: sb: size 1000 nblocks 963 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 36
// ここでストール	: init()が読み込めない? : kernelとuserでobjectの仮想アドレスを変える必要あり（option?）
```

## initを調べる

```bash
[3]main: cpu 3 init finished
[0]main: cpu 0 init finished
[1]main: cpu 1 init finished
[2]main: cpu 2 init finished
[3]mbox_set_sdhost_clock: unexpected tag resp 0x80000000, normal for qemu
[3]sdhost_set_ios: ios clock 400000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[3]sdhost_finish_command: error detected: CMD 0x4205, HSTS 0x40, EDM 0x10800
[3]sdhost_finish_command: command 5 timeout
[3]emmc_card_reset: OCR: 0xffff, 1.8v support: 0, SDHC support: 0
[3]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[3]emmc_card_reset: card CID: 0xaa585951, 0x454d5521, 0x1deadbe, 0xef006219
[3]emmc_card_reset: RCA: 0x4567
[2]emmc_card_reset: SCR: version 2.00, bus_widths 0x5
[2]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 1, timing 0, vdd 0, drv_type 0
[2]emmc_card_reset: found valid version 2.00 SD card
[3]sd_init: partition[0]: TYPE: 12, LBA = 0x800, #SECS = 0x20000
[3]sd_init: partition[1]: TYPE: 131, LBA = 0x20800, #SECS = 0x1f800
[3]sd_init: sd_init ok

[2]iinit: sb: size 1000 nblocks 963 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 36
[1]execve: path='/init', argv=0x0, envp=0x0
[3]execve: check elf header finish
[1]execve: init bss [0x401fe8, 0x401fe8)
[1]execve: init bss [0x403140, 0x4037b8)
[3]execve: argv: 0xffffffffffd8, envp: 0xffffffffffe0, auxv: 0xffffffffffe8
[3]execve: newsp: 0xffffffffffd0
[3]execve: entry 0x400118
[3]execve: finish init
// ストール					// initの呼び出しまでは成功で、init処理中にエラー
```

## ログシステムの問題と判明

- ブロックサイズの変更に合わせてログシステムの修正も必要がある（x86版xv6で実施）と思われたが
  ログシステム自体あまり意味がなく、最近は使用しないようにしていたのでここでも使用しないことにした

```bash
[3]iinit: sb: size 1000 nblocks 963 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 36
[3]namex: path: /init
[3]namex: cwd->inum: 1
namex 1235789b: ip->inum: 3
[1]execve: finish init
[1]sys_openat: dirfd -100, path 'console', flag 0x20002
[1]sys_openat: begin_op: omode: 0x20002
[1]namex: path: console
[1]namex: cwd->inum: 1
namex 12356 dirlookup failed
[3]sys_mknodat: path 'console', major:minor 1:1
[3]namex: path: console
[3]namex: cwd->inum: 1
namex 1234 ip->inum=1
[3]create: dp->inum: 1
create: 189abdef ip->inum=9
ip->inum: 92	// ストール
QEMU: Terminated
```

```c
int sys_mknodat() {

    if ((ip = create(path, T_DEV, major, minor)) == 0) {
        end_op();
        return -1;
    }
    iunlockput(ip);
    end_op();		// ここでストール
    return 0;
}
```

- QEMUでも実機でも稼働

```bash
[2]iinit: sb: size 1000 nblocks 963 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 36
[2]initlog: not use log
init: starting sh
sh: argv[0] = 'sh'
sh: testenv = 'FROM_INIT'
$ ls
.              4000 1 4096
..             4000 1 4096
cat            8000 2 38568
init           8000 3 22400
echo           8000 4 39480
mkfs           8000 5 45528
sh             8000 6 54056
utest          8000 7 17744
ls             8000 8 41304
console        0 9 0
$ echo ab
ab
$ echo abc > test.txt
$ cat test.txt
abc
```

## mbox_get_arm_memory()

```bash
[0]mbox_test: memory: 0x3b400000		// 948 MB
```

## usb/net機能追加にともないPAGE_STARTとPAGE_NUMを修正

```c
#define PAGE_START ((char*)(0xffff000000800000))
#define PAGE_NUM  0x3b9c0
// pages: 0xffff000000269600 = PAGE_START - sizeof(struct page) * PAGE_NUM
```

```bash
[0]buddy_init: end: 0xffff0000001bf208, space: 0x640df8
[0]buddy_init: pages: 0xffff000000269600 = 0xffff000000800000 - 0x18 * 0x3b9c0
```
