# Network (Ethernet) 機能を導入

- [circleのnetサブシステムの調査](circle_net.md)
- [xv6-netのnetサブシステムを調査](xv6_net.md)
- [linuxのドライバを調査](dw2.md)

## pcからのpingに正常対応

- xv6-riscv-netのnetwork機能を導入
- 外部からの受信はkthread()で読み取り関数をloop呼び出し
- QEMUはネットワーク機能が動かないことが判明。実機で作業を行う

## これまでの作業履歴

### 1. USB割り込みの調査

- usb_init()におけるUSB割り込み
    - GINTSTS = 0x7000029 = 0b0000_0111_0000_0000_0000_0000_0010_1001
        - PTxFEmp (26) | HChInt (25) | PrtInt (24) | RxFLvl (5) | Sof (3) | CurMod (1)
- RxFLvlを有効にして、受信パケット到着を知ることができるか？

**結論** 知ることはできなかった

### 2. xv6-riscv-netのネットワーク機能を導入

- 何も動かずフリーズ

```bash
qemu-system-aarch64 -M raspi3b -nographic -serial null -serial mon:stdio -drive file=obj/sd.img,if=sd,format=raw -netdev user,id=net0,hostfwd=tcp::8080-:80 -device usb-net,netdev=net0 -trace events=events,file=trace.log -kernel obj/kernel8.img
```

- qemuトレース

```bash
usb_port_claim bus 0, port 1
usb_hub_reset dev 0
usb_port_attach bus 0, port 1, devspeed full, portspeed full+high
usb_dwc2_attach port 0x7fb493a794d0
usb_dwc2_attach_speed full-speed device attached
usb_dwc2_bus_start start SOFs
usb_dwc2_raise_global_irq 0x01000000
usb_port_claim bus 0, port 1.1
usb_port_attach bus 0, port 1.1, devspeed full, portspeed full
usb_hub_attach dev 0, port 1
usb_dwc2_reset_enter === RESET enter ===
usb_dwc2_detach port 0x7fb493a794d0
usb_dwc2_bus_stop stop SOFs
usb_dwc2_bus_stop stop SOFs
usb_dwc2_reset_hold === RESET hold ===
usb_dwc2_reset_exit === RESET exit ===
usb_dwc2_attach port 0x7fb493a794d0
usb_dwc2_attach_speed full-speed device attached
usb_dwc2_bus_start start SOFs
usb_dwc2_raise_global_irq 0x01000000
usb_hub_reset dev 0
usb_dwc2_raise_global_irq 0x00000008        // ここでフリーズ
```

- mm_init()で異常終了していた
- これはbuddyシステムで使用可能なファイル容量の定義が実機(1GB)用であり、
  QEMU環境(64MB)ではpages配列（PAGE_STARTに直下に作成する1ページ1項目の
  ページ使用済みフラグ）がカーネル領域を破壊していた
- PAGE_NUMの定義を64MB用に修正したところシェルまで進んだ(usb, netはコメントアウト)

**注** これは勘違いで、なにか別の原因だと思われる。

### 3. usb_init(), net_init()を追加

- usbの初期化ができない
- dwhc_xfer_stageで非同期転送を開始し、処理が終了するのを待つが処理終了フラグが立たない。
  これはUSB機能を導入した際と同じ現象。その際はCPUを4つ動かしたら解決したが、まだCPUは
  1つしか動いていない。
- usb_init()を呼び出した時点でirqはマスクされている。色々なタイミングでマスクを外したが
  依然として割り込みは発生しない

- lan7800とusb_cdcetherのnet部分の初期化をnet_init()からusb_init()に移し、
  変数`usb_lib`をstaticに戻す
- enable_irq(), pop_intr()など、xv6-riscvから取り入れた機能を外す。
- ARMは例外発生時に自動的に割り込みが無効になるので、例外ハンドラ内で明示的に有効にする必要が
  ある場合を除いてプログラムで有効にする必要はない（Lab3講義資料）。

**結論** proc.c#forkret()内で実行することで対応しているが、別の方法があるはず。要検討.

### 4. ether-input/trasmit-helper周りを実装

- muslにはSYS_sendとSYS_recvがない（それぞれSYS_sendto, SYS_recvfromを使用）
- ユーザプログラム`ifconfig`を導入
- MACアドレス固定でipアドレスを付番

- ネットワーク周りの定数がmuslと違った
- CDCのネットワーク名の指定がファイルにより異なっていた; "eth01"と"eth10"

```bash
[1]usb_dev_init: Device ven409-55aa, dev9-0-0 found
[1]usb_dev_init: Product: QEMU QEMU USB Hub
[1]usb_function_get_if_name: func name=int9-0-0
[1]usb_dev_init: Interface int9-0-0 found
[1]usb_dev_factory_get_device: Using device/interface int9-0-0
[3]usb_dev_init: Device ven525-a4a2, dev2-0-0 found
[2]usb_dev_init: Product: QEMU RNDIS/QEMU USB Network Device
[2]usb_function_get_if_name: func name=int2-6-0
[2]usb_dev_init: Interface int2-6-0 found
[2]usb_dev_factory_get_device: Using device/interface int2-6-0
[2]usb_function_get_if_name: func name=inta-0-0
[2]usb_dev_init: Interface inta-0-0 found
[2]usb_dev_init: Function is not supported
[2]usb_function_get_if_name: func name=inta-0-0
[2]usb_dev_init: Interface inta-0-0 found
[2]usb_dev_init: Function is not supported
[3]usb_cdcether_configure: MAC address is 40:54:0:12:34:57
[3]usb_standard_hub_enumerate_ports: Port 1: Device configured
[2]dwhc_root_port_init: Device configured
[2]usb_init: dwhc initialized

[2]net_device_register: registered, dev=net0, type=0x0002   // インタフェース名はnet0
[2]usb_init: usb_init ok
[2]net_protocol_register: registered, type=0x0800
[2]net_protocol_register: registered, type=0x0806
[2]net_timer_register: registered: interval={1, 0}
[2]ip_protocol_register: registered, type=1
[2]ip_protocol_register: registered, type=17
[2]ip_protocol_register: registered, type=6
[2]net_timer_register: registered: interval={0, 100000}
[2]netinit: initialized
[2]net_device_open: dev=net0, state=up
[2]netrun: running...
init: starting sh
sh: argv[0] = 'sh'
sh: testenv = 'FROM_INIT'

$ ifconfig net0 192.168.10.111 netmask 255.255.255.0
[1]ip_route_add: route added: network=192.168.10.111, netmask=255.255.255.255, nexthop=0.0.0.0, iface=192.168.10.111 dev=net0
[1]ip_iface_register: registered: dev=net0, unicast=192.168.10.111, netmask=255.255.255.255, broadcast=192.168.10.111
$ ifconfig
net0: flags=83<UP|BROADCAST|NEEDARP> mtu 1500
  ether 40:54:0:12:34:57
  inet 192.168.10.111 netmask 255.255.255.0 broadcast 192.168.10.255
$ ifconfig net0 down
[2]net_device_close: dev=net0, state=down
$ ifconfig
net0: flags=82<BROADCAST|NEEDARP> mtu 1500
  ether 40:54:0:12:34:57
  inet 192.168.10.111 netmask 255.255.255.0 broadcast 192.168.10.255
$ ifconfig net0 up
[1]net_device_open: dev=net0, state=up
$ ifconfig
net0: flags=83<UP|BROADCAST|NEEDARP> mtu 1500
  ether 40:54:0:12:34:57
  inet 192.168.10.111 netmask 255.255.255.0 broadcast 192.168.10.255
```

