# wifiの実装

## Circleの`addon/wlan`を移植

```bash
[3]console_preinit: console_preinit ok
[3]buddy_init: buddy_init ok
[3]slab_cache_init: slab_cache_init ok
[3]cachepage_init: cachepage_init ok
[3]rand_init: rand_init ok
[3]proc_init: proc_init ok
[3]console_init: console_init ok
[3]tty_init: tty_init ok
[3]sd_init: sd_init ok

[3]init_vfs: init_vfs ok
[3]main: root_dev mount ok
[3]user_init: user_init ok
[3]timer_init: [3] timer_init ok
[0]timer_init: [0] timer_init ok
[1]timer_init: [1] timer_init ok
[3]trap_init: [3] trap_init ok
[3]main: cpu 3 init finished
[0]trap_init: [0] trap_init ok
[3]sdhost_probe: firmware sets clock divider
[1]trap_init: [1] trap_init ok
[1]main: cpu 1 init finished
[0]main: cpu 0 init finished
[2]timer_init: [2] timer_init ok
[2]trap_init: [2] trap_init ok
[2]main: cpu 2 init finished
[3]sdhost_set_ios: ios clock 400000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[3]sdhost_finish_command: error detected: CMD 0x4205, HSTS 0x40, EDM 0x10800
[3]sdhost_finish_command: command 5 timeout
[3]emmc_card_reset: OCR: 0xff80, 1.8v support: 0, SDHC support: 1
[3]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[3]emmc_card_reset: card CID: 0x27504853, 0x44333247, 0x506c5d21, 0xcc017421
[3]emmc_card_reset: RCA: 0x5048
[1]emmc_card_reset: SCR: version 3.0x, bus_widths 0x5
[1]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 1, timing 0, vdd 0, drv_type 0
[1]emmc_card_reset: found valid version 3.0x SD card
[2]sd_postinit: partition[0]: TYPE: 12, LBA = 0x800, #SECS = 0x20000
[2]sd_postinit: partition[1]: TYPE: 12, LBA = 0x20800, #SECS = 0x20000
[2]sd_postinit: partition[2]: TYPE: 131, LBA = 0x40800, #SECS = 0x20000
[2]sd_postinit: sd_postinit ok

[2]release: error: card is not locked
[2]v6_set_super: v6_sb: size 1000 nblocks 960 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 39
[2]disk_initialize: disk_initialize, drv 4
[1]usb_dev_init: Device ven424-2514, dev9-0-2 found
[1]usb_function_get_if_name: func name=int9-0-1
[1]usb_dev_init: Interface int9-0-1 found
[1]usb_dev_init: Function is not supported
[1]usb_function_get_if_name: func name=int9-0-2
[1]usb_dev_init: Interface int9-0-2 found
[1]usb_dev_factory_get_device: Using device/interface int9-0-2
[1]usb_dev_init: Device ven424-2514, dev9-0-2 found
[1]usb_function_get_if_name: func name=int9-0-1
[1]usb_dev_init: Interface int9-0-1 found
[1]usb_dev_init: Function is not supported
[1]usb_function_get_if_name: func name=int9-0-2
[1]usb_dev_init: Interface int9-0-2 found
[1]usb_dev_factory_get_device: Using device/interface int9-0-2
[3]usb_dev_init: Device ven424-7800 found
[3]usb_dev_factory_get_device: Using device/interface ven424-7800
[3]lan7800_init_macaddr: MAC address is b8:27:eb:ab:e8:48
[3]usb_standard_hub_enumerate_ports: Port 1: Device configured
[2]usb_standard_hub_enumerate_ports: Port 1: Device configured
[1]dwhc_root_port_init: Device configured
[1]usb_init: dwhc initialized

[1]net_device_register: dev=net1, type=2 (ETHERNET)
[1]usb_init: usb_init ok
[1]bcm4343_init: bcm4343_init ok
[1]malloc_dma30: malloc_dma30 returns 73d5ce0
[1]dma_init: dma chan: 5
[1]malloc_dma30: malloc_dma30 returns 73d5c90
[1]malloc_dma30: malloc_dma30 returns 73d5c40
[1]malloc_dma30: malloc_dma30 returns 73d5bf0
[1]malloc_dma30: malloc_dma30 returns 73d5ba0
123456[2]etherbcmattach:
ether_bcm_attach 0
1a
sdioinit 0
[1]gpiosel: pin[34] reg: 0x0c oval: 0x0, nval: 0x7000
[1]gpiosel: pin[35] reg: 0x0c oval: 0x7000, nval: 0x3f000
[1]gpiosel: pin[36] reg: 0x0c oval: 0x3f000, nval: 0x1ff000
[1]gpiosel: pin[37] reg: 0x0c oval: 0x1ff000, nval: 0xfff000
[1]gpiosel: pin[38] reg: 0x0c oval: 0xfff000, nval: 0x7fff000
[1]gpiosel: pin[39] reg: 0x0c oval: 0x7fff000, nval: 0x3ffff000
1
emmcinit 0123
[1]emmcinit: clock: 0xbebc200
[1]emmcinit: r=0xffff00003f300000
[1]emmcinit: control0=0x0
456 ok
2345 ocr: 0xb0ffff00 a6789abcdefg ok
bcfgreadl 18000000: 45 43 26 15
ether4330: chip 0x4345 rev 6 type 1
cfgreadl 180000fc: 00 a0 10 18
core 800 mem 0x0x18000000
core 800 mem 0x0x1c000000
core 800 ctl 0x0x18100000
core 812 mem 0x0x18001000
core 812 ctl 0x0x18101000
core 83e mem 0x0x18002000
core 83e mem 0x0x18005000
core 83e mem 0x0x0
core 83e mem 0x0x180000
core 83e mem 0x0x200000
core 83e ctl 0x0x18102000
core 83e ctl 0x0x18105000
core 83c mem 0x0x18003000
core 83c mem 0x0x8000000
core 83c ctl 0x0x18103000
core 83c ctl 0x0x18106000
core 829 mem 0x0x18004000
core 829 ctl 0x0x18104000
core 135 ctl 0x0x18000000
core 135 ctl 0x0x18001000
core 135 ctl 0x0x18002000
core 135 ctl 0x0x18003000
core 135 ctl 0x0x18004000
core 135 ctl 0x0x18005000
core 135 ctl 0x0x18107000
core 240 ctl 0x0x19000000
core 240 ctl 0x0x18108000
core 367 mem 0x0x18109000
core 366 mem 0x0x1810a000
core 301 mem 0x0x18200000
core fff mem 0x0xa0000
core fff mem 0x0x240000
core fff mem 0x0x10000000
core fff mem 0x0x18008000
core fff mem 0x0x1810e000
core fff mem 0x0x18300000
core fff mem 0x0x1a000000
core fff mem 0x0x1d000000
cfgreadl 18102800: 00 00 00 00
cfgwritel 18102408: 23 00 00 00
cfgreadl 18102408: 23 00 00 00
cfgwritel 18102800: 01 00 00 00
cfgreadl 18102800: 01 00 00 00
cfgwritel 18102408: 23 00 00 00
cfgreadl 18102408: 23 00 00 00
cfgreadl 18102408: 23 00 00 00
cfgreadl 18102800: 01 00 00 00
sbreset 0x18102000 0x23 0x1 ->cfgreadl 18102800: 01 00 00 00
cfgwritel 18102800: 00 00 00 00
cfgreadl 18102800: 00 00 00 00
cfgwritel 18102408: 21 00 00 00
cfgreadl 18102408: 21 00 00 00
cfgreadl 18102408: 21 00 00 00
cfgreadl 18102800: 00 00 00 00
0x21 0x0
cfgreadl 18101800: 01 00 00 00
cfgwritel 18101408: 07 00 00 00
cfgreadl 18101408: 07 00 00 00
cfgreadl 18101408: 07 00 00 00
cfgreadl 18101800: 01 00 00 00
sbreset 0x18101000 0x7 0x1 ->cfgreadl 18101800: 01 00 00 00
cfgwritel 18101800: 00 00 00 00
cfgreadl 18101800: 00 00 00 00
cfgwritel 18101408: 05 00 00 00
cfgreadl 18101408: 05 00 00 00
cfgreadl 18101408: 05 00 00 00
cfgreadl 18101800: 00 00 00 00
0x5 0x0
cfgreadl 18002004: 44 0b 00 00
cr4 banks b44
cfgwritel 18002040: 00 00 00 00
cfgreadl 18002044: 0f 8c 10 00
bank 0 reg 108c0f size 131072
cfgwritel 18002040: 01 00 00 00
cfgreadl 18002044: 0f 8c 10 00
bank 1 reg 108c0f size 131072
cfgwritel 18002040: 02 00 00 00
cfgreadl 18002044: 0f 8c 10 00
bank 2 reg 108c0f size 131072
cfgwritel 18002040: 03 00 00 00
cfgreadl 18002044: 03 8c 10 00
bank 3 reg 108c03 size 32768
cfgwritel 18002040: 04 00 00 00
cfgreadl 18002044: 0f 8c 10 00
bank 4 reg 108c0f size 131072
cfgwritel 18002040: 05 00 00 00
cfgreadl 18002044: 07 8c 10 00
bank 5 reg 108c07 size 65536
cfgwritel 18002040: 06 00 00 00
cfgreadl 18002044: 0f 8c 10 00
bank 6 reg 108c0f size 131072
cfgwritel 18002040: 07 00 00 00
cfgreadl 18002044: 07 8c 10 00
bank 7 reg 108c07 size 65536
ARM 0x0x18102000 D11 0x0x18101000 SOCRAM 0x0x0,0x0x0 819200 bytes @ 0x0x198000
chipclk: 40
chipclk: 61
cfgwritel 1c000058: 00 00 00 00
cfgwritel 1c00005c: 00 00 00 00
cfirmware brcmfmac43455-sdio.bin load...dir: /d/firmware, file: brcmfmac43455-sdio.bin, path: 4:/firmware/brcmfmac43455-sdio.bin
compare...  // ここでストール

//

cfirmware brcmfmac43455-sdio.bin load...
f_read 012345.6789cdefghijkn.678abcdefghijkn.678abcdefghijkn.678abcdefghijkn ok
byte: 2048, offset: 0x800
compare... off: 0x0
f_read 012345.6789cdefghijkn.678abcdefghi

// 一番長く実行されたケース

cfirmware brcmfmac43455-sdio.bin load... readchan c->off: 0x0, offset: 0x0
after seek c->off: 0x0

f_read 012345.6789cdefghijkn.678abcdefghijkn.678abcdefghi
[1]get_block: dev: 0x101, bno: 0x812, issec: 1
[1]get_block: no hit: dev: 0x101, blockno: 0x812, issec: true
[1]_find_free_entry: i: 0, last: 0xffff0000003bd1c0, ref: 0
01234 ok    // こちらは last->block = 0 なのでkmfree()は実行せず　_find_free_entry() が正常終了
[1]_load_block: recycle entry: 0xffff0000003bd1c0
[1]_read_entry: read: dev: 0x101, buffer: 0x73d0140, bno: 0x812, size: 0x200
[2]_read_entry: ok
jkn.678abcdefghijkn ok
byte: 2048, offset: 0x800

 compare... off: 0x0 readchan c->off: 0x85f78, offset: 0x0
after seek c->off: 0x0

f_read 012345.6789cdefghijkn.678abcdefghijkn.678abcdefghi
[2]get_block: dev: 0x101, bno: 0x812, issec: 1
[2]get_block: no hit: dev: 0x101, blockno: 0x812, issec: true
[2]_find_free_entry: i: 0, last: 0xffff0000003bd1f0, ref: 0
0123456   // kmfree(last->block)が帰ってこず、_find_free_entry()でストール

// bufcache->blockのkmalloc(512)を　kalloc()　または slabcache で書き換えると firmware も読み込まなくなる

cfirmware brcmfmac43455-sdio.bin load...

// kmalloc(512)に戻す。compare で get_block()が実行されない

 firmware brcmfmac43455-sdio.bin
load...
readchan c->off: 0x0, offset: 0x0, f->fptr: 0x0
after seek c->off: 0x0, f->fptr: 0x0

f_read 012345.6789cdefghi
[3]get_block: dev: 0x101, bno: 0x810, issec: 1
[3]get_block: no hit: dev: 0x101, blockno: 0x810, issec: true
[3]_find_free_entry: i: 0, last: 0xffff0000003bc130, ref: 0
01234 ok
[1]_load_block: recycle entry: 0xffff0000003bc130
[3]_read_entry: read: dev: 0x101, buffer: 0x73cd5f0, bno: 0x810, size: 0x200
[1]_read_entry: ok

jkn.678abcdefghi
[2]get_block: dev: 0x101, bno: 0x811, issec: 1
[2]get_block: no hit: dev: 0x101, blockno: 0x811, issec: true
[2]_find_free_entry: i: 0, last: 0xffff0000003bc190, ref: 0
01234 ok
[1]_load_block: recycle entry: 0xffff0000003bc190
[2]_read_entry: read: dev: 0x101, buffer: 0x73cd1d0, bno: 0x811, size: 0x200
[2]_read_entry: ok

jkn.678abcdefghi
[2]disk_read: drv: 4, buff: 0xffff0000073d1c00, sector: 0x812, count: 1
[2]get_block: dev: 0x101, bno: 0x812, issec: 1
[2]get_block: no hit: dev: 0x101, blockno: 0x812, issec: true
[2]_find_free_entry: i: 0, last: 0xffff0000003bc1c0, ref: 0
01234 ok
[2]_load_block: recycle entry: 0xffff0000003bc1c0
[2]_read_entry: read: dev: 0x101, buffer: 0x73d0140, bno: 0x812, size: 0x200
[1]_read_entry: ok
jkn.678abcdefghijkn ok
byte: 2048, offset: 0x800

 compare...
readchan c->off: 0x85f78, offset: 0x0, f->fptr: 0x85f78
after seek c->off: 0x0, f->fptr: 0x0

f_read 012345.6789cdefghi
[0]get_block: dev: 0x101, bno: 0x810, issec: 1
[0]get_block: no hit: dev: 0x101, blockno: 0x810, issec: true
[0]_find_free_entry: i: 0, last: 0xffff0000003bc160, ref: 0
012345 0xffff0000073cd3e0 67.9 ok
[0]_load_block: recycle entry: 0xffff0000003bc160
[0]_read_entry: read: dev: 0x101, buffer: 0x73cd3e0, bno: 0x810, size: 0x200
[2]_read_entry: ok

jkn.678abcdefghi
[3]get_block: dev: 0x101, bno: 0x811, issec: 1
[3]get_block: no hit: dev: 0x101, blockno: 0x811, issec: true
[3]_find_free_entry: i: 0, last: 0xffff0000003bc1c0, ref: 0
012345 0xffff0000073cb0d0 67.9 ok
[2]_load_block: recycle entry: 0xffff0000003bc1c0
```

## wlanの設定が正常終了

以下のようにether4330.cを変更

- firmwareはバインリファイルでカーネルに組み込む
- configファイルはincludeファイルに変換
- 制御ファイルはそのままファイルから読み込む

