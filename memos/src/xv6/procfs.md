# procfsを移植する

## 12月30日

```bash
[0]init_vfs: init_vfs ok
[0]v6_init: v6_init ok
[0]procfs_init: procfs_init ok
[0]vfs_mount: vnode->ino: -1
[0]vfs_mount: vnode->ino: -2

[1]vfs_mount: vnode->ino: 5
[1]_alloc_vnode: vnode->ino: 0, filenum: 0
[1]vfs_mount: vnode->ino: 5
[1]forkret: mount proc ok

$ /bin/ls
drwxrwxr-x    1 root wheel  4096 12 30 10:24 .
drwxrwxr-x    1 root wheel  4096 12 30 10:24 ..
drwxrwxr-x    2 root wheel   960 12 30 10:24 bin
drwxrwxr-x    3 root wheel   192 12 30 10:24 dev
drwxr-xr-x    0 root wheel     0  1  1 09:00 proc
-rwxr-xr-x    6 root wheel    34 12 30 10:24 test.txt
$ cd /proc
$ /bin/ls
[1]trap: [9] unknown trap code: 33 at 0x0 with 0x0
[1]exit: exit: pid 9, err 1
```

```bash
$ /bin/ls
[2]sys_openat: dirfd: -100, path: ., flags : 0x20000, mode: 0x0
[2]sys_openat: vnode->ino: 1
[2]sys_openat: fd: 3
drwxrwxr-x    1 root wheel  4096 12 30 10:24 .
drwxrwxr-x    1 root wheel  4096 12 30 10:24 ..
drwxrwxr-x    2 root wheel   960 12 30 10:24 bin
drwxrwxr-x    3 root wheel   192 12 30 10:24 dev
drwxr-xr-x    0 root wheel     0  1  1 09:00 proc
-rwxr-xr-x    6 root wheel    34 12 30 10:24 test.txt
$ /bin/ls proc
[0]sys_openat: dirfd: -100, path: proc, flags : 0x20000, mode: 0x0
[0]sys_openat: vnode->ino: 1
[0]sys_openat: fd: 3
[0]trap: [9] unknown trap code: 33 at 0x0 with 0x0
[0]exit: exit: pid 9, err 1
```

- `ls`はできない？

```bash
$ /bin/ls /proc
[3]vfs_open: path: /bin/ls, vnode: 16
[3]vfs_open: cwd: ino: 1, rdev: 0x101, mp->dev: 0x101, mp->ops: 0xb7290; child: /bin/ls, ino: 16, rdev: 0x101, mp->dev: 0x101, mp->ops: 0xb7290
[1]sys_openat: dirfd: -100, path: /proc, flags : 0x20000, mode: 0x0
[1]sys_openat: vnode->ino: 1
[1]sys_openat: fd: 3
[1]vfs_open: path: /proc, vnode: 0
[1]vfs_open: cwd: ino: 1, rdev: 0x101, mp->dev: 0x101, mp->ops: 0xb7290; child: /proc, ino: 0, rdev: 0x0, mp->dev: 0x102, mp->ops: 0xb74f8
[1]trap: [8] unknown trap code: 33 at 0x0 with 0x0
[1]exit: exit: pid 8, err 1

$ /bin/cat /proc/mounts
[3]vfs_open: path: /bin/cat, vnode: 7
[1]sys_openat: dirfd: -100, path: /proc/mounts, flags : 0x20000, mode: 0x0
[1]sys_openat: vnode->ino: 1
[1]sys_openat: fd: 3
[1]_alloc_vnode: [1] vnode->ino: 10, filenum: 10
[1]vfs_open: path: /proc/mounts, vnode: 10
[1]procfs_open: file: 0x1c54e8, f->vnode->ino: 10
[1]procfs_read: file: pid: 0, ino: 10, nbytes: 0x200
[1]procfs_read: proc->pid: 0
[1]procfs_read: entry: 0xb7468      // ここでストール

$ cd /proc/mounts
[2]_alloc_vnode: [1] vnode->ino: 10, filenum: 10
$ /bin/ls
[1]vfs_open: path: /bin/ls, vnode: 16
[1]sys_openat: dirfd: -100, path: ., flags : 0x20000, mode: 0x0
[1]sys_openat: vnode->ino: 10
[1]sys_openat: fd: 3
[1]sys_openat: vfs_open . error: -20
ls: cannot open .
$ /bin/ls /proc/mounts/
[0]vfs_open: path: /bin/ls, vnode: 16
[0]sys_openat: dirfd: -100, path: /proc/mounts/, flags : 0x20000, mode: 0x0
[0]sys_openat: vnode->ino: 10
[0]sys_openat: fd: 3
[0]vfs_open: path: /proc/mounts/, vnode: 10
[0]procfs_open: file: 0x1c54e8, f->vnode->ino: 10
[0]procfs_read: file: pid: 0, ino: 10, nbytes: 0x40
[0]procfs_read: proc->pid: 0
[0]procfs_read: entry: 0xb7468      // ここでストール


$ /bin/cat /proc/mounts/cmdline
[1]vfs_open: path: /bin/cat, vnode: 7
[2]sys_openat: dirfd: -100, path: /proc/mounts/cmdline, flags : 0x20000, mode: 0x0
[2]sys_openat: vnode->ino: 1
[2]sys_openat: fd: 3
[2]_alloc_vnode: [1] vnode->ino: 10, filenum: 10
[2]sys_openat: vfs_open /proc/mounts/cmdline error: -20
/bin/cat: /proc/mounts/cmdline : Not a directory

$ /bin/cat /proc/mounts/stat
[3]vfs_open: path: /bin/cat, vnode: 7
[3]sys_openat: dirfd: -100, path: /proc/mounts/stat, flags : 0x20000, mode: 0x0
[3]sys_openat: vnode->ino: 1
[3]sys_openat: fd: 3
[3]_alloc_vnode: [1] vnode->ino: 10, filenum: 10
[3]sys_openat: vfs_open /proc/mounts/stat error: -20
/bin/cat: /proc/mounts/stat : Not a directory

$ /bin/cat /proc/cmdline
[1]vfs_open: path: /bin/cat, vnode: 7
[2]sys_openat: dirfd: -100, path: /proc/cmdline, flags : 0x20000, mode: 0x0
[2]sys_openat: vnode->ino: 1
[2]sys_openat: fd: 3
[2]_alloc_vnode: [1] vnode->ino: 2, filenum: 2
[2]vfs_open: path: /proc/cmdline, vnode: 2
[2]procfs_open: file: 0x1c54e8, f->vnode^>ino: 2
[2]procfs_read: file: pid: 0, ino: 2, nbytes: 0x200
[2]procfs_read: proc->pid: 0
[2]procfs_read: entry: 0xb73d8      // ここでストール
```