- ただしpingは通らず

```bash
$ ping 192.168.10.111
PING 192.168.10.111 (192.168.10.111): 56 data bytes
Request timeout for icmp_seq 0
Request timeout for icmp_seq 1
Request timeout for icmp_seq 2
Request timeout for icmp_seq 3
ping: sendto: No route to host
Request timeout for icmp_seq 4
ping: sendto: Host is down
^C
--- 192.168.10.111 ping statistics ---
4 packets transmitted, 0 packets received, 100.0% packet loss
```

- DWHCI_CORE_INT_STAT_RXFLVL 割り込みが起きないためと判明

- 実機も同じ

```bash
$ ifconfig net0 192.168.10.110 netmask 255.255.255.0
[2]ip_route_add: route added: network=192.168.10.110, netmask=255.255.255.255, nexthop=0.0.0.0, iface=192.168.10.110 dev=net0
[2]ip_iface_register: registered: dev=net0, unicast=192.168.10.110, netmask=255.255.255.255, broadcast=192.168.10.110
$ ifconfig
net0: flags=83<UP|BROADCAST|NEEDARP> mtu 1500
  ether b8:27:eb:ab:e8:48
  inet 192.168.10.110 netmask 255.255.255.0 broadcast 192.168.10.255
$ ifconfig net0 up
$ ifconfig
net0: flags=83<UP|BROADCAST|NEEDARP> mtu 1500
  ether b8:27:eb:ab:e8:48
  inet 192.168.10.110 netmask 255.255.255.0 broadcast 192.168.10.255
$
```

```bash
$ ping 192.168.10.110
PING 192.168.10.110 (192.168.10.110): 56 data bytes
Request timeout for icmp_seq 0
Request timeout for icmp_seq 1
Request timeout for icmp_seq 2
Request timeout for icmp_seq 3
ping: sendto: No route to host
Request timeout for icmp_seq 4
ping: sendto: Host is down
Request timeout for icmp_seq 5
ping: sendto: Host is down
Request timeout for icmp_seq 6
^C
--- 192.168.10.110 ping statistics ---
8 packets transmitted, 0 packets received, 100.0% packet loss
```

### 5. circle/sample/18-ntptimeを確認

- 他の端末からpingを発行したところ、GINtSTS#RxFLvl ビットが立った
- pingも通っている

```bash
logger: Circle 50 started on Raspberry Pi 3 Model B+ 1GB (AArch64)
logger: Revision code is a020d3, compiler has been GCC 11.2.1
00:00:00.66 timer: SpeedFactor is 1.51
00:00:01.29 kernel: Compile time: Sep 30 2025 09:49:32
00:00:01.52 usbdev0-1: Device ven424-2514, dev9-0-2 found (HS)
00:00:01.57 usbdev0-1: Interface int9-0-1 found
00:00:01.58 usbdev0-1: Function is not supported
00:00:01.58 usbdev0-1: Interface int9-0-2 found
00:00:01.59 usbdev0-1: Using device/interface int9-0-2
00:00:02.25 usbdev0-1: Device ven424-2514, dev9-0-2 found (HS)
00:00:02.31 usbdev0-1: Interface int9-0-1 found
00:00:02.31 usbdev0-1: Function is not supported
00:00:02.32 usbdev0-1: Interface int9-0-2 found
00:00:02.32 usbdev0-1: Using device/interface int9-0-2
00:00:02.99 usbdev0-1: Device ven424-7800 found (HS)
00:00:03.04 usbdev0-1: Using device/interface ven424-7800
00:00:03.13 lan7800: MAC address is B8:27:EB:AB:E8:48
00:00:03.27 usbhub: Port 1: Device configured
00:00:03.28 usbhub: Port 1: Device configured
00:00:03.28 dwroot: Device configured
00:00:06.03 dhcp: IP address is 192.168.10.110
00:00:06.04 kernel: Try "ping 192.168.10.110" from another computer!

00:00:07.89 dwhci: USB INTSTAT: 0x6000031           // bit: 26, 25, 5, 4, 1 are ON

Sep 30 00:50:31.04 ntpd: System time updated
Sep 30 00:50:47.79 dwhci: USB INTSTAT: 0x6000031

Sep 30 00:50:58.80 dwhci: USB INTSTAT: 0x6000031

Sep 30 00:50:59.06 dwhci: USB INTSTAT: 0x6000031

Sep 30 00:51:05.47 dwhci: USB INTSTAT: 0x6000031
```

```bash
$ ping 192.168.10.110
PING 192.168.10.110 (192.168.10.110): 56 data bytes
64 bytes from 192.168.10.110: icmp_seq=0 ttl=64 time=0.870 ms
64 bytes from 192.168.10.110: icmp_seq=1 ttl=64 time=0.577 ms
...
^C
--- 192.168.10.110 ping statistics ---
15 packets transmitted, 15 packets received, 0.0% packet loss
round-trip min/avg/max/stddev = 0.494/0.678/0.870/0.104 ms
```

- GINTSTS.RxFLvlアサート時のGRXSTSRを出力

```bash
00:00:06.04 dhcp: IP address is 192.168.10.110
00:00:06.05 kernel: Try "ping 192.168.10.110" from another computer!
00:00:06.05 ntpd: Resolve start
00:00:07.05 ntpd: Resolve end
00:00:07.05 ntpd: GetTime start
00:00:07.06 ntp: send start
00:00:07.06 ntp: send end
00:00:08.06 ntp: recv start
00:00:08.06 ntp: recv end: result=48
00:00:08.07 ntpd: GetTime end
Oct  1 15:55:45.07 ntpd: System time updated
Oct  1 15:55:45.66 dwhci: IntStatus: 0x6000031      // [20:17] = 0011 : IN転送完了
Oct  1 15:55:45.66 dwhci: GRXSTRSR: 0x60000         // [16:15] = 00   : DATA 0

Oct  1 15:55:58.46 dwhci: IntStatus: 0x6000031      // [20:17] = 0010 : INパケット受信
Oct  1 15:55:58.46 dwhci: GRXSTRSR: 0x40000         // [16:15] = 00   : DATA 0

Oct  1 15:56:02.82 dwhci: IntStatus: 0x6000031      // [20:17] = 0010 : INパケット受信
Oct  1 15:56:02.82 dwhci: GRXSTRSR: 0x50000         // [16:15] = 10   : DATA 1

Oct  1 15:56:12.04 dwhci: IntStatus: 0x6000031      // [20:17] = 0011 : IN転送完了
Oct  1 15:56:12.04 dwhci: GRXSTRSR: 0x70000         // [16:15] = 10   : DATA 1
```

