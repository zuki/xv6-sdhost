# 時計機能を追加

## clock.c

- ローカル64ビットタイマー(19.2MHzで上下エッジでカウント)で計時
- ローカル64ビットタイマー割り込みをHZ=100 (10ミリ秒)で発生させる(1tick -> jiffiesを増分)
、タイマー割り込み時にjiffies, xtimeを更新、タイマーを実行
- jiffies: 電源投入後のticksを保持（HZ毎に++）
- xtime: epoch秒を保持

## timer.c

- タイマー機能を追加（登録されたタイマーを時系列に並べ、発火時刻の差分で管理）
- タイマーリストは各コア毎に持ち、core_intr()から操作している。すなわち、
  1 tick毎に先頭の残り時間を更新して、時間が来たら発火）
- タイマーは各コアが別個に持っているが今のところ、このタイマーは未使用
  1秒毎に発火しているがリセットしているだけ。使い所は今後考える。

## rtc.c

- DS3231チップを搭載したRTCを外付け。DS3231とRaspiはI2C (0x68)で接続
- rtc機能を追加し、起動時とdateコマンドで時刻設定時にxtimeに反映

## random.c

- 乱数発生機能を追加

## 追加変更したファイル

```bash
$ git diff c6bfdf1 --name-status
M	inc/arm.h
M	inc/clock.h
A	inc/ds3231.h
A	inc/i2c.h
A	inc/linux/errno.h
A	inc/linux/fcntl.h
A	inc/linux/mman.h
A	inc/linux/syscall.h
M	inc/linux/time.h
A	inc/random.h
A	inc/rtc.h
A	inc/syscall.h
M	inc/timer.h
M	inc/trap.h
M	kern/clock.c
A	kern/ds3231.c
A	kern/i2c.c
M	kern/icode.S
M	kern/main.c
A	kern/random.c
A	kern/rtc.c
M	kern/syscall.c
M	kern/sysfile.c
M	kern/sysproc.c
M	kern/timer.c
A	usr/src/date/main.c
```

## 実行画面

### 日付設定

```bash
Press Meta-Z for help on special keys

[0]rand_init: rand_init ok
[0]timer_init: timerfreq = 0x124f800
[0]main: cpu 0 init finished
[0]sdhost_probe: firmware sets clock divider
[0]sdhost_set_ios: ios clock 400000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[0]sdhost_finish_command: error detected: CMD 0x4205, HSTS 0x40, EDM 0x10800
[0]sdhost_finish_command: command 5 timeout
[0]emmc_card_reset: OCR: 0xff80, 1.8v support: 0, SDHC support: 1
[0]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[0]emmc_card_reset: card CID: 0x27504853, 0x44333247, 0x506c5d21, 0xcc017421
[0]emmc_card_reset: RCA: 0x5048
[0]emmc_card_reset: SCR: version 3.0x, bus_widths 0x5
[0]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 1, timing 0, vdd 0, drv_type 0
[0]emmc_card_reset: found valid version 3.0x SD card
[0]sd_init: partition[0]: TYPE: 12, LBA = 0x800, #SECS = 0x20000
[0]sd_init: partition[1]: TYPE: 131, LBA = 0x20800, #SECS = 0x1f800
[0]sd_init: sd_init ok

[0]iinit: sb: size 1000 nblocks 963 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 36
[0]initlog: not use log
init: starting sh
sh: argv[0] = 'sh'
sh: testenv = 'FROM_INIT'
$ date
2025年 9日 9日 火曜日  9時48分41秒 JST      // RTC未設定の場合のデフォルト日付+1
$ date 1757378950
2025年 9日 9日 火曜日  9時49分10秒 JST
$ date
2025年 9日 9日 火曜日  9時49分12秒 JST
$
```

### 電源再投入

```bash
$ date
2025年 9日 9日 火曜日  9時49分47秒 JST      // RTCから設定されている
$ date
2025年 9日 9日 火曜日  9時50分 1秒 JST
```
