# Network (Ethernet) 機能を導入

- [circleのnetサブシステムの調査](circle_net.md)
- [xv6-netのnetサブシステムを調査](xv6_net.md)

## USB割り込みの調査

- usb_init()におけるUSB割り込み
    - GINTSTS = 0x7000029 = 0b0000_0111_0000_0000_0000_0000_0010_1001
        - PTxFEmp (26) | HChInt (25) | PrtInt (24) | RxFLvl (5) | Sof (3) | CurMod (1)
- RxFLvlを有効にして、受信パケット到着を知ることができるか？

```bash
// trace_0919.log
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000021
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[2]usb_dev_init: Device ven409-55aa, dev9-0-0 found
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[3]usb_dev_init: Product: QEMU QEMU USB Hub
[3]usb_function_get_if_name: func name=int9-0-0
[3]usb_dev_init: Interface int9-0-0 found
[3]usb_dev_factory_get_device: Using device/interface int9-0-0
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[2]dwhc_intr_hdl: [2](5) USB int: 0x7000029
[3]dwhc_intr_hdl: [3](3) USB int: 0x7000029
[2]dwhc_intr_hdl: [2](5)   haint: 0x1
[3]dwhc_intr_hdl: [3](3)   haint: 0x1
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000021
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000021
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000021
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[1]usb_dev_init: Device ven525-a4a2, dev2-0-0 found
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[3]dwhc_intr_hdl: [3](3) USB int: 0x7000029
[2]dwhc_intr_hdl: [2](5) USB int: 0x7000029
[3]dwhc_intr_hdl: [3](3)   haint: 0x1
[2]dwhc_intr_hdl: [2](5)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000021
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[1]dwhc_intr_hdl: [1](2) USB int: 0x7000029
[1]dwhc_intr_hdl: [1](2)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x0
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
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[1]usb_cdcether_configure: MAC address is 40:54:0:12:34:57
[1]usb_standard_hub_enumerate_ports: Port 1: Device configured
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000021
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000021
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[0]dwhc_intr_hdl: [0](4) USB int: 0x7000029
[0]dwhc_intr_hdl: [0](4)   haint: 0x1
[3]dwhc_root_port_init: Device configured
[3]usb_init: dwhc initialized

[3]usb_init: usb_init ok
init: starting sh
sh: argv[0] = 'sh'
sh: testenv = 'FROM_INIT'
$
```
