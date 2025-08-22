#include <sd.h>
#include <emmc.h>
#include <list.h>
#include <spinlock.h>
#include <buf.h>
#include <types.h>
#include <console.h>
#include <irq.h>
#include <fs.h>
#include <string.h>

static struct emmc card;
static struct list_head sdque;
static struct spinlock cardlock;
struct partition_info ptinfo[PARTITIONS];

// 使用中のパーティション数
static int ptnum = 0;

static void sd_sleep(void *chan)
{
    sleep(chan, &cardlock);
}

/*
 * Initialize SD card and parse MBR.
 * 1. The first partition should be FAT and is used for booting.
 * 2. The second partition is used by our file system.
 *
 * See https://en.wikipedia.org/wiki/Master_boot_record
 */
void
sd_init(void)
{
    struct mbr mbr;

    char buf[BSIZE];

    list_init(&sdque);
    initlock(&cardlock);

#if RASPI == 3
    irq_enable(IRQ_SDIO);
    irq_register(IRQ_SDIO, sd_intr);
#elif RASPI == 4
#endif

    acquire(&cardlock);
    int ret = emmc_init(&card, sd_sleep, (void *)&card);
    //assert(ret == 0);
    if (ret)
      panic("failed emmc_init\n");

    emmc_seek(&card, 0);
    assert(emmc_read(&card, buf, BSIZE) == BSIZE);

    release(&cardlock);

    //assert(mbr.signature == 0xAA55);

    memmove(&mbr, buf, 512);
    for (int i = 0; i < PARTITIONS; i++) {
        if (mbr.ptables[i].lba == 0) break;

        ptinfo[i].type = mbr.ptables[i].type;
        ptinfo[i].lba = mbr.ptables[i].lba;
        ptinfo[i].nsecs = mbr.ptables[i].nsecs;
        info("partition[%d]: TYPE: %d, LBA = 0x%x, #SECS = 0x%x",
            i, ptinfo[i].type, ptinfo[i].lba, ptinfo[i].nsecs);
        ptnum++;
    }

    info("sd_init ok\n");
}

void
sd_intr(void)
{
    acquire(&cardlock);
    emmc_intr(&card);
    disb();
    wakeup(&card);
    release(&cardlock);
}

/*
 * SDカードのリクエスト処理を開始する.
 * Callerはcardlockを保持していなければならない.
 */
static void sd_start(void)
{
    //uint32_t bno;
    while (!list_empty(&sdque)) {
        struct buf *b =
            container_of(list_front(&sdque), struct buf, dlink);      

        //bno = b->blockno + ptinfo[b->dev].lba;
        emmc_seek(&card, b->blockno * SECTOR_SIZE);
        trace("dev: %d, blockno: 0x%x, seek: 0x%x", b->dev, b->blockno, b->blockno * SECTOR_SIZE);
        if (b->flags & B_DIRTY) {
            assert(emmc_write(&card, b->data, BSIZE) == BSIZE);
        } else {
#if 0
            size_t bytes = emmc_read(&card, b->data, BSIZE);
            debug("bytes: %d, b->data: 0x%p", bytes, b->data);
            assert(bytes == BSIZE);
#endif
            assert(emmc_read(&card, b->data, BSIZE) == BSIZE);
        }

        b->flags |= B_VALID;
        b->flags &= ~B_DIRTY;

        list_pop_front(&sdque);
        disb();
        wakeup(b);
    }
}

void sd_rw(struct buf *b)
{
    acquire(&cardlock);
    trace("bno: 0x%x, flags: %d", b->blockno, b->flags);

    // Append to request queue.
    list_push_back(&sdque, &b->dlink);

    // Start disk if necessary.
    if (list_front(&sdque) == &b->dlink)
        sd_start();

    // Wait for request to finish.
    while ((b->flags & (B_VALID | B_DIRTY)) != B_VALID)
        sd_sleep(b);

    release(&cardlock);
}

void sd_flush(void)
{
    acquire(&cardlock);
    sd_start();
    release(&cardlock);
}
