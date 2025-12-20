#ifndef INC_FILEDESC_H
#define INC_FILEDESC_H

#include <vfs.h>

#define OPEN_MAX	20

typedef struct vfile *fd_table_t[OPEN_MAX];

struct vnode;

void init_fd_table(fd_table_t table);
void release_fd_table(fd_table_t table);
void dup_fd_table(fd_table_t dest, fd_table_t source);
int find_unused_fd(fd_table_t table, int start);
struct vfile *get_fd(fd_table_t table, int fd);
void set_fd(fd_table_t table, int fd, struct vfile *file);
void dup_fd(fd_table_t table, int fd, struct vfile *file);
void unset_fd(fd_table_t table, int fd);
struct vfile *get_vnode(fd_table_t table, struct vnode *vnode);

#endif
