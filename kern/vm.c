#include <types.h>
#include <vm.h>
#include <string.h>
#include <arm.h>
#include <mmu.h>
#include <memlayout.h>
#include <linux/mman.h>
#include <proc.h>
#include <mmap.h>
#include <console.h>
#include <mm.h>

/* For simplicity, we only support 4k pages in user pgdir. */

extern uint64_t kpgdir[512];

uint64_t *
vm_init()
{
    uint64_t *pgdir = kalloc();
    if (pgdir)
        memset(pgdir, 0, PGSIZE);
    else
        warn("failed");
    return pgdir;
}

/*
 * return the address of the pte in user page table
 * pgdir that corresponds to virtual address va.
 * if alloc != 0, create any required page table pages.
 */
uint64_t *
pgdir_walk(uint64_t * pgdir, void *vap, int alloc)
{
    uint64_t *pgt = pgdir, va = (uint64_t) vap;
    for (int i = 0; i < 3; i++) {
        int idx = (va >> (12 + (3 - i) * 9)) & 0x1FF;
        if (!(pgt[idx] & PTE_VALID)) {
            void *p;
            /* FIXME Free allocated pages and restore modified pgt */
            if (alloc && (p = kalloc())) {
                memset(p, 0, PGSIZE);
                pgt[idx] = V2P(p) | PTE_TABLE;
            } else {
                warn("failed: pgt[%d]: 0x%x", idx, pgt[idx]);
                return 0;
            }
        }
        pgt = P2V(PTE_ADDR(pgt[idx]));
    }
    return &pgt[(va >> 12) & 0x1FF];
}

/* Fork a process's page table. */
uint64_t *
uvm_copy(uint64_t * pgdir)
{
    uint64_t *newpgdir = vm_init();
    if (!newpgdir)
        return 0;

    for (int i = 0; i < 512; i++)
        if (pgdir[i] & PTE_VALID) {
            assert(pgdir[i] & PTE_TABLE);
            uint64_t *pgt1 = P2V(PTE_ADDR(pgdir[i]));
            for (int i1 = 0; i1 < 512; i1++)
                if (pgt1[i1] & PTE_VALID) {
                    assert(pgt1[i1] & PTE_TABLE);
                    uint64_t *pgt2 = P2V(PTE_ADDR(pgt1[i1]));
                    for (int i2 = 0; i2 < 512; i2++)
                        if (pgt2[i2] & PTE_VALID) {
                            assert(pgt2[i2] & PTE_TABLE);
                            uint64_t *pgt3 = P2V(PTE_ADDR(pgt2[i2]));
                            for (int i3 = 0; i3 < 512; i3++)
                                if (pgt3[i3] & PTE_VALID) {

                                    assert(pgt3[i3] & PTE_PAGE);
                                    assert(pgt3[i3] & PTE_USER);
                                    assert(pgt3[i3] & PTE_NORMAL);

                                    assert(PTE_ADDR(pgt3[i3]) < KERNBASE);

                                    uint64_t pa = PTE_ADDR(pgt3[i3]);
                                    uint64_t va =
                                          (uint64_t)  i << (12 + 9 * 3)
                                        | (uint64_t) i1 << (12 + 9 * 2)
                                        | (uint64_t) i2 << (12 + 9)
                                        | (uint64_t) i3 << 12;
// mmapされたアドレスでMAP_SHAREDの場合は親のpaをそのまま使用。
// それ以外は新規paに親のpaをコピーして使用
                                    struct vma *vma = find_vma(thisproc(), (void *)va);
                                    void *np;
                                    if (vma && vma->flags & MAP_SHARED) {
                                        np = P2V(pa);
                                        inc_kmem_ref(np);
                                    } else {
                                        np = kalloc();
                                        if (np == 0) {
                                            vm_free(newpgdir);
                                            warn("kalloc failed");
                                            return 0;
                                        }
                                        memmove(np, P2V(pa), PGSIZE);
                                    }
                                    // disb();
                                    // Flush to memory to sync with icache.
                                    // dccivac(P2V(pa), PGSIZE);
                                    // disb();
                                    if (uvm_map
                                        (newpgdir, (void *)va, PGSIZE,
                                         V2P((uint64_t) np)) < 0) {
                                        vm_free(newpgdir);
                                        kfree(np);
                                        warn("uvm_map failed");
                                        return 0;
                                    }
                                }
                        }
                }
        }
    return newpgdir;
}