```bash
$ /bin/ls /proc/mounts
[2]syscall1: proc[7] sys_clone called
[1]syscall1: proc[8] sys_gettid called
[2]syscall1: proc[7] sys_wait4 called
[1]syscall1: proc[8] sys_brk called
[1]syscall1: proc[8] sys_brk called
[1]syscall1: proc[8] sys_execve called
[3]vfs_open: path: /bin/ls, vnode: 16
[2]syscall1: proc[8] sys_gettid called
[2]syscall1: proc[8] sys_openat called
[2]sys_openat: dirfd: -100, path: /proc/mounts, flags : 0x20000, mode: 0x0
[2]sys_openat: vnode->ino: 1
[2]sys_openat: fd: 3
[2]_alloc_vnode: [1] vnode->ino: 10, filenum: 10
[2]vfs_open: path: /proc/mounts, vnode: 10
[2]procfs_open: file: 0x1c54e8, f->vnode->ino: 10
[2]sys_openat: fd: 3, file->vnode->ino: 10, ref: 2
[2]syscall1: proc[8] sys_fstat called
[2]syscall1: proc[8] sys_read called
```

## 2026年1月2日

```bash
[0]console_preinit: console_preinit ok
[0]rand_init: rand_init ok
[0]proc_init: proc_init ok
[0]console_init: console_init ok
[0]tty_init: tty_init ok
[0]sd_init: sd_init ok

[0]init_vfs: init_vfs ok
[0]v6_init: v6_init ok
[0]procfs_init: procfs_init ok
[0]vfs_mount: path: /, vnode->ino: -1
[0]vfs_mount: mount / to mp[0] mount_node is null
[0]vfs_mount: set root_fs with mp[0]
[0]user_init: user_init ok
[0]timer_init: [0] timer_init ok
[1]timer_init: [1] timer_init ok
[0]trap_init: [0] trap_init ok
[0]main: cpu 0 init finished
[1]trap_init: [1] trap_init ok
[1]main: cpu 1 init finished
[2]timer_init: [2] timer_init ok
[0]sdhost_probe: firmware sets clock divider
[3]timer_init: [3] timer_init ok
[2]trap_init: [2] trap_init ok
[3]trap_init: [3] trap_init ok
[2]main: cpu 2 init finished
[3]main: cpu 3 init finished
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
[1]sd_postinit: partition[0]: TYPE: 12, LBA = 0x800, #SECS = 0x20000
[1]sd_postinit: partition[1]: TYPE: 131, LBA = 0x20800, #SECS = 0x1f800
[1]sd_postinit: sd_postinit ok

[1]release: error: card is not locked
[1]v6_set_super: v6_sb: size 1000 nblocks 960 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 39
[3]vfs_mount: path: /proc, vnode->ino: 5
[3]_alloc_vnode: pid: 0, filenum: 0, mode: 0x41ed, mp->root_node: 0x55555555
[3]_alloc_vnode: [0] ALLOC: vnode->ino: 0, mode: 0x41ed, filenum: 0
[3]vfs_mount: mount /proc with flags 0x1 to mp[1] mount_node->ino: 5, ops: 0xb84f8, dev: 0x102, bits: 0x1
[3]forkret: mount proc ok
[1]usb_dev_init: Device ven424-2514, dev9-0-2 found
[1]usb_function_get_if_name: func name=int9-0-1
[1]usb_dev_init: Interface int9-0-1 found
[1]usb_dev_init: Function is not supported
[1]usb_function_get_if_name: func name=int9-0-2
[1]usb_dev_init: Interface int9-0-2 found
[1]usb_dev_factory_get_device: Using device/interface int9-0-2
[2]usb_dev_init: Device ven424-2514, dev9-0-2 found
[2]usb_function_get_if_name: func name=int9-0-1
[2]usb_dev_init: Interface int9-0-1 found
[2]usb_dev_init: Function is not supported
[2]usb_function_get_if_name: func name=int9-0-2
[2]usb_dev_init: Interface int9-0-2 found
[2]usb_dev_factory_get_device: Using device/interface int9-0-2
[3]usb_dev_init: Device ven424-7800 found
[3]usb_dev_factory_get_device: Using device/interface ven424-7800
[1]lan7800_init_macaddr: MAC address is b8:27:eb:ab:e8:48
[2]usb_standard_hub_enumerate_ports: Port 1: Device configured
[3]usb_standard_hub_enumerate_ports: Port 1: Device configured
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
[3]netrun: running...
[2]sys_openat: vnode->ino: 1, path: /dev/tty1, flags : 0x20002, mode: 0x0
[1]sys_openat: vfs_open ok: set file->vnode->ino: 4 to fd=0
[1]sys_openat: fd: 0, file->vnode->ino: 4, ref: 2
[1]sys_openat: vnode->ino: 1, path: /dev/tty1, flags : 0x20002, mode: 0x0
[1]sys_openat: vfs_open ok: set file->vnode->ino: 4 to fd=3
[1]sys_openat: fd: 3, file->vnode->ino: 4, ref: 2
$ /bin/ls /proc/mounts
[2]sys_openat: vnode->ino: 1, path: /proc/mounts, flags : 0x20000, mode: 0x0
[2]procfs_lookup: vnode->ino: 0, filename: mounts, filenum: 0
[2]_find_filenum_by_name: fn mounts != ent[0] .
[2]_find_filenum_by_name: fn mounts != ent[1] ..
[2]_find_filenum_by_name: fn mounts == ent[2] mounts and return 10
[2]procfs_lookup: [0] mounts is filenum: 10
[2]_find_vnode: pid: 0, filenum: 10, mode: 0x41ed, mp->root_node: 0x0
[2]_find_vnode: vnode_table[0] vn.refcount: 1, pid: 0, filenum: 0
[2]_find_vnode: vnode_table[1] vn.refcount: 0, pid: 0, filenum: 0
[2]_find_vnode: vnode_table[2] vn.refcount: 0, pid: 0, filenum: 0
[2]_find_vnode: vnode_table[3] vn.refcount: 0, pid: 0, filenum: 0
[2]_find_vnode: vnode_table[4] vn.refcount: 0, pid: 0, filenum: 0
[2]_find_vnode: vnode_table[5] vn.refcount: 0, pid: 0, filenum: 0
[2]_find_vnode: call _alloc_vnode
[2]_alloc_vnode: pid: 0, filenum: 10, mode: 0x41ed, mp->root_node: 0x0
[2]_alloc_vnode: [1] ALLOC: vnode->ino: 10, mode: 0x41ed, filenum: 10
[2]procfs_open: file: 0x1c64e8, f->vnode->ino: 10
[2]sys_openat: vfs_open ok: set file->vnode->ino: 10 to fd=3
[2]sys_openat: fd: 3, file->vnode->ino: 10, ref: 2
[2]procfs_read: file: pid: 0, ino: 10, nbytes: 0x40
[2]procfs_read: proc->pid: 0
[2]procfs_read: entry from root_files
[2]procfs_read: exec func: 0xffff0000000a59a0 with proc->pid: 0
[2]vfs_reverse_lookup: cwd->ino: -1, buf: 0xffff000000ba7c98, size: 0x64, uid: 0
[2]vfs_reverse_lookup: cwd is NUll and set rootf_fs->rooot_node: ino: 1
[2]vfs_reverse_lookup: cwd->mode: 0x41fd
[2]vfs_reverse_lookup: start while loop from cur: 0xffff0000001c33d8 (1), root_fs->root_node: 0xffff0000001c33d8 (1)
[2]vfs_reverse_lookup: buf: /???
[2]vfs_reverse_lookup: cwd->ino: 5, buf: 0xffff000000ba7c98, size: 0x64, uid: 0     # /proc : v6上の/procファイル (ino=5)
[2]vfs_reverse_lookup: cwd->mode: 0x41ed
[2]vfs_reverse_lookup: start while loop from cur: 0xffff0000001c34b0 (5), root_fs->root_node: 0xffff0000001c33d8 (1)
[2]vfs_reverse_lookup: check cur: 0xffff0000001c34b0 (5) with cur->mp->root_node: 0xffff0000001c33d8 (1)
[2]vfs_reverse_lookup: cur/..
```