- ntpを5秒間隔で実行
- 結論: GINTSTS.RxFLvlがアサートしたタイミングで処理を開始することはできない

### 6. kthreadを作成し、ここでnet_handler()を実行する方法に変更

```bash
$ ifconfig net0 192.168.10.111 netmask 255.255.255.0
[3]socket_ioctl: register iface
[3]ip_route_add: route added: network=192.168.10.0, netmask=255.255.255.0, nexthop=0.0.0.0, iface=192.168.10.111 dev=net0
[3]ip_iface_register: registered: dev=net0, unicast=192.168.10.111, netmask=255.255.255.0, broadcast=192.168.10.255
$ ifconfig
net0: flags=83<UP|BROADCAST|NEEDARP> mtu 1500
  ether 40:54:0:12:34:57
  inet 192.168.10.111 netmask 255.255.255.0 broadcast 192.168.10.255
$
[1]dwhc_submit_block_request: failed bulk or interrupt xter
[1]usb_cdcether_receive_frame: failed submit block request
...
[0]dwhc_start_channel: host=0xffff000000a40ed8, channel=-763359352      // channel番号がマイナス
[1]usb_cdcether_receive_frame: kern/usb/dwhc_device.c:804: assertion failed.
QEMU: Terminated
```

### 7. udpechoを導入

```bash
$ ifconfig net0 192.168.10.111 netmask 255.255.255.0
[2]socket_alloc: bad domain: 2 or protocol: 0
$ udpecho
Starting UDP Echo Server
socket: success, soc=3
[1]udp_bind: bound, id=0, local=0.0.0.0:7
bind: success, self=0.0.0.0:7
waiting for message...

[0]dwhc_start_channel: host=0xffff000000a40ea8, channel=-763359352
[1]rekleeaser: n/usb/dwhc_device.c:802: assertion failed.

$ udpecho
Starting UDP Echo Server
socket: success, soc=3
[2]udp_bind: bound, id=1, local=0.0.0.0:7
bind: success, self=0.0.0.0:7
waiting for message...          // メッセージを受け付けない
```

```bash
$ nc -uv 192.168.10.111 7
Connection to 192.168.10.111 port 7 [udp/echo] succeeded!
abc         // エコーされない
```

- trace.log

```bash
usb_packet_state_change bus 0, port 1.1, ep 2, packet 0x7ff36f179518, state complete -> setup
usb_packet_state_change bus 0, port 1.1, ep 2, packet 0x7ff36f179518, state setup -> setup
usb_packet_state_change bus 0, port 1.1, ep 2, packet 0x7ff36f179518, state setup -> setup  // 以後ずっとこの行が続く
```

```bash
$ nc -uv 192.168.10.111 7
Connection to 192.168.10.111 port 7

$ arp -a
aterm.me (192.168.10.1) at f8:b7:97:87:2c:dc on en0 ifscope [ethernet]
? (192.168.10.101) at 90:8c:43:8c:c4:5a on en0 ifscope [ethernet]
? (192.168.10.111) at (incomplete) on en0 ifscope [ethernet]            // 解決されない
? (192.168.10.255) at ff:ff:ff:ff:ff:ff on en0 ifscope [ethernet]
mdns.mcast.net (224.0.0.251) at 1:0:5e:0:0:fb on en0 ifscope permanent [ethernet]
```

### 8. stdata, params, usbの各種構造体をslabから割り当てるように変更

- slabのバグが判明し、だいぶ手戻りした

```bash
[0]trap: unknown trap code: 37 at 0x15e8008
[0]exit: exit: pid 6, err 1
```

- データ例外
- trap出力を修正

```bash
[0]slab_cache_free: cache: stdat, obj: 0xffff00000108d080
[0]slab_cache_free: page: 0x0, heaer: 0xffff1d28015e8000
[0]slab_cache_free: list: 0xffff1d28015e8018
[0]trap: unknown trap code: 37 at 0xffff000000080764 with 0xffff1d28015e8008
[0]exit: exit: pid 6, err 1
```

- slab_cache_freeの問題で、対象データは0xffff1d28015e8008

```bash
[0]slab_cache_free: cache: stdat, obj: 0xffff00000108d080
[0]page_find_by_address: addr: 0xa2d080, START: 0xffff000000660000, index: 68719477709
[0]slab_cache_free: page: 0x0, heaer: 0xffff1d28015e8000
[0]slab_cache_free: list: 0xffff1d28015e8018
[0]trap: unknown trap code: 37 at 0xffff000000080764 with 0xffff1d28015e8008
[0]exit: exit: pid 4, err 1
```

- objの2重補正

```bash
[0]slab_cache_free: cache: stdat, obj: 0xffff00000108d080
[0]page_find_by_address: addr: 0xffff00000108d080, START: 0xffff000000660000, index: 2605
[0]slab_cache_free: page: 0xffff000000605fe8, heaer: 0xffff000000a5f000
[0]slab_cache_free: list: 0xffff000000a5f018
[0]trap: unknown trap code: 37 at 0xffff000000080778 with 0xffff000400000000    //
[0]exit: exit: pid 4, err 1
```

- slabを修正して修正前の状況（sh画面まで出る）になる

