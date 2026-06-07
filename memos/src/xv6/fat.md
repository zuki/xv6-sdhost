# FAT

```bash
[1]emmc_card_reset: found valid version 3.0x SD card
[1]sd_postinit: partition[0]: TYPE: 12, LBA = 0x800, #SECS = 0x20000
[1]sd_postinit: partition[1]: TYPE: 131, LBA = 0x20800, #SECS = 0x20000
[1]sd_postinit: partition[2]: TYPE: 12, LBA = 0x40800, #SECS = 0x20000
[1]sd_postinit: sd_postinit ok

[1]release: error: card is not locked
[1]v6_set_super: v6_sb: size 1000 nblocks 960 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 39
[1]f_mount: start
[1]f_mount: vol: 2
[1]f_mount: call mount_volume
[1]mount_volume: start
[1]disk_initialize: disk_initialize, drv 2
[1]mount_volume: mode: 0x0 ok
[1]find_volume: fs: 0xffff0000003475f0, part: 0
[1]check_fs: called
[1]trap: [1] unknown trap code: 34 at elr: 0x17fffffed4000001 with far: 0x17fffffed4000001
=== dump trapframe        ===
  spsr: 0x800003c5
   elr: 0x17fffffed4000001
    sp: 0x0
 tpidr: 0xaaaaaaaaaaaaaaaa
    x1: 0xffff0000073dde00
    x2: 0x0
    x3: 0x200
    x8: 0x31
   x29: 0xffff0000073fecd0
   x30: 0xffff0000000b11d4
===-----------------------===
```

- V6とFAT2の順番を変え、FAT2のデバイス番号を4とした。

```bash
[3]emmc_card_reset: found valid version 3.0x SD card
[2]sd_postinit: partition[0]: TYPE: 12, LBA = 0x800, #SECS = 0x20000
[2]sd_postinit: partition[1]: TYPE: 12, LBA = 0x20800, #SECS = 0x20000
[2]sd_postinit: partition[2]: TYPE: 131, LBA = 0x40800, #SECS = 0x20000
[2]sd_postinit: sd_postinit ok

[2]release: error: card is not locked
[1]v6_set_super: v6_sb: size 1000 nblocks 960 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 39
[2]forkret: mount /proc is failed: -2
[2]f_mount: start
[2]f_mount: vol: 4
[2]f_mount: call mount_volume
[2]mount_volume: start
[2]disk_initialize: disk_initialize, drv 4
[2]mount_volume: mode: 0x0 ok
[2]find_volume: fs: 0xffff0000003475f0, part: 0
[2]check_fs: called
[2]trap: [1] unknown trap code: 34 at elr: 0x17fffffed4000001 with far: 0x17fffffed4000001
=== dump trapframe        ===
  spsr: 0x800003c5
   elr: 0x17fffffed4000001
    sp: 0x0
 tpidr: 0x2aaaaa8a2a8a3aaa
    x1: 0xffff0000073dde00
    x2: 0x0
    x3: 0x200
    x8: 0x32
   x29: 0xffff0000073fecd0
   x30: 0xffff0000000b11d4
===-----------------------===
```

- 6/6