/* Free a user page table and all the physical memory pages. */
void
vm_free(uint64_t * pgdir)
{
    for (int i = 0; i < 512; i++)
        if (pgdir[i] & PTE_VALID) {
            assert(pgdir[i] & PTE_TABLE);
            uint64_t *pgt1 = P2V(PTE_ADDR(pgdir[i]));
            for (int i = 0; i < 512; i++)
                if (pgt1[i] & PTE_VALID) {
                    assert(pgt1[i] & PTE_TABLE);
                    uint64_t *pgt2 = P2V(PTE_ADDR(pgt1[i]));
                    for (int i = 0; i < 512; i++)
                        if (pgt2[i] & PTE_VALID) {
                            assert(pgt2[i] & PTE_TABLE);
                            uint64_t *pgt3 = P2V(PTE_ADDR(pgt2[i]));
                            for (int i = 0; i < 512; i++)
                                if (pgt3[i] & PTE_VALID) {
                                    uint64_t *p = P2V(PTE_ADDR(pgt3[i]));
                                    kfree(p);
                                }
                            kfree(pgt3);
                        }
                    kfree(pgt2);
                }
            kfree(pgt1);
        }
    kfree(pgdir);
}

/*
 * paから始まる物理アドレスを参照するvaから始まる
 * 仮想アドレスのPTEを作成する. vaとsizeはページ
 * アラインされていない場合がある。失敗したら -1,
 * それ以外は0を返す.
 */
int
uvm_map(uint64_t * pgdir, void *va, size_t sz, uint64_t pa)
{
    void *p = ROUNDDOWN(va, PGSIZE), *end = va + sz;
    assert(pa < USERTOP);
    pa = ROUNDDOWN(pa, PGSIZE);
    for (; p < end; pa += PGSIZE, p += PGSIZE) {
        debug("p: 0x%llx, pa: 0x%llx, end: 0x%llx", p, pa, end);
        uint64_t *pte = pgdir_walk(pgdir, p, 1);
        debug("pte: 0x%llx, *pte: 0x%llx", pte, *pte);
        if (!pte) {
            warn("walk failed");
            return -1;
        }
        if (*pte & PTE_VALID)
            panic("remap.");
        *pte = pa | PTE_UDATA;
    }
    return 0;
}

/*
 * Allocate page tables and physical memory to grow process
 * from oldsz to newsz, which need not be page aligned.
 * Stack size stksz should be page aligned.
 * Returns new size or 0 on error.
 */
int
uvm_alloc(uint64_t * pgdir, size_t base, size_t stksz, size_t oldsz,
          size_t newsz)
{
    assert(stksz % PGSIZE == 0);
    if (!(stksz < USERTOP &&
          base <= oldsz && oldsz <= newsz && newsz < USERTOP - stksz)) {

        warn("invalid arg");
        return 0;
    }

    for (size_t a = ROUNDUP(oldsz, PGSIZE); a < newsz; a += PGSIZE) {
        void *p = kalloc();
        if (p == 0) {
            warn("kalloc failed");
            uvm_dealloc(pgdir, base, newsz, oldsz);
            return 0;
        }
        if (uvm_map(pgdir, (void *)a, PGSIZE, V2P((uint64_t) p)) < 0) {
            warn("uvm_map failed");
            kfree(p);
            uvm_dealloc(pgdir, base, newsz, oldsz);
            return 0;
        }
    }

    return newsz;
}

/*
 * vaから始まるマッピングをnpagesページ削除する。vaはページアライン
 * されていなければならない。マッピングは存在しなければならない。
 * オプションで物理メモリを解放する。
 */
void uvm_unmap(uint64_t *pgdir, uint64_t va, uint64_t npages, int do_free)
{
    uint64_t a;
    uint64_t *pte;

    trace("papetable: 0x%llx, va: 0x%llx, np: %lld, free: %d", pgdir, va, npages, do_free);

    if ((va % PGSIZE) != 0)
        panic("uvm_unmap: not aligned");

    for (a = va; a < va + npages*PGSIZE; a += PGSIZE) {
        if ((pte = pgdir_walk(pgdir, (void *)a, 0)) == 0) {
            trace("no pte for va: 0x%lx", a);
            continue;
        }

        // uvm_copyと同じ理由
        if ((pte && *pte & PTE_VALID) == 0) {
            trace("[%d] pte: %p, *pte: 0x%lx", a, pte, *pte);
            continue;
        }
        if ((PTE_FLAGS(*pte) & 0x3ff) == PTE_VALID)
            panic("uvm_unmap: not a leaf");
        if (do_free) {
            uint64_t pa = PTE_ADDR(*pte);
            kfree(P2V(pa));
        }
        *pte = 0;
    }
}

/*
 * Deallocate user pages to bring the process size from oldsz to
 * newsz.  oldsz and newsz need not be page-aligned, nor does newsz
 * need to be less than oldsz.  oldsz can be larger than the actual
 * process size.  Returns the new process size.
 */
int
uvm_dealloc(uint64_t * pgdir, size_t base, size_t oldsz, size_t newsz)
{
    if (newsz >= oldsz || newsz < base)
        return oldsz;

    for (size_t a = ROUNDUP(newsz, PGSIZE); a < oldsz; a += PGSIZE) {
        uint64_t *pte = pgdir_walk(pgdir, (char *)a, 0);
        if (pte && (*pte & PTE_VALID)) {
            uint64_t pa = PTE_ADDR(*pte);
            assert(pa);
            kfree(P2V(pa));
            *pte = 0;
        } else {
            warn("attempt to free unallocated page");
        }
    }
    return newsz;
}