```bash
qemu-system-aarch64 -M raspi3b -nographic -serial null -serial mon:stdio -drive file=obj/sd.img,if=sd,format=raw -netdev user,id=net0,hostfwd=tcp::8080-:80 -device usb-net,netdev=net0 -trace events=events,file=trace.log -kernel obj/kernel8.img
[2]rand_init: rand_init ok
[0]main: cpu 0 init finished
[1]main: cpu 1 init finished
[3]main: cpu 3 init finished
[2]main: cpu 2 init finished
[0]mbox_set_sdhost_clock: unexpected tag resp 0x80000000, normal for qemu
[0]sdhost_set_ios: ios clock 400000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[0]sdhost_finish_command: error detected: CMD 0x4205, HSTS 0x40, EDM 0x10800
[0]sdhost_finish_command: command 5 timeout
[0]emmc_card_reset: OCR: 0xffff, 1.8v support: 0, SDHC support: 0
[0]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[0]emmc_card_reset: card CID: 0xaa585951, 0x454d5521, 0x1deadbe, 0xef006219
[0]emmc_card_reset: RCA: 0x4567
[3]emmc_card_reset: SCR: version 2.00, bus_widths 0x5
[3]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 1, timing 0, vdd 0, drv_type 0
[3]emmc_card_reset: found valid version 2.00 SD card
[3]sd_init: partition[0]: TYPE: 12, LBA = 0x800, #SECS = 0x20000
[3]sd_init: partition[1]: TYPE: 131, LBA = 0x20800, #SECS = 0x1f800
[3]sd_init: sd_init ok

[2]iinit: sb: size 1000 nblocks 963 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 36
[2]initlog: not use log
[3]usb_dev_init: Device ven409-55aa, dev9-0-0 found
[2]usb_dev_init: Product: QEMU QEMU USB Hub
[2]usb_function_get_if_name: func name=int9-0-0
[2]usb_dev_init: Interface int9-0-0 found
[2]usb_dev_factory_get_device: Using device/interface int9-0-0
[2]usb_dev_init: Device ven525-a4a2, dev2-0-0 found
[3]usb_dev_init: Product: QEMU RNDIS/QEMU USB Network Device
[3]usb_function_get_if_name: func name=int2-6-0
[3]usb_dev_init: Interface int2-6-0 found
[3]usb_dev_factory_get_device: Using device/interface int2-6-0
[3]usb_function_get_if_name: func name=inta-0-0
[3]usb_dev_init: Interface inta-0-0 found
[3]usb_dev_init: Function is not supported
[3]usb_function_get_if_name: func name=inta-0-0
[3]usb_dev_init: Interface inta-0-0 found
[3]usb_dev_init: Function is not supported
[3]usb_cdcether_configure: MAC address is 40:54:0:12:34:57
[3]usb_standard_hub_enumerate_ports: Port 1: Device configured
[2]dwhc_root_port_init: Device configured
[2]dwhc_init: dwhc_init ok: intmask: 0x2000010
[2]usb_init: dwhc initialized

[2]net_device_register: registered, dev=net0, type=0x0002
[2]usb_init: usb_init ok
[2]net_protocol_register: registered, type=0x0800 (IP)
[2]net_protocol_register: registered, type=0x0806 (ARP)
[2]ip_protocol_register: registered, type=1 (ICMP)
[2]ip_protocol_register: registered, type=17 (UDP)
[2]ip_protocol_register: registered, type=6 (TCP)
[2]netinit: initialized
[2]net_device_open: dev=net0, state=up
[2]netrun: running...
init: starting sh
sh: argv[0] = 'sh'
sh: testenv = 'FROM_INIT'
$ ifconfig net0 192.168.10.111 netmask 255.255.255.0
[3]socket_ioctl: register iface
[3]ip_route_add: route added: network=192.168.10.0, netmask=255.255.255.0, nexthop=0.0.0.0, iface=192.168.10.111 dev=net0
[3]ip_iface_register: registered: dev=net0, unicast=192.168.10.111, netmask=255.255.255.0, broadcast=192.168.10.255
$ ifconfig
net0: flags=83<UP|BROADCAST|NEEDARP> mtu 1500
  ether 40:54:0:12:34:57
  inet 192.168.10.111 netmask 255.255.255.0 broadcast 192.168.10.255
$ udpecho
Starting UDP Echo Server
socket: success, soc=3
[3]udp_bind: bound, id=3, local=0.0.0.0:7
bind: success, self=0.0.0.0:7
waiting for message...
[1]dwhc_xfer_data_get_ep_type: bad ep_type: -1440807965
```

### 9. scheduler構造体を修正

- 実機で受信されるようになる
- 送信が失敗する

