# cachepage機能を実装

- ファイルの内容をキャッシュする

## buddy_init()でストール

```bash
[0]buddy_init: end: 0xffff0000002ebd68, space: 0x514298
1234

end 	0xffff0000002ebd68
start 	0xffff000000800000

PAGENUM 0x3b9c0
```

## pagesのアドレスがおかしい

- cachepage機能導入後

```bash
[0]buddy_init: [B] pages: 0xffff000000269600 = 0xffff000000800000 - 0x18 * 0x3b9c0
[0]buddy_init: [A] pages: 0x0 = 0xffff000000800000 - 0x18 * 0x3b9c0

[0]buddy_init: pages[0]: 0x0
[0]buddy_init: pages[1]: 0x18
[0]buddy_init: pages[2]: 0x30
[0]buddy_init: pages[3]: 0x48
[0]buddy_init: pages[4]: 0x60
```

- cachepage機能導入前

```bash
[0]buddy_init: [B] pages: 0xffff000000269600 = 0xffff000000800000 - 0x18 * 0x3b9c0
[0]buddy_init: [A] pages: 0xffff000000269600 = 0xffff000000800000 - 0x18 * 0x3b9c0
[0]buddy_init: pages[0]: 0xffff000000269600
[0]buddy_init: pages[1]: 0xffff000000269618
[0]buddy_init: pages[2]: 0xffff000000269630
[0]buddy_init: pages[3]: 0xffff000000269648
[0]buddy_init: pages[4]: 0xffff000000269660
```

### cachepages[]の領域によりpagesが領域がbuf.0以降の領域を壊していた

```bash
ffff0000001bfe08 B cachepage		// 0xffff000000269600
ffff0000002bfe20 b buf.0
ffff0000002c0020 B pages
ffff0000002ebd68 B end
```

## 以下の修正でbuddy_init()はパスしたが

- cachepagesをslabで個別に用意
- buddy_init()はok
- cachepage_init()でストール

```bash
[0]console_preinit: console_preinit ok
[0]buddy_init: [B] pages: 0xffff000000269600 = 0xffff000000800000 - 0x18 * 0x3b9c0
[0]buddy_init: [A] pages: 0xffff000000269600 = 0xffff000000800000 - 0x18 * 0x3b9c0
[0]buddy_init: pages[0]: 0xffff000000269600
[0]buddy_init: pages[1]: 0xffff000000269618
[0]buddy_init: pages[2]: 0xffff000000269630
[0]buddy_init: pages[3]: 0xffff000000269648
[0]buddy_init: pages[4]: 0xffff000000269660
1aaa2[0]buddy_init: free_lists[10].head: 0xffff000000269600
```

## cachepages.pages[]を0x100個のリストのHashとした

- hash関数は`CPHASH(dev, ino) ((uint32_t)(((uint64_t)(dev + ino)) % CPSIZE))`で同一ファイルは同一リストに
  なるようにした。
- mmapしないプログラムは正常に動くようになった

```bash
[0]console_preinit: console_preinit ok
[0]buddy_init: buddy_init ok
[0]slab_cache_init: slab_cache_init ok
[0]cachepage_init: cachepage_init ok
[0]rand_init: rand_init ok

$ /bin/ls
drwxrwxr-x    1 root wheel  4096  1 16 17:41 .
drwxrwxr-x    1 root wheel  4096  1 16 17:41 ..
drwxrwxr-x    2 root wheel  1344  1 16 17:41 bin
drwxrwxr-x    3 root wheel   192  1 16 17:41 dev
drwxr-xr-x    0 root wheel     0  1 17 10:14 proc
-rwxr-xr-x    6 root wheel    34  1 16 17:41 test.txt
$ /bin/mmaptest2
[F-01] 不正なfdを指定した場合のテスト
 error: Bad file descriptor
 error: Bad file descriptor
 error: Bad file descriptor
[F-01] ok

[F-02] 不正なフラグを指定した場合のテスト
[2]sys_mmap: invalid flags: 0x0
 error: Invalid argument
[F-02] ok

[F-03] readonlyファイルにPROT_WRITEな共有マッピングをした場合のテスト
[2]sys_mmap: file is not writable
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
[3]get_cachepage: alloc new cachepage[0]: dev: 0x1a, ino=0, offset=0xffff0000000b0410
[F-09] ok

[F-10] ファイルが背後にある共有マッピングのテスト	// ストール
```

- F-10のみテスト

