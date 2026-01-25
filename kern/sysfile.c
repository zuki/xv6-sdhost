//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//

#include <types.h>
#include <mmu.h>
#include <proc.h>
#include <spinlock.h>
#include <sleeplock.h>
#include <string.h>
#include <console.h>
#include <vfs.h>
#include <fs/vfile.h>
#include <filedesc.h>
#include <linux/fcntl.h>
#include <linux/errno.h>
#include <syscall.h>
#include <net/sockio.h>
#include <linux/ioctl.h>
#include <linux/stat.h>

extern int execve(const char *, char *const, char *const);

struct iovec {
    void *iov_base;             /* Starting address. */
    size_t iov_len;             /* Number of bytes to transfer. */
};

long check_fdcwd(char *path, int dirfd, struct vnode **cwd)
{
    struct vfile *f = NULL;
    struct proc *p = thisproc();

    if (*path != '/' && dirfd != AT_FDCWD) {
        if ((f = get_fd(p->fd_table, dirfd)) == NULL) {
            error("pid[%d] path: %s (ofile[%d]) is not open", p->pid, path, dirfd);
            return -EBADF;
        }

        if (!S_ISDIR(f->vnode->mode)) {
            error("pid[%d] path: %s, dirfd: %d, mode: 0x%x is not dir", p->pid, path, dirfd, f->vnode->mode);
            return -ENOTDIR;
        }
        if (cwd)
            *cwd = f->vnode;
    } else {
        if (cwd)
            *cwd = thisproc()->cwd;
    }

    return 0;
}

/* int dup(int oldfd); */
long sys_dup(void)
{
    int ofd, fd;
    struct vfile *f;
    struct proc *p = thisproc();
    long error;

    //print_fd_table(thisproc()->fd_table, "sys_dup 1");
    if ((error = argfd(0, &ofd, &f)) < 0) {
        error("[%d] argfd err: %d fd: %d", p->pid, error, ofd);
        return error;
    }

    trace("[%d] oldfd: %d", p->pid, ofd);
    //print_fd_table(thisproc()->fd_table, "sys_dup 1");
    if ((fd = find_unused_fd(p->fd_table, 0)) < 0) {
        error("[%d] no unused fd", p->pid);
        return fd;
    }

    dup_fd(p->fd_table, fd, f);
    trace("[%d] dup ofd: %d to fd: %d ref: %d", p->pid, ofd, fd, f->refcount);
    //print_fd_table(thisproc()->fd_table, "sys_dup 2");
    return fd;
}

/* int dup3(int oldfd, int newfd, int flags); */
long sys_dup3(void)
{
    int fd1, fd2, flags;
    struct vfile *f1;
    struct proc *p = thisproc();
    long error;

    if ((error = argfd(0, &fd1, &f1)) < 0)
        return error;
    if (argint(1, &fd2) < 0 || argint(2, &flags) < 0)
        return -EINVAL;

    if (flags & ~O_CLOEXEC) return -EINVAL;

    if (fd1 == fd2) return fd1;

    if ((fd2 = find_unused_fd(p->fd_table, fd2)) < 0)
        return fd2;

    if (flags & O_CLOEXEC)
        bit_add(p->fdflag, fd2);

    dup_fd(p->fd_table, fd2, f1);
    trace("retrun fd: %d", fd2);
    return fd2;
}

/* ssize_t read(int fd, void *buf, size_t count); */
ssize_t sys_read(void)
{
    int fd;
    struct vfile *f;
    size_t count;
    char *buf;
    long error;

    if ((error = argfd(0, &fd, &f)) < 0)
        return error;
    if ((argu64(2, (uint64_t *)&count)) < 0)
        return error;
    if ((error = argptr(1, (void **)&buf, count)) < 0)
        return error;

    int ret = vfs_read(f, buf, count);
    trace("[%d] fd: %d, buf: 0x%llx ('%s'), count: 0x%x, ret: 0x%x", thisproc()->pid, fd, buf, buf, count, ret);
    return ret;
    //return vfs_read(f, buf, count);
}

