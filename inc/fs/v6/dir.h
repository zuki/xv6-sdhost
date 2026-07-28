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
int v6_dir_is_empty(struct vnode *dir);

 int dir_find_entry_by_inode(struct vnode *dir, ino_t ino, struct dirent *de, off_t *offp);

/* ディレクトリ dir で名前が filename のディレクトリエントリを探して返す */
int dir_find_entry_by_name(struct vnode *dir,  const char *filename, struct dirent *de, off_t *offp);

int v6_write_dirent(struct vnode *dir, struct dirent *de, off_t offset);

#endif
