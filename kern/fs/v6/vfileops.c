/* File descriptors */

#include <types.h>
#include <param.h>
#include <vfs.h>
#include <fs/v6/param.h>
#include <fs/v6/fs.h>
#include <fs/v6/file.h>
#include <spinlock.h>
#include <sleeplock.h>
#include <console.h>
#include <string.h>
#include <linux/errno.h>

struct vfile_ops v6_file_ops = {
    v6_open,
    v6_close,
    v6_read,
    v6_write,
    v6_ioctl,
    v6_poll,
    v6_seek,
    v6_readdir,
    v6_getdents,
};


int v6_open(struct vfile *file, int flags)
{
    // 特にすることはない
    return 0;
}

/* Close file f. refcountはさわらない */
int v6_close(struct vfile *file)
{
    v6_iput(FTOI(file));
    return 0;
}

/* Read from file f. */
int v6_read(struct vfile *file, char *buffer, size_t size)
{
    int r;
    trace("file: 0x%x,", file);
    trace("vnode: 0x%x", file->vnode);
    trace("inode: 0x%x", file->vnode->data);
    struct v6_inode *ip = FTOI(file);

    v6_ilock(ip);
    if ((r = v6_readi(ip, buffer, file->offset, size)) > 0)
        file->offset += r;
    v6_iunlock(ip);
    return r;
}


/* Write to file f. */
int v6_write(struct vfile *file, const char *buffer, size_t size)
{
    int error;
    struct v6_inode *ip = FTOI(file);

    size_t max = ((MAXOPBLOCKS - 1 - 1 - 2) / 2) * SECTOR_SIZE;
    size_t i = 0;
    while (i < size) {
        size_t n1 = size - i;
        if (n1 > max)
            n1 = max;

        v6_ilock(ip);
        if ((error = v6_writei(ip, buffer + i, file->offset, n1)) > 0)
            file->offset += error;
        v6_iunlock(ip);

        if (error < 0)
            break;
        if (error != n1) {
            error = -EIO;
            break;
        }
        i += error;
    }
    return i == size ? size : error;
}

int v6_ioctl(struct vfile *file, unsigned int request, void *argp, uid_t uid)
{
    return -ENOTTY;
}

int v6_poll(struct vfile *file, int events)
{
    return 0;
}

off_t v6_seek(struct vfile *file, off_t offset, int whence)
{
    switch(whence) {
        case SEEK_SET:
            if (offset < 0)
                goto bad;
            else
                file->offset = offset;
            break;
        case SEEK_CUR:
            if (file->offset + offset < 0)
                goto bad;
            else
                file->offset += offset;
            break;
        case SEEK_END:
            if (file->vnode->size + offset < 0)
                goto bad;
            else
                file->offset = file->vnode->size + offset;
            break;
        default:
            goto bad;
    }

    return file->offset;

bad:
    error("invalid offset %d", offset)
    return -EINVAL;
}

int v6_readdir(struct vfile *file, struct dirent *dir)
{
    struct dirent de;
    int error;
    int offset = file->offset;

    for (; offset < file->vnode->size; offset += DESIZE) {
        error = v6_read(file, (char *)&de, DESIZE);
        if (error < 0 || error != DESIZE) {
            return error;
        }
        if (de.ino != 0) {
            memmove(dir, &de, DESIZE);
            return file->offset;
        }
    }
    return 0;
}

int v6_getdents(struct vfile *file, void *buffer, size_t size)
{
    size_t n;
    int namelen, reclen, tlen = 0;
    off_t offset = file->offset;
    struct dirent de;
    struct dirent64 de64;

    while (1) {
        n = v6_read(file, (char *)&de, DESIZE);
        if (n == 0) {
            trace("read 0");
            return tlen ? tlen : 0;
        }
        if (n < 0 || n != DESIZE) {
            error("read invalid n=%ld, tlen=%ld", n, tlen);
            return tlen ? tlen : -EIO;
        }

        if (de.ino == 0)
            continue;

        namelen = MIN(strlen(de.name), DIRSIZ) + 1;
        reclen = (size_t)(&((struct dirent64*)0)->d_name);
        reclen = reclen + namelen;
        reclen = (reclen + 0x7) & ~0x7;

        // sizeまで詰めたらreturn
        if ((tlen + reclen) > size) {
            trace("break; tlen: %d, reclen: %d, size: %d", tlen, reclen, size);
            break;
        }
        de64.d_ino = de.ino;
        de64.d_off = offset;
        de64.d_reclen = reclen;
        de64.d_type = IFTODT(file->vnode->mode);
        memmove(de64.d_name, de.name, namelen);
        memmove(buffer + tlen, (char *)&de64, reclen);
        tlen += reclen;
        offset = file->offset;
    }
    return tlen;
}
