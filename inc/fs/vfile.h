#ifndef INC_FS_VFILE_H
#define INC_FS_VFILE_H

#include <vfs.h>

void init_vfile(void);
struct vfile *alloc_file(struct vnode *vnode, int flags);
struct vfile *dup_vfile(struct vfile *file);
void free_vfile(struct vfile *file);

#endif
