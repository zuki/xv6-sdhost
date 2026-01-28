# インターバルタイマーの実装

```bash
$ /bin/timertest 2 0 1 0
       Elapsed   Value Interval
START:    0.00
[1]sys_setitimer: which: 0, new_value: 0xfffffffffe78, old_value: 0x0
[1]tvtojiffies: return HZ: 0x64 * sec: 0x1 + usec: 0x0 = 0x64
[1]tvtojiffies: return HZ: 0x64 * sec: 0x2 + usec: 0x0 = 0xc8
1236[1]setitimer: 7
8
[1]trap: [9] unknown trap code: 34 at 0x2 with 0x2
						// ここでストール
```

ec = 34 (0b100110) : SP alignment fault exception.

```bash
$ /bin/timertest 2 0 1 0
[1]sys_clock_gettime: clk_id: 0, tp: 0x0xfffffffffdb0

[1]clock_gettime: clk: 0, tv_sec: 1769411344, tv_nsec: 940000000
       Elapsed   Value Interval
[1]sys_clock_gettime: clk_id: 0, tp: 0x0xfffffffffdb0

[1]clock_gettime: clk: 0, tv_sec: 1769411344, tv_nsec: 950000000
START:    0.01
[1]sys_clock_gettime: clk_id: 2, tp: 0x0xfffffffffe10

prevClock: 0xffffffffffffffff
[1]trap: [8] unknown trap code: 34 at 0x2 with 0x2
```

## clock_gettimeのclk_idがCLOCK_PROCESS_CPUTIME_IDであった

p->stime, utimeを追加

```bash
$ /bin/timertest 2 0 1 0
       Elapsed   Value Interval
START:    0.00
prevClock: 0x0
[2]clock_gettime: [1000] clk: 2, tv_sec: 0, tv_nsec: 0
Main:     0.25    1.75    1.00
[2]clock_gettime: [32000] clk: 2, tv_sec: 1, tv_nsec: 0
Main:     1.25    0.75    1.00
[2]clock_gettime: [154000] clk: 2, tv_sec: 2, tv_nsec: 0
[2]trap: [8] unknown trap code: 34 at 0x2 with 0x2
$
```

##

```bash
$ /bin/timertest 2 0 1 0
try 3 times
       Elapsed   Value Interval
START:    0.00
[1]sys_setitimer: which: 0, new_value: inter->sec: 0x1, inter->usec: 0x0, val->sec: 0x2, val->usec: 0x0
[1]setitimer: interval: 0x64 (100), value: 0xc8 (200)
[1]setitimer: p[8]: it_real_value: 0xc8, real_incr: 0x64, expires: 0xc8
Main:     0.52    1.50    1.00
Main:     1.02    1.00    1.00
Main:     1.52    0.50    1.00
[1]trap: [8] unknown trap code: 34 at 0x2 with 0x2
$
```

## signal関係でデバッグ出力

