#include <proc.h>
#include <trap.h>
#include <console.h>
#include <vm.h>
#include <syscall.h>
#include <filedesc.h>
#include <mmap.h>
#include <vfs.h>
#include <fs/vfile.h>
#include <linux/mman.h>
#include <linux/errno.h>

long sys_yield(void)
{
    yield();
    return 0;
}

size_t sys_brk(void)
{
    struct proc *p = thisproc();
    size_t sz, newsz, oldsz = p->sz;

    //panic("sys_brk: unimplemented. ");

    if (argu64(0, &newsz) < 0)
        return oldsz;

    trace("[%d] name %s: 0x%llx to 0x%llx, old p->sz: 0x%x", p->pid, p->name, oldsz, newsz, p->sz);

    if (newsz == 0)
        return oldsz;

    if (newsz < oldsz) {
        p->sz = uvm_dealloc(p->pgdir, p->base, oldsz, newsz);
    } else {
        sz = uvm_alloc(p->pgdir, p->base, p->stksz, oldsz, newsz);
        if (sz == 0)
            return oldsz;
        p->sz = sz;
    }
    trace("[%d] return new p->sz: 0x%x", p->pid, p->sz);
    return p->sz;
}

// void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
void *sys_mmap(void)
{
    void *addr;
    size_t length, offset;
    int prot, flags, fd;
    struct vfile *f;

    if (argu64(0, (uint64_t *) & addr) < 0 ||
        argu64(1, &length) < 0 ||
        argint(2, &prot) < 0 ||
        argint(3, &flags) < 0 ||
        argint(4, &fd) < 0 ||
        argu64(5, &offset) < 0)
        return (void *)-EINVAL;

    if (flags & MAP_ANONYMOUS) {
        if (fd != -1) return (void *)-EINVAL;
        f = NULL;
    } else {
        if (fd < 0 || fd >= OPEN_MAX) return (void *)-EBADF;
        struct vfile *get_fd(fd_table_t table, int fd);
        if ((f = get_fd(thisproc()->fd_table, fd)) == NULL) return (void *)-EBADF;
    }

    if ((flags & (MAP_PRIVATE | MAP_SHARED)) == 0) {
        warn("invalid flags: 0x%x", flags);
        return (void *)-EINVAL;
    }

    if ((ssize_t)length <= 0 || (ssize_t)offset < 0) {
        warn("invalid length: %lld or offset: %lld", length, offset);
        return (void *)-EINVAL;
    }

    // MAP_FIXEDの場合、addrが指定されていなければならない
    if ((flags & MAP_FIXED) && addr == NULL) {
        warn("MAP_FIXED and addr is NULL");
        return (void *)-EINVAL;
    }

    // バックにあるファイルはreadableでなければならない
    if (!(flags & MAP_ANONYMOUS) && !FILE_READABLE(f)) {
        warn("file is not readable");
        return (void *)-EACCES;
    }

    // MAP_SHAREかつPROT_WRITEの場合はバックにあるファイルがwritableでなければならない
    if (!(flags & MAP_ANONYMOUS) && (flags & MAP_SHARED)
     && (prot & PROT_WRITE) && !FILE_WRITABLE(f)) {
        warn("file is not writable");
        return (void *)-EACCES;
    }

    trace("addr: 0x%llx, length: 0x%x, prot: 0x%x, flags: 0x%x, f: %d, offset: 0x%x",
        addr, length, prot, flags, f ? f->vnode->ino : -1, offset);
    return mmap(addr, length, prot, flags, f, offset);
}

// int munmap(void *addr, size_t length);
long sys_munmap(void)
{
    void *addr;
    size_t length;

    if (argu64(0, (uint64_t *)&addr) < 0 || argu64(1, &length) < 0)
        return -EINVAL;

    trace("addr: 0x%llx, length: 0x%llx", addr, length);

    return munmap(addr, length);
}

// void *mremap(void *old_address, size_t old_size, size_t new_size, int flags, void *new_address);
void *sys_mremap(void)
{
    void *old_addr, *new_addr;
    size_t old_size, new_size;
    int flags;

    if (argu64(0, (uint64_t *)&old_addr) < 0 || argu64(1, &old_size) < 0
     || argu64(2, &new_size) < 0 || argint(3, &flags) < 0
     || argu64(4, (uint64_t *)&new_addr) < 0)
        return -EINVAL;

    return mremap(old_addr, old_size, new_size, flags, new_addr);
}

// int mprotect(void *addr, size_t len, int prot);
long sys_mprotect(void)
{
    void *addr;
    size_t length;
    int prot;

    if (argu64(0, (uint64_t *)&addr) < 0 || argu64(1, &length) < 0
     || argint(2, &prot) < 0)
        return -EINVAL;

    return mprotect(addr, length, prot);
}

// int msync(void *addr, size_t length, int flags);
long sys_msync(void)
{
    void *addr;
    size_t length;
    int flags;

    if (argu64(0, (uint64_t *)&addr) < 0 || argu64(1, &length) < 0
     || argint(2, &flags) < 0)
        return -EINVAL;

    return msync(addr, length, flags);
}


long sys_clone(void)
{
    void *childstk;
    uint64_t flag;
    if (argu64(0, &flag) < 0 || argu64(1, (uint64_t *) & childstk) < 0)
        return -1;
    trace("flags 0x%llx, child stack 0x%p", flag, childstk);
    if (flag != 17) {
        warn("flags other than SIGCHLD are not supported");
        return -1;
    }
    return fork();
}

// pid_t wait4(pid_t wpid, int *status, int options, struct rusage *rusage);
long sys_wait4(void)
{
    int pid, opt;
    int *wstatus;
    void *rusage;
    if (argint(0, &pid) < 0 ||
        argu64(1, (uint64_t *) & wstatus) < 0 ||
        argint(2, &opt) < 0 || argu64(3, (uint64_t *) & rusage) < 0)
        return -1;

    trace("[%d] pid: %d, status: 0x%x, options: 0x%x, rusage: 0x%x", pid, wstatus, opt, rusage);
    // FIXME:
    if (pid != -1 || wstatus != 0 || opt != 0 || rusage != 0) {
        warn("unimplemented. pid %d, wstatus 0x%p, opt 0x%x, rusage 0x%p",
             pid, wstatus, opt, rusage);
        return -1;
    }

    return wait();
}


// FIXME: use pid instead of tid since we don't have threads :)
long sys_set_tid_address(void) {
    trace("set_tid_address: name '%s'", thisproc()->name);
    return thisproc()->pid;
}

long sys_getpid(void) {
    return thisproc()->pid;
}

long sys_gettid(void) {
    trace("gettid: name '%s'", thisproc()->name);
    return thisproc()->pid;
}


long sys_rt_sigprocmask(void)
{
    return -EINVAL;
}

// FIXME: exit_group should kill every thread in the current thread group.
long sys_exit_group(void) {
    trace("[%d] sys_exit_group: '%s' exit with code %d", thisproc()->pid, thisproc()->name, thisproc()->tf->x[0]);
    exit(thisproc()->tf->x[0]);
    return 0;
}

long sys_exit(void) {
    trace("[%d] exit '%s' with code %d", thisproc()->pid, thisproc()->name, thisproc()->tf->x[0]);
    exit(thisproc()->tf->x[0]);
    return 0;
}