/*  ssize_t write(int fd, const void *buf, size_t count);  */
ssize_t sys_write(void)
{
    struct vfile *f;
    size_t count;
    char *buf;
    long error;

    if ((error = argfd(0, 0, &f)) < 0)
        return error;
    if ((error = argu64(2, (uint64_t *)&count)) < 0)
         return error;
    if ((error = argptr(1, (void **)&buf, count)) < 0)
        return error;

    return vfs_write(f, buf, count);
}

/* ssize_t writev(int fd, const struct iovec *iov, int iovcnt); */
ssize_t sys_writev(void)
{
    struct vfile *f;
    int fd, iovcnt;
    struct iovec *iov, *p;
    long error;

    if ((error =argfd(0, &fd, &f)) < 0)
        return error;
    if ((error = argint(2, &iovcnt)) < 0)
        return error;
    if ((error = argptr(1, (void **)&iov, iovcnt * sizeof(struct iovec))) < 0)
        return error;

#if 0
    debug("[%d] fd %d, iovcnt: %d", thisproc()->pid, fd, iovcnt);
    for (int i=0; i < iovcnt; i++) {
        debug("iov[%d]: base=%p, len=%lld", i, iov[i].iov_base, iov[i].iov_len);
    }
#endif

    int tot = 0;
    for (p = iov; p < iov + iovcnt; p++) {
        if (!in_user(p->iov_base, p->iov_len)) {
            trace("iov_base: 0x%x, len: 0x%x not in user", p->iov_base, p->iov_len);
            //hexdump(p->iov_base, p->iov_len, "sys_writev");
            continue;
            //return -EFAULT;
        }
        tot += vfs_write(f, p->iov_base, p->iov_len);
    }
    return tot;
}

/* int close(int fd); */
long sys_close(void)
{
    int fd;
    struct vfile *f;
    long error;

    if ((error = argfd(0, &fd, &f)) < 0) {
        error("[%d] argfd err: %d fd: %d", thisproc()->pid, error, fd);
        return error;
    }

    trace("[%d] fd: %d, ref: %d", thisproc()->pid, fd, f->refcount);
    vfs_close(f);
    unset_fd(thisproc()->fd_table, fd);
    bit_remove(thisproc()->fdflag, fd);
    return 0;
}

/* int fstat(int fd, struct stat *sb); */
long sys_fstat(void)
{
    struct vfile *file;
    struct stat *st, sst;
    long error;
    int fd;

    if ((error = argfd(0, &fd, &file)) < 0)
        return error;
    if ((error = argptr(1, (void **)&st, sizeof(struct stat))) < 0)
        return error;

    sst.st_dev = 0;
    sst.st_ino = file->vnode->ino;
    sst.st_mode = file->vnode->mode;
    sst.st_nlink = file->vnode->nlink;
    sst.st_uid = file->vnode->uid;
    sst.st_gid = file->vnode->gid;
    sst.st_rdev = file->vnode->rdev;
    sst.st_size = file->vnode->size;
    memmove(&sst.st_atime, &file->vnode->atime, sizeof(struct timespec));
    memmove(&sst.st_mtime, &file->vnode->mtime, sizeof(struct timespec));
    memmove(&sst.st_ctime, &file->vnode->ctime, sizeof(struct timespec));
    memmove(st, &sst, sizeof(struct stat));

    trace("fd: %d, ino: %d, mode: 0x%x", fd, st->st_ino, st->st_mode);
    return 0;
}

