# 動的リンク機能の実装

```bash
Makefile
dyn/Makefile
dyn/src/hello-dyn/main.c
inc/exec.h
kern/exec.c
mkfs/files.h
mkfs/mkfs.c
mksd.mk
usr/lib/ld-musl-aarch64.so.1        // これが使われる
usr/lib/libc.so                     // いずれこのファイルのsymbolic linkにする
```

```bash
$ /bin/hello-dyn
[1]execve: found ip
[1]execve: ip: /lib/ld-musl-aarch64.so.1
[1]execve: load interpreter
[2]load_interpreter: elf header check ok
[3]load_interpreter: mapped: 0xffdeffff0000, start: 0xffdeffff0000, size: 0x987ba, offset: 0x0
[3]load_interpreter: load_addr: 0xffdeffff0000
[3]load_interpreter: mapped: 0xffdf00098000, start: 0xffdf00098000, size: 0x1062, offset: 0x98000
[3]load_interpreter: ph read ok
[3]load_interpreter: padzero ok
[3]load_interpreter: need set_brk: bss_start: 0xffdf0009a000, end: 0xffdf0009ba98
[3]load_interpreter: set_brk ok
[3]load_interpreter: ok. base: 0xffdeffff0000, return addr: 0xffdf000517d0
[3]execve: copy argv ok
[3]execve: copy envp ok
[3]execve: argv: 0x0xfffffffffea8, envp: 0x0xfffffffffeb8, auxv: 0x0xfffffffffed0
[3]execve: proc: base=0x0, size=0x15d8, stack: sp=0xfffffea0, size=0xa000
[3]execve: entry 0x0xffdf000517d0
[3]execve: exec hello-dyn ok
Hello dynamic world!
```

## 今後の課題

- /etc/inittabによるln -s /lib/libc.so /lib/ld-musl-aarch64.so.1
- より本格的な動的リンクプログラムの作成
- cachepagesの確認
