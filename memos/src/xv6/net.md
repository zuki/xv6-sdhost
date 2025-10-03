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

## xv6-riscv-netのネットワーク機能を導入

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

***修正前***

```bash
qemu-system-aarch64 -M raspi3b -nographic -serial null -serial mon:stdio -drive file=obj/sd.img,if=sd,format=raw -kernel obj/kernel8.img
[1]main: console ok
[1]mm_init: mm_init called
[1]buddy_init: start: 0xffff000000660000, end: 0xffff0000001bc8b8
[1]buddy_init: 1
[1]buddy_init: 2
[1]buddy_init: pages: ffff0000000c9600
[1]buddy_init: pages: 0, size: 0x596a00, PAGE_START: 0xffff000000660000
[1]buddy_init: sizeof(struct page): 0x18
```

***修正後***

```bash
qemu-system-aarch64 -M raspi3b -nographic -serial null -serial mon:stdio -drive file=obj/sd.img,if=sd,format=raw -kernel obj/kernel8.img
[1]main: console ok
[1]buddy_init: start: 0xffff000000660000, end: 0xffff0000001bc8b8
[1]buddy_init: pages: ffff000000600000
[1]buddy_init: pages: ffff000000600000, size: 0x60000, PAGE_START: 0xffff000000660000
[1]buddy_init: sizeof(struct page): 0x18
[1]buddy_init: free_lists[10].head: ffff000000600000
[1]mm_init: buddy ok
[1]mm_init: slab ok
[1]rand_init: rand_init ok
[1]main: user ok
[1]main: block ok
[1]timer_init: timerfreq = 0x3b9aca0
[0]timer_init: timerfreq = 0x3b9aca0
[3]timer_init: timerfreq = 0x3b9aca0
[2]timer_init: timerfreq = 0x3b9aca0
[0]main: cpu 0 init finished
[1]main: cpu 1 init finished
[2]main: cpu 2 init finished
[3]main: cpu 3 init finished
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

[1]iinit: sb: size 1000 nblocks 963 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 36
[1]initlog: not use log
init: starting sh
sh: argv[0] = 'sh'
sh: testenv = 'FROM_INIT'
$
```

## usb_init(), net_init()を追加

- usbの初期化ができない
- dwhc_xfer_stageで非同期転送を開始し、処理が終了するのを待つが処理終了フラグが立たない。
  これはUSB機能を導入した際と同じ現象。その際はCPUを4つ動かしたら解決したが、まだCPUは
  1つしか動いていない。
- usb_init()を呼び出した時点でirqはマスクされている。色々なタイミングでマスクを外したが
  依然として割り込みは発生しない

```bash
qemu-system-aarch64 -M raspi3b -nographic -serial null -serial mon:stdio -drive file=obj/sd.img,if=sd,format=raw -netdev user,id=net0,hostfwd=tcp::8080-:80 -device usb-net,netdev=net0 -trace events=events,file=trace.log -kernel obj/kernel8.img
[3]rand_init: rand_init ok
[3]usb_init: daif: 0x7
[3]dwhc_xfer_stage: wait for wblk
```

**trace.log**

