# mmapの実装

## mmaptest

```bash
$ /bin/mmaptest
mmap_testスタート
- open mmap.dur (fd=3) READ ONLY
[1] test mmap f
- mmap fd=3 -> 2ページ分PROT_READ MAP_PRIVATEでマップ: addr=0xffdfffff0000
- munmmap 2PAGE: addr=0xffdfffff0000
[1] OK
[2] test mmap private
- mmap fd=3 -> 2ページ分をPAGE PROT_READ/WRITE, MAP_PRIVATEでマップ: addr=0xffdfffff0000
- closed 3
- write 2PAGE = Z
- p[1]=Z
- munmmap 2PAGE: addr=0xffdfffff0000
[2] OK
[3] test mmap read-only
- open mmap.dur (3) RDONALY
[1]sys_mmap: file is not writable
- mmap 3 -> 3PAGE PROT_READ/WRITE MAP_SHARED: addr=0xffffffffffffffff
- close 3
[3] OK
[4] test mmap read/write
- open mmap.dur (3) RDWR
- mmap 3 -> 3PAGE PROT_READ|WRITE MAP_SHARED: addr=0xffdfffff0000
- close 3
- print 2PAGE = Z
- munmmap 2PAGE: addr=0xffdfffff0000
[4] OK
[5] test mmap dirty
- open mmap.dur (3) RDWR
- close 3
[5] OK
[6] test not-mapped unmap
- munmmap PAGE: addr=0xffdfffff2000
[6] OK
[7] test mmap two files                 // ここでストール
```

## 1月9日

- get_fd()のバグ修正(table[fd]がNULLの場合にtable[fd]->vnodeを参照)
- fileサイズが0の場合、map_file_pages()でuvm_map()されない。サイズが0の場合は0詰めの1ページをuvm_map()
- wait4()を実装（fork()がまともに動かなかった）
- mmaptestは変わらず([7]でストール)

```bash
$ /bin/mmaptest2
[F-13] file backed shared mapping with fork test
[3]sys_clone: flags 0x11, child stack 0x0x0
[3]fork: 'mmaptest2'(8) fork ''(10)
[3]sys_wait4: [-1] pid: -612, status: 0x0, options: 0x0, rusage: 0x8
[F-13] strcmp parent

[F-18] ２つのマッピングの間のアドレスを指定したマッピングのテスト
[F-18] expect: 0xffdfffff1000, mapped: 0x600000000000

[A-02] anonymous shared mapping test
[2]sys_clone: flags 0x11, child stack 0x0x0
[2]fork: 'mmaptest2'(8) fork ''(11)
[2]sys_wait4: [-1] pid: -228, status: 0x0, options: 0x0, rusage: 0x8
[A-02] ng: p1[1]: 0 != 1

[A-04] anonymous shared mapping with multiple forks test
[2]sys_clone: flags 0x11, child stack 0x0x0
[2]fork: 'mmaptest2'(8) fork ''(13)
[3]sys_clone: flags 0x11, child stack 0x0x0
[2]sys_wait4: [-1] pid: -1228, status: 0x0, options: 0x0, rusage: 0x8
[3]fork: ''(13) fork ''(14)
[1]sys_clone: flags 0x11, child stack 0x0x0
[3]sys_wait4: [-1] pid: -1228, status: 0x0, options: 0x0, rusage: 0x8
[1]fork: ''(14) fork ''(15)
[1]sys_wait4: [-1] pid: -1228, status: 0x0, options: 0x0, rusage: 0x8
[A-04] failed at strcmp fork 1 parent

[A-05] anonymous private & shared mapping together with fork test
[0]sys_clone: flags 0x11, child stack 0x0x0
[0]fork: 'mmaptest2'(8) fork ''(16)
[0]sys_wait4: [-1] pid: -628, status: 0x0, options: 0x0, rusage: 0x8
[A-05] failed at strcmp share

[A-13] anonymous intermediate provided address test
[A-13] expect: 0x600000001000, mapped: 0x600000000000

[O-04] mmap valid address map fixed flag test
[0]is_usable: addr: 0x600000001000 is invalid
[0]mmap: addr 0x600000001000 is not available
[O-04] failed at mmap

file_test:  ok: 19, ng: 2
anon_test:  ok: 9, ng: 4
other_test: ok: 4, ng: 1
```