```
$ /bin/mmaptest2
[2]syscall1: proc[8] sys_gettid called
[2]syscall1: proc[8] sys_brk called
[2]syscall1: proc[8] sys_brk called
[2]syscall1: proc[8] sys_execve called
[3]syscall1: proc[8] sys_gettid called
[3]syscall1: proc[8] sys_ioctl called
[3]syscall1: proc[8] sys_writev called

[3]syscall1: proc[8] sys_writev called
[F-10] ファイルが背後にある共有マッピングのテスト
[3]syscall1: proc[8] sys_openat called
[1]syscall1: proc[8] sys_write called
[1]syscall1: proc[8] sys_close called
[1]syscall1: proc[8] sys_openat called
[1]syscall1: proc[8] sys_read called
[1]syscall1: proc[8] sys_mmap called
[1]get_cachepage: alloc new cachepage[0]: dev: 0x1a, ino=0, offset=0xffff0000000b0490
[1]syscall1: proc[8] sys_munmap called
[1]syscall1: proc[8] sys_openat called
[1]syscall1: proc[8] sys_read called
[1]syscall1: proc[8] sys_close called
[1]syscall1: proc[8] sys_close called
[1]syscall1: proc[8] sys_writev called
[F-10] ok
[1]syscall1: proc[8] sys_writev called

[1]syscall1: proc[8] sys_writev called
file_test:  ok: 1, ng: 0
[1]syscall1: proc[8] sys_writev called
anon_test:  ok: 0, ng: 0
[1]syscall1: proc[8] sys_writev called
other_test: ok: 0, ng: 0
[1]syscall1: proc[8] sys_unlinkat called
[1]syscall1: proc[8] sys_exit_group called
```

- F-09, F-10を続けて実行

```bash
[F-09] ファイルが背後にあるプライベートマッピング
[1]syscall1: proc[8] sys_openat called
[1]syscall1: proc[8] sys_write called
[3]syscall1: proc[8] sys_close called
[3]syscall1: proc[8] sys_openat called
[3]syscall1: proc[8] sys_read called
[3]syscall1: proc[8] sys_mmap called
[3]get_cachepage: alloc new cachepage[0]: dev: 0x101, ino=26, offset=0x0
[3]syscall1: proc[8] sys_munmap called
[3]syscall1: proc[8] sys_openat called
[3]syscall1: proc[8] sys_read called
[3]syscall1: proc[8] sys_close called
[3]syscall1: proc[8] sys_close called
[3]syscall1: proc[8] sys_writev called
[F-09] ok
[3]syscall1: proc[8] sys_writev called

[3]syscall1: proc[8] sys_writev called
[F-10] ファイルが背後にある共有マッピングのテスト
[3]syscall1: proc[8] sys_openat called
[3]syscall1: proc[8] sys_write called
[3]syscall1: proc[8] sys_close called
[3]syscall1: proc[8] sys_openat called
[3]syscall1: proc[8] sys_read called
[3]syscall1: proc[8] sys_mmap called		// ここでストール
```

- mmap.cの修正をチェック
- cachepageの作成で行っているfile-offsetの変更の是非をチェック
- MAP_SHAREとMAP_PRIVATEの違いをチェック

## fork()のuvm_copy()の間にデータ消失

- F-12が成功の場合

```bash
[F-12] ファイルが背後にあるプライベートマッピングのforkのテスト
[3]v6_write: called: file: 26, offset: 0x0, buffer: 0x410970, size: 0x10c8
[3]find_cachepage: found: cpque[27]->cpage: 0xffff000000bfd0f0
[3]update_cachepage: update_page: memmove from 0x410970 to 0xffff000000bfc000 with 0x1000 bytes
[3]find_cachepage: found: cpque[28]->cpage: 0xffff000000bfd138
[3]update_cachepage: update_page: memmove from 0x411970 to 0xffff000000baf000 with 0xc8 bytes
[3]mmap: addr: 0x0, length: 0xc8, prot: 0x3, flags: 0x2, f: 26, off: 0x0
[3]mmap_load_pages: addr: 0xffdfffff0000, length: 0xc8, prot: 0x3, flags: 0x2, f: 26, offset: 0x0
[3]map_file_pages: called: addr: 0xffdfffff0000, length: 0xc8, perm: 0x747, f_ino: 26, f_size: 0x10c8, offset: 0x0
[3]find_cachepage: found: cpque[27]->cpage: 0xffff000000bfd0f0
[3]map_file_pages: cpage->page: 0xffff000000bfc000, V2P(cpage->page): 0xbfc000, ino: 26, dev: 0x101, offset: 0x0, mapsize: 0xc8
[3]map_file_pages: cpage->page[49-50]: 0x6161                       // fork()前のデータは'aa'
[3]mmap: addr[49-50]: 0x6161
before   fork: ret: 0xffdfffff0000, ret[49-50]: 0x6161, buf[49-50]: 0x6161
[3]fork: call copy_vmas: p: 8, np: 9
[3]fork: call uvm_copy: pgdir: 0xffff000000b76000
[3]uvm_copy: [0] pa: 0xbfd000, P2V(pa): 0xffff000000bfd000, pa[49-50]: 0x0000
[3]uvm_copy: [i=0] pa: 0xbfd000, P2V(pa): 0xffff000000bfd000, pa[49-50]: 0x0000
[3]uvm_copy: [i1=0] pa: 0xbfd000, P2V(pa): 0xffff000000bfd000, pa[49-50]: 0x0000
[3]uvm_copy: [i2=0] pa: 0xbfd000, P2V(pa): 0xffff000000bfd000, pa[49-50]: 0x0200
[3]uvm_copy: [i=511] pa: 0xbfd000, P2V(pa): 0xffff000000bfd000, pa[49-50]: 0x0000
[3]uvm_copy: [i1=383] pa: 0xbfd000, P2V(pa): 0xffff000000bfd000, pa[49-50]: 0x0000
[3]uvm_copy: [i2=0] pa: 0xbfd000, P2V(pa): 0xffff000000bfd000, pa[49-50]: 0x1ff00
[3]uvm_copy: pa: page->ref: 2
[3]uvm_copy: va: 0xffdfffff0000, P2V(pa): 0xffff000000bfc000, np: 0xffff000000bb0000, share: no
[3]uvm_copy: pa[49-50]: 0x6161                                      // uvm_copy()でコピーするデータも'aa'
[3]uvm_copy: np[49-50]: 0x6161
[3]uvm_copy: [i1=511] pa: 0xbfd000, P2V(pa): 0xffff000000bfd000, pa[49-50]: 0x0000
[3]uvm_copy: [i2=0] pa: 0xbfd000, P2V(pa): 0xffff000000bfd000, pa[49-50]: 0x1ff00
parent before: ret: 0xffdfffff0000, ret[49-50]: 0x6161, buf[49-50]: 0x6161
child   after: ret: 0xffdfffff0000, ret[49-50]: 0x6e61, buf[49-50]: 0x6161

[2]munmap: called addr: 0xffdfffff0000, length: 0x1000
[1]munmap: called addr: 0xffdfffff0000, length: 0xc8
[F-12] ok
```