/* int fstatat(int dirfd, const char *pathname, struct stat *buf, int flags); */
long sys_fstatat(void)
{
    int dirfd, flags;
    char *path;
    char linkpath[512];
    struct stat *st, sst;
    struct vnode *vnode, *cwd = NULL;
    long error;
    memset(linkpath, 0, 512);

    if ((error = argint(0, &dirfd)) < 0)
        return error;
    if ((error = argstr(1, &path)) < 0)
        return error;
    if ((error = argptr(2, (void *)&st, sizeof(*st))) < 0)
        return error;
    if ((error = argint(3, &flags)) < 0)
        return error;
    trace("dirfd: %d, path: %s, flags: 0x%x", dirfd, path, flags);

    if (flags != 0 && (flags & ~AT_SYMLINK_NOFOLLOW) != 0) {
        warn("unimplemented flags=0x%x", flags);
        return -EINVAL;
    }

    if ((error = check_fdcwd(path, dirfd, &cwd)) < 0)
        return error;

    if ((error = vfs_lookup(cwd, path, VLOOKUP_NORMAL, thisproc()->uid, &vnode)) < 0) {
        error("vfs_lookup error: cwd: %d, path: %s", cwd->ino, path);
        return error;
    }

    trace("(1) path: %s, vnode: %d", path, vnode->ino);

    if (S_ISLNK(vnode->mode) && !(flags & AT_SYMLINK_NOFOLLOW)) {
        char *name = path_last_component(path);
        if ((error = vfs_readlink(cwd, name, linkpath, 512, thisproc()->uid)) < 0) {
            error("readlink: vnode: %d, name: %s", vnode->ino, name);
            vfs_release_vnode(cwd);
            return error;
        }
        linkpath[strlen(linkpath)] = '\0';
        trace("name: %s, linkpath: %s", name, linkpath);
        flags &= ~AT_SYMLINK_NOFOLLOW;
        vfs_release_vnode(vnode);
        if ((error = vfs_lookup(cwd, linkpath, VLOOKUP_NORMAL, thisproc()->uid, &vnode)) < 0) {
            error("vfs_lookup error (%d): vnode: %d, linkpath: %s", error, vnode->ino, linkpath);
            vfs_release_vnode(cwd);
            return error;
        }
        trace("(2) linkvnode: %d", vnode->ino);
    }

    trace("(3) vnode: %d", vnode->ino);
    sst.st_dev = vnode->mp->dev;
    sst.st_ino = vnode->ino;
    sst.st_mode = vnode->mode;
    sst.st_nlink = vnode->nlink;
    sst.st_uid = vnode->uid;
    sst.st_gid = vnode->gid;
    sst.st_rdev = vnode->rdev;
    sst.st_size = vnode->size;
    memmove(&sst.st_atime, &vnode->atime, sizeof(struct timespec));
    memmove(&sst.st_mtime, &vnode->mtime, sizeof(struct timespec));
    memmove(&sst.st_ctime, &vnode->ctime, sizeof(struct timespec));
    memmove(st, &sst, sizeof(struct stat));
    vfs_release_vnode(vnode);

    trace("dirfd: %d, path: %s, ino: %d", dirfd, path, st->st_ino);
    return 0;
}

/* int linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags);  */
long sys_linkat(void)
{
    char *newpath, *oldpath;
    int olddirfd, newdirfd, flags;
    struct vnode *oldv = NULL, *newv = NULL, *vnode;
    long error;
    uid_t uid = thisproc()->uid;

    if ((error = argint(0, &olddirfd)) < 0) return error;
    if ((error = argint(2, &newdirfd)) < 0) return error;
    if ((error = argint(4, &flags)) < 0) return error;
    if ((error = argstr(1, &oldpath)) < 0) return error;
    if ((error = argstr(3, &newpath)) < 0) return error;

    if ((error = check_fdcwd(oldpath, olddirfd, &oldv)) < 0) return error;
    if ((error = check_fdcwd(newpath, newdirfd, &newv)) < 0) return error;

    if (flags & ~AT_SYMLINK_FOLLOW)
        return -EINVAL;

    if ((error = vfs_lookup(oldv, oldpath, VLOOKUP_NORMAL, uid, &vnode)) < 0)
        return error;

    if (S_ISLNK(vnode->mode) && flags & AT_SYMLINK_FOLLOW) {
        char linkpath[512];
        char *name = path_last_component(oldpath);
        vfs_release_vnode(oldv);
        if ((error = vfs_readlink(vnode, name, linkpath, 512, uid)) < 0) {
            vfs_release_vnode(vnode);
            return error;
        }
        linkpath[strlen(linkpath)] = '\0';
        memmove(oldpath, linkpath, strlen(linkpath) + 1);
        oldv = vnode;
        flags &= ~AT_SYMLINK_FOLLOW;
    }

    return vfs_link(oldv, oldpath, newv, newpath, uid);
}

