#include <fs/fatfs/ffconf.h>
#include <fs/fatfs/ff.h>
#include <fs/fatfs/diskio-ff.h>
#include <fs/bufcache.h>
#include <console.h>
#include <sd.h>
#include <string.h>
#include <param.h>

/* Initialize a Drive */
// extern int sd_init();
DSTATUS disk_initialize(BYTE drv)
{
    warn("disk_initialize, drv %u", drv);
    return 0;
}

/* Return Disk Status */
DSTATUS disk_status(BYTE drv)
{
    return 0;
}

/* Read Sector(s) */
DRESULT disk_read(BYTE drv, BYTE *buff, DWORD sector, UINT count)
{
    trace("drv: %u, buff: %p, sector: %d, count: %d", drv, buff, sector, count);
    struct buf *bp;
    device_t dev = GET_DEV(drv);
    for (unsigned int i = 0; i < count; i++) {
      if (!(bp = get_block(dev, sector + i, true))) {
          return RES_ERROR;
      }
      memmove(buff + i * SECTOR_SIZE, bp->block, SECTOR_SIZE);
      release_block(bp, 0);
    }
    return RES_OK;
}

/* Write Sector(s) */
DRESULT disk_write(BYTE drv, const BYTE *buff, DWORD sector, UINT count)
{
    struct buf *bp;
    device_t dev = GET_DEV(drv);
    unsigned int i;
    for (i = 0; i < count; i++) {
      if ((bp = get_block(dev, sector + i, true))) {
        memmove(bp->block, buff + i * SECTOR_SIZE, SECTOR_SIZE);
        put_block(bp);
        release_block(bp, 0);
      } else
        break;
    }
    if (i == count) return RES_OK;
    return RES_ERROR;
}

/* Miscellaneous Functions */

DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void *buff)
{
    if (ctrl == GET_SECTOR_COUNT)
    {
        *(DWORD *)buff = sd_get_nsecs((device_t)drv);
    }
    else if (ctrl == GET_SECTOR_SIZE)
    {
        *(DWORD *)buff = 512;
    }
    else if (ctrl == GET_BLOCK_SIZE) /* Get erase block size in unit of sectors (DWORD) */
    {
        *(DWORD *)buff = 512;
    }
    else if (ctrl == CTRL_SYNC)
    {
    }
    else if (ctrl == CTRL_TRIM)
    {
    }
    return RES_OK;
}

