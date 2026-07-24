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
01234 ok        // こちらは last->block = 0 なのでkmfree()は実行せず　_find_free_entry() が正常終了
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
0123456     // kmfree(last->block)が帰ってこず、_find_free_entry()でストール

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
li2anp.prot'c           // net_init()で問題が発生している
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
[3]net_device_open: dev=net1, state=up      // これのlinkupを待っている。有線LANは抜いていたのでlinkupはしない
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
[2]release:         // unlockのasserttionエラーだと思われる
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
[2]wpa_config_debug_dump_networks:    id=0 ssid='xxx'
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
add_iface 012345[2]wpa_supplicant_init_iface: Initializing interface 'wlan0' conf '/d/wpa_supplicant.conf' driver 'xv6' ctrl_interface 'N/A' bridge 'N/A'
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
[2]wpa_driver_xv6_set_country: Setting country code to 'JP'     // ここでストール
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
kmalloc 124.9.57                                   // kmalloc()でストール
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
