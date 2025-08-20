# ブロックサイズを4096に拡大する

## OSが立ち上がらない

```bash
[1]main: console init
[0]buddy_init: end: 0xffff0000000bd108  // end + size = 0xFFFF000000653B08  -> PAGE_STARTを超えていた
[1]buddy_init: pages: 0x0, size: 0x596a00, PAGE_START: 0xffff000000640000   // pages: 0x0がおかしい
[1]buddy_init: sizeof(struct page): 0x18
// ここでストール
```

- pagesの領域がPAGE_STARTを超えていた
- PAGE_START を 0xffff000000660000に変更したら正常に立ち上がるようになった

```bash
[1]buddy_init: end: 0xffff0000000bd108
[1]buddy_init: pages: 0xffff0000000c9600, size: 0x596a00, PAGE_START: 0xffff000000660000
[0]mm_init: buddy init
[0]main: cpu 0 init finished
[1]main: cpu 1 init finished
[2]main: cpu 2 init finished
[0]sdhost_probe: firmware sets clock divider
[3]main: cpu 3 init finished                                             
[0]sdhost_set_ios: ios clock 400000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[0]sdhost_finish_command: error detected: CMD 0x4205, HSTS 0x40, EDM 0x10800
[0]sdhost_finish_command: command 5 timeout                              
[0]emmc_card_reset: OCR: 0xff80, 1.8v support: 0, SDHC support: 1
[0]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[0]emmc_card_reset: card CID: 0x2544d53, 0x41313647, 0x31299f69, 0x3a0138c3
[0]emmc_card_reset: RCA: 0x1234
[3]emmc_card_reset: SCR: version 3.0x, bus_widths 0x5
[3]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 1, timing 0, vdd 0, drv_type 0
[3]emmc_card_reset: found valid version 3.0x SD card
[2]dev_init: LBA of 1st block 0x20800, 0x1f800 blocks totally
[3]iinit: sb: size 1000 nblocks 963 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 36
init: starting sh
sh: argv[0] = 'sh'
sh: testenv = 'FROM_INIT'
$ ls
.              4000 1 4096      // サイズが512から4096になっている
..             4000 1 4096
cat            8000 2 38568
init           8000 3 22400
echo           8000 4 39480
mkfs           8000 5 45528
sh             8000 6 54056
utest          8000 7 17744
ls             8000 8 41304
console        0 9 0
```

## 変更したファイル (commit: f8be6d4)

```bash
$ git status
On branch mac
Changes to be committed:
  (use "git restore --staged <file>..." to unstage)
	modified:   inc/file.h
	modified:   inc/fs.h
	modified:   inc/mm.h
	modified:   kern/buddy.c
	modified:   kern/dev.c
	modified:   kern/fs.c
	modified:   kern/mm.c
	deleted:    usr/inc/files.h
	deleted:    usr/inc/param.h
	renamed:    usr/inc/fs.h -> usr/inc/usr_fs.h
	modified:   usr/src/ls/main.c
	modified:   usr/src/mkfs/main.c
```