#include <types.h>
#include <cachepage.h>
#include <console.h>
#include <linux/errno.h>
#include <mmu.h>
#include <mm.h>
#include <list.h>
#include <sleeplock.h>
#include <spinlock.h>
#include <string.h>
#include <vfs.h>
#include <proc.h>

struct slab_cache *CPAGE;

struct {
    struct list_head cpque[CPSIZE];
    int count;
    struct spinlock lock;
} cachepages;

/* cachepage機能を初期化する */
void cachepage_init(void)
{
    CPAGE = slab_cache_create("cachepage", sizeof(struct cachepage), 0);

    initlock(&cachepages.lock, "cachepage");

    acquire(&cachepages.lock);
    for (int i = 0; i < CPSIZE; i++) {
        list_init(&cachepages.cpque[i]);
    }
    cachepages.count = 0;
    release(&cachepages.lock);
    info("cachepage_init ok");
}

/* dev/ino/offsetを含むcachepageを探す.
 * cachepage.lockを保持していなければならない
 */
static struct cachepage *find_cachepage(device_t dev, ino_t ino, off_t offset)
{
    trace("dev: %d, ino: %lld, offset: %lld", dev, ino, offset);
    if (!cachepages.lock.locked)
        panic("not locked");

    struct list_head *q = &cachepages.cpque[CPHASH(dev, ino)];
    struct cachepage *page, *next;
    LIST_FOREACH_ENTRY_SAFE(page, next, q, link) {
        if (page->dev == dev && page->ino == ino
         && page->offset == offset) {
            return page;
        }
    }
    return 0;
}

struct cachepage *get_cachepage(struct vfile *file, off_t offset)
{
    int err;
    device_t rdev = file->vnode->rdev;
    ino_t ino = file->vnode->ino;

    offset -= offset % PGSIZE;  // ページアライン
    acquire(&cachepages.lock);
    // 該当のページがキャッシュに存在するか?
    struct cachepage *res = find_cachepage(rdev, ino, offset);
    // あれば返す
    if (res) {
        release(&cachepages.lock);
        res->ref_count++;
        acquiresleep(&res->lock);
        return res;
    }
    // なければcachepageを作成してファイルから読み込んでページを返す
    struct cachepage *cpage = slab_cache_alloc(CPAGE);
    initsleeplock(&cpage->lock, "cachepage");
    release(&cachepages.lock);
    acquiresleep(&cpage->lock);
    cpage->page = kalloc();
    if (cpage->page == NULL) {
        error("no memory: rdev: 0x%x, ino: %lld, offset: 0x%x", rdev, ino, offset);
        goto err2;
    }
    memset(cpage->page, 0, PGSIZE);
    if ((err = vfs_seek(file, offset, SEEK_SET)) < 0) {
        error("seek error");
        goto err1;
    }
    int n = vfs_read(file, cpage->page, PGSIZE);
    if (n < 0) {
        error("get_cachepage readi failed: n=%d, offset=%ld, size=%d",
            n, offset, PGSIZE);
        goto err1;
    }
    cpage->dev = file->vnode->rdev;
    cpage->ino = file->vnode->ino;
    cpage->offset = offset;
    cpage->ref_count = 1;
    list_push_back(&cachepages.cpque[CPHASH(rdev, ino)], &cpage->link);
    cachepages.count++;
    debug("alloc new cachepage[%d]: dev: 0x%x, ino=%d, offset=0x%llx", cachepages.count - 1, cpage->ino, cpage->offset);

    return cpage;

err1:
    kalloc(cpage->page);
err2:
    releasesleep(&cpage->lock);
    slab_cache_free(CPAGE, cpage);

    return NULL;
}

/* fileオフセットoffsetを含むcachepageのオフセットdest_offsetから
 * sizeバイトのデータをdestにコピー
 */
long copy_cachepage(struct vfile *file, off_t offset, char *dest, size_t size, off_t dest_offset)
{
    trace("dev: 0x%x, ino=%d, offset=0x%llx, dest=0x%p, size=0x%x, dest_offset=0x%llx",
            file->vnode->rdev, file->vnode->ino, offset, dest, size, dest_offset);
    struct cachepage *cpage = get_cachepage(file, offset);
    if (cpage == NULL) {
        warn("get_cachepage failed");
        return -ENOMEM;
    }

    if (!holdingsleep(&cpage->lock))
        panic("not holding sleeqlock");

    trace("memmove from %p to %p with 0x%x bytes",
        cpage->page + dest_offset, dest, size);
    memmove(dest, cpage->page + dest_offset, size);
    releasesleep(&cpage->lock);
    return 0;
}

/* fileのオフセットoffsetからsizeバイトのデータをdestにコピー */
long copy_cachepages(struct vfile *file, char *dest, size_t size, off_t offset)
{
    char *addr = dest;
    off_t ioff = offset < PGSIZE ? 0 : offset & ~(PGSIZE - 1);
    off_t doff = offset & (uint64_t)(PGSIZE - 1);
    uint64_t sz = (doff + size) > PGSIZE ? PGSIZE - doff : size;
    int npages = (size + doff + PGSIZE - 1) / PGSIZE;
    debug("copy %d pages", npages);

    long error;
    for (int i = 0; i < npages; i++) {
        if ((error = copy_cachepage(file, ioff, addr, sz, doff)) < 0)
            return error;
        ioff += PGSIZE;
        addr += sz;
        size -= sz;
        sz = size > PGSIZE ? PGSIZE : size;
        doff = 0;
    }
    return 0;
}

/* dev/ino/offsetに対応するcachepageをaddrのデータでsizeバイト更新する */
void update_cachepage(device_t dev, ino_t ino, off_t offset, char *addr, size_t size)
{
    trace("dev: 0x%x, ino: %lld, offset=0x%x, addr=0x%p, size=0x%x", dev, ino, offset, addr, size);

    off_t alligned_offset = offset - (offset % PGSIZE);
    off_t start_addr = offset % PGSIZE;
    trace("  - aligned_offset=0x%d", alligned_offset);

    acquire(&cachepages.lock);
    struct cachepage *res = find_cachepage(dev, ino, alligned_offset);
    release(&cachepages.lock);

    if (!res) return;   // 該当ページなし

    acquiresleep(&res->lock);
    char *page = res->page;
    trace("    - addr=0x%p, page_offset=0x%x, size=0x%x", addr, start_addr, size);
    trace("update_page: memmove from %p to %p with 0x%x bytes", addr, page + start_addr, size);
    memmove(page + start_addr, addr, size);
    releasesleep(&res->lock);
}