```bash
usb_port_claim bus 0, port 1
usb_hub_reset dev 0
usb_port_attach bus 0, port 1, devspeed full, portspeed full+high
usb_dwc2_attach port 0x7fae811794d0
usb_dwc2_attach_speed full-speed device attached
usb_dwc2_bus_start start SOFs
usb_dwc2_raise_global_irq 0x01000000
usb_port_claim bus 0, port 1.1
usb_port_attach bus 0, port 1.1, devspeed full, portspeed full
usb_hub_attach dev 0, port 1
usb_dwc2_reset_enter === RESET enter ===
usb_dwc2_detach port 0x7fae811794d0
usb_dwc2_bus_stop stop SOFs
usb_dwc2_bus_stop stop SOFs
usb_dwc2_reset_hold === RESET hold ===
usb_dwc2_reset_exit === RESET exit ===
usb_dwc2_attach port 0x7fae811794d0
usb_dwc2_attach_speed full-speed device attached
usb_dwc2_bus_start start SOFs
usb_dwc2_raise_global_irq 0x01000000
usb_hub_reset dev 0
usb_dwc2_raise_global_irq 0x00000008
usb_dwc2_glbreg_read  0x0040 GSNPSID   val 0x4f54294a
usb_dwc2_glbreg_read  0x0008 GAHBCFG   val 0x00000000
usb_dwc2_glbreg_write 0x0008 GAHBCFG   val 0x00000000 old 0x00000000 result 0x00000000
usb_dwc2_glbreg_read  0x000c GUSBCFG   val 0x00001400
usb_dwc2_glbreg_write 0x000c GUSBCFG   val 0x00001400 old 0x00001400 result 0x00001400
usb_dwc2_glbreg_read  0x0010 GRSTCTL   val 0x80000000
usb_dwc2_glbreg_read  0x0010 GRSTCTL   val 0x80000000
usb_dwc2_glbreg_write 0x0010 GRSTCTL   val 0x80000001 old 0x80000000 result 0x80000001
usb_dwc2_glbreg_read  0x0010 GRSTCTL   val 0x80000000
usb_dwc2_glbreg_read  0x000c GUSBCFG   val 0x00001400
usb_dwc2_glbreg_write 0x000c GUSBCFG   val 0x00001400 old 0x00001400 result 0x00001400
usb_dwc2_glbreg_read  0x0048 GHWCFG2   val 0x250dc016
usb_dwc2_glbreg_read  0x000c GUSBCFG   val 0x00001400
usb_dwc2_glbreg_write 0x000c GUSBCFG   val 0x00001400 old 0x00001400 result 0x00001400
usb_dwc2_glbreg_read  0x0008 GAHBCFG   val 0x00000000
usb_dwc2_glbreg_write 0x0008 GAHBCFG   val 0x00000030 old 0x00000000 result 0x00000030
usb_dwc2_glbreg_read  0x000c GUSBCFG   val 0x00001400
usb_dwc2_glbreg_write 0x000c GUSBCFG   val 0x00001400 old 0x00001400 result 0x00001400
usb_dwc2_glbreg_write 0x0014 GINTSTS   val 0xffffffff old 0x15000029 result 0xffffffff05000021
usb_dwc2_glbreg_read  0x0008 GAHBCFG   val 0x00000030
usb_dwc2_glbreg_write 0x0008 GAHBCFG   val 0x00000031 old 0x00000030 result 0x00000031
usb_dwc2_pcgreg_write 0x0e00 PCGCTL    val 0x00000000 old 0x00000000 result 0x00000000
usb_dwc2_hreg0_read   0x0400 HCFG      val 0x00000200
usb_dwc2_glbreg_read  0x0048 GHWCFG2   val 0x250dc016
usb_dwc2_glbreg_read  0x000c GUSBCFG   val 0x00001400
usb_dwc2_hreg0_write  0x0400 HCFG      val 0x00000200 old 0x00000200 result 0x00000200
usb_dwc2_glbreg_write 0x0024 GRXFSIZ   val 0x00000400 old 0x00000400 result 0x00000400
usb_dwc2_glbreg_write 0x0028 GNPTXFSIZ val 0x04000400 old 0x04000000 result 0x04000400
usb_dwc2_fszreg_write 0x0100 HPTXFSIZ  val 0x04000800 old 0x01f40000 result 0x04000800
usb_dwc2_glbreg_write 0x0010 GRSTCTL   val 0x00000420 old 0x80000000 result 0x80000420
usb_dwc2_glbreg_read  0x0010 GRSTCTL   val 0x80000400
usb_dwc2_glbreg_write 0x0010 GRSTCTL   val 0x00000010 old 0x80000400 result 0x80000010
usb_dwc2_raise_global_irq 0x00000008
usb_dwc2_glbreg_read  0x0010 GRSTCTL   val 0x80000000
usb_dwc2_hreg0_read   0x0440 HPRT0     val 0x00021003
usb_dwc2_glbreg_write 0x0018 GINTMSK   val 0x00000000 old 0x00000000 result 0x00000000
usb_dwc2_glbreg_write 0x0014 GINTSTS   val 0xffffffff old 0x05000029 result 0xffffffff05000021
usb_dwc2_glbreg_read  0x0018 GINTMSK   val 0x00000000
usb_dwc2_glbreg_write 0x0018 GINTMSK   val 0x02000000 old 0x00000000 result 0x02000000
usb_dwc2_hreg0_read   0x0440 HPRT0     val 0x00021003
usb_dwc2_raise_global_irq 0x00000008
usb_dwc2_hreg0_read   0x0440 HPRT0     val 0x00021003
usb_dwc2_hreg0_write  0x0440 HPRT0     val 0x00021101 old 0x00021003 result 0x00021103
usb_dwc2_hreg0_action enable PRTINT
usb_dwc2_hreg0_read   0x0440 HPRT0     val 0x00021103
usb_dwc2_hreg0_write  0x0440 HPRT0     val 0x00021001 old 0x00021103 result 0x0002100d
usb_dwc2_hreg0_action call usb_port_reset
usb_dwc2_detach port 0x7fae811794d0
usb_dwc2_bus_stop stop SOFs
usb_dwc2_attach port 0x7fae811794d0
usb_dwc2_attach_speed full-speed device attached
usb_dwc2_bus_start start SOFs
usb_hub_reset dev 0
usb_dwc2_hreg0_action enable PRTINT
usb_dwc2_hreg0_read   0x0440 HPRT0     val 0x0002100d
usb_dwc2_hreg0_read   0x0418 HAINTMSK  val 0x00000000
usb_dwc2_hreg0_write  0x0418 HAINTMSK  val 0x00000001 old 0x00000000 result 0x00000001
usb_dwc2_hreg1_read   0x0500 HCCHAR  40 val 0x00000000
usb_dwc2_hreg1_write  0x0508 HCINT   0 val 0xffffffff old 0x00000000 result 0x00000000
usb_dwc2_hreg1_write  0x0510 HCTSIZ  0 val 0x60080008 old 0x00000000 result 0x60080008
usb_dwc2_hreg1_write  0x0514 HCDMA   0 val 0xc007ce60 old 0x00000000 result 0xc007ce60
usb_dwc2_hreg1_write  0x0504 HCSPLT  0 val 0x00000000 old 0x00000000 result 0x00000000
usb_dwc2_hreg1_read   0x0500 HCCHAR  40 val 0x00000000
usb_dwc2_hreg1_write  0x050c HCINTMSK0 val 0x0000078f old 0x00000000 result 0x0000078f
usb_dwc2_hreg1_write  0x0500 HCCHAR  0 val 0x80100008 old 0x00000000 result 0x80100008
usb_dwc2_find_device 0
usb_dwc2_device_found device found on port 0
usb_dwc2_enable_chan ch 0 dev 0x7fae7f924800 pkt 0x7fae81179518 ep 0
usb_dwc2_handle_packet ch 0 dev 0x7fae7f924800 pkt 0x7fae81179518 ep 0 type Ctrl dir Out mps 8 len 8 pcnt 1
usb_dwc2_memory_read addr -1073230240 len 8     //
usb_packet_state_change bus 0, port 1, ep 0, packet 0x7fae81179518, state undef -> setup
usb_hub_control dev 0, req 0x8006, value 256, index 0, length 8
usb_desc_device dev 0 query device, len 8, ret 18
usb_packet_state_change bus 0, port 1, ep 0, packet 0x7fae81179518, state setup -> complete
usb_dwc2_packet_status status USB_RET_SUCCESS len 8
usb_dwc2_packet_done status USB_RET_SUCCESS actual 8 len 0 pcnt 0
usb_dwc2_raise_host_irq 0x0001
usb_dwc2_raise_global_irq 0x02000000
usb_dwc2_update_irq level=1
usb_dwc2_work_bh                    // qemu#hcd-dwc2.cに dwc2_work_bh(void *opaque) 関数がある
# usb_dwc2_glbreg_read  0x0014 GINTSTS   val 0x07000029  // 正常であれば次はこの行
```