```bash
[0]init_vfs: init_vfs ok
[0]main: root_dev mount ok
[0]user_init: user_init ok
[0]timer_init: [0] timer_init ok
[1]timer_init: [1] timer_init ok
[3]timer_init: [3] timer_init ok
[0]trap_init: [0] trap_init ok
[3]trap_init: [3] trap_init ok
[0]main: cpu 0 init finished
[1]trap_init: [1] trap_init ok
[1]main: cpu 1 init finished
[3]main: cpu 3 init finished
[0]sdhost_probe: firmware sets clock divider
[2]timer_init: [2] timer_init ok
[2]trap_init: [2] trap_init ok
[2]main: cpu 2 init finished
[0]sdhost_set_ios: ios clock 400000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[0]sdhost_finish_command: error detected: CMD 0x4205, HSTS 0x40, EDM 0x10800
[0]sdhost_finish_command: command 5 timeout
[0]emmc_card_reset: OCR: 0xff80, 1.8v support: 0, SDHC support: 1
[0]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[0]emmc_card_reset: card CID: 0x27504853, 0x44333247, 0x506c5d21, 0xcc017421
[0]emmc_card_reset: RCA: 0x5048
[1]emmc_card_reset: SCR: version 3.0x, bus_widths 0x5
[1]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 1, timing 0, vdd 0, drv_type 0
[1]emmc_card_reset: found valid version 3.0x SD card
[2]sd_postinit: partition[0]: TYPE: 12, LBA = 0x800, #SECS = 0x20000
[2]sd_postinit: partition[1]: TYPE: 12, LBA = 0x20800, #SECS = 0x20000
[2]sd_postinit: partition[2]: TYPE: 131, LBA = 0x40800, #SECS = 0x20000
[2]sd_postinit: sd_postinit ok

[2]release: error: card is not locked
[3]v6_set_super: v6_sb: size 1000 nblocks 960 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 39
[3]disk_initialize: disk_initialize, drv 4
[2]forkret: mount fatfs to /d/ ok
[2]usb_dev_init: Device ven424-2514, dev9-0-2 found
[2]usb_function_get_if_name: func name=int9-0-1
[2]usb_dev_init: Interface int9-0-1 found
[2]usb_dev_init: Function is not supported
[2]usb_function_get_if_name: func name=int9-0-2
[2]usb_dev_init: Interface int9-0-2 found
[2]usb_dev_factory_get_device: Using device/interface int9-0-2
[2]usb_dev_init: Device ven424-2514, dev9-0-2 found
[2]usb_function_get_if_name: func name=int9-0-1
[2]usb_dev_init: Interface int9-0-1 found
[2]usb_dev_init: Function is not supported
[2]usb_function_get_if_name: func name=int9-0-2
[2]usb_dev_init: Interface int9-0-2 found
[2]usb_dev_factory_get_device: Using device/interface int9-0-2
[3]usb_dev_init: Device ven424-7800 found
[3]usb_dev_factory_get_device: Using device/interface ven424-7800
[3]lan7800_init_macaddr: MAC address is b8:27:eb:ab:e8:48
[1]usb_standard_hub_enumerate_ports: Port 1: Device configured
[2]usb_standard_hub_enumerate_ports: Port 1: Device configured
[2]dwhc_root_port_init: Device configured
[2]usb_init: dwhc initialized

[2]net_device_register: dev=net1, type=2 (ETHERNET)
[2]usb_init: usb_init ok
[2]malloc_dma30: malloc_dma30 returns 73d0ce0
[2]dma_init: dma chan: 5
[2]malloc_dma30: malloc_dma30 returns 73d0c90
[2]malloc_dma30: malloc_dma30 returns 73d0c40
[2]malloc_dma30: malloc_dma30 returns 73d0bf0
[2]malloc_dma30: malloc_dma30 returns 73d0ba0
emmc control 0x0 0x0 0x0
cfgreadl 18000000: 45 43 26 15
ether4330: chip 0x4345 rev 6 type 1
cfgreadl 180000fc: 00 a0 10 18
core 800 mem 0x0x18000000
core 800 mem 0x0x1c000000
core 800 ctl 0x0x18100000
core 812 mem 0x0x18001000
core 812 ctl 0x0x18101000
core 83e mem 0x0x18002000
core 83e mem 0x0x18005000
core 83e mem 0x0x0
core 83e mem 0x0x180000
core 83e mem 0x0x200000
core 83e ctl 0x0x18102000
core 83e ctl 0x0x18105000
core 83c mem 0x0x18003000
core 83c mem 0x0x8000000
core 83c ctl 0x0x18103000
core 83c ctl 0x0x18106000
core 829 mem 0x0x18004000
core 829 ctl 0x0x18104000
core 135 ctl 0x0x18000000
core 135 ctl 0x0x18001000
core 135 ctl 0x0x18002000
core 135 ctl 0x0x18003000
core 135 ctl 0x0x18004000
core 135 ctl 0x0x18005000
core 135 ctl 0x0x18107000
core 240 ctl 0x0x19000000
core 240 ctl 0x0x18108000
core 367 mem 0x0x18109000
core 366 mem 0x0x1810a000
core 301 mem 0x0x18200000
core fff mem 0x0xa0000
core fff mem 0x0x240000
core fff mem 0x0x10000000
core fff mem 0x0x18008000
core fff mem 0x0x1810e000
core fff mem 0x0x18300000
core fff mem 0x0x1a000000
core fff mem 0x0x1d000000
cfgreadl 18102800: 00 00 00 00
cfgwritel 18102408: 23 00 00 00
cfgreadl 18102408: 23 00 00 00
cfgwritel 18102800: 01 00 00 00
cfgreadl 18102800: 01 00 00 00
cfgwritel 18102408: 23 00 00 00
cfgreadl 18102408: 23 00 00 00
cfgreadl 18102408: 23 00 00 00
cfgreadl 18102800: 01 00 00 00
sbreset 0x18102000 0x23 0x1 ->cfgreadl 18102800: 01 00 00 00
cfgwritel 18102800: 00 00 00 00
cfgreadl 18102800: 00 00 00 00
cfgwritel 18102408: 21 00 00 00
cfgreadl 18102408: 21 00 00 00
cfgreadl 18102408: 21 00 00 00
cfgreadl 18102800: 00 00 00 00
0x21 0x0
cfgreadl 18101800: 01 00 00 00
cfgwritel 18101408: 07 00 00 00
cfgreadl 18101408: 07 00 00 00
cfgreadl 18101408: 07 00 00 00
cfgreadl 18101800: 01 00 00 00
sbreset 0x18101000 0x7 0x1 ->cfgreadl 18101800: 01 00 00 00
cfgwritel 18101800: 00 00 00 00
cfgreadl 18101800: 00 00 00 00
cfgwritel 18101408: 05 00 00 00
cfgreadl 18101408: 05 00 00 00
cfgreadl 18101408: 05 00 00 00
cfgreadl 18101800: 00 00 00 00
0x5 0x0
cfgreadl 18002004: 44 0b 00 00
cr4 banks b44
cfgwritel 18002040: 00 00 00 00
cfgreadl 18002044: 0f 8c 10 00
bank 0 reg 108c0f size 131072
cfgwritel 18002040: 01 00 00 00
cfgreadl 18002044: 0f 8c 10 00
bank 1 reg 108c0f size 131072
cfgwritel 18002040: 02 00 00 00
cfgreadl 18002044: 0f 8c 10 00
bank 2 reg 108c0f size 131072
cfgwritel 18002040: 03 00 00 00
cfgreadl 18002044: 03 8c 10 00
bank 3 reg 108c03 size 32768
cfgwritel 18002040: 04 00 00 00
cfgreadl 18002044: 0f 8c 10 00
bank 4 reg 108c0f size 131072
cfgwritel 18002040: 05 00 00 00
cfgreadl 18002044: 07 8c 10 00
bank 5 reg 108c07 size 65536
cfgwritel 18002040: 06 00 00 00
cfgreadl 18002044: 0f 8c 10 00
bank 6 reg 108c0f size 131072
cfgwritel 18002040: 07 00 00 00
cfgreadl 18002044: 07 8c 10 00
bank 7 reg 108c07 size 65536
ARM 0x0x18102000 D11 0x0x18101000 SOCRAM 0x0x0,0x0x0 819200 bytes @ 0x0x198000
chipclk: 40
chipclk: 61
cfgwritel 1c000058: 00 00 00 00
cfgwritel 1c00005c: 00 00 00 00
firmware brcmfmac43455-sdio.bin load...  compare... ok
config brcmfmac43455-sdio.txt load... ok
cfgwritel 18004020: ff ff ff ff
b83ef198
cfgreadl 18102800: 00 00 00 00
cfgwritel 18102408: 23 00 00 00
cfgreadl 18102408: 23 00 00 00
cfgwritel 18102800: 01 00 00 00
cfgreadl 18102800: 01 00 00 00
cfgwritel 18102408: 03 00 00 00
cfgreadl 18102408: 03 00 00 00
cfgreadl 18102408: 03 00 00 00
cfgreadl 18102800: 01 00 00 00
sbreset 0x18102000 0x3 0x1 ->cfgreadl 18102800: 01 00 00 00
cfgwritel 18102800: 00 00 00 00
cfgreadl 18102800: 00 00 00 00
cfgwritel 18102408: 01 00 00 00
cfgreadl 18102408: 01 00 00 00
cfgreadl 18102408: 01 00 00 00
cfgreadl 18102800: 00 00 00 00
0x1 0x0
enabling HT clock...chipclk: d2
cfgwritel 18004048: 00 00 04 00
cfgwritel 18004024: e0 00 00 00
cfgreadl 18004020: c0 00 00 20
readchan c->off: 0x0, offset: 0x0, f->fptr: 0x0
cfgwritel 18004020: c0 00 00 20
after seek c->off: 0x0, f->fptr: 0x0
cfgreadl 1800404c: 08 00 04 00
cfgwritel 18004040: 02 00 00 00
ether4330: firmware ready
byte: 1401, offset: 0x579
cfgreadl 18004020: 80 00 00 00
cfgwritel 18004020: 80 00 00 00
cfgreadl 1800404c: 02 00 04 00
cfgwritel 18004040: 02 00 00 00
cfgreadl 18004020: 40 00 80 00
cfgwritel 18004020: 40 00 80 00
cfgreadl 18004020: 40 00 80 00
cfgwritel 18004020: 40 00 80 00
cclose ok
cfgreadl 18004020: 40 00 80 00
cfgwritel 18004020: 40 00 80 00
ether4330: addr b8:27:eb:fe:bd:1d
cfgreadl 18004020: 40 00 80 00
cfgwritel 18004020: 40 00 80 00
cfgreadl 18004020: 40 00 80 00
cfgwritel 18004020: 40 00 80 00
cfgreadl 18004020: 40 00 80 00
cfgwritel 18004020: 40 00 80 00
cfgreadl 18004020: 40 00 80 00
cfgwritel 18004020: 40 00 80 00
cfgreadl 18004020: 40 00 80 00
cfgwritel 18004020: 40 00 80 00
cfgreadl 18004020: 40 00 80 00
cfgwritel 18004020: 40 00 80 00
cfgreadl 18004020: 40 00 80 00
cfgwritel 18004020: 40 00 80 00
cfgreadl 18004020: 40 00 80 00
cfgwritel 18004020: 40 00 80 00
cfgreadl 18004020: 40 00 80 00
cfgwritel 18004020: 40 00 80 00
cfgreadl 18004020: 40 00 80 00
cfgwritel 18004020: 40 00 80 00
cfgreadl 18004020: 40 00 80 00
cfgwritel 18004020: 40 00 80 00
cfgreadl 18004020: 40 00 80 00
cfgwritel 18004020: 40 00 80 00
cfgreadl 18004020: 40 00 80 00
cfgwritel 18004020: 40 00 80 00
cfgreadl 18004020: 40 00 80 00
cfgwritel 18004020: 40 00 80 00
cfgreadl 18004020: 40 00 80 00
cfgwritel 18004020: 40 00 80 00
[2]net_device_register: dev=net3, type=0 (DUMMY)
[2]bcm4343_init_internal: bcm4343_init_internal ok

[a] o_protoc l_negistati n ped/ (I_MP)(Iile '/d/wt_proppcic_rtgcsnf': erper:x5806 edRt)
li2anp.prot'c       // net_init()で問題が発生している
```

## wpasupplicant_init()でwpa_supplicant.confが開けなかった

- fatfsのパス名は`/d/file`ではなく`4:/file`。この変換を`config_file.c#wpa_config_read()`に追加

```bash
emmc control 0x0 0x0 0x0
ether4330: chip 0x4345 rev 6 type 1
[1]etherbcmattacr: kthread wifireader created
3k0:rfarmwiri eeder
reated
[1]etherbcmattach: kthread wifitimer created
ether4330: addr b8:27:eb:fe:bd:1d
[3]net_device_register: dev=net3, type=0 (DUMMY)
[3]bcm4343_init_internal: bcm4343_init_internal ok
[3]initialize: kthread wpa_supplicant created
[3]wpasupplicant_init: wpasupplicant_init ok
[3]net_protocol_register: type=0x0800 (IP)
[3]net_protocol_register: type=0x0806 (ARP)
[3]ip_protocol_register: type=1 (ICMP)
[3]ip_protocol_register: type=17 (UDP)
[3]ip_protocol_register: type=6 (TCP)
[3]ip_route_add: route added: network=192.168.10.0, netmask=255.255.255.0, nexthop=0.0.0.0, iface=192.168.10.110 dev=net3
[3]ip_iface_register: registered: dev=net3, unicast=192.168.10.110, netmask=255.255.255.0, broadcast=192.168.10.255
[3]ip_route_add: route added: network=0.0.0.0, netmask=0.0.0.0, nexthop=192.168.10.1, iface=192.168.10.110 dev=net3
[3]net_init: net_init ok
[3]net_device_open: dev=net3, state=up
[3]net_device_open: dev=net1, state=up    // これのlinkupを待っている。有線LANは抜いていたのでlinkupはしない
```

## wpa_supplicant_init()をnet_run()の後に移動

- 有線LANを接続した
- wpa_supplicant_init()でエラー（wpa_printf()でバグがあるようで正常出力されない）

```
emmc control 0x0 0x0 0x0
ether4330: chip 0x4345 rev 6 type 1
core 800 mem 0x0x18000000
core 800 mem 0x0x1c000000
core 800 ctl 0x0x18100000
core 812 mem 0x0x18001000
core 812 ctl 0x0x18101000
core 83e mem 0x0x18002000
core 83e mem 0x0x18005000
core 83e mem 0x0x0
core 83e mem 0x0x180000
core 83e mem 0x0x200000
core 83e ctl 0x0x18102000
core 83e ctl 0x0x18105000
core 83c mem 0x0x18003000
core 83c mem 0x0x8000000
core 83c ctl 0x0x18103000
core 83c ctl 0x0x18106000
core 829 mem 0x0x18004000
core 829 ctl 0x0x18104000
core 135 ctl 0x0x18000000
core 135 ctl 0x0x18001000
core 135 ctl 0x0x18002000
core 135 ctl 0x0x18003000
core 135 ctl 0x0x18004000
core 135 ctl 0x0x18005000
core 135 ctl 0x0x18107000
core 240 ctl 0x0x19000000
core 240 ctl 0x0x18108000
core 367 mem 0x0x18109000
core 366 mem 0x0x1810a000
core 301 mem 0x0x18200000
core fff mem 0x0xa0000
core fff mem 0x0x240000
core fff mem 0x0x10000000
core fff mem 0x0x18008000
core fff mem 0x0x1810e000
core fff mem 0x0x18300000
core fff mem 0x0x1a000000
core fff mem 0x0x1d000000
sbreset 0x18102000 0x23 0x1 ->0x21 0x0
sbreset 0x18101000 0x7 0x1 ->0x5 0x0
cr4 banks b44
bank 0 reg 108c0f size 131072
bank 1 reg 108c0f size 131072
bank 2 reg 108c0f size 131072
bank 3 reg 108c03 size 32768
bank 4 reg 108c0f size 131072
bank 5 reg 108c07 size 65536
bank 6 reg 108c0f size 131072
bank 7 reg 108c07 size 65536
ARM 0x0x18102000 D11 0x0x18101000 SOCRAM 0x0x0,0x0x0 819200 bytes @ 0x0x198000
chipclk: 40
chipclk: 61
b83ef198
sbreset 0x18102000 0x3 0x1 ->0x1 0x0
enabling HT clock...chipclk: d2
ether4330: firmware ready
ether4330: addr b8:27:eb:fe:bd:1d
[2]net_device_register: dev=net3, type=3 (WLAN)
[2]bcm4343_init_internal: bcm4343_init_internal ok
[2]net_protocol_register: type=0x0800 (IP)
[2]net_protocol_register: type=0x0806 (ARP)
[2]ip_protocol_register: type=1 (ICMP)
[2]ip_protocol_register: type=17 (UDP)
[2]ip_protocol_register: type=6 (TCP)
[2]ip_route_add: route added: network=192.168.10.0, netmask=255.255.255.0, nexthop=0.0.0.0, iface=192.168.10.104 dev=net3
[2]ip_iface_register: registered: dev=net3, unicast=192.168.10.104, netmask=255.255.255.0, broadcast=192.168.10.255
[2]ip_route_add: route added: network=0.0.0.0, netmask=0.0.0.0, nexthop=192.168.10.1, iface=192.168.10.104 dev=net3
[2]net_init: net_init ok
[2]net_device_open: dev=net3, state=up
[2]net_device_open: dev=net1, state=up
[1]netrun: running...
I1iwiasizilgcint_rnite 'wlanp' ccnf _/n/tpak
pplicant.conf' driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'kern/sdhost.c:1330: assertion failed.
```

## `sdhost.c#L1330`の`assert(host->mrq == 0);`がアサーションエラー

```bash
[1]netrun: running...
I1iwiasizilgcint_rnite 'wlanp' ccnf _/n/tpak
pplicant.conf' driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'kern/sdhost.c:1330: assertion failed.
```

- 一時的にassert()の前にerror出力して、`host->mrq=0`として実行
- wpa_printf()の代わりにdebug()を使用（必要なところは変えたほうが早いか）

```bash
[0]wpa_supplicant_init_iface: Initializing interface 'wlan0' conf '/d/wpa_supplicant.conf' driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
[2]trap: [9] unknown trap code: 34 at elr: 0xffff0000073c7fc7 with far: 0xffff0000073c7fc7
xfsffr0 00800033c5 h s ->mr==
[  sdhrs _xfqfes000p73c7fu7
comm np:(1x0
[0]?: [8] unknown trap code: 121403952 at elr: 0xffff0000073c7fc7 with far: 0xa
```

```bash
[2]trap: [9] unknown trap code: 34 at elr: 0xffff0000073c7fc7 with far: 0xffff0000073c7fc7
xfsffr0 00800033c5 h s ->mr==
[  sdhrs _xfqfes000p73c7fu7
comm np:(1x0
[0]?: [8] unknown trap code: 121403952 at elr: 0xffff0000073c7fc7 with far: 0xa
```

- SBDEBUG = 0 としてデバッグ出力を削減

```bash
[1]net_device_register: dev=net1, type=2 (ETHERNET)
[1]usb_init: usb_init ok
emmc control 0x0 0x0 0x0
ether4330: chip 0x4345 rev 6 type 1
ether4330: firmware ready
ether4330: addr b8:27:eb:fe:bd:1d
[1]net_device_register: dev=net3, type=3 (WLAN)
[1]bcm4343_init_internal: bcm4343_init_internal ok
[1]net_protocol_register: type=0x0800 (IP)
[1]net_protocol_register: type=0x0806 (ARP)
[1]ip_protocol_register: type=1 (ICMP)
[1]ip_protocol_register: type=17 (UDP)
[1]ip_protocol_register: type=6 (TCP)
[1]ip_route_add: route added: network=192.168.10.0, netmask=255.255.255.0, nexthop=0.0.0.0, iface=192.168.10.104 dev=net3
[1]ip_iface_register: registered: dev=net3, unicast=192.168.10.104, netmask=255.255.255.0, broadcast=192.168.10.255
[1]ip_route_add: route added: network=0.0.0.0, netmask=0.0.0.0, nexthop=192.168.10.1, iface=192.168.10.104 dev=net3
[1]net_init: net_init ok
[1]net_device_open: dev=net3, state=up
[1]net_device_open: dev=net1, state=up
[2]netrun: running...
[2]wpasupplicant_init: wpasupplicant_init ok
[1]wpa_supplicant_init_iface: Initializing interface 'wlan0' conf '/d/wpa_supplicant.conf' driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
[2]release:     // unlockのasserttionエラーだと思われる
```

```bash
[2]netrun: running...
[2]3p5supplicant_init: wpasupplicant_init ok
[0]wpa_supplicant_init_iface: Initializing interface 'wlan0' conf '/d/wpa_supplicant.conf' driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
[0]wpa_config_read: Reading configuration file '4:/wpa_supplicant.conf'
[1]trap: [9] unknown trap code: 34 at elr: 0xffff0000073c7f87 with far: 0xffff0000073c7f87
==]sduost_rapursme h   -  ==:
0  spsr0000800033c5
```

## wpa_config_read()にわたすパス名は`/d/wpa_supplicant.conf`のままでいい



```bash
[3]netrun: running...
[3]wpasupplicant_init: wpasupplicant_init ok
[2]wpa_supplicant_init: wpa_supplicant v2.11
add_iface 012345[2]wpa_supplicant_init_iface: Initializing interface 'wlan0' conf '/d/wpa_supplicant.conf' driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
[2]wpa_config_read: Reading configuration file '/d/wpa_supplicant.conf'
[1]sdhost_request: host->mrq: 0xffff000000140dd8, done: 0x73fe710,
sbc 0x0 opcode: 0x0, cmd 0xffffffff opcode: 0x73fe7a0
data 0x12 flag: 0x73fe7d0,
kern/sdhost.c:1336: assertion failed.
kern/drivers/console.c:264: kernel panic at cpu 2.
```
- これは間違いでassertionエラーはこれが原因だった

```bash
[2]sdhost_send_command: send_command 0xd arg 0x50480000 (flags 0x5)
[1]wpa_supplicant_init_iface: Initializing interface 'wlan0' conf '/d/wpa_supplicant.conf' driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
[2]sdhost_send_command: finish
[1]wpa_config_read: Reading configuration file '4:/wpa_supplicant.conf'
[2]sdhost_finish_command: finish_command (0xd)
[2]sdhost_finish_command: finish_command resp[0] 0x900
[2]sdhost_tasklet_finish: delete host->mrq, cmd, data
[2]mmc_request_done: request done
[2]sdhost_finish_command: finish
[2]sdhost_send_command: send_command 0x12 arg 0x40940 (flags 0x5) - read 8*512
[2]sdhost_send_command: finish
[2]sdhost_finish_command: finish_command (0x52)
[2]sdhost_finish_command: finish_command resp[0] 0x900
[2]sdhost_finish_command: finish
```

## wpa_supplicant.confの内容をプログラムで直接設定

```bash
emmc control 0x0 0x0 0x0
ether4330: chip 0x4345 rev 6 type 1
ether4330: firmware ready
ether4330: addr b8:27:eb:fe:bd:1d
[0]net_device_register: dev=net3, type=3 (WLAN)
[0]bcm4343_init_internal: bcm4343_init_internal ok
[0]net_protocol_register: type=0x0800 (IP)
[0]net_protocol_register: type=0x0806 (ARP)
[0]ip_protocol_register: type=1 (ICMP)
[0]ip_protocol_register: type=17 (UDP)
[0]ip_protocol_register: type=6 (TCP)
[0]net_init: net_init ok
[0]net_device_open: dev=net3, state=up
[0]net_device_open: dev=net1, state=up
[1]netrun: running...
[1]wpasupplicant_init: wpasupplicant_init ok
[2]wpa_supplicant_init: wpa_supplicant v2.11
add_iface 012345[2]wpa_supplicant_init_iface: Initializing interface 'wlan0' conf '/d/wpa_supplicant.conf' driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
[2]wpa_config_debug_dump_networks: Priority group 0
[2]wpa_config_debug_dump_networks:  id=0 ssid='xxx'
[2]radio_add_interface: Add interface wlan0 to a new radio N/A
kmalloc: Cannot allocate the requested size of memory 5880 ( > 4080 )   // wpa_sm_init()#sizeof(*sm)
kern/drivers/console.c:264: kernel panic at cpu 2.
```

## kmalloc()の上限(4080)を変えられないか

- 良い案が見つからなかったので、kalloc()に割り当てるページ数を指定できるようにして
  該当のkmalloc()をkalloc()に変更した（`wpa_sm_init()`のみ）
- ssidとpskを出力するデバッガ行は削除した

```bash
emmc control 0x0 0x0 0x0
ether4330: chip 0x4345 rev 6 type 1
ether4330: firmware ready
ether4330: addr b8:27:eb:fe:bd:1d
[1]net_device_register: dev=net3, type=3 (WLAN)
[1]bcm4343_init_internal: bcm4343_init_internal ok
[1]net_protocol_register: type=0x0800 (IP)
[1]net_protocol_register: type=0x0806 (ARP)
[1]ip_protocol_register: type=1 (ICMP)
[1]ip_protocol_register: type=17 (UDP)
[1]ip_protocol_register: type=6 (TCP)
[1]net_init: net_init ok
[1]net_device_open: dev=net3, state=up
[1]net_device_open: dev=net1, state=up
[1]netrun: running...
[1]wpasupplicant_init: wpasupplicant_init ok
[2]wpa_supplicant_init: wpa_supplicant v2.11
add_iface 012345
[2]wpa_supplicant_init_iface: Initializing interface 'wlan0' conf '/d/wpa_supplicant.conf' driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
[2]wpa_supplicant_init_iface: init_driver
[2]radio_add_interface: Add interface wlan0 to a new radio N/A
[2]wpas_init_driver: radio_add_interface: radio=0xffff0000073cc5b0
[2]wpa_supplicant_init_iface: init_wpa
[2]wpa_supplicant_init_iface: set_ifname
[2]wpa_supplicant_init_iface: set_fast_reauth
[2]wpa_supplicant_init_iface: lifetime
[2]wpa_supplicant_init_iface: threshold
[2]wpa_supplicant_init_iface: sattimeout
[2]wpa_supplicant_init_iface: set_param
[2]wpa_supplicant_init_iface: get_hw_feature
[2]wpa_supplicant_init_iface: iget_capa
[2]wpa_supplicant_init_iface: bss_select
[2]wpa_supplicant_init_iface: driver_init
[2]wpa_supplicant_init_iface: set_sountry
[2]wpa_driver_xv6_set_country: Setting country code to 'JP'   // ここでストール
```

