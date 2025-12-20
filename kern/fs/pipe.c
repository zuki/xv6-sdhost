#include <types.h>
#include <linux/errno.h>
#include <mm.h>
#include <syscall.h>
#include <proc.h>
#include <vfs.h>
#include <spinlock.h>

#include <fs/nop.h>
#include <fs/vfile.h>

#define PIPESIZE 512

int pipe_close(struct vfile *file);
int pipe_read(struct vfile *file, char *buf, size_t nbytes);
int pipe_write(struct vfile *file, const char *buf, size_t nbytes);
int pipe_release(struct vnode *vnode);
int pipe_poll(struct vfile *file, int events);

struct vfile_ops pipe_vfile_ops = {
    nop_open,
    pipe_close,
    pipe_read,
    pipe_write,
    nop_ioctl,
    nop_poll,
    nop_seek,
    nop_readdir,
    nop_getdents,
};

struct vnode_ops pipe_vnode_ops = {
    &pipe_vfile_ops,
    nop_create,
    nop_mknod,
    nop_lookup,
    nop_link,
    nop_symlink,
    nop_unlink,
    nop_rename,
    nop_truncate,
    nop_update,
    pipe_release,
    nop_rmdir,
};

struct pipe_vnode {
    struct vnode vn;            // 先頭に置くことでvnode/pipe_vnodeとして使用
    char data[PIPESIZE];
    size_t nread;               // number of bytes read
    size_t nwrite;              // number of bytes written
    int readopen;               // read fd is still open
    int writeopen;              // write fd is still open
    struct spinlock lock;
};

int vfs_create_pipe(struct vfile **rfile, struct vfile **wfile)
{
    struct vnode *vnode;
    struct pipe_vnode *pipe;

    if ((vnode = kmalloc(sizeof(struct pipe_vnode))) == NULL)
        return -ENOMEM;
    vfs_init_vnode(vnode, &pipe_vnode_ops, NULL, 0600, 1, 0, 0, 0, 0, 0, 0, 0, 0);
    vfs_clone_vnode(vnode);
    *rfile = alloc_file(vnode, 0);
    if (!*rfile) {
        kmfree(vnode);
        return -ENFILE;
    }
    *wfile = alloc_file(vnode, 0);
    if (!*wfile) {
        kmfree(vnode);
        free_vfile(*rfile);
        return -ENFILE;
    }

    pipe = (struct pipe_vnode *)vnode;
    pipe->readopen = 1;
    pipe->writeopen = 0;
    pipe->nread = 0;
    pipe->nwrite = 0;
    initlock(&pipe->lock, "pipe");

    return 0;
}

int pipe_release(struct vnode *vnode)
{
    kmfree(vnode);
    return 0;
}

int pipe_close(struct vfile *file)
{
    struct pipe_vnode *pipe = (struct pipe_vnode *)(file->vnode);
    pipe->writeopen = 0;
    wakeup(&pipe->nread);

    if (pipe->readopen == 0 && pipe->writeopen == 0) {
        kmfree(pipe);
    }
    release(&pipe->lock);
    return 0;
}

int pipe_read(struct vfile *file, char *buffer, size_t nbytes)
{
    struct pipe_vnode *pipe = (struct pipe_vnode *)(file->vnode);
    ssize_t i;

    acquire(&pipe->lock);
    while (pipe->nread == pipe->nwrite && pipe->writeopen) {
        if (thisproc()->killed) {
            release(&pipe->lock);
            return -EIO;
        }
        sleep(&pipe->nread, &pipe->lock);
    }
    for (i = 0; i < nbytes; i++) {
        if (pipe->nread == pipe->nwrite)
            break;
        buffer[i] = pipe->data[pipe->nread++ % PIPESIZE];
    }
    wakeup(&pipe->nwrite);
    release(&pipe->lock);
    return i;
}

int pipe_write(struct vfile *file, const char *buffer, size_t nbytes)
{
    struct pipe_vnode *pipe = (struct pipe_vnode *)(file->vnode);
    ssize_t i;

    acquire(&pipe->lock);
    for (i = 0; i < nbytes; i++) {
        while (pipe->nwrite == pipe->nread + PIPESIZE) {
            if (pipe->readopen == 0 || thisproc()->killed) {
                release(&pipe->lock);
                return -EBADF;
            }
            wakeup(&pipe->nread);
            sleep(&pipe->nwrite, &pipe->lock);
        }
        pipe->data[pipe->nwrite++ % PIPESIZE] = buffer[i];
    }
    wakeup(&pipe->nread);
    release(&pipe->lock);
    return nbytes;
}

#if 0
int pipe_poll(struct vfile *file, int events)
{
    int revents = 0;
    struct vnode *vnode = file->vnode;

    if ((events & VFS_POLL_READ) && (vnode->size - file->offset > 0))
        revents |= VFS_POLL_READ;
    if ((events & VFS_POLL_WRITE) && vnode->size >= PIPESIZE)
        revents |= VFS_POLL_WRITE;
    return revents;
}
#endif