//int symlinkat(const char *oldpath, int newdirfd, const char *newpath);
ssize_t sys_symlinkat(void)
{
    char *oldpath, *newpath;
    int dirfd;
    struct vnode *cwd = NULL;
    long error;

    if ((error = argint(1, &dirfd)) < 0) return error;
    if ((error = argstr(0, &oldpath)) < 0) return error;
    if ((error = argstr(2, &newpath)) < 0) return error;

    if (strlen(oldpath) == 0 || strlen(newpath) == 0)
        return -ENOENT;

    if ((error = check_fdcwd(newpath, dirfd, &cwd)) < 0) return error;

    return vfs_symlink(cwd, oldpath, newpath);

}

/* int unlinkat(int dirfd, const char *pathname, int flags); */
long sys_unlinkat(void)
{
    char *pathname;
    int dirfd, flags;
    struct vnode *cwd;
    long error;

    if ((error = argstr(1, &pathname)) < 0) return error;
    if ((error = argint(0, &dirfd)) < 0) return error;
    if ((error = argint(2, &flags)) < 0) return error;
    if ((error = check_fdcwd(pathname, dirfd, &cwd)) < 0) return error;

    if (flags & ~AT_REMOVEDIR)
        return -EINVAL;

    return vfs_unlink(cwd, pathname, flags, thisproc()->uid);
}

/* ssize_t readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz); */
long sys_readlinkat(void)
{
    char *pathname, *buf;
    int dirfd;
    size_t bufsize;
    struct vnode *cwd;
    long error;

    if ((error = argint(0, &dirfd)) < 0) return error;
    if ((error = argstr(1, &pathname)) < 0) return error;
    if ((error = argu64(3, &bufsize)) < 0) return error;
    if ((error = argptr(2, (void **)&buf, bufsize)) < 0) return error;

    if ((error = check_fdcwd(pathname, dirfd, &cwd)) < 0) return error;

    return vfs_readlink(cwd, pathname, buf, bufsize, thisproc()->uid);
}

/* int renameat2(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags); */
long sys_renameat2(void)
{
    char *newpath, *oldpath;
    int olddirfd, newdirfd, flags;
    struct vnode *oldv = NULL, *newv = NULL;
    long error;

    if ((error = argint(0, &olddirfd)) < 0) return error;
    if ((error = argint(2, &newdirfd)) < 0) return error;
    if ((error = argint(4, &flags)) < 0) return error;
    if ((error = argstr(1, &oldpath)) < 0) return error;
    if ((error = argstr(3, &newpath)) < 0) return error;

    /* muslでは RENAME_NOREPLACEもRENAME_EXCHANGEも対応していない */
    if (flags)
        return -EINVAL;

    if ((error = check_fdcwd(oldpath, olddirfd, &oldv)) < 0) return error;
    if ((error = check_fdcwd(newpath, newdirfd, &newv)) < 0) return error;

    return vfs_rename(oldv, oldpath, newv, newpath, thisproc()->uid);
}

/* int renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath); */
long sys_renameat(void)
{
    char *newpath, *oldpath;
    int olddirfd, newdirfd;
    struct vnode *oldv = NULL, *newv = NULL;
    long error;

    if ((error = argint(0, &olddirfd)) < 0) return error;
    if ((error = argint(2, &newdirfd)) < 0) return error;
    if ((error = argstr(1, &oldpath)) < 0) return error;
    if ((error = argstr(3, &newpath)) < 0) return error;

    if ((error = check_fdcwd(oldpath, olddirfd, &oldv)) < 0) return error;
    if ((error = check_fdcwd(newpath, newdirfd, &newv)) < 0) return error;

    return vfs_rename(oldv, oldpath, newv, newpath, thisproc()->uid);
}

