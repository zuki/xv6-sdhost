/*
Copyright 2014 Akira Midorikawa

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
/**
 * @file mm.c
 */
#include <mm.h>
#include <mmu.h>
#include <buddy.h>
#include <string.h>
#include <console.h>

/**
 * @ingroup mm
 * @var pages
 * @brief ページ配列. 全物理ページの配列.
 */
struct page *pages;

/**
 * @ingroup mm
 * @brief ページシステムを初期化する.
 */
void mm_init(void) {
    buddy_init();
    slab_cache_init();
}

/**
 * @ingroup mm
 * @brief 指定されたページの仮想アドレスを返す.
 *
 * @param page ページヘのポインタ
 * @return ページの仮想アドレス
 */
void *page_address(const struct page *page) {
    return PAGE_START + (PGSIZE * page->index);
}

/**
 * @ingroup mm
 * @brief 指定された仮想アドレスのページを返す.
 *
 * @param address ページの仮想アドレス
 * @return ページ構造体へのポインタ
 */
struct page *page_find_by_address(void *address) {
    page_index index = (page_index)(((char*)address - PAGE_START) / PGSIZE);

    if (index < PAGE_NUM) {
        return &pages[index];
    } else {
        return NULL;
    }
}

/**
 * @ingroup mm
 * @brief 指定されたページが含まれるブロックの先頭ページを返す.
 *
 * @param address ページ構造体へのポインタ
 * @return ページ構造体へのポインタ
 */
struct page *page_find_head(const struct page *page) {
    page_index index = page->index;

    while (index < PAGE_NUM) {
        if (pages[index].flags & PF_FIRST_PAGE) {
            return &pages[index];
        }

        index--;
    }

    return NULL;
}

/**
 * @ingroup mm
 * @brief ページブロックを解放する.
 *
 * @param address ページブロックの先頭のページ構造体へのポインタ
 * @return ページ構造体へのポインタ
 */
void page_cleanup(struct page **page) {
    if (*page) {
        buddy_free(*page);
    }
}

/**
 * @ingroup mm
 * @brief ページを解放する.
 *
 * @param pa 解放するページへのポインタ
 */
void kfree(void *pa)
{
    buddy_free(page_find_by_address(pa));
}

/**
 * @ingroup mm
 * @brief ページを割り当てる.
 *
 * @return 割り当てられたページへのポインタ
 */
void *kalloc(void)
{
    struct page *page = buddy_alloc(PGSIZE);
    // return page_address(page);
    void *addr = page_address(page);
    trace("addr: 0x%p", addr);
    return addr;
}

typedef __uint128_t Align;

union header {
    struct {
        union header *ptr;
        uint64_t size;
    } s;
    Align x;
};

typedef union header Header;

static Header base;
static Header *freep = NULL;


/**
 * @ingroup mm
 * @brief カーネルメモリを解放する.
 *
 * @param ap 解放するメールへのポインタ
 */
void kmfree(void *ap)
{
    Header *bp, *p;

    bp = (Header*)ap - 1;
    for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
            break;
    if (bp + bp->s.size == p->s.ptr){
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else
        bp->s.ptr = p->s.ptr;
    if (p + p->s.size == bp){
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else
        p->s.ptr = bp;
    freep = p;
}

//use kalloc instead of growproc to allocate memory
//to kernel data structures.
//kalloc always returns PGSIZE memory on success
//size parameter of the Header takes Header sized chunks
static Header* morecore()
{
    char *p;
    Header *hp;

    p = page_address(buddy_alloc(PGSIZE));
    if (p == NULL)
        return NULL;
    memset(p, 0, PGSIZE);
    hp = (Header*)p;
    hp->s.size = PGSIZE / sizeof(Header);
    kmfree((void*)(hp + 1));
    return freep;
}

/**
 * @ingroup mm
 * @brief カーネルメモリを割り当てる.
 *
 * @param nbytes 割り当てるメモリのバイト数（最大4080）
 * @return 割り当てられたメモリへのポインタ
 */
void *kmalloc(size_t nbytes)
{
    Header *p, *prevp;
    uint64_t nunits;

    // panic if we try to allocate more than 4080
    if (nbytes > (PGSIZE - sizeof(Header)))
        panic("kmalloc: Cannot allocate the requested size of memory %d ( > 4080 )\n", nbytes);
    nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;

    if((prevp = freep) == 0){
        base.s.ptr = freep = prevp = &base;
        base.s.size = 0;
    }

    for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
        if (p->s.size >= nunits){
            if (p->s.size == nunits)
                prevp->s.ptr = p->s.ptr;
            else {
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            return (void*)(p + 1);
        }
        if (p == freep)
            if ((p = morecore()) == 0)
                return NULL;
    }
}

void mm_test(void)
{
#ifdef DEBUG
    static void *p[0x100000000 / PGSIZE];
    int i;
    for (i = 0; (p[i] = kalloc()); i++) {
        memset(p[i], 0xFF, PGSIZE);
        if (i % 10000 == 0)
            debug("0x%p", p[i]);
    }
    while (i--)
        kfree(p[i]);
#endif
}
