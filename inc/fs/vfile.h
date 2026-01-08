#ifndef INC_FS_VFILE_H
#define INC_FS_VFILE_H

#include <vfs.h>
#include <linux/fcntl.h>

#define FILE_READABLE(file) ((((file->flags) & O_ACCMODE) == O_RDWR) || (((file->flags) & O_ACCMODE) == O_RDONLY))

#define FILE_WRITABLE(file) ((((file->flags) & O_ACCMODE) == O_RDWR) || (((file->flags) & O_ACCMODE) == O_WRONLY))

void init_vfile(void);
struct vfile *alloc_file(struct vnode *vnode, int flags);
struct vfile *dup_vfile(struct vfile *file);
void free_vfile(struct vfile *file);

#endif
