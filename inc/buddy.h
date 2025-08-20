#ifndef INC_BUDDY_H
#define INC_BUDDY_H

#include <mm.h>
#include <types.h>

void            buddy_init(void);
struct page *   buddy_alloc(size_t size);
void            buddy_free(struct page *page);

#endif