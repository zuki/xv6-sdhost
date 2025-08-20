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