## 1月10日

- wait4()を実装
- mmaptest2に直書きされていたMMAPBASE値を変更
- mmapの各関数の最後にdsb()を追加

```bash
$ /bin/mmaptest2
[F-13] file backed shared mapping with fork test
child : ret2[0, 1, 49, 50]=[o, o, o, a], buf=[a, a, a, a]
parent: ret2[0, 1, 49, 50]=[a, a, a, a], buf=[a, a, a, a]
[F-13] strcmp parent

[A-02] anonymous shared mapping test
[0]sys_msync: addr: 0xffdfffff0000, length: 0x2710, flags: 0x1
[A-02] p1[1]: 0 != 1

[A-04] anonymous shared mapping with multiple forks test
[A-04] failed at strcmp fork 1 parent

[A-05] anonymous private & shared mapping together with fork test
[A-05] failed at strcmp share

file_test:  ok: 20, ng: 1
anon_test:  ok: 10, ng: 3
other_test: ok: 5, ng: 0
```

- 共有マッピングでforkした子の行った変更が親に見えない?
- writebackはmunmapしないと行われないのが原因か? mac-rpis-osを要チェック

```bash
[F-13] file backed shared mapping with fork test
child : ret2[0, 1, 49, 50]=[o, o, o, a], buf=[a, a, a, a]
[3]munmap: writeback: f: 23, addr: 0xffdfffff0000, size: 0x1000
parent: ret2[0, 1, 49, 50]=[a, a, a, a], buf=[a, a, a, a]
[F-13] strcmp parent
[3]munmap: writeback: f: 23, addr: 0xffdfffff0000, size: 0x1000

[F-14] オフセットを指定したプライベートマッピングのテスト
[F-14] failed at read 2, n=4296

[A-02] anonymous shared mapping test
[A-02] parent: p1[1]: 0 != 1

[A-04] anonymous shared mapping with multiple forks test
[A-04] pid2 child ok
[A-04] pid3 child ok
[A-04] pid3 parent ok
[A-04] pid2 parent ok
[A-04] failed at strcmp pid parent: data[0,  1000] = [r, @] != p1[0, 1000] = [, ]

[A-05] anonymous private & shared mapping together with fork test
[2]munmap: no vma with addr 0xffdfffff0000
[A-05] failed at strcmp share]: data2[0,  200] = [r, @] != p2[0, 200] = [, ]

file_test:  ok: 19, ng: 2
anon_test:  ok: 10, ng: 3
other_test: ok: 5, ng: 0
```

- file->offsetがおかしくなってF-14がエラーになるようになった

## 1月11日

- vm_freeでret2のpaを削除しているかららしい(rpi-os作業メモの13.1 mmaptestでエラー発生より)
- kallocで取得するメモリに参照カウンタを導入した
- user_init()でpanic

```bash
[0]uvm_map: va: 0x0, size: 0x1000, pa: 0x800000
[0]uvm_map: p: 0x0, pa: 0x800000, end: 0x1000
[0]pgdir_walk: pgt[0]: 0x800003, PTE_ADDR: 0x800000, P2V: 0xffff000000800000    // idx値が変わらない
[0]pgdir_walk: pgt[0]: 0x800003, PTE_ADDR: 0x800000, P2V: 0xffff000000800000
[0]pgdir_walk: pgt[0]: 0x800003, PTE_ADDR: 0x800000, P2V: 0xffff000000800000
[0]pgdir_walk: ret: va: 0x0, &pgt: 0xffff000000800000, pgt: 0x800003
[0]uvm_map: remap: pte: 0xffff000000800000, *pte: 0x800003
remap.
kern/drivers/console.c:265: kernel panic at cpu 0.
```

## 1月12日

- 前回のコミットに戻し、段階的に修正を適用した結果、mmaptest2のすべてのテストをクリア