```bash
$ /bin/timertest 2 0 1 0
try 3 times
       Elapsed   Value Interval
START:    0.00
Main:     0.49    1.51    1.00
Main:     0.99    1.01    1.00
Main:     1.49    0.51    1.00
Main:     1.99    0.01    1.00
[0]kill: pid: 8, sig: 14
[0]send_signal: pid=8, sig=14, state=4, paused=0
[0]send_signal: set sig 14 to pending 0x2000
[2]handle_signal: [8]: sig=14, handler=0x4002f0
[2]user_handler: sig=14
[2]user_handler: sp1: 0xfffffffffde0
[2]user_handler: sp2: 0xfffffffffcb0
[2]user_handler: sig_ret: start: 0xffff0000000a80f8, size: 0x8  // 16バイトアライメントされていない
[2]user_handler: sp3: 0xfffffffffca8
[2]user_handler: sp4: 0xfffffffffca0
[2]user_handler: tf->sp: 0xfffffffffca0, elf: 0x4002f0
ALARM:    2.03    0.97    1.00
Main:     2.52    0.48    1.00
[0]kill: pid: 8, sig: 14
[0]send_signal: pid=8, sig=14, state=4, paused=0
[0]send_signal: set sig 14 to pending 0x2000
[2]handle_signal: [8]: sig=14, handler=0x4002f0
[2]user_handler: sig=14
[2]user_handler: sp1: 0xfffffffffde0
[2]user_handler: sp2: 0xfffffffffcb0
[2]user_handler: sig_ret: start: 0xffff0000000a80f8, size: 0x8
[2]user_handler: sp3: 0xfffffffffca8
[2]user_handler: sp4: 0xfffffffffca0
[2]user_handler: tf->sp: 0xfffffffffca0, elf: 0x4002f0
ALARM:    3.04    0.96    1.00
Main:     3.05    0.95    1.00
Main:     3.55    0.45    1.00
[0]kill: pid: 8, sig: 14
[0]send_signal: pid=8, sig=14, state=4, paused=0
[0]send_signal: set sig 14 to pending 0x2000
[2]handle_signal: [8]: sig=14, handler=0x4002f0
[2]user_handler: sig=14
[2]user_handler: sp1: 0xfffffffffde0
[2]user_handler: sp2: 0xfffffffffcb0
[2]user_handler: sig_ret: start: 0xffff0000000a80f8, size: 0x8
[2]user_handler: sp3: 0xfffffffffca8
[2]user_handler: sp4: 0xfffffffffca0
[2]user_handler: tf->sp: 0xfffffffffca0, elf: 0x4002f0
ALARM:    4.04    0.96    1.00
That's all folks
$
[0]kill: pid: 8, sig: 14                               // SIGALRMがpendingから削除されていない
[0]send_signal: pid=8, sig=14, state=0, paused=0
[0]send_signal: set sig 14 to pending 0x2000
[0]kill: pid: 8, sig: 14
[0]send_signal: pid=8, sig=14, state=0, paused=0
[0]send_signal: sig 14 is already pending
[0]kill: pid: 8, sig: 14
[0]send_signal: pid=8, sig=14, state=0, paused=0
[0]send_signal: sig 14 is already pending
[0]kill: pid: 8, sig: 14
[0]send_signal: pid=8, sig=14, state=0, paused=0
[0]send_signal: sig 14 is already pending
[0]kill: pid: 8, sig: 14
[0]send_signal: pid=8, sig=14, state=0, paused=0
[0]send_signal: sig 14 is already pending
[0]kill: pid: 8, sig: 14
[0]send_signal: pid=8, sig=14, state=0, paused=0
[0]send_signal: sig 14 is already pending
[0]kill: pid: 8, sig: 14
[0]send_signal: pid=8, sig=14, state=0, paused=0
[0]send_signal: sig 14 is already pending
[0]kill: pid: 8, sig: 14
[0]send_signal: pid=8, sig=14, state=0, paused=0
[0]send_signal: sig 14 is already pending
[0]kill: pid: 8, sig: 14
[0]send_signal: pid=8, sig=14, state=0, paused=0
[0]send_signal: sig 14 is already pending
```

## デバッグ出力の有無で結果がかわる

- アプリケーション終了後もSIGALRMが発生する問題は`exit()`仮数でtimerを削除することで
  解決したと思われる。

- ある場合は正常終了

```bash
$ /bin/timertest 2 0 1 0
try 3 times
       Elapsed   Value Interval
START:    0.00
Main:     0.50    1.50    1.00
Main:     1.00    1.00    1.00
Main:     1.50    0.50    1.00
[3]user_handler: tf->sp: 0xfffffffffc90, elf: 0x4002f0
ALARM:    2.00    1.00    1.00
Main:     2.00    1.00    1.00
Main:     2.51    0.49    1.00
[3]user_handler: tf->sp: 0xfffffffffc90, elf: 0x4002f0
ALARM:    3.00    1.00    1.00
Main:     3.01    0.99    1.00
Main:     3.51    0.49    1.00
[3]user_handler: tf->sp: 0xfffffffffc90, elf: 0x4002f0
ALARM:    4.00    1.00    1.00
That's all folks
```

