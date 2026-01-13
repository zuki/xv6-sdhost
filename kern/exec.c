#include <types.h>
#include <linux/elf.h>
#include <linux/auxvec.h>
#include <trap.h>

#include <string.h>
#include <console.h>
#include <vm.h>
#include <proc.h>
#include <mm.h>
#include <memlayout.h>
#include <syscall.h>
#include <vfs.h>
#include <filedesc.h>
#include <linux/errno.h>

static uint64_t auxv[][2] = { { AT_PAGESZ, PGSIZE } };

// 親から受け継いだ不要な情報を破棄する
static void flush_old_exec(void)
{
    struct proc *p = thisproc();

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

#if 0
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
#endif
}

int execve(const char *path, char *const argv[], char *const envp[])
{
    struct vfile *file;
    char *s;
    int err;

    trace("path='%s', argv=0x%p, envp=0x%p", path, argv, envp);

    // Save previous page table.
    struct proc *curproc = thisproc();

    if (vfs_access(curproc->cwd, path, X_OK, curproc->uid) < 0) {
        error("uid %d can't access %s", curproc->uid, path);
        return -EPERM;
    }

    if ((err = vfs_open(curproc->cwd, path, O_RDONLY, 0, curproc->uid, &file)) < 0) {
        error("failed open %s", path);
        return err;
    }
    trace("open ok: f->vnode->ino: %d", file->vnode->ino);
    if (!S_ISREG(file->vnode->mode)) {
        vfs_close(file);
        return -EISDIR;
    }

    void *oldpgdir = curproc->pgdir, *pgdir = vm_init();

    if (pgdir == 0) {
        trace("vm init failed");
        goto bad;
    }

    Elf64_Ehdr elf;
    if (vfs_read(file, (char *)&elf, sizeof(elf)) < 0) {
        trace("readelf bad");
        goto bad;
    }

    if (!(  elf.e_ident[EI_MAG0] == ELFMAG0
         && elf.e_ident[EI_MAG1] == ELFMAG1
         && elf.e_ident[EI_MAG2] == ELFMAG2
         && elf.e_ident[EI_MAG3] == ELFMAG3)) {
        trace("elf header magic invalid");
        goto bad;
    }
    if (elf.e_ident[EI_CLASS] != ELFCLASS64) {
        trace("64 bit program not supported");
        goto bad;
    }
    trace("elf header check ok");

    int i;
    Elf64_Phdr ph;

    curproc->pgdir = pgdir;     // Required since readi(sdrw) involves context switch(switch page table).

    flush_old_exec();

    // Load program into memory.
    size_t sz = 0, base = 0, stksz = 0, offset;
    int first = 1;

    for (i = 0; i < elf.e_phnum; i++) {
        if (vfs_read(file, (char *)&ph, sizeof(ph)) < 0) {
            trace("read bad");
            goto bad;
        }

        if (ph.p_type != PT_LOAD) {
            // trace("unsupported type 0x%x, skipped\n", ph.p_type);
            continue;
        }

        if (ph.p_memsz < ph.p_filesz) {
            trace("memsz smaller than filesz");
            goto bad;
        }

        if (ph.p_vaddr + ph.p_memsz < ph.p_vaddr) {
            trace("vaddr + memsz overflow");
            goto bad;
        }

        if (first) {
            first = 0;
            sz = base = ph.p_vaddr;
            if (base % PGSIZE != 0) {
                trace("first section should be page aligned!");
                goto bad;
            }
        }

        if ((sz =
             uvm_alloc(pgdir, base, stksz, sz,
                       ph.p_vaddr + ph.p_memsz)) == 0) {
            trace("uvm_alloc bad");
            goto bad;
        }

        uvm_switch(pgdir);

        offset = file->offset;
        if (vfs_seek(file, ph.p_offset, SEEK_SET) < 0) {
            trace("failed seek");
            goto bad;
        }
        if (vfs_read(file, (char *)ph.p_vaddr, ph.p_filesz) < 0) {
            trace("read section bad");
            goto bad;
        }
        if ((offset = vfs_seek(file, offset, SEEK_SET)) < 0) {
            trace("failed seek");
            goto bad;
        }

        // Initialize BSS.
        memset((void *)ph.p_vaddr + ph.p_filesz, 0,
               ph.p_memsz - ph.p_filesz);

        // Flush dcache to memory so that icache can retrieve the correct one.
        dccivac((void *)ph.p_vaddr, ph.p_memsz);

        trace("init bss [0x%p, 0x%p)", ph.p_vaddr + ph.p_filesz,
              ph.p_vaddr + ph.p_memsz);
    }

    vfs_close(file);
    trace("load file ok");

    // Push argument strings, prepare rest of stack in ustack.
    uvm_switch(oldpgdir);
    char *sp = (char *)USERTOP;
    int argc = 0, envc = 0;
    size_t len;
    if (argv) {
        for (; in_user((void *)(argv + argc), sizeof(*argv)) && argv[argc];
             argc++) {
            if ((len = fetchstr((uint64_t) argv[argc], &s)) < 0) {
                trace("argv fetchstr bad");
                goto bad;
            }
            trace("argv[%d] = '%s', len: %d", argc, argv[argc], len);
            sp -= len + 1;
            if (copyout(pgdir, sp, argv[argc], len + 1) < 0)    // include '\0';
                goto bad;
        }
    }
    trace("copy argv ok");
    if (envp) {
        for (; in_user((void *)(envp + envc), sizeof(*envp)) && envp[envc];
             envc++) {
            if ((len = fetchstr((uint64_t) envp[envc], &s)) < 0) {
                trace("envp fetchstr bad");
                goto bad;
            }
            trace("envp[%d] = '%s', len: %d", envc, envp[envc], len);
            sp -= len + 1;
            if (copyout(pgdir, sp, envp[envc], len + 1) < 0)    // include '\0';
                goto bad;
        }
    }
    trace("copy envp ok")
    // Align to 16B. 3 zero terminator of auxv/envp/argv and 1 argc.
    void *newsp =
        (void *)ROUNDDOWN((size_t)sp - sizeof(auxv) -
                          (envc + argc + 4) * 8, 16);
    if (copyout(pgdir, newsp, 0, (size_t)sp - (size_t)newsp) < 0)
        goto bad;

    uvm_switch(pgdir);

    uint64_t *newargv = newsp + 8;
    uint64_t *newenvp = (void *)newargv + 8 * (argc + 1);
    uint64_t *newauxv = (void *)newenvp + 8 * (envc + 1);
    trace("argv: 0x%p, envp: 0x%p, auxv: 0x%p", newargv, newenvp, newauxv);
    memmove(newauxv, auxv, sizeof(auxv));

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
        goto bad;

    assert((uint64_t) sp > USERTOP - stksz);
    trace("proc: base=0x%x, size=0x%x, stack: sp=0x%x, size=0x%x", base, sz, sp, stksz);

    // Commit to the user image.
    curproc->pgdir = pgdir;

    curproc->base = base;
    curproc->sz = sz;
    curproc->stksz = stksz;

    // memset(curproc->tf, 0, sizeof(*curproc->tf));

    curproc->tf->elr = elf.e_entry;
    curproc->tf->sp = (uint64_t) sp;

    trace("entry 0x%p", elf.e_entry);

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
    return 0;

  bad:
    vfs_close(file);
    if (pgdir)
        vm_free(pgdir);
    thisproc()->pgdir = oldpgdir;
    trace("bad");
    return -1;
}