/* int openat(int dirfd, const char *pathname, int flags, mode_t mode); */
long sys_openat(void)
{
    char *path;
    int dirfd, flags, fd;
    mode_t mode;
    struct vnode *vnode;
    struct vfile *file;
    long error;
    //print_fd_table(thisproc()->fd_table, "sys_openat_1");

    if ((error = argint(0, &dirfd)) < 0) return error;
    if ((error = argstr(1, &path)) < 0) return error;
    if ((error = argint(2, &flags)) < 0) return error;
    if ((error = argint(3, (int *)&mode)) < 0) return error;

    trace("dirfd: %d, path: %s, flags : 0x%x, mode: 0x%x", dirfd, path, flags, mode);
    if ((error = check_fdcwd(path, dirfd, &vnode)) < 0) return error;
    trace("vnode->ino: %d", vnode->ino);
    fd = find_unused_fd(thisproc()->fd_table, 0);
    trace("fd: %d", fd);

    if (mode)
        mode = (mode & ~(thisproc()->umask)) & 0777;

    trace("vnode->ino: %d, path: %s, flags : 0x%x, mode: 0x%x", vnode->ino, path, flags, mode);

    if ((error = vfs_open(vnode, path, flags, mode, thisproc()->uid, &file)) < 0) {
        error("vfs_open %s error: %d", path, error);
        return error;
    }
    trace("vfs_open ok: set file->vnode->ino: %d to fd=%d", file->vnode->ino, fd);
    set_fd(thisproc()->fd_table, fd, file);
    if (flags & O_CLOEXEC)
        bit_add(thisproc()->fdflag, fd);

    trace("fd: %d, file->vnode->ino: %d, ref: %d", fd, file->vnode->ino, file->vnode->refcount);
    //print_fd_table(thisproc()->fd_table, "sys_openat_2");
    return fd;
}

int mkdirat(int dirfd, const char *pathname, mode_t mode);
long sys_mkdirat(void)
{
    char *path;
    int dirfd;
    mode_t mode;
    struct vnode *vnode;
    struct vfile *file;
    long error;

    if ((error = argint(0, &dirfd)) < 0) return error;
    if ((error = argstr(1, &path)) < 0) return error;
    if ((error = argint(2, (int *)&mode)) < 0) return error;

    if ((error = check_fdcwd(path, dirfd, &vnode)) < 0) return error;

    mode = (mode & ~(thisproc()->umask)) & 0777;
    if ((error = vfs_open(vnode, path, O_CREAT, S_IFDIR | mode, thisproc()->uid, &file)) < 0)
        return error;
    vfs_close(file);
    return 0;
}

/* int mknodat(int dirfd, const char *pathname, mode_t mode, dev_t dev); */
long sys_mknodat(void)
{
    char *path;
    int dirfd;
    mode_t mode;
    dev_t dev;
    struct vnode *cwd, *vnode;
    long error;

    if ((error = argint(0, &dirfd)) < 0) return error;
    if ((error = argstr(1, &path)) < 0) return error;
    if ((error = argint(2, (int *)&mode)) < 0) return error;
    if ((error = argu64(3, &dev)) < 0) return error;
    if ((error = check_fdcwd(path, dirfd, &cwd)) < 0) return error;

    trace("cwd->ino: %d, path: %s, mode: 0x%x, dev: 0x%x", cwd->ino, path, mode, dev);

    if ((error = vfs_mknod(cwd, path, mode, (device_t)dev, thisproc()->uid, &vnode)) < 0)
        return error;

    vfs_release_vnode(vnode);
    return 0;
}

/* int chdir(const char *path); */
long sys_chdir(void)
{
    char *path;
    struct vnode *vnode;
    struct proc *p = thisproc();
    long error;

    if ((error = argstr(0, &path)) < 0) return error;
    trace("path: %s", path);
    if ((error = vfs_lookup(p->cwd, path, VLOOKUP_NORMAL, p->uid, &vnode)) < 0)
        return error;

    if (!S_ISDIR(vnode->mode))
        return -ENOTDIR;

    p->cwd = vfs_clone_vnode(vnode);
    trace("p->cwd: %d, vnode: %d", p->cwd->ino, vnode->ino);
    return 0;
}