### QEMUのソース

```c
static void dwc2_class_init(ObjectClass *klass, void *data) {
    DeviceClass *dc = DEVICE_CLASS(klass);
    DWC2Class *c = DWC2_USB_CLASS(klass);
    ResettableClass *rc = RESETTABLE_CLASS(klass);

    dc->realize = dwc2_realize;
    dc->vmsd = &vmstate_dwc2_state;
    set_bit(DEVICE_CATEGORY_USB, dc->categories);
    device_class_set_props(dc, dwc2_usb_properties);
    resettable_class_set_parent_phases(rc, dwc2_reset_enter, dwc2_reset_hold,
                                       dwc2_reset_exit, &c->parent_phases);
}

void dwc2_realize(DeviceState *dev, Error **errp) {
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);
    DWC2State *s = DWC2_USB(dev);
    Object *obj;

    obj = object_property_get_link(OBJECT(dev), "dma-mr", &error_abort);

    s->dma_mr = MEMORY_REGION(obj);
    address_space_init(&s->dma_as, s->dma_mr, "dwc2");

    usb_bus_new(&s->bus, sizeof(s->bus), &dwc2_bus_ops, dev);
    usb_register_port(&s->bus, &s->uport, s, 0, &dwc2_port_ops,
                      USB_SPEED_MASK_LOW | USB_SPEED_MASK_FULL |
                      (s->usb_version == 2 ? USB_SPEED_MASK_HIGH : 0));
    s->uport.dev = 0;

    s->usb_frame_time = NANOSECONDS_PER_SECOND / 1000;          /* 1000000 */
    if (NANOSECONDS_PER_SECOND >= USB_HZ_FS) {
        s->usb_bit_time = NANOSECONDS_PER_SECOND / USB_HZ_FS;   /* 83.3 */
    } else {
        s->usb_bit_time = 1;
    }

    s->fi = USB_FRMINTVL - 1;
    s->eof_timer = timer_new_ns(QEMU_CLOCK_VIRTUAL, dwc2_frame_boundary, s);
    s->frame_timer = timer_new_ns(QEMU_CLOCK_VIRTUAL, dwc2_work_timer, s);
    s->async_bh = qemu_bh_new(dwc2_work_bh, s);

    sysbus_init_irq(sbd, &s->irq);
}

static void dwc2_work_bh(void *opaque) {
    DWC2State *s = opaque;
    DWC2Packet *p;
    USBDevice *dev;
    USBEndpoint *ep;
    int64_t t_now, expire_time;
    int chan;
    bool found = false;

    if (s->working) {
        return;
    }
    s->working = true;

    t_now = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
    chan = s->next_chan;

    do {
        p = &s->packet[chan];
        if (p->needs_service) {
            dev = dwc2_find_device(s, p->devadr);
            ep = usb_ep_get(dev, p->pid, p->epnum);
            dwc2_handle_packet(s, p->devadr, dev, ep, p->index, true);
            found = true;
        }
        if (++chan == DWC2_NB_CHAN) {
            chan = 0;
        }
        if (found) {
            s->next_chan = chan;
        }
    } while (chan != s->next_chan);

    if (found) {
        expire_time = t_now + NANOSECONDS_PER_SECOND / 4000;
        timer_mod(s->frame_timer, expire_time);
    }
    s->working = false;
}
```

