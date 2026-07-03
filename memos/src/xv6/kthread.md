# kthreadの実装を修正

- googleの回答をもとに修正
    - 実行関数とその引数, kthreadフラグをstruct procに持つ
    - p->context->lrにはkthread_stub()をセットする
    - kthread_stubはそのプロセスのkthreadフラグが立っていたら
      関数を実行し、実行後はkthread_exit()を呼び出して自分を終了させる
    - kthread_exitでは状態をzombieとし、recycle_procスレッドを呼び出す
    - recycle_procではメモリを解放して状態をunusedとする

- cpuが100%になるのを防ぐためにsigreturn()に10msのdelayを追加した。
    - ktrhead_read_ether()にもdelayを追加したら /bin/ls /d/ が戻らなくなった。
    - 4つのcpuがすべてsleepになるとcpuをwakeupするものがなくなるためではないか?
      割り込みハンドラでwakeupできればよいが、そのようなハンドラがあるのか要チェック

```bash
[0]console_preinit: console_preinit ok
[0]buddy_init: buddy_init ok
[0]slab_cache_init: slab_cache_init ok
[0]cachepage_init: cachepage_init ok
[0]rand_init: rand_init ok
[0]proc_init: proc_init ok
[0]console_init: console_init ok
[0]tty_init: tty_init ok
[0]sd_init: sd_init ok

[0]init_vfs: init_vfs ok
[0]main: root_dev mount ok
[0]user_init: user_init ok
[0]timer_init: [0] timer_init ok
[3]timer_init: [3] timer_init ok
[0]trap_init: [0] trap_init ok
[3]trap_init: [3] trap_init ok
[0]main: cpu 0 init finished
[1]timer_init: [1] timer_init ok
[0]sdhost_probe: firmware sets clock divider
[3]main: cpu 3 init finished
[1]trap_init: [1] trap_init ok
[2]timer_init: [2] timer_init ok
[1]main: cpu 1 init finished
[2]trap_init: [2] trap_init ok
[2]main: cpu 2 init finished
[0]sdhost_set_ios: ios clock 400000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[0]sdhost_finish_command: error detected: CMD 0x4205, HSTS 0x40, EDM 0x10800
[0]sdhost_finish_command: command 5 timeout
[0]emmc_card_reset: OCR: 0xff80, 1.8v support: 0, SDHC support: 1
[0]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[0]emmc_card_reset: card CID: 0x27504853, 0x44333247, 0x60371922, 0x9f0184b5
[0]emmc_card_reset: RCA: 0x1
[3]emmc_card_reset: SCR: version 3.0x, bus_widths 0x5
[3]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 1, timing 0, vdd 0, drv_type 0
[3]emmc_card_reset: found valid version 3.0x SD card
[1]sd_postinit: partition[0]: TYPE: 12, LBA = 0x800, #SECS = 0x20000
[1]sd_postinit: partition[1]: TYPE: 12, LBA = 0x20800, #SECS = 0x20000
[1]sd_postinit: partition[2]: TYPE: 131, LBA = 0x40800, #SECS = 0x20000
[1]sd_postinit: sd_postinit ok

[1]release: error: card is not locked
[3]v6_set_super: v6_sb: size 1000 nblocks 960 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 39
[2]disk_initialize: disk_initialize, drv 4
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
[2]lan7800_init_macaddr: MAC address is b8:27:eb:ab:e8:48
[2]usb_standard_hub_enumerate_ports: Port 1: Device configured
[3]usb_standard_hub_enumerate_ports: Port 1: Device configured
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
[2]workqueue_init: workqueue_init ok, worker pid - 7
$ /bin/dns
$ /bin/date
2026年 7日 3日 金曜日 16時54分 8秒 JST
$ /bin/ls
drwxrwxr-x    1 root wheel 2026-07-03 07:46:30   4096 .
drwxrwxr-x    1 root wheel 2026-07-03 07:46:30   4096 ..
drwxrwxr-x    2 root wheel 2026-07-03 07:46:30   1472 bin
drwxrwxr-x    3 root wheel 2026-07-03 07:46:30    192 dev
drwxr-xr-x    4 root wheel 2026-07-03 07:46:30    128 proc
drwxrwxrwx    5 root wheel 2026-07-03 07:46:30    256 lib
-rwxr-xr-x    9 root wheel 2026-07-03 07:46:30     34 test.txt
$ /bin/ls /d/
-rw-r--r--    0 root wheel 2026-07-03 16:46:30     44 fat.txt
-rw-r--r--    0 root wheel 2026-07-03 16:46:30     37 longlongname.txt
$
  1 sleeping init
  2 running  idle
  3 running  idle
  4 runnable idle
  5 running  idle
  6 runnable ether
  7 sleeping workqueue
  8 sleeping recycle
  9 sleeping sh ppid: 1

$
  1 sleeping init
  2 running  idle
  3 running  idle
  4 runnable idle
  5 runnable idle
  6 running  ether
  7 sleeping workqueue
  8 sleeping recycle
  9 sleeping sh ppid: 1

$
  1 sleeping init
  2 running  idle
  3 runnable idle
  4 running  idle
  5 running  idle
  6 runnable ether
  7 sleeping workqueue
  8 sleeping recycle
  9 sleeping sh ppid: 1

$ /bin/ls /d/
-rw-r--r--    0 root wheel 2026-07-03 16:46:30     44 fat.txt
-rw-r--r--    0 root wheel 2026-07-03 16:46:30     37 longlongname.txt
$
 1 sleeping init
  2 running  idle
  3 runnable idle
  4 runnable idle
  5 running  idle
  6 running  ether
  7 sleeping workqueue
  8 sleeping recycle
  9 sleeping sh ppid: 1

execve: Operation not permitted
$
```

#
