# symlinkの修正

- symlinkしたファイルがlsで読めない

```bash
$ /bin/ln test.txt test2.txt
$ /bin/ln -s test.txt test_s.txt
$ /bin/ls
drwxrwxr-x    1 root wheel  4096  1 24 09:06 .
drwxrwxr-x    1 root wheel  4096  1 24 09:06 ..
drwxrwxr-x    2 root wheel  1472  1 24 09:06 bin
drwxrwxr-x    3 root wheel   192  1 24 09:06 dev
drwxr-xr-x    0 root wheel     0  6 21 10:31 proc
drwxrwxrwx    6 root wheel   256  1 24 09:06 lib
-rwxr-xr-x    9 root wheel    34  1 24 09:06 test.txt
-rwxr-xr-x    9 root wheel    34  1 24 09:06 test2.txt      // ハードリンクしたファイルは読める
                                                            // ここでストール
```

## v6_symlink()にバグ発見

- ipを処理した後、sleeplockをreleaseしていなかった。
  そのため、別の関数でこのsleeplockをacquireしようとしてデッドロックとなっていた.
- symlinkしたファイルをお世ディレクトリに登録していなかった。
- symlinkしたもとのファイル名をファイルに書き込んでいなかった。

```diff
 int v6_symlink(struct vnode *parent, const char *target, const char *filename)
 {
     struct proc *p = thisproc();
     struct timespec ts;
     clock_gettime(CLOCK_REALTIME, &ts);
     struct v6_inode *ip = v6_ialloc(parent->mp, T_SYMLINK);
+    int err;

     v6_ilock(ip);
+    // 1. ipに対応するvnodeを初期化する
     vfs_init_vnode(&ip->vnode, &v6_vnode_ops, parent->mp, S_IFLNK|(0777 & ~p->umask), 1, p->uid, p->gid, parent->rdev, ITOV(ip)->ino, strlen(target), &ts, &ts, &ts);
+    ip->type = T_SYMLINK;
+    // 2. 親ディレクトリにこのエントリを追加
+    if (( err = v6_dirlink(VTOI(parent), filename, ITOV(ip)->ino, T_SYMLINK)) < 0) {
+        vfs_release_vnode(parent);
+        v6_iunlockput(ip);
+        return err;
+    }
+    vfs_release_vnode(parent);
+    // 3. このファイルの内容としてtarget名を書き込む
+    v6_writei(ip, target, 0, strlen(target));
     v6_iupdate(ip);
-    return v6_dirlink(VTOI(parent), filename, ITOV(ip)->ino, T_SYMLINK);
+    v6_iunlockput(ip);
+    return 0;
 }
```

## vfs_readlink()にバグ発見

- vnodeからvfileを取り出すのに`get_vnode(p->fd_table, vnode)`を使っていたが、これではプロセスpで
  対象のファイルをopenしていないと取り出せないことに気づく。
- vnodeでファイルをopenしてvfileを取り出す`get_file(vnode, flags)`関数を作成

```diff
# kern/fs/vfs.c

+static struct vfile *get_file(struct vnode *vnode, int flags)
+{
+    struct vfile *f;
+    int error;
+    if ((f = alloc_file(vnode, flags)) == NULL)
+        return NULL;
+    if ((error = f->ops->open(f, flags)) < 0) {
+        error("failed file open");
+        free_vfile(f);
+        return NULL;
+    } else {
+        return f;
+    }
+}

# in int vfs_readlink()
     file = get_vnode(thisproc()->fd_table, vnode);
     if (file) {
         error = file->ops->read(file, buf, bufsize);
+        //if (error > 0 && error < bufsize)
+        //    buf[error] = 0;
+        hexdump(buf, 16, "get_vnode buffer");
+    } else if (file = get_file(vnode, O_RDONLY)) {
+        error = file->ops->read(file, buf, bufsize);
+        //if (error > 0 && error < bufsize)
+        //    buf[error] = 0;
+        hexdump(buf, 16, "get_file buffer");
+        file->ops->close(file);
     } else {
         error = -ENOENT;
     }
```

## readlink()で取り出したsymlinkもとのファイル名にゴミが付いて読み込めない

- sys_fstatat(void)のローカル変数`char linkpath[512]`にゴミが残ったまま使用しているのが原因だった
- sys_fstatat()でvfs_readlink()でファイル名を取り出した後で終端処理をしているが、vfs_readlink()内で
  ゴミがついていた。
- linkpath[]を0クリアしてから使用することで解決できた