- strcspn()のバグだった. muslのソースで置き換えた。

```bash
[2]wpasupplicant_init: wpasupplicant_init ok
[0]wpa_supplicant_init: wpa_supplicant v2.11
add_iface 012345[0]wpa_supplicant_init_iface: Initializing interface 'wlan0' conf '/d/wpa_supplicant.conf' driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
[0]wpa_supplicant_init_iface: init_driver
[0]radio_add_interface: Add interface wlan0 to a new radio N/A
[0]wpas_init_driver: radio_add_interface: radio=0xffff0000073c39c0
[0]wpa_supplicant_init_iface: init_wpa
[0]wpa_supplicant_init_iface: set_ifname
[0]wpa_supplicant_init_iface: set_fast_reauth
[0]wpa_supplicant_init_iface: lifetime
[0]wpa_supplicant_init_iface: threshold
[0]wpa_supplicant_init_iface: sattimeout
[0]wpa_supplicant_init_iface: set_param
[0]wpa_supplicant_init_iface: get_hw_feature
[0]wpa_supplicant_init_iface: iget_capa
[0]wpa_supplicant_init_iface: bss_select
[0]wpa_supplicant_init_iface: driver_init
[0]wpa_supplicant_driver_init: Own MAC address: b8:27:eb:fe:bd:1d
[0]wpa_supplicant_init_iface: set_country
[0]wpa_driver_xv6_set_country: Setting country code to 'JP'
[0]etherbcmctl: buf: country JP, n=0
[0]strtok_r: s: 0x0xffff0000073c3690, s[0]: 0x63
[0]strtok_r: end: 0x0xffff0000073c3697, end[0]: 0x20
[0]strtok_r: *save_ptr: 0x0xffff0000073c3698, s: country
[0]parsecmd: f[0]: 'country'
[0]str_replace: source: country, find: \x20, replace:
[0]parsecmd: arg[0]: 'country
[0]strtok_r: s: 0x0xffff0000073c3698, s[0]: 0x4a
[0]strtok_r: end: 0x0xffff0000073c369a, end[0]: 0x00
[0]parsecmd: f[1]: 'JP'
[0]str_replace: source: JP, find: \x20, replace:
[0]parsecmd: arg[1]: 'JP
[0]parsecmd: f[2]: '(null)'
[0]etherbcmctl: cb f[0]: country, f[1]: JP
[0]etherbcmctl: cmdtab index: 18, cmd: country, maxarg: 2
[0]wpa_supplicant_init_iface: wps_init
[0]wpa_supplicant_init_iface: init_eapol
[0]sm_SUPP_PAE_DISCONNECTED_Enter: EAPOL: SUPP_PAE entering state DISCONNECTED
[0]wpa_supplicant_port_cb: EAPOL: Supplicant port status: Unauthorized
[0]sm_KEY_RX_NO_KEY_RECEIVE_Enter: EAPOL: KEY_RX entering state NO_KEY_RECEIVE
[0]sm_SUPP_BE_INITIALIZE_Enter: EAPOL: SUPP_BE entering state INITIALIZE
```

## f_open, f_readをfat_open, vfs_readで書き換える

- バイナリファイルやincludeファイルから読み込んでいたのをファイルから読み込む方式に戻す
- しかし、fatfsのファイルが2度読むとエラーは元通りだった。
- 結局、malloc()のバグであった事が判明した

```bash
open brcmfmac43455-sdio.bin with fd 0, file: 0x443cc0
readchan: offset=0x0
[2]_load_block: recycle entry: 0xffff000000443890
[2]_load_block: entry->block: 0xffff0000073ccde0
[2]_load_block: recycle entry: 0xffff0000004438d0
[2]_load_block: entry->block: 0xffff0000073ccbd0
[1]_load_block: recycle entry: 0xffff000000443910
[1]_load_block: entry->block: 0xffff0000073cc9c0
[1]_load_block: recycle entry: 0xffff000000443950
[1]_load_block: entry->block: 0xffff0000073cc7b0
[3]_load_block: recycle entry: 0xffff000000443990
[3]_load_block: entry->block: 0xffff0000073cc5a0
rreadchan: rbyte: 2048, new_offset: 0x800
readchan: offset=0x800
kmalloc 124.568 ok
[3]_load_block: recycle entry: 0xffff0000004439d0
kmalloc 124.9.57                   // kmalloc()でストール
```

- 2度読みができるようなったがcclose() (vfs_close())でストール
- f_open, f_closeで問題ないと思われるのでこちらも戻すことにする

```bash
firmware brcmfmac43455-sdio.bin load...  compare... [0]upload: break
cclose 0
```

## f_open, f_closeを戻す

```bash
emmc control 0x0 0x0 0x0
ether4330: chip 0x4345 rev 6 type 1
core 800 mem 0x0x18000000
core 800 mem 0x0x1c000000
core 800 ctl 0x0x18100000
core 812 mem 0x0x18001000
core 812 ctl 0x0x18101000
core 83e mem 0x0x18002000
core 83e mem 0x0x18005000
core 83e mem 0x0x0
core 83e mem 0x0x180000
core 83e mem 0x0x200000
core 83e ctl 0x0x18102000
core 83e ctl 0x0x18105000
core 83c mem 0x0x18003000
core 83c mem 0x0x8000000
core 83c ctl 0x0x18103000
core 83c ctl 0x0x18106000
core 829 mem 0x0x18004000
core 829 ctl 0x0x18104000
core 135 ctl 0x0x18000000
core 135 ctl 0x0x18001000
core 135 ctl 0x0x18002000
core 135 ctl 0x0x18003000
core 135 ctl 0x0x18004000
core 135 ctl 0x0x18005000
core 135 ctl 0x0x18107000
core 240 ctl 0x0x19000000
core 240 ctl 0x0x18108000
core 367 mem 0x0x18109000
core 366 mem 0x0x1810a000
core 301 mem 0x0x18200000
core fff mem 0x0xa0000
core fff mem 0x0x240000
core fff mem 0x0x10000000
core fff mem 0x0x18008000
core fff mem 0x0x1810e000
core fff mem 0x0x18300000
core fff mem 0x0x1a000000
core fff mem 0x0x1d000000
sbreset 0x18102000 0x23 0x1 ->0x21 0x0
sbreset 0x18101000 0x7 0x1 ->0x5 0x0
cr4 banks b44
bank 0 reg 108c0f size 131072
bank 1 reg 108c0f size 131072
bank 2 reg 108c0f size 131072
bank 3 reg 108c03 size 32768
bank 4 reg 108c0f size 131072
bank 5 reg 108c07 size 65536
bank 6 reg 108c0f size 131072
bank 7 reg 108c07 size 65536
ARM 0x0x18102000 D11 0x0x18101000 SOCRAM 0x0x0,0x0x0 819200 bytes @ 0x0x198000
chipclk: 40
chipclk: 61
firmware brcmfmac43455-sdio.bin load...  compare... ok
config brcmfmac43455-sdio.txt load...  compare... ok
b83ef198
sbreset 0x18102000 0x3 0x1 ->0x1 0x0
enabling HT clock...chipclk: d2
ether4330: firmware ready
ether4330: addr b8:27:eb:fe:bd:1d
[2]net_device_register: dev=net3, type=3 (WLAN)
[2]bcm4343_init_internal: bcm4343_init_internal ok
[2]net_protocol_register: type=0x0800 (IP)
[2]net_protocol_register: type=0x0806 (ARP)
[2]ip_protocol_register: type=1 (ICMP)
[2]ip_protocol_register: type=17 (UDP)
[2]ip_protocol_register: type=6 (TCP)
[2]ip_route_add: route added: network=192.168.10.0, netmask=255.255.255.0, nexthop=0.0.0.0, iface=192.168.10.104 dev=net3
[2]ip_iface_register: registered: dev=net3, unicast=192.168.10.104, netmask=255.255.255.0, broadcast=192.168.10.255
[2]ip_route_add: route added: network=0.0.0.0, netmask=0.0.0.0, nexthop=192.168.10.1, iface=192.168.10.104 dev=net3
[2]net_init: net_init ok
[2]net_device_open: dev=net3, state=up
[2]net_device_open: dev=net1, state=up
[3]netrun: running...
kern/sdhost.c:1349: assertion failed.
kern/drivers/console.c:264: kernel panic at cpu 3.
```

## wpa_suplicant_wpa_supplicant_init_iface()を調査中

```bash
[3]wpa_supplicant_main: start
wpa_supplicant v2.11Initializing interface 'wlan0' conf '4:/wpa_supplicant.conf'
  driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
012Configuration file '4:/wpa_supplicant.conf' -> '4:/wpa_supplicant.conf'
4Reading configuration file '4:/wpa_supplicant.conf'
[3]trap: [8] unknown trap code: 34 at elr: 0x2c070201022320a6 with far: 0x2c070201022320a6

4fp: 0xffff0000073db278 0[1]trap: [8] unknown trap code: 34 at elr: 0x660e6ed08dccc267 with far: 0x660e6ed08dccc267
```

- プログラムカウンタに変な値がセットされ、alignmetエラーが発生している
- elrとfarは常に同じ値になっており、実行するたびにこれらの値は変わる。
- f_open()の最初の行で発生している
- スタックオーバーフローくさい

## p->kstackを4ページにしたところ、このエラーはなくなった

- おなじみのsdhost.cのアサーションエラー

```bash
f_open 012345               // 4:/firmware/配下のファイルの場合
follow_path 01234569.abcjkmn.abcj ok
679ayzBDEFGLMNOPXY ok

[0]wpa_supplicant_main: start
wwpa_supplicant v2.11Initializing interface 'wlan0' conf '4:/wpa_supplicant.conf'
  driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
012
Configuration file '4:/wpa_supplicant.conf' -> '4:/wpa_supplicant.conf'
4
f_open 012345
follow_path 01234569.ab         // 4:/wpa_supplicant.conf
kern/sdhost.c:1349: assertion failed.
kern/drivers/console.c:264: kernel panic at cpu 2.
```

- wpa_supplicant.confを`4:/firmware/`配下においたが結果は同じ

```bash
Configuration file '4:/firmware/wpa_supplicant.conf' -> '4:/firmware/wpa_supplicant.conf'
4
f_open 012345
follow_path 01234569.ab
kern/sdhost.c:1349: assertion failed.
kern/drivers/console.c:264: kernel panic at cpu 1.
```

```bash
[0]sdhost_request: pre-command: 0x133ba8, host->mrq: 0x52, done: 0x73f8710,
  host->mrq->sbc 0x0 opcode: 0x0,
  host->mrq->cmd 0xffffffff opcode: 0x73f87a0
  host->mrq->data 0xabab8eaa00000012 flag: 0x73f87d0,
kern/sdhost.c:1349: assertion failed.
```

- circleのメインスレッドのスタックサイズは32ページ
- kstackを32ページ（40ページもテスト）に増やしても4ページでも結果は同じ
- opcodeは最大255 (0xff)、cmdのポインタアドレスは8バイトのハズ、mrq->stopが出力されない
- sdhost.cで`host->irq = IRQ_SDIO`をセットしたが変化なし

```bash
Configuration file '4:/wpa_supplicant.conf' -> '4:/wpa_supplicant.conf'
[0]sdhost_request: pre-command: 0x133c58, host->mrq: 0x52, done: 0x7380710
        host->mrq->sbc 0x0 opcode: 0x0
        host->mrq->cmd 0xffffffff opcode: 0x73807a0
        host->mrq->data 0xaaaaaaaa00000012 flag: 0x73807d0

[0]sdhost_request: mrq->sbc 0xffff000000133d10 opcode: 0x0
        mrq->cmd 0xffffffff opcode: 0x71bfbb0
        mrq->data 0xd flag: 0x0

kern/sdhost.c:1359: assertion failed.
```

- 発行したコマンドを出力

```bash
[2]emmc_issue_command: issuing command 13
[2]emmc_issue_command: issuing command 17
ether4330: addr b8:27:eb:fe:bd:1d
[1]net_device_register: dev=net3, type=3 (WLAN)
[1]bcm4343_init_internal: bcm4343_init_internal ok
[1]net_protocol_register: type=0x0800 (IP)
[1]net_protocol_register: type=0x0806 (ARP)
[1]ip_protocol_register: type=1 (ICMP)
[1]ip_protocol_register: type=17 (UDP)
[1]ip_protocol_register: type=6 (TCP)
[1]net_init: net_init ok
[1]net_device_open: dev=net3, state=up
[1]net_device_open: dev=net1, state=up
[2]netrun: running...
[3]wpa_supplicant_main: start
[2a_sup_iicuet omm1nI:iisalizgngoimted 13
iver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
012
Configuration file '4:/wpa_supplicant.conf' -> '4:/wpa_supplicant.conf'
4[3]emmc_issue_command: issuing command 13
[3]sdhost_request: pre-command: 0x133d18, host->mrq: 0x52, done: 0x73f8700
        host->mrq->sbc 0x0 opcode: 0x0
        host->mrq->cmd 0xffffffff opcode: 0x73f8790
        host->mrq->data 0x12 flag: 0x73f87c0

[3]sdhost_request: mrq->sbc 0xffff000000133dd0 opcode: 0x0
        mrq->cmd 0xffffffff opcode: 0x73afba0
        mrq->data 0xd flag: 0x0

kern/sdhost.c:1359: assertion failed.
```

- カーネルスタックの設定ミスを修正、string.hの関数をmuslのコードに変更をしたが
  結果は変わらず

```bash
[1]wpa_supplicant_main: start
wpa_supplicant v2.11Initializing interface 'wlan0' conf '4:/wpa_supplicant.conf'
  driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
012
Configuration file '4:/wpa_supplicant.conf' -> '4:/wpa_supplicant.conf'
4[1]sdhost_request: pre-command: 0x12fe48, host->mrq: 0x52, done: 0x73fe730
        host->mrq->sbc 0x0 opcode: 0x0
        host->mrq->cmd 0xffffffff opcode: 0x73fe7c0
        host->mrq->data 0xaaa2aaaa00000012 flag: 0x73fe7f8

[1]sdhost_request: mrq->sbc 0xffff00000012ff00 opcode: 0x0
        mrq->cmd 0xffffffff opcode: 0x73bebc0
        mrq->data 0xd flag: 0x0

kern/sdhost.c:1359: assertion failed.
```

## 有線LANのデータ読み込みをキックするkthreadの実行を止めたらwpa_supplicantの初期化が進んだ

- デバッグ出力を削除したら、たまにしか進まなくなった
- 進まない場合、アサートエラー前の出力がでない

```bash
[2]net_device_register: dev=net1, type=2 (ETHERNET)
[2]usb_init: usb_init ok
emmc control 0x0 0x0 0x0
ether4330: chip 0x4345 rev 6 type 1
[1]kthread_stub: call kthread wifireader
itimerethers330: firm kthreeddwi
ether4330: addr b8:27:eb:fe:bd:1d
[1]net_device_register: dev=net3, type=3 (WLAN)
[1]bcm4343_init_internal: bcm4343_init_internal ok
[1]net_protocol_register: type=0x0800 (IP)
[1]net_protocol_register: type=0x0806 (ARP)
[1]ip_protocol_register: type=1 (ICMP)
[1]ip_protocol_register: type=17 (UDP)
[1]ip_protocol_register: type=6 (TCP)
[1]net_init: net_init ok
[1]net_device_open: dev=net3, state=up
[1]net_device_open: dev=net1, state=up
[3]netrun: running...
[2]kthread_stub: call kthread wpa_supplicant
[2]wpa_supplicant_main: start
wpa_supplicant v2.11
Initializing interface 'wlan0' conf '4:/wpa_supplicant.conf'
  driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
ws_init_iface 012
3 Configuration file '4:/wpa_supplicant.conf' -> '4:/wpa_supplicant.conf'
4
opened. res: 0
get_line
, esz:: 512get_line 0[3]1d o:t_r
buflen: 512 len: 2 buf[buflen-1]: 0 buf[len-1]: a
[2] s: # wpa_supplicant.conf
, size: 512
buflen: 512 len: 22 buf[buflen-1]: 0 buf[len-1]: a
[3] s: #
, size: 512
buflen: 512 len: 2 buf[buflen-1]: 0 buf[len-1]: a
[4] s:
, size: 512
buflen: 512 len: 1 buf[buflen-1]: 0 buf[len-1]: a
[5] s: country=JP
, size: 512
buflen: 512 len: 11 buf[buflen-1]: 0 buf[len-1]: a
123456 ok
pos: country=JP
country='JP'wpa_config_get_line 0[6] s:
, size: 512
buflen: 512 len: 1 buf[buflen-1]: 0 buf[len-1]: a
[7] s: network={
, size: 512
buflen: 512 len: 10 buf[buflen-1]: 0 buf[len-1]: a
123456 ok
pos: network={
wpa_config_get_line 0[8] s:     ssid="MSRS_TDF_A5_A11"
, size: 2000
buflen: 2000 len: 24 buf[buflen-1]: 0 buf[len-1]: a
123456 ok
```

-
```bash
[0]kthread_stub: call kthread wpa_supplicant
[0]wpa_supplicant_main: start
wpa_supplicant v2.11
Initializing interface 'wlan0' conf '4:/wpa_supplicant.conf'
  driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
ws_init_iface 012
3 Configuration file '4:/wpa_supplicant.conf' -> '4:/wpa_supplicant.conf'
4
[0]sdhost_request:
        host->mrq: 0xffff00000012fb70, done: 0x73fe730
        host->mrq->sbc 0x0 opcode: 0x0
        host->mrq->cmd 0xffffffff opcode: 0x73fe7c0
        host->mrq->data 0x12 flag: 0x73fe7f8

[0]sdhost_request: mrq: 0xffff00000012fc18
        mrq->sbc 0xffff0000073beb30 opcode: 0x0
        mrq->cmd 0xffffffff opcode: 0x73bebc0
        mrq->data 0xd flag: 0x0

kern/sdhost.c:1362: assertion failed.
kern/drivers/console.c:264: kernel panic at cpu 0.
$
```

- アサーションエラーが発生した後、いったんスイッチを切り、再度実行すると
  少し進むことが多い（進まない場合もある）

```bash
[3]kthread_stub: call kthread wpa_supplicant
[3]wpa_supplicant_main: start
wpa_supplicant v2.11
Initializing interface 'wlan0' conf '4:/wpa_supplicant.conf'
  driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
ws_init_iface 012
3 Configuration file '4:/wpa_supplicant.conf' -> '4:/wpa_supplicant.conf'
4
[1]sdhostcruntry='JP'
Line: 7 - start of a new network block
=== ssid dump ===
+------+-------------------------------------------------+------------------+
| 0000 | 4d 53 52 53 5f 54 44 46 5f 41 35 5f 41 31 31    | MSRS_TDF_A5_A11  |
+------+-------------------------------------------------+------------------+
```

## NULLポインタをkmfree()しているのが原因だった

- `kern/wlan/hostap/wpa_supplicant/config.c#L126`
- kmfree(void *ap)を修正してapがNULLの場合は何もせずreturnとした

```bash
[2]netrun: running...
[3]kthread_stub: call kthread wpa_supplicant
[3]wpa_supplicant_main: start
wpa_supplicant v2.11
Initializing interface 'wlan0' conf '4:/wpa_supplicant.conf'
  driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
Configuration file '4:/wpa_supplicant.conf' -> '4:/wpa_supplicant.conf'
country='JP'
Line: 7 - start of a new network block
ssid: 4d 53 52 53 5f 54 44 46 5f 41 35 5f 41 31 31
proto: 0x2
key_mgmt: 0x2
Priority group 0
   id=0 ssid='MSRS_TDF_A5_A11'
Add interface wlan0 to a new radio N/A
Failed to attach pkt_type filter
Own MAC address: b8:27:eb:fe:bd:1d
RSN: flushing PMKID list in the driver
Setting scan request: 0.100000 sec
Setting country code to 'JP'
ether4330: cmd 263 error status -2
=== ether4330: cmd error dump ===
+------+-------------------------------------------------+------------------+
| 0000 | 30 00 cf ff 14 00 00 0c 00 3b 00 00 07 01 00 00 | 0........;...... |
| 0010 | 00 00 00 00 01 00 13 00 fe ff ff ff 63 6f 75 6e | ............coun |
| 0020 | 74 72 79 00 4a 00 aa a8 ff ff ff ff 4a 00 8a a2 | try.J.......J... |
+------+-------------------------------------------------+------------------+

wlcmd error
kern/dma.c:160: assertion failed.
```

