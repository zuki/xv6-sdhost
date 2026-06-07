#ifndef INC_FS_V6_FS_H
#define INC_FS_V6_FS_H

#include <types.h>
#include <vfs.h>
#include <sleeplock.h>
#include <clock.h>
#include <linux/stat.h>
#include <fs/v6/param.h>

/* v6ファイルシステムのディスクレイアウト:
 * [ boot block | super block | log | inode blocks | free bit map | data blocks ]
 *
 * usr/mkfs で作成
 */
struct v6_superblock {
  uint32_t size;         // v6 fs imageのブロック単位のサイズ       0x03e8
  uint32_t nblocks;      // データブロック数                        0x03c3
  uint32_t ninodes;      // dinode (= inode) 数                     0x00c8
  uint32_t nlog;         // ログブロック数（領域は取るが未使用）    0x001e
  uint32_t logstart;     // ログブロックの開始ブロック番号          0x0002
  uint32_t inodestart;   // inodeブロックの開始ブロック番号         0x0020
  uint32_t bmapstart;    // 未使用マップブロックの開始ブロック番号  0x0024
};

/* v6_vfs構造体へのポインタを返す */
#define getfs(vfsp) ((struct v6_superblock *)((struct v6_vfs *)vfsp->vfs_data)->vfs_sb)

/* in-memory v6_inode structure */
struct v6_inode {
    struct vnode        vnode;      // vnode構造体
    struct sleeplock    lock;       // 以下のフィールドを保護
    int                 valid;      // inodeはdiskから読み込まれたか?
    // v6_dinodeのtypeとaddrs[]以外はvnodeにあり。
    uint16_t            type;       // v6ファイルタイプ
    uint32_t addrs[NDIRECT+2];      // データブロックアドレス
};

/* On-disk inode構造体 : 128バイト */
struct v6_dinode {
    uint16_t            type;       //  0: V6ファイルタイプ : T_XXX
    uint16_t            nlink;      //  2: ハードリンク数
    device_t            rdev;       //  4:デバイス番号
    uint32_t            size;       //  8: Size of file (bytes)
    mode_t              mode;       // 12: file mode
    uid_t               uid;        // 16: owner's user id
    gid_t               gid;        // 20: owner's gropu id
    struct timespec     atime;      // 24: last accessed time
    struct timespec     mtime;      // 40: last modified time
    struct timespec     ctime;      // 56: created time
    uint32_t addrs[NDIRECT+2];      // 72: Data block addresses
    char             _dummy[4];     // 124:
};

/* v6 VFS固有データ */
#if 0
struct v6_vfs {
    struct vnode *  root;       /* ルートvnode */
    struct vnode *  devv;       /* ブロックデバイスのvnode */
    struct v6_superblock sb;    /* スーパーブロックを保持するバッファ */
};
#endif

extern struct v6_superblock v6_sb;

#define ITOV(ip)    ((struct vnode *)ip)
#define VTOI(vp)    ((struct v6_inode *)(vp)->data)
#define FTOI(fp)    ((struct v6_inode *)(fp)->vnode->data)


/* v6mountops.c */
extern struct mount_ops v6_mount_ops;
/* v6vnodeops.c */
extern struct vnode_ops v6_vnode_ops;
/* v6vfileops.c */
extern struct vfile_ops v6_file_ops;

/* v6fs.c */
extern struct _v6_icache v6_icache;
extern struct v6_vfs v6_vfs;

void v6_readsb(device_t dev, struct v6_superblock *sb);
void v6_set_super(void);
void v6_bzero(int dev, int bno);
uint32_t v6_balloc(uint32_t dev);
void v6_bfree(device_t dev, uint32_t b);
void v6_iinit(device_t dev);
struct v6_inode *v6_ialloc(struct mount *mp, uint16_t type);
void v6_iupdate(struct v6_inode *ip);
struct v6_inode *v6_iget(struct mount *mp, uint32_t ino);
struct v6_inode *v6_idup(struct v6_inode *ip);
void v6_ilock(struct v6_inode *ip);
void v6_iunlock(struct v6_inode *ip);
void v6_iput(struct v6_inode *ip);
void v6_iunlockput(struct v6_inode *ip);
uint32_t v6_bmap(struct v6_inode *ip, uint32_t bn);
void v6_itrunc(struct v6_inode *ip);
size_t v6_readi(struct v6_inode *ip, char *dst, off_t offset, size_t n);
size_t v6_writei(struct v6_inode *ip, char *src, off_t offset, size_t n);
struct v6_inode *v6_dirlookup(struct v6_inode *dp, char *name);
int v6_dirlink(struct v6_inode *dp, char *name, uint32_t ino, uint16_t type);
void sync_v6_inodes(void);

int mode2v6type(mode_t mode);
void v6_dump(struct v6_inode *inode, const char *title);

#endif