```bash
[2]sys_fstatat: dirfd: -100, path: ./test2.txt, flags: 0x0
[2]v6_dirlookup: dp->ino: 1, name: .
[2]v6_lookup: OK: ., ip->ino: 1, ip->type: 1
[2]v6_dirlookup: dp->ino: 1, name: test2.txt
[2]v6_lookup: OK: test2.txt, ip->ino: 31, ip->type: 5
[2]sys_fstatat: (1) path: ./test2.txt, vnode: 31
[2]v6_dirlookup: dp->ino: 1, name: test2.txt
[2]v6_lookup: OK: test2.txt, ip->ino: 31, ip->type: 5
=== vfs_readlink buffer dump ===
+------+-------------------------------------------------+------------------+
| 0000 | d0 bc 02 0b 00 00 ff ff 70 4e 09 00 00 00 ff ff | ........pN...... |
+------+-------------------------------------------------+------------------+

[2]v6_read: file->offset: 0
[2]v6_read: r: 8, file->offset: 8

+------+-------------------------------------------------+------------------+
| 0000 | 74 65 73 74 2e 74 78 74 70 4e 09 00 00 00 ff ff | text.txtpN...... |
+------+-------------------------------------------------+------------------+
[2]vfs_readlink: get_file: buf=test.txtpN                   // ゴミ 'pN.' がついている
[2]sys_fstatat: name: test2.txt, linkpath: test.txtpN
[2]v6_dirlookup: dp->ino: 1, name: test.txtpN
[2]v6_dirlookup: no entry found by name: test.txtpN
[2]sys_fstatat: vfs_lookup error (-2): vnode: 31, linkpath: test.txtpN
ls: cannot get buf's stat ./test2.txt
```

## 修正版で実行

- `usr/bin/ls`で各行の情報取得は`stat(buf, &st)`を使用

```bash
$ /bin/ls
drwxrwxr-x    1 root wheel  4096  1 24 16:31 .
drwxrwxr-x    1 root wheel  4096  1 24 16:31 ..
drwxrwxr-x    2 root wheel  1472  1 24 16:31 bin
drwxrwxr-x    3 root wheel   192  1 24 16:31 dev
drwxr-xr-x    0 root wheel     0  6 21 10:31 proc
drwxrwxrwx    6 root wheel   256  1 24 16:31 lib
-rwxr-xr-x    9 root wheel    34  1 24 16:31 test.txt
$ /bin/ln test.txt test_h.txt
$ /bin/ln -s test.txt test_s.txt
$ /bin/ls
drwxrwxr-x    1 root wheel  4096  1 24 16:31 .
drwxrwxr-x    1 root wheel  4096  1 24 16:31 ..
drwxrwxr-x    2 root wheel  1472  1 24 16:31 bin
drwxrwxr-x    3 root wheel   192  1 24 16:31 dev
drwxr-xr-x    0 root wheel     0  6 21 10:31 proc
drwxrwxrwx    6 root wheel   256  1 24 16:31 lib
-rwxr-xr-x    9 root wheel    34  1 24 16:31 test.txt
-rwxr-xr-x    9 root wheel    34  1 24 16:31 test_h.txt
-rwxr-xr-x    9 root wheel    34  1 24 16:31 test_s.txt     // symlinkをたどって元ファイルの情報(ino, mode)が表示されている
```

- 情報取得に`fstatat(AT_FDCWD, buf, &st, AT_SYMLINK_NOFOLLOW)`を使う

```bash
$ /bin/ls
drwxrwxr-x    1 root wheel  4096  1 25 10:13 .
drwxrwxr-x    1 root wheel  4096  1 25 10:13 ..
drwxrwxr-x    2 root wheel  1472  1 25 10:13 bin
drwxrwxr-x    3 root wheel   192  1 25 10:13 dev
drwxr-xr-x    0 root wheel     0  6 21 10:31 proc
drwxrwxrwx    6 root wheel   256  1 25 10:13 lib
-rwxr-xr-x    9 root wheel    34  1 25 10:13 test.txt
-rwxr-xr-x    9 root wheel    34  1 25 10:13 test_h.txt
lrwxrwxrwx   31 root wheel     8  6 21 10:31 test_s.txt     // symlinkしたファイルの情報が表示されている
```

- test_s.txt, procの日付がおかしいのはconfig.hでUSING_RASPIをundefしていたせい

```bash
$ /bin/date
2026年 1日25日 日曜日 11時 8分44秒 JST
$ /bin/ls
drwxrwxr-x    1 root wheel  4096  1 25 11:01 .
drwxrwxr-x    1 root wheel  4096  1 25 11:01 ..
drwxrwxr-x    2 root wheel  1472  1 25 11:01 bin
drwxrwxr-x    3 root wheel   192  1 25 11:01 dev
drwxr-xr-x    0 root wheel     0  1 25 11:08 proc
drwxrwxrwx    6 root wheel   256  1 25 11:01 lib
-rwxr-xr-x    9 root wheel    34  1 25 11:01 test.txt
$ /bin/ln -s test.txt test_s.txt
$ /bin/ls
drwxrwxr-x    1 root wheel  4096  1 25 11:01 .
drwxrwxr-x    1 root wheel  4096  1 25 11:01 ..
drwxrwxr-x    2 root wheel  1472  1 25 11:01 bin
drwxrwxr-x    3 root wheel   192  1 25 11:01 dev
drwxr-xr-x    0 root wheel     0  1 25 11:08 proc
drwxrwxrwx    6 root wheel   256  1 25 11:01 lib
-rwxr-xr-x    9 root wheel    34  1 25 11:01 test.txt
lrwxrwxrwx   31 root wheel     8  1 25 11:08 test_s.txt
```