```bash
$ /bin/ls /proc/mounts
[0]vfs_lookup: cwd->ino: 1, path: /bin/ls, flags: 0x0
[1]vfs_open: cwd: 1, path: /bin/ls, create: 0
[1]vfs_lookup: cwd->ino: 1, path: /bin/ls, flags: 0x0
[1]vfs_open: path: /bin/ls, vnode: 16
[2]sys_openat: vnode->ino: 1, path: /proc/mounts, flags : 0x20000, mode: 0x0
[2]vfs_open: cwd: 1, path: /proc/mounts, create: 0
[2]vfs_lookup: cwd->ino: 1, path: /proc/mounts, flags: 0x0
[2]procfs_lookup: vnode->ino: 0, filename: mounts, filenum: 0
[2]_find_filenum_by_name: fn mounts != ent[0] .
[2]_find_filenum_by_name: fn mounts != ent[1] ..
[2]_find_filenum_by_name: fn mounts == ent[2] mounts and return 10
[2]procfs_lookup: [0] mounts is filenum: 10
[2]_find_vnode: pid: 0, filenum: 10, mode: 0x41ed, mp->root_node: 0x0
[2]_find_vnode: vnode_table[0] vn.refcount: 1, pid: 0, filenum: 0
[2]_find_vnode: vnode_table[1] vn.refcount: 0, pid: 0, filenum: 0
[2]_find_vnode: vnode_table[2] vn.refcount: 0, pid: 0, filenum: 0
[2]_find_vnode: vnode_table[3] vn.refcount: 0, pid: 0, filenum: 0
[2]_find_vnode: vnode_table[4] vn.refcount: 0, pid: 0, filenum: 0
[2]_find_vnode: vnode_table[5] vn.refcount: 0, pid: 0, filenum: 0
[2]_find_vnode: call _alloc_vnode
[2]_alloc_vnode: pid: 0, filenum: 10, mode: 0x41ed, mp->root_node: 0x0
[2]_alloc_vnode: [1] ALLOC: vnode->ino: 10, mode: 0x41ed, filenum: 10

[2]vfs_open: path: /proc/mounts, vnode: 10
[2]procfs_open: file: 0x1c74e8, f->vnode->ino: 10
[2]sys_openat: vfs_open ok: set file->vnode->ino: 10 to fd=3
[2]sys_openat: fd: 3, file->vnode->ino: 10, ref: 2
[2]procfs_read: file: pid: 0, ino: 10, nbytes: 0x40
[2]procfs_read: proc->pid: 0
[2]procfs_read: entry from root_files
[2]procfs_read: exec func: 0xffff0000000a59a0 with proc->pid: 0
[2]vfs_reverse_lookup: cwd->ino: -1, buf: 0xffff000000ba6c98, size: 0x64, uid: 0
[2]vfs_reverse_lookup: cwd is NUll and set rootf_fs->rooot_node: ino: 1
[2]vfs_reverse_lookup: cwd->mode: 0x41fd
[2]vfs_reverse_lookup: start while loop from cur: 0xffff0000001c43d8 (1), root_fs->root_node: 0xffff0000001c43d8 (1)
[2]vfs_reverse_lookup: buf: /, j: 98, len: 1
[2]vfs_reverse_lookup: cwd->ino: 5, buf: 0xffff000000ba6c98, size: 0x64, uid: 0
[2]vfs_reverse_lookup: cwd->mode: 0x41ed
[2]vfs_reverse_lookup: start while loop from cur: 0xffff0000001c44b0 (5), root_fs->root_node: 0xffff0000001c43d8 (1)
[2]vfs_reverse_lookup: check cur: 0xffff0000001c44b0 (5) with cur->mp->root_node: 0xffff0000001c43d8 (1)
[2]vfs_reverse_lookup: cur/..
[2]vfs_open: cwd: 5, path: .., create: 0
[2]vfs_lookup: cwd->ino: 5, path: .., flags: 0x0
```

## 1月3日

- ファイルシステムのルートノードで`..`に移行する場合のcurの変更先をcur->mp->root_nodeに変更した場合は永久ループ