- ether4343.c#wlsetcountry()を修正

```bash
[3]netrun: running...
[2]kthread_stub: call kthread wpa_supplicant
[2]wpa_supplicant_main: start
wpa_supplicant v2.11
Initializing interface 'wlan0' conf '4:/wpa_supplicant.conf'
  driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
Configuration file '4:/wpa_supplicant.conf' -> '4:/wpa_supplicant.conf'
country='JP'
Line: 7 - start of a new network block
ssid: 4d 53 52 53 5f 54 44 46 5f 41 35 5f 41 31 31
proto: 0x2
key_mgmt: 0x2
$ Priority group 0
   id=0 ssid='MSRS_TDF_A5_A11'
Add interface wlan0 to a new radio N/A
Failed to attach pkt_type filter
Own MAC address: b8:27:eb:fe:bd:1d
RSN: flushing PMKID list in the driver
Setting scan request: 0.100000 sec
Setting country code to 'JP'
[2]bcm4343_control: command: 'country JP'
[2]wlsetcountry: ccode: JP, country: JP
EAPOL: SUPP_PAE entering state DISCONNECTED
EAPOL: Supplicant port status: Unauthorized
EAPOL: KEY_RX entering state NO_KEY_RECEIVE
EAPOL: SUPP_BE entering state INITI'
```

```bash
[2]wpa_supplicant_main: start
wpa_supplicant v2.11
Initializing interface 'wlan0' conf '4:/wpa_supplicant.conf'
  driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
Configuration file '4:/wpa_supplicant.conf' -> '4:/wpa_supplicant.conf'
country='JP'
Line: 7 - start of a new network block
ssid: 4d 53 52 53 5f 54 44 46 5f 41 35 5f 41 31 31
proto: 0x2
key_mgmt: 0x2
$ Priority group 0
   id=0 ssid='MSRS_TDF_A5_A11'
Add interface wlan0 to a new radio N/A
Failed to attach pkt_type filter
Own MAC address: b8:27:eb:fe:bd:1d
RSN: flushing PMKID list in the driver
Setting scan request: 0.100000 sec
Setting country code to 'JP'
[2]bcm4343_control: command: 'country JP'
[2]wlsetcountry: ccode: JP, country: JP
EAPOL: SUPP_PAE entering state DISCONNECTED
EAPOL: Supplicant port status: Unauthorized
EAPOL: KEY_RX entering state NO_KEY_RECEIVE
EAPOL: SUPP_BE entering state INITIALIZE
eapol_sm_abortSupp 01234
eap_sm_abort 012345 ok
5 ok
EAP: EAP entering state DISABLED
eapol_sm_abortSupp 01234
eap_sm_abort 012345 ok
5 ok
eapol_sm_abortSupp 01234
eap_sm_abort 012345 ok
5 ok
Added interface wlan0
State: DISCONNECTED -> DISCONNECTED
State: DISCONNECTED -> SCANNING
Starting AP scan for wildcard SSID
Add radio work 'scan'@ffff0000073d874
eap_sm_abort 012345 ok
5 ok
Starting radio work 'scan'@ffff0000073d8710 after 6099.610000 second wait
[2]bcm4343_control: command: 'escan 5'
[2]bcm4343_control: command: 'escan 0'
Event SCAN_RESULTS (3) received
```

- ether4330.cで起動しているkthreadのスタックサイズを32とした

```bash
[2]wpa_supplicant_main: start
wpa_supplicant v2.11
Initializing interface 'wlan0' conf '4:/wpa_supplicant.conf'
  driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
Configuration file '4:/wpa_supplicant.conf' -> '4:/wpa_supplicant.conf'
country='JP'
Line: 7 - start of a new network block
ssid: 4d 53 52 53 5f 54 44 46 5f 41 35 5f 41 31 31
proto: 0x2
key_mgmt: 0x2
Priority group 0
   id=0 ssid='MSRS_TDF_A5_A11'
Add interface wlan0 to a new radio N/A
Failed to attach pkt_type filter
Own MAC address: b8:27:eb:fe:bd:1d
RSN: flushing PMKID list in the driver
Setting scan request: 0.100000 sec
Setting country code to 'JP'
[1]bcm4343_control: command: 'country JP'
[1]wlsetcountry: ccode: JP, country: JP
EAPOL: SUPP_PAE entering state DISCONNECTED
EAPOL: Supplicant port status: UnauthorizedEAPOL: KEY_RX entering state NO_KEY_RECEIVE
EAPOL: SUPP_BE entering state INITIALIZE
EAP: EAP entering state DISABLED
Added interface wlan0
State: DISCONNECTED -> DISCONNECTED
State: DISCONNECTED -> SCANNING
Starting AP scan for wildcard SSID
Add radio work 'scan'@ffff0000073d8710
First radio work item in the queue - schedule start immediately

 radio_work_check_next 01345 ok
EAPOL: disable timer tick
Starting radio work 'scan'@ffff0000073d8710 after 5999.600000 second wait
[3]bcm4343_control: command: 'escan 5'
[3]bcm4343_control: command: 'escan 0'
Event SCAN_RESULTS (3) received
scan result 012
wpa_supplicant_event_scan_results 0
_wpa_supplicant_event_scan_results 0123
wpa_supplicant_get_scan_results 0
wpa_drv_get_scan_results 02
wpa_driver_xv6_get_scan_results2 01234      // driver_xv6.c#bcm4343_recv_scan_result()でストール
```

- enqueueしたエントリがdequeueで出てこず、別の何かが返る

```bash
[1]wpa_supplicant_main: start
wpa_supplicant v2.11
Initializing interface 'wlan0' conf '4:/wpa_supplicant.conf'
  driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
Configuration file '4:/wpa_supplicant.conf' -> '4:/wpa_supplicant.conf'
country='JP'
Line: 7 - start of a new network block
ssid: 4d 53 52 53 5f 54 44 46 5f 41 35 5f 41 31 31
proto: 0x2
key_mgmt: 0x2
$ Priority group 0
   id=0 ssid='MSRS_TDF_A5_A11'
Add interface wlan0 to a new radio N/A
Failed to attach pkt_type filter
Own MAC address: b8:27:eb:fe:bd:1d
RSN: flushing PMKID list in the driver
Setting scan request: 0.100000 sec
Setting country code to 'JP'
[1]bcm4343_control: command: 'country JP'
[1]wlsetcountry: ccode: JP, country: JP
eap_peer_sm_init ok
EAPOL: SUPP_PAE entering state DISCONNECTED
EAPOL: Supplicant port status: Unauthorized
EAPOL: KEY_RX entering state NO_KEY_RECEIVE
EAPOL: SUPP_BE entering state INITIALIZE
EAP: EAP entering state DISABLED
eapol_sm_init ok
wpa_supplicant_init_eapol ok
Added interface wlan0
State: DISCONNECTED -> DISCONNECTED
State: DISCONNECTED -> SCANNING
Starting AP scan for wildcard SSID
Add radio work 'scan'@ffff0000073d8710
First radio work item in the queue - schedule start immediately

 radio_work_check_next 01345 ok
Starting radio work 'scan'@ffff0000073d8710 after 0.210000 second wait
[2]bcm4343_control: command: 'escan 5'
[1]bcm4343_scan_result_recv: scan_queue: 0xffff0000073bfda0, &scan_queue: 0xffff0000073dae68
[1]bcm4343_scan_result_recv: enqueue: entry: 0xffff0000073479b0, len=0x1e6, q->size: 0
[1]bcm4343_scan_result_recv: enqueue: q->size: 1
[1]bcm4343_scan_result_recv: scan_queue: 0xffff0000073bfda0, &scan_queue: 0xffff0000073dae68
[1]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007347330, len=0x1e6, q->size: 1
[1]bcm4343_scan_result_recv: enqueue: q->size: 2
[1]bcm4343_scan_result_recv: scan_queue: 0xffff0000073bfda0, &scan_queue: 0xffff0000073dae68
[1]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007346430, len=0x19e, q->size: 2
[1]bcm4343_scan_result_recv: enqueue: q->size: 3
EAPOL: disable timer tick
[3]bcm4343_scan_result_recv: scan_queue: 0xffff0000073bfda0, &scan_queue: 0xffff0000073dae68
[3]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007345530, len=0x242, q->size: 3
[3]bcm4343_scan_result_recv: enqueue: q->size: 4
[3]bcm4343_scan_result_recv: scan_queue: 0xffff0000073bfda0, &scan_queue: 0xffff0000073dae68
[3]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007346cb0, len=0x1e6, q->size: 4
[3]bcm4343_scan_result_recv: enqueue: q->size: 5
[1]bcm4343_scan_result_recv: scan_queue: 0xffff0000073bfda0, &scan_queue: 0xffff0000073dae68
[1]bcm4343_scan_result_recv: enqueue: entry: 0xffff0000073cc010, len=0x1a6, q->size: 5
[1]bcm4343_scan_result_recv: enqueue: q->size: 6
[0]bcm4343_scan_result_recv: scan_queue: 0xffff0000073bfda0, &scan_queue: 0xffff0000073dae68
[0]bcm4343_scan_result_recv: enqueue: entry: 0xffff0000073ce4c0, len=0x1ea, q->size: 6
[0]bcm4343_scan_result_recv: enqueue: q->size: 7
[0]bcm4343_scan_result_recv: scan_queue: 0xffff0000073bfda0, &scan_queue: 0xffff0000073dae68
[0]bcm4343_scan_result_recv: enqueue: entry: 0xffff0000073cde40, len=0x1ea, q->size: 7
[0]bcm4343_scan_result_recv: enqueue: q->size: 8
[1]bcm4343_scan_result_recv: scan_queue: 0xffff0000073bfda0, &scan_queue: 0xffff0000073dae68
[1]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007344eb0, len=0x1ea, q->size: 8
[1]bcm4343_scan_result_recv: enqueue: q->size: 9
[0]bcm4343_scan_result_recv: scan_queue: 0xffff0000073bfda0, &scan_queue: 0xffff0000073dae68
[0]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007344830, len=0x16e, q->size: 9
[0]bcm4343_scan_result_recv: enqueue: q->size: 10
[1]bcm4343_scan_result_recv: scan_queue: 0xffff0000073bfda0, &scan_queue: 0xffff0000073dae68
[1]bcm4343_scan_result_recv: enqueue: entry: 0xffff0000073441b0, len=0x1be, q->size: 10
[1]bcm4343_scan_result_recv: enqueue: q->size: 11
[1]bcm4343_scan_result_recv: scan_queue: 0xffff0000073bfda0, &scan_queue: 0xffff0000073dae68
[1]bcm4343_scan_result_recv: enqueue: entry: 0xffff0000073539b0, len=0x1ea, q->size: 11
[1]bcm4343_scan_result_recv: enqueue: q->size: 12
[0]bcm4343_scan_result_recv: scan_queue: 0xffff0000073bfda0, &scan_queue: 0xffff0000073dae68
[0]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007353330, len=0x1ea, q->size: 12
[0]bcm4343_scan_result_recv: enqueue: q->size: 13
[3]bcm4343_scan_result_recv: scan_queue: 0xffff0000073bfda0, &scan_queue: 0xffff0000073dae68
[3]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007352cb0, len=0x1a6, q->size: 13
[3]bcm4343_scan_result_recv: enqueue: q->size: 14
[3]bcm4343_scan_result_recv: scan_queue: 0xffff0000073bfda0, &scan_queue: 0xffff0000073dae68
[3]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007352630, len=0x1de, q->size: 14
[3]bcm4343_scan_result_recv: enqueue: q->size: 15
[3]bcm4343_scan_result_recv: scan_queue: 0xffff0000073bfda0, &scan_queue: 0xffff0000073dae68  //
[3]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007351fb0, len=0xe, q->size: 15
[3]bcm4343_scan_result_recv: enqueue: q->size: 16
[2]bcm4343_control: command: 'escan 0'
Event SCAN_RESULTS (3) received
scan result 012
wpa_supplicant_event_scan_results 0
_wpa_supplicant_event_scan_results 0123
wpa_supplicant_get_scan_results 0
wpa_drv_get_scan_results 02
wpa_driver_xv6_get_scan_results2 01234
[2]bcm4343_recv_scan_result: scan_queue: 0xffff00000737f3a0, &scan_queue: 0xffff0000073dae08  // 0x60 アドレスが違う
[2]bcm4343_recv_scan_result: dequeue: q->size: 8
[2]bcm4343_recv_scan_result: dequeue: entry: 0xffff0000000bbed0, len=0xa9bf7bfd, q->size: 7
                // 0xffff0000000bbed0 は is_connected() のアドレス
```

- bcm4343のアドレスも異なる（bcm4343と&scan_queueのアドレスはどちらも0x60だけ違う）

```bash
[3]bcm4343_scan_result_recv: bcm4343: 0xffff0000073dae20, &scan_queue: 0xffff0000073dae68
[3]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007351fb0, len=0xe, q->size: 14
[1]bcm4343_control: command: 'escan 0'
Event SCAN_RESULTS (3) received
scan result 012
wpa_supplicant_event_scan_results 0
_wpa_supplicant_event_scan_results 0123
wpa_supplicant_get_scan_results 0
wpa_drv_get_scan_results 02
wpa_driver_xv6_get_scan_results2 01234
[1]bcm4343_recv_scan_result: bcm4343: 0xffff0000073dadc0, &scan_queue: 0xffff0000073dae08
[1]bcm4343_recv_scan_result: dequeue: entry: 0xffff0000073dadc0, len=0x73daea0, q->size: 7
```

- struct net_deviceとstruct bcm4343の関係を修正

- net_deviceのprivにbcm4343をセット
- bcm4343にvoid *dataを追加し、ここにドライバ固有データを置く（いらない可能性あり）

```bash
[3]net_device_register: dev=net1, type=2 (ETHERNET)
[3]usb_init: usb_init ok
emmc control 0x0 0x0 0x0
ether4330: chip 0x4345 rev 6 type 1
[1]kthread_stub: call kthread wifireader
itimereead_stu0: callwath eea yi
ether4330: addr b8:27:eb:fe:bd:1d
[1]net_device_register: dev=net3, type=3 (WLAN)
[1]net_protocol_register: type=0x0800 (IP)
[1]net_protocol_register: type=0x0806 (ARP)
[1]ip_protocol_register: type=1 (ICMP)
[1]ip_protocol_register: type=17 (UDP)
[1]ip_protocol_register: type=6 (TCP)
[1]net_init: net_init ok
[1]netrun: open net3
[1]net_device_open: dev=net3, state=up
[1]netrun: open net1
[1]net_device_open: dev=net1, state=up
[1]netrun: running...
[2]kthread_stub: call kthread wpa_supplicant
[2]wpa_supplicant_main: start
wpa_supplicant v2.11
Initializing interface 'wlan0' conf '4:/wpa_supplicant.conf'
  driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
Configuration file '4:/wpa_supplicant.conf' -> '4:/wpa_supplicant.conf'
country='JP'
Line: 7 - start of a new network block
ssid: 4d 53 52 53 5f 54 44 46 5f 41 35 5f 41 31 31
proto: 0x2
key_mgmt: 0x2
$ Priority group 0
   id=0 ssid='MSRS_TDF_A5_A11'
wpa_s->drv_priv: ffff0000073db0c0
Add interface wlan0 to a new radio N/A
Failed to attach pkt_type filter
Own MAC address: c0:b0:3d:07:00:00
RSN: flushing PMKID list in the driver
Setting scan request: 0.100000 sec
Setting country code to 'JP'
[2]bcm4343_control: command: 'country JP'
[2]wlsetcountry: ccode: JP, country: JP
eap_peer_sm_init ok
EAPOL: SUPP_PAE entering state DISCONNECTED
EAPOL: Supplicant port status: Unauthorized
EAPOL: KEY_RX entering state NO_KEY_RECEIVE
EAPOL: SUPP_BE entering state INITIALIZE
EAP: EAP entering state DISABLED
eapol_sm_init ok
wpa_supplicant_init_eapol ok
Added interface wlan0
State: DISCONNECTED -> DISCONNECTED
State: DISCONNECTED -> SCANNING
Starting AP scan for wildcard SSID
Add radio work 'scan'@ffff0000073ce370
First radio work item in the queue - schedule start immediately

 radio_work_check_next 01345 ok
Starting radio work 'scan'@ffff0000073ce370 after 0.210000 second wait
[1]bcm4343_control: command: 'escan 5'
[0]bcm4343_scan_result_recv: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[0]bcm4343_scan_result_recv: enqueue: entry: 0xffff0000073cdd10, len=0x1e6, q->size: 0
EAPOL: disable timer tick
[0]bcm4343_scan_result_recv: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[0]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007347130, len=0x242, q->size: 1
[0]bcm4343_scan_result_recv: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[0]bcm4343_scan_result_recv: enqueue: entry: 0xffff0000073cc5a0, len=0x1ea, q->size: 2
[2]bcm4343_scan_result_recv: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[2]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007346ab0, len=0x1ea, q->size: 3
[3]bcm4343_scan_result_recv: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[3]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007346430, len=0x1ea, q->size: 4
[2]bcm4343_scan_result_recv: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[2]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007345db0, len=0x16e, q->size: 5
[2]bcm4343_scan_result_recv: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[2]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007345730, len=0x1ea, q->size: 6
[3]bcm4343_scan_result_recv: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[3]bcm4343_scan_result_recv: enqueue: entry: 0xffff0000073450b0, len=0x1be, q->size: 7
[3]bcm4343_scan_result_recv: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[3]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007344a30, len=0x1a6, q->size: 8
[3]bcm4343_scan_result_recv: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[3]bcm4343_scan_result_recv: enqueue: entry: 0xffff0000073443b0, len=0x1ea, q->size: 9
[2]bcm4343_scan_result_recv: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[2]bcm4343_scan_result_recv: enqueue: entry: 0xffff0000073539b0, len=0x16e, q->size: 10
[0]bcm4343_scan_result_recv: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[0]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007353330, len=0x1de, q->size: 11
[0]bcm4343_scan_result_recv: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[0]bcm4343_scan_result_recv: enqueue: entry: 0xffff000007352cb0, len=0xe, q->size: 12
[1]bcm4343_control: command: 'escan 0'
Event SCAN_RESULTS (3) received
scan result 012
wpa_supplicant_event_scan_results 0
_wpa_supplicant_event_scan_results 0123
wpa_supplicant_get_scan_results 0
wpa_drv_get_scan_results 02
wpa_driver_xv6_get_scan_results2 01234
[1]bcm4343_recv_scan_result: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58               // bcm4343, scan_queue共にpushとpopで
[1]bcm4343_recv_scan_result: dequeue: entry: 0xffff0000073cdd10, len=0x1e6, q->size: 12                 // 同じアドレスとなった
.56^789ab[1]bcm4343_recv_scan_result: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[1]bcm4343_recv_scan_result: dequeue: entry: 0xffff000007347130, len=0x242, q->size: 11
.56^789ab[1]bcm4343_recv_scan_result: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[1]bcm4343_recv_scan_result: dequeue: entry: 0xffff0000073cc5a0, len=0x1ea, q->size: 10
.56^789ab[1]bcm4343_recv_scan_result: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[1]bcm4343_recv_scan_result: dequeue: entry: 0xffff000007346ab0, len=0x1ea, q->size: 9
.56^789ab[1]bcm4343_recv_scan_result: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[1]bcm4343_recv_scan_result: dequeue: entry: 0xffff000007346430, len=0x1ea, q->size: 8
.56^789ab[1]bcm4343_recv_scan_result: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[1]bcm4343_recv_scan_result: dequeue: entry: 0xffff000007345db0, len=0x16e, q->size: 7
.56^789ab[1]bcm4343_recv_scan_result: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[1]bcm4343_recv_scan_result: dequeue: entry: 0xffff000007345730, len=0x1ea, q->size: 6
.56^789ab[1]bcm4343_recv_scan_result: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[1]bcm4343_recv_scan_result: dequeue: entry: 0xffff0000073450b0, len=0x1be, q->size: 5
.56^789ab[1]bcm4343_recv_scan_result: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[1]bcm4343_recv_scan_result: dequeue: entry: 0xffff000007344a30, len=0x1a6, q->size: 4
.56^789ab[1]bcm4343_recv_scan_result: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[1]bcm4343_recv_scan_result: dequeue: entry: 0xffff0000073443b0, len=0x1ea, q->size: 3
.56^789ab[1]bcm4343_recv_scan_result: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[1]bcm4343_recv_scan_result: dequeue: entry: 0xffff0000073539b0, len=0x16e, q->size: 2
.56^789ab[1]bcm4343_recv_scan_result: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[1]bcm4343_recv_scan_result: dequeue: entry: 0xffff000007353330, len=0x1de, q->size: 1
.56^789ab[1]bcm4343_recv_scan_result: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[1]bcm4343_recv_scan_result: dequeue: entry: 0xffff000007352cb0, len=0xe, q->size: 0
..56
[1]bcm4343_recv_scan_result: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[3]queue_pop: no entry in queue                                                                 // queue_popはNULLを返している
[1]bcm4343_recv_scan_result: dequeue: entry: 0x0, len=0x58000540, q->size: 0, isnull : no       // NULL判定されない
[2]trap: SP alignment fault exception at elr: 0x5555555575555555 with far: 0x5555555575555555
```

