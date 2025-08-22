#ifndef INC_MEMLAYOUT_H
#define INC_MEMLAYOUT_H

/* Stack must always be 16 bytes aligned. */
#define KSTACKSIZE 4096
#define USTACKSIZE 4096

// Deprecated: use mbox_get_arm_memory() instead
// #define PHYSTOP 0x3E000000            /* Top physical memory */

#define KERNBASE 0xFFFF000000000000   /* First kernel virtual address */
#define KERNLINK (KERNBASE+0x80000)   /* Address where kernel is linked */
#define USERTOP  0x0001000000000000   /* Top address of user space. */

#define V2P_WO(x) ((x) - KERNBASE)    /* Same as V2P, but without casts */
#define P2V_WO(x) ((x) + KERNBASE)    /* Same as P2V, but without casts */

/*
 * カーネルメモリレイアウト:
 * ```
 *  PHYSTOP       -> ----------------------------   0xffff_0000_3E00_0000
 *  (0x3E000000)
 *  
 *                           heap                   RW-
 *  PAGE_START    -> ----------------------------   0xffff_0000_0066_0000
 *                          pages[PAGE_NUM]         RW-
 *  *pages        -> ----------------------------   0xffff_0000_000c_9600
 *  end           -> ----------------------------   0xffff_0000_0009_f268
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
 *
 *                         ヒープ領域
 *                       ガードページ (4KB)
 *                    ---------------------------
 *                         コード領域
 *  SEGMENT_START -> ---------------------------    0x0000_0000_0040_0000
 */


#ifndef __ASSEMBLER__

#include <stdint.h>
#define V2P(a) (((uint64_t) (a)) - KERNBASE)
#define P2V(a) ((void *)(((char *) (a)) + KERNBASE))

#endif

#endif
