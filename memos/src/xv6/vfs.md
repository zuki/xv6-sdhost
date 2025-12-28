# vfsの実装

[gloworm](https://jabberwocky.ca/projects/gloworm/)のVFS（minix由来）を使用する

## 12月20日

- コンパイルエラーがなくなった
- エラーはv6_initとv6_mountのタイミングが正しくないためと思われる

### 実行画面1

```bash
[0]rand_init: rand_init ok
```

### 実行画面2/3

```bash
[0]console_preinit: console_preinit ok
[0]rand_init: rand_init ok
[0]proc_init: proc_init ok
[0]console_init: console_init ok
[0]tty_init: tty_init ok
[0]sd_init: sd_init ok

[0]init_vfs: init_vfs ok
// 実行2はここで停止。実行3ではvfs_mount()のどこで停止しているか確認
// 以下は vfs_mount(NULL, "/", root_dev, &v6_mount_ops, 0, 0) のデバッグ出力
abocde
```

### 実行画面4/5

```bash
[0]console_preinit: console_preinit ok
[0]rand_init: rand_init ok
[0]proc_init: proc_init ok
[0]console_init: console_init ok
[0]tty_init: tty_init ok
[0]sd_init: sd_init ok

[0]sdhost_probe: firmware sets clock divider
[0]sdhost_set_ios: ios clock 400000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[0]sdhost_finish_command: error detected: CMD 0x4205, HSTS 0x40, EDM 0x10800
[0]sdhost_finish_command: command 5 timeout
[0]emmc_card_reset: OCR: 0xff80, 1.8v support: 0, SDHC support: 1
[0]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[0]emmc_card_reset: card CID: 0x27504853, 0x44333247, 0x506c5d21, 0xcc017421
[0]emmc_card_reset: RCA: 0x5048
// 実行4はここで停止.実行5ではどのsleep()でassertが失敗しているか確認
[0]sd_sleep: [0] pid=?, chan=0xffff0000001b73f0
[0]sleep: [0] pid=?, chan=0xffff0000001b73f0
kern/proc.c:245: assertion failed.
kern/drivers/console.c:264: kernel panic at cpu 0.
```

### 実行画面6/7

```bash
[0]console_preinit: console_preinit ok
[0]rand_init: rand_init ok
[0]proc_init: proc_init ok
[0]console_init: console_init ok
[0]tty_init: tty_init ok
[0]sd_init: sd_init ok

[0]init_vfs: init_vfs ok
// 実行6ではここで停止。実行7ではsizeを確認
[0]_read_entry: size: -6
read_entry
kern/drivers/console.c:264: kernel panic at cpu 0.
```

- おそらく v6_superblock の読み込みのタイミングが間違っているためではないか

## 12月21日

- init関係の順番を調整

```bash
drivers->init
init_vfs
filesystems->init
	v6_iinit()
		readsb() : v6_superをセット	=> ここではv6_superをセットしない
			sd_read() : ptinfoがセットされていること
vfs_mount(root_dev)
	v6_mount() : v6_superがセットされていること => とりあえず mp->super = NULL
user_init
	get_rootfs() : rootfsがセットされていること : ok
sd_postinit
	emmc_read(): sleepあり　: processがあること : ok
	ptinfo[]のセット : 多分OK
v6_set_super	： 新規作成
```

### 実行1/2

```bash
[0]console_preinit: console_preinit ok
[0]rand_init: rand_init ok
[0]proc_init: proc_init ok
[0]console_init: console_init ok
[0]tty_init: tty_init ok
[0]sd_init: sd_init ok

[0]init_vfs: init_vfs ok
// 実行1はここで停止。v6_mount()とv6_iget()にデバッグ出力
abocde1A..................................................B
```

- vfs_init_vnode()のtimespecが0の場合の処理を考慮していなかった

### 実行3

```bash
[0]console_preinit: console_preinit ok
[0]rand_init: rand_init ok
[0]proc_init: proc_init ok
[0]console_init: console_init ok
[0]tty_init: tty_init ok
[0]sd_init: sd_init ok

[0]init_vfs: init_vfs ok
abocde1A..................................................BCD23
f
[0]user_init: user_init start: icode = 0x8cc20, size = 0x30
[0]user_init: p->pid = 1
[0]user_init: p->cwd->ino = 1
[0]user_init: user_init ok
[0]timer_init: [0] timer_init ok
[3]timer_init: [3] timer_init ok
[2]timer_init: [2] timer_init ok
[1]timer_init: [1] timer_init ok
[2]trap_init: [2] trap_init ok
[3]trap_init: [3] trap_init ok
[0]trap_init: [0] trap_init ok
[0]main: cpu 0 init finished
[3]main: cpu 3 init finished
[0]sdhost_probe: firmware sets clock divider
[2]main: cpu 2 init finished
[1]trap_init: [1] trap_init ok
[1]main: cpu 1 init finished
[0]sdhost_set_ios: ios clock 400000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[0]sdhost_finish_command: error detected: CMD 0x4205, HSTS 0x40, EDM 0x10800
[0]sdhost_finish_command: command 5 timeout
[0]emmc_card_reset: OCR: 0xff80, 1.8v support: 0, SDHC support: 1
[0]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[0]emmc_card_reset: card CID: 0x27504853, 0x44333247, 0x506c5d21, 0xcc017421
[0]emmc_card_reset: RCA: 0x5048
[2]emmc_card_reset: SCR: version 3.0x, bus_widths 0x5
[2]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 1, timing 0, vdd 0, drv_type 0
[2]emmc_card_reset: found valid version 3.0x SD card
[3]sd_postinit: partition[0]: TYPE: 12, LBA = 0x800, #SECS = 0x20000
[3]sd_postinit: partition[1]: TYPE: 131, LBA = 0x20800, #SECS = 0x1f800
[3]sd_postinit: sd_postinit ok

[3]release: name: card  // spinlock.c#release()でcardlockのlock/unlock
release: not locked
kern/drivers/console.c:264: kernel panic at cpu 3.
```

- spinlock.release()のエラーはとりあえず無視する

### 実行4/5

```bash
[0]console_preinit: console_preinit ok
[0]rand_init: rand_init ok
[0]proc_init: proc_init ok
[0]console_init: console_init ok
[0]tty_init: tty_init ok
[0]sd_init: sd_init ok

[0]init_vfs: init_vfs ok
abocdef
[0]user_init: user_init start: icode = 0x8cc20, size = 0x30
[0]user_init: p->pid = 1
[0]user_init: p->cwd->ino = 1
[0]user_init: user_init ok
[0]timer_init: [0] timer_init ok
[1]timer_init: [1] timer_init ok
[3]timer_init: [3] timer_init ok
[1]trap_init: [1] trap_init ok
[0]trap_init: [0] trap_init ok
[1]main: cpu 1 init finished
[0]main: cpu 0 init finished
[2]timer_init: [2] timer_init ok
[3]trap_init: [3] trap_init ok
[3]main: cpu 3 init finished
[2]trap_init: [2] trap_init ok
[1]sdhost_probe: firmware sets clock divider
[2]main: cpu 2 init finished
[1]sdhost_set_ios: ios clock 400000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[1]sdhost_finish_command: error detected: CMD 0x4205, HSTS 0x40, EDM 0x10800
[1]sdhost_finish_command: command 5 timeout
[1]emmc_card_reset: OCR: 0xff80, 1.8v support: 0, SDHC support: 1
[1]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[1]emmc_card_reset: card CID: 0x27504853, 0x44333247, 0x506c5d21, 0xcc017421
[1]emmc_card_reset: RCA: 0x5048
[1]emmc_card_reset: SCR: version 3.0x, bus_widths 0x5
[1]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 1, timing 0, vdd 0, drv_type 0
[1]emmc_card_reset: found valid version 3.0x SD card
[2]sd_postinit: partition[0]: TYPE: 12, LBA = 0x800, #SECS = 0x20000
[2]sd_postinit: partition[1]: TYPE: 131, LBA = 0x20800, #SECS = 0x1f800
[2]sd_postinit: sd_postinit ok

[2]release: panic: not locked card
[3]usb_dev_init: Device ven424-2514, dev9-0-2 found
[3]usb_function_get_if_name: func name=int9-0-1
[3]usb_dev_init: Interface int9-0-1 found
[3]usb_dev_init: Function is not supported
[3]usb_function_get_if_name: func name=int9-0-2
[3]usb_dev_init: Interface int9-0-2 found
[3]usb_dev_factory_get_device: Using device/interface int9-0-2
[2]usb_dev_init: Device ven424-2514, dev9-0-2 found
[2]usb_function_get_if_name: func name=int9-0-1
[2]usb_dev_init: Interface int9-0-1 found
[2]usb_dev_init: Function is not supported
[2]usb_function_get_if_name: func name=int9-0-2
[2]usb_dev_init: Interface int9-0-2 found
[2]usb_dev_factory_get_device: Using device/interface int9-0-2
[2]usb_dev_init: Device ven424-7800 found
[2]usb_dev_factory_get_device: Using device/interface ven424-7800
[1]lan7800_init_macaddr: MAC address is b8:27:eb:ab:e8:48
[3]usb_standard_hub_enumerate_ports: Port 1: Device configured
[1]usb_standard_hub_enumerate_ports: Port 1: Device configured
[2]dwhc_root_port_init: Device configured
[2]usb_init: dwhc initialized

[2]net_device_register: dev=net0, type=2 (ETHERNET)
[2]usb_init: usb_init ok
[2]net_protocol_register: type=0x0800 (IP)
[2]net_protocol_register: type=0x0806 (ARP)
[2]ip_protocol_register: type=1 (ICMP)
[2]ip_protocol_register: type=17 (UDP)
[2]ip_protocol_register: type=6 (TCP)
[2]ip_route_add: route added: network=192.168.10.0, netmask=255.255.255.0, nexthop=0.0.0.0, iface=192.168.10.110 dev=net0
[2]ip_iface_register: registered: dev=net0, unicast=192.168.10.110, netmask=255.255.255.0, broadcast=192.168.10.255
[2]ip_route_add: route added: network=0.0.0.0, netmask=0.0.0.0, nexthop=192.168.10.1, iface=192.168.10.110 dev=net0
[2]net_init: net_init ok
[2]net_device_open: dev=net0, state=up
[1]netrun: running...
[1]execve: namei bad    // 実行4. 実行5はexec.cのデバッグ行を変更
[1]execve: vfs_lookup failed: cwd->ino: 1, path: /init, err: -20 : ENOTDIR
[2]execve: cwd: mode: 0x0, rdev: 0x101  // modeが設定されてない
[1]trap: unknown trap code: 37 at 0xffff0000000a461c with 0x5555555555555565
[1]exit: exit: pid 1, err 1
```

- v6_mount()でmodeをセット

### 実行6

```bash
[3]netrun: running...
dirlookup not DIR       //
kern/drivers/console.c:264: kernel panic at cpu 3.
```

### 実行7

```bash
[3]execve: path='/init', argv=0x0, envp=0x0
[3]v6_lookup: ino: 1, mode: 0x41ed, ip: 0x0,  // mode = 040755 = S_IFDIR | 0755 でOK
[3]v6_dirlookup: dirlookup not DIR: dp: 0x0, name: init
[2]v6_dump: vnode : 0xd2801ba8
[2]v6_dump:     mp: 0xd2800001
[2]v6_dump:    ref: 0xd4000001
[2]v6_dump:   mode: 0xd2800f88
[2]v6_dump:  nlink: 0xd4000001
[2]v6_dump:   bits: 0x74
[2]v6_dump:   rdev: 0xd503201f
[2]v6_dump:    ino: 0x41455555
[2]v6_dump:   size: 0x55157451
[2]v6_dump:   data: 0x51757555
[2]v6_dump: inode : 0x0
[2]v6_dump:  vnode: 0x0
[2]v6_dump:  valid: 152
[2]v6_dump:   type: 156
[2]execve: vfs_lookup failed: cwd->ino: 1, path: /init, err: -2
[2]execve: cwd: mode: 0x41ed, rdev: 0x101
[2]trap: unknown trap code: 37 at 0xffff0000000a4a2c with 0x5154555555545565
[2]exit: exit: pid 1, err 1
```

- '/init'を'/bin/init'に置くようmkfsを変更していた
- icode.Sを修正

```bash
[1]execve: path='/bin/init', argv=0x0, envp=0x0
[1]v6_lookup: ino: 1, mode: 0x41ed, ip: 0x0,    // '/' のv6_inodeがセットされていない
[1]v6_dirlookup: dirlookup not DIR: dp: 0x0, name: bin
[1]v6_dump: vnode : 0xd2801ba8
[1]v6_dump:     mp: 0xd2800001
[1]v6_dump:    ref: 0xd4000001
[1]v6_dump:   mode: 0xd2800f88
[1]v6_dump:  nlink: 0xd4000001
[1]v6_dump:   bits: 0x696e692f
[1]v6_dump:   rdev: 0x74
[1]v6_dump:    ino: 0x51455555
[1]v6_dump:   size: 0x55155555
[1]v6_dump:   data: 0x55757555
[1]v6_dump: inode : 0x0
[1]v6_dump:  vnode: 0x0
[1]v6_dump:  valid: 152
[1]v6_dump:   type: 156
[1]execve: vfs_lookup failed: cwd->ino: 1, path: /bin/init, err: -2
[1]execve: cwd: mode: 0x41ed, rdev: 0x101
[1]trap: unknown trap code: 37 at 0xffff0000000a5a9c with 0x5554555555555565
[1]exit: exit: pid 1, err 1
```

- v6_iget()でv6_inodeを作成した際、inode.vnode.dataにinodeをセットしていなかった

### 実行8

```bash
[3]execve: path='/bin/init', argv=0x0, envp=0x0
[3]vfs_lookup: cwd->ino: 1, path: /bin/init, flags: 0x0
[3]vfs_lookup: cur->ino: 1
[3]v6_lookup: ino: 1, mode: 0x41ed, ip: 0x1c2218,
[3]v6_dirlookup: dirlookup not DIR: dp: 0x1c2218, name: bin
[3]v6_dump: vnode : 0xb71c0
[3]v6_dump:     mp: 0x1c6358
[3]v6_dump:    ref: 0x3
[3]v6_dump:   mode: 0x41ed
[3]v6_dump:  nlink: 0x1
[3]v6_dump:   bits: 0x0
[3]v6_dump:   rdev: 0x101
[3]v6_dump:    ino: 0x1
[3]v6_dump:   size: 0x0
[3]v6_dump:   data: 0x1c2218
[3]v6_dump: inode : 0x1c2218
[3]v6_dump:  vnode: 0x1c2218
[3]v6_dump:  valid: 1843888
[3]v6_dump:   type: 1843892
[3]execve: vfs_lookup failed: cwd->ino: 1, path: /bin/init, err: -2 (ENOENT)
[3]execve: cwd: mode: 0x41ed, rdev: 0x101
[3]trap: unknown trap code: 37 at 0xffff0000000a5aac with 0x5154555555545565
[3]exit: exit: pid 1, err 1
```

- v6_mount()でv6_iget()した後、v6_ilock()してファイルから読み込むようにした

```bash
[0]init_vfs: init_vfs ok
[0]v6_iget: dev: 0x101, ino: 1
=== dump vnode: 0x1c2218 ===
   ops: 0xb71c0
    mp: 0x1c6358
   ref: 0x1
  mode: 0x0
 nlink: 0x1
  bits: 0x0
  rdev: 0x101
   ino: 0x1
  size: 0x0
  data: 0x1c2218
inode : 0x1c2218
 vnode: 0x1c2218
 valid: 0
  type: 0
==========================
[0]v6_ilock: bno: 0x20
[0]_read_entry: READING 0x101: 0xbe0080 <- 0x20000 x 0x1000
[0]_read_entry: size: -6
read_entry
kern/drivers/console.c:264: kernel panic at cpu 0.
```

- sdの読み込みができていない

```bash
[0]console_preinit: console_preinit ok
[0]rand_init: rand_init ok
[0]proc_init: proc_init ok
[0]register_driver: drv_table[2] = 0xb4150
[0]console_init: console_init ok
[0]register_driver: drv_table[3] = 0xb41d0
[0]tty_init: tty_init ok
[0]register_driver: drv_table[1] = 0xb4108
[0]sd_init: sd_init ok

[0]init_vfs: init_vfs ok
[0]v6_iget: dev: 0x101, ino: 1
=== dump vnode: 0x1c3218 ===
   ops: 0xb81c0
    mp: 0x1c7358
   ref: 0x1
  mode: 0x0
 nlink: 0x1
  bits: 0x0
  rdev: 0x101
   ino: 0x1
  size: 0x0
  data: 0x1c3218
inode : 0x1c3218
 vnode: 0x1c3218
 valid: 0
  type: 0
==========================
[0]v6_ilock: bno: 0x20
[0]_read_entry: READING 0x101: 0xbe0080 <- 0x20000 x 0x1000
U]UUUUE?W]UU?uQ?uuUUUUUUUU5UUUUUUUEW_UWUUGUu?UT?UUU???UU]UUWU?UWUEU_UUUUUUUUUQU}uUUuuUW?UUUUUUUUUUUWUWUUU?uUU?UWuU?UUUUUwU]U]UUWUUUUWUuUUUUUU
[0]_read_entry: size: -6
read_entry
kern/drivers/console.c:264: kernel panic at cpu 0.
```

## 12月22日

- vfs_lookup周りをチェックし、sd.cのブロック番号とseekの関係を調整

```bash
[0]console_preinit: console_preinit ok
[0]rand_init: rand_init ok
[0]proc_init: proc_init ok
[0]register_driver: drv_table[2] = 0xb4150
[0]console_init: console_init ok
[0]register_driver: drv_table[3] = 0xb41d0
[0]tty_init: tty_init ok
[0]register_driver: drv_table[1] = 0xb4108
[0]sd_init: sd_init ok

[0]init_vfs: init_vfs ok
[0]v6_iget: dev: 0x101, ino: 1
=== dump vnode: 0x1c3208 ===
   ops: 0xb81c0
    mp: 0x1c7348
   ref: 0x1
  mode: 0x0
 nlink: 0x1
  bits: 0x0
  rdev: 0x101
   ino: 0x1
  size: 0x0
  data: 0x1c3208
inode : 0x1c3208
 vnode: 0x1c3208
 valid: 0
  type: 0
==========================
[0]user_init: user_init ok
[0]timer_init: [0] timer_init ok
[3]timer_init: [3] timer_init ok
[0]trap_init: [0] trap_init ok
[2]timer_init: [2] timer_init ok
[3]trap_init: [3] trap_init ok
[2]trap_init: [2] trap_init ok
[1]timer_init: [1] timer_init ok
[2]main: cpu 2 init finished
[3]main: cpu 3 init finished
[0]main: cpu 0 init finished
[1]trap_init: [1] trap_init ok
[2]sdhost_probe: firmware sets clock divider
[1]main: cpu 1 init finished
[2]sdhost_set_ios: ios clock 400000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[2]sdhost_finish_command: error detected: CMD 0x4205, HSTS 0x40, EDM 0x10800
[2]sdhost_finish_command: command 5 timeout
[2]emmc_card_reset: OCR: 0xff80, 1.8v support: 0, SDHC support: 1
[2]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[2]emmc_card_reset: card CID: 0x27504853, 0x44333247, 0x506c5d21, 0xcc017421
[2]emmc_card_reset: RCA: 0x5048
[3]emmc_card_reset: SCR: version 3.0x, bus_widths 0x5
[3]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 1, timing 0, vdd 0, drv_type 0
[3]emmc_card_reset: found valid version 3.0x SD card
[2]sd_postinit: partition[0]: TYPE: 12, LBA = 0x800, #SECS = 0x20000
[2]sd_postinit: partition[1]: TYPE: 131, LBA = 0x20800, #SECS = 0x1f800
[2]sd_postinit: sd_postinit ok

[2]_read_entry: read: dev: 0x101, buffer: 0xbd0080, bno: 0x1, size: 0x1000
[2]sd_read: minor: 1, buffer: 0xbd0080, offset: 0x1, size: 0x1000
[2]sd_read: lba: 0x20800, offset: 8, bno: 0x20808 (0x4101000 byte), size: 0x1000
[2]release: error: card is not locked
[3]v6_set_super: v6_sb: size 1000 nblocks 960 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 39 // v6_superblockが正しく読めた
[3]usb_dev_init: Device ven424-2514, dev9-0-2 found
[3]usb_function_get_if_name: func name=int9-0-1
[3]usb_dev_init: Interface int9-0-1 found
[3]usb_dev_init: Function is not supported
[3]usb_function_get_if_name: func name=int9-0-2
[3]usb_dev_init: Interface int9-0-2 found
[3]usb_dev_factory_get_device: Using device/interface int9-0-2
[3]usb_dev_init: Device ven424-2514, dev9-0-2 found
[3]usb_function_get_if_name: func name=int9-0-1
[3]usb_dev_init: Interface int9-0-1 found
[3]usb_dev_init: Function is not supported
[3]usb_function_get_if_name: func name=int9-0-2
[3]usb_dev_init: Interface int9-0-2 found
[3]usb_dev_factory_get_device: Using device/interface int9-0-2
[2]usb_dev_init: Device ven424-7800 found
[2]usb_dev_factory_get_device: Using device/interface ven424-7800
[1]lan7800_init_macaddr: MAC address is b8:27:eb:ab:e8:48
[2]usb_standard_hub_enumerate_ports: Port 1: Device configured
[1]usb_standard_hub_enumerate_ports: Port 1: Device configured
[1]dwhc_root_port_init: Device configured
[1]usb_init: dwhc initialized

[1]net_device_register: dev=net0, type=2 (ETHERNET)
[1]usb_init: usb_init ok
[1]net_protocol_register: type=0x0800 (IP)
[1]net_protocol_register: type=0x0806 (ARP)
[1]ip_protocol_register: type=1 (ICMP)
[1]ip_protocol_register: type=17 (UDP)
[1]ip_protocol_register: type=6 (TCP)
[1]ip_route_add: route added: network=192.168.10.0, netmask=255.255.255.0, nexthop=0.0.0.0, iface=192.168.10.110 dev=net0
[1]ip_iface_register: registered: dev=net0, unicast=192.168.10.110, netmask=255.255.255.0, broadcast=192.168.10.255
[1]ip_route_add: route added: network=0.0.0.0, netmask=0.0.0.0, nexthop=192.168.10.1, iface=192.168.10.110 dev=net0
[1]net_init: net_init ok
[1]net_device_open: dev=net0, state=up
[3]netrun: running...
[3]execve: path='/bin/init', argv=0x0, envp=0x0
[3]vfs_lookup: cwd->ino: 1, path: /bin/init, flags: 0x0
[3]vfs_lookup: [L186] cur->ino: 1, ref: 3
[3]vfs_lookup: componet: bin
[3]vfs_lookup: flags: 0, path[i]: i
[3]v6_lookup: vnode->ino: 1, mode: 0x41ed, ip->valid: 0, filename: bin
[3]v6_ilock: vp->ino: 1, valid: 0
[3]v6_ilock: super: 0xb8220
[3]v6_ilock: bno2: 0x20, v6_sb: 0xbfec40, inostart: 0x20
[3]_read_entry: read: dev: 0x101, buffer: 0xbd1080, bno: 0x20, size: 0x1000
[3]sd_read: minor: 1, buffer: 0xbd1080, offset: 0x20, size: 0x1000
[3]sd_read: lba: 0x20800, offset: 0x100, bno: 0x20900 (0x4120000 byte), size: 0x1000    // このbnoは正しいがここでストール (sleepしてwakeupしない?)
```


```bash
[1]execve: path='/bin/init', argv=0x0, envp=0x0
[1]vfs_lookup: cwd->ino: 1, path: /bin/init, flags: 0x0
[1]vfs_lookup: [L186] cur->ino: 1, ref: 3
[1]vfs_lookup: componet: bin
[1]vfs_lookup: flags: 0, path[i]: i
[1]v6_lookup: vnode->ino: 1, mode: 0x41ed, ip->valid: 0, filename: bin
[1]v6_ilock: vp->ino: 1, valid: 0
[1]v6_ilock: bno: 0x20, v6_sb: 0xbfec40, inostart: 0x20
[1]_read_entry: read: dev: 0x101, buffer: 0xbd1080, bno: 0x20, size: 0x1000
[1]sd_read: minor: 1, buffer: 0xbd1080, offset: 0x20, size: 0x1000
[1]sd_read: lba: 0x20800, offset: 0x100, bno: 0x20900 (0x4120000 byte), size: 0x1000
[2]v6_ilock: bp: 0xbd1080, bno: 0x20, dev: 0x101, ref: 1
[2]v6_ilock: dip: 0xbd1100
[1]v6_ilock: type: 1, nlink: 1, rdev: 0x0, size: 0x1000, mode: 0x41fd, addrs[0]: 0x28   // rdevが0はまずい
=== bno=0x20900 dump ===    // これは inodeブロックの最初の512バイトで正しい
+------+-------------------------------------------------+------------------+
| 0000 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
| 0010 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
| 0020 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
| 0030 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
| 0040 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
| 0050 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
| 0060 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
| 0070 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
| 0080 | 01 00 01 00 00 00 00 00 00 10 00 00 fd 41 00 00 | .............A.. |
| 0090 | 00 00 00 00 00 00 00 00 a4 bc 47 69 00 00 00 00 | ..........Gi.... |
| 00a0 | 98 3c 2d 30 00 00 00 00 a4 bc 47 69 00 00 00 00 | .<-0......Gi.... |
| 00b0 | 98 3c 2d 30 00 00 00 00 a4 bc 47 69 00 00 00 00 | .<-0......Gi.... |
| 00c0 | 98 3c 2d 30 00 00 00 00 28 00 00 00 00 00 00 00 | .<-0....(....... |
| 00d0 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
| 00e0 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
| 00f0 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
| 0100 | 01 00 01 00 00 00 00 00 40 03 00 00 fd 41 00 00 | ........@....A.. |
| 0110 | 00 00 00 00 00 00 00 00 a4 bc 47 69 00 00 00 00 | ..........Gi.... |
| 0120 | 58 17 2e 30 00 00 00 00 a4 bc 47 69 00 00 00 00 | X..0......Gi.... |
| 0130 | 58 17 2e 30 00 00 00 00 a4 bc 47 69 00 00 00 00 | X..0......Gi.... |
| 0140 | 58 17 2e 30 00 00 00 00 29 00 00 00 00 00 00 00 | X..0....)....... |
| 0150 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
| 0160 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
| 0170 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
| 0180 | 01 00 01 00 00 00 00 00 c0 00 00 00 fd 41 00 00 | .............A.. |
| 0190 | 00 00 00 00 00 00 00 00 a4 bc 47 69 00 00 00 00 | ..........Gi.... |
| 01a0 | 20 c7 2e 30 00 00 00 00 a4 bc 47 69 00 00 00 00 |  ..0......Gi.... |
| 01b0 | 20 c7 2e 30 00 00 00 00 a4 bc 47 69 00 00 00 00 |  ..0......Gi.... |
| 01c0 | 20 c7 2e 30 00 00 00 00 2a 00 00 00 00 00 00 00 |  ..0....*....... |
| 01d0 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
| 01e0 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
| 01f0 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
+------+-------------------------------------------------+------------------+
```

- v6_ilock()でget_block()したバッファをdipにセットするところまでは問題なし、
- その後、put_block(bp); release_block(bp, 0); としているが、そのどこかでフリーズ

## 12月23日

- blockcacheとv6_inodeでsleeplockが使われており、これを同時に取得する場面があった。
- blockcacheのsleeplockをflagsに変更
- mkfsでv6_dinode.modeを設定していなかったので設定

```bash
[3]execve: path='/bin/init', argv=0x0, envp=0x0
[3]vfs_lookup: cwd->ino: 1, path: /bin/init, flags: 0x0
[3]vfs_lookup: [L186] cur->ino: 1, ref: 3
[3]vfs_lookup: componet: bin
[3]vfs_lookup: flags: 0, path[i]: i
[3]v6_lookup: vnode->ino: 1, mode: 0x41ed, ip->valid: 0, filename: bin
[3]v6_ilock: vp->ino: 1, valid: 0
[3]v6_ilock: bno: 0x20, v6_sb: 0xbfec40, inostart: 0x20
[3]_read_entry: read: dev: 0x101, buffer: 0xbd1080, bno: 0x20, size: 0x1000
[3]sd_read: minor: 1, buffer: 0xbd1080, offset: 0x20, size: 0x1000
[3]sd_read: lba: 0x20800, offset: 0x100, bno: 0x20900 (0x4120000 byte), size: 0x1000
[2]v6_ilock: type: 1, nlink: 1, rdev: 0x41fd, size: 0x1000, mode: 0x41fd, addrs[0]: 0x28
[2]v6_dirlookup: dp->ino: 1, name: bin
[2]trap: unknown trap code: 37 at 0xffff0000000a42e0 with 0xd2800002d2800071
[2]exit: exit: pid 1, err 1
```

- エラー箇所は以下のとおり

```bash
ffff0000000a42c0 <v6_read>:
{
ffff0000000a42c0:   a9bd7bfd    stp x29, x30, [sp, #-48]!
ffff0000000a42c4:   910003fd    mov x29, sp
ffff0000000a42c8:   a90153f3    stp x19, x20, [sp, #16]
ffff0000000a42cc:   aa0003f3    mov x19, x0     // x19 = *file
ffff0000000a42d0:   aa0103f4    mov x20, x1     // x20 = *buffer
    struct v6_inode *ip = FTOI(file);
ffff0000000a42d4:   f9400400    ldr x0, [x0, #8]    // x0 = file->vnode
{
ffff0000000a42d8:   a9025bf5    stp x21, x22, [sp, #32]
ffff0000000a42dc:   aa0203f6    mov x22, x2     // x22 = size
    struct v6_inode *ip = FTOI(file);
ffff0000000a42e0:   f9403815    ldr x21, [x0, #112] // x21 = file->vnode->data
    v6_ilock(ip);
ffff0000000a42e4:   aa1503e0    mov x0, x21
ffff0000000a42e8:   97fffb5a    bl  ffff0000000a3050 <v6_ilock>
```

- v6_dirlookup()でget_vnode(dir)がNULL => dirはopenはしていないのでfd_tableからは
  とれない

```bash
[1]v6_dirlookup: dp->ino: 1, name: bin
[1]get_vnode: no hit
[1]dir_find_entry_by_name: dir: 0x1c3208, file: 0x0
[1]v6_read: file: 0x0,
[1]v6_read: vnode: 0xd2800001
[1]v6_read:
```

- v6_read(FTOI)ではなく、v6_readi(VTOI)を使う
- dir_find_entry_by_name/ino()でoffsetを返し、v6_write_dirent()でoffsetを
  指定するよう変更

```bash
[1]execve: path='/bin/init', argv=0x0, envp=0x0
[1]vfs_lookup: cwd->ino: 1, path: /bin/init, flags: 0x0
[1]vfs_lookup: [L186] cur->ino: 1, ref: 3
[1]vfs_lookup: componet: bin
[1]vfs_lookup: flags: 0, path[i]: i
[1]v6_lookup: vnode->ino: 1, mode: 0x41ed, ip->valid: 0, filename: bin
[1]v6_ilock: vp->ino: 1, valid: 0
[1]v6_ilock: bno: 0x20, v6_sb: 0xbfec40, inostart: 0x20
[1]_read_entry: read: dev: 0x101, buffer: 0xbd1080, bno: 0x20, size: 0x1000
[1]sd_read: minor: 1, buffer: 0xbd1080, offset: 0x20, size: 0x1000
[1]sd_read: lba: 0x20800, offset: 0x100, bno: 0x20900 (0x4120000 byte), size: 0x1000
[3]v6_ilock: type: 1, nlink: 1, rdev: 0x41fd, size: 0x1000, mode: 0x41fd, addrs[0]: 0x28
[3]v6_dirlookup: dp->ino: 1, name: bin
[3]_read_entry: read: dev: 0x41fd, buffer: 0xbd2080, bno: 0x28, size: 0x1000
[3]_read_entry: size: -6
read_entry
kern/drivers/console.c:264: kernel panic at cpu 3.
```

```bash
[1]execve: path='/bin/init', argv=0x0, envp=0x0
[1]vfs_lookup: cwd->ino: 1, path: /bin/init, flags: 0x0
[1]vfs_lookup: LP[1] cur->ino: 1, ref: 3
[1]v6_ilock: slock: ino=1
[1]_read_entry: read: dev: 0x101, buffer: 0xbd1080, bno: 0x20, size: 0x1000
[1]v6_ilock: type: 1, nlink: 1, rdev: 0x101, size: 0x1000, mode: 0x41fd, addrs[0]: 0x28
[1]v6_dirlookup: dp->ino: 1, name: bin
[1]_read_entry: read: dev: 0x101, buffer: 0xbd2080, bno: 0x28, size: 0x1000
[2]v6_iunlock: relslock: ino=1
[2]v6_iput: slock: ino=1
[2]v6_iput: relslock: ino=1
[2]v6_lookup: OK: bin, ip->ino: 2
[2]v6_ilock: slock: ino=2
[2]v6_ilock: type: 1, nlink: 1, rdev: 0x101, size: 0x340, mode: 0x41fd, addrs[0]: 0x29
[2]vfs_lookup: LP[2] cur->ino: 2, ref: 1
[2]v6_ilock: slock: ino=2
```

- sleeeplockをreleaseせずにロックしようとしている.
- v6_lookup()でv6_ilock()した後、v6_iunlock()するようにした

```bash
[1]execve: path='/bin/init', argv=0x0, envp=0x0
[1]vfs_lookup: cwd->ino: 1, path: /bin/init, flags: 0x0
[1]vfs_lookup: LP[1] cur->ino: 1, ref: 3
[1]v6_ilock: slock: ino=1
[1]_read_entry: read: dev: 0x101, buffer: 0xbd1080, bno: 0x20, size: 0x1000
[3]v6_ilock: type: 1, nlink: 1, rdev: 0x101, size: 0x1000, mode: 0x41fd, addrs[0]: 0x28
[3]v6_dirlookup: dp->ino: 1, name: bin
[3]_read_entry: read: dev: 0x101, buffer: 0xbd2080, bno: 0x28, size: 0x1000
[1]v6_iunlock: relslock: ino=1
[1]v6_iput: slock: ino=1
[1]v6_iput: relslock: ino=1
[1]v6_lookup: OK: bin, ip->ino: 2
[1]v6_ilock: slock: ino=2
[1]v6_ilock: type: 1, nlink: 1, rdev: 0x101, size: 0x340, mode: 0x41fd, addrs[0]: 0x29
[1]v6_iunlock: relslock: ino=2
[1]vfs_lookup: LP[2] cur->ino: 2, ref: 1
[1]v6_ilock: slock: ino=2
[1]v6_dirlookup: dp->ino: 2, name: init
[1]_read_entry: read: dev: 0x101, buffer: 0xbd3080, bno: 0x29, size: 0x1000
[1]v6_iunlock: relslock: ino=2
[1]v6_iput: slock: ino=2
[1]v6_iput: relslock: ino=2
[1]v6_lookup: OK: init, ip->ino: 6
[1]v6_ilock: slock: ino=6
[1]v6_ilock: type: 2, nlink: 1, rdev: 0x101, size: 0x5790, mode: 0x81ed, addrs[0]: 0x35
[1]v6_iunlock: relslock: ino=6
[1]vfs_lookup: LP[3] cur->ino: 6, ref: 1
[1]vfs_lookup: OK: cur->ino: 6, ref: 1
[1]get_vnode: no hit
[1]trap: unknown trap code: 37 at 0xffff0000000a59c8 with 0x100000e0d2801bb8
[1]exit: exit: pid 1, err 1
```

- vfs_release_vnode(), v6_release()におけるrefcount==0近辺の処理の整合性チェック

```bash
[1]execve: path='/bin/init', argv=0x0, envp=0x0
[1]vfs_lookup: cwd->ino: 1, path: /bin/init, flags: 0x0
[1]vfs_lookup: LP[1] cur->ino: 1, ref: 3
[1]v6_ilock: slock: ino=1
[1]_read_entry: read: dev: 0x101, buffer: 0xbd1080, bno: 0x20, size: 0x1000
[2]v6_ilock: type: 1, nlink: 1, rdev: 0x101, size: 0x1000, mode: 0x41fd, addrs[0]: 0x28
[2]v6_dirlookup: dp->ino: 1, name: bin
[2]_read_entry: read: dev: 0x101, buffer: 0xbd2080, bno: 0x28, size: 0x1000
[2]v6_iunlock: relslock: ino=1
[2]v6_iput: slock: ino=1
[2]v6_iput: relslock: ino=1
[2]v6_lookup: OK: bin, ip->ino: 2
[2]v6_ilock: slock: ino=2
[2]v6_ilock: type: 1, nlink: 1, rdev: 0x101, size: 0x340, mode: 0x41fd, addrs[0]: 0x29
[2]v6_iunlock: relslock: ino=2
[2]vfs_lookup: LP[2] cur->ino: 2, ref: 1
[2]v6_ilock: slock: ino=2
[2]v6_dirlookup: dp->ino: 2, name: init
[2]_read_entry: read: dev: 0x101, buffer: 0xbd3080, bno: 0x29, size: 0x1000
[3]v6_iunlock: relslock: ino=2
[3]v6_iput: slock: ino=2
[3]v6_iput: relslock: ino=2
[3]v6_lookup: OK: init, ip->ino: 6
[3]v6_ilock: slock: ino=6
[3]v6_ilock: type: 2, nlink: 1, rdev: 0x101, size: 0x5790, mode: 0x81ed, addrs[0]: 0x35
[3]v6_iunlock: relslock: ino=6
[3]vfs_lookup: LP[3] cur->ino: 6, ref: 1
[3]vfs_lookup: OK: cur->ino: 6, ref: 1
[3]vfs_release_vnode: refcoutn=1
[3]v6_ilock: slock: ino=6
[3]v6_iunlock: relslock: ino=6
[3]v6_iput: slock: ino=6
[3]v6_iput: relslock: ino=6
[3]vfs_lookup: cwd->ino: 1, path: /bin/init, flags: 0x0
[3]vfs_lookup: LP[1] cur->ino: 1, ref: 4
[3]v6_ilock: slock: ino=1
[3]v6_dirlookup: dp->ino: 1, name: bin
[3]v6_iget: hit ip->ino: 2
[3]v6_iunlock: relslock: ino=1
[3]v6_iput: slock: ino=1
[3]v6_iput: relslock: ino=1
[3]v6_lookup: OK: bin, ip->ino: 2
[3]v6_ilock: slock: ino=2
[3]v6_iunlock: relslock: ino=2
[3]vfs_lookup: LP[2] cur->ino: 2, ref: 2
[3]v6_ilock: slock: ino=2
[3]v6_dirlookup: dp->ino: 2, name: init
[3]v6_iunlock: relslock: ino=2
[3]v6_iput: slock: ino=2
[3]v6_iput: relslock: ino=2
[3]v6_lookup: OK: init, ip->ino: 6
[3]v6_ilock: slock: ino=6
[3]v6_ilock: type: 2, nlink: 1, rdev: 0x101, size: 0x5790, mode: 0x81ed, addrs[0]: 0x35
[3]v6_iunlock: relslock: ino=6
[3]vfs_lookup: LP[3] cur->ino: 6, ref: 1
[3]vfs_lookup: OK: cur->ino: 6, ref: 1
[3]v6_read: file: 0x1c6300,
[3]v6_read: vnode: 0x1c33b8
[3]v6_read: inode: 0x1c33b8
[3]v6_ilock: slock: ino=6
[3]_read_entry: read: dev: 0x101, buffer: 0xbd4080, bno: 0x35, size: 0x1000
[1]v6_iunlock: relslock: ino=6
[1]execve: elf header check ok
[1]v6_read: file: 0x1c6300,
[1]v6_read: vnode: 0x1c33b8
[1]v6_read: inode: 0x1c33b8
[1]v6_ilock: slock: ino=6
[1]v6_iunlock: relslock: ino=6
[1]v6_read: file: 0x1c6300,
[1]v6_read: vnode: 0x1c33b8
[1]v6_read: inode: 0x1c33b8
[1]v6_ilock: slock: ino=6
[1]_read_entry: read: dev: 0x101, buffer: 0xbd5080, bno: 0x36, size: 0x1000
[3]_read_entry: read: dev: 0x101, buffer: 0xbd6080, bno: 0x37, size: 0x1000
[1]v6_iunlock: relslock: ino=6
[1]v6_read: file: 0x1c6300,
[1]v6_read: vnode: 0x1c33b8
[1]v6_read: inode: 0x1c33b8
[1]v6_ilock: slock: ino=6
[1]v6_iunlock: relslock: ino=6
[1]v6_read: file: 0x1c6300,
[1]v6_read: vnode: 0x1c33b8
[1]v6_read: inode: 0x1c33b8
[1]v6_ilock: slock: ino=6
[1]_read_entry: read: dev: 0x101, buffer: 0xbd7080, bno: 0x38, size: 0x1000
[2]v6_iunlock: relslock: ino=6
[2]v6_read: file: 0x1c6300,
[2]v6_read: vnode: 0x1c33b8
[2]v6_read: inode: 0x1c33b8
[2]v6_ilock: slock: ino=6
[2]v6_iunlock: relslock: ino=6
[2]v6_iput: slock: ino=6
[2]v6_iput: relslock: ino=6
[2]vfs_release_vnode: refcoutn=1
[2]v6_ilock: slock: ino=6
[2]v6_iunlock: relslock: ino=6
[2]v6_iput: slock: ino=6
[2]v6_iput: relslock: ino=6
[2]execve: load file ok
[2]execve: copy argv ok
[2]execve: copy envp ok
[2]execve: argv: 0xffffffffffd8, envp: 0xffffffffffe0, auxv: 0xffffffffffe8
[2]execve: finish init
[2]vfs_lookup:  // ここでストール : exec.cは終了、/bin/initの処理に入った模様
                // おそらく /dev/tty1 のデバイス番号の整合性の問題だと思われる
```

## 12月24日

- usrプログラムで/dev/tty1が正しく設定されていなかった

```bash
[3]execve: path='/bin/init', argv=0x0, envp=0x0
[3]vfs_lookup: cwd->ino: 1, path: /bin/init, flags: 0x0
[3]vfs_lookup: LP[1] cur->ino: 1, ref: 3
[3]v6_ilock: slock: ino=1
[3]_read_entry: read: dev: 0x101, buffer: 0xbd1080, bno: 0x20, size: 0x1000
[1]v6_ilock: type: 1, nlink: 1, rdev: 0x101, size: 0x1000, mode: 0x41fd, addrs[0]: 0x28
[1]v6_dirlookup: dp->ino: 1, name: bin
[1]_read_entry: read: dev: 0x101, buffer: 0xbd2080, bno: 0x28, size: 0x1000
[1]v6_iunlock: relslock: ino=1
[1]v6_iput: slock: ino=1
[1]v6_iput: relslock: ino=1
[1]v6_lookup: OK: bin, ip->ino: 2
[1]v6_ilock: slock: ino=2
[1]v6_ilock: type: 1, nlink: 1, rdev: 0x101, size: 0x340, mode: 0x41fd, addrs[0]: 0x29
[1]v6_iunlock: relslock: ino=2
[1]vfs_lookup: LP[2] cur->ino: 2, ref: 1
[1]v6_ilock: slock: ino=2
[1]v6_dirlookup: dp->ino: 2, name: init
[1]_read_entry: read: dev: 0x101, buffer: 0xbd3080, bno: 0x29, size: 0x1000
[1]v6_iunlock: relslock: ino=2
[1]v6_iput: slock: ino=2
[1]v6_iput: relslock: ino=2
[1]v6_lookup: OK: init, ip->ino: 6
[1]v6_ilock: slock: ino=6
[1]v6_ilock: type: 2, nlink: 1, rdev: 0x101, size: 0x57b0, mode: 0x81ed, addrs[0]: 0x35
[1]v6_iunlock: relslock: ino=6
[1]vfs_lookup: LP[3] cur->ino: 6, ref: 1
[1]vfs_lookup: OK: cur->ino: 6, ref: 1
[1]vfs_release_vnode: ino: 6, refcount = 1
[1]v6_release: ino: 6, refcount: 0
[1]v6_ilock: slock: ino=6
[1]v6_iunlock: relslock: ino=6
[1]v6_iput: slock: ino=6
[1]v6_iput: relslock: ino=6
[1]vfs_lookup: cwd->ino: 1, path: /bin/init, flags: 0x0
[1]vfs_lookup: LP[1] cur->ino: 1, ref: 4
[1]v6_ilock: slock: ino=1
[1]v6_dirlookup: dp->ino: 1, name: bin
[1]v6_iget: hit ip->ino: 2
[1]v6_iunlock: relslock: ino=1
[1]v6_iput: slock: ino=1
[1]v6_iput: relslock: ino=1
[1]v6_lookup: OK: bin, ip->ino: 2
[1]v6_ilock: slock: ino=2
[1]v6_iunlock: relslock: ino=2
[1]vfs_lookup: LP[2] cur->ino: 2, ref: 2
[1]v6_ilock: slock: ino=2
[1]v6_dirlookup: dp->ino: 2, name: init
[1]v6_iunlock: relslock: ino=2
[1]v6_iput: slock: ino=2
[1]v6_iput: relslock: ino=2
[1]v6_lookup: OK: init, ip->ino: 6
[1]v6_ilock: slock: ino=6
[1]v6_ilock: type: 2, nlink: 1, rdev: 0x101, size: 0x57b0, mode: 0x81ed, addrs[0]: 0x35
[1]v6_iunlock: relslock: ino=6
[1]vfs_lookup: LP[3] cur->ino: 6, ref: 1
[1]vfs_lookup: OK: cur->ino: 6, ref: 1
[1]v6_read: file: 0x1c6300,
[1]v6_read: vnode: 0x1c33b8
[1]v6_read: inode: 0x1c33b8
[1]v6_ilock: slock: ino=6
[1]_read_entry: read: dev: 0x101, buffer: 0xbd4080, bno: 0x35, size: 0x1000
[1]v6_iunlock: relslock: ino=6
[1]execve: elf header check ok
[1]v6_read: file: 0x1c6300,
[1]v6_read: vnode: 0x1c33b8
[1]v6_read: inode: 0x1c33b8
[1]v6_ilock: slock: ino=6
[1]v6_iunlock: relslock: ino=6
[1]v6_read: file: 0x1c6300,
[1]v6_read: vnode: 0x1c33b8
[1]v6_read: inode: 0x1c33b8
[1]v6_ilock: slock: ino=6
[1]_read_entry: read: dev: 0x101, buffer: 0xbd5080, bno: 0x36, size: 0x1000
[2]_read_entry: read: dev: 0x101, buffer: 0xbd6080, bno: 0x37, size: 0x1000
[2]_read_entry: read: dev: 0x101, buffer: 0xbd7080, bno: 0x38, size: 0x1000
[1]v6_iunlock: relslock: ino=6
[1]v6_read: file: 0x1c6300,
[1]v6_read: vnode: 0x1c33b8
[1]v6_read: inode: 0x1c33b8
[1]v6_ilock: slock: ino=6
[1]v6_iunlock: relslock: ino=6
[1]v6_read: file: 0x1c6300,
[1]v6_read: vnode: 0x1c33b8
[1]v6_read: inode: 0x1c33b8
[1]v6_ilock: slock: ino=6
[1]v6_iunlock: relslock: ino=6
[1]v6_read: file: 0x1c6300,
[1]v6_read: vnode: 0x1c33b8
[1]v6_read: inode: 0x1c33b8
[1]v6_ilock: slock: ino=6
[1]v6_iunlock: relslock: ino=6
[1]v6_iput: slock: ino=6
[1]v6_iput: relslock: ino=6
[1]vfs_release_vnode: ino: 6, refcount = 1
[1]v6_release: ino: 6, refcount: 0
[1]v6_ilock: slock: ino=6
[1]v6_iunlock: relslock: ino=6
[1]v6_iput: slock: ino=6
[1]v6_iput: relslock: ino=6
[1]execve: load file ok
[1]execve: copy argv ok
[1]execve: copy envp ok
[1]execve: argv: 0xffffffffffd8, envp: 0xffffffffffe0, auxv: 0xffffffffffe8
[1]execve: finish init
[1]sys_openat: dirfd: -100, path: /dev/tty1, flags : 0x20002, mode: 0x0
[1]sys_openat: vnode->ino: 1
[1]sys_openat: fd: 0
[1]sys_openat: vnode->ino: 1, path: /dev/tty1, flags : 0x20002, mode: 0x0
[1]vfs_lookup: cwd->ino: 1, path: /dev/tty1, flags: 0x0
[1]vfs_lookup: LP[1] cur->ino: 1, ref: 5
[1]v6_ilock: slock: ino=1
[1]v6_dirlookup: dp->ino: 1, name: dev
[1]v6_iunlock: relslock: ino=1
[1]v6_iput: slock: ino=1
[1]v6_iput: relslock: ino=1
[1]v6_lookup: OK: dev, ip->ino: 3
[1]v6_ilock: slock: ino=3
[1]v6_ilock: type: 1, nlink: 1, rdev: 0x101, size: 0xc0, mode: 0x41fd, addrs[0]: 0x2a
[1]v6_iunlock: relslock: ino=3
[1]vfs_lookup: LP[2] cur->ino: 3, ref: 1
[1]v6_ilock: slock: ino=3
[1]v6_dirlookup: dp->ino: 3, name: tty1
[1]_read_entry: read: dev: 0x101, buffer: 0xbd8080, bno: 0x2a, size: 0x1000
[2]v6_iunlock: relslock: ino=3
[2]v6_iput: slock: ino=3
[2]v6_iput: relslock: ino=3
[2]v6_lookup: OK: tty1, ip->ino: 4
[2]v6_ilock: slock: ino=4
[2]v6_ilock: type: 3, nlink: 1, rdev: 0x301, size: 0x0, mode: 0x21b6, addrs[0]: 0x0
[2]v6_iunlock: relslock: ino=4
[2]vfs_lookup: LP[3] cur->ino: 4, ref: 1
[2]vfs_lookup: OK: cur->ino: 4, ref: 1
[2]sys_openat: fd: 0, file->vnode->ino: 4
init: starting sh
[1]execve: path='/bin/sh', argv=0x403060, envp=0x403040
[1]vfs_lookup: cwd->ino: 1, path: /bin/sh, flags: 0x0
[1]vfs_lookup: LP[1] cur->ino: 1, ref: 7
[1]v6_ilock: slock: ino=1
[1]v6_dirlookup: dp->ino: 1, name: bin
[1]v6_iget: hit ip->ino: 2
[1]v6_iunlock: relslock: ino=1
[1]v6_iput: slock: ino=1
[1]v6_iput: relslock: ino=1
[1]v6_lookup: OK: bin, ip->ino: 2
[1]v6_ilock: slock: ino=2
[1]v6_iunlock: relslock: ino=2
[1]vfs_lookup: LP[2] cur->ino: 2, ref: 3
[1]v6_ilock: slock: ino=2
[1]v6_dirlookup: dp->ino: 2, name: sh
[1]v6_iunlock: relslock: ino=2
[1]v6_iput: slock: ino=2
[1]v6_iput: relslock: ino=2
[1]v6_lookup: OK: sh, ip->ino: 10
[1]v6_ilock: slock: ino=10
[1]v6_ilock: type: 2, nlink: 1, rdev: 0x101, size: 0xd328, mode: 0x81ed, addrs[0]: 0x5e
[1]v6_iunlock: relslock: ino=10
[1]vfs_lookup: LP[3] cur->ino: 10, ref: 1
[1]vfs_lookup: OK: cur->ino: 10, ref: 1
[1]vfs_release_vnode: ino: 10, refcount = 1
[1]v6_release: ino: 10, refcount: 0
[1]v6_ilock: slock: ino=10
[1]v6_iunlock: relslock: ino=10
[1]v6_iput: slock: ino=10
[1]v6_iput: relslock: ino=10
[1]vfs_lookup: cwd->ino: 1, path: /bin/sh, flags: 0x0
[1]vfs_lookup: LP[1] cur->ino: 1, ref: 8
[1]v6_ilock: slock: ino=1
[1]v6_dirlookup: dp->ino: 1, name: bin
[1]v6_iget: hit ip->ino: 2
[1]v6_iunlock: relslock: ino=1
[1]v6_iput: slock: ino=1
[1]v6_iput: relslock: ino=1
[1]v6_lookup: OK: bin, ip->ino: 2
[1]v6_ilock: slock: ino=2
[1]v6_iunlock: relslock: ino=2
[1]vfs_lookup: LP[2] cur->ino: 2, ref: 4
[1]v6_ilock: slock: ino=2
[1]v6_dirlookup: dp->ino: 2, name: sh
[1]v6_iunlock: relslock: ino=2
[1]v6_iput: slock: ino=2
[1]v6_iput: relslock: ino=2
[1]v6_lookup: OK: sh, ip->ino: 10
[1]v6_ilock: slock: ino=10
[1]v6_ilock: type: 2, nlink: 1, rdev: 0x101, size: 0xd328, mode: 0x81ed, addrs[0]: 0x5e
[1]v6_iunlock: relslock: ino=10
[1]vfs_lookup: LP[3] cur->ino: 10, ref: 1
[1]vfs_lookup: OK: cur->ino: 10, ref: 1
[1]v6_read: file: 0x1c6320,
[1]v6_read: vnode: 0x1c3568
[1]v6_read: inode: 0x1c3568
[1]v6_ilock: slock: ino=10
[1]_read_entry: read: dev: 0x101, buffer: 0xbd9080, bno: 0x5e, size: 0x1000
[2]v6_iunlock: relslock: ino=10
[2]execve: elf header check ok
[2]v6_read: file: 0x1c6320,
[2]v6_read: vnode: 0x1c3568
[2]v6_read: inode: 0x1c3568
[2]v6_ilock: slock: ino=10
[2]v6_iunlock: relslock: ino=10
[2]v6_read: file: 0x1c6320,
[2]v6_read: vnode: 0x1c3568
[2]v6_read: inode: 0x1c3568
[2]v6_ilock: slock: ino=10
[2]_read_entry: read: dev: 0x101, buffer: 0xbda080, bno: 0x5f, size: 0x1000
[1]_read_entry: read: dev: 0x101, buffer: 0xbdb080, bno: 0x60, size: 0x1000
[1]_read_entry: read: dev: 0x101, buffer: 0xbdc080, bno: 0x61, size: 0x1000
[1]_read_entry: read: dev: 0x101, buffer: 0xbdd080, bno: 0x62, size: 0x1000
[3]_read_entry: read: dev: 0x101, buffer: 0xbde080, bno: 0x63, size: 0x1000
[1]_read_entry: read: dev: 0x101, buffer: 0xb80080, bno: 0x64, size: 0x1000
[3]_read_entry: read: dev: 0x101, buffer: 0xb81080, bno: 0x65, size: 0x1000
[1]_read_entry: read: dev: 0x101, buffer: 0xb82080, bno: 0x66, size: 0x1000
[1]_read_entry: read: dev: 0x101, buffer: 0xb83080, bno: 0x67, size: 0x1000
[2]v6_iunlock: relslock: ino=10
[2]v6_read: file: 0x1c6320,
[2]v6_read: vnode: 0x1c3568
[2]v6_read: inode: 0x1c3568
[2]v6_ilock: slock: ino=10
[2]v6_iunlock: relslock: ino=10
[2]v6_read: file: 0x1c6320,
[2]v6_read: vnode: 0x1c3568
[2]v6_read: inode: 0x1c3568
[2]v6_ilock: slock: ino=10
[2]v6_iunlock: relslock: ino=10
[2]v6_read: file: 0x1c6320,
[2]v6_read: vnode: 0x1c3568
[2]v6_read: inode: 0x1c3568
[2]v6_ilock: slock: ino=10
[2]v6_iunlock: relslock: ino=10
[2]v6_iput: slock: ino=10
[2]v6_iput: relslock: ino=10
[2]vfs_release_vnode: ino: 10, refcount = 1
[2]v6_release: ino: 10, refcount: 0
[2]v6_ilock: slock: ino=10
[2]v6_iunlock: relslock: ino=10
[2]v6_iput: slock: ino=10
[2]v6_iput: relslock: ino=10
[2]execve: load file ok
[2]execve: copy argv ok
[2]execve: copy envp ok
[2]execve: argv: 0xffffffffff98, envp: 0xffffffffffa8, auxv: 0xffffffffffc0
[2]execve: finish sh
sh: argv[0] = 'sh'
sh: testenv = 'FROM_INIT'
[2]sys_openat: dirfd: -100, path: /dev/tty1, flags : 0x20002, mode: 0x0
[2]sys_openat: vnode->ino: 1
[2]sys_openat: fd: 3
[2]sys_openat: vnode->ino: 1, path: /dev/tty1, flags : 0x20002, mode: 0x0
[2]vfs_lookup: cwd->ino: 1, path: /dev/tty1, flags: 0x0
[2]vfs_lookup: LP[1] cur->ino: 1, ref: 9
[2]v6_ilock: slock: ino=1
[2]v6_dirlookup: dp->ino: 1, name: dev
[2]v6_iget: hit ip->ino: 3
[2]v6_iunlock: relslock: ino=1
[2]v6_iput: slock: ino=1
[2]v6_iput: relslock: ino=1
[2]v6_lookup: OK: dev, ip->ino: 3
[2]v6_ilock: slock: ino=3
[2]v6_iunlock: relslock: ino=3
[2]vfs_lookup: LP[2] cur->ino: 3, ref: 2
[2]v6_ilock: slock: ino=3
[2]v6_dirlookup: dp->ino: 3, name: tty1
[2]v6_iunlock: relslock: ino=3
[2]v6_iput: slock: ino=3
[2]v6_iput: relslock: ino=3
[2]v6_lookup: OK: tty1, ip->ino: 4
[2]v6_ilock: slock: ino=4
[2]v6_ilock: type: 3, nlink: 1, rdev: 0x301, size: 0x0, mode: 0x21b6, addrs[0]: 0x0
[2]v6_iunlock: relslock: ino=4
[2]vfs_lookup: LP[3] cur->ino: 4, ref: 1
[2]vfs_lookup: OK: cur->ino: 4, ref: 1
[2]sys_openat: fd: 3, file->vnode->ino: 4
[2]vfs_release_vnode: ino: 4, refcount = 1

[2]v6_release: ino: 4, refcount: 0
[2]v6_ilock: slock: ino=4
[2]_read_entry: read: dev: 0x301, buffer: 0xb84080, bno: 0x20, size: 0x1000
```

- /dev/tty1が削除対象になっている

```bash
[3]v6_dirlookup: dp->ino: 3, name: tty1     // /dev で tty1 をlookup
[3]v6_lookup: OK: tty1, ip->ino: 4          // tty1発見, ino=4
[3]v6_ilock: slock: ino=4
[3]v6_ilock: vp->ino: 4, valid: 0, type: 0, mode: 0x0   // ino=4のv6_dinodeを読み込み
[3]v6_ilock: type: 3, nlink: 1, rdev: 0x301, size: 0x0, mode: 0x21b6, addrs[0]: 0x0
[3]v6_iunlock: relslock: ino=4
[3]vfs_lookup: LP[3] cur->ino: 4, ref: 1
[3]vfs_lookup: OK: cur->ino: 4, ref: 1      // /dev/tty1 のlookup成功, ino=4のrefが1
[3]sys_openat: fd: 3, file->vnode->ino: 4   // sys_openat()正常終了
[3]vfs_release_vnode: ino: 4, refcount = 1
[3]v6_release: ino: 4, refcount: 0
[3]v6_ilock: slock: ino=4
[3]v6_ilock: vp->ino: 4, valid: 1, type: 3, mode: 0x21b6
[3]v6_iupdate: type: 0, mode: 0x21b6, valid: 1
[3]_read_entry: read: dev: 0x301, buffer: 0xb84080, bno: 0x20, size: 0x1000
```

- openat()でvfs_lookup()で取得したvnodeをvfs_clone_vnode()してrefをカウントアップ

```bash
[1]v6_dirlookup: dp->ino: 3, name: tty1
[1]_read_entry: read: dev: 0x101, buffer: 0xbd8080, bno: 0x2a, size: 0x1000
[3]v6_iunlock: relslock: ino=3
[3]v6_iput: slock: ino=3
[3]v6_iput: relslock: ino=3
[3]v6_lookup: OK: tty1, ip->ino: 4
[3]v6_ilock: slock: ino=4
[3]v6_ilock: vp->ino: 4, valid: 0, type: 0, mode: 0x0
[3]v6_ilock: type: 3, nlink: 1, rdev: 0x301, size: 0x0, mode: 0x21b6, addrs[0]: 0x0
[3]v6_iunlock: relslock: ino=4
[3]vfs_lookup: LP[3] cur->ino: 4, ref: 1
[3]vfs_lookup: OK: cur->ino: 4, ref: 1
[3]sys_openat: fd: 0, file->vnode->ino: 4, ref: 2 // ref=2となり、vfs_release_vnode()が呼ばれない
[3]sys_dup: oldfd: 0
[3]sys_dup: retrun fd: 1
[3]sys_dup: oldfd: 0
[3]sys_dup: retrun fd: 2
init: starting sh
$ /bin/ls /bin
.                                                          41fd 2 832
..                                                         41fd 1 4096
cat                                                        81ed 5 38568
...
$
```

- /bin/init -> /bin/sh -> /bin/ls が正常に動いているようだ
- /bin/ls の出力を現状に合わせる

```bash
[1]execve: path='/bin/init', argv=0x0, envp=0x0
[1]vfs_lookup: cwd->ino: 1, path: /bin/init, flags: 0x0
[3]vfs_lookup: OK: cur->ino: 6, ref: 1
[3]vfs_lookup: cwd->ino: 1, path: /bin/init, flags: 0x0
[3]vfs_lookup: OK: cur->ino: 6, ref: 1
[1]execve: proc: base=0x400000, size=0x4037b8, stack: sp=0xffffffd0, size=0xa000    # /bin/init

[1]sys_openat: vnode->ino: 1, path: /dev/tty1, flags : 0x20002, mode: 0x0
[1]vfs_lookup: cwd->ino: 1, path: /dev/tty1, flags: 0x0
[2]vfs_lookup: OK: cur->ino: 4, ref: 1
[3]execve: path='/bin/sh', argv=0x403030, envp=0x403010
[3]vfs_lookup: cwd->ino: 1, path: /bin/sh, flags: 0x0
[3]vfs_lookup: OK: cur->ino: 10, ref: 1
[3]vfs_lookup: cwd->ino: 1, path: /bin/sh, flags: 0x0
[3]vfs_lookup: OK: cur->ino: 10, ref: 1
[2]execve: proc: base=0x400000, size=0x40c5a0, stack: sp=0xffffff80, size=0xa000    # /bin/sh
[2]sys_openat: vnode->ino: 1, path: /dev/tty1, flags : 0x20002, mode: 0x0
[2]vfs_lookup: cwd->ino: 1, path: /dev/tty1, flags: 0x0
[2]vfs_lookup: OK: cur->ino: 4, ref: 1
1 sleep  init           // CNTL-p でプロセス一覧
2 run    idle
3 run    idle
4 run    idle
5 runble idle
6 run    rxether
7 sleep  sh fa: 1
$ /bin/date
[3]execve: path='/bin/date', argv=0x409818, envp=0xffffffffff98
[3]vfs_lookup: cwd->ino: 1, path: /bin/date, flags: 0x0
[3]vfs_lookup: OK: cur->ino: 9, ref: 1
[3]vfs_lookup: cwd->ino: 1, path: /bin/date, flags: 0x0
[3]vfs_lookup: OK: cur->ino: 9, ref: 1
[1]execve: proc: base=0x400000, size=0x409728, stack: sp=0xffffff80, size=0xa000    # /bin/date
2025年12日24日 水曜日 17時44分37秒 JST                          // コマンドは正常に実行しているが
$ [1]trap: [7] unknown trap code: 36 at 0x403be4 with 0x40d000  // shがこのエラーで再起動
[1]exit: exit: pid 7, err 1
[1]execve: path='/bin/sh', argv=0x403030, envp=0x403010
[1]vfs_lookup: cwd->ino: 1, path: /bin/sh, flags: 0x0
[1]vfs_lookup: OK: cur->ino: 10, ref: 2
[1]vfs_lookup: cwd->ino: 1, path: /bin/sh, flags: 0x0
[1]vfs_lookup: OK: cur->ino: 10, ref: 2
[1]execve: proc: base=0x400000, size=0x40c5a0, stack: sp=0xffffff80, size=0xa000    # /bin/sh

[1]sys_openat: vnode->ino: 1, path: /dev/tty1, flags : 0x20002, mode: 0x0
[1]vfs_lookup: cwd->ino: 1, path: /dev/tty1, flags: 0x0
[1]vfs_lookup: OK: cur->ino: 4, ref: 1
$ /bin/ls /                                                     // 再起動するが
[0]execve: path='/bin/ls', argv=0x409818, envp=0xffffffffff98
[0]vfs_lookup: cwd->ino: 1, path: /bin/ls, flags: 0x0
[0]vfs_lookup: OK: cur->ino: 12, ref: 1
[0]vfs_lookup: cwd->ino: 1, path: /bin/ls, flags: 0x0
[0]vfs_lookup: OK: cur->ino: 12, ref: 1
[3]release: error: slab is not locked                           // ここでストール(dateコマンドは実行可)
```

- コマンド実行後につねに`trap: unknown trap code: 36 at 0x403be4 with 0x40d000`が発生し、shが再起動する
- このtrapは/bin/sh で発生。コマンドから送信でも発生する

```bash
$ /bin/ls /bin                                                  // OK, ただしunknown trapは発生
[2]execve: path='/bin/ls', argv=0x409818, envp=0xffffffffff98
[2]vfs_lookup: cwd->ino: 1, path: /bin/ls, flags: 0x0
[2]vfs_lookup: OK: cur->ino: 12, ref: 1
[2]vfs_lookup: cwd->ino: 1, path: /bin/ls, flags: 0x0
[2]vfs_lookup: OK: cur->ino: 12, ref: 1
[3]execve: proc: base=0x400000, size=0x40a2d8, stack: sp=0xffffff70, size=0xa000
[3]sys_openat: vnode->ino: 1, path: /bin, flags : 0x20000, mode: 0x0
[3]vfs_lookup: cwd->ino: 1, path: /bin, flags: 0x0
[3]vfs_lookup: OK: cur->ino: 2, ref: 7
[3]vfs_lookup: cwd->ino: 1, path: /bin/., flags: 0x0
[3]vfs_lookup: OK: cur->ino: 2, ref: 10
drwxrwxr-x    2 root wheel   832  1  1 09:00 .
[3]vfs_lookup: cwd->ino: 1, path: /bin/.., flags: 0x0
[3]vfs_lookup: OK: cur->ino: 1, ref: 16
drwxrwxr-x    1 root wheel  4096  1  1 09:00 ..
[3]vfs_lookup: cwd->ino: 1, path: /bin/cat, flags: 0x0
[3]vfs_lookup: OK: cur->ino: 5, ref: 1
-rwxr-xr-x    5 root wheel 38568  1  1 09:00 cat
[3]vfs_lookup: cwd->ino: 1, path: /bin/init, flags: 0x0
[3]vfs_lookup: OK: cur->ino: 6, ref: 2
-rwxr-xr-x    6 root wheel 22400  1  1 09:00 init
[3]vfs_lookup: cwd->ino: 1, path: /bin/echo, flags: 0x0
[3]vfs_lookup: OK: cur->ino: 7, ref: 1
-rwxr-xr-x    7 root wheel 39480  1  1 09:00 echo
[3]vfs_lookup: cwd->ino: 1, path: /bin/ifconfig, flags: 0x0
[3]vfs_lookup: OK: cur->ino: 8, ref: 1
-rwxr-xr-x    8 root wheel 44184  1  1 09:00 ifconfig
[3]vfs_lookup: cwd->ino: 1, path: /bin/date, flags: 0x0
[3]vfs_lookup: OK: cur->ino: 9, ref: 1
-rwxr-xr-x    9 root wheel 49368  1  1 09:00 date
[3]vfs_lookup: cwd->ino: 1, path: /bin/sh, flags: 0x0
[3]vfs_lookup: OK: cur->ino: 10, ref: 2
-rwxr-xr-x   10 root wheel 52200  1  1 09:00 sh
[3]vfs_lookup: cwd->ino: 1, path: /bin/utest, flags: 0x0
[3]vfs_lookup: OK: cur->ino: 11, ref: 1
-rwxr-xr-x   11 root wheel 17744  1  1 09:00 utest
[3]vfs_lookup: cwd->ino: 1, path: /bin/ls, flags: 0x0
[3]vfs_lookup: OK: cur->ino: 12, ref: 2
-rwxr-xr-x   12 root wheel 53064  1  1 09:00 ls
[3]vfs_lookup: cwd->ino: 1, path: /bin/udpecho, flags: 0x0
[3]vfs_lookup: OK: cur->ino: 13, ref: 1
-rwxr-xr-x   13 root wheel 39032  1  1 09:00 udpecho
[3]vfs_lookup: cwd->ino: 1, path: /bin/dns, flags: 0x0
[3]vfs_lookup: OK: cur->ino: 14, ref: 1
-rwxr-xr-x   14 root wheel 11056  1  1 09:00 dns
[3]vfs_lookup: cwd->ino: 1, path: /bin/tcpecho, flags: 0x0
[3]vfs_lookup: OK: cur->ino: 15, ref: 1
-rwxr-xr-x   15 root wheel 40616  1  1 09:00 tcpecho

$ /bin/ls /                                                     // NG
[1]execve: path='/bin/ls', argv=0x409818, envp=0xffffffffff98
[1]vfs_lookup: cwd->ino: 1, path: /bin/ls, flags: 0x0
[1]vfs_lookup: OK: cur->ino: 12, ref: 1
[1]vfs_lookup: cwd->ino: 1, path: /bin/ls, flags: 0x0
[1]vfs_lookup: OK: cur->ino: 12, ref: 1
[1]execve: proc: base=0x400000, size=0x40a2d8, stack: sp=0xffffff70, size=0xa000
[1]sys_openat: vnode->ino: 1, path: /, flags : 0x20000, mode: 0x0
[1]vfs_lookup: cwd->ino: 1, path: /, flags: 0x0
[1]vfs_lookup: OK: cur->ino: 1, ref: 13
[1]vfs_lookup: cwd->ino: 1, path: //., flags: 0x0
ilock: no type
kern/drivers/console.c:264: kernel panic at cpu 1.
```

- trapの該当部分

```bash
0000000000403b70 <memchr>:
  403b70:   aa0003e3    mov x3, x0
  403b74:   12001c21    and w1, w1, #0xff
  403b78:   f240081f    tst x0, #0x7
  403b7c:   54000120    b.eq    403ba0 <memchr+0x30>  // b.none
  403b80:   b4000522    cbz x2, 403c24 <memchr+0xb4>
  403b84:   39400060    ldrb    w0, [x3]
  403b88:   6b01001f    cmp w0, w1
  403b8c:   54000380    b.eq    403bfc <memchr+0x8c>  // b.none
  403b90:   91000463    add x3, x3, #0x1
  403b94:   d1000442    sub x2, x2, #0x1
  403b98:   f240087f    tst x3, #0x7
  403b9c:   54ffff21    b.ne    403b80 <memchr+0x10>  // b.any
  403ba0:   d2800000    mov x0, #0x0                    // #0
  403ba4:   b40003e2    cbz x2, 403c20 <memchr+0xb0>
  403ba8:   39400060    ldrb    w0, [x3]
  403bac:   6b01001f    cmp w0, w1
  403bb0:   54000260    b.eq    403bfc <memchr+0x8c>  // b.none
  403bb4:   93407c25    sxtw    x5, w1
  403bb8:   b200c3e0    mov x0, #0x101010101010101      // #72340172838076673
  403bbc:   b207dbe6    mov x6, #0xfefefefefefefefe     // #-72340172838076674
  403bc0:   f29fdfe6    movk    x6, #0xfeff
  403bc4:   9b007ca5    mul x5, x5, x0
  403bc8:   f1001c5f    cmp x2, #0x7
  403bcc:   540000c8    b.hi    403be4 <memchr+0x74>  // b.pmore
  403bd0:   1400000b    b   403bfc <memchr+0x8c>
  403bd4:   d1002042    sub x2, x2, #0x8
  403bd8:   91002063    add x3, x3, #0x8
  403bdc:   f1001c5f    cmp x2, #0x7
  403be0:   54000269    b.ls    403c2c <memchr+0xbc>  // b.plast
  403be4:   f9400060    ldr x0, [x3]
  ...
0000000000403db0 <strnlen>:
  403db0:   a9be53f3    stp x19, x20, [sp, #-32]!
  403db4:   aa0003f3    mov x19, x0
  403db8:   aa0103f4    mov x20, x1
  403dbc:   aa0103e2    mov x2, x1
  403dc0:   52800001    mov w1, #0x0                    // #0
  403dc4:   f9000bfe    str x30, [sp, #16]
  403dc8:   97ffff6a    bl  403b70 <memchr>
  ...
0000000000401ab0 <fgets>:
  ...
  401b40:   9400080c    bl  403b70 <memchr>

$ nm sh
000000000040a3f0 B _end

0x40b000 - 0x40a3f0 = 0xc10 = 3088
```

## 12月25日

- usr/shを修正(空改行の判定), mallloc1()をmalloc()に変更

```bash
[3]execve: path='/bin/init', argv=0x0, envp=0x0
[3]execve: p->name: init
[3]sys_openat: vnode->ino: 1, path: /dev/tty1, flags : 0x20002, mode: 0x0
[3]syscall1: proc[7] sys_gettid called
[1]sys_wait4: [-1] pid: 0, status: 0x0, options: 0x0, rusage: 0x8
[3]syscall1: proc[7] sys_execve called
[3]execve: path='/bin/sh', argv=0x403030, envp=0x403010
[2]execve: p->name: sh
[2]syscall1: proc[7] sys_gettid called
[2]syscall1: proc[7] sys_openat called
[2]sys_openat: vnode->ino: 1, path: /dev/tty1, flags : 0x20002, mode: 0x0
[2]syscall1: proc[7] sys_close called
[2]syscall1: proc[7] sys_writev called
[2]sys_writev: fd 2, iovcnt: 2
[2]sys_writev: iov[0]: base=409e00, len=0
[2]sys_writev: iov[1]: base=407d50, len=2
$ [2]syscall1: proc[7] sys_read called
/bin/date
[1]console_read: buffer: 0x409e12, size: 0x3f6, buffer: /bin/date   // console_read
, r: 10, w: 10, e: 10
[1]syscall1: proc[7] sys_clone called
[2]syscall1: proc[8] sys_gettid called
[1]syscall1: proc[7] sys_wait4 called
[2]syscall1: proc[8] sys_brk called
[1]sys_wait4: [-1] pid: 0, status: 0x0, options: 0x0, rusage: 0x8
[2]sys_brk: [8] name : 0x40a3f0 to 0x0, old p->sz: 0x40a3f0
[2]syscall1: proc[8] sys_brk called
[2]sys_brk: [8] name : 0x40a3f0 to 0x40c000, old p->sz: 0x40a3f0
[2]sys_brk: [8] new p->sz: 0x40c000
[2]syscall1: proc[8] sys_execve called
[2]execve: path='/bin/date', argv=0x40b008, envp=0x409ad0
[3]execve: p->name: date
[3]syscall1: proc[8] sys_gettid called
[3]syscall1: proc[8] sys_clock_gettime called
[3]syscall1: proc[8] sys_ioctl called
[3]syscall1: proc[8] sys_writev called
[3]sys_writev: fd 1, iovcnt: 2
[3]sys_writev: iov[0]: base=409068, len=40
[3]sys_writev: iov[1]: base=4070cc, len=8
2025年12日25日 木曜日 11時 9分10秒 JST
[3]syscall1: proc[8] sys_exit called
[3]sys_exit_group: [8] sys_exit_group: 'date' exit with code 0
[3]syscall1: proc[7] sys_writev called
[3]sys_writev: fd 2, iovcnt: 2
[3]sys_writev: iov[0]: base=409e00, len=0
[3]sys_writev: iov[1]: base=407d50, len=2
$ [3]trap: [7] unknown trap code: 36 at 0x403f64 with 0x40b000
[3]exit: exit: pid 7, err 1
[1]syscall1: proc[9] sys_gettid called
[2]sys_wait4: [-1] pid: 0, status: 0x0, options: 0x0, rusage: 0x8
[1]syscall1: proc[9] sys_execve called
[1]execve: path='/bin/sh', argv=0x403030, envp=0x403010
[1]execve: p->name: sh
[1]syscall1: proc[9] sys_gettid called
[1]syscall1: proc[9] sys_openat called
[1]sys_openat: vnode->ino: 1, path: /dev/tty1, flags : 0x20002, mode: 0x0
[1]syscall1: proc[9] sys_close called
[1]syscall1: proc[9] sys_writev called
[1]sys_writev: fd 2, iovcnt: 2
[1]sys_writev: iov[0]: base=409e00, len=0
[1]sys_writev: iov[1]: base=407d50, len=2
$ [1]syscall1: proc[9] sys_read called
```

- consoleread()の返り値が間違っていた

```bash
[0]console_preinit: console_preinit ok
[0]rand_init: rand_init ok
[0]proc_init: proc_init ok
[0]console_init: console_init ok
[0]tty_init: tty_init ok
[0]sd_init: sd_init ok

[0]slab_cache_create: create buf.data, size: 4096, alignment: 64
[0]init_vfs: init_vfs ok
[0]user_init: user_init ok
[0]slab_cache_create: create timer_list, size: 40, alignment: 64
[2]slab_cache_create: create timer_list, size: 40, alignment: 64
[0]timer_init: [0] timer_init ok
[3]slab_cache_create: create timer_list, size: 40, alignment: 64
[0]trap_init: [0] trap_init ok
[0]main: cpu 0 init finished
[3]timer_init: [3] timer_init ok
[0]sdhost_probe: firmware sets clock divider
[2]timer_init: [2] timer_init ok
[1]slab_cache_create: create timer_list, size: 40, alignment: 64
[3]trap_init: [3] trap_init ok
[1]timer_init: [1] timer_init ok
[3]main: cpu 3 init finished
[2]trap_init: [2] trap_init ok
[1]trap_init: [1] trap_init ok
[1]main: cpu 1 init finished
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
[3]sd_postinit: partition[0]: TYPE: 12, LBA = 0x800, #SECS = 0x20000
[3]sd_postinit: partition[1]: TYPE: 131, LBA = 0x20800, #SECS = 0x1f800
[3]sd_postinit: sd_postinit ok

[3]release: error: card is not locked
[3]v6_set_super: v6_sb: size 1000 nblocks 960 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 39
[3]slab_cache_create: create usb_dev_desc, size: 18, alignment: 64
[3]slab_cache_create: create usb_cfg_desc, size: 9, alignment: 64
[3]slab_cache_create: create usb_if_desc, size: 9, alignment: 64
[3]slab_cache_create: create usb_ep_desc, size: 18, alignment: 64
[3]slab_cache_create: create usb_str_desc, size: 2, alignment: 64
[3]slab_cache_create: create hub_desc, size: 9, alignment: 64
[3]slab_cache_create: create usb_4byte, size: 4, alignment: 64
[3]slab_cache_create: create params, size: 16, alignment: 0
[3]slab_cache_create: create stdata, size: 192, alignment: 64
[3]slab_cache_create: create periodic, size: 32, alignment: 64
[3]slab_cache_create: create stdata, size: 24, alignment: 64
[3]slab_cache_create: create stdata, size: 24, alignment: 64
[3]slab_cache_create: create urb, size: 72, alignment: 64
[3]slab_cache_create: create urb, size: 72, alignment: 64
[2]usb_dev_init: Device ven424-2514, dev9-0-2 found
[2]usb_function_get_if_name: func name=int9-0-1
[2]usb_dev_init: Interface int9-0-1 found
[2]usb_dev_init: Function is not supported
[2]usb_function_get_if_name: func name=int9-0-2
[2]usb_dev_init: Interface int9-0-2 found
[2]usb_dev_factory_get_device: Using device/interface int9-0-2
[1]usb_dev_init: Device ven424-2514, dev9-0-2 found
[1]usb_function_get_if_name: func name=int9-0-1
[1]usb_dev_init: Interface int9-0-1 found
[1]usb_dev_init: Function is not supported
[1]usb_function_get_if_name: func name=int9-0-2
[1]usb_dev_init: Interface int9-0-2 found
[1]usb_dev_factory_get_device: Using device/interface int9-0-2
[2]usb_dev_init: Device ven424-7800 found
[2]usb_dev_factory_get_device: Using device/interface ven424-7800
[2]lan7800_init_macaddr: MAC address is b8:27:eb:ab:e8:48
[3]usb_standard_hub_enumerate_ports: Port 1: Device configured
[2]usb_standard_hub_enumerate_ports: Port 1: Device configured
[1]dwhc_root_port_init: Device configured
[1]usb_init: dwhc initialized

[1]net_device_register: dev=net0, type=2 (ETHERNET)
[1]usb_init: usb_init ok
[1]net_protocol_register: type=0x0800 (IP)
[1]net_protocol_register: type=0x0806 (ARP)
[1]ip_protocol_register: type=1 (ICMP)
[1]ip_protocol_register: type=17 (UDP)
[1]ip_protocol_register: type=6 (TCP)
[1]ip_route_add: route added: network=192.168.10.0, netmask=255.255.255.0, nexthop=0.0.0.0, iface=192.168.10.110 dev=net0
[1]ip_iface_register: registered: dev=net0, unicast=192.168.10.110, netmask=255.255.255.0, broadcast=192.168.10.255
[1]ip_route_add: route added: network=0.0.0.0, netmask=0.0.0.0, nexthop=192.168.10.1, iface=192.168.10.110 dev=net0
[1]net_init: net_init ok
[1]net_device_open: dev=net0, state=up
[2]netrun: running...
$ /bin/date
2025年12日25日 木曜日 13時53分19秒 JST
$ /bin/echo abc
[1]release: error: slab is not locked       // slab_cache_alloc()とslab_cache_free()の無限ループっぽい
```

- slab_cache_free()でreleaseするlockが間違っていた

```bash
$ /bin/date
2025年12日25日 木曜日 14時19分21秒 JST
$ /bin/echo abc
abc
$ /bin/ls /bin
drwxrwxr-x    2 root wheel   832  1  1 09:00 .
drwxrwxr-x    1 root wheel  4096  1  1 09:00 ..
-rwxr-xr-x    5 root wheel 38568  1  1 09:00 cat
-rwxr-xr-x    6 root wheel 22400  1  1 09:00 init
-rwxr-xr-x    7 root wheel 39480  1  1 09:00 echo
-rwxr-xr-x    8 root wheel 44184  1  1 09:00 ifconfig
-rwxr-xr-x    9 root wheel 49368  1  1 09:00 date
-rwxr-xr-x   10 root wheel 54664  1  1 09:00 sh
-rwxr-xr-x   11 root wheel 17744  1  1 09:00 utest
-rwxr-xr-x   12 root wheel 53064  1  1 09:00 ls
-rwxr-xr-x   13 root wheel 39032  1  1 09:00 udpecho
-rwxr-xr-x   14 root wheel 11056  1  1 09:00 dns
-rwxr-xr-x   15 root wheel 40616  1  1 09:00 tcpecho

$ /bin/cat > test.txt
[2]v6_dirlookup: dirlookup not DIR: dp->ino: 0x10, mode: 0x81ed, name: test.txt
=== dump vnode: 0x1c1208 ===
   ops: 0xb61c0
    mp: 0x1c4f88
   ref: 0x1
  mode: 0x81ed
 nlink: 0x1
  bits: 0x0
  rdev: 0x101
   ino: 0x10
  size: 0x0
  data: 0x1c1208
inode : 0x1c1208
 vnode: 0x1c1208
 valid: 1
  type: 1
==========================
```

- lsで日付が正しくないのはilock()でv6_dinodeからv6_inodeにコピーする際に日付をコピーしていなかった
- '/'が連続する場合、すべて呼び飛ばすようにした

```bash
$ /bin/ls /bin
drwxrwxr-x    2 root wheel   832 12 25 14:52 .
drwxrwxr-x    1 root wheel  4096 12 25 14:52 ..
-rwxr-xr-x    5 root wheel 38568 12 25 14:52 cat
-rwxr-xr-x    6 root wheel 22400 12 25 14:52 init
-rwxr-xr-x    7 root wheel 39480 12 25 14:52 echo
-rwxr-xr-x    8 root wheel 44184 12 25 14:52 ifconfig9
-rwxr-xr-x    9 root wheel 49368 12 25 14:52 date
-rwxr-xr-x   10 root wheel 54664 12 25 14:52 sh
-rwxr-xr-x   11 root wheel 17744 12 25 14:52 utest
-rwxr-xr-x   12 root wheel 53128 12 25 14:52 ls
-rwxr-xr-x   13 root wheel 39032 12 25 14:52 udpecho
-rwxr-xr-x   14 root wheel 11056 12 25 14:52 dns
-rwxr-xr-x   15 root wheel 40616 12 25 14:52 tcpecho

$ cd /bin                                                       // cdできるようになっている
[2]sys_chdir: path: /bin
[2]sys_chdir: p->cwd: 2, vnode: 2
$ echo abc                                                      // カレントディレクトリのコマンドを実行できる
abc
$ cd ..                                                         // 親ディレクトリにcdできる
[1]sys_chdir: path: ..
[1]v6_lookup: vnode->ino: 2, mode: 0x41fd, ip->valid: 1, COMP: ..
[1]sys_chdir: p->cwd: 1, vnode: 1
$ /bin/ls /                                                     // ルートディレクトリのlsができる
drwxrwxr-x    1 root wheel  4096 12 25 14:52 .
drwxrwxr-x    1 root wheel  4096 12 25 14:52 ..
drwxrwxr-x    2 root wheel   832 12 25 14:52 bin
drwxrwxr-x    3 root wheel   192 12 25 14:52 dev
$ /bin/ls .                                                     // カレントディレクトリのlsができる
drwxrwxr-x    1 root wheel  4096 12 25 14:52 .
drwxrwxr-x    1 root wheel  4096 12 25 14:52 ..
drwxrwxr-x    2 root wheel   832 12 25 14:52 bin
drwxrwxr-x    3 root wheel   192 12 25 14:52 dev
$ /bin/ls                                                       // 何も指定しないとカレントディレクトリのlsをする
drwxrwxr-x    1 root wheel  4096 12 25 14:52 .
drwxrwxr-x    1 root wheel  4096 12 25 14:52 ..
drwxrwxr-x    2 root wheel   832 12 25 14:52 bin
drwxrwxr-x    3 root wheel   192 12 25 14:52 dev
$ echo abc > test.txt                                           // リダイレクトができない
[0]v6_lookup: vnode->ino: 1, mode: 0x41fd, ip->valid: 1, COMP: test.txt
[0]v6_dirlookup: dirlookup not DIR: dp->ino: 0x10, mode: 0x81ed, name: test.txt
```

```bash
$ /bin/echo abc > test.txt
[1]sys_openat: vnode->ino: 1, path: test.txt, flags : 0x20041, mode: 0x1ed
[1]v6_lookup: vnode->ino: 1, mode: 0x41fd, ip->valid: 1, COMP: test.txt
[1]v6_dirlookup: dp->ino: 1, name: test.txt
[1]v6_dirlink: dp->ino: 16, name: test.txt
[1]v6_dirlookup: dp->ino: 16, name: test.txt
[1]v6_dirlink: no hit
```

```bash
$ /bin/echo abc > test.txt
[0]sys_openat: vnode->ino: 1, path: test.txt, flags : 0x20041, mode: 0x1ed
[0]vfs_open: cwd: 1, path: test.txt, create: 64
[0]vfs_open: path: test.txt, vnode: 1
[0]v6_lookup: vnode->ino: 1, mode: 0x41fd, ip->valid: 1, COMP: test.txt
[0]v6_dirlookup: dp->ino: 1, name: test.txt
[0]v6_create: parent->ino: 1, name: test.txt
[0]v6_create: call v6_dirlink with: dp->ino: 1, name: test.txt, ino: 16, type: 2
[0]v6_dirlink: dp->ino: 1, name: test.txt
[0]v6_dirlookup: dp->ino: 1, name: test.txt
[0]v6_dirlink: offset: 0x100
[0]v6_dirlink: de: ino=16, type=2, name=test.txt, off=0x100
[0]trap: [8] unknown trap code: 37 at 0xffff0000000a8a94 with 0x18  // Level1のdata abort
[0]exit: exit: pid 8, err 1
```

```bash
ffff0000000a8a70 <dev_read>:

int dev_read(device_t dev, char *buffer, off_t offset, size_t size)
{
ffff0000000a8a70:   2a0003e4    mov w4, w0
    major_t major = major(dev);
    minor_t minor = minor(dev);
ffff0000000a8a74:   12001c00    and w0, w0, #0xff
    major_t major = major(dev);
ffff0000000a8a78:   d3483c84    ubfx    x4, x4, #8, #8

    if (major >= MAX_DRIVERS)
ffff0000000a8a7c:   7100149f    cmp w4, #0x5
ffff0000000a8a80:   54000108    b.hi    ffff0000000a8aa0 <dev_read+0x30>  // b.pmore
        return -ENXIO;
    return drv_table[major]->read(minor, buffer, offset, size);
ffff0000000a8a84:   2a0403e4    mov w4, w4
ffff0000000a8a88:   b00008e5    adrp    x5, ffff0000001c5000 <mountpoints+0x78>
ffff0000000a8a8c:   9100a0a5    add x5, x5, #0x28
ffff0000000a8a90:   f86478a4    ldr x4, [x5, x4, lsl #3]
ffff0000000a8a94:   f9400c84    ldr x4, [x4, #24]           // ここ
ffff0000000a8a98:   aa0403f0    mov x16, x4
ffff0000000a8a9c:   d61f0200    br  x16                     // drv-table[major]->read
}
ffff0000000a8aa0:   128000a0    mov w0, #0xfffffffa             // #-6 (return -ENXIO;)
ffff0000000a8aa4:   d65f03c0    ret
```

## 12月26日

- v6_create()で新規作成したv6_inode()をディスクに書き出す際にv6_update(ITOV(ip))を使うと
  内部でv6_ilock()を実行してまだ書き込んでいないv6_dinode()からipを取り込み、
  ip->vnode->rdevが0になるのが原因のようだった
- v6_iupdate(ip)で直接書き出すようにした。

```bash
$ /bin/echo abc > test.txt
[3]sys_openat: vnode->ino: 1, path: test.txt, flags : 0x20041, mode: 0x1ed
[3]v6_create: parent->ino: 1, name: test.txt
[3]v6_create: call v6_dirlink with: dp->ino: 1, name: test.txt, ino: 16, type: 2, rdev: 0x101
[3]v6_dirlink: dp->ino: 1, name: test.txt
[3]v6_dirlink: write to dp->ino: 1, rdev: 0x101 with de: ino=16, type=2, name=test.txt, off=0x100
[3]v6_writei: ip: ino:1, rdev: 0x101, off: 0x100, n: 64
[3]v6_writei: get_block: dev: 0x101, bno: 0x28
[3]v6_writei: put_block: dev: 0x101, bno: 0x28
[3]v6_dirlink: ok                                               // direntの書き出しは成功
[3]_read_entry: read: dev: 0x0, buffer: 0xb77000, bno: 0x20, size: 0x1000   // その後のv6_update()でdevが0に
[3]trap: [8] unknown trap code: 37 at 0xffff0000000a8bf4 with 0x18
[3]exit: exit: pid 8, err 1
```

- ただし、今度は /bin/echoの実行でストール. リダイレクトしないechoは正常動作

```bash
$ /bin/echo abc > /test.txt
[0]sys_openat: vnode->ino: 1, path: /test.txt, flags : 0x20041, mode: 0x1ed
[1]sys_openat: fd: 1, file->vnode->ino: 16, ref: 2
[1]execve: path='/bin/echo', argv=0x40b008, envp=0x409ad0   // ここでストール
    // vfs_open("/bin/echo")だとおもわれるがデバッグ出力もされない
```

- `usr/src/sh/main.c`

```c
    case REDIR:
        rcmd = (struct redircmd *)cmd;
        close(rcmd->fd);                                // close(1)
        if (open(rcmd->file, rcmd->mode, 0755) < 0) {   // open("test.txt") -> fd = 1
            fprintf(stderr, "open %s failed\n", rcmd->file);
            exit(1);
        }
        runcmd(rcmd->cmd);                              // /bin/echo abcを実行
        break;
```

- 現状

```bash
$ /bin/ls
[1]execve: path='/bin/ls', argv=0x40b008, envp=0x409ad0
[3]execve: exec ls ok
[3]sys_openat: vnode->ino: 1, path: ., flags : 0x20000, mode: 0x0
[3]sys_openat: fd: 3, file->vnode->ino: 1, ref: 15
drwxrwxr-x    1 root wheel  4096 12 26 18:06 .
drwxrwxr-x    1 root wheel  4096 12 26 18:06 ..
drwxrwxr-x    2 root wheel   832 12 26 18:06 bin
drwxrwxr-x    3 root wheel   192 12 26 18:06 dev
$ /bin/echo abc
[1]execve: path='/bin/echo', argv=0x40b008, envp=0x409ad0
[2]execve: exec echo ok
abc
$ /bin/ls /bin
[3]execve: path='/bin/ls', argv=0x40b008, envp=0x409ad0
[3]execve: exec ls ok
[3]sys_openat: vnode->ino: 1, path: /bin, flags : 0x20000, mode: 0x0
[3]sys_openat: fd: 3, file->vnode->ino: 2, ref: 12
drwxrwxr-x    2 root wheel   832 12 26 18:06 .
drwxrwxr-x    1 root wheel  4096 12 26 18:06 ..
-rwxr-xr-x    5 root wheel 38568 12 26 18:06 cat
-rwxr-xr-x    6 root wheel 22400 12 26 18:06 init
-rwxr-xr-x    7 root wheel 39480 12 26 18:06 echo
-rwxr-xr-x    8 root wheel 44184 12 26 18:06 ifconfig
-rwxr-xr-x    9 root wheel 49368 12 26 18:06 date
-rwxr-xr-x   10 root wheel 54664 12 26 18:06 sh
-rwxr-xr-x   11 root wheel 17744 12 26 18:06 utest
-rwxr-xr-x   12 root wheel 53064 12 26 18:06 ls
-rwxr-xr-x   13 root wheel 39032 12 26 18:06 udpecho
-rwxr-xr-x   14 root wheel 11056 12 26 18:06 dns
-rwxr-xr-x   15 root wheel 40616 12 26 18:06 tcpecho
$ /bin/date
[3]execve: path='/bin/date', argv=0x40b008, envp=0x409ad0
[3]execve: exec date ok
2025年12日26日 金曜日 18時29分 3秒 JST
$ /bin/echo abc | /bin/cat
[3]execve: path='/bin/echo', argv=0x40b008, envp=0x409ad0
[1]trap: [14] unknown trap code: 37 at 0xffff000000091b0c with 0x8
[3]execve: exec echo ok
[1]
xit: exit: pid 14, err 1
[3]release: error: pipe is not locked
[3]release: error: pipe is not locked
$
```

## 12月27日

- v6_create()におけるparent, childのilock(), iunlockput()の整合性が取れて
  いなかった

```bash
$ /bin/echo abc > test.txt
[0]print_fd_table: [8] sys_openat_1
    file[0] = 4
    file[2] = 4
[0]sys_openat: vnode->ino: 1, path: test.txt, flags : 0x20041, mode: 0x1ed
=== dump v6_iget: 0x1c17f0 ===
   ops: 0xb61c0
    mp: 0x1c4f80
   ref: 0x1
  mode: 0x0
 nlink: 0x1
  bits: 0x0
  rdev: 0x101
   ino: 0x10
  size: 0x0
  data: 0x1c17f0
inode : 0x1c17f0
 vnode: 0x1c17f0
 valid: 0
  type: 0
==========================
=== dump v6_create 1: 0x1c17f0 ===
   ops: 0xb61c0
    mp: 0x1c4f80
   ref: 0x1
  mode: 0x0
 nlink: 0x0                             // v6_ialloc()で設定された値が
  bits: 0x0                             // v6_ilock()でクリアされる
  rdev: 0x0
   ino: 0x10
  size: 0x0
  data: 0x1c17f0
inode : 0x1c17f0
 vnode: 0x1c17f0
 valid: 1
  type: 2
==========================
=== dump v6_create 2: 0x1c17f0 ===
   ops: 0xb61c0
    mp: 0x1c4f80
   ref: 0x1
  mode: 0x81ed
 nlink: 0x1                             // v6_ilock()後に再度設定する
  bits: 0x0
  rdev: 0x101
   ino: 0x10
  size: 0x0
  data: 0x1c17f0
inode : 0x1c17f0
 vnode: 0x1c17f0
 valid: 1
  type: 2
==========================
=== dump v6_create 3: 0x1c17f0 ===
   ops: 0xb61c0
    mp: 0x1c4f80
   ref: 0x1
  mode: 0x81ed
 nlink: 0x1
  bits: 0x0
  rdev: 0x101
   ino: 0x10
  size: 0x0
  data: 0x1c17f0
inode : 0x1c17f0
 vnode: 0x1c17f0
 valid: 1
  type: 2
==========================
[3]sys_openat: fd: 1, file->vnode->ino: 16, ref: 2
[3]print_fd_table: [8] sys_openat_2
    file[0] = 4
    file[1] = 16
    file[2] = 4
[3]print_fd_table: [8] sys_ecexve
    file[0] = 4
    file[1] = 16
    file[2] = 4
[3]execve: path='/bin/echo', argv=0x40b008, envp=0x409ad0
=== dump v6_iget: 0x1c18c8 ===
   ops: 0xb61c0
    mp: 0x1c4f80
   ref: 0x1
  mode: 0x0
 nlink: 0x1
  bits: 0x0
  rdev: 0x101
   ino: 0x7
  size: 0x0
  data: 0x1c18c8
inode : 0x1c18c8
 vnode: 0x1c18c8
 valid: 0
  type: 0
==========================
=== dump v6_iget: 0x1c18c8 ===
   ops: 0xb61c0
    mp: 0x1c4f80
   ref: 0x1
  mode: 0x0
 nlink: 0x1
  bits: 0x0
  rdev: 0x101
   ino: 0x7
  size: 0x0
  data: 0x1c18c8
inode : 0x1c18c8
 vnode: 0x1c18c8
 valid: 0
  type: 0
==========================
[1]execve: exec echo ok                 // echoコマンドが正常終了
$ /bin/ls
[0]print_fd_table: [9] sys_ecexve
    file[0] = 4
    file[1] = 4
    file[2] = 4
[0]execve: path='/bin/ls', argv=0x40b008, envp=0x409ad0
=== dump v6_iget: 0x1c19a0 ===
   ops: 0xb61c0
    mp: 0x1c4f80
   ref: 0x1
  mode: 0x0
 nlink: 0x1
  bits: 0x0
  rdev: 0x101
   ino: 0xc
  size: 0x0
  data: 0x1c19a0
inode : 0x1c19a0
 vnode: 0x1c19a0
 valid: 0
  type: 0
==========================
=== dump v6_iget: 0x1c19a0 ===
   ops: 0xb61c0
    mp: 0x1c4f80
   ref: 0x1
  mode: 0x0
 nlink: 0x1
  bits: 0x0
  rdev: 0x101
   ino: 0xc
  size: 0x0
  data: 0x1c19a0
inode : 0x1c19a0
 vnode: 0x1c19a0
 valid: 0
  type: 0
==========================
[2]execve: exec ls ok
[2]print_fd_table: [9] sys_openat_1
    file[0] = 4
    file[1] = 4
    file[2] = 4
[2]sys_openat: vnode->ino: 1, path: ., flags : 0x20000, mode: 0x0
[2]sys_openat: fd: 3, file->vnode->ino: 1, ref: 17
[2]print_fd_table: [9] sys_openat_2
    file[0] = 4
    file[1] = 4
    file[2] = 4
    file[3] = 1
drwxrwxr-x    1 root wheel  4096 12 26 18:06 .
drwxrwxr-x    1 root wheel  4096 12 26 18:06 ..
drwxrwxr-x    2 root wheel   832 12 26 18:06 bin
drwxrwxr-x    3 root wheel   192 12 26 18:06 dev
-rwxr-xr-x   16 root wheel     4 12 27 11:27 test.txt   // test.txtが作成されている
$ /bin/cat test.txt
[2]print_fd_table: [10] sys_ecexve
    file[0] = 4
    file[1] = 4
    file[2] = 4
[2]execve: path='/bin/cat', argv=0x40b008, envp=0x409ad0
=== dump v6_iget: 0x1c1a78 ===
   ops: 0xb61c0
    mp: 0x1c4f80
   ref: 0x1
  mode: 0x0
 nlink: 0x1
  bits: 0x0
  rdev: 0x101
   ino: 0x5
  size: 0x0
  data: 0x1c1a78
inode : 0x1c1a78
 vnode: 0x1c1a78
 valid: 0
  type: 0
==========================
=== dump v6_iget: 0x1c1a78 ===
   ops: 0xb61c0
    mp: 0x1c4f80
   ref: 0x1
  mode: 0x0
 nlink: 0x1
  bits: 0x0
  rdev: 0x101
   ino: 0x5
  size: 0x0
  data: 0x1c1a78
inode : 0x1c1a78
 vnode: 0x1c1a78
 valid: 0
  type: 0
==========================
[3]execve: exec cat ok
[3]print_fd_table: [10] sys_openat_1
    file[0] = 4
    file[1] = 4
    file[2] = 4
[3]sys_openat: vnode->ino: 1, path: test.txt, flags : 0x20000, mode: 0x0
[3]sys_openat: fd: 3, file->vnode->ino: 16, ref: 3
[3]print_fd_table: [10] sys_openat_2
    file[0] = 4
    file[1] = 4
    file[2] = 4
    file[3] = 16
                                    // catは正常終了だが出力されていない
$
```

- パイプでエラー

```bash
$ /bin/cat test.txt | /bin/echo
[2]execve: path='/bin/cat', argv=0x40b008, envp=0x409ad0
[1]trap: [18] unknown trap code: 37 at 0xffff000000091b0c with 0x8
[2]execve: exec cat ok
[1st it3
aic: f
208,-e2r 0
ebetsu
[2]release: error: pipe is not locked
[2]release: error: pipe is not locked
```

- releaseエラーはpipe_close()内の不要なrelease()を削除で解決

```bash
$ /bin/cat test.txt | /bin/echo
[3]execve: path='/bin/cat', argv=0x40b008, envp=0x409ad0
[0]trap: [11] unknown trap code: 37 at 0xffff000000091b0c with 0x8  //
[0]exit: exit: pid 11, err 1
[1]execve: exec cat ok
test 123
abcdef
2025-02-20
ebetsu
```

```bash
ffff000000091b00 <get_fd>:
ffff000000091b00:   71004c3f    cmp w1, #0x13       // OPEN_MAX = 0x14
ffff000000091b04:   540000cc    b.gt    ffff000000091b1c <get_fd+0x1c>
ffff000000091b08:   f861d800    ldr x0, [x0, w1, sxtw #3]   // x0 = table[fd]
ffff000000091b0c:   f9400401    ldr x1, [x0, #8]            // x1 = table[fd]->vnode
ffff000000091b10:   f100003f    cmp x1, #0x0
ffff000000091b14:   9a9f1000    csel    x0, x0, xzr, ne  // ne = any
ffff000000091b18:   d65f03c0    ret
ffff000000091b1c:   d2800000    mov x0, #0x0                    // return NULL
ffff000000091b20:   d65f03c0    ret
```

```c
struct vfile *get_fd(fd_table_t table, int fd)
{
    if (fd >= OPEN_MAX || !table[fd]->vnode)
        return NULL;
    return table[fd];
}
```

- リダイレクト問題

```bash
$ [2]sys_read: [7] fd: 0, buf: 0x409e08, count: 0x400
/bin/echo abc > test2.txt
[2]execve: path='/bin/echo', argv=0x40b008, envp=0x409ad0
[1]execve: exec echo ok
[1]sys_ioctl: [8] fd: 1, file: 17, req: 0x5413
[1]sys_writev: [8] fd 1, iovcnt: 2                  // test2.txtへのwrite
[1]sys_writev: iov[0]: base=407d58, len=3
[2]v6_writei: [8] ip: ino: 17, rdev: 0x101, size: 0x0: off: 0x0, n: 3
[1]v6_bmap: [8] ino: 17, bno: 0
[3]v6_balloc: BBLOCK(0): 0x22
[3]v6_bmap: [8] addr: 0                             // ip->addrs[0] = 0 がおかしい
[3]v6_writei: get_block: dev: 0x101, bno: 0x0       // bnoがゼロ
=== v6_writei dump ===
+------+-------------------------------------------------+------------------+
| 0000 | 61 62 63                                        | abc              |
+------+-------------------------------------------------+------------------+
[1]sys_writev: iov[1]: base=ffffffffff1f, len=1
[2]v6_writei: get_block: dev: 0x101, bno: 0x1
=== v6_writei dump ===
+------+-------------------------------------------------+------------------+
| 0000 | 0a                                              | .                |
+------+-------------------------------------------------+------------------+

[2]sys_writev: [7] fd 2, iovcnt: 2              // '$' の書き出し
[2]sys_writev: iov[0]: base=409e00, len=0
[2]sys_writev: iov[1]: base=407d50, len=2
$
```

balloc周りをチェック: dataブロックのbnoに換算していない?

## 12月28日

- mkfsでビットマップが作成されていなかったこととBBLOCK()が正しく計算されて
  いなかったことの二重障害だった

```bash
$ /bin/echo abc > test2.txt
[3]v6_writei: [8] ip: ino: 1, rdev: 0x101, size: 0x1000: off: 0x140, n: 64
[1]execve: exec echo ok
[1]sys_ioctl: [8] fd: 1, file: 17, req: 0x5413
[1]v6_writei: [8] ip: ino: 17, rdev: 0x101, size: 0x0: off: 0x0, n: 3
[1]v6_bmap: [8] ino: 17, rdev: 0x101, bno: 0
[1]v6_balloc: BBLOCK(0): 0x27                   // bitmapブロックを正しく計算
[3]v6_balloc: b: 0x0, bi: 0x98, [1] = 0x0
[3]v6_bmap: [8] addr: 98                        // 空きブロックを正しく計算
[3]v6_writei: get_block: dev: 0x101, bno: 0x98
=== v6_writei dump ===
+------+-------------------------------------------------+------------------+
| 0000 | 61 62 63                                        | abc              |
+------+-------------------------------------------------+------------------+

[3]v6_writei: [8] ip: ino: 17, rdev: 0x101, size: 0x3: off: 0x3, n: 1
[3]v6_bmap: [8] ino: 17, rdev: 0x101, bno: 0
[3]v6_bmap: [8] addr: 98
[3]v6_writei: get_block: dev: 0x101, bno: 0x98
=== v6_writei dump ===
+------+-------------------------------------------------+------------------+
| 0000 | 0a                                              | .                |
+------+-------------------------------------------------+------------------+

$ /bin/ls
[3]execve: exec ls ok
[3]sys_ioctl: [9] fd: 1, file: 4, req: 0x5413
drwxrwxr-x    1 root wheel  4096 12 28 10:52 .
drwxrwxr-x    1 root wheel  4096 12 28 10:52 ..
drwxrwxr-x    2 root wheel   832 12 28 10:52 bin
drwxrwxr-x    3 root wheel   192 12 28 10:52 dev
-rwxr-xr-x    5 root wheel    34 12 28 10:52 test.txt
-rwxr-xr-x   17 root wheel     4 12 28 11:09 test2.txt
$ /bin/cat test2.txt
[1]execve: exec cat ok
[1]v6_bmap: [10] ino: 17, rdev: 0x101, bno: 0
[1]v6_bmap: [10] addr: 98
[1]sys_ioctl: [10] fd: 1, file: 4, req: 0x5413
abc                                                         // 正しく保存されている
```

- デバッグ行を削除

```bash
$ /bin/echo abc > test2.txt
$ /bin/ls
drwxrwxr-x    1 root wheel  4096 12 28 10:52 .
drwxrwxr-x    1 root wheel  4096 12 28 10:52 ..
drwxrwxr-x    2 root wheel   832 12 28 10:52 bin
drwxrwxr-x    3 root wheel   192 12 28 10:52 dev
-rwxr-xr-x    5 root wheel    34 12 28 10:52 test.txt
-rwxr-xr-x   17 root wheel     4 12 28 11:26 test2.txt
$ /bin/cat test2.txt
abc
$ /bin/date
2025年12日28日 日曜日 11時26分59秒 JST
```

- pipe処理のエラー

```bash
$ /bin/ls | /bin/cat
[2]trap: [13] unknown trap code: 37 at 0xffff000000091b0c with 0x8
[1]execve: exec ls ok
[2]exit: exit: pid 13, err 1
[1]sys_ioctl: [12] fd: 1, file: 4, req: 0x5413
drwxrwxr-x    1 root wheel  4096 12 28 10:52 .
drwxrwxr-x    1 root wheel  4096 12 28 10:52 ..
drwxrwxr-x    2 root wheel   832 12 28 10:52 bin
drwxrwxr-x    3 root wheel   192 12 28 10:52 dev
-rwxr-xr-x    5 root wheel    34 12 28 10:52 test.txt
-rwxr-xr-x   17 root wheel     4 12 28 11:09 test2.txt
```

```bash
$ /bin/grep abc test.txt | /bin/wc      // [8] : コマンドグループ
[1]get_fd: [9] fd: 1                    // [9] : grep
[2]get_fd: [8] fd: 4234497              // [10] : wc
[1]get_fd: [9] fd: 0
[3]get_fd: [10] fd: 0
[1]get_fd: [9] fd: 4234497
[2]get_fd: [8] fd: 0
[1]get_fd: [9] fd: 0
[3]get_fd: [10] fd: 4234497
[3]get_fd: [10] fd: 4234497
[3]get_fd: [10] fd: 0
[3]trap: [10] unknown trap code: 37 at 0xffff000000091b98 with 0x8
[1]get_fd: [9] fd: 0
[3]exit: exit: pid 10, err 1
[1]get_fd: [9] fd: 1
abcdef
[1]get_fd: [9] fd: 0
[1]get_fd: [9] fd: 0
```

```bash
$ /bin/grep abc test.txt | /bin/wc
[2]sys_pipe2: pfd[0]: 3, pfd[1]: 4
[0]sys_close: [9] fd: 1, ref: 12                // close(1)
[2]sys_close: [8] argfd err: -9 fd: 0           // close(p[0])
[0]sys_dup: [9] dup ofd: 0 to fd: 1 ref: 15     // dup(p[1])
[1]sys_close: [10] fd: 0, ref: 15               // close(0)
[0]sys_close: [9] argfd err: -9 fd: 0           // close(p[0])
[1]sys_dup: [10] argfd err: -9 fd: 0            // dup(p[0])
[2]sys_close: [8] fd: 0, ref: 14                // close(p[1])
[0]sys_close: [9] fd: 0, ref: 13                // close(p[0])
[1]sys_close: [10] argfd err: -9 fd: 0          // close(p[0])
            // [10] close(p[1])
[1]trap: [10] unknown trap code: 37 at 0xffff000000091b0c with 0x8
[3]sys_close: [9] fd: 0, ref: 1                 // close(0)
[1]exit: exit: pid 10, err 1
```

```bash
# proc[8]   grep | wc
close(p[0])
close(p[1])

# proc[9] : grep
close(1)
dup(p[1])
close(p[0])
close(p[1])

# proc[10] : wc
close(0)
dup(p[0])
close(p[0])
close(p[1])
```

- sys_pipe2()でユーザ領域のポインタに直接fdをセットしていたがセットされなかった。一旦、カーネル変数で受けて、ユーザ領域にmemmoveするようにした

```bash
$ /bin/grep abc test.txt | /bin/wc
p[0]: 3, p[1]: 4
[3]sys_close: [9] fd: 1, ref: 12
[0]sys_close: [8] fd: 3, ref: 3
[1]sys_close: [10] fd: 0, ref: 14
[0]sys_close: [8] fd: 4, ref: 4
[3]sys_dup: [9] dup ofd: 4 to fd: 1 ref: 3
[1]sys_dup: [10] dup ofd: 3 to fd: 0 ref: 3
[3]sys_close: [9] fd: 3, ref: 3
[1]sys_close: [10] fd: 3, ref: 2
[3]sys_close: [9] fd: 4, ref: 3
[1]sys_close: [10] fd: 4, ref: 2
kern/sdhost.c:1330: assertion failed.               // 発生しないこともあり
kern/drivers/console.c:264: kernel panic at cpu 1.
```