## usbブランチで種々調査

1. usb_init()はforkret()の最後に実行
2. usb_init()の先頭と最後でirqはdisable
3. 割り込みは終始、起きている
	- src: 0x800 (timer), 0x2 (core timer), 0x100 (gpu)
	- irq: 0x100000000000000 (SDIO), 0x200 (USB)
4. main()のconsole_init()以後でもすべてirqはdisable
5. dwhc_xfer_stage()のwaitフラグの設定と変更をするCPUは同じ場合もあれば、違う場合もある
6. dwhc_xfer_stage()のwaitフラグを設定する際のirqもdisable
7. usb_init()をmain()で実行するとwaitフラグは更新されない

```bash
// 3.
[2]emmc_card_reset: RCA: 0x4567
[0]irq_handler: src: 0x100
[0]irq_handler: irq: 0x100000000000000
[0]irq_handler: src: 0x800
[0]irq_handler: irq: 0x0
[0]irq_handler: src: 0x100
[0]irq_handler: irq: 0x100000000000000
[1]emmc_card_reset: SCR: version 2.00, bus_widths 0x5
[0]irq_handler: src: 0x800
[1]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 1, timing 0, vdd 0, drv_type 0
[0]irq_handler: irq: 0x0
[1]emmc_card_reset: found valid version 2.00 SD card
[0]irq_handler: src: 0x900
[0]irq_handler: irq: 0x100000000000000

// 2.
[2]usb_init: [S] daif: 0x3c0	// daif = bit[9-6] = 0b11_1100_0000
[2]usb_dev_init: Device ven409-55aa, dev9-0-0 found
...
[2]usb_init: dwhc initialized
[2]usb_init: [E] daif: 0x3c0

// 4.
[2]main: [a] daif: 0x1c0	// console_init()の後
[2]main: [b] daif: 0x1c0	// vm_test()の後
[2]main: [c] daif: 0x1c0	// timer_init()の前
[2]main: [d] daif: 0x1c0	// timer_init()の後
[2]main: [e] daif: 0x1c0	// trap_init()の後

// 5.               wait番号           　　設定/解除したcpu
[2]dwhc_xfer_stage: [1] xfer_async start [2]
[1]dwhc_xfer_stage: [1] xfer_async end   [1]
[1]dwhc_xfer_stage: [2] xfer_async start [1]
[2]dwhc_xfer_stage: [2] xfer_async end   [2]
[2]dwhc_xfer_stage: [3] xfer_async start [2]
[2]dwhc_xfer_stage: [3] xfer_async end   [2]
[2]dwhc_xfer_stage: [4] xfer_async start [2]
[3]dwhc_xfer_stage: [4] xfer_async end   [3]
[3]dwhc_xfer_stage: [5] xfer_async start [3]
[3]dwhc_xfer_stage: [5] xfer_async end   [3]
[3]dwhc_xfer_stage: [6] xfer_async start [3]
[1]dwhc_xfer_stage: [6] xfer_async end   [1]

// 6.
[2]dwhc_xfer_stage: daif: 0x3c0

// 7.
[0]dwhc_xfer_stage: wait start [0]
// waitが解消されない

usb_dwc2_raise_host_irq 0x0001
usb_dwc2_raise_global_irq 0x02000000
usb_dwc2_update_irq level=1
usb_dwc2_work_bh
```