/*  int execve(const char *filename, char *const argv[], char *const envp[]); */
long sys_execve(void)
{
    char *filename;
    void *argv, *envp;
    long error;

    if ((error = argstr(0, &filename)) < 0) return error;
    if ((error = argu64(1, (uint64_t *)&argv)) < 0) return error;
    if ((error = argu64(2, (uint64_t *)&envp)) < 0) return error;
    //print_fd_table(thisproc()->fd_table, "sys_ecexve");
    return execve(filename, argv, envp);
}

/* int pipe2(int pipefd[2], int flags); */
long sys_pipe2(void)
{
    int *pipefd;
    int fd[2];
    int flags;
    struct vfile *file[2];
    struct proc *p = thisproc();
    long error;

    if ((error = argint(1, &flags)) < 0) return error;
    if ((error = argptr(0, (void **)&pipefd, 2 * sizeof(int))) < 0) return error;

    if (flags & ~O_CLOEXEC) {
        return -EINVAL;
    }

    fd[0] = find_unused_fd(p->fd_table, 0);
    trace("[%d] fd0=%d", p->pid, fd[0]);
    if (fd[0] < 0) return -EMFILE;
    set_fd(p->fd_table, fd[0], (struct vfile *)&file[0]);    // fd[0]を仮押さえ

    fd[1] = find_unused_fd(p->fd_table, 0);
    trace("[%d] fd1=%d", p->pid, fd[1]);
    if (fd[1] < 0) {
        unset_fd(p->fd_table, fd[0]);
        return -EMFILE;
    }
    set_fd(p->fd_table, fd[1], (struct vfile *)&file[1]);    // fd[1]を仮押さえ

    if ((error = vfs_create_pipe(&file[0], &file[1])) < 0) {
        error("[%d] failed create pipe", p->pid);
        return error;
    }

    set_fd(p->fd_table, fd[0], file[0]);    // 本設定
    set_fd(p->fd_table, fd[1], file[1]);

    memmove((void *)pipefd, &fd[0], sizeof(int));
    memmove((void *)pipefd+sizeof(int), &fd[1], sizeof(int));

    if (flags & O_CLOEXEC) {
        bit_add(thisproc()->fdflag, fd[0]);
        bit_add(thisproc()->fdflag, fd[1]);
    }

    trace("pfd[0]: %d, pfd[1]: %d", fd[0], fd[1]);
    return 0;
}

// int ioctl(int fd, unsigned long request, ...);
long sys_ioctl(void)
{
    struct vfile *file;
    uint64_t req;
    uint64_t argp;
    long error;

    if ((error = argfd(0, 0, &file)) < 0) return error;
    if ((error = argu64(1, &req)) < 0) return error;
    if ((error = argu64(2, &argp)) < 0) return error;
    trace("[%d] fd: %d, file: %d, req: 0x%x", thisproc()->pid, fd, file->vnode->ino, req);
    return vfs_ioctl(file, req, (void *)argp, thisproc()->uid);
}

/* int fcntl(int fd, int cmd, ...); */
long sys_fcntl(void)
{
    struct vfile *file;
    struct proc *p = thisproc();
    int fd, fd1, cmd, args;
    long error;

    if ((error = argfd(0, &fd, &file)) < 0) return error;
    if ((error = argint(1, &cmd)) < 0) return error;
    if ((error = argint(2, &args)) < 0) return error;

    trace("fd: %d, cmd: 0x%x", fd, cmd);

    switch (cmd) {
        case F_DUPFD:
            if ((fd1 = find_unused_fd(p->fd_table, args)) < 0)
            return fd1;
            set_fd(p->fd_table, fd1, file);
            return fd1;
        case F_GETFD:
            return bit_test(p->fdflag, fd) ? FD_CLOEXEC : 0;
        case F_SETFD:
            if (args & FD_CLOEXEC)
                bit_add(p->fdflag, fd);
            else
                bit_remove(p->fdflag, fd);
            return 0;

        case F_GETFL:
            return (file->flags & (FILE_STATUS_FLAGS | O_ACCMODE));

        case F_SETFL:
            file->flags = ((args & FILE_STATUS_FLAGS) | (file->flags & O_ACCMODE));
            return 0;
    }

    return -EINVAL;
}

