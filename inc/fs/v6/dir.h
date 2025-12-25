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
#include <console.h>

/* ディレクトリ dir が空の場合は 1を, エントリがあれば 0 を
 * エラーがあったら負値を返す */
static int v6_dir_is_empty(struct vnode *dir)
{
    struct v6_inode *dp = VTOI(dir);
    struct dirent de;
    int ret = 1;

    for (int offset = 0; offset < dir->size; offset += DESIZE) {
        if (v6_readi(dp, (char *)&de, offset, DESIZE) < 0)
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

static int dir_find_entry_by_inode(struct vnode *dir, ino_t ino, struct dirent *de, off_t *offp)
{
    struct v6_inode *dp = VTOI(dir);
    struct dirent de0;
    int err;

    for (int offset = 0; offset < dir->size; offset += DESIZE) {
        err = v6_readi(dp, (char *)&de0, offset, DESIZE);
        if (err < 0 || err != DESIZE) {
            return err;
        }
        if (de0.ino == ino) {
            if (offp) *offp = offset;
            if (de) memmove(de, &de0, DESIZE);
            return 0;
        }
    }
    return -ENOENT;
}

/* ディレクトリ dir で名前が filename のディレクトリエントリを探して返す */
static int dir_find_entry_by_name(struct vnode *dir,  const char *filename, struct dirent *de, off_t *offp)
{
    struct v6_inode *dp = VTOI(dir);
    struct dirent de0;
    int err;
    trace("dp->ino: %d, filename: '%s'", dir->ino, filename);
    for (int offset = 0; offset < dir->size; offset += DESIZE) {
        err = v6_readi(dp, (char *)&de0, offset, DESIZE);
        if (err < 0 || err != DESIZE) {
            return err;
        }
        trace("de0.name: '%s'", de0.name);
        if (strncmp(filename, de0.name, DIRSIZ) == 0) {
            if (offp) *offp = offset;
            if (de) memmove(de, &de0, DESIZE);
            return 0;
        }
    }
    return -ENOENT;
}

static int v6_write_dirent(struct vnode *dir, struct dirent *de, off_t offset) {
    struct v6_inode *dp = VTOI(dir);
    struct timespec tp;
    int err;

    err = v6_writei(dp, (char *)de, offset, DESIZE);
    if (err < 0)
        return err;

    clock_gettime(CLOCK_REALTIME, &tp);
    dir->mtime.tv_sec = tp.tv_sec;
    dir->mtime.tv_nsec = tp.tv_nsec;
    dir->bits |= VBF_DIRTY;

    return 0;
}

#endif
