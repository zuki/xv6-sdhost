
#ifndef INC_VFS_H
#define INC_VFS_H

#include <types.h>
#include <clock.h>
#include <linux/fcntl.h>
#include <linux/stat.h>
#include <linux/time.h>
#include <sleeplock.h>
#include <console.h>

#define SDMAJOR         1       // SD card major block device
#define FATMINOR        0       // FAT system partition [1,0]
#define FAT2MINOR       1       // FAT data partition [1,1]
#define XV6MINOR        2       // xv6 partition [1,2]
#define EXT2MINOR       3       // ext2 partition [1,3]
#define PROCFSMINOR     4       // procfs [1, 4]

/* v6/fatのファイルタイプ */
#define T_DIR       1   // ディレクトリ
#define T_FILE      2   // 通常ファイル
#define T_DEV       3   // ブロックデバイス
#define T_CHR       4   // キャラクタデバイス
#define T_SYMLINK   5   // シンボリックリンク
#define T_SOCK      6   // ソケット
#define T_FIFO      7   // FIFO
#define T_FILE_FAT  8   // FAT32ファイル
#define T_DIR_FAT   9   // FAT32ディレクトリ
#define T_UNKNOWN   10

#define VFS_SEP             '/'
#define VFS_FILENAME_MAX    58

#define MAXSYMLINKS         20
#define MAXPATHLEN          1024

/* struct mount のビットフラグ */
#define VFS_MBF_READ_ONLY   0x01

/* struct Vnode のビットフラグ */
#define VBF_MOUNTED         0x01
#define VBF_DIRTY           0x02

/* vfs_poolで使用する定数 */
#define VFS_POLL_READ       0x01
#define VFS_POLL_WRITE      0x02
#define VFS_POLL_ERROR      0x04
#define VFS_POLL_HUP        0x08

/* vfs_update_timeで更新する時間種別を指定する */
#define ATIME               0x01
#define MTIME               0x02
#define CTIME               0x04

enum fsname {
    FSFAT,
    FSEXT2,
    FSV6,
    FSPROCFS,
    NOTFS,
};

struct mount;
struct vnode;
struct vfile;
struct dirent;

/* mount操作関数 */
struct mount_ops {
    char *fstype;                       /* ファイルシステムタイプ名 */
    int (*init)();                      /*  boot時にファイルシステムを初期化する */
    int (*mount)(struct mount *mp, device_t dev, struct vnode *parent);
        /* 事前に割り当てたstruct mount を使ってファイルシステムをマウントする */
    int (*unmount)(struct mount *mp);   /* ファイルシステムをアンマウントする */
    int (*sync)(struct mount *mp);      /* データをディスクに書き出す */
};

/* vnode操作関数 */
struct vnode_ops {
    struct vfile_ops *fops;

    int (*create)(struct vnode *vnode, const char *filename, mode_t mode, uid_t uid, struct vnode **result);
    int (*mknod)(struct vnode *vnode, const char *filename, mode_t mode, device_t dev, uid_t uid, struct vnode **result);
    /*  NOTE: lookupは*result vnode が存在しNULLでない場合、そのvnodeに対して */
    /*  vfs_release_vnode を呼び出さなければならない。これにより、パス解決時に  */
    /*  vnode参照の交換がしやすくなる */
    int (*lookup)(struct vnode *vnode, const char *filename, struct vnode **result);
    int (*link)(struct vnode *oldvnode, struct vnode *newparent, const char *filename);
    int (*symlink)(struct vnode *parent, const char *target, const char *filename);
    int (*unlink)(struct vnode *parent, struct vnode *vnode, const char *filename);
    int (*rename)(struct vnode *vnode, struct vnode *oldparent, const char *oldname, struct vnode *newparent, const char *newname);
    int (*truncate)(struct vnode *vnode);   /* ファイルデータを切り詰める */
    int (*update)(struct vnode *vnode);
    int (*release)(struct vnode *vnode);    /*  vnodeを解放する */
    int (*rmdir)(struct vnode *vnode);
};

/* vfile操作関数 */
struct vfile_ops {
    int (*open)(struct vfile *file, int flags);
    int (*close)(struct vfile *file);
    int (*read)(struct vfile *file, char *buffer, size_t size);
    int (*write)(struct vfile *file, const char *buffer, size_t size);
    int (*ioctl)(struct vfile *file, unsigned int request, void *argp, uid_t uid);
    int (*poll)(struct vfile *file, int events);
    off_t (*seek)(struct vfile *file, off_t offset, int whence);
    int (*readdir)(struct vfile *file, struct dirent *dir);
    int (*getdents)(struct vfile *file, void *buffer, size_t size);
    int (*writeback)(struct vfile *file, off_t offset, uint64_t addr);
    int (*chown)(struct vfile *file, uid_t owner, gid_t group);
};