```bash
[3]emmc_card_reset: found valid version 3.0x SD card
[1]sd_postinit: partition[0]: TYPE: 12, LBA = 0x800, #SECS = 0x20000
[1]sd_postinit: partition[1]: TYPE: 12, LBA = 0x20800, #SECS = 0x20000
[1]sd_postinit: partition[2]: TYPE: 131, LBA = 0x40800, #SECS = 0x20000
[1]sd_postinit: sd_postinit ok

[1]get_block: dev: 0x102, bno: 1, issec: 0
[1]_read_entry: read: dev: 0x102, buffer: 0x73e0000, bno: 0x1, size: 0x1000
[1]sd_read: minor: 2, buffer: 0x73e0000, offset: 0x1, size: 0x1000
[1]release: error: card is not locked
[3]v6_set_super: v6_sb: size 1000 nblocks 960 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 39
[3]vfs_mount: lockup path: /proc
[3]vfs_lookup: [[v6]] LP[1] cur->ino: 1, ref: 3, bits: 0x0
[3]vfs_lookup: COMP[1]: 'proc'
[3]vfs_lookup: FLG (SELF), path[i]: ''
[3]vfs_lookup: fs of cur->ino 1 is [[v6]] root_node: 1
[3]vfs_lookup: call ops->lookup with [[v6]] cur: 0xffff000000343f80 (1), component: proc, &cur: 0xffff0000073fede8
[3]v6_lookup: vnode->ino: 1, rdev: 0x102, mode: 0x41ed, ip->valid: 0, COMP: proc
[3]v6_ilock: vp->ino: 1, rdev: 0x102, valid: 0, type: 1, mode: 0x41ed
[3]get_block: dev: 0x102, bno: 32, issec: 0
[3]_read_entry: read: dev: 0x102, buffer: 0x73df000, bno: 0x20, size: 0x1000
[3]sd_read: minor: 2, buffer: 0x73df000, offset: 0x20, size: 0x1000
[3]v6_ilock: type: 1, nlink: 1, rdev: 0x102, size: 0x1000, mode: 0x41fd, addrs[0]: 0x28
[3]v6_dirlookup: dp->ino: 1, name: proc
[3]v6_readi: ip: ino:1, rdev: 0x102, off: 0x0, n: 64
[3]get_block: dev: 0x102, bno: 40, issec: 0
[3]_read_entry: read: dev: 0x102, buffer: 0x73de000, bno: 0x28, size: 0x1000
[3]sd_read: minor: 2, buffer: 0x73de000, offset: 0x28, size: 0x1000
[3]v6_readi: ip: ino:1, rdev: 0x102, off: 0x40, n: 64
[3]get_block: dev: 0x102, bno: 40, issec: 0
[3]v6_readi: ip: ino:1, rdev: 0x102, off: 0x80, n: 64
[3]get_block: dev: 0x102, bno: 40, issec: 0
[3]v6_readi: ip: ino:1, rdev: 0x102, off: 0xc0, n: 64
[3]get_block: dev: 0x102, bno: 40, issec: 0
[3]v6_readi: ip: ino:1, rdev: 0x102, off: 0x100, n: 64
[3]get_block: dev: 0x102, bno: 40, issec: 0
[3]v6_dirlookup: found by name: proc, dev: 0x102, ino: 5
[3]v6_lookup: ip->ino: 5
[3]v6_ilock: vp->ino: 5, rdev: 0x102, valid: 0, type: 0, mode: 0x0
[3]get_block: dev: 0x102, bno: 32, issec: 0
[3]v6_ilock: type: 1, nlink: 1, rdev: 0x102, size: 0x80, mode: 0x41ed, addrs[0]: 0x2b
[3]vfs_lookup: [[v6]] LP[2] cur->ino: 5, ref: 1, bits: 0x0
[3]vfs_lookup: [[v6]] OK: cur->ino: 5, ref: 1
[3]vfs_mount: found path: /proc, vnode->ino: 5
[3]dump_mp: === dump mount point from vfs_mount ===
[3]dump_mp: fstype: procfs
[3]dump_mp: mount_node: 0xffff000000344058 (5)
[3]dump_mp: root_node: 0xffff00000034a110 (0)
[3]dump_mp: super: 0x0
[3]dump_mp: dev: 0x106
[3]dump_mp: bits: 0x1

[3]vfs_mount: lockup path: /d/
[3]vfs_mount: found path: /d/, vnode->ino: -1
[3]f_mount: start
[3]f_mount: vol: 4
[3]f_mount: call mount_volume
[3]mount_volume: start
[3]disk_initialize: disk_initialize, drv 4
[3]mount_volume: mode: 0x0
[3]find_volume: fs: 0xffff000000346a10, part: 0
[3]check_fs: called
[3]move_window: sect: 0, winsect: -1
[3]move_window: fs->win: 0xffff000000346a50
[3]disk_read: drv: 4, buff: 0xffff000000346a50, sector: 0, count: 1
[3]get_block: dev: 0x1, bno: 0, issec: 1
[3]_read_entry: read: dev: 0x1, buffer: 0x73dde00, bno: 0x0, size: 0x200
[3]trap: [1] unknown trap code: 34 at elr: 0x17fffffed4000001 with far: 0x17fffffed4000001
=== dump trapframe        ===
  spsr: 0x800003c5
   elr: 0x17fffffed4000001
    sp: 0x0
 tpidr: 0xaa22abaaaaaaaa0a
    x1: 0xffff0000073dde00
    x2: 0x0
    x3: 0x200
    x8: 0x32
   x29: 0xffff0000073feba0
   x30: 0xffff0000000b18fc
===-----------------------===
```

