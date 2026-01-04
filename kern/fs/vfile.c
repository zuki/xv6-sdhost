
#include <types.h>
#include <fs/vfile.h>
#include <linux/errno.h>
#include <vfs.h>
#include <console.h>
#include <spinlock.h>

#define FILE_TABLE_MAX  100

/* 16 + 32 * 100 */
struct file_table {
    struct spinlock lock;
    struct vfile file[FILE_TABLE_MAX];
};

struct file_table file_table;

void init_vfile(void)
{
    initlock(&file_table.lock, "file table");
    for (int i = 0; i < FILE_TABLE_MAX; i++) {
        file_table.file[i].ops = NULL;
        file_table.file[i].vnode = NULL;
        file_table.file[i].refcount = 0;
        file_table.file[i].flags = 0;
        file_table.file[i].offset = 0;
    }
}

struct vfile *alloc_file(struct vnode *vnode, int flags)
{
    acquire(&file_table.lock);

    for (int i = 0; i < FILE_TABLE_MAX; i++) {
        if (!file_table.file[i].vnode) {
            file_table.file[i].ops = vnode->ops->fops;
            file_table.file[i].vnode = vnode;
            file_table.file[i].offset = 0;
            file_table.file[i].refcount = 1;
            file_table.file[i].flags = flags;
            release(&file_table.lock);
            return &file_table.file[i];
        }
    }

    release(&file_table.lock);
    return NULL;
}

struct vfile *dup_vfile(struct vfile *file)
{
    acquire(&file_table.lock);
    file->refcount++;
    release(&file_table.lock);
    return file;
}

void free_vfile(struct vfile *file)
{
    acquire(&file_table.lock);
    --file->refcount;
    if (file->refcount < 0) {
        error("double free of file pointer, %x", file);
    } else if (file->refcount == 0) {
        if (vfs_release_vnode(file->vnode))
            warn("failed release vnode: %d", file->vnode->ino);
        file->vnode = NULL;
    }
    release(&file_table.lock);
}
