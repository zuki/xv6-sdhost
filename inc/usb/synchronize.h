//
// synchronize64.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright(C) 2014-2020  R. Stange <rsta2@o2online.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef INC_USB_SYNCHRONIZE_H
#define INC_USB_SYNCHRONIZE_H

#include <types.h>

#define ARM_ALLOW_MULTI_CORE    1

#define CORES                   4

//
// 実行レベル: どの割り込みが有効になっているかを示す
//
#define TASK_LEVEL      0        // IRQもFIQも有効
#define IRQ_LEVEL       1        // FIQは有効だがIRQは無効
#define FIQ_LEVEL       2        // FIQもIRQも無効

unsigned current_execution_level(void);

// EnterCritical()は対象レベルが同じか大きいネストは可能
void enter_critical(unsigned level);    // Default: IRQ_LEVEL
void leave_critical(void);

//
// キャッシュ制御
//
#define DATA_CACHE_LINE_LENGTH_MIN      64    // CTR_EL0から
#define DATA_CACHE_LINE_LENGTH_MAX      64

#define invalidate_instruction_cache()    asm volatile("ic iallu" ::: "memory")
#define flush_prefetch_buffer()           asm volatile("isb" ::: "memory")

// 注: データキャッシュの操作にはDataSyncBarrierを含む
void invalidate_data_cache(void) MAXOPT;
void invalidate_data_cache_l1_only(void) MAXOPT;
void clean_data_cache(void) MAXOPT;

void invalidate_data_cache_range(uint64_t nAddress, uint64_t nLength) MAXOPT;
void clean_data_cache_range(uint64_t nAddress, uint64_t nLength) MAXOPT;
void clean_and_invalidate_data_cache_range(uint64_t nAddress, uint64_t nLength) MAXOPT;

void sync_data_and_instruction_cache(void);

//
// キャッシュアライメント
//
#define CACHE_ALIGN                     GALIGN(DATA_CACHE_LINE_LENGTH_MAX)

#define CACHE_ALIGN_SIZE(type, num)    ((( ((num)*sizeof(type) - 1)        \
                            |(DATA_CACHE_LINE_LENGTH_MAX-1)    \
                        ) + 1) / sizeof(type))

#define IS_CACHE_ALIGNED(ptr, size)    (((uintptr_t)(ptr) &(DATA_CACHE_LINE_LENGTH_MAX-1)) == 0 \
                        &&((size) &(DATA_CACHE_LINE_LENGTH_MAX-1)) == 0)

#define DMA_BUFFER(type, name, num)     type name[CACHE_ALIGN_SIZE(type, num)] CACHE_ALIGN

#endif
