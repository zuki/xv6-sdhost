# シグナル機能の実装

## 変更したファイル

```bash
inc/proc.h
kern/exec.c
kern/proc.c
kern/syscall.c
kern/sysproc.c
kern/trap.c
```

## 追加したファイル

```bash
inc/linux/ppoll.h
usr/src/sigtest/main.c
usr/src/sigtest2/main.c
usr/src/sigtest3/main.c
```

## 実行結果

-sigtest2, sigtest3はパスしたが、sigtest1が子プロセスがexit()せずストール
- いずれもmain()がreturn 0で終わるとプロンプトが戻らない。exiit()する必要がある。

```bash
$ /bin/sigtest2
part1 start
PID 8 function A got 10
PID 8 function A got 999
PID 8 function B got 10

part2 start
PID 8 sends signal to PID 9
PID 9 function C got 10
PID 9 got signal and sends signal to PID 8
PID 8 function C got 10
PID 8 got signal from PID 9

part3 start
PID 10 function D got 10
PID 10 function E got 12
bye bye

all ok
$ /bin/sigtest3
Got signal!
$ /bin/sigtest          // ストール
```

## デバッグあり

```bash
$ /bin/sigtest
sig_catcher: Proc[0]: 9sig_catcher: Proc[1]: 10 ready
sig_catcher: Proc[2]: 11sig_catcher: Proc[3]: 12 ready
 ready
sig_catcher: Proc[4]: 13 ready
 ready
[2]sys_kill: pid=13, sig=2
[2]send_signal: pid=13, sig=2, state=2, paused=1
[2]send_signal: set sig 2 to pending 0x2
[2]handle_signal: [13]: sig=18, handler=0x0
[1]check_pending_signal: pid=13, sig=2
[1]handle_signal: [13]: sig=2, handler=0x400170
sig_catcher: Proc[3]: 13 caught sig 2, j 0
 - kill Proc[3]: 12 with sig 2
[1]sys_kill: pid=12, sig=2
[1]send_signal: pid=12, sig=2, state=2, paused=1
[1]send_signal: set sig 2 to pending 0x2
[1]handle_signal: [12]: sig=18, handler=0x0
[0]check_pending_signal: pid=12, sig=2
[0]handle_signal: [12]: sig=2, handler=0x400170
sig_catcher: Proc[2]: 12 caught sig 2, j 0
 - kill Proc[2]: 11 with sig 2
[0]sys_kill: pid=11, sig=2
[0]send_signal: pid=11, sig=2, state=2, paused=1
[0]send_signal: set sig 2 to pending 0x2
[0]handle_signal: [11]: sig=18, handler=0x0
[2]check_pending_signal: pid=11, sig=2
[2]handle_signal: [11]: sig=2, handler=0x400170
sig_catcher: Proc[1]: 11 caught sig 2, j 0
 - kill Proc[1]: 10 with sig 2
[2]sys_kill: pid=10, sig=2
[2]send_signal: pid=10, sig=2, state=2, paused=1
[2]send_signal: set sig 2 to pending 0x2
[2]handle_signal: [10]: sig=18, handler=0x0
[3]check_pending_signal: pid=10, sig=2
[3]handle_signal: [10]: sig=2, handler=0x400170
sig_catcher: Proc[0]: 10 caught sig 2, j 0
 - kill Proc[0]: 9 with sig 2
[3]sys_kill: pid=9, sig=2
[3]send_signal: pid=9, sig=2, state=2, paused=1
[3]send_signal: set sig 2 to pending 0x2
[3]handle_signal: [9]: sig=18, handler=0x0
    // 子プロセスがexit()しない

$ /bin/sigtest2
part1 start
[1]sys_kill: pid=8, sig=10
[1]send_signal: pid=8, sig=10, state=4, paused=0
[1]send_signal: set sig 10 to pending 0x200
[1]check_pending_signal: pid=8, sig=10
[1]handle_signal: [8]: sig=10, handler=0x0
[1]term_handler: pid: 8
[1]sys_kill: pid=8, sig=10
[1]send_signal: pid=8, sig=10, state=4, paused=0
[1]send_signal: set sig 10 to pending 0x200
[1]check_pending_signal: pid=8, sig=10
[1]handle_signal: [8]: sig=10, handler=0x4002a0
PID 8 function A got 10
PID 8 function A got 999
[1]sys_kill: pid=8, sig=10
[1]send_signal: pid=8, sig=10, state=4, paused=0
[1]send_signal: set sig 10 to pending 0x200
[1]check_pending_signal: pid=8, sig=10
[1]handle_signal: [8]: sig=10, handler=0x4002d0
PID 8 function B got 10

part2 start
PID 8 sends signal to PID 9
[1]sys_kill: pid=9, sig=10
[1]send_signal: pid=9, sig=10, state=3, paused=0
[1]send_signal: set sig 10 to pending 0x200
[1]sys_kill: pid=9, sig=10
[1]send_signal: pid=9, sig=10, state=3, paused=0
[1]send_signal: sig 10 is already pending
[2]check_pending_signal: pid=9, sig=10
[2]handle_signal: [9]: sig=10, handler=0x400300
PID 9 function C got 10
PID 9 got signal and sends signal to PID 8
[2]sys_kill: pid=8, sig=10
[2]send_signal: pid=8, sig=10, state=4, paused=0
[2]send_signal: set sig 10 to pending 0x200
[2]sys_exit_group: [9] sys_exit_group: '' exit with code 0
[1]check_pending_signal: pid=8, sig=10
[1]handle_signal: [8]: sig=10, handler=0x400300
PID 8 function C got 10
PID 8 got signal from PID 9

part3 start
[1]sys_kill: pid=10, sig=10
[1]send_signal: pid=10, sig=10, state=3, paused=0
[1]send_signal: set sig 10 to pending 0x200
[3]check_pending_signal: pid=10, sig=10
[3]handle_signal: [10]: sig=10, handler=0x400340
PID 10 function D got 10
[1]sys_kill: pid=10, sig=12
[1]send_signal: pid=10, sig=12, state=3, paused=0
[1]send_signal: set sig 12 to pending 0x800
[3]check_pending_signal: pid=10, sig=12
[3]handle_signal: [10]: sig=12, handler=0x400370
PID 10 function E got 12
bye bye
[3]sys_exit_group: [10] sys_exit_group: '' exit with code 0

all ok
[1]sys_exit_group: [8] sys_exit_group: 'sigtest2' exit with code 0

$ /bin/sigtest3
[3]sys_kill: pid=11, sig=15
[3]send_signal: pid=11, sig=15, state=4, paused=0
[3]send_signal: set sig 15 to pending 0x4000
[3]check_pending_signal: pid=11, sig=15
[3]handle_signal: [11]: sig=15, handler=0x4001a0
Got signal!
[3]sys_exit_group: [11] sys_exit_group: 'sigtest3' exit with code 0
$
```
