# VFS機能の実装

- minixをベースとした[gloworm OS](https://jabberwocky.ca/projects/gloworm/)の
  VFS機能を導入した。
- 以下の3層構造で各ファイルシステムは任意の独自データ構造体と専用の操作関数を提供する
- api操作関数は引数を取り込んだ後、vfsの処理関数に処理を依頼する。vfs処理関数は必要に応じて
  ファイルシステムの操作関数を呼び出して処理を行う
    - mount構造体とmount操作関数構造体
    - vnode構造体とvnode操作関数構造体
    - vfile構造体とvfile操作関数構造体
- デバイスとドライバがより明確になり、inodeから分離された。

## v6ファイルシステムの既存処理がvfs機能を使って実行できるようになった

```bash
$ /bin/ls
drwxrwxr-x    1 root wheel  4096 12 29 11:19 .
drwxrwxr-x    1 root wheel  4096 12 29 11:19 ..
drwxrwxr-x    2 root wheel   960 12 29 11:19 bin
drwxrwxr-x    3 root wheel   192 12 29 11:19 dev
-rwxr-xr-x    5 root wheel    34 12 29 11:19 test.txt
$ /bin/grep abc test.txt
abcdef
$ /bin/grep abc test.txt | /bin/wc
1 1 7
$ /bin/date
2025年12日24日 水曜日 17時44分37秒 JST
$ cd /bin
$ echo abc > test2.txt
$ ls
drwxrwxr-x    2 root wheel  1024 12 29 11:19 .
drwxrwxr-x    1 root wheel  4096 12 29 11:19 ..
-rwxr-xr-x    6 root wheel 38568 12 29 11:19 cat
-rwxr-xr-x    7 root wheel 22400 12 29 11:19 init
-rwxr-xr-x    8 root wheel 39480 12 29 11:19 echo
-rwxr-xr-x    9 root wheel 44184 12 29 11:19 ifconfig
-rwxr-xr-x   10 root wheel 49368 12 29 11:19 date
-rwxr-xr-x   11 root wheel 38896 12 29 11:19 wc
-rwxr-xr-x   12 root wheel 40224 12 29 11:19 grep
-rwxr-xr-x   13 root wheel 54664 12 29 11:19 sh
-rwxr-xr-x   14 root wheel 17744 12 29 11:19 utest
-rwxr-xr-x   15 root wheel 53064 12 29 11:19 ls
-rwxr-xr-x   16 root wheel 39032 12 29 11:19 udpecho
-rwxr-xr-x   17 root wheel 11056 12 29 11:19 dns
-rwxr-xr-x   18 root wheel 40616 12 29 11:19 tcpecho
-rwxr-xr-x   19 root wheel     4 12 29 16:17 test2.txt
$ cat test2.txt
abc
```
