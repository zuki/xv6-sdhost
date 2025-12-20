#include <emmc.h>
#include <list.h>
#include <spinlock.h>
#include <fs/bufcache.h>
#include <types.h>
#include <console.h>
#include <irq.h>
#include <string.h>
#include <driver.h>
#include <linux/errno.h>
#include <sd.h>

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

// Driver Definition
int sd_init(void);
int sd_open(minor_t minor, int access);
int sd_close(minor_t minor);
int sd_read(minor_t minor, char *buffer, off_t offset, size_t size);
int sd_write(minor_t minor, const char *buffer, off_t offset, size_t size);
int sd_ioctl(minor_t minor, unsigned int request, void *argp, uid_t uid);
int sd_poll(minor_t minor, int events);
off_t sd_seek(minor_t minor, off_t position, int whence, off_t offset);

struct driver sd_driver = {
    sd_init,
    sd_open,
    sd_close,
    sd_read,
    sd_write,
    sd_ioctl,
    sd_poll,
    sd_seek,
};

/*
 * Initialize SD card and parse MBR.
 * 1. The first partition should be FAT and is used for booting.
 * 2. The second partition is used by our file system.
 *
 * See https://en.wikipedia.org/wiki/Master_boot_record
 */
int sd_init(void)
{
    struct mbr mbr;

    char buf[BSIZE];

    list_init(&sdque);
    initlock(&cardlock, "card");

#if RASPI == 3
    irq_enable(IRQ_SDIO);
    irq_register(IRQ_SDIO, sd_intr, 0);
#elif RASPI == 4
#endif

    register_driver(DEVMAJOR_SD, &sd_driver);

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

    return 0;
}

void sd_intr(void *params)
{
    acquire(&cardlock);
    emmc_intr(&card);
    disb();
    wakeup(&card);
    release(&cardlock);
}

#if 0
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
#endif

int sd_open(minor_t minor, int access)
{
    if (minor > ptnum)
        return -ENXIO;
    return 0;
}

int sd_close(minor_t minor)
{
    if (minor > ptnum)
        return -ENXIO;
    return 0;
}

int sd_read(minor_t minor, char *buffer, off_t offset, size_t size)
{
    if (minor > ptnum)
        return -ENXIO;

    if (offset > (ptinfo[minor].nsecs << 9))
        return -EFAULT;
    if (offset + size > (ptinfo[minor].nsecs << 9))
        size = (ptinfo[minor].nsecs << 9) - offset;

    offset >>= 9;
    emmc_seek(&card, ptinfo[minor].lba + offset);
    for (int count = size >> 9; count > 0; count--, offset++, buffer = &buffer[512]) {
        if (emmc_read(&card, buffer, BSIZE) != BSIZE)
            return -EIO;
    }

    return size;
}

int sd_write(minor_t minor, const char *buffer, off_t offset, size_t size)
{
    if (minor > ptnum)
        return -ENXIO;

    if (offset > (ptinfo[minor].nsecs << 9))
        return -EFAULT;
    if (offset + size > (ptinfo[minor].nsecs << 9))
        size = (ptinfo[minor].nsecs << 9) - offset;

    offset >>= 9;
    emmc_seek(&card, ptinfo[minor].lba + offset);
    for (int count = size >> 9; count > 0; count--, offset++, buffer = &buffer[512]) {
        if (emmc_write(&card, buffer, BSIZE) != BSIZE)
            return -EIO;
    }

    return size;
}

int sd_ioctl(minor_t minor, unsigned int request, void *argp, uid_t uid)
{
    if (minor > ptnum)
        return -ENXIO;
    return 0;
}

int sd_poll(minor_t minor, int events)
{
	return events & (VFS_POLL_READ | VFS_POLL_WRITE);
}

off_t sd_seek(minor_t minor, off_t position, int whence, off_t offset)
{
    if (minor > ptnum)
        return -ENXIO;

    switch(whence) {
        case SEEK_SET:
            break;
        case SEEK_CUR:
            position = offset * position;
            break;
        case SEEK_END:
            position = ptinfo[minor].nsecs + position;
        default:
            return -EINVAL;
    }

    if (position > ptinfo[minor].nsecs)
        position = ptinfo[minor].nsecs;

    return position;
}