## shが立ち上がるとこまで成功

- drvからdeviceへの変換が間違っていた
- ドライブ名でFATを判定するコードが間違っていた

```bash
$ /bin/ls /
drwxrwxr-x    1 root wheel  4096  6  7 09:28 .
drwxrwxr-x    1 root wheel  4096  6  7 09:28 ..
drwxrwxr-x    2 root wheel  1536  6  7 09:28 bin
drwxrwxr-x    3 root wheel   192  6  7 09:28 dev
drwxr-xr-x    0 root wheel     0  1  1 09:00 proc
drwxrwxrwx    6 root wheel   256  6  7 09:28 lib
-rwxr-xr-x    9 root wheel    34  6  7 09:28 test.txt
$ /bin/ls /d/
ilockkern/drivers/console.c:264: kernel panic at cpu 1.
```

- refcountのチェックを外す

```bash
$ /bin/ls /d/
[3]sys_openat: dirfd: -100, path: /d/, flags : 0x20000, mode: 0x0
[3]open_path: path: 4:/ mode: 0x0
[3]open_path: ino: 193435586, mode: 0x0, dir: 1
[3]sys_fstat: fd: 3, st: 0xfffffffffb80
[3]sys_fstat: fd: 3, ino: 193435586, mode: 0x0
```

- lsコマンドのコードではst.stmodeで通常ファイルとディレクトリを判定している。
  modeが設定されていないので表示されないものと思われる

## fatｆｓのｌｓ表示に成功

- lsコマンドもtypeコードでT_DIR_FATは別処理をしていた。
- typeはv6とfatで別のコードとしてたので統一してvfs.hに移した。
- muslの`struct stat`にはこのtypeコードはないため、当面、使用されていない
  st_devにセットしてlsのコードで使うようにした。
- fat_inodeのinoはhash値を使っているが、大きな数 (193435586など)になる場合が
  ある。lsの通常ファイルの表示は4桁となっている。

```bash
$ /bin/ls /d/
[3]fat_open: fd: 3, ip: ino: 193435586, mode: 0x4000, type: 9
--- FAT dir ---
name  attr  sz              // fatのlsはino, 時刻などは表示していない
fat.txt 32 44
longlongname.txt 32 37
.fseventsd 18 0
$ /bin/ls /
drwxrwxr-x    1 root wheel  4096  6  7 15:52 .
drwxrwxr-x    1 root wheel  4096  6  7 15:52 ..
drwxrwxr-x    2 root wheel  1536  6  7 15:52 bin
drwxrwxr-x    3 root wheel   192  6  7 15:52 dev
drwxr-xr-x    0 root wheel     0  1  1 09:00 proc
drwxrwxrwx    6 root wheel   256  6  7 15:52 lib
-rwxr-xr-x    9 root wheel    34  6  7 15:52 test.txt
$ /bin/date
1900年 1日 1日 月曜日  9時 0分30秒 JST          // 電池が切れた?
$ /bin/dns
// ストール
```