/* マウント済みファイルシステムの数を表し、マウント済みファイルシステムの走査に使用する */
struct mount_iter {
    int slot;
};

/* マウント構造体 */
struct mount {
    struct mount_ops *ops;
    struct vnode *mount_node;   /*  このfsがマウントされているvnode */
    struct vnode *root_node;    /*  このfsのroot vnode */
    void *super;                /*  fs固有のスーパーブロックデータ */
    device_t dev;               /*  マウントされているデバイス */
    uint32_t bits;              /*  このマウントポイント用のビットフラグ */
};

/* vnode構造体 */
struct vnode {
    struct vnode_ops *ops;      /*  0: */
    struct mount *mp;           /*  8: このvnodeが属しているマウントポイント */
    int refcount;               /* 16: このvnodeを使用中の参照カウント */

    mode_t mode;                /* 20: */
    int nlink;                  /* 24: */
    uid_t uid;                  /* 28: */
    gid_t gid;                  /* 32: */
    uint32_t bits;              /* 36: vnodeビットフラグ*/
    device_t rdev;              /* 40: */
    ino_t ino;                  /* 44: */
    off_t size;                 /* 48: */
    enum fsname fsname;

    struct timespec atime;      /* 54:  */
    struct timespec mtime;      /* */
    struct timespec ctime;

    void *data;                 /*  ファイルシステム固有のデータ */
};

/* vfile構造体 : 32 byte */
struct vfile {
    struct vfile_ops *ops;
    struct vnode *vnode;
    int refcount;
    int flags;
    off_t offset;
};

#define VLOOKUP_NORMAL       0x00
#define VLOOKUP_PARENT_OF    0x01

/* ディレクトリエントリ */

/* ディレクトリエントリ構造体 */
struct dirent {
    uint32_t  ino;
    uint16_t  type;
    char name[VFS_FILENAME_MAX];
};

#define DESIZE (sizeof(struct dirent))

/* ファイルシステム独立なディレクトリエントリ */
struct dirent64 {
    ino64_t     d_ino;
    off64_t     d_off;
    uint16_t    d_reclen;
    uint8_t     d_type;
    char        d_name[256];
};

#define DT_UNKNOWN  0
#define DT_FIFO     1
#define DT_CHR      2
#define DT_DIR      4
#define DT_BLK      6
#define DT_REG      8
#define DT_LNK      10
#define DT_SOCK     12
#define DT_WHT      14
#define IFTODT(x) ((x)>>12 & 017)
#define DTTOIF(x) ((x)<<12)

int init_vfs(void);

struct mount *get_rootfs(void);

void vfs_mount_iter_start(struct mount_iter *iter);
struct mount *vfs_mount_iter_next(struct mount_iter *iter);
int vfs_mount(struct vnode *cwd, const char *path, device_t dev, struct mount_ops *ops, int mountflags, uid_t uid);
int vfs_unmount(device_t dev, uid_t uid);
int vfs_sync(device_t dev);

int vfs_lookup(struct vnode *cwd, const char *path, int flags, uid_t uid, struct vnode **result);
int vfs_reverse_lookup(struct vnode *cwd, char *buf, size_t size, uid_t uid);
int vfs_access(struct vnode *cwd, const char *path, int mode, uid_t uid, int flags);
int vfs_chmod(struct vnode *cwd, const char *path, int mode, uid_t uid);
int vfs_chown(struct vnode *cwd, const char *path, uid_t owner, gid_t group, uid_t uid);
int vfs_mknod(struct vnode *cwd, const char *path, mode_t mode, device_t dev, uid_t uid, struct vnode **result);
int vfs_link(struct vnode *oldv, char *oldpath, struct vnode *newv, const char *newpath, uid_t uid);
int vfs_symlink(struct vnode *cwd, const char *oldpath, const char *newpath);
int vfs_unlink(struct vnode *cwd, const char *path, int flags, uid_t uid);
int vfs_readlink(struct vnode *cwd, const char *path, char *buf, size_t bufsize, uid_t uid);
int vfs_rename(struct vnode *oldv, const char *oldpath, struct vnode *newv, const char *newpath, uid_t uid);
int vfs_open(struct vnode *cwd, const char *path, int flags, mode_t mode, uid_t uid, struct vfile **file);