```bash
$ /bin/mmaptest2
[F-01] 不正なfdを指定した場合のテスト
 error: Bad file descriptor
 error: Bad file descriptor
 error: Bad file descriptor
[F-01] ok

[F-02] 不正なフラグを指定した場合のテスト
[1]sys_mmap: invalid flags: 0x0
 error: Invalid argument
[F-02] ok

[F-03] readonlyファイルにPROT_WRITEな共有マッピングをした場合のテスト
[1]sys_mmap: file is not writable
 error: Permission denied
[F-03] ok

[F-04] readonlyファイルにPROT_READのみの共有マッピングをした場合のテスト
[F-04] ok

[F-05] PROT指定の異なるプライベートマッピンのテスト
[F-05] ok

[F-06] MMAPTOPを超えるサイズをマッピングした場合のテスト
[F-06] ok

[F-07] 連続したマッピングを行うテスト
[F-07] ok

[F-08] 空のファイルを共有マッピングした場合のテスト
[F-08] ok

[F-09] ファイルが背後にあるプライベートマッピング
[F-09] ok

[F-10] ファイルが背後にある共有マッピングのテスト
[F-10] ok

[F-11] file backed mapping pagecache coherency test
[2]munmap: length 0x2000 is bigger than vma->length 0x1000
[F-11] ok

[F-12] file backed private mapping with fork test
[F-12] ok

[F-13] file backed shared mapping with fork test
buf2[0]=0x6f6f6f6f6f6f6f6f
[F-13] ok

[F-14] オフセットを指定したプライベートマッピングのテスト
[F-14] ok

[F-15] file backed valid provided address test
[F-15] ok

[F-16] file backed invalid provided address test
[F-16] ok

[F-17] 指定されたアドレスが既存のマッピングアドレスと重なる場合
[F-17] ok

[F-18] ２つのマッピングの間のアドレスを指定したマッピングのテスト
[F-18] ok

[F-19] ２つのマッピングの間に不可能なアドレスを指定した場合
[F-19]  ok

[F-20] 共有マッピングでファイル容量より大きなサイズを指定した場合
[F-20] ok

[F-21] write onlyファイルへのREAD/WRITE共有マッピングのテスト
[2]sys_mmap: file is not readable
[F-21] ok

[A-01] anonymous private mapping test
p[0]=0, p[2499]=2499
[A-01] ok

[A-02] anonymous shared mapping test
[A-02] ok

[A-03] anonymous private mapping with fork test
[A-03] ok

[A-04] anonymous shared mapping with multiple forks test
[A-04] ok

[A-05] anonymous private & shared mapping together with fork test
[A-05] ok

[A-06] anonymous missing flags test
[3]sys_mmap: invalid flags: 0x20
[A-06] ok

[A-07] anonymous exceed mapping count test
[A-07] ok

[A-08] anonymous exceed mapping size test
[A-08] ok

[A-09] anonymous zero size mapping test
[3]sys_mmap: invalid length: 0 or offset: 0
[A-09] ok

[A-10] anonymous valid provided address test
[A-10] ok

[A-11] anonymous invalid provided address test
[A-11] ok: not running because of an invalid test

[A-12] anonymous overlapping provided address test
[A-12] ok

[A-13] anonymous intermediate provided address test
[A-13] ok

[A-14] anonymous intermediate provided address not possible test
[A-14] ok

[O-01] munmap only partial size test
[3]munmap: length 0x3000 is bigger than vma->length 0x2000
[O-01] ok

[O-02] write on read only mapping test
[O-02] ok

[O-03] none permission on mapping test
[O-03] ok

[O-04] mmap valid address map fixed flag test
[O-04] ok

[O-05] mmap invalid address map fixed flag test
[2]mmap: fixed address should be page align: 0xffdfffff0100
[2]is_usable: addr: 0xffdfffff0000 is used
[2]mmap: addr 0xffdfffff0000 is not available
[O-05] test ok

file_test:  ok: 21, ng: 0
anon_test:  ok: 13, ng: 0
other_test: ok: 5, ng: 0
```
