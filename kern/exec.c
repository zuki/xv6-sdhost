#include <types.h>
#include <linux/elf.h>
#include <linux/auxvec.h>
#include <exec.h>
#include <trap.h>

#include <string.h>
#include <console.h>
#include <vm.h>
#include <proc.h>
#include <mm.h>
#include <mmap.h>
#include <memlayout.h>
#include <syscall.h>
#include <vfs.h>
#include <filedesc.h>
#include <cachepage.h>
#include <linux/errno.h>
#include <linux/mman.h>

static uint64_t auxv[][2] = { { AT_PAGESZ, PGSIZE } };

// 親から受け継いだ不要な情報を破棄する
static void flush_old_exec(struct proc *p)
{
    // (1) signalを開放
    flush_signal_handlers(p);

    // (2) close_on_execのfileをclose
    for (int i = 0; i < OPEN_MAX; i++) {
        if (p->fd_table[i] && bit_test(p->fdflag, i)) {
            vfs_close(p->fd_table[i]);
            unset_fd(p->fd_table, i);
            bit_remove(p->fdflag, i);
        }
    }

    // (3) capabilityを再設定
    cap_clear(p->cap_inheritable);
    cap_clear(p->cap_permitted);
    cap_clear(p->cap_effective);

    if (p->uid == 0 || p->euid == 0) {
        cap_set_full(p->cap_inheritable);
        cap_set_full(p->cap_permitted);
    }

    if (p->euid == 0 || p->fsuid == 0)
        cap_set_full(p->cap_effective);
}

static int get_file(char *path, struct proc *p, struct vfile **file)
{
    int err;

    // 指定されたファイルが実行可能であるか確認する
    if (vfs_access(p->cwd, path, X_OK, p->uid, 0) < 0) {
        error("uid %d can't access %s", p->uid, path);
        return -EPERM;
    }

    // 指定されたファイルをopen
    if ((err = vfs_open(p->cwd, path, O_RDONLY, 0, p->uid, file)) < 0) {
        error("failed open %s", path);
        return err;
    }
    trace("open ok: f->vnode->ino: %d", (*file)->vnode->ino);
    if (!S_ISREG((*file)->vnode->mode)) {
        vfs_close(*file);
        return -EISDIR;
    }
    return 0;
}

// bssからページ境界までゼロクリアする
// |----bss_0000000000000000|
static void padzero(uint64_t bss)
{
    uint64_t nbytes = ELF_PAGEOFFSET(bss);
    if (nbytes) {
        nbytes = ELF_MIN_ALIGN - nbytes;
        memset((void *)bss, 0, nbytes);
    }
}

// bss領域用のマッピングを必要であれば作成する
static void *set_brk(uint64_t start, uint64_t end)
{
    uint64_t old_start = start;
    uint64_t old_end = end;

    start = ELF_PAGEALIGN(start);
    end = ELF_PAGEALIGN(end);
    // 既存のマッピング（データ用）で間に合う場合は何もしない（既存のマッピングは0詰め済み）
    if (end <= start) {
        padzero(old_start);
        return (void *)old_start;
    }

    trace("old_start=0x%llx, start=0x%llx, old_end=0x%llx, end=0x%llx",
        old_start, start, old_end, end);

    return mmap((void *)start, end - start, PROT_READ | PROT_WRITE,
        MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, NULL, 0);
}