- NULL判定にqueue.num==0を追加

```bash
.56^789ab[3]bcm4343_recv_scan_result: bcm4343: 0xffff0000073dae10, &scan_queue: 0xffff0000073dae58
[3]bcm4343_recv_scan_result: dequeue: entry: 0xffff000007352cb0, len=0xe, q->size: 0, isnull : no
[3]bcm4343_recv_scan_result: no more result                                                     // NULL判定ok
 ok
 ok; 1234.56.56.56.56.56.56.56.56.56.56.56.56.56789ab
BSS: Start scan result update 1
c.
BSS: Add new id 0 BSSID f8:b7:97:87:2c:df SSID 'MSRS_TDF_A5_A11' freq 5210
.
BSS: f8:b7:97:87:2c:df has multiple entries in the scan results - select the most current one
489c
New scan results available (own=0 ext=0)
defgijlmnopqrstu
Radio work 'scan'@ffff0000073d8700 done in 3.650000 secondsradio_work_free('scan'@ffff0000073d8700): num_active_works --> 0
 radio_work_check_next 012vyz ok
Selecting BSS from priority group 00: f8:b7:97:87:2c:df ssid='MSRS_TDF_A5_A11' wpa_ie_len=0 rsn_ie_len=20 caps=0x431 level=65502 freq=5210  wps   selected based on0
First radio work item in the queue - schedule start immediately
radio_work_check_next 01345 ok
135. ok
456
radio_work_check_next 01345 ok
 ok
Starting radio work 'connect'@ffff0000073d8c80 after 0.210000 second wait
WPA: clearing own WPA/RSN IE
RSN: clearing own RSNXE
RSN: PMKSA cache search - network_ctx=ffff0000073cca60 try_opportunistic=0 akmp=0x0RSN: Search for BSSID f8:b7:9
00 0f ac 02 00 00
WPA: clearing AP RSNXE
WPA: AP group 0x10 network profile group 0x18; available group 0x10
WPA: using GTK CCMP
WPA: AP pairwise 0x10 network profile pairwise 0x18; av
00 0f ac 02 0c 00
RSN: Set own RSNXE default: Automatic auth_alg selection: 0x1
No supported operating classes IE to add
Trying to associate with f8:b7:97:87:2c:df (SSID='MSRS_TDF_A5_t
State: SCANNING -> ASSOCIATING
Limit connection to BSSID f8:b7:97:87:2c:df freq=5210 MHz based on scan results (bssid_set=0 wps=0)
[3]bcm4343_control: command: 'join MSRS_TDF_A5_A1 % 0 %%'
join MSRS_TDF_A5_A1 % 0 %%
bad bssid
kern/wlan/p9proc.c:-1274501865
```

- joinの引数の作成部分を修正

```bash

[1]net_device_register: dev=net1, type=2 (ETHERNET)
[1]usb_init: usb_init ok
emmc control 0x0 0x0 0x0
ether4330: chip 0x4345 rev 6 type 1
[1]kthread_stub: call kthread wifireader
[3]kthrehdrs33b: cirlwkre ead y
itimer
ther4330: addr b8:27:eb:fe:bd:1d
[0]net_device_register: dev=net3, type=3 (WLAN)
[0]net_protocol_register: type=0x0800 (IP)
[0]net_protocol_register: type=0x0806 (ARP)
[0]ip_protocol_register: type=1 (ICMP)
[0]ip_protocol_register: type=17 (UDP)
[0]ip_protocol_register: type=6 (TCP)
[0]net_init: net_init ok
[0]netrun: open net3
[0]net_device_open: dev=net3, state=up
[0]netrun: open net1
[0]net_device_open: dev=net1, state=up
[0]netrun: running...
[1]kthread_stub: call kthread wpa_supplicant
[1]wpa_supplicant_main: start
wpa_supplicant v2.11

Initializing interface 'wlan0' conf '4:/wpa_supplicant.conf'
  driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'

Configuration file '4:/wpa_supplicant.conf' -> '4:/wpa_supplicant.conf'

country='JP'

Line: 7 - start of a new network block

ssid: 4d 53 52 53 5f 54 44 46 5f 41 35 5f 41 31 31
proto: 0x2

key_mgmt: 0x2

$ Priority group 0

   id=0 ssid='MSRS_TDF_A5_A11'

wpa_s->drv_priv: ffff0000073db0c0

Add interface wlan0 to a new radio N/A

Failed to attach pkt_type filter

Own MAC address: c0:b0:3d:07:00:00

RSN: flushing PMKID list in the driver

Setting scan request: 0.100000 sec

Setting country code to 'JP'

[1]bcm4343_control: command: 'country JP'
[1]wlsetcountry: ccode: JP, country: JP
eap_peer_sm_init ok

EAPOL: SUPP_PAE entering state DISCONNECTED

EAPOL: Supplicant port status: Unauthorized

EAPOL: KEY_RX entering state NO_KEY_RECEIVE

EAPOL: SUPP_BE entering state INITIALIZE

EAP: EAP entering state DISABLED

eapol_sm_init ok

wpa_supplicant_init_eapol ok

Added interface wlan0

State: DISCONNECTED -> DISCONNECTED

State: DISCONNECTED -> SCANNING

Starting AP scan for wildcard SSID

Add radio work 'scan'@ffff0000073d8700

First radio work item in the queue - schedule start immediately

Starting radio work 'scan'@ffff0000073d8700 after 0.210000 second wait

[3]bcm4343_control: command: 'escan 5'
EAPOL: disable timer tick

[3]bcm4343_control: command: 'escan 0'
Event SCAN_RESULTS (3) received

[3]bcm4343_recv_scan_result: queue num 0
BSS: Start scan result update 1

BSS: Add new id 0 BSSID f8:b7:97:87:2c:df SSID 'MSRS_TDF_A5_A11' freq 5210
BSS: f8:b7:97:87:2c:df has multiple entries in the scan results - select the most current one
Previous last_update: 8.110000 (freq 5210)
New last_update: 8.110000 (freq 5210)
Ignore this BSS entry since the previous update looks more current
BSS: Add new id 1 BSSID fa:b7:97:87:2c:df SSID '\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' freq 5210
BSS: Add new id 2 BSSID f8:b7:97:87:2c:de SSID 'MSRS_TDF_A2_S04' freq 2462
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 8.110000 (freq 2462)
New last_update: 8.110000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 8.110000 (freq 2462)
New last_update: 8.110000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: Add new id 3 BSSID fa:b7:97:87:2c:de SSID '\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' freq 2452
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 8.110000 (freq 2462)
New last_update: 8.110000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 8.110000 (freq 2462)
New last_update: 8.110000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 8.110000 (freq 2462)
New last_update: 8.110000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: Add new id 4 BSSID a0:95:7f:d3:b4:3c SSID 'A0957FD3B43D-2G' freq 2437
BSS: a0:95:7f:d3:b4:3c has multiple entries in the scan results - select the most current one
Previous last_update: 8.110000 (freq 2437)
New last_update: 8.110000 (freq 2437)
Ignore this BSS entry since the previous update looks more current
BSS: a0:95:7f:d3:b4:3c has multiple entries in the scan results - select the most current one
Previous last_update: 8.110000 (freq 2437)
New last_update: 8.110000 (freq 2437)
Ignore this BSS entry since the previous update looks more current
BSS: Add new id 5 BSSID 56:9b:49:d3:e7:f2 SSID 'xg100n-d3e7f1-2' freq 2462
BSS: last_scan_res_used=%zu/%zu
New scan results available (own=0 ext=0)

Radio work 'scan'@ffff0000073d8700 done in 3.410000 seconds
radio_work_free('scan'@ffff0000073d8700): num_active_works --> 0
Selecting BSS from priority group 0
0: f8:b7:97:87:2c:df ssid='MSRS_TDF_A5_A11' wpa_ie_len=0 rsn_ie_len=20 caps=0x431 level=65499 freq=5210  wps
   selected based on RSN IE
MLD: No Multi-Link element
   selected BSS f8:b7:97:87:2c:df ssid='MSRS_TDF_A5_A11'
Considering connect request: reassociate: 0
  selected: f8:b7:97:87:2c:df  bssid: 00:00:00:00:00:00  pending: 00:00:00:00:00:00
  wpa_state: SCANNING  ssid=ffff0000073cca60  current_ssid=0
Request association with f8:b7:97:87:2c:df
No ongoing scan/p2p-scan found to abort
Add radio work 'connect'@ffff0000073d8c80

First radio work item in the queue - schedule start immediately

Starting radio work 'connect'@ffff0000073d8c80 after 0.200000 second wait

WPA: clearing own WPA/RSN IE
RSN: clearing own RSNXE
RSN: PMKSA cache search - network_ctx=ffff0000073cca60 try_opportunistic=0 akmp=0x0
RSN: Search for BSSID f8:b7:97:87:2c:df
RSN: No PMKSA cache entry found
RSN: using IEEE 802.11i/D9.0
WPA: Selected cipher suites: group 16 pairwise 16 key_mgmt 2 proto 2
WPA: Selected mgmt group cipher 32
WPA: clearing AP WPA IE
WPA: set AP RSN IE: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 00 00
WPA: clearing AP RSNXE
WPA: AP group 0x10 network profile group 0x18; available group 0x10
WPA: using GTK CCMP
WPA: AP pairwise 0x10 network profile pairwise 0x18; available pairwise 0x10
WPA: using PTK CCMP
WPA: AP key_mgmt 0x2 network profile key_mgmt 0x2; available key_mgmt 0x2
WPA: using KEY_MGMT WPA-PSK
WPA: AP mgmt_group_cipher 0x20 network profile mgmt_group_cipher 0x0; available mgmt_group_cipher 0x0
WPA: not using MGMT group cipher
WPA: Set own WPA IE default: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 0c 00
RSN: Set own RSNXE default: Automatic auth_alg selection: 0x1
No supported operating classes IE to add
Trying to associate with f8:b7:97:87:2c:df (SSID='MSRS_TDF_A5_A11' freq=5210 MHz)

Cancelling scan request

State: SCANNING -> ASSOCIATING

Limit connection to BSSID f8:b7:97:87:2c:df freq=5210 MHz based on scan results (bssid_set=0 wps=0)
[3]bcm4343_control: command: 'join MSRS_TDF_A5_A11 f8b797872cdf 0 30140100000fac040100000fac040100000fac020c00
'
join MSRS_TDF_A5_A11 f8b797872cdf 0 30140100000fac040100000fac040100000fac020c00
bad bssid
kern/wlan/p9proc.c:63: assertion failed.
kern/drivers/console.c:264: kernel panic at cpu 3.
```

- bssidを`f8:b7:97:87:2c:df` とした

```bash
[3]net_device_register: dev=net1, type=2 (ETHERNET)
[3]usb_init: usb_init ok
emmc control 0x0 0x0 0x0
ether4330: chip 0x4345 rev 6 type 1
[3]kthread_stub: call kthread wifireader
[2]kthether43u0: farl kthread wifitimer
ether4330: addr b8:27:eb:fe:bd:1d
[0]net_device_register: dev=net3, type=3 (WLAN)
[0]net_protocol_register: type=0x0800 (IP)
[0]net_protocol_register: type=0x0806 (ARP)
[0]ip_protocol_register: type=1 (ICMP)
[0]ip_protocol_register: type=17 (UDP)
[0]ip_protocol_register: type=6 (TCP)
[0]net_init: net_init ok
[0]netrun: open net3
[0]net_device_open: dev=net3, state=up
[0]netrun: open net1
[0]net_device_open: dev=net1, state=up
[0]netrun: running...
[1]kthread_stub: call kthread wpa_supplicant
[1]wpa_supplicant_main: start
wpa_supplicant v2.11
Initializing interface 'wlan0' conf '4:/wpa_supplicant.conf'
  driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
Configuration file '4:/wpa_supplicant.conf' -> '4:/wpa_supplicant.conf'country='JP'
Line: 7 - start of a new network block
ssid: 4d 53 52 53 5f 54 44 46 5f 41 35 5f 41 31 31
proto: 0x2
key_mgmt: 0x2
$ Priority group 0
   id=0 ssid='MSRS_TDF_A5_A11'
wpa_s->drv_priv: ffff0000073db0c0
Add interface wlan0 to a new radio N/A
Failed to attach pkt_type filter
Own MAC address: c0:b0:3d:07:00:00                  // own MACアドレスが変わっている
RSN: flushing PMKID list in the driver
Setting scan request: 0.100000 sec
Setting country code to 'JP'
[1]bcm4343_control: command: 'country JP'
[1]wlsetcountry: ccode: JP, country: JP
eap_peer_sm_init ok
EAPOL: SUPP_PAE entering state DISCONNECTED
EAPOL: Supplicant port status: Unauthorized
EAPOL: KEY_RX entering state NO_KEY_RECEIVE
EAPOL: SUPP_BE entering state INITIALIZE
EAP: EAP entering state DISABLED
eapol_sm_init ok
wpa_supplicant_init_eapol ok
Added interface wlan0
State: DISCONNECTED -> DISCONNECTED
State: DISCONNECTED -> SCANNING
Starting AP scan for wildcard SSID
Add radio work 'scan'@ffff0000073d8700
First radio work item in the queue - schedule start immediately
Starting radio work 'scan'@ffff0000073d8700 after 0.210000 second wait
[3]bcm4343_control: command: 'escan 5'
EAPOL: disable timer tick
[3]bcm4343_control: command: 'escan 0'
Event SCAN_RESULTS (3) received
[3]bcm4343_recv_scan_result: queue num 0
BSS: Start scan result update 1
BSS: Add new id 0 BSSID f8:b7:97:87:2c:df SSID 'MSRS_TDF_A5_A11' freq 5210
BSS: f8:b7:97:87:2c:df has multiple entries in the scan results - select the most current one
Previous last_update: 7.570000 (freq 5210)
New last_update: 7.570000 (freq 5210)
Ignore this BSS entry since the previous update looks more current
BSS: Add new id 1 BSSID fa:b7:97:87:2c:df SSID '\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' freq 5210
BSS: Add new id 2 BSSID a0:95:7f:d3:b4:3e SSID 'A0957FD3B43D-5G' freq 5210
BSS: Add new id 3 BSSID f8:b7:97:87:2c:de SSID 'MSRS_TDF_A2_S04' freq 2462
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 7.570000 (freq 2462)
New last_update: 7.570000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 7.570000 (freq 2462)
New last_update: 7.570000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 7.570000 (freq 2462)
New last_update: 7.570000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 7.570000 (freq 2462)
New last_update: 7.570000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: Add new id 4 BSSID fa:b7:97:87:2c:de SSID '\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' freq 2452
BSS: fa:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 7.570000 (freq 2452)
New last_update: 7.570000 (freq 2452)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 7.570000 (freq 2462)
New last_update: 7.570000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: Add new id 5 BSSID 54:9b:49:d3:e7:f2 SSID 'xg100n-d3e7f1-1' freq 2462
BSS: Add new id 6 BSSID 56:9b:49:d3:e7:f2 SSID 'xg100n-d3e7f1-2' freq 2462
BSS: last_scan_res_used=%zu/%zu
New scan results available (own=0 ext=0)
Radio work 'scan'@ffff0000073d8700 done in 3.400000 seconds
radio_work_free('scan'@ffff0000073d8700): num_active_works --> 0
Selecting BSS from priority group 0
0: f8:b7:97:87:2c:df ssid='MSRS_TDF_A5_A11' wpa_ie_len=0 rsn_ie_len=20 caps=0x431 level=65496 freq=5210  wps
   selected based on RSN IE
MLD: No Multi-Link element
   selected BSS f8:b7:97:87:2c:df ssid='MSRS_TDF_A5_A11'
Considering connect request: reassociate: 0
  selected: f8:b7:97:87:2c:df  bssid: 00:00:00:00:00:00  pending: 00:00:00:00:00:00
  wpa_state: SCANNING  ssid=ffff0000073cca60  current_ssid=0
Request association with f8:b7:97:87:2c:df
No ongoing scan/p2p-scan found to abort
Add radio work 'connect'@ffff0000073d8c80
First radio work item in the queue - schedule start immediately
Starting radio work 'connect'@ffff0000073d8c80 after 0.210000 second wait
WPA: clearing own WPA/RSN IE
RSN: clearing own RSNXE
RSN: PMKSA cache search - network_ctx=ffff0000073cca60 try_opportunistic=0 akmp=0x0
RSN: Search for BSSID f8:b7:97:87:2c:df
RSN: No PMKSA cache entry found
RSN: using IEEE 802.11i/D9.0
WPA: Selected cipher suites: group 16 pairwise 16 key_mgmt 2 proto 2
WPA: Selected mgmt group cipher 32
WPA: clearing AP WPA IE
WPA: set AP RSN IE: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 00 00
WPA: clearing AP RSNXE
WPA: AP group 0x10 network profile group 0x18; available group 0x10
WPA: using GTK CCMP
WPA: AP pairwise 0x10 network profile pairwise 0x18; available pairwise 0x10
WPA: using PTK CCMP
WPA: AP key_mgmt 0x2 network profile key_mgmt 0x2; available key_mgmt 0x2
WPA: using KEY_MGMT WPA-PSK
WPA: AP mgmt_group_cipher 0x20 network profile mgmt_group_cipher 0x0; available mgmt_group_cipher 0x0
WPA: not using MGMT group cipher
WPA: Set own WPA IE default: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 0c 00
RSN: Set own RSNXE default: Automatic auth_alg selection: 0x1
No supported operating classes IE to add
Trying to associate with f8:b7:97:87:2c:df (SSID='MSRS_TDF_A5_A11' freq=5210 MHz)
Cancelling scan request
State: SCANNING -> ASSOCIATING
Limit connection to BSSID f8:b7:97:87:2c:df freq=5210 MHz based on scan results (bssid_set=0 wps=0)
[3]bcm4343_control: command: 'join MSRS_TDF_A5_A11 f8:b7:97:87:2c:df 0 30140100000fac040100000fac040100000fac020c00'
Event ASSOC (0) received
State: ASSOCIATING -> ASSOCIATED
Associated to a new BSS: BSSID=a3:66:7c:29:0e:4d    // ここでBSSIDが違うものになっている
[2]bcm4343_recv_scan_result: no more result
BSS: Start scan result update 2
Associated with a3:66:7c:29:0e:4d
WPA: Association event - clear replay counter
WPA: Clear old PTK
EAPOL: External notification - portEnabled=0
EAPOL: External notification - portValid=0
EAPOL: External notification - EAP success=0
EAPOL: External notification - portEnabled=1
EAPOL: SUPP_PAE entering state CONNECTING
EAPOL: enable timer tick
EAPOL: SUPP_BE entering state IDLE
EAP: EAP entering state INITIALIZE
EAP: EAP entering state IDLE
Setting authentication timeout: 10 sec 0 usec
Cancelling scan request
Setting authentication timeout: 10 sec 0 usec
EAPOL: External notification - EAP success=0
EAPOL: External notification - EAP fail=0
EAPOL: External notification - portControl=Auto
RX EAPOL from f8:b7:97:87:2c:df (encrypted=-1)
RX EAPOL: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
01 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 0c a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
Setting authentication timeout: 10 sec 0 usec
IEEE 802.1X RX: version=1 type=3 length=117
WPA: RX EAPOL-Key: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
01 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 0c a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
  EAPOL-Key type=2
  key_info 0x8a (ver=2 keyidx=0 rsvd=0 Pairwise Ack)
  key_length=16 key_data_length=22
  replay_counter: 00 00 00 00 00 00 00 01
  key_nonce: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 0c a7 4d 58
  key_iv: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  key_rsc: 00 00 00 00 00 00 00 00
  key_id (reserved): 00 00 00 00 00 00 00 00
  key_mic: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
EAP: EAP entering state DISABLED
WPA: RX message 1 of 4-Way Handshake from f8:b7:97:87:2c:df (ver=2)
RSN: msg 1/4 key data: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
WPA: PMKID in EAPOL-Key: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
RSN: PMKID from Authenticator: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
RSN: no matching PMKID found
State: ASSOCIATED -> 4WAY_HANDSHAKE
WPA: Failed to get random data for SNonce
Request to deauthenticate - bssid=a3:66:7c:29:0e:4d pending_bssid=00:00:00:00:00:00 reason=1 (UNSPECIFIED) state=4WAY_HANDSHAKE valid_links=0x0 ap_mld_addr=0
Event DEAUTH (11) received
Deauthentication notification
 * reason 1 (UNSPECIFIED) locally_generated=1
Deauthentication frame IE(s): CTRL-EVENT-DISCONNECTED bssid=a3:66:7c:29:0e:4d reason=1 locally_generated=1
WPA: 4-Way Handshake failed - pre-shared key may be incorrect
CTRL-EVENT-SSID-TEMP-DISABLED id=0 ssid="MSRS_TDF_A5_A11" auth_failures=1 duration=10 reason=WRONG_KEY
Auto connect enabled: try to reconnect (wps=0/0 wpa_state=7)
Setting scan request: 0.100000 sec
Radio work 'connect'@ffff0000073d8c80 done in 3.220000 seconds
radio_work_free('connect'@ffff0000073d8c80): num_active_works --> 0
Added BSSID a3:66:7c:29:0e:4d into ignore list, ignoring for 10 seconds
Consecutive connection failures: 1 --> request scan in 100 ms
Ignore new scan request for 0.100000 sec since an earlier request is scheduled to trigger sooner
WPA: Clear old PMK and PTK
Disconnect event - remove keys
State: 4WAY_HANDSHAKE -> DISCONNECTED
EAPOL: External notification - portEnabled=0
EAPOL: SUPP_PAE entering state DISCONNECTED
EAPOL: Supplicant port status: Unauthorized
EAPOL: SUPP_BE entering state INITIALIZE
EAPOL: External notification - portValid=0
EAPOL: External notification - EAP success=0
State: DISCONNECTED -> DISCONNECTED
EAPOL: External notification - portEnabled=0
EAPOL: External notification - portValid=0
Ignore new scan request for 5.000000 sec since an earlier request is scheduled to trigger sooner
State: DISCONNECTED -> SCANNING
Starting AP scan for wildcard SSID
Add radio work 'scan'@ffff0000073d8700
First radio work item in the queue - schedule start immediately
Starting radio work 'scan'@ffff0000073d8700 after 0.020000 second wait
[3]bcm4343_control: command: 'escan 5'
RX EAPOL from f8:b7:97:87:2c:df (encrypted=-1)
RX EAPOL: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
02 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 0c a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 c1 29 96 85 16 87 5d b2 e8 66 07 92 37 61 36
b4 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
Not associated - Delay processing of received EAPOL frame (state=SCANNING connected_addr=00:00:00:00:00:00)
RX EAPOL from f8:b7:97:87:2c:df (encrypted=-1)
RX EAPOL: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
04 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 0c a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 8d f8 de fe 97 8e 1e 61 f8 40 46 7e 9a bb 00
59 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
Not associated - Delay processing of received EAPOL frame (state=SCANNING connected_addr=00:00:00:00:00:00)
RX EAPOL from f8:b7:97:87:2c:df (encrypted=-1)
RX EAPOL: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
06 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 0c a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 ba e9 71 76 98 57 58 6c ea b1 97 13 d4 21 cd
c4 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
Not associated - Delay processing of received EAPOL frame (state=SCANNING connected_addr=00:00:00:00:00:00)
Event DEAUTH (11) received
Deauthentication notification
Auto connect enabled: try to reconnect (wps=0/0 wpa_state=3)
Do not request new immediate scan
WPA: Clear old PMK and PTK
Disconnect event - remove keys
State: SCANNING -> DISCONNECTED
EAPOL: External notification - portEnabled=0
EAPOL: External notification - portValid=0
Event DISASSOC (1) received
Disassociation notification
Auto connect enabled: try to reconnect (wps=0/0 wpa_state=0)
Do not request new immediate scan
WPA: Clear old PMK and PTK
Disconnect event - remove keys
State: DISCONNECTED -> DISCONNECTED
EAPOL: External notification - portEnabled=0
EAPOL: External notification - portValid=0
```

