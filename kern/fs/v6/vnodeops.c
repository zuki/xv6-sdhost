/* File descriptors */

#include <types.h>
#include <vfs.h>
#include <fs/v6/fs.h>
#include <fs/v6/file.h>
#include <fs/v6/dir.h>
#include <spinlock.h>
#include <sleeplock.h>
#include <console.h>
#include <string.h>
#include <linux/errno.h>
#include <linux/time.h>

struct vnode_ops v6_vnode_ops = {
    &v6_file_ops,
    v6_create,
    v6_mknod,
    v6_lookup,
    v6_link,
    v6_symlink,
    v6_unlink,
    v6_rename,
    v6_truncate,
    v6_update,
    v6_release,
    v6_rmdir,
};

int v6_create(struct vnode *parent, const char *name, mode_t mode, uid_t uid, struct vnode **result)
{
    struct vnode *vnode;
    struct v6_inode *ip, *dp = VTOI(parent);
    struct timespec ts;
    struct proc *p = thisproc();
    int err = -EINVAL;
    ino_t ino;
    uint16_t v6type = mode2v6type(mode);

    trace("parent->ino: %d, name: %s", parent->ino, name);

    if (!result)
        return -EINVAL;

    v6_ilock(dp);
    if ((ip = v6_ialloc(parent->mp, v6type)) == 0) {
        return -ENOMEM;
    }
    v6_ilock(ip);
    //v6_dump(ip, "v6_create 1");
    vnode = ITOV(ip);
    clock_gettime(CLOCK_REALTIME, &ts);
    ip->type = v6type;
    //ip->valid = 1;
    vnode->rdev = parent->rdev;
    vnode->mode = mode;
    vnode->uid = uid;
    vnode->nlink = 1;
    memmove(&vnode->atime, &ts, sizeof(struct timespec));
    memmove(&vnode->mtime, &ts, sizeof(struct timespec));
    memmove(&vnode->ctime, &ts, sizeof(struct timespec));
    memset(ip->addrs, 0, sizeof(uint32_t)*(NDIRECT+2));
    v6_iupdate(ip);
    //v6_dump(ip, "v6_create 2");
    if (S_ISDIR(mode)) {  // Create . and .. entries.
        vfs_clone_vnode(parent);
        v6_iupdate(dp);
        if (v6_dirlink(ip, ".", ino, v6type) < 0 || v6_dirlink(ip, "..", vnode->ino, mode2v6type(vnode->mode)) < 0) {
            err = -ENOMEM;
            goto fail;
        }
    }

    if ((err = v6_dirlink(dp, name, vnode->ino, v6type)) < 0)
        goto fail;

    *result = ITOV(ip);
    v6_iunlockput(dp);
    v6_iunlockput(ip);
    //v6_dump(VTOI(*result), "v6_create 3");

    return 0;

fail:
    // something went wrong. de-allocate ip.
    //v6_release(ITOV(ip));
    return err;
}

int v6_mknod(struct vnode *parent, const char *filename, mode_t mode, device_t dev, uid_t uid, struct vnode **result)
{
    struct vnode *vnode;
    int err;
    if ((err = v6_create(parent, filename, mode, uid, &vnode)) < 0)
        return err;
    if (result)
        *result = vnode;
    return 0;
}

int v6_lookup(struct vnode *vnode, const char *filename, struct vnode **result)
{
    struct v6_inode *ip;
    trace("vnode->ino: %d, mode: 0x%x, ip->valid: %d, COMP: %s", vnode->ino, vnode->mode, VTOI(vnode)->valid, filename);
    v6_ilock(VTOI(vnode));
    ip = v6_dirlookup(VTOI(vnode), filename);
    v6_iunlockput(VTOI(vnode));
    if (ip) {
        trace("OK: %s, ip->ino: %d, ip->type: %d", filename, ITOV(ip)->ino, ip->type);
        v6_ilock(ip);
        v6_iunlock(ip);
        if (result)
            *result = ITOV(ip);
        return 0;
    } else {
        return -ENOENT;
    }
}

