
#ifndef INC_FS_PROCFS_PROCFS_H
#define INC_FS_PROCFS_PROCFS_H

#include <types.h>
#include <vfs.h>

#define PROCFS_DATA(vnode)    (((struct procfs_vnode *) (vnode))->data)

#define PFN_ROOTDIR     0
#define PFN_PROCDIR     1
#define PFN_CMDLINE     2
#define PFN_STAT        3
#define PFN_STATM       4

#define PFN_MOUNTS      10


typedef int procfs_filenum_t;
typedef int (*procfs_data_t)(struct proc *proc, char *buffer, int max);
typedef int (*procfs_func2_t)(struct vfile *file, char *buffer, int max);

struct procfs_dir_entry {
    procfs_filenum_t filenum;
    char *filename;
    procfs_data_t func;
    procfs_func2_t func2;
};

struct procfs_data {
    pid_t pid;
    int filenum;
};

struct procfs_vnode {
    struct vnode vn;
    struct procfs_data data;
};

int procfs_init();
int procfs_mount(struct mount *mp, device_t dev, struct vnode *parent);
int procfs_unmount(struct mount *mp);

int procfs_lookup(struct vnode *vnode, const char *name, struct vnode **result);
int procfs_release(struct vnode *vnode);

int procfs_open(struct vfile *file, int flags);
int procfs_close(struct vfile *file);
int procfs_read(struct vfile *file, char *buf, size_t nbytes);
int procfs_write(struct vfile *file, const char *buf, size_t nbytes);
int procfs_ioctl(struct vfile *file, unsigned int request, void *argp, uid_t uid);
off_t procfs_seek(struct vfile *file, off_t position, int whence);
int procfs_readdir(struct vfile *file, struct dirent *dir);

struct procfs_dir_entry *get_dir_entry_by_index(const char *dir, int index);

#endif
