
#ifndef INC_FS_BLOCKCACHE_H
#define INC_FS_BLOCKCACHE_H

#include <types.h>
#include <queue.h>
#include <list.h>
#include <sleeplock.h>

#define BCF_ALLOCATED   0x01
#define BCF_DIRTY       0x02
#define BCF_BUSY        0x04

struct buf {
    struct queue_node node;
    int refcount;
    short flags;
    device_t dev;
    uint32_t blockno;
    uint8_t *block;
    struct list_head dlink;
};

void init_bufcache(void);
void sync_bufcache(void);
struct buf *get_block(device_t dev, uint32_t blockno);
void put_block(struct buf *buf);
int release_block(struct buf *buf, int dirty);
void mark_block_dirty(struct buf *buf);

#endif