```bash
[1]sys_openat: vnode->ino: 1, path: /proc/mounts, flags : 0x20000, mode: 0x0
[1]vfs_open: [[v6]] cwd: 1, path: /proc/mounts, create: 0
[1]vfs_lookup: start with [[v6]] cwd->ino: 1, path: /proc/mounts, flags: 0x0
[1]vfs_lookup: [[v6]] LP[1] cur->ino: 1, ref: 14, bits: 0x0
[1]vfs_lookup: COMP[1]: 'proc'
[1]vfs_lookup: FLG (SELF), path[i]: 'm'
[1]vfs_lookup: fs of cur->ino 1 is [[v6]] root_node: 1
[1]vfs_lookup: call ops->lookup with [[v6]] cur: 0xffff0000001c43d8 (1), component: proc, &cur: 0xffff000000bfbd78
[1]vfs_lookup: [[v6]] LP[2] cur->ino: 5, ref: 2, bits: 0x1
[1]vfs_lookup: COMP[2]: 'mounts'
[1]vfs_lookup: FLG (SELF), path[i]: ''
[1]vfs_lookup: fs of cur->ino 0 is [[procfs]] root_node: 0
[1]vfs_lookup: call ops->lookup with [[procfs]] cur: 0xffff0000001c8148 (0), component: mounts, &cur: 0xffff000000bfbd78
[1]procfs_lookup: vnode: 0xffff0000001c8148 (0), filename: mounts, result: 0xffff000000bfbd78
[1]_find_filenum_by_name: fn mounts != ent[0] .
[1]_find_filenum_by_name: fn mounts != ent[1] ..
[1]_find_filenum_by_name: fn mounts == ent[2] mounts and return 10
[1]procfs_lookup: [0] mounts is filenum: 10
[1]_find_vnode: pid: 0, filenum: 10, mode: 0x41ed, mp->root_node: 0x0
[1]_find_vnode: vnode_table[0] vn.refcount: 1, pid: 0, filenum: 0
[1]_find_vnode: vnode_table[1] vn.refcount: 0, pid: 0, filenum: 0
[1]_find_vnode: vnode_table[2] vn.refcount: 0, pid: 0, filenum: 0
[1]_find_vnode: vnode_table[3] vn.refcount: 0, pid: 0, filenum: 0
[1]_find_vnode: vnode_table[4] vn.refcount: 0, pid: 0, filenum: 0
[1]_find_vnode: vnode_table[5] vn.refcount: 0, pid: 0, filenum: 0
[1]_find_vnode: call _alloc_vnode
[1]_alloc_vnode: pid: 0, filenum: 10, mode: 0x41ed, mp->root_node: 0x0
[1]_alloc_vnode: [1] ALLOC: vnode->ino: 10, mode: 0x41ed, filenum: 10
[1]vfs_lookup: [[procfs]] LP[3] cur->ino: 10, ref: 1, bits: 0x0
[1]vfs_lookup: [[procfs]] OK: cur->ino: 10, ref: 1
[1]vfs_open: path: /proc/mounts, vnode: 10
[1]procfs_open: file: 0x1c74e8, f->vnode->ino: 10
[1]sys_openat: vfs_open ok: set file->vnode->ino: 10 to fd=3
[1]sys_openat: fd: 3, file->vnode->ino: 10, ref: 2
[1]procfs_read: file: pid: 0, ino: 10, nbytes: 0x40
[1]procfs_read: PROCFS_DATA(file->vnode): pid: 0, filenum: 10
[1]procfs_read: entry from root_files
[1]procfs_read: exec func: 0xffff0000000a5a50 with proc->pid: 0
[1]dump_mp: fs: v6, mount_node: [[ND]] -1, root_node: 1, dev: 0x101
[1]get_data_mounts: mp fs: v6, mount_node: -1
[1]vfs_reverse_lookup: [[ND]] cwd->ino: -1, buf: 0xffff000000bfbc98, size: 0x64, uid: 0
[1]vfs_reverse_lookup: cwd is NUll and set to [[v6]] rootf_fs->rooot_node: ino: 1
[1]vfs_reverse_lookup: cwd->mode: 0x41fd
[1]vfs_reverse_lookup: start while loop from [[v6]] cur: 0x1 (1852376), root_fs->root_node: 0x1 (8)
[1]vfs_reverse_lookup: buf: /, j: 98, len: 1
[1]dump_mp: fs: procfs, mount_node: [[v6]] 5, root_node: 0, dev: 0x102
[1]get_data_mounts: mp fs: procfs, mount_node: 5
[1]vfs_reverse_lookup: [[v6]] cwd->ino: 5, buf: 0xffff000000bfbc98, size: 0x64, uid: 0
[1]vfs_reverse_lookup: cwd->mode: 0x41ed
[1]vfs_reverse_lookup: start while loop from [[v6]] cur: 0x5 (1852376), root_fs->root_node: 0x1 (8)
[1]vfs_reverse_lookup: check [[v6]] cur: 0xffff0000001c44b0 (5) with cur->mp->root_node: 0xffff0000001c43d8 (1)
[1]vfs_reverse_lookup: cur/..
[1]vfs_open: [[v6]] cwd: 5, path: .., create: 0
[1]vfs_lookup: start with [[v6]] cwd->ino: 5, path: .., flags: 0x0
[1]vfs_lookup: [[v6]] LP[1] cur->ino: 5, ref: 3, bits: 0x1
[1]vfs_lookup: COMP[1]: '..'
[1]vfs_lookup: FLG (SELF), path[i]: ''
[1]vfs_lookup: fs of cur->ino 0 is [[procfs]] root_node: 0
[1]vfs_lookup: CHG cur to [[procfs]]'s mount_node: 0
[1]vfs_lookup: call ops->lookup with [[procfs]] cur: 0xffff0000001c8148 (0), component: .., &cur: 0xffff000000bfbad8
[1]procfs_lookup: vnode: 0xffff0000001c8148 (0), filename: .., result: 0xffff000000bfbad8
[1]_find_filenum_by_name: fn .. != ent[0] .
[1]_find_filenum_by_name: fn .. == ent[1] .. and return 0
[1]procfs_lookup: [0] .. is filenum: 0
[1]_find_vnode: pid: 0, filenum: 0, mode: 0x41ed, mp->root_node: 0x0
[1]_find_vnode: vnode_table[0] vn.refcount: 1, pid: 0, filenum: 0
[1]_find_vnode: [0] CLONE: vnode->ino: 0, mode: 0x41ed, filenum: 0
[1]_find_vnode: return vfs_clone_vnode
[1]vfs_lookup: [[procfs]] LP[2] cur->ino: 0, ref: 2, bits: 0x0
[1]vfs_lookup: [[procfs]] OK: cur->ino: 0, ref: 2
[1]vfs_open: path: .., vnode: 0
[1]procfs_open: file: 0x1c7508, f->vnode->ino: 0
[1]procfs_readdir: file->vnode: ino: 0, mode: 0x41ed
[1]procfs_readdir: PROCFS_DATA(file->vnode).filenum: 0
[1]procfs_readdir: (1-1) dir->name: 1
[1]procfs_readdir: dir ino: 0, type: 0, name: 1
[1]procfs_readdir: file->vnode: ino: 0, mode: 0x41ed
[1]procfs_readdir: PROCFS_DATA(file->vnode).filenum: 0
[1]procfs_readdir: (1-1) dir->name: 2
[1]procfs_readdir: dir ino: 0, type: 0, name: 2
[1]procfs_readdir: file->vnode: ino: 0, mode: 0x41ed
[1]procfs_readdir: PROCFS_DATA(file->vnode).filenum: 0
[1]procfs_readdir: (1-1) dir->name: 3
[1]procfs_readdir: dir ino: 0, type: 0, name: 3
[1]procfs_readdir: file->vnode: ino: 0, mode: 0x41ed
[1]procfs_readdir: PROCFS_DATA(file->vnode).filenum: 0
[1]procfs_readdir: (1-1) dir->name: 4
[1]procfs_readdir: dir ino: 0, type: 0, name: 4
[1]procfs_readdir: file->vnode: ino: 0, mode: 0x41ed
[1]procfs_readdir: PROCFS_DATA(file->vnode).filenum: 0
[1]procfs_readdir: (1-1) dir->name: 5
[1]procfs_readdir: dir ino: 0, type: 0, name: 5
[1]procfs_readdir: file->vnode: ino: 0, mode: 0x41ed
[1]procfs_readdir: PROCFS_DATA(file->vnode).filenum: 0
[1]procfs_readdir: (1-1) dir->name: 6
[1]procfs_readdir: dir ino: 0, type: 0, name: 6
[1]procfs_readdir: file->vnode: ino: 0, mode: 0x41ed
[1]procfs_readdir: PROCFS_DATA(file->vnode).filenum: 0
[1]procfs_readdir: (1-1) dir->name: 7
[1]procfs_readdir: dir ino: 0, type: 0, name: 7
[1]procfs_readdir: file->vnode: ino: 0, mode: 0x41ed
[1]procfs_readdir: PROCFS_DATA(file->vnode).filenum: 0
[1]procfs_readdir: (1-1) dir->name: 8
[1]procfs_readdir: dir ino: 0, type: 0, name: 8
[1]procfs_readdir: file->vnode: ino: 0, mode: 0x41ed
[1]procfs_readdir: PROCFS_DATA(file->vnode).filenum: 0
[1]procfs_readdir: (1-2) dir->name: .
[1]procfs_readdir: dir ino: 1, type: 0, name: .
[1]procfs_readdir: file->vnode: ino: 0, mode: 0x41ed
[1]procfs_readdir: PROCFS_DATA(file->vnode).filenum: 0
[1]procfs_readdir: (1-2) dir->name: ..
[1]procfs_readdir: dir ino: 2, type: 0, name: ..
[1]procfs_readdir: file->vnode: ino: 0, mode: 0x41ed
[1]procfs_readdir: PROCFS_DATA(file->vnode).filenum: 0
[1]procfs_readdir: (1-2) dir->name: mounts
[1]procfs_readdir: dir ino: 3, type: 0, name: mounts
[1]procfs_readdir: file->vnode: ino: 0, mode: 0x41ed
[1]procfs_readdir: PROCFS_DATA(file->vnode).filenum: 0
[1]procfs_readdir: filename is null and return 0
[1]vfs_reverse_lookup: check [[procfs]] cur: 0xffff0000001c8148 (0) with cur->mp->root_node: 0xffff0000001c8148 (0)
[1]vfs_reverse_lookup: change cur
[1]vfs_reverse_lookup: CHG to [[v6]]
[1]vfs_reverse_lookup: cur/..
[1]vfs_open: [[v6]] cwd: 5, path: .., create: 0
[1]vfs_lookup: start with [[v6]] cwd->ino: 5, path: .., flags: 0x0
[1]vfs_lookup: [[v6]] LP[1] cur->ino: 5, ref: 3, bits: 0x1
[1]vfs_lookup: COMP[1]: '..'
[1]vfs_lookup: FLG (SELF), path[i]: ''
[1]vfs_lookup: fs of cur->ino 0 is [[procfs]] root_node: 0
[1]vfs_lookup: CHG cur to [[procfs]]'s mount_node: 0
[1]vfs_lookup: call ops->lookup with [[procfs]] cur: 0xffff0000001c8148 (0), component: .., &cur: 0xffff000000bfbad8
[1]procfs_lookup: vnode: 0xffff0000001c8148 (0), filename: .., result: 0xffff000000bfbad8
[1]_find_filenum_by_name: fn .. != ent[0] .
[1]_find_filenum_by_name: fn .. == ent[1] .. and return 0
[1]procfs_lookup: [0] .. is filenum: 0
[1]_find_vnode: pid: 0, filenum: 0, mode: 0x41ed, mp->root_node: 0x0
[1]_find_vnode: vnode_table[0] vn.refcount: 2, pid: 0, filenum: 0
[1]_find_vnode: [0] CLONE: vnode->ino: 0, mode: 0x41ed, filenum: 0
[1]_find_vnode: return vfs_clone_vnode
[1]vfs_lookup: [[procfs]] LP[2] cur->ino: 0, ref: 3, bits: 0x0
[1]vfs_lookup: [[procfs]] OK: cur->ino: 0, ref: 3
[1]vfs_open: path: .., vnode: 0
[1]procfs_open: file: 0x1c7508, f->vnode->ino: 0
[1]procfs_readdir: file->vnode: ino: 0, mode: 0x41ed
[1]procfs_readdir: PROCFS_DATA(file->vnode).filenum: 0
[1]procfs_readdir: (1-1) dir->name: 1
[1]procfs_readdir: dir ino: 0, type: 0, name: 1
[1]procfs_readdir: file->vnode: ino: 0, mode: 0x41ed
```

