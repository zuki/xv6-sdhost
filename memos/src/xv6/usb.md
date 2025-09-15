# CircleからUSB機能を導入

## uspiの導入は失敗

- usbの割り込みがかからず、転送待ちでフリーズ

## circleをcに移植して導入

- dwhcの初期化でフリーズ

### デバッグ出力

- dw2_hc_channel_intr_hdlがcallされず

```bash
[0]dwhc_device: dw2_hc created
[0]dwhc_enable_root_port: enabled rootport
[0]dwhc_get_desc: call dwhc_control_message
[0]dwhc_control_message: setup_data: 0xffff00000007fe20
[0]dwhc_control_message: 3
[0]dwhc_submit_block_request: host=0xffff000000a58ed8, urb=0xffff00000007fe28, timeout=0
[0]dwhc_submit_block_request: Control xfer: IN request
[0]dwhc_xfer_stage: host=0xffff000000a58ed8, urb=0xffff00000007fe28, in=0, stage=0, timeout=0
[0]dwhc_xfer_stage: 3
[0]dwhc_xfer_stage_async: host=0xffff000000a58ed8, urb=0xffff00000007fe28, in=0, stage=0, timeout=0
[0]dwhc_xfer_stage_async: 1: ch=0
[0]dwhc_xfer_stage_async: 2: stdata=0xffff000000a58cc0
[0]dwhc_xfer_stage_async: 3
[0]dwhc_xfer_stage_async: enable channel interrupt: stdata[0]=0xffff000000a58cc0
[0]dwhc_xfer_stage_async: 4
[0]dwhc_xfer_stage_async: 6
[0]dwhc_start_trans: host=0xffff000000a58ed8, stdata=0xffff000000a58cc0
[0]dwhc_start_trans: 2-2
[0]dwhc_start_channel: host=0xffff000000a58ed8, stdata=0xffff000000a58cc0
[0]dwhc_start_channel: 3
[0]dwhc_start_channel: 4
[0]dwhc_start_channel: 5
[0]dwhc_start_channel: 6
[0]dwhc_start_channel: 7
[0]dwhc_start_channel: 9
[0]dwhc_start_channel: 10-9: char=0x%08x
[0]dwhc_start_channel: ch=0x0, intrmask=0x78f
[0]dwhc_start_channel: char=0x80100008
[0]dwhc_start_channel: 10-13
```

### QEMUのトレース出力

```bash
usb_port_claim bus 0, port 1
usb_hub_reset dev 0
usb_port_attach bus 0, port 1, devspeed full, portspeed full+high
usb_dwc2_attach port 0x7fed819794d0
usb_dwc2_attach_speed full-speed device attached
usb_dwc2_bus_start start SOFs
usb_dwc2_raise_global_irq 0x01000000
usb_port_claim bus 0, port 1.1
usb_port_attach bus 0, port 1.1, devspeed full, portspeed full
usb_hub_attach dev 0, port 1
usb_dwc2_reset_enter === RESET enter ===
usb_dwc2_detach port 0x7fed819794d0
usb_dwc2_bus_stop stop SOFs
usb_dwc2_bus_stop stop SOFs
usb_dwc2_reset_hold === RESET hold ===
usb_dwc2_reset_exit === RESET exit ===
usb_dwc2_attach port 0x7fed819794d0
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
usb_dwc2_raise_global_irq 0x00000008
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
usb_dwc2_detach port 0x7fed819794d0
usb_dwc2_bus_stop stop SOFs
usb_dwc2_attach port 0x7fed819794d0
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
usb_dwc2_hreg1_write  0x0514 HCDMA   0 val 0xc007fe20 old 0x00000000 result 0xc007fe20
usb_dwc2_hreg1_write  0x0504 HCSPLT  0 val 0x00000000 old 0x00000000 result 0x00000000
usb_dwc2_hreg1_read   0x0500 HCCHAR  40 val 0x00000000
usb_dwc2_hreg1_write  0x050c HCINTMSK0 val 0x0000078f old 0x00000000 result 0x0000078f
usb_dwc2_hreg1_write  0x0500 HCCHAR  0 val 0x80100008 old 0x00000000 result 0x80100008
usb_dwc2_find_device 0
usb_dwc2_device_found device found on port 0
usb_dwc2_enable_chan ch 0 dev 0x7fed8106bc00 pkt 0x7fed81979518 ep 0
usb_dwc2_handle_packet ch 0 dev 0x7fed8106bc00 pkt 0x7fed81979518 ep 0 type Ctrl dir Out mps 8 len 8 pcnt 1
usb_dwc2_memory_read addr -1073218016 len 8
usb_packet_state_change bus 0, port 1, ep 0, packet 0x7fed81979518, state undef -> setup
usb_hub_control dev 0, req 0x8006, value 256, index 0, length 8
usb_desc_device dev 0 query device, len 8, ret 18
usb_packet_state_change bus 0, port 1, ep 0, packet 0x7fed81979518, state setup -> complete
usb_dwc2_packet_status status USB_RET_SUCCESS len 8
usb_dwc2_packet_done status USB_RET_SUCCESS actual 8 len 0 pcnt 0
usb_dwc2_raise_host_irq 0x0001
usb_dwc2_raise_global_irq 0x02000000
usb_dwc2_update_irq level=1
usb_dwc2_work_bh
```