```bash
[2]set_ip_config: dev: 0xffff000000be02b0, iface: 0xffff000000be00c0, unicast: 192.168.10.255, netmask: 192.168.10.255, broadcast: 192.168.10.255
[2]net_device_add_iface: dev->ifaces: 0x0
[2]ip_route_add: route added: network=192.168.10.0, netmask=255.255.255.0, nexthop=0.0.0.0, iface=192.168.10.110 dev=net0
[2]ip_iface_register: registered: dev=net0, unicast=192.168.10.110, netmask=255.255.255.0, broadcast=192.168.10.255
[2]net_device_open: dev=net0, state=up
[2]netrun: running...
init: starting sh
sh: argv[0] = 'sh'
sh: testenv = 'FROM_INIT'
$ [2]lan7800_receive_frame: Frame received (status 0x2000040)
[2]ether_input_helper: dev=net0, type=0x0806, len=60
        src: ac:87:a3:19:bc:d8
        dst: ff:ff:ff:ff:ff:ff
       type: 0x0806
+------+-------------------------------------------------+------------------+
| 0000 | ff ff ff ff ff ff ac 87 a3 19 bc d8 08 06 00 01 | ................ |
| 0010 | 08 00 06 04 00 01 ac 87 a3 19 bc d8 c0 a8 0a 67 | ...............g |
| 0020 | 00 00 00 00 00 00 c0 a8 0a 6e 00 00 00 00 00 00 | .........n...... |
| 0030 | 00 00 00 00 00 00 00 00 00 00 00 00             | ............     |
+------+-------------------------------------------------+------------------+
[2]net_input_handler: queue pushed (num:1), dev=net0, type=0x0806, len=46
+------+-------------------------------------------------+------------------+
| 0000 | 00 01 08 00 06 04 00 01 ac 87 a3 19 bc d8 c0 a8 | ................ |
| 0010 | 0a 67 00 00 00 00 00 00 c0 a8 0a 6e 00 00 00 00 | .g.........n.... |
| 0020 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00       | ..............   |
+------+-------------------------------------------------+------------------+
[1]softintr: irqs: 0x2
[1]net_softirq_handler: queue popped (num:0), dev=net0, type=0x0806, len=46
+------+-------------------------------------------------+------------------+
| 0000 | 00 01 08 00 06 04 00 01 ac 87 a3 19 bc d8 c0 a8 | ................ |
| 0010 | 0a 67 00 00 00 00 00 00 c0 a8 0a 6e 00 00 00 00 | .g.........n.... |
| 0020 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00       | ..............   |
+------+-------------------------------------------------+------------------+
[1]arp_input: data: 0xffff000000bc79d0, size: 46, device: net0
[1]arp_input: dev=net0, len=46
        hrd: 0x0001
        pro: 0x0800
        hln: 6
        pln: 4
         op: 1 (Request)
        sha: ac:87:a3:19:bc:d8
        spa: 192.168.10.103
        tha: 00:00:00:00:00:00
        tpa: 192.168.10.110
arp_dump
+------+-------------------------------------------------+------------------+
| 0000 | 00 01 08 00 06 04 00 01 ac 87 a3 19 bc d8 c0 a8 | ................ |
| 0010 | 0a 67 00 00 00 00 00 00 c0 a8 0a 6e 00 00 00 00 | .g.........n.... |
| 0020 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00       | ..............   |
+------+-------------------------------------------------+------------------+
[1]arp_cache_insert: INSERT: pa=192.168.10.103, ha=ac:87:a3:19:bc:d8
[1]arp_reply: dev=net0, len=28
        hrd: 0x0001
        pro: 0x0800
        hln: 6
        pln: 4
         op: 2 (Reply)
        sha: b8:27:eb:ab:e8:48
        spa: 192.168.10.110
        tha: ac:87:a3:19:bc:d8
        tpa: 192.168.10.103
arp_dump
+------+-------------------------------------------------+------------------+
| 0000 | 00 01 08 00 06 04 00 02 b8 27 eb ab e8 48 c0 a8 | .........'...H.. |
| 0010 | 0a 6e ac 87 a3 19 bc d8 c0 a8 0a 67             | .n.........g     |
+------+-------------------------------------------------+------------------+
[1]net_device_output: dev=net0, type=0x0806, len=28
+------+-- net_device_output ----------------------------+------------------+
+------+-------------------------------------------------+------------------+
| 0000 | 00 01 08 00 06 04 00 02 b8 27 eb ab e8 48 c0 a8 | .........'...H.. |
| 0010 | 0a 6e ac 87 a3 19 bc d8 c0 a8 0a 67             | .n.........g     |
+------+-------------------------------------------------+------------------+
[1]ether_transmit_helper: dev=net0, type=0x0806, len=60
        src: b8:27:eb:ab:e8:48
        dst: ac:87:a3:19:bc:d8
       type: 0x0806
+------+-------------------------------------------------+------------------+
| 0000 | ac 87 a3 19 bc d8 b8 27 eb ab e8 48 08 06 00 01 | .......'...H.... |
| 0010 | 08 00 06 04 00 02 b8 27 eb ab e8 48 c0 a8 0a 6e | .......'...H...n |
| 0020 | ac 87 a3 19 bc d8 c0 a8 0a 67 00 00 00 00 00 00 | .........g...... |
| 0030 | 00 00 00 00 00 00 00 00 00 00 00 00             | ............     |
+------+-------------------------------------------------+------------------+
[1]net_device_output: device transmit failure, dev=net0, len=28
[3]lan7800_receive_frame: Frame received (status 0x2000040)
[3]ether_input_helper: dev=net0, type=0x0806, len=60
        src: ac:87:a3:19:bc:d8
        dst: ff:ff:ff:ff:ff:ff
       type: 0x0806
+------+-------------------------------------------------+------------------+
| 0000 | ff ff ff ff ff ff ac 87 a3 19 bc d8 08 06 00 01 | ................ |
| 0010 | 08 00 06 04 00 01 ac 87 a3 19 bc d8 c0 a8 0a 67 | ...............g |
| 0020 | 00 00 00 00 00 00 c0 a8 0a 6e 00 00 00 00 00 00 | .........n...... |
| 0030 | 00 00 00 00 00 00 00 00 00 00 00 00             | ............     |
+------+-------------------------------------------------+------------------+
[3]net_input_handler: queue pushed (num:1), dev=net0, type=0x0806, len=46
+------+-------------------------------------------------+------------------+
| 0000 | 00 01 08 00 06 04 00 01 ac 87 a3 19 bc d8 c0 a8 | ................ |
| 0010 | 0a 67 00 00 00 00 00 00 c0 a8 0a 6e 00 00 00 00 | .g.........n.... |
| 0020 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00       | ..............   |
+------+-------------------------------------------------+------------------+
[1]softintr: irqs: 0x2
[1]net_softirq_handler: queue popped (num:0), dev=net0, type=0x0806, len=46
+------+-------------------------------------------------+------------------+
| 0000 | 00 01 08 00 06 04 00 01 ac 87 a3 19 bc d8 c0 a8 | ................ |
| 0010 | 0a 67 00 00 00 00 00 00 c0 a8 0a 6e 00 00 00 00 | .g.........n.... |
| 0020 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00       | ..............   |
+------+-------------------------------------------------+------------------+
[1]arp_input: data: 0xffff000000ba13d0, size: 46, device: net0
[1]arp_input: dev=net0, len=46
        hrd: 0x0001
        pro: 0x0800
        hln: 6
        pln: 4
         op: 1 (Request)
        sha: ac:87:a3:19:bc:d8
        spa: 192.168.10.103
        tha: 00:00:00:00:00:00
        tpa: 192.168.10.110
arm_dump
+------+-------------------------------------------------+------------------+
| 0000 | 00 01 08 00 06 04 00 01 ac 87 a3 19 bc d8 c0 a8 | ................ |
| 0010 | 0a 67 00 00 00 00 00 00 c0 a8 0a 6e 00 00 00 00 | .g.........n.... |
| 0020 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00       | ..............   |
+------+-------------------------------------------------+------------------+
[1]arp_cache_update: UPDATE: pa=192.168.10.103, ha=ac:87:a3:19:bc:d8
[1]arp_reply: dev=net0, len=28
        hrd: 0x0001
        pro: 0x0800
        hln: 6
        pln: 4
         op: 2 (Reply)
        sha: b8:27:eb:ab:e8:48
        spa: 192.168.10.110
        tha: ac:87:a3:19:bc:d8
        tpa: 192.168.10.103
arm_dump
+------+-------------------------------------------------+------------------+
| 0000 | 00 01 08 00 06 04 00 02 b8 27 eb ab e8 48 c0 a8 | .........'...H.. |
| 0010 | 0a 6e ac 87 a3 19 bc d8 c0 a8 0a 67             | .n.........g     |
+------+-------------------------------------------------+------------------+
[1]net_device_output: dev=net0, type=0x0806, len=28
+------+-- net_device_output ----------------------------+------------------+
+------+-------------------------------------------------+------------------+
| 0000 | 00 01 08 00 06 04 00 02 b8 27 eb ab e8 48 c0 a8 | .........'...H.. |
| 0010 | 0a 6e ac 87 a3 19 bc d8 c0 a8 0a 67             | .n.........g     |
+------+-------------------------------------------------+------------------+
[1]ether_transmit_helper: dev=net0, type=0x0806, len=60       // 有効バイトは42 + (4) : 残りの14バイトはpadding
        src: b8:27:eb:ab:e8:48                                // 42バイトまではEthernet + Arp replyで正しい
        dst: ac:87:a3:19:bc:d8                                // Ethernetパケットの最小サイズは60バイト
       type: 0x0806
+------+-------------------------------------------------+------------------+
| 0000 | ac 87 a3 19 bc d8 b8 27 eb ab e8 48 08 06 00 01 | .......'...H.... |
| 0010 | 08 00 06 04 00 02 b8 27 eb ab e8 48 c0 a8 0a 6e | .......'...H...n |
| 0020 | ac 87 a3 19 bc d8 c0 a8 0a 67 00 00 00 00 00 00 | .........g...... |
| 0030 | 00 00 00 00 00 00 00 00 00 00 00 00             | ............     |
+------+-------------------------------------------------+------------------+
[0]dwhc_channel_intr_hdl: Transaction failed 1 (status 0xa)   // 0xa はデータトグル例外（これは関係なかった。ゴミ?）
[1]net_device_output: device transmit failure, dev=net0, len=28
```

### 10. lan7800_send_frame()が呼ばれていないことが判明

- lan7800の登録名を間違えてcdcetherと同名で登録していたため、cdcetherのsend_frame()関数が呼ばれていた
- 修正することで送信するようになる

