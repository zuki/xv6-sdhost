#ifndef INC_V6_FILE_H
#define INC_V6_FILE_H

#include <linux/stat.h>
#include <types.h>
#include <vfs.h>
#include <sleeplock.h>
#include <fs/v6/fs.h>
#include <fs/v6/param.h>
#include <net/socket.h>

/* v6mountops.c */
int v6_init(void);
int v6_mount(struct mount *mp, device_t dev, struct vnode *parent);
int v6_unmount(struct mount *mp);
int v6_sync(struct mount *mp);
void v6_set_super(void);

/* v6vfileops.c */
int v6_open(struct vfile *file, int flags);
int v6_close(struct vfile *file);
int v6_read(struct vfile *file, char *buffer, size_t size);
int v6_write(struct vfile *file, const char *buffer, size_t size);
int v6_ioctl(struct vfile *file, unsigned int request, void *argp, uid_t uid);
int v6_poll(struct vfile *file, int events);
off_t v6_seek(struct vfile *file, off_t offset, int whence);
int v6_readdir(struct vfile *file, struct dirent *dir);
int v6_getdents(struct vfile *file, void *buffer, size_t size);
int v6_writeback(struct vfile *file, off_t offset, uint64_t addr);
int v6_chown(struct vfile *file, uid_t owner, gid_t group);

/* v6vnodeops.c */
int v6_create(struct vnode *vnode, const char *filename, mode_t mode, uid_t uid, struct vnode **result);
int v6_mknod(struct vnode *vnode, const char *filename, mode_t mode, device_t dev, uid_t uid, struct vnode **result);
int v6_lookup(struct vnode *vnode, const char *filename, struct vnode **result);
int v6_link(struct vnode *oldvnode, struct vnode *newparent, const char *filename);
int v6_symlink(struct vnode *cwd, const char *target, const char *filename);
int v6_unlink(struct vnode *parent, struct vnode *vnode, const char *filename);
int v6_rename(struct vnode *vnode, struct vnode *oldparent, const char *oldname, struct vnode *newparent, const char *newname);
int v6_truncate(struct vnode *vnode);
int v6_update(struct vnode *vnode);
int v6_release(struct vnode *vnode);
int v6_stat(struct vnode *vnode, struct stat *st);
int v6_rmdir(struct vnode *vnode);

#endif
