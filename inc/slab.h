#ifndef INC_SLAB_H
#define INC_SLAB_H

#include <types.h>

struct slab_cache;

void            slab_cache_init(void);
struct slab_cache *slab_cache_create(const char *name, size_t size);
void            slab_cache_destroy(struct slab_cache *cache);
void *          slab_cache_alloc(struct slab_cache *cache);
void            slab_cache_free(struct slab_cache *cache, void *obj);

#endif