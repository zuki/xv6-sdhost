
#include <types.h>
#include <sd.h>
#include <string.h>
#include <console.h>
#include <driver.h>
#include <fs/bufcache.h>
#include <queue.h>
#include <mm.h>
#include <spinlock.h>
#include <arm.h>

#define BC_SECTOR_SIZE      512
#define BC_BLOCK_SIZE       4096
#define BLOCKCACHE_MAX      30

static struct queue bufcache;
static struct buf blocks[BLOCKCACHE_MAX];

static inline struct buf *_find_free_entry();
static struct buf *_load_block(device_t dev, uint32_t blockno, boolean issec);
static inline int _read_entry(struct buf *entry);
static inline int _write_entry(struct buf *entry);

/* bufcacheを初期化する : binit を置き換え */
void init_bufcache(void)
{
    _queue_init(&bufcache);

    for (int i = 0; i < BLOCKCACHE_MAX; i++) {
        _queue_node_init(&blocks[i].node);
        blocks[i].refcount = 0;
        blocks[i].flags = 0;
        blocks[i].issec = false;         // デフォルトはblock (4096バイト)
        blocks[i].dev = 0;
        blocks[i].block = NULL;
        _queue_insert(&bufcache, &blocks[i].node);
    }
}

void sync_bufcache()
{
    for (int i = 0; i < BLOCKCACHE_MAX; i++) {
        blocks[i].flags |= BCF_BUSY;
        if (blocks[i].flags & BCF_ALLOCATED)
            _write_entry(&blocks[i]);
        blocks[i].flags &= ~BCF_BUSY;
    }
}

/* デバイスdevのブロック番号blocknoのブロックを読み込む : bread を置き換え
 * BCF_BUSYフラグを立てたstruct bufを返す。読み込みが失敗したらpanic */
struct buf *get_block(device_t dev, uint32_t blockno, boolean issec)
{
    struct buf *cur;
    trace("dev: 0x%x, bno: 0x%x, issec: %d", dev, blockno, issec);
    /* 1. bufcacheにあればrefcountを増分して返す */
    acquire(&bufcache.lock);
    for (cur = (struct buf *) bufcache.head; cur; cur = (struct buf *) cur->node.next) {
        if (/*cur->flags & BCF_ALLOCATED && */ cur->dev == dev && cur->blockno == blockno && cur->issec == issec) {
            cur->refcount++;
            trace("hit: dev: 0x%x, bno: 0x%x", cur->dev, cur->blockno);
            /* curをキャッシュリストの先頭に移動させる */
            _queue_remove(&bufcache, &cur->node);
            _queue_insert(&bufcache, &cur->node);
            cur->flags |= BCF_BUSY;
            release(&bufcache.lock);
            return cur;
        }
    }
    /* 2. bufcacheになければ読み込む */
    trace("no hit: dev: 0x%x, blockno: 0x%x, issec: %s", dev, blockno, issec ? "true" : "false");
    return _load_block(dev, blockno, issec);
}

/* bwrite を置き換え */
void put_block(struct buf *buf)
{
    _write_entry(buf);
}

/* brelseを置き換える */
int release_block(struct buf *buf, int dirty)
{
    if (dirty)
        mark_block_dirty(buf);

    if (--buf->refcount == 0) {
        _write_entry(buf);
        buf->flags &= ~(BCF_BUSY | BCF_ALLOCATED);
    } else if (buf->refcount < 0) {
        buf->refcount = 0;
        error("possible double free for block %d:%d", buf->dev, buf->blockno);
    }
    disb();
    return 0;
}

void mark_block_dirty(struct buf *buf)
{
    buf->flags |= BCF_DIRTY;
}

/* デバイスdevのブロック番号blocknoのデータを返す.
 * 呼び出し元は bufcache.lockを保持していなければならない
 */
static struct buf *_load_block(device_t dev, uint32_t blockno, boolean issec)
{
    disb();
    struct buf *entry;

    /* bufcacheからリサイクルする */
    entry = _find_free_entry();
    trace("recycle entry: %p", entry);
    entry->refcount = 1;
    entry->flags |= BCF_ALLOCATED;  // すでにBCF_BUSYがセットされている
    entry->dev = dev;
    entry->issec = issec;
    entry->blockno = blockno;
    if (issec) {
        entry->block = (uint8_t *)kmalloc(BC_SECTOR_SIZE);
        memset(entry->block, 0, BC_SECTOR_SIZE);
    } else {
        entry->block = (uint8_t *)kalloc(1);
        memset(entry->block, 0, BC_BLOCK_SIZE);
    }
    trace("entry->block: %p", entry->block);
    disb();
    _read_entry(entry);

    return entry;
}

/* bufcacheからrefcount=0の最後に使用したエントリを返す
 * 呼び出し元は bufcache.lockを保持していなければならない
 */
static inline struct buf *_find_free_entry(void)
{
    struct buf *last;
    int i = 0;

    /* 最後に使用したエントリをリサイクル */
    for (last = (struct buf *) bufcache.tail; last && last->refcount > 0; last = (struct buf *) last->node.prev)
        {
            if (++i > BLOCKCACHE_MAX)
                break;
        }

    trace("i: %d, last: %p, ref: %d", i, last, last ? last->refcount : -1);

    if (!last) {
        release(&bufcache.lock);
        panic("Error: ran out of bufcache entries\n");
        return NULL;
    }
    last->flags |= BCF_BUSY;
    /* リサイクルするエントリを先頭に移動する */
    _queue_remove(&bufcache, &last->node);
    _queue_insert(&bufcache, &last->node);
    //release(&bufcache.lock);
    if (last->block) {
        // ここでlastがdirtyだったら書き戻す
        _write_entry(last);
        if (last->issec) {
            kmfree(last->block);
        } else {
            kfree(last->block);
        }
        last->block = NULL;
        disb();
    }
    release(&bufcache.lock);
    return last;
}

static inline int _read_entry(struct buf *entry)
{
    assert(entry->flags & BCF_BUSY);
    int size = entry->issec ? BC_SECTOR_SIZE : BC_BLOCK_SIZE;
    int bytes;

    trace("read: dev: 0x%x, buffer: 0x%x, bno: 0x%x, size: 0x%x", entry->dev, entry->block, entry->blockno, size);

    bytes = dev_read(entry->dev, (char *)entry->block, entry->blockno, size);
    if (bytes != size) {
        error("panic: bytes: %d != size: %d", bytes, size);
        panic("read_entry\n");
        return -1;
    }
    disb();
    return 0;
}

static inline int _write_entry(struct buf *entry)
{
    assert(entry->flags & BCF_BUSY);
    int size = entry->issec ? BC_SECTOR_SIZE : BC_BLOCK_SIZE;
    int bytes;

    /* 変更されていなければ何もしない */
    if (!(entry->flags & BCF_DIRTY))
        return 0;

    trace("WRITING 0x%x: 0x%x <- 0x%x + 0x%x", entry->dev, entry->blockno, entry->block, size);
    // デバイスに書き込む
    bytes = dev_write(entry->dev, (const char *)entry->block, entry->blockno, size);
    if (bytes != size) {
        panic("write_entry");
        return -1;
    }
    entry->flags &= ~BCF_DIRTY;
    return 0;
}
