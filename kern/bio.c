/* Buffer cache.
 *
 * The buffer cache is a linked list of buf structures holding
 * cached copies of disk block contents.  Caching disk blocks
 * in memory reduces the number of disk reads and also provides
 * a synchronization point for disk blocks used by multiple processes.
 *
 * Interface:
 * * To get a buffer for a particular disk block, call bread.
 * * After changing buffer data, call bwrite to write it to disk.
 * * When done with the buffer, call brelse.
 * * Do not use the buffer after calling brelse.
 * * Only one process at a time can use a buffer,
 *     so do not keep them longer than necessary.
 *
 * The implementation uses two state flags internally:
 * * B_VALID: the buffer data has been read from the disk.
 * * B_DIRTY: the buffer data has been modified
 *     and needs to be written to disk.
 */

#include <spinlock.h>
#include <sleeplock.h>
#include <buf.h>
#include <console.h>
#include <fs.h>
#include <sd.h>
#include <slab.h>

#define DMA_MINALIGN   64

static struct slab_cache *BUFDATA;

struct {
    struct spinlock lock;
    struct buf buf[NBUF];

    // Linked list of all buffers, through prev/next.
    // head.next is most recently used.
    struct list_head head;
} bcache;

void
binit()
{
    struct buf *b;

    BUFDATA = slab_cache_create("buf.data", BSIZE, DMA_MINALIGN);
    // initlock(&bcache.lock, "bcache");

    // Create linked list of buffers
    list_init(&bcache.head);
    for (b = bcache.buf; b < bcache.buf + NBUF; b++) {
        list_push_back(&bcache.head, &b->clink);
    }
}

/*
 * Look through buffer cache for block on device dev.
 * If not found, allocate a buffer.
 * In either case, return locked buffer.
 */
static struct buf *
bget(uint32_t dev, uint32_t bno)
{
    struct buf *b;

    uint32_t blockno = fs_lba(dev) + bno * BLKSECT;

    acquire(&bcache.lock);

    // Is the block already cached?
    LIST_FOREACH_ENTRY(b, &bcache.head, clink) {
        if (b->dev == dev && b->blockno == blockno) {
            b->refcnt++;
            trace("buf cached: dev: 0x%x, blockno: 0x%x, data: %p", dev, blockno, b->data);
            release(&bcache.lock);
            acquiresleep(&b->lock);
            return b;
        }
    }

    trace("not cached: bno %d", blockno);

    // Not cached; recycle an unused buffer.
    // Even if refcnt==0, B_DIRTY indicates a buffer is in use
    // because log.c has modified it but not yet committed it.
    LIST_FOREACH_ENTRY_REVERSE(b, &bcache.head, clink) {
        if (b->refcnt == 0 && (b->flags & B_DIRTY) == 0) {
            b->dev = dev;
            b->blockno = blockno;
            b->flags = 0;
            b->refcnt = 1;
            if (b->data == NULL) {
                b->data = (uint8_t *)slab_cache_alloc(BUFDATA);
                trace("b->data: 0x%p", b->data);
            }
            release(&bcache.lock);
            acquiresleep(&b->lock);
            return b;
        }
    }
    panic("bget: no buffers");
    return 0;
}

/* Return a locked buf with the contents of the indicated block. */
struct buf *
bread(uint32_t dev, uint32_t blockno)
{
    struct buf *b = bget(dev, blockno);
    if ((b->flags & B_VALID) == 0) {
        sd_rw(b);
    }
    return b;
}

/* Write b's contents to disk. Must be locked. */
void
bwrite(struct buf *b)
{
    if (!holdingsleep(&b->lock))
        panic("bwrite");
    b->flags |= B_DIRTY;
    sd_rw(b);
}

/*
 * Release a locked buffer.
 * Move to the head of the MRU list.
 */
void
brelse(struct buf *b)
{
    if (!holdingsleep(&b->lock))
        panic("brelse");

    releasesleep(&b->lock);

    acquire(&bcache.lock);
    b->refcnt--;
    if (b->refcnt == 0) {
        // no one is waiting for it.
        list_drop(&b->clink);
        list_push_back(&bcache.head, &b->clink);
    }
    release(&bcache.lock);
}