- os_get_random()を修正

```bash
[2]wpa_supplicant_main: start
wpa_supplicant v2.11
Initializing interface 'wlan0' conf '4:/wpa_supplicant.conf'
  driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
Configuration file '4:/wpa_supplicant.conf' -> '4:/wpa_supplicant.conf'
country='JP'
Line: 7 - start of a new network block
ssid: 4d 53 52 53 5f 54 44 46 5f 41 35 5f 41 31 31
proto: 0x2
key_mgmt: 0x2
Priority group 0
   id=0 ssid='MSRS_TDF_A5_A11'
wpa_s->drv_priv: ffff0000073db0c0
Add interface wlan0 to a new radio N/A
Failed to attach pkt_type filter
Own MAC address: c0:b0:3d:07:00:00
RSN: flushing PMKID list in the driver
Setting scan request: 0.100000 sec
Setting country code to 'JP'
[2]bcm4343_control: command: 'country JP'
[2]wlsetcountry: ccode: JP, country: JP
eap_peer_sm_init ok
EAPOL: SUPP_PAE entering state DISCONNECTED
EAPOL: Supplicant port status: Unauthorized
EAPOL: KEY_RX entering state NO_KEY_RECEIVE
EAPOL: SUPP_BE entering state INITIALIZE
EAP: EAP entering state DISABLED
eapol_sm_init ok
wpa_supplicant_init_eapol ok
Added interface wlan0
State: DISCONNECTED -> DISCONNECTED
State: DISCONNECTED -> SCANNING
Starting AP scan for wildcard SSID
Add radio work 'scan'@ffff0000073d8700
First radio work item in the queue - schedule start immediately
Starting radio work 'scan'@ffff0000073d8700 after 0.210000 second wait
[3]bcm4343_control: command: 'escan 5'
EAPOL: disable timer tick
[3]bcm4343_control: command: 'escan 0'
Event SCAN_RESULTS (3) received
[3]bcm4343_recv_scan_result: queue num 0
BSS: Start scan result update 1
BSS: Add new id 0 BSSID f8:b7:97:87:2c:df SSID 'MSRS_TDF_A5_A11' freq 5210
BSS: f8:b7:97:87:2c:df has multiple entries in the scan results - select the most current one
Previous last_update: 7.580000 (freq 5210)
New last_update: 7.580000 (freq 5210)
Ignore this BSS entry since the previous update looks more current
BSS: Add new id 1 BSSID fa:b7:97:87:2c:df SSID '\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' freq 5210
BSS: Add new id 2 BSSID d8:21:da:38:8e:c5 SSID 'NSD3K-8EC0-a' freq 5690
BSS: Add new id 3 BSSID f8:b7:97:87:2c:de SSID 'MSRS_TDF_A2_S04' freq 2462
BSS: Add new id 4 BSSID fa:b7:97:87:2c:de SSID '\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' freq 2452
BSS: fa:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 7.580000 (freq 2452)
New last_update: 7.580000 (freq 2452)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 7.580000 (freq 2462)
New last_update: 7.580000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 7.580000 (freq 2462)
New last_update: 7.580000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 7.580000 (freq 2462)
New last_update: 7.580000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 7.580000 (freq 2462)
New last_update: 7.580000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: Add new id 5 BSSID a0:95:7f:d3:b4:3c SSID 'A0957FD3B43D-2G' freq 2437
BSS: a0:95:7f:d3:b4:3c has multiple entries in the scan results - select the most current one
Previous last_update: 7.580000 (freq 2437)
New last_update: 7.580000 (freq 2437)
Ignore this BSS entry since the previous update looks more current
BSS: a0:95:7f:d3:b4:3c has multiple entries in the scan results - select the most current one
Previous last_update: 7.580000 (freq 2437)
New last_update: 7.580000 (freq 2437)
Ignore this BSS entry since the previous update looks more current
BSS: a0:95:7f:d3:b4:3c has multiple entries in the scan results - select the most current one
Previous last_update: 7.580000 (freq 2437)
New last_update: 7.580000 (freq 2437)
Ignore this BSS entry since the previous update looks more current
BSS: Add new id 6 BSSID 54:9b:49:d3:e7:f2 SSID 'xg100n-d3e7f1-1' freq 2462
BSS: last_scan_res_used=%zu/%zu
New scan results available (own=0 ext=0)
Radio work 'scan'@ffff0000073d8700 done in 3.440000 seconds
radio_work_free('scan'@ffff0000073d8700): num_active_works --> 0
Selecting BSS from priority group 0
0: f8:b7:97:87:2c:df ssid='MSRS_TDF_A5_A11' wpa_ie_len=0 rsn_ie_len=20 caps=0x431 level=65499 freq=5210  wps
   selected based on RSN IE
MLD: No Multi-Link element
   selected BSS f8:b7:97:87:2c:df ssid='MSRS_TDF_A5_A11'
Considering connect request: reassociate: 0
  selected: f8:b7:97:87:2c:df  bssid: 00:00:00:00:00:00  pending: 00:00:00:00:00:00
  wpa_state: SCANNING  ssid=ffff0000073cca60  current_ssid=0
Request association with f8:b7:97:87:2c:df
No ongoing scan/p2p-scan found to abort
Add radio work 'connect'@ffff0000073d8c80
First radio work item in the queue - schedule start immediately
Starting radio work 'connect'@ffff0000073d8c80 after 0.210000 second wait
WPA: clearing own WPA/RSN IE
RSN: clearing own RSNXE
RSN: PMKSA cache search - network_ctx=ffff0000073cca60 try_opportunistic=0 akmp=0x0
RSN: Search for BSSID f8:b7:97:87:2c:df
RSN: No PMKSA cache entry found
RSN: using IEEE 802.11i/D9.0
WPA: Selected cipher suites: group 16 pairwise 16 key_mgmt 2 proto 2
WPA: Selected mgmt group cipher 32
WPA: clearing AP WPA IE
WPA: set AP RSN IE: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 00 00
WPA: clearing AP RSNXE
WPA: AP group 0x10 network profile group 0x18; available group 0x10
WPA: using GTK CCMP
WPA: AP pairwise 0x10 network profile pairwise 0x18; available pairwise 0x10
WPA: using PTK CCMP
WPA: AP key_mgmt 0x2 network profile key_mgmt 0x2; available key_mgmt 0x2
WPA: using KEY_MGMT WPA-PSK
WPA: AP mgmt_group_cipher 0x20 network profile mgmt_group_cipher 0x0; available mgmt_group_cipher 0x0
WPA: not using MGMT group cipher
WPA: Set own WPA IE default: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 0c 00
RSN: Set own RSNXE default: Automatic auth_alg selection: 0x1
No supported operating classes IE to add
Trying to associate with f8:b7:97:87:2c:df (SSID='MSRS_TDF_A5_A11' freq=5210 MHz)
Cancelling scan request
State: SCANNING -> ASSOCIATING
Limit connection to BSSID f8:b7:97:87:2c:df freq=5210 MHz based on scan results (bssid_set=0 wps=0)
[3]bcm4343_control: command: 'join MSRS_TDF_A5_A11 f8:b7:97:87:2c:df 0 30140100000fac040100000fac040100000fac020c00'
Event ASSOC (0) received
State: ASSOCIATING -> ASSOCIATED
Associated to a new BSS: BSSID=a3:66:7c:0f:0e:4d
[1]bcm4343_recv_scan_result: no more result
BSS: Start scan result update 2
Associated with a3:66:7c:0f:0e:4d
WPA: Association event - clear replay counter
WPA: Clear old PTK
EAPOL: External notification - portEnabled=0
EAPOL: External notification - portValid=0
EAPOL: External notification - EAP success=0
EAPOL: External notification - portEnabled=1
EAPOL: SUPP_PAE entering state CONNECTING
EAPOL: enable timer tick
EAPOL: SUPP_BE entering state IDLE
EAP: EAP entering state INITIALIZE
EAP: EAP entering state IDLE
Setting authentication timeout: 10 sec 0 usec
Cancelling scan request
Setting authentication timeout: 10 sec 0 usec
EAPOL: External notification - EAP success=0
EAPOL: External notification - EAP fail=0
EAPOL: External notification - portControl=Auto
RX EAPOL from f8:b7:97:87:2c:df (encrypted=-1)
RX EAPOL: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
01 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
Setting authentication timeout: 10 sec 0 usec
IEEE 802.1X RX: version=1 type=3 length=117
WPA: RX EAPOL-Key: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
01 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
  EAPOL-Key type=2
  key_info 0x8a (ver=2 keyidx=0 rsvd=0 Pairwise Ack)
  key_length=16 key_data_length=22
  replay_counter: 00 00 00 00 00 00 00 01
  key_nonce: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d 58
  key_iv: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  key_rsc: 00 00 00 00 00 00 00 00
  key_id (reserved): 00 00 00 00 00 00 00 00
  key_mic: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
EAP: EAP entering state DISABLED
WPA: RX message 1 of 4-Way Handshake from f8:b7:97:87:2c:df (ver=2)
RSN: msg 1/4 key data: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
WPA: PMKID in EAPOL-Key: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
RSN: PMKID from Authenticator: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
RSN: no matching PMKID found
State: ASSOCIATED -> 4WAY_HANDSHAKE
WPA: Renewed SNonce: d5 2c 20 d5 4e cd d3 17 e4 43 b7 44 fa f8 a7 ea
78 70 9c 93 95 78 74 05 51 29 8d 39 8e e7 e9 c8
WPA: PTK derivation using PRF(SHA1)
WPA: PTK derivation - A1=c0:b0:3d:07:00:00 A2=a3:66:7c:0f:0e:4d
WPA: Nonce1: d5 2c 20 d5 4e cd d3 17 e4 43 b7 44 fa f8 a7 ea
78 70 9c 93 95 78 74 05 51 29 8d 39 8e e7 e9 c8
WPA: Nonce2: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d 58
WPA: WPA IE for msg 2/4: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 0c 00
WPA: Replay Counter: 00 00 00 00 00 00 00 01
WPA: Sending EAPOL-Key 2/4
WPA: Send EAPOL-Key frame to a3:66:7c:0f:0e:4d ver=2 mic_len=16 key_mgmt=0x2
WPA: EAPOL-Key MIC using HMAC-SHA1
WPA: Derived Key MIC: 2b a8 3c 7f cb 91 c2 96 87 fc 34 ff 8b 5e 82 46
WPA: TX EAPOL-Key: 01 03 00 75 02 01 0a 00 00 00 00 00 00 00 00 00
01 d5 2c 20 d5 4e cd d3 17 e4 43 b7 44 fa f8 a7
ea 78 70 9c 93 95 78 74 05 51 29 8d 39 8e e7 e9
c8 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 2b a8 3c 7f cb 91 c2 96 87 fc 34 ff 8b 5e 82
46 00 16 30 14 01 00 00 0f ac 04 01 00 00 0f ac
04 01 00 00 0f ac 02 0c 00
RX EAPOL from f8:b7:97:87:2c:df (encrypted=-1)
RX EAPOL: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
02 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 a0 cf f3 9e b5 6d d4 c4 53 47 6c c6 4e 21
aa 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
IEEE 802.1X RX: version=1 type=3 length=117
WPA: RX EAPOL-Key: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
02 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 a0 cf f3 9e b5 6d d4 c4 53 47 6c c6 4e 21
aa 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
  EAPOL-Key type=2
  key_info 0x8a (ver=2 keyidx=0 rsvd=0 Pairwise Ack)
  key_length=16 key_data_length=22
  replay_counter: 00 00 00 00 00 00 00 02
  key_nonce: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d 58
  key_iv: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  key_rsc: 00 00 00 00 00 00 00 00
  key_id (reserved): 00 00 00 00 00 00 00 00
  key_mic: 00 a0 cf f3 9e b5 6d d4 c4 53 47 6c c6 4e 21 aa
WPA: RX message 1 of 4-Way Handshake from f8:b7:97:87:2c:df (ver=2)
RSN: msg 1/4 key data: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
WPA: PMKID in EAPOL-Key: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
RSN: PMKID from Authenticator: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
RSN: no matching PMKID found
State: 4WAY_HANDSHAKE -> 4WAY_HANDSHAKE
WPA: PTK derivation using PRF(SHA1)
WPA: PTK derivation - A1=c0:b0:3d:07:00:00 A2=a3:66:7c:0f:0e:4d
WPA: Nonce1: d5 2c 20 d5 4e cd d3 17 e4 43 b7 44 fa f8 a7 ea
78 70 9c 93 95 78 74 05 51 29 8d 39 8e e7 e9 c8
WPA: Nonce2: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d 58
WPA: WPA IE for msg 2/4: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 0c 00
WPA: Replay Counter: 00 00 00 00 00 00 00 02
WPA: Sending EAPOL-Key 2/4
WPA: Send EAPOL-Key frame to a3:66:7c:0f:0e:4d ver=2 mic_len=16 key_mgmt=0x2
WPA: EAPOL-Key MIC using HMAC-SHA1
WPA: Derived Key MIC: 6c ae 8d 07 61 3d 9f 21 6d cb 97 5e a2 ad 0f 2c
WPA: TX EAPOL-Key: 01 03 00 75 02 01 0a 00 00 00 00 00 00 00 00 00
02 d5 2c 20 d5 4e cd d3 17 e4 43 b7 44 fa f8 a7
ea 78 70 9c 93 95 78 74 05 51 29 8d 39 8e e7 e9
c8 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 6c ae 8d 07 61 3d 9f 21 6d cb 97 5e a2 ad 0f
2c 00 16 30 14 01 00 00 0f ac 04 01 00 00 0f ac
04 01 00 00 0f ac 02 0c 00
RX EAPOL from f8:b7:97:87:2c:df (encrypted=-1)
RX EAPOL: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
03 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 69 23 f3 92 16 a6 96 08 b7 e6 93 cb 98 d8 5f
16 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
IEEE 802.1X RX: version=1 type=3 length=117
WPA: RX EAPOL-Key: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
03 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 69 23 f3 92 16 a6 96 08 b7 e6 93 cb 98 d8 5f
16 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
  EAPOL-Key type=2
  key_info 0x8a (ver=2 keyidx=0 rsvd=0 Pairwise Ack)
  key_length=16 key_data_length=22
  replay_counter: 00 00 00 00 00 00 00 03
  key_nonce: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d 58
  key_iv: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  key_rsc: 00 00 00 00 00 00 00 00
  key_id (reserved): 00 00 00 00 00 00 00 00
  key_mic: 69 23 f3 92 16 a6 96 08 b7 e6 93 cb 98 d8 5f 16
WPA: RX message 1 of 4-Way Handshake from f8:b7:97:87:2c:df (ver=2)
RSN: msg 1/4 key data: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
WPA: PMKID in EAPOL-Key: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
RSN: PMKID from Authenticator: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
RSN: no matching PMKID found
State: 4WAY_HANDSHAKE -> 4WAY_HANDSHAKE
WPA: PTK derivation using PRF(SHA1)
WPA: PTK derivation - A1=c0:b0:3d:07:00:00 A2=a3:66:7c:0f:0e:4d
WPA: Nonce1: d5 2c 20 d5 4e cd d3 17 e4 43 b7 44 fa f8 a7 ea
78 70 9c 93 95 78 74 05 51 29 8d 39 8e e7 e9 c8
WPA: Nonce2: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d 58
WPA: WPA IE for msg 2/4: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 0c 00
WPA: Replay Counter: 00 00 00 00 00 00 00 03
WPA: Sending EAPOL-Key 2/4
WPA: Send EAPOL-Key frame to a3:66:7c:0f:0e:4d ver=2 mic_len=16 key_mgmt=0x2
WPA: EAPOL-Key MIC using HMAC-SHA1
WPA: Derived Key MIC: 3c 80 e8 53 2b 43 2d 14 f3 98 e6 03 6f 55 d1 fa
WPA: TX EAPOL-Key: 01 03 00 75 02 01 0a 00 00 00 00 00 00 00 00 00
03 d5 2c 20 d5 4e cd d3 17 e4 43 b7 44 fa f8 a7
ea 78 70 9c 93 95 78 74 05 51 29 8d 39 8e e7 e9
c8 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 3c 80 e8 53 2b 43 2d 14 f3 98 e6 03 6f 55 d1
fa 00 16 30 14 01 00 00 0f ac 04 01 00 00 0f ac
04 01 00 00 0f ac 02 0c 00
EAPOL: startWhen --> 0
EAPOL: disable timer tick
EAPOL: SUPP_PAE entering state CONNECTING
EAPOL: enable timer tick
EAPOL: txStart
WPA: drop TX EAPOL in non-IEEE 802.1X mode (type=1 len=0)
RX EAPOL from f8:b7:97:87:2c:df (encrypted=-1)
RX EAPOL: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
04 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 ec 05 3a 32 40 57 38 f4 51 0d f2 b9 cc 19 db
e4 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
IEEE 802.1X RX: version=1 type=3 length=117
WPA: RX EAPOL-Key: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
04 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 ec 05 3a 32 40 57 38 f4 51 0d f2 b9 cc 19 db
e4 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
  EAPOL-Key type=2
  key_info 0x8a (ver=2 keyidx=0 rsvd=0 Pairwise Ack)
  key_length=16 key_data_length=22
  replay_counter: 00 00 00 00 00 00 00 04
  key_nonce: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d 58
  key_iv: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  key_rsc: 00 00 00 00 00 00 00 00
  key_id (reserved): 00 00 00 00 00 00 00 00
  key_mic: ec 05 3a 32 40 57 38 f4 51 0d f2 b9 cc 19 db e4
WPA: RX message 1 of 4-Way Handshake from f8:b7:97:87:2c:df (ver=2)
RSN: msg 1/4 key data: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
WPA: PMKID in EAPOL-Key: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
RSN: PMKID from Authenticator: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
RSN: no matching PMKID found
State: 4WAY_HANDSHAKE -> 4WAY_HANDSHAKE
WPA: PTK derivation using PRF(SHA1)
WPA: PTK derivation - A1=c0:b0:3d:07:00:00 A2=a3:66:7c:0f:0e:4d
WPA: Nonce1: d5 2c 20 d5 4e cd d3 17 e4 43 b7 44 fa f8 a7 ea
78 70 9c 93 95 78 74 05 51 29 8d 39 8e e7 e9 c8
WPA: Nonce2: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d 58
WPA: WPA IE for msg 2/4: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 0c 00
WPA: Replay Counter: 00 00 00 00 00 00 00 04
WPA: Sending EAPOL-Key 2/4
WPA: Send EAPOL-Key frame to a3:66:7c:0f:0e:4d ver=2 mic_len=16 key_mgmt=0x2
WPA: EAPOL-Key MIC using HMAC-SHA1
WPA: Derived Key MIC: 84 79 7b fe c2 eb 57 fc 90 e0 79 8f a8 6b 53 fb
WPA: TX EAPOL-Key: 01 03 00 75 02 01 0a 00 00 00 00 00 00 00 00 00
04 d5 2c 20 d5 4e cd d3 17 e4 43 b7 44 fa f8 a7
ea 78 70 9c 93 95 78 74 05 51 29 8d 39 8e e7 e9
c8 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 84 79 7b fe c2 eb 57 fc 90 e0 79 8f a8 6b 53
fb 00 16 30 14 01 00 00 0f ac 04 01 00 00 0f ac
04 01 00 00 0f ac 02 0c 00
RX EAPOL from f8:b7:97:87:2c:df (encrypted=-1)
RX EAPOL: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
05 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 8c 91 d1 61 5b 6e 46 a3 80 f7 30 a6 29 33 9d
b2 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
IEEE 802.1X RX: version=1 type=3 length=117
WPA: RX EAPOL-Key: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
05 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 8c 91 d1 61 5b 6e 46 a3 80 f7 30 a6 29 33 9d
b2 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
  EAPOL-Key type=2
  key_info 0x8a (ver=2 keyidx=0 rsvd=0 Pairwise Ack)
  key_length=16 key_data_length=22
  replay_counter: 00 00 00 00 00 00 00 05
  key_nonce: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d 58
  key_iv: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  key_rsc: 00 00 00 00 00 00 00 00
  key_id (reserved): 00 00 00 00 00 00 00 00
  key_mic: 8c 91 d1 61 5b 6e 46 a3 80 f7 30 a6 29 33 9d b2
WPA: RX message 1 of 4-Way Handshake from f8:b7:97:87:2c:df (ver=2)
RSN: msg 1/4 key data: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
WPA: PMKID in EAPOL-Key: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
RSN: PMKID from Authenticator: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
RSN: no matching PMKID found
State: 4WAY_HANDSHAKE -> 4WAY_HANDSHAKE
WPA: PTK derivation using PRF(SHA1)
WPA: PTK derivation - A1=c0:b0:3d:07:00:00 A2=a3:66:7c:0f:0e:4d
WPA: Nonce1: d5 2c 20 d5 4e cd d3 17 e4 43 b7 44 fa f8 a7 ea
78 70 9c 93 95 78 74 05 51 29 8d 39 8e e7 e9 c8
WPA: Nonce2: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d 58
WPA: WPA IE for msg 2/4: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 0c 00
WPA: Replay Counter: 00 00 00 00 00 00 00 05
WPA: Sending EAPOL-Key 2/4
WPA: Send EAPOL-Key frame to a3:66:7c:0f:0e:4d ver=2 mic_len=16 key_mgmt=0x2
WPA: EAPOL-Key MIC using HMAC-SHA1
WPA: Derived Key MIC: fa 24 2b 43 9e 6c 42 8c c9 3b 39 13 24 d6 ce 60
WPA: TX EAPOL-Key: 01 03 00 75 02 01 0a 00 00 00 00 00 00 00 00 00
05 d5 2c 20 d5 4e cd d3 17 e4 43 b7 44 fa f8 a7
ea 78 70 9c 93 95 78 74 05 51 29 8d 39 8e e7 e9
c8 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 fa 24 2b 43 9e 6c 42 8c c9 3b 39 13 24 d6 ce
60 00 16 30 14 01 00 00 0f ac 04 01 00 00 0f ac
04 01 00 00 0f ac 02 0c 00
RX EAPOL from f8:b7:97:87:2c:df (encrypted=-1)
RX EAPOL: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
06 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 55 99 9d 4f c4 36 fd b2 28 c7 6e d5 de a3 1c
a8 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
IEEE 802.1X RX: version=1 type=3 length=117
WPA: RX EAPOL-Key: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
06 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 55 99 9d 4f c4 36 fd b2 28 c7 6e d5 de a3 1c
a8 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
  EAPOL-Key type=2
  key_info 0x8a (ver=2 keyidx=0 rsvd=0 Pairwise Ack)
  key_length=16 key_data_length=22
  replay_counter: 00 00 00 00 00 00 00 06
  key_nonce: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d 58
  key_iv: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  key_rsc: 00 00 00 00 00 00 00 00
  key_id (reserved): 00 00 00 00 00 00 00 00
  key_mic: 55 99 9d 4f c4 36 fd b2 28 c7 6e d5 de a3 1c a8
WPA: RX message 1 of 4-Way Handshake from f8:b7:97:87:2c:df (ver=2)
RSN: msg 1/4 key data: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
WPA: PMKID in EAPOL-Key: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
RSN: PMKID from Authenticator: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
RSN: no matching PMKID found
State: 4WAY_HANDSHAKE -> 4WAY_HANDSHAKE
WPA: PTK derivation using PRF(SHA1)
WPA: PTK derivation - A1=c0:b0:3d:07:00:00 A2=a3:66:7c:0f:0e:4d
WPA: Nonce1: d5 2c 20 d5 4e cd d3 17 e4 43 b7 44 fa f8 a7 ea
78 70 9c 93 95 78 74 05 51 29 8d 39 8e e7 e9 c8
WPA: Nonce2: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 31 a7 4d 58
WPA: WPA IE for msg 2/4: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 0c 00
WPA: Replay Counter: 00 00 00 00 00 00 00 06
WPA: Sending EAPOL-Key 2/4
WPA: Send EAPOL-Key frame to a3:66:7c:0f:0e:4d ver=2 mic_len=16 key_mgmt=0x2
WPA: EAPOL-Key MIC using HMAC-SHA1
WPA: Derived Key MIC: ba 98 bd cd 92 7a 83 cd de 68 be 7c 8f d2 2a 34
WPA: TX EAPOL-Key: 01 03 00 75 02 01 0a 00 00 00 00 00 00 00 00 00
06 d5 2c 20 d5 4e cd d3 17 e4 43 b7 44 fa f8 a7
ea 78 70 9c 93 95 78 74 05 51 29 8d 39 8e e7 e9
c8 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 ba 98 bd cd 92 7a 83 cd de 68 be 7c 8f d2 2a
34 00 16 30 14 01 00 00 0f ac 04 01 00 00 0f ac
04 01 00 00 0f ac 02 0c 00
Event DEAUTH (11) received
Deauthentication notification
CTRL-EVENT-DISCONNECTED bssid=a3:66:7c:0f:0e:4d reason=0
WPA: 4-Way Handshake failed - pre-shared key may be incorrect
CTRL-EVENT-SSID-TEMP-DISABLED id=0 ssid="MSRS_TDF_A5_A11" auth_failures=1 duration=10 reason=WRONG_KEY
Auto connect enabled: try to reconnect (wps=0/0 wpa_state=7)
Setting scan request: 0.100000 sec
Radio work 'connect'@ffff0000073d8c80 done in 7.120000 seconds
radio_work_free('connect'@ffff0000073d8c80): num_active_works --> 0
Added BSSID a3:66:7c:0f:0e:4d into ignore list, ignoring for 10 seconds
Consecutive connection failures: 1 --> request scan in 100 ms
Ignore new scan request for 0.100000 sec since an earlier request is scheduled to trigger sooner
WPA: Clear old PMK and PTK
Disconnect event - remove keys
State: 4WAY_HANDSHAKE -> DISCONNECTED
EAPOL: External notification - portEnabled=0
EAPOL: SUPP_PAE entering state DISCONNECTED
EAPOL: Supplicant port status: Unauthorized
EAPOL: SUPP_BE entering state INITIALIZE
EAPOL: External notification - portValid=0
EAPOL: External notification - EAP success=0
State: DISCONNECTED -> SCANNING
Starting AP scan for wildcard SSID
Add radio work 'scan'@ffff000007347e10
First radio work item in the queue - schedule start immediately
Starting radio work 'scan'@ffff000007347e10 after 0.020000 second wait
[2]bcm4343_control: command: 'escan 5'
[2]bcm4343_control: command: 'escan 0'
Event SCAN_RESULTS (3) received
[2]bcm4343_recv_scan_result: queue num 0
BSS: Start scan result update 3
BSS: f8:b7:97:87:2c:df has multiple entries in the scan results - select the most current one
Previous last_update: 18.480000 (freq 5210)
New last_update: 18.480000 (freq 5210)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 18.480000 (freq 2462)
New last_update: 18.480000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 18.480000 (freq 2462)
New last_update: 18.480000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 18.480000 (freq 2462)
New last_update: 18.480000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 18.480000 (freq 2462)
New last_update: 18.480000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 18.480000 (freq 2462)
New last_update: 18.480000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 18.480000 (freq 2462)
New last_update: 18.480000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: Add new id 7 BSSID a0:95:7f:d3:b4:3c SSID 'A0957FD3B43D-2G' freq 2437
BSS: Add new id 8 BSSID 56:9b:49:d3:e7:f2 SSID 'xg100n-d3e7f1-2' freq 2462
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 18.480000 (freq 2462)
New last_update: 18.480000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: Remove id 827012172 BSSID 08:00:06:00:50:f2 SSID '\tWG1200HS3\x10\x08\x00\x02 \x08\x10<\x00\x01\x03\x10I\x00\x06\x007*\x00\x01 \x7f\x08\x00\x00\xn
BSS: last_scan_res_used=%zu/%zu
New scan results available (own=0 ext=0)
Radio work 'scan'@ffff000007347e10 done in 3.340000 seconds
radio_work_free('scan'@ffff000007347e10): num_active_works --> 0
Postpone network selection by 7 seconds since all networks are disabled
Try to associate due to network getting re-enabled
Fast associate: Old scan results
Setting scan request: 0.000000 sec
Starting AP scan for wildcard SSID
Add radio work 'scan'@ffff0000073ce940
First radio work item in the queue - schedule start immediately
Event DISASSOC (1) received
Disassociation notification
Auto connect enabled: try to reconnect (wps=0/0 wpa_state=3)
Do not request new immediate scan
WPA: Clear old PMK and PTK
Disconnect event - remove keys
State: SCANNING -> DISCONNECTED
EAPOL: External notification - portEnabled=0
EAPOL: External notification - portValid=0
Starting radio work 'scan'@ffff0000073ce940 after 0.210000 second wait
[2]bcm4343_control: command: 'escan 5'
[2]bcm4343_control: command: 'escan 0'
Event SCAN_RESULTS (3) received
[2]bcm4343_recv_scan_result: queue num 0
BSS: Start scan result update 4
BSS: f8:b7:97:87:2c:df has multiple entries in the scan results - select the most current one
Previous last_update: 29.640000 (freq 5210)
New last_update: 29.640000 (freq 5210)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:df has multiple entries in the scan results - select the most current one
Previous last_update: 29.640000 (freq 5210)
New last_update: 29.640000 (freq 5210)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 29.640000 (freq 2462)
New last_update: 29.640000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 29.640000 (freq 2462)
New last_update: 29.640000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 29.640000 (freq 2462)
New last_update: 29.640000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: fa:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 29.640000 (freq 2452)
New last_update: 29.640000 (freq 2452)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 29.640000 (freq 2462)
New last_update: 29.640000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 29.640000 (freq 2462)
New last_update: 29.640000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: a0:95:7f:d3:b4:3c has multiple entries in the scan results - select the most current one
Previous last_update: 29.640000 (freq 2437)
New last_update: 29.640000 (freq 2437)
Ignore this BSS entry since the previous update looks more current
BSS: fa:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 29.640000 (freq 2452)
New last_update: 29.640000 (freq 2452)
Ignore this BSS entry since the previous update looks more current
BSS: Remove id 6 BSSID 54:9b:49:d3:e7:f2 SSID 'xg100n-d3e7f1-1' due to no match in scan
BSS: last_scan_res_used=%zu/%zu
New scan results available (own=0 ext=0)
Radio work 'scan'@ffff0000073ce940 done in 3.420000 seconds
radio_work_free('scan'@ffff0000073ce940): num_active_works --> 0
CTRL-EVENT-SSID-REENABLED id=0 ssid="MSRS_TDF_A5_A11"
Selecting BSS from priority group 0
0: d8:21:da:38:8e:c5 ssid='NSD3K-8EC0-a' wpa_ie_len=0 rsn_ie_len=20 caps=0x1111 level=65450 freq=5690  wps
   skip - SSID mismatch
1: f8:b7:97:87:2c:df ssid='MSRS_TDF_A5_A11' wpa_ie_len=0 rsn_ie_len=20 caps=0x431 level=65498 freq=5210  wps
   selected based on RSN IE
MLD: No Multi-Link element
   selected BSS f8:b7:97:87:2c:df ssid='MSRS_TDF_A5_A11'
Considering connect request: reassociate: 0
  selected: f8:b7:97:87:2c:df  bssid: 00:00:00:00:00:00  pending: 00:00:00:00:00:00
  wpa_state: DISCONNECTED  ssid=ffff0000073cca60  current_ssid=0
Request association with f8:b7:97:87:2c:df
Re-association to the same ESS
No ongoing scan/p2p-scan found to abort
Add radio work 'connect'@ffff0000073d8300
First radio work item in the queue - schedule start immediately
Starting radio work 'connect'@ffff0000073d8300 after 0.200000 second wait
WPA: clearing own WPA/RSN IE
RSN: clearing own RSNXE
RSN: PMKSA cache search - network_ctx=ffff0000073cca60 try_opportunistic=0 akmp=0x0
RSN: Search for BSSID f8:b7:97:87:2c:df
RSN: No PMKSA cache entry found
RSN: using IEEE 802.11i/D9.0
WPA: Selected cipher suites: group 16 pairwise 16 key_mgmt 2 proto 2
WPA: Selected mgmt group cipher 32
WPA: clearing AP WPA IE
WPA: set AP RSN IE: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 00 00
WPA: clearing AP RSNXE
WPA: AP group 0x10 network profile group 0x18; available group 0x10
WPA: using GTK CCMP
WPA: AP pairwise 0x10 network profile pairwise 0x18; available pairwise 0x10
WPA: using PTK CCMP
WPA: AP key_mgmt 0x2 network profile key_mgmt 0x2; available key_mgmt 0x2
WPA: using KEY_MGMT WPA-PSK
WPA: AP mgmt_group_cipher 0x20 network profile mgmt_group_cipher 0x0; available mgmt_group_cipher 0x0
WPA: not using MGMT group cipher
WPA: Set own WPA IE default: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 0c 00
RSN: Set own RSNXE default: Automatic auth_alg selection: 0x1
No supported operating classes IE to add
Trying to associate with f8:b7:97:87:2c:df (SSID='MSRS_TDF_A5_A11' freq=5210 MHz)
Cancelling scan request
State: DISCONNECTED -> ASSOCIATING
Limit connection to BSSID f8:b7:97:87:2c:df freq=5210 MHz based on scan results (bssid_set=0 wps=0)
[2]bcm4343_control: command: 'join MSRS_TDF_A5_A11 f8:b7:97:87:2c:df 0 30140100000fac040100000fac040100000fac020c00'
Event ASSOC (0) received
State: ASSOCIATING -> ASSOCIATED
Associated to a new BSS: BSSID=6e:33:ea:00:60:2d
[0]bcm4343_recv_scan_result: queue num 0
BSS: Start scan result update 5
Associated with 6e:33:ea:00:60:2d
WPA: Association event - clear replay counter
WPA: Clear old PTK
EAPOL: External notification - portEnabled=0
EAPOL: External notification - portValid=0
EAPOL: External notification - EAP success=0
EAPOL: External notification - portEnabled=1
EAPOL: SUPP_PAE entering state CONNECTING
EAPOL: SUPP_BE entering state IDLE
Setting authentication timeout: 10 sec 0 usec
Cancelling scan request
Setting authentication timeout: 10 sec 0 usec
EAPOL: External notification - EAP success=0
EAPOL: External notification - EAP fail=0
EAPOL: External notification - portControl=Auto
RX EAPOL from f8:b7:97:87:2c:df (encrypted=-1)
RX EAPOL: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
00 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
Setting authentication timeout: 10 sec 0 usec
IEEE 802.1X RX: version=1 type=3 length=117
WPA: RX EAPOL-Key: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
00 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
  EAPOL-Key type=2
  key_info 0x8a (ver=2 keyidx=0 rsvd=0 Pairwise Ack)
  key_length=16 key_data_length=22
  replay_counter: 00 00 00 00 00 00 00 00
  key_nonce: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d 58
  key_iv: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  key_rsc: 00 00 00 00 00 00 00 00
  key_id (reserved): 00 00 00 00 00 00 00 00
  key_mic: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
WPA: RX message 1 of 4-Way Handshake from f8:b7:97:87:2c:df (ver=2)
RSN: msg 1/4 key data: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
WPA: PMKID in EAPOL-Key: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
RSN: PMKID from Authenticator: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
RSN: no matching PMKID found
State: ASSOCIATED -> 4WAY_HANDSHAKE
WPA: Renewed SNonce: c3 16 e2 c4 62 a2 3b 54 fa d1 88 8f 75 c3 0e 2b
5e ae e3 5b 83 9e a8 6f 44 53 36 c0 13 3e 45 84
WPA: PTK derivation using PRF(SHA1)
WPA: PTK derivation - A1=c0:b0:3d:07:00:00 A2=6e:33:ea:00:60:2d
WPA: Nonce1: c3 16 e2 c4 62 a2 3b 54 fa d1 88 8f 75 c3 0e 2b
5e ae e3 5b 83 9e a8 6f 44 53 36 c0 13 3e 45 84
WPA: Nonce2: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d 58
WPA: WPA IE for msg 2/4: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 0c 00
WPA: Replay Counter: 00 00 00 00 00 00 00 00
WPA: Sending EAPOL-Key 2/4
WPA: Send EAPOL-Key frame to 6e:33:ea:00:60:2d ver=2 mic_len=16 key_mgmt=0x2
WPA: EAPOL-Key MIC using HMAC-SHA1
WPA: Derived Key MIC: 50 d1 60 68 40 92 48 86 21 78 69 b5 28 58 81 39
WPA: TX EAPOL-Key: 01 03 00 75 02 01 0a 00 00 00 00 00 00 00 00 00
00 c3 16 e2 c4 62 a2 3b 54 fa d1 88 8f 75 c3 0e
2b 5e ae e3 5b 83 9e a8 6f 44 53 36 c0 13 3e 45
84 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 50 d1 60 68 40 92 48 86 21 78 69 b5 28 58 81
39 00 16 30 14 01 00 00 0f ac 04 01 00 00 0f ac
04 01 00 00 0f ac 02 0c 00
RX EAPOL from f8:b7:97:87:2c:df (encrypted=-1)
RX EAPOL: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
01 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 6a cf f3 de 8b 0a 78 a7 1c 4b ef ee 31 ed a8
a4 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
IEEE 802.1X RX: version=1 type=3 length=117
WPA: RX EAPOL-Key: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
01 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 6a cf f3 de 8b 0a 78 a7 1c 4b ef ee 31 ed a8
a4 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
  EAPOL-Key type=2
  key_info 0x8a (ver=2 keyidx=0 rsvd=0 Pairwise Ack)
  key_length=16 key_data_length=22
  replay_counter: 00 00 00 00 00 00 00 01
  key_nonce: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d 58
  key_iv: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  key_rsc: 00 00 00 00 00 00 00 00
  key_id (reserved): 00 00 00 00 00 00 00 00
  key_mic: 6a cf f3 de 8b 0a 78 a7 1c 4b ef ee 31 ed a8 a4
WPA: RX message 1 of 4-Way Handshake from f8:b7:97:87:2c:df (ver=2)
RSN: msg 1/4 key data: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
WPA: PMKID in EAPOL-Key: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
RSN: PMKID from Authenticator: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
RSN: no matching PMKID found
State: 4WAY_HANDSHAKE -> 4WAY_HANDSHAKE
WPA: PTK derivation using PRF(SHA1)
WPA: PTK derivation - A1=c0:b0:3d:07:00:00 A2=6e:33:ea:00:60:2d
WPA: Nonce1: c3 16 e2 c4 62 a2 3b 54 fa d1 88 8f 75 c3 0e 2b
5e ae e3 5b 83 9e a8 6f 44 53 36 c0 13 3e 45 84
WPA: Nonce2: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d 58
WPA: WPA IE for msg 2/4: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 0c 00
WPA: Replay Counter: 00 00 00 00 00 00 00 01
WPA: Sending EAPOL-Key 2/4
WPA: Send EAPOL-Key frame to 6e:33:ea:00:60:2d ver=2 mic_len=16 key_mgmt=0x2
WPA: EAPOL-Key MIC using HMAC-SHA1
WPA: Derived Key MIC: c0 2f 74 81 b9 9a ff cc 6b d8 e6 b3 b9 f2 09 0a
WPA: TX EAPOL-Key: 01 03 00 75 02 01 0a 00 00 00 00 00 00 00 00 00
01 c3 16 e2 c4 62 a2 3b 54 fa d1 88 8f 75 c3 0e
2b 5e ae e3 5b 83 9e a8 6f 44 53 36 c0 13 3e 45
84 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 c0 2f 74 81 b9 9a ff cc 6b d8 e6 b3 b9 f2 09
0a 00 16 30 14 01 00 00 0f ac 04 01 00 00 0f ac
04 01 00 00 0f ac 02 0c 00
EAPOL: startWhen --> 0
EAPOL: disable timer tick
EAPOL: SUPP_PAE entering state CONNECTING
EAPOL: enable timer tick
EAPOL: txStart
WPA: drop TX EAPOL in non-IEEE 802.1X mode (type=1 len=0)
RX EAPOL from f8:b7:97:87:2c:df (encrypted=-1)
RX EAPOL: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
02 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 ae 79 28 f7 15 3e c0 f7 a9 af 25 5f c9 89 a0
98 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
IEEE 802.1X RX: version=1 type=3 length=117
WPA: RX EAPOL-Key: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
02 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 ae 79 28 f7 15 3e c0 f7 a9 af 25 5f c9 89 a0
98 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
  EAPOL-Key type=2
  key_info 0x8a (ver=2 keyidx=0 rsvd=0 Pairwise Ack)
  key_length=16 key_data_length=22
  replay_counter: 00 00 00 00 00 00 00 02
  key_nonce: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d 58
  key_iv: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  key_rsc: 00 00 00 00 00 00 00 00
  key_id (reserved): 00 00 00 00 00 00 00 00
  key_mic: ae 79 28 f7 15 3e c0 f7 a9 af 25 5f c9 89 a0 98
WPA: RX message 1 of 4-Way Handshake from f8:b7:97:87:2c:df (ver=2)
RSN: msg 1/4 key data: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
WPA: PMKID in EAPOL-Key: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
RSN: PMKID from Authenticator: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
RSN: no matching PMKID found
State: 4WAY_HANDSHAKE -> 4WAY_HANDSHAKE
WPA: PTK derivation using PRF(SHA1)
WPA: PTK derivation - A1=c0:b0:3d:07:00:00 A2=6e:33:ea:00:60:2d
WPA: Nonce1: c3 16 e2 c4 62 a2 3b 54 fa d1 88 8f 75 c3 0e 2b
5e ae e3 5b 83 9e a8 6f 44 53 36 c0 13 3e 45 84
WPA: Nonce2: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d 58
WPA: WPA IE for msg 2/4: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 0c 00
WPA: Replay Counter: 00 00 00 00 00 00 00 02
WPA: Sending EAPOL-Key 2/4
WPA: Send EAPOL-Key frame to 6e:33:ea:00:60:2d ver=2 mic_len=16 key_mgmt=0x2
WPA: EAPOL-Key MIC using HMAC-SHA1
WPA: Derived Key MIC: 1d f9 31 2e 86 0f db 76 2e 0a 2a 71 81 fb f9 cc
WPA: TX EAPOL-Key: 01 03 00 75 02 01 0a 00 00 00 00 00 00 00 00 00
02 c3 16 e2 c4 62 a2 3b 54 fa d1 88 8f 75 c3 0e
2b 5e ae e3 5b 83 9e a8 6f 44 53 36 c0 13 3e 45
84 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 1d f9 31 2e 86 0f db 76 2e 0a 2a 71 81 fb f9
cc 00 16 30 14 01 00 00 0f ac 04 01 00 00 0f ac
04 01 00 00 0f ac 02 0c 00
RX EAPOL from f8:b7:97:87:2c:df (encrypted=-1)
RX EAPOL: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
03 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 39 c9 f5 81 4c ae 9f e1 1f 3c 06 eb 3a 4b c8
20 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
IEEE 802.1X RX: version=1 type=3 length=117
WPA: RX EAPOL-Key: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
03 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 39 c9 f5 81 4c ae 9f e1 1f 3c 06 eb 3a 4b c8
20 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
  EAPOL-Key type=2
  key_info 0x8a (ver=2 keyidx=0 rsvd=0 Pairwise Ack)
  key_length=16 key_data_length=22
  replay_counter: 00 00 00 00 00 00 00 03
  key_nonce: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d 58
  key_iv: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  key_rsc: 00 00 00 00 00 00 00 00
  key_id (reserved): 00 00 00 00 00 00 00 00
  key_mic: 39 c9 f5 81 4c ae 9f e1 1f 3c 06 eb 3a 4b c8 20
WPA: RX message 1 of 4-Way Handshake from f8:b7:97:87:2c:df (ver=2)
RSN: msg 1/4 key data: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
WPA: PMKID in EAPOL-Key: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
RSN: PMKID from Authenticator: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
RSN: no matching PMKID found
State: 4WAY_HANDSHAKE -> 4WAY_HANDSHAKE
WPA: PTK derivation using PRF(SHA1)
WPA: PTK derivation - A1=c0:b0:3d:07:00:00 A2=6e:33:ea:00:60:2d
WPA: Nonce1: c3 16 e2 c4 62 a2 3b 54 fa d1 88 8f 75 c3 0e 2b
5e ae e3 5b 83 9e a8 6f 44 53 36 c0 13 3e 45 84
WPA: Nonce2: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d 58
WPA: WPA IE for msg 2/4: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 0c 00
WPA: Replay Counter: 00 00 00 00 00 00 00 03
WPA: Sending EAPOL-Key 2/4
WPA: Send EAPOL-Key frame to 6e:33:ea:00:60:2d ver=2 mic_len=16 key_mgmt=0x2
WPA: EAPOL-Key MIC using HMAC-SHA1
WPA: Derived Key MIC: cb 7e 71 e0 fd 95 8f 1b d7 4a 00 4d 49 2c ae 7b
WPA: TX EAPOL-Key: 01 03 00 75 02 01 0a 00 00 00 00 00 00 00 00 00
03 c3 16 e2 c4 62 a2 3b 54 fa d1 88 8f 75 c3 0e
2b 5e ae e3 5b 83 9e a8 6f 44 53 36 c0 13 3e 45
84 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 cb 7e 71 e0 fd 95 8f 1b d7 4a 00 4d 49 2c ae
7b 00 16 30 14 01 00 00 0f ac 04 01 00 00 0f ac
04 01 00 00 0f ac 02 0c 00
RX EAPOL from f8:b7:97:87:2c:df (encrypted=-1)
RX EAPOL: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
04 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 6c 10 a7 0b d9 5b db fe 1d c0 68 ae 01 2e 6c
c5 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
IEEE 802.1X RX: version=1 type=3 length=117
WPA: RX EAPOL-Key: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
04 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 6c 10 a7 0b d9 5b db fe 1d c0 68 ae 01 2e 6c
c5 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
  EAPOL-Key type=2
  key_info 0x8a (ver=2 keyidx=0 rsvd=0 Pairwise Ack)
  key_length=16 key_data_length=22
  replay_counter: 00 00 00 00 00 00 00 04
  key_nonce: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d 58
  key_iv: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  key_rsc: 00 00 00 00 00 00 00 00
  key_id (reserved): 00 00 00 00 00 00 00 00
  key_mic: 6c 10 a7 0b d9 5b db fe 1d c0 68 ae 01 2e 6c c5
WPA: RX message 1 of 4-Way Handshake from f8:b7:97:87:2c:df (ver=2)
RSN: msg 1/4 key data: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
WPA: PMKID in EAPOL-Key: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
RSN: PMKID from Authenticator: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
RSN: no matching PMKID found
State: 4WAY_HANDSHAKE -> 4WAY_HANDSHAKE
WPA: PTK derivation using PRF(SHA1)
WPA: PTK derivation - A1=c0:b0:3d:07:00:00 A2=6e:33:ea:00:60:2d
WPA: Nonce1: c3 16 e2 c4 62 a2 3b 54 fa d1 88 8f 75 c3 0e 2b
5e ae e3 5b 83 9e a8 6f 44 53 36 c0 13 3e 45 84
WPA: Nonce2: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d 58
WPA: WPA IE for msg 2/4: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 0c 00
WPA: Replay Counter: 00 00 00 00 00 00 00 04
WPA: Sending EAPOL-Key 2/4
WPA: Send EAPOL-Key frame to 6e:33:ea:00:60:2d ver=2 mic_len=16 key_mgmt=0x2
WPA: EAPOL-Key MIC using HMAC-SHA1
WPA: Derived Key MIC: 0e 71 34 97 2d 7f 1b ae 51 e3 ed 32 89 0d bd 7d
WPA: TX EAPOL-Key: 01 03 00 75 02 01 0a 00 00 00 00 00 00 00 00 00
04 c3 16 e2 c4 62 a2 3b 54 fa d1 88 8f 75 c3 0e
2b 5e ae e3 5b 83 9e a8 6f 44 53 36 c0 13 3e 45
84 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 0e 71 34 97 2d 7f 1b ae 51 e3 ed 32 89 0d bd
7d 00 16 30 14 01 00 00 0f ac 04 01 00 00 0f ac
04 01 00 00 0f ac 02 0c 00
RX EAPOL from f8:b7:97:87:2c:df (encrypted=-1)
RX EAPOL: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
05 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 a0 86 2c 31 66 18 c0 ef 49 8d eb 2d d1 40 e0
bd 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
IEEE 802.1X RX: version=1 type=3 length=117
WPA: RX EAPOL-Key: 01 03 00 75 02 00 8a 00 10 00 00 00 00 00 00 00
05 df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12
2b 12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d
58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 a0 86 2c 31 66 18 c0 ef 49 8d eb 2d d1 40 e0
bd 00 16 dd 14 00 0f ac 04 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
  EAPOL-Key type=2
  key_info 0x8a (ver=2 keyidx=0 rsvd=0 Pairwise Ack)
  key_length=16 key_data_length=22
  replay_counter: 00 00 00 00 00 00 00 05
  key_nonce: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d 58
  key_iv: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  key_rsc: 00 00 00 00 00 00 00 00
  key_id (reserved): 00 00 00 00 00 00 00 00
  key_mic: a0 86 2c 31 66 18 c0 ef 49 8d eb 2d d1 40 e0 bd
WPA: RX message 1 of 4-Way Handshake from f8:b7:97:87:2c:df (ver=2)
RSN: msg 1/4 key data: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
WPA: PMKID in EAPOL-Key: dd 14 00 0f ac 04 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00
RSN: PMKID from Authenticator: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
RSN: no matching PMKID found
State: 4WAY_HANDSHAKE -> 4WAY_HANDSHAKE
WPA: PTK derivation using PRF(SHA1)
WPA: PTK derivation - A1=c0:b0:3d:07:00:00 A2=6e:33:ea:00:60:2d
WPA: Nonce1: c3 16 e2 c4 62 a2 3b 54 fa d1 88 8f 75 c3 0e 2b
5e ae e3 5b 83 9e a8 6f 44 53 36 c0 13 3e 45 84
WPA: Nonce2: df 6f 5e ab ad 4a 51 fd f2 32 39 6d 08 8e 12 2b
12 8c f5 c8 81 67 af 55 46 a0 ad 38 33 a7 4d 58
WPA: WPA IE for msg 2/4: 30 14 01 00 00 0f ac 04 01 00 00 0f ac 04 01 00
00 0f ac 02 0c 00
WPA: Replay Counter: 00 00 00 00 00 00 00 05
WPA: Sending EAPOL-Key 2/4
WPA: Send EAPOL-Key frame to 6e:33:ea:00:60:2d ver=2 mic_len=16 key_mgmt=0x2
WPA: EAPOL-Key MIC using HMAC-SHA1
WPA: Derived Key MIC: ee e3 fc 1f 47 54 5a 3a 0b a7 b2 f6 e9 f2 f4 5f
WPA: TX EAPOL-Key: 01 03 00 75 02 01 0a 00 00 00 00 00 00 00 00 00
05 c3 16 e2 c4 62 a2 3b 54 fa d1 88 8f 75 c3 0e
2b 5e ae e3 5b 83 9e a8 6f 44 53 36 c0 13 3e 45
84 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 ee e3 fc 1f 47 54 5a 3a 0b a7 b2 f6 e9 f2 f4
5f 00 16 30 14 01 00 00 0f ac 04 01 00 00 0f ac
04 01 00 00 0f ac 02 0c 00
Event DEAUTH (11) received
Deauthentication notification
CTRL-EVENT-DISCONNECTED bssid=6e:33:ea:00:60:2d reason=0
WPA: 4-Way Handshake failed - pre-shared key may be incorrect
CTRL-EVENT-SSID-TEMP-DISABLED id=0 ssid="MSRS_TDF_A5_A11" auth_failures=2 duration=20 reason=WRONG_KEY
Auto connect enabled: try to reconnect (wps=0/0 wpa_state=7)
Setting scan request: 0.100000 sec
Radio work 'connect'@ffff0000073d8300 done in 8.540000 seconds
radio_work_free('connect'@ffff0000073d8300): num_active_works --> 0
Added BSSID 6e:33:ea:00:60:2d into ignore list, ignoring for 10 seconds
Consecutive connection failures: 2 --> request scan in 500 ms
Ignore new scan request for 0.500000 sec since an earlier request is scheduled to trigger sooner
WPA: Clear old PMK and PTK
Disconnect event - remove keys
State: 4WAY_HANDSHAKE -> DISCONNECTED
EAPOL: External notification - portEnabled=0
EAPOL: SUPP_PAE entering state DISCONNECTED
EAPOL: Supplicant port status: Unauthorized
EAPOL: SUPP_BE entering state INITIALIZE
EAPOL: External notification - portValid=0
EAPOL: External notification - EAP success=0
State: DISCONNECTED -> SCANNING
Starting AP scan for wildcard SSID
Add radio work 'scan'@ffff000007352e20
First radio work item in the queue - schedule start immediately
Starting radio work 'scan'@ffff000007352e20 after 0.020000 second wait
[3]bcm4343_control: command: 'escan 5'
[3]bcm4343_control: command: 'escan 0'
Event SCAN_RESULTS (3) received
[3]bcm4343_recv_scan_result: queue num 0
BSS: Start scan result update 6
BSS: f8:b7:97:87:2c:df has multiple entries in the scan results - select the most current one
Previous last_update: 41.920000 (freq 5210)
New last_update: 41.920000 (freq 5210)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 41.920000 (freq 2462)
New last_update: 41.920000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 41.920000 (freq 2462)
New last_update: 41.920000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 41.920000 (freq 2462)
New last_update: 41.920000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 41.920000 (freq 2462)
New last_update: 41.920000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: f8:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 41.920000 (freq 2462)
New last_update: 41.920000 (freq 2462)
Ignore this BSS entry since the previous update looks more current
BSS: fa:b7:97:87:2c:de has multiple entries in the scan results - select the most current one
Previous last_update: 41.920000 (freq 2452)
New last_update: 41.920000 (freq 2452)
Ignore this BSS entry since the previous update looks more current
BSS: a0:95:7f:d3:b4:3c has multiple entries in the scan results - select the most current one
Previous last_update: 41.920000 (freq 2437)
New last_update: 41.920000 (freq 2437)
Ignore this BSS entry since the previous update looks more current
BSS: a0:95:7f:d3:b4:3c has multiple entries in the scan results - select the most current one
Previous last_update: 41.920000 (freq 2437)
New last_update: 41.920000 (freq 2437)
Ignore this BSS entry since the previous update looks more current
BSS: Add new id 9 BSSID 54:9b:49:d3:e7:f2 SSID 'xg100n-d3e7f1-1' freq 2462
BSS: Remove id 8 BSSID 56:9b:49:d3:e7:f2 SSID 'xg100n-d3e7f1-2' due to no match in scan
BSS: last_scan_res_used=%zu/%zu
New scan results available (own=0 ext=0)
Radio work 'scan'@ffff000007352e20 done in 3.310000 seconds
radio_work_free('scan'@ffff000007352e20): num_active_works --> 0
Postpone network selection by 16 seconds since all networks are disabled
Event DISASSOC (1) received
Disassociation notification
Auto connect enabled: try to reconnect (wps=0/0 wpa_state=3)
Do not request new immediate scan
WPA: Clear old PMK and PTK
Disconnect event - remove keys
State: SCANNING -> DISCONNECTED
EAPOL: External notification - portEnabled=0
EAPOL: External notification - portValid=0
```

