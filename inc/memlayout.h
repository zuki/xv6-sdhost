#ifndef INC_MEMLAYOUT_H
#define INC_MEMLAYOUT_H

/* Stack must always be 16 bytes aligned. */
#define KSTACKSIZE 4096
#define USTACKSIZE 4096

// Deprecated: use mbox_get_arm_memory() instead
// #define PHYSTOP 0x3E000000            /* Top physical memory */

#define KERNBASE 0xFFFF000000000000UL   /* First kernel virtual address */
#define KERNLINK (KERNBASE+0x80000UL)   /* Address where kernel is linked */


#define V2P_WO(x) ((x) - KERNBASE)    /* Same as V2P, but without casts */
#define P2V_WO(x) ((x) + KERNBASE)    /* Same as P2V, but without casts */

/*
 * カーネルメモリレイアウト:
 * ```
 *  PHYSTOP       -> ----------------------------   0xffff_0000_3E00_0000
 *  (0x3E000000)
 *
 *                           heap                   RW-
 *  PAGE_START    -> ----------------------------   0xffff_0000_0100_0000
 *                          pages[PAGE_NUM]         RW-
 *                      PAGE_NUM = 0x3c000
 *  *pages        -> ----------------------------   0xffff_0000_00a6_0000
 *  end           -> ----------------------------   0xffff_0000_0033_cbd8
 *                    カーネル rodata, data, bss    RW-
 *                   ----------------------------
 *                         カーネル text            R-X
 *  KERNLINK      -> ----------------------------   0xFFFF_0000_0008_0000
 *
 *  KERNBASE      -> ----------------------------   0xFFFF_0000_0000_0000
 *
 */

/* ユーザメモリレイアウト.
 *  USERTOP        -> ---------------------------   0x0001_0000_0000_0000
 *                        スタック (16 * 4KB)
 *                                                  <- sp
 *  STACKBASE      -> ---------------------------   0x0000_FFFF_FFFF_0000
 *
 *                         mmap領域 (128GB)
 *  MMAPBASE         -> --------------------------- 0x0000_FFDF_FFFF_0000
 *
 *                         libc.so (4GB)
 *  ELF_ET_DYN_BASE  -> --------------------------- 0x0000_FFDE_FFFF_0000
 *
 *
 *                         ヒープ領域
 *                       ガードページ (4KB)
 *                    ---------------------------
 *                         コード領域
 *  SEGMENT_START -> ---------------------------    0x0000_0000_0040_0000
 */

#define USERTOP     0x0001000000000000UL    /* Top address of user space. */
#define STACKPAGE   16
#define STACKTOP    USERTOP                 /* スタックはUSERTOPから */
#define STACKBASE   (STACKTOP - STACKPAGE * PGSIZE) /* スタックは16ページ */
#define MMAPBASE    0x0000FFDFFFFF0000      /* mmapアドレスの基底アドレス */
#define ELF_ET_DYN_BASE 0x0000FFDEFFFF0000  /* dynamic lib を置くアドレス */
#define MMAPTOP     USERTOP                 /* ARGV, ENVPなどはstack_topからmmapするため */


#ifndef __ASSEMBLER__

#include <types.h>
#define V2P(a) (((uint64_t) (a)) - KERNBASE)
#define P2V(a) ((void *)(((char *) (a)) + KERNBASE))

#endif

#endif
