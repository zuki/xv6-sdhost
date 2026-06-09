/*
 * On-disk file system format.
 * Both the kernel and user programs use this header file.
 */
#ifndef USR_INC_FS_H
#define USR_INC_FS_H

#include <stdint.h>
#include <time.h>
#include "types.h"

#define MAXOPBLOCKS     10  // Max # of blocks any FS op writes
#define LOGSIZE         (MAXOPBLOCKS*3)     // Max data blocks in on-disk log
#define V6_ROOTINO      1                   // Root i-number

#define FSSIZE          1000    // ファイルシステムのブロック単位のサイズ
#define BLKSIZE         4096    // ブロックサイズ

#define NDIRECT     11
#define NINDIRECT   (BLKSIZE / sizeof(uint32_t))    // 第一間接指定のブロック数
#define NINDIRECT2  (NINDIRECT * NINDIRECT)       // 第二間接指定のブロック数
#define MAXFILE     (NDIRECT + NINDIRECT + NINDIRECT2)   // 1ファイルの最大ブロック数 - 11 + 1024 + 1,048,576 = 1,049,611 : * 4096 = 4100 MB

/* Disk layout:
 * [ boot block | super block | log | inode blocks | free bit map | data blocks ]
 *
 * mkfs computes the super block and builds an initial file system. The
 * super block describes the disk layout:
 */
struct superblock {
    uint32_t size;         // Size of file system image (blocks)
    uint32_t nblocks;      // Number of data blocks
    uint32_t ninodes;      // Number of inodes.
    uint32_t nlog;         // Number of log blocks
    uint32_t logstart;     // Block number of first log block
    uint32_t inodestart;   // Block number of first inode block
    uint32_t bmapstart;    // Block number of first free map block
};

/* On-disk inode構造体 : 128バイト */
struct dinode {
    uint16_t            type;       //  0: V6ファイルタイプ : T_XXX
    uint16_t            nlink;      //  2: ハードリンク数
    device_t            dev;        //  4:デバイス番号
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

/* Inodes per block: 4 */
#define IPB           (BLKSIZE / sizeof(struct dinode))

/* Block containing inode i. */
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)

/* Bitmap bits per block;  4096 = 0x1000 */
#define BPB           (BLKSIZE*8)

/* Block of free map containing bit for block b. */
#define BBLOCK(b, sb) (b/BPB + sb.bmapstart)

/* Directory is a file containing a sequence of dirent structures. */
#define DIRSIZ 58

struct dirent {
  uint32_t  inum;
  uint16_t  type;
  char name[DIRSIZ];
};

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

#define makedev(x,y) ((((x) & 0xff) << 8) | \
        ((y) & 0xff))
#define major(x) (((x)>>8) & 0xff)
#define minor(x) ((x) & 0xff)

#define DEVFAT          makedev(1, 0)
#define DEVFAT2         makedev(1, 1)
#define DEVV6           makedev(1, 2)
#define DEVEXT2         makedev(1, 3)
#define DEVPROCFS       makedev(1, 4)
#define DEVCONS         makedev(2, 1)
#define DEVTTY1         makedev(3, 1)
#define DEVTTY2         makedev(3, 2)

#define ROOTDEV         (minor(DEVV6))      // Device number of file system root disk

#define SDMAJOR         (major(DEVV6))
#define TTYMAJOR        (major(DEVTTY1))

#endif
