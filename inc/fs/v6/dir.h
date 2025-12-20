#ifndef INC_FS_V6_DIR_H
#define INC_FS_V6_DIR_H

#include <types.h>
#include <vfs.h>
#include <clock.h>
#include <string.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <fs/v6/fs.h>
#include <filedesc.h>

/* ディレクトリ dir が空の場合は 1を, エントリがあれば 0 を
 * エラーがあったら負値を返す */
static int v6_dir_is_empty(struct vnode *dir)
{
    struct vfile *file = get_vnode(thisproc()->fd_table, dir);
    struct dirent de;
    int ret = 1;

    for (int offset = 0; offset < dir->size; offset += DESIZE) {
        if (v6_read(file, (char *)&de, DESIZE) < 0)
            return -EIO;
        if (de.ino != 0) {
            if (strncmp("..",de.name, 2) == 0)
                continue;
            else if (strncmp(".", de.name, 1) == 0)
                continue;
            else {
                ret = 0;
                break;
            }
        }
    }
    return ret;
}

static int dir_find_entry_by_inode(struct vnode *dir, ino_t ino, struct dirent *de)
{
    struct vfile *file = get_vnode(thisproc()->fd_table, dir);
    struct dirent de0;
    int err;

    for (int offset = 0; offset < dir->size; offset += DESIZE) {
        err = v6_read(file, (char *)&de0, DESIZE);
        if (err < 0 || err != DESIZE) {
            return err;
        }
        if (de0.ino == ino) {
            if (de) memmove(de, &de0, DESIZE);
            return 0;
        }
    }
    return -ENOENT;
}

/* ディレクトリ dir で名前が filename のディレクトリエントリを探して返す */
static int dir_find_entry_by_name(struct vnode *dir,  const char *filename, struct dirent *de)
{
    struct vfile *file = get_vnode(thisproc()->fd_table, dir);
    struct dirent de0;
    int err;

    for (int offset = 0; offset < dir->size; offset += DESIZE) {
        err = v6_read(file, (char *)&de0, DESIZE);
        if (err < 0 || err != DESIZE) {
            return err;
        }
        if (strncmp(filename, de0.name, DIRSIZ) == 0) {
            if (de) memmove(de, &de0, DESIZE);
            return 0;
        }
    }
    return -ENOENT;
}

static int v6_write_dirent(struct vnode *dir, struct dirent *de, boolean adj) {
    struct vfile *file = get_vnode(thisproc()->fd_table, dir);
    struct timespec tp;
    int err;

    if (adj)
        file->offset -= DESIZE;
    err = v6_write(file, (char *)de, DESIZE);
    if (err < 0)
        return err;

    clock_gettime(CLOCK_REALTIME, &tp);
    dir->mtime.tv_sec = tp.tv_sec;
    dir->mtime.tv_nsec = tp.tv_nsec;
    dir->bits |= VBF_DIRTY;

    return 0;
}

#endif