### CPUを4つ動かしたら動かした

- 1つしかないCPUで割り込み待ちになったせいか?
- ただし、次のようなエラーが発生することあり

```bash
[1]dwhc_start_trans: ok
[0]dwhc_channel_intr_hdl: called        // 割り込みハンドラが2つのCPUで実行されている?
[2]dwhc_channel_intr_hdl: called        //
[0]dwhc_channel_intr_hdl: stdata=0xffff000000a40620, scheduler=0xffff000000a406d8
[2]dwhc_channel_intr_hdl: stdata=0xffff000000a40620, scheduler=0xffff000000a406d8
[0]dwhc_channel_intr_hdl: urb=0xffff000000a5fbe8, root_port_enabled=0x1
[2]dwhc_channel_intr_hdl: urb=0xffff000000a5fbe8, root_port_enabled=0x1
[0]dwhc_channel_intr_hdl: substate: wait_for_xfer_complete
[
annels_tiagedatantr:_hdl:  substsagtate: wait_fedoar_xferta: 0x_fcfff0omplete
00000a40620

stagedata: stagedata: 0xffff000000a40620
    : channel: 0, in: false, stage: false, fsused: true, split: false
    : state: 0, substate: 1, trstatus: 0
    : buffp: 0xffff000000a5fbe0, bpt: 8, ppt: 1, packets: 1

[2]dwhc_channel_intr_hdl: state: no_split
    : channel: 0, in: false, stage: false, fsused: true, split: false
[   3 ]dwhc_xfer_sta: sge:tat hoset=0x:f fff000000a0,40ed0, ur b=0xsufffbsf00000t0aate5:fb e11, in=1, s, ttrstaatge=0, timeouuts: 3
=0
  :[3 ]dwhc_xfer_sbutafge: fp3
[3: 0x]dwhc_x0, bptfer:_s tage_asyn0c: h, pptost:=0xffff000000 a40ed8, urb=0xffff000000a5fbe8, in=1, stage=0, timeout=0
[3]dwhc_xfer_stage_async: 1: ch=0
[3]dwh0,c_ xfer_stpage_async: 2: satdata=0xckffeff00t0000s: a040

20
[3]dwhc_xfer_data: nosplit: 0xffff000000a406d8

stagedata: stagedata: 0xffff000000a40620
    : channel: [0]dwhc_channel_intr_hdl: state: no_split
0, in: true, stage: false, fsused: true, split: false
    : state: 0, substate: 0, trstatus: 3
    : buffp: 0x0, bpt: 0, ppt: 0, packets: 0

[3]dwhc_xfer_stage_async: 3
[3]dwhc_xfer_stage_async: enable channel interrupt: stdata[0]=0xffff000000a40620
[3]dwhc_xfer_stage_async: 4
[3]dwhc_xfer_stage_async: 6
[3]dwhc_start_trans: host=0xffff000000a40ed8, stdata=0xffff000000a40620
[3]dwhc_start_trans: 2-2 start_channel: 0
[3]dwhc_start_channel: host=0xffff000000a40ed8, channel=0
[3]dwhc_start_channel: 3
[3]dwhc_xfer_data_get_pid: bad next: 402653182              // エラー発生
kern/usb/dwhc_xfer_data.c:323: assertion failed.
kern/console.c:285: kernel panic at cpu 3.
```