- ファイルシステムのルートノードで`..`に移行する場合のcurの変更先をcur->mp->mount_nodeにした場合はストール

```bash
[1]sys_openat: vnode->ino: 1, path: /proc/mounts, flags : 0x20000, mode: 0x0
[1]vfs_open: [[v6]] cwd: 1, path: /proc/mounts, create: 0
[1]vfs_lookup: start with [[v6]] cwd->ino: 1, path: /proc/mounts, flags: 0x0
[1]vfs_lookup: [[v6]] LP[1] cur->ino: 1, ref: 14, bits: 0x0
[1]vfs_lookup: COMP[1]: 'proc'
[1]vfs_lookup: FLG (SELF), path[i]: 'm'
[1]vfs_lookup: fs of cur->ino 1 is [[v6]] root_node: 1
[1]vfs_lookup: call ops->lookup with [[v6]] cur: 0xffff0000001c43d8 (1), component: proc, &cur: 0xffff000000ba7d78
[1]vfs_lookup: [[v6]] LP[2] cur->ino: 5, ref: 2, bits: 0x1
[1]vfs_lookup: CHG: cur to [[procfs]] 0
[1]vfs_lookup: COMP[2]: 'mounts'
[1]vfs_lookup: FLG (SELF), path[i]: ''
[1]vfs_lookup: fs of cur->ino 0 is [[procfs]] root_node: 0
[1]vfs_lookup: call ops->lookup with [[procfs]] cur: 0xffff0000001c8148 (0), component: mounts, &cur: 0xffff000000ba7d78
[1]procfs_lookup: vnode: 0xffff0000001c8148 (0), filename: mounts, result: 0xffff000000ba7d78
[1]_find_filenum_by_name: fn mounts != ent[0] .
[1]_find_filenum_by_name: fn mounts != ent[1] ..
[1]_find_filenum_by_name: fn mounts == ent[2] mounts and return 10
[1]procfs_lookup: [0] mounts is filenum: 10
[1]_find_vnode: pid: 0, filenum: 10, mode: 0x41ed, mp->root_node: 0x0
[1]_find_vnode: vnode_table[0] vn.refcount: 1, pid: 0, filenum: 0
[1]_find_vnode: vnode_table[1] vn.refcount: 0, pid: 0, filenum: 0
[1]_find_vnode: vnode_table[2] vn.refcount: 0, pid: 0, filenum: 0
[1]_find_vnode: vnode_table[3] vn.refcount: 0, pid: 0, filenum: 0
[1]_find_vnode: vnode_table[4] vn.refcount: 0, pid: 0, filenum: 0
[1]_find_vnode: vnode_table[5] vn.refcount: 0, pid: 0, filenum: 0
[1]_find_vnode: call _alloc_vnode
[1]_alloc_vnode: pid: 0, filenum: 10, mode: 0x41ed, mp->root_node: 0x0
[1]_alloc_vnode: [1] ALLOC: vnode->ino: 10, mode: 0x41ed, filenum: 10
[1]vfs_lookup: [[procfs]] LP[3] cur->ino: 10, ref: 1, bits: 0x0
[1]vfs_lookup: [[procfs]] OK: cur->ino: 10, ref: 1
[1]vfs_open: path: /proc/mounts, vnode: 10
[1]procfs_open: file: 0x1c74e8, f->vnode->ino: 10
[1]sys_openat: vfs_open ok: set file->vnode->ino: 10 to fd=3
[1]sys_openat: fd: 3, file->vnode->ino: 10, ref: 2
[1]procfs_read: file: pid: 0, ino: 10, nbytes: 0x40
[1]procfs_read: PROCFS_DATA(file->vnode): pid: 0, filenum: 10
[1]procfs_read: entry from root_files
[1]procfs_read: exec func: 0xffff0000000a5a50 with proc->pid: 0
[1]dump_mp: [vfs_mount_iter_next] fs: v6, mount_node: [[ND]] -1, root_node: 1, dev: 0x101
[1]get_data_mounts: mp fs: v6, mount_node: -1
[1]vfs_reverse_lookup: [[ND]] cwd->ino: -1, buf: 0xffff000000ba7c98, size: 0x64, uid: 0
[1]vfs_reverse_lookup: cwd is NUll and set to [[v6]] rootf_fs->rooot_node: ino: 1
[1]vfs_reverse_lookup: start while loop from [[v6]] cur: 0xffff0000001c43d8 (1), root_fs->root_node: 0xffff0000001c43d8 (1)
[1]vfs_reverse_lookup: buf: /, j: 98, len: 1
[1]dump_mp: [vfs_mount_iter_next] fs: procfs, mount_node: [[v6]] 5, root_node: 0, dev: 0x102
[1]get_data_mounts: mp fs: procfs, mount_node: 5
[1]vfs_reverse_lookup: [[v6]] cwd->ino: 5, buf: 0xffff000000ba7c98, size: 0x64, uid: 0
[1]vfs_reverse_lookup: start while loop from [[v6]] cur: 0xffff0000001c44b0 (5), root_fs->root_node: 0xffff0000001c43d8 (1)
[1]vfs_reverse_lookup: check [[v6]] cur: 0xffff0000001c44b0 (5) with cur->mp->root_node: 0xffff0000001c43d8 (1)
[1]vfs_reverse_lookup: cur/..
[1]vfs_open: [[v6]] cwd: 5, path: .., create: 0
[1]vfs_lookup: start with [[v6]] cwd->ino: 5, path: .., flags: 0x0
[1]vfs_lookup: [[v6]] LP[1] cur->ino: 5, ref: 3, bits: 0x1
[1]vfs_lookup: CHG: cur to [[procfs]] 0
[1]vfs_lookup: COMP[1]: '..'
[1]vfs_lookup: FLG (SELF), path[i]: ''
[1]vfs_lookup: fs of cur->ino 0 is [[procfs]] root_node: 0
[1]vfs_lookup: CHG cur to [[procfs]]'s mount_node: 5
[1]vfs_lookup: call ops->lookup with [[v6]] cur: 0xffff0000001c44b0 (5), component: .., &cur: 0xffff000000ba7ad8        // ここでストール
```