/*  off_t lseek(int fd, off_t offset, int whence); */
long sys_lseek(void)
{
    int fd;
    off_t offset;
    int whence;
    struct vfile *file;
    long error;

    if ((error = argfd(0, &fd, &file)) < 0) return error;
    if ((error = argu64(1, (uint64_t *)&offset)) < 0) return error;
    if ((error = argint(2, &whence)) < 0) return error;

    return vfs_seek(file, offset, whence);
}

// ssize_t getdents64(int fd, void *dirp, size_t count);
long sys_getdents64(void)
{
    char *buffer;
    uint64_t size;
    struct vfile *file;
    long error;

    if ((error = argfd(0, 0, &file)) < 0) return error;
    if ((error = argu64(2, &size)) < 0) return error;
    if ((error = argptr(1, (void **)&buffer, size)) < 0) return error;

    return vfs_getdents(file, (void *)buffer, size);
}

/* オープンされたファイルディスクリプター fd により参照されるファイルの許可を変更する */
// int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);
long sys_fchmodat()
{
    int dirfd, flags;
    char *path;
    mode_t mode;
    long error;
    struct vnode *vnode;

    if (argint(0, &dirfd) < 0 || argstr(1, &path) < 0
     || argint(2, (int *)&mode) < 0 || argint(3, &flags) < 0)
        return -EINVAL;

    if ((error = check_fdcwd(path, dirfd, &vnode)) < 0) return error;

    if (flags & AT_SYMLINK_NOFOLLOW)
        return -ENOTSUP;

    return vfs_chmod(vnode, path, mode, thisproc()->uid);
}

/* pathname で指定されたファイルの所有権を変更する*/
// int fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags);
long sys_fchownat(void)
{
    int dirfd, flags;
    char *path;
    uid_t owner;
    gid_t group;
    long error;
    struct vnode *vnode;

    if (argint(0, &dirfd) < 0 || argstr(1, &path) < 0
     || argint(2, (int *)&owner) < 0 || argint(3, (int *)&group) < 0
     || argint(4, &flags) < 0)
        return -EINVAL;

    trace("dirfd=%d, path=%s, uid=%d, gid=%d, flags=%d\n", dirfd, path, owner, group, flags);

    if ((error = check_fdcwd(path, dirfd, &vnode)) < 0) return error;

    if (flags)
        return -ENOTSUP;

    return vfs_chown(vnode, path, owner, group, thisproc()->uid);
}

/* オープンされたファイルディスクリプター fd により参照されるファイルの所有権を変更する */
// int fchown(int fd, uid_t owner, gid_t group);
long sys_fchown(void)
{
    //int fd;
    uid_t owner;
    gid_t group;
    struct vfile *f;

    if (argfd(0, 0, &f) < 0 || argint(1, (int *)&owner) < 0
     || argint(2, (int *)&group) < 0)
        return -EINVAL;

    return vfs_fchown(f, owner, group);
}

/* 呼び出し元プロセスがファイル pathname にアクセスできるか否かチェックする */
// int faccessat(int dirfd, const char *pathname, int mode, int flags); flags = 0
long sys_faccessat(void)
{
    int dirfd, mode;
    char *path;
    long error;
    struct vnode *vnode;

    if (argint(0, &dirfd) < 0 || argstr(1, &path) < 0
     || argint(2, &mode) < 0)
        return -EINVAL;

    if ((error = check_fdcwd(path, dirfd, &vnode)) < 0) return error;

    return vfs_access(vnode, path, mode, thisproc()->uid, 0);
}

/* 呼び出し元プロセスがファイル pathname にアクセスできるか否かチェックする */
// int faccessat(int dirfd, const char *pathname, int mode, int flags);
long sys_faccessat2(void)
{
    int dirfd, mode, flags;
    char *path;
    long error;
    struct vnode *vnode;

    if (argint(0, &dirfd) < 0 || argstr(1, &path) < 0
     || argint(2, &mode) < 0  || argint(3, &flags) < 0)
        return -EINVAL;

    if ((error = check_fdcwd(path, dirfd, &vnode)) < 0) return error;

    return vfs_access(vnode, path, mode, thisproc()->uid, flags);
}
