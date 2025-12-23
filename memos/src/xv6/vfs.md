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