- ない場合はsigalrm発生でストール

```
$ /bin/timertest 2 0 1 0
try 3 times
       Elapsed   Value Interval
START:    0.00
Main:     0.50    1.50    1.00
Main:     1.00    1.00    1.00
Main:     1.50    0.50    1.00
                                    // ここでストール
```

## p->tf (trapframe)を出力

- debug出力無しでエラー発生

```bash
$ /bin/timertest 2 0 1 0
try 3 times
       Elapsed   Value Interval
START:    0.01
Main:     0.51    1.50    1.00
Main:     1.01    1.00    1.00
Main:     1.51    0.50    1.00
[3]trap: [8] unknown trap code: 34 at elr: 0x2 with far: 0x2
=== dump trapframe        ===
  spsr: 0x80000000
   elr: 0x2
    sp: 0xfffffffffcb0
 tpidr: 0x409160
    x1: 0x1
    x2: 0x0
    x3: 0x404180
    x8: 0x71
   x29: 0xfffffffffe20
   x30: 0x2
===-----------------------===
$
```

- debug出力ありでエラーなし

```bash
$ /bin/timertest 2 0 1 0
try 3 times
       Elapsed   Value Interval
START:    0.00
Main:     0.50    1.50    1.00
Main:     1.00    1.00    1.00
Main:     1.50    0.50    1.00
[1]user_handler: sig=14
[1]user_handler: sp1: 0xfffffffffde0
[1]user_handler: sp2: 0xfffffffffcb0
[1]user_handler: sig_ret: start: 0xffff0000000a8078, size: 0x8
[1]user_handler: sp3: 0xfffffffffca0
[1]user_handler: sp4: 0xfffffffffc90
[1]user_handler: tf->sp: 0xfffffffffc90, elf: 0x4002f0
=== dump trapframe        ===
  spsr: 0x80000000
   elr: 0x4002f0
    sp: 0xfffffffffc90
 tpidr: 0x409160
    x1: 0xfffffffffe10
    x2: 0x0
    x3: 0x404180
    x8: 0x71
   x29: 0xfffffffffe20
   x30: 0x402f54
===-----------------------===
ALARM:    2.04    0.96    1.00
Main:     2.05    0.95    1.00
Main:     2.54    0.46    1.00
[1]user_handler: sig=14
[1]user_handler: sp1: 0xfffffffffde0
[1]user_handler: sp2: 0xfffffffffcb0
[1]user_handler: sig_ret: start: 0xffff0000000a8078, size: 0x8
[1]user_handler: sp3: 0xfffffffffca0
[1]user_handler: sp4: 0xfffffffffc90
[1]user_handler: tf->sp: 0xfffffffffc90, elf: 0x4002f0
=== dump trapframe        ===
  spsr: 0x80000000
   elr: 0x4002f0
    sp: 0xfffffffffc90
 tpidr: 0x409160
    x1: 0xfffffffffe10
    x2: 0x0
    x3: 0x404180
    x8: 0x71
   x29: 0xfffffffffe20
   x30: 0x402f54
===-----------------------===
ALARM:    3.04    0.96    1.00
Main:     3.08    0.92    1.00
Main:     3.58    0.42    1.00
[1]user_handler: sig=14
[1]user_handler: sp1: 0xfffffffffde0
[1]user_handler: sp2: 0xfffffffffcb0
[1]user_handler: sig_ret: start: 0xffff0000000a8078, size: 0x8
[1]user_handler: sp3: 0xfffffffffca0
[1]user_handler: sp4: 0xfffffffffc90
[1]user_handler: tf->sp: 0xfffffffffc90, elf: 0x4002f0
=== dump trapframe        ===
  spsr: 0x80000000
   elr: 0x4002f0
    sp: 0xfffffffffc90
 tpidr: 0x409160
    x1: 0xfffffffffe10
    x2: 0x0
    x3: 0x404180
    x8: 0x71
   x29: 0xfffffffffe20
   x30: 0x402f54
===-----------------------===
ALARM:    4.04    0.96    1.00
That's all folks
$
```