## usb_init()とnet_init()が動く

- lan7800とusb_cdcetherのnet部分の初期化をnet_init()からusb_init()に移し、
  変数`usb_lib`をstaticに戻す
- enable_irq(), pop_intr()など、xv6-riscvから取り入れた機能を外す。
- ARMは例外発生時に自動的に割り込みが無効になるので、例外ハンドラ内で明示的に有効にする必要が
  ある場合を除いてプログラムで有効にする必要はない（Lab3講義資料）。

```bash
qemu-system-aarch64 -M raspi3b -nographic -serial null -serial mon:stdio -drive file=obj/sd.img,if=sd,format=raw -netdev user,id=net0,hostfwd=tcp::8080-:80 -device usb-net,netdev=net0 -trace events=events,file=trace.log -kernel obj/kernel8.img
[0]rand_init: rand_init ok
[0]timer_init: timerfreq = 0x3b9aca0
[3]timer_init: timerfreq = 0x3b9aca0
[2]timer_init: timerfreq = 0x3b9aca0
[3]main: cpu 3 init finished
[1]timer_init: timerfreq = 0x3b9aca0
[2]main: cpu 2 init finished
[0]main: cpu 0 init finished
[1]main: cpu 1 init finished
[3]mbox_set_sdhost_clock: unexpected tag resp 0x80000000, normal for qemu
[3]sdhost_set_ios: ios clock 400000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[3]sdhost_finish_command: error detected: CMD 0x4205, HSTS 0x40, EDM 0x10800
[3]sdhost_finish_command: command 5 timeout
[3]emmc_card_reset: OCR: 0xffff, 1.8v support: 0, SDHC support: 0
[3]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[3]emmc_card_reset: card CID: 0xaa585951, 0x454d5521, 0x1deadbe, 0xef006219
[3]emmc_card_reset: RCA: 0x4567
[1]emmc_card_reset: SCR: version 2.00, bus_widths 0x5
[1]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 1, timing 0, vdd 0, drv_type 0
[1]emmc_card_reset: found valid version 2.00 SD card
[2]sd_init: partition[0]: TYPE: 12, LBA = 0x800, #SECS = 0x20000
[2]sd_init: partition[1]: TYPE: 131, LBA = 0x20800, #SECS = 0x1f800
[2]sd_init: sd_init ok

[2]iinit: sb: size 1000 nblocks 963 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 36
[2]initlog: not use log
[1]usb_dev_init: Device ven409-55aa, dev9-0-0 found
[3]usb_dev_init: Product: QEMU QEMU USB Hub
[3]usb_function_get_if_name: func name=int9-0-0
[3]usb_dev_init: Interface int9-0-0 found
[3]usb_dev_factory_get_device: Using device/interface int9-0-0
[1]usb_dev_init: Device ven525-a4a2, dev2-0-0 found
[1]usb_dev_init: Product: QEMU RNDIS/QEMU USB Network Device
[1]usb_function_get_if_name: func name=int2-6-0
[1]usb_dev_init: Interface int2-6-0 found
[1]usb_dev_factory_get_device: Using device/interface int2-6-0
[1]usb_function_get_if_name: func name=inta-0-0
[1]usb_dev_init: Interface inta-0-0 found
[1]usb_dev_init: Function is not supported
[1]usb_function_get_if_name: func name=inta-0-0
[1]usb_dev_init: Interface inta-0-0 found
[1]usb_dev_init: Function is not supported
[3]usb_cdcether_configure: MAC address is 40:54:0:12:34:57
[3]usb_standard_hub_enumerate_ports: Port 1: Device configured
[2]dwhc_root_port_init: Device configured
[2]usb_init: dwhc initialized

[2]usb_init: usb_init ok
[2]net_protocol_register: registered, type=0x0800
[2]net_protocol_register: registered, type=0x0806
[2]net_timer_register: registered: interval={1, 0}
[2]ip_protocol_register: registered, type=1
[2]ip_protocol_register: registered, type=17
[2]ip_protocol_register: registered, type=6
[2]net_timer_register: registered: interval={0, 100000}
[2]netinit: initialized
[2]netrun: open all devices...
[2]netrun: running...
init: starting sh
sh: argv[0] = 'sh'
sh: testenv = 'FROM_INIT'
$ ls
.              4000 1 4096
..             4000 1 4096
cat            8000 2 38568
init           8000 3 22400
echo           8000 4 39480
mkfs           8000 5 45528
date           8000 6 49368
sh             8000 7 54056
utest          8000 8 17744
ls             8000 9 41304
console        0 10 0
$
```