- Own MAC addressの件

```bash
wpa_s->drv_priv: ffff0000073db0c0
Add interface wlan0 to a new radio N/A
old own_addr: 00:00:00:00:00:00
Failed to attach pkt_type filter
new own_addr: c0:b0:3d:07:00:00
Own MAC address: c0:b0:3d:07:00:00
```

## macアドレス

- raspi wlan:      b8:27:eb:fe:bd:1d
- MSRS_TDF_A5_A11: f8:b7:97:87:2c:df
- MSRS_TDF_A2_S04: f8:b7:97:87:2c:de
```bash
wpasupplicant_init()
  initialize()
    kthread_create("wpa_supplicant", proc_entry, self);
      proc_entry()
        wpa_supplicant_main()
          wpa_supplicant_init()
          wpa_supplicant_add_iface()
            wpa_supplicant_init_iface()
              wpa_supplicant_init_iface()
                wpa_config_read()
                  f_open()
                    mount_volume()
                      follow_path()
                        dir_find()
                          move_window()
                            disk_read()
                              get_block()
                                _load_block()
                                _find_free_entry()
                                _read_entry()
                                  dev_read()
                                    sd_read()
                                      emmc_read()
                                        emmc_do_read()
                                          emmc_do_data_command()
                                            emmc_issue_command()
                                              emmc_issue_command_int()
                                                sdhost_command()
                                                  sdhost_request_sync()
                                                    sdhost_request()
                                                      assert(host->mrq == 0) // ここでアサーションエラー
```