- F-12が失敗する場合

```bash
[F-12] ファイルが背後にあるプライベートマッピングのforkのテスト
[2]v6_write: called: file: 26, offset: 0x0, buffer: 0x410970, size: 0x10c8
[2]find_cachepage: found: cpque[27]->cpage: 0xffff000000b480f0
[2]update_cachepage: update_page: memmove from 0x410970 to 0xffff000000bfd000 with 0x1000 bytes
[2]find_cachepage: found: cpque[28]->cpage: 0xffff000000b48138
[2]update_cachepage: update_page: memmove from 0x411970 to 0xffff000000bfa000 with 0xc8 bytes
[2]mmap: addr: 0x0, length: 0xc8, prot: 0x3, flags: 0x2, f: 26, off: 0x0
[2]mmap_load_pages: addr: 0xffdfffff0000, length: 0xc8, prot: 0x3, flags: 0x2, f: 26, offset: 0x0
[2]map_file_pages: called: addr: 0xffdfffff0000, length: 0xc8, perm: 0x747, f_ino: 26, f_size: 0x10c8, offset: 0x0
[2]find_cachepage: found: cpque[27]->cpage: 0xffff000000b480f0
[2]map_file_pages: cpage->page: 0xffff000000bfd000, V2P(cpage->page): 0xbfd000, ino: 26, dev: 0x101, offset: 0x0, mapsize: 0xc8
[2]map_file_pages: cpage->page[49-50]: 0x6161
[2]mmap: addr[49-50]: 0x6161
before   fork: ret: 0xffdfffff0000, ret[49-50]: 0x6161, buf[49-50]: 0x6161      // fork()前のデータは'aa'

[2]fork: call copy_vmas: p: 8, np: 9
[2]fork: call uvm_copy: pgdir: 0xffff000000b77000
[2]uvm_copy: [0] pa: 0xbfd000, P2V(pa): 0xffff000000bfd000, pa[49-50]: 0x6161   // uvm_copy()の処理開始前も'aa'
[2]uvm_copy: [i3=496] pa: 0xbfd000, P2V(pa): 0xffff000000bfd000, pa[49-50]: 0x0000
[2]uvm_copy: [4] pa: 0xbfd000, P2V(pa): 0xffff000000bfd000, pa[49-50]: 0x0000
[2]uvm_copy: pa: 0xbfd000, va: 0xffdfffff0000
[2]uvm_copy: pa: page->ref: 2
[2]uvm_copy: va: 0xffdfffff0000, P2V(pa): 0xffff000000bfd000, np: 0xffff000000bb1000, share: no
[2]uvm_copy: pa[49-50]: 0x0000                                                  // uvm_copy()でコピーするデータが'\0\0'
[2]uvm_copy: np[49-50]: 0x0000
parent before: ret: 0xffdfffff0000, ret[49-50]: 0x0000, buf[49-50]: 0x6161child   after: ret: 0xffdfffff0000, ret[49-50]: 0x6e00, buf[49-50]: 0x6161

[3]munmap: called addr: 0xffdfffff0000, length: 0x1000
[3]kfree: page->ref: 2
parent after : ret: 0xffdfffff0000, ret[49-50]: 0x0000, buf[49-50]: 0x6161
[F-12] failed at strcmp parent
[3]munmap: called addr: 0xffdfffff0000, length: 0xc8
[3]kfree: page->ref: 1
```