void
uvm_switch(uint64_t * pgdir)
{
    // FIXME: Use NG and ASID for efficiency.
    lttbr0(V2P(pgdir));
}

/*
 * Copy len bytes from p to user address va in page table pgdir.
 * Allocate physical pages if required.
 * Most useful when pgdir is not the current page table.
 */
int
copyout(uint64_t * pgdir, void *va, void *p, size_t len)
{
    void *page;
    size_t n, pgoff;
    uint64_t *pte;
    if ((size_t)va + len > USERTOP)
        return -1;
    for (; len; len -= n, va += n) {
        pgoff = va - ROUNDDOWN(va, PGSIZE);
        if ((pte = pgdir_walk(pgdir, va, 1)) == 0)
            return -1;
        if (*pte & PTE_VALID) {
            page = P2V(PTE_ADDR(*pte));
        } else {
            if ((page = kalloc()) == 0)
                return -1;
            *pte = V2P(page) | PTE_UDATA;
        }
        n = MIN(PGSIZE - pgoff, len);
        if (p) {
            memmove(page + pgoff, p, n);
            p += n;
        } else
            memset(page + pgoff, 0, n);
        // disb();
        // Flush to memory to sync with icache.
        // dccivac(page + pgoff, n);
        // disb();
    }
    return 0;
}

void
vm_stat(uint64_t * pgdir)
{
    debug("pgdir: 0x%p", pgdir);
    uint64_t va_start = 0, va_end = 0;

    for (int i = 0; i < 512; i++)
        if (pgdir[i] & PTE_VALID) {
            assert(pgdir[i] & PTE_TABLE);
            uint64_t *pgt1 = P2V(PTE_ADDR(pgdir[i]));
            for (int i1 = 0; i1 < 512; i1++)
                if (pgt1[i1] & PTE_VALID) {
                    assert(pgt1[i1] & PTE_TABLE);
                    uint64_t *pgt2 = P2V(PTE_ADDR(pgt1[i1]));
                    for (int i2 = 0; i2 < 512; i2++)
                        if (pgt2[i2] & PTE_VALID) {
                            assert(pgt2[i2] & PTE_TABLE);
                            uint64_t *pgt3 = P2V(PTE_ADDR(pgt2[i2]));
                            for (int i3 = 0; i3 < 512; i3++)
                                if (pgt3[i3] & PTE_VALID) {

                                    assert(pgt3[i3] & PTE_PAGE);
                                    assert(pgt3[i3] & PTE_USER);
                                    assert(pgt3[i3] & PTE_NORMAL);

                                    assert(PTE_ADDR(pgt3[i3]) < KERNBASE);

                                    uint64_t *p = P2V(PTE_ADDR(pgt3[i3]));
                                    uint64_t va =
                                        (uint64_t) i << (12 +
                                                         9 *
                                                         3) | (uint64_t) i1
                                        << (12 +
                                            9 * 2) | (uint64_t) i2 << (12 +
                                                                       9) |
                                        i3 << 12;
                                    debug
                                        ("va: 0x%p, pa: 0x%p, pte: 0x%p, PTE_ADDR(pte): 0x%p, P2V(...): 0x%p",
                                         va, p, pgt3[i3],
                                         PTE_ADDR(pgt3[i3]),
                                         P2V(PTE_ADDR(pgt3[i3])));

                                    if (va == va_end)
                                        va_end = va + PGSIZE;
                                    else {
                                        if (va_start < va_end)
                                            debug("va: [0x%p ~ 0x%p)",
                                                  va_start, va_end);

                                        va_start = va;
                                        va_end = va + PGSIZE;
                                    }
                                }
                        }
                }
        }
    if (va_start < va_end) {
        debug("va: [0x%p ~ 0x%p)", va_start, va_end);
    }
}


void
vm_test()
{
#ifdef DEBUG
    void *pgdir = vm_init();
    void *p = kalloc(), *p2 = kalloc(), *va = (void *)0x1000;
    memset(p, 0xAB, PGSIZE);
    memset(p2, 0xAC, PGSIZE);
    uvm_map(pgdir, va, PGSIZE, V2P((uint64_t) p));
    uvm_switch(pgdir);
    for (char *i = va; (void *)i < va + PGSIZE; i++) {
        assert(*i == 0xAB);
    }
    uvm_dealloc(pgdir, (size_t)va, (size_t)va + PGSIZE, (size_t)va);

    uvm_map(pgdir, va, PGSIZE, V2P((uint64_t) p2));
    uvm_switch(pgdir);
    for (char *i = va; (void *)i < va + PGSIZE; i++) {
        assert(*i == 0xAC);
    }
    vm_free(pgdir);
    info("pass");
#endif
}
