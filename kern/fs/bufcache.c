
#include <types.h>
#include <string.h>
#include <console.h>
#include <driver.h>
#include <fs/bufcache.h>
#include <queue.h>
#include <slab.h>
#include <spinlock.h>

#define BC_BLOCK_SIZE       4096
#define BC_ALLOC_BLOCK()    (slab_cache_alloc(BUFDATA))
#define BC_FREE_BLOCK(ptr)  (slab_cache_free(BUFDATA, ptr))

#define BLOCKCACHE_MAX      30

#define DMA_MINALIGN        64
static struct slab_cache    *BUFDATA;

static struct queue bufcache;
static struct buf blocks[BLOCKCACHE_MAX];

static inline struct buf *_find_free_entry();
static struct buf *_load_block(device_t dev, uint32_t blockno);
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
        blocks[i].block = NULL;
        //initsleeplock(&blocks[i].lock, "buf");
        _queue_insert(&bufcache, &blocks[i].node);
    }

    BUFDATA = slab_cache_create("buf.data", 4096, DMA_MINALIGN);
}

void sync_bufcache()
{
    for (int i = 0; i < BLOCKCACHE_MAX; i++) {
        blocks[i].flags |= BCF_BUSY;
        //acquiresleep(&blocks[i].lock);
        if (blocks[i].flags & BCF_ALLOCATED)
            _write_entry(&blocks[i]);
        //releasesleep(&blocks[i].lock);
        blocks[i].flags &= ~BCF_BUSY;
    }
}

/* デバイスdevのブロック番号blocknoのブロックを読み込む : bread を置き換え
 * sleeplockを持ったstruct bufを返す。読み込みが失敗したらpanic */
struct buf *get_block(device_t dev, uint32_t blockno)
{
    struct buf *cur;

    /* 1. bufcacheにあればrefcountを増分して返す */
    acquire(&bufcache.lock);
    for (cur = (struct buf *) bufcache.head; cur; cur = (struct buf *) cur->node.next) {
        if (cur->flags & BCF_ALLOCATED && cur->dev == dev && cur->blockno == blockno) {
            cur->refcount++;

            /* curをキャッスリストの先頭に移動させる */
            _queue_remove(&bufcache, &cur->node);
            _queue_insert(&bufcache, &cur->node);
            release(&bufcache.lock);
            cur->flags |= BCF_BUSY;
            //acquiresleep(&cur->lock);
            return cur;
        }
    }
    /* 2. bufcacheになければ読み込む */
    return _load_block(dev, blockno);
}

/* bwrite を置き換え */
void put_block(struct buf *buf)
{
    //acquiresleep(&buf->lock);
    _write_entry(buf);
    //releasesleep(&buf->lock);
}

/* brelseを置き換える */
int release_block(struct buf *buf, int dirty)
{
    if (dirty)
        mark_block_dirty(buf);

    if (--buf->refcount == 0) {
        /* TODO: このエントリがリサイクルされるまでは書き込みを
         * 控えたほうが良いだろう。でなければ、ビット単位の
         * 変更が発生するたびに即座に書き込みが必要になるから */
        //_write_entry(buf);
    } else if (buf->refcount < 0) {
        buf->refcount = 0;
        error("possible double free for block %d:%d", buf->dev, buf->blockno);
    }
    //releasesleep(&buf->lock);
    buf->flags &= ~BCF_BUSY;
    return 0;
}

void mark_block_dirty(struct buf *buf)
{
    buf->flags |= BCF_DIRTY;
}

/* デバイスdevのブロック番号blocknoのデータを返す.
 * 呼び出し元は bufcache.lockを保持していなければならない
 */
static struct buf *_load_block(device_t dev, uint32_t blockno)
{
    struct buf *entry;

    /* bufcacheからリサイクルする */
    entry = _find_free_entry();

    entry->refcount = 1;
    entry->flags |= BCF_ALLOCATED;  // すでにBCF_BUSYがセットされている
    entry->dev = dev;
    entry->blockno = blockno;
    entry->block = BC_ALLOC_BLOCK();

    _read_entry(entry);

    return entry;
}

/* bufcacheからrefcount=0の最後に使用したエントリを返す
 * 呼び出し元は bufcache.lockを保持していなければならない
 */
static inline struct buf *_find_free_entry()
{
    struct buf *last;

    /* 最後に使用したエントリをリサイクル */
    for (last = (struct buf *) bufcache.tail; last && last->refcount > 0; last = (struct buf *) last->node.prev)
        { /* 何もしない */}

    if (!last) {
        panic("Error: ran out of bufcache entries\n");
        return NULL;
    }

    release(&bufcache.lock);
    //acquiresleep(&last->lock);
    last->flags |= BCF_BUSY;
    /* リサイクルするエントリを先頭に移動する */
    _queue_remove(&bufcache, &last->node);
    _queue_insert(&bufcache, &last->node);
    if (last->block) {
        // ここでlastがdirtyだったら書き戻す
        _write_entry(last);
        BC_FREE_BLOCK(last->block);
        last->block = NULL;
    }

    return last;
}

static inline int _read_entry(struct buf *entry)
{
    //assert(holdingsleep(&entry->lock));
    assert(entry->flags & BCF_BUSY);

    debug("read: dev: 0x%x, buffer: 0x%x, bno: 0x%x, size: 0x%x", entry->dev, entry->block, entry->blockno, BC_BLOCK_SIZE);
    int size = dev_read(entry->dev, entry->block, entry->blockno, BC_BLOCK_SIZE);
    if (size != BC_BLOCK_SIZE) {
        info("size: %d", size);
        panic("read_entry\n");
        return -1;
    }
    return 0;
}

static inline int _write_entry(struct buf *entry)
{
    //assert(holdingsleep(&entry->lock));
    assert(entry->flags & BCF_BUSY);

    /* 変更されていなければ何もしない */
    if (!(entry->flags & BCF_DIRTY))
        return 0;

    trace("WRITING 0x%x: 0x%x <- 0x%x + 0x%x", entry->dev, entry->blockno, entry->block, BC_BLOCK_SIZE);
    // デバイスに書き込む
    int size = dev_write(entry->dev, entry->block, entry->blockno, BC_BLOCK_SIZE);
    if (size != BC_BLOCK_SIZE) {
        panic("write_entry");
        return -1;
    }
    entry->flags &= ~BCF_DIRTY;
    return 0;
}
