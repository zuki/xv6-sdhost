#ifndef INC_FS_V6_PARAM_H
#define INC_FS_V6_PARAM_H

#include <param.h>
#include <linux/stat.h>

struct v6_superblock;

#define BLKSIZE         4096                /* ブロックサイズ */
#define SECTOR_SIZE     512                 /* セクタサイズ */
#define BLKSECT         (BLKSIZE / SECTOR_SIZE)   /* ブロックあたりのセクタ数 */

#define FSSIZE          1000                /* ファイルシステムのサイズ */

#define LOGSIZE         (MAXOPBLOCKS*3)     /* ログサイズ（未使用） */
#define V6_ROOTINO      1                   /* v6ファイルシステムのルートのinode番号 */

#define NDIRECT     11                              /* 直接指定のブロック数 */
#define NINDIRECT   (BLKSIZE / sizeof(uint32_t))    /* 第一間接指定のブロック数 */
#define NINDIRECT2  (NINDIRECT * NINDIRECT)         /* 第二間接指定のブロック数 */
#define MAXFILE     (NDIRECT + NINDIRECT + NINDIRECT2)   /* 1ファイルの最大ブロック数 */
                                                    /* 11 + 1024 + 1,048,576 = 1,049,611 : * 4096 = 4100 MB */

/* ブロックあたりのinode数 : Inodes per block */
#define IPB         (BLKSIZE / sizeof(struct v6_dinode))

#define INOSTART    (2 + LOGSIZE)           /* Boot + Super + Log */
#define INOBLKS     (NINODE / IPB + 1)
#define BITSTART    (INOSTART + INOBLKS)

/* inode iが含まれるブロック */
#define IBLOCK(i)   ((i) / IPB + INOSTART)

/* ブロックあたりのBitmapビット数 */
#define BPB         (BLKSIZE*8)

/* ブロック b を記録する未使用マップのあるブロック */
#define BBLOCK(b) (b/BPB + BITSTART)

/* ディレクトリエントリ名の最大サイズ */
#define DIRSIZ  58
/* ブロックあたりのディレクトリエントリ数 */
#define V6_DIRENTS_PER_BLOCK    (BLKSIZE / DESIZE)
/* 最大リンク数 */
#define MAXLINK 100



#endif
