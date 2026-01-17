#ifndef INC_CACHEPAGE_H
#define INC_CACHEPAGE_H

#include <types.h>
#include <list.h>
#include <vfs.h>
#include <sleeplock.h>

#define CPSIZE 0x100
#define CPHASH(dev, ino) ((uint32_t)(((uint64_t)(dev + ino)) % CPSIZE))

struct cachepage {
    char *      page;           // 0
    ino_t       ino;            // 8
    device_t    dev;            // 16
    int         ref_count;      // 20
    off_t       offset;         // 24
    struct sleeplock lock;      // 32
    struct list_head link;
};

void cachepage_init(void);
struct cachepage *get_cachepage(struct vfile *file, off_t offset);
void update_cachepage(device_t dev, ino_t ino, off_t offset, char *addr, size_t size);
long copy_cachepage(struct vfile *file, off_t offset, char *dest, size_t size, off_t dest_offset);
long copy_cachepages(struct vfile *file, char *dest, size_t size, off_t offset);
#endif