static uint64_t load_interpreter(char *path, uint64_t *base, struct proc *p)
{
    struct vfile *file;                     // interpreterファイル
    Elf64_Ehdr elf;                         // プログラムヘッダ作業用
    Elf64_Phdr ph;                          // プログラムヘッダー格納用
    uint64_t load_addr = 0;                 // interpreterロードアドレス
    uint64_t bss_start = 0, bss_end = 0;    // bssの開始/終了アドレス
    void *mapped;
    size_t size;
    boolean set = false;
    long err = -ENOEXEC;
    int i;

    if ((err = get_file(path, p, &file)) < 0) {
        error("get_file: %s, err: %d", path, err);
        return err;
    }

    // elfヘッダーを読み込み、妥当性をチェックする
    if (copy_cachepages(file, (char *)&elf, sizeof(elf), 0) < 0) {
        trace("readelf bad");
        goto bad;
    }

    if (ELF_CHECK_E_IDNET(&elf) == 0) {
        trace("elf header magic invalid");
        goto bad;
    }
    if (elf.e_ident[EI_CLASS] != ELFCLASS64) {
        trace("64 bit program not supported");
        goto bad;
    }
    if (elf.e_type != ET_DYN && elf.e_type != ET_EXEC) {
        error("bad header type %d", elf.e_type);
        goto bad;
    }
    if (elf.e_phentsize != sizeof(Elf64_Phdr)) {
        error("e_phentsize is invalid: %d", elf.e_phentsize);
        goto bad;
    }

    if (elf.e_phnum > ELF_MIN_ALIGN / sizeof(Elf64_Phdr)) {
        error("e_phnum is invalide: %d", elf.e_phnum);
        goto bad;
    }
    if (ph.p_memsz < ph.p_filesz) {
        error("memsz smaller than filesz");
        goto bad;
    }
    if (ph.p_vaddr + ph.p_memsz < ph.p_vaddr) {
        trace("vaddr + memsz overflow");
        goto bad;
    }
    trace("elf header check ok");

    for (i = 0; i < elf.e_phnum; i++) {
        int flags = 0;
        int prot  = 0;
        uint64_t vaddr = 0;
        uint64_t k;

        // プログラムヘッダーを1つ読み込む
        if (copy_cachepages(file, (char *)&ph, sizeof(ph), elf.e_phoff + i * elf.e_phentsize)) {
            error("read bad");
            goto bad;
        }

        // PT_LOAD以外のプログラムヘッダーは無視する
        if (ph.p_type != PT_LOAD) {
            // trace("unsupported type 0x%x, skipped\n", ph.p_type);
            continue;
        }

        // interpreterをmappingする
        if (ph.p_flags & PF_R) prot |= PROT_READ;
        if (ph.p_flags & PF_W) prot |= PROT_WRITE;
        if (ph.p_flags & PF_X) prot |= PROT_EXEC;

        /* テキスト領域はMAP_SHARED| MAP_DENYWRITE、データ領域はMAP_PRIVATE */
        if (prot & PROT_WRITE)
            flags = MAP_PRIVATE;
        else
            flags = MAP_SHARED | MAP_DENYWRITE;

        vaddr = ph.p_vaddr;

        // ET_EXECまたは2番目以降のプログラムヘッダはアドレス固定
        if (elf.e_type == ET_EXEC || set)
            flags |= MAP_FIXED;
        // それ以外は固定ではないが、初期値を与える
        else
            /* load_addrの初期値をELF_ET_DYN_BASEの1ページ下に設定する */
            load_addr = ELF_PAGESTART(ELF_ET_DYN_BASE - vaddr);

        // データをmappingする
        mapped = mmap((void *)ELF_PAGESTART(load_addr + vaddr),
                      ph.p_filesz + ELF_PAGEOFFSET(vaddr),
                      prot, flags, file, ph.p_offset - ELF_PAGEOFFSET(vaddr));
        if (IS_ERR(mapped)) {
            err = (long)mapped;
            goto bad;
        }

        // 1番目のPT_LOAD固有の処理
        if (!set && elf.e_type == ET_DYN) {
            load_addr = (uint64_t)mapped - ELF_PAGESTART(vaddr);
            set = true;
        }

        // bss領域の設定
        k = load_addr + vaddr + ph.p_filesz;
        if (k > bss_start) bss_start = k;
        k = load_addr + vaddr + ph.p_memsz;
        if (k > bss_end) bss_end = k;
    }

    padzero(bss_start);
    bss_start = ELF_PAGESTART(bss_start + ELF_MIN_ALIGN - 1);
    if (bss_end > bss_start) {
        mapped = set_brk(bss_start, bss_end);
        if (IS_ERR(mapped)) {
            err = (long)mapped;
            goto bad;
        }
    }

    *base = load_addr;
    err = ((uint64_t)elf.e_entry + load_addr);

bad:
    vfs_close(file);
    return err;
}