```bash
[2]net_device_output: transmit: 0xffff0000000958e0
[2]ether_transmit_helper: called with callback: 0xffff0000000958f0      // cdcetherを呼んでる
[2]ether_transmit_helper: dev=net0, type=0x0806, len=60
        src: b8:27:eb:ab:e8:48
        dst: ac:87:a3:19:bc:d8
       type: 0x0806
+------+-------------------------------------------------+------------------+
| 0000 | ac 87 a3 19 bc d8 b8 27 eb ab e8 48 08 06 00 01 | .......'...H.... |
| 0010 | 08 00 06 04 00 02 b8 27 eb ab e8 48 c0 a8 0a 6e | .......'...H...n |
| 0020 | ac 87 a3 19 bc d8 c0 a8 0a 67 00 00 00 00 00 00 | .........g...... |
| 0030 | 00 00 00 00 00 00 00 00 00 00 00 00             | ............     |
+------+-------------------------------------------------+------------------+
[0]dwhc_channel_intr_hdl: Transaction failed 1 (status 0xa)
[2]net_device_output: device transmit failure, dev=net0, len=28


ffff0000000958e0 <usb_cdcether_net_transmit>:
ffff0000000958f0 <usb_cdcether_send_frame>:
ffff00000009cfc0 <lan7800_send_frame>:
```

```diff
diff --git a/kern/usb/lan7800.c b/kern/usb/lan7800.c
@@ -207,7 +206,7 @@ boolean lan7800_configure(usb_function_t *super)
     }

     // USBデバイスとして登録
-    usb_device_ns_add_dev(usb_device_ns_get(), "eth01", self, false);      // cdcetherの名前で登録してた
+    usb_device_ns_add_dev(usb_device_ns_get(), "eth00", self, false);
```