- 以下のような使用方が正しい？

```bash
$ /bin/cat /proc/mounts
/ 101 v6 rw
/proc 102 procfs ro
# ctrl+p
1 sleep  init
2 runble idle
3 runble idle
4 run    idle
5 run    idle
6 run    rxether
7 sleep  sh fa: 1
$ /bin/cat /proc/7/stat
7 sh S 1 0 0 0 0 4236272
$ /bin/cat /proc/1/stat
[0]trap: [11] unknown trap code: 37 at 0xffff0000000a5aa0 with 0x2c
[0]exit: exit: pid 11, err 1    // 0xffff0000000a5aa0 は get_data_stat()のget_proc_state()
1 init S -1 0 0 0 0 4208568
$ /bin/cat /proc/6/stat         // これはinitにparentがなかったため
[1]trap: [18] unknown trap code: 37 at 0xffff0000000a5aa0 with 0x2c
[1]exit: exit: pid 18, err 1    // これはproc 6はカーネルスレッドでやはりparentを設定していないため
6 rxether R -1 0 0 0 0 4096
$ /bin/cat /proc/7/cmdline
[0]_alloc_vnode: pid: 7, filenum: 1, mode: 0x41ed, mp->root_node: 0x0
[0]_alloc_vnode: return NULL
[0]vfs_lookup:      // ここでストール
shx
$ /bin/cat /proc/7/statm
4236272 400000 a000 0 0 1bb848 bb8ed0 bb8d70
$ /bin/ls /proc
$                   // 何も表示されない
$ /bin/ls /proc/7
$                   // 何も表示されない
```

- procfsの仕様自体を検討する必要があるだろう

## 1月4日

- デバッグ行を削除
-

```bash
$ /bin/cat /proc/mounts
/ 101 v6 rw
/proc 102 procfs ro
$ /bin/cat /proc/7/stat
7 sh S 1 0 0 0 0 4236272
$ /bin/cat /proc/7/statm
4236272 400000 a000 0 0 1b9848 bb8ed0 bb8d70
$ /bin/cat /proc/7/cmdline
sh
$ /bin/cat /proc/1/stat
1 init S -1 0 0 0 0 4208568$
$ /bin/cat /proc/6/stat
[2]trap: [13] unknown trap code: 37 at 0xffff0000000a56f8 with 0x24
[2]exit: exit: pid 13, err 1
$ /bin/cat /proc/1/statm
[0]trap: [14] unknown trap code: 37 at 0xffff0000000a56f8 with 0x24
[0]exit: exit: pid 14, err 1

$ /bin/cat /proc/1/stat
1 init S -1 0 0 0 0 4208568$
$ /bin/cat /proc/1/statm
4208568 400000 a000 0 0 1b9068 bffed0 bffd70
$ /bin/cat /proc/1/cmdline
init
$ /bin/cat /proc/7/stat
7 sh S 1 0 0 0 0 4236272
$ /bin/cat /proc/7/statm
4236272 400000 a000 0 0 1b9848 bfded0 bfdd70
$ /bin/cat /proc/7/cmdline
[0]trap: [13] unknown trap code: 37 at 0xffff0000000a56f8 with 0x24
[0]exit: exit: pid 13, err 1
```

```bash
<vfs_lookup>
ffff0000000a56f8:   b9402401    ldr w1, [x0, #36]		# w1 = cwd->bits
```

- `ls`出力を検討

```bash
$ /bin/ls /proc
[3]syscall1: proc[8] sys_gettid called
[3]syscall1: proc[8] sys_openat called
[3]syscall1: proc[8] sys_fstat called
[3]syscall1: proc[8] sys_read called
[3]syscall1: proc[8] sys_close called
[3]syscall1: proc[8] sys_exit_group called
$ /bin/ls /proc/7
[1]syscall1: proc[8] sys_gettid called
[1]syscall1: proc[8] sys_openat called
[1]syscall1: proc[8] sys_fstat called
[1]syscall1: proc[8] sys_read called
[1]syscall1: proc[8] sys_close called
[1]syscall1: proc[8] sys_exit_group called
$ /bin/ls /proc/7/stat
[1]syscall1: proc[10] sys_gettid called
[1]syscall1: proc[10] sys_openat called
[1]syscall1: proc[10] sys_fstat called
[1]syscall1: proc[10] sys_read called
[1]syscall1: proc[10] sys_close called
[1]syscall1: proc[10] sys_exit_group called

$ /bin/ls /proc
[1]sys_fstat: fd: 3, ino: 0, mode: 0x41ed
[3]sys_read: [8] fd: 3, buf: 0xfffffffffc70 (''), count: 0x40, ret: 0x0
$ /bin/ls /proc/7
[3]sys_fstat: fd: 3, ino: 1, mode: 0x41ed
[1]sys_read: [9] fd: 3, buf: 0xfffffffffc70 (''), count: 0x40, ret: 0x0
$ /bin/ls /proc/7/stat
[0]sys_fstat: fd: 3, ino: 3, mode: 0x41ed
[3]sys_read: [10] fd: 3, buf: 0xfffffffffc70 ('7 sh S 1 0 0 0 0 4236272
'), count: 0x40, ret: 0x19
```

