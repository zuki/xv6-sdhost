#ifndef I_FS_FATFS_FS_H
#define I_FS_FATFS_FS_H

#include <types.h>
#include <vfs.h>
#include <sleeplock.h>
#include <fs/fatfs/ff.h>

#define MAXPATH    128
#define FAT_ROOTINO 2

struct fat_inode {
    struct vnode        vnode;      // vnode構造体
    struct sleeplock    lock;       // 以下のフィールドを保護
    int                 valid;
    uint16_t            type;       // fatファイルタイプ
    FIL *               fatfp;
    DIR *               fatdir;
};

#define ITOV(ip)    ((struct vnode *)ip)
#define VTOI(vp)    ((struct fat_inode *)vp->data)

long fat_open(char *path, int flags, mode_t mode);
long fat_unlink(char *path);
long fat_mkdir(char *path);
long fat_chdir(char *path);
boolean is_fatfs(const char *path);

#endif