```bash
[1]lan7800_receive_frame: Frame received (status 0x2000040)
[1]ether_input_helper: dev=net0, type=0x0806, len=60
        src: ac:87:a3:19:bc:d8
        dst: ff:ff:ff:ff:ff:ff
       type: 0x0806
+------+-------------------------------------------------+------------------+
| 0000 | ff ff ff ff ff ff ac 87 a3 19 bc d8 08 06 00 01 | ................ |
| 0010 | 08 00 06 04 00 01 ac 87 a3 19 bc d8 c0 a8 0a 67 | ...............g |
| 0020 | 00 00 00 00 00 00 c0 a8 0a 6e 00 00 00 00 00 00 | .........n...... |
| 0030 | 00 00 00 00 00 00 00 00 00 00 00 00             | ............     |
+------+-------------------------------------------------+------------------+
[1]net_input_handler: queue pushed (num:1), dev=net0, type=0x0806, len=46
+------+-------------------------------------------------+------------------+
| 0000 | 00 01 08 00 06 04 00 01 ac 87 a3 19 bc d8 c0 a8 | ................ |
| 0010 | 0a 67 00 00 00 00 00 00 c0 a8 0a 6e 00 00 00 00 | .g.........n.... |
| 0020 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00       | ..............   |
+------+-------------------------------------------------+------------------+
[3]softintr: irqs: 0x2
[3]net_softirq_handler: queue popped (num:0), dev=net0, type=0x0806, len=46
+------+-------------------------------------------------+------------------+
| 0000 | 00 01 08 00 06 04 00 01 ac 87 a3 19 bc d8 c0 a8 | ................ |
| 0010 | 0a 67 00 00 00 00 00 00 c0 a8 0a 6e 00 00 00 00 | .g.........n.... |
| 0020 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00       | ..............   |
+------+-------------------------------------------------+------------------+
[3]arp_input: data: 0xffff000000bc79d0, size: 46, device: net0
[3]arp_input: dev=net0, len=46
        hrd: 0x0001
        pro: 0x0800
        hln: 6
        pln: 4
         op: 1 (Request)
        sha: ac:87:a3:19:bc:d8
        spa: 192.168.10.103
        tha: 00:00:00:00:00:00
        tpa: 192.168.10.110
arm_dump
+------+-------------------------------------------------+------------------+
| 0000 | 00 01 08 00 06 04 00 01 ac 87 a3 19 bc d8 c0 a8 | ................ |
| 0010 | 0a 67 00 00 00 00 00 00 c0 a8 0a 6e 00 00 00 00 | .g.........n.... |
| 0020 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00       | ..............   |
+------+-------------------------------------------------+------------------+
[3]arp_cache_insert: INSERT: pa=192.168.10.103, ha=ac:87:a3:19:bc:d8
[3]arp_reply: dev=net0, len=28
        hrd: 0x0001
        pro: 0x0800
        hln: 6
        pln: 4
         op: 2 (Reply)
        sha: b8:27:eb:ab:e8:48
        spa: 192.168.10.110
        tha: ac:87:a3:19:bc:d8
        tpa: 192.168.10.103
arm_dump
+------+-------------------------------------------------+------------------+
| 0000 | 00 01 08 00 06 04 00 02 b8 27 eb ab e8 48 c0 a8 | .........'...H.. |
| 0010 | 0a 6e ac 87 a3 19 bc d8 c0 a8 0a 67             | .n.........g     |
+------+-------------------------------------------------+------------------+
[3]net_device_output: dev=net0, type=0x0806, len=28
+------+-- net_device_output ----------------------------+------------------+
+------+-------------------------------------------------+------------------+
| 0000 | 00 01 08 00 06 04 00 02 b8 27 eb ab e8 48 c0 a8 | .........'...H.. |
| 0010 | 0a 6e ac 87 a3 19 bc d8 c0 a8 0a 67             | .n.........g     |
+------+-------------------------------------------------+------------------+
[3]net_device_output: transmit: 0xffff00000009cc60                      // lan7800_net_transmit
[3]lan7800_net_transmit: called
[3]ether_transmit_helper: called with callback: 0xffff00000009d010      // lan7800_send_frame
[3]ether_transmit_helper: dev=net0, type=0x0806, len=60
        src: b8:27:eb:ab:e8:48
        dst: ac:87:a3:19:bc:d8
       type: 0x0806
+------+-------------------------------------------------+------------------+
| 0000 | ac 87 a3 19 bc d8 b8 27 eb ab e8 48 08 06 00 01 | .......'...H.... |
| 0010 | 08 00 06 04 00 02 b8 27 eb ab e8 48 c0 a8 0a 6e | .......'...H...n |
| 0020 | ac 87 a3 19 bc d8 c0 a8 0a 67 00 00 00 00 00 00 | .........g...... |
| 0030 | 00 00 00 00 00 00 00 00 00 00 00 00             | ............     |
+------+-------------------------------------------------+------------------+
[3]lan7800_send_frame: called: buf: 0xffff000000ba1a10, size: 60
[3]lan7800_send_frame: dwhr_xfer result: 0                              // 転送成功

[1]lan7800_receive_frame: Frame received (status 0x1c008066)            // icmpの受信開始
[3]net_device_output: device transmit failure, dev=net0, len=28         // ?
[1]ether_input_helper: dev=net0, type=0x0800, len=98
        src: ac:87:a3:19:bc:d8
        dst: b8:27:eb:ab:e8:48
       type: 0x0800
+------+-------------------------------------------------+------------------+
| 0000 | b8 27 eb ab e8 48 ac 87 a3 19 bc d8 08 00 45 00 | .'...H........E. |
| 0010 | 00 54 ae e8 00 00 40 01 35 9b c0 a8 0a 67 c0 a8 | .T....@.5....g.. |
| 0020 | 0a 6e 08 00 3b 3f 3f 7b 00 00 68 e9 df 13 00 09 | .n..;??{..h..... |
| 0030 | 4a 3c 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 | J<.............. |
| 0040 | 16 17 18 19 1a 1b 1c 1d 1e 1f 20 21 22 23 24 25 | .......... !"#$% |
| 0050 | 26 27 28 29 2a 2b 2c 2d 2e 2f 30 31 32 33 34 35 | &'()*+,-./012345 |
| 0060 | 36 37                                           | 67               |
+------+-------------------------------------------------+------------------+
[1]net_input_handler: queue pushed (num:1), dev=net0, type=0x0800, len=84
+------+-------------------------------------------------+------------------+
| 0000 | 45 00 00 54 ae e8 00 00 40 01 35 9b c0 a8 0a 67 | E..T....@.5....g |
| 0010 | c0 a8 0a 6e 08 00 3b 3f 3f 7b 00 00 68 e9 df 13 | ...n..;??{..h... |
| 0020 | 00 09 4a 3c 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 | ..J<............ |
| 0030 | 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f 20 21 22 23 | ............ !"# |
| 0040 | 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f 30 31 32 33 | $%&'()*+,-./0123 |
| 0050 | 34 35 36 37                                     | 4567             |
+------+-------------------------------------------------+------------------+
[2]softintr: irqs: 0x2
[2]net_softirq_handler: queue popped (num:0), dev=net0, type=0x0800, len=84
+------+-------------------------------------------------+------------------+
| 0000 | 45 00 00 54 ae e8 00 00 40 01 35 9b c0 a8 0a 67 | E..T....@.5....g |
| 0010 | c0 a8 0a 6e 08 00 3b 3f 3f 7b 00 00 68 e9 df 13 | ...n..;??{..h... |
| 0020 | 00 09 4a 3c 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 | ..J<............ |
| 0030 | 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f 20 21 22 23 | ............ !"# |
| 0040 | 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f 30 31 32 33 | $%&'()*+,-./0123 |
| 0050 | 34 35 36 37                                     | 4567             |
+------+-------------------------------------------------+------------------+
[2]ip_input: dev=net0, iface=192.168.10.110, protocol=1, total=84
        vhl: 0x45 [v: 4, hl: 5 (20)]
        tos: 0x00
      total: 84 (payload: 64)
         id: 44776
     offset: 0x0000 [flags=0, offset=0]
        ttl: 64
   protocol: 1
        sum: 0x359b
        src: 192.168.10.103
        dst: 192.168.10.110
[2]icmp_input: 192.168.10.103 => 192.168.10.110, len=64
       type: 8 (Echo)
       code: 0
        sum: 0x3b3f
         id: 16251
        seq: 0
[2]icmp_output: 192.168.10.110 => 192.168.10.103, len=64
       type: 0 (EchoReply)
       code: 0
        sum: 0x433f
         id: 16251
        seq: 0
[2]ip_output_core: dev=net0, dst=192.168.10.103, protocol=1, len=84
        vhl: 0x45 [v: 4, hl: 5 (20)]
        tos: 0x00
      total: 84 (payload: 64)
         id: 128
     offset: 0x0000 [flags=0, offset=0]
        ttl: 255
   protocol: 1
        sum: 0x2503
        src: 192.168.10.110
        dst: 192.168.10.103
[2]arp_resolve: resolved, pa=192.168.10.103, ha=ac:87:a3:19:bc:d8               // arp解決
[2]net_device_output: dev=net0, type=0x0800, len=84
+------+-- net_device_output ----------------------------+------------------+
+------+-------------------------------------------------+------------------+
| 0000 | 45 00 00 54 00 80 00 00 ff 01 25 03 c0 a8 0a 6e | E..T......%....n |
| 0010 | c0 a8 0a 67 00 00 43 3f 3f 7b 00 00 68 e9 df 13 | ...g..C??{..h... |
| 0020 | 00 09 4a 3c 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 | ..J<............ |
| 0030 | 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f 20 21 22 23 | ............ !"# |
| 0040 | 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f 30 31 32 33 | $%&'()*+,-./0123 |
| 0050 | 34 35 36 37                                     | 4567             |
+------+-------------------------------------------------+------------------+
[2]net_device_output: transmit: 0xffff00000009cc60
[2]lan7800_net_transmit: called
[2]ether_transmit_helper: called with callback: 0xffff00000009d010
[2]ether_transmit_helper: dev=net0, type=0x0800, len=98
        src: b8:27:eb:ab:e8:48
        dst: ac:87:a3:19:bc:d8
       type: 0x0800
+------+-------------------------------------------------+------------------+
| 0000 | ac 87 a3 19 bc d8 b8 27 eb ab e8 48 08 00 45 00 | .......'...H..E. |
| 0010 | 00 54 00 80 00 00 ff 01 25 03 c0 a8 0a 6e c0 a8 | .T......%....n.. |
| 0020 | 0a 67 00 00 43 3f 3f 7b 00 00 68 e9 df 13 00 09 | .g..C??{..h..... |
| 0030 | 4a 3c 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 | J<.............. |
| 0040 | 16 17 18 19 1a 1b 1c 1d 1e 1f 20 21 22 23 24 25 | .......... !"#$% |
| 0050 | 26 27 28 29 2a 2b 2c 2d 2e 2f 30 31 32 33 34 35 | &'()*+,-./012345 |
| 0060 | 36 37                                           | 67               |
+------+-------------------------------------------------+------------------+
[2]lan7800_send_frame: called: buf: 0xffff000000b82430, size: 98
[2]lan7800_send_frame: dwhr_xfer result: 0                                  // 転送は成功だが
[2]net_device_output: device transmit failure, dev=net0, len=84             // result: 0 !~ len: 84 なのでfailure
[2]ip_output: ip_output_core() failure                                      // networkドライバの違いによると思われる
[1]lan7800_receive_frame: Frame received (status 0x1c008066)                // (判定を変更する)
[1]ether_input_helper: dev=net0, type=0x0800, len=98
        src: ac:87:a3:19:bc:d8
        dst: b8:27:eb:ab:e8:48
       type: 0x0800
```

- pingが成功
- arpの解決も成功

```bash
$ ping 192.168.10.110
PING 192.168.10.110 (192.168.10.110): 56 data bytes
64 bytes from 192.168.10.110: icmp_seq=0 ttl=255 time=856.959 ms
64 bytes from 192.168.10.110: icmp_seq=1 ttl=255 time=458.185 ms
64 bytes from 192.168.10.110: icmp_seq=2 ttl=255 time=458.171 ms
64 bytes from 192.168.10.110: icmp_seq=3 ttl=255 time=458.275 ms
^C
--- 192.168.10.110 ping statistics ---
4 packets transmitted, 4 packets received, 0.0% packet loss                         // ping 成功
round-trip min/avg/max/stddev = 458.171/557.898/856.959/172.663 ms

$ arp -a
aterm.me (192.168.10.1) at f8:b7:97:87:2c:dc on en0 ifscope [ethernet]
? (192.168.10.101) at 90:8c:43:8c:c4:5a on en0 ifscope [ethernet]
? (192.168.10.110) at b8:27:eb:ab:e8:48 on en0 ifscope [ethernet]                   // arp解決されている
mdns.mcast.net (224.0.0.251) at 1:0:5e:0:0:fb on en0 ifscope permanent [ethernet]
```