## ether-input/trasmit-helper周りを実装

- muslにはSYS_sendとSYS_recvがない（それぞれSYS_sendto, SYS_recvfromを使用）
- ユーザプログラム`ifconfig`を導入
- MACアドレス固定でipアドレスを付番

```bash
qemu-system-aarch64 -M raspi3b -nographic -serial null -serial mon:stdio -drive file=obj/sd.img,if=sd,format=raw -netdev user,id=net0,hostfwd=tcp::8080-:80 -device usb-net,netdev=net0 -trace events=events,file=trace.log -kernel obj/kernel8.img
[0]rand_init: rand_init ok
[0]timer_init: timerfreq = 0x3b9aca0
[2]timer_init: timerfreq = 0x3b9aca0
[1]timer_init: timerfreq = 0x3b9aca0
[3]timer_init: timerfreq = 0x3b9aca0
[0]main: cpu 0 init finished
[2]main: cpu 2 init finished
[3]main: cpu 3 init finished
[1]main: cpu 1 init finished
[0]mbox_set_sdhost_clock: unexpected tag resp 0x80000000, normal for qemu
[0]sdhost_set_ios: ios clock 400000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[0]sdhost_finish_command: error detected: CMD 0x4205, HSTS 0x40, EDM 0x10800
[0]sdhost_finish_command: command 5 timeout
[0]emmc_card_reset: OCR: 0xffff, 1.8v support: 0, SDHC support: 0
[0]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[0]emmc_card_reset: card CID: 0xaa585951, 0x454d5521, 0x1deadbe, 0xef006219
[0]emmc_card_reset: RCA: 0x4567
[2]emmc_card_reset: SCR: version 2.00, bus_widths 0x5
[2]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 1, timing 0, vdd 0, drv_type 0
[2]emmc_card_reset: found valid version 2.00 SD card
[3]sd_init: partition[0]: TYPE: 12, LBA = 0x800, #SECS = 0x20000
[3]sd_init: partition[1]: TYPE: 131, LBA = 0x20800, #SECS = 0x1f800
[3]sd_init: sd_init ok

[1]iinit: sb: size 1000 nblocks 963 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 36
[1]initlog: not use log
[3]usb_dev_init: Device ven409-55aa, dev9-0-0 found
[3]usb_dev_init: Product: QEMU QEMU USB Hub
[3]usb_function_get_if_name: func name=int9-0-0
[3]usb_dev_init: Interface int9-0-0 found
[3]usb_dev_factory_get_device: Using device/interface int9-0-0
[3]usb_dev_init: Device ven525-a4a2, dev2-0-0 found
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
[1]dwhc_root_port_init: Device configured
[1]usb_init: dwhc initialized

[1]usb_init: usb_init ok
[1]net_protocol_register: registered, type=0x0800
[1]net_protocol_register: registered, type=0x0806
[1]net_timer_register: registered: interval={1, 0}
[1]ip_protocol_register: registered, type=1
[1]ip_protocol_register: registered, type=17
[1]ip_protocol_register: registered, type=6
[1]net_timer_register: registered: interval={0, 100000}
[1]netinit: initialized
[1]netrun: open all devices...
[1]netrun: running...
init: starting sh
sh: argv[0] = 'sh'
sh: testenv = 'FROM_INIT'
$ ifconfig net0 192.168.10.111 netmask 255.255.255.0
[0]syscall1: proc[9] sys_gettid called
[0]syscall1: proc[9] sys_rt_sigprocmask called
[0]syscall1: proc[9] sys_rt_sigprocmask called
[0]syscall1: proc[9] sys_execve called
[1]syscall1: proc[9] sys_gettid called
[1]syscall1: proc[9] sys_socket called
[1]syscall1: proc[9] sys_exit called
$ ifconfig net0 up
[2]syscall1: proc[10] sys_gettid called
[2]syscall1: proc[10] sys_rt_sigprocmask called
[2]syscall1: proc[10] sys_rt_sigprocmask called
[2]syscall1: proc[10] sys_execve called
[1]syscall1: proc[10] sys_gettid called
[1]syscall1: proc[10] sys_socket called
[1]syscall1: proc[10] sys_exit called
$ ifconfig -a
[0]syscall1: proc[11] sys_gettid called
[0]syscall1: proc[11] sys_rt_sigprocmask called
[0]syscall1: proc[11] sys_rt_sigprocmask called
[0]syscall1: proc[11] sys_execve called
[2]syscall1: proc[11] sys_gettid called
[2]syscall1: proc[11] sys_socket called
[2]syscall1: proc[11] sys_exit called
[2]exit: exit: pid 11, err -1
$
```

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