## user_handler()を`q.lock`で挟むことでエラー回避

- `clock.c`, `fs/v6/fs.c#v6_iget()`, `bufcache.c#_find_free_memory()`, `proc.c#exit()`内の
  acquire, releaseも修正


```diff
diff --git a/kern/proc.c b/kern/proc.c
index 5dd208b..c3a7295 100644
--- a/kern/proc.c
+++ b/kern/proc.c
@@ -722,6 +723,8 @@ void stop_handler(struct proc *p)
 // ユーザハンドラを処理する
 void user_handler(struct proc *p, int sig)
 {
+    extern void dump_tf(struct trapframe *tf);
+    acquire(&q.lock);                              // これと
     trace("sig=%d", sig);
     uint64_t sp = p->tf->sp;
     trace("sp1: 0x%llx", sp);
@@ -730,7 +733,6 @@ void user_handler(struct proc *p, int sig)
     sp = ROUNDDOWN(sp, 0x10);
     memmove((void *)sp, (void *)p->tf, sizeof(struct trapframe));
     p->oldtf = (struct trapframe *)sp;
-    sp = ROUNDDOWN(sp, 0x10);
     trace("sp2: 0x%llx", sp);

     // sigret_syscall.Sのコードをユーザスタックにプッシュする
@@ -760,7 +762,8 @@ void user_handler(struct proc *p, int sig)

     // ユーザハンドラを実行するようにeipを変更する
     p->tf->elr = (uint64_t)p->signal.actions[sig].sa_handler;
-    debug("tf->sp: 0x%llx, elf: 0x%llx", p->tf->sp, p->tf->elr);
+    //dump_tf(p->tf);
+    release(&q.lock);                              // これ
 }
```

```bash
$ /bin/timertest 2 0 1 0
try 3 times
       Elapsed   Value Interval
START:    0.01
Main:     0.50    1.51    1.00
Main:     1.00    1.01    1.00
Main:     1.50    0.51    1.00
Main:     2.00    0.01    1.00
ALARM:    2.01    1.00    1.00
Main:     2.50    0.51    1.00
Main:     3.00    0.01    1.00
ALARM:    3.01    1.00    1.00
Main:     3.50    0.51    1.00
Main:     4.00    0.01    1.00
ALARM:    4.01    1.00    1.00
That's all folks
$
```

- `$`プロンプトが出てる状態でraspiを切り、再度立ち上げた場合のlockエラーもなくなった。
- sigtest/2/3も正常終了

```bash
$ /bin/sigtest
PID 13 caught sig 2, j 3
PID 12 caught sig 2, j 2
13 is dead
PID 11 caught sig 2, j 1
12 is dead
PID 10 caught sig 2, j 0
11 is dead
PID 9 caught sig 2, j -1
10 is dead
9 is dead
$ /bin/sigtest2
part1 start
PID 14 function A got 10
PID 14 function A got 999
PID 14 function B got 10

part2 start
PID 14 sends signal to PID 15
PID 15 function C got 10
PID 15 got signal and sends signal to PID 14
PID 14 function C got 10
PID 14 got signal from PID 15

part3 start
PID 16 function D got 10
PID 16 function E got 12
bye bye

all ok
$ /bin/sigtest3
Got signal!
$ /bin/timertest 2 0 1 0
try 3 times
       Elapsed   Value Interval
START:    0.00
Main:     0.50    1.50    1.00
Main:     1.00    1.00    1.00
Main:     1.50    0.50    1.00
ALARM:    2.00    1.00    1.00
Main:     2.00    1.00    1.00
Main:     2.50    0.50    1.00
ALARM:    3.00    1.00    1.00
Main:     3.00    1.00    1.00
Main:     3.50    0.50    1.00
ALARM:    4.00    1.00    1.00
That's all folks
$
```