- root_filesの'.'にfuncを追加

```bash
$ /bin/ls /proc
[3]sys_read: [7] fd: 0, buf: 0x409e08 ('/bin/ls /proc
'), count: 0x400, ret: 0xe
[1]sys_fstat: fd: 3, ino: 0, mode: 0x41ed
[1]procfs_read: exec func: 0xffff0000000a52d0 with proc->pid: 0  // get_root_dir
[1]sys_read: [8] fd: 0, buf: 0x0 ('(null)'), count: 0x0, ret: 0x40
[1]trap: [8] unknown trap code: 34 at 0x73746e75 with 0x73746e75
[1]exit: exit: pid 8, err 1

$ /bin/cat /proc/mounts
[3]sys_read: [7] fd: 0, buf: 0x409e08 ('/bin/cat /proc/mounts
'), count: 0x400, ret: 0x16
[2]_find_filenum_by_name: fn mounts == ent[2] mounts and return 10
[2]procfs_read: exec func: 0xffff0000000a5270 with proc->pid: 0
[2]sys_read: [9] fd: 3, buf: 0xfffffffffd50 ('/ 101 v6 rw
/proc 102 procfs ro
'), count: 0x200, ret: 0x20
/ 101 v6 rw
/proc 102 procfs ro
[2]procfs_read: exec func: 0xffff0000000a5270 with proc->pid: 0
[2]sys_read: [9] fd: 3, buf: 0xfffffffffd50 ('/ 101 v6 rw
/proc 102 procfs ro
'), count: 0x200, ret: 0x0
$ /bin/ls /proc
[3]sys_read: [7] fd: 0, buf: 0x409e08 ('/bin/ls /proc
/mounts
'), count: 0x400, ret: 0xe
[2]sys_fstat: fd: 3, ino: 0, mode: 0x41ed
[2]procfs_read: exec func: 0xffff0000000a5350 with proc->pid: 0
[2]sys_read: [10] fd: 0, buf: 0x0 ('(null)'), count: 0x0, ret: 0x40
[2]trap: [10] unknown trap code: 34 at 0x73746e75 with 0x73746e75
[2]exit: exit: pid 10, err 1
$ /bin/ls /proc/1
[1]sys_read: [7] fd: 0, buf: 0x409e08 ('/bin/ls /proc/1
'), count: 0x400, ret: 0x22
[2]execve: uid 0 can't access /bin/ls
execve: Operation not permitted
$ /bin/ls
[1]sys_read: [7] fd: 0, buf: 0x409e08 ('/bin/ls
/bin/ls /proc/1
'), count: 0x400, ret: 0x8
[3]sys_fstat: fd: 3, ino: 1, mode: 0x41fd
[3]sys_read: [12] fd: 3, buf: 0xfffffffffc80 (''), count: 0x40, ret: 0x40
drwxrwxr-x    1 root wheel  4096  1  3 11:12 .
[3]sys_read: [12] fd: 3, buf: 0xfffffffffc80 (''), count: 0x40, ret: 0x40
drwxrwxr-x    1 root wheel  4096  1  3 11:12 ..
```

## 1月5日

```bash
$ /bin/ls /proc
[2]sys_fstat: fd: 3, ino: 0, mode: 0x41ed
1234a567g
[2]procfs_readdir: file->vnode: ino: 0, mode: 0x41ed, offset: 0
=== struct dirent dump ===
+------+-------------------------------------------------+------------------+
| 0000 | 00 00 00 00 00 00 31 00 01 00 00 00 00 00 00 00 | ......1......... |
| 0010 | 54 50 21 00 00 00 ff ff 40 50 21 3f 00 00 ff ff | TP!.....@P!?.... |
| 0020 | c0 7c ba 00 00 00 ff ff 01 00 00 00 00 00 00 00 | .|.............. |
| 0030 | 00 00 00 00 00 00 00 00 00 90 40 00 00 00 00 00 | ..........@..... |
+------+-------------------------------------------------+------------------+

h8
[2]procfs_read: [0] offset: 0x0, nbytes: 0x40, limit: 0x40
[2]trap: [8] unknown trap code: 37 at 0xffff0000000a4c9c with 0xffff000100ba7d60
[2]exit: exit: pid 8, err 1
```

```bash
$ /bin/ls /proc
[2]sys_fstat: fd: 3, ino: 0, mode: 0x41ed
1234a567g
[2]procfs_readdir: file->vnode: ino: 0, mode: 0x41ed, offset: 0
[2]get_root_dir: nbytes: 0x1, file->offset: 0x0
[2]trap: [8] unknown trap code: 37 at 0xffff0000000a56c0 with 0xffff000100ba7d5f
[2]exit: exit: pid 8, err 1
```

- `ls`コマンド用に`struct vfile`を引数に持つ`procfs_data_t`とは異なる関数を追加し、
  root_filesとproc_filesの`.`に設定

```bash
$ /bin/ls /proc
drwxr-xr-x    0 root wheel     0  1  1 09:00 .
drwxrwxr-x    1 root wheel  4096  1  3 11:12 ..
drwxr-xr-x   10 root wheel     0  1  1 09:00 mounts
$ /bin/ls /proc/7
drwxr-xr-x    1 root wheel     0  1  1 09:00 .
drwxr-xr-x    0 root wheel     0  1  1 09:00 ..
drwxr-xr-x    2 root wheel     0  1  1 09:00 cmdline
drwxr-xr-x    3 root wheel     0  1  1 09:00 stat
drwxr-xr-x    4 root wheel     0  1  1 09:00 statm
$ /bin/cat /proc
123456710...mounts$
$ /bin/cat /proc/mounts
/ 101 v6 rw
/proc 102 procfs ro
$ /bin/cat /proc/7/stat
7 sh S 1 0 0 0 0 4236272
$ /bin/cat /proc/7
...cmdlinestatstatm$
$ /bin/cat /proc/7/statm
4236272 400000 a000 0 0 1b9898 bb8ed0 bb8d70
$ /bin/cat /proc/7/cmdline
sh
```

- `ls /proc`で実行中のpidも出力されるようにした
- inoは`PFN_PROCDIR`とした

