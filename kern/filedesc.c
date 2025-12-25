#include <types.h>
#include <linux/errno.h>
#include <vfs.h>
#include <console.h>
#include <filedesc.h>
#include <fs/vfile.h>

void init_fd_table(fd_table_t table)
{
    for (int i = 0; i < OPEN_MAX; i++)
        table[i] = NULL;
}

void release_fd_table(fd_table_t table)
{
    for (int i = 0; i < OPEN_MAX; i++) {
        if (table[i])
            vfs_close(table[i]);
    }
}

void dup_fd_table(fd_table_t dest, fd_table_t source)
{
    for (int i = 0; i < OPEN_MAX; i++) {
        if (source[i])
            dest[i] = dup_vfile(source[i]);
    }
}

int find_unused_fd(fd_table_t table, int start)
{
    for (int i = start; i < OPEN_MAX; i++) {
        if (!table[i])
            return i;
    }
    return -EMFILE;
}

struct vfile *get_fd(fd_table_t table, int fd)
{
    if (fd >= OPEN_MAX || !table[fd]->vnode)
        return NULL;
    return table[fd];
}

void set_fd(fd_table_t table, int fd, struct vfile *file)
{
    if (fd >= OPEN_MAX)
        return;
    table[fd] = file;
}

void dup_fd(fd_table_t table, int fd, struct vfile *file)
{
    if (fd >= OPEN_MAX)
        return;
    table[fd] = dup_vfile(file);
}

void unset_fd(fd_table_t table, int fd)
{
    set_fd(table, fd, NULL);
}

struct vfile *get_vnode(fd_table_t table, struct vnode *vnode)
{
    for (int i = 0; i < OPEN_MAX; i++) {
        if (table[i]->vnode == vnode) {
            trace("hit with %d", i);
            return table[i];
        }
    }
    trace("no hit");
    return NULL;
}