### ただしpingは通らず

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
Request timeout for icmp_seq 5
ping: sendto: Host is down
Request timeout for icmp_seq 6
^Axping: sendto: Host is down
Request timeout for icmp_seq 7
^C
--- 192.168.10.111 ping statistics ---
9 packets transmitted, 0 packets received, 100.0% packet loss
```

- DWHCI_CORE_INT_STAT_RXFLVL 割り込みが起きないため

### 実機も同じ

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

## circle/sample/18-ntptime

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
64 bytes from 192.168.10.110: icmp_seq=2 ttl=64 time=0.516 ms
64 bytes from 192.168.10.110: icmp_seq=3 ttl=64 time=0.683 ms
64 bytes from 192.168.10.110: icmp_seq=4 ttl=64 time=0.807 ms
64 bytes from 192.168.10.110: icmp_seq=5 ttl=64 time=0.773 ms
64 bytes from 192.168.10.110: icmp_seq=6 ttl=64 time=0.577 ms
64 bytes from 192.168.10.110: icmp_seq=7 ttl=64 time=0.701 ms
64 bytes from 192.168.10.110: icmp_seq=8 ttl=64 time=0.650 ms
64 bytes from 192.168.10.110: icmp_seq=9 ttl=64 time=0.745 ms
64 bytes from 192.168.10.110: icmp_seq=10 ttl=64 time=0.494 ms
64 bytes from 192.168.10.110: icmp_seq=11 ttl=64 time=0.604 ms
64 bytes from 192.168.10.110: icmp_seq=12 ttl=64 time=0.716 ms
64 bytes from 192.168.10.110: icmp_seq=13 ttl=64 time=0.754 ms
64 bytes from 192.168.10.110: icmp_seq=14 ttl=64 time=0.703 ms
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

```bash
00:00:06.12 dhcp: IP address is 192.168.10.110
00:00:06.12 kernel: Try "ping 192.168.10.110" from another computer!
00:00:06.13 ntpd: Resolve start
00:00:06.36 dwhci: IntStatus: 0x6000031
00:00:06.36 dwhci: GRXSTRSR: 0x70000
00:00:07.13 ntpd: Resolve end
00:00:07.13 ntpd: GetTime start
00:00:07.14 ntp: send start
00:00:07.14 ntp: send end
00:00:08.14 ntp: recv start
00:00:08.14 ntp: recv end: result=48
00:00:08.14 ntpd: GetTime end
Oct  1 16:11:02.15 ntpd: System time updated
Oct  1 16:11:03.44 dwhci: IntStatus: 0x6000031
Oct  1 16:11:03.44 dwhci: GRXSTRSR: 0x60000
Oct  1 16:11:05.22 dwhci: IntStatus: 0x6000031
Oct  1 16:11:05.22 dwhci: GRXSTRSR: 0x40000
Oct  1 16:11:07.03 dwhci: IntStatus: 0x6000031
Oct  1 16:11:07.03 dwhci: GRXSTRSR: 0x60000
Oct  1 16:11:07.15 ntpd: Resolve start
Oct  1 16:11:08.15 ntpd: Resolve end
Oct  1 16:11:08.16 ntpd: GetTime start
Oct  1 16:11:08.16 ntp: send start
Oct  1 16:11:08.16 ntp: send end
Oct  1 16:11:09.17 ntp: recv start
Oct  1 16:11:09.17 ntp: recv end: result=48
Oct  1 16:11:09.17 ntpd: GetTime end
Oct  1 16:11:09.17 ntpd: System time updated
Oct  1 16:11:14.18 ntpd: Resolve start
Oct  1 16:11:15.18 ntpd: Resolve end
Oct  1 16:11:15.18 ntpd: GetTime start
Oct  1 16:11:15.19 ntp: send start
Oct  1 16:11:15.19 ntp: send end
Oct  1 16:11:16.19 ntp: recv start
Oct  1 16:11:16.19 ntp: recv end: result=48
Oct  1 16:11:16.20 ntpd: GetTime end
Oct  1 16:11:16.20 ntpd: System time updated
Oct  1 16:11:21.20 ntpd: Resolve start
Oct  1 16:11:22.21 ntpd: Resolve end
Oct  1 16:11:22.21 ntpd: GetTime start
Oct  1 16:11:22.21 ntp: send start
Oct  1 16:11:22.22 ntp: send end
Oct  1 16:11:23.22 ntp: recv start
Oct  1 16:11:23.22 ntp: recv end: result=48
Oct  1 16:11:23.22 ntpd: GetTime end
Oct  1 16:11:23.23 ntpd: System time updated
```


```c
static void lan78xx_defer_kevent(struct lan78xx_net *dev, int work);