```bash
$ /bin/ls /proc
drwxr-xr-x    1 root wheel     0  1  5 16:08 1
drwxr-xr-x    1 root wheel     0  1  5 16:08 2
drwxr-xr-x    1 root wheel     0  1  5 16:08 3
drwxr-xr-x    1 root wheel     0  1  5 16:08 4
drwxr-xr-x    1 root wheel     0  1  5 16:08 5
drwxr-xr-x    1 root wheel     0  1  5 16:08 6
drwxr-xr-x    1 root wheel     0  1  5 16:08 7
drwxr-xr-x    1 root wheel     0  1  5 16:08 8
drwxr-xr-x    0 root wheel     0  1  5 16:08 .
drwxrwxr-x    1 root wheel  4096  1  3 11:12 ..
drwxr-xr-x   10 root wheel     0  1  5 16:08 mounts
$ /bin/cat /proc
12345679...mounts$              // pid=8は`/bin/ls /proc`ですでに終了, 9は`/bin/cat /proc`
$ /bin/ls /proc/7
drwxr-xr-x    1 root wheel     0  1  5 16:09 .
drwxr-xr-x    0 root wheel     0  1  5 16:09 ..
drwxr-xr-x    2 root wheel     0  1  5 16:09 cmdline
drwxr-xr-x    3 root wheel     0  1  5 16:09 stat
drwxr-xr-x    4 root wheel     0  1  5 16:09 statm
$ /bin/cat /proc/7/stat
7 sh S 1 0 0 0 0 4236272
$ /bin/cat /proc/7/statm
4236272 400000 a000 0 0 1b9898 bfded0 bfdd70
$ /bin/cat /proc/7/cmdline
sh
$ /bin/cat /proc/1/cmdline
init
```

## 1月6日

- lsコマンドの実装がxv6とglowormで異なる(direntの取得を前者はread()で校舎はreaddir())で
  行っていたので後者の実装をls2としてxv6に移植
- ただし、muslではreaddir()はライブラリ関数で内部でgetdents64()を呼んでいるので
  procfsでもgetdents()を実装

```bash
$ /bin/ls /proc
drwxr-xr-x    1 root wheel     0  1  6 12:08 1
drwxr-xr-x    1 root wheel     0  1  6 12:08 2
drwxr-xr-x    1 root wheel     0  1  6 12:08 3
drwxr-xr-x    1 root wheel     0  1  6 12:08 4
drwxr-xr-x    1 root wheel     0  1  6 12:08 5
drwxr-xr-x    1 root wheel     0  1  6 12:08 6
drwxr-xr-x    1 root wheel     0  1  6 12:08 7
drwxr-xr-x    1 root wheel     0  1  6 12:08 8
drwxr-xr-x    0 root wheel     0  1  6 12:08 .
drwxrwxr-x    1 root wheel  4096  1  6 11:56 ..
drwxr-xr-x   10 root wheel     0  1  6 12:08 mounts

$ /bin/ls2
[2]sys_fcntl: fd: 3, cmd: 0x2
[2]sys_mmap: addr: 0x0, len: 8192, prot: 0x3, flags: 0x22
[2]trap: [12] unknown trap code: 36 at 0x40033c with 0xc
```

- sys_mmap()の実装が必要だった

## 1月8日

- sys_mmap()を実装（xv6-milkv-muslで改造したものをベースにarm部分は
  mac-rpi-osの実装を使用した。

```bash
$ /bin/ls /proc
drwxr-xr-x    1 root wheel     0  1  8 16:07 1
drwxr-xr-x    1 root wheel     0  1  8 16:07 2
drwxr-xr-x    1 root wheel     0  1  8 16:07 3
drwxr-xr-x    1 root wheel     0  1  8 16:07 4
drwxr-xr-x    1 root wheel     0  1  8 16:07 5
drwxr-xr-x    1 root wheel     0  1  8 16:07 6
drwxr-xr-x    1 root wheel     0  1  8 16:07 7
drwxr-xr-x    1 root wheel     0  1  8 16:07 8
drwxr-xr-x    0 root wheel     0  1  8 16:07 .
drwxrwxr-x    1 root wheel  4096  1  8 15:04 ..
drwxr-xr-x   10 root wheel     0  1  8 16:07 mounts
$ /bin/ls2 /proc
drwxr-xr-x      0 2026-01-08 07:07:36 1
drwxr-xr-x      0 2026-01-08 07:07:36 2
drwxr-xr-x      0 2026-01-08 07:07:36 3
drwxr-xr-x      0 2026-01-08 07:07:36 4
drwxr-xr-x      0 2026-01-08 07:07:36 5
drwxr-xr-x      0 2026-01-08 07:07:36 6
drwxr-xr-x      0 2026-01-08 07:07:36 7
drwxr-xr-x      0 2026-01-08 07:07:36 9
drwxr-xr-x      0 2026-01-08 07:07:23 .
drwxrwxr-x   4096 2026-01-08 06:04:44 ..
drwxr-xr-x      0 2026-01-08 07:07:36 mounts
$ /bin/ls /proc/7
drwxr-xr-x    1 root wheel     0  1  8 16:07 .
drwxr-xr-x    0 root wheel     0  1  8 16:07 ..
drwxr-xr-x    2 root wheel     0  1  8 16:07 cmdline
drwxr-xr-x    3 root wheel     0  1  8 16:07 stat
drwxr-xr-x    4 root wheel     0  1  8 16:07 statm
$ /bin/ls2 /proc/7
drwxr-xr-x      0 2026-01-08 07:07:50 .
drwxr-xr-x      0 2026-01-08 07:07:59 ..
drwxr-xr-x      0 2026-01-08 07:07:59 cmdline
drwxr-xr-x      0 2026-01-08 07:07:59 stat
drwxr-xr-x      0 2026-01-08 07:07:59 statm
$ /bin/cat /proc/mounts
/ 101 v6 rw
/proc 102 procfs ro
$ /bin/cat /proc/7/stat
7 sh S 1 0 0 0 0 4236272
$ /bin/cat /proc/7/statm
4236272 400000 a000 0 0 1bc8f0 bb8ed0 bb8d50
$ /bin/cat /proc/7/cmdline
sh
$ /bin/ls .
drwxrwxr-x    1 root wheel  4096  1  8 15:04 .
drwxrwxr-x    1 root wheel  4096  1  8 15:04 ..
drwxrwxr-x    2 root wheel  1152  1  8 15:04 bin
drwxrwxr-x    3 root wheel   192  1  8 15:04 dev
drwxr-xr-x    0 root wheel     0  1  8 16:07 proc
-rwxr-xr-x    6 root wheel    34  1  8 15:04 test.txt
$ /bin/ls2 /
drwxrwxr-x   4096 2026-01-08 06:04:44 .
drwxrwxr-x   4096 2026-01-08 06:04:44 ..
drwxrwxr-x   1152 2026-01-08 06:04:44 bin
drwxrwxr-x    192 2026-01-08 06:04:44 dev
drwxr-xr-x      0 2026-01-08 07:07:23 proc
-rwxr-xr-x     34 2026-01-08 06:04:44 test.txt
```

- mmaptest, mmaptest2共にまともに動かないので要チェック