int execve(const char *path, char *const argv[], char *const envp[])
{
    struct vfile *file;
    char *s, *ip;
    int has_ip = 0;
    uint64_t ip_entry;
    uint64_t ip_base;
    long err = -EACCES;

    trace("path='%s', argv=%p, envp=%p", path, argv, envp);

    // 呼び出し元のプロセスをcurprocとする
    struct proc *curproc = thisproc();

    if ((err = get_file(path, curproc, &file)) > 0) {
        error("get_file: %s, err: %d", path, err);
        return err;
    }

    // 呼び出し元のページテーブルをoldpgdirに保存し、
    // 実行ファイル用のページテーブルをpgdirとして新規作成する
    void *oldpgdir = curproc->pgdir, *pgdir = vm_init();

    if (pgdir == 0) {
        trace("vm init failed");
        goto bad;
    }

    // elfヘッダーを読み込み、妥当性をチェックする
    Elf64_Ehdr elf;
    if (copy_cachepages(file, (char *)&elf, sizeof(elf), 0) < 0) {
        trace("readelf bad");
        goto bad;
    }

    if (ELF_CHECK_E_IDNET(&elf) == 0) {
        trace("elf header magic invalid");
        goto bad;
    }

    if (elf.e_ident[EI_CLASS] != ELFCLASS64) {
        trace("64 bit program not supported");
        goto bad;
    }

    if (elf.e_type != ET_EXEC && elf.e_type != ET_DYN) {
        error("bad header type %d", elf.e_type);
        goto bad;
    }
    trace("elf header check ok");

    curproc->pgdir = pgdir;     // Required since readi(sdrw) involves context switch(switch page table).

    // ファイルのSet-uidをセットする: stickyビットの対応
    if (file->vnode->mode & S_ISUID && curproc->uid != 0)
        curproc->fsuid = file->vnode->uid;
    else
        curproc->fsuid = curproc->uid;

    // ファイルのSet-gidをセットする: stickyビットの対応
    if (((file->vnode->mode & (S_ISGID | S_IXGRP)) == (S_ISGID | S_IXGRP)) && curproc->gid != 0)
        curproc->fsgid = file->vnode->gid;
    else
        curproc->fsgid = curproc->gid;

    // 親から受け継いだ情報を破棄する
    flush_old_exec(curproc);

    // プログラムをメモリにロードする
    int i;
    Elf64_Phdr ph;      // プログラムヘッダー格納用
    size_t sz = 0;      // 最後にuvm_allocしたプログラムヘッダのp_msizeを保持
    size_t base = 0;    // 最初のプロセスヘッダのp_vaddrを保持
    size_t stksz = 0;   // スタックサイズを保持
    int first = 1;      // 最初のプログラムヘッダか? : 特別な処理が必要なため

    for (i = 0; i < elf.e_phnum; i++) {
        // プログラムヘッダーを1つ読み込む
        if (copy_cachepages(file, (char *)&ph, sizeof(ph), elf.e_phoff + i * elf.e_phentsize)) {
            error("read bad");
            goto bad;
        }

        if (ph.p_type == PT_INTERP) {
            if (ph.p_filesz > PGSIZE) {
                error("name of interpretor is too long: 0x%x", ph.p_filesz);
                goto bad;
            }
            ip = (char *)kmalloc(ph.p_filesz);
            if (!ip) {
                error("could't get memory for interpretor name");
                goto bad;
            }
            if ((err = copy_cachepages(file, ip, ph.p_filesz, ph.p_offset)) < 0) {
                error("failed get interpretor name");
                goto bad;
            }
            has_ip = 1;
            continue;
        }

        // PT_LOAD以外のプログラムヘッダーは無視する
        if (ph.p_type != PT_LOAD) {
            // trace("unsupported type 0x%x, skipped\n", ph.p_type);
            continue;
        }
        // データチェック1: fileサイズとメモリサイズの妥当性チェック
        if (ph.p_memsz < ph.p_filesz) {
            error("memsz smaller than filesz");
            goto bad;
        }
        // データチェック2: 桁溢れのチェック
        if (ph.p_vaddr + ph.p_memsz < ph.p_vaddr) {
            trace("vaddr + memsz overflow");
            goto bad;
        }

        // 最初のプログラムヘッダのチェック: ページアラインでなければならない
        if (first) {
            first = 0;
            sz = base = ph.p_vaddr;
            if (base % PGSIZE != 0) {
                trace("first section should be page aligned!");
                goto bad;
            }
        }
        trace("ph[%d]: offset: 0x%x, vaddr: 0x%llx, filesz: 0x%x, memsz: 0x%x, flags: 0x%x, align: 0x%x",
            i, ph.p_offset, ph.p_vaddr, ph.p_filesz, ph.p_memsz, ph.p_flags, ph.p_align);
        // phdr->p_vaddr + phdr->p_memsz までメモリを割り当てる
        // 0クリアはしていない
        if ((sz = uvm_alloc(pgdir, base, stksz, sz, ph.p_vaddr + ph.p_memsz)) == 0) {
            trace("uvm_alloc bad");
            goto bad;
        }

        uvm_switch(pgdir);

        if (copy_cachepages(file, (char *)ph.p_vaddr, ph.p_filesz, ph.p_offset) < 0) {
            error("ph[%d].p_files load error", i);
            goto bad;
        }
        //hexdump(ph.p_vaddr, ph.p_filesz, "rodata + data");

        // BSS部分(p_memsz - p_filesz)を0で初期kする
        memset((void *)ph.p_vaddr + ph.p_filesz, 0,
               ph.p_memsz - ph.p_filesz);
        //hexdump(ph.p_vaddr+ph.p_filesz, ph.p_memsz, "bss");
        // データキャッシュ (dcache) をメモリに書き出し、命令キャッシュ
        // (icache) が正しいデータにアクセスできるようにする
        dccivac((void *)ph.p_vaddr, ph.p_memsz);

        trace("init bss [0x%p, 0x%p)", ph.p_vaddr + ph.p_filesz,
              ph.p_vaddr + ph.p_memsz);
    }

    // これ以前は、エラーがあった場合は goto bad
    vfs_close(file);
    // 以後、エラーがあった場合は goto free_ip
    trace("load file ok");

    // インタープリタが指定されていた場合
    if (has_ip) {
        ip_entry = load_interpreter(ip, &ip_base, curproc);
        if (IS_ERR((void *)ip_entry)) {
            error("failed load interpretor: %s", ip);
            goto free_ip;
        }
    }

    // Push argument strings, prepare rest of stack in ustack.
    uvm_switch(oldpgdir);
    char *sp = (char *)USERTOP;
    int argc = 0, envc = 0;
    size_t len;

    // platform名("aarch64")のセット
    uint64_t platform = 0;
    if (has_ip) {
        len = strlen(ELF_PLATFORM) + 1;
        sp -= len;
        platform = (uint64_t)sp;
        if (copyout(pgdir, sp, ELF_PLATFORM, len) < 0) {
            error("failed copyout platform");
            goto free_ip;
        }
    }

    if (argv) {
        for (; in_user((void *)(argv + argc), sizeof(*argv)) && argv[argc];
             argc++) {
            if ((len = fetchstr((uint64_t) argv[argc], &s)) < 0) {
                trace("argv fetchstr bad");
                goto free_ip;
            }
            trace("argv[%d] = '%s', len: %d", argc, argv[argc], len);
            sp -= len + 1;
            if (copyout(pgdir, sp, argv[argc], len + 1) < 0)    // include '\0';
                goto free_ip;
        }
    }
    trace("copy argv ok");
    if (envp) {
        for (; in_user((void *)(envp + envc), sizeof(*envp)) && envp[envc];
             envc++) {
            if ((len = fetchstr((uint64_t) envp[envc], &s)) < 0) {
                trace("envp fetchstr bad");
                goto free_ip;
            }
            trace("envp[%d] = '%s', len: %d", envc, envp[envc], len);
            sp -= len + 1;
            if (copyout(pgdir, sp, envp[envc], len + 1) < 0)    // include '\0';
                goto free_ip;
        }
    }
    trace("copy envp ok")
    uint64_t auxv_size;
    uint64_t auxv_dyn[][2] = {
        { AT_PHDR,    elf.e_phoff },
        { AT_PHENT,   sizeof(Elf64_Phdr) },
        { AT_PHNUM,   elf.e_phnum },
        { AT_PAGESZ,  PGSIZE },
        { AT_BASE,    ip_base },                // インタプリタのbaseアドレス
        { AT_FLAGS,   0 },
        { AT_ENTRY,   elf.e_entry },            // プログラムのエントリポイント
        { AT_UID,     (uint64_t) curproc->uid },
        { AT_EUID,    (uint64_t) curproc->euid },
        { AT_GID,     (uint64_t) curproc->gid },
        { AT_EGID,    (uint64_t) curproc->egid },
        { AT_PLATFORM, platform },
        { AT_HWCAP,   ELF_HWCAP },
        { AT_CLKTCK,  HZ },
        { AT_NULL,    0 }

    };
    uint64_t auxv_sta[][2] = { { AT_PAGESZ, PGSIZE } };

    auxv_size = has_ip ? sizeof(auxv_dyn) : sizeof(auxv_sta);

    // Align to 16B. 3 zero terminator of auxv/envp/argv and 1 argc.
    void *newsp =
        (void *)ROUNDDOWN((size_t)sp - auxv_size -
                          (envc + argc + 4) * 8, 16);
    if (copyout(pgdir, newsp, 0, (size_t)sp - (size_t)newsp) < 0)
        goto free_ip;

    uvm_switch(pgdir);

    uint64_t *newargv = newsp + 8;
    uint64_t *newenvp = (void *)newargv + 8 * (argc + 1);
    uint64_t *newauxv = (void *)newenvp + 8 * (envc + 1);
    trace("argv: 0x%p, envp: 0x%p, auxv: 0x%p", newargv, newenvp, newauxv);
    memmove(newauxv, auxv, sizeof(auxv));

    if (has_ip)
        memmove(newauxv, auxv_dyn, auxv_size);
    else
        memmove(newauxv, auxv_sta, auxv_size);

    for (int i = envc - 1; i >= 0; i--) {
        newenvp[i] = (uint64_t) sp;
        for (; *sp; sp++) ;
        sp++;
    }
    for (int i = argc - 1; i >= 0; i--) {
        newargv[i] = (uint64_t) sp;
        for (; *sp; sp++) ;
        sp++;
    }
    *(size_t *)(newsp) = argc;

    sp = newsp;
    trace("newsp: 0x%p", sp);

    // Allocate user stack.
    stksz = ROUNDUP(USERTOP - (size_t)sp, 10 * PGSIZE);
    if (copyout
        (pgdir, (void *)(USERTOP - stksz), 0,
         stksz - (USERTOP - (size_t)sp)) < 0)
        goto free_ip;

    assert((uint64_t) sp > USERTOP - stksz);
    trace("proc: base=0x%x, size=0x%x, stack: sp=0x%x, size=0x%x", base, sz, sp, stksz);

    // Commit to the user image.
    curproc->pgdir = pgdir;

    curproc->base = base;
    curproc->sz = sz;
    curproc->stksz = stksz;

    if (has_ip)
        curproc->tf->elr = ip_entry;
    else
        curproc->tf->elr = elf.e_entry;

    curproc->tf->sp = (uint64_t) sp;

    trace("entry 0x%p", curproc->tf->elr);

    uvm_switch(oldpgdir);

    // Save program name for debugging.
    const char *last, *cur;
    for (last = cur = path; *cur; cur++)
        if (*cur == '/')
            last = cur + 1;
    trace("p->name: %s", last);
    safestrcpy(curproc->name, last, sizeof(curproc->name));

    uvm_switch(curproc->pgdir);
    vm_free(oldpgdir);
    trace("exec %s ok", curproc->name);
    if (has_ip && ip)
        kmfree((void *)ip);

    return 0;

free_ip:
    if (has_ip && ip)
        kmfree(ip);
bad:
    vfs_close(file);
    if (pgdir)
        vm_free(pgdir);
    thisproc()->pgdir = oldpgdir;
    trace("bad");
    return -1;
}