static int lan78xx_start_tx_path(struct lan78xx_net *dev);
static int lan78xx_stop_tx_path(struct lan78xx_net *dev);
static int lan78xx_start_rx_path(struct lan78xx_net *dev);
static int lan78xx_stop_rx_path(struct lan78xx_net *dev);

static int lan78xx_reset(struct lan78xx_net *dev);
static int lan78xx_open(struct net_device *net);
static int lan78xx_stop(struct net_device *net);
static int lan78xx_rx(struct lan78xx_net *dev, struct sk_buff *skb,
              int budget, int *work_done);
static inline void rx_process(struct lan78xx_net *dev, struct sk_buff *skb,
                  int budget, int *work_done);

static int lan78xx_bh(struct lan78xx_net *dev, int budget);
```

## kthreadを作成し、ここでnet_handler()を実行する


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
[3]dwhc_submit_block_request: failed bulk or interrupt xter
[3]usb_cdcether_receive_frame: failed submit block request
[2]dwhc_submit_block_request: failed bulk or interrupt xter
[2]usb_cdcether_receive_frame: failed submit block request
[1]dwhc_submit_block_request: failed bulk or interrupt xter
[0]dwhc_start_channel: host=0xffff000000a40ed8, channel=-763359352      // channel番号がマイナス
[1]usb_cdcether_receive_frame: kern/usb/dwhc_device.c:804: assertion failed.
QEMU: Terminated
```
