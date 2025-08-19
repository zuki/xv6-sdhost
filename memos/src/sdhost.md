# SDHOSTを使う

```bash
Welcome to minicom 2.8

OPTIONS: 
Compiled on Jan  4 2021, 00:04:46.
Port /dev/cu.usbserial-AI057C9L, 10:30:57
Using character set conversion

Press Meta-Z for help on special keys

[0]free_range: 0xffff0000000a1000 ~ 0xffff00003b400000, 242527 pages
[0]main: cpu 0 init finished
[3]main: cpu 3 init finished
[0]sdhost_probe: firmware sets clock divider
[2]main: cpu 2 init finished                                          
[1]main: cpu 1 init finished                                          
[0]sdhost_set_ios: ios clock 400000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[0]sdhost_finish_command: error detected: CMD 0x4205, HSTS 0x40, EDM 0x10800
[0]sdhost_finish_command: command 5 timeout
[0]emmc_card_reset: OCR: 0xff80, 1.8v support: 0, SDHC support: 1
[0]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 0, timing 0, vdd 0, drv_type 0
[0]emmc_card_reset: card CID: 0x2544d53, 0x41313647, 0x31299f69, 0x3a0138c3
[0]emmc_card_reset: RCA: 0x1234
[3]emmc_card_reset: SCR: version 3.0x, bus_widths 0x5
[3]sdhost_set_ios: ios clock 25000000, pwr 0, bus_width 1, timing 0, vdd 0, drv_type 0
[3]emmc_card_reset: found valid version 3.0x SD card
[3]dev_init: LBA of 1st block 0x20800, 0x1f800 blocks totally
[1]iinit: sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmapstart 58
init: starting sh
sh: argv[0] = 'sh'
sh: testenv = 'FROM_INIT'
$ ls
.              4000 1 512
..             4000 1 512
cat            8000 2 38568
init           8000 3 22400
echo           8000 4 39480
mkfs           8000 5 45128
sh             8000 6 54056
utest          8000 7 17744
ls             8000 8 41304
console        0 9 0
$ echo abc
abc
$ echo abc > test.txt
$ cat test.txt
abc
$ 
```

## ファイル修正

```bash
$ git diff inc/sdhost.h 
diff --git a/inc/sdhost.h b/inc/sdhost.h
index 87f6903..558fff9 100644
--- a/inc/sdhost.h
+++ b/inc/sdhost.h
@@ -5,7 +5,7 @@
 
 #if RASPI == 3
 // FIXME: Use sdhost and reserve sdhci for wifi.
-// #define USE_SDHOST
+#define USE_SDHOST
 #endif
 
 struct sg_mapping_iter

$ git diff kern/emmc.c
@@ -486,6 +486,7 @@ emmc_intr(struct emmc *self)
 #endif
 }
 
+#ifndef USE_SDHOST
 void
 emmc_clear_interrupt()
 {
@@ -493,6 +494,7 @@ emmc_clear_interrupt()
     debug("irpts: 0x%x", irpts);
     put32(EMMC_INTERRUPT, irpts);
 }
+#endif
 
 int
 emmc_init(struct emmc *self, void (*sleep_fn)(void *), void *sleep_arg)
```

## minicomの設定

```bash
pu port				/dev/cu.usbserial-AI057C9L
pu baudrate			115200
```