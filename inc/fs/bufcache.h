
#ifndef INC_FS_BLOCKCACHE_H
#define INC_FS_BLOCKCACHE_H

#include <types.h>
#include <queue.h>
#include <list.h>
#include <sleeplock.h>

#define BCF_ALLOCATED   0x01
#define BCF_DIRTY       0x02
#define BCF_BUSY        0x04

// バッファキャッシュ構造体 (64バイト)
struct buf {
    struct queue_node node;     // bufcacheにつながる 16
    int refcount;               // 参照カウント         4
    int flags;                  // フラグ               4
    device_t dev;               // デバイス番号         4
    uint32_t blockno;           // 先頭セクタ番号       4
    uint8_t *block;             // データバッファ       8
    boolean  issec;             // blockはセクタか      4
    //struct list_head dlink;     // 未使用
};

void init_bufcache(void);
void sync_bufcache(void);
struct buf *get_block(device_t dev, uint32_t blockno, boolean issec);
void put_block(struct buf *buf);
int release_block(struct buf *buf, int dirty);
void mark_block_dirty(struct buf *buf);

#endif