### 11. 時々スイッチオンで次のエラーが無限出力（未解決）


```bash
[0]dwhc_channel_intr_hdl: Transaction failed 1 (status 0x421)
[0]dwhc_channel_intr_hdl: Transaction failed 1 (status 0x423)   // これがほとんど
[0]dwhc_channel_intr_hdl: Transaction failed 1 (status 0x400)
```

## udpecho

```bash
$ udpecho
Starting UDP Echo Server
socket: success, soc=3
bind: success, self=0.0.0.0:7
waiting for message...

recvfrom: 1 bytes data received, peer=192.168.10.103:56513
58
        src: 56513
        dst: 7
        len: 9
        sum: 0x34ed
+------+-------------------------------------------------+------------------+
| 0000 | dc c1 00 07 00 09 34 ed 58                      | ......4.X        |
+------+-------------------------------------------------+------------------+

recvfrom: 1 bytes data received, peer=192.168.10.103:56513
58
[2]udp_output: 192.168.10.110:7 => 192.168.10.103:56513, len=9 (payload=1)
        src: 7
        dst: 56513
        len: 9
        sum: 0x34ed
+------+-------------------------------------------------+------------------+
| 0000 | 00 07 dc c1 00 09 34 ed 58                      | ......4.X        |
+------+-------------------------------------------------+------------------+
recvfrom: 1 bytes data received, peer=192.168.10.103:56513
58
[1]udp_output: 192.168.10.110:7 => 192.168.10.103:56513, len=9 (payload=1)
        src: 7
        dst: 56513
        len: 9
        sum: 0x34ed
+------+-------------------------------------------------+------------------+
| 0000 | 00 07 dc c1 00 09 34 ed 58                      | ......4.X        |
+------+-------------------------------------------------+------------------+
recvfrom: 1 bytes data received, peer=192.168.10.103:56513
58
[1]udp_output: 192.168.10.110:7 => 192.168.10.103:56513, len=9 (payload=1)
        src: 7
        dst: 56513
        len: 9
        sum: 0x34ed
+------+-------------------------------------------------+------------------+
| 0000 | 00 07 dc c1 00 09 34 ed 58                      | ......4.X        |
+------+-------------------------------------------------+------------------+
        src: 56513
        dst: 7
        len: 12
        sum: 0xc87a
+------+-------------------------------------------------+------------------+
| 0000 | dc c1 00 07 00 0c c8 7a 61 62 63 0a             | .......zabc.     |
+------+-------------------------------------------------+------------------+
recvfrom: 4 bytes data received, peer=192.168.10.103:56513
6162630a
[3]udp_output: 192.168.10.110:7 => 192.168.10.103:56513, len=12 (payload=4)
        src: 7
        dst: 56513
        len: 12
        sum: 0xc87a
+------+-------------------------------------------------+------------------+
| 0000 | 00 07 dc c1 00 0c c8 7a 61 62 63 0a             | .......zabc.     |
+------+-------------------------------------------------+------------------+
        src: 56513
        dst: 7
        len: 10
        sum: 0x5ee1
+------+-------------------------------------------------+------------------+
| 0000 | dc c1 00 07 00 0a 5e e1 2e 0a                   | ......^...       |
+------+-------------------------------------------------+------------------+
quit
$
```

- macから接続
- 接続時に3回'X'を送信している（コマンドの仕様か?）

```bash
$ nc -vvv -u 192.168.10.110 7
Connection to 192.168.10.110 port 7 [udp/echo] succeeded!
XXXabc      // abcを入力
abc         // abcがエコー
.           // ピリオド+改行で終了
^C
```

## DNSクライアントを作成

```bash
$ dns
[2]dns_resolve: resolve www.google.com
=== DNS Packet ===
        id: 1
      flag: 0x0001
   qdcount: 1
   ancount: 0
   nscount: 0
   arcount: 0
      dist: 03 77 77 77 06 67 6f 6f 67 6c 65 03 63 6f 6d 00
     qtype: 1
    qclass: 1
=== DNS dump ===
+------+-------------------------------------------------+------------------+
| 0000 | 00 01 01 00 00 01 00 00 00 00 00 00 03 77 77 77 | .............www |
| 0010 | 06 67 6f 6f 67 6c 65 03 63 6f 6d 00 00 01 00 01 | .google.com..... |
+------+-------------------------------------------------+------------------+

=== DNS Packet ===
        id: 1
      flag: 0x8081
   qdcount: 1
   ancount: 1
   nscount: 0
   arcount: 0
      dist: 03 77 77 77 06 67 6f 6f 67 6c 65 03 63 6f 6d 00
     qtype: 1
    qclass: 1
      name: 0x0cc0
      type: 1
     class: 1
       ttl: 284
       len: 4
        ip: 142.250.196.100
=== DNS dump ===
+------+-------------------------------------------------+------------------+
| 0000 | 00 01 81 80 00 01 00 01 00 00 00 00 03 77 77 77 | .............www |
| 0010 | 06 67 6f 6f 67 6c 65 03 63 6f 6d 00 00 01 00 01 | .google.com..... |
| 0020 | c0 0c 00 01 00 01 00 00 01 1c 00 04 8e fa c4 64 | ...............d |
+------+-------------------------------------------------+------------------+

$
```

## NTPクライアントを作成

```bash
$ dns
=== ntp_q dump ===
+------+-------------------------------------------------+------------------+
| 0000 | 23 00 0a 00 00 00 00 00 00 00 00 00 00 00 00 00 | #............... |
| 0010 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
| 0020 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................ |
+------+-------------------------------------------------+------------------+

=== ntp dump ===
+------+-------------------------------------------------+------------------+
| 0000 | 24 02 0a e7 00 00 0c 16 00 00 00 7e 85 f3 ec 11 | $..........~.... |
| 0010 | ec 9c 8a 61 db 80 66 93 00 00 00 00 00 00 00 00 | ...a..f......... |
| 0020 | ec 9c 90 c9 4c fc 52 53 ec 9c 90 c9 4c fd 12 36 | ....L.RS....L..6 |
+------+-------------------------------------------------+------------------+

[3]ntp_get_time: timestamp: 0x4cfc5253, utc: 1760694857

```

## `/bin/date`で現在時刻が表示されない (2026/01/25に判明)

```bash
$ /bin/date
2022年 6日21日 火曜日 10時31分47秒 JST
$ /bin/dns
[3]ip_output_device: arp not resolved
```

- idleプロセスのテストで`config.h`で`USING_RASPI`をundefしていたためだった
- defineして`make clean; make`で復活

```bash
$ /bin/date
2026年 1日25日 日曜日 11時 2分35秒 JST
$ /bin/dns
[0]ntp_get_time: timestamp: 0xcf17027, utc: 1769306558
```