int v6_link(struct vnode *oldvnode, struct vnode *newparent, const char *filename)
{
    struct v6_inode *parent = VTOI(newparent);
    uint16_t type = mode2v6type(oldvnode->mode);
    return v6_dirlink(parent, filename, oldvnode->ino, type);
}

int v6_symlink(struct vnode *parent, const char *target, const char *filename)
{
    struct proc *p = thisproc();
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct v6_inode *ip = v6_ialloc(parent->mp, T_SYMLINK);

    v6_ilock(ip);
    vfs_init_vnode(&ip->vnode, &v6_vnode_ops, parent->mp, S_IFLNK|(0777 & ~p->umask), 1, p->uid, p->gid, parent->rdev, ITOV(ip)->ino, strlen(target), &ts, &ts, &ts);
    v6_iupdate(ip);
    return v6_dirlink(VTOI(parent), filename, ITOV(ip)->ino, T_SYMLINK);
}

int v6_unlink(struct vnode *parent, struct vnode *vnode, const char *filename)
{
    struct dirent de, de0;
    int err;
    off_t offset;

    memset(&de0, 0, DESIZE);

    if (!strncmp(filename, ".", DIRSIZ) || !strncmp(filename, "..", DIRSIZ))
        return -EPERM;
    if (vnode->nlink > 1)
        return -EBUSY;

    if ((err = dir_find_entry_by_inode(parent, vnode->ino, &de, &offset)) < 0)
        return err;

    return v6_write_dirent(parent, &de0, offset);
}

int v6_rename(struct vnode *vnode, struct vnode *oldparent, const char *oldname, struct vnode *newparent, const char *newname)
{
    struct dirent olddir, newdir;
    struct dirent de;
    int err;
    off_t oldoff, newoff;

    err = dir_find_entry_by_name(newparent, newname, &newdir, &newoff);
    if (err == 0)
        return -EEXIST;
    else if (err != -ENOENT)
        return err;

    err = dir_find_entry_by_name(oldparent, oldname, &olddir, &oldoff);
    if (err < 0)
        return err;

    memset(&de, 0, sizeof(de));
    de.ino = vnode->ino;
    de.type = mode2v6type(vnode->mode);
    memmove(de.name, newname, DIRSIZ);
    err = v6_write_dirent(newparent, &de, newoff);
    if (err < 0)
        return -ENOSPC;

    olddir.ino = 0;
    return v6_write_dirent(oldparent, &olddir, oldoff);
}

int v6_truncate(struct vnode *vnode)
{
    struct v6_inode *ip = VTOI(vnode);
    struct timespec tp;

    if (S_ISDIR(vnode->mode))
        return -EISDIR;

    v6_ilock(ip);
    v6_itrunc(ip);
    clock_gettime(CLOCK_REALTIME, &tp);
    vnode->mtime.tv_sec = tp.tv_sec;
    vnode->mtime.tv_nsec = tp.tv_nsec;
    vnode->bits |= VBF_DIRTY;
    v6_iunlockput(ip);
    return 0;
}

int v6_update(struct vnode *vnode)
{
    struct v6_inode *ip = VTOI(vnode);
    v6_ilock(ip);
    v6_iupdate(ip);
    v6_iunlockput(ip);

    return 0;
}

int v6_release(struct vnode *vnode)
{
    struct v6_inode *ip = VTOI(vnode);

    if (vnode->ino == 0 || vnode->refcount == 0) {
        trace("ino: %d, refcount: %d", vnode->ino, vnode->refcount);
        v6_ilock(ip);
        ip->type = 0;
        vnode->ino = 0;
        vnode->nlink = 0;
        v6_iupdate(ip);
        v6_iunlockput(ip);
    }
    return 0;
}

int v6_rmdir(struct vnode *vnode)
{
    struct v6_inode *dp = VTOI(vnode);
    int err;

    err= v6_dir_is_empty(vnode);
    if (err == 1)
        return -ENOTEMPTY;
    else if (err < 0)
        return err;

    v6_ilock(dp);
    if (vnode->nlink > 1) {
        v6_iunlockput(dp);
        return -EBUSY;
    }

    dp->type = 0;
    dp->valid = 0;
    v6_itrunc(dp);
    v6_iunlockput(dp);
    return 0;
}