int vfs_close(struct vfile *file);
int vfs_read(struct vfile *file, char *buffer, size_t size);
int vfs_write(struct vfile *file, const char *buffer, size_t size);
int vfs_ioctl(struct vfile *file, unsigned int request, void *argp, uid_t uid);
int vfs_poll(struct vfile *file, int events);
off_t vfs_seek(struct vfile *file, off_t offset, int whence);
int vfs_readdir(struct vfile *file, struct dirent *dir);
int vfs_getdents(struct vfile *file, void *buffer, size_t size);
int vfs_writeback(struct vfile *file, off_t offset, uint64_t addr);

int vfs_fchown(struct vfile *file, uid_t owner, gid_t group);

int vfs_create_pipe(struct vfile **rfile, struct vfile **wfile);

const char *path_last_component(const char *path);
int path_valid_component(const char *path);
int verify_mode_access(uid_t current_uid, mode_t require_mode, uid_t file_uid, gid_t file_gid, mode_t file_mode, int flags);

static inline enum fsname get_fsname(struct vnode *vnode)
{
    return vnode->fsname;
}

static inline void vfs_init_vnode(
    struct vnode *vnode,
    struct vnode_ops *ops,
    struct mount *mp,
    mode_t mode,
    int nlink,
    uid_t uid,
    gid_t gid,
    device_t rdev,
    ino_t ino,
    off_t size,
    enum fsname fsname,
    struct timespec *atime,
    struct timespec *mtime,
    struct timespec *ctime)
{
    vnode->ops = ops;
    vnode->mp = mp;
    vnode->refcount = 1;
    vnode->mode = mode;
    vnode->nlink = nlink;
    vnode->uid = uid;
    vnode->gid = gid;
    vnode->bits = 0;
    vnode->rdev = rdev;
    vnode->ino = ino;
    vnode->size = size;
    vnode->fsname = fsname;
    if (atime) {
        vnode->atime.tv_sec = atime->tv_sec;
        vnode->atime.tv_nsec = atime->tv_nsec;
    } else {
        vnode->atime.tv_sec = 0;
        vnode->atime.tv_nsec = 0;
    }
    if (mtime) {
        vnode->mtime.tv_sec = mtime->tv_sec;
        vnode->mtime.tv_nsec = mtime->tv_nsec;
    } else {
        vnode->mtime.tv_sec = 0;
        vnode->mtime.tv_nsec = 0;
    }
    if (ctime) {
        vnode->ctime.tv_sec = ctime->tv_sec;
        vnode->ctime.tv_nsec = ctime->tv_nsec;
    } else {
        vnode->ctime.tv_sec = 0;
        vnode->ctime.tv_nsec = 0;
    }

}

static inline struct vnode *vfs_clone_vnode(struct vnode *vnode)
{
    vnode->refcount++;
    return vnode;
}

int vfs_release_vnode(struct vnode *vnode);

static inline void vfs_update_time(struct vnode *vnode, char update)
{
    /*  fsが読み取り専用でマウントされていない場合のみ、タイムスタンプを更新する */
    if (vnode->mp->bits & VFS_MBF_READ_ONLY)
        return;

    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    if (update & ATIME) {
        vnode->atime.tv_nsec = t.tv_nsec;
        vnode->atime.tv_sec = t.tv_sec;
    }
    if (update & MTIME) {
        vnode->mtime.tv_nsec = t.tv_nsec;
        vnode->mtime.tv_sec = t.tv_sec;
    }
    if (update & CTIME) {
        vnode->ctime.tv_nsec = t.tv_nsec;
        vnode->ctime.tv_sec = t.tv_sec;
    }
    vnode->bits |= VBF_DIRTY;
}

static inline void dump_mp(struct mount *mp, const char *title)
{
    debug("=== dump mount point from %s ===", title);
    debug("fstype: %s", mp->ops ? mp->ops->fstype : "ND");
    debug("mount_node: 0x%llx (%d)", mp->mount_node, mp->mount_node ? mp->mount_node->ino : -1);
    debug("root_node: 0x%llx (%d)", mp->root_node, mp->root_node->ino);
    debug("super: 0x%llx", mp->super);
    debug("dev: 0x%x", mp->dev);
    debug("bits: 0x%x\n", mp->bits);
}

#endif
